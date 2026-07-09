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
 * @brief Fast 2D/3D temporal point clipping against 2D geometries
 * @details Support (multi)point, (multi)line, triangle, (multi)polygons with
 * holes and islands inside holes (recursively), and collection of the above
 * @note Avoid processing in GEOS to improve performance
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include "postgres.h"
#include <utils/float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
/* MEOS */
#include "meos.h"
#include "meos_internal_geo.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/temporal_restrict.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"

/* Minimum number of edges to use an R-tree index in order to compensate the
 * overhead of the tree construction and destruction */
#define RTREE_MIN_NUMBER_ELEMS 100

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/* Per-thread arrays for accumulating the results of the clipping process.
 * MEOS_TLS is required: concurrent callers from different threads would
 * otherwise race on these file-scope pointers, causing heap corruption. */
static MEOS_TLS MeosArray *events = NULL;
static MEOS_TLS MeosArray *intervals = NULL;
static MEOS_TLS MeosArray *periods = NULL;
static MEOS_TLS MeosArray *rtree_results = NULL;

/**
 * @brief Enumeration defining the edge types 
 */
typedef enum
{
  EDGE_POINT = 0,
  EDGE_LINE,
  EDGE_POLY,
  EDGE_ARC,
  EDGE_POLYARC
} EdgeType;

/**
 * @brief Structure keeping a geometry edge
 */
typedef struct
{
  double x1, y1, x2, y2;         /**< Coordinates of the start/end 2D points */
  double xmin, ymin, xmax, ymax; /**< Precomputed bounding box of the edge */
  double dx, dy, length;         /**< Precomputed dx, dy, and length */
  double cx, cy, radius;         /**< Arc center and radius (EDGE_ARC only) */
  double theta0, theta1;         /**< Arc start/end angles (EDGE_ARC only) */
  bool ccw;                      /**< Arc traversed counterclockwise (EDGE_ARC) */
  EdgeType etype;                /**< Edge type */
} Edge;

/**
 * @brief Enumeration defining the intersection types 
 */
typedef enum
{
  INTERSECT_NONE = 0,
  INTERSECT_POINT,
  INTERSECT_OVERLAP
} IntersectType;

/**
 * @brief Structure keeping an intersection result
 */
typedef struct
{
  IntersectType type;  /**< Intersection type */
  double t0;           /**< Always valid if type != NONE */
  double t1;           /**< Only valid for OVERLAP */
} IntersectResult;

/*****************************************************************************
 * Line segment intersection
 *****************************************************************************/

/**
 * @brief Return the intersection value obtained by computing the intersection 
 * of a line segment defined by two 2D points intersects an edge
 * @details Possible result values
 * - No intersection: INTERSECT_NONE -> t0 and t1 undefined
 * - Single point: INTERSECT_POINT -> t1 in [0,1], t1 ignored
 * - Overlap segment: INTERSECT_OVERLAP -> t0 <= t1 in [0,1]
 * Invariants:
 * - 0 <= t0 <= 1
 * - 0 <= t1 <= 1
 * - t0 <= t1
 * - Overlap must satisfy: t1 - t0 > FP_TOLERANCE
 * @param[in] ax,ay Coordinates of the first point defining the first segment
 * @param[in] rx,ry Vector AB
 * @param[in] cx,cy,dx,dy Coordinates of the points defining the second segment
 * @note To avoid recomputing vector AB in EVERY call to the functions,
 * we pass the vector instead of the second point b computed as follows
 * @code
 * double rx = bx - ax, ry = by - ay;
 * @endcode
 */
static inline IntersectResult
linesegm_intersect(double ax, double ay, double rx, double ry,
  double cx, double cy, double dx, double dy)
{
  IntersectResult res = {INTERSECT_NONE, 0, 0};
  double sx = dx - cx, sy = dy - cy; /* vector CD */
  /* Where is the start of the second segment relative to the first? */
  double qpx = cx - ax, qpy = cy - ay;

  /* Are the two segments parallel?  */
  double rxs = rx * sy - ry * sx;

  /* Collinear / parallel */
  if (fabs(rxs) < FP_TOLERANCE)
  {
    /* Is point C aligned with segment AB? */
    double qpxr = qpx * ry - qpy * rx;
    /* If qpxr != 0: parallel, if qpxr == 0: collinear */
    if (fabs(qpxr) > FP_TOLERANCE)
      return res;

    /* Collinear case */
    double r2 = rx * rx + ry * ry;
    if (r2 < FP_TOLERANCE)
      return res;

    double t0 = (qpx * rx + qpy * ry) / r2;
    double t1 = t0 + (sx * rx + sy * ry) / r2;

    /* Order t0 < t1 */
    if (t0 > t1) { double tmp = t0; t0 = t1; t1 = tmp; }
    /* No intersection */
    if (t1 < 0 || t0 > 1)
      return res;

    /* Clamp values */
    if (t0 < 0) t0 = 0;
    if (t1 > 1) t1 = 1;

    if (fabs(t1 - t0) < FP_TOLERANCE)
    {
      res.type = INTERSECT_POINT;
      res.t0 = t0;
      return res;
    }

    res.type = INTERSECT_OVERLAP;
    res.t0 = t0;
    res.t1 = t1;
    return res;
  }

  /* Proper intersection */
  double t = (qpx * sy - qpy * sx) / rxs;
  double u = (qpx * ry - qpy * rx) / rxs;

  if (t < -FP_TOLERANCE || t > 1 + FP_TOLERANCE ||
      u < -FP_TOLERANCE || u > 1 + FP_TOLERANCE)
    return res;

  /* Clamp values */
  if (fabs(t) < FP_TOLERANCE) t = 0;
  if (fabs(t - 1) < FP_TOLERANCE) t = 1;

  res.type = INTERSECT_POINT;
  res.t0 = t;
  return res;
}

/*****************************************************************************
 * Circular arc intersection
 *****************************************************************************/

/**
 * @brief Normalize an angle into the range [0, 2*pi)
 */
static inline double
angle_normalize(double a)
{
  double r = fmod(a, 2 * M_PI);
  if (r < 0)
    r += 2 * M_PI;
  return r;
}

/**
 * @brief Return true if an angle lies within the angular span of an arc edge
 * @details The span is traversed from #theta0 to #theta1, counterclockwise
 * when #ccw is true and clockwise otherwise
 */
static bool
arc_contains_angle(const Edge *e, double phi)
{
  double sweep = e->ccw ?
    angle_normalize(e->theta1 - e->theta0) :
    angle_normalize(e->theta0 - e->theta1);
  double off = e->ccw ?
    angle_normalize(phi - e->theta0) :
    angle_normalize(e->theta0 - phi);
  return off <= sweep + FP_TOLERANCE;
}

/**
 * @brief Set the bounding box of an arc edge
 * @details The box spans the two endpoints plus any of the four cardinal
 * extreme points of the circle that fall within the arc's angular span
 */
static void
arc_set_bbox(Edge *e)
{
  double xmin = FP_MIN(e->x1, e->x2), xmax = FP_MAX(e->x1, e->x2);
  double ymin = FP_MIN(e->y1, e->y2), ymax = FP_MAX(e->y1, e->y2);
  const double ang[4] = {0.0, M_PI_2, M_PI, -M_PI_2};
  const double ex[4] = {e->cx + e->radius, e->cx, e->cx - e->radius, e->cx};
  const double ey[4] = {e->cy, e->cy + e->radius, e->cy, e->cy - e->radius};
  for (int k = 0; k < 4; k++)
    if (arc_contains_angle(e, ang[k]))
    {
      if (ex[k] < xmin) xmin = ex[k];
      if (ex[k] > xmax) xmax = ex[k];
      if (ey[k] < ymin) ymin = ey[k];
      if (ey[k] > ymax) ymax = ey[k];
    }
  e->xmin = xmin; e->xmax = xmax; e->ymin = ymin; e->ymax = ymax;
  return;
}

/**
 * @brief Return true if a point is located on an arc edge
 */
static bool
point_on_arc(double px, double py, const Edge *e)
{
  double d = hypot(px - e->cx, py - e->cy);
  if (fabs(d - e->radius) > FP_TOLERANCE)
    return false;
  return arc_contains_angle(e, atan2(py - e->cy, px - e->cx));
}

/**
 * @brief Return the trajectory parameters at which a trajectory segment
 * intersects an arc edge
 * @details Solves |A + t*R - C|^2 = r^2 for the trajectory parameter t in
 * [0,1], keeping only the roots whose point lies within the arc's angular
 * span. A straight segment meets a circle in at most two points, so the
 * result is never an overlap
 * @param[in] ax,ay Coordinates of the start of the trajectory segment
 * @param[in] rx,ry Vector of the trajectory segment
 * @param[in] e Arc edge
 * @param[out] out Accepted trajectory parameters, ordered as found
 * @return Number of accepted parameters (0, 1, or 2)
 */
static int
arcsegm_intersect(double ax, double ay, double rx, double ry, const Edge *e,
  double out[2])
{
  double aa = rx * rx + ry * ry;
  /* Degenerate (zero-length) trajectory segment */
  if (aa < FP_TOLERANCE)
    return 0;

  double wx = ax - e->cx, wy = ay - e->cy;
  double bb = 2 * (wx * rx + wy * ry);
  double cc = wx * wx + wy * wy - e->radius * e->radius;
  double disc = bb * bb - 4 * aa * cc;
  /* No real root */
  if (disc < -FP_TOLERANCE)
    return 0;
  if (disc < 0)
    disc = 0;

  double sq = sqrt(disc);
  double roots[2];
  int nroots = 0;
  roots[nroots++] = (-bb - sq) / (2 * aa);
  /* Distinct second root only when the line is not tangent */
  if (sq > FP_TOLERANCE)
    roots[nroots++] = (-bb + sq) / (2 * aa);

  int n = 0;
  for (int k = 0; k < nroots; k++)
  {
    double t = roots[k];
    if (t < -FP_TOLERANCE || t > 1 + FP_TOLERANCE)
      continue;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    double px = ax + t * rx, py = ay + t * ry;
    if (arc_contains_angle(e, atan2(py - e->cy, px - e->cx)))
      out[n++] = t;
  }
  return n;
}

