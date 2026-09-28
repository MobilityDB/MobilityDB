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
 * @brief Closest-feature walk for the distance between rigid geometries
 * @details The walk finds the distance between a convex polygon A and a
 * convex polygon, a segment or a point B over one temporal segment.  It follows the
 * feature of the Minkowski difference A - B that is closest to the origin.
 * See meos/src/rgeo/trgeo_distance.txt for the algorithm.
 */

#include "rgeo/trgeo_distwalk.h"

/* C */
#include <assert.h>
#include <math.h>
/* MEOS */
#include "temporal/temporal.h"


/*****************************************************************************
 * Motion in one segment
 *****************************************************************************/

/**
 * @brief Return the rotation from the angle @p th1 to the angle @p th2
 * @details The rotation takes the shorter way, as in
 * #posesegm_interpolate(), with the same rule for a half turn.  Thus the walk
 * and the database place a body at the same position.
 */
static double
motion_omega(double th1, double th2)
{
  double d = th2 - th1;
  if (fabs(d) < MEOS_EPSILON)
    return 0.0;
  if (d > 0.0)
    return (d <= M_PI) ? d : d - 2.0 * M_PI;
  return (d > -M_PI) ? d : d + 2.0 * M_PI;
}

/**
 * @brief Set the motion of a body from its two poses (x1, y1, th1) and
 * (x2, y2, th2)
 */
void
distmotion_set(DistMotion *m, double x1, double y1, double th1, double x2,
  double y2, double th2)
{
  assert(m);
  m->cx = x1;
  m->cy = y1;
  m->dx = x2 - x1;
  m->dy = y2 - y1;
  m->th0 = th1;
  m->w = motion_omega(th1, th2);
}

/**
 * @brief Set the motion of a static body with rotation center (@p x, @p y)
 */
void
distmotion_set_static(DistMotion *m, double x, double y)
{
  assert(m);
  m->cx = x;
  m->cy = y;
  m->dx = m->dy = 0.0;
  m->th0 = m->w = 0.0;
}

/**
 * @brief Set the motion of a body from two poses
 */
void
distmotion_from_pose(DistMotion *m, const Pose *p1, const Pose *p2)
{
  assert(m); assert(p1); assert(p2);
  distmotion_set(m, p1->data[0], p1->data[1], p1->data[2], p2->data[0],
    p2->data[1], p2->data[2]);
}

/**
 * @brief Return in (@p x, @p y) the position at @p t of the point
 * (@p px, @p py) of a body
 */
void
distmotion_place(const DistMotion *m, double px, double py, double t,
  double *x, double *y)
{
  assert(m); assert(x); assert(y);
  double th = m->th0 + m->w * t;
  double c = cos(th), s = sin(th);
  *x = px * c - py * s + m->cx + m->dx * t;
  *y = px * s + py * c + m->cy + m->dy * t;
}

/**
 * @brief Set the motion of two bodies
 */
void
distsegm_set(DistSegm *seg, const DistMotion *ma, const DistMotion *mb)
{
  assert(seg); assert(ma); assert(mb);
  seg->ma = *ma;
  seg->mb = *mb;
  seg->wx0 = mb->cx - ma->cx;
  seg->wy0 = mb->cy - ma->cy;
  seg->wx1 = mb->dx - ma->dx;
  seg->wy1 = mb->dy - ma->dy;
}

/**
 * @brief Set an edge from its start and end vertices
 */
void
distrefedge_set(DistRefEdge *e, double sx, double sy, double ex, double ey)
{
  assert(e);
  double ux = ex - sx, uy = ey - sy;
  e->sx = sx; e->sy = sy;
  e->ex = ex; e->ey = ey;
  e->len = hypot(ux, uy);
  assert(e->len > 0.0);
  e->beta = atan2(uy, ux);
  e->dots = sx * ux + sy * uy;
  e->crss = sx * uy - sy * ux;
}

/**
 * @brief Set a vertex
 */
void
distrefvert_set(DistRefVert *v, double x, double y)
{
  assert(v);
  v->x = x;
  v->y = y;
  v->rho = hypot(x, y);
  v->alpha = (v->rho > 0.0) ? atan2(y, x) : 0.0;
}

/*****************************************************************************
 * Functions of the walk
 *
 * In debug builds, each function compares its result with a direct
 * computation from the placed geometry.  A wrong sign in a coefficient then
 * fails an assertion at once.
 *****************************************************************************/

#ifndef NDEBUG
/**
 * @brief Assert that @p f gives the value @p direct at @p t
 */
static void
distwalk_check(const DistFun *f, double direct, double t)
{
  double got = distfun_eval(f, t);
  double scale = 1.0 + fabs(direct) + fabs(got);
  assert(fabs(got - direct) <= 1e-6 * scale);
  (void) got; (void) scale; (void) t;
}
#endif

/**
 * @brief Set @p f to dot(p - s, e - s), with p the vertex @p v of B and s, e
 * the ends of the edge @p e of A
 * @details The projection parameter of p on the edge is f / L^2, with L the
 * length of the edge.  Thus f = 0 at the start of the edge and f = L^2 at its
 * end.  In terms of the motion,
 * @code
 * f(t) = L * [ Wx(t) * cos(wA*t + thA + beta) + Wy(t) * sin(wA*t + thA + beta) ]
 *      + rho * L * cos((wA - wB)*t + thA - thB + beta - alpha) - dots
 * @endcode
 */
void
distwalk_transition(const DistSegm *seg, const DistRefEdge *e,
  const DistRefVert *v, DistFun *f)
{
  assert(seg); assert(e); assert(v); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  double L = e->len;
  distfun_init(f);
  distfun_add(f, ma->w, ma->th0 + e->beta, L * seg->wx1, L * seg->wx0,
    L * seg->wy1, L * seg->wy0);
  if (v->rho > 0.0)
    distfun_add(f, ma->w - mb->w,
      (ma->th0 - mb->th0) + e->beta - v->alpha, 0.0, v->rho * L, 0.0, 0.0);
  distfun_add_poly(f, 0.0, 0.0, -e->dots);
#ifndef NDEBUG
  for (int i = 0; i <= 2; i++)
  {
    double t = 0.5 * i, qsx, qsy, qex, qey, px, py;
    distmotion_place(ma, e->sx, e->sy, t, &qsx, &qsy);
    distmotion_place(ma, e->ex, e->ey, t, &qex, &qey);
    distmotion_place(mb, v->x, v->y, t, &px, &py);
    distwalk_check(f, (px - qsx) * (qex - qsx) + (py - qsy) * (qey - qsy), t);
  }
#endif
}

