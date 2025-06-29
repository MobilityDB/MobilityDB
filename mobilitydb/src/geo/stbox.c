/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Functions for spatiotemporal bounding boxes
 */

#include "geo/stbox.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <lib/stringinfo.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input/Ouput functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_in);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Text (WKT)
 * representation
 * @sqlfn stbox_in()
 */
Datum
Stbox_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_STBOX_P(stbox_parse(&str));
}

PGDLLEXPORT Datum Stbox_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_out);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal
 * box
 * @sqlfn stbox_out()
 */
Datum
Stbox_out(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_CSTRING(stbox_out(box, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Stbox_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_recv);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Binary (WKB)
 * representation
 * @sqlfn stbox_recv()
 */
Datum
Stbox_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  STBox *result = stbox_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_send);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return the Well-Known Binary (WKB) representation of a spatiotemporal
 * box
 * @sqlfn stbox_send()
 */
Datum
Stbox_send(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  size_t wkb_size = VARSIZE_ANY_EXHDR(box);
  /* A spatiotemporal box always outputs the SRID */
  uint8_t *wkb = stbox_as_wkb(box, WKB_EXTENDED, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Input/output in WKT and WKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_as_text);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return the Well-Known Text (WKT) representation of a spatiotemporal
 * box
 * @sqlfn asText()
 */
Datum
Stbox_as_text(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = stbox_out(box, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_as_wkb);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return the Well-Known Binary (WKB) representation of a spatiotemporal
 * box
 * @sqlfn asBinary()
 */
Datum
Stbox_as_wkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  /* A spatiotemporal box always outputs the SRID */
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, box, T_STBOX, true));
}

PGDLLEXPORT Datum Stbox_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_as_hexwkb);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a spatiotemporal box
 * @sqlfn asHexWKB()
 */
Datum
Stbox_as_hexwkb(PG_FUNCTION_ARGS)
{
  Datum box = PG_GETARG_DATUM(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, box, T_STBOX));
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_from_wkb);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return a spatiotemporal box from its Well-Known Binary (WKB)
 * representation
 * @sqlfn stboxFromBinary()
 */
Datum
Stbox_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  STBox *result = stbox_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_from_hexwkb);
/**
 * @ingroup mobilitydb_geo_box_inout
 * @brief Return a spatiotemporal box from its ASCII hex-encoded Well-Known
 * Binary (HexWKB) representation
 * @sqlfn stboxFromHexWKB()
 */
Datum
Stbox_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  STBox *result = stbox_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @brief Return a spatiotemporal box constructed from the arguments
 */
static Datum
Stbox_constructor(FunctionCallInfo fcinfo, bool hasx, bool hasz,
  bool hast, bool geodetic)
{
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  int32_t srid = 0;
  Span *period = NULL;

  int i = 0;
  if (hasx)
  {
    if (! hasz)
    {
      xmin = PG_GETARG_FLOAT8(i++);
      ymin = PG_GETARG_FLOAT8(i++);
      xmax = PG_GETARG_FLOAT8(i++);
      ymax = PG_GETARG_FLOAT8(i++);
    }
    else /* hasz */
    {
      xmin = PG_GETARG_FLOAT8(i++);
      ymin = PG_GETARG_FLOAT8(i++);
      zmin = PG_GETARG_FLOAT8(i++);
      xmax = PG_GETARG_FLOAT8(i++);
      ymax = PG_GETARG_FLOAT8(i++);
      zmax = PG_GETARG_FLOAT8(i++);
    }
  }
  if (hast)
  {
    meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, i));
    assert(basetype == T_TSTZSPAN || basetype == T_TIMESTAMPTZ);
    if (basetype == T_TSTZSPAN)
      period = PG_GETARG_SPAN_P(i++);
    else /* basetype == T_TIMESTAMPTZ */
    {
      TimestampTz t = PG_GETARG_TIMESTAMPTZ(i++);
      period = span_make(t, t, true, true, T_TIMESTAMPTZ);
    }
  }
  if (hasx)
    srid = PG_GETARG_INT32(i++);

  /* Construct the box */
  PG_RETURN_STBOX_P(stbox_make(hasx, hasz, geodetic, srid, xmin, xmax,
    ymin, ymax, zmin, zmax, period));
}

