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
 * @brief Swept-edge-polygon clip primitive (M1 + M2)
 * @details Computes time intervals during which a moving polygon edge
 * intersects a static polygon. The swept envelope of the edge between
 * two timestamps is:
 *   - M1 (pure translation): a parallelogram; P(u, t) is affine in
 *     both u and t. Boundary residuals are linear → closed form.
 *   - M2 (rotation, 2D only): a curved quadrilateral whose two
 *     "lateral" sides are arcs; P(u, t) is non-linear in t. Boundary
 *     residuals are `a + b*t + c*cos(theta(t)) + d*sin(theta(t))`.
 *     Two solver paths: closed-form Taylor for |Delta theta| < 0.10
 *     rad, and 20-bin sign-change scan + bracket bisection otherwise.
 *
 * Note: "trapezoid" in this codebase refers to the shape swept by a
 * tcbuffer segment (two circle arcs + two tangent lines), implemented
 * in trapezoid_make(). The trgeo swept quadrilateral is a distinct
 * shape — do not conflate the two.
 *
 * 3D pose input is rejected: trgeometry surfaces operate in 2D
 * (PostGIS curve arithmetic bottoms out in 2D).
 *
 * Geographic input is approximated as Euclidean — same status as
 * tpoint_geom_clip.c. Fixing geographic correctly (great-circle
 * trajectories, spherical point-in-polygon) is a separate work item.
 *
 * See `doc/rfc/trgeo_geom_clip_design.md` (M1) and
 * `doc/rfc/trgeo_geom_clip_design_m2.md` (M2) for derivations.
 */

#include "rgeo/trgeo_geom_clip.h"

/* C */
#include <math.h>
#include <stdlib.h>
/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/span.h"
#include "temporal/meos_catalog.h"
#include "pose/pose.h"

/*****************************************************************************
 * Tunables
 *****************************************************************************/

/* Floating-point tolerance — matches the convention in tpoint_geom_clip.c */
#ifndef FP_TOLERANCE
#define FP_TOLERANCE 1e-12
#endif

/* M2: below this absolute angular delta (radians) we use a Taylor
 * closed-form solver; above it we fall back to numerical bisection. */
#ifndef TRGEO_GEOM_CLIP_TAYLOR_THRESHOLD
#define TRGEO_GEOM_CLIP_TAYLOR_THRESHOLD  0.10
#endif

/* M2: number of equally-spaced bins for the sign-change scan. */
#ifndef TRGEO_GEOM_CLIP_BISECT_BINS
#define TRGEO_GEOM_CLIP_BISECT_BINS  20
#endif

/* M2: maximum bracket-bisection iterations per sign-change interval. */
#ifndef TRGEO_GEOM_CLIP_BISECT_ITERS
#define TRGEO_GEOM_CLIP_BISECT_ITERS  30
#endif

/*****************************************************************************
 * Common helpers
 *****************************************************************************/

static int
double_cmp(const void *a, const void *b)
{
  double da = *(const double *)a, db = *(const double *)b;
  if (da < db) return -1;
  if (da > db) return 1;
  return 0;
}

/**
 * @brief Append a critical t to an events buffer if it lies in [0, 1].
 * Tolerance-aware: out-of-range values within FP_TOLERANCE are clamped.
 */
static inline void
push_event(double t, double *events, int *nevents, int cap)
{
  if (t < -FP_TOLERANCE || t > 1.0 + FP_TOLERANCE)
    return;
  if (t < 0.0) t = 0.0;
  if (t > 1.0) t = 1.0;
  if (*nevents < cap)
    events[(*nevents)++] = t;
}

/**
 * @brief Test whether segments (sa, sb) and (pa, pb) intersect.
 * Returns true on any non-empty intersection (point or overlap).
 */