/**
 * @brief Set @p f to cross(p - s, e - s), with p the vertex @p v of B and s,
 * e the ends of the edge @p e of A
 * @details The signed distance from p to the line of the edge is f / L, with
 * L the length of the edge.  The distance is positive outside A, which is on
 * the right of a counterclockwise edge.
 */
void
distwalk_cross(const DistSegm *seg, const DistRefEdge *e,
  const DistRefVert *v, DistFun *f)
{
  assert(seg); assert(e); assert(v); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  double L = e->len;
  distfun_init(f);
  /* The same term with cos and sin exchanged: the cos coefficient carries
   * -Wy and the sin coefficient carries Wx */
  distfun_add(f, ma->w, ma->th0 + e->beta, -L * seg->wy1, -L * seg->wy0,
    L * seg->wx1, L * seg->wx0);
  if (v->rho > 0.0)
    distfun_add(f, ma->w - mb->w,
      (ma->th0 - mb->th0) + e->beta - v->alpha, 0.0, 0.0, 0.0, v->rho * L);
  distfun_add_poly(f, 0.0, 0.0, -e->crss);
#ifndef NDEBUG
  for (int i = 0; i <= 2; i++)
  {
    double t = 0.5 * i, qsx, qsy, qex, qey, px, py;
    distmotion_place(ma, e->sx, e->sy, t, &qsx, &qsy);
    distmotion_place(ma, e->ex, e->ey, t, &qex, &qey);
    distmotion_place(mb, v->x, v->y, t, &px, &py);
    distwalk_check(f, (px - qsx) * (qey - qsy) - (py - qsy) * (qex - qsx), t);
  }
#endif
}

/**
 * @brief Set @p f to the squared distance between the vertex @p va of A and
 * the vertex @p vb of B
 * @details
 * @code
 * f(t) = |W(t)|^2 + ra^2 + rb^2
 *      + 2*rb * [ Wx(t)*cos(wB*t + thB + alpha_b) + Wy(t)*sin(wB*t + thB + alpha_b) ]
 *      - 2*ra * [ Wx(t)*cos(wA*t + thA + alpha_a) + Wy(t)*sin(wA*t + thA + alpha_a) ]
 *      - 2*ra*rb * cos((wB - wA)*t + thB - thA + alpha_b - alpha_a)
 * @endcode
 */
void
distwalk_vertdist2(const DistSegm *seg, const DistRefVert *va,
  const DistRefVert *vb, DistFun *f)
{
  assert(seg); assert(va); assert(vb); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  double wx1 = seg->wx1, wx0 = seg->wx0, wy1 = seg->wy1, wy0 = seg->wy0;
  distfun_init(f);
  /* |W(t)|^2, a quadratic, plus the two constant radii */
  distfun_add_poly(f, wx1 * wx1 + wy1 * wy1,
    2.0 * (wx0 * wx1 + wy0 * wy1),
    wx0 * wx0 + wy0 * wy0 + va->rho * va->rho + vb->rho * vb->rho);
  if (vb->rho > 0.0)
  {
    double k = 2.0 * vb->rho;
    distfun_add(f, mb->w, mb->th0 + vb->alpha, k * wx1, k * wx0, k * wy1,
      k * wy0);
  }
  if (va->rho > 0.0)
  {
    double k = -2.0 * va->rho;
    distfun_add(f, ma->w, ma->th0 + va->alpha, k * wx1, k * wx0, k * wy1,
      k * wy0);
  }
  if (va->rho > 0.0 && vb->rho > 0.0)
    distfun_add(f, mb->w - ma->w,
      (mb->th0 - ma->th0) + vb->alpha - va->alpha,
      0.0, -2.0 * va->rho * vb->rho, 0.0, 0.0);
#ifndef NDEBUG
  for (int i = 0; i <= 2; i++)
  {
    double t = 0.5 * i, ax, ay, bx, by;
    distmotion_place(ma, va->x, va->y, t, &ax, &ay);
    distmotion_place(mb, vb->x, vb->y, t, &bx, &by);
    distwalk_check(f, (bx - ax) * (bx - ax) + (by - ay) * (by - ay), t);
  }
#endif
}

/**
 * @brief Set @p f to cross(ua, ub), with ua the edge @p ea of A and ub the
 * edge @p eb of B, as vectors
 * @details The rotations do not change the lengths of the edges, thus
 * @code
 * f(t) = La * Lb * sin((wB - wA)*t + thB - thA + beta_b - beta_a)
 * @endcode
 * The function is zero when the two edges are parallel.
 */
void
distwalk_parallel(const DistSegm *seg, const DistRefEdge *ea,
  const DistRefEdge *eb, DistFun *f)
{
  assert(seg); assert(ea); assert(eb); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  distfun_init(f);
  distfun_add(f, mb->w - ma->w,
    (mb->th0 - ma->th0) + eb->beta - ea->beta, 0.0, 0.0, 0.0,
    ea->len * eb->len);
#ifndef NDEBUG
  for (int i = 0; i <= 2; i++)
  {
    double t = 0.5 * i, asx, asy, aex, aey, bsx, bsy, bex, bey;
    distmotion_place(ma, ea->sx, ea->sy, t, &asx, &asy);
    distmotion_place(ma, ea->ex, ea->ey, t, &aex, &aey);
    distmotion_place(mb, eb->sx, eb->sy, t, &bsx, &bsy);
    distmotion_place(mb, eb->ex, eb->ey, t, &bex, &bey);
    distwalk_check(f, (aex - asx) * (bey - bsy) - (aey - asy) * (bex - bsx),
      t);
  }
#endif
}

/*****************************************************************************
 * Events
 *****************************************************************************/

/**
 * @brief Initialize an empty array of events
 */
void
distevents_init(DistEvents *events)
{
  assert(events);
  events->size = 64;
  events->count = 0;
  events->ev = palloc(sizeof(DistEvent) * events->size);
}

/**
 * @brief Free an array of events
 */
void
distevents_free(DistEvents *events)
{
  assert(events);
  pfree(events->ev);
  events->ev = NULL;
  events->count = events->size = 0;
}

/**
 * @brief Add the distance @p dist at the time @p t to an array of events
 * @details The walk gives the events in time order.  Two events at the same
 * time become one event with the smaller distance.
 */
static void
distevents_add(DistEvents *events, double t, double dist)
{
  if (events->count > 0 && events->ev[events->count - 1].t == t)
  {
    if (dist < events->ev[events->count - 1].dist)
      events->ev[events->count - 1].dist = dist;
    return;
  }
  if (events->count == events->size)
  {
    events->size *= 2;
    events->ev = repalloc(events->ev, sizeof(DistEvent) * events->size);
  }
  events->ev[events->count].t = t;
  events->ev[events->count].dist = dist;
  events->count++;
}

