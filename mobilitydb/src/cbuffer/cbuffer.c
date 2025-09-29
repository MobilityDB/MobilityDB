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
 * @brief Static circular buffer type
 */

/* PostgreSQL */
#include <postgres.h>
#include <libpq/pqformat.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_cbuffer.h>
#include "temporal/temporal.h"
#include "temporal/tnumber_mathfuncs.h"
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
#include "geo/stbox.h"
#include "cbuffer/cbuffer.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_in);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return a circular buffer from its Well-Known Text (WKT)
 * representation
 * @details Example of input:
 * @code
 *    Cbuffer(Point(1 1), 10)
 * @endcode
 * @sqlfn cbuffer_in()
 */
Datum
Cbuffer_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  PG_RETURN_CBUFFER_P(cbuffer_parse(&str, true));
}

PGDLLEXPORT Datum Cbuffer_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_out);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 * @sqlfn cbuffer_out()
 */
Datum
Cbuffer_out(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_CSTRING(cbuffer_out(cb, OUT_DEFAULT_DECIMAL_DIGITS));
}

PGDLLEXPORT Datum Cbuffer_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_recv);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return a circular buffer from its Well-Known Binary (WKB)
 * representation
 * @sqlfn cbuffer_recv()
 */
Datum
Cbuffer_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Cbuffer *result = cbuffer_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_CBUFFER_P(result);
}

PGDLLEXPORT Datum Cbuffer_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_send);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a circular
 * buffer
 * @sqlfn cbuffer_send()
 */
Datum
Cbuffer_send(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  size_t wkb_size = VARSIZE_ANY_EXHDR(cb);
  /* A circular buffer always outputs the SRID */
  uint8_t *wkb = cbuffer_as_wkb(cb, WKB_EXTENDED, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Output in WKT and EWKT representation
 *****************************************************************************/

/**
 * @brief Return the (Extended) Well-Known Text (WKT or EWKT) representation of
 * a circular buffer
 * @sqlfn asText()
 */
static Datum
Cbuffer_as_text_common(FunctionCallInfo fcinfo, bool extended)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = extended ? cbuffer_as_ewkt(cb, dbl_dig_for_wkt) : 
    cbuffer_as_text(cb, dbl_dig_for_wkt);
  text *result = cstring2text(str);
  pfree(str);
  PG_FREE_IF_COPY(cb, 0);
  PG_RETURN_TEXT_P(result);
}

PGDLLEXPORT Datum Cbuffer_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_as_text);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return the Well-Known Text (WKT) representation of a circular buffer
 * @sqlfn asText()
 */
Datum
Cbuffer_as_text(PG_FUNCTION_ARGS)
{
  return Cbuffer_as_text_common(fcinfo, false);
}

PGDLLEXPORT Datum Cbuffer_as_ewkt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_as_ewkt);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return the Extended Well-Known Text (EWKT) representation of a
 * circular buffer
 * @note It is the WKT representation prefixed with the SRID
 * @sqlfn asEWKT()
 */
Datum
Cbuffer_as_ewkt(PG_FUNCTION_ARGS)
{
  return Cbuffer_as_text_common(fcinfo, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbuffer_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_from_wkb);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return a circular buffer from its Well-Known Binary (WKB)
 * representation
 * @sqlfn cbufferFromBinary()
 */
Datum
Cbuffer_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Cbuffer *result = cbuffer_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_CBUFFER_P(result);
}

PGDLLEXPORT Datum Cbuffer_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_from_hexwkb);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return a circular buffer from its ASCII hex-encoded Well-Known Binary
 * (HexWKB) representation
 * @sqlfn cbufferFromHexWKB()
 */
Datum
Cbuffer_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Cbuffer *result = cbuffer_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_CBUFFER_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Cbuffer_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_as_wkb);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return the Well-Known Binary (WKB) representation of a circular
 * buffer
 * @sqlfn asBinary()
 */
