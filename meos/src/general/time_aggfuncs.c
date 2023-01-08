/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
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
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporal_aggfuncs.h"
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
timestamp_tagg(TimestampTz *times1, int count1, TimestampTz *times2,
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
Span **
period_tagg(Span **periods1, int count1, Span **periods2, int count2,
  int *newcount)
{
  Span **periods = palloc(sizeof(Span *) * (count1 + count2));
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
  Span **result = spanarr_normalize(periods, k, SORT_NO, newcount);
  pfree(periods);
  return result;
}

/*****************************************************************************
 * Generic combine function
 *****************************************************************************/

/**
 * Generic combine function for temporal aggregate of time values
 *
 * @param[in] state1, state2 State values
 */
SkipList *
time_tagg_combinefn(SkipList *state1, SkipList *state2)
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
 * extent
 * N.B. transition function for extent of periods is defined in span_aggfuncs.c
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for extent aggregate of timestamp set values
 */
Span *
timestamp_extent_transfn(Span *p, TimestampTz t)
{
  /* Null period: return the period of the timestamp */
  if (! p)
    return span_make(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true,
      true, T_TIMESTAMPTZ);

  Span p1;
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true,
      true, T_TIMESTAMPTZ, &p1);
  span_expand(&p1, p);
  return p;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for extent aggregate of timestamp set values
 */
Span *
tstzset_extent_transfn(Span *p, const Set *ts)
{
  /* Can't do anything with null inputs */
  if (! p && ! ts)
    return NULL;
  /* Null period and non-null timestamp set, return the bbox of the timestamp set */
  if (! p)
    return set_to_span(ts);
  /* Non-null period and null timestamp set, return the period */
  if (! ts)
    return span_copy(p);

  Span s;
  set_set_span(ts, &s);
  span_expand(&s, p);
  return p;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for extent aggregate of span values
 */
Span *
span_extent_transfn(Span *s1, const Span *s2)
{
  /* Can't do anything with null inputs */
  if (! s1 && ! s2)
    return NULL;
  /* Null span and non-null span, return the span */
  if (! s1)
    return span_copy(s2);
  /* Non-null span and null span, return the span */
  if (! s2)
    return span_copy(s1);

  span_expand(s2, s1);
  return s1;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for extent aggregate of span set values
 */
Span *
spanset_extent_transfn(Span *s, const SpanSet *ss)
{
  /* Can't do anything with null inputs */
  if (! s && ! ss)
    return NULL;
  /* Null  and non-null span set, return the bbox of the span set */
  if (! s)
    return span_copy(&ss->span);
  /* Non-null span and null temporal, return the span */
  if (! ss)
    return span_copy(s);

  span_expand(&ss->span, s);
  return s;
}

/*****************************************************************************
 * tunion
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for tunion aggregate of timestamps
 *
 * @param[in,out] state Timestamp array containing the state
 * @param[in] t Timestamp value
 */
SkipList *
timestamp_tunion_transfn(SkipList *state, TimestampTz t)
{
  SkipList *result;
  if (! state)
    result = skiplist_make((void **) &t, 1, TIMESTAMPTZ);
  else
  {
    assert(state->elemtype == TIMESTAMPTZ);
    skiplist_splice(state, (void **) &t, 1, NULL, CROSSINGS_NO);
    result = state;
  }
  return result;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for aggregating timestamp sets
 *
 * @param[in,out] state Timestamp array containing the state
 * @param[in] ts Timestamp set value
 */
SkipList *
tstzset_tunion_transfn(SkipList *state, const Set *ts)
{
  TimestampTz *times = tstzset_timestamps(ts);
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
 * @ingroup libmeos_setspan_agg
 * @brief Generic transition function for aggregating period values
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] p Period
 */
SkipList *
period_tunion_transfn(SkipList *state, const Span *p)
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
 * @ingroup libmeos_setspan_agg
 * @brief Generic transition function for aggregating period set values
 *
 * @param[in,out] state Skiplist containing the state
 * @param[in] ps Period set value
 */
SkipList *
periodset_tunion_transfn(SkipList *state, const SpanSet *ps)
{
  int count;
  const Span **periods = spanset_spans(ps, &count);
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

/*****************************************************************************/

/**
 * @ingroup libmeos_setspan_agg
 * @brief Final function for union aggregation of timestamp set values
 */
Set *
timestamp_tunion_finalfn(SkipList *state)
{
  if (! state || state->length == 0)
    return NULL;

  assert(state->elemtype == TIMESTAMPTZ);
  Datum *values = (Datum *) skiplist_values(state);

  Set *result = set_make(values, state->length, T_TIMESTAMPTZ, ORDERED);
  pfree(values);
  return (Set *) result;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Final function for union aggregation of period (set) values
 */
SpanSet *
period_tunion_finalfn(SkipList *state)
{
  if (! state || state->length == 0)

  assert(state->elemtype == PERIOD);
  const Span **values = (const Span **) skiplist_values(state);
  SpanSet *result = spanset_make(values, state->length, NORMALIZE);
  pfree(values);
  return result;
}

/*****************************************************************************
 * tcount
 *****************************************************************************/

/**
 * Transform a timestamp value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
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
static TInstant **
tstzset_transform_tcount(const Set *ts, const Interval *interval,
  TimestampTz origin, int *newcount)
{
  TInstant **result = palloc(sizeof(TInstant *) * ts->count);

  TimestampTz t = DatumGetTimestampTz(set_val_n(ts, 0));
  if (interval)
    t = timestamptz_bucket(t, interval, origin);
  int k = 0, count = 1;
  for (int i = 1; i < ts->count; i++)
  {
    TimestampTz t1 = DatumGetTimestampTz(set_val_n(ts, i));
    if (interval)
      t1 = timestamptz_bucket(t1, interval, origin);
    if (timestamptz_cmp_internal(t, t1) == 0)
      count++;
    else
    {
      result[k++] = tinstant_make(Int32GetDatum(count), T_TINT, t);
      count = 1;
      t = t1;
    }
  }
  result[k++] = tinstant_make(Int32GetDatum(count), T_TINT, t);
  *newcount = k;
  return result;
}

/**
 * Transform a period value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
period_transform_tcount(const Span *p, const Interval *interval,
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
    result = tsequence_make((const TInstant **) instants, 1, p->lower_inc,
      p->upper_inc, STEPWISE, NORMALIZE_NO);
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
    result = tsequence_make((const TInstant **) instants, 2,
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
static TSequence **
periodset_transform_tcount(const SpanSet *ps, const Interval *interval,
  TimestampTz origin)
{
  TSequence **result = palloc(sizeof(TSequence *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    result[i] = period_transform_tcount(p, interval, origin);
  }
  return result;
}

void
ensure_same_timetype_skiplist(SkipList *state, uint8 subtype)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (head->subtype != subtype)
    elog(ERROR, "Cannot aggregate temporal values of different duration");
  return;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of timestamps
 */
SkipList *
timestamp_tcount_transfn(SkipList *state, TimestampTz t,
  const Interval *interval, TimestampTz origin)
{
  TInstant **instants = timestamp_transform_tcount(t, interval, origin);
  if (! state)
  {
    state = skiplist_make((void **) instants, 1, TEMPORAL);
  }
  else
  {
    ensure_same_timetype_skiplist(state, TINSTANT);
    skiplist_splice(state, (void **) instants, 1, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree_array((void **) instants, 1);
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of timestamp sets
 */
SkipList *
tstzset_tcount_transfn(SkipList *state, const Set *ts,
  const Interval *interval, TimestampTz origin)
{
  int count;
  TInstant **instants = tstzset_transform_tcount(ts, interval, origin,
    &count);
  /* Due to the bucketing, it is possible that count < ts->count */

  if (! state)
  {
    state = skiplist_make((void **) instants, count, TEMPORAL);
  }
  else
  {
    ensure_same_timetype_skiplist(state, TINSTANT);
    skiplist_splice(state, (void **) instants, count, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree_array((void **) instants, count);
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of periods
 */
SkipList *
period_tcount_transfn(SkipList *state, const Span *p, const Interval *interval,
  TimestampTz origin)
{
  TSequence *seq = period_transform_tcount(p, interval, origin);
  if (! state)
  {
    state = skiplist_make((void **) &seq, 1, TEMPORAL);
  }
  else
  {
    ensure_same_timetype_skiplist(state, TSEQUENCE);
    skiplist_splice(state, (void **) &seq, 1, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree(seq);
  return state;
}

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for temporal count aggregate of period sets
 */
SkipList *
periodset_tcount_transfn(SkipList *state, const SpanSet *ps,
  const Interval *interval, TimestampTz origin)
{
  TSequence **sequences = periodset_transform_tcount(ps, interval, origin);
  int start = 0;
  if (! state)
  {
    state = skiplist_make((void **) &sequences[0], 1, TEMPORAL);
    start++;
  }
  else
    ensure_same_timetype_skiplist(state, TSEQUENCE);
  for (int i = start; i < ps->count; i++)
  {
    skiplist_splice(state, (void **) &sequences[i], 1, &datum_sum_int32,
      CROSSINGS_NO);
  }

  pfree_array((void **) sequences, ps->count);
  return state;
}

/*****************************************************************************/
