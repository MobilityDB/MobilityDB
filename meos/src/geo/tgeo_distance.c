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
 * @brief Temporal distance functions for temporal geos
 */

#include "geo/tgeo_distance.h"

/* C */
#include <assert.h>
#include <postgres.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <lwgeodetic_tree.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/lifting.h"
#include "temporal/tinstant.h"
#include "temporal/temporal_restrict.h"
#include "temporal/tsequence.h"
#include "temporal/type_util.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/stbox.h"
#include "temporal/temporal_rtree.h"

/* Functions not exported by PostGIS */
extern double circ_tree_distance_tree_internal(const CIRC_NODE* n1,
  const CIRC_NODE* n2, double threshold, double* min_dist, double* max_dist,
  GEOGRAPHIC_POINT* closest1, GEOGRAPHIC_POINT* closest2);

/*****************************************************************************
 * GEOS-free analytic distance engine (shared with the tcbuffer family)
 *
 * A moving disc (centre c1->c2, radius r1->r2; r may be 0 for a moving
 * point) is tested against a geometry decomposed into boundary edges. The
 * nearest-approach kernels (nad/shortestLine/nai) walk a Morton bucket
 * hierarchy with branch-and-bound; the interior test and relationship
 * kernels use the generic R-tree. Radii are parameters, so temporal points
 * (r=0) and temporal circular buffers share one engine.
 *****************************************************************************/

/**
 * @brief Minimise f(t) = sqrt(A t^2 + B t + C) - (R0 + DR t) over [lo,hi]
 * @details Candidates are the interval endpoints and the real roots of the
 * stationarity equation squared, (A^2 - A DR^2) t^2 + (A B - B DR^2) t +
 * (B^2/4 - C DR^2) = 0. Spurious roots introduced by squaring only add
 * larger values, so the minimum over the candidates is exact.
 */
static double
geodist_minfun(double A, double B, double C, double R0, double DR, double lo,
  double hi)
{
  double cand[6];
  int nc = 0;
  cand[nc++] = lo;
  cand[nc++] = hi;
  double a2 = A * (A - DR * DR);
  double a1 = B * (A - DR * DR);
  double a0 = 0.25 * B * B - C * DR * DR;
  if (fabs(a2) > 1e-18)
  {
    double disc = a1 * a1 - 4.0 * a2 * a0;
    if (disc >= 0.0)
    {
      double sd = sqrt(disc);
      cand[nc++] = (-a1 + sd) / (2.0 * a2);
      cand[nc++] = (-a1 - sd) / (2.0 * a2);
    }
  }
  else if (fabs(a1) > 1e-18)
    cand[nc++] = -a0 / a1;
  /* Vertex of the quadratic under the root. In the perpendicular regime that
   * quadratic is a perfect square, so the stationarity discriminant above is a
   * rounding-noise negative and its double root -B/(2A) is dropped; that root
   * is the instant the moving centre crosses the edge's supporting line (an
   * exact zero distance, i.e. an overlap), so add it explicitly to never miss
   * it. */
  if (fabs(A) > 1e-18)
    cand[nc++] = -B / (2.0 * A);
  double best = DBL_MAX;
  for (int i = 0; i < nc; i++)
  {
    double t = cand[i];
    if (t < lo) t = lo;
    if (t > hi) t = hi;
    double q = A * t * t + B * t + C;
    if (q < 0.0) q = 0.0;
    double f = sqrt(q) - (R0 + DR * t);
    if (f < best) best = f;
  }
  return best;
}

/**
 * @brief Append the segments of a point array to the segment array, growing
 * it as needed. A single-point array contributes one degenerate segment.
 */
static void
geodist_geom_edges_add_ptarray(const POINTARRAY *pa, bool is_poly, GeoDistEdge **arr,
  int *cap, int *cnt)
{
  if (! pa || pa->npoints == 0)
    return;
  uint32_t np = pa->npoints;
  uint32_t nseg = (np == 1) ? 1 : np - 1;
  for (uint32_t i = 0; i < nseg; i++)
  {
    const POINT2D *a = getPoint2d_cp(pa, i);
    const POINT2D *b = (np == 1) ? a : getPoint2d_cp(pa, i + 1);
    if (*cnt == *cap)
    {
      int newcap = (*cap == 0) ? 64 : *cap * 2;
      *arr = (*arr == NULL) ? palloc(sizeof(GeoDistEdge) * newcap) :
        repalloc(*arr, sizeof(GeoDistEdge) * newcap);
      *cap = newcap;
    }
    GeoDistEdge *s = &(*arr)[(*cnt)++];
    s->x1 = a->x; s->y1 = a->y; s->x2 = b->x; s->y2 = b->y;
    s->xmin = fmin(a->x, b->x); s->xmax = fmax(a->x, b->x);
    s->ymin = fmin(a->y, b->y); s->ymax = fmax(a->y, b->y);
    s->is_poly = is_poly;
    s->is_arc = false;
  }
}

/**
 * @brief Normalise an angle to [0, 2*pi)
 */
static double
geodist_angle_norm(double a)
{
  double r = fmod(a, 2.0 * M_PI);
  if (r < 0.0)
    r += 2.0 * M_PI;
  return r;
}

/**
 * @brief True if the angle @p phi lies within the arc's angular span
 */
static bool
geodist_geom_arc_contains_angle(const GeoDistEdge *e, double phi)
{
  double sweep = e->accw ?
    geodist_angle_norm(e->at1 - e->at0) : geodist_angle_norm(e->at0 - e->at1);
  double off = e->accw ?
    geodist_angle_norm(phi - e->at0) : geodist_angle_norm(e->at0 - phi);
  return off <= sweep + 1e-12;
}

/**
 * @brief Set an arc edge's bounding box: the chord endpoints extended with any
 * cardinal-direction circle extreme (0, pi/2, pi, -pi/2) that lies within the
 * arc's angular span, so the bucket hierarchy never prunes away the arc bulge
 */
static void
geodist_geom_arc_set_bbox(GeoDistEdge *e)
{
  double xmin = fmin(e->x1, e->x2), xmax = fmax(e->x1, e->x2);
  double ymin = fmin(e->y1, e->y2), ymax = fmax(e->y1, e->y2);
  const double ang[4] = {0.0, M_PI_2, M_PI, -M_PI_2};
  const double ex[4] = {e->acx + e->arad, e->acx, e->acx - e->arad, e->acx};
  const double ey[4] = {e->acy, e->acy + e->arad, e->acy, e->acy - e->arad};
  for (int k = 0; k < 4; k++)
    if (geodist_geom_arc_contains_angle(e, ang[k]))
    {
      if (ex[k] < xmin) xmin = ex[k];
      if (ex[k] > xmax) xmax = ex[k];
      if (ey[k] < ymin) ymin = ey[k];
      if (ey[k] > ymax) ymax = ey[k];
    }
  e->xmin = xmin; e->xmax = xmax; e->ymin = ymin; e->ymax = ymax;
}

/**
 * @brief Append one arc edge, defined by three consecutive points of a
 * circular string (start, any interior point, end), to the segment array.
 * Collinear triples degenerate to two straight segments. Mirrors the exact
 * circumcentre construction of the native clip engine (@ref tpoint_geom_clip.c).
 */
static void
geodist_segs_add_arc(double ax, double ay, double bx, double by, double cx,
  double cy, bool is_poly, GeoDistEdge **arr, int *cap, int *cnt)
{
  if (*cnt + 2 > *cap)
  {
    int newcap = (*cap == 0) ? 64 : *cap * 2;
    while (*cnt + 2 > newcap)
      newcap *= 2;
    *arr = (*arr == NULL) ? palloc(sizeof(GeoDistEdge) * newcap) :
      repalloc(*arr, sizeof(GeoDistEdge) * newcap);
    *cap = newcap;
  }
  /* Twice the signed area of triangle ABC; zero => collinear */
  double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));
  if (fabs(d) < 1e-12)
  {
    /* Collinear: two straight segments A->B, B->C */
    for (int seg = 0; seg < 2; seg++)
    {
      double p1x = seg == 0 ? ax : bx, p1y = seg == 0 ? ay : by;
      double p2x = seg == 0 ? bx : cx, p2y = seg == 0 ? by : cy;
      GeoDistEdge *s = &(*arr)[(*cnt)++];
      s->x1 = p1x; s->y1 = p1y; s->x2 = p2x; s->y2 = p2y;
      s->xmin = fmin(p1x, p2x); s->xmax = fmax(p1x, p2x);
      s->ymin = fmin(p1y, p2y); s->ymax = fmax(p1y, p2y);
      s->is_poly = is_poly; s->is_arc = false;
    }
    return;
  }
  double a2 = ax * ax + ay * ay, b2 = bx * bx + by * by, c2 = cx * cx + cy * cy;
  GeoDistEdge *s = &(*arr)[(*cnt)++];
  s->acx = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
  s->acy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
  s->arad = hypot(ax - s->acx, ay - s->acy);
  s->x1 = ax; s->y1 = ay; s->x2 = cx; s->y2 = cy;
  s->at0 = atan2(ay - s->acy, ax - s->acx);
  s->at1 = atan2(cy - s->acy, cx - s->acx);
  s->accw = ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax)) > 0.0;
  s->is_poly = is_poly; s->is_arc = true;
  geodist_geom_arc_set_bbox(s);
}

/**
 * @brief Append the arc edges of a circular string (walked in three-point
 * groups) to the segment array
 */
static void
geodist_segs_add_circstring(const LWCIRCSTRING *circ, bool is_poly,
  GeoDistEdge **arr, int *cap, int *cnt)
{
  const POINTARRAY *pa = circ->points;
  int np = (int) pa->npoints;
  for (int i = 0; i + 2 < np; i += 2)
  {
    const POINT2D *a = getPoint2d_cp(pa, i);
    const POINT2D *b = getPoint2d_cp(pa, i + 1);
    const POINT2D *c = getPoint2d_cp(pa, i + 2);
    geodist_segs_add_arc(a->x, a->y, b->x, b->y, c->x, c->y, is_poly, arr, cap,
      cnt);
  }
}

/**
 * @brief Append the boundary segments of a curve polygon ring (a line string,
 * a circular string, or a compound curve chaining both) with polygon (region)
 * semantics. Returns false when an arc ring is present but the caller does not
 * consume arc edges (@p allow_arc is false), so the exact path is used.
 */
static bool
geodist_segs_add_curvepoly_ring(const LWGEOM *ring, bool allow_arc,
  GeoDistEdge **arr, int *cap, int *cnt)
{
  switch (ring->type)
  {
    case LINETYPE:
      geodist_geom_edges_add_ptarray(lwgeom_as_lwline(ring)->points, true, arr, cap,
        cnt);
      return true;
    case CIRCSTRINGTYPE:
      if (! allow_arc)
        return false;
      geodist_segs_add_circstring(lwgeom_as_lwcircstring(ring), true, arr, cap,
        cnt);
      return true;
    case COMPOUNDTYPE:
    {
      const LWCOLLECTION *c = lwgeom_as_lwcollection(ring);
      for (uint32_t i = 0; i < c->ngeoms; i++)
        if (! geodist_segs_add_curvepoly_ring(c->geoms[i], allow_arc, arr, cap,
              cnt))
          return false;
      return true;
    }
    default:
      return false;
  }
}

/**
 * @brief Recursively collect the boundary segments of a geometry. Returns
 * false for a curved type that cannot be decomposed exactly (the caller
 * then falls back to the exact traversed-area path).
 */
bool
geodist_geom_edges(const LWGEOM *lw, bool allow_arc, GeoDistEdge **arr, int *cap,
  int *cnt, bool *has_poly)
{
  switch (lw->type)
  {
    case POINTTYPE:
      geodist_geom_edges_add_ptarray(lwgeom_as_lwpoint(lw)->point, false, arr, cap,
        cnt);
      return true;
    case CIRCSTRINGTYPE:
      /* Arc-exact decomposition, only where the caller consumes arc edges (the
       * nearest-approach distance). Otherwise fall back to the exact path. */
      if (! allow_arc)
        return false;
      geodist_segs_add_circstring(lwgeom_as_lwcircstring(lw), false, arr, cap,
        cnt);
      return true;
    case LINETYPE:
      geodist_geom_edges_add_ptarray(lwgeom_as_lwline(lw)->points, false, arr, cap,
        cnt);
      return true;
    case TRIANGLETYPE:
      geodist_geom_edges_add_ptarray(lwgeom_as_lwtriangle(lw)->points, true, arr,
        cap, cnt);
      *has_poly = true;
      return true;
    case POLYGONTYPE:
    {
      const LWPOLY *p = lwgeom_as_lwpoly(lw);
      for (uint32_t i = 0; i < p->nrings; i++)
        geodist_geom_edges_add_ptarray(p->rings[i], true, arr, cap, cnt);
      if (p->nrings > 0)
        *has_poly = true;
      return true;
    }
    case CURVEPOLYTYPE:
    {
      /* A curve polygon is bounded by rings that are line strings, circular
       * strings, or compound curves. Decompose each ring with polygon (region)
       * semantics so the arc-aware even-odd test in #geodist_geom_point_inside
       * treats it as a boundary. */
      const LWCURVEPOLY *cp = lwgeom_as_lwcurvepoly(lw);
      for (uint32_t i = 0; i < cp->nrings; i++)
        if (! geodist_segs_add_curvepoly_ring(cp->rings[i], allow_arc, arr,
              cap, cnt))
          return false;
      if (cp->nrings > 0)
        *has_poly = true;
      return true;
    }
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    /* A compound curve chains line strings and circular strings, a multi curve
     * groups line/circular/compound components, and a multi surface groups
     * curve polygons; all share the collection memory layout, so their
     * components are decomposed recursively. Circular-arc components resolve
     * through the CIRCSTRINGTYPE / CURVEPOLYTYPE cases, which gate on
     * @p allow_arc. */
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *c = lwgeom_as_lwcollection(lw);
      for (uint32_t i = 0; i < c->ngeoms; i++)
        if (! geodist_geom_edges(c->geoms[i], allow_arc, arr, cap, cnt, has_poly))
          return false;
      return true;
    }
    default:
      /* Curved or unsupported type: let the caller use the exact path */
      return false;
  }
}

/**
 * @brief Apply the rightward-ray crossings of one polygon-boundary segment to
 * the even-odd accumulator @p inside. A straight edge contributes at most one
 * crossing; a circular arc contributes the crossings of the horizontal line
 * y with its supporting circle that fall within the arc's angular span. Point,
 * line, and standalone (1D) arc edges (not @p is_poly) contribute nothing.
 */
