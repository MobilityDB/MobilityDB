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

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/skiplist.h"

/*****************************************************************************
 * Aggregate transition functions for time types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_extent_transfn);
/**
 * Transition function for extent aggregation of timestamp values
 */
PGDLLEXPORT Datum
Timestamp_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  if (PG_ARGISNULL(1))
    PG_RETURN_POINTER(p);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  p = timestamp_extent_transfn(p, t);
  if (! p)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(p);
}

PG_FUNCTION_INFO_V1(Tstzset_extent_transfn);
/**
 * Transition function for extent aggregation of timestamp set values
 */
PGDLLEXPORT Datum
Tstzset_extent_transfn(PG_FUNCTION_ARGS)
{
  Span *p = PG_ARGISNULL(0) ? NULL : PG_GETARG_SPAN_P(0);
  Set *ts = PG_ARGISNULL(1) ? NULL : PG_GETARG_SET_P(1);
  p = tstzset_extent_transfn(p, ts);
  PG_FREE_IF_COPY(ts, 1);
  if (! p)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(p);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Timestamp_tunion_transfn);
/**
 * Transition function for union aggregate of timestamp sets
 */
PGDLLEXPORT Datum
Timestamp_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  store_fcinfo(fcinfo);
  SkipList *result = timestamp_tunion_transfn(state, t);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tstzset_tunion_transfn);
/**
 * Transition function for union aggregate of timestamp sets
 */
PGDLLEXPORT Datum
Tstzset_tunion_transfn(PG_FUNCTION_ARGS)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Set *ts = PG_GETARG_SET_P(1);
  store_fcinfo(fcinfo);
  SkipList *result = tstzset_tunion_transfn(state, ts);
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
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Span *p = PG_GETARG_SPAN_P(1);
  store_fcinfo(fcinfo);
  SkipList *result = period_tunion_transfn(state, p);
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
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  SpanSet *ps = PG_GETARG_SPANSET_P(1);
  store_fcinfo(fcinfo);
  SkipList *result = periodset_tunion_transfn(state, ps);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Transition function for temporal count aggregate of timestamps
 */
Datum
Timestamp_tcount_transfn_ext(FunctionCallInfo fcinfo, bool bucket)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Interval *interval = NULL;
  TimestampTz origin = 0;
  if (bucket)
  {
    if (PG_NARGS() > 1 && ! PG_ARGISNULL(2))
      interval = PG_GETARG_INTERVAL_P(2);
    origin = PG_GETARG_TIMESTAMPTZ(3);
  }
  store_fcinfo(fcinfo);
  state = timestamp_tcount_transfn(state, t, interval, origin);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Timestamp_tcount_transfn);
/**
 * Transition function for temporal count aggregate of timestamps
 */
PGDLLEXPORT Datum
Timestamp_tcount_transfn(PG_FUNCTION_ARGS)
{
  return Timestamp_tcount_transfn_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Timestamp_tcount_bucket_transfn);
/**
 * Transition function for temporal count aggregate of timestamps
 */
PGDLLEXPORT Datum
Timestamp_tcount_bucket_transfn(PG_FUNCTION_ARGS)
{
  return Timestamp_tcount_transfn_ext(fcinfo, true);
}

/**
 * Transition function for temporal count aggregate of timestamp sets
 */
Datum
Tstzset_tcount_transfn_ext(FunctionCallInfo fcinfo, bool bucket)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Set *ts = PG_GETARG_SET_P(1);
  Interval *interval = NULL;
  TimestampTz origin = 0;
  if (bucket)
  {
    if (PG_NARGS() > 1 && ! PG_ARGISNULL(2))
      interval = PG_GETARG_INTERVAL_P(2);
    origin = PG_GETARG_TIMESTAMPTZ(3);
  }
  store_fcinfo(fcinfo);
  state = tstzset_tcount_transfn(state, ts, interval, origin);
  PG_FREE_IF_COPY(ts, 1);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Tstzset_tcount_transfn);
/**
 * Transition function for temporal count aggregate of timestamp sets
 */
PGDLLEXPORT Datum
Tstzset_tcount_transfn(PG_FUNCTION_ARGS)
{
  return Tstzset_tcount_transfn_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Tstzset_tcount_bucket_transfn);
/**
 * Transition function for temporal count aggregate of timestamp sets
 */
PGDLLEXPORT Datum
Tstzset_tcount_bucket_transfn(PG_FUNCTION_ARGS)
{
  return Tstzset_tcount_transfn_ext(fcinfo, true);
}

/**
 * Transition function for temporal count aggregate of periods
 */
Datum
Period_tcount_transfn_ext(FunctionCallInfo fcinfo, bool bucket)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  Span *p = PG_GETARG_SPAN_P(1);
  Interval *interval = NULL;
  TimestampTz origin = 0;
  if (bucket)
  {
    if (PG_NARGS() > 1 && ! PG_ARGISNULL(2))
      interval = PG_GETARG_INTERVAL_P(2);
    origin = PG_GETARG_TIMESTAMPTZ(3);
  }
  store_fcinfo(fcinfo);
  state = period_tcount_transfn(state, p, interval, origin);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Period_tcount_transfn);
/**
 * Transition function for temporal count aggregate of periods
 */
PGDLLEXPORT Datum
Period_tcount_transfn(PG_FUNCTION_ARGS)
{
  return Period_tcount_transfn_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Period_tcount_bucket_transfn);
/**
 * Transition function for temporal count aggregate of periods
 */
PGDLLEXPORT Datum
Period_tcount_bucket_transfn(PG_FUNCTION_ARGS)
{
  return Period_tcount_transfn_ext(fcinfo, true);
}

/**
 * Transition function for temporal count aggregate of period sets
 */
Datum
Periodset_tcount_transfn_ext(FunctionCallInfo fcinfo, bool bucket)
{
  SkipList *state;
  INPUT_AGG_TRANS_STATE(fcinfo, state);
  SpanSet *ps = PG_GETARG_SPANSET_P(1);
  Interval *interval = NULL;
  TimestampTz origin = 0;
  if (bucket)
  {
    if (PG_NARGS() > 1 && ! PG_ARGISNULL(2))
      interval = PG_GETARG_INTERVAL_P(2);
    origin = PG_GETARG_TIMESTAMPTZ(3);
  }
  store_fcinfo(fcinfo);
  state = periodset_tcount_transfn(state, ps, interval, origin);
  PG_FREE_IF_COPY(ps, 1);
  PG_RETURN_POINTER(state);
}

PG_FUNCTION_INFO_V1(Periodset_tcount_transfn);
/**
 * Transition function for temporal count aggregate of period sets
 */
PGDLLEXPORT Datum
Periodset_tcount_transfn(PG_FUNCTION_ARGS)
{
  return Periodset_tcount_transfn_ext(fcinfo, false);
}

PG_FUNCTION_INFO_V1(Periodset_tcount_bucket_transfn);
/**
 * Transition function for temporal count aggregate of period sets
 */
PGDLLEXPORT Datum
Periodset_tcount_bucket_transfn(PG_FUNCTION_ARGS)
{
  return Periodset_tcount_transfn_ext(fcinfo, true);
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
  INPUT_AGG_COMB_STATE(fcinfo, state1, state2);
  store_fcinfo(fcinfo);
  SkipList *result = time_tagg_combinefn(state1, state2);
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
  Set *result = timestamp_tunion_finalfn(state);
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
  SpanSet *result = period_tunion_finalfn(state);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
