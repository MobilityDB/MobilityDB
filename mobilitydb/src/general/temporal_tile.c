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
 * @brief Bin and tile functions for temporal types
 *
 * @note The time bin functions are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "general/temporal_tile.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
#include <funcapi.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/span.h"
#include "general/tbox.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/type_util.h"

/*****************************************************************************/

/**
 * @brief Return the bins of a span
 */
Datum
Span_spans_ext(FunctionCallInfo fcinfo, bool valuelist)
{
  FuncCallContext *funcctx;
  bool isnull[2] = {0,0}; /* needed to say no value is null */

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
    funcctx->user_fctx = span_bin_state_make(bounds, size, origin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  SpanBinState *state = funcctx->user_fctx;
  /* Stop when we've used up all bins */
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
  Datum tuple_arr[2]; /* used to construct the composite return value */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate bin */
  tuple_arr[1] = PointerGetDatum(span_bin_state_get(state->value, state->size,
    state->basetype));
  /* Advance state */
  span_bin_state_next(state);
  /* Form tuple and return */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Numberspan_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Numberspan_spans);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the bins of a number span
 * @sqlfn valueSpans()
 */
Datum
Numberspan_spans(PG_FUNCTION_ARGS)
{
  return Span_spans_ext(fcinfo, true);
}

PGDLLEXPORT Datum Tstzspan_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_spans);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the bins of a timestamptz span
 * @sqlfn timeSpans()
 */
Datum
Tstzspan_spans(PG_FUNCTION_ARGS)
{
  return Span_spans_ext(fcinfo, false);
}

/*****************************************************************************/

PGDLLEXPORT Datum Value_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_span);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for number spans
 * @sqlfn getValueSpan()
 */
Datum
Value_span(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  meosType type = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Datum value_bin = datum_bin(value, size, origin, type);
  PG_RETURN_SPAN_P(span_bin_state_get(value_bin, size, type));
}

PGDLLEXPORT Datum Tstzspan_span(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_span);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for timestamptz spans
 * @sqlfn getTimeSpan()
 */
Datum
Tstzspan_span(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz time_bin = timestamptz_get_bin(t, duration, origin);
  int64 tunits = interval_units(duration);
  PG_RETURN_SPAN_P(span_bin_state_get(TimestampTzGetDatum(time_bin),
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
  bool isnull[2] = {0,0}; /* needed to say no value is null */

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
    funcctx->user_fctx = tbox_tile_state_make(NULL, bounds, 
      Float8GetDatum(xsize), duration, Float8GetDatum(xorigin), torigin);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  TboxGridState *state = funcctx->user_fctx;
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
  Datum tuple_arr[2]; /* used to construct the composite return value */
  tuple_arr[0] = Int32GetDatum(state->i);
  /* Generate box */
  tbox_tile_set(state->value, state->t, state->vsize, state->tunits,
    state->box.span.basetype, state->box.span.spantype, box);
  tuple_arr[1] = PointerGetDatum(box);
  /* Advance state */
  tbox_tile_state_next(state);
  /* Form tuple and return */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  Datum result = HeapTupleGetDatum(tuple);
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
  meosType spantype = basetype_spantype(basetype);
  PG_RETURN_TBOX_P(tbox_value_time_tile(value, t, vsize, duration, vorigin,
    torigin, basetype, spantype));
}

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

PGDLLEXPORT Datum Tnumber_value_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_time_boxes);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and time grid
 * @sqlfn valueTimeBoxes()
 */
Datum
Tnumber_value_time_boxes(PG_FUNCTION_ARGS)
{
  /* Get input parameters */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum vsize = PG_GETARG_DATUM(1);
  Interval *duration = PG_GETARG_INTERVAL_P(2);
  Datum vorigin = PG_GETARG_DATUM(3);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(4);

  /* Get the tiles */
  int count;
  TBox *boxes = tnumber_value_time_boxes(temp, vsize, duration, vorigin,
    torigin, &count);
  ArrayType *result = tboxarr_to_array(boxes, count);
  pfree(boxes);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * Value and time split functions for temporal numbers
 *****************************************************************************/

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] vsize Value bin size
 * @param[in] tunits Time bin size
 * @param[in] value_bins Initial values of the tiles
 * @param[in] time_bins Initial timestamps of the tiles
 * @param[in] fragments Fragments of the input temporal value
 * @param[in] count Number of elements in the input arrays
 * @pre count is greater than 0
 */
ValueTimeSplitState *
value_time_split_state_make(Datum vsize, int64 tunits, Datum *value_bins,
  TimestampTz *time_bins, Temporal **fragments, int count)
{
  assert(count > 0);
  ValueTimeSplitState *state = palloc0(sizeof(ValueTimeSplitState));
  /* Fill in state */
  state->done = false;
  state->vsize = vsize;
  state->tunits = tunits;
  state->value_bins = value_bins;
  state->time_bins = time_bins;
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
  bool isnull[3] = {0,0,0}; /* needed to say no value is null */

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
    int i = 1;
    Datum vsize = valuesplit ? PG_GETARG_DATUM(i++) : 0;
    Interval *duration = timesplit ? PG_GETARG_INTERVAL_P(i++) : NULL;
    Datum vorigin = valuesplit ? PG_GETARG_DATUM(i++) : 0;
    TimestampTz torigin = timesplit ? PG_GETARG_TIMESTAMPTZ(i++) : 0;

    Datum *value_bins = NULL;
    TimestampTz *time_bins = NULL;
    int count;
    Temporal **fragments;
    if (valuesplit && ! timesplit)
      fragments = tnumber_value_split(temp, vsize, vorigin, &value_bins,
        &count);
    else if (! valuesplit && timesplit)
      fragments = temporal_time_split(temp, duration, torigin, &time_bins,
        &count);
    else /* valuesplit && timesplit */
      fragments = tnumber_value_time_split(temp, vsize, duration, vorigin,
        torigin, &value_bins, &time_bins, &count);

    assert(count > 0);
    int64 tunits = 0;
    if (timesplit)
      tunits = interval_units(duration);

    /* Create function state */
    funcctx->user_fctx = value_time_split_state_make(vsize, tunits,
      value_bins, time_bins, fragments, count);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp, 0);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  ValueTimeSplitState *state = funcctx->user_fctx;
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
  Datum tuple_arr[3]; /* used to construct the composite return value */
  int j = 0;
  if (valuesplit)
    tuple_arr[j++] = state->value_bins[state->i];
  if (timesplit)
    tuple_arr[j++] = TimestampTzGetDatum(state->time_bins[state->i]);
  tuple_arr[j++] = PointerGetDatum(state->fragments[state->i]);
  /* Advance state */
  value_time_split_state_next(state);
  /* Form tuple and return */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_time_split);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the fragments of a temporal value split according to
 * time bins
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
 * bins
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
 * and time bins
 * @sqlfn valueTimeSplit()
 */
Datum
Tnumber_value_time_split(PG_FUNCTION_ARGS)
{
  return Temporal_value_time_split_ext(fcinfo, true, true);
}

/*****************************************************************************/