static void
geodist_poly_seg_raycross(const GeoDistEdge *s, double x, double y, bool *inside)
{
  if (! s->is_poly)
    return;
  if (s->is_arc)
  {
    /* The horizontal line at height y meets the supporting circle at
     * acx +/- sqrt(arad^2 - (y - acy)^2). Flip the parity for each crossing
     * strictly to the right of x that lies within the arc's angular span; a
     * ray that only grazes the circle tangentially does not cross. */
    const double dyc = y - s->acy;
    const double h2 = s->arad * s->arad - dyc * dyc;
    if (h2 <= 1e-12)
      return;
    const double h = sqrt(h2);
    const double xhit[2] = {s->acx - h, s->acx + h};
    /* Forward traversal direction of the arc in the angle parameter */
    const double sdir = s->accw ? 1.0 : -1.0;
    for (int k = 0; k < 2; k++)
    {
      const double xi = xhit[k];
      if (xi <= x)
        continue;
      if (! geodist_geom_arc_contains_angle(s, atan2(dyc, xi - s->acx)))
        continue;
      /* Half-open ownership, mirroring the straight-edge rule: a crossing at an
       * arc endpoint (a ring junction on the ray) is owned by this edge only if
       * the arc interior rises above the ray there; an interior crossing is
       * always transversal and always counted. */
      const bool at_ep0 = fabs(xi - s->x1) < 1e-12 && fabs(y - s->y1) < 1e-12;
      const bool at_ep1 = fabs(xi - s->x2) < 1e-12 && fabs(y - s->y2) < 1e-12;
      if (at_ep0 || at_ep1)
      {
        const double theta_e = at_ep0 ? s->at0 : s->at1;
        const double dtheta_in = at_ep0 ? sdir : -sdir;
        if (dtheta_in * cos(theta_e) <= 0)
          continue;
      }
      *inside = ! *inside;
    }
    return;
  }
  double y1 = s->y1, y2 = s->y2;
  if ((y1 > y) != (y2 > y))
  {
    double xint = s->x1 + (y - y1) / (y2 - y1) * (s->x2 - s->x1);
    if (x < xint)
      *inside = ! *inside;
  }
}

/**
 * @brief Minimum of [ dist(c(t), edge) - r(t) ] for t in [0,1], where the
 * centre moves from (cx1,cy1) to (cx2,cy2) and the radius from r1 to r2
 */
double
geodist_segm_edge_mindist(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const GeoDistEdge *e)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1;
  const double dr = r2 - r1;
  const double ax = e->x1, ay = e->y1, bx = e->x2, by = e->y2;
  const double ux = bx - ax, uy = by - ay;
  const double l2 = ux * ux + uy * uy;

  /* Degenerate edge (a point): distance to that point over the whole t */
  if (l2 <= 1e-24)
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
    double C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    return geodist_minfun(A, B, C, r1, dr, 0.0, 1.0);
  }

  /* Projection parameter s(t) = (s0 + s1 t) / l2, split [0,1] at s=0, s=1 */
  const double s0 = (cx1 - ax) * ux + (cy1 - ay) * uy;
  const double s1 = dcx * ux + dcy * uy;
  double bp[4];
  int nb = 0;
  bp[nb++] = 0.0;
  bp[nb++] = 1.0;
  if (fabs(s1) > 1e-18)
  {
    double ta = -s0 / s1, tb = (l2 - s0) / s1;
    if (ta > 0.0 && ta < 1.0) bp[nb++] = ta;
    if (tb > 0.0 && tb < 1.0) bp[nb++] = tb;
  }
  /* Sort the (at most 4) breakpoints */
  for (int i = 0; i < nb; i++)
    for (int j = i + 1; j < nb; j++)
      if (bp[j] < bp[i]) { double tmp = bp[i]; bp[i] = bp[j]; bp[j] = tmp; }

  double best = DBL_MAX;
  for (int k = 0; k + 1 < nb; k++)
  {
    double lo = bp[k], hi = bp[k + 1];
    if (hi - lo < 1e-15) continue;
    double mt = 0.5 * (lo + hi);
    double s = (s0 + s1 * mt) / l2;
    double A, B, C;
    if (s <= 0.0)
    {
      /* Closest to the edge start A */
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
      C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    }
    else if (s >= 1.0)
    {
      /* Closest to the edge end B */
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - bx) * dcx + (cy1 - by) * dcy);
      C = (cx1 - bx) * (cx1 - bx) + (cy1 - by) * (cy1 - by);
    }
    else
    {
      /* Perpendicular distance: cross(c(t)-A, u) / |u| */
      double k0 = (cx1 - ax) * uy - (cy1 - ay) * ux;
      double k1 = dcx * uy - dcy * ux;
      A = k1 * k1 / l2;
      B = 2.0 * k0 * k1 / l2;
      C = k0 * k0 / l2;
    }
    double m = geodist_minfun(A, B, C, r1, dr, lo, hi);
    if (m < best) best = m;
  }
  return best;
}

/**
 * @brief Minimum of [ dist(c(t), arc) - r(t) ] for t in [0,1], where the centre
 * moves from (cx1,cy1) to (cx2,cy2) and the radius from r1 to r2, and @p e is a
 * circular-arc edge.
 * @details Let Q(t) = |c(t) - centre|^2 = A t^2 + B t + C. Where the foot angle
 * phi(t) lies within the arc's angular span the distance to the arc is
 * | sqrt(Q(t)) - R |, so the distance to the moving disc is
 * | sqrt(Q(t)) - R | - r(t); its minimisers over t are the interval endpoints,
 * the circle crossings sqrt(Q)=R (kinks of the absolute value), and the
 * stationary points, whose squared equation (Q')^2 = 4 dr^2 Q is independent of
 * the sign branch and of R (identical to #geodist_minfun). Where phi(t) is
 * outside the span the nearest arc point is an endpoint, covered by the two
 * endpoint point-distance minimisations. Taking the minimum over the angle-gated
 * on-span candidates and the two endpoint candidates is exact.
 */
double
geodist_segm_arc_mindist(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const GeoDistEdge *e)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1;
  const double dr = r2 - r1;
  const double px = e->acx, py = e->acy, R = e->arad;
  const double A = dcx * dcx + dcy * dcy;
  const double B = 2.0 * ((cx1 - px) * dcx + (cy1 - py) * dcy);
  const double C = (cx1 - px) * (cx1 - px) + (cy1 - py) * (cy1 - py);

  double cand[8];
  int nc = 0;
  cand[nc++] = 0.0;
  cand[nc++] = 1.0;
  /* Circle crossings Q(t) = R^2 */
  {
    double c0 = C - R * R;
    if (fabs(A) > 1e-18)
    {
      double disc = B * B - 4.0 * A * c0;
      if (disc >= 0.0)
      {
        double sd = sqrt(disc);
        cand[nc++] = (-B + sd) / (2.0 * A);
        cand[nc++] = (-B - sd) / (2.0 * A);
      }
    }
    else if (fabs(B) > 1e-18)
      cand[nc++] = -c0 / B;
  }
  /* Vertex of Q (closest approach to the centre) */
  if (fabs(A) > 1e-18)
    cand[nc++] = -B / (2.0 * A);
  /* Stationary points of | sqrt(Q) - R | - r(t): (Q')^2 = 4 dr^2 Q */
  {
    double a2 = A * (A - dr * dr);
    double a1 = B * (A - dr * dr);
    double a0 = 0.25 * B * B - C * dr * dr;
    if (fabs(a2) > 1e-18)
    {
      double disc = a1 * a1 - 4.0 * a2 * a0;
      if (disc >= 0.0)
      {
        double sd = sqrt(disc);
        cand[nc++] = (-a1 + sd) / (2.0 * a2);
        cand[nc++] = (-a1 - sd) / (2.0 * a2);
      }
    }
    else if (fabs(a1) > 1e-18)
      cand[nc++] = -a0 / a1;
  }

  double best = DBL_MAX;
  /* On-span candidates: exact distance to the arc's circle, angle-gated */
  for (int i = 0; i < nc; i++)
  {
    double t = cand[i];
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    double q = A * t * t + B * t + C;
    if (q < 0.0) q = 0.0;
    double cpx = cx1 + dcx * t, cpy = cy1 + dcy * t;
    if (! geodist_geom_arc_contains_angle(e, atan2(cpy - py, cpx - px)))
      continue;
    double f = fabs(sqrt(q) - R) - (r1 + dr * t);
    if (f < best) best = f;
  }
  /* Off-span regions are nearest to an arc endpoint: two point-distance
   * minimisations over the whole [0,1] (never under-estimate the true arc
   * distance, and equal it where an endpoint is the nearest arc point) */
  for (int ep = 0; ep < 2; ep++)
  {
    double ex = ep == 0 ? e->x1 : e->x2;
    double ey = ep == 0 ? e->y1 : e->y2;
    double Ae = A;
    double Be = 2.0 * ((cx1 - ex) * dcx + (cy1 - ey) * dcy);
    double Ce = (cx1 - ex) * (cx1 - ex) + (cy1 - ey) * (cy1 - ey);
    double m = geodist_minfun(Ae, Be, Ce, r1, dr, 0.0, 1.0);
    if (m < best) best = m;
  }
  return best;
}

/**
 * @brief Like #geodist_minfun but also returns the argument that attains the
 * minimum (clamped into [lo,hi])
 */
static double
geodist_minfun_w(double A, double B, double C, double R0, double DR, double lo,
  double hi, double *argt)
{
  double cand[6];
  int nc = 0;
  cand[nc++] = lo;
  cand[nc++] = hi;
  double a2 = A * (A - DR * DR);
  double a1 = B * (A - DR * DR);
  double a0 = 0.25 * B * B - C * DR * DR;
  if (fabs(a2) > 1e-18)
  {
    double disc = a1 * a1 - 4.0 * a2 * a0;
    if (disc >= 0.0)
    {
      double sd = sqrt(disc);
      cand[nc++] = (-a1 + sd) / (2.0 * a2);
      cand[nc++] = (-a1 - sd) / (2.0 * a2);
    }
  }
  else if (fabs(a1) > 1e-18)
    cand[nc++] = -a0 / a1;
  /* Vertex of the quadratic under the root; in the perpendicular regime it is a
   * perfect square whose stationarity double root -B/(2A) rounds out above, so
   * add it explicitly (the instant the centre crosses the edge line). */
  if (fabs(A) > 1e-18)
    cand[nc++] = -B / (2.0 * A);
  double best = DBL_MAX, bt = lo;
  for (int i = 0; i < nc; i++)
  {
    double t = cand[i];
    if (t < lo) t = lo;
    if (t > hi) t = hi;
    double q = A * t * t + B * t + C;
    if (q < 0.0) q = 0.0;
    double f = sqrt(q) - (R0 + DR * t);
    if (f < best) { best = f; bt = t; }
  }
  *argt = bt;
  return best;
}

/**
 * @brief Minimum of [ dist(c(t), edge) - r(t) ] for t in [0,1] together
 * with the argument t that attains it (mirrors #geodist_segm_edge_mindist)
 */
static double
geodist_segm_edge_dt(double cx1, double cy1, double cx2, double cy2, double r1,
  double r2, const GeoDistEdge *e, double *out_t)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1;
  const double dr = r2 - r1;
  const double ax = e->x1, ay = e->y1, bx = e->x2, by = e->y2;
  const double ux = bx - ax, uy = by - ay;
  const double l2 = ux * ux + uy * uy;
  if (l2 <= 1e-24)
  {
    double A = dcx * dcx + dcy * dcy;
    double B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
    double C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    return geodist_minfun_w(A, B, C, r1, dr, 0.0, 1.0, out_t);
  }
  const double s0 = (cx1 - ax) * ux + (cy1 - ay) * uy;
  const double s1 = dcx * ux + dcy * uy;
  double bp[4];
  int nb = 0;
  bp[nb++] = 0.0;
  bp[nb++] = 1.0;
  if (fabs(s1) > 1e-18)
  {
    double ta = -s0 / s1, tb = (l2 - s0) / s1;
    if (ta > 0.0 && ta < 1.0) bp[nb++] = ta;
    if (tb > 0.0 && tb < 1.0) bp[nb++] = tb;
  }
  for (int i = 0; i < nb; i++)
    for (int j = i + 1; j < nb; j++)
      if (bp[j] < bp[i]) { double tmp = bp[i]; bp[i] = bp[j]; bp[j] = tmp; }
  double best = DBL_MAX, bt = 0.0;
  for (int k = 0; k + 1 < nb; k++)
  {
    double lo = bp[k], hi = bp[k + 1];
    if (hi - lo < 1e-15) continue;
    double mt = 0.5 * (lo + hi);
    double s = (s0 + s1 * mt) / l2;
    double A, B, C;
    if (s <= 0.0)
    {
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - ax) * dcx + (cy1 - ay) * dcy);
      C = (cx1 - ax) * (cx1 - ax) + (cy1 - ay) * (cy1 - ay);
    }
    else if (s >= 1.0)
    {
      A = dcx * dcx + dcy * dcy;
      B = 2.0 * ((cx1 - bx) * dcx + (cy1 - by) * dcy);
      C = (cx1 - bx) * (cx1 - bx) + (cy1 - by) * (cy1 - by);
    }
    else
    {
      double k0 = (cx1 - ax) * uy - (cy1 - ay) * ux;
      double k1 = dcx * uy - dcy * ux;
      A = k1 * k1 / l2;
      B = 2.0 * k0 * k1 / l2;
      C = k0 * k0 / l2;
    }
    double rt;
    double m = geodist_minfun_w(A, B, C, r1, dr, lo, hi, &rt);
    if (m < best) { best = m; bt = rt; }
  }
  *out_t = bt;
  return best;
}

/**
 * @brief Like #geodist_segm_arc_mindist but also returns the argument t that
 * attains the minimum (mirrors #geodist_segm_edge_dt for arc edges)
 */
static double
geodist_segm_arc_dt(double cx1, double cy1, double cx2, double cy2, double r1,
  double r2, const GeoDistEdge *e, double *out_t)
{
  const double dcx = cx2 - cx1, dcy = cy2 - cy1;
  const double dr = r2 - r1;
  const double px = e->acx, py = e->acy, R = e->arad;
  const double A = dcx * dcx + dcy * dcy;
  const double B = 2.0 * ((cx1 - px) * dcx + (cy1 - py) * dcy);
  const double C = (cx1 - px) * (cx1 - px) + (cy1 - py) * (cy1 - py);

  double cand[8];
  int nc = 0;
  cand[nc++] = 0.0;
  cand[nc++] = 1.0;
  /* Circle crossings Q(t) = R^2 */
  {
    double c0 = C - R * R;
    if (fabs(A) > 1e-18)
    {
      double disc = B * B - 4.0 * A * c0;
      if (disc >= 0.0)
      {
        double sd = sqrt(disc);
        cand[nc++] = (-B + sd) / (2.0 * A);
        cand[nc++] = (-B - sd) / (2.0 * A);
      }
    }
    else if (fabs(B) > 1e-18)
      cand[nc++] = -c0 / B;
  }
  /* Vertex of Q (closest approach to the centre) */
  if (fabs(A) > 1e-18)
    cand[nc++] = -B / (2.0 * A);
  /* Stationary points of | sqrt(Q) - R | - r(t): (Q')^2 = 4 dr^2 Q */
  {
    double a2 = A * (A - dr * dr);
    double a1 = B * (A - dr * dr);
    double a0 = 0.25 * B * B - C * dr * dr;
    if (fabs(a2) > 1e-18)
    {
      double disc = a1 * a1 - 4.0 * a2 * a0;
      if (disc >= 0.0)
      {
        double sd = sqrt(disc);
        cand[nc++] = (-a1 + sd) / (2.0 * a2);
        cand[nc++] = (-a1 - sd) / (2.0 * a2);
      }
    }
    else if (fabs(a1) > 1e-18)
      cand[nc++] = -a0 / a1;
  }

  double best = DBL_MAX, bt = 0.0;
  /* On-span candidates: exact distance to the arc's circle, angle-gated */
  for (int i = 0; i < nc; i++)
  {
    double t = cand[i];
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    double q = A * t * t + B * t + C;
    if (q < 0.0) q = 0.0;
    double cpx = cx1 + dcx * t, cpy = cy1 + dcy * t;
    if (! geodist_geom_arc_contains_angle(e, atan2(cpy - py, cpx - px)))
      continue;
    double f = fabs(sqrt(q) - R) - (r1 + dr * t);
    if (f < best) { best = f; bt = t; }
  }
  /* Off-span regions are nearest to an arc endpoint */
  for (int ep = 0; ep < 2; ep++)
  {
    double ex = ep == 0 ? e->x1 : e->x2;
    double ey = ep == 0 ? e->y1 : e->y2;
    double Be = 2.0 * ((cx1 - ex) * dcx + (cy1 - ey) * dcy);
    double Ce = (cx1 - ex) * (cx1 - ex) + (cy1 - ey) * (cy1 - ey);
    double rt;
    double m = geodist_minfun_w(A, Be, Ce, r1, dr, 0.0, 1.0, &rt);
    if (m < best) { best = m; bt = rt; }
  }
  *out_t = bt;
  return best;
}

