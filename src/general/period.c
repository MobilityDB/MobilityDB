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
 * @file period.c
 * @brief General functions for time periods composed of two `TimestampTz`
 * values and two Boolean values stating whether the bounds are inclusive.
 */

#include "general/period.h"

/* PostgreSQL */
#include <assert.h>
#include <access/hash.h>
#include <utils/builtins.h>
/* MobilityDB */
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/temporal.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
#include "general/rangetypes_ext.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Deconstruct the period
 *
 * @param[in] p Period value
 * @param[out] lower,upper Bounds
 */
void
period_deserialize(const Period *p, PeriodBound *lower, PeriodBound *upper)
{
  if (lower)
  {
    lower->t = p->lower;
    lower->inclusive = p->lower_inc;
    lower->lower = true;
  }
  if (upper)
  {
    upper->t = p->upper;
    upper->inclusive = p->upper_inc;
    upper->lower = false;
  }
}

/*****************************************************************************/

/**
 * Compare two period boundary points, returning <0, 0, or >0 according to
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
period_bound_cmp(const PeriodBound *b1, const PeriodBound *b2)
{
  int32 result;

  /* Compare the values */
  result = timestamp_cmp_internal(b1->t, b2->t);

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
 * Comparison function for sorting period bounds.
 */
int
period_bound_qsort_cmp(const void *a1, const void *a2)
{
  PeriodBound *b1 = (PeriodBound *) a1;
  PeriodBound *b2 = (PeriodBound *) a2;
  return period_bound_cmp(b1, b2);
}

/**
 * Compare the lower bound of two periods, returning <0, 0, or >0 according to
 * whether a's bound is less than, equal to, or greater than b's bound.
 *
 * @note This function does the same as period_bound_cmp but avoids
 * deserializing the periods into lower and upper bounds
 */
