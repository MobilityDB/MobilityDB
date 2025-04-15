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
 * @brief Distance functions for temporal rigid geometries
 */

#include "rgeo/trgeo_vclip.h"

/* C */
#include <assert.h>
#include <c.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <stdio.h>
#include <utils/timestamp.h>
#include <utils/float.h>
/* PostGIS */
#include <lwgeodetic_tree.h>
#include <liblwgeom.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h"
#include "general/type_util.h"
#include "general/meos_catalog.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"

/*****************************************************************************
 * V-clip
 *****************************************************************************/

/**
 * @brief 
 */
static inline uint32_t
uint_mod_add(uint32_t i, uint32_t j, uint32_t n)
{
  return (i + j) % n;
}

/**
 * @brief 
 */
// Handle negative values correctly
// Requirement: j < n
static inline uint32_t
uint_mod_sub(uint32_t i, uint32_t j, uint32_t n)
{
  return (i + n - j) % n;
}

/**
 * @brief 
 */
static void
apply_pose_point4d(POINT4D *p, const Pose *pose)
{
  double c = cos(pose->data[2]);
  double s = sin(pose->data[2]);
  double x = p->x, y = p->y;
  p->x = x * c - y * s + pose->data[0];
  p->y = x * s + y * c + pose->data[1];
  return;
}

/**
 * @brief Computes the relative position of point on segment v(vs, ve)
 * s < 0      -> p before point vs
 * s = 0      -> p = vs
 * 0 < s < 1  -> p = vs * (1 - s)  + ve * s
 * s = 1      -> p = ve
 * 1 < s      -> p after point ve
 */
static inline double
compute_s(POINT4D p, POINT4D vs, POINT4D ve)
{
  return ((p.x - vs.x) * (ve.x - vs.x) + (p.y - vs.y) * (ve.y - vs.y)) /
    (pow(ve.x - vs.x, 2) + pow(ve.y - vs.y, 2));
}

/**
 * @brief Computes the signed length of the cross product of the vectors
 * (vs, p) and (vs, ve)
 * @details The sign of this value determines the relative position between p
 * and the line l going though segment (vs, ve) (oriented towards ve)
 * angle > 0: p is on the right of l
 * angle = 0: p is on l
 * angle < 0: P is on the left of l
 */
static inline double
compute_angle(POINT4D p, POINT4D vs, POINT4D ve)
{
  return (p.x - vs.x) * (ve.y - vs.y) - (p.y - vs.y) * (ve.x - vs.x);
}

/**
 * @brief Computes the distance between point p and segment v(vs, ve)
 *
 * Note: this assumes that the projection of p
 * on the line l going through (vs, ve) is
 * between vs and ve. (0 <= compute_s(p, vs, ve) <= 1)
 */
static inline double
compute_dist2(POINT4D p, POINT4D vs, POINT4D ve)
{
  double s = compute_s(p, vs, ve);
  return pow(p.x - vs.x - (ve.x - vs.x) * s, 2) +
    pow(p.y - vs.y - (ve.y - vs.y) * s, 2);
}

/**
 * @brief Computes the distance between point p and segment v(vs, ve)
 */
static inline double
compute_dist2_safe(POINT4D p, POINT4D vs, POINT4D ve)
{
  double s = compute_s(p, vs, ve);
  if (s <= 0)
    return pow(p.x - vs.x, 2) + pow(p.y - vs.y, 2);
  else if (s >= 1)
    return pow(p.x - ve.x, 2) + pow(p.y - ve.y, 2);
  else
    return pow(p.x - vs.x - (ve.x - vs.x) * s, 2) + pow(p.y - vs.y - (ve.y - vs.y) * s, 2);
}

/**
 * @brief Tests if a polygon is defined in counter-clockwise order (ccw)
 * @result Returns True if it is the case
 * @note The polygon must be convex
 */