/**
 * @brief Closest point on edge @p e to (px,py)
 */
static void
geodist_geom_closest_on_edge(double px, double py, const GeoDistEdge *e, double *qx,
  double *qy)
{
  double ux = e->x2 - e->x1, uy = e->y2 - e->y1;
  double l2 = ux * ux + uy * uy;
  if (l2 <= 1e-24) { *qx = e->x1; *qy = e->y1; return; }
  double s = ((px - e->x1) * ux + (py - e->y1) * uy) / l2;
  if (s < 0.0) s = 0.0;
  if (s > 1.0) s = 1.0;
  *qx = e->x1 + s * ux;
  *qy = e->y1 + s * uy;
}

/**
 * @brief Closest point on arc edge @p e to (px,py): the projection onto the
 * supporting circle when its angle lies in the arc span, otherwise the nearer
 * arc endpoint
 */
static void
geodist_geom_closest_on_arc(double px, double py, const GeoDistEdge *e, double *qx,
  double *qy)
{
  double vx = px - e->acx, vy = py - e->acy;
  double vl = hypot(vx, vy);
  if (vl > 1e-12 && geodist_geom_arc_contains_angle(e, atan2(vy, vx)))
  {
    *qx = e->acx + vx * (e->arad / vl);
    *qy = e->acy + vy * (e->arad / vl);
    return;
  }
  double d1 = (px - e->x1) * (px - e->x1) + (py - e->y1) * (py - e->y1);
  double d2 = (px - e->x2) * (px - e->x2) + (py - e->y2) * (py - e->y2);
  if (d1 <= d2) { *qx = e->x1; *qy = e->y1; }
  else { *qx = e->x2; *qy = e->y2; }
}

/** @brief A segment's Morton (Z-order) key paired with its index, for sorting.
 * Sorting these lightweight handles rather than the ~128-byte edge payloads
 * keeps qsort from swapping whole edges. */
typedef struct
{
  uint32_t key;
  int idx;
} GeoDistSortItem;

/** @brief Spread the low 16 bits of @p v with one zero bit between each */
static uint32_t
geodist_geom_morton_part(uint32_t v)
{
  v &= 0x0000ffff;
  v = (v | (v << 8)) & 0x00ff00ff;
  v = (v | (v << 4)) & 0x0f0f0f0f;
  v = (v | (v << 2)) & 0x33333333;
  v = (v | (v << 1)) & 0x55555555;
  return v;
}

/** @brief Order GeoDistSortItem by Morton key */
static int
geodist_geom_morton_cmp(const void *a, const void *b)
{
  uint32_t ka = ((const GeoDistSortItem *) a)->key;
  uint32_t kb = ((const GeoDistSortItem *) b)->key;
  return (ka > kb) - (ka < kb);
}

/**
 * @brief Reorder the segments along a Morton (Z-order) curve and group them
 * into ~sqrt(n) spatially-local buckets, each with its bounding box. The
 * buckets let a swept-capsule unit skip whole groups of edges that are farther
 * than the running minimum, turning the per-unit edge scan from O(edges) into
 * roughly O(sqrt(edges) + matches) — the geometry's overall bounding box is too
 * coarse for large coastal polygons, but the bucket boxes are tight.
 */
static GeoDistBucket *
geodist_geom_build_buckets(GeoDistEdge *segs, int n, double gxmin, double gymin,
  double gxmax, double gymax, int *nbk_out)
{
  double sx = (gxmax > gxmin) ? 65535.0 / (gxmax - gxmin) : 0.0;
  double sy = (gymax > gymin) ? 65535.0 / (gymax - gymin) : 0.0;
  GeoDistSortItem *items = palloc(sizeof(GeoDistSortItem) * n);
  for (int i = 0; i < n; i++)
  {
    double cx = 0.5 * (segs[i].xmin + segs[i].xmax);
    double cy = 0.5 * (segs[i].ymin + segs[i].ymax);
    uint32_t ix = (uint32_t) ((cx - gxmin) * sx);
    uint32_t iy = (uint32_t) ((cy - gymin) * sy);
    items[i].key = geodist_geom_morton_part(ix) | (geodist_geom_morton_part(iy) << 1);
    items[i].idx = i;
  }
  qsort(items, n, sizeof(GeoDistSortItem), geodist_geom_morton_cmp);
  /* Apply the resulting permutation to the segments through one scratch pass.
   * Sorting the lightweight key/index handles above (rather than the segments
   * themselves) keeps qsort from copying the ~128-byte edge payloads on every
   * swap, which dominated the bucket build for large polygons. */
  GeoDistEdge *sorted = palloc(sizeof(GeoDistEdge) * n);
  for (int i = 0; i < n; i++)
    sorted[i] = segs[items[i].idx];
  for (int i = 0; i < n; i++)
    segs[i] = sorted[i];
  pfree(sorted);
  pfree(items);

  int bsize = (int) ceil(sqrt((double) n));
  if (bsize < 1) bsize = 1;
  int nbk = (n + bsize - 1) / bsize;
  GeoDistBucket *bks = palloc(sizeof(GeoDistBucket) * nbk);
  for (int b = 0; b < nbk; b++)
  {
    int s = b * bsize, e = s + bsize;
    if (e > n) e = n;
    double xmn = DBL_MAX, ymn = DBL_MAX, xmx = -DBL_MAX, ymx = -DBL_MAX;
    for (int k = s; k < e; k++)
    {
      if (segs[k].xmin < xmn) xmn = segs[k].xmin;
      if (segs[k].ymin < ymn) ymn = segs[k].ymin;
      if (segs[k].xmax > xmx) xmx = segs[k].xmax;
      if (segs[k].ymax > ymx) ymx = segs[k].ymax;
    }
    bks[b].start = s; bks[b].n = e - s;
    bks[b].xmin = xmn; bks[b].ymin = ymn; bks[b].xmax = xmx; bks[b].ymax = ymx;
  }
  *nbk_out = nbk;
  return bks;
}

/**
 * @brief Per-operation scratch buffer for the R-tree candidate ids in the
 * relationship kernels. It is created together with the R-tree in
 * #tcbuffer_geo_ctx_make and destroyed together with it in
 * #tcbuffer_geo_ctx_free, so it lives exactly as long as the geometry context
 * (the same create-with-rtree / destroy-with-rtree lifetime as rtree_results in
 * tpoint_geom_clip.c). MEOS_TLS keeps concurrent threads from sharing it.
 */
MEOS_TLS MeosArray *geodist_pip_results = NULL;

/**
 * @brief Ray-casting interior test. Over the R-tree (relationship path) the
 * candidates are the edges overlapping the rightward ray box [x, xmax] x [y, y];
 * over the bucket hierarchy (nad path) a bucket holds a crossing edge only when y
 * is inside its y-range and its xmax reaches x, so distant buckets are skipped
 * wholesale. Both yield the same crossing edges, and the even-odd parity is
 * order-independent, so the result matches.
 */
bool
geodist_geom_point_inside(double x, double y, const GeoDistGeom *g)
{
  bool inside = false;
  if (g->rtree)
  {
    STBox query;
    stbox_set(true, false, false, 0, x, g->xmax, y, y, 0, 0, NULL, &query);
    int nc = rtree_search(g->rtree, RTREE_OVERLAPS, &query, geodist_pip_results);
    for (int j = 0; j < nc; j++)
      geodist_poly_seg_raycross(
        &g->segs[*(int *) meos_array_get(geodist_pip_results, j)], x, y, &inside);
    return inside;
  }
  for (int b = 0; b < g->nbk; b++)
  {
    const GeoDistBucket *bk = &g->bks[b];
    if (y < bk->ymin || y > bk->ymax || bk->xmax < x)
      continue;
    int e = bk->start + bk->n;
    for (int i = bk->start; i < e; i++)
    {
      geodist_poly_seg_raycross(&g->segs[i], x, y, &inside);
    }
  }
  return inside;
}

/**
 * @brief Squared distance from a point to a 2D segment
 */
static inline double
geodist_pt_seg_dist2(double px, double py, double ax, double ay, double bx,
  double by)
{
  double abx = bx - ax, aby = by - ay;
  double l2 = abx * abx + aby * aby;
  double t = (l2 > 0.0) ? ((px - ax) * abx + (py - ay) * aby) / l2 : 0.0;
  t = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
  double ex = px - (ax + t * abx), ey = py - (ay + t * aby);
  return ex * ex + ey * ey;
}

/**
 * @brief Squared minimum distance between two 2D segments (0 when they cross),
 * a scalar-only alternative to lw_dist2d_seg_seg for the hot nearest-approach
 * prune (no closest-point bookkeeping)
 */
static inline double
geodist_seg_seg_dist2(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  /* Conservative crossing test (orientation signs, no division): a touch or
   * collinear overlap is treated as a crossing, so the returned squared
   * distance is never above the true one -- a valid lower bound for the prune,
   * robust near parallel where a t/u division would be ill-conditioned. */
  double d1 = (bx - ax) * (cy - ay) - (by - ay) * (cx - ax);
  double d2 = (bx - ax) * (dy - ay) - (by - ay) * (dx - ax);
  double d3 = (dx - cx) * (ay - cy) - (dy - cy) * (ax - cx);
  double d4 = (dx - cx) * (by - cy) - (dy - cy) * (bx - cx);
  if (((d1 <= 0.0 && d2 >= 0.0) || (d1 >= 0.0 && d2 <= 0.0)) &&
      ((d3 <= 0.0 && d4 >= 0.0) || (d3 >= 0.0 && d4 <= 0.0)))
    return 0.0;
  double m = geodist_pt_seg_dist2(ax, ay, cx, cy, dx, dy);
  double v;
  v = geodist_pt_seg_dist2(bx, by, cx, cy, dx, dy); if (v < m) m = v;
  v = geodist_pt_seg_dist2(cx, cy, ax, ay, bx, by); if (v < m) m = v;
  v = geodist_pt_seg_dist2(dx, dy, ax, ay, bx, by); if (v < m) m = v;
  return m;
}

/**
 * @brief Update the running minimum with one swept-capsule unit (the centre
 * moves from c1 to c2 with radius r1 to r2; a stationary disk has c1 == c2)
 */
void
geodist_segm_nad(double cx1, double cy1, double r1, double cx2, double cy2,
  double r2, const GeoDistGeom *g, double *best)
{
  double sxmin = Min(cx1 - r1, cx2 - r2);
  double sxmax = Max(cx1 + r1, cx2 + r2);
  double symin = Min(cy1 - r1, cy2 - r2);
  double symax = Max(cy1 + r1, cy2 + r2);
  /* Coarse branch-and-bound prune: the distance between this unit's swept
   * bounding box and the geometry's overall bounding box is a lower bound on
   * the unit's nearest approach (a box contains its geometry, so the box-to-box
   * distance cannot exceed the geometry-to-capsule distance). If that lower
   * bound already reaches the running minimum, no edge can improve it, so the
   * point-in-polygon test and the whole per-edge loop are skipped. A segment
   * whose centre is inside a polygon overlaps the geometry bounding box, giving
   * a zero lower bound, so it is never wrongly pruned. */
  if (*best != DBL_MAX)
  {
    double dgx = Max(Max(g->xmin - sxmax, sxmin - g->xmax), 0.0);
    double dgy = Max(Max(g->ymin - symax, symin - g->ymax), 0.0);
    if (dgx * dgx + dgy * dgy >= (*best) * (*best))
      return;
  }
  if (g->has_poly &&
      (geodist_geom_point_inside(cx1, cy1, g) || geodist_geom_point_inside(cx2, cy2, g)))
  {
    *best = 0.0;
    return;
  }
  /* Bucket bounding-volume hierarchy: skip whole buckets of edges that are
   * farther than the running minimum, then the per-edge prune within a bucket */
  for (int b = 0; b < g->nbk && *best > 0.0; b++)
  {
    const GeoDistBucket *bk = &g->bks[b];
    if (*best != DBL_MAX)
    {
      double dx = Max(Max(bk->xmin - sxmax, sxmin - bk->xmax), 0.0);
      double dy = Max(Max(bk->ymin - symax, symin - bk->ymax), 0.0);
      if (dx * dx + dy * dy >= (*best) * (*best))
        continue;
    }
    int e = bk->start + bk->n;
    for (int k = bk->start; k < e && *best > 0.0; k++)
    {
      const GeoDistEdge *ed = &g->segs[k];
      if (*best != DBL_MAX)
      {
        double dx = Max(Max(ed->xmin - sxmax, sxmin - ed->xmax), 0.0);
        double dy = Max(Max(ed->ymin - symax, symin - ed->ymax), 0.0);
        if (dx * dx + dy * dy >= (*best) * (*best))
          continue;
      }
      /* Radius-aware tighter prune (moving disc only, straight edges): the
       * exact swept-disc distance is at least the centre-segment-to-edge
       * distance minus the larger radius (min_t dist(centre(t),edge) - r(t) >=
       * segdist - rmax), so when segdist >= best + rmax the exact per-edge solve
       * is skipped. The axis-aligned swept box expanded by the radius is a loose
       * filter for a round disc; this rejects the many box survivors it leaves.
       * A scalar squared seg-seg distance is used (not lw_dist2d_seg_seg, which
       * also computes closest points and is far heavier in this hot loop). */
      if (! ed->is_arc && *best != DBL_MAX)
      {
        double rmax = Max(r1, r2);
        double thr = *best + rmax;
        if (rmax > 0.0 &&
            geodist_seg_seg_dist2(cx1, cy1, cx2, cy2, ed->x1, ed->y1,
              ed->x2, ed->y2) >= thr * thr)
          continue;
      }
      double m = ed->is_arc ?
        geodist_segm_arc_mindist(cx1, cy1, cx2, cy2, r1, r2, ed) :
        geodist_segm_edge_mindist(cx1, cy1, cx2, cy2, r1, r2, ed);
      if (m < *best) *best = m;
    }
  }
}

/**
 * @brief Update the witness with one swept-capsule unit against the geometry,
 * pruned by the bucket bounding-volume hierarchy (the centre moves from c1 to
 * c2 with radius r1 to r2; a stationary disk has c1 == c2)
 */
