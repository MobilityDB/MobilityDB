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
 * @file tpoint_discfrechet.c
 * Compute the discrete Frechet distance for temporal points.
 */

#include "point/tpoint_discfrechet.h"

#include <postgres.h>
#include <assert.h>
#include <funcapi.h>
#include <math.h>
#include <utils/float.h>
#if POSTGRESQL_VERSION_NUMBER < 120000
#include <access/htup_details.h>
#endif

#include <liblwgeom.h>

#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Compute the distance between two instants
 *****************************************************************************/

/**
 * Compute the distance between two temporal instants.
 *
 * @param[in] inst1, inst2 Temporal instants
 */
static double
tpointinst_distance(const TInstant *inst1, const TInstant *inst2)
{
  datum_func2 func = get_pt_distance_fn(inst1->flags);
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  double result = func(value1, value2);
  return result;
}

/**
 * Compute the distance between two temporal instants.
 *
 * @param[in] inst1, inst2 Temporal instants
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
 * @param[in] inst1, inst2 Temporal instants
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
 * Linear space implementation of the discrete Frechet distance
 *****************************************************************************/

/**
 * Linear space function computing the discrete Frechet distance between
 * two temporal points. Only two rows of the full matrix are used.
 *
 * @param[in] instants1, instants2 Temporal instants
 * @param[in] count1, count2 Number of instants
 * @param[out] dist Array keeping the distances
 */
static double
tpointseq_discfrechet1(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, double *dist)
{
  const TInstant *inst1, *inst2;
  double d;
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      inst1 = instants1[i];
      inst2 = instants2[j];
      d = tpointinst_distance(inst1, inst2);
      if (i > 0 && j > 0)
      {
        dist[i%2 * count2 + j] = Max(
          Min(dist[(i - 1)%2 * count2 + j - 1],
            Min(dist[(i - 1)%2 * count2 + j], dist[i%2 * count2 + j - 1])),
          d);
      }
      else if (i > 0 && j == 0)
        dist[i%2 * count2] = Max(dist[(i - 1)%2 * count2], d);
      else if (i == 0 && j > 0)
        dist[j] = Max(dist[j - 1], d);
      else /* i == 0 && j == 0 */
        dist[0] = d;
    }
  }
  return dist[(count1 - 1)%2 * count2 + count2 - 1];
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] instants1, instants2 Temporal instants
 * @param[in] count1, count2 Number of instants
 */
double
tpointinstarr_discfrechet(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Allocate memory for two rows of the distance matrix */
  double *dist = (double *) palloc(sizeof(double) * 2 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < 2 * count2; k++)
    *(dist + k) = -1.0;

  /* Call the linear_space computation of the discrete Frechet distance */
  double result = tpointseq_discfrechet1(instants1, count1, instants2, count2, dist);
  /* Free memory */
  pfree(dist);

  return result;
}

/*****************************************************************************/

double
tpoint_discfrechet_internal(Temporal *temp1, Temporal *temp2)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = count1 > count2 ?
    tpointinstarr_discfrechet(instants1, count1, instants2, count2) :
    tpointinstarr_discfrechet(instants2, count2, instants1, count1);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_discfrechet);
/**
 * Computes the discrete Frechet distance between two temporal points.
 */
