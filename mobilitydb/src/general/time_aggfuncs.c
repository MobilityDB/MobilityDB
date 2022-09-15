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

#include "pg_general/time_aggfuncs.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <libpq/pqformat.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "pg_general/skiplist.h"
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
 * Generic transition function for aggregating timestamp sets
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in,out] state Timestamp array containing the state
 * @param[in] ts Timestamp set value
 */
static SkipList *
timestampset_agg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const TimestampSet *ts)
{
  TimestampTz *times = timestampset_timestamps(ts);
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) times, ts->count, TIMESTAMPTZ);
  else
  {
    assert(state->elemtype == TIMESTAMPTZ);
    skiplist_splice(fcinfo, state, (void **) times, ts->count, NULL,
      CROSSINGS_NO);
    result = state;
  }
  pfree(times);
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in,out] state Skiplist containing the state
 * @param[in] p Period
 */
static SkipList *
period_agg_transfn(FunctionCallInfo fcinfo, SkipList *state, const Period *p)
{
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) &p, 1, PERIOD);
  else
  {
    assert(state->elemtype == PERIOD);
    skiplist_splice(fcinfo, state, (void **) &p, 1, NULL, CROSSINGS_NO);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating period set values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in,out] state Skiplist containing the state
 * @param[in] ps Period set value
 */
static SkipList *
periodset_agg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const PeriodSet *ps)
{
  int count;
  const Period **periods = periodset_periods(ps, &count);
  SkipList *result;
  if (! state)
    /* Periods are copied while constructing the skiplist */
    result = skiplist_make(fcinfo, (void **) periods, ps->count, PERIOD);
  else
  {
    assert(state->elemtype == PERIOD);
    skiplist_splice(fcinfo, state, (void **) periods, ps->count, NULL,
      CROSSINGS_NO);
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
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] state1, state2 State values
 */
SkipList *
time_agg_combinefn(FunctionCallInfo fcinfo, SkipList *state1,
  SkipList *state2)
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
  skiplist_splice(fcinfo, largest, values, smallest->length, NULL,
    CROSSINGS_NO);
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

PG_FUNCTION_INFO_V1(Timestampset_extent_transfn);
/**
 * Transition function for temporal extent aggregation of timestamp set values
 * with period bounding box
 */
PGDLLEXPORT Datum
Timestampset_extent_transfn(PG_FUNCTION_ARGS)
{
  Period *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  TimestampSet *ts = PG_ARGISNULL(1) ? NULL : PG_GETARG_TIMESTAMPSET_P(1);
  Period *result;

  /* Can't do anything with null inputs */
  if (! p && ! ts)
    PG_RETURN_NULL();
  /* Null period and non-null timestampset, return the bbox of the timestampset */
  if (! p)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, &ts->period, sizeof(Period));
    PG_RETURN_POINTER(result);
  }
  /* Non-null period and null timestampset, return the period */
  if (! ts)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, p, sizeof(Period));
    PG_RETURN_POINTER(result);
  }

  result = union_span_span(p, &ts->period, false);

  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_extent_transfn);
/**
 * Transition function for temporal extent aggregation of period set values
 * with period bounding box
 */
PGDLLEXPORT Datum
Periodset_extent_transfn(PG_FUNCTION_ARGS)
{
  Period *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  PeriodSet *ps = PG_ARGISNULL(1) ? NULL : PG_GETARG_PERIODSET_P(1);
  Period *result;

  /* Can't do anything with null inputs */
  if (! p && ! ps)
    PG_RETURN_NULL();
  /* Null period and non-null period set, return the bbox of the period set */
  if (! p)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, &ps->period, sizeof(Period));
    PG_RETURN_POINTER(result);
  }
  /* Non-null period and null temporal, return the period */
  if (! ps)
  {
    result = palloc0(sizeof(Period));
    memcpy(result, p, sizeof(Period));
    PG_RETURN_POINTER(result);
  }

  result = union_span_span(p, &ps->period, false);

  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Timestampset_tunion_transfn);
/**
 * Transition function for union aggregate of timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  SkipList *result = timestampset_agg_transfn(fcinfo, state, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_tunion_transfn);
/**
 * Transition function for union aggregate of periods
 */
PGDLLEXPORT Datum
Period_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Period *p = PG_GETARG_SPAN_P(1);
  SkipList *result = period_agg_transfn(fcinfo, state, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Periodset_tunion_transfn);
/**
 * Transition function for union aggregate of period sets
 */
