/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 *****************************************************************************/

/**
 * @file
 * @brief C implementation of raster_value() — PostGIS raster band sampling
 * along tgeompoint trajectories.
 *
 * The function iterates over the instants of the trajectory in C, filters
 * out-of-range positions with a bounding-box pre-check derived from the
 * raster's convex hull, calls PostGIS ST_Value() for each in-range instant,
 * and assembles the surviving (value, timestamp) pairs directly into a
 * heap-allocated TSequence (DISCRETE interpolation), bypassing the SQL
 * string_agg / to_char / text→tfloat pipeline.
 *
 * PostGIS raster internals (rt_api.h) are not exported from
 * postgis_raster-3.so.  The implementation therefore calls the published SQL
 * surface (ST_ConvexHull, ST_Value) via PostgreSQL's OidFunctionCall
 * mechanism, resolving OIDs once per session through regprocedurein.
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/*
 * utils/builtins.h pulls in fmgrprotos.h, which declares `json_object` as a
 * PostgreSQL callable function.  meos_internal.h then includes json-c/json.h
 * which tries to typedef the same name as a struct — a C-level conflict.
 * Forward-declare only the symbols we need to avoid the full builtins.h include.
 */

extern Datum regprocedurein(PG_FUNCTION_ARGS);

/* PostgreSQL */
#include <utils/array.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_raster.h>
#include <pgtypes.h>
#include "temporal/span.h"    /* PG_GETARG_SPAN_P */
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h" /* bstring2bytea */
#include "geo/stbox.h"        /* PG_RETURN_STBOX_P */
#include "raster/raquet.h"    /* Raquet, PG_GETARG_RAQUET_P, raquet_pixtype_size */
#include "raster/raster_quadbin.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h" /* raquetarr_extract */
#include "pg_raster/temporal_raster.h"

/*****************************************************************************
 * OID cache helpers
 *****************************************************************************/

/**
 * @brief Return the OID of a function identified by its SQL signature string.
 *
 * Uses regprocedurein so that type names are resolved in the current search
 * path without hard-coding any numeric OIDs.
 */
static Oid
lookup_func_oid(const char *signature)
{
  return DatumGetObjectId(DirectFunctionCall1(regprocedurein,
    CStringGetDatum(signature)));
}

/** Cached OID for ST_ConvexHull(raster) → geometry */
static Oid st_convexhull_raster_oid = InvalidOid;

/** Cached OID for ST_Value(raster, integer, geometry, boolean) → float8 */
static Oid st_value_oid = InvalidOid;

static void
init_oids(void)
{
  if (st_convexhull_raster_oid == InvalidOid)
    st_convexhull_raster_oid = lookup_func_oid("st_convexhull(raster)");
  if (st_value_oid == InvalidOid)
    st_value_oid =
      lookup_func_oid("st_value(raster,integer,geometry,boolean,text)");
}

/*****************************************************************************
 * raster_value
 *****************************************************************************/

/**
 * @brief Raster sampling callback backed by PostGIS raster's ST_Value: the
 * context is a prepared FunctionCallInfo whose raster/band/nodata/resample
 * arguments are constant, only the point argument varies per call
 */
static bool
raster_st_value_sample(void *ctx, const GSERIALIZED *point, double *value)
{
  FunctionCallInfo fcinfo_val = (FunctionCallInfo) ctx;
  fcinfo_val->args[2].value = PointerGetDatum(point);
  fcinfo_val->args[2].isnull = false;
  fcinfo_val->isnull = false;   /* reset before every call */
  Datum pixval = FunctionCallInvoke(fcinfo_val);
  if (fcinfo_val->isnull)
    return false;   /* nodata pixel, or point outside the raster or the band */
  *value = DatumGetFloat8(pixval);
  return true;
}

/**
 * @brief Build the convex-hull STBox pre-filter and the reusable ST_Value
 * FunctionCallInfo shared by every raster sampling/restriction/predicate
 * wrapper
 * @details Arg 2 (the point) of @p fcinfo_val is left uninitialized: it is
 * set on every call by #raster_st_value_sample. The FmgrInfo backing
 * @p fcinfo_val is palloc'd so that its lifetime matches the calling
 * wrapper's memory context, same as the LOCAL_FCINFO buffer itself.
 */
