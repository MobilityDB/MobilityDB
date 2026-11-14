/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @details The temporal distance comes from the closest-feature walk of
 * trgeo_distwalk.c, which needs convex bodies, and is the turning point
 * function of a lift of lifting.c over the segments of a sequence.  See
 * meos/src/rgeo/trgeo_distance.txt for the algorithm.
 */

#include "rgeo/trgeo_distance.h"

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "temporal/temporal_aggfuncs.h"
#include "temporal/temporal_compops.h"
#include "temporal/lifting.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_distance.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"
#include "rgeo/trgeo_distwalk.h"

/*****************************************************************************
 * Bodies
 *****************************************************************************/

/**
 * @brief Set the body of the reference geometry of a temporal rigid geometry
 * @details The walk needs a convex body.  A polygon that is not convex or has
 * holes gives an error, also where the walk is not necessary, so that the
 * distance functions accept the same values in all cases.
 * @return False, with an error, if the reference geometry is not a convex
 * polygon without holes
 */
static bool
dist_ref_body(const GSERIALIZED *ref_gs, DistRefPoly *rp)
{
  LWGEOM *geom = lwgeom_from_gserialized(ref_gs);
  bool result = distrefpoly_set(rp, lwgeom_as_lwpoly(geom));
  lwgeom_free(geom);
  if (! result)
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The distance of a temporal rigid geometry needs a reference geometry "
      "that is a convex polygon without holes");
  return result;
}

/**
 * @brief A convex part of a static geometry
 */
typedef struct
{
  DistRefPoly body;          /**< Point, segment or convex polygon */
  DistMotion motion;         /**< Static motion at the center of the body */
  double *xy;                /**< Vertices of the part as in the geometry,
                                  x, y pairs, the first not repeated */
  int npts;                  /**< Number of vertices */
} DistPart;

/**
 * @brief Set the vertices of a part to the @p n points of @p pa from @p i
 */
static void
dist_part_points(DistPart *part, const POINTARRAY *pa, uint32_t i, int n)
{
  part->xy = palloc(sizeof(double) * 2 * n);
  part->npts = n;
  for (int k = 0; k < n; k++)
  {
    const POINT2D *p = getPoint2d_cp(pa, i + (uint32_t) k);
    part->xy[2 * k] = p->x;
    part->xy[2 * k + 1] = p->y;
  }
  return;
}

/**
 * @brief Add a point part to an array of parts
 */
static void
dist_part_point(DistPart *parts, int *n, double x, double y)
{
  distrefpoly_set_point(&parts[*n].body);
  distmotion_set_static(&parts[*n].motion, x, y);
  parts[*n].xy = palloc(sizeof(double) * 2);
  parts[*n].xy[0] = x;
  parts[*n].xy[1] = y;
  parts[*n].npts = 1;
  (*n)++;
}

/**
 * @brief Add the segments of the points @p pa, a line or a ring of a polygon,
 * to an array of parts
 * @details A segment of length zero is a point, as is a line of one point.
 */
static void
dist_parts_add_points(const POINTARRAY *pa, DistPart *parts, int *n)
{
  for (uint32_t i = 0; i + 1 < pa->npoints; i++)
  {
    const POINT2D *p = getPoint2d_cp(pa, i);
    const POINT2D *q = getPoint2d_cp(pa, i + 1);
    if (p->x == q->x && p->y == q->y)
      dist_part_point(parts, n, p->x, p->y);
    else
    {
      double cx, cy;
      distrefpoly_set_segment(&parts[*n].body, p->x, p->y, q->x, q->y, &cx,
        &cy);
      distmotion_set_static(&parts[*n].motion, cx, cy);
      dist_part_points(&parts[*n], pa, i, 2);
      (*n)++;
    }
  }
  if (pa->npoints == 1)
  {
    const POINT2D *p = getPoint2d_cp(pa, 0);
    dist_part_point(parts, n, p->x, p->y);
  }
  return;
}

/**
 * @brief Add the parts of a simple geometry to an array of parts
 * @details A point is one part and a line is one part for each segment.  A
 * convex polygon without holes is one part.  Any other polygon is one part
 * for each segment of its rings, and is added to @p polys, whose area
 * #dist_inside() reads.
 */
static void
dist_parts_add(const LWGEOM *geom, DistPart *parts, int *n,
  const LWPOLY **polys, int *npolys)
{
  if (lwgeom_is_empty(geom))
    return;
  if (geom->type == POINTTYPE)
  {
    const POINT2D *p = getPoint2d_cp(((LWPOINT *) geom)->point, 0);
    dist_part_point(parts, n, p->x, p->y);
    return;
  }
  if (geom->type == LINETYPE)
  {
    dist_parts_add_points(((LWLINE *) geom)->points, parts, n);
    return;
  }
  /* POLYGONTYPE */
  const LWPOLY *poly = (LWPOLY *) geom;
  double cx, cy;
  if (distrefpoly_set_static(&parts[*n].body, poly, &cx, &cy))
  {
    distmotion_set_static(&parts[*n].motion, cx, cy);
    const POINTARRAY *ring = poly->rings[0];
    dist_part_points(&parts[*n], ring, 0, (int) ring->npoints - 1);
    (*n)++;
    return;
  }
  for (uint32_t r = 0; r < poly->nrings; r++)
    dist_parts_add_points(poly->rings[r], parts, n);
  polys[(*npolys)++] = poly;
  return;
}

/**
 * @brief Free an array of parts
 */
static void
dist_parts_free(DistPart *parts, int count)
{
  for (int i = 0; i < count; i++)
  {
    distrefpoly_free(&parts[i].body);
    pfree(parts[i].xy);
  }
  pfree(parts);
}

/**
 * @brief Add the parts of a geometry to an array of parts, reading the
 * members of a multi-geometry or a geometry collection, nested collections
 * included, as #dist_parts_add() reads a simple geometry
 * @return False, with an error, if the type of a member is not supported
 */
static bool
dist_parts_add_all(const LWGEOM *geom, DistPart *parts, int *n,
  const LWPOLY **polys, int *npolys)
{
  uint32_t type = geom->type;
  if (type == MULTIPOINTTYPE || type == MULTILINETYPE ||
      type == MULTIPOLYGONTYPE || type == COLLECTIONTYPE)
  {
    const LWCOLLECTION *coll = lwgeom_as_lwcollection(geom);
    for (uint32_t i = 0; i < coll->ngeoms; i++)
    {
      if (! dist_parts_add_all(coll->geoms[i], parts, n, polys, npolys))
        return false;
    }
    return true;
  }
  if (type != POINTTYPE && type != LINETYPE && type != POLYGONTYPE)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "Unsupported geometry type: %s", lwtype_name(type));
    return false;
  }
  dist_parts_add(geom, parts, n, polys, npolys);
  return true;
}

/*****************************************************************************
 * Distance at an instant
 *****************************************************************************/

/**
 * @brief The operands of a temporal distance
 * @details The first operand is always a temporal rigid geometry.  The second
 * operand is a static geometry, a temporal point or a temporal rigid
 * geometry.
 */
typedef struct
{
  const GSERIALIZED *ref1;   /**< Reference geometry of the first operand */
  DistRefPoly body1;         /**< Body of the first operand */
  const GSERIALIZED *gs;     /**< Static geometry @nullable */
  LWGEOM *geom;              /**< Static geometry, which @p polys read */
  DistPart *parts;           /**< Parts of the static geometry */
  int nparts;                /**< Number of parts */
  const LWPOLY **polys;      /**< Polygons of the static geometry read by
                                  the segments of their rings */
  int npolys;                /**< Number of such polygons */
  double *refxy;             /**< Vertices of the reference geometry of the
                                  first operand, x, y pairs, the first not
                                  repeated, for a static geometry */
  int nref;                  /**< Number of vertices */
  const GSERIALIZED *ref2;   /**< Reference geometry of a temporal rigid
                                  geometry, or NULL for a temporal point */
  DistRefPoly body2;         /**< Body of a temporal point or temporal rigid
                                  geometry */
  double level;              /**< Distance whose crossings are events, or a
                                  negative value, see distwalk_segm() */
} DistOps;

/**
 * @brief Return true if a vertex of the first operand at the pose @p pose
 * lies in a polygon of the static geometry of @p ops read by its rings
 * @details A body with a vertex in such a polygon meets it, and its distance
 * is 0.  A body with no vertex in it and no point on its rings, at a
 * positive distance from every segment of its rings, lies outside it, since
 * it cannot pass from the interior to the exterior without meeting a ring.
 * Thus the distance of the body to the polygon is 0 or the least of the
 * distances to the segments of its rings.  One vertex decides it, located
 * exactly (#translated_point_in_rings_exact): at the angle 0, the vertex is
 * the exact sum of its coordinates and the position, as
 * #convex_motion_nad_exact reads it; at another angle, it is the vertex of the
 * body placed at the pose (#lwgeom_apply_pose), as #dist_pose_geo() places it.
 */
static bool
dist_inside(const DistOps *ops, const Pose *pose)
{
  if (ops->npolys == 0)
    return false;
  double x = ops->refxy[0], y = ops->refxy[1];
  double tx = pose->data[0], ty = pose->data[1];
  if (pose->data[2] != 0.0)
  {
    LWPOINT *pt = lwpoint_make2d(SRID_UNKNOWN, x, y);
    lwgeom_apply_pose(pose, (LWGEOM *) pt);
    const POINT2D *p = getPoint2d_cp(pt->point, 0);
    x = p->x;
    y = p->y;
    tx = ty = 0.0;
    lwpoint_free(pt);
  }
  for (int i = 0; i < ops->npolys; i++)
  {
    if (translated_point_in_rings_exact(x, y, tx, ty, ops->polys[i]->rings,
        ops->polys[i]->nrings) != 0)
      return true;
  }
  return false;
}

/**
 * @brief Return the nearest approach of the first operand, translating from
 * the pose @p pose1 to the pose @p pose2 at the angle 0, and the static
 * geometry of @p ops, the double nearest its exact value
 * @details The least of the nearest approaches to the parts
 * (#convex_motion_nad_exact), or 0 where the body at @p pose1 has a vertex
 * in a polygon read by its rings (#dist_inside()).  A body that enters such
 * a polygon meets one of its rings, whose segments are parts.
 */
static double
dist_parts_nad(const DistOps *ops, const Pose *pose1, const Pose *pose2)
{
  if (dist_inside(ops, pose1))
    return 0.0;
  double result = DBL_MAX;
  for (int i = 0; i < ops->nparts; i++)
    result = fmin(result, convex_motion_nad_exact(ops->refxy, ops->nref,
      pose1->data, pose2->data, ops->parts[i].xy, ops->parts[i].npts));
  return result;
}

/**
 * @brief Return the distance between the first operand at the pose @p pose
 * and the static geometry of @p ops
 * @details A body at the angle 0 is its reference geometry moved by the
 * position of the pose, and its distance to the geometry is the least of the
 * distances to the parts, each the double nearest its exact value
 * (#convex_motion_nad_exact) on the exact sums of the coordinates and the
 * position, or 0 in a polygon read by its rings (#dist_parts_nad()).  Where
 * no such sum rounds (#translation_is_exact), the placed body is that body,
 * and its distance is the one of #geom_distance2d().  Otherwise, and for a
 * body that turns, the distance is the one of the body placed at the pose,
 * 0 where it has a vertex in a polygon read by its rings (#dist_inside()).
 */
