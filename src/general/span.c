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

/* C */
#include <assert.h>
/* PostgreSQL */
#if POSTGRESQL_VERSION_NUMBER < 130000
  #include <access/hash.h>
#else
  #include <common/hashfn.h>
#endif
#include <libpq/pqformat.h>
#include <utils/fmgrprotos.h>
/* MobilityDB */
#include "general/pg_call.h"
#include "general/periodset.h"
#include "general/span_ops.h"
#include "general/temporal.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
#include "general/tnumber_mathfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Deconstruct a span
 *
 * @param[in] s Span value
 * @param[out] lower,upper Bounds
 */
void
span_deserialize(const Span *s, SpanBound *lower, SpanBound *upper)
{
  if (lower)
  {
    lower->val = s->lower;
    lower->inclusive = s->lower_inc;
    lower->lower = true;
    lower->spantype = s->spantype;
    lower->basetype = s->basetype;
  }
  if (upper)
  {
    upper->val = s->upper;
    upper->inclusive = s->upper_inc;
    upper->lower = false;
    upper->spantype = s->spantype;
    upper->basetype = s->basetype;
  }
}

/*
 * @brief Construct a span value from the bounds
 *
 * This does not force canonicalization of the span value.  In most cases,
 * external callers should only be canonicalization functions.
 */
