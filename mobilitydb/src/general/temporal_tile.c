/***********************************************************************
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
 * @brief Bucket and tile functions for temporal types.
 *
 * @note The time bucket functions are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "pg_general/temporal_tile.h"

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
#include "general/temporal_util.h"
/* MobilityDB */
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Number bucket functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Number_bucket);
/**
 * Return the initial value of the bucket in which an integer value falls.
 */
PGDLLEXPORT Datum
Number_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  mobdbType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  ensure_positive_datum(size, basetype);
  Datum result = datum_bucket(value, size, origin, basetype);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Timestamp bucket functions
 *****************************************************************************/

/**
 * Return the interval in the same representation as Postgres timestamps.
 */
int64
get_interval_units(Interval *interval)
{
  return interval->time + (interval->day * USECS_PER_DAY);
}

PG_FUNCTION_INFO_V1(Timestamptz_bucket);
/**
 * Return the initial timestamp of the bucket in which a timestamp falls.
 */
PGDLLEXPORT Datum
Timestamptz_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz timestamp = PG_GETARG_TIMESTAMPTZ(0);
  if (TIMESTAMP_NOT_FINITE(timestamp))
    PG_RETURN_TIMESTAMPTZ(timestamp);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz result = timestamptz_bucket(timestamp, tunits, origin);
  PG_RETURN_TIMESTAMPTZ(result);
}

/*****************************************************************************
 * Span bucket functions
 *****************************************************************************/

/**
 * Generate an integer or float span bucket from a bucket list
 *
 * @param[in] lower Start value of the bucket
 * @param[in] size Size of the buckets
 * @param[in] basetype Type of the arguments
 */
static Span *
span_bucket_get(Datum lower, Datum size, mobdbType basetype)
{
  Datum upper = (basetype == T_TIMESTAMPTZ) ?
    TimestampTzGetDatum(DatumGetTimestampTz(lower) + DatumGetInt64(size)) :
    datum_add(lower, size, basetype, basetype);
  return span_make(lower, upper, true, false, basetype);
}

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] temp Temporal number to split
 * @param[in] s Bounds for generating the bucket list
 * @param[in] size Size of the buckets
 * @param[in] origin Origin of the buckets
 *
 * @pre The size argument must be greater to 0.
 * @note The first argument is NULL when generating the bucket list, otherwise
 * it is a temporal number to be split and in this case s is the value span
 * of the temporal number
 */
static SpanBucketState *
span_bucket_state_make(Temporal *temp, Span *s, Datum size, Datum origin)
{
  SpanBucketState *state = palloc0(sizeof(SpanBucketState));
  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->temp = temp;
  state->basetype = s->basetype;
  state->size = size;
  state->origin = origin;
  /* intspans are in canonical form so their upper bound is exclusive */
  Datum upper = (s->basetype == T_INT4) ?
    Int32GetDatum(DatumGetInt32(s->upper) - 1) : s->upper;
  state->minvalue = state->value =
    datum_bucket(s->lower, size, origin, state->basetype);
  state->maxvalue = datum_bucket(upper, size, origin, state->basetype);
  return state;
}

/**
 * Increment the current state to the next bucket of the bucket list
 *
 * @param[in] state State to increment
 */
