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
#include "general/temporal.h"
#include "general/tinstant.h"
#include "general/tsequence.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Hash structures for storing a sparse matrix
 *****************************************************************************/

typedef struct {
  int i;
  int j;
} indices_t;

typedef struct {
  indices_t key;      /* key */
  double distance;
  UT_hash_handle hh;  /* makes this structure hashable */
} distance_t;

distance_t *distances = NULL;

void add_distance(int i, int j, double distance)
{
  distance_t *s;
  indices_t key;
  key.i = i; key.j = j;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* (i,j) already in the hash? */
  if (s == NULL)
  {
    s = (distance_t *) malloc(sizeof *s);
    s->key.i = i; s->key.j = j;
    HASH_ADD(hh, distances, key, sizeof(indices_t), s);   /* key: name of key field */
  }
  s->distance = distance;
}

distance_t *find_distance(int i, int j)
{
  distance_t *s;
  indices_t key;
  key.i = i; key.j = j;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
  return s;
}

void delete_distance(distance_t *distance)
{
  HASH_DEL(distances, distance);  /* distance: pointer to deletee */
  free(distance);
}

void delete_all()
{
  distance_t *current_distance, *tmp;
  HASH_ITER(hh, distances, current_distance, tmp)
  {
    HASH_DEL(distances, current_distance);  /* delete it (distances advances to next) */
    free(current_distance);                 /* free it */
  }
}

void print_distances()
{
  distance_t *s;
  for (s = distances; s != NULL; s = (distance_t *)(s->hh.next))
  {
    printf("key: (%d,%d), distance: %lf\n", s->key.i, s->key.j, s->distance);
  }
}

int distance_sort(distance_t *a, distance_t *b)
{
  return (a->distance > b->distance) - (a->distance < b->distance);
}

int key_sort(distance_t *a, distance_t *b)
{
  int cmp = (a->key.i - b->key.i);
  return cmp ? cmp : (a->key.j - b->key.j);
}

void sort_by_distance()
{
  HASH_SORT(distances, distance_sort);
}

void sort_by_key()
{
  HASH_SORT(distances, key_sort);
}

/*****************************************************************************
 * Bresenham's line algorithm
 *****************************************************************************/

#define NDIMS 2

int *
bresenham(int n1, int n2, int *count)
{
  assert(n1 > 0);
  assert(n2 > 0);
  /* Allocate array for maximum number of elements in the result
   * max(n1, n2) * number of dimensions */
  int max = Max(n1, n2);
  int min = Min(n1, n2);
  int maxelem = max * NDIMS;
  int *result = malloc(sizeof(int) * maxelem);
  int x = 0, y = 0, j = 0;
  int err = n1 > n2 ? n1 / 2: n2 / 2;
  for (int i = 0; i < max; i++)
  {
    /* Store the element of the answer */
    result[NDIMS * j] = x;
    result[NDIMS * j + 1] = y;
    j++;
    if (j == maxelem)
    {
      printf("ERROR: Maximum number of elements reached\n");
      exit(-1);
    }
    err -= min;
    if (err < 0)
    {
      if (n1 > n2)
        y++;
      else
        x++;
      err += max;
    }
    if (n1 > n2)
      x++;
    else
      y++;
  }
  *count = j;
  return result;
}

/*****************************************************************************
 * Recursive implementation of the discrete Frechet distance
 *****************************************************************************/

/**
 * Compute the distance between two temporal instants.
 *
 * @param[in] inst1, inst2 Temporal instants
 * @param[in] func Distance function
 */
static double
tpointinst_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  return DatumGetFloat8(func(value1, value2));
}

/**
 * Recursive function computing the discrete Frechet distance between
 * two temporal points.
 *
 * @param[in] seq1, seq2 Temporal sequences
 * @param[in] i,j Indexes of the instants
 * @param[in] hasz True if the temporal points have Z coordinates
 * @param[in] ca Array keeping the distances
 */
static double
tpointseq_dfd_rec1(const TSequence *seq1, const TSequence *seq2, int i, int j,
  datum_func2 func, double *ca)
{
  const TInstant *inst1, *inst2;
  double dist, *ca_ij;

  /*
  * Target the shortcut to the (i, j)-th entry of the matrix ca
  *
  * Once again, notice the 1-offset.
  */
  ca_ij = ca + i * seq2->count + j;

  /* This implements the algorithm from [1] */
  if (*ca_ij > -1.0)
  {
    return *ca_ij;
  }
  else if (i == 0 && j == 0)
  {
    inst1 = tsequence_inst_n(seq1, 0);
    inst2 = tsequence_inst_n(seq2, 0);
    *ca_ij = tpointinst_distance(inst1, inst2, func);
  }
  else if (i > 0 && j == 0)
  {
    inst1 = tsequence_inst_n(seq1, i - 1);
    inst2 = tsequence_inst_n(seq2, 0);
    dist = tpointinst_distance(inst1, inst2, func);
    *ca_ij = fmax(tpointseq_dfd_rec1(seq1, seq2, i - 1, 1, func, ca), dist);
  }
  else if (i == 0 && j > 0)
  {
    inst1 = tsequence_inst_n(seq1, 0);
    inst2 = tsequence_inst_n(seq2, j - 1);
    dist = tpointinst_distance(inst1, inst2, func);
    *ca_ij = fmax(tpointseq_dfd_rec1(seq1, seq2, 1, j - 1, func, ca), dist);
  }
  else if (i > 0 && j > 0)
  {
    inst1 = tsequence_inst_n(seq1, i - 1);
    inst2 = tsequence_inst_n(seq2, j - 1);
    dist = tpointinst_distance(inst1, inst2, func);
    *ca_ij =
      fmax(
        fmin(
          fmin(
             tpointseq_dfd_rec1(seq1, seq2, i - 1, j, func, ca),
             tpointseq_dfd_rec1(seq1, seq2, i - 1, j - 1, func, ca)),
           tpointseq_dfd_rec1(seq1, seq2, i, j - 1, func, ca)),
        dist);
  }
  else
  {
    *ca_ij = get_float8_infinity();
  }

  return *ca_ij;
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointseq_dfd_rec(const TSequence *seq1, const TSequence *seq2)
{
  int n1 = seq1->count;
  int n2 = seq2->count;
  datum_func2 func = get_distance_fn(seq1->flags);

  /* Allocate memory for ca */
  double *ca = (double *) palloc(sizeof(double) * n1 * n2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < n1 * n2; k++)
    *(ca + k) = -1.0;

  /* Call the recursive computation of the discrete Frechet distance */
  double result = tpointseq_dfd_rec1(seq1, seq2, n1 - 1, n2 - 1, func, ca);
  /* Free memory */
  pfree(ca);

  return result;
}

double
tpoint_dfd_rec_internal(Temporal *temp1, Temporal *temp2)
{
  double result;
  ensure_valid_tempsubtype(temp1->subtype);
  if (temp1->subtype == INSTANT)
    result = 0.0;
  else if (temp1->subtype == INSTANTSET)
    result = 0.0;
  if (! MOBDB_FLAGS_GET_LINEAR(temp1->flags))
    result = 0.0;
  else if (temp1->subtype == SEQUENCE)
    result = tpointseq_dfd_rec((TSequence *) temp1, (TSequence *) temp2);
  else /* temp1->subtype == SEQUENCESET */
    result = 0.0;
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_dfd_rec);
/**
 * Computes the discrete Frechet distance between two temporal points.
 */
Datum
tpoint_dfd_rec(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_dfd_rec_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/