static bool
segments_intersect(double sax, double say, double sbx, double sby,
  double pax, double pay, double pbx, double pby)
{
  double rx = sbx - sax, ry = sby - say;
  double sx = pbx - pax, sy = pby - pay;
  double rxs = rx * sy - ry * sx;
  double qpx = pax - sax, qpy = pay - say;
  if (fabs(rxs) < FP_TOLERANCE)
  {
    double qpxr = qpx * ry - qpy * rx;
    if (fabs(qpxr) > FP_TOLERANCE) return false;
    double r2 = rx * rx + ry * ry;
    if (r2 < FP_TOLERANCE) return false;
    double t0 = (qpx * rx + qpy * ry) / r2;
    double t1 = t0 + (sx * rx + sy * ry) / r2;
    if (t0 > t1) { double tmp = t0; t0 = t1; t1 = tmp; }
    return (t1 >= -FP_TOLERANCE && t0 <= 1.0 + FP_TOLERANCE);
  }
  double t = (qpx * sy - qpy * sx) / rxs;
  double u = (qpx * ry - qpy * rx) / rxs;
  return (t >= -FP_TOLERANCE && t <= 1.0 + FP_TOLERANCE &&
          u >= -FP_TOLERANCE && u <= 1.0 + FP_TOLERANCE);
}

/**
 * @brief Ray-cast point-in-polygon test against a closed POINTARRAY.
 * Returns 1 if (x, y) is strictly inside, 0 if outside or on boundary.
 * Holes are not considered (exterior ring only).
 */
static int
point_in_ring(double x, double y, const POINTARRAY *pa)
{
  if (! pa || pa->npoints < 3)
    return 0;
  int inside = 0;
  for (uint32_t i = 0, j = pa->npoints - 1; i < pa->npoints; j = i++)
  {
    const POINT2D *pi = getPoint2d_cp(pa, i);
    const POINT2D *pj = getPoint2d_cp(pa, j);
    if (((pi->y > y) != (pj->y > y)) &&
        (x < (pj->x - pi->x) * (y - pi->y) /
                (pj->y - pi->y) + pi->x))
      inside = !inside;
  }
  return inside;
}

/**
 * @brief Test whether a fixed (already-computed-in-world-space) line
 * segment intersects the polygon described by `pa`.
 *
 * Caller passes the moving edge's world-space endpoints at the test
 * time. Both M1 (linear translation) and M2 (rotational pose) use this
 * helper after computing their respective endpoints; the difference
 * between the two regimes lives in the endpoint computation, not here.
 */
static bool
segment_intersects_polygon(double sax, double say, double sbx, double sby,
  const POINTARRAY *pa)
{
  if (point_in_ring(sax, say, pa) || point_in_ring(sbx, sby, pa))
    return true;
  for (uint32_t i = 0, j = pa->npoints - 1; i < pa->npoints; j = i++)
  {
    const POINT2D *pi = getPoint2d_cp(pa, i);
    const POINT2D *pj = getPoint2d_cp(pa, j);
    if (segments_intersect(sax, say, sbx, sby, pj->x, pj->y, pi->x, pi->y))
      return true;
  }
  return false;
}

/**
 * @brief Drop near-duplicate values within FP_TOLERANCE.
 * Operates in place on the sorted `events` array.
 */
static int
dedup_sorted(double *events, int n)
{
  int nuniq = 0;
  for (int k = 0; k < n; k++)
  {
    if (nuniq == 0 || fabs(events[k] - events[nuniq - 1]) > FP_TOLERANCE)
      events[nuniq++] = events[k];
  }
  return nuniq;
}

/**
 * @brief Walk sorted events, midpoint-test each gap; emit merged
 * intervals to `out`.
 *
 * The midpoint-test predicate `intersects(t_m, ctx)` is provided by
 * the caller — it computes the moving edge's world-space endpoints at
 * `t_m` (M1: linear; M2: rotational) and forwards to
 * `segment_intersects_polygon`.
 *
 * @return number of merged intervals emitted.
 */
static int
walk_events_and_emit(double *events, int nuniq,
  bool (*intersects)(double t_m, void *ctx), void *ctx,
  Span *out)
{
  int nout = 0;
  for (int k = 0; k < nuniq - 1; k++)
  {
    double t_a = events[k];
    double t_b = events[k + 1];
    if (t_b - t_a <= FP_TOLERANCE)
      continue;
    double t_m = 0.5 * (t_a + t_b);
    if (intersects(t_m, ctx))
    {
      if (nout > 0 &&
          fabs(DatumGetFloat8(out[nout - 1].upper) - t_a) <= FP_TOLERANCE)
        out[nout - 1].upper = Float8GetDatum(t_b);
      else
      {
        span_set(Float8GetDatum(t_a), Float8GetDatum(t_b),
          true, true, T_FLOAT8, T_FLOATSPAN, &out[nout]);
        nout++;
      }
    }
  }
  return nout;
}