/*****************************************************************************
 * Compute the intervals in [0,1] resulting from the intersection of a
 * trajectory segment and an array of edges obtained from a (collection of)
 * polygon/line/point geometries
 *****************************************************************************/

/**
 * @brief Return true if a point is located on a segment
 */
static bool
point_on_segment(double px, double py, double x1, double y1, double x2,
  double y2)
{
  /* Vectors AP and AB */
  double apx = px - x1;
  double apy = py - y1;
  double abx = x2 - x1;
  double aby = y2 - y1;

  /* Fast bounding-box rejection */
  if ((px < fmin(x1, x2) - FP_TOLERANCE) ||
      (px > fmax(x1, x2) + FP_TOLERANCE) ||
      (py < fmin(y1, y2) - FP_TOLERANCE) ||
      (py > fmax(y1, y2) + FP_TOLERANCE))
    return false;

  /* Collinearity check via cross product */
  double cross = apx * aby - apy * abx;
  if (fabs(cross) > FP_TOLERANCE)
    return false;

  /* Projection check via dot product */
  double dot = apx * abx + apy * aby;
  if (dot < -FP_TOLERANCE)
    return false;

  /* Check if P lies between A and B */
  double ab2 = abx * abx + aby * aby;
  if (dot > ab2 + FP_TOLERANCE)
    return false;
  return true;
}

/**
 * @brief Return true if a point is located in a polygon 
 */
static inline int
point_in_polygon(double x, double y, Edge **edges, int nedges)
{
  int inside = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *restrict e = edges[i];

    /* Only polygon boundary edges bound a region. Point, line, and standalone
     * (1D) arc edges are ignored by the even-odd containment test */
    if (e->etype == EDGE_POLYARC)
    {
      /* Boundary check */
      if (point_on_arc(x, y, e))
        return 1;
      /* Cast a ray towards +x. The horizontal line at height y meets the
       * supporting circle at cx +/- sqrt(r^2 - (y - cy)^2); flip the parity
       * for each crossing that lies strictly to the right of the point and
       * within the arc's angular span. A ray that only grazes the circle
       * tangentially (h2 ~ 0) does not cross the boundary */
      const double dyc = y - e->cy;
      const double h2 = e->radius * e->radius - dyc * dyc;
      if (h2 <= FP_TOLERANCE)
        continue;
      const double h = sqrt(h2);
      const double xhit[2] = {e->cx - h, e->cx + h};
      /* Forward traversal direction of the arc in the angle parameter */
      const double s = e->ccw ? 1.0 : -1.0;
      for (int k = 0; k < 2; k++)
      {
        const double xi = xhit[k];
        if (xi <= x)
          continue;
        const double phi = atan2(dyc, xi - e->cx);
        if (! arc_contains_angle(e, phi))
          continue;
        /* Half-open ownership, mirroring the straight-edge
         * (y1 > y) != (y2 > y) rule below: a crossing shared with a
         * neighbouring edge (a ring junction lying exactly on the ray) must be
         * counted once. A crossing at an arc endpoint is owned by this edge
         * only if the arc's interior rises above the ray there; an interior
         * crossing is always transversal and always counted */
        const bool at_ep0 = fabs(xi - e->x1) < FP_TOLERANCE &&
          fabs(y - e->y1) < FP_TOLERANCE;
        const bool at_ep1 = fabs(xi - e->x2) < FP_TOLERANCE &&
          fabs(y - e->y2) < FP_TOLERANCE;
        if (at_ep0 || at_ep1)
        {
          const double theta_e = at_ep0 ? e->theta0 : e->theta1;
          const double dtheta_in = at_ep0 ? s : -s;
          if (dtheta_in * cos(theta_e) <= 0)
            continue;
        }
        inside ^= 1;
      }
      continue;
    }

    if (e->etype != EDGE_POLY)
      continue;

    const double dx  = e->dx;
    const double dy  = e->dy;
    const double x1  = e->x1;
    const double y1  = e->y1;

    const double dxp = x - x1;
    const double dyp = y - y1;

    /* Boundary check */
    const double cross = dx * dyp - dy * dxp;
    if (fabs(cross) < FP_TOLERANCE)
    {
      const double dot = dxp * dx + dyp * dy;
      if (dot >= -FP_TOLERANCE && dot <= (e->length) + FP_TOLERANCE)
        return 1;
    }

    /* Ray casting */
    if ((y1 > y) != ((y1 + dy) > y))
    {
      const double rhs = dx * dyp;
      const double lhs = dxp * dy;
      inside ^= ((dy > 0) ? (rhs > lhs) : (rhs < lhs));
    }
  }
  return inside;
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of point edges
 */
