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
 * @brief General functions for spans (a.k.a. ranges) composed of two `Datum`
 * values and two Boolean values stating whether the bounds are inclusive.
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
#include "general/type_out.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/span.h"
#include "pg_general/tnumber_mathfuncs.h"
#include "pg_general/type_util.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_in);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Input function for periods
 * @sqlfunc span_in()
 */
PGDLLEXPORT Datum
Span_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid spantypid = PG_GETARG_OID(1);
  Span *result = span_in(input, oid_type(spantypid));
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_out);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Output function for periods
 * @sqlfunc span_out()
 */
PGDLLEXPORT Datum
Span_out(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_CSTRING(span_out(s, Int32GetDatum(OUT_DEFAULT_DECIMAL_DIGITS)));
}

/* Needed for time aggregation */

/**
 * @brief Return a span from its binary representation read from a buffer.
 */
Span *
span_recv(StringInfo buf)
{
  Span *result = palloc0(sizeof(Span));
  result->spantype = (char) pq_getmsgbyte(buf);
  result->basetype = spantype_basetype(result->spantype);
  result->lower = call_recv(result->basetype, buf);
  result->upper = call_recv(result->basetype, buf);
  result->lower_inc = (char) pq_getmsgbyte(buf);
  result->upper_inc = (char) pq_getmsgbyte(buf);
  return result;
}

/**
 * @brief Write the binary representation of a span into a buffer.
 */
void
span_write(const Span *s, StringInfo buf)
{
  pq_sendbyte(buf, s->spantype);
  bytea *lower = call_send(s->basetype, s->lower);
  bytea *upper = call_send(s->basetype, s->upper);
  pq_sendbytes(buf, VARDATA(lower), VARSIZE(lower) - VARHDRSZ);
  pq_sendbytes(buf, VARDATA(upper), VARSIZE(upper) - VARHDRSZ);
  pq_sendbyte(buf, s->lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, s->upper_inc ? (uint8) 1 : (uint8) 0);
  pfree(lower); pfree(upper);
}

PG_FUNCTION_INFO_V1(Span_recv);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Generic receive function for spans
 * @sqlfunc span_recv()
 */
PGDLLEXPORT Datum
Span_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  Span *result = span_from_wkb((uint8_t *) buf->data, buf->len);
  /* Set cursor to the end of buffer (so the backend is happy) */
  buf->cursor = buf->len;
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_send);
/*
 * @ingroup mobilitydb_setspan_inout
 * @brief Generic send function for spans
 * @sqlfunc span_send()
 */
PGDLLEXPORT Datum
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
 * Output in WKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_as_text);
/**
 * @ingroup mobilitydb_setspan_inout
 * @brief Output function for periods
 * @sqlfunc asText()
 */
PGDLLEXPORT Datum
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

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_constructor);
/**
 * @ingroup mobilitydb_setspan_constructor
 * @brief Construct a span from the four arguments
 * @sqlfunc intspan(), bigintspan(), floatspan(), period()
 */
PGDLLEXPORT Datum
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
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Value_to_span);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Cast a value as a span
 * @sqlfunc intspan(), bigintspan(), floatspan(), period()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Value_to_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Span *result = value_to_span(d, basetype);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_to_range);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Convert a span as a range value
 * @sqlfunc int4range(), tstzrange()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Span_to_range(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  assert(span->basetype == T_INT4 || span->basetype == T_TIMESTAMPTZ);
  RangeType *range;
  range = range_make(span->lower, span->upper, span->lower_inc,
    span->upper_inc, span->basetype);
  PG_RETURN_POINTER(range);
}

/**
 * @brief Convert the PostgreSQL range value as a span value
 */
Span *
range_to_span(RangeType *range, TypeCacheEntry *typcache)
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
  meosType basetype = (typcache->rngelemtype->type_id == INT4OID) ?
    T_INT4 : T_TIMESTAMPTZ;
  Span *result = span_make(lower.val, upper.val, lower.inclusive,
    upper.inclusive, basetype);
  return result;
}

PG_FUNCTION_INFO_V1(Range_to_span);
/**
 * @ingroup mobilitydb_setspan_cast
 * @brief Convert the PostgreSQL range value as a span value
 * @sqlfunc intspan(), period()
 * @sqlop @p ::
 */
PGDLLEXPORT Datum
Range_to_span(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == INT4OID ||
    typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  Span *span = range_to_span(range, typcache);
  PG_RETURN_POINTER(span);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* span -> timestamptz functions */

PG_FUNCTION_INFO_V1(Span_lower);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the lower bound value
 * @sqlfunc lower()
 */
PGDLLEXPORT Datum
Span_lower(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->lower);
}

PG_FUNCTION_INFO_V1(Span_upper);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the upper bound value
 * @sqlfunc upper()
 */
PGDLLEXPORT Datum
Span_upper(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->upper);
}

/* span -> bool functions */

PG_FUNCTION_INFO_V1(Span_lower_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the lower bound value is inclusive
 * @sqlfunc lower_inc()
 */
PGDLLEXPORT Datum
Span_lower_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->lower_inc != 0);
}

