/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * @file temporal_similarity.c
 * Similarity distance for temporal values. Currently, the discrete Frechet
 * distance and the Dynamic Time Warping (DTW) distance are implemented.
 */

#include "general/temporal_similarity.h"

#include <postgres.h>
#include <assert.h>
#include <funcapi.h>
#include <math.h>
#if POSTGRESQL_VERSION_NUMBER < 120000
#include <access/htup_details.h>
#endif

#include <liblwgeom.h>

#include "general/tempcache.h"
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Compute the distance between two instants depending on their type
 *****************************************************************************/

/**
 * Compute the distance between two temporal instants.
 *
 * @param[in] inst1,inst2 Temporal instants
 */
static double
tnumberinst_distance(const TInstant *inst1, const TInstant *inst2)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  double result = fabs(datum_double(value1, inst1->basetypid) -
    datum_double(value2, inst2->basetypid));
  return result;
}

/**
 * Compute the distance between two temporal instants.
 *
 * @param[in] inst1,inst2 Temporal instants
 */
static double
tpointinst_distance(const TInstant *inst1, const TInstant *inst2)
{
  datum_func2 func = get_pt_distance_fn(inst1->flags);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  double result = DatumGetFloat8(func(value1, value2));
  return result;
}

/**
 * Compute the distance between two temporal instants.
 *
 * @param[in] inst1,inst2 Temporal instants
 */
static double
tinstant_distance(const TInstant *inst1, const TInstant *inst2)
{
  if (tnumber_base_type(inst1->basetypid))
    return tnumberinst_distance(inst1, inst2);
  if (tgeo_base_type(inst1->basetypid))
    return tpointinst_distance(inst1, inst2);
  elog(ERROR, "Unexpected base type in function tinstant_distance");
}

/*****************************************************************************
 * Linear space computation of the similarity distance
 *****************************************************************************/

/**
 * Linear space computation of the similarity distance between two temporal
 * values. Only two rows of the full matrix are used.
 *
 * @param[out] dist Array keeping the distances
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
static double
tinstantarr_similarity1(double *dist, const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc)
{
  
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      const TInstant *inst1 = instants1[i];
      const TInstant *inst2 = instants2[j];
      double d = tinstant_distance(inst1, inst2);
      if (i > 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i%2 * count2 + j] = Max(d,
            Min(dist[(i - 1)%2 * count2 + j - 1],
              Min(dist[(i - 1)%2 * count2 + j], dist[i%2 * count2 + j - 1])));
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i%2 * count2 + j] = d +
            Min(dist[(i - 1)%2 * count2 + j - 1],
              Min(dist[(i - 1)%2 * count2 + j], dist[i%2 * count2 + j - 1]));
        }
      }
      else if (i > 0 && j == 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i%2 * count2] = Max(d, dist[(i - 1)%2 * count2]);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i%2 * count2] = d + dist[(i - 1)%2 * count2];
        }
      }
      else if (i == 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[j] = Max(d, dist[j - 1]);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[j] = d + dist[j - 1];
        }
      }
      else /* i == 0 && j == 0 */
      {
        dist[0] = d;
      }
    }
  }
  return dist[(count1 - 1)%2 * count2 + count2 - 1];
}

/**
 * Linear space computation of the similarity distance between two temporal
 * values. Only two rows of the full matrix are used.
 *
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
double
tinstantarr_similarity(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc)
{
  /* Allocate memory for two rows of the distance matrix */
  double *dist = (double *) palloc(sizeof(double) * 2 * count2);
  /* Initialise it with -1.0 */
  for (int i = 0; i < 2 * count2; i++)
    *(dist + i) = -1.0;
  /* Call the linear_space computation of the similarity distance */
  double result = tinstantarr_similarity1(dist, instants1, count1, instants2,
    count2, simfunc);
  /* Free memory */
  pfree(dist);
  return result;
}

/**
 * Compute the similarity distance between two temporal values
 * (internal function).
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
double
temporal_similarity_internal(Temporal *temp1, Temporal *temp2, SimFunc simfunc)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = count1 > count2 ?
    tinstantarr_similarity(instants1, count1, instants2, count2, simfunc) :
    tinstantarr_similarity(instants2, count2, instants1, count1, simfunc);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

/*****************************************************************************
 * Linear space computation of the similarity distance
 *****************************************************************************/