/*****************************************************************************
 * Features of the Minkowski difference
 *
 * The edges of the difference, in counterclockwise order, are the edges of A
 * and the reversed edges of B, sorted by direction angle.  Each edge of A
 * goes with the vertex of B that supports it, which gives EV(i,j), and each
 * edge of B goes with the vertex of A that supports it, which gives VE(i,j).
 *****************************************************************************/

/**
 * @brief State of the walk in one segment
 */
typedef struct
{
  const DistSegm *sab;       /**< Motion with A first */
  DistSegm sba;              /**< Motion with B first */
  const DistRefPoly *ra;     /**< Body A */
  const DistRefPoly *rb;     /**< Body B */
  uint32_t n;                /**< Number of vertices of A */
  uint32_t m;                /**< Number of vertices of B */
  double ftol;               /**< Tolerance of the function values */
} DistWalk;

static inline uint32_t
idx_prev(uint32_t i, uint32_t n)
{
  return (i + n - 1) % n;
}

static inline uint32_t
idx_next(uint32_t i, uint32_t n)
{
  return (i + 1) % n;
}

static inline DistPair
pair_make(int kind, uint32_t i, uint32_t j)
{
  DistPair p;
  p.kind = kind;
  p.i = i;
  p.j = j;
  return p;
}

/**
 * @brief Return true if, just after @p t, edge @p i of A comes before the
 * reversed edge @p j of B in counterclockwise order
 * @details Edge i of A comes before the reversed edge j of B if the angle
 * from the first to the second is in [0, pi).  While the angle is not 0 or
 * pi, its sine gives the answer.  The parallel-edge function is
 * cross(uA, uB) = -cross(uA, reversed uB), which is negative when the angle
 * is in (0, pi).
 *
 * When the two edges stay parallel, the sine is zero and the cosine tells
 * the angle 0 from the angle pi.  The angle 0 counts as before: the edge of
 * A comes first.  Any fixed rule is correct there, because the two edges are
 * then on the same line of the difference; what is necessary is that each
 * question gets the same answer.  The angle pi does not count as before.  For
 * a polygon B this makes no difference, but for a segment it does: its two
 * reversed edges are at a half turn from each other, thus an edge of A that
 * is parallel to the segment is at the angle 0 from one and pi from the
 * other, and only one of them must be before it.
 */
static bool
walk_before(const DistWalk *w, uint32_t i, uint32_t j, double t)
{
  const DistRefEdge *ea = &w->ra->edges[i], *eb = &w->rb->edges[j];
  DistFun p;
  distwalk_parallel(w->sab, ea, eb, &p);
  int s = distfun_sign_after(&p, 0.0, t, w->ftol);
  if (s != 0)
    return s < 0;
  /* The edges are parallel: dot(uA, reversed uB) > 0 for the angle 0 */
  const DistMotion *ma = &w->sab->ma, *mb = &w->sab->mb;
  double phase = (mb->w - ma->w) * t + (mb->th0 - ma->th0) + eb->beta -
    ea->beta;
  return cos(phase) < 0.0;
}

/**
 * @brief Return true if the edge @p e is an edge of the difference just after
 * @p t, that is, if its vertex supports its edge
 * @details EV(i,j) is an edge of the difference if edge i of A is between
 * the reversed edges j - 1 and j of B.  VE(i,j) is an edge of the difference
 * if the reversed edge j of B is between edges i - 1 and i of A.  If B is a
 * point, all EV(i,0) are edges of the difference.
 */
static bool
walk_valid(const DistWalk *w, const DistPair *e, double t)
{
  if (e->kind == DISTPAIR_EV)
    return w->rb->nedge == 0 ||
      (! walk_before(w, e->i, idx_prev(e->j, w->m), t) &&
       walk_before(w, e->i, e->j, t));
  return walk_before(w, idx_prev(e->i, w->n), e->j, t) &&
    ! walk_before(w, e->i, e->j, t);
}

/**
 * @brief Return the edge of the difference that starts at the vertex
 * VV(@p i, @p j) just after @p t
 */
static DistPair
walk_vertex_next(const DistWalk *w, uint32_t i, uint32_t j, double t)
{
  if (w->rb->nedge > 0 && ! walk_before(w, i, j, t))
    return pair_make(DISTPAIR_VE, i, j);
  return pair_make(DISTPAIR_EV, i, j);
}

/**
 * @brief Return the edge of the difference that ends at the vertex
 * VV(@p i, @p j) just after @p t
 */
static DistPair
walk_vertex_prev(const DistWalk *w, uint32_t i, uint32_t j, double t)
{
  uint32_t ip = idx_prev(i, w->n);
  if (w->rb->nedge > 0)
  {
    uint32_t jp = idx_prev(j, w->m);
    if (walk_before(w, ip, jp, t))
      return pair_make(DISTPAIR_VE, i, jp);
  }
  return pair_make(DISTPAIR_EV, ip, j);
}

/**
 * @brief Return the start and end vertices of the edge @p e
 */
static void
walk_edge_ends(const DistWalk *w, const DistPair *e, DistPair *start,
  DistPair *end)
{
  *start = pair_make(DISTPAIR_VV, e->i, e->j);
  *end = (e->kind == DISTPAIR_EV) ?
    pair_make(DISTPAIR_VV, idx_next(e->i, w->n), e->j) :
    pair_make(DISTPAIR_VV, e->i, idx_next(e->j, w->m));
}

/**
 * @brief Set @p f to the projection function of the origin on the edge @p e
 * and return the length L of the edge
 * @details The origin projects on the start of the edge when f = 0 and on
 * its end when f = L^2.  For EV(i,j), this is the projection of b_j on edge
 * i of A.  For VE(i,j), this is the projection of a_i on edge j of B, which
 * goes in the same direction along the reversed edge.
 */
static double
walk_edge_n(const DistWalk *w, const DistPair *e, DistFun *f)
{
  if (e->kind == DISTPAIR_EV)
  {
    distwalk_transition(w->sab, &w->ra->edges[e->i], &w->rb->verts[e->j], f);
    return w->ra->edges[e->i].len;
  }
  distwalk_transition(&w->sba, &w->rb->edges[e->j], &w->ra->verts[e->i], f);
  return w->rb->edges[e->j].len;
}

/**
 * @brief Set @p g to the cross function of the edge @p e and return the
 * length L of the edge
 * @details The signed distance from the origin to the line of the edge is
 * g / L.  It is positive outside the difference, that is, when b_j is outside
 * edge i of A for EV(i,j), and when a_i is outside edge j of B for VE(i,j).
 */