static double
dist_pose_geo(const DistOps *ops, const Pose *pose)
{
  if (ops->refxy && pose->data[2] == 0.0 &&
      ! translation_is_exact(ops->refxy, ops->nref, pose->data))
    return dist_parts_nad(ops, pose, pose);
  if (ops->refxy && dist_inside(ops, pose))
    return 0.0;
  GSERIALIZED *geo = pose_apply_geo(pose, ops->ref1);
  double result = geom_distance2d(geo, ops->gs);
  pfree(geo);
  return result;
}

/**
 * @brief Return the distance between the first operand at the pose @p pose1
 * and the second operand
 * @details The function places the bodies at their poses and returns the
 * double nearest the distance of the placed geometries, see
 * #geom_distance2d().  The distance to a static geometry is the one of
 * #dist_pose_geo().  The second operand is the static geometry of @p ops,
 * the point @p point2 of a temporal point, or the body of a temporal rigid
 * geometry at the pose @p pose2.
 * @param[in] pose2 Pose of a temporal rigid geometry, or NULL @nullable
 * @param[in] point2 Point of a temporal point, or NULL @nullable
 */
static double
dist_placed(const DistOps *ops, const Pose *pose1, const Pose *pose2,
  const GSERIALIZED *point2)
{
  if (ops->gs)
    return dist_pose_geo(ops, pose1);
  GSERIALIZED *geo1 = pose_apply_geo(pose1, ops->ref1);
  double result;
  if (! ops->ref2)
    result = geom_distance2d(geo1, point2);
  else
  {
    GSERIALIZED *geo2 = pose_apply_geo(pose2, ops->ref2);
    result = geom_distance2d(geo1, geo2);
    pfree(geo2);
  }
  pfree(geo1);
  return result;
}

/**
 * @brief Return the distance between the operands with the value @p value1
 * of the first operand and @p value2 of the second operand
 * @details @p value2 is a pose for a temporal rigid geometry, a point for a
 * temporal point, and is not read for a static geometry.
 */
static double
dist_values(const DistOps *ops, Datum value1, Datum value2)
{
  const Pose *pose1 = DatumGetPoseP(value1);
  if (ops->gs)
    return dist_placed(ops, pose1, NULL, NULL);
  if (! ops->ref2)
    return dist_placed(ops, pose1, NULL, DatumGetGserializedP(value2));
  return dist_placed(ops, pose1, DatumGetPoseP(value2), NULL);
}

/**
 * @brief Return the distance between the operands at the instant @p inst1 of
 * the first operand and @p inst2 of the second operand
 * @details @p inst2 is NULL for a static geometry.
 */
static double
dist_instant(const DistOps *ops, const TInstant *inst1, const TInstant *inst2)
{
  const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
  if (! inst2)
    return dist_placed(ops, pose1, NULL, NULL);
  return dist_values(ops, tinstant_value_p(inst1), tinstant_value_p(inst2));
}

/*****************************************************************************
 * Distance of a sequence
 *****************************************************************************/

/**
 * @brief Set the motion in segment @p i of a sequence of poses or points
 * @details With step interpolation, the value keeps its value at instant
 * @p i during the segment.  A point is a body without rotation.
 */
static void
dist_segmotion(DistMotion *m, const TSequence *seq, int i, bool ispose)
{
  int j = MEOS_FLAGS_LINEAR_INTERP(seq->flags) ? i + 1 : i;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, j);
  if (ispose)
  {
    distmotion_from_pose(m, DatumGetPoseP(tinstant_value_p(inst1)),
      DatumGetPoseP(tinstant_value_p(inst2)));
    return;
  }
  const POINT2D *p1 = GSERIALIZED_POINT2D_P(
    DatumGetGserializedP(tinstant_value_p(inst1)));
  const POINT2D *p2 = GSERIALIZED_POINT2D_P(
    DatumGetGserializedP(tinstant_value_p(inst2)));
  distmotion_set(m, p1->x, p1->y, 0.0, p2->x, p2->y, 0.0);
}

/**
 * @brief Return the instant of an event at the fraction @p ratio of the
 * segment from @p lower to @p upper
 * @details An event at the end of the segment is at @p upper.
 */
static TimestampTz
dist_event_time(TimestampTz lower, TimestampTz upper, double ratio)
{
  if (ratio <= 0.0)
    return lower;
  if (ratio >= 1.0)
    return upper;
  /* The nearest microsecond */
  return lower + (TimestampTz) llround((double) (upper - lower) * ratio);
}

/*****************************************************************************
 * Distance to a geometry of several parts: the lower envelope
 *****************************************************************************/

/**
 * @brief Set @p h to a function whose sign is the sign of the distance of
 * the piece @p o minus the distance of the piece @p c, and @p htol to the
 * tolerance of its values
 * @details The distance of a vertex is sqrt(V) and the distance of an edge is
 * G / L, with G not negative in the region of the edge, see #DistPiece.
 * Thus, with the squares and the lengths that keep the sign,
 * @code
 * vertex, vertex   Vo - Vc
 * edge, edge       Lc * Go - Lo * Gc
 * vertex, edge     Lc^2 * Vo - Gc^2
 * edge, vertex     Go^2 - Lo^2 * Vc
 * @endcode
 * The squares have the frequencies w and 2w of the rotation w of the body
 * and quadratic coefficients, see #distfun_square().  The tolerance is the
 * one of #distwalk_ftol() for the size of the values of the two sides.
 * @pre Neither piece is an overlap
 * @return False if a function is not in the family
 */
static bool
dist_piece_diff(const DistPiece *o, const DistPiece *c, double lo, double hi,
  DistFun *h, double *htol)
{
  DistFun a, b;
  if (o->len == 0.0 && c->len == 0.0)
  {
    a = o->f;
    b = c->f;
  }
  else if (o->len > 0.0 && c->len > 0.0)
  {
    distfun_init(&a);
    distfun_init(&b);
    if (! distfun_add_fun(&a, &o->f, c->len) ||
        ! distfun_add_fun(&b, &c->f, o->len))
      return false;
  }
  else if (o->len == 0.0)
  {
    distfun_init(&a);
    if (! distfun_add_fun(&a, &o->f, c->len * c->len) ||
        ! distfun_square(&c->f, &b))
      return false;
  }
  else
  {
    distfun_init(&b);
    if (! distfun_square(&o->f, &a) ||
        ! distfun_add_fun(&b, &c->f, o->len * o->len))
      return false;
  }
  distfun_init(h);
  if (! distfun_add_fun(h, &a, 1.0) || ! distfun_add_fun(h, &b, -1.0))
    return false;
  *htol = 1e-14 * (distfun_absmax(&a, lo, hi) + distfun_absmax(&b, lo, hi));
  return true;
}

/**
 * @brief Return -1 if the piece @p o is closer than the piece @p c just
 * after @p t, 1 if it is farther, and 0 if they are at the same distance
 * @details An overlap is at the distance zero.  Two pieces whose difference
 * is zero on the interval are at the same distance, as two parts that share
 * the vertex closest to the body.
 * @param[out] ok False if a function is not in the family
 */
static int
dist_piece_cmp(const DistPiece *o, const DistPiece *c, double t, double hi,
  bool *ok)
{
  *ok = true;
  if (o->zero || c->zero)
    return o->zero ? (c->zero ? 0 : -1) : 1;
  DistFun h;
  double htol;
  if (! dist_piece_diff(o, c, t, hi, &h, &htol))
  {
    *ok = false;
    return 0;
  }
  return distfun_sign_after(&h, 0.0, t, htol);
}

/**
 * @brief An interval of a segment in which one part is the closest
 */
typedef struct
{
  double t0, t1;   /**< Interval, in [0, 1] */
  int part;        /**< Part */
} DistActive;

/**
 * @brief Return the piece of the walk @p ev in which @p t is, from the piece
 * @p *from on, which it advances
 */
static const DistPiece *
dist_piece_at(const DistEvents *ev, int *from, double t)
{
  while (*from < ev->npc - 1 && ev->pc[*from].t1 <= t)
    (*from)++;
  return &ev->pc[*from];
}

/**
 * @brief Set @p active to the intervals of a segment in which each part of
 * the geometry is the closest, and @p corners to the times at which the
 * closest part changes
 * @details The distance to the geometry is the least of the distances to its
 * parts.  In an interval in which every part keeps one piece, the closest
 * part changes where the distance of another part crosses its own, a root of
 * #dist_piece_diff(), which #distfun_first_crossing() isolates with its
 * proof.  At the start of each piece, the closest part is the one that is
 * closest just after it.  Each change is a corner of the distance, a local
 * maximum, whose time is the root, not where two interpolations cross.
 *
 * The comparison just after a time reads the terms of the Taylor series of
 * the difference there, within its tolerance, see #distfun_sign_after(),
 * while the crossings read the signs between the roots.  Where two distances
 * meet with the same slope, as where the nearest point of one part leaves a
 * vertex it shares with another part for its edge, the value of the
 * difference at the root can be a rounding error above the tolerance, so
 * the comparison keeps the closest part while the crossing of another part
 * is at that time.  Such a part is the closest just after the time.  A part
 * that was the closest at a time does not become it again at that time, thus
 * the time advances after at most as many changes as there are parts.
 * @param[in] ev Events and pieces of the walks of the parts, which cover
 * [0, 1]
 * @return False if a function is not in the family or a root isolation fails
 */
static bool
dist_envelope(const DistEvents *ev, int nparts, DistActive **active,
  int *nactive, double **corners, int *ncorners)
{
  int *at = palloc0(sizeof(int) * nparts);
  int size = 8, csize = 8;
  *active = palloc(sizeof(DistActive) * size);
  *corners = palloc(sizeof(double) * csize);
  *nactive = *ncorners = 0;
  /* The parts that were the closest at t, and the part whose crossing is at
   * t */
  bool *taken = palloc0(sizeof(bool) * nparts);
  int force = -1;
  double t = 0.0;
  int cur = -1;
  bool ok = true;
  while (ok && t < 1.0)
  {
    /* The end of the pieces in which t is */
    double end = 1.0;
    for (int k = 0; k < nparts; k++)
    {
      double t1 = dist_piece_at(&ev[k], &at[k], t)->t1;
      /* The last piece of a walk ends at 1 */
      if (t1 > t)
        end = fmin(end, t1);
    }
    /* The closest part just after t */
    int best = (force >= 0) ? force : ((cur >= 0) ? cur : 0);
    for (int k = 0; k < nparts && ok; k++)
      if (k != best && ! taken[k] && dist_piece_cmp(&ev[k].pc[at[k]],
          &ev[best].pc[at[best]], t, end, &ok) < 0)
        best = k;
    if (! ok)
      break;
    if (best != cur && *nactive > 0 && (*active)[*nactive - 1].t0 == t)
    {
      /* The closest part changes again at t */
      (*active)[*nactive - 1].part = best;
      cur = best;
    }
    else if (best != cur)
    {
      if (cur >= 0)
      {
        (*active)[*nactive - 1].t1 = t;
        if (*ncorners == csize)
        {
          csize *= 2;
          *corners = repalloc(*corners, sizeof(double) * csize);
        }
        (*corners)[(*ncorners)++] = t;
      }
      if (*nactive == size)
      {
        size *= 2;
        *active = repalloc(*active, sizeof(DistActive) * size);
      }
      (*active)[*nactive].t0 = t;
      (*active)[*nactive].part = best;
      (*nactive)++;
      cur = best;
    }
    (*active)[*nactive - 1].t1 = end;
    /* The first part whose distance goes below the one of the closest part
     * before the end of the pieces */
    const DistPiece *pc = &ev[cur].pc[at[cur]];
    double tnext = end;
    force = -1;
    if (! pc->zero)
      for (int k = 0; k < nparts; k++)
      {
        const DistPiece *po = &ev[k].pc[at[k]];
        if (k == cur || po->zero)
          continue;
        DistFun h;
        double htol, root;
        if (! dist_piece_diff(po, pc, t, tnext, &h, &htol))
        {
          ok = false;
          break;
        }
        int rc = (t < tnext) ?
          distfun_first_crossing(&h, 0.0, t, tnext, htol, -1, &root) : 0;
        if (rc < 0 && rc != DISTFUN_ZERO)
        {
          ok = false;
          break;
        }
        if (rc == 1 && root <= t)
        {
          if (! taken[k] && force < 0)
            force = k;
        }
        else if (rc == 1 && root < tnext)
          tnext = root;
      }
    if (! ok)
      break;
    if (force >= 0)
    {
      /* The part whose crossing is at t is the closest just after t */
      taken[cur] = taken[force] = true;
      continue;
    }
    for (int k = 0; k < nparts; k++)
      taken[k] = false;
    t = tnext;
  }
  pfree(at);
  pfree(taken);
  if (! ok)
  {
    pfree(*active);
    pfree(*corners);
  }
  return ok;
}