int
period_lower_cmp(const Period *a, const Period *b)
{
  int result = timestamp_cmp_internal(a->lower, b->lower);
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
 * Compare the upper bound of two periods, returning <0, 0, or >0 according to
 * whether a's bound is less than, equal to, or greater than b's bound.
 *
 * @note This function does the same as period_bound_cmp but avoids
 * deserializing the periods into lower and upper bounds
 */
int
period_upper_cmp(const Period *a, const Period *b)
{
  int result = timestamp_cmp_internal(a->upper, b->upper);
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
 * @ingroup libmeos_time_constructor
 * @brief Construct a period from the bounds.
 */
Period *
period_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc)
{
  /* Note: zero-fill is done in the period_set function */
  Period *period = (Period *) palloc(sizeof(Period));
  period_set(lower, upper, lower_inc, upper_inc, period);
  return period;
}

/**
 * @ingroup libmeos_time_constructor
 * @brief Set the period from the argument values.
 */
void
period_set(TimestampTz lower, TimestampTz upper, bool lower_inc,
  bool upper_inc, Period *p)
{
  int cmp = timestamp_cmp_internal(lower, upper);
  /* error check: if lower bound value is above upper, it's wrong */
  if (cmp > 0)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Period lower bound must be less than or equal to period upper bound")));

  /* error check: if bounds are equal, and not both inclusive, period is empty */
  if (cmp == 0 && !(lower_inc && upper_inc))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Period cannot be empty")));

  /* Note: zero-fill is required here, just as in heap tuples */
  memset(p, 0, sizeof(Period));
  /* Now fill in the period */
  p->lower = lower;
  p->upper = upper;
  p->lower_inc = lower_inc;
  p->upper_inc = upper_inc;
}

/**
 * @ingroup libmeos_time_constructor
 * @brief Return a copy of the period.
 */
Period *
period_copy(const Period *p)
{
  Period *result = (Period *) palloc(sizeof(Period));
  memcpy((char *) result, (char *) p, sizeof(Period));
  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the number of seconds of the period as a float8 value
 */
float8
period_to_secs(TimestampTz upper, TimestampTz lower)
{
  return ((float8) upper - (float8) lower) / USECS_PER_SEC;
}

/**
 * Normalize an array of periods
 *
 * The input periods may overlap and may be non contiguous.
 * The normalized periods are new periods that must be freed.
 *
 * @param[in] periods Array of periods
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @pre It is supposed that the periods are sorted.
 * This should be ensured by the calling function !!!
 */
Period **
periodarr_normalize(Period **periods, int count, int *newcount)
{
  int k = 0;
  Period **result = palloc(sizeof(Period *) * count);
  Period *current = periods[0];
  bool isnew = false;
  for (int i = 1; i < count; i++)
  {
    Period *next = periods[i];
    if (overlaps_period_period(current, next) ||
      adjacent_period_period(current, next))
    {
      /* Compute the union of the periods */
      Period *newper = period_copy(current);
      period_expand(next, newper);
      if (isnew)
        pfree(current);
      current = newper;
      isnew = true;
    }
    else
    {
      if (isnew)
        result[k++] = current;
      else
        result[k++] = period_copy(current);
      current = next;
      isnew = false;
    }
  }
  if (isnew)
    result[k++] = current;
  else
    result[k++] = period_copy(current);

  *newcount = k;
  return result;
}

/**
 * Return the smallest period that contains p1 and p2
 *
 * This differs from regular period union in a critical ways:
 * It won't throw an error for non-adjacent p1 and p2, but just absorb
 * the intervening values into the result period.
 */
Period *
period_super_union(const Period *p1, const Period *p2)
{
  Period *result = period_copy(p1);
  period_expand(p2, result);
  return result;
}

/**
 * @ingroup libmeos_time_transf
 * @brief Expand the second period with the first one
 */
void
period_expand(const Period *p1, Period *p2)
{
  int cmp1 = timestamp_cmp_internal(p2->lower, p1->lower);
  int cmp2 = timestamp_cmp_internal(p2->upper, p1->upper);
  bool lower1 = cmp1 < 0 || (cmp1 == 0 && (p2->lower_inc || ! p1->lower_inc));
  bool upper1 = cmp2 > 0 || (cmp2 == 0 && (p2->upper_inc || ! p1->upper_inc));
  p2->lower = lower1 ? p2->lower : p1->lower;
  p2->lower_inc = lower1 ? p2->lower_inc : p1->lower_inc;
  p2->upper = upper1 ? p2->upper : p1->upper;
  p2->upper_inc = upper1 ? p2->upper_inc : p1->upper_inc;
  return;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Remove the quotes from the string representation of a period
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
 * @ingroup libmeos_time_input_output
 * @brief Return the string representation of the period.
 */
char *
period_to_string(const Period *p)
{
  char *lower = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(p->lower));
  char *upper = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(p->upper));
  StringInfoData buf;
  initStringInfo(&buf);
  appendStringInfoChar(&buf, p->lower_inc ? (char) '[' : (char) '(');
  appendStringInfoString(&buf, lower);
  appendStringInfoString(&buf, ", ");
  appendStringInfoString(&buf, upper);
  appendStringInfoChar(&buf, p->upper_inc ? (char) ']' : (char) ')');
  unquote(buf.data);
  pfree(lower); pfree(upper);
  return buf.data;
}

/**
 * @ingroup libmeos_time_input_output
 * @brief Write the binary representation of the time value into the buffer.
 */
void
period_write(const Period *p, StringInfo buf)
{
  bytea *lower = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(p->lower));
  bytea *upper = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(p->upper));
  pq_sendbytes(buf, VARDATA(lower), VARSIZE(lower) - VARHDRSZ);
  pq_sendbytes(buf, VARDATA(upper), VARSIZE(upper) - VARHDRSZ);
  pq_sendbyte(buf, p->lower_inc ? (uint8) 1 : (uint8) 0);
  pq_sendbyte(buf, p->upper_inc ? (uint8) 1 : (uint8) 0);
  pfree(lower);
  pfree(upper);
}

/**
 * @ingroup libmeos_time_input_output
 * @brief Return a new time value from its binary representation
 * read from the buffer.
 */