static double
walk_edge_g(const DistWalk *w, const DistPair *e, DistFun *g)
{
  if (e->kind == DISTPAIR_EV)
  {
    distwalk_cross(w->sab, &w->ra->edges[e->i], &w->rb->verts[e->j], g);
    return w->ra->edges[e->i].len;
  }
  distwalk_cross(&w->sba, &w->rb->edges[e->j], &w->ra->verts[e->i], g);
  return w->rb->edges[e->j].len;
}

/**
 * @brief Set @p f to the distance function of the feature @p cf and return
 * the value to give to #fun_dist()
 * @details For a vertex, @p f is the squared distance and the result is 0.
 * For an edge, @p f is the cross function and the result is the length of
 * the edge.
 */
static double
walk_feature_fun(const DistWalk *w, const DistPair *cf, DistFun *f)
{
  if (cf->kind == DISTPAIR_VV)
  {
    distwalk_vertdist2(w->sab, &w->ra->verts[cf->i], &w->rb->verts[cf->j], f);
    return 0.0;
  }
  return walk_edge_g(w, cf, f);
}

/**
 * @brief Return the distance at @p t given by the function @p f of a feature,
 * see #walk_feature_fun()
 */
static double
fun_dist(const DistFun *f, double len, double t)
{
  double v = distfun_eval(f, t);
  if (len == 0.0)
    return (v > 0.0) ? sqrt(v) : 0.0;
  return fabs(v) / len;
}

/*****************************************************************************
 * Region of a feature
 *
 * The region of an edge of the difference is the set of points that project
 * on the edge and are outside the difference.  The region of a vertex is the
 * set of points that project before the start of the next edge and after the
 * end of the previous edge.  The walk keeps the feature whose region has the
 * origin just after the current time.
 *
 * The walk checks the region again at each event, and does not trust the
 * event alone.  Two events come from two different functions, and in a thin
 * region their times can disagree by a rounding error.  Then the origin is
 * already past the next boundary when the walk arrives in the region.
 *
 * On a boundary between two regions, the side is the one of the origin just
 * after the current time, see #distfun_sign_after().  If the origin moves
 * along the boundary, the two features give the same distance, and the walk
 * takes the edge, not the vertex.  This has two reasons.  The walk finds the
 * entry of the origin in the difference on an edge, thus it must not stay on
 * a vertex through which the origin enters.  And the test to go from a vertex
 * to an edge is the exact complement of the test to go back, thus the walk
 * cannot go back and forth between the two.  A point on the line of an edge
 * is outside the difference.
 *****************************************************************************/

/* Result of #walk_settle_local() when the origin is inside the line of the
 * edge of its feature */
#define WALK_INNER  1
/* Result of #walk_settle() when the origin is inside the difference */
#define WALK_INSIDE 2

/**
 * @brief Set @p cf to the feature whose region has the origin just after
 * @p t, with steps to neighbor features
 * @return #DISTWALK_OK, #WALK_INNER if the origin is inside the line of the
 * edge @p cf, or #DISTWALK_CYCLE
 */
static int
walk_settle_local(const DistWalk *w, double t, DistPair *cf)
{
  DistFun f, g;
  double len;
  uint32_t maxstep = 4 * (w->n + w->m) + 8;
  for (uint32_t step = 0; step < maxstep; step++)
  {
    if (cf->kind == DISTPAIR_VV)
    {
      DistPair next = walk_vertex_next(w, cf->i, cf->j, t);
      walk_edge_n(w, &next, &f);
      if (distfun_sign_after(&f, 0.0, t, w->ftol) >= 0)
      {
        *cf = next;
        continue;
      }
      DistPair prev = walk_vertex_prev(w, cf->i, cf->j, t);
      len = walk_edge_n(w, &prev, &f);
      if (distfun_sign_after(&f, len * len, t, w->ftol) <= 0)
      {
        *cf = prev;
        continue;
      }
      return DISTWALK_OK;
    }

    /* Move the vertex to the one that supports the edge */
    if (cf->kind == DISTPAIR_EV && w->rb->nedge > 0)
    {
      if (! walk_before(w, cf->i, cf->j, t))
      {
        cf->j = idx_next(cf->j, w->m);
        continue;
      }
      if (walk_before(w, cf->i, idx_prev(cf->j, w->m), t))
      {
        cf->j = idx_prev(cf->j, w->m);
        continue;
      }
    }
    else if (cf->kind == DISTPAIR_VE)
    {
      if (walk_before(w, cf->i, cf->j, t))
      {
        cf->i = idx_next(cf->i, w->n);
        continue;
      }
      if (! walk_before(w, idx_prev(cf->i, w->n), cf->j, t))
      {
        cf->i = idx_prev(cf->i, w->n);
        continue;
      }
    }

    DistPair start, end;
    walk_edge_ends(w, cf, &start, &end);
    len = walk_edge_n(w, cf, &f);
    if (distfun_sign_after(&f, 0.0, t, w->ftol) < 0)
    {
      *cf = start;
      continue;
    }
    if (distfun_sign_after(&f, len * len, t, w->ftol) > 0)
    {
      *cf = end;
      continue;
    }
    walk_edge_g(w, cf, &g);
    if (distfun_sign_after(&g, 0.0, t, w->ftol) < 0)
      return WALK_INNER;
    return DISTWALK_OK;
  }
  return DISTWALK_CYCLE;
}

/**
 * @brief Set @p cf to the feature of the difference that is closest to the
 * origin at @p t, by a scan of all the edges of the difference
 * @details The origin is inside the difference if it is inside the line of
 * each edge.  Else the closest point is on an edge that has the origin
 * outside its line.  Where two features have the same distance, the scan
 * takes any of them, without regard to where the origin goes.  Thus the
 * result is only a start for #walk_settle_local().
 * @return #DISTWALK_OK, or #WALK_INSIDE if the origin is inside
 */
static int
walk_scan(const DistWalk *w, double t, DistPair *cf)
{
  double best = INFINITY;
  bool inside = true;
  int lastkind = (w->rb->nedge > 0) ? DISTPAIR_VE : DISTPAIR_EV;
  for (uint32_t i = 0; i < w->n; i++)
    for (uint32_t j = 0; j < w->m; j++)
      for (int kind = DISTPAIR_EV; kind <= lastkind; kind++)
      {
        DistPair e = pair_make(kind, i, j);
        if (! walk_valid(w, &e, t))
          continue;
        DistFun f, g;
        double len = walk_edge_g(w, &e, &g);
        if (distfun_sign_after(&g, 0.0, t, w->ftol) < 0)
          continue;
        inside = false;
        walk_edge_n(w, &e, &f);
        double s = distfun_eval(&f, t) / (len * len);
        DistPair cand = e;
        double d;
        if (s <= 0.0 || s >= 1.0)
        {
          DistPair start, end;
          walk_edge_ends(w, &e, &start, &end);
          cand = (s <= 0.0) ? start : end;
          len = walk_feature_fun(w, &cand, &f);
          d = fun_dist(&f, len, t);
        }
        else
          d = fun_dist(&g, len, t);
        if (d < best)
        {
          best = d;
          *cf = cand;
        }
      }
  return inside ? WALK_INSIDE : DISTWALK_OK;
}

