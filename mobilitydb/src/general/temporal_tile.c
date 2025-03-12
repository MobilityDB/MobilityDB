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
#include "geo/tgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/meos_catalog.h"
#include "pg_general/type_util.h"

/*****************************************************************************/

PGDLLEXPORT Datum Span_bins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_bins);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the bins of a span
 * @sqlfn bins()
 */
Datum
Span_bins(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Get input parameters */
    Span *bounds = PG_GETARG_SPAN_P(0);
    Datum size, origin;
    assert(numspan_type(bounds->spantype) || timespan_type(bounds->spantype));
    if (numspan_type(bounds->spantype))
    {
      size = PG_GETARG_DATUM(1);
      origin = PG_GETARG_DATUM(2);
      meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
      ensure_positive_datum(size, basetype);
    }
    else if (bounds->spantype == T_DATESPAN)
    {
      Interval *duration = PG_GETARG_INTERVAL_P(1);
      origin = PG_GETARG_DATUM(2);
      ensure_valid_day_duration(duration);
      size = Int32GetDatum((int)(interval_units(duration) / USECS_PER_DAY));
    }
    else /*(span->spantype == T_TSTZSPAN) */
    {
      Interval *duration = PG_GETARG_INTERVAL_P(1);
      origin = PG_GETARG_DATUM(2);
      ensure_valid_duration(duration);
      size = Int64GetDatum(interval_units(duration));
    }

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = span_bin_state_make(NULL, bounds, size, origin);
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

  /* Allocate span */
  Span *span = palloc(sizeof(Span));
  /* Used to construct the composite return value */
  Datum values[2];
  /* Store index */
  values[0] = Int32GetDatum(state->i);
  /* Generate bin */
  span_bin_state_set(state->value, state->size, state->span.basetype,
    state->span.spantype, span);
  values[1] = PointerGetDatum(span);
  /* Advance state */
  span_bin_state_next(state);
  /* Form tuple and return */
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Value_bin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_bin);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for number spans
 * @sqlfn getValueBin()
 */
Datum
Value_bin(PG_FUNCTION_ARGS)
{
  Datum value = PG_GETARG_DATUM(0);
  Datum size = PG_GETARG_DATUM(1);
  Datum origin = PG_GETARG_DATUM(2);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 1));
  meosType spantype = basetype_spantype(basetype);
  Datum value_bin = datum_bin(value, size, origin, basetype);
  Span *result = palloc(sizeof(Span));
  span_bin_state_set(value_bin, size, basetype, spantype, result);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Date_bin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Date_bin);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for date spans
 * @sqlfn getTimeBin()
 */