/*****************************************************************************/

PGDLLEXPORT Datum Stbox_constructor_x(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_x);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn stbox()
 */
inline Datum
Stbox_constructor_x(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, true, false, false, false);
}

PGDLLEXPORT Datum Stbox_constructor_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_z);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn stbox_z()
 */
inline Datum
Stbox_constructor_z(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, true, true, false, false);
}

PGDLLEXPORT Datum Stbox_constructor_t(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_t);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn stbox_t()
 */
inline Datum
Stbox_constructor_t(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, false, false, true, false);
}

PGDLLEXPORT Datum Stbox_constructor_xt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_xt);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn stbox_xt()
 */
inline Datum
Stbox_constructor_xt(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, true, false, true, false);
}

PGDLLEXPORT Datum Stbox_constructor_zt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_constructor_zt);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn stbox_zt()
 */
inline Datum
Stbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, true, true, true, false);
}

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */

PGDLLEXPORT Datum Geodstbox_constructor_z(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geodstbox_constructor_z);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn geodstbox_z()
 */
inline Datum
Geodstbox_constructor_z(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, true, true, false, true);
}

PGDLLEXPORT Datum Geodstbox_constructor_t(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geodstbox_constructor_t);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn geodstbox_t()
 */
inline Datum
Geodstbox_constructor_t(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, false, false, true, true);
}

PGDLLEXPORT Datum Geodstbox_constructor_zt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geodstbox_constructor_zt);
/**
 * @ingroup mobilitydb_geo_box_constructor
 * @brief Return a spatiotemporal box constructed from the arguments
 * @sqlfn geodstbox_zt()
 */
inline Datum
Geodstbox_constructor_zt(PG_FUNCTION_ARGS)
{
  return Stbox_constructor(fcinfo, true, true, true, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Geo_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Return a spatiotemporal box constructed from a geometry/geography and
 * a timestamptz
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Geo_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  STBox *result = geo_timestamptz_to_stbox(gs, t);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Geo_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Return a spatiotemporal box constructed from a geometry/geography and
 * a timestamptz span
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Geo_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Span *p = PG_GETARG_SPAN_P(1);
  STBox *result = geo_tstzspan_to_stbox(gs, p);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_to_box2d(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_to_box2d);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a spatiotemporal box into a PostGIS @p box2d
 * @sqlfn box2d()
 * @sqlop @p ::
 */
Datum
Stbox_to_box2d(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  GBOX *result = stbox_to_gbox(box);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Stbox_to_box3d(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_to_box3d);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a spatiotemporal box into a PostGIS @p box3d
 * @sqlfn box3d()
 * @sqlop @p ::
 */
Datum
Stbox_to_box3d(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  BOX3D *result = stbox_to_box3d(box);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Stbox_to_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_to_geo);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a spatiotemporal box into a PostGIS geometry/geography
 * @sqlfn geometry()
 * @sqlop @p ::
 */
Datum
Stbox_to_geo(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Datum result = PointerGetDatum(stbox_to_geo(box));
  PG_RETURN_DATUM(result);
}

PGDLLEXPORT Datum Stbox_to_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_to_tstzspan);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a spatiotemporal box into a timestamptz span
 * @sqlfn timeSpan()
 * @sqlop @p ::
 */
Datum
Stbox_to_tstzspan(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Span *result = stbox_to_tstzspan(box);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Box2d_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Box2d_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a PostGIS @p box2d into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Box2d_to_stbox(PG_FUNCTION_ARGS)
{
  GBOX *box = (GBOX *) PG_GETARG_POINTER(0);
  PG_RETURN_STBOX_P(gbox_to_stbox(box));
}

PGDLLEXPORT Datum Box3d_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Box3d_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a PostGIS @p box3d into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Box3d_to_stbox(PG_FUNCTION_ARGS)
{
  BOX3D *box = (BOX3D *) PG_GETARG_POINTER(0);
  PG_RETURN_STBOX_P(box3d_to_stbox(box));
}

PGDLLEXPORT Datum Geo_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a geometry/geography into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Geo_to_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  STBox *result = geo_to_stbox(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a timestamptz into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  PG_RETURN_STBOX_P(timestamptz_to_stbox(t));
}

/**
 * @brief Peek into a set datum to find the bounding box. If the datum
 * needs to be detoasted, extract only the header and not the full object.
 */
void
tstzset_stbox_slice(Datum sdatum, STBox *box)
{
  Set *s = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) sdatum))
    s = (Set *) PG_DETOAST_DATUM_SLICE(sdatum, 0, TIME_MAX_HEADER_SIZE);
  else
    s = (Set *) sdatum;
  tstzset_set_stbox(s, box);
  PG_FREE_IF_COPY_P(s, DatumGetPointer(sdatum));
  return;
}

