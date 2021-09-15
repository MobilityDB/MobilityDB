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
 * Based on https://towardsdatascience.com/fast-discrete-fr%C3%A9chet-distance-d6b422a8fb77
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

void delete_all_distances()
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
 * Based on https://gist.github.com/bert/1085538
 *****************************************************************************/

#define NDIMS 2

int *
bresenham(int x0, int y0, int x1, int y1, int *count)
{
  int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
  int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
  int err = dx + dy, e2; /* error value e_xy */

  /* Allocate array for maximum number of elements in the result
   * max(count1, count2) * number of dimensions */
  int max = Max(dx + 1, -dy + 1);
  int maxelem = max * NDIMS;
  int *result = malloc(sizeof(int) * maxelem);
  int j = 0;
  for (;;)
  {
    /* Store the element of the answer */
    result[NDIMS * j] = x0;
    result[NDIMS * j + 1] = y0;
    j++;
    if (x0 == x1 && y0 == y1)
      break;
    e2 = 2 * err;
    if (e2 >= dy)
    {
      err += dy;
      x0 += sx;
    } /* e_xy+e_x > 0 */
    if (e2 <= dx) 
    {
      err += dx;
      y0 += sy; 
    } /* e_xy+e_y < 0 */
  }
  *count = j;
  return result;
}

/*****************************************************************************
 * Compute the distance between two points
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

/*****************************************************************************
 * Recursive implementation of the discrete Frechet distance
 *****************************************************************************/

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
  double dist,
    *ca_ij; /* Pointer to ca(i, j), just to simplify notation */

  /*
  * Target the shortcut to the (i, j)-th entry of the matrix ca
  *
  * Once again, notice the 1-offset.
  */
  ca_ij = ca + (i - 1) * seq2->count + (j - 1);

  /* This implements the algorithm from [1] */
  if (*ca_ij > -1.0)
  {
    return *ca_ij;
  }
  else if ((i == 1) && (j == 1))
  {
    inst1 = tsequence_inst_n(seq1, 0);
    inst2 = tsequence_inst_n(seq2, 0);
    *ca_ij = tpointinst_distance(inst1, inst2, func);
  }
  else if ((i > 1) && (j == 1))
  {
    inst1 = tsequence_inst_n(seq1, i - 1);
    inst2 = tsequence_inst_n(seq2, 0);
    dist = tpointinst_distance(inst1, inst2, func);
    *ca_ij = fmax(tpointseq_dfd_rec1(seq1, seq2, i - 1, 1, func, ca), dist);
  }
  else if ((i == 1) && (j > 1))
  {
    inst1 = tsequence_inst_n(seq1, 0);
    inst2 = tsequence_inst_n(seq2, j - 1);
    dist = tpointinst_distance(inst1, inst2, func);
    *ca_ij = fmax(tpointseq_dfd_rec1(seq1, seq2, 1, j - 1, func, ca), dist);
  }
  else if ((i > 1) && (j > 1))
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
  int count1 = seq1->count;
  int count2 = seq2->count;
  datum_func2 func = get_distance_fn(seq1->flags);

  /* Allocate memory for ca */
  double *ca = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < count1 * count2; k++)
    *(ca + k) = -1.0;

  /* Call the recursive computation of the discrete Frechet distance */
  double result = tpointseq_dfd_rec1(seq1, seq2, count1, count2, func, ca);
  /* Free memory */
  pfree(ca);

  return result;
}

/*****************************************************************************
 * Linear implementation of the discrete Frechet distance
 *****************************************************************************/

/**
 * Linear function computing the discrete Frechet distance between
 * two temporal points.
 *
 * @param[in] seq1, seq2 Temporal sequences
 * @param[in] i,j Indexes of the instants
 * @param[in] hasz True if the temporal points have Z coordinates
 * @param[in] ca Array keeping the distances
 */
