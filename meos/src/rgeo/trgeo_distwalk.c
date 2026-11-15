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
#include <string.h>
/* MEOS */
#include "temporal/temporal.h"
#include "geo/geo_funcs.h"
#include "geo/geo_ival.h"


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
  e->ux = ux; e->uy = uy;
  e->len = sqrt(ux * ux + uy * uy);
  assert(e->len > 0.0);
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
  v->rho = sqrt(x * x + y * y);
}

/**
 * @brief Return in (@p rc, @p rs) the vector (@p c, @p s) turned by the
 * angle @p th
 * @details This is the rotation of #distmotion_place().  The phase of a term
 * of the walk is the angle of a vector of the bodies, such as an edge,
 * turned by the angles of the poses.  The function keeps the vector as it is
 * when the angle is zero, thus a vector along an axis keeps its zero
 * coordinate.
 */
static void
rotate_cs(double c, double s, double th, double *rc, double *rs)
{
  if (th == 0.0)
  {
    *rc = c;
    *rs = s;
    return;
  }
  double ct = cos(th), st = sin(th);
  *rc = c * ct - s * st;
  *rs = s * ct + c * st;
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
 * with beta the angle of the edge and alpha the angle of the vertex.  Each
 * phase is given by a vector, see #distfun_add_cs(): L * (cos(beta),
 * sin(beta)) is the edge (ux, uy), and rho * L * (cos(beta - alpha),
 * sin(beta - alpha)) is (dot(u, v), cross(v, u)).
 */