static void
span_bucket_state_next(SpanBucketState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next bucket */
  state->i++;
  state->value = (state->basetype == T_TIMESTAMPTZ) ?
    TimestampTzGetDatum(DatumGetTimestampTz(state->value) +
      DatumGetInt64(state->size)) :
    datum_add(state->value, state->size, state->basetype, state->basetype);
  if (datum_gt(state->value, state->maxvalue, state->basetype))
    state->done = true;
  return;
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
      mobdbType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
      ensure_positive_datum(size, type);
    }
    else
    {
      Interval *duration = PG_GETARG_INTERVAL_P(1);
      TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
      origin = TimestampTzGetDatum(torigin);
      ensure_valid_duration(duration);
      size = Int64GetDatum(get_interval_units(duration));
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = span_bucket_state_make(NULL, bounds, size, origin);
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

PG_FUNCTION_INFO_V1(Span_bucket_list);
/**
 * Generate a span bucket list.
 */
PGDLLEXPORT Datum
Span_bucket_list(PG_FUNCTION_ARGS)
{
  return Span_bucket_list_ext(fcinfo, true);
}

PG_FUNCTION_INFO_V1(Period_bucket_list);
/**
 * Generate a period bucket list.
 */
PGDLLEXPORT Datum
Period_bucket_list(PG_FUNCTION_ARGS)
{
  return Span_bucket_list_ext(fcinfo, false);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Span_bucket);
/**
 * Generate an integer or float span bucket in a bucket list for spans.
*/
PGDLLEXPORT Datum
Span_bucket(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  mobdbType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  ensure_positive_datum(size, type);
  Datum origin = PG_GETARG_DATUM(2);
  Datum value_bucket = datum_bucket(value, size, origin, type);
  Span *result = span_bucket_get(value_bucket, size, type);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Period_bucket);
/**
 * Generate a bucket in a bucket list for periods.
*/
PGDLLEXPORT Datum
Period_bucket(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz time_bucket = timestamptz_bucket(t, tunits, torigin);
  Period *result = span_bucket_get(TimestampTzGetDatum(time_bucket),
    Int64GetDatum(tunits), T_TIMESTAMPTZ);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * TBOX tile functions
 *****************************************************************************/

/**
 * Generate a tile from the a multidimensional grid
 *
 * @param[in] value Start value of the tile to output
 * @param[in] t Start timestamp of the tile to output
 * @param[in] xsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 */
static TBOX *
tbox_tile_get(double value, TimestampTz t, double xsize, int64 tunits)
{
  Datum xmin = Float8GetDatum(value);
  Datum xmax = Float8GetDatum(value + xsize);
  Datum tmin = TimestampTzGetDatum(t);
  Datum tmax = TimestampTzGetDatum(t + tunits);
  Period period;
  Span span;
  span_set(tmin, tmax, true, false, T_TIMESTAMPTZ, &period);
  span_set(xmin, xmax, true, false, T_FLOAT8, &span);
  return (TBOX *) tbox_make(&period, &span);
}

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] box Bounds of the multidimensional grid
 * @param[in] xsize Value size of the tiles
 * @param[in] tunits Time size of the tiles in PostgreSQL time units
 * @param[in] xorigin Value origin of the tiles
 * @param[in] torigin Time origin of the tiles
 *
 * @pre Both xsize and tunits must be greater than 0.
 */
static TboxGridState *
tbox_tile_state_make(TBOX *box, double xsize, int64 tunits, double xorigin,
  TimestampTz torigin)
{
  assert(xsize > 0 || tunits > 0);
  TboxGridState *state = palloc0(sizeof(TboxGridState));

  /* Fill in state */
  state->done = false;
  state->i = 1;
  state->xsize = xsize;
  state->tunits = tunits;
  if (xsize)
  {
    state->box.span.lower = Float8GetDatum(float_bucket(
      DatumGetFloat8(box->span.lower), xsize, xorigin));
    state->box.span.upper = Float8GetDatum(float_bucket(
      DatumGetFloat8(box->span.upper), xsize, xorigin));
  }
  if (tunits)
  {
    state->box.period.lower = TimestampTzGetDatum(timestamptz_bucket(
      DatumGetTimestampTz(box->period.lower), tunits, torigin));
    state->box.period.upper = TimestampTzGetDatum(timestamptz_bucket(
      DatumGetTimestampTz(box->period.upper), tunits, torigin));
  }
  state->value = DatumGetFloat8(state->box.span.lower);
  state->t = DatumGetTimestampTz(state->box.period.lower);
  return state;
}

/**
 * Increment the current state to the next tile of the multidimensional grid
 *
 * @param[in] state State to increment
 */
static void
tbox_tile_state_next(TboxGridState *state)
{
  if (!state || state->done)
      return;
  /* Move to the next tile */
  state->i++;
  state->value += state->xsize;
  if (state->value > DatumGetFloat8(state->box.span.upper))
  {
    state->value = DatumGetFloat8(state->box.span.lower);
    state->t += state->tunits;
    if (state->t > DatumGetTimestampTz(state->box.period.upper))
    {
      state->done = true;
      return;
    }
  }
  return;
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_multidim_grid);
/**
 * Generate a multidimensional grid for temporal numbers.
 */
PGDLLEXPORT Datum
Tbox_multidim_grid(PG_FUNCTION_ARGS)
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
    TBOX *bounds = PG_GETARG_TBOX_P(0);
    double xsize = PG_GETARG_FLOAT8(1);
    Interval *duration = PG_GETARG_INTERVAL_P(2);
    double xorigin = PG_GETARG_FLOAT8(3);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

    /* Ensure parameter validity */
    ensure_has_X_tbox(bounds);
    ensure_has_T_tbox(bounds);
    ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
    ensure_valid_duration(duration);
    int64 tunits = get_interval_units(duration);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = tbox_tile_state_make(bounds, xsize, tunits, xorigin, torigin);
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

  /* Store tile value and time */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate box */
  tuple_arr[1] = PointerGetDatum(tbox_tile_get(state->value, state->t,
    state->xsize, state->tunits));
  /* Advance state */
  tbox_tile_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tbox_multidim_tile);
