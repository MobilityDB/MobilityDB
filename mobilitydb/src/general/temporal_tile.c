/***********************************************************************
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
 * @file
 * @brief Bucket and tile functions for temporal types.
 *
 * @note The time bucket functions are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/datetime.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
#include "general/temporal_tile.h"
#include "general/type_util.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Number bucket functions
 *****************************************************************************/

PGDLLEXPORT Datum Number_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_bucket);
/**
 * @brief Return the initial value of the bucket in which an integer value falls.
 */
Datum
Number_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  Datum result = datum_bucket(value, size, origin, basetype);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Timestamp bucket functions
 *****************************************************************************/

PGDLLEXPORT Datum Timestamptz_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_bucket);
/**
 * @brief Return the initial timestamp of the bucket in which a timestamp falls.
 */
Datum
Timestamptz_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz result = timestamptz_bucket(t, duration, origin);
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************/

Datum
Span_bucket_list_ext(FunctionCallInfo fcinfo, bool valuelist)
{
  FuncCallContext *funcctx;
  SpanBucketState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Span *bounds = PG_GETARG_SPAN_P(0);
    Datum size, origin;
    if (valuelist)
    {
      size = PG_GETARG_DATUM(1);
      origin = PG_GETARG_DATUM(2);
      meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
      ensure_positive_datum(size, basetype);
    }
    else
    {
      Interval *duration = PG_GETARG_INTERVAL_P(1);
      TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
      origin = TimestampTzGetDatum(torigin);
      ensure_valid_duration(duration);
      size = Int64GetDatum(interval_units(duration));
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = span_bucket_state_make(bounds, size, origin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all buckets */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store index */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate bucket */
  tuple_arr[1] = PointerGetDatum(span_bucket_get(state->value, state->size,
    state->basetype));
  /* Advance state */
  span_bucket_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_bucket_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_bucket_list);
/**
 * @brief Generate a span bucket list.
 */
Datum
Span_bucket_list(PG_FUNCTION_ARGS)
{
  return Span_bucket_list_ext(fcinfo, true);
}

PGDLLEXPORT Datum Period_bucket_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Period_bucket_list);
/**
 * @brief Generate a period bucket list.
 */
Datum
Period_bucket_list(PG_FUNCTION_ARGS)
{
  return Span_bucket_list_ext(fcinfo, false);
}

/*****************************************************************************/

PGDLLEXPORT Datum Span_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_bucket);
/**
 * @brief Generate an integer or float span bucket in a bucket list for spans.
*/
Datum
Span_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  meosType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum value_bucket = datum_bucket(value, size, origin, type);
  Span *result = span_bucket_get(value_bucket, size, type);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Period_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Period_bucket);
/**
 * @brief Generate a bucket in a bucket list for periods.
*/
Datum
Period_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz time_bucket = timestamptz_bucket(t, duration, origin);
  int64 tunits = interval_units(duration);
  Span *result = span_bucket_get(TimestampTzGetDatum(time_bucket),
    Int64GetDatum(tunits), T_TIMESTAMPTZ);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_tile_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tile_list);
/**
 * @brief Generate a multidimensional grid for temporal numbers.
 */