/**
 * @brief Set @p cf to the feature whose region has the origin just after
 * @p t
 * @details The walk steps to neighbor features.  A scan is necessary when
 * there is no feature yet, and when the origin is inside the line of the
 * edge of its feature: then the origin is inside the difference, or its
 * closest feature is not a neighbor.
 * @return #DISTWALK_OK, #WALK_INSIDE if the origin is inside the difference,
 * or #DISTWALK_CYCLE
 */
static int
walk_settle(const DistWalk *w, double t, DistPair *cf)
{
  if (cf->kind == DISTPAIR_NONE && walk_scan(w, t, cf) == WALK_INSIDE)
    return WALK_INSIDE;
  int rc = walk_settle_local(w, t, cf);
  if (rc != WALK_INNER)
    return rc;
  if (walk_scan(w, t, cf) == WALK_INSIDE)
    return WALK_INSIDE;
  rc = walk_settle_local(w, t, cf);
  return (rc == WALK_INNER) ? DISTWALK_CYCLE : rc;
}

/*****************************************************************************
 * Changes of feature
 *****************************************************************************/

/**
 * @brief Return in @p root the first crossing of @p level by @p f in the
 * direction @p dir in (@p lo, @p hi)
 * @return 1 if there is such a crossing, 0 if there is none, or
 * #DISTWALK_SOLVE
 */
static int
walk_crossing(const DistFun *f, double level, int dir, double lo, double hi,
  double ftol, double *root)
{
  if (lo >= hi)
    return 0;
  int rc = distfun_first_crossing(f, level, lo, hi, ftol, dir, root);
  if (rc == DISTFUN_ZERO)
    return 0;
  return (rc < 0) ? DISTWALK_SOLVE : rc;
}

/**
 * @brief If @p f crosses @p level in the direction @p dir before @p *tnext,
 * set @p tnext to the crossing and @p next to the feature @p to
 * @return #DISTWALK_OK or #DISTWALK_SOLVE
 */
static int
walk_candidate(const DistWalk *w, const DistFun *f, double level, int dir,
  double t, const DistPair *to, double *tnext, DistPair *next)
{
  double root;
  int rc = walk_crossing(f, level, dir, t, *tnext, w->ftol, &root);
  if (rc < 0)
    return rc;
  if (rc == 1)
  {
    *tnext = root;
    *next = *to;
  }
  return DISTWALK_OK;
}

/**
 * @brief Return in @p tnext the first time after @p t at which the origin
 * leaves the region of the feature @p cf, and in @p next the feature that it
 * enters
 * @details The events are crossings of a boundary in the direction that
 * leaves the region, see #distfun_crossings().  A root is not sufficient: the
 * boundary that the walk crossed at @p t is a root at @p t, and a touch of a
 * boundary does not leave the region.  A crossing has a known sign on the two
 * sides, thus neither of them is a crossing.
 *
 * An edge feature changes when the origin projects before its start
 * or after its end, and when its vertex stops supporting its edge.  The
 * support changes when an edge of A and a reversed edge of B become
 * parallel.  A vertex feature changes when the origin projects on its next
 * or previous edge.  Also, when the edges around a vertex change order, the
 * feature stays the same but its next or previous edge changes.
 * @return #DISTWALK_OK or #DISTWALK_SOLVE.  If the feature does not change
 * before the end of the segment, @p tnext is 1 and @p next is @p cf.
 */
static int
walk_next(const DistWalk *w, const DistPair *cf, double t, double *tnext,
  DistPair *next)
{
  DistFun f;
  double len;
  int rc;
  *tnext = 1.0;
  *next = *cf;

  if (cf->kind == DISTPAIR_VV)
  {
    DistPair vnext = walk_vertex_next(w, cf->i, cf->j, t);
    DistPair vprev = walk_vertex_prev(w, cf->i, cf->j, t);
    walk_edge_n(w, &vnext, &f);
    rc = walk_candidate(w, &f, 0.0, 1, t, &vnext, tnext, next);
    if (rc != DISTWALK_OK)
      return rc;
    len = walk_edge_n(w, &vprev, &f);
    rc = walk_candidate(w, &f, len * len, -1, t, &vprev, tnext, next);
    if (rc != DISTWALK_OK)
      return rc;
    /* Change of order of the edges around the vertex */
    if (w->rb->nedge > 0)
    {
      uint32_t ii[2] = {cf->i, idx_prev(cf->i, w->n)};
      uint32_t jj[2] = {cf->j, idx_prev(cf->j, w->m)};
      for (int k = 0; k < 2; k++)
      {
        distwalk_parallel(w->sab, &w->ra->edges[ii[k]],
          &w->rb->edges[jj[k]], &f);
        int dir = walk_before(w, ii[k], jj[k], t) ? 1 : -1;
        rc = walk_candidate(w, &f, 0.0, dir, t, cf, tnext, next);
        if (rc != DISTWALK_OK)
          return rc;
      }
    }
    return DISTWALK_OK;
  }

  DistPair start, end;
  walk_edge_ends(w, cf, &start, &end);
  len = walk_edge_n(w, cf, &f);
  rc = walk_candidate(w, &f, 0.0, -1, t, &start, tnext, next);
  if (rc != DISTWALK_OK)
    return rc;
  rc = walk_candidate(w, &f, len * len, 1, t, &end, tnext, next);
  if (rc != DISTWALK_OK)
    return rc;
  if (w->rb->nedge == 0)
    return DISTWALK_OK;

  /* Change of the supporting vertex.  For EV(i,j), the vertex of B moves on
   * when edge i of A passes the reversed edge j of B, and moves back when the
   * reversed edge j - 1 of B passes edge i of A.  For VE(i,j), the same with
   * the roles of A and B exchanged. */
  uint32_t fi, fj, bi, bj;
  int fdir, bdir;
  DistPair fwd, bwd;
  if (cf->kind == DISTPAIR_EV)
  {
    fi = cf->i; fj = cf->j; fdir = 1;
    bi = cf->i; bj = idx_prev(cf->j, w->m); bdir = -1;
    fwd = pair_make(DISTPAIR_EV, cf->i, idx_next(cf->j, w->m));
    bwd = pair_make(DISTPAIR_EV, cf->i, idx_prev(cf->j, w->m));
  }
  else
  {
    fi = cf->i; fj = cf->j; fdir = -1;
    bi = idx_prev(cf->i, w->n); bj = cf->j; bdir = 1;
    fwd = pair_make(DISTPAIR_VE, idx_next(cf->i, w->n), cf->j);
    bwd = pair_make(DISTPAIR_VE, idx_prev(cf->i, w->n), cf->j);
  }
  distwalk_parallel(w->sab, &w->ra->edges[fi], &w->rb->edges[fj], &f);
  rc = walk_candidate(w, &f, 0.0, fdir, t, &fwd, tnext, next);
  if (rc != DISTWALK_OK)
    return rc;
  distwalk_parallel(w->sab, &w->ra->edges[bi], &w->rb->edges[bj], &f);
  return walk_candidate(w, &f, 0.0, bdir, t, &bwd, tnext, next);
}

