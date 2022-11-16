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
    p = spanset_sp_n(ps, middle);
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
 * Cast functions
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
  PeriodSet *result = span_to_spanset(&p);
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
  PeriodSet *result = spanset_make_free(periods, ts->count, NORMALIZE_NO);
  return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the timespan of a period set
 * @sqlfunc timespan()
 * @pymeosfunc timespan()
 */
Interval *
periodset_timespan(const PeriodSet *ps)
{
  const Period *p1 = spanset_sp_n(ps, 0);
  const Period *p2 = spanset_sp_n(ps, ps->count - 1);
  Interval *result = pg_timestamp_mi(p2->upper, p1->lower);
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the duration of a period set
 * @sqlfunc duration()
 * @pymeosfunc duration()
 */
Interval *
periodset_duration(const PeriodSet *ps)
{
  const Period *p = spanset_sp_n(ps, 0);
  Interval *result = pg_timestamp_mi(p->upper, p->lower);
  for (int i = 1; i < ps->count; i++)
  {
    p = spanset_sp_n(ps, i);
    Interval *interval1 = pg_timestamp_mi(p->upper, p->lower);
    Interval *interval2 = pg_interval_pl(result, interval1);
    pfree(result); pfree(interval1);
    result = interval2;
  }
  return result;
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
  const Period *p = spanset_sp_n(ps, 0);
  TimestampTz prev = p->lower;
  bool start = false;
  int result = 1;
  TimestampTz d;
  int i = 1;
  while (i < ps->count || !start)
  {
    if (start)
    {
      p = spanset_sp_n(ps, i++);
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
  const Period *p = spanset_sp_n(ps, 0);
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
  const Period *p = spanset_sp_n(ps, ps->count - 1);
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
  const Period *p = spanset_sp_n(ps, pernum);
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

      p = spanset_sp_n(ps, pernum);
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
  const Period *p = spanset_sp_n(ps, 0);
  result[0] = p->lower;
  int k = 1;
  if (p->lower != p->upper)
    result[k++] = p->upper;
  for (int i = 1; i < ps->count; i++)
  {
    p = spanset_sp_n(ps, i);
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
  bool instant = (ps->span.lower == ps->span.upper);

  /* Copy the input period set to the output period set */
  PeriodSet *result = spanset_copy(ps);
  /* Shift and/or scale the bounding period */
  period_shift_tscale(shift, duration, &result->span);
  /* Shift and/or scale the periods of the period set */
  TimestampTz delta;
  if (shift != NULL)
    delta = result->span.lower - ps->span.lower;
  /* If the periodset is instantaneous we cannot scale */
  double scale;
  if (duration != NULL && ! instant)
    scale = (double) (result->span.upper - result->span.lower) /
      (double) (ps->span.upper - ps->span.lower);
  for (int i = 0; i < ps->count; i++)
  {
    if (shift != NULL)
    {
      result->elems[i].lower += delta;
      result->elems[i].upper += delta;
    }
    if (duration != NULL && ! instant)
    {
      result->elems[i].lower = result->span.lower +
        (result->elems[i].lower - result->span.lower) * scale;
      result->elems[i].upper = result->span.lower +
        (result->elems[i].upper - result->span.lower) * scale;
    }
  }
  return result;
}

/*****************************************************************************
 * B-tree support
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span set is equal to the second one.
 * @note The internal B-tree comparator is not used to increase efficiency
 * @sqlop @p =
 * @pymeosfunc __eq__()
 */
bool
spanset_eq(const SpanSet *ss1, const SpanSet *ss2)
{
  if (ss1->count != ss2->count)
    return false;
  /* ss1 and ss2 have the same number of SpanSet */
  for (int i = 0; i < ss1->count; i++)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, i);
    if (span_ne(s1, s2))
      return false;
  }
  /* All spans of the two span sets are equal */
  return true;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span set is different from the
 * second one.
 * @sqlop @p <>
 */
bool
spanset_ne(const SpanSet *ss1, const SpanSet *ss2)
{
  return ! spanset_eq(ss1, ss2);
}
/**
 * @ingroup libmeos_spantime_comp
 * @brief Return -1, 0, or 1 depending on whether the first span set
 * is less than, equal, or greater than the second one.
 * @note Function used for B-tree comparison
 * @sqlfunc spanset_cmp()
 */
int
spanset_cmp(const SpanSet *ss1, const SpanSet *ss2)
{
  int count1 = ss1->count;
  int count2 = ss2->count;
  int count = count1 < count2 ? count1 : count2;
  int result = 0;
  for (int i = 0; i < count; i++)
  {
    const Span *s1 = spanset_sp_n(ss1, i);
    const Span *s2 = spanset_sp_n(ss2, i);
    result = span_cmp(s1, s2);
    if (result)
      break;
  }
  /* The first count spans of the two SpanSet are equal */
  if (! result)
  {
    if (count < count1) /* ss1 has more SpanSet than ss2 */
      result = 1;
    else if (count < count2) /* ss2 has more SpanSet than ss1 */
      result = -1;
    else
      result = 0;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span set is less than the second one
 * @sqlop @p <
 */
bool
spanset_lt(const SpanSet *ss1, const SpanSet *ss2)
{
  int cmp = spanset_cmp(ss1, ss2);
  return cmp < 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span set is less than or equal to
 * the second one
 * @sqlop @p <=
 */
bool
spanset_le(const SpanSet *ss1, const SpanSet *ss2)
{
  int cmp = spanset_cmp(ss1, ss2);
  return cmp <= 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span set is greater than or equal to
 * the second one
 * @sqlop @p >=
 */
bool
spanset_ge(const SpanSet *ss1, const SpanSet *ss2)
{
  int cmp = spanset_cmp(ss1, ss2);
  return cmp >= 0;
}

/**
 * @ingroup libmeos_spantime_comp
 * @brief Return true if the first span set is greater than the second one
 * @sqlop @p >
 */
bool
spanset_gt(const SpanSet *ss1, const SpanSet *ss2)
{
  int cmp = spanset_cmp(ss1, ss2);
  return cmp > 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of
 * the elements.
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 32-bit hash value of a span set.
 * @sqlfunc spanset_hash()
 */
uint32
spanset_hash(const SpanSet *ps)
{
  uint32 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    uint32 per_hash = span_hash(p);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_accessor
 * @brief Return the 64-bit hash value of a span set using a seed
 * @sqlfunc spanset_hash_extended()
 */
uint64
spanset_hash_extended(const SpanSet *ps, uint64 seed)
{
  uint64 result = 1;
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    uint64 per_hash = span_hash_extended(p, seed);
    result = (result << 5) - result + per_hash;
  }
  return result;
}

/*****************************************************************************/
