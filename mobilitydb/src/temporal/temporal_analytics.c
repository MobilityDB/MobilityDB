/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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

#include "temporal/temporal_analytics.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <access/htup_details.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include "temporal/set.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
/* MobilityDB */
#include "pg_temporal/skiplist.h"  /* For store_fcinfo */
#include "pg_temporal/temporal.h"  /* For input_interp_string */
#include "pg_temporal/type_util.h" /* For geoarr_extract */

/*****************************************************************************
 * Time precision functions for time types
 *****************************************************************************/

PGDLLEXPORT Datum Timestamptz_tprecision(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Timestamptz_tprecision);
/**
 * @ingroup mobilitydb_temporal_analytics_reduction
 * @brief Return the initial timestamptz of the bin in which a timestamptz
 * falls
 * @sqlfn tPrecision()
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
 * @brief Return a tstzset with the precision set to time bins
 * @sqlfn tPrecision()
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
 * @brief Return a tstzspan with the precision set to time bins
 * @sqlfn tPrecision()
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
 * @brief Return a tstzspanset value with the precision set to time bins
 * @sqlfn tPrecision()
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
 * @brief Return a temporal value with the precision set to time bins
 * @sqlfn tPrecision()
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
 * @brief Return a temporal value sampled at time bins
 * @sqlfn tSample()
 */
Datum
Temporal_tsample(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *duration = PG_GETARG_INTERVAL_P(1);
  TimestampTz origin = PG_GETARG_TIMESTAMPTZ(2);
  interpType interp = input_interp_string(fcinfo, 3);
  Temporal *result = temporal_tsample(temp, duration, origin, interp);
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
  assert(path); assert(size > 0);

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
  if (! state || state->done)
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
  SimilarityPathState *state = funcctx->user_fctx;
  /* Stop when we've used up all bins */
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
  Datum values[2]; /* used to construct the composite return value */
  values[0] = Int32GetDatum(state->path[state->i].i);
  values[1] = Int32GetDatum(state->path[state->i].j);
  /* Advance state */
  similarity_path_state_next(state);
  /* Form tuple and return */
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
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

PGDLLEXPORT Datum Temporal_simplify_min_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_min_dist);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Return a temporal sequence (set) float or point simplified ensuring
 * that consecutive values are at least a given distance apart
 * @sqlfn minDistSimplify()
 */
Datum
Temporal_simplify_min_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  Temporal *result = temporal_simplify_min_dist(temp, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_simplify_min_tdelta(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_min_tdelta);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Return a temporal sequence (set) float or point simplified ensuring
 * that consecutive values are at least a given distance apart
 * @sqlfn minTimeDeltaSimplify()
 */
Datum
Temporal_simplify_min_tdelta(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *mint = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_simplify_min_tdelta(temp, mint);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_simplify_max_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_max_dist);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Return a temporal sequence (set) float or point simplified using a
 * single-pass Douglas-Peucker line simplification algorithm
 * @sqlfn maxDistSimplify
 */
Datum
Temporal_simplify_max_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = temporal_simplify_max_dist(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Temporal_simplify_dp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_dp);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Return a temporal sequence (set) float or point simplified using a
 * Douglas-Peucker line simplification algorithm
 * @sqlfn douglasPeuckerSimplify()
 */
Datum
Temporal_simplify_dp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = temporal_simplify_dp(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Outlier dectection functions
 *****************************************************************************/

/**
 * @brief Struct for storing the state that persists across multiple calls for
 * output a set of records composed of a geometry and a score
 */
typedef struct WLOF_State
{
  int count;
  GSERIALIZED **geos;
  double *scores;
  int i;
} WLOF_State;

/**
 * @brief Create the initial state to output the set of records obtained from
 * the weighted local outlier factor
 * @param[in] geos Array of geometries
 * @param[in] scores Array of scores
 * @param[in] count Number of elements in the arrays
 */
WLOF_State *
wlof_state_make(GSERIALIZED **geos, double *scores, int count)
{
  /* palloc0 to initialize the counters to 0 */
  WLOF_State *state = palloc0(sizeof(WLOF_State));
  /* Fill in state */
  state->count = count;
  state->geos = geos;
  state->scores = scores;
  return state;
}

/**
 * @brief Get the current record to output
 * @param[in] state State to increment
 * @param[out] gs Current geometry
 * @param[out] score Current score
 */
bool
wlof_state_get(WLOF_State *state, GSERIALIZED **gs, double *score)
{
  if (! state || state->i >= state->count)
    return false;
  /* Get the box of the n-th geometry */
  *gs = state->geos[state->i];
  *score = state->scores[state->i++];
  return true;
}

PGDLLEXPORT Datum Geo_wlof(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geo_wlof);
/**
 * @ingroup mobilitydb_temporal_modif
 * @brief Compute the weighted local outlier factor (WLOF) of an array of
 * geometries
 * @sqlfn wlocalOutlierFunction()
 */
Datum
Geo_wlof(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  /* If the function is being called for the first time */
  if (SRF_IS_FIRSTCALL())
  {
    ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
    int k = PG_GETARG_INT32(1);
    double distance = PG_GETARG_FLOAT8(2);
    ensure_not_empty_array(array);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Extract the geometries */
    int count;
    GSERIALIZED **geoarr = geoarr_extract(array, &count);
    /* Compute the function */
    uint32_t newcount;
    GSERIALIZED **clusters;
    double *scores = geo_wlof((const GSERIALIZED **) geoarr, count, k,
      distance, &newcount, &clusters);
    /* If it was not possible to compute the scores, e.g. when the number of
     * resulting clusters is not greater than k */
    if (! scores)
    {
      funcctx = SRF_PERCALL_SETUP();
      SRF_RETURN_DONE(funcctx);
    }
    /* Create function state */
    funcctx->user_fctx = wlof_state_make(clusters, scores, newcount);
    /* Build a tuple description for a multidim_grid tuple */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }

  /* Stuff done on every call of the function */
  funcctx = SRF_PERCALL_SETUP();
  /* Get state */
  WLOF_State *state = funcctx->user_fctx;
  /* Stop when we've used up all geometries */
  GSERIALIZED *gs;
  double score;
  if (! wlof_state_get(state, &gs, &score))
  {
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    pfree(state);
    MemoryContextSwitchTo(oldcontext);
    SRF_RETURN_DONE(funcctx);
  }

  /* Form tuple and return
   * The i value was incremented with the previous _next function call */
  Datum values[2]; /* used to construct the composite return value */
  values[0] = PointerGetDatum(gs);
  values[1] = Float8GetDatum(score);
  bool isnull[2] = {0,0}; /* needed to say no value is null */
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  Datum result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/