Datum
Date_bin(PG_FUNCTION_ARGS)
{
  DateADT d = PG_GETARG_DATEADT(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  DateADT origin = PG_GETARG_DATEADT(2);
  DateADT date_bin = date_get_bin(d, duration, origin);
  Span *result = palloc(sizeof(Span));
  int32 ndays = (int32) (interval_units(duration) / USECS_PER_DAY);
  span_bin_state_set(DateADTGetDatum(date_bin), Int32GetDatum(ndays),
    T_DATE, T_DATESPAN, result);
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Timestamptz_bin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_bin);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for timestamptz spans
 * @sqlfn getTimeBin()
 */
Datum
Timestamptz_bin(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  TimestampTz time_bin = timestamptz_get_bin(t, duration, origin);
  int64 tunits = interval_units(duration);
  Span *result = palloc(sizeof(Span));
  span_bin_state_set(TimestampTzGetDatum(time_bin), Int64GetDatum(tunits),
    T_TIMESTAMPTZ, T_TSTZSPAN, result);
  PG_RETURN_SPAN_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Spanset_time_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_time_spans);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return an array of spans obtained by splitting a spanset with respect
 * to time bins
 * @sqlfn timeSpans()
 */
Datum
Spanset_time_spans(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  Datum torigin = PG_GETARG_DATUM(2);
  /* Get the spans */
  int count;
  Span *spans = spanset_time_spans(ss, duration, torigin, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Spanset_value_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_value_spans);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return an array of spans obtained by splitting a spanset with respect
 * to value bins
 * @sqlfn valueSpans()
 */
Datum
Spanset_value_spans(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Datum vsize = PG_GETARG_DATUM(1);
  Datum vorigin = PG_GETARG_DATUM(2);
  /* Get the spans */
  int count;
  Span *spans = spanset_value_spans(ss, vsize, vorigin, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_time_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_time_spans);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the value spans of a temporal number split with respect to
 * value bins
 * @sqlfn timeSpans()
 */
Datum
Temporal_time_spans(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  /* Get the spans */
  int count;
  Span *spans = temporal_time_spans(temp, duration, torigin, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Tnumber_value_spans(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_spans);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the value spans of a temporal number split with respect to
 * value bins
 * @sqlfn valueSpans()
 */
Datum
Tnumber_value_spans(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum vsize = PG_GETARG_DATUM(1);
  Datum vorigin = PG_GETARG_DATUM(2);
  /* Get the spans */
  int count;
  Span *spans = tnumber_value_spans(temp, vsize, vorigin, &count);
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************
 * TBox functions
 *****************************************************************************/

/**
 * @brief Return the tiles of a temporal box
 */
Datum
Tbox_value_time_tiles_ext(FunctionCallInfo fcinfo, bool valuetiles,
  bool timetiles)
{
  assert(valuetiles || timetiles);

  FuncCallContext *funcctx;
  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize to 0 missing parameters */
    double xsize = 0, xorigin = 0;
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    /* Get input parameters and ensure their validity */
    TBox *bounds = PG_GETARG_TBOX_P(0);
    int i = 1;
    if (valuetiles)
    {
      ensure_has_X(T_TBOX, bounds->flags);
      xsize = PG_GETARG_FLOAT8(i++);
      ensure_positive_datum(Float8GetDatum(xsize), T_FLOAT8);
    }
    if (timetiles)
    {
      ensure_has_T(T_TBOX, bounds->flags);
      duration = PG_GETARG_INTERVAL_P(i++);
      ensure_valid_duration(duration);
    }
    if (valuetiles)
      xorigin = PG_GETARG_FLOAT8(i++);
    if (timetiles)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);

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
  Datum values[2]; /* used to construct the composite return value */
  values[0] = Int32GetDatum(state->i);
  /* Generate box */
  tbox_tile_state_set(state->value, state->t, state->vsize, state->tunits,
    state->box.span.basetype, state->box.span.spantype, box);
  values[1] = PointerGetDatum(box);
  /* Advance state */
  tbox_tile_state_next(state);
  /* Form tuple and return */
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

PGDLLEXPORT Datum Tbox_value_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_value_tiles);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the tile list of a temporal box
 * @sqlfn valueTimeTiles()
 */
Datum
Tbox_value_tiles(PG_FUNCTION_ARGS)
{
  return Tbox_value_time_tiles_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tbox_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_time_tiles);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the tile list of a temporal box
 * @sqlfn valueTimeTiles()
 */
Datum
Tbox_time_tiles(PG_FUNCTION_ARGS)
{
  return Tbox_value_time_tiles_ext(fcinfo, false, true);
}

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
  return Tbox_value_time_tiles_ext(fcinfo, true, true);
}

/*****************************************************************************/

/**
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * (external function)
 */
Datum
Tbox_get_value_time_tile_ext(FunctionCallInfo fcinfo, bool valuetile,
  bool timetile)
{
  assert(valuetile || timetile);

  /* Initialize to 0 missing dimensions */
  Datum value = (Datum) 0, vsize = (Datum) 0, vorigin = (Datum) 0;
  Interval *duration = NULL;
  TimestampTz t = 0, torigin = 0;
  /* Get input parameters */
  int i = 0;
  if (valuetile)
    value = PG_GETARG_DATUM(i++);
  if (timetile)
    t = PG_GETARG_TIMESTAMPTZ(i++);
  if (valuetile)
    vsize = PG_GETARG_DATUM(i++);
  if (timetile)
    duration = PG_GETARG_INTERVAL_P(i++);
  if (valuetile)
    vorigin = PG_GETARG_DATUM(i++);
  if (timetile)
    torigin = PG_GETARG_TIMESTAMPTZ(i++);
  meosType basetype = oid_type(get_fn_expr_argtype(fcinfo->flinfo, 0));
  meosType spantype = basetype_spantype(basetype);
  PG_RETURN_TBOX_P(tbox_get_value_time_tile(value, t, vsize, duration, vorigin,
    torigin, basetype, spantype));
}

PGDLLEXPORT Datum Tbox_get_value_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_get_value_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @sqlfn tile()
 */
Datum
Tbox_get_value_tile(PG_FUNCTION_ARGS)
{
  return Tbox_get_value_time_tile_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tbox_get_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_get_time_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @sqlfn tile()
 */
Datum
Tbox_get_time_tile(PG_FUNCTION_ARGS)
{
  return Tbox_get_value_time_tile_ext(fcinfo, false, true);
}

PGDLLEXPORT Datum Tbox_get_value_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_get_value_time_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in a multidimensional grid for temporal numbers
 * @sqlfn tile()
 */
Datum
Tbox_get_value_time_tile(PG_FUNCTION_ARGS)
{
  return Tbox_get_value_time_tile_ext(fcinfo, true, true);
}

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and/or time grid (external function)
 */
Datum
Tnumber_value_time_boxes_ext(FunctionCallInfo fcinfo, bool valueboxes,
  bool timeboxes)
{
  assert(valueboxes || timeboxes);

  /* Initialize to 0 missing dimensions */
  Datum vsize = (Datum) 0, vorigin = (Datum) 0;
  Interval *duration = NULL;
  TimestampTz torigin = 0;
  /* Get input parameters */
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int i = 1;
  if (valueboxes)
    vsize = PG_GETARG_DATUM(i++);
  if (timeboxes)
    duration = PG_GETARG_INTERVAL_P(i++);
  if (valueboxes)
    vorigin = PG_GETARG_DATUM(i++);
  if (timeboxes)
    torigin = PG_GETARG_TIMESTAMPTZ(i++);
  /* Get the tiles */
  int count;
  TBox *boxes = tnumber_value_time_boxes(temp, vsize, duration, vorigin,
    torigin, &count);
  ArrayType *result = tboxarr_to_array(boxes, count);
  /* Clean up and return */
  pfree(boxes);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Tnumber_value_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_boxes);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * value bins
 * @sqlfn valueBoxes()
 */
Datum
Tnumber_value_boxes(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_boxes_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tnumber_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_time_boxes);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * time bins
 * @sqlfn timeBoxes()
 */
Datum
Tnumber_time_boxes(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_boxes_ext(fcinfo, false, true);
}

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
  return Tnumber_value_time_boxes_ext(fcinfo, true, true);
}

/*****************************************************************************
 * Split functions
 *****************************************************************************/

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
  FuncCallContext *funcctx;

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
    Interval *duration = PG_GETARG_INTERVAL_P(1);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);

    /* Initialize state and verify parameter validity */
    int nbins;
    SpanBinState *state = temporal_time_bin_init(temp, duration, torigin,
      &nbins);

    /* Create function state */
    funcctx->user_fctx = state;

    /* Build a tuple description for a multidimensional grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* State that no value is null */
  bool isnull[2] = {0,0};
  /* Get state */
  SpanBinState *state = funcctx->user_fctx;
  /* We need to loop since atTbox may be NULL */
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      /* Switch to memory context appropriate for multiple function calls */
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }

    /* Get current tile (if any) and advance state */
    Span span;
    if (! span_bin_state_get(state, &span))
    {
      /* Switch to memory context appropriate for multiple function calls */
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }
    span_bin_state_next(state);

    /* Restrict the temporal point to the span */
    Temporal *atspan = temporal_restrict_tstzspan(state->to_split, &span,
      REST_AT);
    if (atspan == NULL)
      continue;

    /* Form tuple and return */
    Datum tuple_arr[2]; /* used to construct the composite return value */
    tuple_arr[0] = span.lower;
    tuple_arr[1] = PointerGetDatum(atspan);
    HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
    Datum result = HeapTupleGetDatum(tuple);
    SRF_RETURN_NEXT(funcctx, result);
  }
}