Span *
span_serialize(SpanBound *lower, SpanBound *upper)
{
  assert(lower->basetype == upper->basetype);

  /* If lower bound value is above upper, it's wrong */
  int cmp = datum_cmp2(lower->val, upper->val, lower->basetype,
    upper->basetype);

  if (cmp > 0)
    elog(ERROR, "span lower bound must be less than or equal to span upper bound");

  /* If bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && ! (lower->inclusive && upper->inclusive))
    elog(ERROR, "a span cannot be empty");

  Span *result = span_make(lower->val, upper->val, lower->inclusive,
    upper->inclusive, lower->basetype);
  return result;
}

/*****************************************************************************/

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
  int32 result = datum_cmp2(b1->val, b2->val, b1->basetype, b2->basetype);

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
  int result = datum_cmp2(a->lower, b->lower, a->basetype, b->basetype);
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
  int result = datum_cmp2(a->upper, b->upper, a->basetype, b->basetype);
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
 * @brief Canonicalize discrete spans.
 */
void
span_canonicalize(Span *s)
{
  if (s->basetype == T_INT4)
  {
    if (! s->lower_inc)
    {
      s->lower = Int32GetDatum(DatumGetInt32(s->lower) + 1);
      s->lower_inc = true;
    }

    if (s->upper_inc)
    {
      s->upper = Int32GetDatum(DatumGetInt32(s->upper) + 1);
      s->upper_inc = false;
    }
  }
}

/**
 * @brief Normalize an array of spans
 *
 * The input spans may overlap and may be non contiguous.
 * The normalized spans are new spans that must be freed.
 *
 * @param[in] spans Array of spans
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 */
Span **
spanarr_normalize(Span **spans, int count, int *newcount)
{
  /* Sort the spans before normalization */
  spanarr_sort(spans, count);
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
 * Get the bounds of a span as double values.
 *
 * @param[in] s Input span
 * @param[out] xmin, xmax Lower and upper bounds
 */
void
span_bounds(const Span *s, double *xmin, double *xmax)
{
  ensure_tnumber_spantype(s->spantype);
  if (s->spantype == T_INTSPAN)
  {
    *xmin = (double)(DatumGetInt32(s->lower));
    /* intspans are in canonical form so their upper bound is exclusive */
    *xmax = (double)(DatumGetInt32(s->upper) - 1);
  }
  else /* s->spantype == T_FLOATSPAN */
  {
    *xmin = DatumGetFloat8(s->lower);
    *xmax = DatumGetFloat8(s->upper);
  }
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its string representation.
 */
Span *
span_in(char *str, CachedType spantype)
{
  return span_parse(&str, spantype, true);
}

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
 * @ingroup libmeos_spantime_input_output
 * @brief Return the string representation of a span.
 */
char *
span_out(const Span *s)
{
  char *lower = basetype_output(s->basetype, s->lower);
  char *upper = basetype_output(s->basetype, s->upper);
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
 * @ingroup libmeos_spantime_input_output
 * @brief Return a span from its binary representation read from a buffer.
 */
Span *
span_recv(StringInfo buf)
{
  Span *result = (Span *) palloc0(sizeof(Span));
  result->spantype = (char) pq_getmsgbyte(buf);
  result->basetype = spantype_basetype(result->spantype);
  result->lower = basetype_recv(result->basetype, buf);
  result->upper = basetype_recv(result->basetype, buf);
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
  bytea *lower = basetype_send(s->basetype, s->lower);
  bytea *upper = basetype_send(s->basetype, s->upper);
  pq_sendbytes(buf, VARDATA(lower), VARSIZE(lower) - VARHDRSZ);
  pq_sendbytes(buf, VARDATA(upper), VARSIZE(upper) - VARHDRSZ);
  pq_sendbyte(buf, s->lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, s->upper_inc ? (uint8) 1 : (uint8) 0);
  pfree(lower); pfree(upper);
}

/**
 * @ingroup libmeos_spantime_input_output
 * @brief Return the binary representation of a span.
 */
bytea *
span_send(const Span *s)
{
  StringInfoData buf;
  pq_begintypsend(&buf);
  span_write(s, &buf);
  return (bytea *) pq_endtypsend(&buf);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Construct a span from the bounds.
 */
Span *
span_make(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  CachedType basetype)
{
  /* Note: zero-fill is done in the span_set function */
  Span *s = (Span *) palloc(sizeof(Span));
  span_set(lower, upper, lower_inc, upper_inc, basetype, s);
  return s;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Set a span from the arguments.
 */
void
span_set(Datum lower, Datum upper, bool lower_inc, bool upper_inc,
  CachedType basetype, Span *s)
{
  CachedType spantype = basetype_spantype(basetype);
  int cmp = datum_cmp2(lower, upper, basetype, basetype);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
    elog(ERROR, "Span lower bound must be less than or equal to span upper bound");

  /* error check: if bounds are equal, and not both inclusive, span is empty */
  if (cmp == 0 && !(lower_inc && upper_inc))
    elog(ERROR, "Span cannot be empty");

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(s, 0, sizeof(Span));
  /* Now fill in the span */
  s->lower = lower;
  s->upper = upper;
  s->lower_inc = lower_inc;
  s->upper_inc = upper_inc;
  s->spantype = spantype;
  s->basetype = basetype;
  span_canonicalize(s);
  return;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Return a copy of a span.
 */
Span *
span_copy(const Span *s)
{
  Span *result = (Span *) palloc(sizeof(Span));
  memcpy((char *) result, (char *) s, sizeof(Span));
  return result;
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast an element as a span
 */
Span *
elem_to_span(Datum d, CachedType basetype)
{
  ensure_span_basetype(basetype);
  Span *result = span_make(d, d, true, true, basetype);
  return result;
}

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast a timestamp as a period
 */
Period *
timestamp_to_period(TimestampTz t)
{
  Period *result = span_make(t, t, true, true, T_TIMESTAMPTZ);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

#ifdef MEOS
/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the lower bound of a span
 */
Datum
span_lower(Span *s)
{
  return s->lower;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the upper bound of a span
 */
Datum
span_upper(Span *s)
{
  return s->upper;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return true if the lower bound of a span is inclusive
 */
bool
span_lower_inc(Span *s)
{
  return s->lower_inc != 0;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return true if the upper bound of a span is inclusive
 */
bool
span_upper_inc(Span *s)
{
  return s->upper_inc != 0;
}
#endif

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the width of a span as a double.
 */
double
span_width(const Span *s)
{
  return distance_elem_elem(s->lower, s->upper, s->basetype, s->basetype);
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the duration of a period as an interval.
 */
Interval *
period_duration(const Span *s)
{
#if POSTGRESQL_VERSION_NUMBER >= 140000
  return pg_timestamp_mi(s->upper, s->lower);
#else
  return (Interval *) DatumGetPointer(call_function2(timestamp_mi,
    s->upper, s->lower));
#endif
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_transf
 * @brief Expand the second span with the first one
 */
void
span_expand(const Span *s1, Span *s2)
{
  assert(s1->spantype == s2->spantype);
  int cmp1 = datum_cmp(s2->lower, s1->lower, s1->basetype);
  int cmp2 = datum_cmp(s2->upper, s1->upper, s1->basetype);
  bool lower1 = cmp1 < 0 || (cmp1 == 0 && (s2->lower_inc || ! s1->lower_inc));
  bool upper1 = cmp2 > 0 || (cmp2 == 0 && (s2->upper_inc || ! s1->upper_inc));
  s2->lower = lower1 ? s2->lower : s1->lower;
  s2->lower_inc = lower1 ? s2->lower_inc : s1->lower_inc;
  s2->upper = upper1 ? s2->upper : s1->upper;
  s2->upper_inc = upper1 ? s2->upper_inc : s1->upper_inc;
  return;
}

/**
 * @ingroup libmeos_spantime_transf
 * @brief Shift and/or scale a period by the intervals.
 */
void
period_shift_tscale(const Interval *start, const Interval *duration,
  Period *result)
{
  assert(start != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  bool instant = (result->lower == result->upper);

  if (start != NULL)
  {
    result->lower = DatumGetTimestampTz(DirectFunctionCall2(
      timestamptz_pl_interval, TimestampTzGetDatum(result->lower),
      PointerGetDatum(start)));
    if (instant)
      result->upper = result->lower;
    else
      result->upper = DatumGetTimestampTz(DirectFunctionCall2(
        timestamptz_pl_interval, TimestampTzGetDatum(result->upper),
        PointerGetDatum(start)));
  }
  if (duration != NULL && ! instant)
    result->upper =
      DatumGetTimestampTz(DirectFunctionCall2(timestamptz_pl_interval,
         TimestampTzGetDatum(result->lower), PointerGetDatum(duration)));
  return;
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is equal to the second one.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
span_eq(const Span *s1, const Span *s2)
{
  assert(s1->spantype == s2->spantype);
  if (s1->lower != s2->lower || s1->upper != s2->upper ||
    s1->lower_inc != s2->lower_inc || s1->upper_inc != s2->upper_inc)
    return false;
  return true;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is different from the second one.
 */
bool
span_ne(const Span *s1, const Span *s2)
{
  return (! span_eq(s1, s2));
}

/* B-tree comparator */

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first span
 * is less than, equal, or greater than the second one.
 *
 * @note Function used for B-tree comparison
 */
int
span_cmp(const Span *s1, const Span *s2)
{
  int cmp = datum_cmp2(s1->lower, s2->lower, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->lower_inc != s2->lower_inc)
    return s1->lower_inc ? -1 : 1;
  cmp = datum_cmp2(s1->upper, s2->upper, s1->basetype, s2->basetype);
  if (cmp != 0)
    return cmp;
  if (s1->upper_inc != s2->upper_inc)
    return s1->upper_inc ? 1 : -1;
  return 0;
}

/* Inequality operators using the span_cmp function */

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span is less than the second one.
 */
bool
span_lt(const Span *s1, const Span *s2)
{
  int cmp = span_cmp(s1, s2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_spantime_comp
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
 * @ingroup libmeos_spantime_comp
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
 * @ingroup libmeos_spantime_comp
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

uint32
span_hash(const Span *s)
{
  /* Create flags from the lower_inc and upper_inc values */
  char flags = '\0';
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Create type from the spantype and basetype values */
  uint16 type = ((uint16) (s->spantype) << 8) | (uint16) (s->basetype);
#if POSTGRESQL_VERSION_NUMBER >= 140000
  uint32 type_hash = hash_uint32((int32) type);

  /* Apply the hash function to each bound */
  uint32 lower_hash = pg_hashint8(s->lower);
  uint32 upper_hash = pg_hashint8(s->upper);
#else
  uint32 type_hash = DatumGetUInt32(call_function1(hashint4, type));

  /* Apply the hash function to each bound */
  uint32 lower_hash = DatumGetUInt32(call_function1(hashint8, s->lower));
  uint32 upper_hash = DatumGetUInt32(call_function1(hashint8, s->upper));
#endif
  /* Merge hashes of flags, type, and bounds */
  uint32 result = DatumGetUInt32(hash_uint32((uint32) flags));
  result ^= type_hash;
  result = (result << 1) | (result >> 31);
  result ^= lower_hash;
  result = (result << 1) | (result >> 31);
  result ^= upper_hash;

  return result;
}

uint64
span_hash_extended(const Span *s, Datum seed)
{
  uint64 result;
  char flags = '\0';
  uint64 type_hash;
  uint64 lower_hash;
  uint64 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (s->lower_inc)
    flags |= 0x01;
  if (s->upper_inc)
    flags |= 0x02;

  /* Create type from the spantype and basetype values */
  uint16 type = ((uint16) (s->spantype) << 8) | (uint16) (s->basetype);
#if POSTGRESQL_VERSION_NUMBER >= 140000
  type_hash = DatumGetUInt64(hash_uint32_extended(type, seed));

  /* Apply the hash function to each bound */
  lower_hash = pg_hashint8extended(s->lower, seed);
  upper_hash = pg_hashint8extended(s->upper, seed);
#else
  type_hash = DatumGetUInt64(call_function2(hashint4extended, type, seed));

  /* Apply the hash function to each bound */
  lower_hash = DatumGetUInt64(call_function2(hashint8extended, s->lower, seed));
  upper_hash = DatumGetUInt64(call_function2(hashint8extended, s->upper, seed));
#endif
  /* Merge hashes of flags and bounds */
  result = DatumGetUInt64(hash_uint32_extended((uint32) flags,
    DatumGetInt64(seed)));
  result ^= type_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
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
  Span *result = span_in(input, oid_type(spantypid));
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
  PG_RETURN_CSTRING(span_out(s));
}

PG_FUNCTION_INFO_V1(Span_send);
/**
 * Send function for periods
 */
PGDLLEXPORT Datum
Span_send(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  PG_RETURN_BYTEA_P(span_send(s));
}

PG_FUNCTION_INFO_V1(Span_recv);
/**
 * Receive function for periods
 */
PGDLLEXPORT Datum
Span_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(span_recv(buf));
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
  CachedType basetype = spantype_basetype(spantype);
  Span *span;
  span = span_make(lower, upper, true, false, basetype);
  PG_RETURN_SPAN_P(span);
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
  CachedType basetype = spantype_basetype(spantype);
  Span *span;
  span = span_make(lower, upper, lower_inc, upper_inc, basetype);
  PG_RETURN_SPAN_P(span);
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
  Span *result = elem_to_span(d, basetype);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Span_to_range);
/**
 * Convert the integer span as a integer range value
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

PG_FUNCTION_INFO_V1(Range_to_span);
/**
 * Convert the integer range value as a integer span
 */
PGDLLEXPORT Datum
Range_to_span(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache;
  char flags = range_get_flags(range);
  RangeBound lower, upper;
  bool empty;
  Span *span;

  typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == INT4OID ||
    typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  if (flags & RANGE_EMPTY)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range cannot be empty")));
  if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range bounds cannot be infinite")));

  CachedType basetype = (typcache->rngelemtype->type_id == INT4OID) ?
    T_INT4 : T_TIMESTAMPTZ;
  range_deserialize(typcache, range, &lower, &upper, &empty);
  span = span_make(lower.val, upper.val, lower.inclusive, upper.inclusive,
    basetype);
  PG_RETURN_POINTER(span);
}

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

PG_FUNCTION_INFO_V1(Period_duration);
/**
 * Return the duration of the period
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

PG_FUNCTION_INFO_V1(Period_shift);
/**
 * Shift the period value by the interval
 */
PGDLLEXPORT Datum
Period_shift(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Period *result = span_copy(p);
  period_shift_tscale(start, NULL, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_tscale);
/**
 * Shift the period  value by the interval
 */
PGDLLEXPORT Datum
Period_tscale(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Period *result = span_copy(p);
  period_shift_tscale(NULL, duration, result);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_shift_tscale);
/**
 * Shift the period value by the interval
 */
PGDLLEXPORT Datum
Period_shift_tscale(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_SPAN_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Period *result = span_copy(p);
  period_shift_tscale(start, duration, result);
  PG_RETURN_POINTER(result);
}

/******************************************************************************/

/**
 * @brief Set the precision of the float span to the number of decimal places.
 */
Span *
floatspan_round(Span *span, Datum size)
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
 * Set the precision of the float range to the number of decimal places
 */
PGDLLEXPORT Datum
Floatspan_round(PG_FUNCTION_ARGS)
{
  Span *span = PG_GETARG_SPAN_P(0);
  Datum size = PG_GETARG_DATUM(1);
  Span *result = floatspan_round(span, size);
  PG_RETURN_POINTER(result);
}

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
  uint64 seed = PG_GETARG_INT64(1);
  uint64 result = span_hash_extended(s, seed);
  PG_RETURN_UINT64(result);
}

#endif /* #ifndef MEOS */

/******************************************************************************/
