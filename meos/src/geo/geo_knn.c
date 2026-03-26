/*****************************************************************************
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
 * @brief Function for defining the Local Outlier Factor (LOF) of an array of
 * geometries
 * @details The LOF function uses a data structure referred to as k-sorted
 * list, which is a sorted double-linked list of maximum k elements stored in
 * an array. This data structure is used to compute the k nearest neighbors
 * (KNN) for every geometry on the input array.
 */

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>
#include "temporal/temporal.h"

/**
 * @brief Structure defining the elements of a k-sorted list
 */
typedef struct KSLElem
{
  int key;           /**< Key of this value */
  double value;      /**< Value */
  int prev;          /**< Array index of the previous element */
  int next;          /**< Array index of the next element */
} KSLElem;

/**
 * @brief Structure defining a k-sorted list
 */
typedef struct KSortedList
{
  int capacity;     /**< Maximum number of elements, which is k */
  int length;       /**< Number of elements */
  int head;         /**< Array index of the head element */
  int tail;         /**< Array index of the tail element */
  int next;         /**< Array index of the next available element */
  int *freed;       /**< Array of indices of deleted elements */
  int freecount;    /**< Number of deleted elements */
  KSLElem *elems;   /**< Array of k elements */
} KSortedList;

/**
 * @brief Structure keeping the k nearest neighbors of an input geometry
 */
typedef struct KnnElem
{
  uint32_t *neighbors;     /**< Array of indices of the k neighbors */
  double *distances;       /**< Distances to the k neighbors */
} KnnElem;


/**
 * @brief Constructs a k-sorted list stored in an array
 * @param[in] capacity Number of elements in the list
 * @note The `freed` field of the structure will be initialized the first time
 * the #ksortedlist_free_elem function is called 
 */
KSortedList *
ksortedlist_make(int capacity)
{
  assert(capacity > 0);
  KSortedList *result = palloc0(sizeof(KSortedList));
  result->elems = palloc0(sizeof(KSLElem) * capacity);
  result->capacity = capacity;
  return result;
}

/**
 * @brief Free an array element from the list
 * @param[in] list K-sorted list
 * @param[in] pos Index of the element to free
 */
static void
ksortedlist_free_elem(KSortedList *list, int pos)
{
  assert(list); assert(list->length); assert(pos >= 0);
  /* If the free list has not been yet created */
  if (! list->freed)
    list->freed = palloc(sizeof(int) * list->capacity);
  /* Mark the element as free */
  list->freed[list->freecount++] = pos;
  list->length--;
  return;
}

/**
 * @brief Delete the tail element of the list
 * @param[in] list K-sorted list
 */
void ksortedlist_del_tail(KSortedList *list)
{
  assert(list);
  if (! list->length)
    return; /* Nothing to delete */

  int tail = list->tail;
  int prev = list->elems[tail].prev;
  list->tail = prev;
  if (prev != -1)
    list->elems[prev].next = -1;
  ksortedlist_free_elem(list, tail);
  return;
}

/**
 * @brief Return the position to store a new element in the k-sorted list
 * @param[in] list K-sorted list
 */
int ksortedlist_alloc(KSortedList *list)
{
  /* If list is full, delete tail element containing the maximum value */
  if (list->length == list->capacity)
    ksortedlist_del_tail(list);

  /* If there is unused space left by a previously deleted element, reuse it */
  if (list->freecount)
  {
    list->freecount--;
    return list->freed[list->freecount];
  }

  /* Return the first available entry */
  list->next++;
  return list->next - 1;
}

/**
 * @brief Insert an element into the k-sorted list
 * @param[in] list K-sorted list
 * @param[in] key Key for this value
 * @param[in] value Value
 */
void
ksortedlist_insert(KSortedList *list, int key, double value)
{
  assert(list);

  /* If list is full */
  if (list->length == list->capacity)
  {
    /* Verify whether we need to add the value */
    if (list->elems[list->tail].value < value)
      /* Nothing to do */
      return;
    else
      /* Delete tail element containing the maximum value */
      ksortedlist_del_tail(list);
  }

  /* Insert the element */
  int pos = ksortedlist_alloc(list);
  /* If list is empty or new element should be new head */
  if (! list->length || list->elems[list->head].value >= value)
  {
    list->elems[pos].key = key;
    list->elems[pos].value = value;
    list->elems[pos].next = list->length ? list->head : -1;
    list->elems[pos].prev = -1;
    list->head = pos;
    if (! list->length)
      /* Set the head and the tail to the single element of the list */
      list->tail = list->head;
    else
      /* Set the previous pointer of the old head to the new element */
      list->elems[list->elems[pos].next].prev = pos;
  }
  else
  {
    /* Find position to insert */
    int prev = -1;
    int curr = list->head;
    while (curr >= 0 && list->elems[curr].value < value)
    {
      prev = curr;
      curr = list->elems[curr].next;
    }
    /* Update the tail or get the next element */
    if (curr < 0)
      list->tail = pos;
    /* Set the values of the new element */
    list->elems[pos].key = key;
    list->elems[pos].value = value;
    list->elems[pos].next = curr;
    list->elems[pos].prev = prev;
    /* Set the values of the previous and next element (if any) or the tail */
    if (prev >= 0)
      list->elems[prev].next = pos;
    if (curr >= 0)
      list->elems[curr].prev = pos;
  }
  list->length++;
  return;
}