static void
intervals_from_points(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  /* Segment vector */
  double dx = b->x - a->x;
  double dy = b->y - a->y;

  /* Improve performance by removing the division inside the loop */
  bool use_x = fabs(dx) >= fabs(dy);
  double inv = use_x ?
    ((fabs(dx) > FP_TOLERANCE) ? 1.0 / dx : 0.0) :
    ((fabs(dy) > FP_TOLERANCE) ? 1.0 / dy : 0.0);

  if (inv == 0.0)
    return;
  
  /* Iterate through the points */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i]; 
    /* Iterate only for the points */
    if (e->etype != EDGE_POINT)
      continue;
    // assert(e->x1 == e->x2 && e->y1 == e->y2);

    /* Solve parameter t */
    double t = use_x ? (e->x1 - a->x) * inv : (e->y1 - a->y) * inv;
    /* Check bounds */
    if (t < -FP_TOLERANCE || t > 1.0 + FP_TOLERANCE)
      continue;

    /* Reconstruct point and add interval */
    double x = a->x + t * dx;
    double y = a->y + t * dy;
    if (fabs(x - e->x1) < FP_TOLERANCE && fabs(y - e->y1) < FP_TOLERANCE)
    {
      Span in;
      span_set(Float8GetDatum(t), Float8GetDatum(t), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }
  return;
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of linear or point edges
 */
static void
intervals_from_lines(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;

  /* Segment bounding box */
  const double seg_xmin = FP_MIN(ax, bx);
  const double seg_xmax = FP_MAX(ax, bx);
  const double seg_ymin = FP_MIN(ay, by);
  const double seg_ymax = FP_MAX(ay, by);
  /* Segment vector */
  const double rx = bx - ax;  
  const double ry = by - ay;  

  bool has_intersection = false;
  Span in;

  /* Iterate through the lines */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    /* Iterate only for the line edges */
    if (e->etype != EDGE_LINE)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    /* Compute the intersection */
    IntersectResult r = linesegm_intersect(ax, ay, rx, ry,
      e->x1, e->y1, e->x2, e->y2);
    /* If there is no intersection  */
    if (r.type == INTERSECT_NONE)
      continue;

    /* Intersection found: compute the interval */
    has_intersection = true;
    if (r.type == INTERSECT_POINT)
      span_set(Float8GetDatum(r.t0), Float8GetDatum(r.t0), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
    else
      span_set(Float8GetDatum(r.t0), Float8GetDatum(r.t1), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
    meos_array_add(intervals, &in);
  }

  /* Full collinear segment */
  if (! has_intersection)
  {
    /* Test midpoint */
    double mx = (ax + bx) * 0.5;
    double my = (ay + by) * 0.5;
    for (int i = 0; i < nedges; i++)
    {
      const Edge *e = edges[i];
      /* Iterate only for the lines edges */
      if (e->etype != EDGE_LINE)
        continue;

      /* Fast bbox check first */
      if (mx < e->xmin - FP_TOLERANCE || mx > e->xmax + FP_TOLERANCE ||
          my < e->ymin - FP_TOLERANCE || my > e->ymax + FP_TOLERANCE)
        continue;

      if (point_on_segment(mx, my, e->x1, e->y1, e->x2, e->y2))
      {
        span_set(Float8GetDatum(0.0), Float8GetDatum(1.0), true, true,
          T_FLOAT8, T_FLOATSPAN, &in);
        meos_array_add(intervals, &in);
        return;
      }
    }
  }
  return;
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of arc edges
 */
static void
intervals_from_arcs(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;
  /* Segment bounding box */
  const double seg_xmin = FP_MIN(ax, bx);
  const double seg_xmax = FP_MAX(ax, bx);
  const double seg_ymin = FP_MIN(ay, by);
  const double seg_ymax = FP_MAX(ay, by);
  /* Segment vector */
  const double rx = bx - ax;
  const double ry = by - ay;

  Span in;
  /* Iterate through the arc edges */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_ARC)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    /* Compute the intersection: at most two point crossings */
    double t[2];
    int n = arcsegm_intersect(ax, ay, rx, ry, e, t);
    for (int k = 0; k < n; k++)
    {
      span_set(Float8GetDatum(t[k]), Float8GetDatum(t[k]), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }
  return;
}

/**
 * @brief Comparison function for sorting float8 values
 */
static int
float8_qsort_cmp(const void *a1, const void *a2)
{
  double diff = *(const double *)a1 - *(const double *)a2;
  return (diff > 0) - (diff < 0);
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of polygon edges
 * @details For the #point_in_polygon ray-casting we must use ALL edges, not
 * only those filtered by the R-tree, otherwise the even-odd containment test
 * breaks
 */
static void
intervals_from_polygons(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges, Edge **all_edges, int all_nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  /* Reset event array */
  events->count = 0;

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;

  /* Segment bounding box */
  const double seg_xmin = FP_MIN(ax, bx);
  const double seg_xmax = FP_MAX(ax, bx);
  const double seg_ymin = FP_MIN(ay, by);
  const double seg_ymax = FP_MAX(ay, by);
  /* Segment vector */
  const double rx = bx - ax;
  const double ry = by - ay;

  /* Check whether any polygon boundary edges exist using the full edge array.
   * A curve polygon contributes straight (EDGE_POLY) and arc (EDGE_POLYARC)
   * boundary edges */
  bool has_polys = false;
  for (int i = 0; i < all_nedges; i++)
  {
    EdgeType et = all_edges[i]->etype;
    if (et == EDGE_POLY || et == EDGE_POLYARC)
    {
      has_polys = true;
      break;
    }
  }
  /* If no polygon edges have been found, we do not continue */
  if (! has_polys)
    return;

  /* Collect all intersection parameters from the (possibly filtered) edges */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    /* Iterate only for the polygon boundary edges (straight or arc) */
    if (e->etype != EDGE_POLY && e->etype != EDGE_POLYARC)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    if (e->etype == EDGE_POLY)
    {
      /* Compute the crossing with the straight boundary segment */
      IntersectResult r = linesegm_intersect(ax, ay, rx, ry,
        e->x1, e->y1, e->x2, e->y2);
      if (r.type == INTERSECT_POINT)
      {
        double t = r.t0;
        if (t >= -FP_TOLERANCE && t <= 1.0 + FP_TOLERANCE)
          meos_array_add(events, &t);
      }
    }
    else
    {
      /* Compute the crossings with the arc boundary edge (at most two) */
      double t[2];
      int n = arcsegm_intersect(ax, ay, rx, ry, e, t);
      for (int k = 0; k < n; k++)
        if (t[k] >= -FP_TOLERANCE && t[k] <= 1.0 + FP_TOLERANCE)
          meos_array_add(events, &t[k]);
    }
  }

  /* Add endpoints */
  double t0 = 0.0, t1 = 1.0;
  meos_array_add(events, &t0);
  meos_array_add(events, &t1);

  /* Sort */
  qsort(events->elems, events->count, sizeof(double), float8_qsort_cmp);

  /* Deduplicate */
  int newcount = 0;
  double *evtarr = (double *) events->elems;
  for (int i = 0; i < (int) events->count; i++)
  {
    if (i == 0 ||
        fabs(evtarr[i] - evtarr[newcount - 1]) > FP_TOLERANCE)
    {
      evtarr[newcount++] = evtarr[i];
    }
  }
  events->count = newcount;

  /* Build intervals using midpoint test */
  for (int i = 0; i < (int) events->count - 1; i++)
  {
    double ta = evtarr[i];
    double tb = evtarr[i + 1];
    if (tb - ta <= FP_TOLERANCE)
      continue;

    /* Midpoint test */
    double tm = (ta + tb) * 0.5;
    double x = ax + tm * rx;
    double y = ay + tm * ry;
    if (point_in_polygon(x, y, all_edges, all_nedges))
    {
      Span in;
      span_set(Float8GetDatum(ta), Float8GetDatum(tb), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }
  return;
}

/*****************************************************************************/

/**
 * @brief Return true if a trajectory point intersects with an array of point
 * and linear edges
 */
static bool
point_inter_points_lines(const POINT2D *a, Edge **edges, int nedges)
{
  assert(a); assert(edges); assert(nedges >= 0);

  const double ax = a->x, ay = a->y;

  /* Iterate only through the point and linear edges */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype == EDGE_POINT)
    {
      if (fabs(e->x1 - ax) < FP_TOLERANCE && fabs(e->y1 - ay) < FP_TOLERANCE)
        return true;
    }
    else if (e->etype == EDGE_LINE)
    {
      if (point_on_segment(ax, ay, e->x1, e->y1, e->x2, e->y2))
        return true;
    }
    else if (e->etype == EDGE_ARC)
    {
      if (point_on_arc(ax, ay, e))
        return true;
    }
  }
  return false;
}

/*****************************************************************************
 * Clip a temporal geometry point
 *****************************************************************************/

/**
 * @brief Clip a 2D/3D trajectory with linear interpolation with respect to a
 * geometry
 * @param[in] inst Temporal sequence
 * @param[in] edges Array of geometry edges
 * @param[in] nedges Number of edges in the array
 * @param[in] rtree R-tree for the edges, may be `NULL` if no index is used
 * @param[in] cand_edges Edge array buffer of size `nedges` for storing the
 * result of an R-tree look up, may be `NULL` if no index is used
 */
static void
tpointinst_clip_edges(const TInstant *inst, Edge **edges, int nedges,
  const RTree *rtree, Edge **cand_edges)
{
  assert(inst); assert(edges); assert(nedges > 0);
  assert(inst->temptype == T_TGEOMPOINT);

  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst));

  /* Edges to process: all of them (default) or those filtered by an R-tree */
  Edge **sel_edges = edges;
  int sel_nedges = nedges;
  bool use_index = (rtree != NULL && cand_edges != NULL);
  if (use_index)
  {
    /* Build the segment bounding box */
    STBox query;
    int32_t srid = tspatial_srid((Temporal *) inst);
    stbox_set(true, false, false, srid, a->x, a->x, a->y, a->y, 0, 0, NULL,
      &query);
    /* Query the R-tree */
    int cand_nedges = rtree_search(rtree, RTREE_OVERLAPS, &query, rtree_results);

    /* Convert the result of an R-tree look up into an edge pointer array */
    for (int j = 0; j < cand_nedges; j++)
      cand_edges[j] = edges[*(int *) meos_array_get(rtree_results, j)];
    sel_edges = cand_edges;
    sel_nedges = cand_nedges;
  }

  /* Reset the interval array */
  intervals->count = 0;
  /* Compute the intervals for the points, lines, and polygon edges */
  bool found = point_inter_points_lines(a, sel_edges, sel_nedges);
  if (! found)
  {
    intervals_from_polygons(a, a, sel_edges, sel_nedges, edges, nedges);
    if (intervals->count == 0)
      return;
  }
  
  /* Generate the instantantaneous span */
  Span s;
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
  meos_array_add(periods, &s);
  return;
}

/**
 * @brief Clip a 2D/3D trajectory with linear interpolation with respect to a
 * geometry
 * @param[in] seq Temporal sequence
 * @param[in] edges Array of geometry edges
 * @param[in] nedges Number of edges in the array
 * @param[in] rtree R-tree for the edges, may be `NULL` if no index is used
 * @param[in] cand_edges Edge array buffer of size `nedges` for storing the
 * result of an R-tree look up, may be `NULL` if no index is used
 */
static void
tpointseq_clip_edges(const TSequence *seq, Edge **edges, int nedges,
  const RTree *rtree, Edge **cand_edges)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
    return tpointinst_clip_edges(TSEQUENCE_INST_N(seq, 0), edges, nedges,
      rtree, cand_edges);

  bool use_index = (rtree != NULL && cand_edges != NULL);
  int32_t srid = tspatial_srid((Temporal *) seq);

  /* Initialize variables for the loop */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst1));
  bool lower_inc = seq->period.lower_inc;
  /* Edges to process: either all of them or those filtered by an R-tree */
  Edge **sel_edges = edges;
  int sel_nedges = nedges;
  /* Loop for each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *b = DATUM_POINT2D_P(tinstant_value_p(inst2));
    bool upper_inc = (i < seq->count - 1) ? false : seq->period.upper_inc;

    /* Filter the edges to process by a R-tree, if any */
    if (use_index)
    {
      /* Build the segment bounding box */
      STBox query;
      stbox_set(true, false, false, srid, FP_MIN(a->x, b->x),
        FP_MAX(a->x, b->x), FP_MIN(a->y, b->y), FP_MAX(a->y, b->y),
        0, 0, NULL, &query);
      /* Query the R-tree */
      int cand_nedges = rtree_search(rtree, RTREE_OVERLAPS, &query, rtree_results);

      /* Convert the result of an R-tree look up into an edge pointer array */
      for (int j = 0; j < cand_nedges; j++)
        cand_edges[j] = edges[*(int *) meos_array_get(rtree_results, j)];
      sel_edges = cand_edges;
      sel_nedges = cand_nedges;
    }

    /* Reset the interval array */
    intervals->count = 0;
    /* Compute the intervals for the points, lines, and polygon edges */
    intervals_from_points(a, b, sel_edges, sel_nedges);
    intervals_from_lines(a, b, sel_edges, sel_nedges);
    intervals_from_arcs(a, b, sel_edges, sel_nedges);
    intervals_from_polygons(a, b, sel_edges, sel_nedges, edges, nedges);
    if (intervals->count == 0)
      goto next_segment;

    /* Normalize the intervals */
    int count;
    Span *intervarr = NULL;
    if (intervals->count > 1)
      intervarr = spanarr_normalize(intervals->elems, intervals->count,
        ORDER_NO, &count);
    else
    {
      intervarr = intervals->elems;
      count = 1;
    }

    /* Generate the periods from the float spans taking into account exclusive
     * temporal bounds */
    double duration = (double) (inst2->t - inst1->t);
    for (int j = 0; j < count; j++)
    {
      Span s;
      double lower = DatumGetFloat8(intervarr[j].lower);
      double upper = DatumGetFloat8(intervarr[j].upper);
      if (fabs(upper - lower) < FP_TOLERANCE)
      {
        /* Remove intersection points on exclusive lower and upper bounds */
        if (! lower_inc && fabs(lower) < FP_TOLERANCE &&
            fabs(upper) < FP_TOLERANCE)
          continue;
        if (! upper_inc && fabs(lower - 1.0) < FP_TOLERANCE &&
            fabs(upper - 1.0) < FP_TOLERANCE)
          continue;

        /* Interpolate only if 0 < lower/upper < 1 */
        TimestampTz t = (lower == 0.0) ?
          inst1->t : inst1->t + (TimestampTz) (duration * lower);
        span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
          T_TIMESTAMPTZ, T_TSTZSPAN, &s);
        meos_array_add(periods, &s);
      }
      else
      {
        TimestampTz t1 = (lower == 0.0) ?
          inst1->t : inst1->t + (TimestampTz) (duration * lower);
        TimestampTz t2 = (upper == 1.0) ?
          inst2->t : inst1->t + (TimestampTz) (duration * upper);
        span_set(TimestampTzGetDatum(t1), TimestampTzGetDatum(t2), true, true,
          T_TIMESTAMPTZ, T_TSTZSPAN, &s);
        meos_array_add(periods, &s);
      }
    }
    