/**
 * @brief Return true if the part @p k is the closest at @p t, see
 * #dist_envelope()
 */
static bool
dist_active(const DistActive *active, int nactive, int k, double t)
{
  for (int i = 0; i < nactive; i++)
    if (active[i].part == k && active[i].t0 <= t && t <= active[i].t1)
      return true;
  return false;
}

/**
 * @brief Walk of the closest features over the segments of a sequence, the
 * parameter of the turning point functions of the lift of the temporal
 * distance (#dist_lift_temp_turnpt, #dist_lift_base_turnpt)
 * @details The closest feature at the end of a segment is the one at the
 * start of the next one.  The walk keeps the instants and distances of the
 * last segment it walked, which a second call for that segment, made with an
 * array that holds them all, reads without walking it again.  For a static
 * geometry of several parts, the walk of each part keeps its pieces, and the
 * instants are those of the lower envelope, see #dist_lift_parts().
 */
typedef struct
{
  const DistOps *ops;        /**< Operands */
  const DistRefPoly *rb;     /**< Body of the second operand, or NULL for a
                                  static geometry of several parts */
  const DistMotion *mb;      /**< Motion of a static second operand, or NULL
                                  for a temporal one */
  bool exact;                /**< True if the value at an end of a segment is
                                  the distance of the operands placed there */
  DistPair cf;               /**< Closest feature at the end of the last
                                  segment walked */
  DistEvents events;         /**< Events of the last segment walked */
  TimestampTz lower;         /**< Start of the last segment walked */
  TimestampTz upper;         /**< End of the last segment walked */
  int count;                 /**< Number of instants of the last segment
                                  walked, or -1 if none is */
  int size;                  /**< Number of instants the arrays hold */
  TimestampTz *ts;           /**< Instants of the last segment walked */
  double *ds;                /**< Distances at these instants */
  int nparts;                /**< Number of parts walked, 0 if @p rb is
                                  walked */
  DistPair *pcf;             /**< Closest feature of each part */
  DistEvents *pev;           /**< Events and pieces of each part */
  int nsegs;                 /**< Number of segments walked */
  bool failed;               /**< True if the walk of a segment failed */
} DistLift;

/**
 * @brief Set the walk over the segments of a sequence
 */
static void
dist_lift_init(DistLift *lift, const DistOps *ops, const DistRefPoly *rb,
  const DistMotion *mb, bool exact)
{
  lift->ops = ops;
  lift->rb = rb;
  lift->mb = mb;
  lift->exact = exact;
  lift->cf.kind = DISTPAIR_NONE;
  lift->cf.i = lift->cf.j = 0;
  distevents_init(&lift->events);
  lift->lower = lift->upper = 0;
  lift->count = -1;
  lift->size = 16;
  lift->ts = palloc(sizeof(TimestampTz) * lift->size);
  lift->ds = palloc(sizeof(double) * lift->size);
  lift->nsegs = 0;
  lift->failed = false;
  lift->nparts = rb ? 0 : ops->nparts;
  lift->pcf = NULL;
  lift->pev = NULL;
  if (lift->nparts)
  {
    lift->pcf = palloc(sizeof(DistPair) * lift->nparts);
    lift->pev = palloc(sizeof(DistEvents) * lift->nparts);
    for (int k = 0; k < lift->nparts; k++)
    {
      lift->pcf[k].kind = DISTPAIR_NONE;
      lift->pcf[k].i = lift->pcf[k].j = 0;
      distevents_init(&lift->pev[k]);
      distevents_keep_pieces(&lift->pev[k]);
    }
  }
}

/**
 * @brief Free the walk over the segments of a sequence
 */
static void
dist_lift_free(DistLift *lift)
{
  distevents_free(&lift->events);
  pfree(lift->ts);
  pfree(lift->ds);
  if (lift->nparts)
  {
    for (int k = 0; k < lift->nparts; k++)
      distevents_free(&lift->pev[k]);
    pfree(lift->pcf);
    pfree(lift->pev);
  }
}

/**
 * @brief Add an instant to the instants of the segment walked
 * @details Two events at the same timestamp become one instant with the
 * smaller distance.  This happens for two events closer than one
 * microsecond, and for an event inside the segment that is closer than half
 * a microsecond to one of its ends.  The lift merges in the same way the end
 * of a segment and the start of the next one (#datum_min_float8).
 */
static pg_attribute_always_inline void
dist_lift_append(DistLift *lift, TimestampTz t, double dist)
{
  if (lift->count > 0 && lift->ts[lift->count - 1] >= t)
  {
    if (dist < lift->ds[lift->count - 1])
    {
      lift->ts[lift->count - 1] = t;
      lift->ds[lift->count - 1] = dist;
    }
    return;
  }
  if (lift->count == lift->size)
  {
    lift->size *= 2;
    lift->ts = repalloc(lift->ts, sizeof(TimestampTz) * lift->size);
    lift->ds = repalloc(lift->ds, sizeof(double) * lift->size);
  }
  lift->ts[lift->count] = t;
  lift->ds[lift->count++] = dist;
}

/**
 * @brief Add to the instants of the segment walked those of the walk of the
 * closest features between the first operand, moving with @p ma, and the body
 * of the second one, and return false if the walk fails
 * @details The second operand goes from @p start2 to @p end2, a pose or a
 * point, unless it is static.  At an end of the segment, the value is the
 * distance between the operands placed there (#dist_values) where the walk is
 * exact, and the value of the walk there otherwise.
 */
static bool
dist_lift_walk(DistLift *lift, const DistMotion *ma, Datum start1,
  Datum end1, Datum start2, Datum end2, TimestampTz lower, TimestampTz upper)
{
  const DistOps *ops = lift->ops;
  DistMotion mb;
  if (lift->mb)
    mb = *lift->mb;
  else if (ops->ref2)
    distmotion_from_pose(&mb, DatumGetPoseP(start2), DatumGetPoseP(end2));
  else
  {
    const POINT2D *p1 = GSERIALIZED_POINT2D_P(DatumGetGserializedP(start2));
    const POINT2D *p2 = GSERIALIZED_POINT2D_P(DatumGetGserializedP(end2));
    distmotion_set(&mb, p1->x, p1->y, 0.0, p2->x, p2->y, 0.0);
  }
  DistSegm dseg;
  distsegm_set(&dseg, ma, &mb);
  lift->events.count = 0;
  if (distwalk_segm(&dseg, &ops->body1, lift->rb,
      distwalk_ftol(&dseg, &ops->body1, lift->rb), ops->level, &lift->cf,
      &lift->events) != DISTWALK_OK)
    return false;
  for (int j = 0; j < lift->events.count; j++)
  {
    double ratio = lift->events.ev[j].t;
    double dist = lift->events.ev[j].dist;
    if (lift->exact && ratio <= 0.0)
      dist = dist_values(ops, start1, start2);
    else if (lift->exact && ratio >= 1.0)
      dist = dist_values(ops, end1, end2);
    dist_lift_append(lift, dist_event_time(lower, upper, ratio), dist);
  }
  return true;
}

/**
 * @brief Add to the instants of the segment walked those of the least of the
 * distances to the parts of a static geometry, the first operand moving with
 * @p ma, and return false if a walk or the envelope fails
 * @details The walk of each part keeps its pieces, and the lower envelope of
 * the pieces (#dist_envelope) gives the intervals in which each part is the
 * closest and the corners between them, the instants where two parts are at
 * the same distance.  The instants are the events of each part while it is
 * the closest and the corners, in time order, each at its nearest
 * microsecond (#dist_event_time).  The value at a corner is the one of the
 * part that was closest before it, equal to the one that is closest after it.
 */
static bool
dist_lift_parts(DistLift *lift, const DistMotion *ma, TimestampTz lower,
  TimestampTz upper)
{
  const DistOps *ops = lift->ops;
  int nparts = lift->nparts;
  DistEvents *ev = lift->pev;
  for (int k = 0; k < nparts; k++)
  {
    DistSegm dseg;
    distsegm_set(&dseg, ma, &ops->parts[k].motion);
    ev[k].count = ev[k].npc = 0;
    if (distwalk_segm(&dseg, &ops->body1, &ops->parts[k].body,
        distwalk_ftol(&dseg, &ops->body1, &ops->parts[k].body), ops->level,
        &lift->pcf[k], &ev[k]) != DISTWALK_OK)
      return false;
  }
  DistActive *active = NULL;
  double *corners = NULL;
  int nactive, ncorners;
  if (! dist_envelope(ev, nparts, &active, &nactive, &corners, &ncorners))
    return false;
  /* A merge of the sorted events of each part and of the sorted corners */
  int *next = palloc0(sizeof(int) * nparts);
  int nc = 0;
  while (true)
  {
    double tmin = 2.0, dist = 0.0;
    int kmin = -1;
    for (int k = 0; k < nparts; k++)
    {
      while (next[k] < ev[k].count && ! dist_active(active, nactive, k,
          ev[k].ev[next[k]].t))
        next[k]++;
      if (next[k] < ev[k].count && ev[k].ev[next[k]].t < tmin)
      {
        tmin = ev[k].ev[next[k]].t;
        kmin = k;
      }
    }
    if (nc < ncorners && corners[nc] < tmin)
    {
      tmin = corners[nc++];
      int a = 0;
      while (a < nactive - 1 && active[a].t1 < tmin)
        a++;
      int from = 0;
      dist = distpiece_dist(dist_piece_at(&ev[active[a].part], &from, tmin),
        tmin);
    }
    else if (kmin >= 0)
      dist = ev[kmin].ev[next[kmin]++].dist;
    else
      break;
    dist_lift_append(lift, dist_event_time(lower, upper, tmin), dist);
  }
  pfree(next); pfree(active); pfree(corners);
  return true;
}