static void
raster_value_setup(Datum rast_datum, int32 band, STBox *box,
  FunctionCallInfo fcinfo_val)
{
  /* OID resolution (once per session) */
  init_oids();

  /* Raster convex hull — the bounding-box pre-filter of the MEOS kernel */
  Datum hull_datum =
    OidFunctionCall1(st_convexhull_raster_oid, rast_datum);
  GSERIALIZED *hull_gs = (GSERIALIZED *) DatumGetPointer(hull_datum);
  GBOX hull_box;
  gserialized_get_gbox_p(hull_gs, &hull_box);
  pfree(hull_gs);
  memset(box, 0, sizeof(STBox));
  box->xmin = hull_box.xmin;
  box->xmax = hull_box.xmax;
  box->ymin = hull_box.ymin;
  box->ymax = hull_box.ymax;

  /* Prepare a reusable FunctionCallInfo for ST_Value (handles NULL returns) */
  FmgrInfo *flinfo = palloc0(sizeof(FmgrInfo));
  fmgr_info(st_value_oid, flinfo);
  InitFunctionCallInfoData(*fcinfo_val, flinfo, 5, DEFAULT_COLLATION_OID,
    NULL, NULL);
  /* Arg 0 (raster) and Arg 1 (band) are constant for every instant */
  fcinfo_val->args[0].value = rast_datum;
  fcinfo_val->args[0].isnull = false;
  fcinfo_val->args[1].value = Int32GetDatum(band);
  fcinfo_val->args[1].isnull = false;
  /* Arg 3 (exclude_nodata) = true — ST_Value reports a nodata pixel as NULL,
   * which #raster_st_value_sample turns into "no value here", as the
   * #raster_sample_fn contract requires. With false the sentinel comes back as
   * an ordinary number and is sampled as though it were data */
  fcinfo_val->args[3].value  = BoolGetDatum(true);
  fcinfo_val->args[3].isnull = false;
  /* Arg 4 (resample) = 'nearest' — PostGIS 3.6+ added this parameter */
  fcinfo_val->args[4].value = PointerGetDatum(cstring_to_text("nearest"));
  fcinfo_val->args[4].isnull = false;
}

PGDLLEXPORT Datum Raster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory
 * @param[in] rast Raster
 * @param[in] traj Trajectory
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn #rasterValue()
 */
Datum
Raster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Datum rast_datum = PG_GETARG_DATUM(1);
  int32 band = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);

  STBox box;
  LOCAL_FCINFO(fcinfo_val, 5);
  raster_value_setup(rast_datum, band, &box, fcinfo_val);

  Temporal *result = raster_value(traj, &box, &raster_st_value_sample,
    fcinfo_val);

  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * atRasterValue / minusRasterValue / eRasterValue / aRasterValue
 *****************************************************************************/

PGDLLEXPORT Datum Raster_at_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_at_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls inside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range (inclusive bounds)
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn #atRasterValue()
 */
Datum
Raster_at_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Datum rast_datum = PG_GETARG_DATUM(1);
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  STBox box;
  LOCAL_FCINFO(fcinfo_val, 5);
  raster_value_setup(rast_datum, band, &box, fcinfo_val);

  Temporal *result = raster_at_value(traj, &box, &raster_st_value_sample,
    fcinfo_val, vspan);

  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_minus_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_minus_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the instants of a trajectory where the sampled raster pixel
 * value falls outside a float range
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] rast Raster
 * @param[in] vspan Float value range to exclude
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn #minusRasterValue()
 */
Datum
Raster_minus_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Datum rast_datum = PG_GETARG_DATUM(1);
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  STBox box;
  LOCAL_FCINFO(fcinfo_val, 5);
  raster_value_setup(rast_datum, band, &box, fcinfo_val);

  Temporal *result = raster_minus_value(traj, &box, &raster_st_value_sample,
    fcinfo_val, vspan);

  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Eraster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eraster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if a trajectory ever samples a raster pixel value
 * inside a float range
 * @param[in] rast Raster
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn #eRasterValue()
 */