Datum
temporal_similarity(FunctionCallInfo fcinfo, SimFunc simfunc)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable for temporal geographic points */
  if (temp1->basetypid == type_oid(T_GEOGRAPHY))
    store_fcinfo(fcinfo);
  double result = temporal_similarity_internal(temp1, temp2, simfunc);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(temporal_frechet_distance);
/**
 * Compute the discrete Frechet distance between two temporal values.
 */
Datum
temporal_frechet_distance(PG_FUNCTION_ARGS)
{
  return temporal_similarity(fcinfo, FRECHET);
}

PG_FUNCTION_INFO_V1(temporal_dynamic_time_warp);
/**
 * Compute the Dynamic Time Match (DTW) distance between two temporal values.
 */
Datum
temporal_dynamic_time_warp(PG_FUNCTION_ARGS)
{
  return temporal_similarity(fcinfo, DYNTIMEWARP);
}

/*****************************************************************************
 * Iterative implementation of the similarity distance with a full matrix
 *****************************************************************************/

#ifdef DEBUG_BUILD
/**
 * Print a distance matrix in tabular form
 */
void
matrix_print(double *dist, int count1, int count2)
{
  int len = 0;
  char buf[65536];
  int i, j;
  len += sprintf(buf+len, "\n      ");
  for (j = 0; j < count2; j++)
    len += sprintf(buf+len, "    %2d    ", j);
  len += sprintf(buf+len, "\n");
  for (j = 0; j < count2; j++)
    len += sprintf(buf+len, "------------");
  len += sprintf(buf+len, "\n");
  for (i = 0; i < count1; i++)
  {
    len += sprintf(buf+len, "%2d | ", i);
    for (j = 0; j < count2; j++)
      len += sprintf(buf+len, " %9.3f", dist[i * count2 + j]);
    len += sprintf(buf+len, "\n");
  }
  for (j = 0; j < count2; j++)
    len += sprintf(buf+len, "------------");
  len += sprintf(buf+len, "\n      ");
  for (j = 0; j < count2; j++)
    len += sprintf(buf+len, "    %2d    ", j);
  len += sprintf(buf+len, "\n");
  ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("MATRIX:\n%s", buf)));
  return;
}

/**
 * Print a distant path found from the distance matrix
 */
void
path_print(Match *path, int count)
{
  int len = 0;
  char buf[65536];
  int i, k = 0;
  for (i = count - 1; i >= 0; i--)
    len += sprintf(buf+len, "%d: (%2d,%2d)\n", k++, path[i].i, path[i].j);
  ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("PATH:\n%s", buf)));
  return;
}
#endif

/**
 * Compute the similarity path between two temporal values based on the 
 * distance matrix.
 *
 * @param[in] dist Matrix keeping the distances
 * @param[in] count1,count2 Number of rows and columns of the matrix
 * @param[out] count Number of elements of the similarity path
 */
