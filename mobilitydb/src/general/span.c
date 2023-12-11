/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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

#include "general/span.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type_d.h>
#include <libpq/pqformat.h>
#include <utils/rangetypes.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/tnumber_mathfuncs.h"
#include "general/type_out.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Span_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_in);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Input function for span types
 * @sqlfn span_in()
 */
Datum
Span_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid spantypid = PG_GETARG_OID(1);
  Span *result = span_in(input, oid_type(spantypid));
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_out);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Output function for span types
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
 * @brief Receive function for span types
 * @sqlfn span_recv()
 */
Datum
Span_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Span *result = span_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Span_send(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_send);
/*
 * @ingroup mobilitydb_setspan_inout
 * @brief Send function for span types
 * @sqlfn span_send()
 */
Datum
Span_send(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  uint8_t variant = 0;
  size_t wkb_size = VARSIZE_ANY_EXHDR(span);
  uint8_t *wkb = span_as_wkb(span, variant, &wkb_size);
  bytea *result = bstring2bytea(wkb, wkb_size);
  pfree(wkb);
  PG_FREE_IF_COPY(span, 0);
  PG_RETURN_BYTEA_P(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Span_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_constructor);
/**
 * @ingroup mobilitydb_setspan_constructor
 * @brief Construct a span from the arguments
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
  Span *span;
  span = span_make(lower, upper, lower_inc, upper_inc, basetype);
  PG_RETURN_SPAN_P(span);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Value_to_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_to_span);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a value to a span
 * @sqlfn span()
 * @sqlop @p ::
 */
Datum
Value_to_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Span *result = value_to_span(d, basetype);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Intspan_to_floatspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Intspan_to_floatspan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert an integer span to a float span
 * @sqlfn floatspan()
 * @sqlop @p ::
 */
Datum
Intspan_to_floatspan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Span *result = intspan_to_floatspan(s);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Floatspan_to_intspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_to_intspan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a float span to a integer span
 * @sqlfn intspan()
 * @sqlop @p ::
 */
Datum
Floatspan_to_intspan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Span *result = floatspan_to_intspan(s);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Datespan_to_tstzspan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Datespan_to_tstzspan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a date span to a timestamptz span
 * @sqlfn tstzspan()
 * @sqlop @p ::
 */
Datum
Datespan_to_tstzspan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Span *result = datespan_to_tstzspan(s);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tstzspan_to_datespan(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_to_datespan);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a timestamptz span to a date span
 * @sqlfn datespan()
 * @sqlop @p ::
 */
Datum
Tstzspan_to_datespan(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Span *result = tstzspan_to_datespan(s);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_to_range(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_to_range);
/**
 * @ingroup mobilitydb_setspan_conversion
 * @brief Convert a span to a range 
 * @sqlfn int4range(), tstzrange()
 * @sqlop @p ::
 */
Datum
Span_to_range(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  assert(span->basetype == T_INT4 || span->basetype == T_DATE ||
    span->basetype == T_TIMESTAMPTZ);
  RangeType *range;
  range = range_make(span->lower, span->upper, span->lower_inc,
    span->upper_inc, span->basetype);
  PG_RETURN_POINTER(range);
}

/**
 * @brief Convert a PostgreSQL range to a span
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
  meosType basetype;
  Oid type_id = typcache->rngelemtype->type_id;
  assert(type_id == INT4OID || type_id == DATEOID || type_id == TIMESTAMPTZOID);
  if (type_id == INT4OID)
    basetype = T_INT4;
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
 * @brief Convert a PostgreSQL range to a span
 * @sqlfn intspan(), tstzspan()
 * @sqlop @p ::
 */
Datum
Range_to_span(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == INT4OID ||
    typcache->rngelemtype->type_id == DATEOID ||
    typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  Span *result = palloc(sizeof(Span));
  range_set_span(range, typcache, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* span -> timestamptz functions */

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

PGDLLEXPORT Datum Span_width(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_width);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the width of a number span
 * @sqlfn width()
 */
Datum
Span_width(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  double result = span_width(s);
  PG_RETURN_FLOAT8(result);
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
  Interval *result = datespan_duration(s);
  PG_RETURN_POINTER(result);
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
  Interval *result = tstzspan_duration(s);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

PGDLLEXPORT Datum Numspan_shift(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span shifted by a value
 * @sqlfn shift()
 */
Datum
Numspan_shift(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Span *result = numspan_shift_scale(s, shift, 0, true, false);
  PG_RETURN_POINTER(result);
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
  Span *result = tstzspan_shift_scale(s, shift, NULL);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Numspan_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span scaled by a value
 * @sqlfn scale()
 */
Datum
Numspan_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum duration = PG_GETARG_DATUM(1);
  Span *result = numspan_shift_scale(s, 0, duration, false, true);
  PG_RETURN_POINTER(result);
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
  Span *result = tstzspan_shift_scale(s, NULL, duration);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Numspan_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numspan_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a number span shifted and scaled by the values
 * @sqlfn shiftScale()
 */
Datum
Numspan_shift_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Datum duration = PG_GETARG_DATUM(2);
  Span *result = numspan_shift_scale(s, shift, duration, true, true);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tstzspan_shift_scale(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_shift_scale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a timestamptz span shifted and scaled by the intervals
 * @sqlfn shiftScale()
 */
Datum
Tstzspan_shift_scale(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Span *result = tstzspan_shift_scale(s, shift, duration);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Floatspan_round(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Floatspan_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Return a float span where the precision of the values is set to
 * a number of decimal places
 * @sqlfn round()
 */
Datum
Floatspan_round(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  int maxdd = PG_GETARG_INT32(1);
  Span *result = floatspan_round(span, maxdd);
  PG_RETURN_POINTER(result);
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
 * is less than, equal, or greater than the second one
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
 * Hash support
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
  uint32 result = span_hash(s);
  PG_RETURN_UINT32(result);
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
  uint64 result = span_hash_extended(s, seed);
  PG_RETURN_UINT64(result);
}

/******************************************************************************/
