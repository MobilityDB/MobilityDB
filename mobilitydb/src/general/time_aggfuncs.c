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
#include <libpq/pqformat.h>
#include <utils/memutils.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/skiplist.h"
#include "general/temporal_util.h"
#include "general/time_aggfuncs.h"
/* MobilityDB */
#include "pg_general/skiplist.h"
#include "pg_general/temporal.h"

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
  store_fcinfo(fcinfo);
  SkipList *result = timestampset_agg_transfn(state, ts);
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
  store_fcinfo(fcinfo);
  SkipList *result = period_agg_transfn(state, p);
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
  store_fcinfo(fcinfo);
  SkipList *result = periodset_agg_transfn(state, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

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
  store_fcinfo(fcinfo);
  TInstant **instants = timestampset_transform_tcount(ts);
  if (state)
  {
    ensure_same_timetype_skiplist(state, TINSTANT);
    skiplist_splice(state, (void **) instants, ts->count, &datum_sum_int32,
      CROSSINGS_NO);
  }
  else
  {
    state = skiplist_make((void **) instants, ts->count, TEMPORAL);
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
  store_fcinfo(fcinfo);
  TSequence *seq = period_transform_tcount(p);
  if (state)
  {
    ensure_same_timetype_skiplist(state, TSEQUENCE);
    skiplist_splice(state, (void **) &seq, 1, &datum_sum_int32,
      CROSSINGS_NO);
  }
  else
  {
    state = skiplist_make((void **) &seq, 1, TEMPORAL);
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
  store_fcinfo(fcinfo);
  TSequence **sequences = periodset_transform_tcount(ps);
  if (state)
  {
    ensure_same_timetype_skiplist(state, TSEQUENCE);
    skiplist_splice(state, (void **) sequences, ps->count,
      &datum_sum_int32, CROSSINGS_NO);
  }
  else
  {
    state = skiplist_make((void **) sequences, ps->count, TEMPORAL);
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
  store_fcinfo(fcinfo);
  SkipList *result = time_agg_combinefn(state1, state2);
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