Datum
Cbuffer_as_wkb(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, PointerGetDatum(cb), T_CBUFFER,
    false));
}

PGDLLEXPORT Datum Cbuffer_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_as_hexwkb);
/**
 * @ingroup mobilitydb_cbuffer_base_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a circular buffer
 * @sqlfn asHexWKB()
 */
Datum
Cbuffer_as_hexwkb(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, PointerGetDatum(cb), T_CBUFFER));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_constructor);
/**
 * @ingroup mobilitydb_cbuffer_base_constructor
 * @brief Construct a circular buffer from a point and a radius
 * @sqlfn cbuffer()
 */
Datum
Cbuffer_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *point = PG_GETARG_GSERIALIZED_P(0);
  double radius = PG_GETARG_FLOAT8(1);
  PG_RETURN_CBUFFER_P(cbuffer_make(point, radius));
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_to_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_to_geom);
/**
 * @ingroup mobilitydb_cbuffer_base_conversion
 * @brief Convert a circular buffer into a geometry
 * @sqlfn geometry()
 * @sqlop @p ::
 */
Datum
Cbuffer_to_geom(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_GSERIALIZED_P(cbuffer_to_geom(cb));
}

PGDLLEXPORT Datum Geom_to_cbuffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_to_cbuffer);
/**
 * @ingroup mobilitydb_cbuffer_base_conversion
 * @brief Convert a geometry into a circular buffer
 * @sqlfn cbuffer()
 * @sqlop @p ::
 */
Datum
Geom_to_cbuffer(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Cbuffer *result = geom_to_cbuffer(gs);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_CBUFFER_P(result);
}

/*****************************************************************************
 * Bounding box functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_to_stbox);
/**
 * @ingroup mobilitydb_cbuffer_base_box
 * @brief Construct a spatiotemporal box from a circular buffer
 * @sqlfn stbox()
 * @sqlop @p ::
 */
Datum
Cbuffer_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_STBOX_P(cbuffer_to_stbox(cb));
}

PGDLLEXPORT Datum Cbuffer_timestamptz_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_timestamptz_to_stbox);
/**
 * @ingroup mobilitydb_cbuffer_base_box
 * @brief Construct a spatiotemporal box from a circular buffer and a
 * timestamptz
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Cbuffer_timestamptz_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  PG_RETURN_STBOX_P(cbuffer_timestamptz_to_stbox(cb, t));
}

PGDLLEXPORT Datum Cbuffer_tstzspan_to_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_tstzspan_to_stbox);
/**
 * @ingroup mobilitydb_cbuffer_base_box
 * @brief Construct a spatiotemporal box from a circular buffer and a
 * timestamptz span
 * box
 * @sqlfn stbox()
 * @sqlop @p
 */
Datum
Cbuffer_tstzspan_to_stbox(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Span *s = PG_GETARG_SPAN_P(1);
  PG_RETURN_STBOX_P(cbuffer_tstzspan_to_stbox(cb, s));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_point(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_point);
/**
 * @ingroup mobilitydb_cbuffer_base_accessor
 * @brief Return the point of a circular buffer
 * @sqlfn point()
 */
Datum
Cbuffer_point(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Datum d = PointerGetDatum(&cb->point);
  PG_RETURN_DATUM(datum_copy(d, T_GEOMETRY));
}

PGDLLEXPORT Datum Cbuffer_radius(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_radius);
/**
 * @ingroup mobilitydb_cbuffer_base_accessor
 * @brief Return the radius of a circular buffer
 * @sqlfn radius()
 */
Datum
Cbuffer_radius(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  PG_RETURN_FLOAT8(cbuffer_radius(cb));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_round);
/**
 * @ingroup mobilitydb_cbuffer_base_transf
 * @brief Return a circular buffer with the precision of the values set to a
 * number of decimal places
 * @sqlfn round()
 */
Datum
Cbuffer_round(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  Datum size = PG_GETARG_DATUM(1);
  PG_RETURN_CBUFFER_P(cbuffer_round(cb, size));
}

PGDLLEXPORT Datum Cbufferarr_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbufferarr_round);
/**
 * @ingroup mobilitydb_cbuffer_base_transf
 * @brief Return an array of circular buffers with the precision of the values
 * set to a number of decimal places
 * @sqlfn round()
 */