/**
 * @brief Write in @p tps and @p values the instants of the temporal distance
 * in a segment, its two ends included, at most @p cap of them, and return
 * how many there are
 * @details The segment goes from @p lower to @p upper, the first operand from
 * the pose @p start1 to @p end1 and the second one from @p start2 to @p end2,
 * a pose or a point, which a static operand does not read.  A sequence with
 * step interpolation keeps its value during the segment, so that the values
 * it receives are equal.
 *
 * The walk of the closest features (#distwalk_segm) gives the events of the
 * segment, from its start to its end, see #dist_lift_walk(), and for a
 * static geometry of several parts, the lower envelope of the walks of the
 * parts gives them, see #dist_lift_parts().  An event inside a
 * segment is at a time that is not a microsecond, and its value is the one
 * of the walk at that time: zero where the bodies meet, the distance level of
 * the operands where the distance crosses it, and the value of the function
 * of the feature at an extremum.  The instant of such an event is the nearest
 * microsecond (#dist_event_time), where the value of the bodies placed
 * differs from the value at the event.
 *
 * A failure of the walk raises an error, gives no instant and marks the
 * walk as failed, so that the lift answers NULL (#dist_lift_seq) and no
 * later segment is walked.  It is not a property of the data but a defect.
 * The readers call the function on the segments in their order, so that the
 * number of segments walked before is the index of the segment.
 */
static int
dist_lift_segm(DistLift *lift, Datum start1, Datum end1, Datum start2,
  Datum end2, TimestampTz lower, TimestampTz upper, TimestampTz *tps,
  Datum *values, int cap)
{
  assert(lift); assert(tps); assert(values); assert(cap >= 2);
  assert(lower < upper);
  if (lift->failed)
    return 0;
  if (lift->count < 0 || lift->lower != lower || lift->upper != upper)
  {
    DistMotion ma;
    distmotion_from_pose(&ma, DatumGetPoseP(start1), DatumGetPoseP(end1));
    lift->count = 0;
    lift->lower = lower;
    lift->upper = upper;
    bool ok = lift->nparts ? dist_lift_parts(lift, &ma, lower, upper) :
      dist_lift_walk(lift, &ma, start1, end1, start2, end2, lower, upper);
    if (! ok)
    {
      lift->count = -1;
      lift->failed = true;
      if (lift->nparts)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "The temporal distance of a temporal rigid geometry failed");
        return 0;
      }
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The temporal distance of a temporal rigid geometry failed at "
        "segment %d", lift->nsegs);
      return 0;
    }
    lift->nsegs++;
  }
  for (int k = 0; k < lift->count && k < cap; k++)
  {
    tps[k] = lift->ts[k];
    values[k] = Float8GetDatum(lift->ds[k]);
  }
  return lift->count;
}

/**
 * @brief Turning point function of the lift of the temporal distance between
 * a temporal rigid geometry and a temporal point or temporal rigid geometry,
 * see #dist_lift_segm()
 * @param[in] param Walk over the segments (#DistLift)
 */
static int
dist_lift_temp_turnpt(Datum start1, Datum end1, Datum start2, Datum end2,
  Datum param, TimestampTz lower, TimestampTz upper, TimestampTz *tps,
  Datum *values, int cap)
{
  return dist_lift_segm((DistLift *) DatumGetPointer(param), start1, end1,
    start2, end2, lower, upper, tps, values, cap);
}

/**
 * @brief Turning point function of the lift of the temporal distance between
 * a temporal rigid geometry and a part of a static geometry, see
 * #dist_lift_segm()
 * @param[in] value Walk over the segments (#DistLift), the base value of the
 * lift
 */
static int
dist_lift_base_turnpt(Datum start, Datum end, Datum value, TimestampTz lower,
  TimestampTz upper, TimestampTz *tps, Datum *values, int cap)
{
  return dist_lift_segm((DistLift *) DatumGetPointer(value), start, end,
    value, value, lower, upper, tps, values, cap);
}

/**
 * @brief Return the distance between a pose of the first operand and a value
 * of the second one, the function of the lift of the temporal distance to a
 * temporal point or temporal rigid geometry (#dist_values)
 * @param[in] param Walk over the segments (#DistLift)
 */
static Datum
dist_lift_temp_value(Datum value1, Datum value2, Datum param)
{
  const DistLift *lift = (const DistLift *) DatumGetPointer(param);
  return Float8GetDatum(dist_values(lift->ops, value1, value2));
}

/**
 * @brief Return the distance between a pose of the first operand and the
 * static geometry, the function of the lift of the temporal distance to a
 * static geometry (#dist_values)
 * @details The lift reads it only at an instant that ends no segment, since
 * its turning point function writes the ends of every segment.
 * @param[in] value Walk over the segments (#DistLift)
 */
static Datum
dist_lift_base_value(Datum value1, Datum value)
{
  const DistLift *lift = (const DistLift *) DatumGetPointer(value);
  return Float8GetDatum(dist_values(lift->ops, value1, value));
}

/**
 * @brief Return the temporal distance between the first operand @p seq1 and
 * the body @p rb over the segments of @p seq1, through the lifting
 * @details If @p seq2 is NULL, @p rb is static with the motion @p mb.  Else
 * @p seq2 gives the motion of @p rb and has the same timestamps as
 * @p seq1.
 *
 * The turning point function of the lift (#dist_lift_segm) writes every
 * instant of each segment with its value, its ends included, and the lift
 * keeps them all, so that the result is not normalized, see
 * dist_normalize().  If one sequence has linear interpolation and the other
 * has step interpolation, the distance jumps at each instant, and the result
 * is a sequence set with one sequence [t_i, t_{i+1}) for each segment, and a
 * last sequence with the last instant if the period includes it.  Else the
 * result is one sequence.
 * @param[in] exact True if the value at an end of a segment is the distance
 * of the operands placed there, see #dist_lift_segm()
 */
static Temporal *
dist_lift_seq(const DistOps *ops, const TSequence *seq1,
  const TSequence *seq2, const DistRefPoly *rb, const DistMotion *mb,
  bool exact)
{
  DistLift lift;
  dist_lift_init(&lift, ops, rb, mb, exact);
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.argtype[0] = temptype_basetype(seq1->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = true;
  lfinfo.tpfn_many = true;
  lfinfo.tpfn_value = true;
  lfinfo.tpfn_ends = true;
  lfinfo.tpfn_merge = &datum_min_float8;
  lfinfo.reskeep = true;
  lfinfo.ressegs = true;
  Temporal *result;
  if (seq2)
  {
    lfinfo.func = (varfunc) &dist_lift_temp_value;
    lfinfo.numparam = 1;
    lfinfo.param[0] = PointerGetDatum(&lift);
    lfinfo.argtype[1] = temptype_basetype(seq2->temptype);
    lfinfo.tpfn_temp = &dist_lift_temp_turnpt;
    result = tfunc_tcontseq_tcontseq(seq1, seq2, &lfinfo);
    /* With a linear and a step sequence, the result is a sequence set even
     * when it has one sequence */
    if (result && result->subtype == TSEQUENCE &&
        MEOS_FLAGS_LINEAR_INTERP(seq1->flags) !=
        MEOS_FLAGS_LINEAR_INTERP(seq2->flags))
      result = (Temporal *) tsequence_to_tsequenceset_free(
        (TSequence *) result);
  }
  else
  {
    lfinfo.func = (varfunc) &dist_lift_base_value;
    lfinfo.argtype[1] = T_GEOMETRY;
    lfinfo.tpfn_base = &dist_lift_base_turnpt;
    result = tfunc_temporal_base((const Temporal *) seq1,
      PointerGetDatum(&lift), &lfinfo);
  }
  /* A failed walk makes the answer NULL, whether or not the error handler
   * records the error that the walk raised */
  if (lift.failed && result)
  {
    pfree(result);
    result = NULL;
  }
  dist_lift_free(&lift);
  return result;
}

/**
 * @brief Set the value of each instant of @p dist to the exact distance
 * between the rigid geometry @p seq and the static geometry of @p ops
 * @details The least of the distances to the parts has its corners at the
 * nearest microsecond of the instants where two parts are at the same
 * distance, and the value of the walk at its other events, see
 * #dist_lift_parts().  The function computes the distance at each instant,
 * see #dist_pose_geo().  Then the result is exact at each instant, as for one
 * part, see #dist_lift_segm().
 *
 * An instant whose value is exactly the distance level of @p ops keeps it.
 * Such a value comes from the crossing of the level by the part that is
 * closest at that time, see distwalk_segm(), thus it is the exact distance.
 * A value from PostGIS has a rounding error, which can put the crossing in
 * the same microsecond as the instant, where the comparison with the level
 * does not see it.  Where the body has a vertex in a polygon read by its
 * rings (#dist_inside()), the crossing is one of the distance to the rings
 * and not of the distance, which is 0 there.
 * @param[in] dist Minimum of the distances to the parts, freed
 */
static TSequence *
dist2d_seq_exact_values(const DistOps *ops, TSequence *dist,
  const TSequence *seq)
{
  double level = ops->level;
  TInstant **instants = palloc(sizeof(TInstant *) * dist->count);
  for (int i = 0; i < dist->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(dist, i);
    bool atlevel = (level >= 0.0 &&
      DatumGetFloat8(tinstant_value_p(inst)) == level);
    if (atlevel && ops->npolys == 0)
    {
      instants[i] = tinstant_copy(inst);
      continue;
    }
    Datum pose;
    /* The instants of the minimum are in the period of the rigid geometry */
    if (! tsequence_value_at_timestamptz(seq, inst->t, false, &pose))
    {
      instants[i] = tinstant_copy(inst);
      continue;
    }
    /* A crossing of the level by the distance to the rings of a polygon is
     * not one of the distance where the body lies in that polygon */
    const Pose *p = DatumGetPoseP(pose);
    if (atlevel && ! dist_inside(ops, p))
      instants[i] = tinstant_copy(inst);
    else
      instants[i] = tinstant_make(Float8GetDatum(dist_pose_geo(ops, p)),
        T_TFLOAT, inst->t);
    pfree(DatumGetPointer(pose));
  }
  TSequence *result = tsequence_make_free(instants, dist->count,
    dist->period.lower_inc, dist->period.upper_inc, LINEAR, NORMALIZE_NO);
  pfree(dist);
  return result;
}

/**
 * @brief Return true if a body that translates from @p pose1 to @p pose2
 * meets the static geometry of @p ops, decided exactly
 * @details The body meets the geometry where it meets a part
 * (#convex_motion_meet_exact), or where it has a vertex at @p pose1 in a
 * polygon read by its rings (#dist_inside()), see #dist_parts_nad().
 */
static bool
dist_parts_meet(const DistOps *ops, const Pose *pose1, const Pose *pose2)
{
  for (int j = 0; j < ops->nparts; j++)
    if (convex_motion_meet_exact(ops->refxy, ops->nref, pose1->data,
        pose2->data, ops->parts[j].xy, ops->parts[j].npts))
      return true;
  return dist_inside(ops, pose1);
}

/**
 * @brief Give the least instant inside a segment of a body that does not
 * turn the distance 0 where the body meets the static geometry at no instant
 * of the walk, see #dist_seq_least()
 * @details At the distance @p level 0 of #DistOps, the result answers only on
 * which side of 0 each value lies.  The least distance over the segment is 0
 * where the body meets the geometry (#dist_parts_meet), and positive, as
 * every value of the walk inside it, where it does not.  Where it is 0, no
 * instant of the walk is at 0 and the body meets the geometry at neither end,
 * the instant of least value takes 0, as #dist_seq_least() gives it the
 * least distance.
 * @param[in,out] values Values of the instants of the distance
 * @param[in] first,last The instants first to last - 1 are inside the segment
 * @return True if a value is changed
 */
static bool
dist_seq_least_zero(const DistOps *ops, const Pose *pose1, const Pose *pose2,
  double *values, int first, int last)
{
  int min = first;
  for (int j = first; j < last; j++)
  {
    if (values[j] == 0.0)
      return false;
    if (values[j] < values[min])
      min = j;
  }
  if (! dist_parts_meet(ops, pose1, pose2) ||
      dist_parts_meet(ops, pose1, pose1) || dist_parts_meet(ops, pose2, pose2))
    return false;
  values[min] = 0.0;
  return true;
}

/**
 * @brief Give the instants inside each segment of a body that does not turn
 * the exact least distance to a static geometry
 * @details A body whose poses at the two ends of a segment have the angle 0
 * translates.  Its least distance over the segment to a part at rest is the
 * nearest approach of the two, the double nearest its exact value
 * (#convex_motion_nad_exact), and the least over the parts is the least
 * distance to the geometry, 0 in a polygon read by its rings
 * (#dist_parts_nad()).  No instant inside the segment is below it.
 * Where it is below the distances at the two ends, computed in the same way,
 * the instant of least value inside the segment, which the walk puts at the
 * least distance, takes it: the value at the event, whose time is not a
 * microsecond.  An instant whose value is exactly the distance @p level of
 * #DistOps keeps it, see #dist2d_seq_exact_values().  At the distance
 * @p level 0, whether the body meets the geometry decides the segment, see
 * #dist_seq_least_zero().
 * @param[in] dist Temporal distance of @p seq to the static geometry, freed
 */
static TSequence *
dist_seq_least(const DistOps *ops, const TSequence *seq, TSequence *dist)
{
  double *values = palloc(sizeof(double) * dist->count);
  for (int j = 0; j < dist->count; j++)
    values[j] = DatumGetFloat8(tinstant_value_p(TSEQUENCE_INST_N(dist, j)));
  bool changed = false;
  int k = 0;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const TInstant *inst1 = TSEQUENCE_INST_N(seq, i);
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
    /* The instants first to last - 1 are inside the segment */
    while (k < dist->count && TSEQUENCE_INST_N(dist, k)->t <= inst1->t)
      k++;
    int first = k;
    while (k < dist->count && TSEQUENCE_INST_N(dist, k)->t < inst2->t)
      k++;
    int last = k;
    const Pose *pose1 = DatumGetPoseP(tinstant_value_p(inst1));
    const Pose *pose2 = DatumGetPoseP(tinstant_value_p(inst2));
    if (first == last || pose1->data[2] != 0.0 || pose2->data[2] != 0.0)
      continue;
    if (ops->level == 0.0)
    {
      changed |= dist_seq_least_zero(ops, pose1, pose2, values, first, last);
      continue;
    }
    double least = dist_parts_nad(ops, pose1, pose2);
    int min = -1;
    bool reached = false;
    for (int j = first; j < last; j++)
    {
      if (values[j] == ops->level)
      {
        /* The walk has put the least distance on a crossing of the level */
        reached |= (values[j] == least);
        continue;
      }
      if (values[j] < least)
      {
        values[j] = least;
        changed = true;
      }
      if (min < 0 || values[j] < values[min])
        min = j;
    }
    if (reached || min < 0 || values[min] == least)
      continue;
    double end1 = dist_parts_nad(ops, pose1, pose1);
    double end2 = dist_parts_nad(ops, pose2, pose2);
    if (least < end1 && least < end2)
    {
      values[min] = least;
      changed = true;
    }
  }
  if (! changed)
  {
    pfree(values);
    return dist;
  }
  TInstant **instants = palloc(sizeof(TInstant *) * dist->count);
  for (int j = 0; j < dist->count; j++)
    instants[j] = tinstant_make(Float8GetDatum(values[j]), T_TFLOAT,
      TSEQUENCE_INST_N(dist, j)->t);
  TSequence *result = tsequence_make_free(instants, dist->count,
    dist->period.lower_inc, dist->period.upper_inc, LINEAR, NORMALIZE_NO);
  pfree(values);
  pfree(dist);
  return result;
}

