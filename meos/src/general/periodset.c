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
 * @brief General functions for period sets, that is, set of disjoint periods.
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

/*****************************************************************************/