static bool
poly_is_ccw(const LWPOLY *poly)
{
  POINT4D v1, v2, v3;
  getPoint4d_p(poly->rings[0], 0, &v1);
  getPoint4d_p(poly->rings[0], 1, &v2);
  getPoint4d_p(poly->rings[0], 2, &v3);
  return compute_angle(v1, v2, v3) < 0;
}

/**
 * @brief 
 */
static int
vertex_vertex_tpoly_point(const LWPOLY *poly, POINT4D point,
  const Pose *pose, uint32_t *poly_feature)
{
  double s_next, s_prev;
  POINT4D v, v_prev, v_next;
  uint32_t n = poly->rings[0]->npoints - 1;
  uint32_t i = *poly_feature / 2;

  /* Get endpoints of previous and next edge */
  getPoint4d_p(poly->rings[0], uint_mod_sub(i, 1, n), &v_prev);
  getPoint4d_p(poly->rings[0], i, &v);
  getPoint4d_p(poly->rings[0], uint_mod_add(i, 1, n), &v_next);
  if (pose)
  {
    apply_pose_point4d(&v_prev, pose);
    apply_pose_point4d(&v, pose);
    apply_pose_point4d(&v_next, pose);
  }

  /* Check if the point is in v's Voronoi region */
  s_prev = compute_s(point, v_prev, v);
  if (s_prev < 1)
  {
    /* Go to the previous edge */
    *poly_feature = uint_mod_sub(*poly_feature, 1, 2*n);
    return MEOS_CONTINUE;
  }
  s_next = compute_s(point, v, v_next);
  if (s_next > 0)
  {
    /* Go to the next edge */
    *poly_feature = uint_mod_add(*poly_feature, 1, 2*n);
    return MEOS_CONTINUE;
  }

  /* We found the closest feature */
  if (s_prev > 1 || s_next < 0)
    return MEOS_DISJOINT;
  /* Point is on the vertex */
  return MEOS_INTERSECT;
}

/**
 * @brief 
 */
static int
edge_vertex_tpoly_point(const LWPOLY *poly, POINT4D point,
  bool ccw_poly, const Pose *pose, uint32_t *poly_feature)
{
  double s, angle;
  POINT4D v_start, v_end;
  uint32_t n = poly->rings[0]->npoints - 1;
  uint32_t i = *poly_feature / 2;

  /* Get edge endpoints */
  getPoint4d_p(poly->rings[0], i, &v_start);
  getPoint4d_p(poly->rings[0], uint_mod_add(i, 1, n), &v_end);
  if (pose)
  {
    apply_pose_point4d(&v_start, pose);
    apply_pose_point4d(&v_end, pose);
  }

  /* Check if the point is in the edge's Voronoi region */
  s = compute_s(point, v_start, v_end);
  if (s < 0)
  {
    /* Go to the start vertex */
    *poly_feature = uint_mod_sub(*poly_feature, 1, 2*n);
    return MEOS_CONTINUE;
  }
  else if (s > 1)
  {
    /* Go to the end vertex */
    *poly_feature = uint_mod_add(*poly_feature, 1, 2*n);
    return MEOS_CONTINUE;
  }

  /* Check for local minimum */
  angle = compute_angle(point, v_start, v_end);
  if ((ccw_poly && angle < 0)
    || (!ccw_poly && angle > 0))
  {
    /* Found local minimum */
    double dmax = -1;
    getPoint4d_p(poly->rings[0], 0, &v_start);
    if (pose)
      apply_pose_point4d(&v_start, pose);
    for (i = 0; i < n; ++i)
    {
      /* Find edge with the largest positive distance
         to the given point */
      double distance = -1;
      getPoint4d_p(poly->rings[0], i + 1, &v_end);
      if (pose)
        apply_pose_point4d(&v_end, pose);
      angle = compute_angle(point, v_start, v_end);
      if ((ccw_poly && angle > 0)
        || (!ccw_poly && angle < 0))
      {
        distance = compute_dist2(point, v_start, v_end);
        if (distance > dmax)
        {
          dmax = distance;
          *poly_feature = 2*i + 1;
        }
      }
      v_start = v_end;
    }

    /* If no positive distance, point is inside polygon */
    if (dmax == -1)
      return MEOS_INTERSECT;
    return MEOS_CONTINUE;
  }

  /* We found the closest feature */
  return MEOS_DISJOINT;
}