Datum
tpoint_discfrechet(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_discfrechet_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Linear space implementation of the Dynamic Time Warp (DTW) distance
 *****************************************************************************/

/**
 * Linear space function computing the Dynamic Time Warp (DTW) distance
 * between two temporal points. Only two rows of the full matrix are used.
 *
 * @param[in] instants1, instants2 Temporal instants
 * @param[in] count1, count2 Number of instants
 * @param[in] dist Array keeping the distances
 */
static double
tinstantarr_dtw1(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, double *dist)
{
  const TInstant *inst1, *inst2;
  double d;
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      inst1 = instants1[i];
      inst2 = instants2[j];
      d = tinstant_distance(inst1, inst2);
      if (i > 0 && j > 0)
      {
        dist[i%2 * count2 + j] = d +
          Min(dist[(i - 1)%2 * count2 + j - 1],
            Min(dist[(i - 1)%2 * count2 + j], dist[i%2 * count2 + j - 1]));
      }
      else if (i > 0 && j == 0)
        dist[i%2 * count2] = d + dist[(i - 1)%2 * count2];
      else if (i == 0 && j > 0)
        dist[j] = d + dist[j - 1];
      else /* i == 0 && j == 0 */
        dist[0] = d;
    }
  }
  return dist[(count1 - 1)%2 * count2 + count2 - 1];
}

/**
 * Computes the the Dynamic Time Warp (DTW) distance for two temporal sequence points.
 *
 * @param[in] instants1, instants2 Temporal instants
 * @param[in] count1, count2 Number of instants
 */
double
tinstantarr_dtw(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Allocate memory for dist */
  double *dist = (double *) palloc(sizeof(double) * 2 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < 2 * count2; k++)
    *(dist + k) = -1.0;

  /* Call the linear_space computation of the the Dynamic Time Warp (DTW) distance */
  double result = tinstantarr_dtw1(instants1, count1, instants2, count2, dist);
  /* Free memory */
  pfree(dist);

  return result;
}

/**
 * Computes the Dynamic Time Warp (DTW) distance between two temporal points.
 * (internal function)
 */
double
temporal_dtw_internal(Temporal *temp1, Temporal *temp2)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = count1 > count2 ?
    tinstantarr_dtw(instants1, count1, instants2, count2) :
    tinstantarr_dtw(instants2, count2, instants1, count1);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_dtw);
/**
 * Computes the Dynamic Time Warp (DTW) distance between two temporal points.
 */
Datum
temporal_dtw(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = temporal_dtw_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Iterative implementation of the dynamic time warp distance
 *****************************************************************************/

#ifdef DEBUG_BUILD
void
matrix_print(double *dist, int count1, int count2)
{
  int len = 0;
  char buf[65536];
  int i, j;
  len += sprintf(buf+len, "\n    ");
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
      len += sprintf(buf+len, " %lf ", dist[i * count2 + j]);
    len += sprintf(buf+len, "\n");
  }
  for (j = 0; j < count2; j++)
    len += sprintf(buf+len, "------------");
  len += sprintf(buf+len, "\n    ");
  for (j = 0; j < count2; j++)
    len += sprintf(buf+len, "    %2d    ", j);
  len += sprintf(buf+len, "\n");
  ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("MATRIX:\n%s", buf)));
  return;
}
#endif

/**
 * Function computing the dynamic time warp path between
 * two temporal points.
 *
 * @param[in] count1,count2 Number of instants of the temporal points
 * @param[in] dist Array keeping the distances
 * @param[out] count Number of elements of the warp path
 */
static Warp *
tinstantarr_dtw_path(int count1, int count2, double *dist, int *count)
{
  Warp *result = palloc(sizeof(Warp) * Max(count1, count2));
  int i = count1 - 1;
  int j = count2 - 1;
  int k = 0;
  while (true)
  {
    result[k].i = i;
    result[k++].j = j;
    if (i == 0 && j == 0)
      break;
    /* Compute the minimum distance of the 3 neighboring cells */
    double d = Min(dist[(i - 1) * count2 + j - 1],
      Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
    /* We prioritize the diagonal in case of ties */
    if (i > 0 && j > 0 && dist[(i - 1) * count2 + j - 1] == d)
    {
      i--; j--;
    }
    else if (i > 0 && dist[(i - 1) * count2 + j] == d)
      i--;
    else /* (j > 0 && dist[(i) * count2 + j - 1] == d) */
      j--;
  }
  *count = k;
  return result;
}

/**
 * Iterative function computing the dynamic time warp distance between
 * two temporal points.
 *
 * @param[in] instants1, instants2 Instants of the temporal points
 * @param[in] count1,count2 Number of instants of the temporal points
 * @param[in] dist Array keeping the distances
 */
static void
tinstantarr_dtw_iterative1(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, double *dist)
{
  const TInstant *inst1, *inst2;
  double d;
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      inst1 = instants1[i];
      inst2 = instants2[j];
      d = tinstant_distance(inst1, inst2);
      if (i > 0 && j > 0)
      {
        dist[i * count2 + j] = d +
          Min(dist[(i - 1) * count2 + j - 1],
            Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
      }
      else if (i > 0 && j == 0)
        dist[i * count2] = d + dist[(i - 1) * count2];
      else if (i == 0 && j > 0)
        dist[j] = d + dist[j - 1];
      else /* i == 0 && j == 0 */
        dist[0] = d;
    }
  }
  return;
}

