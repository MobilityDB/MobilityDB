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
 * @brief General functions for span types (a.k.a. ranges) composed of two
 * `Datum` values and two Boolean values stating whether the bounds are
 * inclusive or not
 */

#include "temporal/span.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type_d.h>
#include <utils/rangetypes.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/type_inout.h"
#include "temporal/type_util.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Span_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_in);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span from its Well-Known Text (WKT) representation
 * @sqlfn span_in()
 */
Datum
Span_in(PG_FUNCTION_ARGS)
{
  const char *str = PG_GETARG_CSTRING(0);
  Oid spantypid = PG_GETARG_OID(1);
  PG_RETURN_SPAN_P(span_in(str, oid_type(spantypid)));
}

PGDLLEXPORT Datum Span_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_out);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @sqlfn span_out()
 */
Datum
Span_out(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_CSTRING(span_out(s, Int32GetDatum(OUT_DEFAULT_DECIMAL_DIGITS)));
}

PGDLLEXPORT Datum Span_recv(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_recv);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span from its Well-Known Binary (WKB) representation
 * @sqlfn span_recv()
 */
Datum
Span_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Span *result = span_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Span_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_send);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the Well-Known Binary (WKB) representation of a span
 * @sqlfn span_send()
 */
Datum
Span_send(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  size_t wkb_size = VARSIZE_ANY_EXHDR(s);
  /* A span does not have an extended variant */
  uint8_t *wkb = span_as_wkb(s, 0, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Input/output in WKT, WKB, and HexWKB representation
 *****************************************************************************/

PGDLLEXPORT Datum Span_as_text(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_as_text);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the Well-Known Text (WKT) representation of a span
 * @sqlfn asText()
 */
Datum
Span_as_text(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  int dbl_dig_for_wkt = OUT_DEFAULT_DECIMAL_DIGITS;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    dbl_dig_for_wkt = PG_GETARG_INT32(1);
  char *str = span_out(s, Int32GetDatum(dbl_dig_for_wkt));
  text *result = cstring2text(str);
  pfree(str);
  PG_RETURN_TEXT_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_from_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_from_wkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span from its Well-Known Binary (WKB) representation
 * @sqlfn intspanFromBinary(), floatspanFromBinary(), ...
 */
Datum
Span_from_wkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Span *result = span_from_wkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Span_from_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_from_hexwkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return a span from its ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation
 * @sqlfn intspanFromHexWKB(), floatspanFromHexWKB(), ...
 */
Datum
Span_from_hexwkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Span *result = span_from_hexwkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_as_wkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_as_wkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the Well-Known Binary (WKB) representation of a span
 * @sqlfn asBinary()
 */
Datum
Span_as_wkb(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BYTEA_P(Datum_as_wkb(fcinfo, PointerGetDatum(s), s->spantype,
    false));
}

PGDLLEXPORT Datum Span_as_hexwkb(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_as_hexwkb);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Return the ASCII hex-encoded Well-Known Binary (HexWKB)
 * representation of a span
 * @sqlfn asHexWKB()
 */
Datum
Span_as_hexwkb(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_TEXT_P(Datum_as_hexwkb(fcinfo, PointerGetDatum(s), s->spantype));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Span_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_constructor);
/**
 * @ingroup mobilitydb_setspan_constructor
 * @brief Return a span from the bounds
 * @sqlfn intspan(), floatspan(), ...
 */
Datum
Span_constructor(PG_FUNCTION_ARGS)
{
  Datum lower = PG_GETARG_DATUM(0);
  Datum upper = PG_GETARG_DATUM(1);
  bool lower_inc = PG_GETARG_BOOL(2);
  bool upper_inc = PG_GETARG_BOOL(3);
  meosType spantype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  meosType basetype = spantype_basetype(spantype);
  PG_RETURN_SPAN_P(span_make(lower, upper, lower_inc, upper_inc, basetype));
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Value_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_to_span);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a value into a span
 * @sqlfn span()
 * @sqlop @p ::
 */
Datum
Value_to_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_SPAN_P(value_span(value, basetype));
}

PGDLLEXPORT Datum Set_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_to_span);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a set into a span
 * @sqlfn span()
 */
Datum
Set_to_span(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Span *result = set_span(s);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Set_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_spans);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return an array of spans from the elements of a set
 * @sqlfn spans()
 */
Datum
Set_spans(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Span *spans = set_spans(s);
  ArrayType *result = spanarr_to_array(spans, s->count);
  pfree(spans);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Set_split_n_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_split_n_spans);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return an array of N spans from the elements of a set
 * @sqlfn splitNspans()
 */
Datum
Set_split_n_spans(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int span_count = PG_GETARG_INT32(1);
  int count;
  Span *spans = set_split_n_spans(s, span_count, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  pfree(spans);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Set_split_each_n_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Set_split_each_n_spans);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Return an array of spans from the elements of a set obtained by
 * merging a given number of successive elements
 * @sqlfn splitEachNSpans()
 */
Datum
Set_split_each_n_spans(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  int elem_count = PG_GETARG_INT32(1);
  int count;
  Span *spans = set_split_each_n_spans(s, elem_count, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  pfree(spans);
  PG_FREE_IF_COPY(s, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Intspan_to_floatspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intspan_to_floatspan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert an integer span into a float span
 * @sqlfn floatspan()
 * @sqlop @p ::
 */
Datum
Intspan_to_floatspan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(intspan_to_floatspan(s));
}

PGDLLEXPORT Datum Floatspan_to_intspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_to_intspan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a float span into a integer span
 * @sqlfn intspan()
 * @sqlop @p ::
 */
Datum
Floatspan_to_intspan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(floatspan_to_intspan(s));
}

PGDLLEXPORT Datum Datespan_to_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespan_to_tstzspan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a date span into a timestamptz span
 * @sqlfn tstzspan()
 * @sqlop @p ::
 */
Datum
Datespan_to_tstzspan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(datespan_to_tstzspan(s));
}

PGDLLEXPORT Datum Tstzspan_to_datespan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_to_datespan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a timestamptz span into a date span
 * @sqlfn datespan()
 * @sqlop @p ::
 */
Datum
Tstzspan_to_datespan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(tstzspan_to_datespan(s));
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_to_range(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_to_range);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a span into a range
 * @sqlfn int4range(), tstzrange()
 * @sqlop @p ::
 */
Datum
Span_to_range(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  assert(s->basetype == T_INT4 || s->basetype == T_INT8 || 
    s->basetype == T_DATE || s->basetype == T_TIMESTAMPTZ);
  RangeType *range = range_make(s->lower, s->upper, s->lower_inc, s->upper_inc,
    s->basetype);
  PG_RETURN_POINTER(range);
}

/**
 * @brief Convert a PostgreSQL range into a span
 */
void
range_set_span(RangeType *range, TypeCacheEntry *typcache, Span *result)
{
  char flags = range_get_flags(range);
  if (flags & RANGE_EMPTY)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range cannot be empty")));
  if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range bounds cannot be infinite")));

  RangeBound lower, upper;
  bool empty;
  range_deserialize(typcache, range, &lower, &upper, &empty);
  Oid type_id = typcache->rngelemtype->type_id;
  assert(type_id == INT4OID || type_id == INT8OID || type_id == DATEOID || 
    type_id == TIMESTAMPTZOID);
  meosType basetype;
  if (type_id == INT4OID)
    basetype = T_INT4;
  else if (type_id == INT8OID)
    basetype = T_INT8;
  else if (type_id == DATEOID)
    basetype = T_DATE;
  else /* type_id == TIMESTAMPTZOID */
    basetype = T_TIMESTAMPTZ;
  meosType spantype = basetype_spantype(basetype);
  span_set(lower.val, upper.val, lower.inclusive, upper.inclusive, basetype,
    spantype, result);
  return;
}

PGDLLEXPORT Datum Range_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Range_to_span);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a PostgreSQL range into a span
 * @sqlfn intspan(), tstzspan()
 * @sqlop @p ::
 */
Datum
Range_to_span(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == INT4OID ||
    typcache->rngelemtype->type_id == INT8OID ||
    typcache->rngelemtype->type_id == DATEOID ||
    typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  Span *result = palloc(sizeof(Span));
  range_set_span(range, typcache, result);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* span -> base type functions */

PGDLLEXPORT Datum Span_lower(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_lower);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the lower bound of a span
 * @sqlfn lower()
 */
Datum
Span_lower(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->lower);
}

PGDLLEXPORT Datum Span_upper(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_upper);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the upper bound of a span
 * @sqlfn upper()
 */
Datum
Span_upper(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->upper);
}