/**
 * @brief Return the temporal distance between two operands over one
 * sequence
 * @details If no operand has linear interpolation, the distance is the
 * distance at each instant.  Else the walk gives it.  For a static geometry,
 * the result is the minimum of the distances to its parts.
 * @param[in] seq1 Sequence of the rigid geometry
 * @param[in] seq2 Sequence of the second operand with the same timestamps,
 * or NULL for a static geometry @nullable
 */
static Temporal *
dist_seq(const DistOps *ops, const TSequence *seq1, const TSequence *seq2)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq1->flags) ||
    (seq2 && MEOS_FLAGS_LINEAR_INTERP(seq2->flags));
  if (seq1->count == 1 || ! linear)
  {
    /* Discrete interpolation if one operand has it, else step */
    interpType interp = MEOS_FLAGS_GET_INTERP(seq1->flags);
    if (seq2 && MEOS_FLAGS_GET_INTERP(seq2->flags) == DISCRETE)
      interp = DISCRETE;
    TInstant **instants = palloc(sizeof(TInstant *) * seq1->count);
    for (int i = 0; i < seq1->count; i++)
    {
      const TInstant *inst1 = TSEQUENCE_INST_N(seq1, i);
      instants[i] = tinstant_make(Float8GetDatum(dist_instant(ops, inst1,
        seq2 ? TSEQUENCE_INST_N(seq2, i) : NULL)), T_TFLOAT, inst1->t);
    }
    return (Temporal *) tsequence_make_free(instants, seq1->count,
      seq1->period.lower_inc, seq1->period.upper_inc, interp, NORMALIZE);
  }
  if (seq2)
    return dist_lift_seq(ops, seq1, seq2, &ops->body2, NULL, true);

  /* The least of the distances to the parts has the values of the walk,
   * which dist2d_seq_exact_values() sets to the distance to the geometry */
  bool parts = ops->nparts > 1;
  TSequence *result = (TSequence *) dist_lift_seq(ops, seq1, NULL,
    parts ? NULL : &ops->parts[0].body, parts ? NULL : &ops->parts[0].motion,
    ! parts);
  if (result && parts)
    result = dist2d_seq_exact_values(ops, result, seq1);
  if (! result)
    return NULL;
  if (ops->refxy)
    result = dist_seq_least(ops, seq1, result);
  return (Temporal *) result;
}

/**
 * @brief Return the temporal distance between two operands
 * @param[in] temp1 Temporal rigid geometry
 * @param[in] temp2 Second operand with the same subtype and timestamps, or
 * NULL for a static geometry @nullable
 */