next_segment:
    /* Prepare the next iteration */
    if (intervarr && intervals->count > 1)
      pfree(intervarr);
    inst1 = inst2;
    a = b;
  }
  return;
}

/*****************************************************************************
 * Extract edges from a geometry that can be of type point, line, polygon or
 * collection of these
 *****************************************************************************/

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a ring
 */
static void
emit_ring_edges(const POINTARRAY *pa, MeosArray *edges, EdgeType etype)
{
  for (int i = 0; i < (int) pa->npoints - 1; i++)
  {
    POINT4D a, b;
    (void) getPoint4d_p(pa, i, &a);
    (void) getPoint4d_p(pa, i + 1, &b);
    Edge e;
    e.x1 = a.x; e.y1 = a.y;
    e.x2 = b.x; e.y2 = b.y;
    e.xmin = FP_MIN(e.x1, e.x2); e.xmax = FP_MAX(e.x1, e.x2);
    e.ymin = FP_MIN(e.y1, e.y2); e.ymax = FP_MAX(e.y1, e.y2);
    e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
    e.length = e.dx * e.dx + e.dy * e.dy;
    e.etype = etype;
    meos_array_add(edges, &e);
  }
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edge obtained
 * from a point
 */
static void
extract_point(const LWPOINT *pt, MeosArray *edges)
{
  POINT4D p;
  (void) getPoint4d_p(pt->point, 0, &p);
  Edge e;
  e.x1 = e.x2 = e.xmin = e.xmax = p.x;
  e.y1 = e.y2 = e.ymin = e.ymax = p.y;
  e.dx = e.dy = e.length = 0;
  e.etype = EDGE_POINT;
  meos_array_add(edges, &e);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a multipoint
 */
static void
extract_mpoint(const LWMPOINT *mp, MeosArray *edges)
{
  for (int i = 0; i < (int) mp->ngeoms; i++)
    extract_point((const LWPOINT *) mp->geoms[i], edges);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the segments obtained
 * from a line
 */
static void
extract_line(const LWLINE *line, MeosArray *edges)
{
  emit_ring_edges(line->points, edges, EDGE_LINE);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the segments obtained
 * from a multiline
 */
static void
extract_mline(const LWMLINE *ml, MeosArray *edges)
{
  for (int i = 0; i < (int) ml->ngeoms; i++)
    extract_line(ml->geoms[i], edges);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a polygon
 */
static void
extract_poly(const LWPOLY *poly, MeosArray *edges)
{
  for (int r = 0; r < (int) poly->nrings; r++)
    emit_ring_edges(poly->rings[r], edges, EDGE_POLY);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a multipolygon
 */
static void
extract_mpoly(const LWMPOLY *mp, MeosArray *edges)
{
  for (int i = 0; i < (int) mp->ngeoms; i++)
    extract_poly(mp->geoms[i], edges);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a triangle
 * @details In PostGIS a triangle has a single (outer) ring stored as
 * POINTARRAY, which is already closed or implicitly closed
 */
static void
extract_triangle(const LWTRIANGLE *tri, MeosArray *edges)
{
  emit_ring_edges(tri->points, edges, EDGE_POLY);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edge obtained from
 * three consecutive points of a circular string
 * @details The three points are the start, an intermediate point, and the end
 * of the arc. Three collinear points degenerate to straight segments and are
 * emitted as line edges
 */
static void
emit_arc_edge(const POINT4D *pa, const POINT4D *pb, const POINT4D *pc,
  MeosArray *edges, EdgeType line_etype, EdgeType arc_etype)
{
  double ax = pa->x, ay = pa->y;
  double bx = pb->x, by = pb->y;
  double cx = pc->x, cy = pc->y;
  /* Twice the signed area of the triangle; zero when the points are collinear */
  double d = 2 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));

  /* Collinear points: emit straight line edges */
  if (fabs(d) < FP_TOLERANCE)
  {
    const double px[3] = {ax, bx, cx}, py[3] = {ay, by, cy};
    for (int i = 0; i < 2; i++)
    {
      Edge e;
      e.x1 = px[i]; e.y1 = py[i];
      e.x2 = px[i + 1]; e.y2 = py[i + 1];
      e.xmin = FP_MIN(e.x1, e.x2); e.xmax = FP_MAX(e.x1, e.x2);
      e.ymin = FP_MIN(e.y1, e.y2); e.ymax = FP_MAX(e.y1, e.y2);
      e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
      e.length = e.dx * e.dx + e.dy * e.dy;
      e.etype = line_etype;
      meos_array_add(edges, &e);
    }
    return;
  }

  double a2 = ax * ax + ay * ay;
  double b2 = bx * bx + by * by;
  double c2 = cx * cx + cy * cy;
  Edge e;
  /* Circumcenter of the three points */
  e.cx = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
  e.cy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
  e.radius = hypot(ax - e.cx, ay - e.cy);
  e.x1 = ax; e.y1 = ay;
  e.x2 = cx; e.y2 = cy;
  e.theta0 = atan2(ay - e.cy, ax - e.cx);
  e.theta1 = atan2(cy - e.cy, cx - e.cx);
  /* Traversal orientation from the signed area of (start, mid, end) */
  e.ccw = ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax)) > 0;
  e.dx = e.dy = e.length = 0;
  e.etype = arc_etype;
  arc_set_bbox(&e);
  meos_array_add(edges, &e);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the arc edges obtained
 * from a circular string, emitting them with the given line/arc edge types
 * @details Straight components (collinear point triples) are emitted with
 * @p line_etype and genuine arcs with @p arc_etype. A standalone circular
 * string uses the 1D types (#EDGE_LINE / #EDGE_ARC); a circular string that
 * bounds a curve polygon ring uses the region types (#EDGE_POLY /
 * #EDGE_POLYARC)
 */
static void
emit_circstring_edges(const LWCIRCSTRING *circ, MeosArray *edges,
  EdgeType line_etype, EdgeType arc_etype)
{
  const POINTARRAY *pa = circ->points;
  int np = (int) pa->npoints;
  for (int i = 0; i + 2 < np; i += 2)
  {
    POINT4D pa4, pb4, pc4;
    (void) getPoint4d_p(pa, i, &pa4);
    (void) getPoint4d_p(pa, i + 1, &pb4);
    (void) getPoint4d_p(pa, i + 2, &pc4);
    emit_arc_edge(&pa4, &pb4, &pc4, edges, line_etype, arc_etype);
  }
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the arc edges obtained
 * from a circular string
 */
static void
extract_circstring(const LWCIRCSTRING *circ, MeosArray *edges)
{
  emit_circstring_edges(circ, edges, EDGE_LINE, EDGE_ARC);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the region-boundary
 * edges obtained from a ring of a curve polygon
 * @details A ring is a line string, a circular string, or a compound curve
 * chaining both. Every edge is emitted with polygon (region) semantics
 * (#EDGE_POLY / #EDGE_POLYARC) so that the even-odd containment test in
 * #point_in_polygon treats it as a boundary rather than a 1D feature
 */
static void
extract_curvepoly_ring(const LWGEOM *ring, MeosArray *edges)
{
  switch (ring->type)
  {
    case LINETYPE:
      emit_ring_edges(((const LWLINE *) ring)->points, edges, EDGE_POLY);
      break;

    case CIRCSTRINGTYPE:
      emit_circstring_edges((const LWCIRCSTRING *) ring, edges, EDGE_POLY,
        EDGE_POLYARC);
      break;

    /* A compound curve is a chain of line strings and circular strings; it
     * shares the collection memory layout, so its components are processed as
     * ring pieces in the same way */
    case COMPOUNDTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) ring;
      for (int i = 0; i < (int) col->ngeoms; i++)
        extract_curvepoly_ring(col->geoms[i], edges);
      break;
    }

    /* Unsupported ring type */
    default:
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "Unsupported curve polygon ring type");
      break;
  }
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a curve polygon
 */
static void
extract_curvepoly(const LWCURVEPOLY *cp, MeosArray *edges)
{
  for (int r = 0; r < (int) cp->nrings; r++)
    extract_curvepoly_ring(cp->rings[r], edges);
  return;
}

/**
 * @brief Return the edges of a geometry in a dynamic array (iterator)
 */
static void
geom_extract_edges_iter(const LWGEOM *geom, MeosArray *edges)
{
  if (! geom)
    return;

  switch (geom->type)
  {
    case POINTTYPE:
      extract_point((const LWPOINT *) geom, edges);
      break;

    case MULTIPOINTTYPE:
      extract_mpoint((const LWMPOINT *) geom, edges);
      break;

    case LINETYPE:
      extract_line((const LWLINE *) geom, edges);
      break;

    case MULTILINETYPE:
      extract_mline((const LWMLINE *) geom, edges);
      break;

    case POLYGONTYPE:
      extract_poly((const LWPOLY *) geom, edges);
      break;

    case MULTIPOLYGONTYPE:
      extract_mpoly((const LWMPOLY *) geom, edges);
      break;

    case TRIANGLETYPE:
      extract_triangle((const LWTRIANGLE *) geom, edges);
      break;

    /* A compound curve (chain of line/circular strings), a multicurve
     * (collection of line/circular/compound curves) and a multisurface
     * (collection of polygons/curve polygons) all share the collection memory
     * layout, so their components are extracted the same way as a collection */
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (int i = 0; i < (int) col->ngeoms; i++)
        geom_extract_edges_iter(col->geoms[i], edges);
      break;
    }

    case CIRCSTRINGTYPE:
      extract_circstring((const LWCIRCSTRING *) geom, edges);
      break;

    case CURVEPOLYTYPE:
      extract_curvepoly((const LWCURVEPOLY *) geom, edges);
      break;

    /* Unsupported type */
    default:
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "Unsupported geometry type");
      break;
  }
  return;
}

/**
 * @brief Return the edges of a geometry in a dynamic array 
 */
static MeosArray *
geom_extract_edges(const LWGEOM *geom)
{
  MeosArray *edges = meos_array_create(sizeof(Edge));
  geom_extract_edges_iter(geom, edges);
  return edges;
}

/**
 * @brief Return true if a geometry is composed solely of the types the clip
 * engine can extract into edges
 * @details Mirrors the type dispatch of #geom_extract_edges_iter. Geometries
 * containing any other type (curved polygons, TIN, polyhedral surfaces, ...)
 * are not supported and must be handled by the caller
 */
bool
geom_clip_supported(const LWGEOM *geom)
{
  if (! geom)
    return false;
  switch (geom->type)
  {
    case POINTTYPE:
    case MULTIPOINTTYPE:
    case LINETYPE:
    case MULTILINETYPE:
    case POLYGONTYPE:
    case MULTIPOLYGONTYPE:
    case TRIANGLETYPE:
    case CIRCSTRINGTYPE:
      return true;
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case COLLECTIONTYPE:
    {
      /* A multicurve/multisurface is supported when every component is: its
       * components are line/circular/compound curves and polygons/curve
       * polygons, each validated by the recursive call */
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
        if (! geom_clip_supported(col->geoms[i]))
          return false;
      return true;
    }
    case CURVEPOLYTYPE:
    {
      /* Mirrors the ring dispatch of #extract_curvepoly_ring: a ring must be a
       * line string, a circular string, or a compound curve of those */
      const LWCURVEPOLY *cp = (const LWCURVEPOLY *) geom;
      for (uint32_t r = 0; r < cp->nrings; r++)
      {
        uint8_t rt = cp->rings[r]->type;
        if (rt != LINETYPE && rt != CIRCSTRINGTYPE && rt != COMPOUNDTYPE)
          return false;
        if (rt == COMPOUNDTYPE && ! geom_clip_supported(cp->rings[r]))
          return false;
      }
      return true;
    }
    default:
      return false;
  }
}

/**
 * @brief Build an R-tree from edges
 */
static RTree *
build_edge_rtree(const Edge *edges, int nedges, int32_t srid)
{
  RTree *rtree = rtree_create_stbox();
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = &edges[i];
    STBox box;
    stbox_set(true, false, false, srid, e->xmin, e->xmax, e->ymin, e->ymax,
      0, 0, NULL, &box);
    /* Store pointer to edge */
    rtree_insert(rtree, &box, i);
  }
  return rtree;
}

/*****************************************************************************/

/**
 * @brief Return the temporal intersection/intersects of a temporal  geometric
 * point with linear interpolation and a 2D geometry
 * @details The temporal geometric point may be in 2D or 3D and the Z dimension 
 * is also computed
 * @note For performance reasons we avoid the call to ST_Intersection
 * which delegates the computation to GEOS
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
Temporal *
tpoint_linear_inter_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool clip)
{
  assert(temp); assert(gs); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  assert(! gserialized_is_empty(gs)); 

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
  {
    if (clip)
      return NULL;
    SpanSet *ss = temporal_time(temp);
    Temporal *result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
    return result;
  }

  /* Initialize result to NULL to quickly clean up and return */
  Temporal *result = NULL;

  /* Extract the edges */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  MeosArray *edges = geom_extract_edges(geom);
  lwgeom_free(geom);
  /* Create an array of edge pointers */
  Edge **edge_ptrs = palloc(sizeof(Edge *) * edges->count);
  /* Transform the edge array into an edge pointer array */
  for (int i = 0; i < (int) edges->count; i++)
    edge_ptrs[i] = (Edge *) meos_array_get(edges, i);

  /* R-tree pointer: A NULL pointer passed to function #tpointseq_clip_edges
   * means that no index is used */
  RTree *rtree = NULL;
  /* Array of edge pointers for storing the edges filtered by the R-tree */
  Edge **cand_edges = NULL;
  /* Minimum number of edges to use an R-tree index in order to compensate the
   * overhead of the tree construction and destruction */
  if (edges->count > RTREE_MIN_NUMBER_ELEMS)
  {
    /* Build R-tree */
    int32_t srid = tspatial_srid(temp);
    rtree = build_edge_rtree(edges->elems, edges->count, srid);
    if (! rtree)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      return NULL;
    }
    /* Array of edge pointers for storing the edges filtered by the R-tree */
    cand_edges = palloc(sizeof(Edge *) * edges->count);
    if (! cand_edges)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      rtree_free(rtree); 
      return NULL;
    }
    /* Array for collecting the ids resulting from an R-tree search */
    rtree_results = meos_array_create(sizeof(int));
    if (! rtree_results)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      rtree_free(rtree); 
      return NULL;
    }
  }

  /* Initialize the static global arrays accumulating the clipping results */
  events = meos_array_create(sizeof(double));
  intervals = meos_array_create(sizeof(Span));
  periods = meos_array_create(sizeof(Span));
  
  /* Collect the clipping periods */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tpointinst_clip_edges((TInstant *) temp, edge_ptrs, edges->count,
        rtree, cand_edges);
      break;
    case TSEQUENCE:
      tpointseq_clip_edges((TSequence *) temp, edge_ptrs, edges->count,
        rtree, cand_edges);
      break;
    default: /* TSEQUENCESET */
    {
      /* Loop for each segment */
      TSequenceSet *ss = (TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        tpointseq_clip_edges(TSEQUENCESET_SEQ_N(ss, i), edge_ptrs,
          edges->count, rtree, cand_edges);
    }
  }

  SpanSet *ss;
  if (periods->count == 0)
  {
    if (clip)
      goto cleanup_return;
    ss = temporal_time(temp);
    result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
  }
  else
  {
    ss = spanset_make_exp(periods->elems, periods->count,
      periods->count, NORMALIZE, ORDER);
    if (clip)
      result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    else
    {
      SpanSet *ss1 = temporal_time(temp);
      Temporal *temp1 = (Temporal *) tsequenceset_from_base_tstzspanset(
        BoolGetDatum(false), T_TBOOL, ss1, STEP);
      Temporal *temp2 = temporal_restrict_tstzspanset(temp1, ss, REST_MINUS);
      if (temp2)
      {
        Temporal *temp3 = (Temporal *) tsequenceset_from_base_tstzspanset(
          BoolGetDatum(true), T_TBOOL, ss, STEP);
        result = temporal_merge(temp2, temp3);
        pfree(temp2); pfree(temp3);
      }
      else
        result = (Temporal *) tsequenceset_from_base_tstzspanset(
          BoolGetDatum(true), T_TBOOL, ss1, STEP);
      pfree(ss1); pfree(temp1);
    }
    pfree(ss);
  }
  
  /* Clean up and return */
cleanup_return:
  if (edges->count > RTREE_MIN_NUMBER_ELEMS)
  {
    rtree_free(rtree); pfree(cand_edges); meos_array_destroy(rtree_results);
  }
  meos_array_destroy(events);
  meos_array_destroy(intervals);
  meos_array_destroy(periods);
  meos_array_destroy(edges); pfree(edge_ptrs);
  return result;  
}

/*****************************************************************************
 * Within-distance (tDwithin / ever-always dwithin) native engine
 *
 * Distance-threshold sibling of the exact intersection engine above. The
 * within region of a geometry at distance @p dist is its Minkowski sum with a
 * closed disc of radius @p dist: a capsule around each segment, a disc around
 * each point, an annular sector around each arc, and the filled polygon
 * dilated by @p dist. For each moving-point segment the candidate boundary
 * crossing times are solved in closed form per edge (the roots of
 * dist(seg(t), edge) = dist), then each sub-interval is classified by the
 * exact interior-aware unit distance at its midpoint. This mirrors the
 * within-roots + midpoint-classification spanset assembler of the merged
 * temporal circular-buffer engine (tcbuffer_distance.c) specialized to a
 * moving point, i.e. a moving disc with radius r(t) = 0.
 *
 * A zero distance is exactly the temporal intersects relationship and is
 * delegated to #tpoint_linear_inter_geom so that tDwithin(., ., 0) is
 * bit-identical to tIntersects (including isolated contact instants, which are
 * measure-zero and therefore dropped by the positive-distance midpoint
 * classification, exactly as in the temporal circular-buffer engine).
 *****************************************************************************/

/**
 * @brief Return the squared distance from a point to a segment
 */
static double
point_seg_dist2(double px, double py, double x1, double y1, double x2,
  double y2)
{
  const double ux = x2 - x1, uy = y2 - y1;
  const double l2 = ux * ux + uy * uy;
  if (l2 < FP_TOLERANCE)
  {
    const double dx = px - x1, dy = py - y1;
    return dx * dx + dy * dy;
  }
  double s = ((px - x1) * ux + (py - y1) * uy) / l2;
  if (s < 0.0) s = 0.0; else if (s > 1.0) s = 1.0;
  const double qx = x1 + s * ux, qy = y1 + s * uy;
  const double dx = px - qx, dy = py - qy;
  return dx * dx + dy * dy;
}

/**
 * @brief Return the squared distance from a point to an arc edge
 * @details When the point projects within the arc's angular span the distance
 * is the difference to the supporting circle, otherwise it is the distance to
 * the nearer arc endpoint
 */
static double
point_arc_dist2(double px, double py, const Edge *e)
{
  const double dxc = px - e->cx, dyc = py - e->cy;
  const double dc = hypot(dxc, dyc);
  if (arc_contains_angle(e, atan2(dyc, dxc)))
  {
    const double dd = dc - e->radius;
    return dd * dd;
  }
  const double d0x = px - e->x1, d0y = py - e->y1;
  const double d1x = px - e->x2, d1y = py - e->y2;
  const double d0 = d0x * d0x + d0y * d0y;
  const double d1 = d1x * d1x + d1y * d1y;
  return FP_MIN(d0, d1);
}

/**
 * @brief Return the squared distance from a point to a single edge
 */
static double
point_edge_dist2(double px, double py, const Edge *e)
{
  switch (e->etype)
  {
    case EDGE_POINT:
    {
      const double dx = px - e->x1, dy = py - e->y1;
      return dx * dx + dy * dy;
    }
    case EDGE_LINE:
    case EDGE_POLY:
      return point_seg_dist2(px, py, e->x1, e->y1, e->x2, e->y2);
    default: /* EDGE_ARC / EDGE_POLYARC */
      return point_arc_dist2(px, py, e);
  }
}

/**
 * @brief Return true if a point is within @p dist of the geometry, taking the
 * polygon interior into account (a point inside a polygon is at distance 0)
 */
static bool
point_geom_within(double px, double py, Edge **edges, int nedges, double dist)
{
  const double d2 = dist * dist;
  for (int i = 0; i < nedges; i++)
    if (point_edge_dist2(px, py, edges[i]) <= d2 + FP_TOLERANCE)
      return true;
  return point_in_polygon(px, py, edges, nedges) ? true : false;
}

/**
 * @brief Append a candidate crossing time to the event array if it lies in
 * [0,1] (clamping tiny out-of-range values to the endpoints)
 */
static void
add_within_root(double t, MeosArray *ev)
{
  if (t > -FP_TOLERANCE && t < 1.0 + FP_TOLERANCE)
  {
    if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
    meos_array_add(ev, &t);
  }
}

/**
 * @brief Append the [0,1] roots of the quadratic @p A t^2 + @p B t + @p C to
 * the event array
 */
static void
add_within_quad_roots(double A, double B, double C, MeosArray *ev)
{
  if (fabs(A) < FP_TOLERANCE)
  {
    if (fabs(B) > FP_TOLERANCE)
      add_within_root(-C / B, ev);
    return;
  }
  const double disc = B * B - 4.0 * A * C;
  if (disc < 0.0)
    return;
  const double sq = sqrt(disc);
  add_within_root((-B - sq) / (2.0 * A), ev);
  add_within_root((-B + sq) / (2.0 * A), ev);
}

/**
 * @brief Append to @p ev the trajectory-segment times at which the moving
 * point crosses the distance-@p dist boundary of one edge
 * @details The boundary of the edge's within region is composed of: for a
 * point, the disc of radius @p dist; for a segment, the two endpoint caps and
 * the two parallel offset lines; for an arc, the inner/outer offset circles
 * and the two endpoint caps. The candidate set is a superset (offset lines are
 * infinite, offset circles ignore the angular span); spurious candidates are
 * filtered out by the exact midpoint distance classification
 */
static void
within_roots_from_edge(double ax, double ay, double rx, double ry,
  const Edge *e, double dist, MeosArray *ev)
{
  const double A = rx * rx + ry * ry;
  const double d2 = dist * dist;
  switch (e->etype)
  {
    case EDGE_POINT:
    {
      const double wx = ax - e->x1, wy = ay - e->y1;
      add_within_quad_roots(A, 2.0 * (wx * rx + wy * ry),
        wx * wx + wy * wy - d2, ev);
      return;
    }
    case EDGE_LINE:
    case EDGE_POLY:
    {
      /* Endpoint caps: discs of radius dist around each segment endpoint */
      const double w0x = ax - e->x1, w0y = ay - e->y1;
      add_within_quad_roots(A, 2.0 * (w0x * rx + w0y * ry),
        w0x * w0x + w0y * w0y - d2, ev);
      const double w1x = ax - e->x2, w1y = ay - e->y2;
      add_within_quad_roots(A, 2.0 * (w1x * rx + w1y * ry),
        w1x * w1x + w1y * w1y - d2, ev);
      /* Parallel offset lines at distance dist on both sides. The signed
       * perpendicular distance is (k0 + t k1) / sqrt(l2) */
      const double ux = e->x2 - e->x1, uy = e->y2 - e->y1;
      const double l2 = ux * ux + uy * uy;
      if (l2 > FP_TOLERANCE)
      {
        const double k0 = w0x * uy - w0y * ux;
        const double k1 = rx * uy - ry * ux;
        if (fabs(k1) > FP_TOLERANCE)
        {
          const double off = dist * sqrt(l2);
          add_within_root((off - k0) / k1, ev);
          add_within_root((-off - k0) / k1, ev);
        }
      }
      return;
    }
    default: /* EDGE_ARC / EDGE_POLYARC */
    {
      const double wx = ax - e->cx, wy = ay - e->cy;
      const double B = 2.0 * (wx * rx + wy * ry);
      const double C0 = wx * wx + wy * wy;
      const double ro = e->radius + dist;
      add_within_quad_roots(A, B, C0 - ro * ro, ev);
      const double ri = e->radius - dist;
      if (ri > 0.0)
        add_within_quad_roots(A, B, C0 - ri * ri, ev);
      /* Endpoint caps: discs of radius dist around each arc endpoint */
      const double e0x = ax - e->x1, e0y = ay - e->y1;
      add_within_quad_roots(A, 2.0 * (e0x * rx + e0y * ry),
        e0x * e0x + e0y * e0y - d2, ev);
      const double e1x = ax - e->x2, e1y = ay - e->y2;
      add_within_quad_roots(A, 2.0 * (e1x * rx + e1y * ry),
        e1x * e1x + e1y * e1y - d2, ev);
      return;
    }
  }
}

/**
 * @brief Collect into the interval array the [0,1] sub-intervals of one
 * trajectory segment along which the moving point is within @p dist of the
 * geometry
 * @param[in] a,b Endpoints of the trajectory segment
 * @param[in] sel_edges,sel_nedges Edges to gather crossing candidates from
 * (possibly R-tree filtered)
 * @param[in] all_edges,all_nedges Full edge array, used for the interior-aware
 * midpoint classification (the polygon ray-cast needs every edge)
 * @param[in] dist Distance threshold
 */
static void
intervals_within_edges(const POINT2D *a, const POINT2D *b, Edge **sel_edges,
  int sel_nedges, Edge **all_edges, int all_nedges, double dist)
{
  events->count = 0;
  const double ax = a->x, ay = a->y;
  const double rx = b->x - ax, ry = b->y - ay;
  const double seg_xmin = FP_MIN(a->x, b->x), seg_xmax = FP_MAX(a->x, b->x);
  const double seg_ymin = FP_MIN(a->y, b->y), seg_ymax = FP_MAX(a->y, b->y);

  /* Gather boundary crossing candidates from the (filtered) edges */
  for (int i = 0; i < sel_nedges; i++)
  {
    const Edge *e = sel_edges[i];
    /* Bounding-box filter expanded by dist: the moving point may be within
     * dist of an edge whose own box does not overlap the segment box */
    if (e->xmax + dist < seg_xmin || e->xmin - dist > seg_xmax ||
        e->ymax + dist < seg_ymin || e->ymin - dist > seg_ymax)
      continue;
    within_roots_from_edge(ax, ay, rx, ry, e, dist, events);
  }
  /* Add the segment endpoints */
  double t0 = 0.0, t1 = 1.0;
  meos_array_add(events, &t0);
  meos_array_add(events, &t1);

  /* Sort and deduplicate the candidates */
  qsort(events->elems, events->count, sizeof(double), float8_qsort_cmp);
  int newcount = 0;
  double *ev = (double *) events->elems;
  for (int i = 0; i < (int) events->count; i++)
    if (i == 0 || fabs(ev[i] - ev[newcount - 1]) > FP_TOLERANCE)
      ev[newcount++] = ev[i];
  events->count = newcount;

  /* Keep each sub-interval whose midpoint is within dist of the geometry */
  for (int i = 0; i < (int) events->count - 1; i++)
  {
    const double ta = ev[i], tb = ev[i + 1];
    if (tb - ta <= FP_TOLERANCE)
      continue;
    const double tm = 0.5 * (ta + tb);
    const double x = ax + tm * rx, y = ay + tm * ry;
    if (point_geom_within(x, y, all_edges, all_nedges, dist))
    {
      Span in;
      span_set(Float8GetDatum(ta), Float8GetDatum(tb), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }

  /* Isolated within instants: a trajectory that only grazes the distance
   * boundary tangentially touches the within region at a single time (a double
   * root, where the distance equals dist exactly) which the midpoint test
   * above cannot see. Emit a degenerate interval for each candidate time that
   * is within dist (inclusive). The span normalization absorbs the ones that
   * coincide with an interval endpoint, leaving only the genuine isolated
   * touches. This is what keeps the distance-inclusive semantics exact and, at
   * a zero distance, matches the isolated contact points of the intersection
   * engine (which the zero-distance path delegates to anyway). */
  for (int i = 0; i < (int) events->count; i++)
  {
    const double t = ev[i];
    const double x = ax + t * rx, y = ay + t * ry;
    if (point_geom_within(x, y, all_edges, all_nedges, dist))
    {
      Span in;
      span_set(Float8GetDatum(t), Float8GetDatum(t), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }
  return;
}

/**
 * @brief Add the within-distance instantaneous period of a temporal instant
 * point to the period array
 */
static void
tpointinst_dwithin_edges(const TInstant *inst, Edge **edges, int nedges,
  double dist)
{
  assert(inst); assert(edges); assert(nedges > 0);
  assert(inst->temptype == T_TGEOMPOINT);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst));
  if (! point_geom_within(a->x, a->y, edges, nedges, dist))
    return;
  Span s;
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
  meos_array_add(periods, &s);
  return;
}

/**
 * @brief Add to the period array the sub-periods of a temporal sequence point
 * with linear interpolation during which it is within @p dist of a geometry
 */
static void
tpointseq_dwithin_edges(const TSequence *seq, Edge **edges, int nedges,
  const RTree *rtree, Edge **cand_edges, double dist)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
    return tpointinst_dwithin_edges(TSEQUENCE_INST_N(seq, 0), edges, nedges,
      dist);

  bool use_index = (rtree != NULL && cand_edges != NULL);
  int32_t srid = tspatial_srid((Temporal *) seq);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst1));
  bool lower_inc = seq->period.lower_inc;
  Edge **sel_edges = edges;
  int sel_nedges = nedges;
  /* Loop for each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *b = DATUM_POINT2D_P(tinstant_value_p(inst2));
    bool upper_inc = (i < seq->count - 1) ? false : seq->period.upper_inc;

    /* Filter the edges by an R-tree, expanding the query box by dist */
    if (use_index)
    {
      STBox query;
      stbox_set(true, false, false, srid, FP_MIN(a->x, b->x) - dist,
        FP_MAX(a->x, b->x) + dist, FP_MIN(a->y, b->y) - dist,
        FP_MAX(a->y, b->y) + dist, 0, 0, NULL, &query);
      int cand_nedges = rtree_search(rtree, RTREE_OVERLAPS, &query,
        rtree_results);
      for (int j = 0; j < cand_nedges; j++)
        cand_edges[j] = edges[*(int *) meos_array_get(rtree_results, j)];
      sel_edges = cand_edges;
      sel_nedges = cand_nedges;
    }

    /* Reset and compute the within intervals for this segment */
    intervals->count = 0;
    intervals_within_edges(a, b, sel_edges, sel_nedges, edges, nedges, dist);
    if (intervals->count == 0)
      goto next_segment;

    /* Normalize the intervals (sort: the midpoint intervals and the isolated
     * within points are appended in two separate passes, not globally sorted) */
    int count;
    Span *intervarr = NULL;
    if (intervals->count > 1)
      intervarr = spanarr_normalize(intervals->elems, intervals->count,
        ORDER, &count);
    else
    {
      intervarr = intervals->elems;
      count = 1;
    }

    /* Generate the periods from the float spans taking into account exclusive
     * temporal bounds */
    double duration = (double) (inst2->t - inst1->t);
    for (int j = 0; j < count; j++)
    {
      Span s;
      double lower = DatumGetFloat8(intervarr[j].lower);
      double upper = DatumGetFloat8(intervarr[j].upper);
      if (fabs(upper - lower) < FP_TOLERANCE)
      {
        /* Remove within points on exclusive lower and upper bounds */
        if (! lower_inc && fabs(lower) < FP_TOLERANCE &&
            fabs(upper) < FP_TOLERANCE)
          continue;
        if (! upper_inc && fabs(lower - 1.0) < FP_TOLERANCE &&
            fabs(upper - 1.0) < FP_TOLERANCE)
          continue;
        TimestampTz t = (lower == 0.0) ?
          inst1->t : inst1->t + (TimestampTz) (duration * lower);
        span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
          T_TIMESTAMPTZ, T_TSTZSPAN, &s);
        meos_array_add(periods, &s);
      }
      else
      {
        TimestampTz t1 = (lower == 0.0) ?
          inst1->t : inst1->t + (TimestampTz) (duration * lower);
        TimestampTz t2 = (upper == 1.0) ?
          inst2->t : inst1->t + (TimestampTz) (duration * upper);
        span_set(TimestampTzGetDatum(t1), TimestampTzGetDatum(t2), true, true,
          T_TIMESTAMPTZ, T_TSTZSPAN, &s);
        meos_array_add(periods, &s);
      }
    }

