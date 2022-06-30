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
 * @brief Similarity distance for temporal values. Currently, discrete Frechet
 * distance and Dynamic Time Warping (DTW) distance are implemented.
 */

#include "general/temporal_similarity.h"

/* C */
#include <assert.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <access/htup_details.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Linear space computation of the similarity distance
 *****************************************************************************/

Datum
temporal_similarity_ext(FunctionCallInfo fcinfo, SimFunc simfunc)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable for temporal geographic points */
  if (temp1->temptype == T_TGEOGPOINT)
    store_fcinfo(fcinfo);
  double result = temporal_similarity(temp1, temp2, simfunc);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Temporal_frechet_distance);
/**
 * Compute the discrete Frechet distance between two temporal values.
 */
PGDLLEXPORT Datum
Temporal_frechet_distance(PG_FUNCTION_ARGS)
{
  return temporal_similarity_ext(fcinfo, FRECHET);
}

PG_FUNCTION_INFO_V1(Temporal_dynamic_time_warp);
/**
 * Compute the Dynamic Time Match (DTW) distance between two temporal values.
 */
PGDLLEXPORT Datum
Temporal_dynamic_time_warp(PG_FUNCTION_ARGS)
{
  return temporal_similarity_ext(fcinfo, DYNTIMEWARP);
}

/*****************************************************************************
 * Compute the similarity path between two temporal values from the distance
 * matrix
 *****************************************************************************/

/**
 * Create the initial state that persists across multiple calls of the function
 *
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
 * Increment the current state to the next warp of the path
 *
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
 * Compute the Dynamic Time Match (DTW) path between two temporal values.
 */
Datum
temporal_similarity_path_ext(FunctionCallInfo fcinfo, SimFunc simfunc)
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
    /* Get input parameters */
    Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
    Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
    /* Store fcinfo into a global variable for temporal geographic points */
    if (temp1->temptype == T_TGEOGPOINT)
      store_fcinfo(fcinfo);
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
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

PG_FUNCTION_INFO_V1(Temporal_frechet_path);
/**
 * Compute the Frechet path between two temporal values.
 */
PGDLLEXPORT Datum
Temporal_frechet_path(PG_FUNCTION_ARGS)
{
  return temporal_similarity_path_ext(fcinfo, FRECHET);
}

PG_FUNCTION_INFO_V1(Temporal_dynamic_time_warp_path);
/**
 * Compute the Dynamic Time Warp (DTW) path between two temporal values.
 */
PGDLLEXPORT Datum
Temporal_dynamic_time_warp_path(PG_FUNCTION_ARGS)
{
  return temporal_similarity_path_ext(fcinfo, DYNTIMEWARP);
}

/*****************************************************************************/