/**
 * @brief 
 */
int
v_clip_tpoly_point(const LWPOLY *poly, const LWPOINT *point,
  const Pose *pose, uint32_t *poly_feature, double *dist)
{
  int result;
  int loop = 0;
  bool ccw_poly = poly_is_ccw(poly);
  POINT4D pt;
  lwpoint_getPoint4d_p(point, &pt);

  do
  {
    if (*poly_feature % 2 == 0) /* poly_feature is a vertex */
      result = vertex_vertex_tpoly_point(poly, pt, pose, poly_feature);
    else /* poly_feature is an edge */
      result = edge_vertex_tpoly_point(poly, pt, ccw_poly, pose, poly_feature);

    if (loop++ == MEOS_MAX_ITERS) break;

  } while (result == MEOS_CONTINUE);

  if (loop > MEOS_MAX_ITERS)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, 
      "V-clip: Cycle detected, current feature: %d", *poly_feature);

  if (dist && result == MEOS_DISJOINT)
  {
    /* compute the distance */
    if (*poly_feature % 2 == 0)
    {
      POINT4D v;
      uint32_t i = *poly_feature / 2;
      getPoint4d_p(poly->rings[0], i, &v);
      if (pose)
        apply_pose_point4d(&v, pose);
      *dist = sqrt(pow(pt.x - v.x, 2) + pow(pt.y - v.y, 2));
    }
    else
    {
      POINT4D v_start, v_end;
      uint32_t i = *poly_feature / 2;
      getPoint4d_p(poly->rings[0], i, &v_start);
      getPoint4d_p(poly->rings[0], i + 1, &v_end);
      if (pose)
      {
        apply_pose_point4d(&v_start, pose);
        apply_pose_point4d(&v_end, pose);
      }
      *dist = sqrt(compute_dist2(pt, v_start, v_end));
    }
  }
  return MEOS_DISJOINT;
}

/**
 * @brief 
 */
static int
vertex_vertex_tpoly_tpoly(const LWPOLY *poly1, const LWPOLY *poly2,
  const Pose *pose1, const Pose *pose2, uint32_t *poly1_feature,
  uint32_t *poly2_feature)
{
  double s1_next, s1_prev, s2_prev, s2_next;
  POINT4D v1, v1_prev, v1_next, v2, v2_prev, v2_next;
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  uint32_t i1 = *poly1_feature / 2;
  uint32_t i2 = *poly2_feature / 2;

  /* Get endpoints of previous and next edges */
  /* poly1 */
  getPoint4d_p(poly1->rings[0], uint_mod_sub(i1, 1, n1), &v1_prev);
  getPoint4d_p(poly1->rings[0], i1, &v1);
  getPoint4d_p(poly1->rings[0], uint_mod_add(i1, 1, n1), &v1_next);
  if (pose1)
  {
    apply_pose_point4d(&v1_prev, pose1);
    apply_pose_point4d(&v1, pose1);
    apply_pose_point4d(&v1_next, pose1);
  }
  /* poly2 */
  getPoint4d_p(poly2->rings[0], uint_mod_sub(i2, 1, n2), &v2_prev);
  getPoint4d_p(poly2->rings[0], i2, &v2);
  getPoint4d_p(poly2->rings[0], uint_mod_add(i2, 1, n2), &v2_next);
  if (pose2)
  {
    apply_pose_point4d(&v2_prev, pose2);
    apply_pose_point4d(&v2, pose2);
    apply_pose_point4d(&v2_next, pose2);
  }

  /* Check if v2 is in v1's Voronoi region */
  s1_prev = compute_s(v2, v1_prev, v1);
  if (s1_prev + MEOS_EPSILON < 1)
  {
    /* Go to the previous edge */
    *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2*n1);
    return MEOS_CONTINUE;
  }
  s1_next = compute_s(v2, v1, v1_next);
  if (s1_next - MEOS_EPSILON > 0)
  {
    /* Go to the next edge */
    *poly1_feature = uint_mod_add(*poly1_feature, 1, 2*n1);
    return MEOS_CONTINUE;
  }
  /* Check if v1 is in v2's Voronoi region */
  s2_prev = compute_s(v1, v2_prev, v2);
  if (s2_prev + MEOS_EPSILON < 1)
  {
    /* Go to the previous edge */
    *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2*n2);
    return MEOS_CONTINUE;
  }
  s2_next = compute_s(v1, v2, v2_next);
  if (s2_next - MEOS_EPSILON > 0)
  {
    /* Go to the next edge */
    *poly2_feature = uint_mod_add(*poly2_feature, 1, 2*n2);
    return MEOS_CONTINUE;
  }

  /* We found the closest feature */
  if ((s1_prev > 1 || s1_next < 0)
    && (s2_prev > 1 || s2_next < 0))
    return MEOS_DISJOINT;
  /* Point is on the vertex */
  return MEOS_INTERSECT;
}