Datum
Tbox_tile_list(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  TboxGridState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    TBox *bounds = PG_GETARG_TBOX_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    double xorigin = PG_GETARG_FLOAT8(3);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

    /* Ensure parameter validity */
    ensure_has_X_tbox(bounds);
    ensure_has_T_tbox(bounds);
    ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
    ensure_valid_duration(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = tbox_tile_state_make(bounds, xsize, duration, xorigin, torigin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've used up all tiles */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Allocate box */
  TBox *box = palloc(sizeof(STBox));
  /* Store tile value and time */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate box */
  tbox_tile_get(state->value, state->t, state->xsize, state->tunits, box);
  tuple_arr[1] = PointerGetDatum(box);
  /* Advance state */
  tbox_tile_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_tile);
/**
 * @brief Generate a tile in a multidimensional grid for temporal numbers.
*/
Datum
Tbox_tile(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double xsize = PG_GETARG_FLOAT8(2);
  ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
  Interval *duration = PG_GETARG_INTERVAL_P(3);
  ensure_valid_duration(duration);
  int64 tunits = interval_units(duration);
  double xorigin = PG_GETARG_FLOAT8(4);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(5);
  double value_bucket = float_bucket(value, xsize, xorigin);
  TimestampTz time_bucket = timestamptz_bucket(t, duration, torigin);
  TBox *result = palloc(sizeof(TBox));
  tbox_tile_get(value_bucket, time_bucket, xsize, tunits, result);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Value and time split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] size Value bucket size
 * @param[in] tunits Time bucket size
 * @param[in] value_buckets Initial values of the tiles
 * @param[in] time_buckets Initial timestamps of the tiles
 * @param[in] fragments Fragments of the input temporal value
 * @param[in] count Number of elements in the input arrays
 *
 * @pre count is greater than 0
 */
ValueTimeSplitState *
value_time_split_state_make(Datum size, int64 tunits, Datum *value_buckets,
  TimestampTz *time_buckets, Temporal **fragments, int count)
{
  assert(count > 0);
  ValueTimeSplitState *state = palloc0(sizeof(ValueTimeSplitState));
  /* Fill in state */
  state->done = false;
  state->size = size;
  state->tunits = tunits;
  state->value_buckets = value_buckets;
  state->time_buckets = time_buckets;
  state->fragments = fragments;
  state->i = 0;
  state->count = count;
  return state;
}

/**
 * @brief Increment the current state to the next tile of the multidimensional
 * grid
 * @param[in] state State to increment
 */
void
value_time_split_state_next(ValueTimeSplitState *state)
{
  /* Move to the next split */
  state->i++;
  if (state->i == state->count)
    state->done = true;
  return;
}

/*****************************************************************************
 * External value and time split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Split a temporal value with respect to a base value and possibly a
 * temporal grid.
 */
Datum
Temporal_value_time_split_ext(FunctionCallInfo fcinfo, bool valuesplit,
  bool timesplit)
{
  assert(valuesplit || timesplit);
  FuncCallContext *funcctx;
  ValueTimeSplitState *state;
  bool isnull[3] = {0,0,0}; /* needed to say no value is null */
  Datum tuple_arr[3]; /* used to construct the composite return value */
  HeapTuple tuple;
  Datum result; /* the actual composite return value */

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    Datum size = 0, vorigin = 0; /* make compiler quiet */
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    int i = 1;
    if (valuesplit)
      size = PG_GETARG_DATUM(i++);
    if (timesplit)
      duration = PG_GETARG_INTERVAL_P(i++);
    if (valuesplit)
      vorigin = PG_GETARG_DATUM(i++);
    if (timesplit)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);

    Datum *value_buckets = NULL;
    TimestampTz *time_buckets = NULL;
    int count;
    Temporal **fragments = temporal_value_time_split1(temp, size, duration,
      vorigin, torigin, valuesplit, timesplit, &value_buckets, &time_buckets,
      &count);

    assert(count > 0);
    int64 tunits = 0;
    if (timesplit)
      tunits = interval_units(duration);

    /* Create function state */
    funcctx->user_fctx = value_time_split_state_make(size, tunits,
      value_buckets, time_buckets, fragments, count);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp, 0);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  state = funcctx->user_fctx;
  /* Stop when we've output all the fragments */
  if (state->done)
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    for (int i = 0; i < state->count; i++)
      pfree(state->fragments[i]);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Store value, timestamp, and split */
  int j = 0;
  if (valuesplit)
    tuple_arr[j++] = state->value_buckets[state->i];
  if (timesplit)
    tuple_arr[j++] = TimestampTzGetDatum(state->time_buckets[state->i]);
  tuple_arr[j++] = PointerGetDatum(state->fragments[state->i]);
  /* Advance state */
  value_time_split_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_time_split);
/**
 * @brief Split a temporal value into fragments with respect to period buckets.
 */
Datum
Temporal_time_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Tnumber_value_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_split);
/**
 * @brief Split a temporal value into fragments with respect to period tiles.
 */
Datum
Tnumber_value_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tnumber_value_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_time_split);
/**
 * @brief Split a temporal value into fragments with respect to span and period
 * tiles.
 */
Datum
Tnumber_value_time_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, true, true);
}

/*****************************************************************************/