Datum
Eraster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Datum rast_datum = PG_GETARG_DATUM(1);
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  STBox box;
  LOCAL_FCINFO(fcinfo_val, 5);
  raster_value_setup(rast_datum, band, &box, fcinfo_val);

  int result = eraster_value(traj, &box, &raster_st_value_sample,
    fcinfo_val, vspan);

  PG_FREE_IF_COPY(traj, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Araster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Araster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if every in-raster-extent instant of a trajectory
 * samples a pixel value inside a float range
 * @param[in] rast Raster
 * @param[in] traj Trajectory (SRID matching the raster)
 * @param[in] vspan Float value range
 * @param[in] band Band number (1-based, default 1)
 * @sqlfn #aRasterValue()
 */
Datum
Araster_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Datum rast_datum = PG_GETARG_DATUM(1);
  Span *vspan = PG_GETARG_SPAN_P(2);
  int32 band = PG_ARGISNULL(3) ? 1 : PG_GETARG_INT32(3);

  STBox box;
  LOCAL_FCINFO(fcinfo_val, 5);
  raster_value_setup(rast_datum, band, &box, fcinfo_val);

  int result = araster_value(traj, &box, &raster_st_value_sample,
    fcinfo_val, vspan);

  PG_FREE_IF_COPY(traj, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * raster_num_bands
 *****************************************************************************/

PGDLLEXPORT Datum Raster_num_bands(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_num_bands);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the number of bands of a raster
 * @param[in] rast Raster
 * @sqlfn numBands()
 */
Datum
Raster_num_bands(PG_FUNCTION_ARGS)
{
  Datum rast_datum = PG_GETARG_DATUM(0);
  Raster *rast = (Raster *) PG_DETOAST_DATUM(rast_datum);
  int result = raster_num_bands(rast);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * raster_tile_value_quadbin
 *****************************************************************************/

/** Map a pixtype name text argument to a MeosPixType code. */
static MeosPixType
text_to_pixtype(const text *pt)
{
  char *s = text_to_cstring(pt);
  MeosPixType result = raquet_pixtype_from_string(s);
  pfree(s);
  return result;
}

PGDLLEXPORT Datum Raster_tile_value_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value_quadbin);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample a Raquet raster chip along a tgeompoint trajectory
 * @param[in] traj Trajectory (tgeompoint, SRID 4326)
 * @param[in] pixels Row-major pixel bytes (bytea)
 * @param[in] width Tile width in pixels
 * @param[in] height Tile height in pixels
 * @param[in] quadbin CARTO QUADBIN cell (bigint)
 * @param[in] pixtype Pixel type name: UINT8, INT16, INT32, FLOAT32, FLOAT64
 * @param[in] nodata Nodata sentinel value
 * @param[in] has_nodata  Enable nodata filtering
 * @sqlfn rasterTileValueQuadbin()
 */
Datum
Raster_tile_value_quadbin(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  bytea *pxbytea = PG_GETARG_BYTEA_PP(1);
  int32 width = PG_GETARG_INT32(2);
  int32 height = PG_GETARG_INT32(3);
  int64 quadbin = PG_GETARG_INT64(4);
  text *pixtype_t = PG_GETARG_TEXT_PP(5);
  float8 nodata = PG_GETARG_FLOAT8(6);
  bool has_nd = PG_GETARG_BOOL(7);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  size_t pixels_size = (size_t) VARSIZE_ANY_EXHDR(pxbytea);
  MeosPixType pixtype = text_to_pixtype(pixtype_t);

  Temporal *result = raster_tile_value_quadbin(traj, pixels, pixels_size,
    (uint16_t) width, (uint16_t) height, (uint64) quadbin,
    pixtype, nodata, has_nd);

  PG_FREE_IF_COPY(traj, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Raquet type: input/output, constructor, and typed sampling
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_in);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile from its HexWKB representation
 * @sqlfn raquet_in()
 */
Datum
Raquet_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_RAQUET_P(raquet_in(str));
}

PGDLLEXPORT Datum Raquet_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_out);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the HexWKB representation of a Raquet tile
 * @sqlfn raquet_out()
 */
Datum
Raquet_out(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  char *result = raquet_out(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_CSTRING(result);
}

PGDLLEXPORT Datum Raquet_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_recv);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile from its Well-Known Binary (WKB) representation
 * @sqlfn raquet_recv()
 */
Datum
Raquet_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Raquet *result = raquet_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_send);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the Well-Known Binary (WKB) representation of a Raquet tile
 * @sqlfn raquet_send()
 */