next_segment:
    if (intervarr && intervals->count > 1)
      pfree(intervarr);
    inst1 = inst2;
    a = b;
  }
  return;
}

/**
 * @ingroup meos_internal_geo
 * @brief Return a temporal Boolean that states whether a temporal geometric
 * point with linear interpolation is within a distance of a 2D geometry
 * @details Native GEOS-free counterpart of the polygonal-buffer approximation:
 * for a zero distance it is exactly #tpoint_linear_inter_geom (tIntersects),
 * otherwise it solves the per-segment within-distance sub-intervals in closed
 * form. The result is a temporal Boolean defined over the whole time of the
 * temporal point
 * @pre The arguments have the same SRID, are 2D and planar, and the geometry
 * is not empty and is supported by the clip engine. This is verified by the
 * caller
 */
Temporal *
tpoint_linear_dwithin_geom(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  assert(temp); assert(gs); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  assert(! gserialized_is_empty(gs));

  /* A zero distance is exactly the temporal intersects relationship */
  if (dist <= 0.0)
    return tpoint_linear_inter_geom(temp, gs, false);

  /* Bounding box test: the geometry box expanded by dist must overlap the
   * temporal point box, otherwise the relationship is false throughout */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  geo_set_stbox(gs, &box2);
  STBox *box2e = stbox_expand_space(&box2, dist);
  bool overlap = overlaps_stbox_stbox(&box1, box2e);
  pfree(box2e);
  if (! overlap)
  {
    SpanSet *ss = temporal_time(temp);
    Temporal *result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
    return result;
  }

  /* Extract the edges */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  MeosArray *edges = geom_extract_edges(geom);
  lwgeom_free(geom);
  Edge **edge_ptrs = palloc(sizeof(Edge *) * edges->count);
  for (int i = 0; i < (int) edges->count; i++)
    edge_ptrs[i] = (Edge *) meos_array_get(edges, i);

  /* Optional R-tree index for many-edge geometries */
  RTree *rtree = NULL;
  Edge **cand_edges = NULL;
  if (edges->count > RTREE_MIN_NUMBER_ELEMS)
  {
    int32_t srid = tspatial_srid(temp);
    rtree = build_edge_rtree(edges->elems, edges->count, srid);
    if (! rtree)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      return NULL;
    }
    cand_edges = palloc(sizeof(Edge *) * edges->count);
    rtree_results = meos_array_create(sizeof(int));
  }

  /* Initialize the static global arrays accumulating the results */
  events = meos_array_create(sizeof(double));
  intervals = meos_array_create(sizeof(Span));
  periods = meos_array_create(sizeof(Span));

  /* Collect the within-distance periods */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TSEQUENCE:
      tpointseq_dwithin_edges((TSequence *) temp, edge_ptrs, edges->count,
        rtree, cand_edges, dist);
      break;
    default: /* TSEQUENCESET */
    {
      TSequenceSet *ss = (TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        tpointseq_dwithin_edges(TSEQUENCESET_SEQ_N(ss, i), edge_ptrs,
          edges->count, rtree, cand_edges, dist);
    }
  }

  /* Assemble the temporal Boolean over the whole time of the temporal point */
  Temporal *result;
  if (periods->count == 0)
  {
    SpanSet *ss = temporal_time(temp);
    result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
  }
  else
  {
    SpanSet *ss = spanset_make_exp(periods->elems, periods->count,
      periods->count, NORMALIZE, ORDER);
    SpanSet *ss1 = temporal_time(temp);
    Temporal *temp1 = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss1, STEP);
    Temporal *temp2 = temporal_restrict_tstzspanset(temp1, ss, REST_MINUS);
    if (temp2)
    {
      Temporal *temp3 = (Temporal *) tsequenceset_from_base_tstzspanset(
        BoolGetDatum(true), T_TBOOL, ss, STEP);
      result = temporal_merge(temp2, temp3);
      pfree(temp2); pfree(temp3);
    }
    else
      result = (Temporal *) tsequenceset_from_base_tstzspanset(
        BoolGetDatum(true), T_TBOOL, ss1, STEP);
    pfree(ss1); pfree(temp1); pfree(ss);
  }

  /* Clean up and return */
  if (edges->count > RTREE_MIN_NUMBER_ELEMS)
  {
    rtree_free(rtree); pfree(cand_edges); meos_array_destroy(rtree_results);
  }
  meos_array_destroy(events);
  meos_array_destroy(intervals);
  meos_array_destroy(periods);
  meos_array_destroy(edges); pfree(edge_ptrs);
  return result;
}

