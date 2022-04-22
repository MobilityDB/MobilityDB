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

#ifdef MEOS
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
#endif

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
 * @ingroup libmeos_time_comp
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
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period is different from the second one.
 */
bool
period_ne(const Period *p1, const Period *p2)
{
  return (! period_eq(p1, p2));
}

/* B-tree comparator */

/**
 * @ingroup libmeos_time_comp
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
 * @ingroup libmeos_time_comp
 * @brief Return true if the first period is less than the second one.
 */
bool
period_lt(const Period *p1, const Period *p2)
{
  int cmp = period_cmp(p1, p2);
  return (cmp < 0);
}

/**
 * @ingroup libmeos_time_comp
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
 * @ingroup libmeos_time_comp
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
 * @ingroup libmeos_time_comp
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
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_in);
/**
 * Input function for periods
 */
PGDLLEXPORT Datum
Period_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Period *result = period_parse(&input, true);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_out);
/**
 * Output function for periods
 */
PGDLLEXPORT Datum
Period_out(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_CSTRING(period_to_string(p));
}

PG_FUNCTION_INFO_V1(Period_send);
/**
 * Send function for periods
 */
PGDLLEXPORT Datum
Period_send(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  StringInfoData buf;
  pq_begintypsend(&buf);
  period_write(p, &buf);
  PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(Period_recv);
/**
 * Receive function for periods
 */
PGDLLEXPORT Datum
Period_recv(PG_FUNCTION_ARGS)
{
  StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
  PG_RETURN_POINTER(period_read(buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_constructor2);
/**
 * Construct a period from the two arguments
 */
PGDLLEXPORT Datum
Period_constructor2(PG_FUNCTION_ARGS)
{
  TimestampTz lower = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz upper = PG_GETARG_TIMESTAMPTZ(1);
  Period *period;
  period = period_make(lower, upper, true, false);
  PG_RETURN_PERIOD_P(period);
}


PG_FUNCTION_INFO_V1(Period_constructor4);
/**
 * Construct a period from the four arguments
 */
PGDLLEXPORT Datum
Period_constructor4(PG_FUNCTION_ARGS)
{
  TimestampTz lower = PG_GETARG_TIMESTAMPTZ(0);
  TimestampTz upper = PG_GETARG_TIMESTAMPTZ(1);
  bool lower_inc = PG_GETARG_BOOL(2);
  bool upper_inc = PG_GETARG_BOOL(3);
  Period *period;
  period = period_make(lower, upper, lower_inc, upper_inc);
  PG_RETURN_PERIOD_P(period);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_to_period);
/**
 * Cast the timestamp value as a period
 */
PGDLLEXPORT Datum
Timestamp_to_period(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Period *result = timestamp_period(t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_to_tstzrange);
/**
 * Convert the period as a tstzrange value
 */
PGDLLEXPORT Datum
Period_to_tstzrange(PG_FUNCTION_ARGS)
{
  Period *period = PG_GETARG_PERIOD_P(0);
  RangeType *range;
  range = range_make(TimestampTzGetDatum(period->lower),
    TimestampTzGetDatum(period->upper), period->lower_inc,
    period->upper_inc, T_TIMESTAMPTZ);
  PG_RETURN_POINTER(range);
}

PG_FUNCTION_INFO_V1(Tstzrange_to_period);
/**
 * Convert the tstzrange value as a period
 */
PGDLLEXPORT Datum
Tstzrange_to_period(PG_FUNCTION_ARGS)
{
  RangeType *range = PG_GETARG_RANGE_P(0);
  TypeCacheEntry *typcache;
  char flags = range_get_flags(range);
  RangeBound lower;
  RangeBound upper;
  bool empty;
  Period *period;

  typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  assert(typcache->rngelemtype->type_id == TIMESTAMPTZOID);
  if (flags & RANGE_EMPTY)
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range cannot be empty")));
  if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
    ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
      errmsg("Range bounds cannot be infinite")));

  range_deserialize(typcache, range, &lower, &upper, &empty);
  period = period_make(DatumGetTimestampTz(lower.val),
    DatumGetTimestampTz(upper.val), lower.inclusive, upper.inclusive);
  PG_RETURN_POINTER(period);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* period -> timestamptz functions */

PG_FUNCTION_INFO_V1(Period_lower);
/**
 * Return the lower bound value
 */
PGDLLEXPORT Datum
Period_lower(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_TIMESTAMPTZ(p->lower);
}

PG_FUNCTION_INFO_V1(Period_upper);
/**
 * Return the upper bound value
 */
PGDLLEXPORT Datum
Period_upper(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_TIMESTAMPTZ(p->upper);
}

/* period -> bool functions */

PG_FUNCTION_INFO_V1(Period_lower_inc);
/**
 * Return true if the lower bound value is inclusive
 */
PGDLLEXPORT Datum
Period_lower_inc(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_BOOL(p->lower_inc != 0);
}

PG_FUNCTION_INFO_V1(Period_upper_inc);
/**
 * Return true if the upper bound value is inclusive
 */
PGDLLEXPORT Datum
Period_upper_inc(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  PG_RETURN_BOOL(p->upper_inc != 0);
}

PG_FUNCTION_INFO_V1(Period_duration);
/**
 * Return the duration of the period
 */
PGDLLEXPORT Datum
Period_duration(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *result = period_duration(p);
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
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Period *result = period_copy(p);
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
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Period *result = period_copy(p);
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
  Period *p = PG_GETARG_PERIOD_P(0);
  Interval *start = PG_GETARG_INTERVAL_P(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Period *result = period_copy(p);
  period_shift_tscale(start, duration, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_eq);
/**
 * Return true if the first period is equal to the second one
 */
PGDLLEXPORT Datum
Period_eq(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_eq(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_ne);
/**
 * Return true if the first period is different from the second one
 */
PGDLLEXPORT Datum
Period_ne(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_ne(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_cmp);
/**
 * Return -1, 0, or 1 depending on whether the first period
 * is less than, equal, or greater than the second one
 */
PGDLLEXPORT Datum
Period_cmp(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_INT32(period_cmp(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_lt);
/**
 * Return true if the first period is less than the second one
 */
PGDLLEXPORT Datum
Period_lt(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_lt(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_le);
/**
 * Return true if the first period is less than or equal to the second one
 */
PGDLLEXPORT Datum
Period_le(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_le(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_ge);
/**
 * Return true if the first period is greater than or equal to the second one
 */
PGDLLEXPORT Datum
Period_ge(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_ge(p1, p2));
}

PG_FUNCTION_INFO_V1(Period_gt);
/**
 * Return true if the first period is greater than the second one
 */
PGDLLEXPORT Datum
Period_gt(PG_FUNCTION_ARGS)
{
  Period *p1 = PG_GETARG_PERIOD_P(0);
  Period *p2 = PG_GETARG_PERIOD_P(1);
  PG_RETURN_BOOL(period_gt(p1, p2));
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Period_hash);
/**
 * Return the 32-bit hash value of a period.
 */
PGDLLEXPORT Datum
Period_hash(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  uint32 result = period_hash(p);
  PG_RETURN_UINT32(result);
}

PG_FUNCTION_INFO_V1(Period_hash_extended);
/**
 * Return the 64-bit hash value of a period obtained with a seed.
 */
PGDLLEXPORT Datum
Period_hash_extended(PG_FUNCTION_ARGS)
{
  Period *p = PG_GETARG_PERIOD_P(0);
  Datum seed = PG_GETARG_DATUM(1);
  uint64 result = period_hash_extended(p, seed);
  PG_RETURN_UINT64(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
