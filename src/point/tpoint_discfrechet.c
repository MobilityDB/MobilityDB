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
#include "general/temporaltypes.h"
#include "point/tpoint.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Custom structures for storing a sparse matrix for distances
 *****************************************************************************/

/**
 * Structure to represent elements of the dynamic array representing the
 * sparse matrix for distances
 */
typedef struct {
  int i;
  int j;
  double distance;
} distance_t;

/**
 * Structure to represent a sparse matrix for distances
 */
typedef struct
{
  int nelems;   /**< Number of elements we are currently storing */
  int maxelems; /**< Number of elements we have space for in distlist */
  distance_t *distlist; /**< Dynamic array of coordinates and distances */
} DISTARRAY;

/**
 * Construct an empty distance array.
 */
DISTARRAY *
distarray_construct(int maxelems)
{
  /* Create the structure */
  DISTARRAY *da = malloc(sizeof(DISTARRAY));
  da->nelems = 0;
  da->maxelems = maxelems;
  /* Allocate the dynamic array for the matrix cells */
  if (maxelems > 0)
    da->distlist = malloc(sizeof(distance_t) * maxelems);
  else
    da->distlist = NULL;
  return da;
}

/*
 * Add an element into a distance array.
 *
 * @param[in] da Distance array
 * @param[in] dist Matrix cell
 * @param[in] where Location where the value is added
*/
bool
distarray_add(DISTARRAY *da, const distance_t *dist, int where)
{
  distance_t *elem;
  if (!da || !dist)
    return false;
  /* Error on invalid offset value */
  if (where > da->nelems)
  {
    elog(ERROR, "distarray_add: offset out of range (%d)", where);
    return false;
  }
  /* If we have no storage, let's allocate some */
  if (da->maxelems == 0 || ! da->distlist)
  {
    da->maxelems = 32;
    da->nelems = 0;
    da->distlist = malloc(sizeof(distance_t) * da->maxelems);
    elog(WARNING, "Creating the dynamic array for %d elements", da->maxelems);
  }
  /* Error out if we have a bad situation */
  if (da->nelems > da->maxelems)
  {
    elog(ERROR, "nelems (%d) is greater than maxelems (%d)", da->nelems, da->maxelems);
    return false;
  }
  /* Check if we have enough storage, add more if necessary */
  if (da->nelems == da->maxelems)
  {
    da->maxelems *= 2;
    da->distlist = realloc(da->distlist, sizeof(distance_t) * da->maxelems);
    elog(WARNING, "Reallocating the dynamic array for %d elements", da->maxelems);
  }
  /* Make space to insert the new distance */
  elem = da->distlist + where;
  if (where < da->nelems)
  {
    size_t copy_size = sizeof(distance_t) * (da->nelems - where);
    memmove(elem + 1, elem, copy_size);
  }
  /* We have one more element */
  ++da->nelems;
  /* Copy the new distance into the gap */
  memcpy(elem, dist, sizeof(distance_t));
  return true;
}

/**
 * Append a distance into a distance array.
 */
bool
distarray_append(DISTARRAY *da, const distance_t *dist)
{
  return distarray_add(da, dist, da->nelems);
}
/**
 * Free a distance array.
 */
void
distarray_free(DISTARRAY *da)
{
  if (da)
  {
    if (da->distlist)
      free(da->distlist);
    free(da);
  }
}

/**
 * Comparison of 2D array indices
 */
int
indices_cmp(int i1, int j1, int i2, int j2)
{
  int cmp = (i1 - i2);
  return cmp ? cmp : (int) (j1 - j2);
}

/**
 * Returns the location of the index in the distance array using sequential
 * search.
 */
bool
distarray_find_element_seq(const DISTARRAY *da, int i, int j, int *loc)
{
  int cmp;
  const distance_t *dist = NULL; /* make compiler quiet */
  for (int i = 0; i < da->nelems; i++)
  {
    dist = da->distlist + i;
    cmp = indices_cmp(dist->i, dist->j, i, j);
    if (cmp == 0)
    {
      *loc = i;
      return true;
    }
    if (cmp < 0)
    {
      *loc = i;
      return true;
    }
  }
  if (cmp > 0)
    *loc = da->nelems - 1;
  else
    *loc = da->nelems;
  return false;
}