Datum
Cbufferarr_round(PG_FUNCTION_ARGS)
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

  Cbuffer **cbarr = cbufferarr_extract(array, &count);
  Cbuffer **resarr = cbufferarr_round((const Cbuffer **) cbarr, count, maxdd);
  ArrayType *result = cbufferarr_to_array((const Cbuffer **) resarr, count);
  pfree(cbarr);
  PG_FREE_IF_COPY(array, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_srid);
/**
 * @ingroup mobilitydb_cbuffer_base_srid
 * @brief Return the SRID of a circular buffer
 * @sqlfn SRID()
 */
Datum
Cbuffer_srid(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  int result = cbuffer_srid(cb);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Cbuffer_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_set_srid);
/**
 * @ingroup mobilitydb_cbuffer_base_srid
 * @brief Return a circular buffer with the coordinates of the point set to 
 * an SRID
 * @sqlfn setSRID()
 */
Datum
Cbuffer_set_srid(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Cbuffer *result = cbuffer_copy(cb);
  cbuffer_set_srid(result, srid);
  PG_RETURN_CBUFFER_P(result);
}

PGDLLEXPORT Datum Cbuffer_transform(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_transform);
/**
 * @ingroup mobilitydb_cbuffer_base_srid
 * @brief Return a circular buffer transformed to an SRID
 * @sqlfn transform()
 */
Datum
Cbuffer_transform(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  int32_t srid = PG_GETARG_INT32(1);
  Cbuffer *result = cbuffer_transform(cb, srid);
  PG_FREE_IF_COPY(cb, 0);
  PG_RETURN_CBUFFER_P(result);
}

PGDLLEXPORT Datum Cbuffer_transform_pipeline(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_transform_pipeline);
/**
 * @ingroup mobilitydb_cbuffer_base_srid
 * @brief Return a circular buffer transformed to an SRID using a pipeline
 * @sqlfn transformPipeline()
 */
Datum
Cbuffer_transform_pipeline(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  text *pipelinetxt = PG_GETARG_TEXT_P(1);
  int32_t srid = PG_GETARG_INT32(2);
  bool is_forward = PG_GETARG_BOOL(3);
  char *pipelinestr = text2cstring(pipelinetxt);
  Cbuffer *result = cbuffer_transform_pipeline(cb, pipelinestr, srid,
    is_forward);
  pfree(pipelinestr);
  PG_FREE_IF_COPY(cb, 0);
  PG_FREE_IF_COPY(pipelinetxt, 1);
  PG_RETURN_CBUFFER_P(result);
}

/*****************************************************************************
 * Spatial relationships
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_contains(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_contains);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers are disjoint
 * @sqlfn cbuffer_contains()
 */