Datum
Raquet_send(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  size_t wkb_size;
  uint8_t *wkb = raquet_as_wkb(rq, (uint8_t) WKB_NDR, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_BYTEA_P(result);
}

PGDLLEXPORT Datum Raquet_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_constructor);
/**
 * @ingroup mobilitydb_raster
 * @brief Construct a Raquet tile from a QUADBIN cell, dimensions, a pixel type
 * name and a row-major packed pixel array
 * @param[in] pixels   Row-major pixel bytes (bytea)
 * @param[in] width    Tile width in pixels
 * @param[in] height   Tile height in pixels
 * @param[in] quadbin  CARTO QUADBIN cell (bigint)
 * @param[in] pixtype  Pixel type name: UINT8 | INT16 | INT32 | FLOAT32 | FLOAT64
 * @param[in] nodata   Nodata sentinel value (NULL disables nodata filtering)
 * @sqlfn raquet()
 */
Datum
Raquet_constructor(PG_FUNCTION_ARGS)
{
  /* Non-strict: the nodata argument (5) may be NULL to disable nodata */
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) ||
      PG_ARGISNULL(3) || PG_ARGISNULL(4))
    PG_RETURN_NULL();
  bytea *pxbytea = PG_GETARG_BYTEA_PP(0);
  int32 width = PG_GETARG_INT32(1);
  int32 height = PG_GETARG_INT32(2);
  int64 quadbin = PG_GETARG_INT64(3);
  text *pixtype_t = PG_GETARG_TEXT_PP(4);
  bool has_nd = ! PG_ARGISNULL(5);
  float8 nodata = has_nd ? PG_GETARG_FLOAT8(5) : 0.0;
  MeosPixType pixtype = text_to_pixtype(pixtype_t);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  Raquet *result = raquet_make((uint64) quadbin, width, height, pixtype,
    nodata, has_nd, pixels, (size_t) VARSIZE_ANY_EXHDR(pxbytea));
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raquet_read(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_read);
/**
 * @ingroup mobilitydb_raster
 * @brief Return a Raquet tile decoded from an in-memory raster file via GDAL
 * @details The raster file is supplied as bytes in any GDAL-supported format
 * and decoded through GDAL's `/vsimem/` virtual filesystem, so no server-side
 * file access is required.
 * @param[in] rasterfile Raster file bytes (bytea)
 * @param[in] quadbin CARTO QUADBIN cell (bigint), or NULL to derive it from the
 * raster geotransform and EPSG:3857 spatial reference
 * @sqlfn raquetRead()
 */
Datum
Raquet_read(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0))
    PG_RETURN_NULL();
  bytea *rasterfile = PG_GETARG_BYTEA_PP(0);
  /* A NULL quadbin requests deriving the tile identifier from the raster
   * geotransform; raquet_read_bytes treats 0 as that request */
  uint64 quadbin = PG_ARGISNULL(1) ? 0 : (uint64) PG_GETARG_INT64(1);
  const uint8_t *data = (const uint8_t *) VARDATA_ANY(rasterfile);
  size_t size = VARSIZE_ANY_EXHDR(rasterfile);
  Raquet *result = raquet_read_bytes(data, size, quadbin);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raster_tile_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample a Raquet tile along a tgeompoint trajectory
 * @param[in] traj Trajectory (tgeompoint)
 * @param[in] rq Raquet tile
 * @sqlfn rasterTileValue()
 */