/**
 * Returns the location of the index in the distance array using binary
 * search.
 *
 * If the index is in the distance array, its location is returned in the
 * output parameter. Otherwise, returns a number encoding whether the index is
 * before, between two indices, or after the indices in the distance array.
 * For example, given a distance array composed of 3 indices and a given
 * index (i,j), the value returned in the output parameter is as follows:
 * @code
 *         (i0,j0)  (i1,j1)  (i2,j2)
 *            |        |        |
 * 1) (i,j) ^                          => result = 0
 * 2)   (i,j) ^                        => result = 0, found = true
 * 3)        (i,j) ^                   => result = 1
 * 4)               (i,j) ^            => result = 2
 * 5)                       (i,j) ^    => result = 3
 * @endcode
 *
 * @param[in] da Distance array
 * @param[in] i,j Index to search
 * @param[out] loc Location
 * @result Returns true if the index is contained in the index array
 */
bool
distarray_find_element(const DISTARRAY *da, int i, int j, int *loc)
{
  int first = 0;
  int last = da->nelems - 1;
  int cmp;
  int middle = 0; /* make compiler quiet */
  const distance_t *dist = NULL; /* make compiler quiet */
  while (first <= last)
  {
    middle = (first + last)/2;
    dist = da->distlist + middle;
    cmp = indices_cmp(dist->i, dist->j, i, j);
    if (cmp == 0)
    {
      *loc = middle;
      return true;
    }
    if (cmp > 0)
      last = middle - 1;
    else
      first = middle + 1;
  }
  if (cmp < 0)
    middle++;
  *loc = middle;
  return false;
}

/**
 * Add an element to the distance array if the matrix cell does not exist. 
 * If the matrix cell already exists, it updates its distance value.
 */
void
distarray_add_distance(DISTARRAY *da, int i, int j, double distance)
{
  int loc;
  bool found = distarray_find_element(da, i, j, &loc);
  if (! found)
  {
    distance_t dist;
    dist.i = i; dist.j = j; dist.distance = distance;
    distarray_add(da, &dist, loc);
  }
  else
  {
    distance_t *elem = da->distlist + loc;
    elem->distance = distance;
  }
  return;
}

/**
 * Gets the distance associated to the coordinates in the distance array.
 *
 * @param[in] da Distance array
 * @param[in] i,j Array coordinates
 * @note Returns infinity if the matrix cell does not exist
 */
double
distarray_get_distance(const DISTARRAY *da, int i, int j)
{
  int loc;
  // bool found = distarray_find_element(da, i, j, &loc);
  bool found = distarray_find_element_seq(da, i, j, &loc);
  double result;
  if (found)
  {
    distance_t *elem = da->distlist + loc;
    result = elem->distance;
  }
  else 
    result = INFINITY;
  return result;
}

/**
 * Prints the distance array
 *
 * @param[in] da Distance array
 */
#ifdef DEBUG_BUILD
void
distarray_print(const DISTARRAY *da)
{
  int len = 0;
  char buf[16384];
  const distance_t *dist;
  for (int i = 0; i < da->nelems; i++)
  {
    dist = da->distlist + i;
    len += sprintf(buf+len, "(%d,%d): %lf\n", dist->i, dist->j, dist->distance);
  }
  len += sprintf(buf+len, "Number of elements: %d\n", da->nelems);
  ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("DISTARRAY:\n%s", buf)));
  return;
}
#endif

/**
 * Comparison of 2D array indices
 */
int
distances_cmp_index(distance_t *d1, distance_t *d2)
{
  int cmp = (d1->i - d2->i);
  return cmp ? cmp : (int) (d1->j - d2->j);
}

/**
 * Sort function for distance array
 */
void
sort_distances_index(const DISTARRAY *da)
{
  qsort(da->distlist, (size_t) da->nelems, sizeof(distance_t),
    (qsort_comparator) &distances_cmp_index);
  return;
}

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
} hdistance_t;

hdistance_t *distances = NULL;

void add_distance(int i, int j, double distance)
{
  hdistance_t *s;
  indices_t key;
  key.i = i; key.j = j;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* (i,j) already in the hash? */
  if (s == NULL)
  {
    s = (hdistance_t *) malloc(sizeof *s);
    s->key.i = i; s->key.j = j;
    HASH_ADD(hh, distances, key, sizeof(indices_t), s);   /* key: name of key field */
  }
  s->distance = distance;
}

hdistance_t *find_distance(int i, int j)
{
  hdistance_t *s;
  indices_t key;
  key.i = i; key.j = j;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
  return s;
}