/* span -> bool functions */

PGDLLEXPORT Datum Span_lower_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_lower_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the lower bound of a span is inclusive
 * @sqlfn lower_inc()
 */
Datum
Span_lower_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->lower_inc != 0);
}

PGDLLEXPORT Datum Span_upper_inc(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_upper_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the upper bound of a span is inclusive
 * @sqlfn lower_inc()
 */
Datum
Span_upper_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->upper_inc != 0);
}

PGDLLEXPORT Datum Numspan_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_width);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the width of a number span
 * @sqlfn width()
 */
Datum
Numspan_width(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(numspan_width(s));
}

PGDLLEXPORT Datum Datespan_duration(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespan_duration);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the duration of a date span
 * @sqlfn duration()
 */
Datum
Datespan_duration(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(datespan_duration(s));
}

PGDLLEXPORT Datum Tstzspan_duration(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_duration);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the duration of a timestamptz span
 * @sqlfn duration()
 */
Datum
Tstzspan_duration(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(tstzspan_duration(s));
}

/*****************************************************************************
 * Transformation functions
 * Since in PostgreSQL the type date is defined as follows
 *   typedef int32 DateADT;
 * the functions #Numspan_shift, #Numspan_scale, and #Numspan_shift_scale are
 * also used for datespans and datespansets
 *****************************************************************************/

