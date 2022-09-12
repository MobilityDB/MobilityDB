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
 * @brief General functions for set of disjoint periods.
 */

#include "general/periodset.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/span.h"
#include "general/temporal_util.h"
#include "general/time_ops.h"
#include "general/temporal_parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Return the location of the timestamp in the temporal sequence set
 * value using binary search
 *
 * If the timestamp is found, the index of the period is returned
 * in the output parameter. Otherwise, return a number encoding whether the
 * timestamp is before, between two periods, or after the period set.
 * For example, given a value composed of 3 periods and a timestamp, the
 * result of the function is as follows:
 * @code
 *               0          1          2
 *            |-----|    |-----|    |-----|
 * 1)    t^                                        => loc = 0
 * 2)            t^                                => loc = 0
 * 3)                 t^                           => loc = 1
 * 4)                            t^                => loc = 2
 * 5)                                        t^    => loc = 3
 * @endcode
 * @param[in] ps Period set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Return true if the timestamp is contained in the period set
 */
bool
periodset_find_timestamp(const PeriodSet *ps, TimestampTz t, int *loc)
{
  int first = 0;
  int last = ps->count - 1;
  int middle = 0; /* make compiler quiet */
  const Period *p = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    p = periodset_per_n(ps, middle);
    if (contains_period_timestamp(p, t))
    {
      *loc = middle;
      return true;
    }
    if (t <= (TimestampTz) p->lower)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (t >= (TimestampTz) p->upper)
    middle++;
  *loc = middle;
  return false;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a period set from its Well-Known Text (WKT) representation.
 */
PeriodSet *
periodset_in(const char *str)
{
  return periodset_parse(&str);
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return the Well-Known Text (WKT) representation of a period set.
 */
char *
periodset_out(const PeriodSet *ps)
{
  char **strings = palloc(sizeof(char *) * ps->count);
  size_t outlen = 0;

  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    /* The second argument of span_out is not used for periods */
    strings[i] = span_out(p, Int32GetDatum(0));
    outlen += strlen(strings[i]) + 2;
  }
  return stringarr_to_string(strings, ps->count, outlen, "", '{', '}');
}

/*****************************************************************************
 * Constructor functions
 ****************************************************************************/

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Construct a period set from an array of disjoint periods.
 *
 * For example, the memory structure of a PeriodSet with 3 periods is as
 * follows
 * @code
 * ---------------------------------------------------------------------------------
 * ( PeriodSet )_X | ( bbox )_X | ( Period_0 )_X | ( Period_1 )_X | ( Period_2 )_X |
 * ---------------------------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, and `bbox` is the
 * bounding box which is also a period.
 *
 * @param[in] periods Array of periods
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized
 * @sqlfunc periodset()
 */
PeriodSet *
periodset_make(const Period **periods, int count, bool normalize)
{
  /* Test the validity of the periods */
  for (int i = 0; i < count - 1; i++)
  {
    int cmp = timestamptz_cmp_internal(DatumGetTimestampTz(periods[i]->upper),
      DatumGetTimestampTz(periods[i + 1]->lower));
    if (cmp > 0 ||
      (cmp == 0 && periods[i]->upper_inc && periods[i + 1]->lower_inc))
      elog(ERROR, "Invalid value for period set");
  }

  Period **newperiods = (Period **) periods;
  int newcount = count;
  if (normalize && count > 1)
    newperiods = spanarr_normalize((Period **) periods, count, SORT_NO,
      &newcount);
  /* Notice that the first period is already declared in the struct */
  size_t memsize = double_pad(sizeof(PeriodSet)) +
    double_pad(sizeof(Period)) * (newcount - 1);
  PeriodSet *result = palloc0(memsize);
  SET_VARSIZE(result, memsize);
  result->count = newcount;

  /* Compute the bounding period */
  span_set(newperiods[0]->lower, newperiods[newcount - 1]->upper,
    newperiods[0]->lower_inc, newperiods[newcount - 1]->upper_inc,
    T_TIMESTAMPTZ, &result->period);
  /* Copy the period array */
  for (int i = 0; i < newcount; i++)
    memcpy(&result->elems[i], newperiods[i], sizeof(Span));
  /* Free after normalization */
  if (normalize && count > 1)
    pfree_array((void **) newperiods, newcount);
  return result;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Construct a period set from an array of periods and free the array
 * and the periods after the creation.
 *
 * @param[in] periods Array of periods
 * @param[in] count Number of elements in the array
 * @param[in] normalize True if the resulting value should be normalized.
 * @see periodset_make
 * @sqlfunc periodset()
 */
PeriodSet *
periodset_make_free(Period **periods, int count, bool normalize)
{
  if (count == 0)
  {
    pfree(periods);
    return NULL;
  }
  PeriodSet *result = periodset_make((const Period **) periods, count,
    normalize);
  pfree_array((void **) periods, count);
  return result;
}

/**
 * @ingroup libmeos_spantime_constructor
 * @brief Return a copy of a period set.
 */
PeriodSet *
periodset_copy(const PeriodSet *ps)
{
  PeriodSet *result = palloc(VARSIZE(ps));
  memcpy(result, ps, VARSIZE(ps));
  return result;
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast a timestamp as a period set.
 * @sqlop @p ::
 */
PeriodSet *
timestamp_to_periodset(TimestampTz t)
{
  Span p;
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, &p);
  PeriodSet *result = period_to_periodset(&p);
  return result;
}

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast a timestamp set as a period set.
 * @sqlop @p ::
 */
PeriodSet *
timestampset_to_periodset(const TimestampSet *ts)
{
  Period **periods = palloc(sizeof(Period *) * ts->count);
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    periods[i] = span_make(t, t, true, true, T_TIMESTAMPTZ);
  }
  PeriodSet *result = periodset_make_free(periods, ts->count, NORMALIZE_NO);
  return result;
}

/**
 * @ingroup libmeos_spantime_cast
 * @brief Cast a period as a period set.
 * @sqlop @p ::
 */
PeriodSet *
period_to_periodset(const Period *period)
{
  return periodset_make((const Period **) &period, 1, NORMALIZE_NO);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_int_spantime_accessor
 * @brief Return the n-th period of a period set.
 * @pre The argument @p index is less than the number of periods in the period
 * set
 */
const Period *
periodset_per_n(const PeriodSet *ps, int index)
{
  return (Period *) &ps->elems[index];
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the size in bytes of a period set
 * @sqlfunc memSize()
 */
int
periodset_mem_size(const PeriodSet *ps)
{
  return (int) VARSIZE(DatumGetPointer(ps));
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the timespan of a period set
 * @sqlfunc timespan()
 * @pymeosfunc timespan()
 */
Interval *
periodset_timespan(const PeriodSet *ps)
{
  const Period *p1 = periodset_per_n(ps, 0);
  const Period *p2 = periodset_per_n(ps, ps->count - 1);
  Interval *result = pg_timestamp_mi(p2->upper, p1->lower);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_cast
 * @brief Return the bounding period of a period set.
 * @sqlfunc period()
 * @sqlop @p ::
 * @pymeosfunc period()
 */
Period *
periodset_to_period(const PeriodSet *ps)
{
  Period *result = palloc(sizeof(Period));
  memcpy(result, &ps->period, sizeof(Span));
  return result;
}
#endif /* MEOS */

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the duration of a period set
 * @sqlfunc duration()
 * @pymeosfunc duration()
 */
Interval *
periodset_duration(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  Interval *result = pg_timestamp_mi(p->upper, p->lower);
  for (int i = 1; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    Interval *interval1 = pg_timestamp_mi(p->upper, p->lower);
    Interval *interval2 = pg_interval_pl(result, interval1);
    pfree(result); pfree(interval1);
    result = interval2;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the number of periods of a period set
 * @sqlfunc numPeriods()
 * @pymeosfunc numPeriods()
 */
int
periodset_num_periods(const PeriodSet *ps)
{
  return ps->count;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the start period of a period set
 * @sqlfunc startPeriod()
 * @pymeosfunc startPeriod()
 */
Period *
periodset_start_period(const PeriodSet *ps)
{
  Period *result = span_copy(periodset_per_n(ps, 0));
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the end period of a period set
 * @sqlfunc endPeriod()
 * @pymeosfunc endPeriod()
 */
Period *
periodset_end_period(const PeriodSet *ps)
{
  Period *result = span_copy(periodset_per_n(ps, ps->count - 1));
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the n-th period of a period set
 * @sqlfunc periodN()
 * @pymeosfunc periodN()
 */
Period *
periodset_period_n(const PeriodSet *ps, int i)
{
  Period *result = NULL;
  if (i >= 1 && i <= ps->count)
    result = span_copy(periodset_per_n(ps, i - 1));
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the periods of a period set.
 * @post The output parameter @p count is equal to the number of periods of
 * the input period set
 * @sqlfunc periods()
 * @pymeosfunc periods()
 */
const Period **
periodset_periods(const PeriodSet *ps, int *count)
{
  const Period **periods = palloc(sizeof(Period *) * ps->count);
  for (int i = 0; i < ps->count; i++)
    periods[i] = periodset_per_n(ps, i);
  *count = ps->count;
  return periods;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the number of timestamps of a period set
 * @sqlfunc numTimestamps()
 * @pymeosfunc numTimestamps()
 */
int
periodset_num_timestamps(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  TimestampTz prev = p->lower;
  bool start = false;
  int result = 1;
  TimestampTz d;
  int i = 1;
  while (i < ps->count || !start)
  {
    if (start)
    {
      p = periodset_per_n(ps, i++);
      d = p->lower;
      start = !start;
    }
    else
    {
      d = p->upper;
      start = !start;
    }
    if (prev != d)
    {
      result++;
      prev = d;
    }
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the start timestamp of a period set.
 * @sqlfunc startTimestamp()
 * @pymeosfunc startTimestamp()
 */
TimestampTz
periodset_start_timestamp(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, 0);
  return p->lower;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the end timestamp of a period set.
 * @sqlfunc endTimestamp()
 * @pymeosfunc endTimestamp()
 */
TimestampTz
periodset_end_timestamp(const PeriodSet *ps)
{
  const Period *p = periodset_per_n(ps, ps->count - 1);
  return p->upper;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the n-th timestamp of a period set.
 *
 * @param[in] ps Period set
 * @param[in] n Number
 * @param[out] result Timestamp
 * @result Return true if the timestamp is found
 * @note It is assumed that n is 1-based
 * @sqlfunc timestampN()
 * @pymeosfunc timestampN()
 */
bool
periodset_timestamp_n(const PeriodSet *ps, int n, TimestampTz *result)
{
  int pernum = 0;
  const Period *p = periodset_per_n(ps, pernum);
  TimestampTz d = p->lower;
  if (n == 1)
  {
    *result = d;
    return true;
  }

  bool start = false;
  int i = 1;
  TimestampTz prev = d;
  while (i < n)
  {
    if (start)
    {
      pernum++;
      if (pernum == ps->count)
        break;

      p = periodset_per_n(ps, pernum);
      d = p->lower;
      start = !start;
    }
    else
    {
      d = p->upper;
      start = !start;
    }
    if (prev != d)
    {
      i++;
      prev = d;
    }
  }
  if (i != n)
    return false;
  *result = d;
  return true;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the timestamps of a period set
 * @sqlfunc timestamps()
 * @pymeosfunc timestamps()
 */
TimestampTz *
periodset_timestamps(const PeriodSet *ps, int *count)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * 2 * ps->count);
  const Period *p = periodset_per_n(ps, 0);
  result[0] = p->lower;
  int k = 1;
  if (p->lower != p->upper)
    result[k++] = p->upper;
  for (int i = 1; i < ps->count; i++)
  {
    p = periodset_per_n(ps, i);
    if (result[k - 1] != (TimestampTz) p->lower)
      result[k++] = p->lower;
    if (result[k - 1] != (TimestampTz) p->upper)
      result[k++] = p->upper;
  }
  *count = k;
  return result;
}

/*****************************************************************************
 * Modifications functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_transf
 * @brief Return a period set shifted and/or scaled by the intervals.
 * @sqlfunc shift(), tscale(), shiftTscale()
 * @pymeosfunc shift()
 */
PeriodSet *
periodset_shift_tscale(const PeriodSet *ps, const Interval *shift,
  const Interval *duration)
{
  assert(shift != NULL || duration != NULL);
  if (duration != NULL)
    ensure_valid_duration(duration);
  bool instant = (ps->period.lower == ps->period.upper);

  /* Copy the input period set to the output period set */
  PeriodSet *result = periodset_copy(ps);
  /* Shift and/or scale the bounding period */
  period_shift_tscale(shift, duration, &result->period);
  /* Shift and/or scale the periods of the period set */
  TimestampTz delta;
  if (shift != NULL)
    delta = result->period.lower - ps->period.lower;
  /* If the periodset is instantaneous we cannot scale */
  double scale;
  if (duration != NULL && ! instant)
    scale = (double) (result->period.upper - result->period.lower) /
      (double) (ps->period.upper - ps->period.lower);
  for (int i = 0; i < ps->count; i++)
  {
    if (shift != NULL)
    {
      result->elems[i].lower += delta;
      result->elems[i].upper += delta;
    }
    if (duration != NULL && ! instant)
    {
      result->elems[i].lower = result->period.lower +
        (result->elems[i].lower - result->period.lower) * scale;
      result->elems[i].upper = result->period.lower +
        (result->elems[i].upper - result->period.lower) * scale;
    }
  }
  return result;
}

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first period set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
periodset_eq(const PeriodSet *ps1, const PeriodSet *ps2)
{
  if (ps1->count != ps2->count)
    return false;
  /* ps1 and ps2 have the same number of PeriodSet */
  for (int i = 0; i < ps1->count; i++)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, i);
    if (span_ne(p1, p2))
      return false;
  }
  /* All periods of the two PeriodSet are equal */
  return true;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first period set is different from the
 * second one.
 * @sqlop @p <>
 */
bool
periodset_ne(const PeriodSet *ps1, const PeriodSet *ps2)
{
  return ! periodset_eq(ps1, ps2);
}
/**
 * @ingroup libmeos_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first period set
 * is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc periodset_cmp()
 */
int
periodset_cmp(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int count1 = ps1->count;
  int count2 = ps2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    const Period *p1 = periodset_per_n(ps1, i);
    const Period *p2 = periodset_per_n(ps2, i);
    result = span_cmp(p1, p2);
    if (result)
      break;
  }
  /* The first count periods of the two PeriodSet are equal */
  if (! result)
  {
    if (count < count1) /* ps1 has more PeriodSet than ps2 */
      result = 1;
    else if (count < count2) /* ps2 has more PeriodSet than ps1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first period set is less than the second one
 * @sqlop @p <
 */
bool
periodset_lt(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first period set is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
periodset_le(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first period set is greater than or equal to
 * the second one
 * @sqlop @p >=
 */
bool
periodset_ge(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first period set is greater than the second one
 * @sqlop @p >
 */
bool
periodset_gt(const PeriodSet *ps1, const PeriodSet *ps2)
{
  int cmp = periodset_cmp(ps1, ps2);
  return cmp > 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 32-bit hash value of a period set.
 * @sqlfunc periodset_hash()
 */
uint32
periodset_hash(const PeriodSet *ps)
{
  uint32 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    uint32 per_hash = span_hash(p);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 64-bit hash value of a period set using a seed
 * @sqlfunc periodset_hash_extended()
 */
uint64
periodset_hash_extended(const PeriodSet *ps, uint64 seed)
{
  uint64 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    uint64 per_hash = span_hash_extended(p, seed);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/*****************************************************************************/