Datum
Raster_tile_value(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  Raquet *rq = PG_GETARG_RAQUET_P(1);
  Temporal *result = raster_tile_value(traj, rq);
  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(rq, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Raster_tile_value_array(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value_array);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample an array of Raquet tiles along a tgeompoint trajectory
 * @param[in] traj Trajectory (tgeompoint)
 * @param[in] rqarr Array of Raquet tiles
 * @sqlfn rasterTileValue()
 */
Datum
Raster_tile_value_array(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(1);
  ensure_not_empty_array(array);
  int count;
  Raquet **rqarr = raquetarr_extract(array, &count);
  Temporal *result = raster_tile_value_array(traj, (const Raquet **) rqarr,
    count);
  pfree(rqarr);
  PG_FREE_IF_COPY(traj, 0);
  PG_FREE_IF_COPY(array, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * trajectory_quadbins
 *****************************************************************************/

PGDLLEXPORT Datum Trajectory_quadbins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trajectory_quadbins);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the distinct QUADBIN cells at a zoom level covered by a
 * trajectory, for use as a WHERE-clause join key against a Raquet table
 * @param[in] traj  Trajectory (tgeompoint, SRID 4326)
 * @param[in] zoom  QUADBIN zoom level (0–15)
 * @sqlfn #quadbins()
 */
Datum
Trajectory_quadbins(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  int32     zoom = PG_GETARG_INT32(1);

  int       ncells;
  uint64   *cells = trajectory_quadbins(traj, (uint32_t) zoom, &ncells);

  PG_FREE_IF_COPY(traj, 0);

  /* Build int8[] (bigint[]) from the uint64 cell array */
  Datum *elems = palloc(sizeof(Datum) * ncells);
  for (int i = 0; i < ncells; i++)
    elems[i] = Int64GetDatum((int64) cells[i]);
  pfree(cells);

  ArrayType *arr = construct_array(elems, ncells, INT8OID, 8, true, TYPALIGN_DOUBLE);
  pfree(elems);

  PG_RETURN_ARRAYTYPE_P(arr);
}

/*****************************************************************************
 * Raquet type: accessors
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_quadbin);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the QUADBIN cell of a Raquet tile
 * @sqlfn quadbin()
 */
Datum
Raquet_quadbin(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  uint64 result = raquet_quadbin(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_INT64((int64) result);
}

PGDLLEXPORT Datum Raquet_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_width);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the width in pixels of a Raquet tile
 * @sqlfn width()
 */
Datum
Raquet_width(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  int result = raquet_width(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raquet_height(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_height);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the height in pixels of a Raquet tile
 * @sqlfn height()
 */
Datum
Raquet_height(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  int result = raquet_height(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Raquet_nodata(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_nodata);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the nodata sentinel value of a Raquet tile
 * @sqlfn nodata()
 */
Datum
Raquet_nodata(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  double result = raquet_nodata(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Raquet_pixtype(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_pixtype);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the name of the pixel data type of a Raquet tile
 * @sqlfn pixtype()
 */
Datum
Raquet_pixtype(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  char *str = raquet_pixtype(rq);
  text *result = cstring_to_text(str);
  pfree(str);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************
 * Raquet type: comparison
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_eq);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the Raquet tiles are equal
 * @sqlfn eq()
 * @sqlop @p =
 */
Datum
Raquet_eq(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_eq(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_ne);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the Raquet tiles are different
 * @sqlfn ne()
 * @sqlop @p <>
 */
Datum
Raquet_ne(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_ne(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_lt);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is less than the second one
 * @sqlfn lt()
 * @sqlop @p <
 */
Datum
Raquet_lt(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_lt(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_le);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is less than or equal to the
 * second one
 * @sqlfn le()
 * @sqlop @p <=
 */
Datum
Raquet_le(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_le(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_ge);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is greater than or equal to the
 * second one
 * @sqlfn ge()
 * @sqlop @p >=
 */
Datum
Raquet_ge(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_ge(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_gt);
/**
 * @ingroup mobilitydb_raster
 * @brief Return true if the first Raquet tile is greater than the second one
 * @sqlfn gt()
 * @sqlop @p >
 */
Datum
Raquet_gt(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  bool result = raquet_gt(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Raquet_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_cmp);
/**
 * @ingroup mobilitydb_raster
 * @brief Return -1, 0, or 1 depending on whether the first Raquet tile is
 * less than, equal to, or greater than the second one
 * @sqlfn cmp()
 */
Datum
Raquet_cmp(PG_FUNCTION_ARGS)
{
  Raquet *rq1 = PG_GETARG_RAQUET_P(0);
  Raquet *rq2 = PG_GETARG_RAQUET_P(1);
  int result = raquet_cmp(rq1, rq2);
  PG_FREE_IF_COPY(rq1, 0); PG_FREE_IF_COPY(rq2, 1);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * Raquet type: hash
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_hash);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the 32-bit hash of a Raquet tile
 * @sqlfn hash()
 */
Datum
Raquet_hash(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  uint32 result = raquet_hash(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Raquet_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_hash_extended);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the 64-bit hash of a Raquet tile using a seed
 * @sqlfn hashExtended()
 */
Datum
Raquet_hash_extended(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = raquet_hash_extended(rq, seed);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************
 * Raquet type: conversions
 *****************************************************************************/

PGDLLEXPORT Datum Raquet_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raquet_to_stbox);
/**
 * @ingroup mobilitydb_raster
 * @brief Convert a Raquet tile into a spatiotemporal box
 * @sqlfn stbox()
 */
Datum
Raquet_to_stbox(PG_FUNCTION_ARGS)
{
  Raquet *rq = PG_GETARG_RAQUET_P(0);
  STBox *result = raquet_to_stbox(rq);
  PG_FREE_IF_COPY(rq, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************/