/**
 * @brief Search for the n-th value in the list
 * @param[in] list K-sorted list
 * @param[in] n K-th value to find, 1-based
 */
int
ksortedlist_kth_value(KSortedList *list, int n)
{
  assert(list); assert(n > 0 && n <= list->capacity);
  /* If the list is empty */
  if (! list->length)
    return -1;

  /* Traverse the list */
  int pos = 1;
  int curr = list->head;
  while (pos++ < n && curr >= 0)
    curr = list->elems[curr].next;
  return curr;
}

#if DEBUG_BUILD
/**
 * @brief Search for a value in the list
 * @param[in] list K-sorted list
 * @param[in] value Value
 * @return If not found return -1
 */
int
ksortedlist_search(KSortedList *list, int key)
{
  assert(list);
  /* If the list is empty */
  if (! list->length)
    return -1;

  /* Traverse the list */
  int curr = list->head;
  while (curr >= 0 && list->elems[curr].key != key)
    curr = list->elems[curr].next;
  return curr;
}

/**
 * @brief Delete an element with a given value from the list
 * @param[in] list K-sorted list
 * @param[value] value Value
 */
int
ksortedlist_delete(KSortedList *list, double value)
{
  assert(list);
  /* If the list is empty */
  if (! list->length)
    return 0;

  /* Traverse the list */
  int prev = -1;
  int curr = list->head;
  while (curr >= 0 && list->elems[curr].value < value)
  {
    prev = curr;
    curr = list->elems[curr].next;
  }

  /* Element not found */
  if (curr < 0 || list->elems[curr].value != value)
    return 0;

  /* Element found; delete it */
  if (prev < 0)
  {
    /* Element is head */
    list->head = list->elems[curr].next;
  }
  else
  {
    list->elems[prev].next = list->elems[curr].next;
    if (list->elems[curr].next)
      list->elems[list->elems[curr].next].prev = curr;
  }

  ksortedlist_free_elem(list, curr);
  return 1; /* Successful deletion */
}

/**
 * @brief Print the list
 * @param[in] list K-sorted list
 * @param[value] Value
 */
void
ksortedlist_print(KSortedList *list)
{
  assert(list);
  printf("KSortedList: ");
  int curr = list->head;
  while (curr >= 0)
  {
    printf("%lf -> ", list->elems[curr].value);
    curr = list->elems[curr].next;
  }
  printf("NULL\n");
  return;
}
#endif /* DEBUG_BUILD */

/**
 * @brief Free the list
 * @param[in] list K-sorted list
 */
void
ksortedlist_free(KSortedList *list)
{
  if (! list)
    return;
  if (list->elems)
    pfree(list->elems);
  if (list->freed)
    pfree(list->freed);
  pfree(list);
  return;
}

/*****************************************************************************
 * Functions for knn search
 *****************************************************************************/

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the k nearest neighbors of a point
 * @param[in] list Sorted list of values
 * @param[in] geoarr Array of geometries
 * @param[in] count Number of elements in the input array
 * @param[in] point Reference point to compute the nearest neighbors
 * @param[in] k Number of nearest neighbors
 * @param[in] self Array index of the reference point that must be skipped
 * @param[out] distances Distances to the k nearest neighbors
 */
uint32_t *
geo_knn_search(KSortedList *list, const GSERIALIZED **geoarr, uint32_t count,
  const GSERIALIZED *point, uint32_t k, uint32_t self, double **distances)
{
  assert(list); assert(geoarr); assert(point); assert(count > k);
  assert(self <= count);

  /* Determine the distance function to apply */
  double (*func)(const GSERIALIZED *, const GSERIALIZED *);
  if (FLAGS_GET_GEODETIC(geoarr[0]->gflags))
    func = &geog_distance;
  else
    func = FLAGS_GET_Z(geoarr[0]->gflags) ? &geom_distance3d : &geom_distance2d;
  /* Compute the k nearest neighbors of each geometry */
  for (uint32_t i = 0; i < count; i++)
  {
    if (i == self)
      continue;
    double distance = func(geoarr[i], point);
    ksortedlist_insert(list, i, distance);
  }
  uint32_t *result = palloc(sizeof(uint32_t) * k);
  double *dists = palloc(sizeof(double) * k);
  for (uint32_t i = 0; i < k; i++)
  {
    uint32_t pos = ksortedlist_kth_value(list, i + 1);
    result[i] = list->elems[pos].key;
    dists[i] = list->elems[pos].value;
  }
  /* Set the output parameter and return */
  *distances = dists;
  return result;
}