void delete_distance(hdistance_t *distance)
{
  HASH_DEL(distances, distance);  /* distance: pointer to deletee */
  free(distance);
}

void delete_all_distances()
{
  hdistance_t *current_distance, *tmp;
  HASH_ITER(hh, distances, current_distance, tmp)
  {
    HASH_DEL(distances, current_distance);  /* delete it (distances advances to next) */
    free(current_distance);                 /* free it */
  }
}

void print_distances()
{
  hdistance_t *s;
  for (s = distances; s != NULL; s = (hdistance_t *)(s->hh.next))
  {
    printf("key: (%d,%d), distance: %lf\n", s->key.i, s->key.j, s->distance);
  }
}

int distance_sort(hdistance_t *a, hdistance_t *b)
{
  return (a->distance > b->distance) - (a->distance < b->distance);
}

int key_sort(hdistance_t *a, hdistance_t *b)
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
    } /* e_xy + e_y < 0 */
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
 */
static double
tpointinst_distance(const TInstant *inst1, const TInstant *inst2)
{
  Datum value1 = tinstant_value(inst1);
  Datum value2 = tinstant_value(inst2);
  double result;
  if (MOBDB_FLAGS_GET_Z(inst1->flags)) /* 3D */
  {
    const POINT3DZ *p1 = datum_get_point3dz_p(value1);
    const POINT3DZ *p2 = datum_get_point3dz_p(value2);
    result = distance3d_pt_pt((POINT3D *) p1, (POINT3D *) p2);
  }
  else
  {
    const POINT2D *p1 = datum_get_point2d_p(value1);
    const POINT2D *p2 = datum_get_point2d_p(value2);
    result = distance2d_pt_pt(p1, p2);
  }
  return result;
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
 * @param[in] dist Array keeping the distances
 */
static double
tpointseq_dfd_rec1(const TSequence *seq1, const TSequence *seq2, int i, int j,
  double *dist)
{
  const TInstant *inst1 = tsequence_inst_n(seq1, i);
  const TInstant *inst2 = tsequence_inst_n(seq2, j);
  double d;
  /* Pointer to dist[i, j], just to simplify notation */
  double *d_ij = dist + i * seq2->count + j;
  if (*d_ij > -1.0)
    return *d_ij;
  d = tpointinst_distance(inst1, inst2);
  if (i == 0 && j == 0)
    *d_ij = d;
  else if (i > 0 && j == 0)
    *d_ij = Max(tpointseq_dfd_rec1(seq1, seq2, i-1, 0, dist), d);
  else if (i == 0 && j > 0)
    *d_ij = Max(tpointseq_dfd_rec1(seq1, seq2, 0, j-1, dist), d);
  else if (i > 0 && j > 0)
    *d_ij =
      Max(
        Min(tpointseq_dfd_rec1(seq1, seq2, i-1, j, dist),
          Min(tpointseq_dfd_rec1(seq1, seq2, i-1, j-1, dist),
            tpointseq_dfd_rec1(seq1, seq2, i, j-1, dist))),
        d);
  else
    *d_ij = get_float8_infinity();
  return *d_ij;
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

  /* Allocate memory for distrance matrix */
  double *dist = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < count1 * count2; k++)
    *(dist + k) = -1.0;

  /* Call the recursive computation of the discrete Frechet distance */
  double result = tpointseq_dfd_rec1(seq1, seq2, count1 - 1, count2 - 1, dist);
  /* Free memory */
  pfree(dist);

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
 * @param[in] dist Array keeping the distances
 */
static double
tpointseq_dfd_linear1(const TInstant **instants1, int count1,
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
        dist[i * count2 + j] = Max(
          Min(dist[(i - 1) * count2 + j - 1],
            Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1])),
          d);
      }
      else if (i > 0 && j == 0)
        dist[i * count2] = Max(dist[(i - 1) * count2], d);
      else if (i == 0 && j > 0)
        dist[j] = Max(dist[j - 1], d);
      else if (i == 0 && j == 0)
        dist[0] = d;
      else
        dist[i * count2 + j] = get_float8_infinity();
    }
  }
  return dist[count1 * count2 - 1];
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointinstarr_dfd_linear(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Allocate memory for dist */
  double *dist = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with -1.0 */
  for (int k = 0; k < count1 * count2; k++)
    *(dist + k) = -1.0;

  /* Call the linear computation of the discrete Frechet distance */
  double result = tpointseq_dfd_linear1(instants1, count1, instants2, count2, dist);
  /* Free memory */
  pfree(dist);

  return result;
}