PG_FUNCTION_INFO_V1(Span_upper_inc);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return true if the upper bound value is inclusive
 * @sqlfunc lower_inc()
 */
PGDLLEXPORT Datum
Span_upper_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->upper_inc != 0);
}

PG_FUNCTION_INFO_V1(Span_width);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the width of a numeric span
 * @sqlfunc width()
 */
PGDLLEXPORT Datum
Span_width(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  double result = span_width(s);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Period_duration);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the duration of the period
 * @sqlfunc duration()
 */
PGDLLEXPORT Datum
Period_duration(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *result = period_duration(s);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @brief Set the precision of the float span to the number of decimal places.
 */
Span *
floatspan_round(const Span *span, Datum size)
{
  /* Set precision of bounds */
  Datum lower = datum_round_float(span->lower, size);
  Datum upper = datum_round_float(span->upper, size);
  /* Create resulting span */
  Span *result = span_make(lower, upper, span->lower_inc, span->upper_inc,
    span->basetype);
  return result;
}

PG_FUNCTION_INFO_V1(Floatspan_round);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Set the precision of the float span to the number of decimal places
 * @sqlfunc round()
 */
PGDLLEXPORT Datum
Floatspan_round(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Span *result = floatspan_round(span, size);
  PG_RETURN_POINTER(result);
}

/******************************************************************************/

PG_FUNCTION_INFO_V1(Span_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift the span by a value
 * @sqlfunc shift()
 */
PGDLLEXPORT Datum
Span_shift(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum shift = PG_GETARG_DATUM(1);
  Span *result = span_copy(s);
  span_shift(result, shift);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_shift);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift the period value by the interval
 * @sqlfunc shift()
 */
PGDLLEXPORT Datum
Period_shift(PG_FUNCTION_ARGS)
{
  Span *p = PG_GETARG_SPAN_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Span *result = span_copy(p);
  period_shift_tscale(result, shift, NULL);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_tscale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift the period  value by the interval
 * @sqlfunc tscale()
 */
PGDLLEXPORT Datum
Period_tscale(PG_FUNCTION_ARGS)
{
  Span *p = PG_GETARG_SPAN_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Span *result = span_copy(p);
  period_shift_tscale(result, NULL, duration);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_shift_tscale);
/**
 * @ingroup mobilitydb_setspan_transf
 * @brief Shift the period value by the interval
 * @sqlfunc shiftTscale()
 */
PGDLLEXPORT Datum
Period_shift_tscale(PG_FUNCTION_ARGS)
{
  Span *p = PG_GETARG_SPAN_P(0);
  Interval *shift = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Span *result = span_copy(p);
  period_shift_tscale(result, shift, duration);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_eq);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is equal to the second one
 * @sqlfunc span_eq()
 * @sqlop @p =
 */
PGDLLEXPORT Datum
Span_eq(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_eq(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_ne);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is different from the second one
 * @sqlfunc span_ne()
 * @sqlop @p <>
 */
PGDLLEXPORT Datum
Span_ne(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_ne(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_cmp);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return -1, 0, or 1 depending on whether the first span
 * is less than, equal, or greater than the second one
 * @sqlfunc span_cmp()
 */
PGDLLEXPORT Datum
Span_cmp(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_INT32(span_cmp(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_lt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is less than the second one
 * @sqlfunc span_lt()
 * @sqlop @p <
 */
PGDLLEXPORT Datum
Span_lt(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_lt(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_le);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is less than or equal to the second one
 * @sqlfunc span_le()
 * @sqlop @p <=
 */
PGDLLEXPORT Datum
Span_le(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_le(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_ge);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is greater than or equal to the second one
 * @sqlfunc span_ge()
 * @sqlop @p >=
 */
PGDLLEXPORT Datum
Span_ge(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_ge(s1, s2));
}

PG_FUNCTION_INFO_V1(Span_gt);
/**
 * @ingroup mobilitydb_setspan_comp
 * @brief Return true if the first span is greater than the second one
 * @sqlfunc span_gt()
 * @sqlop @p >
 */
PGDLLEXPORT Datum
Span_gt(PG_FUNCTION_ARGS)
{
  Span *s1 = PG_GETARG_SPAN_P(0);
  Span *s2 = PG_GETARG_SPAN_P(1);
  PG_RETURN_BOOL(span_gt(s1, s2));
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_hash);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 32-bit hash value of a span.
 * @sqlfunc span_hash()
 */
PGDLLEXPORT Datum
Span_hash(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint32 result = span_hash(s);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Span_hash_extended);
/**
 * @ingroup mobilitydb_setspan_accessor
 * @brief Return the 64-bit hash value of a span obtained with a seed.
 * @sqlfunc span_hash_extended()
 */
PGDLLEXPORT Datum
Span_hash_extended(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = span_hash_extended(s, seed);
  PG_RETURN_UINT64(result);
}

/******************************************************************************/