void
geodist_segm_shortestline(double cx1, double cy1, double r1, double cx2,
  double cy2, double r2, const GeoDistGeom *g, GeoDistShortLine *w)
{
  if (w->set && w->d <= 0.0)
    return;
  double sxmin = fmin(cx1 - r1, cx2 - r2);
  double sxmax = fmax(cx1 + r1, cx2 + r2);
  double symin = fmin(cy1 - r1, cy2 - r2);
  double symax = fmax(cy1 + r1, cy2 + r2);
  /* Coarse branch-and-bound prune: the swept bounding box to geometry bounding
   * box distance is a lower bound on this unit's nearest approach, so once it
   * reaches the running witness distance no edge can improve the shortest line.
   * A unit whose centre is inside a polygon overlaps the geometry box (zero
   * lower bound), so it is never wrongly pruned. */
  if (w->set)
  {
    double dgx = fmax(fmax(g->xmin - sxmax, sxmin - g->xmax), 0.0);
    double dgy = fmax(fmax(g->ymin - symax, symin - g->ymax), 0.0);
    if (dgx * dgx + dgy * dgy >= w->d * w->d)
      return;
  }
  /* A centre inside a polygon means the swept region overlaps it: the shortest
   * line is a zero-length line at that interior point (mirrors the
   * nearest-approach interior short-circuit) */
  if (g->has_poly)
  {
    bool in1 = geodist_geom_point_inside(cx1, cy1, g);
    bool in2 = (cx2 == cx1 && cy2 == cy1) ? in1 :
      geodist_geom_point_inside(cx2, cy2, g);
    if (in1 || in2)
    {
      double ix = in1 ? cx1 : cx2, iy = in1 ? cy1 : cy2;
      w->d = 0.0; w->px = ix; w->py = iy; w->qx = ix; w->qy = iy;
      w->set = true;
      return;
    }
  }
  /* Bucket bounding-volume hierarchy: skip whole buckets of edges that are
   * farther than the running witness distance, then the per-edge prune within
   * a bucket */
  for (int b = 0; b < g->nbk; b++)
  {
    const GeoDistBucket *bk = &g->bks[b];
    if (w->set)
    {
      double dx = fmax(fmax(bk->xmin - sxmax, sxmin - bk->xmax), 0.0);
      double dy = fmax(fmax(bk->ymin - symax, symin - bk->ymax), 0.0);
      if (dx * dx + dy * dy >= w->d * w->d)
        continue;
    }
    int elast = bk->start + bk->n;
    for (int k = bk->start; k < elast; k++)
    {
      const GeoDistEdge *e = &g->segs[k];
      if (w->set)
      {
        double dx = fmax(fmax(e->xmin - sxmax, sxmin - e->xmax), 0.0);
        double dy = fmax(fmax(e->ymin - symax, symin - e->ymax), 0.0);
        if (dx * dx + dy * dy >= w->d * w->d)
          continue;
      }
      /* Radius-aware tighter prune (moving disc only, straight edges): skip the
       * exact per-edge solve when the centre-segment-to-edge distance minus the
       * larger radius already reaches the running minimum. The same lower bound
       * as geodist_segm_nad, valid because it never exceeds the true swept-disc
       * distance, so the witness cannot change. */
      if (! e->is_arc && w->set)
      {
        double rmax = fmax(r1, r2);
        double thr = w->d + rmax;
        if (rmax > 0.0 &&
            geodist_seg_seg_dist2(cx1, cy1, cx2, cy2, e->x1, e->y1,
              e->x2, e->y2) >= thr * thr)
          continue;
      }
      double t;
      double m = e->is_arc ?
        geodist_segm_arc_dt(cx1, cy1, cx2, cy2, r1, r2, e, &t) :
        geodist_segm_edge_dt(cx1, cy1, cx2, cy2, r1, r2, e, &t);
      if (! w->set || m < w->d)
      {
        double ccx = cx1 + (cx2 - cx1) * t;
        double ccy = cy1 + (cy2 - cy1) * t;
        double rr = r1 + (r2 - r1) * t;
        double qx, qy;
        if (e->is_arc)
          geodist_geom_closest_on_arc(ccx, ccy, e, &qx, &qy);
        else
          geodist_geom_closest_on_edge(ccx, ccy, e, &qx, &qy);
        double vx = qx - ccx, vy = qy - ccy;
        double vl = sqrt(vx * vx + vy * vy);
        double pxp, pyp;
        if (vl <= 1e-12 || m <= 0.0)
        {
          /* Overlap or centre on the edge: degenerate line at the contact */
          pxp = qx; pyp = qy;
        }
        else
        {
          pxp = ccx + vx * (rr / vl);
          pyp = ccy + vy * (rr / vl);
        }
        w->d = m; w->px = pxp; w->py = pyp; w->qx = qx; w->qy = qy;
        w->set = true;
      }
    }
  }
}

/**
 * @brief Update the witness with one swept-capsule unit against the geometry,
 * pruned by the bucket bounding-volume hierarchy (the centre moves from c1 to
 * c2 with radius r1 to r2 over the time span t1 to t2; a stationary disc has
 * c1 == c2 and t1 == t2), mirroring #geodist_segm_shortestline
 */
void
geodist_segm_nai(double cx1, double cy1, double r1, TimestampTz t1, double cx2,
  double cy2, double r2, TimestampTz t2, const GeoDistGeom *g, GeoDistNai *w)
{
  if (w->set && w->d <= 0.0)
    return;
  double sxmin = fmin(cx1 - r1, cx2 - r2);
  double sxmax = fmax(cx1 + r1, cx2 + r2);
  double symin = fmin(cy1 - r1, cy2 - r2);
  double symax = fmax(cy1 + r1, cy2 + r2);
  /* Coarse branch-and-bound prune: the swept bounding box to geometry bounding
   * box distance is a lower bound on this unit's nearest approach, so once it
   * reaches the running witness distance no edge can improve it */
  if (w->set)
  {
    double dgx = fmax(fmax(g->xmin - sxmax, sxmin - g->xmax), 0.0);
    double dgy = fmax(fmax(g->ymin - symax, symin - g->ymax), 0.0);
    if (dgx * dgx + dgy * dgy >= w->d * w->d)
      return;
  }
  /* A centre inside a polygon means the swept region overlaps it: the nearest
   * approach distance is zero, recorded at the earliest inside endpoint */
  if (g->has_poly)
  {
    bool in1 = geodist_geom_point_inside(cx1, cy1, g);
    bool in2 = (cx2 == cx1 && cy2 == cy1) ? in1 :
      geodist_geom_point_inside(cx2, cy2, g);
    if (in1 || in2)
    {
      w->d = 0.0; w->t = in1 ? t1 : t2; w->set = true;
      return;
    }
  }
  /* Bucket bounding-volume hierarchy: skip whole buckets of edges that are
   * farther than the running witness distance, then the per-edge prune */
  for (int b = 0; b < g->nbk; b++)
  {
    const GeoDistBucket *bk = &g->bks[b];
    if (w->set)
    {
      double dx = fmax(fmax(bk->xmin - sxmax, sxmin - bk->xmax), 0.0);
      double dy = fmax(fmax(bk->ymin - symax, symin - bk->ymax), 0.0);
      if (dx * dx + dy * dy >= w->d * w->d)
        continue;
    }
    int elast = bk->start + bk->n;
    for (int k = bk->start; k < elast; k++)
    {
      const GeoDistEdge *e = &g->segs[k];
      if (w->set)
      {
        double dx = fmax(fmax(e->xmin - sxmax, sxmin - e->xmax), 0.0);
        double dy = fmax(fmax(e->ymin - symax, symin - e->ymax), 0.0);
        if (dx * dx + dy * dy >= w->d * w->d)
          continue;
      }
      /* Radius-aware tighter prune (moving disc only, straight edges): skip the
       * exact per-edge solve when the centre-segment-to-edge distance minus the
       * larger radius already reaches the running minimum. The same lower bound
       * as geodist_segm_nad, valid because it never exceeds the true swept-disc
       * distance, so the witness cannot change. */
      if (! e->is_arc && w->set)
      {
        double rmax = fmax(r1, r2);
        double thr = w->d + rmax;
        if (rmax > 0.0 &&
            geodist_seg_seg_dist2(cx1, cy1, cx2, cy2, e->x1, e->y1,
              e->x2, e->y2) >= thr * thr)
          continue;
      }
      double tf;
      double m = e->is_arc ?
        geodist_segm_arc_dt(cx1, cy1, cx2, cy2, r1, r2, e, &tf) :
        geodist_segm_edge_dt(cx1, cy1, cx2, cy2, r1, r2, e, &tf);
      /* Strict improvement keeps the earliest timestamp among equal minima
       * (the units are visited in time order) */
      if (! w->set || m < w->d)
      {
        if (tf < 0.0) tf = 0.0; else if (tf > 1.0) tf = 1.0;
        w->d = m;
        w->t = t1 + (TimestampTz) ((double) (t2 - t1) * tf);
        w->set = true;
        /* Overlap: the distance cannot drop below zero, so stop refining */
        if (m <= 0.0)
          return;
      }
    }
  }
}

/**
 * @brief Build a generic R-tree over the edge bounding boxes for the fixed-reach
 * relationship kernels; the index boxes use SRID 0 so a query needs no geometry
 * SRID
 */
RTree *
geodist_geom_build_rtree(const GeoDistEdge *segs, int n)
{
  RTree *rt = rtree_create_stbox();
  for (int k = 0; k < n; k++)
  {
    STBox box;
    stbox_set(true, false, false, 0, segs[k].xmin, segs[k].xmax, segs[k].ymin,
      segs[k].ymax, 0, 0, NULL, &box);
    rtree_insert(rt, &box, k);
  }
  return rt;
}

/*****************************************************************************
 * Compute the distance between two geometries
 *****************************************************************************/

/**
 * @brief Return the distance between two geometries
 * @details When the first geometry is a segment it also computes a value
 * between 0 and 1 that represents the location in the segment of the closest
 * point to the second geometry, as a fraction of total segment length.
 * @note Function inspired by PostGIS function lw_dist2d_distancepoint
 * from measures.c
 */
static double
lw_distance_fraction(const LWGEOM *geom1, const LWGEOM *geom2, int mode,
  double *fraction)
{
  double result;
  if (FLAGS_GET_GEODETIC(geom1->flags))
  {
    double min_dist = FLT_MAX;
    double max_dist = FLT_MAX;
    GEOGRAPHIC_POINT closest1, closest2;
    GEOGRAPHIC_EDGE e;
    const CIRC_NODE *circ_tree1 = lwgeom_calculate_circ_tree(geom1);
    const CIRC_NODE *circ_tree2 = lwgeom_calculate_circ_tree(geom2);
    circ_tree_distance_tree_internal(circ_tree1, circ_tree2, FP_TOLERANCE,
      &min_dist, &max_dist, &closest1, &closest2);
    result = sphere_distance(&closest1, &closest2);
    if (fraction)
    {
      assert(geom1->type == LINETYPE);
      LWLINE *lwline = lwgeom_as_lwline(geom1);
      /* Initialize edge */
      POINT4D a, b;
      GEOGRAPHIC_POINT proj;
      getPoint4d_p(lwline->points, 0, &a);
      getPoint4d_p(lwline->points, 1, &b);
      geographic_point_init(a.x, a.y, &(e.start));
      geographic_point_init(b.x, b.y, &(e.end));
      /* Get the spherical distance between point and edge */
      edge_distance_to_point(&e, &closest1, &proj);
      /* Compute distance from beginning of the segment to closest point */
      double seglength = sphere_distance(&(e.start), &(e.end));
      double length = sphere_distance(&(e.start), &proj);
      *fraction = length / seglength;
    }
  }
  else
  {
    if (FLAGS_GET_Z(geom1->flags))
    {
      DISTPTS3D dl;
      dl.mode = mode;
      dl.distance= FLT_MAX;
      dl.tolerance = 0;
      lw_dist3d_recursive(geom1, geom2, &dl);
      result = dl.distance;
      if (fraction)
      {
        assert(geom1->type == LINETYPE);
        LWLINE *lwline = lwgeom_as_lwline(geom1);
        POINT3DZ a, b, closest;
        getPoint3dz_p(lwline->points, 0, &a);
        getPoint3dz_p(lwline->points, 1, &b);
        *fraction = closest_point3dz_on_segment_ratio(&dl.p1, &a, &b, &closest);
      }
    }
    else
    {
      DISTPTS dl;
      dl.mode = mode;
      dl.distance= FLT_MAX;
      dl.tolerance = 0;
      lw_dist2d_recursive(geom1, geom2, &dl);
      result = dl.distance;
      if (fraction)
      {
        assert(geom1->type == LINETYPE);
        LWLINE *lwline = lwgeom_as_lwline(geom1);
        POINT2D a, b, closest;
        getPoint2d_p(lwline->points, 0, &a);
        getPoint2d_p(lwline->points, 1, &b);
        *fraction = closest_point2d_on_segment_ratio(&dl.p1, &a, &b, &closest);
      }
    }
  }
  return result;
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return 1 or 2 if a temporal point segment and a point are at a 
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start,end Values defining the first segment
 * @param[in] point Point to locate
 * @param[in] lower,upper Minimum distance at turning point
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segment is not constant
 * @post As there is a single turning point, `t2` is set to `t1`
 */
static int
tpoint_geo_distance_turnpt(Datum start, Datum end, Datum point, 
  TimestampTz lower, TimestampTz upper, TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  long double duration = (long double) (upper - lower);
  double dist;
  double fraction = (double) pointsegm_locate(start, end, point, &dist);
  if (fraction < 0.0)
    return 0;
  *t1 = *t2 = lower + (TimestampTz) (duration * fraction);
  return 1;
}

/**
 * @brief Compute d/dx (Euclidean distance) = 0 for two 2D segments
 */
static bool
point2d_min_dist(const POINT2D *p1, const POINT2D *p2, const POINT2D *p3,
  const POINT2D *p4, double *fraction)
{
  long double denum, dx1, dy1, dx2, dy2, f1, f2, f3, f4;

  dx1 = p2->x - p1->x;
  dy1 = p2->y - p1->y;
  dx2 = p4->x - p3->x;
  dy2 = p4->y - p3->y;

  f1 = p3->x * (dx1 - dx2);
  f2 = p1->x * (dx2 - dx1);
  f3 = p3->y * (dy1 - dy2);
  f4 = p1->y * (dy2 - dy1);

  denum = dx1 * (dx1 - 2 * dx2) + dy1 * (dy1 - 2 * dy2) + dy2 * dy2 +
    dx2 * dx2;
  if (denum == 0)
    return false;

  *fraction = (f1 + f2 + f3 + f4) / denum;
  if (*fraction <= MEOS_EPSILON || *fraction >= (1.0 - MEOS_EPSILON))
    return false;

  return true;
}

/**
 * @brief Compute d/dx (Euclidean distance) = 0 for two 3D segments
 */
bool
point3d_min_dist(const POINT3DZ *p1, const POINT3DZ *p2, const POINT3DZ *p3,
  const POINT3DZ *p4, double *fraction)
{
  long double denum, dx1, dy1, dz1, dx2, dy2, dz2, f1, f2, f3, f4, f5, f6;

  dx1 = p2->x - p1->x;
  dy1 = p2->y - p1->y;
  dz1 = p2->z - p1->z;
  dx2 = p4->x - p3->x;
  dy2 = p4->y - p3->y;
  dz2 = p4->z - p3->z;

  f1 = p3->x * (dx1 - dx2);
  f2 = p1->x * (dx2 - dx1);
  f3 = p3->y * (dy1 - dy2);
  f4 = p1->y * (dy2 - dy1);
  f5 = p3->z * (dz1 - dz2);
  f6 = p1->z * (dz2 - dz1);

  denum = dx1 * (dx1 - 2 * dx2) + dy1 * (dy1 - 2 * dy2) +
    dz1 * (dz1 - 2 * dz2) + dx2 * dx2 + dy2 * dy2 + dz2 * dz2;
  if (denum == 0)
    return false;

  *fraction = (f1 + f2 + f3 + f4 + f5 + f6) / denum;
  if (*fraction <= MEOS_EPSILON || *fraction >= (1.0 - MEOS_EPSILON))
    return false;

  return true;
}

/**
 * @brief Return 1 or 2 if two temporal geometry point segments are at a
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance.
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @note The PostGIS functions @p lw_dist2d_seg_seg and @p lw_dist3d_seg_seg
 * cannot be used since they do not take time into consideration and would
 * return, e.g., that the minimum distance between the two following segments
 * `[Point(2 2)@t1, Point(1 1)@t2]` and `[Point(3 1)@t1, Point(1 1)@t2]`
 * is at `Point(2 2)@t2` instead of `Point(1.5 1.5)@(t1 + (t2 - t1)/2)`.
 * @pre The segments are not both constants.
 * @post As there is a single turning point, `t2` is set to `t`
 */
int
tgeompointsegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  double fraction;
  bool hasz = FLAGS_GET_Z(DatumGetGserializedP(start1)->gflags);
  if (hasz) /* 3D */
  {
    const POINT3DZ *p1 = DATUM_POINT3DZ_P(start1);
    const POINT3DZ *p2 = DATUM_POINT3DZ_P(end1);
    const POINT3DZ *p3 = DATUM_POINT3DZ_P(start2);
    const POINT3DZ *p4 = DATUM_POINT3DZ_P(end2);
    bool found = point3d_min_dist(p1, p2, p3, p4, &fraction);
    if (!found)
      return 0;
  }
  else /* 2D */
  {
    const POINT2D *p1 = DATUM_POINT2D_P(start1);
    const POINT2D *p2 = DATUM_POINT2D_P(end1);
    const POINT2D *p3 = DATUM_POINT2D_P(start2);
    const POINT2D *p4 = DATUM_POINT2D_P(end2);
    bool found = point2d_min_dist(p1, p2, p3, p4, &fraction);
    if (! found)
      return 0;
  }

  double duration = upper - lower;
  *t1 = *t2 = lower + (TimestampTz) (duration * fraction);
  return 1;
}