/**
 * @brief 
 */
static int
edge_vertex_tpoly_tpoly(const LWPOLY *poly1, const LWPOLY *poly2,
  bool ccw_poly1, const Pose *pose1, const Pose *pose2,
  uint32_t *poly1_feature, uint32_t *poly2_feature)
{
  double s1, angle1, s2_prev, s2_next;
  POINT4D v1, v1_start, v1_end, v2, v2_prev, v2_next;
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  uint32_t i1 = *poly1_feature / 2;
  uint32_t i2 = *poly2_feature / 2;

  /* Get edge endpoints of edge of poly1 */
  getPoint4d_p(poly1->rings[0], i1, &v1_start);
  getPoint4d_p(poly1->rings[0], uint_mod_add(i1, 1, n1), &v1_end);
  if (pose1)
  {
    apply_pose_point4d(&v1_start, pose1);
    apply_pose_point4d(&v1_end, pose1);
  }
  /* Get endpoints of previous and next edges of poly2 */
  getPoint4d_p(poly2->rings[0], uint_mod_sub(i2, 1, n2), &v2_prev);
  getPoint4d_p(poly2->rings[0], i2, &v2);
  getPoint4d_p(poly2->rings[0], uint_mod_add(i2, 1, n2), &v2_next);
  if (pose2)
  {
    apply_pose_point4d(&v2_prev, pose2);
    apply_pose_point4d(&v2, pose2);
    apply_pose_point4d(&v2_next, pose2);
  }

  /* Check if v2 is in the Voronoi region of the edge of poly1 */
  s1 = compute_s(v2, v1_start, v1_end);
  if (s1 + MEOS_EPSILON < 0)
  {
    /* Go to the start vertex */
    *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2*n1);
    return MEOS_CONTINUE;
  }
  else if (s1 - MEOS_EPSILON > 1)
  {
    /* Go to the end vertex */
    *poly1_feature = uint_mod_add(*poly1_feature, 1, 2*n1);
    return MEOS_CONTINUE;
  }

  /* Check for local minimum */
  angle1 = compute_angle(v2, v1_start, v1_end);
  if ((ccw_poly1 && angle1 < 0)
    || (!ccw_poly1 && angle1 > 0))
  {
    /* Found local minimum */
    double dmax = -1;
    getPoint4d_p(poly1->rings[0], 0, &v1_start);
    if (pose1)
      apply_pose_point4d(&v1_start, pose1);
    for (i1 = 0; i1 < n1; ++i1)
    {
      /* Find edge of poly1 with the largest
         positive distance to v2 */
      double distance = -1;
      getPoint4d_p(poly1->rings[0], i1 + 1, &v1_end);
      if (pose1)
        apply_pose_point4d(&v1_end, pose1);
      angle1 = compute_angle(v2, v1_start, v1_end);
      if ((ccw_poly1 && angle1 > 0)
        || (!ccw_poly1 && angle1 < 0))
      {
        distance = compute_dist2(v2, v1_start, v1_end);
        if (distance > dmax)
        {
          dmax = distance;
          *poly1_feature = 2*i1 + 1;
        }
      }
      v1_start = v1_end;
    }

    /* If no positive distance, point is inside polygon */
    if (dmax == -1)
      return MEOS_INTERSECT;
    return MEOS_CONTINUE;
  }

  /* Compute the point on poly1 closest to v2 */
  v1.x = v1_start.x * (1 - s1) + v1_end.x * s1;
  v1.y = v1_start.y * (1 - s1) + v1_end.y * s1;
  /* Check if v1 is in v2's Voronoi region */
  s2_prev = compute_s(v1, v2_prev, v2);
  if (s2_prev + MEOS_EPSILON < 1)
  {
    *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2*n2);
    return MEOS_CONTINUE;
  }
  s2_next = compute_s(v1, v2, v2_next);
  if (s2_next - MEOS_EPSILON > 0)
  {
    *poly2_feature = uint_mod_add(*poly2_feature, 1, 2*n2);
    return MEOS_CONTINUE;
  }

  /* We found the closest feature */
  return MEOS_DISJOINT;
}