/*****************************************************************************
 * Temporal distance (tDistance) native engine
 *
 * Distance-value sibling of the within-distance engine above. It produces the
 * temporal float distance from a moving point to a whole (possibly curved) 2D
 * geometry, lifting the point-operand-only restriction of the generic lifting
 * path (whose per-segment turning-point function can only represent the
 * distance to a single static point, i.e. at most one interior extremum).
 *
 * For each trajectory segment the distance to the geometry is the pointwise
 * minimum, over all edges, of the exact point-to-edge distance. Its turning
 * points are the union of the per-edge critical times: the perpendicular-foot
 * and endpoint-closest-approach times of a straight edge, the radial extremum
 * and angular-sector crossing times of an arc edge, and the region-boundary
 * times where the nearest feature of an edge changes. At every such time the
 * exact distance to the whole geometry is emitted as a temporal float instant,
 * with linear interpolation in between, exactly as the point-to-point temporal
 * distance samples its analytic turning points. The global minimum of the
 * distance over a segment is min over edges of the per-edge minimum over the
 * segment (the two minimisations commute), so emitting every per-edge extremum
 * makes minValue exact.
 *****************************************************************************/

/**
 * @brief Return the exact distance from a point to the whole geometry, taking
 * the polygon interior into account (a point inside a filled polygon is at
 * distance zero)
 */