/**
 * @brief Return in @p texit the first time after @p t at which the origin
 * leaves the difference, and in @p edge the edge that it crosses
 * @details The origin leaves through an edge of the difference.  Thus the
 * exit is the first crossing up of a cross function at which its edge is an
 * edge of the difference and the origin projects on the edge.  The function
 * examines each edge feature, which costs one root isolation for each pair
 * of features while the bodies overlap.  trgeo_distance.txt describes a
 * witness that can reduce this cost.
 * @param[out] found False if the origin stays inside until the end of the
 * segment
 * @return #DISTWALK_OK or #DISTWALK_SOLVE
 */
static int
walk_exit(const DistWalk *w, double t, double *texit, DistPair *edge,
  bool *found)
{
  *found = false;
  *texit = 1.0;
  int lastkind = (w->rb->nedge > 0) ? DISTPAIR_VE : DISTPAIR_EV;
  for (uint32_t i = 0; i < w->n; i++)
    for (uint32_t j = 0; j < w->m; j++)
      for (int kind = DISTPAIR_EV; kind <= lastkind; kind++)
      {
        if (t >= *texit)
          return DISTWALK_OK;
        DistPair e = pair_make(kind, i, j);
        DistFun g, f;
        double len = walk_edge_g(w, &e, &g);
        double roots[DISTFUN_MAXROOTS];
        int dirs[DISTFUN_MAXROOTS];
        int n = distfun_crossings(&g, 0.0, t, *texit, w->ftol, roots, dirs,
          DISTFUN_MAXROOTS);
        if (n == DISTFUN_ZERO)
          continue;
        if (n < 0)
          return DISTWALK_SOLVE;
        walk_edge_n(w, &e, &f);
        for (int k = 0; k < n; k++)
        {
          if (dirs[k] < 0)
            continue;
          double s = distfun_eval(&f, roots[k]);
          if (s >= -w->ftol && s <= len * len + w->ftol &&
              walk_valid(w, &e, roots[k]))
          {
            *texit = roots[k];
            *edge = e;
            *found = true;
            break;
          }
        }
      }
  return DISTWALK_OK;
}

/**
 * @brief Add the events in (@p t, @p tend) of the distance given by the
 * function @p f of a feature, see #walk_feature_fun()
 * @details The events are the extrema of the distance, which are the
 * crossings of the derivative of @p f.  A contact, where the distance touches
 * zero, is one of them.  If @p level is not negative, the events also include
 * the crossings of the distance @p level, with the exact value @p level.
 * These are the times at which the result of #tdwithin_trgeometry_geo() and
 * the other threshold functions changes.  Then the distance is on the same
 * side of @p level on each piece of the result, thus the linear interpolation
 * of the result gives the exact answer for the threshold.
 * @return #DISTWALK_OK or #DISTWALK_SOLVE
 */
static int
walk_interval_events(const DistFun *f, double len, double t, double tend,
  double ftol, double level, DistEvents *events)
{
  if (t >= tend)
    return DISTWALK_OK;
  double times[2 * DISTFUN_MAXROOTS], values[2 * DISTFUN_MAXROOTS];
  int dirs[DISTFUN_MAXROOTS];
  int count = 0;

  DistFun df;
  distfun_deriv(f, &df);
  double roots[DISTFUN_MAXROOTS];
  int n = distfun_crossings(&df, 0.0, t, tend, ftol, roots, dirs,
    DISTFUN_MAXROOTS);
  if (n < 0 && n != DISTFUN_ZERO)
    return DISTWALK_SOLVE;
  for (int k = 0; k < n; k++)
  {
    times[count] = roots[k];
    values[count++] = fun_dist(f, len, roots[k]);
  }

  if (level >= 0.0)
  {
    /* The distance is sqrt(f) for a vertex and f / len for an edge.  On an
     * edge outside the difference, f is not negative. */
    double flevel = (len == 0.0) ? level * level : level * len;
    n = distfun_crossings(f, flevel, t, tend, ftol, roots, dirs,
      DISTFUN_MAXROOTS);
    if (n < 0 && n != DISTFUN_ZERO)
      return DISTWALK_SOLVE;
    for (int k = 0; k < n; k++)
    {
      times[count] = roots[k];
      values[count++] = level;
    }
  }

  /* Add the events in time order */
  for (int i = 1; i < count; i++)
  {
    double ti = times[i], vi = values[i];
    int j = i - 1;
    while (j >= 0 && times[j] > ti)
    {
      times[j + 1] = times[j];
      values[j + 1] = values[j];
      j--;
    }
    times[j + 1] = ti;
    values[j + 1] = vi;
  }
  for (int i = 0; i < count; i++)
    distevents_add(events, times[i], values[i]);
  return DISTWALK_OK;
}

/*****************************************************************************
 * The walk
 *****************************************************************************/

/**
 * @brief Return the tolerance of the function values for the bodies @p ra
 * and @p rb in the segment @p seg
 * @details The functions of the walk are squared lengths.  Their terms are at
 * most the square of the size of the configuration: the radii of the two
 * bodies plus the largest distance between their rotation centers.  The
 * tolerance is 1e-14 times that square, which is a small multiple of the
 * rounding error of the terms.
 */
double
distwalk_ftol(const DistSegm *seg, const DistRefPoly *ra,
  const DistRefPoly *rb)
{
  assert(seg); assert(ra); assert(rb);
  double w0 = hypot(seg->wx0, seg->wy0);
  double w1 = hypot(seg->wx0 + seg->wx1, seg->wy0 + seg->wy1);
  double scale = ra->rmax + rb->rmax + fmax(w0, w1);
  return 1e-14 * scale * scale;
}

