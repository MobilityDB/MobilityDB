/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * @file time_aggfuncs.c
 * Aggregate functions for time types
 */

#include "time_aggfuncs.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>

#include "skiplist.h"
#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporal_util.h"

TimestampTz *
timestamp_agg(TimestampTz *times1, int count1, TimestampTz *times2, int count2,
   int *newcount);

Period **
period_agg(Period **periods1, int count1, Period **periods2, int count2,
  int *newcount);

/*****************************************************************************
 * Aggregation functions for time types
 *****************************************************************************/

/*
 * Generic aggregate function for timestamp set values
 *
 * @param[in] times1 Accumulated state
 * @param[in] count1 Number of elements in the accumulated state
 * @param[in] times2 Timestamps of the input timestamp set value
 * @param[in] count2 Number of elements in the timestamp set value
 * @note Returns new timestamps that must be freed by the calling function.
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
    int cmp = timestamp_cmp_internal(t1, t2);
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
 * @param[in] periods2 Periods of a period set value
 * @param[in] count2 Number of elements in the period set value
 * @param[in] newcount Number of elements in the result
 * @note Returns new periods that must be freed by the calling function.
 */
Period **
period_agg(Period **periods1, int count1, Period **periods2, int count2,
  int *newcount)
{
  Period **periods = palloc(sizeof(Period *) * (count1 + count2));
  int k = 0;
  for (int i = 0; i < count1; i++)
    periods[k++] = periods1[i];
  for (int i = 0; i < count2; i++)
    periods[k++] = periods2[i];
  Period **result = periodarr_normalize(periods, k, newcount);
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
 * @param[inout] state Timestamp array containing the state
 * @param[in] ts Timestamp set value
 */
static SkipList *
timestampset_agg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const TimestampSet *ts)
{
  TimestampTz *times = timestampset_timestamps_internal(ts);
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) times, TIMESTAMPTZ, ts->count);
  else
  {
    assert(state->elemtype == TIMESTAMPTZ);
    skiplist_splice(fcinfo, state, (void **) times, ts->count, NULL, false);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating temporal values
 * of sequence subtype
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] per Period value
 */
static SkipList *
period_agg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const Period *per)
{
  SkipList *result;
  if (! state)
    result = skiplist_make(fcinfo, (void **) &per, PERIOD, 1);
  else
  {
    assert(state->elemtype == PERIOD);
    skiplist_splice(fcinfo, state, (void **) &per, 1, NULL, false);
    result = state;
  }
  return result;
}

/**
 * Generic transition function for aggregating period set values
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] ps Period set value
 */
static SkipList *
periodset_agg_transfn(FunctionCallInfo fcinfo, SkipList *state,
  const PeriodSet *ps)
{
  const Period **periods = periodset_periods_internal(ps);
  SkipList *result;
  if (! state)
    /* Periods are copies while constructing the skiplist */
    result = skiplist_make(fcinfo, (void **) periods, PERIOD, ps->count);
  else
  {
    assert(state->elemtype == PERIOD);
    skiplist_splice(fcinfo, state, (void **) periods, ps->count, NULL, false);
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
  int count2 = state2->length;
  void **values = skiplist_values(state2);
  skiplist_splice(fcinfo, state1, values, count2, NULL, false);
  pfree(values);
  return state1;
}

/*****************************************************************************
 * Union transition functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestampset_tunion_transfn);
/**
 * Transition function for union aggregate of timestamp sets
 */
PGDLLEXPORT Datum
timestampset_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
  SkipList *result = timestampset_agg_transfn(fcinfo, state, ts);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(period_tunion_transfn);
/**
 * Transition function for union aggregate of periods
 */
PGDLLEXPORT Datum
period_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  Period *p = PG_GETARG_PERIOD(1);
  SkipList *result = period_agg_transfn(fcinfo, state, p);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(periodset_tunion_transfn);
/**
 * Transition function for union aggregate of period sets
 */
PGDLLEXPORT Datum
periodset_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  if (PG_ARGISNULL(1))
  {
    if (state)
      PG_RETURN_POINTER(state);
    else
      PG_RETURN_NULL();
  }

  PeriodSet *ps = PG_GETARG_PERIODSET(1);
  SkipList *result = periodset_agg_transfn(fcinfo, state, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Union combine functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(time_tunion_combinefn);
/**
 * Combine function for union aggregate of periods
 */
PGDLLEXPORT Datum
time_tunion_combinefn(PG_FUNCTION_ARGS)
{
  SkipList *state1 = PG_ARGISNULL(0) ? NULL :
    (SkipList *) PG_GETARG_POINTER(0);
  SkipList *state2 = PG_ARGISNULL(1) ? NULL :
    (SkipList *) PG_GETARG_POINTER(1);
  if (state1 == NULL && state2 == NULL)
    PG_RETURN_NULL();

  SkipList *result = time_agg_combinefn(fcinfo, state1, state2);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Aggregate final functions for time types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestamp_tunion_finalfn);
/**
 * Final function for aggregation of timestamp set values
 */
PGDLLEXPORT Datum
timestamp_tunion_finalfn(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(period_tunion_finalfn);
/**
 * Final function for aggregation of period (set) values
 */
PGDLLEXPORT Datum
period_tunion_finalfn(PG_FUNCTION_ARGS)
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