/*****************************************************************************
 * M1 — pure-translation kernel
 *
 * The moving edge under pure translation traces a parallelogram. Each
 * polygon edge yields up to 4 critical t values via the boundary cases
 * (u in {0, 1}, s in {0, 1}). All boundary residuals are linear in t,
 * so each case has a closed-form root.
 *****************************************************************************/

/**
 * @brief Solve "moving endpoint a + t*delta lies on polygon edge p1 -> p2".
 *
 * 2x2 linear system in (s, t). Returns the t-coordinate (modulo
 * tolerance) if a valid intersection exists with s in [0, 1].
 */
static bool
solve_m1_endpoint_on_edge(double ax, double ay, double dxd, double dyd,
  double px1, double py1, double px2, double py2,
  double *t_out)
{
  double ex = px2 - px1;
  double ey = py2 - py1;
  double det = -ex * dyd + dxd * ey;
  if (fabs(det) < FP_TOLERANCE)
    return false;
  double rhs_x = px1 - ax;
  double rhs_y = py1 - ay;
  double s = (rhs_x * dyd - dxd * rhs_y) / det;
  double t = (-ex * rhs_y + rhs_x * ey) / det;
  if (s < -FP_TOLERANCE || s > 1.0 + FP_TOLERANCE)
    return false;
  *t_out = t;
  return true;
}

/**
 * @brief Solve "polygon edge endpoint p lies on the moving edge at time t,
 * at parameter u in [0, 1]".
 */
static bool
solve_m1_p_on_movingedge(double px, double py,
  double a1x, double a1y, double b1x, double b1y,
  double dxd, double dyd, double *t_out)
{
  double ux = b1x - a1x;
  double uy = b1y - a1y;
  double det = ux * dyd - dxd * uy;
  if (fabs(det) < FP_TOLERANCE)
    return false;
  double rhs_x = px - a1x;
  double rhs_y = py - a1y;
  double u = (rhs_x * dyd - dxd * rhs_y) / det;
  double t = (ux * rhs_y - rhs_x * uy) / det;
  if (u < -FP_TOLERANCE || u > 1.0 + FP_TOLERANCE)
    return false;
  *t_out = t;
  return true;
}

/* M1 midpoint-test context: linear translation. */
typedef struct {
  double a1x, a1y, b1x, b1y, dxd, dyd;
  const POINTARRAY *pa;
} M1Ctx;

static bool
m1_intersects_at(double t_m, void *ctx_)
{
  M1Ctx *ctx = (M1Ctx *) ctx_;
  double sax = ctx->a1x + t_m * ctx->dxd;
  double say = ctx->a1y + t_m * ctx->dyd;
  double sbx = ctx->b1x + t_m * ctx->dxd;
  double sby = ctx->b1y + t_m * ctx->dyd;
  return segment_intersects_polygon(sax, say, sbx, sby, ctx->pa);
}

