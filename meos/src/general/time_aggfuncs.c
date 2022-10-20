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
 * @brief Aggregate functions for time types.
 */

#include "general/time_aggfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
// #include <libpq/pqformat.h>
// #include <utils/memutils.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporal_tile.h"
#include "general/temporal_util.h"

/*****************************************************************************
 * Aggregate functions for time types
 *****************************************************************************/

/**
 * Return the sum of the two arguments
 */
Datum
datum_sum_int32(Datum l, Datum r)
{
  return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

/*
 * Generic aggregate function for timestamp set values
 *
 * @param[in] times1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] times2 Timestamps of the input timestamp set value
 * @param[in] count2 Number of elements in the timestamp set value
 * @note Return new timestamps that must be freed by the calling function.
 */
TimestampTz *
timestamp_agg(TimestampTz *times1, int count1, TimestampTz *times2,
  int count2, int *newcount)
{
  TimestampTz *result = palloc(sizeof(TimestampTz) * (count1 + count2));
  int i = 0, j = 0, count = 0;
  while (i < count1 && j < count2)
  {
    TimestampTz t1 = times1[i];
    TimestampTz t2 = times2[j];
    int cmp = timestamptz_cmp_internal(t1, t2);
    if (cmp == 0)
    {
      result[count++] = t1;
      i++;
      j++;
    }
    else if (cmp < 0)
    {
      result[count++] = t1;
      i++;
    }
    else
    {
      result[count++] = t2;
      j++;
    }
  }
  /* Copy the timetamps from state1 or state2 that are after the end of the
     other state */
  while (i < count1)
    result[count++] = times1[i++];
  while (j < count2)
    result[count++] = times2[j++];
  *newcount = count;
  return result;
}

/**
 * Generic aggregate function for periods.
 *
 * @param[in] periods1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] periods2 Periods of a period (set) value
 * @param[in] count2 Number of elements in the period set value
 * @param[out] newcount Number of elements in the result
 * @note Return new periods that must be freed by the calling function.
 */
Period **
period_agg(Period **periods1, int count1, Period **periods2, int count2,
  int *newcount)
{
  Period **periods = palloc(sizeof(Period *) * (count1 + count2));
  int i = 0, j = 0, k = 0;
  while (i < count1 && j < count2)
  {
    int cmp = span_cmp(periods1[i], periods2[j]);
    if (cmp == 0)
    {
      periods[k++] = periods1[i++];
      j++;
    }
    else if (cmp < 0)
      periods[k++] = periods1[i++];
    else
      periods[k++] = periods2[j++];
  }
  while (i < count1)
    periods[k++] = periods1[i++];
  while (j < count2)
    periods[k++] = periods2[j++];
  Period **result = spanarr_normalize(periods, k, SORT_NO, newcount);
  pfree(periods);
  return result;
}

/*****************************************************************************
 * Generic aggregate transition functions
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_agg
 * @brief Generic transition function for aggregating timestamp sets
 *
 * @param[in,out] state Timestamp array containing the state
 * @param[in] ts Timestamp set value
 */
SkipList *
timestampset_agg_transfn(SkipList *state, const TimestampSet *ts)
{
  TimestampTz *times = timestampset_timestamps(ts);
  SkipList *result;
  if (! state)
    result = skiplist_make((void **) times, ts->count, TIMESTAMPTZ);
  else
  {
    assert(state->elemtype == TIMESTAMPTZ);
    skiplist_splice(state, (void **) times, ts->count, NULL, CROSSINGS_NO);
    result = state;
  }
  pfree(times);
  return result;
}

/**
 * @ingroup libmeos_spantime_agg
 * @brief Generic transition function for aggregating period values
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] p Period
 */
SkipList *
period_agg_transfn(SkipList *state, const Period *p)
{
  SkipList *result;
  if (! state)
    result = skiplist_make((void **) &p, 1, PERIOD);
  else
  {
    assert(state->elemtype == PERIOD);
    skiplist_splice(state, (void **) &p, 1, NULL, CROSSINGS_NO);
    result = state;
  }
  return result;
}

/**
 * @ingroup libmeos_spantime_agg
 * @brief Generic transition function for aggregating period set values
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] ps Period set value
 */
SkipList *
periodset_agg_transfn(SkipList *state, const PeriodSet *ps)
{
  int count;
  const Period **periods = periodset_periods(ps, &count);
  SkipList *result;
  if (! state)
    /* Periods are copied while constructing the skiplist */
    result = skiplist_make((void **) periods, ps->count, PERIOD);
  else
  {
    assert(state->elemtype == PERIOD);
    skiplist_splice(state, (void **) periods, ps->count, NULL, CROSSINGS_NO);
    result = state;
  }
  pfree(periods);
  return result;
}

/*****************************************************************************
 * Combine function
 *****************************************************************************/

/**
 * Generic combine function for aggregating time values
 *
 * @param[in] state1, state2 State values
 */
SkipList *
time_agg_combinefn(SkipList *state1, SkipList *state2)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  assert(state1->elemtype == state2->elemtype);
  SkipList *smallest, *largest;
  if (state1->length < state2->length)
  {
    smallest = state1; largest = state2;
  }
  else
  {
    smallest = state2; largest = state1;
  }
  void **values = skiplist_values(smallest);
  skiplist_splice(largest, values, smallest->length, NULL, CROSSINGS_NO);
  /* Delete the new aggregate values */
  if (smallest->elemtype == TIMESTAMPTZ)
    pfree(values);
  else
    pfree_array(values, smallest->length);
  return largest;
}