static double
tpointseq_dfd_linear1(const TSequence *seq1, const TSequence *seq2,
  datum_func2 func, double *ca)
{
  const TInstant *inst1, *inst2;
  double dist;
  int count1 = seq1->count;
  int count2 = seq2->count;
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      inst1 = tsequence_inst_n(seq1, i);
      inst2 = tsequence_inst_n(seq2, j);
      dist = tpointinst_distance(inst1, inst2, func);
      if (i > 0 && j > 0)
      {
        ca[j * count1 + i] = Max(
          Min(ca[j * count1 + i - 1], Min(ca[(j - 1) * count1 + i - 1], ca[(j - 1) * count1 + i])),
          dist);
      }
      else if (i > 0 && j == 0)
        ca[j * count1 + i] = Max(ca[i - 1], dist);
      else if (i == 0 && j > 0)
        ca[j * count1 + i] = Max(ca[(j - 1) * count1], dist);
      else if (i == 0 && j == 0)
        ca[0] = dist;
      else
        ca[j * count1 + i] = get_float8_infinity();
    }
  }
  return ca[(count2 - 1) * count1 + count1 - 1];
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointseq_dfd_linear(const TSequence *seq1, const TSequence *seq2)
{
  int count1 = seq1->count;
  int count2 = seq2->count;
  datum_func2 func = get_distance_fn(seq1->flags);

  /* Allocate memory for ca */
  double *ca = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < count1 * count2; k++)
    *(ca + k) = -1.0;

  /* Call the linear computation of the discrete Frechet distance */
  double result = tpointseq_dfd_linear1(seq1, seq2, func, ca);
  /* Free memory */
  pfree(ca);

  return result;
}

/*****************************************************************************
 * Matrix implementation of the fast discrete Frechet distance
 *****************************************************************************/

void
print_matrix(double *ca, int count1, int count2)
{
  int i, j;
  for (j = count2 - 1; j >= 0; j--)
  {
   printf("%2d | ", j);
   for (i = 0; i < count1; i++)
      printf("% lf ", ca[j * count1 + i]);
    printf("\n");
  }
  for (i = 0; i < count1; i++)
    printf("------------");
  printf("\n    ");
  for (i = 0; i < count1; i++)
    printf("    %2d    ", i);
  printf("\n");
  return;
}

void
tpointseq_dfd_fast1(double *dist, const TSequence *seq1, const TSequence *seq2,
  int *diag, int n_diag, datum_func2 func)
{
  double d, diag_max = -INFINITY;
  int i, j, k, i0, j0, i_min = 0, j_min = 0;
  int count1 = seq1->count;
  int count2 = seq2->count;
  const TInstant *inst1, *inst2, *inst;

  // Fill in the diagonal with the seed distance values
  // printf("Distances in the diagonale\n");
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = tsequence_inst_n(seq1, i0);
    inst2 = tsequence_inst_n(seq2, j0);
    d = tpointinst_distance(inst1, inst2, func);
    diag_max = Max(diag_max, d);
    dist[j0 * count1 + i0] = d;
    // printf("(%d,%d): %lf\n", i0, j0, d);
  }
  // Fill in the diagonal with the seed distance values
  // printf("Additional distances\n");
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = tsequence_inst_n(seq1, i0);
    inst2 = tsequence_inst_n(seq2, j0);
    for (i = i0 + 1; i < count1; i++)
    {
      if (dist[j0 * count1 + i] == INFINITY)
      {
        inst = tsequence_inst_n(seq1, i);
        d = tpointinst_distance(inst, inst2, func);
        if (d < diag_max || i < i_min)
        {
          dist[j0 * count1 + i] = d;
          // printf("(%d,%d): %lf\n", i, j0, d);
        }
        else
          break;
      }
      else
        break;
    }
    i_min = i;
    for (j = j0 + 1; j < count2; j++)
    {
      if (dist[j * count1 + i0] == INFINITY)
      {
        inst = tsequence_inst_n(seq2, j);
        d = tpointinst_distance(inst1, inst, func);
        if (d < diag_max || j < j_min)
        {
          dist[j * count1 + i0] = d;
          // printf("(%d,%d): %lf\n", i0, j, d);
        }
        else
          break;
      }
      else
        break;
    }
    j_min = j;
  }
  return;
}