int
trgeo_geom_clip_polygon(const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const POINTARRAY *pa, Span **intervals_out)
{
  if (! a1 || ! b1 || ! a2 || ! b2 || ! pa || ! intervals_out)
    return -1;
  if (pa->npoints < 4)
    return -1;

  double dxd_a = a2->x - a1->x;
  double dyd_a = a2->y - a1->y;
  double dxd_b = b2->x - b1->x;
  double dyd_b = b2->y - b1->y;
  if (fabs(dxd_a - dxd_b) > FP_TOLERANCE ||
      fabs(dyd_a - dyd_b) > FP_TOLERANCE)
    return -1;
  double dxd = dxd_a, dyd = dyd_a;

  uint32_t nedges = pa->npoints - 1;
  int cap = (int) (4 * nedges + 2);
  double *events = palloc(sizeof(double) * (size_t) cap);
  int nevents = 0;
  push_event(0.0, events, &nevents, cap);
  push_event(1.0, events, &nevents, cap);

  for (uint32_t i = 0, j = pa->npoints - 1; i < pa->npoints; j = i++)
  {
    const POINT2D *pi = getPoint2d_cp(pa, i);
    const POINT2D *pj = getPoint2d_cp(pa, j);
    double t;
    if (solve_m1_endpoint_on_edge(a1->x, a1->y, dxd, dyd,
        pj->x, pj->y, pi->x, pi->y, &t))
      push_event(t, events, &nevents, cap);
    if (solve_m1_endpoint_on_edge(b1->x, b1->y, dxd, dyd,
        pj->x, pj->y, pi->x, pi->y, &t))
      push_event(t, events, &nevents, cap);
    if (solve_m1_p_on_movingedge(pj->x, pj->y, a1->x, a1->y, b1->x, b1->y,
        dxd, dyd, &t))
      push_event(t, events, &nevents, cap);
    if (solve_m1_p_on_movingedge(pi->x, pi->y, a1->x, a1->y, b1->x, b1->y,
        dxd, dyd, &t))
      push_event(t, events, &nevents, cap);
  }

  qsort(events, (size_t) nevents, sizeof(double), double_cmp);
  int nuniq = dedup_sorted(events, nevents);

  Span *out = palloc(sizeof(Span) * (size_t) (nuniq + 1));
  M1Ctx ctx = {a1->x, a1->y, b1->x, b1->y, dxd, dyd, pa};
  int nout = walk_events_and_emit(events, nuniq, m1_intersects_at, &ctx, out);

  pfree(events);
  if (nout == 0)
  {
    pfree(out);
    *intervals_out = NULL;
    return 0;
  }
  *intervals_out = out;
  return nout;
}

int
trgeo_geom_clip_lwpoly(const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const LWPOLY *poly, Span **intervals_out)
{
  if (! poly || poly->nrings < 1)
    return -1;
  return trgeo_geom_clip_polygon(a1, b1, a2, b2, poly->rings[0],
    intervals_out);
}

int
trgeo_geom_clip_box(const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  double xmin, double ymin, double xmax, double ymax,
  Span **intervals_out)
{
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 5);
  POINT4D pt;
  pt.z = 0; pt.m = 0;
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  int n = trgeo_geom_clip_polygon(a1, b1, a2, b2, pa, intervals_out);
  ptarray_free(pa);
  return n;
}

/*****************************************************************************
 * M2 — rotational kernel (2D only)
 *
 * P(u, t) = pos(t) + R(theta(t)) * ((1-u)*p_a + u*p_b),
 * where pos(t) = lerp(pos1, pos2; t) and theta(t) = lerp(theta1, theta2; t).
 *
 * Boundary residuals are sums of linear and trigonometric terms in t.
 * Two solver paths:
 *   - Closed-form Taylor (|Delta theta| < TAYLOR_THRESHOLD).
 *   - 20-bin sign-change scan + bracket bisection.
 *
 * Pure-translation input (theta1 == theta2 within tolerance) delegates
 * to the M1 fast path.
 *****************************************************************************/

/**
 * @brief Read 2D pose components into local doubles. Returns false on
 * 3D pose input (M2 is 2D only — same constraint as cbuffer's
 * 2D-only design).
 */
static bool
pose2d_read(const Pose *pose, double *x, double *y, double *theta)
{
  if (! pose || MEOS_FLAGS_GET_Z(pose->flags))
    return false;
  *x = pose->data[0];
  *y = pose->data[1];
  *theta = pose->data[2];
  return true;
}

/**
 * @brief World-space endpoint of a body-local point under the
 * interpolated pose at time t.
 */
static inline void
posed_endpoint_at(double t, double x1, double y1, double th1,
  double x2, double y2, double th2,
  double px, double py,
  double *out_x, double *out_y)
{
  double cx = x1 + t * (x2 - x1);
  double cy = y1 + t * (y2 - y1);
  double theta = th1 + t * (th2 - th1);
  double c = cos(theta), s = sin(theta);
  *out_x = cx + px * c - py * s;
  *out_y = cy + px * s + py * c;
}

/**
 * @brief Boundary residual for the u=0 / u=1 cases.
 *
 * "moving body's local point traced through pose interpolation lies
 * on the static polygon edge from p1 to p2".
 */