static Temporal *
dist_temporal(const DistOps *ops, const Temporal *temp1, const Temporal *temp2)
{
  assert(temptype_subtype(temp1->subtype));
  if (temp1->subtype == TINSTANT)
  {
    const TInstant *inst1 = (const TInstant *) temp1;
    return (Temporal *) tinstant_make(Float8GetDatum(dist_instant(ops, inst1,
      (const TInstant *) temp2)), T_TFLOAT, inst1->t);
  }
  if (temp1->subtype == TSEQUENCE)
    return dist_seq(ops, (const TSequence *) temp1,
      (const TSequence *) temp2);
  /* TSEQUENCESET */
  const TSequenceSet *ss1 = (const TSequenceSet *) temp1;
  const TSequenceSet *ss2 = (const TSequenceSet *) temp2;
  int size = ss1->totalcount, count = 0;
  TSequence **sequences = palloc(sizeof(TSequence *) * size);
  for (int i = 0; i < ss1->count; i++)
  {
    Temporal *dist = dist_seq(ops, TSEQUENCESET_SEQ_N(ss1, i),
      ss2 ? TSEQUENCESET_SEQ_N(ss2, i) : NULL);
    if (! dist)
    {
      pfree_array((void **) sequences, count);
      return NULL;
    }
    /* The distance of a sequence is a sequence or a sequence set */
    if (dist->subtype == TSEQUENCE)
      sequences[count++] = (TSequence *) dist;
    else
    {
      const TSequenceSet *ss = (const TSequenceSet *) dist;
      if (count + ss->count > size)
      {
        size = 2 * (count + ss->count);
        sequences = repalloc(sequences, sizeof(TSequence *) * size);
      }
      for (int j = 0; j < ss->count; j++)
        sequences[count++] = tsequence_copy(TSEQUENCESET_SEQ_N(ss, j));
      pfree(dist);
    }
  }
  return (Temporal *) tsequenceset_make_free(sequences, count, NORMALIZE_NO);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Set the operands of the distance between a temporal rigid geometry
 * and a nonempty geometry
 * @details The walk finds the distance to a convex body.  A point, a segment
 * and a convex polygon without holes are convex bodies, thus the distance to
 * the geometry is the least of the distances to its parts, the members of a
 * multi-geometry or a geometry collection all read as parts, and any other
 * polygon read as the segments of its rings and its area, see
 * #dist_parts_add() and #dist_inside().
 * @return False, with an error, if the body is not one the walk reads, see
 * #dist_ref_body(), or if a type the geometry holds is not supported
 */
static bool
dist_geo_ops_set(DistOps *ops, const Temporal *temp, const GSERIALIZED *gs,
  double level)
{
  memset(ops, 0, sizeof(DistOps));
  ops->ref1 = trgeometry_geom_p(temp);
  ops->gs = gs;
  ops->level = level;
  if (! dist_ref_body(ops->ref1, &ops->body1))
    return false;
  ops->geom = lwgeom_from_gserialized(gs);
  /* The number of parts is at most the number of points, and so is the
   * number of polygons */
  int maxparts = (int) lwgeom_count_vertices(ops->geom) + 1;
  ops->parts = palloc(sizeof(DistPart) * maxparts);
  ops->polys = palloc(sizeof(LWPOLY *) * maxparts);
  if (! dist_parts_add_all(ops->geom, ops->parts, &ops->nparts, ops->polys,
      &ops->npolys))
  {
    dist_parts_free(ops->parts, ops->nparts);
    pfree(ops->polys);
    lwgeom_free(ops->geom);
    distrefpoly_free(&ops->body1);
    return false;
  }
  /* The reference geometry is a polygon, see dist_ref_body() */
  LWGEOM *ref = lwgeom_from_gserialized(ops->ref1);
  const POINTARRAY *ring = lwgeom_as_lwpoly(ref)->rings[0];
  DistPart body;
  dist_part_points(&body, ring, 0, (int) ring->npoints - 1);
  lwgeom_free(ref);
  ops->refxy = body.xy;
  ops->nref = body.npts;
  return true;
}

/**
 * @brief Free the operands that #dist_geo_ops_set() sets
 */
static void
dist_geo_ops_free(DistOps *ops)
{
  pfree(ops->refxy);
  dist_parts_free(ops->parts, ops->nparts);
  pfree(ops->polys);
  lwgeom_free(ops->geom);
  distrefpoly_free(&ops->body1);
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a geometry
 * @param[in] level Distance whose crossings are instants of the result, or a
 * negative value.  Then the result is on the same side of @p level as the
 * distance at each time, which the threshold functions such as
 * #tdwithin_trgeometry_geo() need.
 */
Temporal *
tdistance_trgeometry_geo_intl(const Temporal *temp, const GSERIALIZED *gs,
  double level)
{
  assert(temp); assert(gs); assert(ensure_valid_trgeometry_geo(temp, gs));
  assert(! MEOS_FLAGS_GET_Z(temp->flags));
  /* An empty geometry has no distance to a temporal value */
  if (gserialized_is_empty(gs))
    return NULL;
  DistOps ops;
  if (! dist_geo_ops_set(&ops, temp, gs, level))
    return NULL;
  /* An error raised for one of the instants makes the answer NULL, as the
   * entry functions of lifting.c answer it */
  int last_errno = meos_errno_reset();
  Temporal *result = (Temporal *) lifting_result(dist_temporal(&ops, temp,
    NULL), last_errno);
  dist_geo_ops_free(&ops);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a temporal point or another temporal rigid geometry
 * @param[in] level See #tdistance_trgeometry_geo_intl()
 * @pre The arguments are valid and planar, as the external entries test them
 */
static Temporal *
tdistance_trgeometry_temporal(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  assert(! MEOS_FLAGS_GET_Z(temp1->flags));
  assert(! MEOS_FLAGS_GET_Z(temp2->flags));
  DistOps ops;
  memset(&ops, 0, sizeof(DistOps));
  ops.ref1 = trgeometry_geom_p(temp1);
  ops.ref2 = (temp2->temptype == T_TRGEOMETRY) ? trgeometry_geom_p(temp2) : NULL;
  ops.level = level;
  if (! dist_ref_body(ops.ref1, &ops.body1))
    return NULL;
  if (ops.ref2)
  {
    if (! dist_ref_body(ops.ref2, &ops.body2))
    {
      distrefpoly_free(&ops.body1);
      return NULL;
    }
  }
  else
    distrefpoly_set_point(&ops.body2);

  /* The two values must have the same timestamps.  The synchronization does
   * not add the times at which they cross, which the walk finds. */
  Temporal *sync1, *sync2;
  Temporal *result = NULL;
  if (intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
  {
    /* An error raised for one of the instants makes the answer NULL, as the
     * entry functions of lifting.c answer it */
    int last_errno = meos_errno_reset();
    result = (Temporal *) lifting_result(dist_temporal(&ops, sync1, sync2),
      last_errno);
    pfree(sync1); pfree(sync2);
  }
  distrefpoly_free(&ops.body1);
  distrefpoly_free(&ops.body2);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a temporal point, with the crossings of @p level as instants, see
 * #tdistance_trgeometry_geo_intl()
 */
Temporal *
tdistance_trgeometry_tpoint_intl(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  assert(temp1); assert(temp2);
  assert(ensure_valid_trgeometry_tpoint(temp1, temp2));
  return tdistance_trgeometry_temporal(temp1, temp2, level);
}

/**
 * @brief Return the temporal distance between two temporal rigid geometries,
 * with the crossings of @p level as instants, see #tdistance_trgeometry_geo_intl()
 */
Temporal *
tdistance_trgeometry_trgeometry_intl(const Temporal *temp1, const Temporal *temp2,
  double level)
{
  assert(temp1); assert(temp2);
  assert(ensure_valid_trgeometry_trgeometry(temp1, temp2));
  return tdistance_trgeometry_temporal(temp1, temp2, level);
}

/*****************************************************************************
 * Ever within a distance
 *****************************************************************************/

/**
 * @brief Return true if two bodies are farther apart than @p dist during the
 * whole segment @p seg
 * @details Every point of a body lies within the distance @p rmax of its
 * rotation center, see #distmotion_place(), so the bodies are at least the
 * distance of their centers minus the two @p rmax apart.  The centers move
 * linearly in the segment, and their distance is smallest at the projection
 * of the origin on their relative motion, clamped to the segment.  The bound
 * is grown by #MEOS_EPSILON relative to the magnitude of the values it is
 * computed from, far above the rounding of the computation, so that no
 * segment in which the walk finds a distance at or below @p dist is skipped.
 */
static bool
dist_segm_apart(const DistSegm *seg, const DistRefPoly *ra,
  const DistRefPoly *rb, double dist)
{
  double vv = seg->wx1 * seg->wx1 + seg->wy1 * seg->wy1;
  double t = 0.0;
  if (vv > 0.0)
    t = fmin(fmax(-(seg->wx0 * seg->wx1 + seg->wy0 * seg->wy1) / vv, 0.0),
      1.0);
  double nx = seg->wx0 + t * seg->wx1, ny = seg->wy0 + t * seg->wy1;
  double near = sqrt(nx * nx + ny * ny);
  double bound = dist + ra->rmax + rb->rmax;
  double scale = fabs(seg->ma.cx) + fabs(seg->ma.cy) + fabs(seg->mb.cx) +
    fabs(seg->mb.cy) + sqrt(vv) + bound;
  return near > bound + MEOS_EPSILON * scale;
}

/**
 * @brief Return 1 if the operands are ever within @p dist in two
 * synchronized sequences, 0 if not, -1 if the walk fails
 * @details The values compared with @p dist are those #dist_seq() gives the
 * temporal distance, read until the first one at or below @p dist.  A
 * segment the centers of the bodies keep apart, see #dist_segm_apart(), has
 * none of them and is not walked.
 */
static int
dist_ever_seq(const DistOps *ops, const TSequence *seq1,
  const TSequence *seq2, double dist)
{
  bool linear1 = MEOS_FLAGS_LINEAR_INTERP(seq1->flags);
  bool linear2 = MEOS_FLAGS_LINEAR_INTERP(seq2->flags);
  if (seq1->count == 1 || (! linear1 && ! linear2))
  {
    for (int i = 0; i < seq1->count; i++)
      if (dist_instant(ops, TSEQUENCE_INST_N(seq1, i),
          TSEQUENCE_INST_N(seq2, i)) <= dist)
        return 1;
    return 0;
  }

  DistEvents events;
  distevents_init(&events);
  DistPair cf;
  cf.kind = DISTPAIR_NONE;
  cf.i = cf.j = 0;
  int result = 0;
  for (int i = 0; i < seq1->count - 1 && result == 0; i++)
  {
    DistMotion ma, mb;
    dist_segmotion(&ma, seq1, i, true);
    dist_segmotion(&mb, seq2, i, true);
    DistSegm dseg;
    distsegm_set(&dseg, &ma, &mb);
    if (dist_segm_apart(&dseg, &ops->body1, &ops->body2, dist))
    {
      /* The closest feature at the start of the next segment is unknown */
      cf.kind = DISTPAIR_NONE;
      continue;
    }
    events.count = 0;
    if (distwalk_segm(&dseg, &ops->body1, &ops->body2,
        distwalk_ftol(&dseg, &ops->body1, &ops->body2), ops->level, &cf,
        &events) != DISTWALK_OK)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The temporal distance of a temporal rigid geometry failed at "
        "segment %d", i);
      result = -1;
      break;
    }
    for (int j = 0; j < events.count && result == 0; j++)
      if (events.ev[j].dist <= dist)
        result = 1;
  }
  distevents_free(&events);
  /* With a linear and a step operand, the last instant is a value of its own,
   * see #dist_lift_seq() */
  if (result == 0 && linear1 != linear2 && seq1->period.upper_inc &&
      dist_instant(ops, TSEQUENCE_INST_N(seq1, seq1->count - 1),
        TSEQUENCE_INST_N(seq2, seq2->count - 1)) <= dist)
    result = 1;
  return result;
}

/**
 * @brief Return 1 if a sequence of a temporal rigid geometry is ever within
 * @p dist of the static geometry of @p ops, 0 if not, -1 if the walk fails
 * @details The answer is that of the minimum of the distance #dist_seq()
 * gives.  Without linear interpolation, its values are the distances at the
 * instants.  With it, a segment that the center of the body keeps apart from
 * a part, see #dist_segm_apart(), has no value within @p dist for that part.
 * In a segment where the body does not turn, the least distance to a part is
 * their nearest approach, the double nearest its exact value
 * (#convex_motion_nad_exact), which #dist_seq_least() gives the distance, the
 * ends of the segment included, and at the distance 0 it is 0 where they
 * meet (#convex_motion_meet_exact).  A body with a vertex at the start of a
 * segment in a polygon read by its rings is at the distance 0
 * (#dist_inside()).  A segment where the body turns needs the walk, and then
 * #dist_seq() gives the distance of the sequence.
 */
static int
dist_ever_seq_geo(const DistOps *ops, const TSequence *seq, double dist)
{
  if (seq->count == 1 || ! MEOS_FLAGS_LINEAR_INTERP(seq->flags))
  {
    for (int i = 0; i < seq->count; i++)
      if (dist_instant(ops, TSEQUENCE_INST_N(seq, i), NULL) <= dist)
        return 1;
    return 0;
  }
  bool turns = false;
  for (int i = 0; i < seq->count - 1; i++)
  {
    const Pose *pose1 = DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq,
      i)));
    const Pose *pose2 = DatumGetPoseP(tinstant_value_p(TSEQUENCE_INST_N(seq,
      i + 1)));
    bool still = (pose1->data[2] == 0.0 && pose2->data[2] == 0.0);
    /* A body with a vertex in a polygon read by its rings is at the distance
     * 0, and one that enters it meets a part, see #dist_inside() */
    if (dist_inside(ops, pose1))
      return 1;
    DistMotion ma;
    dist_segmotion(&ma, seq, i, true);
    for (int k = 0; k < ops->nparts; k++)
    {
      DistSegm dseg;
      distsegm_set(&dseg, &ma, &ops->parts[k].motion);
      if (dist_segm_apart(&dseg, &ops->body1, &ops->parts[k].body, dist))
        continue;
      if (! still)
      {
        turns = true;
        break;
      }
      /* At the distance 0 the question is whether they meet */
      if (dist == 0.0 ?
          convex_motion_meet_exact(ops->refxy, ops->nref, pose1->data,
            pose2->data, ops->parts[k].xy, ops->parts[k].npts) :
          convex_motion_nad_exact(ops->refxy, ops->nref, pose1->data,
            pose2->data, ops->parts[k].xy, ops->parts[k].npts) <= dist)
        return 1;
    }
  }
  if (! turns)
    return 0;
  Temporal *tdist = dist_seq(ops, seq, NULL);
  if (! tdist)
    return -1;
  int result = (DatumGetFloat8(temporal_min_value(tdist)) <= dist) ? 1 : 0;
  pfree(tdist);
  return result;
}

/**
 * @brief Return 1 if a temporal rigid geometry and a geometry are ever within
 * a distance, 0 if not, -1 on error
 * @details The answer is that of the minimum of their temporal distance, as
 * #tdistance_trgeometry_geo_intl() gives it, read sequence by sequence until
 * the first one within @p dist, see #dist_ever_seq_geo().
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @pre The arguments are valid and planar, as the external entry tests
 * them
 */
int
edwithin_trgeometry_geo_intl(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  assert(temp); assert(gs); assert(ensure_valid_trgeometry_geo(temp, gs));
  assert(dist >= 0.0);
  assert(! MEOS_FLAGS_GET_Z(temp->flags));
  /* An empty geometry has no distance to a temporal value */
  if (gserialized_is_empty(gs))
    return -1;
  DistOps ops;
  if (! dist_geo_ops_set(&ops, temp, gs, -1.0))
    return -1;
  /* An error raised for one of the instants makes the answer -1, as the
   * temporal distance is then NULL */
  int last_errno = meos_errno_reset();
  int result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
    result = (dist_instant(&ops, (const TInstant *) temp, NULL) <= dist) ?
      1 : 0;
  else if (temp->subtype == TSEQUENCE)
    result = dist_ever_seq_geo(&ops, (const TSequence *) temp, dist);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    result = 0;
    for (int i = 0; i < ss->count && result == 0; i++)
      result = dist_ever_seq_geo(&ops, TSEQUENCESET_SEQ_N(ss, i), dist);
  }
  dist_geo_ops_free(&ops);
  return lifting_result_int(result, last_errno);
}

