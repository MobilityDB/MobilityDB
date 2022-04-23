/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @file span.c
 * @brief General functions for spans (a.k.a. ranges) composed of two `Datum`
 * values and two Boolean values stating whether the bounds are inclusive.
 */

#include "general/span.h"

/* PostgreSQL */
#include <assert.h>
#include <access/hash.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/periodset.h"
#include "general/span_ops.h"
#include "general/temporal.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
#include "general/rangetypes_ext.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Deconstruct the span
 *
 * @param[in] s Span value
 * @param[out] lower,upper Bounds
 */
void
span_deserialize(const Span *s, SpanBound *lower, SpanBound *upper)
{
  if (lower)
  {
    lower->d = s->lower;
    lower->inclusive = s->lower_inc;
    lower->lower = true;
    lower->spantype = s->spantype;
    lower->basetype = s->basetype;
  }
  if (upper)
  {
    upper->d = s->upper;
    upper->inclusive = s->upper_inc;
    upper->lower = false;
    upper->spantype = s->spantype;
    upper->basetype = s->basetype;
  }
}

/*****************************************************************************/

/**
 * Return true if the type is a time type
 */
bool
span_type(CachedType spantype)
{
  if (spantype == T_INTSPAN || spantype == T_FLOATSPAN ||
      spantype == T_TIMESTAMPSPAN)
    return true;
  return false;
}

/**
 * Ensure that the type corresponds to a time type
 */
void
ensure_span_type(CachedType spantype)
{
  if (! span_type(spantype))
    elog(ERROR, "unknown span type: %d", spantype);
  return;
}

/**
 * Ensures that the base type is supported by MobilityDB
 */
void
ensure_span_basetype(CachedType basetype)
{
  if (basetype != T_INT4 && basetype != T_FLOAT8 && basetype != T_TIMESTAMPTZ)
    elog(ERROR, "unknown span base type: %d", basetype);
  return;
}

/**
 * Return true if the first value is less than the second one
 * (base type dispatch function)
 */
int
span_elem_cmp(Datum l, Datum r, CachedType typel, CachedType typer)
{
  ensure_span_basetype(typel);
  if (typel != typer)
    ensure_span_basetype(typer);
  if (typel == T_INT4 && typer == T_INT4)
    return (DatumGetInt32(l) < DatumGetInt32(r)) ? -1 :
      ((DatumGetInt32(l) > DatumGetInt32(r)) ? 1 : 0);
  if (typel == T_INT4 && typer == T_FLOAT8)
    return float8_cmp_internal((double) DatumGetInt32(l), DatumGetFloat8(r));
  if (typel == T_FLOAT8 && typer == T_INT4)
    return float8_cmp_internal(DatumGetFloat8(l), (double) DatumGetInt32(r));
  if (typel == T_FLOAT8 && typer == T_FLOAT8)
    return float8_cmp_internal(DatumGetFloat8(l), DatumGetFloat8(r));
  if (typel == T_TIMESTAMPTZ && typer == T_TIMESTAMPTZ)
    return timestamp_cmp_internal(DatumGetTimestampTz(l),
      DatumGetTimestampTz(r));
  elog(ERROR, "unknown span_elem_cmp function for span base type: %d", typel);
}

/**
 * Compare two span boundary points, returning <0, 0, or >0 according to
 * whether b1 is less than, equal to, or greater than b2.
 *
 * The boundaries can be any combination of upper and lower; so it's useful
 * for a variety of operators.
 *
 * The simple case is when b1 and b2 are both inclusive, in which
 * case the result is just a comparison of the values held in b1 and b2.
 *
 * If a bound is exclusive, then we need to know whether it's a lower bound,
 * in which case we treat the boundary point as "just greater than" the held
 * value; or an upper bound, in which case we treat the boundary point as
 * "just less than" the held value.
 *
 * There is only one case where two boundaries compare equal but are not
 * identical: when both bounds are inclusive and hold the same value,
 * but one is an upper bound and the other a lower bound.
 */