/**
 * @brief 
 */
static int
edge_edge_tpoly_tpoly(const LWPOLY *poly1, const LWPOLY *poly2,
  const Pose *pose1, const Pose *pose2, uint32_t *poly1_feature,
  uint32_t *poly2_feature)
{
  double d1_start, d1_end, d2_start, d2_end;
  POINT4D v1_start, v1_end, v2_start, v2_end;
  uint32_t n1 = poly1->rings[0]->npoints - 1;
  uint32_t n2 = poly2->rings[0]->npoints - 1;
  uint32_t i1 = *poly1_feature / 2;
  uint32_t i2 = *poly2_feature / 2;

  /* Get edge endpoints of edge of poly1 */
  getPoint4d_p(poly1->rings[0], i1, &v1_start);
  getPoint4d_p(poly1->rings[0], uint_mod_add(i1, 1, n1), &v1_end);
  if (pose1)
  {
    apply_pose_point4d(&v1_start, pose1);
    apply_pose_point4d(&v1_end, pose1);
  }
  /* Get edge endpoints of edge of poly2 */
  getPoint4d_p(poly2->rings[0], i2, &v2_start);
  getPoint4d_p(poly2->rings[0], uint_mod_add(i2, 1, n2), &v2_end);
  if (pose2)
  {
    apply_pose_point4d(&v2_start, pose2);
    apply_pose_point4d(&v2_end, pose2);
  }

  /* Check if the edges intersect */
  if (compute_angle(v1_start, v2_start, v2_end) * 
        compute_angle(v1_end, v2_start, v2_end) < 0 && 
      compute_angle(v2_start, v1_start, v1_end) * 
        compute_angle(v2_end, v1_start, v1_end) < 0)
    return MEOS_INTERSECT;

  /* Compute distances of each vertex to opposing edge */
  d1_start = compute_dist2_safe(v1_start, v2_start, v2_end);
  d1_end = compute_dist2_safe(v1_end, v2_start, v2_end);
  d2_start = compute_dist2_safe(v2_start, v1_start, v1_end);
  d2_end = compute_dist2_safe(v2_end, v1_start, v1_end);

  /* Update vertex with the smallest distance */
  if (d1_start <= d1_end && d1_start <= d2_start && d1_start <= d2_end)
    *poly1_feature = uint_mod_sub(*poly1_feature, 1, 2*n1);
  else if (d1_end <= d2_start && d1_end <= d2_end)
    *poly1_feature = uint_mod_add(*poly1_feature, 1, 2*n1);
  else if (d2_start <= d2_end)
    *poly2_feature = uint_mod_sub(*poly2_feature, 1, 2*n2);
  else
    *poly2_feature = uint_mod_add(*poly2_feature, 1, 2*n2);

  return MEOS_CONTINUE;
}

/**
 * @brief 
 */