/**
 * @brief Return the pose at @p t of a temporal rigid geometry sequence in its
 * segment starting at instant @p k
 * @details The pose is the one #tsegment_value_at_timestamptz gives, so that
 * the motion of an interval of #tcont_tcont_merge_walk is that of the
 * synchronized sequences of #tdistance_trgeometry_temporal: the start pose of a
 * constant segment and at the start of the segment, the end pose at its end,
 * and the interpolated pose inside it
 * @param[out] alloc True if the pose is allocated
 */
static const Pose *
dist_seq_pose(const TSequence *seq, int k, TimestampTz t, bool *alloc)
{
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, k);
  const Pose *start = DatumGetPoseP(tinstant_value_p(inst1));
  *alloc = false;
  if (t == inst1->t || seq->count == 1)
    return start;
  const TInstant *inst2 = TSEQUENCE_INST_N(seq, k + 1);
  const Pose *end = DatumGetPoseP(tinstant_value_p(inst2));
  if (pose_eq(start, end))
    return start;
  if (t == inst2->t)
    return end;
  long double ratio = (long double) (t - inst1->t) /
    (long double) (inst2->t - inst1->t);
  *alloc = true;
  return posesegm_interpolate(start, end, (double) ratio);
}

/**
 * @brief The pose of one operand at the end of the last interval walked,
 * which is its pose at the start of the next interval in the same segment
 */
typedef struct
{
  const TSequence *seq;        /**< Sequence */
  int k;                       /**< Instant starting the segment */
  TimestampTz t;               /**< Time of the pose */
  const Pose *pose;            /**< Pose @nullable */
  bool alloc;                  /**< True if the pose is allocated */
} DistPoseCache;

/**
 * @brief Return the pose of #dist_seq_pose, taken from the cache where it
 * holds it, the cache then being emptied
 */
static const Pose *
dist_seq_pose_cached(DistPoseCache *cache, const TSequence *seq, int k,
  TimestampTz t, bool *alloc)
{
  if (cache->pose && cache->seq == seq && cache->k == k && cache->t == t)
  {
    const Pose *result = cache->pose;
    *alloc = cache->alloc;
    cache->pose = NULL;
    return result;
  }
  return dist_seq_pose(seq, k, t, alloc);
}

/**
 * @brief Keep a pose in the cache, freeing the one it held
 */
static void
dist_pose_cache_set(DistPoseCache *cache, const TSequence *seq, int k,
  TimestampTz t, const Pose *pose, bool alloc)
{
  if (cache->pose && cache->alloc)
    pfree((Pose *) cache->pose);
  cache->seq = seq;
  cache->k = k;
  cache->t = t;
  cache->pose = pose;
  cache->alloc = alloc;
}

/**
 * @brief State of #trgeometry_dwithin_interval
 */
typedef struct
{
  const DistOps *ops;          /**< Operands */
  double dist;                 /**< Distance */
  DistPair cf;                 /**< Closest feature at the instant @p cft */
  TimestampTz cft;             /**< Instant of @p cf */
  const TSequence *seq1;       /**< Sequences of @p cf */
  const TSequence *seq2;
  DistPoseCache cache1;        /**< Pose of each operand at the end of the */
  DistPoseCache cache2;        /**< last interval walked */
  DistEvents events;           /**< Events of the last interval walked */
  bool failed;                 /**< True if a walk failed */
} DistEverWalk;

/**
 * @brief Return true if two temporal rigid geometries are within a distance
 * at some instant of an interval of #tcont_tcont_merge_walk
 * @details The values compared with the distance are those #dist_ever_seq()
 * reads on the same interval of the synchronized sequences: the walk of the
 * segment from the poses at the bounds of the interval, see #dist_seq_pose(),
 * or the distance of the placed bodies where the two share one instant. An
 * interval the centers of the bodies keep apart, see #dist_segm_apart(), is
 * not walked.
 */
static bool
trgeometry_dwithin_interval(const TSequence *seq1, int k1, const TSequence *seq2,
  int k2, TimestampTz lower, TimestampTz upper, void *state)
{
  DistEverWalk *dw = (DistEverWalk *) state;
  bool alloc1, alloc2, alloc3, alloc4;
  /* The poses at the start of the interval are those at the end of the last
   * one where the segment is the same */
  const Pose *p1 = dist_seq_pose_cached(&dw->cache1, seq1, k1, lower, &alloc1);
  const Pose *p2 = dist_seq_pose_cached(&dw->cache2, seq2, k2, lower, &alloc2);
  bool result = false;
  if (lower == upper)
  {
    /* The two values share one instant: the distance of the placed bodies */
    GSERIALIZED *geo1 = pose_apply_geo(p1, dw->ops->ref1);
    GSERIALIZED *geo2 = pose_apply_geo(p2, dw->ops->ref2);
    /* A distance raising an error, -1.0, places the bodies within no
     * distance */
    double d = geom_distance2d(geo1, geo2);
    result = d >= 0.0 && d <= dw->dist;
    pfree(geo1); pfree(geo2);
  }
  else
  {
    const Pose *q1 = dist_seq_pose(seq1, k1, upper, &alloc3);
    const Pose *q2 = dist_seq_pose(seq2, k2, upper, &alloc4);
    DistMotion ma, mb;
    distmotion_from_pose(&ma, p1, q1);
    distmotion_from_pose(&mb, p2, q2);
    DistSegm dseg;
    distsegm_set(&dseg, &ma, &mb);
    if (dist_segm_apart(&dseg, &dw->ops->body1, &dw->ops->body2, dw->dist))
      /* The closest feature at the start of the next interval is unknown */
      dw->cf.kind = DISTPAIR_NONE;
    else
    {
      /* The closest feature at the end of the last interval walked starts the
       * walk of the interval that follows it in the same two sequences */
      if (dw->cft != lower || dw->seq1 != seq1 || dw->seq2 != seq2)
        dw->cf.kind = DISTPAIR_NONE;
      dw->events.count = 0;
      if (distwalk_segm(&dseg, &dw->ops->body1, &dw->ops->body2,
          distwalk_ftol(&dseg, &dw->ops->body1, &dw->ops->body2),
          dw->ops->level, &dw->cf, &dw->events) != DISTWALK_OK)
      {
        dw->failed = true;
        result = true;
      }
      else
      {
        dw->cft = upper;
        dw->seq1 = seq1;
        dw->seq2 = seq2;
        for (int i = 0; i < dw->events.count && ! result; i++)
          result = dw->events.ev[i].dist <= dw->dist;
      }
    }
    /* The poses at the end of the interval start the next one */
    dist_pose_cache_set(&dw->cache1, seq1, k1, upper, q1, alloc3);
    dist_pose_cache_set(&dw->cache2, seq2, k2, upper, q2, alloc4);
  }
  if (alloc1) pfree((Pose *) p1);
  if (alloc2) pfree((Pose *) p2);
  return result;
}

/**
 * @brief Return 1 if two temporal rigid geometries are ever within a distance,
 * 0 if not, -1 if they do not intersect in time
 * @details The answer is that of the minimum of their temporal distance, as
 * #tdistance_trgeometry_trgeometry_intl() gives it: the values of that distance are read
 * until the first one at or below @p dist, and a segment that the centers of
 * the bodies keep apart is not walked, see #dist_ever_seq(). Two sequences
 * with linear interpolation are walked over their merged instants without
 * being synchronized, see #trgeometry_dwithin_interval().
 * @param[in] temp1,temp2 Temporal rigid geometries
 * @param[in] dist Distance
 * @pre The arguments are valid and planar, as the external entry tests
 * them
 */
int
edwithin_trgeometry_trgeometry_intl(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  assert(temp1); assert(temp2);
  assert(temp1->temptype == T_TRGEOMETRY);
  assert(temp2->temptype == T_TRGEOMETRY);
  assert(dist >= 0.0);
  assert(! MEOS_FLAGS_GET_Z(temp1->flags));
  assert(! MEOS_FLAGS_GET_Z(temp2->flags));
  DistOps ops;
  memset(&ops, 0, sizeof(DistOps));
  ops.ref1 = trgeometry_geom_p(temp1);
  ops.ref2 = trgeometry_geom_p(temp2);
  ops.level = -1.0;
  if (! dist_ref_body(ops.ref1, &ops.body1))
    return -1;
  if (! dist_ref_body(ops.ref2, &ops.body2))
  {
    distrefpoly_free(&ops.body1);
    return -1;
  }

  int result = -1;
  if (temp1->subtype != TINSTANT && temp2->subtype != TINSTANT &&
      MEOS_FLAGS_LINEAR_INTERP(temp1->flags) &&
      MEOS_FLAGS_LINEAR_INTERP(temp2->flags))
  {
    DistEverWalk dw;
    memset(&dw, 0, sizeof(DistEverWalk));
    dw.ops = &ops;
    dw.dist = dist;
    dw.cf.kind = DISTPAIR_NONE;
    dw.cft = DT_NOBEGIN;
    distevents_init(&dw.events);
    bool shared;
    bool found = tcont_tcont_merge_walk(temp1, temp2, &trgeometry_dwithin_interval,
      &dw, &shared);
    dist_pose_cache_set(&dw.cache1, NULL, 0, 0, NULL, false);
    dist_pose_cache_set(&dw.cache2, NULL, 0, 0, NULL, false);
    distevents_free(&dw.events);
    distrefpoly_free(&ops.body1);
    distrefpoly_free(&ops.body2);
    if (dw.failed)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "The temporal distance of a temporal rigid geometry failed");
      return -1;
    }
    return shared ? (found ? 1 : 0) : -1;
  }

  /* The synchronization of #tdistance_trgeometry_temporal() */
  Temporal *sync1, *sync2;
  if (intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
  {
    assert(temptype_subtype(sync1->subtype));
    if (sync1->subtype == TINSTANT)
      result = (dist_instant(&ops, (const TInstant *) sync1,
        (const TInstant *) sync2) <= dist) ? 1 : 0;
    else if (sync1->subtype == TSEQUENCE)
      result = dist_ever_seq(&ops, (const TSequence *) sync1,
        (const TSequence *) sync2, dist);
    else /* TSEQUENCESET */
    {
      const TSequenceSet *ss1 = (const TSequenceSet *) sync1;
      const TSequenceSet *ss2 = (const TSequenceSet *) sync2;
      result = 0;
      for (int i = 0; i < ss1->count && result == 0; i++)
        result = dist_ever_seq(&ops, TSEQUENCESET_SEQ_N(ss1, i),
          TSEQUENCESET_SEQ_N(ss2, i), dist);
    }
    pfree(sync1); pfree(sync2);
  }
  distrefpoly_free(&ops.body1);
  distrefpoly_free(&ops.body2);
  return result;
}

/**
 * @brief Return a temporal distance with its sequences normalized
 * @details The internal functions keep each instant of the walk.  The
 * normalization of MEOS removes an instant whose value is within
 * #MEOS_EPSILON of the line through its neighbors, and such an instant can
 * be the minimum or a crossing of a threshold.  Thus the nearest approach
 * and the dwithin functions use the internal functions, and only the result
 * of the temporal distance functions is normalized, as are the other
 * temporal distances of MEOS.
 * @param[in] temp Temporal distance, freed
 */