static double
m2_residual_endpoint_on_edge(double t,
  double x1, double y1, double th1,
  double x2, double y2, double th2,
  double px_local, double py_local,
  double e1x, double e1y, double e2x, double e2y)
{
  double ex, ey;
  posed_endpoint_at(t, x1, y1, th1, x2, y2, th2,
    px_local, py_local, &ex, &ey);
  double rx = e2x - e1x, ry = e2y - e1y;
  return (ex - e1x) * ry - (ey - e1y) * rx;
}

/**
 * @brief Boundary residual for the s=0 / s=1 cases.
 *
 * "polygon edge endpoint p_e lies on the moving edge (between
 * the body's two body-local endpoint trajectories) at time t".
 */
static double
m2_residual_polypoint_on_movingedge(double t,
  double x1, double y1, double th1,
  double x2, double y2, double th2,
  double pa_x, double pa_y, double pb_x, double pb_y,
  double pe_x, double pe_y)
{
  double ax, ay, bx, by;
  posed_endpoint_at(t, x1, y1, th1, x2, y2, th2, pa_x, pa_y, &ax, &ay);
  posed_endpoint_at(t, x1, y1, th1, x2, y2, th2, pb_x, pb_y, &bx, &by);
  double rx = bx - ax, ry = by - ay;
  return rx * (pe_y - ay) - ry * (pe_x - ax);
}

typedef double (*residual_fn)(double t, void *state);

typedef struct {
  double x1, y1, th1, x2, y2, th2;
  double px_local, py_local;
  double e1x, e1y, e2x, e2y;
} EndpointEdgeState;

typedef struct {
  double x1, y1, th1, x2, y2, th2;
  double pa_x, pa_y, pb_x, pb_y;
  double pe_x, pe_y;
} PolypointMovingEdgeState;

static double
residual_endpoint_on_edge_wrap(double t, void *state)
{
  EndpointEdgeState *s = (EndpointEdgeState *) state;
  return m2_residual_endpoint_on_edge(t, s->x1, s->y1, s->th1,
    s->x2, s->y2, s->th2, s->px_local, s->py_local,
    s->e1x, s->e1y, s->e2x, s->e2y);
}

static double
residual_polypoint_on_movingedge_wrap(double t, void *state)
{
  PolypointMovingEdgeState *s = (PolypointMovingEdgeState *) state;
  return m2_residual_polypoint_on_movingedge(t, s->x1, s->y1, s->th1,
    s->x2, s->y2, s->th2, s->pa_x, s->pa_y, s->pb_x, s->pb_y,
    s->pe_x, s->pe_y);
}

/**
 * @brief Sign-change scan + bracket bisection in [0, 1] for the
 * non-Taylor regime.
 */
static int
solve_m2_numerical(residual_fn f, void *state, double *roots, int max_roots)
{
  int n = 0;
  double prev_t = 0.0;
  double prev_v = f(0.0, state);
  for (int i = 1; i <= TRGEO_GEOM_CLIP_BISECT_BINS && n < max_roots; i++)
  {
    double cur_t = (double) i / TRGEO_GEOM_CLIP_BISECT_BINS;
    double cur_v = f(cur_t, state);
    if (fabs(cur_v) < FP_TOLERANCE)
    {
      roots[n++] = cur_t;
      prev_t = cur_t;
      prev_v = cur_v;
      continue;
    }
    if ((prev_v < 0 && cur_v > 0) || (prev_v > 0 && cur_v < 0))
    {
      double a = prev_t, b = cur_t;
      double fa = prev_v;
      for (int k = 0; k < TRGEO_GEOM_CLIP_BISECT_ITERS; k++)
      {
        double m = 0.5 * (a + b);
        double fm = f(m, state);
        if (fabs(fm) < FP_TOLERANCE) { a = b = m; break; }
        if ((fa < 0 && fm > 0) || (fa > 0 && fm < 0))
          b = m;
        else { a = m; fa = fm; }
      }
      roots[n++] = 0.5 * (a + b);
    }
    prev_t = cur_t;
    prev_v = cur_v;
  }
  return n;
}

/**
 * @brief Closed-form Taylor solver for the u=0 / u=1 case.
 *
 * Linearises cos(theta(t)) and sin(theta(t)) around theta_1, valid for
 * |Delta theta| < TAYLOR_THRESHOLD. Yields a residual linear in t →
 * single closed-form root.
 *
 * @return 1 if a root in [0, 1] exists, else 0.
 */