int
span_bound_cmp(const SpanBound *b1, const SpanBound *b2)
{
  /* Compare the values */
  int32 result = span_elem_cmp(b1->d, b2->d, b1->basetype, b2->basetype);

  /*
   * If the comparison is not equal and the bounds are both inclusive or
   * both exclusive, we're done. If they compare equal, we still have to
   * consider whether the boundaries are inclusive or exclusive.
  */
  if (result == 0)
  {
    if (! b1->inclusive && ! b2->inclusive)
    {
      /* both are exclusive */
      if (b1->lower == b2->lower)
        return 0;
      else
        return b1->lower ? 1 : -1;
    }
    else if (! b1->inclusive)
      return b1->lower ? 1 : -1;
    else if (! b2->inclusive)
      return b2->lower ? -1 : 1;
  }

  return result;
}

/**
 * Comparison function for sorting span bounds.
 */
int
span_bound_qsort_cmp(const void *a1, const void *a2)
{
  SpanBound *b1 = (SpanBound *) a1;
  SpanBound *b2 = (SpanBound *) a2;
  return span_bound_cmp(b1, b2);
}

/**
 * Compare the lower bound of two spans, returning <0, 0, or >0 according to
 * whether a's bound is less than, equal to, or greater than b's bound.
 *
 * @note This function does the same as span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_lower_cmp(const Span *a, const Span *b)
{
  int result = span_elem_cmp(a->lower, b->lower, a->basetype, b->basetype);
  if (result == 0)
  {
    if (a->lower_inc == b->lower_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (a->lower_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * Compare the upper bound of two spans, returning <0, 0, or >0 according to
 * whether a's bound is less than, equal to, or greater than b's bound.
 *
 * @note This function does the same as span_bound_cmp but avoids
 * deserializing the spans into lower and upper bounds
 */
int
span_upper_cmp(const Span *a, const Span *b)
{
  int result = span_elem_cmp(a->upper, b->upper, a->basetype, b->basetype);
  if (result == 0)
  {
    if (a->upper_inc == b->upper_inc)
      /* both are inclusive or exclusive */
      return 0;
    else if (a->upper_inc)
      /* first is inclusive and second is exclusive */
      return 1;
    else
      /* first is exclusive and second is inclusive */
      return -1;
  }
  return result;
}

/**
 * @ingroup libmeos_span_constructor
 * @brief Construct a span from the bounds.
 */
Span *
span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  CachedType spantype)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = (Span *) palloc(sizeof(Span));
  span_set(lower, upper, lower_inc, upper_inc, spantype, s);
  return s;
}

/**
 * @ingroup libmeos_span_constructor
 * @brief Set the span from the argument values.
 */
void
span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  CachedType spantype, Span *s)
{
  CachedType basetype = spantype_basetype(spantype);
  int cmp = span_elem_cmp(lower, upper, basetype, basetype);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Span lower bound must be less than or equal to span upper bound")));

  /* error check: if bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && !(lower_inc && upper_inc))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Span cannot be empty")));

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(s, 0, sizeof(Span));
  /* Now fill in the span */
  s->lower = lower;
  s->upper = upper;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  s->spantype = spantype;
  s->basetype = basetype;
}

/**
 * @ingroup libmeos_span_constructor
 * @brief Return a copy of the span.
 */
Span *
span_copy(const Span *s)
{
  Span *result = (Span *) palloc(sizeof(Span));
  memcpy((char *) result, (char *) s, sizeof(Span));
  return result;
}

/**
 * @ingroup libmeos_span_accessor
 * @brief Return the number of seconds of the span as a float8 value
 */
float8
span_to_secs(Datum upper, Datum lower)
{
  return ((float8) upper - (float8) lower) / USECS_PER_SEC;
}

/**
 * Normalize an array of spans
 *
 * The input spans may overlap and may be non contiguous.
 * The normalized spans are new spans that must be freed.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @pre It is supposed that the spans are sorted.
 * This should be ensured by the calling function !!!
 */
Span **
spanarr_normalize(Span **spans, int count, int *newcount)
{
  int k = 0;
  Span **result = palloc(sizeof(Span *) * count);
  Span *current = spans[0];
  bool isnew = false;
  for (int i = 1; i < count; i++)
  {
    Span *next = spans[i];
    if (overlaps_span_span(current, next) ||
      adjacent_span_span(current, next))
    {
      /* Compute the union of the spans */
      Span *newspan = span_copy(current);
      span_expand(next, newspan);
      if (isnew)
        pfree(current);
      current = newspan;
      isnew = true;
    }
    else
    {
      if (isnew)
        result[k++] = current;
      else
        result[k++] = span_copy(current);
      current = next;
      isnew = false;
    }
  }
  if (isnew)
    result[k++] = current;
  else
    result[k++] = span_copy(current);

  *newcount = k;
  return result;
}