/*****************************************************************************
 * Aggregate transition functions for time types
 *****************************************************************************/

/**
 * Transform a timestamp value into a temporal integer value for
 * performing temporal count aggregation
 */
TInstant **
timestamp_transform_tcount(TimestampTz t, const Interval *interval,
  TimestampTz origin)
{
  TInstant **result = palloc(sizeof(TInstant *));
  if (interval)
    t = timestamptz_bucket(t, interval, origin);
  result[0] = tinstant_make(Int32GetDatum(1), T_TINT, t);
  return result;
}

/**
 * Transform a timestamp set value into a temporal integer value for
 * performing temporal count aggregation
 */
TInstant **
timestampset_transform_tcount(const TimestampSet *ts, const Interval *interval,
  TimestampTz origin)
{
  TInstant **result = palloc(sizeof(TInstant *) * ts->count);
  Datum datum_one = Int32GetDatum(1);
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    if (interval)
      t = timestamptz_bucket(t, interval, origin);
    result[i] = tinstant_make(datum_one, T_TINT, t);
  }
  return result;
}

/**
 * Transform a period value into a temporal integer value for
 * performing temporal count aggregation
 */
TSequence *
period_transform_tcount(const Period *p, const Interval *interval,
  TimestampTz origin)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  TInstant *instants[2];
  TimestampTz t = p->lower;
  if (interval)
    t = timestamptz_bucket(t, interval, origin);
  instants[0] = tinstant_make(datum_one, T_TINT, t);
  if (p->lower == p->upper)
  {
    result = tsequence_make((const TInstant **) instants, 1, 1,
      p->lower_inc, p->upper_inc, STEPWISE, NORMALIZE_NO);
  }
  else
  {
    t = p->upper;
    /* The upper timestamp must be gridded to the next bucket */
    if (interval)
    {
      int64 size = interval_units(interval);
      t = timestamptz_bucket(t, interval, origin) + size;
    }
    instants[1] = tinstant_make(datum_one, T_TINT, t);
    result = tsequence_make((const TInstant **) instants, 2, 2,
      p->lower_inc, p->upper_inc, STEPWISE, NORMALIZE_NO);
    pfree(instants[1]);
  }
  pfree(instants[0]);
  return result;
}

/**
 * Transform a period set value into a temporal integer value for
 * performing temporal count aggregation
 */
TSequence **
periodset_transform_tcount(const PeriodSet *ps, const Interval *interval,
  TimestampTz origin)
{
  TSequence **result = palloc(sizeof(TSequence *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    result[i] = period_transform_tcount(p, interval, origin);
  }
  return result;
}

void
ensure_same_timetype_skiplist(SkipList *state, uint8 subtype)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (head->subtype != subtype)
    elog(ERROR, "Cannot aggregate temporal values of different type");
  return;
}

/*****************************************************************************/