Period *
period_read(StringInfo buf)
{
  Period *result = (Period *) palloc0(sizeof(Period));
  result->lower = call_recv(TIMESTAMPTZOID, buf);
  result->upper = call_recv(TIMESTAMPTZOID, buf);
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
 * @ingroup libmeos_time_cast
 * @brief Cast a timestamp value as a period
 */
Period *
timestamp_period(TimestampTz t)
{
  Period *result = period_make(t, t, true, true);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_time_cast
 * @brief Return the lower bound value
 */
TimestampTz
period_lower(Period *p)
{
  return p->lower;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the upper bound value
 */
TimestampTz
period_upper(Period *p)
{
  return p->upper;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return true if the lower bound value is inclusive
 */
bool
period_lower_inc(Period *p)
{
  return p->lower_inc != 0;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return true if the upper bound value is inclusive
 */
bool
period_upper_inc(Period *p)
{
  return p->upper_inc != 0;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the duration of the period as an interval.
 */
Interval *
period_duration(const Period *p)
{
  return DatumGetIntervalP(call_function2(timestamp_mi,
    TimestampTzGetDatum(p->upper), TimestampTzGetDatum(p->lower)));
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * @ingroup libmeos_time_transf
 * @brief Shift and/or scale the period by the two intervals.
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
 * @ingroup libmeos_time_oper_comp
 * @brief Return true if the first period is equal to the second one.
 *
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
period_eq(const Period *p1, const Period *p2)
{
  if (p1->lower != p2->lower || p1->upper != p2->upper ||
    p1->lower_inc != p2->lower_inc || p1->upper_inc != p2->upper_inc)
    return false;
  return true;
}

/**
 * @ingroup libmeos_time_oper_comp
 * @brief Return true if the first period is different from the second one.
 */
bool
period_ne(const Period *p1, const Period *p2)
{
  return (! period_eq(p1, p2));
}

/* B-tree comparator */

/**
 * @ingroup libmeos_time_oper_comp
 * @brief Return -1, 0, or 1 depending on whether the first period
 * is less than, equal, or greater than the second one.
 *
 * @note Function used for B-tree comparison
 */
int
period_cmp(const Period *p1, const Period *p2)
{
  int cmp = timestamp_cmp_internal(p1->lower, p2->lower);
  if (cmp != 0)
    return cmp;
  if (p1->lower_inc != p2->lower_inc)
    return p1->lower_inc ? -1 : 1;
  cmp = timestamp_cmp_internal(p1->upper, p2->upper);
  if (cmp != 0)
    return cmp;
  if (p1->upper_inc != p2->upper_inc)
    return p1->upper_inc ? 1 : -1;
  return 0;
}

/* Inequality operators using the period_cmp function */

/**
 * @ingroup libmeos_time_oper_comp
 * @brief Return true if the first period is less than the second one.
 */
bool
period_lt(const Period *p1, const Period *p2)
{
  int cmp = period_cmp(p1, p2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_time_oper_comp
 * @brief Return true if the first period is less than or equal to the
 * second one.
 */
bool
period_le(const Period *p1, const Period *p2)
{
  int cmp = period_cmp(p1, p2);
  return (cmp <= 0);
}

/**
 * @ingroup libmeos_time_oper_comp
 * @brief Return true if the first period is greater than or equal to the
 * second one.
 */
bool
period_ge(const Period *p1, const Period *p2)
{
  int cmp = period_cmp(p1, p2);
  return (cmp >= 0);
}

/**
 * @ingroup libmeos_time_oper_comp
 * @brief Return true if the first period is greater than the second one.
 */
bool
period_gt(const Period *p1, const Period *p2)
{
  int cmp = period_cmp(p1, p2);
  return (cmp > 0);
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the 32-bit hash value of a period.
 */
uint32
period_hash(const Period *p)
{
  uint32 result;
  char flags = '\0';
  uint32 lower_hash;
  uint32 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (p->lower_inc)
    flags |= 0x01;
  if (p->upper_inc)
    flags |= 0x02;

  /* Apply the hash function to each bound */
  lower_hash = DatumGetUInt32(call_function1(hashint8, TimestampTzGetDatum(p->lower)));
  upper_hash = DatumGetUInt32(call_function1(hashint8, TimestampTzGetDatum(p->upper)));

  /* Merge hashes of flags and bounds */
  result = DatumGetUInt32(hash_uint32((uint32) flags));
  result ^= lower_hash;
  result = (result << 1) | (result >> 31);
  result ^= upper_hash;

  return result;
}

/**
 * @ingroup libmeos_time_accessor
 * @brief Return the 64-bit hash value of a period obtained with a seed.
 */
uint64
period_hash_extended(const Period *p, Datum seed)
{
  uint64 result;
  char flags = '\0';
  uint64 lower_hash;
  uint64 upper_hash;

  /* Create flags from the lower_inc and upper_inc values */
  if (p->lower_inc)
    flags |= 0x01;
  if (p->upper_inc)
    flags |= 0x02;

  /* Apply the hash function to each bound */
  lower_hash = DatumGetUInt64(call_function2(hashint8extended,
    TimestampTzGetDatum(p->lower), seed));
  upper_hash = DatumGetUInt64(call_function2(hashint8extended,
    TimestampTzGetDatum(p->upper), seed));

  /* Merge hashes of flags and bounds */
  result = DatumGetUInt64(hash_uint32_extended((uint32) flags,
    DatumGetInt64(seed)));
  result ^= lower_hash;
  result = ROTATE_HIGH_AND_LOW_32BITS(result);
  result ^= upper_hash;

  return result;
}

/*****************************************************************************/