/**
 * Return the smallest span that contains s1 and s2
 *
 * This differs from regular span union in a critical ways:
 * It won'd throw an error for non-adjacent s1 and s2, but just absorb
 * the intervening values into the result span.
 */
Span *
span_super_union(const Span *s1, const Span *s2)
{
  Span *result = span_copy(s1);
  span_expand(s2, result);
  return result;
}

/**
 * @ingroup libmeos_span_transf
 * @brief Expand the second span with the first one
 */
void
span_expand(const Span *s1, Span *s2)
{
  int cmp1 = span_elem_cmp(s1->lower, s2->lower, s1->basetype, s2->basetype);
  int cmp2 = span_elem_cmp(s1->upper, s2->upper, s1->basetype, s2->basetype);
  bool lower1 = cmp1 < 0 || (cmp1 == 0 && (s2->lower_inc || ! s1->lower_inc));
  bool upper1 = cmp2 > 0 || (cmp2 == 0 && (s2->upper_inc || ! s1->upper_inc));
  s2->lower = lower1 ? s2->lower : s1->lower;
  s2->lower_inc = lower1 ? s2->lower_inc : s1->lower_inc;
  s2->upper = upper1 ? s2->upper : s1->upper;
  s2->upper_inc = upper1 ? s2->upper_inc : s1->upper_inc;
  return;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Remove the quotes from the string representation of a span
 */
static void
unquote(char *str)
{
  char *last = str;
  while (*str != '\0')
  {
    if (*str != '"')
    {
      *last++ = *str;
    }
    str++;
  }
  *last = '\0';
  return;
}

/**
 * @ingroup libmeos_span_input_output
 * @brief Return the string representation of the span.
 */
char *
span_to_string(const Span *s)
{
  Oid basetypid = type_oid(s->basetype);
  char *lower = call_output(basetypid, TimestampTzGetDatum(s->lower));
  char *upper = call_output(basetypid, TimestampTzGetDatum(s->upper));
  StringInfoData buf;
  initStringInfo(&buf);
  appendStringInfoChar(&buf, s->lower_inc ? (char) '[' : (char) '(');
  appendStringInfoString(&buf, lower);
  appendStringInfoString(&buf, ", ");
  appendStringInfoString(&buf, upper);
  appendStringInfoChar(&buf, s->upper_inc ? (char) ']' : (char) ')');
  unquote(buf.data);
  pfree(lower); pfree(upper);
  return buf.data;
}

/**
 * @ingroup libmeos_span_input_output
 * @brief Write the binary representation of the time value into the buffer.
 */
void
span_write(const Span *s, StringInfo buf)
{
  Oid basetypid = type_oid(s->basetype);
  bytea *lower = call_send(basetypid, TimestampTzGetDatum(s->lower));
  bytea *upper = call_send(basetypid, TimestampTzGetDatum(s->upper));
  pq_sendbytes(buf, VARDATA(lower), VARSIZE(lower) - VARHDRSZ);
  pq_sendbytes(buf, VARDATA(upper), VARSIZE(upper) - VARHDRSZ);
  pq_sendbyte(buf, s->lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, s->upper_inc ? (uint8) 1 : (uint8) 0);
  pfree(lower);
  pfree(upper);
}

/**
 * @ingroup libmeos_span_input_output
 * @brief Return a new time value from its binary representation
 * read from the buffer.
 */
Span *
span_read(StringInfo buf, CachedType spantype)
{
  Oid spantypid = type_oid(spantype_basetype(spantype));
  Span *result = (Span *) palloc0(sizeof(Span));
  result->lower = call_recv(spantypid, buf);
  result->upper = call_recv(spantypid, buf);
  result->lower_inc = (char) pq_getmsgbyte(buf);
  result->upper_inc = (char) pq_getmsgbyte(buf);
  return result;
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @ingroup libmeos_span_cast
 * @brief Cast an element value as a span
 */
Span *
elem_span(Datum d, CachedType basetype)
{
  CachedType spantype = basetype_spantype(basetype);
  ensure_span_type(spantype);
  Span *result = span_make(d, d, true, true, spantype);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#ifdef MEOS
/**
 * @ingroup libmeos_span_cast
 * @brief Return the lower bound value
 */
Datum
span_lower(Span *s)
{
  return s->lower;
}

/**
 * @ingroup libmeos_span_accessor
 * @brief Return the upper bound value
 */
Datum
span_upper(Span *s)
{
  return s->upper;
}

/**
 * @ingroup libmeos_span_accessor
 * @brief Return true if the lower bound value is inclusive
 */
bool
span_lower_inc(Span *s)
{
  return s->lower_inc != 0;
}

/**
 * @ingroup libmeos_span_accessor
 * @brief Return true if the upper bound value is inclusive
 */
bool
span_upper_inc(Span *s)
{
  return s->upper_inc != 0;
}
#endif

/**
 * @ingroup libmeos_span_accessor
 * @brief Return the duration of the span as an interval.
 */
double
span_distance(const Span *s)
{
  return distance_elem_elem(s->lower, s->upper, s->basetype, s->basetype);
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

// /**
 // * @ingroup libmeos_span_transf
 // * @brief Shift and/or scale the span by the two intervals.
 // */
// void
// span_shift_tscale(const Interval *start, const Interval *duration,
  // Span *result)
// {
  // assert(start != NULL || duration != NULL);
  // if (duration != NULL)
    // ensure_valid_duration(duration);
  // bool instant = (result->lower == result->upper);

  // if (start != NULL)
  // {
    // result->lower = DatumGetTimestampTz(DirectFunctionCall2(
      // timestamptz_pl_interval, TimestampTzGetDatum(result->lower),
      // PointerGetDatum(start)));
    // if (instant)
      // result->upper = result->lower;
    // else
      // result->upper = DatumGetTimestampTz(DirectFunctionCall2(
        // timestamptz_pl_interval, TimestampTzGetDatum(result->upper),
        // PointerGetDatum(start)));
  // }
  // if (duration != NULL && ! instant)
    // result->upper =
      // DatumGetTimestampTz(DirectFunctionCall2(timestamptz_pl_interval,
         // TimestampTzGetDatum(result->lower), PointerGetDatum(duration)));
  // return;
// }

/*****************************************************************************
 * Btree support
 *****************************************************************************/

/**
 * @ingroup libmeos_span_comp
 * @brief Return true if the first span is equal to the second one.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
span_eq(const Span *s1, const Span *s2)
{
  if (s1->lower != s2->lower || s1->upper != s2->upper ||
    s1->lower_inc != s2->lower_inc || s1->upper_inc != s2->upper_inc)
    return false;
  return true;
}

/**
 * @ingroup libmeos_span_comp
 * @brief Return true if the first span is different from the second one.
 */
bool
span_ne(const Span *s1, const Span *s2)
{
  return (! span_eq(s1, s2));
}

/* B-tree comparator */

/**
 * @ingroup libmeos_span_comp
 * @brief Return -1, 0, or 1 depending on whether the first span
 * is less than, equal, or greater than the second one.
 *
 * @note Function used for B-tree comparison
 */
int
span_cmp(const Span *s1, const Span *s2)
{
  int cmp = span_elem_cmp(s1->lower, s2->lower, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->lower_inc != s2->lower_inc)
    return s1->lower_inc ? -1 : 1;
  cmp = span_elem_cmp(s1->upper, s2->upper, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->upper_inc != s2->upper_inc)
    return s1->upper_inc ? 1 : -1;
  return 0;
}

/* Inequality operators using the span_cmp function */

/**
 * @ingroup libmeos_span_comp
 * @brief Return true if the first span is less than the second one.
 */
bool
span_lt(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_span_comp
 * @brief Return true if the first span is less than or equal to the
 * second one.
 */
bool
span_le(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp <= 0);
}

/**
 * @ingroup libmeos_span_comp
 * @brief Return true if the first span is greater than or equal to the
 * second one.
 */
bool
span_ge(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp >= 0);
}

/**
 * @ingroup libmeos_span_comp
 * @brief Return true if the first span is greater than the second one.
 */
bool
span_gt(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp > 0);
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

/**
 * @ingroup libmeos_span_accessor
 * @brief Return the 32-bit hash value of a span.
 */
uint32
span_hash(const Span *s)
{
  uint32 result;
  char flags = '\0';
  uint32 lower_hash;
  uint32 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Apply the hash function to each bound */
  lower_hash = DatumGetUInt32(call_function1(hashint8, TimestampTzGetDatum(s->lower)));
  upper_hash = DatumGetUInt32(call_function1(hashint8, TimestampTzGetDatum(s->upper)));

  /* Merge hashes of flags and bounds */
  result = DatumGetUInt32(hash_uint32((uint32) flags));
  result ^= lower_hash;
  result = (result << 1) | (result >> 31);
  result ^= upper_hash;

  return result;
}

/**
 * @ingroup libmeos_span_accessor
 * @brief Return the 64-bit hash value of a span obtained with a seed.
 */
uint64
span_hash_extended(const Span *s, Datum seed)
{
  uint64 result;
  char flags = '\0';
  uint64 lower_hash;
  uint64 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Apply the hash function to each bound */
  lower_hash = DatumGetUInt64(call_function2(hashint8extended,
    TimestampTzGetDatum(s->lower), seed));
  upper_hash = DatumGetUInt64(call_function2(hashint8extended,
    TimestampTzGetDatum(s->upper), seed));

  /* Merge hashes of flags and bounds */
  result = DatumGetUInt64(hash_uint32_extended((uint32) flags,
    DatumGetInt64(seed)));
  result ^= lower_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
  result ^= upper_hash;

  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_in);
/**
 * Input function for periods
 */
PGDLLEXPORT Datum
Span_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid spantypid = PG_GETARG_OID(1);
  Span *result = span_parse(&input, oid_type(spantypid), true);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_out);
/**
 * Output function for periods
 */
PGDLLEXPORT Datum
Span_out(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_CSTRING(span_to_string(s));
}

PG_FUNCTION_INFO_V1(Span_send);
/**
 * Send function for periods
 */
PGDLLEXPORT Datum
Span_send(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  span_write(s, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Span_recv);
/**
 * Receive function for periods
 */
PGDLLEXPORT Datum
Span_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  CachedType spantype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  PG_RETURN_POINTER(span_read(buf, spantype));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_constructor2);
/**
 * Construct a span from the two arguments
 */
PGDLLEXPORT Datum
Span_constructor2(PG_FUNCTION_ARGS)
{
  Datum lower = PG_GETARG_DATUM(0);
  Datum upper = PG_GETARG_DATUM(1);
  CachedType spantype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  Span *span;
  span = span_make(lower, upper, true, false, spantype);
  PG_RETURN_PERIOD_P(span);
}


PG_FUNCTION_INFO_V1(Span_constructor4);
/**
 * Construct a span from the four arguments
 */
PGDLLEXPORT Datum
Span_constructor4(PG_FUNCTION_ARGS)
{
  Datum lower = PG_GETARG_DATUM(0);
  Datum upper = PG_GETARG_DATUM(1);
  bool lower_inc = PG_GETARG_BOOL(2);
  bool upper_inc = PG_GETARG_BOOL(3);
  CachedType spantype = oid_type(get_fn_expr_rettype(fcinfo->flinfo));
  Span *span;
  span = span_make(lower, upper, lower_inc, upper_inc, spantype);
  PG_RETURN_PERIOD_P(span);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Elem_to_span);
/**
 * Cast the timestamp value as a span
 */
PGDLLEXPORT Datum
Elem_to_span(PG_FUNCTION_ARGS)
{
  Datum d = PG_GETARG_DATUM(0);
  CachedType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Span *result = elem_span(d, basetype);
  PG_RETURN_POINTER(result);
}

// PG_FUNCTION_INFO_V1(Span_to_tstzrange);
// /**
 // * Convert the span as a tstzrange value
 // */
// PGDLLEXPORT Datum
// Span_to_tstzrange(PG_FUNCTION_ARGS)
// {
  // Span *span = PG_GETARG_SPAN_P(0);
  // RangeType *range;
  // range = range_make(TimestampTzGetDatum(span->lower),
    // TimestampTzGetDatum(span->upper), span->lower_inc,
    // span->upper_inc, T_TIMESTAMPTZ);
  // PG_RETURN_POINTER(range);
// }

// PG_FUNCTION_INFO_V1(Tstzrange_to_period);
// /**
 // * Convert the tstzrange value as a span
 // */
// PGDLLEXPORT Datum
// Tstzrange_to_period(PG_FUNCTION_ARGS)
// {
  // RangeType *range = PG_GETARG_RANGE_P(0);
  // TypeCacheEntry *typcache;
  // char flags = range_get_flags(range);
  // RangeBound lower;
  // RangeBound upper;
  // bool empty;
  // Span *span;

  // typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  // assert(typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  // if (flags & RANGE_EMPTY)
    // ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      // errmsg("Range cannot be empty")));
  // if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
    // ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      // errmsg("Range bounds cannot be infinite")));

  // range_deserialize(typcache, range, &lower, &upper, &empty);
  // span = span_make(DatumGetTimestampTz(lower.val),
    // DatumGetTimestampTz(upper.val), lower.inclusive, upper.inclusive);
  // PG_RETURN_POINTER(span);
// }

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* span -> timestamptz functions */

PG_FUNCTION_INFO_V1(Span_lower);
/**
 * Return the lower bound value
 */
PGDLLEXPORT Datum
Span_lower(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_DATUM(s->lower);
}

PG_FUNCTION_INFO_V1(Span_upper);
/**
 * Return the upper bound value
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
 * Return true if the lower bound value is inclusive
 */
PGDLLEXPORT Datum
Span_lower_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->lower_inc != 0);
}

