/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Clustering of geometries by proximity, without GEOS
 * @details The clustering PostGIS performs reaches GEOS for one thing only, an
 * @p STRtree narrowing the pairs whose distance is worth computing. The
 * distance itself is @p lwgeom_mindistance2d_tolerance, the components are
 * merged with the union-find of @p lwunionfind.c, and a cluster is assembled
 * with @p lwcollection_construct, none of which reaches GEOS. The narrowing is
 * answered here by the bounding box of each geometry, which decides the same
 * pairs the index does: two geometries whose boxes stay further apart than the
 * distance cannot be closer than it.
 *
 * ⚠️ Which cluster a geometry lying at the edge of two of them joins depends on
 * the order the pairs are visited in, and an index visits them in an order of
 * its own. That is a property of the clustering itself rather than of either
 * implementation: a geometry that is at the core of no cluster belongs to
 * whichever one reaches it first. The geometries at the core of a cluster are
 * the same either way.
 */

/* C */
#include <float.h>
#include <string.h>
/* PostgreSQL */
#include "postgres.h"
/* PostGIS */
#include "liblwgeom.h"
#include "lwunionfind.h"
/* MEOS */
#include "meos.h"
#include "geo/geo_cluster.h"
#include "geo/geo_funcs.h"

/*****************************************************************************/

/**
 * @brief Return true if two geometries can lie within a distance of each other
 * @details The bounding box of one, widened by the distance, must meet the
 * bounding box of the other.
 */
static bool
cluster_boxes_within(const GBOX *box1, const GBOX *box2, double eps)
{
  assert(box1); assert(box2);
  GBOX widened = *box1;
  widened.xmin -= eps;
  widened.ymin -= eps;
  widened.xmax += eps;
  widened.ymax += eps;
  return gbox_overlaps_2d(&widened, box2) == LW_TRUE;
}

/**
 * @brief Return the bounding box of every geometry of an array
 * @return @p NULL if the box of a geometry cannot be computed
 */
static GBOX *
cluster_boxes(LWGEOM **geoms, uint32_t ngeoms)
{
  assert(geoms);
  GBOX *result = palloc0(sizeof(GBOX) * ngeoms);
  for (uint32_t i = 0; i < ngeoms; i++)
  {
    if (lwgeom_is_empty(geoms[i]))
      continue;
    if (lwgeom_calculate_gbox(geoms[i], &result[i]) == LW_FAILURE)
    {
      pfree(result);
      return NULL;
    }
  }
  return result;
}

/**
 * @brief Take a geometry into the cluster of another one, unless it belongs to
 * a cluster it is not at the core of
 * @details A geometry at the edge of a cluster stays in the first one that
 * reached it.
 */
static void
cluster_union_available(UNIONFIND *uf, uint32_t p, uint32_t q,
  const char *is_core, char *in_cluster)
{
  assert(uf); assert(is_core); assert(in_cluster);
  if (in_cluster[q] && ! is_core[q])
    return;
  UF_union(uf, p, q);
  in_cluster[q] = LW_TRUE;
}

/**
 * @brief Merge the geometries lying within a distance of each other
 * @details Every geometry belongs to a cluster, which is what a minimum of one
 * geometry per cluster asks for, so a pair is visited once and only its
 * distance decides.
 */
static bool
cluster_union_single(LWGEOM **geoms, uint32_t ngeoms, UNIONFIND *uf,
  double eps, const GBOX *boxes)
{
  assert(geoms); assert(uf); assert(boxes);
  for (uint32_t p = 0; p < ngeoms; p++)
  {
    if (lwgeom_is_empty(geoms[p]))
      continue;
    for (uint32_t q = p + 1; q < ngeoms; q++)
    {
      if (lwgeom_is_empty(geoms[q]))
        continue;
      /* Already one cluster, which no distance changes */
      if (UF_find(uf, p) == UF_find(uf, q))
        continue;
      if (! cluster_boxes_within(&boxes[p], &boxes[q], eps))
        continue;
      double distance = lwgeom_mindistance2d_tolerance(geoms[p], geoms[q],
        eps);
      if (distance == FLT_MAX)
        return false;
      if (distance <= eps)
        UF_union(uf, p, q);
    }
  }
  return true;
}

/**
 * @brief Merge the geometries reached by a chain of geometries each holding a
 * minimum number of others within a distance of it
 * @details A geometry holding that many others within the distance is at the
 * core of a cluster and takes every one of them into it. One holding fewer
 * joins the first cluster whose core reaches it and takes nothing into it.
 */