static int
solve_m2_taylor_endpoint_on_edge(EndpointEdgeState *s, double *root)
{
  double dth = s->th2 - s->th1;
  double dx = s->x2 - s->x1, dy = s->y2 - s->y1;
  double c1 = cos(s->th1), si1 = sin(s->th1);
  double rx_const = s->px_local * c1 - s->py_local * si1;
  double ry_const = s->px_local * si1 + s->py_local * c1;
  double ex0 = s->x1 + rx_const;
  double ey0 = s->y1 + ry_const;
  double ex_slope = dx - dth * ry_const;
  double ey_slope = dy + dth * rx_const;
  double erx = s->e2x - s->e1x;
  double ery = s->e2y - s->e1y;
  double a0 = (ex0 - s->e1x) * ery - (ey0 - s->e1y) * erx;
  double a1 = ex_slope * ery - ey_slope * erx;
  if (fabs(a1) < FP_TOLERANCE)
    return 0;
  double t = -a0 / a1;
  if (t < -FP_TOLERANCE || t > 1.0 + FP_TOLERANCE)
    return 0;
  if (t < 0.0) t = 0.0;
  if (t > 1.0) t = 1.0;
  *root = t;
  return 1;
}

/* M2 midpoint-test context: full pose interpolation. */
typedef struct {
  double x1, y1, th1, x2, y2, th2;
  double pa_x, pa_y, pb_x, pb_y;
  const POINTARRAY *pa_ring;
} M2Ctx;

static bool
m2_intersects_at(double t_m, void *ctx_)
{
  M2Ctx *ctx = (M2Ctx *) ctx_;
  double sax, say, sbx, sby;
  posed_endpoint_at(t_m, ctx->x1, ctx->y1, ctx->th1,
    ctx->x2, ctx->y2, ctx->th2, ctx->pa_x, ctx->pa_y, &sax, &say);
  posed_endpoint_at(t_m, ctx->x1, ctx->y1, ctx->th1,
    ctx->x2, ctx->y2, ctx->th2, ctx->pb_x, ctx->pb_y, &sbx, &sby);
  return segment_intersects_polygon(sax, say, sbx, sby, ctx->pa_ring);
}