PGDLLEXPORT Datum Numspan_expand(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_expand);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span with its bounds expanded or shrinded by a value
 * @note This function is also used for `datespan`
 * @sqlfn shift()
 */
Datum
Numspan_expand(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum value = PG_GETARG_DATUM(1);
  Span *result = numspan_expand(s, value);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Tstzspan_expand(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_expand);
/**
 * @ingroup mobilitydb_geo_box_transf
 * @brief Return a timstamptz span expanded/shrinked by an interval
 * @sqlfn expand()
 */
Datum
Tstzspan_expand(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *interval = PG_GETARG_INTERVAL_P(1);
  Span *result = tstzspan_expand(s, interval);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Timestamptz_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz shifted by an interval
 * @sqlfn shift()
 */
Datum
Timestamptz_shift(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_TIMESTAMPTZ(timestamptz_shift(t, shift));
}

PGDLLEXPORT Datum Numspan_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span shifted by a value
 * @note This function is also used for `datespan`
 * @sqlfn shift()
 */
Datum
Numspan_shift(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  PG_RETURN_SPAN_P(numspan_shift_scale(s, shift, 0, true, false));
}

PGDLLEXPORT Datum Tstzspan_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span shifted by an interval
 * @sqlfn shift()
 */
Datum
Tstzspan_shift(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_SPAN_P(tstzspan_shift_scale(s, shift, NULL));
}

PGDLLEXPORT Datum Numspan_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span scaled by a value
 * @note This function is also used for `datespan`
 * @sqlfn scale()
 */
Datum
Numspan_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum duration = PG_GETARG_DATUM(1);
  PG_RETURN_SPAN_P(numspan_shift_scale(s, 0, duration, false, true));
}

PGDLLEXPORT Datum Tstzspan_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span scaled by an interval
 * @sqlfn scale()
 */
Datum
Tstzspan_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  PG_RETURN_SPAN_P(tstzspan_shift_scale(s, NULL, duration));
}

PGDLLEXPORT Datum Numspan_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span shifted and scaled by two values
 * @note This function is also used for `datespan`
 * @sqlfn shiftScale()
 */
Datum
Numspan_shift_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Datum duration = PG_GETARG_DATUM(2);
  PG_RETURN_SPAN_P(numspan_shift_scale(s, shift, duration, true, true));
}

PGDLLEXPORT Datum Tstzspan_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span shifted and scaled by two intervals
 * @sqlfn shiftScale()
 */
