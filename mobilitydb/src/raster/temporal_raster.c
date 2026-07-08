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
extern text *cstring_to_text(const char *s);
/* PostgreSQL array support (does not pull in fmgrprotos.h) */
#include <utils/array.h>
/* PostGIS liblwgeom (vendored) */
#include <liblwgeom.h>        /* GSERIALIZED, GBOX, gserialized_get_gbox_p */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>    /* temporal_insts_p, tsequence_make_free */
#include <meos_raster.h>      /* MeosPixType, raster_tile_value_quadbin */
#include "raster/raquet.h"    /* Raquet, PG_GETARG_RAQUET_P, raquet_pixtype_size */
#include "temporal/tinstant.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h" /* bstring2bytea */
/* MEOS raster kernel */
#include "raster/raster_quadbin.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
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
  return DatumGetObjectId(
    DirectFunctionCall1(regprocedurein,
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
    st_convexhull_raster_oid =
      lookup_func_oid("st_convexhull(raster)");
  if (st_value_oid == InvalidOid)
    st_value_oid =
      lookup_func_oid("st_value(raster,integer,geometry,boolean,text)");
}

/*****************************************************************************
 * raster_value
 *****************************************************************************/

PGDLLEXPORT Datum Raster_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Return the values of a raster band sampled at the instants of a
 * trajectory
 * @param[in] rast Raster
 * @param[in] traj Trajectory
 * @param[in] band Band number (1-based, default 1)
 * @csqlfn #Raster_value()
 */
Datum
Raster_value(PG_FUNCTION_ARGS)
{
  /* ── arguments ──────────────────────────────────────────────────────── */
  Datum     rast_datum = PG_GETARG_DATUM(0);
  Temporal *traj       = PG_GETARG_TEMPORAL_P(1);
  int32     band       = PG_ARGISNULL(2) ? 1 : PG_GETARG_INT32(2);

  /* ── OID resolution (once per session) ──────────────────────────────── */
  init_oids();

  /* ── Raster convex hull — used as a bounding-box pre-filter ─────────── */
  Datum hull_datum =
    OidFunctionCall1(st_convexhull_raster_oid, rast_datum);
  GSERIALIZED *hull_gs = (GSERIALIZED *) DatumGetPointer(hull_datum);
  GBOX         hull_box;
  gserialized_get_gbox_p(hull_gs, &hull_box);
  pfree(hull_gs);

  /* ── Iterate over trajectory instants ───────────────────────────────── */
  int count;
  const TInstant **insts = temporal_insts_p(traj, &count);

  TInstant **result_insts = palloc(sizeof(TInstant *) * count);
  int ninsts = 0;

  /* Prepare a reusable FunctionCallInfo for ST_Value (handles NULL returns) */
  FmgrInfo flinfo;
  fmgr_info(st_value_oid, &flinfo);

  LOCAL_FCINFO(fcinfo_val, 5);
  InitFunctionCallInfoData(*fcinfo_val, &flinfo, 5,
                           DEFAULT_COLLATION_OID, NULL, NULL);
  /* Arg 0 (raster) and Arg 1 (band) are constant for every instant */
  fcinfo_val->args[0].value  = rast_datum;
  fcinfo_val->args[0].isnull = false;
  fcinfo_val->args[1].value  = Int32GetDatum(band);
  fcinfo_val->args[1].isnull = false;
  /* Arg 3 (exclude_nodata) = false — return the nodata sentinel as-is so we
   * can detect a NULL and skip the instant, matching the SQL STRICT semantics */
  fcinfo_val->args[3].value  = BoolGetDatum(false);
  fcinfo_val->args[3].isnull = false;
  /* Arg 4 (resample) = 'nearest' — PostGIS 3.6+ added this parameter */
  fcinfo_val->args[4].value  = PointerGetDatum(cstring_to_text("nearest"));
  fcinfo_val->args[4].isnull = false;

  for (int i = 0; i < count; i++)
  {
    Datum geom_datum = tinstant_value(insts[i]);

    /* ── Bounding-box pre-filter ─────────────────────────────────────── */
    GSERIALIZED *pt_gs = (GSERIALIZED *) DatumGetPointer(geom_datum);
    GBOX         pt_box;
    gserialized_get_gbox_p(pt_gs, &pt_box);
    /* For a POINT, pt_box.xmin == xmax and ymin == ymax */
    if (pt_box.xmin < hull_box.xmin || pt_box.xmin > hull_box.xmax ||
        pt_box.ymin < hull_box.ymin || pt_box.ymin > hull_box.ymax)
      continue;

    /* ── ST_Value call with NULL detection ───────────────────────────── */
    fcinfo_val->args[2].value  = geom_datum;
    fcinfo_val->args[2].isnull = false;
    fcinfo_val->isnull         = false;   /* reset before every call */

    Datum pixval = FunctionCallInvoke(fcinfo_val);
    if (fcinfo_val->isnull)
      continue;   /* nodata pixel or geometry outside the pixel grid */

    result_insts[ninsts++] =
      tinstant_make(pixval, T_TFLOAT, insts[i]->t);
  }

  pfree(insts);

  if (ninsts == 0)
  {
    pfree(result_insts);
    PG_FREE_IF_COPY(traj, 1);
    PG_RETURN_NULL();
  }

  TSequence *result =
    tsequence_make_free(result_insts, ninsts, true, true, DISCRETE, NORMALIZE);

  PG_FREE_IF_COPY(traj, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * raster_tile_value_quadbin
 *****************************************************************************/

/** Map a pixtype name text argument to a MeosPixType code. */
static MeosPixType
text_to_pixtype(const text *pt)
{
  const char *s   = VARDATA_ANY(pt);
  int         len = (int) VARSIZE_ANY_EXHDR(pt);
  if (len == 5 && strncmp(s, "UINT8",   5) == 0) return MEOS_PT_UINT8;
  if (len == 5 && strncmp(s, "INT16",   5) == 0) return MEOS_PT_INT16;
  if (len == 5 && strncmp(s, "INT32",   5) == 0) return MEOS_PT_INT32;
  if (len == 7 && strncmp(s, "FLOAT32", 7) == 0) return MEOS_PT_FLOAT32;
  if (len == 7 && strncmp(s, "FLOAT64", 7) == 0) return MEOS_PT_FLOAT64;
  ereport(ERROR,
    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
     errmsg("unknown pixel type \"%.*s\": use UINT8, INT16, INT32, FLOAT32, "
            "or FLOAT64", len, s)));
  return MEOS_PT_UINT8; /* unreachable */
}

PGDLLEXPORT Datum Raster_tile_value_quadbin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value_quadbin);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample a Raquet raster chip along a tgeompoint trajectory
 * @param[in] pixels   Row-major pixel bytes (bytea)
 * @param[in] width    Tile width in pixels
 * @param[in] height   Tile height in pixels
 * @param[in] quadbin  CARTO QUADBIN cell (bigint)
 * @param[in] pixtype  Pixel type name: UINT8 | INT16 | INT32 | FLOAT32 | FLOAT64
 * @param[in] nodata   Nodata sentinel value
 * @param[in] has_nodata  Enable nodata filtering
 * @param[in] traj     Trajectory (tgeompoint, SRID 4326)
 * @csqlfn #Raster_tile_value_quadbin()
 */
Datum
Raster_tile_value_quadbin(PG_FUNCTION_ARGS)
{
  bytea     *pxbytea   = PG_GETARG_BYTEA_PP(0);
  int32      width     = PG_GETARG_INT32(1);
  int32      height    = PG_GETARG_INT32(2);
  int64      quadbin   = PG_GETARG_INT64(3);
  text      *pixtype_t = PG_GETARG_TEXT_PP(4);
  float8     nodata    = PG_GETARG_FLOAT8(5);
  bool       has_nd    = PG_GETARG_BOOL(6);
  Temporal  *traj      = PG_GETARG_TEMPORAL_P(7);

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  MeosPixType pixtype   = text_to_pixtype(pixtype_t);

  Temporal *result = raster_tile_value_quadbin(pixels,
    (uint16_t) width, (uint16_t) height, (uint64) quadbin,
    pixtype, nodata, has_nd, traj);

  PG_FREE_IF_COPY(traj, 7);

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
  bytea      *pxbytea   = PG_GETARG_BYTEA_PP(0);
  int32       width     = PG_GETARG_INT32(1);
  int32       height    = PG_GETARG_INT32(2);
  int64       quadbin   = PG_GETARG_INT64(3);
  text       *pixtype_t = PG_GETARG_TEXT_PP(4);
  bool        has_nd    = ! PG_ARGISNULL(5);
  float8      nodata    = has_nd ? PG_GETARG_FLOAT8(5) : 0.0;
  MeosPixType pixtype   = text_to_pixtype(pixtype_t);

  if (width <= 0 || height <= 0)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The width and height of a raquet tile must be positive")));
  size_t need = (size_t) width * height * raquet_pixtype_size(pixtype);
  if ((size_t) VARSIZE_ANY_EXHDR(pxbytea) < need)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The pixel array has %zu bytes but %zu are required for a "
        "%d x %d tile", (size_t) VARSIZE_ANY_EXHDR(pxbytea), need, width,
        height)));

  const uint8_t *pixels = (const uint8_t *) VARDATA_ANY(pxbytea);
  Raquet *result = raquet_make((uint64) quadbin, (uint16_t) width,
    (uint16_t) height, pixtype, nodata, has_nd, pixels);
  PG_RETURN_RAQUET_P(result);
}

PGDLLEXPORT Datum Raster_tile_value(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Raster_tile_value);
/**
 * @ingroup mobilitydb_raster
 * @brief Sample a Raquet tile along a tgeompoint trajectory
 * @param[in] rq    Raquet tile
 * @param[in] traj  Trajectory (tgeompoint)
 * @sqlfn raster_tile_value()
 */
Datum
Raster_tile_value(PG_FUNCTION_ARGS)
{
  Raquet   *rq   = PG_GETARG_RAQUET_P(0);
  Temporal *traj = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = raster_tile_value(rq, traj);
  PG_FREE_IF_COPY(rq, 0);
  PG_FREE_IF_COPY(traj, 1);
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
 * @csqlfn #Trajectory_quadbins()
 */
Datum
Trajectory_quadbins(PG_FUNCTION_ARGS)
{
  Temporal *traj = PG_GETARG_TEMPORAL_P(0);
  int32     zoom = PG_GETARG_INT32(1);

  if (zoom < 0 || zoom > 15)
    ereport(ERROR,
      (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
       errmsg("zoom level must be between 0 and 15")));

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