double
get_corner_min_array(double *f_mat, int i, int j, int count1)
{
  double result;
  if (i > 0 && j > 0)
    result = Min(f_mat[(j - 1) * count1 + i - 1],
      Min(f_mat[(j - 1) * count1 + i], f_mat[j * count1 + i - 1]));
  else if (i == 0 && j == 0)
    result = f_mat[j * count1 + i];
  else if (i == 0)
    result = f_mat[(j - 1) * count1 + i];
  else  /* j == 0 */
    result = f_mat[j * count1 + i - 1];
  return result;
}

double
tpointseq_dfd_fast2(double *dist, const TSequence *seq1, const TSequence *seq2,
  int *diag, int n_diag)
{
  int count1 = seq1->count;
  int count2 = seq2->count;
  double c;
  for (int k = 0; k < n_diag; k++)
  {
    int i0 = diag[2 * k];
    int j0 = diag[2 * k + 1];
    // printf("i0,j0: (%d,%d)\n", i0, j0);
    for (int i = i0; i < count1; i++)
    {
      if (dist[j0 * count1 + i] == INFINITY)
        break;
      // printf("(1)--->(i: %d,j0: %d): ", i, j0);
      c = get_corner_min_array(dist, i, j0, count1);
      if (c > dist[j0 * count1 + i])
      {
        dist[j0 * count1 + i] = c;
        // printf("%lf\n", c);
      }
      // else
        // printf("\n");
    }
    /* Add 1 to j0 to avoid recalculating the diagonal */
    for (int j = j0 + 1; j < count2; j++)
    {
      if (dist[j * count1 + i0] == INFINITY)
        break;
      // printf("(2)--->(i0: %d,j: %d): ", i0, j);
      c = get_corner_min_array(dist, i0, j, count1);
      if (c > dist[j * count1 + i0])
      {
        dist[j * count1 + i0] = c;
        // printf("%lf\n", c);
      }
      // else
        // printf("\n");
    }
  }
  return dist[(count2 - 1) * count1 + (count1 - 1)];
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointseq_dfd_fast(const TSequence *seq1, const TSequence *seq2)
{
  int count1 = seq1->count;
  int count2 = seq2->count;
  datum_func2 func = get_distance_fn(seq1->flags);

  /* Allocate memory for ca */
  double *ca = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with infinity */
  for (int k = 0; k < count1 * count2; k++)
    *(ca + k) = INFINITY;
  /* Compute the diagonal */
  int n_diag;
  int *diag = bresenham(0, 0, count1 - 1, count2 - 1, &n_diag);
  /* Compute the distances in the diagonal */
  tpointseq_dfd_fast1(ca, seq1, seq2, diag, n_diag, func);
  /* Call the fast computation of the discrete Frechet distance */
  double result = tpointseq_dfd_fast2(ca, seq1, seq2, diag, n_diag);
  /* Free memory */
  pfree(ca);
  return result;
}

/*****************************************************************************
 * Sparse matrix implementation of the fast discrete Frechet distance
 *****************************************************************************/

double
get_distance(int i, int j)
{
  distance_t *s;
  indices_t key;
  key.i = i; key.j = j;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
  return s != NULL ? s->distance : INFINITY;
}

double
get_corner_min_sparse(int i, int j)
{
  double result;
  if (i > 0 && j > 0)
    result = Min(get_distance(i - 1, j - 1),
      Min(get_distance(i, j - 1), get_distance(i - 1, j)));
  else if (i == 0 && j == 0)
    result = get_distance(i, j);
  else if (i == 0)
    result = get_distance(i, j - 1);
  else  /* j == 0 */
    result = get_distance(i - 1, j);
  return result;
}

void
tpointseq_dfd_sparse1(const TSequence *seq1, const TSequence *seq2,
  int *diag, int n_diag, datum_func2 func)
{
  double d, diag_max = -INFINITY;
  int i, j, k, i0, j0, i_min = 0, j_min = 0;
  distance_t *s;
  indices_t key;
  int count1 = seq1->count;
  int count2 = seq2->count;
  const TInstant *inst1, *inst2, *inst;

  /* Fill in the diagonal with the seed distance values */
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = tsequence_inst_n(seq1, i0);
    inst2 = tsequence_inst_n(seq2, j0);
    d = tpointinst_distance(inst1, inst2, func);
    if (d > diag_max)
      diag_max = d;
    add_distance(i0, j0, d);
  }

  for (k = 0; k < n_diag - 1; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = tsequence_inst_n(seq1, i0);
    inst2 = tsequence_inst_n(seq2, j0);
    for (i = i0 + 1; i < count1; i++)
    {
      key.i = i; key.j = j0;
      HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
      if (s == NULL)
      {
        inst = tsequence_inst_n(seq1, i);
        d = tpointinst_distance(inst, inst2, func);
        if (d < diag_max || i < i_min)
          add_distance(i, j0, d);
        else
          break;
      }
      else
        break;
    }
    i_min = i;

    for (j = j0 + 1; j < count2; j++)
    {
      key.i = i0; key.j = j;
      HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
      if (s == NULL)
      {
        inst = tsequence_inst_n(seq2, j);
        d = tpointinst_distance(inst1, inst, func);
        if (d < diag_max || j < j_min)
          add_distance(i0, j, d);
        else
          break;
      }
      else
        break;
    }
    j_min = j;
  }
  /* Sort the distances by the indices */
  sort_by_key();
  return;
}