/**
 * @ingroup meos_temporal_analytics_outlier
 * @brief Return the weighted local outlier factor (WLOF) of an array of
 * geometries
 * @param[in] geoarr Array of geometries
 * @param[in] count Number of elements in the input array
 * @param[in] k Number of nearest neighbors
 * @param[in] distance Distance value for grouping geometries into clusters
 * @param[out] newcount Number of elements in the output array
 * @param[out] clusters Resulting clusters
 * @csqlfn #Geo_wlof()
 */
double *
geo_wlof(const GSERIALIZED **geoarr, uint32_t count, uint32_t k, 
  double distance, uint32_t *newcount, GSERIALIZED ***clusters)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(geoarr, NULL); VALIDATE_NOT_NULL(newcount, NULL);
  if (! ensure_not_negative(count) || ! ensure_not_negative(k) ||
      ! ensure_not_negative_datum(Float8GetDatum(distance), T_FLOAT8))
      return NULL;

  /* Cluster geometries so that a geometry in a cluster is within the given
     distance of at least another geometry in the same cluster */
  uint32_t count1 = 0;
  GSERIALIZED **clusts = geo_cluster_within(geoarr, count, distance, &count1);
  /* The number of resulting clusters count1 should greater than k */
  if (count1 <= k)
    return NULL;
  GSERIALIZED **geos = palloc(sizeof(GSERIALIZED *) * count1);
  uint32_t *counts = palloc(sizeof(uint32_t) * count1);
  for (uint32_t i = 0; i < count1; i++)
  {
    geos[i] = geo_geo_n(clusts[i], 1);
    counts[i] = geo_num_geos(clusts[i]);
  }

  /* Create the KNN table */
  KnnElem *knn = palloc(sizeof(KnnElem) * count1);
  for (uint32_t i = 0; i < count1; i++)
  {
    KSortedList *list = ksortedlist_make(k);
    knn[i].distances = palloc(sizeof(double) * k);
    knn[i].neighbors = geo_knn_search(list, (const GSERIALIZED **) geos,
      count1, geos[i], k, i, &knn[i].distances);
    ksortedlist_free(list);
  }

  /* Create the reachability distances */
  int destination;
  for (uint32_t i = 0; i < count1; i++)
  {
    for (uint32_t j = 0; j < k; j++)
    {
      destination = knn[i].neighbors[j];
      knn[i].distances[j] = Max(knn[i].distances[j],
        /* Third distance of destination point */
        knn[destination].distances[2]);
    }
  }

  /* Compute the local reachability densities */
  double *reach_dens = palloc(sizeof(double) * count1);
  for (uint32_t i = 0; i < count1; i++)
  {
    reach_dens[i] = 0.0;
    for (uint32_t j = 0; j < k; j++)
      reach_dens[i] += knn[i].distances[j];
    double avg_dist = reach_dens[i] / k;
    reach_dens[i] = fabs(avg_dist) < MEOS_EPSILON ? 1.0 : 1.0 / avg_dist;
  }

  /* Compute the neighborhood reachability densities */
  double *neigh_reach_dens = palloc(sizeof(double) * count1);
  for (uint32_t i = 0; i < count1; i++)
  {
    neigh_reach_dens[i] = 0.0;
    for (uint32_t j = 0; j < k; j++)
    {
      destination = knn[i].neighbors[j];
      if (reach_dens[destination] < 0.0)
      {
        neigh_reach_dens[i] = -1.0;
        break;
      }
      neigh_reach_dens[i] += reach_dens[destination];
    }
    neigh_reach_dens[i] = neigh_reach_dens[i] / k;
  }

  /* Compute the local outlier factor (LOF) */
  double *res = palloc(sizeof(double) * count1);
  for (uint32_t i = 0; i < count1; i++)
    res[i] = neigh_reach_dens[i] / reach_dens[i];

  /* Propagate the local outlier factor (LOF) to the cluster members */
  double *result = palloc(sizeof(double) * count);
  uint32_t l = 0;
  for (uint32_t i = 0; i < count1; i++)
    for (uint32_t j = 0; j < counts[i]; j++)
      result[l++] = res[i];

  /* Clean up and return */
  pfree(geos); pfree(counts); pfree(reach_dens); pfree(neigh_reach_dens);
  pfree(res);
  for (uint32_t i = 0; i < count1; i++)
  {
    pfree(knn[i].neighbors);
    pfree(knn[i].distances);
  }
  pfree(knn);
  *newcount = count1;
  *clusters = clusts;
  return result;
}

/*****************************************************************************/