int
v_clip_tpoly_tpoly(const LWPOLY *poly1, const LWPOLY *poly2,
  const Pose *pose1, const Pose *pose2, uint32_t *poly1_feature,
  uint32_t *poly2_feature, double *dist)
{
  int result;
  int loop = 0;
  bool ccw_poly1 = poly_is_ccw(poly1);
  bool ccw_poly2 = poly_is_ccw(poly2);

  do
  {
    if (*poly1_feature % 2 == 0 && *poly2_feature % 2 == 0) /* vertex <-> vertex */
      result = vertex_vertex_tpoly_tpoly(poly1, poly2, pose1, pose2, poly1_feature, poly2_feature);
    else if (*poly1_feature % 2 == 0) /* vertex <-> edge */
      result = edge_vertex_tpoly_tpoly(poly2, poly1, ccw_poly2, pose2, pose1, poly2_feature, poly1_feature);
    else if (*poly2_feature % 2 == 0) /* edge <-> vertex */
      result = edge_vertex_tpoly_tpoly(poly1, poly2, ccw_poly1, pose1, pose2, poly1_feature, poly2_feature);
    else /* edge <-> edge */
      result = edge_edge_tpoly_tpoly(poly1, poly2, pose1, pose2, poly1_feature, poly2_feature);
      // meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "V-clip: Invalid combination of current features: (%d, %d)", *poly1_feature, *poly2_feature);

    if (loop++ == MEOS_MAX_ITERS) break;

  } while (result == MEOS_CONTINUE);

  if (loop > MEOS_MAX_ITERS)
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "V-clip: Cycle detected, current features: (%d, %d)", *poly1_feature, *poly2_feature);
    // return result;

  if (dist && result == MEOS_DISJOINT)
  {
    /* compute the distance */
    if (*poly1_feature % 2 == 0 && *poly2_feature % 2 == 0) /* vertex <-> vertex */
    {
      POINT4D v1, v2;
      uint32_t i1 = *poly1_feature / 2;
      uint32_t i2 = *poly2_feature / 2;
      getPoint4d_p(poly1->rings[0], i1, &v1);
      if (pose1)
        apply_pose_point4d(&v1, pose1);
      getPoint4d_p(poly2->rings[0], i2, &v2);
      if (pose2)
        apply_pose_point4d(&v2, pose2);
      *dist = sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
    }
    else if (*poly1_feature % 2 == 0) /* vertex <-> edge */
    {
      POINT4D v1, v2_start, v2_end;
      uint32_t i1 = *poly1_feature / 2;
      uint32_t i2 = *poly2_feature / 2;
      getPoint4d_p(poly1->rings[0], i1, &v1);
      if (pose1)
        apply_pose_point4d(&v1, pose1);
      getPoint4d_p(poly2->rings[0], i2, &v2_start);
      getPoint4d_p(poly2->rings[0], i2 + 1, &v2_end);
      if (pose2)
      {
        apply_pose_point4d(&v2_start, pose2);
        apply_pose_point4d(&v2_end, pose2);
      }
      *dist = sqrt(compute_dist2(v1, v2_start, v2_end));
    }
    else if (*poly2_feature % 2 == 0) /* edge <-> vertex */
    {
      POINT4D v1_start, v1_end, v2;
      uint32_t i1 = *poly1_feature / 2;
      uint32_t i2 = *poly2_feature / 2;
      getPoint4d_p(poly1->rings[0], i1, &v1_start);
      getPoint4d_p(poly1->rings[0], i1 + 1, &v1_end);
      if (pose1)
      {
        apply_pose_point4d(&v1_start, pose1);
        apply_pose_point4d(&v1_end, pose1);
      }
      getPoint4d_p(poly2->rings[0], i2, &v2);
      if (pose2)
        apply_pose_point4d(&v2, pose2);
      *dist = sqrt(compute_dist2(v2, v1_start, v1_end));
    }
    else
      meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE, "V-clip: Invalid combination of current features: (%d, %d)", *poly1_feature, *poly2_feature);
  }
  return result;
}

/*****************************************************************************/