/**
 * @brief Add to @p events the distance between the bodies @p ra and @p rb in
 * the segment @p seg
 * @details The distance between the bodies is the distance from the origin to
 * their Minkowski difference.  The walk follows the feature of the
 * difference that is closest to the origin.  While the feature stays the
 * same, the distance is a function of the family, and its extrema are the
 * crossings of its derivative.  When the origin enters the difference, the
 * bodies overlap and the distance is zero until the origin leaves.
 *
 * The events are the start of each feature interval, the extrema, the
 * entries and exits of the difference, the crossings of @p level, and the end
 * of the segment.  The
 * piecewise linear function through them has the true minimum of the
 * distance.
 *
 * @param[in] seg Motion of A and B
 * @param[in] ra,rb Bodies A and B.  A is a polygon, and B is a polygon, a
 * segment or a point.
 * @param[in] ftol Tolerance of the function values, see #distwalk_ftol()
 * @param[in] level Distance whose crossings are also events, or a negative
 * value for none, see walk_interval_events()
 * @param[in,out] cf Closest feature at the start of the segment, or
 * #DISTPAIR_NONE if unknown.  On return, closest feature at the end of the
 * segment.
 * @param[in,out] events Array to which the function adds the events
 * @return #DISTWALK_OK, #DISTWALK_CYCLE or #DISTWALK_SOLVE
 */
int
distwalk_segm(const DistSegm *seg, const DistRefPoly *ra,
  const DistRefPoly *rb, double ftol, double level, DistPair *cf,
  DistEvents *events)
{
  assert(seg); assert(ra); assert(rb); assert(cf); assert(events);
  assert(ra->nedge >= 3); assert(rb->nvert >= 1);

  DistWalk w;
  w.sab = seg;
  distsegm_set(&w.sba, &seg->mb, &seg->ma);
  w.ra = ra;
  w.rb = rb;
  w.n = (uint32_t) ra->nvert;
  w.m = (uint32_t) rb->nvert;
  w.ftol = ftol;

  double t = 0.0;
  DistPair cur = *cf;
  cur.i %= w.n;
  cur.j %= w.m;
  bool inside = false;
  int rc;

  /* The limit stops a walk that does not advance.  It grows with the number
   * of features: a rotation of the bodies passes each feature of the
   * difference in turn, thus a polygon with thousands of vertices takes
   * thousands of steps in a segment. */
  uint32_t maxstep = 16 * (w.n + w.m) + 64;
  for (uint32_t step = 0; step < maxstep; step++)
  {
    if (! inside)
    {
      rc = walk_settle(&w, t, &cur);
      if (rc == WALK_INSIDE)
        inside = true;
      else if (rc != DISTWALK_OK)
        return rc;
    }

    if (inside)
    {
      distevents_add(events, t, 0.0);
      double texit;
      bool found;
      rc = walk_exit(&w, t, &texit, &cur, &found);
      if (rc != DISTWALK_OK)
        return rc;
      if (! found)
      {
        distevents_add(events, 1.0, 0.0);
        cf->kind = DISTPAIR_NONE;
        return DISTWALK_OK;
      }
      distevents_add(events, texit, 0.0);
      t = texit;
      inside = false;
      continue;
    }

    double tnext;
    DistPair next;
    rc = walk_next(&w, &cur, t, &tnext, &next);
    if (rc != DISTWALK_OK)
      return rc;

    DistFun f;
    double len = walk_feature_fun(&w, &cur, &f);
    distevents_add(events, t, fun_dist(&f, len, t));

    /* The origin enters the difference when the cross function of the edge
     * crosses zero down */
    double tentry = tnext;
    if (cur.kind != DISTPAIR_VV)
    {
      rc = walk_crossing(&f, 0.0, -1, t, tnext, ftol, &tentry);
      if (rc < 0)
        return rc;
      if (rc == 0)
        tentry = tnext;
    }

    rc = walk_interval_events(&f, len, t, tentry, ftol, level, events);
    if (rc != DISTWALK_OK)
      return rc;

    if (tentry < tnext)
    {
      t = tentry;
      inside = true;
      continue;
    }
    if (tnext >= 1.0)
    {
      distevents_add(events, 1.0, fun_dist(&f, len, 1.0));
      *cf = cur;
      return DISTWALK_OK;
    }
    t = tnext;
    cur = next;
  }
  return DISTWALK_CYCLE;
}

/*****************************************************************************
 * Bodies
 *****************************************************************************/

/** A vertex at a distance of at most this fraction of the size of the ring
 * from the previous vertex is the same vertex */
#define DISTREF_DUPTOL     1e-12

/** A vertex whose turn has a sine of at most this value, and that does not
 * go back, is on the line between its two neighbors */
#define DISTREF_COLTOL     1e-12

/**
 * @brief Remove the repeated vertices and the vertices on the line between
 * their neighbors, orient the ring counterclockwise, and return the number of
 * vertices
 * @details These changes do not change the distance to the ring.
 * @return Number of vertices, or 0 if the ring is not a convex polygon
 */
static int
refring_canonicalize(double *px, double *py, int n)
{
  if (n < 3)
    return 0;
  double xmin = px[0], xmax = px[0], ymin = py[0], ymax = py[0];
  for (int i = 1; i < n; i++)
  {
    if (px[i] < xmin) xmin = px[i];
    if (px[i] > xmax) xmax = px[i];
    if (py[i] < ymin) ymin = py[i];
    if (py[i] > ymax) ymax = py[i];
  }
  double dtol = DISTREF_DUPTOL * hypot(xmax - xmin, ymax - ymin);

  /* Repeated vertices, also across the end of the ring */
  int m = 0;
  for (int i = 0; i < n; i++)
  {
    if (m > 0 && hypot(px[i] - px[m - 1], py[i] - py[m - 1]) <= dtol)
      continue;
    px[m] = px[i]; py[m] = py[i]; m++;
  }
  while (m > 1 && hypot(px[m - 1] - px[0], py[m - 1] - py[0]) <= dtol)
    m--;

  /* Vertices on the line between their neighbors.  The removal of a vertex
   * can put a neighbor on a line, thus the passes repeat until no vertex
   * goes.  A vertex where the ring goes back on the same line is the end of
   * a spike, not a point on a line: it stays, and the test of convexity
   * refuses the ring. */
  for (int pass = 0; pass < n && m >= 3; pass++)
  {
    int w = 0;
    for (int i = 0; i < m; i++)
    {
      int p = (i + m - 1) % m, q = (i + 1) % m;
      double ux = px[i] - px[p], uy = py[i] - py[p];
      double vx = px[q] - px[i], vy = py[q] - py[i];
      double lu = hypot(ux, uy), lv = hypot(vx, vy);
      if (lu > 0.0 && lv > 0.0 && ux * vx + uy * vy > 0.0 &&
          fabs(ux * vy - uy * vx) <= DISTREF_COLTOL * lu * lv)
        continue;
      px[w] = px[i]; py[w] = py[i]; w++;
    }
    if (w == m)
      break;
    m = w;
  }
  if (m < 3)
    return 0;

  /* Counterclockwise orientation */
  double area2 = 0.0;
  for (int i = 0; i < m; i++)
  {
    int j = (i + 1) % m;
    area2 += px[i] * py[j] - px[j] * py[i];
  }
  if (area2 < 0.0)
    for (int i = 0, j = m - 1; i < j; i++, j--)
    {
      double tx = px[i], ty = py[i];
      px[i] = px[j]; py[i] = py[j];
      px[j] = tx;    py[j] = ty;
    }
  else if (area2 == 0.0)
    return 0;

  /* Convexity */
  for (int i = 0; i < m; i++)
  {
    int j = (i + 1) % m, l = (i + 2) % m;
    if ((px[j] - px[i]) * (py[l] - py[j]) -
        (py[j] - py[i]) * (px[l] - px[j]) <= 0.0)
      return 0;
  }
  return m;
}