static bool
cluster_union_core(LWGEOM **geoms, uint32_t ngeoms, UNIONFIND *uf, double eps,
  uint32_t minpoints, const GBOX *boxes, char *in_cluster)
{
  assert(geoms); assert(uf); assert(boxes); assert(in_cluster);
  char *is_core = palloc0(sizeof(char) * ngeoms);
  uint32_t *neighbors = palloc(sizeof(uint32_t) * minpoints);
  bool result = true;
  for (uint32_t p = 0; p < ngeoms && result; p++)
  {
    if (lwgeom_is_empty(geoms[p]))
      continue;
    uint32_t nneighbors = 0;
    for (uint32_t q = 0; q < ngeoms; q++)
    {
      if (lwgeom_is_empty(geoms[q]))
        continue;
      if (! cluster_boxes_within(&boxes[p], &boxes[q], eps))
        continue;
      if (nneighbors >= minpoints)
      {
        /* Already one cluster, or the other geometry is at the edge of
         * another, and neither answer changes with the distance */
        if (UF_find(uf, p) == UF_find(uf, q))
          continue;
        if (in_cluster[q] && ! is_core[q])
          continue;
      }
      double distance = lwgeom_mindistance2d_tolerance(geoms[p], geoms[q],
        eps);
      if (distance == FLT_MAX)
      {
        result = false;
        break;
      }
      if (distance > eps)
        continue;
      if (nneighbors < minpoints)
      {
        /* Whether the geometry is at the core of a cluster is not settled, so
         * the neighbour waits for it to be */
        neighbors[nneighbors++] = q;
        if (nneighbors == minpoints)
        {
          /* It is settled, and every neighbour that waited joins it */
          is_core[p] = LW_TRUE;
          in_cluster[p] = LW_TRUE;
          for (uint32_t j = 0; j < nneighbors; j++)
            cluster_union_available(uf, p, neighbors[j], is_core, in_cluster);
        }
      }
      else
        cluster_union_available(uf, p, q, is_core, in_cluster);
    }
  }
  pfree(neighbors); pfree(is_core);
  return result;
}

/*****************************************************************************/

/**
 * @brief Merge the geometries of an array into clusters of geometries that
 * share a point with one another
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of geometries
 * @param[in,out] uf Union-find the clusters are merged into
 * @return False where a pair falls outside what the native engine covers,
 * which leaves the caller to answer it another way
 * @details Two geometries whose bounding boxes stand apart share no point, so
 * the boxes decide which pairs are worth asking about, exactly as they decide
 * which pairs are worth measuring for the clusterings by distance. Sharing a
 * point is symmetric, so each pair is asked about once
 * @note The clustering of PostGIS function @p ST_ClusterIntersecting, without
 * GEOS
 */
bool
geo_union_intersecting(LWGEOM **geoms, uint32_t ngeoms, UNIONFIND *uf)
{
  assert(geoms); assert(uf);
  GBOX *boxes = cluster_boxes(geoms, ngeoms);
  if (! boxes)
    return false;

  /* Each geometry is asked about against many others, and every one of those
   * questions reads the same edges, so the edges are read once per geometry
   * and kept. ⛔ They are read where the FIRST question about that geometry is
   * asked, never up front: the box test below rejects most pairs outright, and
   * a geometry every one of whose pairs it rejects is never read at all, so a
   * walk over well-separated geometries reads none of them */
  void **ctxs = palloc0(sizeof(void *) * ngeoms);
  bool *ctx_read = palloc0(sizeof(bool) * ngeoms);

  bool result = true;
  for (uint32_t p = 0; p < ngeoms && result; p++)
  {
    if (lwgeom_is_empty(geoms[p]))
      continue;
    for (uint32_t q = p + 1; q < ngeoms; q++)
    {
      if (lwgeom_is_empty(geoms[q]))
        continue;
      /* Already the same cluster, and sharing a point does not change it */
      if (UF_find(uf, p) == UF_find(uf, q))
        continue;
      if (! cluster_boxes_within(&boxes[p], &boxes[q], 0.0))
        continue;
      /* The first question asked about a geometry reads its edges, and every
       * later one about the same geometry reads them again from here */
      for (uint32_t k = 0; k < 2; k++)
      {
        uint32_t i = k ? q : p;
        if (! ctx_read[i])
        {
          ctxs[i] = relate_ctx_make(geoms[i]);
          ctx_read[i] = true;
        }
      }
      bool meet;
      if (! meos_spatialrel_ctx(ctxs[p], ctxs[q], INTERSECTS, &meet))
      {
        result = false;
        break;
      }
      if (meet)
        UF_union(uf, p, q);
    }
  }

  for (uint32_t i = 0; i < ngeoms; i++)
    relate_ctx_free(ctxs[i]);
  pfree(ctx_read);
  pfree(ctxs);
  pfree(boxes);
  return result;
}

/*****************************************************************************/

/**
 * @brief Merge the geometries of an array into clusters of geometries that
 * share a point with one another, and assemble each cluster
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of geometries
 * @param[out] clusters Clusters
 * @param[out] nclusters Number of clusters
 * @return False where a pair falls outside what the native engine covers
 * @note The clustering of PostGIS function @p ST_ClusterIntersecting, without
 * GEOS
 */