int
trgeo_geom_clip_polygon_posed(const POINT2D *p_a_local,
  const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const POINTARRAY *pa, Span **intervals_out)
{
  if (! p_a_local || ! p_b_local || ! pose1 || ! pose2 || ! pa ||
      ! intervals_out || pa->npoints < 4)
    return -1;

  double x1, y1, th1, x2, y2, th2;
  if (! pose2d_read(pose1, &x1, &y1, &th1) ||
      ! pose2d_read(pose2, &x2, &y2, &th2))
    return -1;

  /* Pure-translation fast path: delegate to M1. */
  if (fabs(th2 - th1) < FP_TOLERANCE)
  {
    POINT2D a1, b1, a2, b2;
    double c1 = cos(th1), si1 = sin(th1);
    a1.x = x1 + p_a_local->x * c1 - p_a_local->y * si1;
    a1.y = y1 + p_a_local->x * si1 + p_a_local->y * c1;
    b1.x = x1 + p_b_local->x * c1 - p_b_local->y * si1;
    b1.y = y1 + p_b_local->x * si1 + p_b_local->y * c1;
    a2.x = x2 + p_a_local->x * c1 - p_a_local->y * si1;
    a2.y = y2 + p_a_local->x * si1 + p_a_local->y * c1;
    b2.x = x2 + p_b_local->x * c1 - p_b_local->y * si1;
    b2.y = y2 + p_b_local->x * si1 + p_b_local->y * c1;
    return trgeo_geom_clip_polygon(&a1, &b1, &a2, &b2, pa, intervals_out);
  }

  double dth = th2 - th1;
  bool use_taylor = fabs(dth) < TRGEO_GEOM_CLIP_TAYLOR_THRESHOLD;

  uint32_t nedges = pa->npoints - 1;
  /* Up to 4 numerical roots × 4 boundary cases per polygon edge. */
  int cap = (int) (16 * nedges + 2);
  double *events = palloc(sizeof(double) * (size_t) cap);
  int nevents = 0;
  push_event(0.0, events, &nevents, cap);
  push_event(1.0, events, &nevents, cap);

  for (uint32_t i = 0, j = pa->npoints - 1; i < pa->npoints; j = i++)
  {
    const POINT2D *pi = getPoint2d_cp(pa, i);
    const POINT2D *pj = getPoint2d_cp(pa, j);

    /* u=0: a-endpoint trajectory crosses edge (pj, pi) */
    {
      EndpointEdgeState s = {x1, y1, th1, x2, y2, th2,
        p_a_local->x, p_a_local->y, pj->x, pj->y, pi->x, pi->y};
      if (use_taylor)
      {
        double r;
        if (solve_m2_taylor_endpoint_on_edge(&s, &r))
          push_event(r, events, &nevents, cap);
      }
      else
      {
        double roots[4];
        int nr = solve_m2_numerical(residual_endpoint_on_edge_wrap, &s,
          roots, 4);
        for (int k = 0; k < nr; k++)
          push_event(roots[k], events, &nevents, cap);
      }
    }
    /* u=1: b-endpoint trajectory crosses edge */
    {
      EndpointEdgeState s = {x1, y1, th1, x2, y2, th2,
        p_b_local->x, p_b_local->y, pj->x, pj->y, pi->x, pi->y};
      if (use_taylor)
      {
        double r;
        if (solve_m2_taylor_endpoint_on_edge(&s, &r))
          push_event(r, events, &nevents, cap);
      }
      else
      {
        double roots[4];
        int nr = solve_m2_numerical(residual_endpoint_on_edge_wrap, &s,
          roots, 4);
        for (int k = 0; k < nr; k++)
          push_event(roots[k], events, &nevents, cap);
      }
    }
    /* s=0: pj lies on the moving edge at time t */
    {
      PolypointMovingEdgeState s = {x1, y1, th1, x2, y2, th2,
        p_a_local->x, p_a_local->y, p_b_local->x, p_b_local->y,
        pj->x, pj->y};
      double roots[4];
      int nr = solve_m2_numerical(residual_polypoint_on_movingedge_wrap,
        &s, roots, 4);
      for (int k = 0; k < nr; k++)
        push_event(roots[k], events, &nevents, cap);
    }
    /* s=1: pi lies on the moving edge at time t */
    {
      PolypointMovingEdgeState s = {x1, y1, th1, x2, y2, th2,
        p_a_local->x, p_a_local->y, p_b_local->x, p_b_local->y,
        pi->x, pi->y};
      double roots[4];
      int nr = solve_m2_numerical(residual_polypoint_on_movingedge_wrap,
        &s, roots, 4);
      for (int k = 0; k < nr; k++)
        push_event(roots[k], events, &nevents, cap);
    }
  }

  qsort(events, (size_t) nevents, sizeof(double), double_cmp);
  int nuniq = dedup_sorted(events, nevents);

  Span *out = palloc(sizeof(Span) * (size_t) (nuniq + 1));
  M2Ctx ctx = {x1, y1, th1, x2, y2, th2,
    p_a_local->x, p_a_local->y, p_b_local->x, p_b_local->y, pa};
  int nout = walk_events_and_emit(events, nuniq, m2_intersects_at, &ctx, out);

  pfree(events);
  if (nout == 0)
  {
    pfree(out);
    *intervals_out = NULL;
    return 0;
  }
  *intervals_out = out;
  return nout;
}

int
trgeo_geom_clip_lwpoly_posed(const POINT2D *p_a_local,
  const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const LWPOLY *poly, Span **intervals_out)
{
  if (! poly || poly->nrings < 1)
    return -1;
  return trgeo_geom_clip_polygon_posed(p_a_local, p_b_local,
    pose1, pose2, poly->rings[0], intervals_out);
}

int
trgeo_geom_clip_box_posed(const POINT2D *p_a_local,
  const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  double xmin, double ymin, double xmax, double ymax,
  Span **intervals_out)
{
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 5);
  POINT4D pt;
  pt.z = 0; pt.m = 0;
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmax; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymax; ptarray_append_point(pa, &pt, LW_TRUE);
  pt.x = xmin; pt.y = ymin; ptarray_append_point(pa, &pt, LW_TRUE);
  int n = trgeo_geom_clip_polygon_posed(p_a_local, p_b_local,
    pose1, pose2, pa, intervals_out);
  ptarray_free(pa);
  return n;
}