static Match *
tinstantarr_similarity_path(double *dist, int count1, int count2, int *count)
{
  Match *result = palloc(sizeof(Match) * (count1 + count2));
  int i = count1 - 1;
  int j = count2 - 1;
  int k = 0;
  while (true)
  {
    result[k].i = i;
    result[k++].j = j;
    if (i == 0 && j == 0)
      break;
    if (i > 0 && j > 0)
    {
      /* Compute the minimum distance of the 3 neighboring cells */
      double d = Min(dist[(i - 1) * count2 + j - 1],
        Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
      /* We prioritize the diagonal in case of ties */
      if (dist[(i - 1) * count2 + j - 1] == d)
      {
        i--; j--;
      }
      else if (dist[(i - 1) * count2 + j] == d)
        i--;
      else /* (dist[(i) * count2 + j - 1] == d) */
        j--;
    }
    else if (i > 0)
      i--;
    else /* j > 0 */
      j--;
  }
  *count = k;
  return result;
}

/**
 * Computing the similarity distance between two temporal values using a 
 * full matrix.
 *
 * @param[out] dist Matrix keeping the distances
 * @param[in] instants1,instants2 Instants of the temporal values
 * @param[in] count1,count2 Number of instants of the temporal values
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
static void
tinstantarr_similarity_matrix1(double *dist, const TInstant **instants1,
  int count1, const TInstant **instants2, int count2, SimFunc simfunc)
{
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      const TInstant *inst1 = instants1[i];
      const TInstant *inst2 = instants2[j];
      double d = tinstant_distance(inst1, inst2);
      if (i > 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i * count2 + j] = Max(d, 
            Min(dist[(i - 1) * count2 + j - 1],
              Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1])));
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i * count2 + j] = d +
            Min(dist[(i - 1) * count2 + j - 1],
              Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
        }
      }
      else if (i > 0 && j == 0)
      {
        if (simfunc == FRECHET)
        {
          dist[i * count2] = Max(d, dist[(i - 1) * count2]);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[i * count2] = d + dist[(i - 1) * count2];
        }
      }
      else if (i == 0 && j > 0)
      {
        if (simfunc == FRECHET)
        {
          dist[j] = Max(dist[j - 1], d);
        }
        else /* simfunc == DYNTIMEWARP */
        {
          dist[j] = d + dist[j - 1];
        }
      }
      else /* i == 0 && j == 0 */
      {
        dist[0] = d;
      }
    }
  }
  return;
}

/**
 * Computes the similarity distance between two temporal values.
 *
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
Match *
tinstantarr_similarity_matrix(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, int *count, SimFunc simfunc)
{
  /* Allocate memory for dist */
  double *dist = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int i = 0; i < count1 * count2; i++)
    *(dist + i) = -1.0;
  /* Call the iterative computation of the similarity distance */
  tinstantarr_similarity_matrix1(dist, instants1, count1, instants2, count2,
    simfunc);
  /* Compute the path */
  Match *result = tinstantarr_similarity_path(dist, count1, count2, count);
  /* Free memory */
  pfree(dist);
  return result;
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

/*****************************************************************************/

/**
 * Compute the similarity path between two temporal values (internal function)
 */
Match *
temporal_similarity_path_internal(Temporal *temp1, Temporal *temp2, int *count,
  SimFunc simfunc)
{
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  Match *result = count1 > count2 ?
    tinstantarr_similarity_matrix(instants1, count1, instants2, count2, count, simfunc) :
    tinstantarr_similarity_matrix(instants2, count2, instants1, count1, count, simfunc);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

/*****************************************************************************
 * Quadratic space computation of the similarity path
 *****************************************************************************/

/**
 * Compute the Dynamic Time Match (DTW) path between two temporal values.
 */
Datum
temporal_similarity_path(FunctionCallInfo fcinfo, SimFunc simfunc)
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
    Temporal *temp1 = PG_GETARG_TEMPORAL(0);
    Temporal *temp2 = PG_GETARG_TEMPORAL(1);
    /* Store fcinfo into a global variable for temporal geographic points */
    if (temp1->basetypid == type_oid(T_GEOGRAPHY))
      store_fcinfo(fcinfo);
    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Compute the path */
    int count;
    Match *path = temporal_similarity_path_internal(temp1, temp2, &count,
      simfunc);
    /* Create function state */
    funcctx->user_fctx = similarity_path_state_make(path, count);
    /* Build a tuple description for the function output */
    get_call_result_type(fcinfo, 0, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
    // PG_FREE_IF_COPY(temp1, 0);
    // PG_FREE_IF_COPY(temp2, 1);
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

/*****************************************************************************
 * Quadratic space computation of the discrete Frechet path
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_frechet_path);
/**
 * Compute the Frechet path between two temporal values.
 */
Datum
temporal_frechet_path(PG_FUNCTION_ARGS)
{
  return temporal_similarity_path(fcinfo, FRECHET);
}

/*****************************************************************************
 * Quadratic space computation of the Dynamic Time Match (DTW) path
 *****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_dynamic_time_warp_path);
/**
 * Compute the Dynamic Time Warp (DTW) path between two temporal values.
 */
Datum
temporal_dynamic_time_warp_path(PG_FUNCTION_ARGS)
{
  return temporal_similarity_path(fcinfo, DYNTIMEWARP);
}

/*****************************************************************************/