/**
 * Computes the dynamic time warp distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
Warp *
tinstantarr_dtw_iterative(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, int *count)
{
  /* Allocate memory for dist */
  double *dist = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < count1 * count2; k++)
    *(dist + k) = -1.0;

  /* Call the iterative computation of the discrete Frechet distance */
  tinstantarr_dtw_iterative1(instants1, count1, instants2, count2, dist);
    
  /* Compute the path */
  Warp *result = tinstantarr_dtw_path(count1, count2, dist, count);

  /* Free memory */
  pfree(dist);

  return result;
}

/*****************************************************************************
 * Compute the Dynamic Time Warp Path between two temporal points
 *****************************************************************************/

/**
 * Create the initial state that persists across multiple calls of the function
 *
 * @param[in] path Warp path
 * @param[in] r Bounds for generating the bucket list
 * @param[in] size Size of the path
 *
 * @pre The size argument must be greater to 0.
 * @note The path is in reverse order and thus, we start from the last element
 */
static WarpPathState *
warp_path_state_make(Warp *path, int size)
{
  WarpPathState *state = palloc0(sizeof(WarpPathState));
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
warp_path_state_next(WarpPathState *state)
{
  if (!state || state->done)
    return;
  /* Move to the next warp */
  state->i--;
  if (state->i == 0)
    state->done = true;
  return;
}

/*****************************************************************************/


/**
 * Computes the Dynamic Time Warp (DTW) distance between two temporal points.
 * (internal function)
 */
Warp *
temporal_dtw_path_internal(Temporal *temp1, Temporal *temp2, int *count)
{
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  Warp *result = count1 > count2 ?
    tinstantarr_dtw_iterative(instants1, count1, instants2, count2, count) :
    tinstantarr_dtw_iterative(instants2, count2, instants1, count1, count);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

PG_FUNCTION_INFO_V1(temporal_dtw_path);
/**
 * Computes the Dynamic Time Warp (DTW) path between two temporal points.
 */
Datum
temporal_dtw_path(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  WarpPathState *state;
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

    /* Compute the path */
    int count;
    Warp *path = temporal_dtw_path_internal(temp1, temp2, &count);

    /* Initialize the FuncCallContext */
    funcctx = SRF_FIRSTCALL_INIT();
    /* Switch to memory context appropriate for multiple function calls */
    MemoryContext oldcontext = 
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    /* Create function state */
    funcctx->user_fctx = warp_path_state_make(path, count);
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
    SRF_RETURN_DONE(funcctx);

  /* Store index */
  tuple_arr[0] = Int32GetDatum(state->path[state->i].i);
  tuple_arr[1] = Int32GetDatum(state->path[state->i].j);
  /* Advance state */
  warp_path_state_next(state);
  /* Form tuple and return */
  tuple = heap_form_tuple(funcctx->tuple_desc, tuple_arr, isnull);
  result = HeapTupleGetDatum(tuple);
  SRF_RETURN_NEXT(funcctx, result);
}

/*****************************************************************************/