double
tpointseq_dfd_sparse2(const TSequence *seq1, const TSequence *seq2, int *diag,
  int n_diag)
{
  int i0, j0;
  int count1 = seq1->count;
  int count2 = seq2->count;
  double c;
  distance_t *s;
  indices_t key;
  for (int k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    for (int i = i0; i < count1; i++)
    {
      key.i = i; key.j = j0;
      HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
      if (s != NULL)
      {
        c = get_corner_min_sparse(i, j0);
        if (c > s->distance)
          s->distance = c;
      }
      else
        break;
    }
    /* Add 1 to j0 to avoid recalculating the diagonal */
    for (int j = j0 + 1; j < count2; j++)
    {
      key.i = i0; key.j = j;
      HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
      if (s != NULL)
      {
        c = get_corner_min_sparse(i0, j);
        if (c > s->distance)
          s->distance = c;
      }
      else
        break;
    }
  }
  key.i = count1 - 1; key.j = count2 - 1;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
  return s->distance;
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointseq_dfd_sparse(const TSequence *seq1, const TSequence *seq2)
{
  int count1 = seq1->count;
  int count2 = seq2->count;
  datum_func2 func = get_distance_fn(seq1->flags);

  /* Compute the diagonal */
  int n_diag;
  int *diag = bresenham(0, 0, count1 - 1, count2 - 1, &n_diag);
  /* Compute the distances in the diagonal */
  tpointseq_dfd_sparse1(seq1, seq2, diag, n_diag, func);
  /* Call the sparse computation of the discrete Frechet distance */
  double result = tpointseq_dfd_sparse2(seq1, seq2, diag, n_diag);
  /* Free memory */
  delete_all_distances();

  return result;
}

/*****************************************************************************/

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

double
tpoint_dfd_linear_internal(Temporal *temp1, Temporal *temp2)
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
    result = tpointseq_dfd_linear((TSequence *) temp1, (TSequence *) temp2);
  else /* temp1->subtype == SEQUENCESET */
    result = 0.0;
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_dfd_linear);
/**
 * Computes the discrete Frechet distance between two temporal points.
 */
Datum
tpoint_dfd_linear(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_dfd_linear_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

double
tpoint_dfd_fast_internal(Temporal *temp1, Temporal *temp2)
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
    result = tpointseq_dfd_fast((TSequence *) temp1, (TSequence *) temp2);
  else /* temp1->subtype == SEQUENCESET */
    result = 0.0;
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_dfd_fast);
/**
 * Computes the discrete Frechet distance between two temporal points.
 */
Datum
tpoint_dfd_fast(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_dfd_fast_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

double
tpoint_dfd_sparse_internal(Temporal *temp1, Temporal *temp2)
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
    result = tpointseq_dfd_sparse((TSequence *) temp1, (TSequence *) temp2);
  else /* temp1->subtype == SEQUENCESET */
    result = 0.0;
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_dfd_sparse);
/**
 * Computes the discrete Frechet distance between two temporal points.
 */
Datum
tpoint_dfd_sparse(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_dfd_sparse_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/
