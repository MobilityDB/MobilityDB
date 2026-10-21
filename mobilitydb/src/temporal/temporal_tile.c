/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @note The time bin functions are inspired from TimescaleDB.
 * https://docs.timescale.com/latest/api#time_bucket
 */

#include "temporal/temporal_tile.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include <utils/array.h>
#include <funcapi.h>
#include <access/htup_details.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/span.h"
#include "temporal/tbox.h"
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/type_util.h"

/*****************************************************************************
 * Bins functions
 *****************************************************************************/

PGDLLEXPORT Datum Span_bins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Span_bins);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return an array of spans obtained by splitting a span with respect
 * to value or time bins
 * @sqlfn bins()
 */
Datum
Span_bins(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  bool border_inc = PG_GETARG_BOOL(3);
  /* Get the spans */
  int count;
  Span *spans;
  switch (s->spantype)
  {
    case T_INTSPAN:
      spans = intspan_bins(s, PG_GETARG_INT32(1), PG_GETARG_INT32(2),
        border_inc, &count);
      break;
    case T_BIGINTSPAN:
      spans = bigintspan_bins(s, PG_GETARG_INT64(1), PG_GETARG_INT64(2),
        border_inc, &count);
      break;
    case T_FLOATSPAN:
      spans = floatspan_bins(s, PG_GETARG_FLOAT8(1), PG_GETARG_FLOAT8(2),
        border_inc, &count);
      break;
    case T_DATESPAN:
      spans = datespan_bins(s, PG_GETARG_INTERVAL_P(1), PG_GETARG_DATEADT(2),
        border_inc, &count);
      break;
    case T_TSTZSPAN:
      spans = tstzspan_bins(s, PG_GETARG_INTERVAL_P(1),
        PG_GETARG_TIMESTAMPTZ(2), border_inc, &count);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE, "Unknown span type: %s",
        meostype_name(s->spantype));
      PG_RETURN_NULL();
  }
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_RETURN_ARRAYTYPE_P(result);
}


PGDLLEXPORT Datum Spanset_bins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Spanset_bins);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return an array of spans obtained by splitting a spanset with respect
 * to value or time bins
 * @sqlfn bins()
 */
Datum
Spanset_bins(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  bool border_inc = PG_GETARG_BOOL(3);
  /* Get the spans */
  int count;
  Span *spans;
  switch (ss->spantype)
  {
    case T_INTSPAN:
      spans = intspanset_bins(ss, PG_GETARG_INT32(1), PG_GETARG_INT32(2),
        border_inc, &count);
      break;
    case T_BIGINTSPAN:
      spans = bigintspanset_bins(ss, PG_GETARG_INT64(1), PG_GETARG_INT64(2),
        border_inc, &count);
      break;
    case T_FLOATSPAN:
      spans = floatspanset_bins(ss, PG_GETARG_FLOAT8(1), PG_GETARG_FLOAT8(2),
        border_inc, &count);
      break;
    case T_DATESPAN:
      spans = datespanset_bins(ss, PG_GETARG_INTERVAL_P(1),
        PG_GETARG_DATEADT(2), border_inc, &count);
      break;
    case T_TSTZSPAN:
      spans = tstzspanset_bins(ss, PG_GETARG_INTERVAL_P(1),
        PG_GETARG_TIMESTAMPTZ(2), border_inc, &count);
      break;
    default:
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_TYPE, "Unknown span type: %s",
        meostype_name(ss->spantype));
      PG_RETURN_NULL();
  }
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Value_bin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Value_bin);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for number spans
 * @sqlfn getBin()
 */
Datum
Value_bin(PG_FUNCTION_ARGS)
{
  MeosType basetype = oid_meostype(get_fn_expr_argtype(fcinfo->flinfo, 1));
  Span *result;
  if (basetype == T_INT4)
    result = intspan_get_bin(PG_GETARG_INT32(0), PG_GETARG_INT32(1),
      PG_GETARG_INT32(2));
  else if (basetype == T_INT8)
    result = bigintspan_get_bin(PG_GETARG_INT64(0), PG_GETARG_INT64(1),
      PG_GETARG_INT64(2));
  else /* basetype == T_FLOAT8 */
    result = floatspan_get_bin(PG_GETARG_FLOAT8(0), PG_GETARG_FLOAT8(1),
      PG_GETARG_FLOAT8(2));
  PG_RETURN_SPAN_P(result);
}