/**
 * @brief Return 1 or 2 if two temporal geography point segments are at a
 * minimum distance during the period defined by the output timestamps, return
 * 0 otherwise
 * @details These are the turning points when computing the temporal distance
 * @param[in] start1,end1 Values defining the first segment
 * @param[in] start2,end2 Values defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not both constants
 * @post As there is a single turning point, `t2` is  set to `t1`
 */
int
tgeogpointsegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  const POINT2D *p1 = DATUM_POINT2D_P(start1);
  const POINT2D *p2 = DATUM_POINT2D_P(end1);
  const POINT2D *q1 = DATUM_POINT2D_P(start2);
  const POINT2D *q2 = DATUM_POINT2D_P(end2);
  GEOGRAPHIC_EDGE e1, e2;
  POINT3D A1, A2, B1, B2;
  geographic_point_init(p1->x, p1->y, &(e1.start));
  geographic_point_init(p2->x, p2->y, &(e1.end));
  geographic_point_init(q1->x, q1->y, &(e2.start));
  geographic_point_init(q2->x, q2->y, &(e2.end));
  geog2cart(&(e1.start), &A1);
  geog2cart(&(e1.end), &A2);
  geog2cart(&(e2.start), &B1);
  geog2cart(&(e2.end), &B2);
  // TODO: The next computation should be done on geodetic coordinates
  // The value found by the linear approximation below could be the starting
  // point for an iterative method such as gradient descent or Newton's method
  double fraction;
  if (! point3d_min_dist((const POINT3DZ *) &A1, (const POINT3DZ *) &A2,
      (const POINT3DZ *) &B1, (const POINT3DZ *) &B2, &fraction))
    return 0;

  /* Compute the timestamp of intersection */
  if (fraction <= MEOS_EPSILON || fraction >= (1.0 - MEOS_EPSILON))
    return 0;
  double duration = (double) (upper - lower);
  *t1 = *t2 = lower + (TimestampTz) (duration * fraction);
  return 1;
}

/**
 * @brief Return 1 or 2 if two temporal point segments are at a minimum
 * distance during the period defined by the output timestamps, return 0
 * otherwise
 * @param[in] start1,end1 Instants defining the first segment
 * @param[in] start2,end2 Instants defining the second segment
 * @param[in] param Additional parameter
 * @param[in] lower,upper Timestamps defining the segments
 * @param[out] t1,t2 Timestamps defining the resulting period, may be equal
 * @pre The segments are not both constants.
 */
int
tpointsegm_distance_turnpt(Datum start1, Datum end1, Datum start2,
  Datum end2, Datum param, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2)
{
  assert(lower < upper); assert(t1); assert(t2);
  if (FLAGS_GET_GEODETIC(DatumGetGserializedP(start1)->gflags))
    return tgeogpointsegm_distance_turnpt(start1, end1, start2, end2,
      param, lower, upper, t1, t2);
  else
    return tgeompointsegm_distance_turnpt(start1, end1, start2, end2,
      param, lower, upper, t1, t2);
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the temporal distance between a temporal geo and a
 * geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #Tdistance_tgeo_geo()
 */
Temporal *
tdistance_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return NULL;

  /* Native GEOS-free branch: the temporal distance between a temporal
   * geometric point with linear interpolation and a non-point clip-supported
   * planar 2D geometry (including curves) is computed exactly, lifting the
   * point-operand-only restriction below. Temporal geographies, step/discrete
   * interpolation, 3D and non-clip-supported geometries keep the generic
   * lifting path */
  if (temp->temptype == T_TGEOMPOINT && temp->subtype != TINSTANT &&
      MEOS_FLAGS_LINEAR_INTERP(temp->flags) &&
      ! MEOS_FLAGS_GET_Z(temp->flags))
  {
    LWGEOM *geom = lwgeom_from_gserialized(gs);
    bool native = geom->type != POINTTYPE && geom_clip_supported(geom);
    lwgeom_free(geom);
    if (native)
      return tpoint_linear_distance_geom(temp, gs);
  }

  /* Only point geometries are accepted by the generic lifting path */
  if (tpoint_type(temp->temptype) && ! ensure_point_type(gs))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) geo_distance_fn(temp->flags);
  lfinfo.argtype[0] = temp->temptype;
  lfinfo.argtype[1] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_base = lfinfo.reslinear ? &tpoint_geo_distance_turnpt : NULL;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the temporal distance between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #Tdistance_tgeo_tgeo()
 */
Temporal *
tdistance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) geo_distance_fn(temp1->flags);
  lfinfo.argtype[0] = lfinfo.argtype[1] = temp1->temptype;
  lfinfo.restype = T_TFLOAT;
  lfinfo.reslinear = MEOS_FLAGS_LINEAR_INTERP(temp1->flags) ||
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags);
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = CONTINUOUS;
  lfinfo.tpfn_temp = lfinfo.reslinear ? &tpointsegm_distance_turnpt : NULL;
  return tfunc_temporal_temporal(temp1, temp2, &lfinfo);
}

/*****************************************************************************
 * Time-synchronous nearest-approach running minimum (temporal point)
 *
 * For two linear temporal points, the minimum of the temporal distance
 * `min_t distance(temp1(t), temp2(t))` and the timestamp achieving it are
 * obtained by a single synchronization pass, WITHOUT materializing
 * `tdistance_tgeo_tgeo(temp1, temp2)` and reducing it.  The minimum of the
 * temporal distance is the minimum over each synchronized segment of its
 * endpoint values and its analytic turning point(s), i.e. exactly the
 * candidate set that `tdistance` emits as instants.  The pass mirrors the
 * synchronization + turning-point logic of `tfunc_tcontseq_tcontseq_single`
 * (lifting.c) and the sequence-set walk of `tfunc_tsequenceset_tsequenceset`,
 * reducing to the minimum instead of building a sequence.  Used by the
 * nad/nai/shortestline `tgeo_tgeo` functions for the temporal point case;
 * other temporal geos keep the `tdistance` path.  Equivalent to
 * `temporal_min_value(tdistance_tgeo_tgeo(...))`.
 *****************************************************************************/

/**
 * @brief Return the minimum time-synchronous distance over the overlapping
 * period of two linear temporal point sequences, updating the running minimum
 * @param[in] seq1,seq2 Temporal point sequences with linear interpolation
 * @param[in] inter Overlapping period of the two sequences
 * @param[in] func Base point distance function
 * @param[in] curmin Current minimum distance, or infinity at the beginning
 * @param[in,out] tmin Timestamp achieving the running minimum
 * @pre Both sequences are linear temporal points; @p inter is their overlap
 */
static double
nad_tpointseq_tpointseq_sync(const TSequence *seq1, const TSequence *seq2,
  const Span *inter, datum_func2 func, double curmin, TimestampTz *tmin)
{
  MeosType temptype = seq1->temptype;
  TInstant *inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, 0);
  TInstant *inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, 0);
  const TInstant *prev1 = NULL, *prev2 = NULL; /* make compiler quiet */
  TimestampTz lower = DatumGetTimestampTz(inter->lower);
  TimestampTz upper = DatumGetTimestampTz(inter->upper);
  int i = 0, j = 0, ninsts = 0, nfree = 0;
  if (inst1->t < lower)
  {
    i = tcontseq_find_timestamptz(seq1, inter->lower) + 1;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
  }
  else if (inst2->t < lower)
  {
    j = tcontseq_find_timestamptz(seq2, inter->lower) + 1;
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  /* Synchronized instants that must be freed at the end (they may still be
   * referenced as the previous instant in the next iteration) */
  int count = seq1->count - i + seq2->count - j;
  TInstant **tofree = palloc(sizeof(TInstant *) * count);
  while (i < seq1->count && j < seq2->count &&
    (inst1->t <= upper || inst2->t <= upper))
  {
    /* Synchronize the two start instants */
    int cmp = timestamptz_cmp_internal(inst1->t, inst2->t);
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
    {
      i++;
      inst2 = tcontseq_at_timestamptz(seq2, inst1->t);
      tofree[nfree++] = inst2;
    }
    else
    {
      j++;
      inst1 = tcontseq_at_timestamptz(seq1, inst2->t);
      tofree[nfree++] = inst1;
    }
    /* If not the first instant, evaluate the distance at the potential
     * turning point(s) interior to the segment */
    if (ninsts > 0)
    {
      Datum start1 = tinstant_value_p(prev1);
      Datum end1 = tinstant_value_p(inst1);
      Datum start2 = tinstant_value_p(prev2);
      Datum end2 = tinstant_value_p(inst2);
      TimestampTz tpt1, tpt2;
      int found = tpointsegm_distance_turnpt(start1, end1, start2, end2,
        (Datum) 0, prev1->t, inst1->t, &tpt1, &tpt2);
      if (found)
      {
        Datum v1 = tsegment_value_at_timestamptz(start1, end1, temptype,
          prev1->t, inst1->t, tpt1);
        Datum v2 = tsegment_value_at_timestamptz(start2, end2, temptype,
          prev1->t, inst1->t, tpt1);
        double d = DatumGetFloat8(func(v1, v2));
        if (d < curmin) { curmin = d; *tmin = tpt1; }
        pfree(DatumGetPointer(v1)); pfree(DatumGetPointer(v2));
        /* Account for the second turning point if any */
        if (found > 1)
        {
          v1 = tsegment_value_at_timestamptz(start1, end1, temptype,
            prev1->t, inst1->t, tpt2);
          v2 = tsegment_value_at_timestamptz(start2, end2, temptype,
            prev1->t, inst1->t, tpt2);
          d = DatumGetFloat8(func(v1, v2));
          if (d < curmin) { curmin = d; *tmin = tpt2; }
          pfree(DatumGetPointer(v1)); pfree(DatumGetPointer(v2));
        }
      }
    }
    /* Evaluate the distance at the synchronized instant */
    double d = DatumGetFloat8(func(tinstant_value_p(inst1),
      tinstant_value_p(inst2)));
    if (d < curmin) { curmin = d; *tmin = inst1->t; }
    ninsts++;
    if (i == seq1->count || j == seq2->count)
      break;
    prev1 = inst1;
    prev2 = inst2;
    inst1 = (TInstant *) TSEQUENCE_INST_N(seq1, i);
    inst2 = (TInstant *) TSEQUENCE_INST_N(seq2, j);
  }
  pfree_array((void **) tofree, nfree);
  return curmin;
}

/**
 * @brief Return the minimum time-synchronous distance between two linear
 * temporal points and the timestamp achieving it
 * @param[in] temp1,temp2 Temporal points with linear interpolation and equal
 * subtype (both #TSEQUENCE or both #TSEQUENCESET)
 * @param[out] tmin Timestamp achieving the minimum (set only when the result
 * is not infinity)
 * @return The minimum distance, or infinity if the time frames do not
 * intersect
 * @pre The temporal points are validated, linear, and of the same subtype
 */
static double
nad_tpoint_tpoint_sync(const Temporal *temp1, const Temporal *temp2,
  TimestampTz *tmin)
{
  datum_func2 func = geo_distance_fn(temp1->flags);
  if (temp1->subtype == TSEQUENCE)
  {
    Span inter;
    if (! inter_span_span(&((TSequence *) temp1)->period,
        &((TSequence *) temp2)->period, &inter))
      return DBL_MAX;
    return nad_tpointseq_tpointseq_sync((TSequence *) temp1,
      (TSequence *) temp2, &inter, func, DBL_MAX, tmin);
  }
  /* Both TSEQUENCESET: walk the overlapping component sequences */
  const TSequenceSet *ss1 = (const TSequenceSet *) temp1;
  const TSequenceSet *ss2 = (const TSequenceSet *) temp2;
  double result = DBL_MAX;
  int i = 0, j = 0;
  while (i < ss1->count && j < ss2->count)
  {
    const TSequence *seq1 = TSEQUENCESET_SEQ_N(ss1, i);
    const TSequence *seq2 = TSEQUENCESET_SEQ_N(ss2, j);
    Span inter;
    if (inter_span_span(&seq1->period, &seq2->period, &inter))
      result = nad_tpointseq_tpointseq_sync(seq1, seq2, &inter, func, result,
        tmin);
    int cmp = timestamptz_cmp_internal(
      DatumGetTimestampTz(seq1->period.upper),
      DatumGetTimestampTz(seq2->period.upper));
    if (cmp == 0)
    {
      i++; j++;
    }
    else if (cmp < 0)
      i++;
    else
      j++;
  }
  return result;
}

/**
 * @brief Return true if the time-synchronous nearest-approach running-minimum
 * fast path applies to two temporal geos
 * @details Applies to linear temporal points of equal, non-instant subtype;
 * everything else (temporal geometries, step interpolation, instants, mixed
 * subtypes) keeps the @ref tdistance_tgeo_tgeo path
 */