PGDLLEXPORT Datum
Periodset_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  SkipList *result = periodset_agg_transfn(fcinfo, state, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transform a timestamp set value into a temporal integer value for
 * performing temporal count aggregation
 */
static TInstant **
timestampset_transform_tcount(const TimestampSet *ts)
{
  TInstant **result = palloc(sizeof(TInstant *) * ts->count);
  Datum datum_one = Int32GetDatum(1);
  for (int i = 0; i < ts->count; i++)
  {
    TimestampTz t = timestampset_time_n(ts, i);
    result[i] = tinstant_make(datum_one, T_TINT, t);
  }
  return result;
}

/**
 * Transform a period value into a temporal integer value for
 * performing temporal count aggregation
 */
static TSequence *
period_transform_tcount(const Period *p)
{
  TSequence *result;
  Datum datum_one = Int32GetDatum(1);
  TInstant *instants[2];
  instants[0] = tinstant_make(datum_one, T_TINT, p->lower);
  if (p->lower == p->upper)
  {
    result = tsequence_make((const TInstant **) instants, 1, 1,
      p->lower_inc, p->upper_inc, STEPWISE, NORMALIZE_NO);
  }
  else
  {
    instants[1] = tinstant_make(datum_one, T_TINT, p->upper);
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
static TSequence **
periodset_transform_tcount(const PeriodSet *ps)
{
  TSequence **result = palloc(sizeof(TSequence *) * ps->count);
  for (int i = 0; i < ps->count; i++)
  {
    const Period *p = periodset_per_n(ps, i);
    result[i] = period_transform_tcount(p);
  }
  return result;
}

static void
ensure_same_timetype_skiplist(SkipList *state, uint8 subtype)
{
  Temporal *head = (Temporal *) skiplist_headval(state);
  if (head->subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
      errmsg("Cannot aggregate temporal values of different type")));
  return;
}

PG_FUNCTION_INFO_V1(Timestampset_tcount_transfn);
/**
 * Transition function for temporal count aggregate of timestamp sets
 */
PGDLLEXPORT Datum
Timestampset_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  TimestampSet *ts = PG_GETARG_TIMESTAMPSET_P(1);
  TInstant **instants = timestampset_transform_tcount(ts);
  if (state)
  {
    ensure_same_timetype_skiplist(state, TINSTANT);
    skiplist_splice(fcinfo, state, (void **) instants, ts->count,
      &datum_sum_int32, CROSSINGS_NO);
  }
  else
  {
    state = skiplist_make(fcinfo, (void **) instants, ts->count, TEMPORAL);
  }

  pfree_array((void **) instants, ts->count);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Period_tcount_transfn);
/**
 * Transition function for temporal count aggregate of periods
 */
PGDLLEXPORT Datum
Period_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  Period *p = PG_GETARG_SPAN_P(1);
  TSequence *seq = period_transform_tcount(p);
  if (state)
  {
    ensure_same_timetype_skiplist(state, TSEQUENCE);
    skiplist_splice(fcinfo, state, (void **) &seq, 1, &datum_sum_int32,
      CROSSINGS_NO);
  }
  else
  {
    state = skiplist_make(fcinfo, (void **) &seq, 1, TEMPORAL);
  }

  pfree(seq);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Periodset_tcount_transfn);
/**
 * Transition function for temporal count aggregate of period sets
 */
PGDLLEXPORT Datum
Periodset_tcount_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(state);
  PeriodSet *ps = PG_GETARG_PERIODSET_P(1);
  TSequence **sequences = periodset_transform_tcount(ps);
  if (state)
  {
    ensure_same_timetype_skiplist(state, TSEQUENCE);
    skiplist_splice(fcinfo, state, (void **) sequences, ps->count,
      &datum_sum_int32, CROSSINGS_NO);
  }
  else
  {
    state = skiplist_make(fcinfo, (void **) sequences, ps->count, TEMPORAL);
  }

  pfree_array((void **) sequences, ps->count);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(state);
}

/*****************************************************************************
 * Aggregate combine functions for time types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Time_tunion_combinefn);
/**
 * Combine function for union aggregate of time types
 */
PGDLLEXPORT Datum
Time_tunion_combinefn(PG_FUNCTION_ARGS)
{
  SkipList *state1, *state2;
  INPUT_AGG_COMB_STATE(state1, state2);
  SkipList *result = time_agg_combinefn(fcinfo, state1, state2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Aggregate final functions for time types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_tunion_finalfn);
/**
 * Final function for union aggregation of timestamp set values
 */
PGDLLEXPORT Datum
Timestamp_tunion_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  assert(state->elemtype == TIMESTAMPTZ);
  TimestampTz *values = (TimestampTz *) skiplist_values(state);
  TimestampSet *result = timestampset_make(values, state->length);
  pfree(values);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_tunion_finalfn);
/**
 * Final function for union aggregation of period (set) values
 */
PGDLLEXPORT Datum
Period_tunion_finalfn(PG_FUNCTION_ARGS)
{
  /* The final function is strict, we do not need to test for null values */
  SkipList *state = (SkipList *) PG_GETARG_POINTER(0);
  if (state->length == 0)
    PG_RETURN_NULL();

  assert(state->elemtype == PERIOD);
  const Period **values = (const Period **) skiplist_values(state);
  PeriodSet *result = periodset_make(values, state->length, NORMALIZE_NO);
  pfree(values);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