PGDLLEXPORT Datum Date_bin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Date_bin);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for date spans
 * @sqlfn getBin()
 */
Datum
Date_bin(PG_FUNCTION_ARGS)
{
  DateADT d = PG_GETARG_DATEADT(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  DateADT origin = PG_GETARG_DATEADT(2);
  PG_RETURN_SPAN_P(datespan_get_bin(d, duration, origin));
}

PGDLLEXPORT Datum Timestamptz_bin(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_bin);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a span bin in a bin list for timestamptz spans
 * @sqlfn getBin()
 */
Datum
Timestamptz_bin(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_SPAN_P(tstzspan_get_bin(t, duration, origin));
}

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_time_bins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_time_bins);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the value spans of a temporal number split with respect to
 * value bins
 * @sqlfn timeBins()
 */
Datum
Temporal_time_bins(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  bool border_inc = PG_GETARG_BOOL(3);
  /* Get the spans */
  int count;
  Span *spans = temporal_time_bins(temp, duration, torigin, border_inc,
    &count);
  ArrayType *result = spanarr_to_array(spans, count);
  /* Clean up and return */
  pfree(spans);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_ARRAYTYPE_P(result);
}

PGDLLEXPORT Datum Tnumber_value_bins(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_bins);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the value spans of a temporal number split with respect to
 * value bins
 * @sqlfn valueBins()
 */