static bool
nad_tpoint_tpoint_sync_applies(const Temporal *temp1, const Temporal *temp2)
{
  return tpoint_type(temp1->temptype) &&
    MEOS_FLAGS_LINEAR_INTERP(temp1->flags) &&
    MEOS_FLAGS_LINEAR_INTERP(temp2->flags) &&
    temp1->subtype != TINSTANT && temp1->subtype == temp2->subtype;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @brief Return the new current nearest approach instant between a temporal
 * point sequence with step interpolation and a geometry/geography
 * (iterator function)
 * @param[in] seq Temporal geo
 * @param[in] geo Geometry/geography
 * @param[in] mindist Current minimum distance, it is set at DBL_MAX at the
 * begining but contains the minimum distance found in the previous
 * sequences of a temporal sequence set
 * @param[out] result Instant with the minimum distance
 * @return Minimum distance
 */
static double
nai_tgeoseq_discstep_geo_iter(const TSequence *seq, const LWGEOM *geo,
  double mindist, const TInstant **result)
{
  for (int i = 0; i < seq->count; i++)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, i);
    const GSERIALIZED *gs = DatumGetGserializedP(tinstant_value_p(inst));
    LWGEOM *point = lwgeom_from_gserialized(gs);
    double dist = lw_distance_fraction(point, geo, DIST_MIN, NULL);
    if (dist < mindist)
    {
      mindist = dist;
      *result = inst;
    }
    lwgeom_free(point);
  }
  return mindist;
}

/**
 * @brief Return the nearest approach instant between a temporal sequence
 * point with step interpolation and a geometry/geography
 * @param[in] seq Temporal geo
 * @param[in] geo Geometry/geography
 */
static TInstant *
nai_tgeoseq_discstep_geo(const TSequence *seq, const LWGEOM *geo)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  nai_tgeoseq_discstep_geo_iter(seq, geo, DBL_MAX, &inst);
  return tinstant_copy(inst);
}

/**
 * @brief Return the nearest approach instant between a temporal sequence set
 * point with step interpolation and a geometry/geography
 * @param[in] ss Temporal geo
 * @param[in] geo Geometry/geography
 */
static TInstant *
nai_tgeoseqset_step_geo(const TSequenceSet *ss, const LWGEOM *geo)
{
  const TInstant *inst = NULL; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ss->count; i++)
    mindist = nai_tgeoseq_discstep_geo_iter(TSEQUENCESET_SEQ_N(ss, i), geo,
      mindist, &inst);
  assert(inst);
  return tinstant_copy(inst);
}

/*****************************************************************************
 * Nearest approach instant between a temporal point with linear interpolation
 * and a geometry/geography
 *****************************************************************************/

/**
 * @brief Return the distance and the timestamp of the nearest approach instant
 * between a temporal point sequence with linear interpolation and a
 * geometry/geography
 * @param[in] inst1,inst2 Temporal segment
 * @param[in] geo Geometry/geography
 * @param[out] t Timestamp
 */
static double
nai_tpointsegm_linear_geo1(const TInstant *inst1, const TInstant *inst2,
  const LWGEOM *geo, TimestampTz *t)
{
  Datum value1 = tinstant_value_p(inst1);
  Datum value2 = tinstant_value_p(inst2);
  double dist;
  double fraction;

  /* Constant segment */
  if (datum_point_eq(value1, value2))
  {
    GSERIALIZED *gs = DatumGetGserializedP(value1);
    LWGEOM *point = lwgeom_from_gserialized(gs);
    dist = lw_distance_fraction(point, geo, DIST_MIN, NULL);
    lwgeom_free(point);
    *t = inst1->t;
    return dist;
  }

  /* The trajectory is a line */
  LWGEOM *line = (LWGEOM *) lwline_make(value1, value2);
  dist = lw_distance_fraction(line, geo, DIST_MIN, &fraction);
  lwgeom_free(line);

  if (fabsl(fraction) < MEOS_EPSILON)
    *t = inst1->t;
  else if (fabsl(fraction - 1.0) < MEOS_EPSILON)
    *t = inst2->t;
  else
  {
    double duration = (double) (inst2->t - inst1->t);
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return dist;
}

/**
 * @brief Return the distance and the timestamp of the nearest approach instant
 * between a temporal point sequence with linear interpolation and a
 * geometry/geography (iterator function)
 * @param[in] seq Temporal geo
 * @param[in] geo Geometry/geography
 * @param[in] mindist Minimum distance found so far, or DBL_MAX at the beginning
 * @param[out] t Timestamp
 */
static double
nai_tpointseq_linear_geo_iter(const TSequence *seq, const LWGEOM *geo,
  double mindist, TimestampTz *t)
{
  double dist;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);

  if (seq->count == 1)
  {
    /* Instantaneous sequence */
    Datum value1 = tinstant_value_p(inst1);
    GSERIALIZED *gs = DatumGetGserializedP(value1);
    LWGEOM *point = lwgeom_from_gserialized(gs);
    dist = lw_distance_fraction(point, geo, DIST_MIN, NULL);
    if (dist < mindist)
    {
      mindist = dist;
      *t = inst1->t;
    }
    lwgeom_free(point);
  }
  else
  {
    /* General case */
    TimestampTz t1;
    for (int i = 0; i < seq->count - 1; i++)
    {
      const TInstant *inst2 = TSEQUENCE_INST_N(seq, i + 1);
      dist = nai_tpointsegm_linear_geo1(inst1, inst2, geo, &t1);
      if (dist < mindist)
      {
        mindist = dist;
        *t = t1;
      }
      if (mindist == 0.0)
        break;
      inst1 = inst2;
    }
  }
  return mindist;
}

/**
 * @brief Return the nearest approach instant between a temporal sequence
 * point with linear interpolation and a geometry (iterator function)
 */
static TInstant *
nai_tpointseq_linear_geo(const TSequence *seq, const LWGEOM *geo)
{
  TimestampTz t;
  nai_tpointseq_linear_geo_iter(seq, geo, DBL_MAX, &t);
  /* The closest point may be at an exclusive bound */
  Datum value;
  tsequence_value_at_timestamptz(seq, t, false, &value);
  return tinstant_make_free(value, seq->temptype, t);
}

/**
 * @brief Return the nearest approach instant between a temporal sequence set
 * point with linear interpolation and a geometry
 */
static TInstant *
nai_tpointseqset_linear_geo(const TSequenceSet *ss, const LWGEOM *geo)
{
  TimestampTz t = 0; /* make compiler quiet */
  double mindist = DBL_MAX;
  for (int i = 0; i < ss->count; i++)
  {
    TimestampTz t1;
    double dist = nai_tpointseq_linear_geo_iter(TSEQUENCESET_SEQ_N(ss, i), geo,
      mindist, &t1);
    if (dist < mindist)
    {
      mindist = dist;
      t = t1;
    }
    if (mindist == 0.0)
      break;
  }
  /* The closest point may be at an exclusive bound. */
  Datum value;
  tsequenceset_value_at_timestamptz(ss, t, false, &value);
  return tinstant_make_free(value, ss->temptype, t);
}

/*****************************************************************************/

/*****************************************************************************
 * GEOS-free analytic distance between a temporal point and a geometry
 *
 * A planar 2D linear temporal point is a moving disc of radius 0, so its
 * nearest approach, shortest line and nearest approach instant against a
 * geometry are computed by the shared engine that backs the temporal circular
 * buffer distance, at radius 0. Geodetic, 3D, moving (multi)polygons and
 * geometries the engine cannot decompose fall back to the liblwgeom
 * trajectory path.
 *****************************************************************************/

/**
 * @brief Decompose a geometry into boundary edges and build the Morton bucket
 * hierarchy for the nearest-approach kernels
 * @return False when the geometry has no usable edges (curved beyond circular
 * arcs, or otherwise unsupported), so the caller falls back
 */
bool
geodist_geom_build(const GSERIALIZED *gs, GeoDistGeom *g)
{
  LWGEOM *lw = lwgeom_from_gserialized(gs);
  GeoDistEdge *segs = NULL;
  int cap = 0, n = 0;
  bool has_poly = false;
  bool ok = geodist_geom_edges(lw, true, &segs, &cap, &n, &has_poly);
  lwgeom_free(lw);
  if (! ok || n == 0)
  {
    if (segs) pfree(segs);
    return false;
  }
  double gxmin = DBL_MAX, gymin = DBL_MAX, gxmax = -DBL_MAX, gymax = -DBL_MAX;
  for (int k = 0; k < n; k++)
  {
    if (segs[k].xmin < gxmin) gxmin = segs[k].xmin;
    if (segs[k].ymin < gymin) gymin = segs[k].ymin;
    if (segs[k].xmax > gxmax) gxmax = segs[k].xmax;
    if (segs[k].ymax > gymax) gymax = segs[k].ymax;
  }
  int nbk = 0;
  GeoDistBucket *bks = geodist_geom_build_buckets(segs, n, gxmin, gymin, gxmax,
    gymax, &nbk);
  g->segs = segs; g->n = n; g->has_poly = has_poly;
  g->xmin = gxmin; g->ymin = gymin; g->xmax = gxmax; g->ymax = gymax;
  g->bks = bks; g->nbk = nbk; g->rtree = NULL;
  return true;
}

/**
 * @brief Free the segments and bucket hierarchy of a geometry decomposition
 */
void
geodist_geom_free(GeoDistGeom *g)
{
  if (g->bks) pfree((void *) g->bks);
  if (g->segs) pfree((void *) g->segs);
}

/**
 * @brief Point of a temporal point instant as a 2D point
 */
static const POINT2D *
tgeoinst_point2d(const TInstant *inst)
{
  return GSERIALIZED_POINT2D_P(DatumGetGserializedP(tinstant_value_p(inst)));
}

/*****************************************************************************/

/**
 * @brief Update the nearest-approach minimum with one temporal point sequence,
 * feeding each segment as a radius-0 disc
 */
static void
tgeoseq_nad(const TSequence *seq, const GeoDistGeom *g, double *best)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && *best > 0.0; i++)
    {
      const POINT2D *p = tgeoinst_point2d(TSEQUENCE_INST_N(seq, i));
      geodist_segm_nad(p->x, p->y, 0.0, p->x, p->y, 0.0, g, best);
    }
    return;
  }
  const TInstant *i1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count && *best > 0.0; i++)
  {
    const TInstant *i2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *p1 = tgeoinst_point2d(i1);
    const POINT2D *p2 = tgeoinst_point2d(i2);
    geodist_segm_nad(p1->x, p1->y, 0.0, p2->x, p2->y, 0.0, g, best);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free nearest approach distance between a planar temporal point
 * and a geometry; returns false when the geometry does not decompose
 */
static bool
nad_tgeo_geo_analytic(const Temporal *temp, const GSERIALIZED *gs,
  double *result)
{
  GeoDistGeom g;
  if (! geodist_geom_build(gs, &g))
    return false;
  double best = DBL_MAX;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const POINT2D *p = tgeoinst_point2d((TInstant *) temp);
    geodist_segm_nad(p->x, p->y, 0.0, p->x, p->y, 0.0, &g, &best);
  }
  else if (temp->subtype == TSEQUENCE)
    tgeoseq_nad((TSequence *) temp, &g, &best);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && best > 0.0; i++)
      tgeoseq_nad(TSEQUENCESET_SEQ_N(ss, i), &g, &best);
  }
  geodist_geom_free(&g);
  *result = best < 0.0 ? 0.0 : best;
  return true;
}

/*****************************************************************************/

/**
 * @brief Update the shortest-line witness with one temporal point sequence
 */
static void
tgeoseq_shortestline(const TSequence *seq, const GeoDistGeom *g,
  GeoDistShortLine *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const POINT2D *p = tgeoinst_point2d(TSEQUENCE_INST_N(seq, i));
      geodist_segm_shortestline(p->x, p->y, 0.0, p->x, p->y, 0.0, g, w);
    }
    return;
  }
  const TInstant *i1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count && ! (w->set && w->d <= 0.0); i++)
  {
    const TInstant *i2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *p1 = tgeoinst_point2d(i1);
    const POINT2D *p2 = tgeoinst_point2d(i2);
    geodist_segm_shortestline(p1->x, p1->y, 0.0, p2->x, p2->y, 0.0, g, w);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free shortest line between a planar temporal point and a
 * geometry; returns NULL when the geometry does not decompose
 */
static GSERIALIZED *
shortestline_tgeo_geo_analytic(const Temporal *temp, const GSERIALIZED *gs)
{
  GeoDistGeom g;
  if (! geodist_geom_build(gs, &g))
    return NULL;
  GeoDistShortLine w;
  w.d = DBL_MAX; w.set = false;
  w.px = w.py = w.qx = w.qy = 0.0;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const POINT2D *p = tgeoinst_point2d((TInstant *) temp);
    geodist_segm_shortestline(p->x, p->y, 0.0, p->x, p->y, 0.0, &g, &w);
  }
  else if (temp->subtype == TSEQUENCE)
    tgeoseq_shortestline((TSequence *) temp, &g, &w);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && ! (w.set && w.d <= 0.0); i++)
      tgeoseq_shortestline(TSEQUENCESET_SEQ_N(ss, i), &g, &w);
  }
  geodist_geom_free(&g);
  if (! w.set)
    return NULL;
  int32_t srid = gserialized_get_srid(gs);
  POINTARRAY *pa = ptarray_construct(0, 0, 2);
  POINT4D p4;
  p4.z = 0.0; p4.m = 0.0;
  p4.x = w.px; p4.y = w.py;
  ptarray_set_point4d(pa, 0, &p4);
  p4.x = w.qx; p4.y = w.qy;
  ptarray_set_point4d(pa, 1, &p4);
  LWLINE *ln = lwline_construct(srid, NULL, pa);
  GSERIALIZED *line = geo_serialize(lwline_as_lwgeom(ln));
  lwline_free(ln);
  return line;
}

/*****************************************************************************/

/**
 * @brief Update the nearest-approach-instant witness with one temporal point
 * sequence
 */
static void
tgeoseq_nai(const TSequence *seq, const GeoDistGeom *g, GeoDistNai *w)
{
  bool linear = MEOS_FLAGS_LINEAR_INTERP(seq->flags);
  if (seq->count == 1 || ! linear)
  {
    for (int i = 0; i < seq->count && ! (w->set && w->d <= 0.0); i++)
    {
      const TInstant *inst = TSEQUENCE_INST_N(seq, i);
      const POINT2D *p = tgeoinst_point2d(inst);
      geodist_segm_nai(p->x, p->y, 0.0, inst->t, p->x, p->y, 0.0, inst->t, g, w);
    }
    return;
  }
  const TInstant *i1 = TSEQUENCE_INST_N(seq, 0);
  for (int i = 1; i < seq->count && ! (w->set && w->d <= 0.0); i++)
  {
    const TInstant *i2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *p1 = tgeoinst_point2d(i1);
    const POINT2D *p2 = tgeoinst_point2d(i2);
    geodist_segm_nai(p1->x, p1->y, 0.0, i1->t, p2->x, p2->y, 0.0, i2->t, g, w);
    i1 = i2;
  }
}

/**
 * @brief GEOS-free nearest approach instant between a planar temporal point
 * and a geometry; returns false when the geometry does not decompose
 */
static bool
nai_tgeo_geo_analytic(const Temporal *temp, const GSERIALIZED *gs,
  TimestampTz *result)
{
  GeoDistGeom g;
  if (! geodist_geom_build(gs, &g))
    return false;
  GeoDistNai w;
  w.d = DBL_MAX; w.t = 0; w.set = false;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TINSTANT)
  {
    const TInstant *inst = (TInstant *) temp;
    const POINT2D *p = tgeoinst_point2d(inst);
    geodist_segm_nai(p->x, p->y, 0.0, inst->t, p->x, p->y, 0.0, inst->t, &g, &w);
  }
  else if (temp->subtype == TSEQUENCE)
    tgeoseq_nai((TSequence *) temp, &g, &w);
  else
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    for (int i = 0; i < ss->count && ! (w.set && w.d <= 0.0); i++)
      tgeoseq_nai(TSEQUENCESET_SEQ_N(ss, i), &g, &w);
  }
  geodist_geom_free(&g);
  if (! w.set)
    return false;
  *result = w.t;
  return true;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach instant between a temporal geo and
 * a geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #NAI_tgeo_geo()
 */