PG_FUNCTION_INFO_V1(Span_upper_inc);
/**
 * Return true if the upper bound value is inclusive
 */
PGDLLEXPORT Datum
Span_upper_inc(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BOOL(s->upper_inc != 0);
}

// PG_FUNCTION_INFO_V1(Span_duration);
// /**
 // * Return the duration of the span
 // */
// PGDLLEXPORT Datum
// Span_duration(PG_FUNCTION_ARGS)
// {
  // Span *s = PG_GETARG_SPAN_P(0);
  // Interval *result = span_duration(s);
  // PG_RETURN_POINTER(result);
// }

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

// PG_FUNCTION_INFO_V1(Span_shift);
// /**
 // * Shift the span value by the interval
 // */
// PGDLLEXPORT Datum
// Span_shift(PG_FUNCTION_ARGS)
// {
  // Span *s = PG_GETARG_SPAN_P(0);
  // Interval *start = PG_GETARG_INTERVAL_P(1);
  // Span *result = span_copy(s);
  // span_shift_tscale(start, NULL, result);
  // PG_RETURN_POINTER(result);
// }

// PG_FUNCTION_INFO_V1(Span_tscale);
// /**
 // * Shift the span  value by the interval
 // */
// PGDLLEXPORT Datum
// Span_tscale(PG_FUNCTION_ARGS)
// {
  // Span *s = PG_GETARG_SPAN_P(0);
  // Interval *duration = PG_GETARG_INTERVAL_P(1);
  // Span *result = span_copy(s);
  // span_shift_tscale(NULL, duration, result);
  // PG_RETURN_POINTER(result);