static Temporal *
dist_normalize(Temporal *temp)
{
  if (! temp || temp->subtype == TINSTANT)
    return temp;
  Temporal *result;
  if (temp->subtype == TSEQUENCE)
  {
    const TSequence *seq = (const TSequence *) temp;
    int count;
    const TInstant **instants = tsequence_insts_p(seq, &count);
    result = (Temporal *) tsequence_make((TInstant **) instants, count,
      seq->period.lower_inc, seq->period.upper_inc,
      MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
    pfree(instants);
  }
  else
  {
    const TSequenceSet *ss = (const TSequenceSet *) temp;
    TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
    for (int i = 0; i < ss->count; i++)
    {
      const TSequence *seq = TSEQUENCESET_SEQ_N(ss, i);
      int count;
      const TInstant **instants = tsequence_insts_p(seq, &count);
      sequences[i] = tsequence_make((TInstant **) instants, count,
        seq->period.lower_inc, seq->period.upper_inc,
        MEOS_FLAGS_GET_INTERP(seq->flags), NORMALIZE);
      pfree(instants);
    }
    result = (Temporal *) tsequenceset_make_free(sequences, ss->count,
      NORMALIZE);
  }
  pfree(temp);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and
 * a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_geo() #Tdistance_geo_trgeometry()
 */
Temporal *
tdistance_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    Temporal * result = tdistance_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  return dist_normalize(tdistance_trgeometry_geo_intl(temp, gs, -1.0));
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and a
 * temporal geometry point
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_tpoint() #Tdistance_tpoint_trgeometry()
 */
Temporal *
tdistance_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_NOT_NULL(temp2, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    Temporal * result = tdistance_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  return dist_normalize(tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0));
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the temporal distance between two temporal rigid geometries
 * @sqlop @p <->
 * @csqlfn #Tdistance_trgeometry_trgeometry()
 */
Temporal *
tdistance_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  return dist_normalize(tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0));
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal rigid
 * geometry and a geometry are within a distance
 * @details The temporal distance has an instant at each time at which it
 * crosses @p dist, thus the comparison of its linear interpolation with
 * @p dist is exact.
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] dist Distance
 * @csqlfn #Tdwithin_trgeometry_geo()
 */
Temporal *
tdwithin_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  if (! ensure_finite(dist))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D, as
   * #tdistance_trgeometry_geo does */
  GSERIALIZED *flat = trgeometry_geo_common_dims(temp, gs);
  if (flat)
  {
    Temporal *result = tdwithin_trgeometry_geo(temp, flat, dist);
    pfree(flat);
    return result;
  }
  if (! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8) ||
      ! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* An empty geometry has no point, so it meets no value at any instant */
  if (gserialized_is_empty(gs))
    return temporal_from_base_temp(BoolGetDatum(false), T_TBOOL, temp);
  Temporal *tdist = tdistance_trgeometry_geo_intl(temp, gs, dist);
  if (! tdist)
    return NULL;
  Temporal *result = tcomp_temporal_base(tdist, Float8GetDatum(dist), &datum2_le);
  pfree(tdist);
  return result;
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal boolean that states whether a geometry and a
 * temporal rigid geometry are within a distance
 * @csqlfn #Tdwithin_geo_trgeometry()
 */
Temporal *
tdwithin_geo_trgeometry(const GSERIALIZED *gs, const Temporal *temp,
  double dist)
{
  if (! ensure_finite(dist))
    return NULL;
  return tdwithin_trgeometry_geo(temp, gs, dist);
}

/**
 * @ingroup meos_rgeo_rel_temp
 * @brief Return a temporal boolean that states whether two temporal rigid
 * geometries are within a distance
 * @details See #tdwithin_trgeometry_geo()
 * @csqlfn #Tdwithin_trgeometry_trgeometry()
 */
Temporal *
tdwithin_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2,
  double dist)
{
  if (! ensure_finite(dist))
    return NULL;
  if (! ensure_not_negative_datum(Float8GetDatum(dist), T_FLOAT8) ||
      ! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;
  Temporal *tdist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, dist);
  if (! tdist)
    return NULL;
  Temporal *result = tcomp_temporal_base(tdist, Float8GetDatum(dist), &datum2_le);
  pfree(tdist);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid geometry
 * and a geometry
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_geo() #NAI_geo_trgeometry()
 */
TInstant *
nai_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    TInstant * result = nai_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      gserialized_is_empty(gs))
    return NULL;

  /* Also an instant goes through the temporal distance, which refuses the
   * same reference geometries for all subtypes */
  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_geo_intl(temp, gs, -1.0);
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp, min->t, false, &value);
    result = trgeometryinst_make(trgeometry_geom_p(temp), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid
 * geometry and a temporal point
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_tpoint() #NAI_tpoint_trgeometry()
 */
TInstant *
nai_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_NOT_NULL(temp2, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    TInstant * result = nai_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0);
  if (dist != NULL)
  {
    /* The instant is read in place, dist being freed only after it */
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeometry_geom_p(temp1), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach instant between two temporal rigid
 * geometries
 * @sqlfn nearestApproachInstant()
 * @csqlfn #NAI_trgeometry_trgeometry()
 */
TInstant *
nai_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  TInstant *result = NULL;
  Temporal *dist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0);
  if (dist != NULL)
  {
    /* The instant is read in place, dist being freed only after it */
    const TInstant *min = temporal_min_inst_p(dist);
    /* The closest point may be at an exclusive bound. */
    Datum value;
    temporal_value_at_timestamptz(temp1, min->t, false, &value);
    result = trgeometryinst_make(trgeometry_geom_p(temp1), DatumGetPoseP(value),
      min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a geometry
 * @csqlfn #NAD_trgeometry_geo() #NAD_geo_trgeometry()
 * @errval -1.0
 */
double
nad_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, -1.0); VALIDATE_NOT_NULL(gs, -1.0);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1.0;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    double result = nad_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1.0;
  /* An empty geometry has no point to measure from, so the operands have no
   * distance, which is an answer */
  if (gserialized_is_empty(gs))
    return DBL_MAX;

  /* A geometry is defined at every instant of the temporal value, so the
   * temporal distance is NULL only where it raises an error */
  Temporal *dist = tdistance_trgeometry_geo_intl(temp, gs, -1.0);
  if (! dist)
    return -1.0;
  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a spatiotemporal box
 * @csqlfn #NAD_trgeometry_stbox() #NAD_stbox_trgeometry()
 * @errval -1.0
 */
double
nad_trgeometry_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_stbox(temp, box) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags))
    return -1.0;

  /* Project the temporal value to the timespan of the box */
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  Temporal *temp1 = (Temporal *) temp;
  if (hast)
  {
    temporal_set_tstzspan(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return DBL_MAX;
    /* The restriction keeps the reference geometry, which it strips before
     * restricting the poses and gives back to the result */
    temp1 = temporal_restrict_tstzspan(temp, &inter, REST_AT);
    /* The two spans meet while no value lies inside the intersection, which a
     * discrete or a step value can do */
    if (! temp1)
      return DBL_MAX;
  }
  /* Convert the stbox to a geometry */
  GSERIALIZED *geo = stbox_geo(box);
  /* Compute the result. The time the operands share is decided above, so the
   * temporal distance is NULL only where it raises an error */
  Temporal *dist = tdistance_trgeometry_geo_intl(temp1, geo, -1.0);
  double result = -1.0;
  if (dist)
  {
    result = DatumGetFloat8(temporal_min_value(dist));
    pfree(dist);
  }
  pfree(geo);
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a temporal rigid geometry
 * @errval -1.0
 */
double
nad_stbox_trgeometry(const STBox *box, const Temporal *temp)
{
  return nad_trgeometry_stbox(temp, box);
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a temporal point
 * @csqlfn #NAD_trgeometry_tpoint() #NAD_tpoint_trgeometry()
 * @errval -1.0
 */
double
nad_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp1, -1.0); VALIDATE_NOT_NULL(temp2, -1.0);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return -1.0;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    double result = nad_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return -1.0;

  int last_errno = meos_errno_reset();
  Temporal *dist = tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0);
  /* The temporal distance is NULL where the operands share no time, which
   * is an answer, and where it raises an error */
  double result = DBL_MAX;
  if (dist)
  {
    result = DatumGetFloat8(temporal_min_value(dist));
    pfree(dist);
  }
  return distance_result(result, last_errno);
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @csqlfn #NAD_trgeometry_trgeometry()
 * @errval -1.0
 */
double
nad_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return -1.0;

  int last_errno = meos_errno_reset();
  Temporal *dist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0);
  /* The temporal distance is NULL where the operands share no time, which
   * is an answer, and where it raises an error */
  double result = DBL_MAX;
  if (dist)
  {
    result = DatumGetFloat8(temporal_min_value(dist));
    pfree(dist);
  }
  return distance_result(result, last_errno);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a geometry
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_geo() #Shortestline_geo_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the geometry */
  if (! ensure_has_not_Z(temp->temptype, temp->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D geometry in 2D */
  if (FLAGS_GET_Z(gs->gflags))
  {
    GSERIALIZED *flat = geo_force2d_intl(gs);
    GSERIALIZED * result = shortestline_trgeometry_geo(temp, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_geo(temp, gs) ||
      ! ensure_has_not_Z(temp->temptype, temp->flags) ||
      gserialized_is_empty(gs))
    return NULL;

  Temporal *dist = tdistance_trgeometry_geo_intl(temp, gs, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value;
  trgeometry_value_at_timestamptz_intl(temp, inst->t, false, &value);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value), gs);
  pfree(DatumGetPointer(value)); pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a temporal geometry point
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_tpoint() #Shortestline_tpoint_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_tpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp1, NULL); VALIDATE_NOT_NULL(temp2, NULL);
  /* The distance of a temporal rigid geometry in three dimensions is not
   * yet supported, whatever the dimensions of the temporal point */
  if (! ensure_has_not_Z(temp1->temptype, temp1->flags))
    return NULL;
  /* A 2D temporal rigid geometry reads a 3D temporal point in 2D */
  if (tpoint_type(temp2->temptype) && MEOS_FLAGS_GET_Z(temp2->flags))
  {
    Temporal *flat = tgeo_force2d_intl(temp2);
    GSERIALIZED * result = shortestline_trgeometry_tpoint(temp1, flat);
    pfree(flat);
    return result;
  }
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_tpoint(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *dist = tdistance_trgeometry_tpoint_intl(temp1, temp2, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeometry_value_at_timestamptz_intl(temp1, inst->t, false, &value1);
  temporal_value_at_timestamptz(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/**
 * @ingroup meos_rgeo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal rigid geometries
 * @sqlfn shortestLine()
 * @csqlfn #Shortestline_trgeometry_trgeometry()
 */
GSERIALIZED *
shortestline_trgeometry_trgeometry(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeometry_trgeometry(temp1, temp2) ||
      ! ensure_has_not_Z(temp1->temptype, temp1->flags) ||
      ! ensure_has_not_Z(temp2->temptype, temp2->flags))
    return NULL;

  Temporal *dist = tdistance_trgeometry_trgeometry_intl(temp1, temp2, -1.0);
  if (dist == NULL)
    return NULL;
  const TInstant *inst = temporal_min_inst_p(dist);
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  trgeometry_value_at_timestamptz_intl(temp1, inst->t, false, &value1);
  trgeometry_value_at_timestamptz_intl(temp2, inst->t, false, &value2);
  GSERIALIZED *result = geom_shortestline2d(DatumGetGserializedP(value1),
    DatumGetGserializedP(value2));
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2)); pfree(dist);
  return result;
}

/*****************************************************************************/