static double
point_geom_dist(double px, double py, Edge **edges, int nedges)
{
  double best = point_edge_dist2(px, py, edges[0]);
  for (int i = 1; i < nedges; i++)
  {
    const double d2 = point_edge_dist2(px, py, edges[i]);
    if (d2 < best)
      best = d2;
  }
  /* On (or numerically on) the boundary: distance is zero */
  if (best <= FP_TOLERANCE)
    return 0.0;
  /* Strictly inside a filled polygon: distance is zero. The horizontal-ray
   * even-odd test of #point_in_polygon miscounts when the query height aligns
   * exactly with a vertex or an arc junction, which the turning-point sampler
   * can hit deterministically. Take the majority vote of the test at the point
   * and at two tiny vertical nudges that move the ray off any aligned junction;
   * the nudge is far below any real feature size so a strictly interior or
   * strictly exterior point is unaffected */
  const double eps = 1e-9 * FP_MAX(1.0, fabs(py));
  int inside = point_in_polygon(px, py, edges, nedges) +
    point_in_polygon(px, py + eps, edges, nedges) +
    point_in_polygon(px, py - eps, edges, nedges);
  return (inside >= 2) ? 0.0 : sqrt(best);
}

/**
 * @brief Append to the event array the [0,1] trajectory-segment critical times
 * of the distance from the moving point to one edge
 * @details The candidates are the local extrema and nearest-feature switch
 * times of the exact point-to-edge distance: for a point the single closest
 * approach; for a straight edge the perpendicular-foot time, the two
 * endpoint-closest-approach times, and the two foot-parameter region
 * boundaries; for an arc edge the radial extremum, the supporting-circle
 * crossings (where the distance reaches its zero minimum), the two
 * endpoint-closest approaches, and the two angular-sector boundary crossings.
 * Spurious candidates are harmless because the distance value emitted at each
 * time is the exact distance to the whole geometry
 */