/*****************************************************************************/

/**
 * @brief Return the fragments of a temporal number split according to value
 * and time bins
 */
Datum
Tnumber_value_time_split_ext(FunctionCallInfo fcinfo, bool valuesplit,
  bool timesplit)
{
  FuncCallContext *funcctx;

  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);

    /* Initialize to 0 missing parameters */
    Datum vsize = (Datum) 0, vorigin = (Datum) 0;
    Interval *duration = NULL;
    TimestampTz torigin = 0;

    /* Get input parameters */
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    int i = 1;
    if (valuesplit)
      vsize = PG_GETARG_DATUM(i++);
    if (timesplit)
      duration = PG_GETARG_INTERVAL_P(i++);
    if (valuesplit)
      vorigin = PG_GETARG_DATUM(i++);
    if (timesplit)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);

    /* Initialize state and verify parameter validity */
    int ntiles;
    TboxGridState *state = tnumber_value_time_tile_init(temp, vsize, duration,
      vorigin, torigin, &ntiles);

    /* Create function state */
    funcctx->user_fctx = state;

    /* Build a tuple description for a multidimensional grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* State that no value is null */
  bool isnull[3] = {0,0,0};
  /* Get state */
  TboxGridState *state = funcctx->user_fctx;
  /* We need to loop since atTbox may be NULL */
  while (true)
  {
    /* Stop when we have used up all the grid tiles */
    if (state->done)
    {
      /* Switch to memory context appropriate for multiple function calls */
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }

    /* Get current tile (if any) and advance state */
    TBox box;
    if (! tbox_tile_state_get(state, &box))
    {
      /* Switch to memory context appropriate for multiple function calls */
      MemoryContext oldcontext =
        MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
      pfree(state);
      MemoryContextSwitchTo(oldcontext);
      SRF_RETURN_DONE(funcctx);
    }
    tbox_tile_state_next(state);

    /* Restrict the temporal point to the box */
    Temporal *attbox = tnumber_at_tbox(state->temp, &box);
    if (attbox == NULL)
      continue;

    /* Form tuple and return */
    Datum tuple_arr[3]; /* used to construct the composite return value */
    int i = 0;
    if (valuesplit)
      tuple_arr[i++] = box.span.lower;
    if (timesplit)
      tuple_arr[i++] = box.period.lower;
    tuple_arr[i++] = PointerGetDatum(attbox);
    HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
    Datum result = HeapTupleGetDatum(tuple);
    SRF_RETURN_NEXT(funcctx, result);
  }
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
  return Tnumber_value_time_split_ext(fcinfo, true, false);
}

PGDLLEXPORT Datum Tnumber_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_time_split);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the fragments of a temporal number split according to value
 * bins
 * @sqlfn valueSplit()
 */
Datum
Tnumber_time_split(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_split_ext(fcinfo, false, true);
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
  return Tnumber_value_time_split_ext(fcinfo, true, true);
}

/*****************************************************************************/
