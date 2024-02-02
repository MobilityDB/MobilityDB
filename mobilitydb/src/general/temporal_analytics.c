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
 * @brief Analytics for time and temporal values
 */

#include "general/temporal_analytics.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <access/htup_details.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include "general/set.h"
#include "general/span.h"
#include "general/temporal.h"
/* MobilityDB */
#include "pg_general/skiplist.h" /* For store_fcinfo */

/*****************************************************************************
 * Time precision functions for time types
 *****************************************************************************/

PGDLLEXPORT Datum Timestamptz_tprecision(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_tprecision);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return the initial timestamptz of the bucket in which a timestamptz
 * falls
 */
Datum
Timestamptz_tprecision(PG_FUNCTION_ARGS)
{
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_TIMESTAMPTZ(timestamptz_tprecision(t, duration, origin));
}

PGDLLEXPORT Datum Tstzset_tprecision(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzset_tprecision);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return a tstzset value with the precision set to period buckets
 * set falls
 */
Datum
Tstzset_tprecision(PG_FUNCTION_ARGS)
{
  Set *s = PG_GETARG_SET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_SET_P(tstzset_tprecision(s, duration, origin));
}

PGDLLEXPORT Datum Tstzspan_tprecision(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspan_tprecision);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return a tstzspan value with the precision set to period buckets
 * span falls
 */
Datum
Tstzspan_tprecision(PG_FUNCTION_ARGS)
{
  Span *s = PG_GETARG_SPAN_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  PG_RETURN_SPAN_P(tstzspan_tprecision(s, duration, origin));
}

PGDLLEXPORT Datum Tstzspanset_tprecision(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tstzspanset_tprecision);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return a tstzspanset value with the precision set to period buckets
 */
Datum
Tstzspanset_tprecision(PG_FUNCTION_ARGS)
{
  SpanSet *ss = PG_GETARG_SPANSET_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  SpanSet *result = tstzspanset_tprecision(ss, duration, origin);
  PG_FREE_IF_COPY(ss, 0);
  PG_RETURN_SPANSET_P(result);
}

/*****************************************************************************
 * Time precision and sample functions for temporal types
 *****************************************************************************/

PGDLLEXPORT Datum Temporal_tprecision(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_tprecision);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return a temporal value with the precision set to period buckets
 * @sqlfn tprecision()
 */
Datum
Temporal_tprecision(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  Temporal *result = temporal_tprecision(temp, duration, origin);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_tsample(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_tsample);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return a temporal value sampled at period buckets
 * @sqlfn tsample()
 */
Datum
Temporal_tsample(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  Temporal *result = temporal_tsample(temp, duration, origin);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Linear space computation of the similarity distance
 *****************************************************************************/

/**
 * @brief Generic similarity function between two temporal values
 */
Datum
Temporal_similarity(FunctionCallInfo fcinfo, SimFunc simfunc)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable for temporal geography points */
  if (temp1->temptype == T_TGEOGPOINT)
    store_fcinfo(fcinfo);
  double result = (simfunc == HAUSDORFF) ?
    temporal_hausdorff_distance(temp1, temp2) :
    temporal_similarity(temp1, temp2, simfunc);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum Temporal_frechet_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_frechet_distance);
/**
 * @ingroup mobilitydb_temporal_analytics_similarity
 * @brief Return the discrete Frechet distance between two temporal values
 * @sqlfn frechetDistance()
 */
Datum
Temporal_frechet_distance(PG_FUNCTION_ARGS)
{
  return Temporal_similarity(fcinfo, FRECHET);
}

PGDLLEXPORT Datum Temporal_dyntimewarp_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_dyntimewarp_distance);
/**
 * @ingroup mobilitydb_temporal_analytics_similarity
 * @brief Return the Dynamic Time Warp (DTW) distance between two temporal
 * values
 * @sqlfn dynTimeWarpDistance()
 */
Datum
Temporal_dyntimewarp_distance(PG_FUNCTION_ARGS)
{
  return Temporal_similarity(fcinfo, DYNTIMEWARP);
}

PGDLLEXPORT Datum Temporal_hausdorff_distance(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_hausdorff_distance);
/**
 * @ingroup mobilitydb_temporal_analytics_similarity
 * @brief Return the Hausdorff distance between two temporal values
 * @sqlfn hausdorffDistance()
 */
Datum
Temporal_hausdorff_distance(PG_FUNCTION_ARGS)
{
  return Temporal_similarity(fcinfo, HAUSDORFF);
}

/*****************************************************************************
 * Similarity path between two temporal values from the distance matrix
 *****************************************************************************/

/**
 * @brief Create the initial state that persists across multiple calls of the
 * function
 * @param[in] path Match path
 * @param[in] size Size of the path
 * @pre The size argument must be greater to 0.
 * @note The path is in reverse order and thus, we start from the last element
 */
static SimilarityPathState *
similarity_path_state_make(Match *path, int size)
{
  assert(size > 0);
  SimilarityPathState *state = palloc0(sizeof(SimilarityPathState));
  /* Fill in state */
  state->done = false;
  state->size = size;
  state->i = size - 1;
  state->path = path;
  return state;
}

/**
 * @brief Increment the current state to the next warp of the path
 * @param[in] state State to increment
 */
static void
similarity_path_state_next(SimilarityPathState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next match */
  state->i--;
  if (state->i < 0)
    state->done = true;
  return;
}

/*****************************************************************************
 * Quadratic space computation of the similarity path
 *****************************************************************************/

/**
 * @brief Compute the similarity path between two temporal values
 */
Datum
Temporal_similarity_path(FunctionCallInfo fcinfo, SimFunc simfunc)
{
  FuncCallContext *funcctx;
  SimilarityPathState *state;
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  Datum tuple_arr[2]; /* used to construct the composite return value */
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
    Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
    Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
    /* Store fcinfo into a global variable for temporal geography points */
    if (temp1->temptype == T_TGEOGPOINT)
      store_fcinfo(fcinfo);
    /* Compute the path */
    int count;
    Match *path = temporal_similarity_path(temp1, temp2, &count,
      simfunc);
    /* Create function state */
    funcctx->user_fctx = similarity_path_state_make(path, count);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
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
    pfree(state->path);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }
  /* Store index */
  tuple_arr[0] = Int32GetDatum(state->path[state->i].i);
  tuple_arr[1] = Int32GetDatum(state->path[state->i].j);
  /* Advance state */
  similarity_path_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

PGDLLEXPORT Datum Temporal_frechet_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_frechet_path);
/**
 * @ingroup mobilitydb_temporal_analytics_similarity
 * @brief Return the Frechet path between two temporal values
 * @sqlfn frechetDistancePath()
 */
Datum
Temporal_frechet_path(PG_FUNCTION_ARGS)
{
  return Temporal_similarity_path(fcinfo, FRECHET);
}

PGDLLEXPORT Datum Temporal_dyntimewarp_path(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_dyntimewarp_path);
/**
 * @ingroup mobilitydb_temporal_analytics_similarity
 * @brief Return the Dynamic Time Warp (DTW) path between two temporal values
 * @sqlfn dynTimeWarpPath()
 */
Datum
Temporal_dyntimewarp_path(PG_FUNCTION_ARGS)
{
  return Temporal_similarity_path(fcinfo, DYNTIMEWARP);
}

/*****************************************************************************/