Datum
Cbuffer_contains(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  int result = contains_cbuffer_cbuffer(cb1, cb2);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Cbuffer_covers(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_covers);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers are disjoint
 * @sqlfn cbuffer_covers()
 */
Datum
Cbuffer_covers(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  int result = covers_cbuffer_cbuffer(cb1, cb2);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Cbuffer_disjoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_disjoint);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers are disjoint
 * @sqlfn cbuffer_disjoint()
 */
Datum
Cbuffer_disjoint(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  int result = disjoint_cbuffer_cbuffer(cb1, cb2);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Cbuffer_intersects(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_intersects);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers intersect
 * @sqlfn cbuffer_intersects()
 */
Datum
Cbuffer_intersects(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  int result = intersects_cbuffer_cbuffer(cb1, cb2);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Cbuffer_touches(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_touches);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers intersect
 * @sqlfn cbuffer_touches()
 */
Datum
Cbuffer_touches(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  int result = touches_cbuffer_cbuffer(cb1, cb2);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Cbuffer_dwithin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_dwithin);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers are within a distance
 * @sqlfn cbuffer_dwithin()
 */
Datum
Cbuffer_dwithin(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = dwithin_cbuffer_cbuffer(cb1, cb2, dist);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************
 * Approximate equality
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_same(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_same);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if two circular buffers are approximately equal with 
 * respect to an epsilon value
 * @sqlfn same()
 */
Datum
Cbuffer_same(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_same(cb1, cb2));
}

/*****************************************************************************
 * Comparison functions
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_eq);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if the first circular buffer is equal to the second one
 * @sqlfn cbuffer_eq()
 * @sqlop @p =
 */
Datum
Cbuffer_eq(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_eq(cb1, cb2));
}

PGDLLEXPORT Datum Cbuffer_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_ne);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if the first circular buffer is different from the second
 * one
 * @sqlfn cbuffer_ne()
 * @sqlop @p <>
 */
Datum
Cbuffer_ne(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_ne(cb1, cb2));
}

PGDLLEXPORT Datum Cbuffer_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_cmp);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return -1, 0, or 1 depending on whether the first circular buffer
 * is less than, equal to, or greater than the second one
 * @note Function used for B-tree comparison
 * @sqlfn cbuffer_cmp()
 */
Datum
Cbuffer_cmp(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_INT32(cbuffer_cmp(cb1, cb2));
}

PGDLLEXPORT Datum Cbuffer_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_lt);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if the first circular buffer is less than the second one
 * @sqlfn cbuffer_lt()
 * @sqlop @p <
 */
Datum
Cbuffer_lt(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_lt(cb1, cb2));
}

PGDLLEXPORT Datum Cbuffer_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_le);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if the first circular buffer is less than or equal to the
 * second one
 * @sqlfn cbuffer_le()
 * @sqlop @p <=
 */
Datum
Cbuffer_le(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_le(cb1, cb2));
}

PGDLLEXPORT Datum Cbuffer_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_ge);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if the first circular buffer is greater than or equal to
 * the second one
 * @sqlfn cbuffer_ge()
 * @sqlop @p >=
 */
Datum
Cbuffer_ge(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_ge(cb1, cb2));
}

PGDLLEXPORT Datum Cbuffer_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_gt);
/**
 * @ingroup mobilitydb_cbuffer_base_comp
 * @brief Return true if the first circular buffer is greater than the second
 * one
 * @sqlfn cbuffer_gt()
 * @sqlop @p >
 */
Datum
Cbuffer_gt(PG_FUNCTION_ARGS)
{
  Cbuffer *cb1 = PG_GETARG_CBUFFER_P(0);
  Cbuffer *cb2 = PG_GETARG_CBUFFER_P(1);
  PG_RETURN_BOOL(cbuffer_gt(cb1, cb2));
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_hash);
/**
 * @ingroup mobilitydb_cbuffer_base_accessor
 * @brief Return the 32-bit hash value of a circular buffer
 * @sqlfn hash()
 */
Datum
Cbuffer_hash(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  uint32 result = cbuffer_hash(cb);
  PG_FREE_IF_COPY(cb, 0);
  PG_RETURN_UINT32(result);
}

PGDLLEXPORT Datum Cbuffer_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Cbuffer_hash_extended);
/**
 * @ingroup mobilitydb_cbuffer_base_accessor
 * @brief Return the 64-bit hash value of a circular buffer using a seed
 * @sqlfn hash_extended()
 */
Datum
Cbuffer_hash_extended(PG_FUNCTION_ARGS)
{
  Cbuffer *cb = PG_GETARG_CBUFFER_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = cbuffer_hash_extended(cb, seed);
  PG_FREE_IF_COPY(cb, 0);
  PG_RETURN_UINT64(result);
}

/*****************************************************************************/