/**
 * @brief Set a body from a polygon whose vertices are relative to
 * (@p ox, @p oy)
 * @details A polygon with holes is refused: the walk uses the outer ring
 * only, and it would give a wrong distance for a body in a hole.
 * @return False if the polygon is not convex or has holes
 */
static bool
distrefpoly_set_offset(DistRefPoly *rp, const LWPOLY *poly, double ox,
  double oy)
{
  assert(rp);
  rp->nvert = rp->nedge = 0;
  rp->rmax = 0.0;
  rp->verts = NULL;
  rp->edges = NULL;
  if (! poly || poly->nrings != 1 || poly->rings[0]->npoints < 4)
    return false;
  int nraw = (int) poly->rings[0]->npoints - 1;
  double *px = palloc(sizeof(double) * nraw);
  double *py = palloc(sizeof(double) * nraw);
  for (int i = 0; i < nraw; i++)
  {
    const POINT2D *p = getPoint2d_cp(poly->rings[0], (uint32_t) i);
    px[i] = p->x - ox;
    py[i] = p->y - oy;
  }
  int n = refring_canonicalize(px, py, nraw);
  if (n == 0)
  {
    pfree(px); pfree(py);
    return false;
  }
  rp->nvert = rp->nedge = n;
  rp->verts = palloc(sizeof(DistRefVert) * n);
  rp->edges = palloc(sizeof(DistRefEdge) * n);
  for (int i = 0; i < n; i++)
  {
    distrefvert_set(&rp->verts[i], px[i], py[i]);
    if (rp->verts[i].rho > rp->rmax)
      rp->rmax = rp->verts[i].rho;
  }
  for (int i = 0; i < n; i++)
  {
    int j = (i + 1) % n;
    distrefedge_set(&rp->edges[i], px[i], py[i], px[j], py[j]);
  }
  pfree(px); pfree(py);
  return true;
}

/**
 * @brief Set a body from the reference geometry of a rigid geometry
 * @details The vertices are relative to the rotation center, which is the
 * origin of the reference geometry.
 * @return False if the polygon is not convex or has holes
 */
bool
distrefpoly_set(DistRefPoly *rp, const LWPOLY *poly)
{
  return distrefpoly_set_offset(rp, poly, 0.0, 0.0);
}

/**
 * @brief Set a body from a static polygon, and return in (@p cx, @p cy) the
 * center relative to which the vertices are
 * @details The center is the mean of the vertices.  Coordinates relative to
 * the center are small, which keeps the functions of the walk accurate when
 * the world coordinates are large.
 * @return False if the polygon is not convex or has holes
 */
bool
distrefpoly_set_static(DistRefPoly *rp, const LWPOLY *poly, double *cx,
  double *cy)
{
  assert(cx); assert(cy);
  *cx = *cy = 0.0;
  if (poly && poly->nrings >= 1 && poly->rings[0]->npoints >= 4)
  {
    uint32_t n = poly->rings[0]->npoints - 1;
    for (uint32_t i = 0; i < n; i++)
    {
      const POINT2D *p = getPoint2d_cp(poly->rings[0], i);
      *cx += p->x;
      *cy += p->y;
    }
    *cx /= n;
    *cy /= n;
  }
  return distrefpoly_set_offset(rp, poly, *cx, *cy);
}

/**
 * @brief Set a body from the segment from (@p x1, @p y1) to (@p x2, @p y2),
 * and return in (@p cx, @p cy) the middle of the segment, relative to which
 * the vertices are
 * @details A segment is a convex body with two vertices and two edges in
 * opposite directions.  The outside of each edge is the inside of the other,
 * thus the body has no inside, which is correct for a segment.  The walk
 * needs no other change for it.  As for #distrefpoly_set_static(), the
 * vertices are relative to the middle to keep the functions accurate.
 * @pre The two ends are different
 */
void
distrefpoly_set_segment(DistRefPoly *rp, double x1, double y1, double x2,
  double y2, double *cx, double *cy)
{
  assert(rp); assert(cx); assert(cy);
  assert(x1 != x2 || y1 != y2);
  *cx = 0.5 * (x1 + x2);
  *cy = 0.5 * (y1 + y2);
  double hx = 0.5 * (x2 - x1), hy = 0.5 * (y2 - y1);
  rp->nvert = rp->nedge = 2;
  rp->verts = palloc(sizeof(DistRefVert) * 2);
  rp->edges = palloc(sizeof(DistRefEdge) * 2);
  distrefvert_set(&rp->verts[0], -hx, -hy);
  distrefvert_set(&rp->verts[1], hx, hy);
  distrefedge_set(&rp->edges[0], -hx, -hy, hx, hy);
  distrefedge_set(&rp->edges[1], hx, hy, -hx, -hy);
  rp->rmax = rp->verts[0].rho;
}

/**
 * @brief Set a body that is a point at its rotation center
 */
void
distrefpoly_set_point(DistRefPoly *rp)
{
  assert(rp);
  rp->nvert = 1;
  rp->nedge = 0;
  rp->rmax = 0.0;
  rp->verts = palloc(sizeof(DistRefVert));
  rp->edges = NULL;
  distrefvert_set(&rp->verts[0], 0.0, 0.0);
}

/**
 * @brief Free the arrays of a body
 */
void
distrefpoly_free(DistRefPoly *rp)
{
  assert(rp);
  if (rp->verts)
    pfree(rp->verts);
  if (rp->edges)
    pfree(rp->edges);
  rp->verts = NULL;
  rp->edges = NULL;
  rp->nvert = rp->nedge = 0;
  rp->rmax = 0.0;
}

/*****************************************************************************/