/*****************************************************************************
 * Matrix implementation of the fast discrete Frechet distance
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

void
tpointinstarr_dfd_fast1(double *dist, const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, int *diag, int n_diag)
{
  double d, diag_max = -INFINITY;
  int i, j, k, i0, j0;
  // int i_min = 0, j_min = 0;
  const TInstant *inst1, *inst2, *inst;

  /* Fill in the diagonal with the seed distance values */
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = instants1[i0];
    inst2 = instants2[j0];
    d = tpointinst_distance(inst1, inst2);
    diag_max = Max(diag_max, d);
    dist[i0 * count2 + j0] = d;
  }
  /* For each element of the diagonal, compute the distances to the right
   * and below the element for the values that are not larger than the
   * maximum of the diagonal */
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = instants1[i0];
    inst2 = instants2[j0];
    /* Going right the diagonal cell */
    for (j = j0 + 1; j < count2; j++)
    {
      // if (dist[i0 * count2 + j] == INFINITY)
      // {
        inst = instants2[j];
        d = tpointinst_distance(inst1, inst);
        if (d <= diag_max)
          dist[i0 * count2 + j] = d;
        // else
          // break;
      // }
      // else
        // break;
    }
    /* Going down the diagonal cell */
    for (i = i0 + 1; i < count1; i++)
    {
      // if (dist[i * count2 + j0] == INFINITY)
      // {
        inst = instants1[i];
        d = tpointinst_distance(inst, inst2);
        if (d <= diag_max)
          dist[i * count2 + j0] = d;
        // else
          // break;
      // }
      // else
        // break;
    }
  }
  return;
}

double
get_corner_min_array(double *dist, int i, int j, int count2)
{
  double result;
  if (i > 0 && j > 0)
    result = Min(dist[(i - 1) * count2 + j - 1],
      Min(dist[(i - 1) * count2 + j], dist[i * count2 + j - 1]));
  else if (i == 0 && j == 0)
    result = dist[0];
  else if (i == 0)
    result = dist[j - 1];
  else  /* j == 0 */
    result = dist[(i - 1) * count2];
  return result;
}

double
tpointinstarr_dfd_fast2(double *dist, int count1, int count2,
  int *diag, int n_diag)
{
  double c;
  for (int i = 0; i < count1; i++)
  {
    for (int j = 0; j < count2; j++)
    {
      c = get_corner_min_array(dist, i, j, count2);
      dist[i * count2 + j] = Max(dist[i * count2 + j], c);
    }
  }
  return dist[count1 * count2 - 1];
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointinstarr_dfd_fast(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Allocate memory for the distance matrix */
  double *dist = (double *) palloc(sizeof(double) * count1 * count2);
  /* Initialise it with infinity */
  for (int k = 0; k < count1 * count2; k++)
    *(dist + k) = INFINITY;
  /* Compute the diagonal */
  int n_diag;
  int *diag = bresenham(0, 0, count1 - 1, count2 - 1, &n_diag);
  /* Compute the distances around the diagonal */
  tpointinstarr_dfd_fast1(dist, instants1, count1, instants2, count2, diag, n_diag);
  // matrix_print(dist, count1, count2);
  /* Call the fast computation of the discrete Frechet distance */
  double result = tpointinstarr_dfd_fast2(dist, count1, count2, diag, n_diag);
  // matrix_print(dist, count1, count2);
  /* Free memory */
  pfree(dist);
  return result;
}

/*****************************************************************************
 * Sparse matrix implementation of the fast discrete Frechet distance
 *****************************************************************************/

double
get_hdistance(int i, int j)
{
  hdistance_t *s;
  indices_t key;
  key.i = i; key.j = j;
  HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
  return s != NULL ? s->distance : INFINITY;
}

double
get_corner_min_hsparse(int i, int j)
{
  double result;
  if (i > 0 && j > 0)
    result = Min(get_hdistance(i - 1, j - 1),
      Min(get_hdistance(i, j - 1), get_hdistance(i - 1, j)));
  else if (i == 0 && j == 0)
    result = get_hdistance(i, j);
  else if (i == 0)
    result = get_hdistance(i, j - 1);
  else  /* j == 0 */
    result = get_hdistance(i - 1, j);
  return result;
}

void
tpointinstarr_dfd_hsparse1(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2, int *diag, int n_diag)
{
  double d, diag_max = -INFINITY;
  int i, j, k, i0, j0, i_min = 0, j_min = 0;
  hdistance_t *s;
  indices_t key;
  const TInstant *inst1, *inst2, *inst;

  /* Fill in the diagonal with the seed distance values */
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = instants1[i0];
    inst2 = instants2[j0];
    d = tpointinst_distance(inst1, inst2);
    if (d > diag_max)
      diag_max = d;
    add_distance(i0, j0, d);
  }
  /* For each element of the diagonal, compute the distances to the right
   * and above the element until we found a value that is larger than the
   * maximum of the diagonal */
  for (k = 0; k < n_diag - 1; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = instants1[i0];
    inst2 = instants2[j0];
    for (i = i0 + 1; i < count1; i++)
    {
      key.i = i; key.j = j0;
      HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
      if (s == NULL)
      {
        inst = instants1[i];
        d = tpointinst_distance(inst, inst2);
        if (d <= diag_max || i <= i_min)
          add_distance(i, j0, d);
        else
          break;
      }
      else
        break;
      i_min = i;
    }
    for (j = j0 + 1; j < count2; j++)
    {
      key.i = i0; key.j = j;
      HASH_FIND(hh, distances, &key, sizeof(indices_t), s); /* s: output pointer */
      if (s == NULL)
      {
        inst = instants2[j];
        d = tpointinst_distance(inst1, inst);
        if (d <= diag_max || j <= j_min)
          add_distance(i0, j, d);
        else
          break;
      }
      else
        break;
      j_min = j;
    }
  }
  /* Sort the distances by the indices */
  sort_by_key();
  return;
}