static void
distance_cands_from_edge(double ax, double ay, double rx, double ry,
  const Edge *e, MeosArray *ev)
{
  const double A = rx * rx + ry * ry;
  /* Constant (zero-length) trajectory segment: no interior turning point */
  if (A < FP_TOLERANCE)
    return;
  switch (e->etype)
  {
    case EDGE_POINT:
    {
      const double wx = ax - e->x1, wy = ay - e->y1;
      add_within_root(-(wx * rx + wy * ry) / A, ev);
      return;
    }
    case EDGE_LINE:
    case EDGE_POLY:
    {
      const double w0x = ax - e->x1, w0y = ay - e->y1;
      const double w1x = ax - e->x2, w1y = ay - e->y2;
      /* Closest approach to each segment endpoint */
      add_within_root(-(w0x * rx + w0y * ry) / A, ev);
      add_within_root(-(w1x * rx + w1y * ry) / A, ev);
      const double ux = e->x2 - e->x1, uy = e->y2 - e->y1;
      const double l2 = ux * ux + uy * uy;
      if (l2 > FP_TOLERANCE)
      {
        /* Perpendicular-foot time (moving point on the supporting line) */
        const double k1 = rx * uy - ry * ux;
        if (fabs(k1) > FP_TOLERANCE)
          add_within_root(-(w0x * uy - w0y * ux) / k1, ev);
        /* Foot-parameter region boundaries (s = 0 and s = 1) */
        const double ru = rx * ux + ry * uy;
        if (fabs(ru) > FP_TOLERANCE)
        {
          const double w0u = w0x * ux + w0y * uy;
          add_within_root(-w0u / ru, ev);
          add_within_root((l2 - w0u) / ru, ev);
        }
      }
      return;
    }
    default: /* EDGE_ARC / EDGE_POLYARC */
    {
      const double wx = ax - e->cx, wy = ay - e->cy;
      /* Radial extremum: the time at which || P(t) - center || is stationary
       * (the distance-to-arc minimum when the segment stays on one side of the
       * supporting circle) */
      add_within_root(-(wx * rx + wy * ry) / A, ev);
      /* Supporting-circle crossings, where the distance to the arc reaches its
       * zero minimum (a kink not seen by the radial extremum): the roots of
       * || P(t) - center ||^2 = radius^2 */
      add_within_quad_roots(A, 2.0 * (wx * rx + wy * ry),
        wx * wx + wy * wy - e->radius * e->radius, ev);
      /* Closest approach to each arc endpoint */
      const double w0x = ax - e->x1, w0y = ay - e->y1;
      const double w1x = ax - e->x2, w1y = ay - e->y2;
      add_within_root(-(w0x * rx + w0y * ry) / A, ev);
      add_within_root(-(w1x * rx + w1y * ry) / A, ev);
      /* Angular-sector boundary crossings (rays from the center through the
       * arc endpoints) */
      const double d0x = e->x1 - e->cx, d0y = e->y1 - e->cy;
      const double den0 = rx * d0y - ry * d0x;
      if (fabs(den0) > FP_TOLERANCE)
        add_within_root(-(wx * d0y - wy * d0x) / den0, ev);
      const double d1x = e->x2 - e->cx, d1y = e->y2 - e->cy;
      const double den1 = rx * d1y - ry * d1x;
      if (fabs(den1) > FP_TOLERANCE)
        add_within_root(-(wx * d1y - wy * d1x) / den1, ev);
      return;
    }
  }
}

/**
 * @brief Return the temporal float distance of one temporal sequence point
 * with linear interpolation to a geometry given as an edge array
 */
static TSequence *
tpointseq_distance_geom(const TSequence *seq, Edge **edges, int nedges)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    const POINT2D *p = DATUM_POINT2D_P(tinstant_value_p(inst));
    double d = point_geom_dist(p->x, p->y, edges, nedges);
    TInstant *resinst = tinstant_make(Float8GetDatum(d), T_TFLOAT, inst->t);
    TSequence *res = tsequence_make(&resinst, 1, true, true, LINEAR, NORMALIZE);
    pfree(resinst);
    return res;
  }

  /* Upper bound on the number of result instants: the two endpoints of every
   * segment plus up to six interior turning points per edge and per segment */
  int maxinsts = 1 + (seq->count - 1) * (nedges * 6 + 3);
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  int ninsts = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst1));
  instants[ninsts++] = tinstant_make(
    Float8GetDatum(point_geom_dist(a->x, a->y, edges, nedges)), T_TFLOAT,
    inst1->t);
  /* Loop for each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *b = DATUM_POINT2D_P(tinstant_value_p(inst2));
    const double ax = a->x, ay = a->y, rx = b->x - ax, ry = b->y - ay;

    /* Gather the interior turning points of the distance to every edge */
    events->count = 0;
    for (int j = 0; j < nedges; j++)
      distance_cands_from_edge(ax, ay, rx, ry, edges[j], events);

    /* Sort the candidate parameters and emit an instant for each interior one
     * with the exact distance to the whole geometry */
    qsort(events->elems, events->count, sizeof(double), float8_qsort_cmp);
    const double *ev = (double *) events->elems;
    const double duration = (double) (inst2->t - inst1->t);
    TimestampTz prevt = inst1->t;
    for (int k = 0; k < (int) events->count; k++)
    {
      const double p = ev[k];
      if (p <= FP_TOLERANCE || p >= 1.0 - FP_TOLERANCE)
        continue;
      if (k > 0 && fabs(p - ev[k - 1]) < FP_TOLERANCE)
        continue;
      TimestampTz t = inst1->t + (TimestampTz) (duration * p);
      /* Keep the instants strictly increasing and off the segment endpoints */
      if (t <= prevt || t >= inst2->t)
        continue;
      const double x = ax + p * rx, y = ay + p * ry;
      instants[ninsts++] = tinstant_make(
        Float8GetDatum(point_geom_dist(x, y, edges, nedges)), T_TFLOAT, t);
      prevt = t;
    }
    /* End instant of the segment */
    instants[ninsts++] = tinstant_make(
      Float8GetDatum(point_geom_dist(b->x, b->y, edges, nedges)), T_TFLOAT,
      inst2->t);
    inst1 = inst2;
    a = b;
  }

  return tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @ingroup meos_internal_geo
 * @brief Return the temporal float distance between a temporal geometric point
 * with linear interpolation and a 2D geometry
 * @details Native GEOS-free counterpart of the generic distance lifting for a
 * non-point geometry operand: the distance to a multi-edge or curved target
 * has an arbitrary number of turning points per segment which the point-only
 * base turning-point function cannot represent. The result is a temporal float
 * with linear interpolation whose values at the analytic turning points and at
 * the trajectory instants are the exact distance to the geometry
 * @pre The arguments have the same SRID, are 2D and planar, and the geometry
 * is not empty and is supported by the clip engine. This is verified by the
 * caller
 */
Temporal *
tpoint_linear_distance_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  assert(temp); assert(gs); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  assert(! gserialized_is_empty(gs));

  /* Extract the edges */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  MeosArray *edges = geom_extract_edges(geom);
  lwgeom_free(geom);
  Edge **edge_ptrs = palloc(sizeof(Edge *) * edges->count);
  for (int i = 0; i < (int) edges->count; i++)
    edge_ptrs[i] = (Edge *) meos_array_get(edges, i);

  /* Static array accumulating the per-segment candidate turning times */
  events = meos_array_create(sizeof(double));

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_distance_geom((TSequence *) temp,
      edge_ptrs, edges->count);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
    for (int i = 0; i < ss->count; i++)
      sequences[i] = tpointseq_distance_geom(TSEQUENCESET_SEQ_N(ss, i),
        edge_ptrs, edges->count);
    result = (Temporal *) tsequenceset_make_free(sequences, ss->count,
      NORMALIZE);
  }

  /* Clean up and return */
  meos_array_destroy(events);
  meos_array_destroy(edges); pfree(edge_ptrs);
  return result;
}

/**
 * @brief Return a temporal geometric point with linear interpolation
 * restricted to a 2D geometry
 * @details The temporal point may be 2D or 3D and the Z dimension is also
 * computed
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
Temporal *
tpoint_linear_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(temp); assert(gs); assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));

  /* Compute atGeometry for the temporal point */
  Temporal *result_at = tpoint_linear_inter_geom(temp, gs, true);

  /* If "at" restriction, return */
  if (atfunc)
    return result_at;

  /* If "minus" restriction, compute the complement wrt time */
  if (! result_at)
    return temporal_copy(temp);

  SpanSet *ss = temporal_time(result_at);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, atfunc);
  pfree(ss); pfree(result_at);
  return result;
}

/*****************************************************************************/