PGDLLEXPORT Datum Tstzset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzset_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a timestamptz set into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Tstzset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum tsdatum = PG_GETARG_DATUM(0);
  STBox *result = palloc(sizeof(STBox));
  tstzset_stbox_slice(tsdatum, result);
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a timestamptz span into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Span *p = PG_GETARG_SPAN_P(0);
  PG_RETURN_STBOX_P(tstzspan_to_stbox(p));
}

/**
 * @brief Peek into a span set datum to find the bounding box. If the datum
 * needs to be detoasted, extract only the header and not the full object.
 */
void
tstzspanset_stbox_slice(Datum ssdatum, STBox *box)
{
  SpanSet *ss = NULL;
  if (PG_DATUM_NEEDS_DETOAST((struct varlena *) ssdatum))
    ss = (SpanSet *) PG_DETOAST_DATUM_SLICE(ssdatum, 0, TIME_MAX_HEADER_SIZE);
  else
    ss = (SpanSet *) ssdatum;
  tstzspanset_set_stbox(ss, box);
  PG_FREE_IF_COPY_P(ss, DatumGetPointer(ssdatum));
  return;
}

PGDLLEXPORT Datum Tstzspanset_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_to_stbox);
/**
 * @ingroup mobilitydb_geo_box_conversion
 * @brief Convert a timestamptz span set into a spatiotemporal box
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Tstzspanset_to_stbox(PG_FUNCTION_ARGS)
{
  Datum psdatum = PG_GETARG_DATUM(0);
  STBox *result = palloc(sizeof(STBox));
  tstzspanset_stbox_slice(psdatum, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_hasx(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_hasx);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return true if a spatiotemporal box has value dimension
 * @sqlfn hasX()
 */
Datum
Stbox_hasx(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasx(box));
}

PGDLLEXPORT Datum Stbox_hasz(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_hasz);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return true if a spatiotemporal box has Z dimension
 * @sqlfn hasZ()
 */
Datum
Stbox_hasz(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hasz(box));
}

PGDLLEXPORT Datum Stbox_hast(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_hast);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return true if a spatiotemporal box has time dimension
 * @sqlfn hasT()
 */
Datum
Stbox_hast(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_hast(box));
}

PGDLLEXPORT Datum Stbox_isgeodetic(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_isgeodetic);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return true if a spatiotemporal box is geodetic
 * @sqlfn isGeodetic()
 */
Datum
Stbox_isgeodetic(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_BOOL(stbox_isgeodetic(box));
}

PGDLLEXPORT Datum Stbox_xmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_xmin);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the minimum X value of a spatiotemporal box, if any
 * @sqlfn Xmin()
 */