double
tpointinstarr_dfd_hsparse2(int count1, int count2, int *diag, int n_diag)
{
  int i0, j0;
  double c;
  hdistance_t *s;
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
        c = get_corner_min_hsparse(i, j0);
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
        c = get_corner_min_hsparse(i0, j);
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
tpointinstarr_dfd_hsparse(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Compute the diagonal */
  int n_diag;
  int *diag = bresenham(0, 0, count1 - 1, count2 - 1, &n_diag);
  /* Compute the distances in the diagonal */
  tpointinstarr_dfd_hsparse1(instants1, count1, instants2, count2, diag, n_diag);
  /* Call the sparse computation of the discrete Frechet distance */
  double result = tpointinstarr_dfd_hsparse2(count1, count2, diag, n_diag);
  /* Free memory */
  delete_all_distances();
  return result;
}

/*****************************************************************************
 * Sparse matrix implementation of the fast discrete Frechet distance
 *****************************************************************************/

double
get_corner_min_sparse(const DISTARRAY *da, int i, int j)
{
  double result;
  if (i > 0 && j > 0)
    result = Min(distarray_get_distance(da, i - 1, j - 1),
      Min(distarray_get_distance(da, i, j - 1), 
        distarray_get_distance(da, i - 1, j)));
  else if (i == 0 && j == 0)
    result = distarray_get_distance(da, i, j);
  else if (i == 0)
    result = distarray_get_distance(da, i, j - 1);
  else  /* j == 0 */
    result = distarray_get_distance(da, i - 1, j);
  return result;
}

void
tpointinstarr_dfd_sparse1(DISTARRAY *da, const TInstant **instants1,
  int count1, const TInstant **instants2, int count2, int *diag, int n_diag)
{
  double d, diag_max = -INFINITY;
  int i, j, k, i0, j0, loc, i_min = 0, j_min = 0;
  bool found;
  const TInstant *inst1, *inst2, *inst;
  distance_t elem;
  /* Fill in the diagonal with the seed distance values */
  for (k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = instants1[i0];
    inst2 = instants2[j0];
    d = tpointinst_distance(inst1, inst2);
    if (d > diag_max)
      diag_max = d;
    elem.i = i0; elem.j = j0; elem.distance = d;
    distarray_append(da, &elem);
  }
  /* For each element of the diagonal, compute the distances to the right
   * and above the element until we found a value that is larger than the
   * maximum of the diagonal */
  for (k = 0; k < n_diag - 1; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    inst1 = instants1[i0];
    inst2 = instants2[j0];
    for (i = i0 + 1; i < count1; i++)
    {
      // found = distarray_find_element(da, i, j0, &loc);
      found = distarray_find_element_seq(da, i, j0, &loc);
      if (! found)
      {
        inst = instants1[i];
        d = tpointinst_distance(inst, inst2);
        if (d <= diag_max || i <= i_min)
        {
          elem.i = i; elem.j = j0; elem.distance = d;
          // distarray_add(da, &elem, loc);
          distarray_append(da, &elem);
        }
        else
          break;
      }
      else
        break;
      i_min = i;
    }

    for (j = j0 + 1; j < count2; j++)
    {
      // found = distarray_find_element(da, i0, j, &loc);
      found = distarray_find_element_seq(da, i0, j, &loc);
      if (! found)
      {
        inst = instants2[j];
        d = tpointinst_distance(inst1, inst);
        if (d <= diag_max || j <= j_min)
        {
          elem.i = i0; elem.j = j; elem.distance = d;
          // distarray_add(da, &elem, loc);
          distarray_append(da, &elem);
        }
        else
          break;
      }
      else
        break;
      j_min = j;
    }
  }
  return;
}

double
tpointinstarr_dfd_sparse2(DISTARRAY *da, int count1, int count2, int *diag,
  int n_diag)
{
  int i0, j0, loc;
  double c;
  bool found;
  distance_t *dist;
  for (int k = 0; k < n_diag; k++)
  {
    i0 = diag[2 * k];
    j0 = diag[2 * k + 1];
    for (int i = i0; i < count1; i++)
    {
      found = distarray_find_element(da, i, j0, &loc);
      if (found)
      {
        c = get_corner_min_sparse(da, i, j0);
        dist = da->distlist + loc;
        if (c > dist->distance)
          dist->distance = c;
      }
      else
        break;
    }
    /* Add 1 to j0 to avoid recalculating the diagonal */
    for (int j = j0 + 1; j < count2; j++)
    {
      found = distarray_find_element(da, i0, j, &loc);
      if (found)
      {
        c = get_corner_min_sparse(da, i0, j);
        dist = da->distlist + loc;
        if (c > dist->distance)
          dist->distance = c;
      }
      else
        break;
    }
  }
  dist = da->distlist + da->nelems - 1;
  return dist->distance;
}

/**
 * Computes the discrete Frechet distance for two temporal sequence points.
 *
 * @param[in] seq1, seq2 Temporal points
 */
double
tpointinstarr_dfd_sparse(const TInstant **instants1, int count1,
  const TInstant **instants2, int count2)
{
  /* Create the distance array
   * We arbitrarily decided that 5 additional distances will be computed 
   * for each column in the array */
  int max = Max(count1, count2);
  DISTARRAY *da = distarray_construct(max * 5);
  /* Compute the diagonal */
  int n_diag;
  int *diag = bresenham(0, 0, count1 - 1, count2 - 1, &n_diag);
  /* Compute the distances in the diagonal */
  tpointinstarr_dfd_sparse1(da, instants1, count1, instants2, count2,
    diag, n_diag);
  /* Call the sparse computation of the discrete Frechet distance */
  double result = tpointinstarr_dfd_sparse2(da, count1, count2, diag, n_diag);
  /* Free memory */
  distarray_free(da);

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
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = tpointinstarr_dfd_linear(instants1, count1, instants2, count2);
  /* Free memory */
  pfree(instants1); pfree(instants2);
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
tpoint_dfd_fast_internal(const Temporal *temp1, const Temporal *temp2)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = tpointinstarr_dfd_fast(instants1, count1, instants2, count2);
  /* Free memory */
  pfree(instants1); pfree(instants2);
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
tpoint_dfd_hsparse_internal(Temporal *temp1, Temporal *temp2)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = tpointinstarr_dfd_hsparse(instants1, count1, instants2, count2);
  /* Free memory */
  pfree(instants1); pfree(instants2);
  return result;
}

PG_FUNCTION_INFO_V1(tpoint_dfd_hsparse);
/**
 * Computes the discrete Frechet distance between two temporal points.
 */
Datum
tpoint_dfd_hsparse(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = tpoint_dfd_hsparse_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

double
tpoint_dfd_sparse_internal(Temporal *temp1, Temporal *temp2)
{
  double result;
  int count1, count2;
  const TInstant **instants1 = temporal_instants_internal(temp1, &count1);
  const TInstant **instants2 = temporal_instants_internal(temp2, &count2);
  result = tpointinstarr_dfd_sparse(instants1, count1, instants2, count2);
  /* Free memory */
  pfree(instants1); pfree(instants2);
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