Datum
Tnumber_value_bins(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum vsize = PG_GETARG_DATUM(1);
  Datum vorigin = PG_GETARG_DATUM(2);
  bool border_inc = PG_GETARG_BOOL(3);
  /* Get the spans */
  int count;
  Span *spans = tnumber_value_bins(temp, vsize, vorigin, border_inc, &count);
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
Tbox_value_time_tiles_common(FunctionCallInfo fcinfo, bool valuetiles,
  bool timetiles)
{
  assert(valuetiles || timetiles);

  FuncCallContext *funcctx;
  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    /* Initialize to 0 missing parameters */
    Datum vsize = Float8GetDatum(0.0), vorigin = Float8GetDatum(0.0);
    MeosType basetype = T_UNKNOWN;
    Interval *duration = NULL;
    TimestampTz torigin = 0;
    /* Get input parameters */
    TBox *bounds = PG_GETARG_TBOX_P(0);
    int i = 1;
    if (valuetiles)
    {
      basetype = oid_meostype(get_fn_expr_argtype(fcinfo->flinfo, i));
      vsize = PG_GETARG_DATUM(i++);
    }
    if (timetiles)
      duration = PG_GETARG_INTERVAL_P(i++);
    if (valuetiles)
      vorigin = PG_GETARG_DATUM(i++);
    if (timetiles)
      torigin = PG_GETARG_TIMESTAMPTZ(i++);
    bool border_inc = PG_GETARG_BOOL(i++);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    int ntiles;
    funcctx->user_fctx = tbox_value_time_tile_init(bounds, vsize, duration,
      vorigin, torigin, basetype, border_inc, &ntiles);
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
  tbox_tile_state_get(state, box);
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
 * @sqlfn valueTiles()
 */
inline Datum
Tbox_value_tiles(PG_FUNCTION_ARGS)
{
  return Tbox_value_time_tiles_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Tbox_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_time_tiles);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the tile list of a temporal box
 * @sqlfn timeTiles()
 */
inline Datum
Tbox_time_tiles(PG_FUNCTION_ARGS)
{
  return Tbox_value_time_tiles_common(fcinfo, false, true);
}

PGDLLEXPORT Datum Tbox_value_time_tiles(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_value_time_tiles);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the tile list of a temporal box
 * @sqlfn valueTimeTiles()
 */
inline Datum
Tbox_value_time_tiles(PG_FUNCTION_ARGS)
{
  return Tbox_value_time_tiles_common(fcinfo, true, true);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tbox_get_value_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_get_value_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in the value grid of a temporal box
 * @sqlfn getValueTile()
 */
Datum
Tbox_get_value_tile(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  double vsize = PG_GETARG_FLOAT8(1);
  double vorigin = PG_GETARG_FLOAT8(2);
  PG_RETURN_TBOX_P(tbox_get_value_tile(value, vsize, vorigin));
}

PGDLLEXPORT Datum Tbox_get_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_get_time_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in the time grid of a temporal box
 * @sqlfn getTBoxTimeTile()
 */
Datum
Tbox_get_time_tile(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_TBOX_P(tbox_get_time_tile(t, duration, torigin));
}

PGDLLEXPORT Datum Tbox_get_value_time_tile(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tbox_get_value_time_tile);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return a tile in the value and time grid of a temporal box
 * @sqlfn getValueTimeTile()
 */
Datum
Tbox_get_value_time_tile(PG_FUNCTION_ARGS)
{
  double value = PG_GETARG_FLOAT8(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  double vsize = PG_GETARG_FLOAT8(2);
  Interval *duration = PG_GETARG_INTERVAL_P(3);
  double vorigin = PG_GETARG_FLOAT8(4);
  TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(5);
  PG_RETURN_TBOX_P(tbox_get_value_time_tile(value, t, vsize, duration,
    vorigin, torigin));
}

/*****************************************************************************
 * Boxes functions
 *****************************************************************************/

/**
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and/or time grid (external function)
 */
Datum
Tnumber_value_time_boxes_common(FunctionCallInfo fcinfo, bool valueboxes,
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
  bool border_inc = PG_GETARG_BOOL(i++);
  /* Get the tiles */
  int count;
  TBox *boxes = tnumber_value_time_boxes(temp, vsize, duration, vorigin,
    torigin, border_inc, &count);
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
inline Datum
Tnumber_value_boxes(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_boxes_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Tnumber_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_time_boxes);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * time bins
 * @sqlfn timeBoxes()
 */
inline Datum
Tnumber_time_boxes(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_boxes_common(fcinfo, false, true);
}

PGDLLEXPORT Datum Tnumber_value_time_boxes(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_time_boxes);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the temporal boxes of a temporal number split with respect to
 * a value and time grid
 * @sqlfn valueTimeBoxes()
 */
inline Datum
Tnumber_value_time_boxes(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_boxes_common(fcinfo, true, true);
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

  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    /* The fragments are kept across the calls, so they are computed in the
     * context that outlives them */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    Temporal *temp = PG_GETARG_TEMPORAL_P(0);
    Interval *duration = PG_GETARG_INTERVAL_P(1);
    TimestampTz torigin = PG_GETARG_TIMESTAMPTZ(2);
    bool border_inc = PG_GETARG_BOOL(3);
    TimestampTz *bins;
    int count;
    Temporal **frags = temporal_time_split(temp, duration, torigin,
      border_inc, &bins, &count);
    Datum *state = palloc(sizeof(Datum) * 2);
    state[0] = PointerGetDatum(bins);
    state[1] = PointerGetDatum(frags);
    funcctx->user_fctx = state;
    funcctx->max_calls = count;
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  funcctx = SRF_PERCALL_SETUP();
  if (funcctx->call_cntr >= funcctx->max_calls)
    SRF_RETURN_DONE(funcctx);
  Datum *state = (Datum *) funcctx->user_fctx;
  const TimestampTz *bins = (const TimestampTz *) DatumGetPointer(state[0]);
  Temporal **frags = (Temporal **) DatumGetPointer(state[1]);
  Datum values[2];
  bool isnull[2] = {false, false};
  values[0] = TimestampTzGetDatum(bins[funcctx->call_cntr]);
  values[1] = PointerGetDatum(frags[funcctx->call_cntr]);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

/*****************************************************************************/

/**
 * @brief Return the fragments of a temporal number split according to value
 * and time bins
 */
Datum
Tnumber_value_time_split_common(FunctionCallInfo fcinfo, bool valuesplit,
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
    bool border_inc = PG_GETARG_BOOL(i++);

    /* Initialize state and verify parameter validity */
    int ntiles;
    TboxGridState *state = tnumber_value_time_tile_init(temp, vsize, duration,
      vorigin, torigin, border_inc, &ntiles);

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
    if (! attbox)
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
inline Datum
Tnumber_value_split(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_split_common(fcinfo, true, false);
}

PGDLLEXPORT Datum Tnumber_value_time_split(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tnumber_value_time_split);
/**
 * @ingroup mobilitydb_temporal_analytics_tile
 * @brief Return the fragments of a temporal number split according to value
 * and time bins
 * @sqlfn valueTimeSplit()
 */
inline Datum
Tnumber_value_time_split(PG_FUNCTION_ARGS)
{
  return Tnumber_value_time_split_common(fcinfo, true, true);
}

/*****************************************************************************/