bool
geo_cluster_intersecting_geoms(LWGEOM **geoms, uint32_t ngeoms,
  LWGEOM ***clusters, uint32_t *nclusters)
{
  assert(geoms); assert(clusters); assert(nclusters);
  UNIONFIND *uf = UF_create(ngeoms);
  if (! geo_union_intersecting(geoms, ngeoms, uf))
  {
    UF_destroy(uf);
    return false;
  }
  bool result = geo_combine_clusters(uf, geoms, ngeoms, clusters, nclusters);
  UF_destroy(uf);
  return result;
}

/*****************************************************************************/

/**
 * @brief Merge the geometries of an array into clusters of geometries lying
 * within a distance of each other
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of geometries
 * @param[in,out] uf Union-find the clusters are merged into
 * @param[in] eps Distance
 * @param[in] minpoints Number of geometries within the distance of a geometry
 * that puts it at the core of a cluster
 * @param[out] in_cluster Whether each geometry belongs to a cluster, which
 * every one does when the minimum is one. May be @p NULL
 * @return False if a distance cannot be computed
 * @note The clustering of PostGIS function @p ST_ClusterDBSCAN, without GEOS
 */
bool
geo_union_dbscan(LWGEOM **geoms, uint32_t ngeoms, UNIONFIND *uf, double eps,
  uint32_t minpoints, char **in_cluster)
{
  assert(geoms); assert(uf);
  char *belongs = palloc0(sizeof(char) * ngeoms);
  if (minpoints <= 1)
    memset(belongs, LW_TRUE, sizeof(char) * ngeoms);

  bool result = true;
  /* Fewer geometries than the minimum leave every one of them alone */
  if (ngeoms > 1 && ngeoms >= minpoints)
  {
    GBOX *boxes = cluster_boxes(geoms, ngeoms);
    if (! boxes)
      result = false;
    else
    {
      result = (minpoints <= 1) ?
        cluster_union_single(geoms, ngeoms, uf, eps, boxes) :
        cluster_union_core(geoms, ngeoms, uf, eps, minpoints, boxes, belongs);
      pfree(boxes);
    }
  }
  if (in_cluster)
    *in_cluster = belongs;
  else
    pfree(belongs);
  return result;
}

/**
 * @brief Assemble each cluster of an array of geometries into one collection
 * @param[in] uf Union-find holding the clusters
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of geometries
 * @param[out] clusters Collections, one per cluster
 * @param[out] nclusters Number of clusters
 * @return False if the union-find and the clusters disagree on their number
 * @note The collections take the geometries
 */
bool
geo_combine_clusters(UNIONFIND *uf, LWGEOM **geoms, uint32_t ngeoms,
  LWGEOM ***clusters, uint32_t *nclusters)
{
  assert(uf); assert(geoms); assert(clusters); assert(nclusters);
  *nclusters = uf->num_clusters;
  *clusters = palloc(sizeof(LWGEOM *) * (*nclusters));
  LWGEOM **members = palloc(sizeof(LWGEOM *) * ngeoms);
  uint32_t *ordered = UF_ordered_by_cluster(uf);
  bool result = true;
  uint32_t nmembers = 0, ncluster = 0;
  for (uint32_t i = 0; i < ngeoms; i++)
  {
    members[nmembers++] = geoms[ordered[i]];
    /* The geometries of one cluster are contiguous, so a different cluster
     * next, or the end of the array, closes this one */
    if (i < ngeoms - 1 &&
        UF_find(uf, ordered[i]) == UF_find(uf, ordered[i + 1]))
      continue;
    if (ncluster >= uf->num_clusters)
    {
      result = false;
      break;
    }
    LWGEOM **components = palloc(sizeof(LWGEOM *) * nmembers);
    memcpy(components, members, sizeof(LWGEOM *) * nmembers);
    (*clusters)[ncluster++] = lwcollection_as_lwgeom(lwcollection_construct(
      COLLECTIONTYPE, components[0]->srid, NULL, nmembers, components));
    nmembers = 0;
  }
  pfree(members); pfree(ordered);
  return result;
}

/**
 * @brief Merge the geometries of an array into clusters of geometries lying
 * within a distance of each other, and assemble each cluster into one
 * collection
 * @param[in] geoms Geometries
 * @param[in] ngeoms Number of geometries
 * @param[in] tolerance Distance
 * @param[out] clusters Collections, one per cluster
 * @param[out] nclusters Number of clusters
 * @return False if a distance cannot be computed
 * @note The clustering of PostGIS function @p ST_ClusterWithin, without GEOS
 */
bool
geo_cluster_within_distance(LWGEOM **geoms, uint32_t ngeoms, double tolerance,
  LWGEOM ***clusters, uint32_t *nclusters)
{
  assert(geoms); assert(clusters); assert(nclusters);
  UNIONFIND *uf = UF_create(ngeoms);
  if (! geo_union_dbscan(geoms, ngeoms, uf, tolerance, 1, NULL))
  {
    UF_destroy(uf);
    return false;
  }
  bool result = geo_combine_clusters(uf, geoms, ngeoms, clusters, nclusters);
  UF_destroy(uf);
  return result;
}

/*****************************************************************************/
