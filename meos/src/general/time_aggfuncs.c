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
#include "general/type_util.h"

/*****************************************************************************
 * Aggregate functions for time types
 *****************************************************************************/

/**
 * @brief Return the sum of the two arguments
 */
Datum
datum_sum_int32(Datum l, Datum r)
{
  return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

/*
 * @brief Generic aggregate function to aggregate timestamp set values
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
    int cmp = timestamptz_cmp_internal(times1[i], times2[j]);
    if (cmp == 0)
    {
      result[count++] = times1[i++];
      j++;
    }
    else if (cmp < 0)

      result[count++] = times1[i++];
    else
      result[count++] = times2[j++];
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
 * @brief Generic aggregate function for periods.
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
 * @brief Generic combine function for temporal aggregate of time values
 * @param[in] state1, state2 State values
 */
SkipList *
time_tagg_combinefn(SkipList *state1, SkipList *state2)
{
  if (! state1)
    return state2;
  if (! state2)
    return state1;

  if (state1->length == 0)
    return state2;
  if (state2->length == 0)
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
  void **values = (smallest->elemtype == TIMESTAMPTZ) ?
    skiplist_values(smallest) : (void **) skiplist_period_values(smallest);
  skiplist_splice(largest, values, smallest->length, NULL, CROSSINGS_NO);
  /* Free memory */
  pfree(values);
  // skiplist_free(smallest);
  return largest;
}

/*****************************************************************************
 * tunion
 *****************************************************************************/

/**
 * @ingroup libmeos_setspan_agg
 * @brief Transition function for tunion aggregate of timestamps
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
 * @param[in,out] state Skiplist containing the state
 * @param[in] ps Period set value
 */
SkipList *
periodset_tunion_transfn(SkipList *state, const SpanSet *ps)
{
  /* Singleton period set */
  if (ps->count == 1)
    return period_tunion_transfn(state, spanset_sp_n(ps, 0));

  /* General case */
  const Span **periods = spanset_spans(ps);
  SkipList *result;
  if (! state)
    /* Periods are copied when constructing the skiplist */
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
  skiplist_free(state);
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
    return NULL;

  assert(state->elemtype == PERIOD);
  const Span **values = (const Span **) skiplist_values(state);
  SpanSet *result = spanset_make(values, state->length, NORMALIZE);
  pfree(values);
  skiplist_free(state);
  return result;
}

/*****************************************************************************
 * tcount
 *****************************************************************************/

/**
 * @brief Transform a timestamp value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
timestamp_transform_tcount(TimestampTz t)
{
  TInstant **result = palloc(sizeof(TInstant *));
  result[0] = tinstant_make(Int32GetDatum(1), T_TINT, t);
  return result;
}

/**
 * @brief Transform a timestamp set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
tstzset_transform_tcount(const Set *ts, int *newcount)
{
  TInstant **result = palloc(sizeof(TInstant *) * ts->count);

  TimestampTz t = DatumGetTimestampTz(set_val_n(ts, 0));
  int k = 0, count = 1;
  for (int i = 1; i < ts->count; i++)
  {
    TimestampTz t1 = DatumGetTimestampTz(set_val_n(ts, i));
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
 * @brief Transform a period value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
period_transform_tcount(const Span *p)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  TInstant *instants[2];
  TimestampTz t = p->lower;
  instants[0] = tinstant_make(datum_one, T_TINT, t);
  if (p->lower == p->upper)
  {
    result = tsequence_make((const TInstant **) instants, 1, p->lower_inc,
      p->upper_inc, STEP, NORMALIZE_NO);
  }
  else
  {
    t = p->upper;
    instants[1] = tinstant_make(datum_one, T_TINT, t);
    result = tsequence_make((const TInstant **) instants, 2,
      p->lower_inc, p->upper_inc, STEP, NORMALIZE_NO);
    pfree(instants[1]);
  }
  pfree(instants[0]);
  return result;
}

/**
 * @brief Transform a period set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence **
periodset_transform_tcount(const SpanSet *ps)
{
  TSequence **result = palloc(sizeof(TSequence *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Span *p = spanset_sp_n(ps, i);
    result[i] = period_transform_tcount(p);
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
timestamp_tcount_transfn(SkipList *state, TimestampTz t)
{
  TInstant **instants = timestamp_transform_tcount(t);
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
tstzset_tcount_transfn(SkipList *state, const Set *ts)
{
  int count;
  TInstant **instants = tstzset_transform_tcount(ts, &count);
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
period_tcount_transfn(SkipList *state, const Span *p)
{
  TSequence *seq = period_transform_tcount(p);
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
periodset_tcount_transfn(SkipList *state, const SpanSet *ps)
{
  TSequence **sequences = periodset_transform_tcount(ps);
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