Datum
Tstzspan_shift_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  PG_RETURN_SPAN_P(tstzspan_shift_scale(s, shift, duration));
}

PGDLLEXPORT Datum Floatspan_floor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_floor);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float span rounded down to the nearest integer
 * @sqlfn floor()
 */
Datum
Floatspan_floor(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(floatspan_floor(s));
}

PGDLLEXPORT Datum Floatspan_ceil(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_ceil);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float span rounded up to the nearest integer
 * @sqlfn ceil()
 */
Datum
Floatspan_ceil(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(floatspan_ceil(s));
}

PGDLLEXPORT Datum Float_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Float_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float with the precision of the values set to a number
 * of decimal places
 * @sqlfn round()
 * @note This solves the issue in PostgreSQL that we need to convert to a
 * numeric in order to apply the round function.
 */
Datum
Float_round(PG_FUNCTION_ARGS)
{
  double d = PG_GETARG_FLOAT8(0);
  int maxdd = PG_GETARG_INT32(1);
  PG_RETURN_FLOAT8(float_round(d, maxdd));
}

PGDLLEXPORT Datum Floatspan_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float span with the precision of the values set to a number
 * of decimal places
 * @sqlfn round()
 */
Datum
Floatspan_round(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  int maxdd = PG_GETARG_INT32(1);
  PG_RETURN_SPAN_P(floatspan_round(s, maxdd));
}

PGDLLEXPORT Datum Floatspan_degrees(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_degrees);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float span with the values converted to degrees
 * @sqlfn degrees()
 */
Datum
Floatspan_degrees(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  bool normalize = false;
  if (PG_NARGS() > 1 && ! PG_ARGISNULL(1))
    normalize = PG_GETARG_BOOL(1);
  PG_RETURN_SPAN_P(floatspan_degrees(s, normalize));
}

PGDLLEXPORT Datum Floatspan_radians(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_radians);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float set with the values converted to radians
 * @sqlfn radians()
 */
Datum
Floatspan_radians(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_SPAN_P(floatspan_radians(s));
}

/*****************************************************************************
 * Comparison functions for defining B-tree indexes
 *****************************************************************************/

PGDLLEXPORT Datum Span_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_eq);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is equal to the second one
 * @sqlfn span_eq()
 * @sqlop @p =
 */
Datum
Span_eq(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_eq(s1, s2));
}

PGDLLEXPORT Datum Span_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_ne);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is different from the second one
 * @sqlfn span_ne()
 * @sqlop @p <>
 */
Datum
Span_ne(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_ne(s1, s2));
}

PGDLLEXPORT Datum Span_cmp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_cmp);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span
 * is less than, equal to, or greater than the second one
 * @sqlfn span_cmp()
 */
Datum
Span_cmp(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_INT32(span_cmp(s1, s2));
}

PGDLLEXPORT Datum Span_lt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_lt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is less than the second one
 * @sqlfn span_lt()
 * @sqlop @p <
 */
Datum
Span_lt(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_lt(s1, s2));
}

PGDLLEXPORT Datum Span_le(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_le);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is less than or equal to the second one
 * @sqlfn span_le()
 * @sqlop @p <=
 */
Datum
Span_le(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_le(s1, s2));
}

PGDLLEXPORT Datum Span_ge(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_ge);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is greater than or equal to the second
 * one
 * @sqlfn span_ge()
 * @sqlop @p >=
 */
Datum
Span_ge(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_ge(s1, s2));
}

PGDLLEXPORT Datum Span_gt(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_gt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is greater than the second one
 * @sqlfn span_gt()
 * @sqlop @p >
 */
Datum
Span_gt(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_gt(s1, s2));
}

/*****************************************************************************
 * Functions for defining hash indexes
 *****************************************************************************/

PGDLLEXPORT Datum Span_hash(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_hash);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 32-bit hash value of a span
 * @sqlfn span_hash()
 */
Datum
Span_hash(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_UINT32(span_hash(s));
}

PGDLLEXPORT Datum Span_hash_extended(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_hash_extended);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 64-bit hash value of a span using a seed
 * @sqlfn hash_extended()
 */
Datum
Span_hash_extended(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  PG_RETURN_UINT64(span_hash_extended(s, seed));
}

/******************************************************************************/