TInstant *
nai_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return NULL;

  /* Planar 2D temporal point: the GEOS-free analytic engine at radius 0 */
  if (tpoint_type(temp->temptype) && ! MEOS_FLAGS_GET_GEODETIC(temp->flags) &&
      ! MEOS_FLAGS_GET_Z(temp->flags))
  {
    TimestampTz t;
    if (nai_tgeo_geo_analytic(temp, gs, &t))
    {
      Datum value;
      temporal_value_at_timestamptz(temp, t, false, &value);
      return tinstant_make_free(value, temp->temptype, t);
    }
  }

  LWGEOM *geo = lwgeom_from_gserialized(gs);
  TInstant *result;
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      result = tinstant_copy((TInstant *) temp);
      break;
    case TSEQUENCE:
      result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        nai_tpointseq_linear_geo((TSequence *) temp, geo) :
        nai_tgeoseq_discstep_geo((TSequence *) temp, geo);
      break;
    default: /* TSEQUENCESET */
      result = MEOS_FLAGS_LINEAR_INTERP(temp->flags) ?
        nai_tpointseqset_linear_geo((TSequenceSet *) temp, geo) :
        nai_tgeoseqset_step_geo((TSequenceSet *) temp, geo);
  }
  lwgeom_free(geo);
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach instant between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #NAI_tgeo_tgeo()
 */
TInstant *
nai_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  /* Fast path: linear temporal points via the time-synchronous running
   * minimum, avoiding the temporal distance materialization */
  if (nad_tpoint_tpoint_sync_applies(temp1, temp2))
  {
    TimestampTz t;
    if (nad_tpoint_tpoint_sync(temp1, temp2, &t) != DBL_MAX)
    {
      /* The closest point may be at an exclusive bound => 3rd arg = false */
      Datum value;
      temporal_value_at_timestamptz(temp1, t, false, &value);
      return tinstant_make_free(value, temp1->temptype, t);
    }
    /* Infinity sentinel (empty or degenerate overlap): defer below */
  }

  /* Compute the temporal distance, it may be NULL if the points do not
   * intersect on time */
  Temporal *dist = tdistance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return NULL;

  const TInstant *min = temporal_min_inst_p(dist);
  TimestampTz t = min->t;
  pfree(dist);
  /* The closest point may be at an exclusive bound => 3rd argument = false */
  Datum value;
  temporal_value_at_timestamptz(temp1, t, false, &value);
  return tinstant_make_free(value, temp1->temptype, t);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between a temporal geo
 * and a geometry/geography
 * @param[in] temp Temporal geo
 * @param[in] gs Geometry/geography
 * @csqlfn #NAD_tgeo_geo()
 * @return On error return infinity
 */
double
nad_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, DBL_MAX); VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return DBL_MAX;

  /* Planar 2D temporal point: the GEOS-free analytic engine at radius 0,
   * avoiding the trajectory materialisation */
  if (tpoint_type(temp->temptype) && ! MEOS_FLAGS_GET_GEODETIC(temp->flags) &&
      ! MEOS_FLAGS_GET_Z(temp->flags))
  {
    double result;
    if (nad_tgeo_geo_analytic(temp, gs, &result))
      return result;
  }

  datum_func2 func = geo_distance_fn(temp->flags);
  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp, UNARY_UNION_NO) :
    tgeo_traversed_area(temp, UNARY_UNION_NO);
  double result = DatumGetFloat8(
    func(PointerGetDatum(traj), PointerGetDatum(gs)));
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry/geography
 * @param[in] box Spatiotemporal box/geography
 * @param[in] gs Geometry
 * @csqlfn #NAD_stbox_geo()
 * @return On error return infinity
 */
double
nad_stbox_geo(const STBox *box, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box, DBL_MAX); VALIDATE_NOT_NULL(gs, DBL_MAX);
  if (! ensure_valid_stbox_geo(box, gs) ||
      ! ensure_same_spatial_dimensionality_stbox_geo(box, gs))
    return DBL_MAX;

  datum_func2 func = geo_distance_fn(box->flags);
  Datum geo = PointerGetDatum(stbox_geo(box));
  double result = DatumGetFloat8(func(geo, PointerGetDatum(gs)));
  pfree(DatumGetPointer(geo));
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between two spatiotemporal
 * boxes
 * @param[in] box1,box2 Spatiotemporal boxes
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_stbox_stbox ()
 */
double
nad_stbox_stbox(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, DBL_MAX); VALIDATE_NOT_NULL(box2, DBL_MAX);
  if (! ensure_valid_spatial_stbox_stbox(box1, box2) ||
      ! ensure_same_spatial_dimensionality(box1->flags, box2->flags))
    return DBL_MAX;

  /* If the boxes do not intersect in the time dimension return infinity */
  bool hast = MEOS_FLAGS_GET_T(box1->flags) && MEOS_FLAGS_GET_T(box2->flags);
  if (hast && ! overlaps_span_span(&box1->period, &box2->period))
      return DBL_MAX;

  /* The nearest approach distance is the spatial-only distance between the
   * boxes (time already tested above) */
  return stbox_spatial_distance(box1, box2);
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between a temporal geo
 * and a spatiotemporal box
 * @param[in] temp Temporal geo
 * @param[in] box Spatiotemporal box
 * @return On error or if the time frames do not intersect return infinity
 * @csqlfn #NAD_tgeo_stbox()
 */
double
nad_tgeo_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, DBL_MAX);  VALIDATE_NOT_NULL(box, DBL_MAX);
  if (! ensure_valid_tgeo_stbox(temp, box) ||
      ! ensure_same_spatial_dimensionality(temp->flags, box->flags))
    return DBL_MAX;

  /* Project the temporal geo to the timespan of the box */
  bool hast = MEOS_FLAGS_GET_T(box->flags);
  Span p, inter;
  if (hast)
  {
    temporal_set_tstzspan(temp, &p);
    if (! inter_span_span(&p, &box->period, &inter))
      return DBL_MAX;
  }

  /* Select the distance function to be applied */
  datum_func2 func = geo_distance_fn(box->flags);
  /* Convert the stbox to a geometry */
  Datum geo = PointerGetDatum(stbox_geo(box));
  Temporal *temp1 = hast ?
    temporal_restrict_tstzspan(temp, &inter, REST_AT) :
    (Temporal *) temp;
  /* Compute the result */
  Datum traj = tpoint_type(temp->temptype) ? 
    PointerGetDatum(tpoint_trajectory(temp, UNARY_UNION_NO)) :
    PointerGetDatum(tgeo_traversed_area(temp, UNARY_UNION_NO));
  double result = DatumGetFloat8(func(traj, geo));

  pfree(DatumGetPointer(traj));
  pfree(DatumGetPointer(geo));
  if (hast)
    pfree(temp1);
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the nearest approach distance between two temporal geos
 * @param[in] temp1,temp2 Temporal geos
 * @csqlfn #NAD_tgeo_tgeo()
 * @return On error or if the time frames do not intersect return infinity
 */
double
nad_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, DBL_MAX); VALIDATE_TGEO(temp2, DBL_MAX);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags))
    return DBL_MAX;

  /* Fast path: linear temporal points via the time-synchronous running
   * minimum, avoiding the temporal distance materialization. A finite
   * result is exact; the infinity sentinel (empty or degenerate overlap)
   * defers to the temporal distance path */
  if (nad_tpoint_tpoint_sync_applies(temp1, temp2))
  {
    TimestampTz t;
    double d = nad_tpoint_tpoint_sync(temp1, temp2, &t);
    if (d != DBL_MAX)
      return d;
  }

  Temporal *dist = tdistance_tgeo_tgeo(temp1, temp2);
  if (dist == NULL)
    return DBL_MAX;

  double result = DatumGetFloat8(temporal_min_value(dist));
  pfree(dist);
  return result;
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @brief Return the point in first input geography that is closest to the
 * second input geography in 2D
 */
GSERIALIZED *
geography_shortestline_internal(const GSERIALIZED *gs1, const GSERIALIZED *gs2,
  bool use_spheroid)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));

  /* Return NULL on empty arguments. */
  if ( gserialized_is_empty(gs1) || gserialized_is_empty(gs2) )
    return NULL;

  /* Initialize spheroid */
  SPHEROID s;
  spheroid_init_from_srid(gserialized_get_srid(gs1), &s);
  
  /* Set to sphere if requested */
  if ( ! use_spheroid )
    s.a = s.b = s.radius;

  LWGEOM *geo1 = lwgeom_from_gserialized(gs1);
  LWGEOM *geo2 = lwgeom_from_gserialized(gs2);
  LWGEOM *line = geography_tree_shortestline(geo1, geo2, FP_TOLERANCE, &s);
  GSERIALIZED *result = geo_serialize(line);
  lwgeom_free(line); lwgeom_free(geo1); lwgeom_free(geo2);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the line connecting the nearest approach point between a
 * temporal geo and a geometry/geography
 * @param[in] temp Temporal value
 * @param[in] gs Geometry/geography
 * @csqlfn #Shortestline_tgeo_geo()
 */
GSERIALIZED *
shortestline_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp, NULL); VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_same_dimensionality_tspatial_geo(temp, gs) ||
      ! ensure_same_geodetic_tspatial_geo(temp, gs) ||
      gserialized_is_empty(gs))
    return NULL;
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(temp->flags);
  if (geodetic && ! ensure_has_not_Z_geo(gs))
    return NULL;

  /* Planar 2D temporal point: the GEOS-free analytic engine at radius 0 */
  if (! geodetic && tpoint_type(temp->temptype) &&
      ! MEOS_FLAGS_GET_Z(temp->flags))
  {
    GSERIALIZED *line = shortestline_tgeo_geo_analytic(temp, gs);
    if (line)
      return line;
  }

  GSERIALIZED *traj = tpoint_type(temp->temptype) ? 
    tpoint_trajectory(temp, UNARY_UNION_NO) :
    tgeo_traversed_area(temp, UNARY_UNION_NO);
  GSERIALIZED *result;
  if (geodetic)
    result = geography_shortestline_internal(traj, gs, true);
  else
  {
    result = MEOS_FLAGS_GET_Z(temp->flags) ?
      geom_shortestline3d(traj, gs) : geom_shortestline2d(traj, gs);
  }
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the line connecting the nearest approach point between two
 * temporal geos
 * @param[in] temp1,temp2 Temporal values
 * @csqlfn #Shortestline_tgeo_tgeo()
 */
GSERIALIZED *
shortestline_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TGEO(temp1, NULL); VALIDATE_TGEO(temp2, NULL);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return NULL;

  /* Fast path: linear temporal points via the time-synchronous running
   * minimum, avoiding the temporal distance materialization. A finite
   * result is exact; the infinity sentinel (empty or degenerate overlap)
   * defers to the temporal distance path */
  TimestampTz tmin;
  bool fast = false;
  if (nad_tpoint_tpoint_sync_applies(temp1, temp2))
    fast = nad_tpoint_tpoint_sync(temp1, temp2, &tmin) != DBL_MAX;

  Temporal *dist = NULL;
  if (! fast)
  {
    dist = tdistance_tgeo_tgeo(temp1, temp2);
    if (dist == NULL)
      return NULL;
    tmin = temporal_min_inst_p(dist)->t;
  }
  /* Timestamp t may be at an exclusive bound */
  Datum value1, value2;
  temporal_value_at_timestamptz(temp1, tmin, false, &value1);
  temporal_value_at_timestamptz(temp2, tmin, false, &value2);
  LWGEOM *line = (LWGEOM *) lwline_make(value1, value2);
  GSERIALIZED *result = geo_serialize(line);
  pfree(DatumGetPointer(value1)); pfree(DatumGetPointer(value2));
  if (dist)
    pfree(dist);
  lwgeom_free(line);
  return result;
}

/*****************************************************************************
 * Set-set spatial minimum distance
 *****************************************************************************/

/**
 * @ingroup meos_geo_distance
 * @brief Return the spatial-only minimum distance between two spatiotemporal
 * boxes, ignoring the time dimension entirely
 * @param[in] box1,box2 Spatiotemporal boxes
 */
double
stbox_spatial_distance(const STBox *box1, const STBox *box2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(box1, false); VALIDATE_NOT_NULL(box2, false);

  /* Spatial extents overlap → exact minimum is 0 (some pair of points
   * inside the joined extent has zero distance) */
  if (box1->xmin <= box2->xmax && box2->xmin <= box1->xmax &&
      box1->ymin <= box2->ymax && box2->ymin <= box1->ymax)
    return 0.0;

  /* Spatial extents disjoint → exact bbox-to-bbox euclidean distance.
   * Drop the time component of each box before serialising to geometry. */
  datum_func2 func = geo_distance_fn(box1->flags);
  STBox b1 = *box1, b2 = *box2;
  MEOS_FLAGS_SET_T(b1.flags, false);
  MEOS_FLAGS_SET_T(b2.flags, false);
  Datum g1 = PointerGetDatum(stbox_geo(&b1));
  Datum g2 = PointerGetDatum(stbox_geo(&b2));
  double result = DatumGetFloat8(func(g1, g2));
  pfree(DatumGetPointer(g1));
  pfree(DatumGetPointer(g2));
  return result;
}

/*****************************************************************************
 * Threshold-aware plane-sweep spatial-min kernel (Option B'')
 *
 * Walks two tgeompoint TSequence instant arrays directly via
 * `lw_dist2d_seg_seg`, no `tpoint_trajectory()` materialisation, no
 * `lw_dist2d_recursive` wrapper.  T2's segment bboxes are computed once
 * and sorted by minx; for each T1 segment we binary-search the candidate
 * window in T2 and walk it with the running threshold tightening as we
 * go.  Same answer as `geom_distance2d(trajectory(seq1), trajectory(seq2))`
 * for the 2D, planar, tgeompoint, linear-interpolation case.
 *****************************************************************************/

typedef struct
{
  int idx;
  float minx;
  float maxx;
  float miny;
  float maxy;
} SegBox;

static int
segbox_cmp_minx(const void *a, const void *b)
{
  float da = ((const SegBox *) a)->minx;
  float db = ((const SegBox *) b)->minx;
  return (da < db) ? -1 : (da > db ? 1 : 0);
}