/*****************************************************************************
 * LWGEOM-level wrappers — accept any geometry type; polygon components
 * contribute clip intervals; POINT, LINESTRING, and other non-polygon types
 * contribute zero intervals (they have no ring for the edge kernel to test
 * against). This gives full geometry-subtype parity with tpoint_geom_clip.
 *****************************************************************************/

static int
clip_lwgeom_m1_accum(const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const LWGEOM *geom, Span **buf, int *cap, int n)
{
  if (!geom) return n;
  switch (geom->type)
  {
    case POLYGONTYPE:
    {
      Span *sub = NULL;
      int k = trgeo_geom_clip_lwpoly(a1, b1, a2, b2,
        lwgeom_as_lwpoly((LWGEOM *) geom), &sub);
      if (k > 0)
      {
        if (n + k > *cap)
        {
          *cap = (n + k) * 2 + 8;
          *buf = repalloc(*buf, (size_t) *cap * sizeof(Span));
        }
        memcpy(*buf + n, sub, (size_t) k * sizeof(Span));
        pfree(sub);
        n += k;
      }
      break;
    }
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      LWCOLLECTION *coll = lwgeom_as_lwcollection((LWGEOM *) geom);
      if (coll)
        for (uint32_t i = 0; i < coll->ngeoms; i++)
          n = clip_lwgeom_m1_accum(a1, b1, a2, b2, coll->geoms[i],
            buf, cap, n);
      break;
    }
    default:
      break; /* POINT, LINESTRING, etc. — no ring, zero contribution */
  }
  return n;
}

int
trgeo_geom_clip_lwgeom(const POINT2D *a1, const POINT2D *b1,
  const POINT2D *a2, const POINT2D *b2,
  const LWGEOM *geom, Span **intervals_out)
{
  if (!geom || !intervals_out) return -1;
  int cap = 16;
  Span *buf = palloc((size_t) cap * sizeof(Span));
  int n = clip_lwgeom_m1_accum(a1, b1, a2, b2, geom, &buf, &cap, 0);
  if (n == 0)
  {
    pfree(buf);
    *intervals_out = NULL;
    return 0;
  }
  *intervals_out = buf;
  return n;
}

static int
clip_lwgeom_m2_accum(const POINT2D *p_a_local, const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const LWGEOM *geom, Span **buf, int *cap, int n)
{
  if (!geom) return n;
  switch (geom->type)
  {
    case POLYGONTYPE:
    {
      Span *sub = NULL;
      int k = trgeo_geom_clip_lwpoly_posed(p_a_local, p_b_local,
        pose1, pose2, lwgeom_as_lwpoly((LWGEOM *) geom), &sub);
      if (k > 0)
      {
        if (n + k > *cap)
        {
          *cap = (n + k) * 2 + 8;
          *buf = repalloc(*buf, (size_t) *cap * sizeof(Span));
        }
        memcpy(*buf + n, sub, (size_t) k * sizeof(Span));
        pfree(sub);
        n += k;
      }
      break;
    }
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
    {
      LWCOLLECTION *coll = lwgeom_as_lwcollection((LWGEOM *) geom);
      if (coll)
        for (uint32_t i = 0; i < coll->ngeoms; i++)
          n = clip_lwgeom_m2_accum(p_a_local, p_b_local, pose1, pose2,
            coll->geoms[i], buf, cap, n);
      break;
    }
    default:
      break; /* POINT, LINESTRING, etc. — no ring, zero contribution */
  }
  return n;
}

int
trgeo_geom_clip_lwgeom_posed(const POINT2D *p_a_local,
  const POINT2D *p_b_local,
  const struct Pose *pose1, const struct Pose *pose2,
  const LWGEOM *geom, Span **intervals_out)
{
  if (!geom || !intervals_out) return -1;
  int cap = 16;
  Span *buf = palloc((size_t) cap * sizeof(Span));
  int n = clip_lwgeom_m2_accum(p_a_local, p_b_local, pose1, pose2,
    geom, &buf, &cap, 0);
  if (n == 0)
  {
    pfree(buf);
    *intervals_out = NULL;
    return 0;
  }
  *intervals_out = buf;
  return n;
}

/*****************************************************************************/