void
distwalk_transition(const DistSegm *seg, const DistRefEdge *e,
  const DistRefVert *v, DistFun *f)
{
  assert(seg); assert(e); assert(v); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  double c, s;
  distfun_init(f);
  rotate_cs(e->ux, e->uy, ma->th0, &c, &s);
  distfun_add_cs(f, ma->w, c, s, seg->wx1, seg->wx0, seg->wy1, seg->wy0);
  if (v->rho > 0.0)
  {
    rotate_cs(e->ux * v->x + e->uy * v->y, e->uy * v->x - e->ux * v->y,
      ma->th0 - mb->th0, &c, &s);
    distfun_add_cs(f, ma->w - mb->w, c, s, 0.0, 1.0, 0.0, 0.0);
  }
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
  double c, s;
  distfun_init(f);
  /* The terms of distwalk_transition() with cos and sin exchanged: the cos
   * coefficient carries -Wy and the sin coefficient carries Wx */
  rotate_cs(e->ux, e->uy, ma->th0, &c, &s);
  distfun_add_cs(f, ma->w, c, s, -seg->wy1, -seg->wy0, seg->wx1, seg->wx0);
  if (v->rho > 0.0)
  {
    rotate_cs(e->ux * v->x + e->uy * v->y, e->uy * v->x - e->ux * v->y,
      ma->th0 - mb->th0, &c, &s);
    distfun_add_cs(f, ma->w - mb->w, c, s, 0.0, 0.0, 0.0, 1.0);
  }
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
 * with alpha_a and alpha_b the angles of the vertices.  Each phase is given
 * by a vector, see #distfun_add_cs(): rb * (cos(alpha_b), sin(alpha_b)) is
 * the vertex (xb, yb), and ra * rb * (cos(alpha_b - alpha_a),
 * sin(alpha_b - alpha_a)) is (dot(a, b), cross(a, b)).
 */
void
distwalk_vertdist2(const DistSegm *seg, const DistRefVert *va,
  const DistRefVert *vb, DistFun *f)
{
  assert(seg); assert(va); assert(vb); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  double wx1 = seg->wx1, wx0 = seg->wx0, wy1 = seg->wy1, wy0 = seg->wy0;
  double c, s;
  distfun_init(f);
  /* |W(t)|^2, a quadratic, plus the two constant radii */
  distfun_add_poly(f, wx1 * wx1 + wy1 * wy1,
    2.0 * (wx0 * wx1 + wy0 * wy1),
    wx0 * wx0 + wy0 * wy0 + va->rho * va->rho + vb->rho * vb->rho);
  if (vb->rho > 0.0)
  {
    rotate_cs(vb->x, vb->y, mb->th0, &c, &s);
    distfun_add_cs(f, mb->w, c, s, 2.0 * wx1, 2.0 * wx0, 2.0 * wy1,
      2.0 * wy0);
  }
  if (va->rho > 0.0)
  {
    rotate_cs(va->x, va->y, ma->th0, &c, &s);
    distfun_add_cs(f, ma->w, c, s, -2.0 * wx1, -2.0 * wx0, -2.0 * wy1,
      -2.0 * wy0);
  }
  if (va->rho > 0.0 && vb->rho > 0.0)
  {
    rotate_cs(va->x * vb->x + va->y * vb->y, va->x * vb->y - va->y * vb->x,
      mb->th0 - ma->th0, &c, &s);
    distfun_add_cs(f, mb->w - ma->w, c, s, 0.0, -2.0, 0.0, 0.0);
  }
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
 * with beta_a and beta_b the angles of the edges.  The phase is given by the
 * vector La * Lb * (cos(beta_b - beta_a), sin(beta_b - beta_a)), which is
 * (dot(ua, ub), cross(ua, ub)), see #distfun_add_cs().  The function is zero
 * when the two edges are parallel.
 */
void
distwalk_parallel(const DistSegm *seg, const DistRefEdge *ea,
  const DistRefEdge *eb, DistFun *f)
{
  assert(seg); assert(ea); assert(eb); assert(f);
  const DistMotion *ma = &seg->ma, *mb = &seg->mb;
  double c, s;
  distfun_init(f);
  rotate_cs(ea->ux * eb->ux + ea->uy * eb->uy,
    ea->ux * eb->uy - ea->uy * eb->ux, mb->th0 - ma->th0, &c, &s);
  distfun_add_cs(f, mb->w - ma->w, c, s, 0.0, 0.0, 0.0, 1.0);
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
  events->pc = NULL;
  events->npc = events->pcsize = 0;
}

/**
 * @brief Ask the walk to keep the pieces of the distance between its events,
 * see #DistPiece
 * @details The minimum over the parts of a geometry reads the pieces to find
 * where two parts are at the same distance.
 */
void
distevents_keep_pieces(DistEvents *events)
{
  assert(events);
  if (events->pc)
    return;
  events->pcsize = 16;
  events->npc = 0;
  events->pc = palloc(sizeof(DistPiece) * events->pcsize);
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
  if (events->pc)
    pfree(events->pc);
  events->pc = NULL;
  events->npc = events->pcsize = 0;
}

/**
 * @brief Add the piece [@p t0, @p t1] of the feature function @p f, or of an
 * overlap if @p f is NULL, to the pieces of @p events if it keeps them
 */
static void
distevents_add_piece(DistEvents *events, double t0, double t1,
  const DistFun *f, double len)
{
  if (! events->pc || t0 >= t1)
    return;
  if (events->npc == events->pcsize)
  {
    events->pcsize *= 2;
    events->pc = repalloc(events->pc, sizeof(DistPiece) * events->pcsize);
  }
  DistPiece *p = &events->pc[events->npc++];
  p->t0 = t0;
  p->t1 = t1;
  p->zero = (f == NULL);
  p->len = len;
  if (f)
    distfun_copy(&p->f, f);
  else
    distfun_init(&p->f);
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
  /* The edges are parallel: dot(uA, reversed uB) > 0 for the angle 0, that
   * is, dot(uA, uB) < 0 with the edges turned to time t */
  const DistMotion *ma = &w->sab->ma, *mb = &w->sab->mb;
  double dotab, crsab;
  rotate_cs(ea->ux * eb->ux + ea->uy * eb->uy,
    ea->ux * eb->uy - ea->uy * eb->ux,
    (mb->w - ma->w) * t + (mb->th0 - ma->th0), &dotab, &crsab);
  return dotab < 0.0;
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
 * @brief Return true if the origin is not inside the line of the edge @p e
 * just after @p t
 * @details This is the test of #walk_settle_local() on the cross function
 * of #walk_edge_g(): a point on the line of an edge is outside the
 * difference.
 */
static bool
walk_edge_outside(const DistWalk *w, const DistPair *e, double t)
{
  DistFun g;
  walk_edge_g(w, e, &g);
  return distfun_sign_after(&g, 0.0, t, w->ftol) >= 0;
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

/**
 * @brief Return the distance at @p t of the piece @p p, as the walk gives it
 * at its events, see #fun_dist()
 */
double
distpiece_dist(const DistPiece *p, double t)
{
  assert(p);
  return p->zero ? 0.0 : fun_dist(&p->f, p->len, t);
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
      /* The region of an edge is the set of points that project on the edge
       * and are not inside its line.  An origin on the boundary of the
       * difference, which moves along an edge from one of its ends, projects
       * on both edges of that end and is inside the line of one of them.  It
       * goes to the edge whose region it is in.  If it is in neither region
       * and projects on an edge, it goes to that edge, where
       * #walk_settle() finds it inside the difference. */
      DistPair next = walk_vertex_next(w, cf->i, cf->j, t);
      walk_edge_n(w, &next, &f);
      bool onnext = distfun_sign_after(&f, 0.0, t, w->ftol) >= 0;
      DistPair prev = walk_vertex_prev(w, cf->i, cf->j, t);
      len = walk_edge_n(w, &prev, &f);
      bool onprev = distfun_sign_after(&f, len * len, t, w->ftol) <= 0;
      if (onnext && walk_edge_outside(w, &next, t))
        *cf = next;
      else if (onprev && walk_edge_outside(w, &prev, t))
        *cf = prev;
      else if (onnext)
        *cf = next;
      else if (onprev)
        *cf = prev;
      else
        return DISTWALK_OK;
      continue;
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
  double x1 = seg->wx0 + seg->wx1, y1 = seg->wy0 + seg->wy1;
  double w2 = fmax(seg->wx0 * seg->wx0 + seg->wy0 * seg->wy0,
    x1 * x1 + y1 * y1);
  double scale = ra->rmax + rb->rmax + sqrt(w2);
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
        distevents_add_piece(events, t, 1.0, NULL, 0.0);
        distevents_add(events, 1.0, 0.0);
        cf->kind = DISTPAIR_NONE;
        return DISTWALK_OK;
      }
      distevents_add_piece(events, t, texit, NULL, 0.0);
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
    distevents_add_piece(events, t, tentry, &f, len);

    if (tentry < tnext)
    {
      t = tentry;
      inside = true;
      continue;
    }
    /* An edge whose function is zero on the interval is a contact that slides
     * until the end of the interval, where the square root of the function of
     * the next feature, a vertex, would give the rounding of its zero */
    if (cur.kind != DISTPAIR_VV && distfun_is_zero(&f, ftol))
      distevents_add(events, tnext, 0.0);
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

/**
 * @brief Return true if @p b is strictly between @p a and @p c
 */
static bool
refring_between(double a, double b, double c)
{
  return (a < b && b < c) || (a > b && b > c);
}

/**
 * @brief Remove the repeated vertices and the vertices on the line between
 * their neighbors, orient the ring counterclockwise, and return the number of
 * vertices
 * @details These changes do not change the distance to the ring.  Every
 * question is decided exactly on the doubles of the ring: two vertices are
 * the same vertex when their coordinates are equal, a vertex is on the line
 * between its neighbors when the cross product of its two edges is zero
 * (#cross_product_sign) and it lies strictly between them, and the turns of
 * a convex ring all have one sign, which is its orientation.
 * @return Number of vertices, or 0 if the ring is not a convex polygon
 */
static int
refring_canonicalize(double *px, double *py, int n)
{
  if (n < 3)
    return 0;

  /* Repeated vertices, also across the end of the ring */
  int m = 0;
  for (int i = 0; i < n; i++)
  {
    if (m > 0 && px[i] == px[m - 1] && py[i] == py[m - 1])
      continue;
    px[m] = px[i]; py[m] = py[i]; m++;
  }
  while (m > 1 && px[m - 1] == px[0] && py[m - 1] == py[0])
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
      if (cross_product_sign(px[p], py[p], px[i], py[i], px[i], py[i],
            px[q], py[q]) == 0 &&
          (refring_between(px[p], px[i], px[q]) ||
           refring_between(py[p], py[i], py[q])))
        continue;
      px[w] = px[i]; py[w] = py[i]; w++;
    }
    if (w == m)
      break;
    m = w;
  }
  if (m < 3)
    return 0;

  /* Convexity: every turn has the sign of the first one, which is not zero */
  int orient = 0;
  for (int i = 0; i < m; i++)
  {
    int j = (i + 1) % m, l = (i + 2) % m;
    int turn = cross_product_sign(px[i], py[i], px[j], py[j], px[j], py[j],
      px[l], py[l]);
    if (turn == 0 || (orient != 0 && turn != orient))
      return 0;
    orient = turn;
  }

  /* Counterclockwise orientation */
  if (orient < 0)
    for (int i = 0, j = m - 1; i < j; i++, j--)
    {
      double tx = px[i], ty = py[i];
      px[i] = px[j]; py[i] = py[j];
      px[j] = tx;    py[j] = ty;
    }
  return m;
}

/**
 * @brief Return a double no less than the exact largest distance of the
 * @p n input vertices to (@p ox, @p oy)
 * @details The bound of the distance #distrefvert_set() rounds to nearest as
 * @p rho, read in intervals (#Ival) on the input doubles: each difference,
 * square, sum and square root holds the exact value, so the upper end of the
 * largest is no less than it.  The vertices the walk keeps are these less the
 * center, rounded, and a body placed at any pose keeps every point of its
 * input within this distance of its rotation center.
 */
static double
distrefpoly_radius_up(const double *px, const double *py, int n, double ox,
  double oy)
{
  double result = 0.0;
  for (int i = 0; i < n; i++)
  {
    Ival x = ival_sub(ival(px[i]), ival(ox));
    Ival y = ival_sub(ival(py[i]), ival(oy));
    double r = ival_sqrt(ival_dot(x, y, x, y)).hi;
    if (r > result)
      result = r;
  }
  return result;
}

/**
 * @brief Set a body from the @p nraw vertices of a ring, without its closing
 * point, relative to (@p ox, @p oy)
 * @details The second half of #distrefpoly_set_offset, which reads the ring
 * of a polygon into the arrays; the pieces of #distrefpoly_cover come here
 * from arrays of their own.  The arrays are freed.
 * @return False if the ring is not a convex polygon
 */
static bool
distrefpoly_set_ring(DistRefPoly *rp, double *px, double *py, int nraw,
  double ox, double oy)
{
  assert(rp);
  rp->nvert = rp->nedge = 0;
  rp->rmax = rp->rmax_up = 0.0;
  rp->verts = NULL;
  rp->edges = NULL;
  /* The bound reads every input vertex: one the cleaning removes lies on a
   * segment between two that stay, no farther from the center */
  double rmax_up = distrefpoly_radius_up(px, py, nraw, ox, oy);
  /* The ring is cleaned on its input vertices, which decide whether it is
   * convex.  The subtraction of a nonzero offset rounds, so the walk's ring
   * is cleaned again on the doubles it uses: two vertices can become one and
   * a turn can become flat */
  int n = refring_canonicalize(px, py, nraw);
  if (n > 0 && (ox != 0.0 || oy != 0.0))
  {
    for (int i = 0; i < n; i++)
    {
      px[i] -= ox;
      py[i] -= oy;
    }
    n = refring_canonicalize(px, py, n);
  }
  if (n == 0)
  {
    pfree(px); pfree(py);
    return false;
  }
  rp->nvert = rp->nedge = n;
  rp->rmax_up = rmax_up;
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
 * @brief Set a body from a polygon whose vertices are relative to
 * (@p ox, @p oy)
 * @details It reads the ring into arrays and leaves the rest to
 * #distrefpoly_set_ring.  A polygon with holes is refused: the walk uses the
 * outer ring only, and it would give a wrong distance for a body in a hole.
 * @return False if the polygon is not convex or has holes
 */
static bool
distrefpoly_set_offset(DistRefPoly *rp, const LWPOLY *poly, double ox,
  double oy)
{
  assert(rp);
  if (! poly || poly->nrings != 1 || poly->rings[0]->npoints < 4)
  {
    rp->nvert = rp->nedge = 0;
    rp->rmax = rp->rmax_up = 0.0;
    rp->verts = NULL;
    rp->edges = NULL;
    return false;
  }
  int nraw = (int) poly->rings[0]->npoints - 1;
  double *px = palloc(sizeof(double) * nraw);
  double *py = palloc(sizeof(double) * nraw);
  for (int i = 0; i < nraw; i++)
  {
    const POINT2D *p = getPoint2d_cp(poly->rings[0], (uint32_t) i);
    px[i] = p->x;
    py[i] = p->y;
  }
  return distrefpoly_set_ring(rp, px, py, nraw, ox, oy);
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
  /* The two input ends, from the rounded middle */
  const double ex[2] = {x1, x2}, ey[2] = {y1, y2};
  rp->rmax_up = distrefpoly_radius_up(ex, ey, 2, *cx, *cy);
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
  rp->rmax = rp->rmax_up = 0.0;
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
  rp->rmax = rp->rmax_up = 0.0;
}

/*****************************************************************************
 * Exact convex cover of a polygon
 *****************************************************************************/

/**
 * @brief Return the sign of the turn from vertex @p a through @p b to @p c,
 * decided exactly by #cross_product_sign
 */
static inline int
cover_orient(const double *x, const double *y, int a, int b, int c)
{
  return cross_product_sign(x[a], y[a], x[b], y[b], x[a], y[a], x[c], y[c]);
}

/**
 * @brief Return true if two vertices have the same coordinates
 */
static inline bool
cover_same(const double *x, const double *y, int a, int b)
{
  return x[a] == x[b] && y[a] == y[b];
}

/**
 * @brief Return true if vertex @p c lies on the closed segment from @p a to
 * @p b
 */
static bool
cover_on_segment(const double *x, const double *y, int a, int b, int c)
{
  return cover_orient(x, y, a, b, c) == 0 &&
    fmin(x[a], x[b]) <= x[c] && x[c] <= fmax(x[a], x[b]) &&
    fmin(y[a], y[b]) <= y[c] && y[c] <= fmax(y[a], y[b]);
}

/**
 * @brief Return true if the closed segments from @p a to @p b and from @p c
 * to @p d share a point, as #seg_meet of the CGAL judges decides it
 */
static bool
cover_segments_meet(const double *x, const double *y, int a, int b, int c,
  int d)
{
  int o1 = cover_orient(x, y, a, b, c), o2 = cover_orient(x, y, a, b, d);
  int o3 = cover_orient(x, y, c, d, a), o4 = cover_orient(x, y, c, d, b);
  if (o1 * o2 < 0 && o3 * o4 < 0)
    return true;
  return (o1 == 0 && cover_on_segment(x, y, a, b, c)) ||
    (o2 == 0 && cover_on_segment(x, y, a, b, d)) ||
    (o3 == 0 && cover_on_segment(x, y, c, d, a)) ||
    (o4 == 0 && cover_on_segment(x, y, c, d, b));
}

/**
 * @brief Return true if the direction from vertex @p p to vertex @p q points
 * strictly into the area at @p p, where the ring comes from @p a and goes on
 * to @p b with the area on its left
 */
static bool
cover_in_wedge(const double *x, const double *y, int a, int p, int b, int q)
{
  int in_left = cover_orient(x, y, a, p, q) > 0;
  int out_left = cover_orient(x, y, p, b, q) > 0;
  if (cover_orient(x, y, a, p, b) > 0)
    return in_left && out_left;
  return in_left || out_left;
}

/**
 * @brief Read a ring into the arrays from position @p at on, without its
 * closing point, its repeated vertices and the vertices on the segment
 * between their neighbors, and orient it counterclockwise if @p ccw, else
 * clockwise
 * @return Number of vertices, or 0 if the ring has fewer than three or a
 * spike, where it goes back on itself
 */
static int
cover_read_ring(const POINTARRAY *pa, double *x, double *y, int at,
  bool ccw)
{
  int n = (int) pa->npoints - 1, m = 0;
  for (int i = 0; i < n; i++)
  {
    const POINT2D *p = getPoint2d_cp(pa, (uint32_t) i);
    if (m > 0 && x[at + m - 1] == p->x && y[at + m - 1] == p->y)
      continue;
    x[at + m] = p->x; y[at + m] = p->y; m++;
  }
  while (m > 1 && x[at + m - 1] == x[at] && y[at + m - 1] == y[at])
    m--;
  /* The vertices on the segment between their neighbors, as
   * #refring_canonicalize removes them; a vertex where the ring turns back
   * on the same line is a spike, and the ring is refused */
  for (int pass = 0; pass < n && m >= 3; pass++)
  {
    int w = 0;
    for (int i = 0; i < m; i++)
    {
      int p = at + (i + m - 1) % m, c = at + i, q = at + (i + 1) % m;
      if (cover_orient(x, y, p, c, q) == 0)
      {
        if (! cover_on_segment(x, y, p, q, c))
          return 0;
        continue;
      }
      x[at + w] = x[c]; y[at + w] = y[c]; w++;
    }
    if (w == m)
      break;
    m = w;
  }
  if (m < 3)
    return 0;
  /* The orientation is the turn at the lowest, then leftmost, vertex, which
   * is a vertex of the convex hull of the ring */
  int k = at;
  for (int i = at + 1; i < at + m; i++)
    if (y[i] < y[k] || (y[i] == y[k] && x[i] < x[k]))
      k = i;
  int turn = cover_orient(x, y, at + (k - at + m - 1) % m, k,
    at + (k - at + 1) % m);
  if ((turn > 0) != ccw)
    for (int i = at, j = at + m - 1; i < j; i++, j--)
    {
      double tx = x[i], ty = y[i];
      x[i] = x[j]; y[i] = y[j];
      x[j] = tx;   y[j] = ty;
    }
  return m;
}

/**
 * @brief Return true if the bridge from vertex @p m of a hole to vertex
 * @p p meets the edge from @p u to @p v anywhere but at its ends
 * @details An edge with an end at @p p or @p m touches the bridge there, and
 * meets it elsewhere only where its other end lies on the bridge
 */
static bool
cover_bridge_meets(const double *x, const double *y, int m, int p, int u,
  int v)
{
  bool u_end = cover_same(x, y, u, p) || cover_same(x, y, u, m);
  bool v_end = cover_same(x, y, v, p) || cover_same(x, y, v, m);
  if (u_end && v_end)
    return true;
  if (u_end)
    return cover_on_segment(x, y, m, p, v);
  if (v_end)
    return cover_on_segment(x, y, m, p, u);
  return cover_segments_meet(x, y, m, p, u, v);
}

/**
 * @brief Return true if the vertex at position @p c of the ring sees vertex
 * @p m of a hole: the bridge between them lies in the area of the polygon,
 * meeting no edge of the ring or of a hole not yet bridged but at its ends
 * @param[in] ring,len Positions of the vertices of the ring
 * @param[in] holes,hlen,nholes First vertex and count of the holes not yet
 * bridged
 * @param[in] mprev,mnext Neighbors of @p m in its hole
 */
static bool
cover_sees(const double *x, const double *y, const int *ring, int len,
  int c, int m, int mprev, int mnext, const int *holes, const int *hlen,
  int nholes)
{
  int p = ring[c];
  if (! cover_in_wedge(x, y, ring[(c + len - 1) % len], p,
        ring[(c + 1) % len], m) ||
      ! cover_in_wedge(x, y, mprev, m, mnext, p))
    return false;
  for (int i = 0; i < len; i++)
    if (cover_bridge_meets(x, y, m, p, ring[i], ring[(i + 1) % len]))
      return false;
  for (int h = 0; h < nholes; h++)
    for (int i = 0; i < hlen[h]; i++)
      if (cover_bridge_meets(x, y, m, p, holes[h] + i,
            holes[h] + (i + 1) % hlen[h]))
        return false;
  return true;
}

/**
 * @brief Return true if the vertex at position @p i of the list, between
 * @p a and @p b, is an ear: its triangle turns counterclockwise and holds no
 * other vertex of the list
 * @details A vertex at a corner of the triangle that is another visit of
 * that corner, as the two ends of a bridge are, blocks the ear only where
 * one of its edges goes into the triangle at that corner
 */
static bool
cover_is_ear(const double *x, const double *y, const int *ring,
  const int *prev, const int *next, int a, int i, int b)
{
  int A = ring[a], P = ring[i], B = ring[b];
  if (cover_orient(x, y, A, P, B) <= 0)
    return false;
  for (int j = next[b]; j != a; j = next[j])
  {
    int V = ring[j];
    /* The corner, and the corners before and after it in the triangle */
    int corner = -1, before = -1, after = -1;
    if (cover_same(x, y, V, A))
      { corner = A; before = B; after = P; }
    else if (cover_same(x, y, V, P))
      { corner = P; before = A; after = B; }
    else if (cover_same(x, y, V, B))
      { corner = B; before = P; after = A; }
    if (corner >= 0)
    {
      const int w[2] = {ring[prev[j]], ring[next[j]]};
      for (int k = 0; k < 2; k++)
        if (cover_orient(x, y, corner, after, w[k]) > 0 &&
            cover_orient(x, y, before, corner, w[k]) > 0)
          return false;
      continue;
    }
    if (cover_orient(x, y, A, P, V) >= 0 && cover_orient(x, y, P, B, V) >= 0 &&
        cover_orient(x, y, B, A, V) >= 0)
      return false;
  }
  return true;
}

/**
 * @brief Return the piece that triangle @p t belongs to
 */
static int
cover_find(int *owner, int t)
{
  while (owner[t] != t)
  {
    owner[t] = owner[owner[t]];
    t = owner[t];
  }
  return t;
}

/**
 * @brief Return the index in @p list of @p n positions of the position
 * @p v that follows @p u, or -1
 */
static int
cover_index_after(const int *list, int n, int u, int v)
{
  for (int k = 0; k < n; k++)
    if (list[k] == v && list[(k + n - 1) % n] == u)
      return k;
  return -1;
}

/**
 * @brief Set the pieces of an exact convex cover of a polygon, holes
 * included, as bodies relative to the rotation center
 * @details The algorithm of section 5.1 of NON-CONVEX-DISTANCE.md, beside
 * #distrefpoly_set, which takes a convex polygon as its only piece.  Every
 * hole, from its rightmost vertex, is bridged to a vertex of the ring that
 * sees it, giving one ring that visits each bridge twice; ear clipping
 * triangulates that ring; and Hertel-Mehlhorn removes every diagonal whose
 * two ends stay convex in the union of its two pieces.  Every question is a
 * turn of three input vertices decided exactly (#cross_product_sign), so
 * every vertex of a piece is a vertex of the polygon, the pieces have
 * disjoint interiors, and their union is the area of the polygon, holes
 * excluded.  The distance from the polygon is then the least distance from
 * a piece, and it is zero exactly where a piece meets the other operand.
 * @param[in] poly Polygon
 * @param[out] pieces Array of the pieces
 * @param[out] npieces Number of pieces
 * @return False if a ring has fewer than three vertices or a spike, or the
 * rings do not bound an area
 */
bool
distrefpoly_cover(const LWPOLY *poly, DistRefPoly **pieces, int *npieces)
{
  assert(pieces); assert(npieces);
  *pieces = NULL;
  *npieces = 0;
  if (! poly || poly->nrings < 1)
    return false;
  if (poly->nrings == 1)
  {
    DistRefPoly *one = palloc(sizeof(DistRefPoly));
    if (distrefpoly_set(one, poly))
    {
      *pieces = one;
      *npieces = 1;
      return true;
    }
    pfree(one);
  }

  /* The vertices of the rings, the outer one counterclockwise and the holes
   * clockwise, so that the area is on the left of every edge */
  int total = 0;
  for (uint32_t r = 0; r < poly->nrings; r++)
    total += (int) poly->rings[r]->npoints;
  double *x = palloc(sizeof(double) * total);
  double *y = palloc(sizeof(double) * total);
  int nholes = (int) poly->nrings - 1;
  int *holes = palloc(sizeof(int) * (nholes + 1));
  int *hlen = palloc(sizeof(int) * (nholes + 1));
  int *ring = palloc(sizeof(int) * (total + 2 * nholes));
  int nv = 0, len = 0, ntri = 0;
  int *prev = NULL, *next = NULL, *tri = NULL, *owner = NULL, *pn = NULL;
  int **pv = NULL;
  bool ok = true;
  for (uint32_t r = 0; r < poly->nrings && ok; r++)
  {
    int m = cover_read_ring(poly->rings[r], x, y, nv, r == 0);
    if (m == 0)
      ok = false;
    else if (r == 0)
    {
      for (int i = 0; i < m; i++)
        ring[len++] = nv + i;
    }
    else
    {
      holes[r - 1] = nv;
      hlen[r - 1] = m;
    }
    nv += m;
  }

  /* The holes in decreasing order of their rightmost vertex, each bridged
   * to the nearest vertex of the ring that sees it.  The distance only
   * orders the candidates; whether one sees the hole is decided exactly */
  int *cand = ok ? palloc(sizeof(int) * (total + 2 * nholes)) : NULL;
  double *cd = ok ? palloc(sizeof(double) * (total + 2 * nholes)) : NULL;
  while (ok && nholes > 0)
  {
    int h = 0, hm = holes[0];
    for (int g = 0; g < nholes; g++)
      for (int i = holes[g]; i < holes[g] + hlen[g]; i++)
        if (x[i] > x[hm] || (x[i] == x[hm] && y[i] > y[hm]))
        {
          hm = i;
          h = g;
        }
    int k = hm - holes[h], n = hlen[h];
    int mprev = holes[h] + (k + n - 1) % n, mnext = holes[h] + (k + 1) % n;
    for (int c = 0; c < len; c++)
    {
      double dx = x[ring[c]] - x[hm], dy = y[ring[c]] - y[hm];
      double d = dx * dx + dy * dy;
      int j = c;
      while (j > 0 && cd[j - 1] > d)
      {
        cand[j] = cand[j - 1];
        cd[j] = cd[j - 1];
        j--;
      }
      cand[j] = c;
      cd[j] = d;
    }
    int found = -1;
    for (int j = 0; j < len && found < 0; j++)
      if (cover_sees(x, y, ring, len, cand[j], hm, mprev, mnext, holes, hlen,
            nholes))
        found = cand[j];
    if (found < 0)
    {
      ok = false;
      break;
    }
    /* The ring up to the vertex that sees the hole, the hole from its
     * rightmost vertex round to it again, the vertex again, the rest */
    int tail = len - found - 1;
    memmove(&ring[found + n + 3], &ring[found + 1], sizeof(int) * tail);
    for (int i = 0; i <= n; i++)
      ring[found + 1 + i] = holes[h] + (k + i) % n;
    ring[found + n + 2] = ring[found];
    len += n + 2;
    holes[h] = holes[nholes - 1];
    hlen[h] = hlen[nholes - 1];
    nholes--;
  }

  /* Ear clipping over a list of the positions of the ring */
  if (ok)
  {
    prev = palloc(sizeof(int) * len);
    next = palloc(sizeof(int) * len);
    tri = palloc(sizeof(int) * 3 * len);
    for (int i = 0; i < len; i++)
    {
      prev[i] = (i + len - 1) % len;
      next[i] = (i + 1) % len;
    }
    int count = len, i = 0, miss = 0;
    while (count > 3)
    {
      if (cover_is_ear(x, y, ring, prev, next, prev[i], i, next[i]))
      {
        tri[3 * ntri] = prev[i];
        tri[3 * ntri + 1] = i;
        tri[3 * ntri + 2] = next[i];
        ntri++;
        next[prev[i]] = next[i];
        prev[next[i]] = prev[i];
        i = next[i];
        count--;
        miss = 0;
        continue;
      }
      i = next[i];
      if (++miss > count)
      {
        ok = false;
        break;
      }
    }
    if (ok && cover_orient(x, y, ring[prev[i]], ring[i], ring[next[i]]) > 0)
    {
      tri[3 * ntri] = prev[i];
      tri[3 * ntri + 1] = i;
      tri[3 * ntri + 2] = next[i];
      ntri++;
    }
    if (ntri == 0)
      ok = false;
  }

  /* Hertel-Mehlhorn: each triangle starts as a piece; the diagonal that cut
   * a triangle off, from its last corner to its first, goes where its two
   * ends stay convex in the union of the two pieces it separates */
  if (ok)
  {
    owner = palloc(sizeof(int) * ntri);
    pn = palloc(sizeof(int) * ntri);
    pv = palloc(sizeof(int *) * ntri);
    for (int t = 0; t < ntri; t++)
    {
      owner[t] = t;
      pn[t] = 3;
      pv[t] = palloc(sizeof(int) * 3);
      memcpy(pv[t], &tri[3 * t], sizeof(int) * 3);
    }
    for (int t = 0; t < ntri; t++)
    {
      int a = tri[3 * t], b = tri[3 * t + 2];
      if (b == (a + 1) % len || a == (b + 1) % len)
        continue;
      /* The triangle on the other side holds the edge from a to b */
      int u = -1;
      for (int s = t + 1; s < ntri && u < 0; s++)
        for (int e = 0; e < 3; e++)
          if (tri[3 * s + e] == a && tri[3 * s + (e + 1) % 3] == b)
          {
            u = s;
            break;
          }
      if (u < 0)
        continue;
      int p1 = cover_find(owner, t), p2 = cover_find(owner, u);
      if (p1 == p2)
        continue;
      int n1 = pn[p1], n2 = pn[p2];
      int ia1 = cover_index_after(pv[p1], n1, b, a);
      int ib2 = cover_index_after(pv[p2], n2, a, b);
      if (ia1 < 0 || ib2 < 0)
        continue;
      int ib1 = (ia1 + n1 - 1) % n1, ia2 = (ib2 + n2 - 1) % n2;
      int a_pred = pv[p2][(ia2 + n2 - 1) % n2], a_succ = pv[p1][(ia1 + 1) % n1];
      int b_pred = pv[p1][(ib1 + n1 - 1) % n1], b_succ = pv[p2][(ib2 + 1) % n2];
      if (cover_orient(x, y, ring[a_pred], ring[a], ring[a_succ]) < 0 ||
          cover_orient(x, y, ring[b_pred], ring[b], ring[b_succ]) < 0)
        continue;
      /* The first piece from a round to b, then the second after b up to
       * the vertex before a */
      int *merged = palloc(sizeof(int) * (n1 + n2 - 2));
      int nm = 0;
      for (int k = 0; k < n1; k++)
        merged[nm++] = pv[p1][(ia1 + k) % n1];
      for (int k = 1; k < n2 - 1; k++)
        merged[nm++] = pv[p2][(ib2 + k) % n2];
      pfree(pv[p1]);
      pv[p1] = merged;
      pn[p1] = nm;
      owner[p2] = p1;
      pn[p2] = 0;
    }

    /* The pieces as bodies */
    *pieces = palloc(sizeof(DistRefPoly) * ntri);
    for (int t = 0; t < ntri && ok; t++)
    {
      if (owner[t] != t)
        continue;
      double *px = palloc(sizeof(double) * pn[t]);
      double *py = palloc(sizeof(double) * pn[t]);
      for (int k = 0; k < pn[t]; k++)
      {
        px[k] = x[ring[pv[t][k]]];
        py[k] = y[ring[pv[t][k]]];
      }
      if (distrefpoly_set_ring(&(*pieces)[*npieces], px, py, pn[t], 0.0,
            0.0))
        (*npieces)++;
      else
        ok = false;
    }
    for (int t = 0; t < ntri; t++)
      pfree(pv[t]);
    pfree(pv); pfree(pn); pfree(owner);
  }

  if (! ok && *pieces)
  {
    for (int k = 0; k < *npieces; k++)
      distrefpoly_free(&(*pieces)[k]);
    pfree(*pieces);
    *pieces = NULL;
    *npieces = 0;
  }
  if (prev) pfree(prev);
  if (next) pfree(next);
  if (tri) pfree(tri);
  if (cand) pfree(cand);
  if (cd) pfree(cd);
  pfree(x); pfree(y); pfree(holes); pfree(hlen); pfree(ring);
  return ok;
}

/*****************************************************************************/
