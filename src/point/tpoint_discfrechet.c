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

#include <postgres.h>
#include <assert.h>
#include <math.h>
#include <utils/float.h>

#include <liblwgeom.h>

#include "general/uthash.h"
#include "general/temporaltypes.h"
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
 * @param[in] seq1, seq2 Temporal points
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
 * @param[in] seq1, seq2 Temporal sequences
 * @param[in] i,j Indexes of the instants
 * @param[in] dist Array keeping the distances
 */
static double
tpointinstarr_dtw1(const TInstant **instants1, int count1,
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
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointinstarr_dtw(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Allocate memory for dist */
  double *dist = (double *) palloc(sizeof(double) * 2 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < 2 * count2; k++)
    *(dist + k) = -1.0;

  /* Call the linear_space computation of the the Dynamic Time Warp (DTW) distance */
  double result = tpointinstarr_dtw1(instants1, count1, instants2, count2, dist);
  /* Free memory */
  pfree(dist);

  return result;
}

/**
 * Computes the Dynamic Time Warp (DTW) distance between two temporal points.
 * (internal function)
 */
double
tpoint_dtw_internal(Temporal *temp1, Temporal *temp2)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = count1 > count2 ?
    tpointinstarr_dtw(instants1, count1, instants2, count2) :
    tpointinstarr_dtw(instants2, count2, instants1, count1);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_dtw);
/**
 * Computes the Dynamic Time Warp (DTW) distance between two temporal points.
 */
Datum
tpoint_dtw(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_dtw_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/