/**
 * Generate a tile in a multidimensional grid for temporal numbers.
*/
PGDLLEXPORT Datum
Tbox_multidim_tile(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double xsize = PG_GETARG_FLOAT8(2);
  ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
  Interval *duration = PG_GETARG_INTERVAL_P(3);
  ensure_valid_duration(duration);
  int64 tunits = get_interval_units(duration);
  double xorigin = PG_GETARG_FLOAT8(4);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(5);
  double value_bucket = float_bucket(value, xsize, xorigin);
  TimestampTz time_bucket = timestamptz_bucket(t, tunits, torigin);
  TBOX *result = tbox_tile_get(value_bucket, time_bucket, xsize, tunits);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Value and time split functions for temporal numbers
 *****************************************************************************/

/**
 * Create the initial state that persists across multiple calls of the function
 *
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
 * Increment the current state to the next tile of the multidimensional grid
 *
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
Tnumber_value_time_split_ext(FunctionCallInfo fcinfo, bool valuesplit,
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
    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    mobdbType basetype = temptype_basetype(temp->temptype);
    Datum size, origin;
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    int64 tunits = 0;
    int i = 1;
    if (valuesplit)
    {
      size = PG_GETARG_DATUM(i++);
      ensure_positive_datum(size, basetype);
    }
    if (timesplit)
    {
      duration = PG_GETARG_INTERVAL_P(i++);
      ensure_valid_duration(duration);
      tunits = get_interval_units(duration);
    }
    if (valuesplit)
      origin = PG_GETARG_DATUM(i++);
    if (timesplit)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);


    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Compute the value bounds, if any */
    Datum start_bucket = Float8GetDatum(0), end_bucket = Float8GetDatum(0);
    int value_count = 1;
    if (valuesplit)
    {
      Span *span = tnumber_to_span((const Temporal *) temp);
      Datum start_value = span->lower;
      /* We need to add size to obtain the end value of the last bucket */
      Datum end_value = datum_add(span->upper, size, basetype, basetype);
      start_bucket = datum_bucket(start_value, size, origin, basetype);
      end_bucket = datum_bucket(end_value, size, origin, basetype);
      value_count = (basetype == T_INT4) ?
        (DatumGetInt32(end_bucket) - DatumGetInt32(start_bucket)) /
          DatumGetInt32(size) :
        floor((DatumGetFloat8(end_bucket) - DatumGetFloat8(start_bucket)) /
          DatumGetFloat8(size));
    }

    /* Compute the time bounds, if any */
    TimestampTz start_time_bucket = 0, end_time_bucket = 0;
    int time_count = 1;
    if (timesplit)
    {
      Period p;
      temporal_set_period(temp, &p);
      TimestampTz start_time = p.lower;
      TimestampTz end_time = p.upper;
      start_time_bucket = timestamptz_bucket(start_time, tunits, torigin);
      /* We need to add tunits to obtain the end timestamp of the last bucket */
      end_time_bucket = timestamptz_bucket(end_time, tunits, torigin) + tunits;
      time_count =
        (int) (((int64) end_time_bucket - (int64) start_time_bucket) / tunits);
    }

    /* Adjust the number of tiles */
    int count = value_count * time_count;

    /* Split the temporal value */
    Datum *value_buckets = NULL;
    TimestampTz *time_buckets = NULL;
    Temporal **fragments;
    int newcount = 0;
    if (valuesplit && ! timesplit)
    {
      fragments = tnumber_value_split(temp, start_bucket, size, count,
        &value_buckets, &newcount);
    }
    else if (! valuesplit && timesplit)
    {
      fragments = temporal_time_split(temp, start_time_bucket, end_time_bucket,
        tunits, torigin, count, &time_buckets, &newcount);
    }
    else /* valuesplit && timesplit */
    {
      value_buckets = palloc(sizeof(Datum) * count);
      time_buckets = palloc(sizeof(TimestampTz) * count);
      fragments = palloc(sizeof(Temporal *) * count);
      int k = 0;
      Datum lower_value = start_bucket;
      while (datum_lt(lower_value, end_bucket, basetype))
      {
        Datum upper_value = datum_add(lower_value, size, basetype, basetype);
        Span s;
        span_set(lower_value, upper_value, true, false, basetype, &s);
        Temporal *atspan = tnumber_restrict_span(temp, &s, REST_AT);
        if (atspan != NULL)
        {
          int num_time_splits;
          TimestampTz *times;
          Temporal **time_splits = temporal_time_split(atspan,
            start_time_bucket, end_time_bucket, tunits, torigin, time_count,
            &times, &num_time_splits);
          for (int i = 0; i < num_time_splits; i++)
          {
            value_buckets[i + k] = lower_value;
            time_buckets[i + k] = times[i];
            fragments[i + k] = time_splits[i];
          }
          k += num_time_splits;
          pfree(time_splits);
          pfree(times);
          pfree(atspan);
        }
        lower_value = upper_value;
      }
      newcount = k;
    }

    assert(newcount > 0);
    /* Create function state */
    funcctx->user_fctx = value_time_split_state_make(size, tunits,
      value_buckets, time_buckets, fragments, newcount);
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

PG_FUNCTION_INFO_V1(Temporal_time_split);
/**
 * Split a temporal value into fragments with respect to period buckets.
 */
PGDLLEXPORT Datum
Temporal_time_split(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_split_ext(fcinfo, false, true);
}

PG_FUNCTION_INFO_V1(Tnumber_value_split);
/**
 * Split a temporal value into fragments with respect to period tiles.
 */
PGDLLEXPORT Datum
Tnumber_value_split(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_split_ext(fcinfo, true, false);
}

PG_FUNCTION_INFO_V1(Tnumber_value_time_split);
/**
 * Split a temporal value into fragments with respect to span and period tiles.
 */
PGDLLEXPORT Datum
Tnumber_value_time_split(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_split_ext(fcinfo, true, true);
}

/*****************************************************************************/