Datum
Stbox_xmin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_xmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_xmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_xmax);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the maximum X value of a spatiotemporal box, if any
 * @sqlfn Xmax()
 */
Datum
Stbox_xmax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_xmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_ymin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ymin);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the minimum Y value of a spatiotemporal box, if any
 * @sqlfn Ymin()
 */
Datum
Stbox_ymin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_ymin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_ymax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ymax);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the maximum Y value of a spatiotemporal box, if any
 * @sqlfn Ymax()
 */
Datum
Stbox_ymax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_ymax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_zmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_zmin);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the minimum Z value of a spatiotemporal box, if any
 * @sqlfn Zmin()
 */
Datum
Stbox_zmin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_zmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_zmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_zmax);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the maximum Z value of a spatiotemporal box, if any
 * @sqlfn Zmax()
 */
Datum
Stbox_zmax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result;
  if (! stbox_zmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_tmin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tmin);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the minimum timestamptz value of a spatiotemporal box, if any
 * @sqlfn Tmin()
 */
Datum
Stbox_tmin(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  TimestampTz result;
  if (! stbox_tmin(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Stbox_tmin_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tmin_inc);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return true if the minimum timestamptz value of a spatiotemporal box
 * is inclusive, if any
 * @sqlfn Tmin_inc()
 */
Datum
Stbox_tmin_inc(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  bool result;
  if (! stbox_tmin_inc(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Stbox_tmax(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tmax);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the maximum T value of a spatiotemporal box, if any
 * @sqlfn Tmax()
 */
Datum
Stbox_tmax(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  TimestampTz result;
  if (! stbox_tmax(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_TIMESTAMPTZ(result);
}

PGDLLEXPORT Datum Stbox_tmax_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_tmax_inc);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return true if the maximum timestamptz value of a spatiotemporal box
 * is inclusive, if any
 * @sqlfn Tmax_inc()
 */
Datum
Stbox_tmax_inc(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  bool result;
  if (! stbox_tmax_inc(box, &result))
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Stbox_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_area);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the area of a spatiotemporal box
 * @sqlfn area()
 */
Datum
Stbox_area(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  bool spheroid = PG_GETARG_BOOL(1);
  double result = stbox_area(box, spheroid);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_volume(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_volume);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the volume of a 3D spatiotemporal box
 * @sqlfn volume()
 */
Datum
Stbox_volume(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double result = stbox_volume(box);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Stbox_perimeter(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_perimeter);
/**
 * @ingroup mobilitydb_geo_box_accessor
 * @brief Return the area of a spatiotemporal box
 * @sqlfn area()
 */
Datum
Stbox_perimeter(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  bool spheroid = PG_GETARG_BOOL(1);
  double result = stbox_perimeter(box, spheroid);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_shift_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_shift_time);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with the time span shifted by an interval
 * @sqlfn shiftTime()
 */
Datum
Stbox_shift_time(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_STBOX_P(stbox_shift_scale_time(box, shift, NULL));
}

PGDLLEXPORT Datum Stbox_scale_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_scale_time);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with the time span scaled by an interval
 * @sqlfn scaleTime()
 */
Datum
Stbox_scale_time(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_STBOX_P(stbox_shift_scale_time(box, NULL, duration));
}

PGDLLEXPORT Datum Stbox_shift_scale_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_shift_scale_time);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with the time span shifted and scaled by
 * two intervals
 * @sqlfn shiftScaleTime()
 */
Datum
Stbox_shift_scale_time(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  PG_RETURN_STBOX_P(stbox_shift_scale_time(box, shift, duration));
}

PGDLLEXPORT Datum Stbox_get_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_get_space);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with only the space bounds
 * @sqlfn getSpace()
 */
Datum
Stbox_get_space(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_STBOX_P(stbox_get_space(box));
}

PGDLLEXPORT Datum Stbox_expand_space(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_expand_space);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with the space bounds expanded/shrinked
 * by a double
 * @sqlfn expandSpace()
 */
Datum
Stbox_expand_space(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  double d = PG_GETARG_FLOAT8(1);
  STBox *result = stbox_expand_space(box, d);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_expand_time(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_expand_time);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with the time span expanded/shrinked by
 * an interval
 * @sqlfn Stbox_expand_time()
 */
Datum
Stbox_expand_time(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  STBox *result = stbox_expand_time(box, interval);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_round);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box with the precision of the space bounds
 * set to a number of decimal values
 * @sqlfn round()
 */
Datum
Stbox_round(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int maxdd = PG_GETARG_INT32(1);
  PG_RETURN_STBOX_P(stbox_round(box, maxdd));
}

PGDLLEXPORT Datum Stboxarr_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stboxarr_round);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return an array of temporal points with the precision of the
 * coordinates set to a number of decimal places
 * @sqlfn round()
 */
Datum
Stboxarr_round(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  /* Return NULL on empty array */
  int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
  if (count == 0)
  {
    PG_FREE_IF_COPY(array, 0);
    PG_RETURN_NULL();
  }
  int maxdd = PG_GETARG_INT32(1);

  STBox *boxarr = stboxarr_extract(array, &count);
  STBox *resarr = stboxarr_round(boxarr, count, maxdd);
  ArrayType *result = stboxarr_to_array(resarr, count);
  pfree(boxarr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_srid);
/**
 * @ingroup mobilitydb_geo_box_srid
 * @brief Return the SRID of a spatiotemporal box
 * @sqlfn SRID()
 */
Datum
Stbox_srid(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  PG_RETURN_INT32(stbox_srid(box));
}

PGDLLEXPORT Datum Stbox_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_set_srid);
/**
 * @ingroup mobilitydb_geo_box_srid
 * @brief Return a spatiotemporal box with the coordinates set to an SRID
 * @sqlfn setSRID()
 */
Datum
Stbox_set_srid(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  PG_RETURN_STBOX_P(stbox_set_srid(box, srid));
}

PGDLLEXPORT Datum Stbox_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_transform);
/**
 * @ingroup mobilitydb_geo_box_srid
 * @brief Return a spatiotemporal box transformed to an SRID
 * @sqlfn transform()
 */
Datum
Stbox_transform(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int32 srid = PG_GETARG_INT32(1);
  STBox *result = stbox_transform(box, srid);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_transform_pipeline);
/**
 * @ingroup mobilitydb_geo_box_srid
 * @brief Return a spatiotemporal box transformed to an SRID using a pipeline
 * @sqlfn transformPipeline()
 */
Datum
Stbox_transform_pipeline(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  STBox *result = stbox_transform_pipeline(box, pipelinestr, srid, is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

PGDLLEXPORT Datum Contains_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contains_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_topo
 * @brief Return true if the first spatiotemporal box contains the second one
 * @sqlfn stbox_contains()
 * @sqlop @p \@>
 */
Datum
Contains_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contains_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Contained_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Contained_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_topo
 * @brief Return true if the first spatiotemporal box is contained in the
 * second one
 * @sqlfn stbox_contained()
 * @sqlop @p <@
 */
Datum
Contained_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(contained_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overlaps_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overlaps_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_topo
 * @brief Return true if two spatiotemporal boxes overlap
 * @sqlfn stbox_overlaps()
 * @sqlop @p &&
 */
Datum
Overlaps_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overlaps_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Same_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Same_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_topo
 * @brief Return true if two spatiotemporal boxes are equal in the common
 * dimensions
 * @sqlfn stbox_same()
 * @sqlop @p ~=
 */
Datum
Same_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(same_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Adjacent_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adjacent_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_topo
 * @brief Return true if two spatiotemporal boxes are adjacent
 * @sqlfn stbox_adjacent()
 * @sqlop @p -|-
 */
Datum
Adjacent_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(adjacent_stbox_stbox(box1, box2));
}

/*****************************************************************************
 * Position operators
 *****************************************************************************/

PGDLLEXPORT Datum Left_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Left_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is to the left of the
 * second one
 * @sqlfn temporal_below()
 * @sqlop @p >>
 */
Datum
Left_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(left_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overleft_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overleft_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * right of the second one
 * @sqlfn temporal_below()
 * @sqlop @p &>
 */
Datum
Overleft_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overleft_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Right_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Right_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is to the right of the
 * second one
 * @sqlfn temporal_below()
 * @sqlop @p <<
 */
Datum
Right_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(right_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overright_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overright_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatio temporal box does not extend to the
 * left of the second one
 * @sqlfn temporal_below()
 * @sqlop @p &<
 */
Datum
Overright_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overright_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Below_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Below_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is below the second
 * one
 * @sqlfn temporal_below()
 * @sqlop @p <<|
 */
Datum
Below_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(below_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overbelow_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbelow_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend above of
 * the second one
 * @sqlfn temporal_below()
 * @sqlop @p &<|
 */
Datum
Overbelow_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbelow_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Above_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Above_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is above of the second
 * one
 * @sqlfn temporal_below()
 * @sqlop @p |>>
 */
Datum
Above_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(above_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overabove_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overabove_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend below of
 * the second one
 * @sqlfn temporal_below()
 * @sqlop @p |&>
 */
Datum
Overabove_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overabove_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Front_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Front_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is in front of the second
 * one
 * @sqlfn temporal_below()
 * @sqlop @p <</
 */
Datum
Front_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(front_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overfront_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overfront_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * back of the second one
 * @sqlfn temporal_below()
 * @sqlop @p &</
 */
Datum
Overfront_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overfront_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Back_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Back_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is at the back of the
 * second one
 * @sqlfn temporal_below()
 * @sqlop @p />>
 */
Datum
Back_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(back_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overback_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overback_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box does not extend to the
 * front of the second one
 * @sqlfn temporal_below()
 * @sqlop @p /&>
 */
Datum
Overback_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overback_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Before_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Before_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is before the second one
 * @sqlfn temporal_below()
 * @sqlop @p <<#
 */
Datum
Before_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(before_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overbefore_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overbefore_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first temporal box is not after the second one
 * @sqlfn temporal_below()
 * @sqlop @p &<#
 */
Datum
Overbefore_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overbefore_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum After_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(After_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first spatiotemporal box is after the second one
 * @sqlfn temporal_below()
 * @sqlop @p #>>
 */
Datum
After_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(after_stbox_stbox(box1, box2));
}

PGDLLEXPORT Datum Overafter_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Overafter_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_pos
 * @brief Return true if the first temporal box is not before the second one
 * @sqlfn temporal_below()
 * @sqlop @p #&>
 */
Datum
Overafter_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(overafter_stbox_stbox(box1, box2));
}

/*****************************************************************************
 * Set operators
 *****************************************************************************/

PGDLLEXPORT Datum Union_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Union_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_set
 * @brief Return the union of two spatiotemporal boxes
 * @sqlfn stbox_union()
 * @sqlop @p +
 */
Datum
Union_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_STBOX_P(union_stbox_stbox(box1, box2, true));
}

PGDLLEXPORT Datum Intersection_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intersection_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_box_set
 * @brief Return the intersection of two spatiotemporal boxes
 * @sqlfn stbox_intersection()
 * @sqlop @p *
 */
Datum
Intersection_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  STBox *result = intersection_stbox_stbox(box1, box2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_quad_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_quad_split);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a spatiotemporal box split with respect to its space bounds
 * in four quadrants (2D) or eight octants (3D)
 * @sqlfn stbox_intersection()
 * @sqlop @p *
 */
Datum
Stbox_quad_split(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  int count;
  STBox *boxes = stbox_quad_split(box, &count);
  ArrayType *result = stboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_extent_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_extent_transfn);
/**
 * @brief Transition function for extent aggregation of spatiotemporal boxes
 */
Datum
Stbox_extent_transfn(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);

  /* Can't do anything with null inputs */
  if (! box1 && ! box2)
    PG_RETURN_NULL();
  /* One of the boxes is null, return the other one */
  if (! box1)
    PG_RETURN_STBOX_P(stbox_copy(box2));
  if (! box2)
    PG_RETURN_STBOX_P(stbox_copy(box1));

  /* Both boxes are not null */
  ensure_same_dimensionality(box1->flags, box2->flags);
  if (MEOS_FLAGS_GET_X(box1->flags))
  {
    ensure_same_srid(stbox_srid(box1), stbox_srid(box2));
    ensure_same_geodetic(box1->flags, box2->flags);
  }
  STBox *result = palloc(sizeof(STBox));
  memcpy(result, box1, sizeof(STBox));
  stbox_expand(box2, result);
  PG_RETURN_STBOX_P(result);
}

PGDLLEXPORT Datum Stbox_extent_combinefn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_extent_combinefn);
/**
 * @brief Combine function for extent aggregation of spatiotemporal boxes
 */
Datum
Stbox_extent_combinefn(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_STBOX_P(1);
  if (!box1 && !box2)
    PG_RETURN_NULL();
  if (box1 && !box2)
    PG_RETURN_STBOX_P(box1);
  if (!box1 && box2)
    PG_RETURN_STBOX_P(box2);
  /* Both boxes are not null */
  ensure_same_dimensionality(box1->flags, box2->flags);
  STBox *result = stbox_copy(box1);
  stbox_expand(box2, result);
  PG_RETURN_STBOX_P(result);
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

PGDLLEXPORT Datum Stbox_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_cmp);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return -1, 0, or 1 depending on whether the first spatiotemporal box
 * is less than, equal to, or greater than the second one
 * @sqlfn stbox_cmp()
 */
Datum
Stbox_cmp(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_INT32(stbox_cmp(box1, box2));
}

PGDLLEXPORT Datum Stbox_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_lt);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return true if the first spatiotemporal box is less than the second
 * one
 * @sqlfn stbox_lt()
 * @sqlop @p <
 */
Datum
Stbox_lt(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_lt(box1, box2));
}

PGDLLEXPORT Datum Stbox_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_le);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return true if the first spatiotemporal box is less than or equal to
 * the second one
 * @sqlfn stbox_le()
 * @sqlop @p <=
 */
Datum
Stbox_le(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_le(box1, box2));
}

PGDLLEXPORT Datum Stbox_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ge);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return true if the first spatiotemporal box is greater than or equal
 * to the second one
 * @sqlfn stbox_ge()
 * @sqlop @p >=
 */
Datum
Stbox_ge(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ge(box1, box2));
}

PGDLLEXPORT Datum Stbox_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_gt);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return true if the first spatiotemporal box is greater than the
 * second one
 * @sqlfn stbox_gt()
 * @sqlop @p >
 */
Datum
Stbox_gt(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_gt(box1, box2));
}

PGDLLEXPORT Datum Stbox_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_eq);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return true if two spatiotemporal boxes are equal
 * @sqlfn stbox_eq()
 * @sqlop @p =
 */
Datum
Stbox_eq(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_eq(box1, box2));
}

PGDLLEXPORT Datum Stbox_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Stbox_ne);
/**
 * @ingroup mobilitydb_geo_box_comp
 * @brief Return true if two spatiotemporal boxes are different
 * @sqlfn stbox_ne()
 * @sqlop @p <>
 */
Datum
Stbox_ne(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  PG_RETURN_BOOL(stbox_ne(box1, box2));
}

/*****************************************************************************/
