/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * @brief Bucket and tile functions for temporal types
 *
 * @note The time bucket functions are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "general/temporal_tile.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/tbox.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"

/*****************************************************************************
 * Number bucket functions
 *****************************************************************************/

PGDLLEXPORT Datum Number_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Number_bucket);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the initial value of the bucket in which an integer value falls
 * @sqlfn valueBucket()
 */
Datum
Number_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_DATUM(datum_bucket(value, size, origin, basetype));
}

/*****************************************************************************
 * Timestamp bucket functions
 *****************************************************************************/

PGDLLEXPORT Datum Timestamptz_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_bucket);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the initial timestamp of the bucket in which a timestamp falls
 * @sqlfn timeBucket()
 */
Datum
Timestamptz_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_TIMESTAMPTZ(timestamptz_bucket(t, duration, origin));
}

/*****************************************************************************/

/**
 * @brief Return the bucket list of a span
 */
Datum
Span_bucket_list(FunctionCallInfo fcinfo, bool valuelist)
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

PGDLLEXPORT Datum Numberspan_bucket_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numberspan_bucket_list);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the bucket list of a number span
 * @sqlfn bucketList()
 */
Datum
Numberspan_bucket_list(PG_FUNCTION_ARGS)
{
  return Span_bucket_list(fcinfo, true);
}

PGDLLEXPORT Datum Tstzspan_bucket_list(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_bucket_list);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the bucket list of a timestamptz span
 * @sqlfn bucketList()
 */
Datum
Tstzspan_bucket_list(PG_FUNCTION_ARGS)
{
  return Span_bucket_list(fcinfo, false);
}

/*****************************************************************************/

PGDLLEXPORT Datum Valuespan_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Valuespan_bucket);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bucket in a bucket list for number spans
 * @sqlfn spanBucket()
 */
Datum
Valuespan_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  meosType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum value_bucket = datum_bucket(value, size, origin, type);
  PG_RETURN_SPAN_P(span_bucket_get(value_bucket, size, type));
}

PGDLLEXPORT Datum Tstzspan_bucket(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_bucket);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a bucket in a bucket list for timestamptz spans.
 * @sqlfn periodBucket()
 */
Datum
Tstzspan_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz time_bucket = timestamptz_bucket(t, duration, origin);
  int64 tunits = interval_units(duration);
  PG_RETURN_SPAN_P(span_bucket_get(TimestampTzGetDatum(time_bucket),
    Int64GetDatum(tunits), T_TIMESTAMPTZ));
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_value_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_value_time_tiles);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the tile list of a temporal box
 * @sqlfn valueTimeTiles()
 */
Datum
Tbox_value_time_tiles(PG_FUNCTION_ARGS)
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
    funcctx->user_fctx = tbox_tile_state_make(bounds, Float8GetDatum(xsize),
      duration, Float8GetDatum(xorigin), torigin);
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
  tbox_tile_get(state->value, state->t, state->vsize, state->tunits,
    state->box.span.basetype, box);
  tuple_arr[1] = PointerGetDatum(box);
  /* Advance state */
  tbox_tile_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_value_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_value_time_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @sqlfn tile()
 */
Datum
Tbox_value_time_tile(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Datum vsize = PG_GETARG_DATUM(2);
  Interval *duration = PG_GETARG_INTERVAL_P(3);
  Datum vorigin = PG_GETARG_DATUM(4);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(5);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  PG_RETURN_TBOX_P(tbox_value_time_tile(value, t, vsize, duration, vorigin,
    torigin, basetype));
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
 * temporal grid
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
    Temporal **fragments;
    if (valuesplit && ! timesplit)
      fragments = tnumber_value_split(temp, size, vorigin, &value_buckets,
        &count);
    else if (! valuesplit && timesplit)
      fragments = temporal_time_split(temp, duration, torigin, &time_buckets,
        &count);
    else /* valuesplit && timesplit */
      fragments = tnumber_value_time_split(temp, size, duration, vorigin,
        torigin, &value_buckets, &time_buckets, &count);

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
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the fragments of a temporal value split according to
 * time buckets
 * @sqlfn timeSplit()
 */
Datum
Temporal_time_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Tnumber_value_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_split);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the fragments of a temporal number split according to value
 * buckets
 * @sqlfn valueSplit()
 */
Datum
Tnumber_value_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tnumber_value_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_time_split);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the fragments of a temporal number split according to value
 * and time buckets
 * @sqlfn valueTimeSplit()
 */
Datum
Tnumber_value_time_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, true, true);
}

/*****************************************************************************/
