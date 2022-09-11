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
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporaltypes.h"
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
  double result = fabs(tnumberinst_double(inst1) - tnumberinst_double(inst2));
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
  datum_func2 func = pt_distance_fn(inst1->flags);
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
  if (tnumber_type(inst1->temptype))
    return tnumberinst_distance(inst1, inst2);
  if (tgeo_type(inst1->temptype))
    return tpointinst_distance(inst1, inst2);
  elog(ERROR, "Unexpected temporal type: inst1->temptype");
}

/*****************************************************************************
 * Linear space computation of the similarity distance
 *****************************************************************************/

/**
 * Linear space computation of the similarity distance between two temporal
 * values. Only two rows of the full matrix are used.
 *
 * @param[in] instants1,instants2 Arrays of temporal instants
 * @param[in] count1,count2 Number of instants in the arrays
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 * @param[out] dist Array keeping the distances
 */
static double
tinstarr_similarity1(double *dist, const TInstant **instants1, int count1,
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
tinstarr_similarity(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc)
{
  /* Allocate memory for two rows of the distance matrix */
  double *dist = palloc(sizeof(double) * 2 * count2);
  /* Initialise it with -1.0 */
  for (int i = 0; i < 2 * count2; i++)
    *(dist + i) = -1.0;
  /* Call the linear_space computation of the similarity distance */
  double result = tinstarr_similarity1(dist, instants1, count1, instants2,
    count2, simfunc);
  /* Free memory */
  pfree(dist);
  return result;
}

/**
 * @brief Compute the similarity distance between two temporal values.
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 */
double
temporal_similarity(const Temporal *temp1, const Temporal *temp2,
  SimFunc simfunc)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants(temp1, &count1);
  const TInstant **instants2 = temporal_instants(temp2, &count2);
  result = count1 > count2 ?
    tinstarr_similarity(instants1, count1, instants2, count2, simfunc) :
    tinstarr_similarity(instants2, count2, instants1, count1, simfunc);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_similarity
 * @brief Compute the Frechet distance between two temporal values.
 *
 * @param[in] temp1,temp2 Temporal values
 * @sqlfunc frechetDistance()
 */
double
temporal_frechet_distance(const Temporal *temp1, const Temporal *temp2)
{
  return temporal_similarity(temp1, temp2, FRECHET);
}

/**
 * @ingroup libmeos_temporal_similarity
 * @brief Compute the Dynamic Time Warp distance between two temporal values.
 *
 * @param[in] temp1,temp2 Temporal values
 * @sqlfunc dynamicTimeWarp()
 */
double
temporal_dyntimewarp_distance(const Temporal *temp1, const Temporal *temp2)
{
  return temporal_similarity(temp1, temp2, DYNTIMEWARP);
}
#endif


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
  sprintf(buf+len, "\n"); /* make Codacy quiet by removing last assignment */
  elog(WARNING, "MATRIX:\n%s", buf);
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
  elog(WARNING, "PATH:\n%s", buf);
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
tinstarr_similarity_path(double *dist, int count1, int count2, int *count)
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
 * @param[in] instants1,instants2 Instants of the temporal values
 * @param[in] count1,count2 Number of instants of the temporal values
 * @param[in] simfunc Similarity function, i.e., Frechet or DTW
 * @param[out] dist Matrix keeping the distances
 */
static void
tinstarr_similarity_matrix1(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc, double *dist)
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
 * @param[out] count Number of elements in the resulting array
 */
Match *
tinstarr_similarity_matrix(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, SimFunc simfunc, int *count)
{
  /* Allocate memory for dist */
  double *dist = palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int i = 0; i < count1 * count2; i++)
    *(dist + i) = -1.0;
  /* Call the iterative computation of the similarity distance */
  tinstarr_similarity_matrix1(instants1, count1, instants2, count2, simfunc,
    dist);
  /* Compute the path */
  Match *result = tinstarr_similarity_path(dist, count1, count2, count);
  /* Free memory */
  pfree(dist);
  return result;
}

/*****************************************************************************
 * Quadratic space computation of the similarity path
 *****************************************************************************/

/**
 * @brief Compute the similarity path between two temporal values
 */
Match *
temporal_similarity_path(const Temporal *temp1, const Temporal *temp2,
  int *count, SimFunc simfunc)
{
  int count1, count2;
  const TInstant **instants1 = temporal_instants(temp1, &count1);
  const TInstant **instants2 = temporal_instants(temp2, &count2);
  Match *result = count1 > count2 ?
    tinstarr_similarity_matrix(instants1, count1, instants2, count2,
      simfunc, count) :
    tinstarr_similarity_matrix(instants2, count2, instants1, count1,
      simfunc, count);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_temporal_similarity
 * @brief Compute the Frechet distance between two temporal values.
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[out] count Number of elements of the output array
 * @sqlfunc frechetDistancePath()
 */
Match *
temporal_frechet_path(const Temporal *temp1, const Temporal *temp2, int *count)
{
  return temporal_similarity_path(temp1, temp2, count, FRECHET);
}

/**
 * @ingroup libmeos_temporal_similarity
 * @brief Compute the Dynamic Time Warp distance between two temporal values.
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[out] count Number of elements of the output array
 * @sqlfunc dynamicTimeWarpPath()
 */
Match *
temporal_dyntimewarp_path(const Temporal *temp1, const Temporal *temp2, int *count)
{
  return temporal_similarity_path(temp1, temp2, count, DYNTIMEWARP);
}
#endif

/*****************************************************************************/