static double
mindist_tpointseq_tpointseq_threshold(const TSequence *seq1,
  const TSequence *seq2, double threshold)
{
  DISTPTS dl;
  lw_dist2d_distpts_init(&dl, DIST_MIN);
  dl.distance = threshold;

  /* Both single-instant */
  if (seq1->count == 1 && seq2->count == 1)
  {
    const POINT2D *p = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, 0)));
    const POINT2D *q = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, 0)));
    lw_dist2d_pt_pt(p, q, &dl);
    return dl.distance;
  }
  if (seq1->count == 1)
  {
    const POINT2D *p = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, 0)));
    for (int j = 0; j < seq2->count - 1; j++)
    {
      const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
      const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1)));
      lw_dist2d_pt_seg(p, q1, q2, &dl);
      if (dl.distance == 0.0) return 0.0;
    }
    return dl.distance;
  }
  if (seq2->count == 1)
  {
    const POINT2D *q = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, 0)));
    for (int i = 0; i < seq1->count - 1; i++)
    {
      const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i)));
      const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i + 1)));
      lw_dist2d_pt_seg(q, p1, p2, &dl);
      if (dl.distance == 0.0) return 0.0;
    }
    return dl.distance;
  }
  /* Both segmented: plane-sweep. */
  int n2 = seq2->count - 1;
  SegBox *boxes2 = palloc(n2 * sizeof(SegBox));
  for (int j = 0; j < n2; j++)
  {
    const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
    const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1)));
    boxes2[j].idx = j;
    boxes2[j].minx = (float) fmin(q1->x, q2->x);
    boxes2[j].maxx = (float) fmax(q1->x, q2->x);
    boxes2[j].miny = (float) fmin(q1->y, q2->y);
    boxes2[j].maxy = (float) fmax(q1->y, q2->y);
  }
  qsort(boxes2, n2, sizeof(SegBox), segbox_cmp_minx);

  for (int i = 0; i < seq1->count - 1; i++)
  {
    const POINT2D *p1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i)));
    const POINT2D *p2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq1, i + 1)));
    double s1_minx = fmin(p1->x, p2->x), s1_maxx = fmax(p1->x, p2->x);
    double s1_miny = fmin(p1->y, p2->y), s1_maxy = fmax(p1->y, p2->y);
    double thresh = dl.distance;
    double hi_x = s1_maxx + thresh;
    double lo_x = s1_minx - thresh;
    /* Binary-search the first j with minx > hi_x (upper bound).  Walk
     * forward from j=0 to that bound; skip those whose maxx < lo_x. */
    int hi_idx;
    {
      int lo = 0, hi = n2;
      while (lo < hi)
      {
        int mid = (lo + hi) / 2;
        if (boxes2[mid].minx > hi_x) hi = mid;
        else lo = mid + 1;
      }
      hi_idx = lo;
    }
    for (int k = 0; k < hi_idx; k++)
    {
      if (boxes2[k].maxx < lo_x) continue;
      /* Y-axis prefilter */
      if (boxes2[k].maxy < s1_miny - dl.distance) continue;
      if (boxes2[k].miny > s1_maxy + dl.distance) continue;
      double dx = fmax(0.0, fmax(s1_minx, (double) boxes2[k].minx)
                          - fmin(s1_maxx, (double) boxes2[k].maxx));
      double dy = fmax(0.0, fmax(s1_miny, (double) boxes2[k].miny)
                          - fmin(s1_maxy, (double) boxes2[k].maxy));
      if (dx >= dl.distance || dy >= dl.distance) continue;
      int j = boxes2[k].idx;
      const POINT2D *q1 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j)));
      const POINT2D *q2 = DATUM_POINT2D_P(tinstant_value_p(TSEQUENCE_INST_N(seq2, j + 1)));
      lw_dist2d_seg_seg(p1, p2, q1, q2, &dl);
      if (dl.distance == 0.0) { pfree(boxes2); return 0.0; }
    }
  }
  pfree(boxes2);
  return dl.distance;
}

/* Dispatch across subtypes, threading the running threshold. */
static double
mindist_tpoint_tpoint_threshold(const Temporal *temp1, const Temporal *temp2,
  double threshold)
{
  if (temp1->subtype == TSEQUENCESET)
  {
    const TSequenceSet *ss1 = (const TSequenceSet *) temp1;
    for (int i = 0; i < ss1->count; i++)
    {
      double d = mindist_tpoint_tpoint_threshold(
        (const Temporal *) TSEQUENCESET_SEQ_N(ss1, i), temp2, threshold);
      if (d < threshold) threshold = d;
      if (threshold == 0.0) return 0.0;
    }
    return threshold;
  }
  if (temp2->subtype == TSEQUENCESET)
  {
    const TSequenceSet *ss2 = (const TSequenceSet *) temp2;
    for (int j = 0; j < ss2->count; j++)
    {
      double d = mindist_tpointseq_tpointseq_threshold(
        (const TSequence *) temp1, TSEQUENCESET_SEQ_N(ss2, j), threshold);
      if (d < threshold) threshold = d;
      if (threshold == 0.0) return 0.0;
    }
    return threshold;
  }
  if (temp1->subtype == TSEQUENCE && temp2->subtype == TSEQUENCE)
    return mindist_tpointseq_tpointseq_threshold(
      (const TSequence *) temp1, (const TSequence *) temp2, threshold);
  /* TInstant: caller falls back to lwgeom path. */
  return DBL_MAX;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the exact minimum spatial distance between two temporal
 * geos, ignoring time
 * @details Computes
 * `ST_Distance(trajectory(temp1), trajectory(temp2))` (the spatial-min
 * over all pairs of points on the two trajectories) using the
 * threshold-aware plane-sweep kernel directly on the `TInstant` arrays.
 * Distinct from `nearestApproachDistance(tgeompoint, tgeompoint)` which
 * is time-synchronous.
 * @param[in] temp1,temp2 Temporal geos (must share SRID, dimensionality,
 *   and geodetic flag)
 * @return Minimum spatial distance; falls back to `geom_distance2d` for
 *   subtype combinations the inline kernel does not handle (e.g.
 *   `TInstant`).
 * @csqlfn #Mindistance_tgeo_tgeo()
 */
double
mindistance_tgeo_tgeo(const Temporal *temp1, const Temporal *temp2,
  double threshold)
{
  VALIDATE_NOT_NULL(temp1, DBL_MAX);
  VALIDATE_NOT_NULL(temp2, DBL_MAX);
  VALIDATE_TGEO(temp1, DBL_MAX);
  VALIDATE_TGEO(temp2, DBL_MAX);
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)) ||
      ! ensure_same_dimensionality(temp1->flags, temp2->flags) ||
      ! ensure_same_geodetic(temp1->flags, temp2->flags))
    return DBL_MAX;
  /* Outer STBox spatial-distance prune: every point on either trajectory
   * lies within the respective STBox, so the spatial distance between the
   * boxes is a lower bound on the minimum distance; when it already meets or
   * exceeds the running threshold the kernels below cannot improve on it, so
   * short-circuit. Not valid for geodetic inputs (the planar bbox distance is
   * not a lower bound); TINSTANT inputs have no precomputed bbox
   * (temporal_bbox_ptr returns NULL) so they are skipped and handled below.
   * Critical for cross-join aggregations where most pairs are far apart. */
  if (! MEOS_FLAGS_GET_GEODETIC(temp1->flags) &&
      temp1->subtype != TINSTANT && temp2->subtype != TINSTANT)
  {
    const STBox *bbox1 = (const STBox *) temporal_bbox_ptr(temp1);
    const STBox *bbox2 = (const STBox *) temporal_bbox_ptr(temp2);
    if (stbox_spatial_distance(bbox1, bbox2) >= threshold)
      return threshold;
  }
  bool inline_eligible =
    ! MEOS_FLAGS_GET_Z(temp1->flags) &&
    ! MEOS_FLAGS_GET_GEODETIC(temp1->flags) &&
    MEOS_FLAGS_LINEAR_INTERP(temp1->flags) &&
    tpoint_type(temp1->temptype) &&
    tpoint_type(temp2->temptype) &&
    temp1->subtype != TINSTANT && temp2->subtype != TINSTANT;
  if (inline_eligible)
    return mindist_tpoint_tpoint_threshold(temp1, temp2, threshold);
  /* Fallback */
  GSERIALIZED *traj1 = tpoint_type(temp1->temptype) ?
    tpoint_trajectory(temp1, UNARY_UNION_NO) :
    tgeo_traversed_area(temp1, UNARY_UNION_NO);
  GSERIALIZED *traj2 = tpoint_type(temp2->temptype) ?
    tpoint_trajectory(temp2, UNARY_UNION_NO) :
    tgeo_traversed_area(temp2, UNARY_UNION_NO);
  double d = MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    geog_distance(traj1, traj2) :
    (MEOS_FLAGS_GET_Z(temp1->flags) ?
      geom_distance3d(traj1, traj2) :
      geom_distance2d(traj1, traj2));
  pfree(traj1); pfree(traj2);
  return (d < threshold) ? d : threshold;
}

/* qsort comparator: pair record ordered by bbox-distance ascending. */
typedef struct
{
  int i;
  int j;
  double bd;
} TgeoarrPair;

static int
tgeoarr_pair_cmp(const void *a, const void *b)
{
  double da = ((const TgeoarrPair *) a)->bd;
  double db = ((const TgeoarrPair *) b)->bd;
  if (da < db) return -1;
  if (da > db) return 1;
  return 0;
}

/**
 * @ingroup meos_geo_distance
 * @brief Return the exact minimum spatial distance between two arrays of
 * temporal geos
 * @details Computes the same value as
 * `ST_Distance(ST_Collect(trajectory(arr1[*])), ST_Collect(trajectory(arr2[*])))`
 * but uses each input's STBox as a sound lower-bound prefilter: trip pairs
 * are processed in ascending bbox-distance order, and the iteration short
 * circuits once the running minimum is provably smaller than every
 * remaining pair's lower bound.  The per-pair distance still goes through
 * `geom_distance2d` (liblwgeom segment-pair sweep, no GEOS call), so the
 * result is bit-equivalent to the materialised aggregate form.
 * @param[in] arr1,arr2 Arrays of temporal geos (each element must be
 *   non-NULL and share SRID / dimensionality with the rest)
 * @param[in] count1,count2 Array lengths (must be > 0)
 * @return Minimum spatial distance; DBL_MAX on validation failure or on
 *   any empty input array.
 * @csqlfn #Mindistance_tgeoarr_tgeoarr()
 */
double
mindistance_tgeoarr_tgeoarr(const Temporal **arr1, int count1,
  const Temporal **arr2, int count2)
{
  VALIDATE_NOT_NULL(arr1, DBL_MAX);
  VALIDATE_NOT_NULL(arr2, DBL_MAX);
  if (count1 <= 0 || count2 <= 0)
    return DBL_MAX;
  for (int i = 0; i < count1; i++)
    VALIDATE_TGEO(arr1[i], DBL_MAX);
  for (int j = 0; j < count2; j++)
    VALIDATE_TGEO(arr2[j], DBL_MAX);

  /* Soundness gate: all inputs share SRID, dimensionality, geodetic flag */
  int32 srid = tspatial_srid(arr1[0]);
  int16 flags = arr1[0]->flags;
  for (int i = 1; i < count1; i++)
    if (! ensure_same_srid(tspatial_srid(arr1[i]), srid) ||
        ! ensure_same_dimensionality(arr1[i]->flags, flags) ||
        ! ensure_same_geodetic(arr1[i]->flags, flags))
      return DBL_MAX;
  for (int j = 0; j < count2; j++)
    if (! ensure_same_srid(tspatial_srid(arr2[j]), srid) ||
        ! ensure_same_dimensionality(arr2[j]->flags, flags) ||
        ! ensure_same_geodetic(arr2[j]->flags, flags))
      return DBL_MAX;

  /* Pre-compute STBoxes for every input.  Each tspatial_to_stbox is a
   * cheap aggregate over the temporal value, amortised across all pairs
   * involving that input. */
  STBox *bb1 = palloc(count1 * sizeof(STBox));
  STBox *bb2 = palloc(count2 * sizeof(STBox));
  for (int i = 0; i < count1; i++) tspatial_set_stbox(arr1[i], &bb1[i]);
  for (int j = 0; j < count2; j++) tspatial_set_stbox(arr2[j], &bb2[j]);

  /* Materialise all candidate pairs with their bbox-distance lower bound */
  int npairs = count1 * count2;
  TgeoarrPair *pairs = palloc(npairs * sizeof(TgeoarrPair));
  int k = 0;
  for (int i = 0; i < count1; i++)
    for (int j = 0; j < count2; j++)
    {
      pairs[k].i = i;
      pairs[k].j = j;
      /* The planar bbox distance is not a lower bound on the geodetic
       * distance, so for geodetic inputs every pair gets a zero lower bound,
       * disabling the ordering short-circuit so that all pairs are tested */
      pairs[k].bd = MEOS_FLAGS_GET_GEODETIC(arr1[0]->flags) ? 0.0 :
        stbox_spatial_distance(&bb1[i], &bb2[j]);
      k++;
    }
  qsort(pairs, npairs, sizeof(TgeoarrPair), tgeoarr_pair_cmp);

  /* Trajectory cache: each input's trajectory is materialised at most
   * once and reused across every pair it participates in. */
  GSERIALIZED **traj1 = palloc0(count1 * sizeof(GSERIALIZED *));
  GSERIALIZED **traj2 = palloc0(count2 * sizeof(GSERIALIZED *));

  /* Eligibility for the inline tgeompoint kernel: 2D, planar, all inputs
   * are tgeompoint with linear interpolation, and at least one of seq /
   * seqset on each side.  TInstant inputs fall through to the lwgeom
   * path. */
  bool inline_eligible =
    ! MEOS_FLAGS_GET_Z(flags) &&
    ! MEOS_FLAGS_GET_GEODETIC(flags) &&
    MEOS_FLAGS_LINEAR_INTERP(flags);
  if (inline_eligible)
  {
    for (int i = 0; i < count1; i++)
      if (! tpoint_type(arr1[i]->temptype) ||
          arr1[i]->subtype == TINSTANT)
      { inline_eligible = false; break; }
  }
  if (inline_eligible)
  {
    for (int j = 0; j < count2; j++)
      if (! tpoint_type(arr2[j]->temptype) ||
          arr2[j]->subtype == TINSTANT)
      { inline_eligible = false; break; }
  }

  double running_min = DBL_MAX;
  for (k = 0; k < npairs; k++)
  {
    /* All remaining pairs have bbox-distance >= pairs[k].bd, which is
     * a sound lower bound on the trajectory pair's actual distance.
     * Once it crosses running_min, no remaining pair can improve it. */
    if (pairs[k].bd >= running_min)
      break;
    int i = pairs[k].i, j = pairs[k].j;
    double d;
    if (inline_eligible)
    {
      d = mindist_tpoint_tpoint_threshold(arr1[i], arr2[j], running_min);
    }
    else
    {
      if (traj1[i] == NULL)
        traj1[i] = tpoint_type(arr1[i]->temptype) ?
          tpoint_trajectory(arr1[i], UNARY_UNION_NO) :
          tgeo_traversed_area(arr1[i], UNARY_UNION_NO);
      if (traj2[j] == NULL)
        traj2[j] = tpoint_type(arr2[j]->temptype) ?
          tpoint_trajectory(arr2[j], UNARY_UNION_NO) :
          tgeo_traversed_area(arr2[j], UNARY_UNION_NO);
      d = MEOS_FLAGS_GET_GEODETIC(flags) ?
        geog_distance(traj1[i], traj2[j]) :
        (MEOS_FLAGS_GET_Z(flags) ?
          geom_distance3d(traj1[i], traj2[j]) :
          geom_distance2d(traj1[i], traj2[j]));
    }
    if (d < running_min)
      running_min = d;
    if (running_min == 0.0)
      break;
  }

  /* Cleanup */
  for (int i = 0; i < count1; i++)
  {
    if (traj1[i] != NULL) pfree(traj1[i]);
  }
  for (int j = 0; j < count2; j++)
  {
    if (traj2[j] != NULL) pfree(traj2[j]);
  }
  pfree(bb1); pfree(bb2); pfree(traj1); pfree(traj2); pfree(pairs);
  return running_min;
}

/*****************************************************************************/