// }

// PG_FUNCTION_INFO_V1(Span_shift_tscale);
// /**
 // * Shift the span value by the interval
 // */
// PGDLLEXPORT Datum
// Span_shift_tscale(PG_FUNCTION_ARGS)
// {
  // Span *s = PG_GETARG_SPAN_P(0);
  // Interval *start = PG_GETARG_INTERVAL_P(1);
  // Interval *duration = PG_GETARG_INTERVAL_P(2);
  // Span *result = span_copy(s);
  // span_shift_tscale(start, duration, result);
  // PG_RETURN_POINTER(result);
// }

/*****************************************************************************
 * Btree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Span_eq);
/**
 * Return true if the first span is equal to the second one
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
 * Return true if the first span is different from the second one
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
 * Return -1, 0, or 1 depending on whether the first span
 * is less than, equal, or greater than the second one
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
 * Return true if the first span is less than the second one
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
 * Return true if the first span is less than or equal to the second one
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
 * Return true if the first span is greater than or equal to the second one
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
 * Return true if the first span is greater than the second one
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
 * Return the 32-bit hash value of a span.
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
 * Return the 64-bit hash value of a span obtained with a seed.
 */
PGDLLEXPORT Datum
Span_hash_extended(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Datum seed = PG_GETARG_DATUM(1);
  uint64 result = span_hash_extended(s, seed);
  PG_RETURN_UINT64(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
