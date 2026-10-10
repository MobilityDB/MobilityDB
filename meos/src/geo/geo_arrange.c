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
 * @brief The arrangement of closed curves of segments and circular arcs, and
 * the boundary of the region they wind around
 * @details The curves are noded where they meet, and every piece between two
 * nodes is labelled with the winding number of the curves on each side of it,
 * by walking around the nodes as GEOS labels the depth of a buffer curve
 * (`BufferSubgraph::computeDepths`). The pieces across which the winding
 * number passes between zero and one bound the region the curves wind around.
 *
 * Every question is decided exactly, or read in intervals that hold the exact
 * value: where two segments meet and where a segment meets an arc are exact
 * (#linesegm_intersect, #arcsegm_meet_exact), and so is the order of the
 * edges around a node where two of them cross; the order of two meetings
 * along an edge, where two arcs meet, and the order of the edges around a
 * vertex several of them pass through are read in intervals. An interval that
 * does not decide its question leaves the arrangement unanswered -- the
 * caller then answers by another route -- so no tolerance decides anything.
 * A point the arrangement constructs, a crossing interior to two edges, is
 * rounded once and is a node by its identity, never located again.
 */

/* C */
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <string.h>
/* PostgreSQL */
#include "postgres.h"
/* MEOS */
#include <meos.h>
#include "geo/geo_arrange.h"
#include "geo/geo_funcs.h"

/*****************************************************************************
 * Intervals
 *****************************************************************************/

/*
 * A value known to lie in [lo, hi]. Each operation rounds its bounds outward
 * by one step, which covers the rounding to nearest of the operation itself,
 * so the interval of a computed quantity holds its exact value: a comparison
 * the intervals decide is the exact comparison.
 */
typedef struct
{
  double lo;
  double hi;
} Ival;

static inline Ival
ival(double x)
{
  Ival r = {x, x};
  return r;
}

static inline Ival
ival_out(double lo, double hi)
{
  Ival r = {nextafter(lo, -INFINITY), nextafter(hi, INFINITY)};
  return r;
}

static inline Ival
ival_add(Ival a, Ival b)
{
  return ival_out(a.lo + b.lo, a.hi + b.hi);
}

static inline Ival
ival_sub(Ival a, Ival b)
{
  return ival_out(a.lo - b.hi, a.hi - b.lo);
}

static inline Ival
ival_mul(Ival a, Ival b)
{
  double p[4] = {a.lo * b.lo, a.lo * b.hi, a.hi * b.lo, a.hi * b.hi};
  double lo = p[0], hi = p[0];
  for (int i = 1; i < 4; i++)
  {
    lo = fmin(lo, p[i]);
    hi = fmax(hi, p[i]);
  }
  return ival_out(lo, hi);
}

/* A divisor holding zero bounds nothing */
static inline Ival
ival_div(Ival a, Ival b)
{
  if (b.lo <= 0.0 && b.hi >= 0.0)
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  double p[4] = {a.lo / b.lo, a.lo / b.hi, a.hi / b.lo, a.hi / b.hi};
  double lo = p[0], hi = p[0];
  for (int i = 1; i < 4; i++)
  {
    lo = fmin(lo, p[i]);
    hi = fmax(hi, p[i]);
  }
  return ival_out(lo, hi);
}

/* The square root of the non-negative part of an interval */
static inline Ival
ival_sqrt(Ival a)
{
  return ival_out(sqrt(fmax(a.lo, 0.0)), sqrt(fmax(a.hi, 0.0)));
}

static inline Ival
ival_neg(Ival a)
{
  Ival r = {- a.hi, - a.lo};
  return r;
}

/* The sign an interval decides, or 2 where it holds zero and another sign */
static inline int
ival_sign(Ival a)
{
  if (a.lo > 0.0)
    return 1;
  if (a.hi < 0.0)
    return -1;
  if (a.lo == 0.0 && a.hi == 0.0)
    return 0;
  return 2;
}

static inline Ival
ival_cross(Ival ax, Ival ay, Ival bx, Ival by)
{
  return ival_sub(ival_mul(ax, by), ival_mul(ay, bx));
}

static inline Ival
ival_dot(Ival ax, Ival ay, Ival bx, Ival by)
{
  return ival_add(ival_mul(ax, bx), ival_mul(ay, by));
}

/* The union of two intervals */
static inline Ival
ival_hull(Ival a, Ival b)
{
  Ival r = {fmin(a.lo, b.lo), fmax(a.hi, b.hi)};
  return r;
}

/*****************************************************************************
 * The circle of an arc, read in intervals
 *****************************************************************************/

/**
 * @brief Set the centre and the square of the radius of the circle of an
 * arc, as intervals holding them
 * @details The circle is the one the arc's input points fix: through its
 * start, middle and end, or on the diameter from its start to its end. The
 * circumcentre is read from the middle point, so that every square is the
 * size of the arc, as #emit_arc_edge forms the centre it carries
 */
static void
arr_circle(const Edge *arc, Ival *cx, Ival *cy, Ival *r2)
{
  if (arc->diameter)
  {
    *cx = ival_mul(ival_add(ival(arc->sx), ival(arc->ex)), ival(0.5));
    *cy = ival_mul(ival_add(ival(arc->sy), ival(arc->ey)), ival(0.5));
  }
  else
  {
    Ival ax = ival_sub(ival(arc->sx), ival(arc->mx));
    Ival ay = ival_sub(ival(arc->sy), ival(arc->my));
    Ival bx = ival_sub(ival(arc->ex), ival(arc->mx));
    Ival by = ival_sub(ival(arc->ey), ival(arc->my));
    /* With the middle at the origin, the centre is equidistant from A, B and
     * the origin: C = (|A|^2 B' - |B|^2 A') / (2 A x B), X' = (y, -x) */
    Ival a2 = ival_dot(ax, ay, ax, ay), b2 = ival_dot(bx, by, bx, by);
    Ival d = ival_mul(ival(2.0), ival_cross(ax, ay, bx, by));
    Ival nx = ival_sub(ival_mul(a2, by), ival_mul(b2, ay));
    Ival ny = ival_sub(ival_mul(b2, ax), ival_mul(a2, bx));
    *cx = ival_add(ival(arc->mx), ival_div(nx, d));
    *cy = ival_add(ival(arc->my), ival_div(ny, d));
  }
  Ival dx = ival_sub(ival(arc->sx), *cx), dy = ival_sub(ival(arc->sy), *cy);
  *r2 = ival_dot(dx, dy, dx, dy);
  return;
}

/**
 * @brief Return true where an arc turns counterclockwise, decided exactly
 */
static bool
arr_arc_ccw(const Edge *arc)
{
  if (arc->diameter)
    return arc->ccw;
  return cross_product_sign(arc->sx, arc->sy, arc->mx, arc->my, arc->sx,
    arc->sy, arc->ex, arc->ey) > 0;
}

/**
 * @brief Return how far along an arc a point of its circle stands, as an
 * interval of a pseudo-angle that grows with the angle turned from its start
 * in the sense the arc turns
 * @details The pseudo-angle of a direction (u, v) is the diamond angle, which
 * grows monotonically with the angle over [0, 4); u and v are the dot and the
 * cross product of the radius to the start with the radius to the point. A
 * point the arithmetic cannot place on either side of the start is read just
 * after it, since every point asked about stands within the arc
 */
static Ival
arr_arc_position(const Edge *arc, Ival cx, Ival cy, Ival px, Ival py)
{
  Ival sx = ival_sub(ival(arc->sx), cx), sy = ival_sub(ival(arc->sy), cy);
  Ival qx = ival_sub(px, cx), qy = ival_sub(py, cy);
  Ival u = ival_dot(sx, sy, qx, qy);
  Ival v = ival_cross(sx, sy, qx, qy);
  if (! arr_arc_ccw(arc))
    v = ival_neg(v);
  Ival res = {INFINITY, -INFINITY};
  bool any = false;
  /* v >= 0, u >= 0: v / (u + v) in [0, 1] */
  if (v.hi >= 0.0 && u.hi >= 0.0)
  {
    Ival vv = {fmax(v.lo, 0.0), v.hi}, uu = {fmax(u.lo, 0.0), u.hi};
    Ival p = ival_div(vv, ival_add(uu, vv));
    res = ival_hull(res, p); any = true;
  }
  /* v >= 0, u <= 0: 1 + (-u) / (v - u) in [1, 2] */
  if (v.hi >= 0.0 && u.lo <= 0.0)
  {
    Ival vv = {fmax(v.lo, 0.0), v.hi}, uu = {u.lo, fmin(u.hi, 0.0)};
    Ival p = ival_add(ival(1.0), ival_div(ival_neg(uu), ival_sub(vv, uu)));
    res = ival_hull(res, p); any = true;
  }
  /* v <= 0, u <= 0: 2 + (-v) / (-u - v) in [2, 3] */
  if (v.lo <= 0.0 && u.lo <= 0.0 && v.lo < 0.0)
  {
    Ival vv = {v.lo, fmin(v.hi, 0.0)}, uu = {u.lo, fmin(u.hi, 0.0)};
    Ival p = ival_add(ival(2.0), ival_div(ival_neg(vv),
      ival_sub(ival_neg(uu), vv)));
    res = ival_hull(res, p); any = true;
  }
  /* v <= 0, u >= 0: 3 + u / (u - v) in [3, 4), unless it is the start */
  if (v.lo < 0.0 && u.hi >= 0.0 && v.hi < 0.0)
  {
    Ival vv = {v.lo, fmin(v.hi, 0.0)}, uu = {fmax(u.lo, 0.0), u.hi};
    Ival p = ival_add(ival(3.0), ival_div(uu, ival_sub(uu, vv)));
    res = ival_hull(res, p); any = true;
  }
  if (! any)
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  return res;
}

/*****************************************************************************
 * The arrangement
 *****************************************************************************/

/* A node: an input vertex, exact, or a crossing of two edges interior to
 * both, rounded once */
typedef struct
{
  double x, y;
  bool vertex;
  int32_t cross;     /**< The crossing it is, or -1 */
} ArrNode;

/* How the point of a cut arises, as #MeetPoint states it with the indices
 * of the edges */
typedef struct
{
  MeetKind kind;
  uint32_t p, q;
  int sigma;
  double x, y;
} ArrMeet;

/* A cut of an edge at a node interior to it, with its position along the
 * edge as an interval and the point it is, exactly */
typedef struct
{
  uint32_t node;
  Ival pos;
  ArrMeet meet;
} ArrCut;

/* The cuts of one edge */
typedef struct
{
  ArrCut *cuts;
  uint32_t n, cap;
} ArrCuts;

/* A crossing of two edges interior to both, with the sign of the cross
 * product of their directions there, which orders the four pieces around it
 * exactly */
typedef struct
{
  uint32_t node;
  uint32_t a, b;
  int sign;
} ArrCross;

/* A piece of an edge between two consecutive nodes along it */
typedef struct
{
  uint32_t edge;
  uint32_t from, to;
  Ival pfrom, pto;   /**< Positions of its ends along the edge, those of
                          the cuts that end it */
  int left, right;   /**< Winding numbers on each side, INT_MIN where not
                          known yet */
  bool result, flip, used;
} ArrPiece;

/* A piece seen from a node: leaving it along the piece's direction, or
 * arriving at it */
typedef struct
{
  uint32_t piece;
  bool out;
} ArrHalf;

typedef struct
{
  const Edge *e;
  uint32_t n;
  Ival *ccx, *ccy, *cr2;  /**< The circle of each arc edge */
  bool *ccw;              /**< The sense each arc edge turns */
  ArrNode *nodes;
  uint32_t nnodes, capnodes;
  uint32_t *start;        /**< The node each edge starts at */
  uint32_t *end;          /**< The node each edge ends at */
  ArrCuts *cuts;
  ArrCross *cross;
  uint32_t ncross, capcross;
  /* The vertex nodes by their coordinates */
  int32_t *vtab;
  uint32_t vmask;
  ArrPiece *pieces;
  uint32_t npieces;
  ArrHalf *halves;        /**< The halves of every node, node by node */
  uint32_t *hoff;         /**< First half of each node, one past the last */
} Arr;

static uint32_t
arr_node_new(Arr *a, double x, double y, bool vertex, int32_t cross)
{
  if (a->nnodes == a->capnodes)
  {
    a->capnodes *= 2;
    a->nodes = repalloc(a->nodes, sizeof(ArrNode) * a->capnodes);
  }
  ArrNode *nd = &a->nodes[a->nnodes];
  nd->x = x; nd->y = y; nd->vertex = vertex; nd->cross = cross;
  return a->nnodes++;
}

static uint32_t
arr_hash(double x, double y)
{
  uint64_t hx, hy;
  memcpy(&hx, &x, sizeof(uint64_t));
  memcpy(&hy, &y, sizeof(uint64_t));
  uint64_t h = hx * UINT64CONST(0x9E3779B97F4A7C15) ^
    hy * UINT64CONST(0xC2B2AE3D27D4EB4F);
  h ^= h >> 29;
  h *= UINT64CONST(0xBF58476D1CE4E5B9);
  h ^= h >> 32;
  return (uint32_t) h;
}

/* The node of an input vertex, the one node of its coordinates */
static uint32_t
arr_vertex(Arr *a, double x, double y)
{
  uint32_t h = arr_hash(x, y) & a->vmask;
  while (a->vtab[h] >= 0)
  {
    const ArrNode *nd = &a->nodes[a->vtab[h]];
    if (nd->x == x && nd->y == y)
      return (uint32_t) a->vtab[h];
    h = (h + 1) & a->vmask;
  }
  uint32_t id = arr_node_new(a, x, y, true, -1);
  a->vtab[h] = (int32_t) id;
  return id;
}

static void
arr_cut(Arr *a, uint32_t edge, uint32_t node, Ival pos, ArrMeet meet)
{
  ArrCuts *c = &a->cuts[edge];
  if (c->n == c->cap)
  {
    c->cap = c->cap ? 2 * c->cap : 4;
    c->cuts = c->cuts ? repalloc(c->cuts, sizeof(ArrCut) * c->cap) :
      palloc(sizeof(ArrCut) * c->cap);
  }
  c->cuts[c->n].node = node;
  c->cuts[c->n].pos = pos;
  c->cuts[c->n].meet = meet;
  c->n++;
  return;
}

static int32_t
arr_cross_new(Arr *a, uint32_t node, uint32_t ea, uint32_t eb, int sign)
{
  if (a->ncross == a->capcross)
  {
    a->capcross = a->capcross ? 2 * a->capcross : 64;
    a->cross = a->cross ? repalloc(a->cross, sizeof(ArrCross) * a->capcross) :
      palloc(sizeof(ArrCross) * a->capcross);
  }
  ArrCross *c = &a->cross[a->ncross];
  c->node = node; c->a = ea; c->b = eb; c->sign = sign;
  return (int32_t) a->ncross++;
}

static inline bool
arr_is_arc(const Edge *e)
{
  return e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC;
}

/* The position of a point lying exactly on a segment, as an interval of its
 * parameter along it */
static Ival
arr_segment_position(const Edge *s, Ival px, Ival py)
{
  Ival rx = ival_sub(ival(s->x2), ival(s->x1));
  Ival ry = ival_sub(ival(s->y2), ival(s->y1));
  Ival qx = ival_sub(px, ival(s->x1)), qy = ival_sub(py, ival(s->y1));
  return ival_div(ival_dot(qx, qy, rx, ry), ival_dot(rx, ry, rx, ry));
}

static inline bool
arr_same(double x1, double y1, double x2, double y2)
{
  return x1 == x2 && y1 == y2;
}

/* Cut an edge at an input vertex lying within it, unless the vertex is one
 * of its ends */
static void
arr_cut_vertex(Arr *a, uint32_t i, double x, double y)
{
  const Edge *e = &a->e[i];
  if (arr_same(x, y, e->x1, e->y1) || arr_same(x, y, e->x2, e->y2))
    return;
  Ival pos = arr_is_arc(e) ?
    arr_arc_position(e, a->ccx[i], a->ccy[i], ival(x), ival(y)) :
    arr_segment_position(e, ival(x), ival(y));
  ArrMeet m = {MEET_VERTEX, 0, 0, 0, x, y};
  arr_cut(a, i, arr_vertex(a, x, y), pos, m);
  return;
}

/**
 * @brief Node two segments where they meet
 */
static ArrangeStatus
arr_meet_segments(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  IntersectResult r = linesegm_intersect(p->x1, p->y1, p->x2, p->y2, q->x1,
    q->y1, q->x2, q->y2);
  if (r.type == INTERSECT_NONE)
    return ARR_ANSWERED;
  if (r.type == INTERSECT_OVERLAP)
    return ARR_OVERLAP;
  /* An end of either lying on the other is the meeting, collected as the
   * vertex it is */
  const Edge *edges[2] = {p, q};
  const uint32_t ids[2] = {i, j};
  bool vertex = false;
  for (int k = 0; k < 4; k++)
  {
    const Edge *own = edges[k / 2], *other = edges[1 - k / 2];
    double x = (k % 2 == 0) ? own->x1 : own->x2;
    double y = (k % 2 == 0) ? own->y1 : own->y2;
    if (! point_on_segment_exact(x, y, other->x1, other->y1, other->x2,
          other->y2))
      continue;
    vertex = true;
    arr_cut_vertex(a, ids[1 - k / 2], x, y);
  }
  if (vertex)
    return ARR_ANSWERED;
  /* Interior to both: one node, rounded once */
  double x = p->x1 + r.t0 * (p->x2 - p->x1);
  double y = p->y1 + r.t0 * (p->y2 - p->y1);
  int sign = cross_product_sign(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1,
    q->x2, q->y2);
  uint32_t node = arr_node_new(a, x, y, false, -1);
  a->nodes[node].cross = arr_cross_new(a, node, i, j, sign);
  /* The positions along each, t = (C - A) x S / (R x S) */
  Ival ax = ival(p->x1), ay = ival(p->y1), cx = ival(q->x1), cy = ival(q->y1);
  Ival rx = ival_sub(ival(p->x2), ax), ry = ival_sub(ival(p->y2), ay);
  Ival sx = ival_sub(ival(q->x2), cx), sy = ival_sub(ival(q->y2), cy);
  Ival den = ival_cross(rx, ry, sx, sy);
  Ival tp = ival_div(ival_cross(ival_sub(cx, ax), ival_sub(cy, ay), sx, sy),
    den);
  Ival tq = ival_div(ival_cross(ival_sub(cx, ax), ival_sub(cy, ay), rx, ry),
    den);
  ArrMeet m = {MEET_SEGSEG, i, j, 0, 0.0, 0.0};
  arr_cut(a, i, node, tp, m);
  arr_cut(a, j, node, tq, m);
  return ARR_ANSWERED;
}

/**
 * @brief Node a segment and an arc where they meet
 */
static ArrangeStatus
arr_meet_segment_arc(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *s = &a->e[i], *arc = &a->e[j];
  ArcsegMeet m[2];
  int n = arcsegm_meet_exact(s->x1, s->y1, s->x2, s->y2, arc, m);
  if (n < 0)
    return ARR_MEETING;
  for (int k = 0; k < n; k++)
  {
    switch (m[k].kind)
    {
      case ARCSEG_AT_SEGMENT_START:
        arr_cut_vertex(a, j, s->x1, s->y1);
        break;
      case ARCSEG_AT_SEGMENT_END:
        arr_cut_vertex(a, j, s->x2, s->y2);
        break;
      case ARCSEG_AT_ARC_START:
        arr_cut_vertex(a, i, arc->x1, arc->y1);
        break;
      case ARCSEG_AT_ARC_END:
        arr_cut_vertex(a, i, arc->x2, arc->y2);
        break;
      default:
      {
        /* A touch without a crossing is a tangency */
        if (m[k].sigma == 0)
          return ARR_MEETING;
        /* The position along the segment: the root of |A + t R - C|^2 = r^2
         * the meeting is, the smaller or the larger */
        Ival ax = ival(s->x1), ay = ival(s->y1);
        Ival rx = ival_sub(ival(s->x2), ax), ry = ival_sub(ival(s->y2), ay);
        Ival wx = ival_sub(ax, a->ccx[j]), wy = ival_sub(ay, a->ccy[j]);
        Ival qa = ival_dot(rx, ry, rx, ry);
        Ival qb = ival_mul(ival(2.0), ival_dot(wx, wy, rx, ry));
        Ival qc = ival_sub(ival_dot(wx, wy, wx, wy), a->cr2[j]);
        Ival disc = ival_sub(ival_mul(qb, qb), ival_mul(ival(4.0),
          ival_mul(qa, qc)));
        Ival sq = ival_sqrt(disc);
        Ival t = ival_div(m[k].smaller ? ival_sub(ival_neg(qb), sq) :
          ival_add(ival_neg(qb), sq), ival_mul(ival(2.0), qa));
        double x = s->x1 + m[k].t * (s->x2 - s->x1);
        double y = s->y1 + m[k].t * (s->y2 - s->y1);
        /* The tangent of the arc crossed with the direction of the segment
         * has the sign of the root (see #arcsegm_meet_exact): the side the
         * segment enters the circle from. A half circle on a diameter reads
         * the quadratic the other way round when it turns
         * counterclockwise */
        int st = (arc->diameter && a->ccw[j]) ? - m[k].sigma : m[k].sigma;
        uint32_t node = arr_node_new(a, x, y, false, -1);
        a->nodes[node].cross = arr_cross_new(a, node, i, j, - st);
        Ival px = ival_add(ax, ival_mul(t, rx)), py = ival_add(ay,
          ival_mul(t, ry));
        ArrMeet am = {MEET_SEGARC, i, j, m[k].sigma, 0.0, 0.0};
        arr_cut(a, i, node, t, am);
        arr_cut(a, j, node, arr_arc_position(arc, a->ccx[j], a->ccy[j], px,
          py), am);
        break;
      }
    }
  }
  return ARR_ANSWERED;
}

/* The tangent of an arc at a point of it, in the sense it turns, as
 * intervals */
static void
arr_arc_tangent(const Arr *a, uint32_t j, Ival px, Ival py, Ival *tx,
  Ival *ty)
{
  Ival rx = ival_sub(px, a->ccx[j]), ry = ival_sub(py, a->ccy[j]);
  if (a->ccw[j])
  {
    *tx = ival_neg(ry); *ty = rx;
  }
  else
  {
    *tx = ry; *ty = ival_neg(rx);
  }
  return;
}

/* Return 1 where a point of the circle of an arc lies within the arc, 0
 * where it lies beyond it, 2 where the intervals cannot tell: the side of the
 * chord from its start to its end, against the side the arc stands on */
static int
arr_on_arc(const Arr *a, uint32_t j, Ival px, Ival py)
{
  const Edge *arc = &a->e[j];
  Ival cx = ival_sub(ival(arc->ex), ival(arc->sx));
  Ival cy = ival_sub(ival(arc->ey), ival(arc->sy));
  int side = ival_sign(ival_cross(cx, cy, ival_sub(px, ival(arc->sx)),
    ival_sub(py, ival(arc->sy))));
  if (side == 2)
    return 2;
  int want = arc->diameter ? (a->ccw[j] ? -1 : 1) :
    cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey, arc->sx, arc->sy,
      arc->mx, arc->my);
  return side == want ? 1 : 0;
}

/* A crossing of two arcs interior to both, at a point the intervals hold */
static ArrangeStatus
arr_arc_crossing(Arr *a, uint32_t i, uint32_t j, Ival px, Ival py, int sign,
  int sigma)
{
  if (! isfinite(px.lo) || ! isfinite(px.hi) || ! isfinite(py.lo) ||
      ! isfinite(py.hi))
    return ARR_MEETING;
  uint32_t node = arr_node_new(a, 0.5 * (px.lo + px.hi),
    0.5 * (py.lo + py.hi), false, -1);
  a->nodes[node].cross = arr_cross_new(a, node, i, j, sign);
  ArrMeet m = {MEET_ARCARC, i, j, sigma, 0.0, 0.0};
  arr_cut(a, i, node, arr_arc_position(&a->e[i], a->ccx[i], a->ccy[i], px,
    py), m);
  arr_cut(a, j, node, arr_arc_position(&a->e[j], a->ccx[j], a->ccy[j], px,
    py), m);
  return ARR_ANSWERED;
}

/**
 * @brief Node two arcs of distinct circles where the intervals cannot decide
 * how they meet, deciding it exactly (#arcarc_meet_exact)
 * @details The meetings are P = B + sigma f rot(d) on the line of the
 * centres d = C2 - C1; the ends of either arc are noded already, and the
 * exact test reads only points strictly within both. The tangents at P cross
 * with the sign sigma times the senses the arcs turn. The coordinates of a
 * crossing are the intervals', rounded once
 */
static ArrangeStatus
arr_meet_arcs_exact(Arr *a, uint32_t i, uint32_t j)
{
  int meet, within[2][2];
  arcarc_meet_exact(&a->e[i], &a->e[j], &meet, within);
  if (meet < 0)
    return ARR_ANSWERED;
  Ival dx = ival_sub(a->ccx[j], a->ccx[i]), dy = ival_sub(a->ccy[j],
    a->ccy[i]);
  Ival d2 = ival_dot(dx, dy, dx, dy);
  Ival u = ival_div(ival_add(d2, ival_sub(a->cr2[i], a->cr2[j])),
    ival_mul(ival(2.0), d2));
  Ival h2 = ival_sub(a->cr2[i], ival_mul(ival_mul(u, u), d2));
  Ival bx = ival_add(a->ccx[i], ival_mul(u, dx));
  Ival by = ival_add(a->ccy[i], ival_mul(u, dy));
  Ival f = ival_sqrt(ival_div(h2, d2));
  int c12 = (a->ccw[i] ? 1 : -1) * (a->ccw[j] ? 1 : -1);
  for (int r = 0; r < 2; r++)
  {
    if (! within[r][0] || ! within[r][1])
      continue;
    /* Circles touching within both arcs meet without crossing */
    if (meet == 0)
      return ARR_MEETING;
    int sigma = (r == 0) ? -1 : 1;
    Ival px = ival_add(bx, ival_mul(ival(-sigma), ival_mul(f, dy)));
    Ival py = ival_add(by, ival_mul(ival(sigma), ival_mul(f, dx)));
    ArrangeStatus st = arr_arc_crossing(a, i, j, px, py, sigma * c12,
      sigma);
    if (st != ARR_ANSWERED)
      return st;
  }
  return ARR_ANSWERED;
}

/**
 * @brief Node two arcs where they meet
 * @details The meetings of the two circles are read in intervals: the
 * radical line of the two crosses the line of the centres at a point at a
 * known fraction of their distance, and the meetings stand either side of it.
 * A meeting at an end of either arc that lies exactly on the other is that
 * end; any other is a crossing where the intervals place it within both arcs
 * and their order around it, and the arrangement is left unanswered where
 * they cannot.
 */
static ArrangeStatus
arr_meet_arcs(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  /* The ends of either lying on the other: exact, as input vertices on the
   * circle and the side of the chord */
  double ends[4][2] = {{p->x1, p->y1}, {p->x2, p->y2}, {q->x1, q->y1},
    {q->x2, q->y2}};
  int nverts = 0;
  bool inner = false;
  double verts[4][2];
  for (int k = 0; k < 4; k++)
  {
    const Edge *other = (k < 2) ? q : p;
    uint32_t oid = (k < 2) ? j : i;
    double x = ends[k][0], y = ends[k][1];
    if (! point_on_arc_circle(other, x, y))
      continue;
    bool on = arr_same(x, y, other->x1, other->y1) ||
      arr_same(x, y, other->x2, other->y2);
    if (! on)
    {
      int r = arr_on_arc(a, oid, ival(x), ival(y));
      if (r == 2)
        return ARR_MEETING;
      on = (r == 1);
      inner = inner || on;
    }
    if (! on)
      continue;
    bool seen = false;
    for (int v = 0; v < nverts; v++)
      seen = seen || arr_same(verts[v][0], verts[v][1], x, y);
    if (seen)
      continue;
    verts[nverts][0] = x; verts[nverts][1] = y; nverts++;
    arr_cut_vertex(a, oid, x, y);
  }
  /* Two arcs of one circle, decided exactly from their input points, meet
   * only at the ends they share, unless an end of either lies within the
   * other and they run along each other */
  if (arc_same_circle(p, q))
    return inner ? ARR_OVERLAP : ARR_ANSWERED;
  /* The meetings of the two circles */
  Ival dx = ival_sub(a->ccx[j], a->ccx[i]), dy = ival_sub(a->ccy[j],
    a->ccy[i]);
  Ival d2 = ival_dot(dx, dy, dx, dy);
  if (ival_sign(d2) != 1)
  {
    /* Centres the intervals cannot tell apart: two circles whose radii
     * differ by more than the distance of their centres do not meet */
    Ival dr = ival_sub(ival_sqrt(a->cr2[i]), ival_sqrt(a->cr2[j]));
    Ival dr2 = ival_mul(dr, dr);
    if (nverts == 0 && dr2.lo > d2.hi)
      return ARR_ANSWERED;
    return arr_meet_arcs_exact(a, i, j);
  }
  Ival u = ival_div(ival_add(d2, ival_sub(a->cr2[i], a->cr2[j])),
    ival_mul(ival(2.0), d2));
  Ival h2 = ival_sub(a->cr2[i], ival_mul(ival_mul(u, u), d2));
  int sh = ival_sign(h2);
  if (sh == -1)
    return nverts == 0 ? ARR_ANSWERED : ARR_MEETING;
  if (sh != 1)
  {
    /* Two circles through an end the two arcs share, built to touch there:
     * they meet again at its reflection across the line of their centres,
     * placed exactly (#arcarc_shared_point_signs) */
    if (nverts != 1)
      return arr_meet_arcs_exact(a, i, j);
    double jx = verts[0][0], jy = verts[0][1];
    bool endp = arr_same(jx, jy, p->x1, p->y1) || arr_same(jx, jy, p->x2,
      p->y2);
    bool endq = arr_same(jx, jy, q->x1, q->y1) || arr_same(jx, jy, q->x2,
      q->y2);
    if (! endp || ! endq)
      return arr_meet_arcs_exact(a, i, j);
    int s1, sp, sq;
    arcarc_shared_point_signs(p, q, jx, jy, &s1, &sp, &sq);
    if (s1 == 0)
      return ARR_ANSWERED;
    int wantp = p->diameter ? (a->ccw[i] ? -1 : 1) :
      cross_product_sign(p->sx, p->sy, p->ex, p->ey, p->sx, p->sy, p->mx,
        p->my);
    int wantq = q->diameter ? (a->ccw[j] ? -1 : 1) :
      cross_product_sign(q->sx, q->sy, q->ex, q->ey, q->sx, q->sy, q->mx,
        q->my);
    if (- s1 * sp != wantp || - s1 * sq != wantq)
      return ARR_ANSWERED;
    /* Q = J - 2 (d x (J - C1)) / |d|^2 rot(d), rot(d) = (-dy, dx) */
    Ival k = ival_div(ival_mul(ival(2.0), ival_cross(dx, dy,
      ival_sub(ival(jx), a->ccx[i]), ival_sub(ival(jy), a->ccy[i]))), d2);
    Ival qx = ival_add(ival(jx), ival_mul(k, dy));
    Ival qy = ival_sub(ival(jy), ival_mul(k, dx));
    int c1 = a->ccw[i] ? 1 : -1, c2 = a->ccw[j] ? 1 : -1;
    uint32_t node = arr_node_new(a, 0.5 * (qx.lo + qx.hi),
      0.5 * (qy.lo + qy.hi), false, -1);
    a->nodes[node].cross = arr_cross_new(a, node, i, j, - s1 * c1 * c2);
    ArrMeet m = {MEET_ARCARC_REFLECT, i, j, 0, jx, jy};
    arr_cut(a, i, node, arr_arc_position(p, a->ccx[i], a->ccy[i], qx, qy), m);
    arr_cut(a, j, node, arr_arc_position(q, a->ccx[j], a->ccy[j], qx, qy), m);
    return ARR_ANSWERED;
  }
  Ival bx = ival_add(a->ccx[i], ival_mul(u, dx));
  Ival by = ival_add(a->ccy[i], ival_mul(u, dy));
  Ival f = ival_sqrt(ival_div(h2, d2));
  /* The crossings, gathered first: one the intervals cannot decide hands the
   * pair to the exact test, which decides both */
  Ival cpx[2], cpy[2];
  int csign[2], csigma[2], nc = 0;
  for (int side = -1; side <= 1; side += 2)
  {
    /* B + side f (-dy, dx) */
    Ival px = ival_add(bx, ival_mul(ival(-side), ival_mul(f, dy)));
    Ival py = ival_add(by, ival_mul(ival(side), ival_mul(f, dx)));
    /* A meeting holding an end that lies on both is that end */
    int holds = 0;
    for (int v = 0; v < nverts; v++)
      if (px.lo <= verts[v][0] && verts[v][0] <= px.hi &&
          py.lo <= verts[v][1] && verts[v][1] <= py.hi)
        holds++;
    if (holds > 1)
      return arr_meet_arcs_exact(a, i, j);
    if (holds == 1)
      continue;
    int on1 = arr_on_arc(a, i, px, py), on2 = arr_on_arc(a, j, px, py);
    if (on1 == 2 || on2 == 2)
      return arr_meet_arcs_exact(a, i, j);
    if (! on1 || ! on2)
      continue;
    Ival t1x, t1y, t2x, t2y;
    arr_arc_tangent(a, i, px, py, &t1x, &t1y);
    arr_arc_tangent(a, j, px, py, &t2x, &t2y);
    int sign = ival_sign(ival_cross(t1x, t1y, t2x, t2y));
    if (sign == 2 || sign == 0)
      return arr_meet_arcs_exact(a, i, j);
    cpx[nc] = px; cpy[nc] = py; csign[nc] = sign; csigma[nc] = side; nc++;
  }
  for (int c = 0; c < nc; c++)
  {
    ArrangeStatus st = arr_arc_crossing(a, i, j, cpx[c], cpy[c], csign[c],
      csigma[c]);
    if (st != ARR_ANSWERED)
      return st;
  }
  return ARR_ANSWERED;
}

/* The box of an edge, widened for an arc by the rounding of the box its
 * extremes are read from: a box that only admits candidates */
static void
arr_box(const Edge *e, double box[4])
{
  if (! arr_is_arc(e))
  {
    box[0] = Min(e->x1, e->x2); box[1] = Max(e->x1, e->x2);
    box[2] = Min(e->y1, e->y2); box[3] = Max(e->y1, e->y2);
    return;
  }
  double pad = 16.0 * DBL_EPSILON * (fabs(e->cx) + fabs(e->cy) + e->radius);
  box[0] = nextafter(e->xmin - pad, -INFINITY);
  box[1] = nextafter(e->xmax + pad, INFINITY);
  box[2] = nextafter(e->ymin - pad, -INFINITY);
  box[3] = nextafter(e->ymax + pad, INFINITY);
  return;
}

typedef struct
{
  double xmin;
  uint32_t id;
} ArrSweep;

static int
arr_sweep_cmp(const void *a, const void *b)
{
  const ArrSweep *p = a, *q = b;
  if (p->xmin < q->xmin)
    return -1;
  if (p->xmin > q->xmin)
    return 1;
  return (p->id < q->id) ? -1 : (p->id > q->id) ? 1 : 0;
}

/**
 * @brief Node every pair of edges whose boxes meet
 */
static ArrangeStatus
arr_node_all(Arr *a)
{
  uint32_t n = a->n;
  double (*box)[4] = palloc(sizeof(double[4]) * n);
  ArrSweep *sw = palloc(sizeof(ArrSweep) * n);
  for (uint32_t i = 0; i < n; i++)
  {
    arr_box(&a->e[i], box[i]);
    sw[i].xmin = box[i][0];
    sw[i].id = i;
  }
  qsort(sw, n, sizeof(ArrSweep), arr_sweep_cmp);
  ArrangeStatus st = ARR_ANSWERED;
  for (uint32_t k = 0; k < n && st == ARR_ANSWERED; k++)
  {
    uint32_t i = sw[k].id;
    for (uint32_t l = k + 1; l < n && st == ARR_ANSWERED; l++)
    {
      uint32_t j = sw[l].id;
      if (box[j][0] > box[i][1])
        break;
      if (box[j][2] > box[i][3] || box[i][2] > box[j][3])
        continue;
      bool ai = arr_is_arc(&a->e[i]), aj = arr_is_arc(&a->e[j]);
      if (! ai && ! aj)
        st = arr_meet_segments(a, i, j);
      else if (! ai)
        st = arr_meet_segment_arc(a, i, j);
      else if (! aj)
        st = arr_meet_segment_arc(a, j, i);
      else
        st = arr_meet_arcs(a, i, j);
    }
  }
  pfree(box); pfree(sw);
  return st;
}

static int
arr_cut_cmp(const void *a, const void *b)
{
  const ArrCut *p = a, *q = b;
  if (p->pos.lo < q->pos.lo)
    return -1;
  if (p->pos.lo > q->pos.lo)
    return 1;
  return (p->node < q->node) ? -1 : (p->node > q->node) ? 1 : 0;
}

/* The meeting of a cut as #meets_order_exact reads it */
static MeetPoint
arr_meet_point(const Arr *a, const ArrMeet *m)
{
  MeetPoint r;
  r.kind = m->kind;
  r.p = (m->kind == MEET_VERTEX) ? NULL : &a->e[m->p];
  r.q = (m->kind == MEET_VERTEX) ? NULL : &a->e[m->q];
  r.sigma = m->sigma;
  r.x = m->x; r.y = m->y;
  return r;
}

/* Return how two cuts of an edge are ordered along it: by their intervals,
 * and where those overlap by #meets_order_exact; 0 for one node, 2 for two
 * nodes at one point or an order that cannot be told */
static int
arr_cut_order(const Arr *a, uint32_t edge, const ArrCut *c1,
  const ArrCut *c2)
{
  if (c1->node == c2->node)
    return 0;
  if (c1->pos.hi < c2->pos.lo)
    return -1;
  if (c2->pos.hi < c1->pos.lo)
    return 1;
  MeetPoint m1 = arr_meet_point(a, &c1->meet);
  MeetPoint m2 = arr_meet_point(a, &c2->meet);
  int c = meets_order_exact(&a->e[edge], a->ccw[edge], &m1, &m2);
  return (c == 0) ? 2 : c;
}

/**
 * @brief Cut every edge into pieces at its nodes, in their order along it
 * @details A node cut twice into one edge is one cut. Two cuts at distinct
 * nodes whose intervals overlap are ordered exactly (#meets_order_exact);
 * two distinct nodes at one point, or an order that cannot be told, leave
 * the arrangement unanswered
 */
static ArrangeStatus
arr_pieces(Arr *a)
{
  uint32_t cap = a->n;
  for (uint32_t i = 0; i < a->n; i++)
    cap += a->cuts[i].n;
  a->pieces = palloc(sizeof(ArrPiece) * Max(cap, 1u));
  a->npieces = 0;
  for (uint32_t i = 0; i < a->n; i++)
  {
    ArrCuts *c = &a->cuts[i];
    if (c->n > 1)
      qsort(c->cuts, c->n, sizeof(ArrCut), arr_cut_cmp);
    /* Cuts whose intervals overlap are put in their exact order: the sort
     * by the lower ends leaves each within its run of overlapping ones */
    for (uint32_t k = 1; k < c->n; k++)
      for (uint32_t m = k; m > 0; m--)
      {
        int o = arr_cut_order(a, i, &c->cuts[m - 1], &c->cuts[m]);
        if (o == 2)
          return ARR_ORDER;
        if (o <= 0)
          break;
        ArrCut t = c->cuts[m]; c->cuts[m] = c->cuts[m - 1];
        c->cuts[m - 1] = t;
      }
    uint32_t from = a->start[i], end = a->end[i];
    /* A segment runs from 0 to 1 and an arc from 0 on, see
     * #arr_arc_position */
    Ival pfrom = ival(0.0);
    Ival pend = ival(arr_is_arc(&a->e[i]) ? INFINITY : 1.0);
    const ArrCut *prev = NULL;
    for (uint32_t k = 0; k < c->n; k++)
    {
      const ArrCut *cut = &c->cuts[k];
      if (prev && cut->node == prev->node)
        continue;
      ArrPiece *p = &a->pieces[a->npieces++];
      p->edge = i; p->from = from; p->to = cut->node;
      p->pfrom = pfrom; p->pto = cut->pos;
      p->left = p->right = INT_MIN;
      p->result = p->flip = p->used = false;
      from = cut->node;
      pfrom = cut->pos;
      prev = cut;
    }
    ArrPiece *p = &a->pieces[a->npieces++];
    p->edge = i; p->from = from; p->to = end;
    p->pfrom = pfrom; p->pto = pend;
    p->left = p->right = INT_MIN;
    p->result = p->flip = p->used = false;
  }
  return ARR_ANSWERED;
}

/* The direction a half leaves its node in, as intervals */
static void
arr_half_direction(const Arr *a, const ArrHalf *h, uint32_t node, Ival *dx,
  Ival *dy)
{
  const ArrPiece *p = &a->pieces[h->piece];
  const Edge *e = &a->e[p->edge];
  if (arr_is_arc(e))
  {
    const ArrNode *nd = &a->nodes[node];
    arr_arc_tangent(a, p->edge, ival(nd->x), ival(nd->y), dx, dy);
  }
  else
  {
    *dx = ival_sub(ival(e->x2), ival(e->x1));
    *dy = ival_sub(ival(e->y2), ival(e->y1));
  }
  if (! h->out)
  {
    *dx = ival_neg(*dx);
    *dy = ival_neg(*dy);
  }
  return;
}

/* The half-plane of a direction: 0 for an angle in [0, pi), 1 for [pi, 2pi),
 * 2 where the intervals cannot tell */
static int
arr_half_plane(Ival dx, Ival dy)
{
  int sy = ival_sign(dy);
  if (sy == 1)
    return 0;
  if (sy == -1)
    return 1;
  if (sy == 0)
  {
    int sx = ival_sign(dx);
    return sx == 1 ? 0 : (sx == -1 ? 1 : 2);
  }
  return 2;
}

/* Return -1 where direction a comes before b counterclockwise from the
 * positive x axis, 1 where after, 2 where the intervals cannot tell */
static int
arr_turn_cmp(Ival ax, Ival ay, Ival bx, Ival by)
{
  int ha = arr_half_plane(ax, ay), hb = arr_half_plane(bx, by);
  if (ha == 2 || hb == 2)
    return 2;
  if (ha != hb)
    return ha < hb ? -1 : 1;
  int s = ival_sign(ival_cross(ax, ay, bx, by));
  if (s == 1)
    return -1;
  if (s == -1)
    return 1;
  return 2;
}

/* Return how two halves of a vertex, or a half and the positive x axis
 * (NULL), are ordered counterclockwise from that axis: #arr_turn_cmp on the
 * intervals of their directions, and #edges_turn_cmp_exact where those
 * cannot tell, as for the directions of an offset segment and the join arc
 * tangent to it */
static int
arr_half_cmp(const Arr *a, uint32_t v, const ArrHalf *h1, Ival d1x,
  Ival d1y, const ArrHalf *h2, Ival d2x, Ival d2y)
{
  int c = arr_turn_cmp(d1x, d1y, d2x, d2y);
  if (c != 2)
    return c;
  const ArrNode *nd = &a->nodes[v];
  uint32_t e1 = h1 ? a->pieces[h1->piece].edge : 0;
  uint32_t e2 = h2 ? a->pieces[h2->piece].edge : 0;
  return edges_turn_cmp_exact(h1 ? &a->e[e1] : NULL, h1 && a->ccw[e1],
    h1 && h1->out, h2 ? &a->e[e2] : NULL, h2 && a->ccw[e2], h2 && h2->out,
    nd->x, nd->y);
}

/**
 * @brief Gather the halves of every node and order them counterclockwise
 */
static ArrangeStatus
arr_order_nodes(Arr *a)
{
  uint32_t nn = a->nnodes;
  a->hoff = palloc0(sizeof(uint32_t) * (nn + 1));
  for (uint32_t k = 0; k < a->npieces; k++)
  {
    a->hoff[a->pieces[k].from + 1]++;
    a->hoff[a->pieces[k].to + 1]++;
  }
  for (uint32_t v = 0; v < nn; v++)
    a->hoff[v + 1] += a->hoff[v];
  a->halves = palloc(sizeof(ArrHalf) * Max(a->hoff[nn], 1u));
  uint32_t *fill = palloc(sizeof(uint32_t) * Max(nn, 1u));
  memcpy(fill, a->hoff, sizeof(uint32_t) * nn);
  for (uint32_t k = 0; k < a->npieces; k++)
  {
    ArrHalf ho = {k, true}, hi = {k, false};
    a->halves[fill[a->pieces[k].from]++] = ho;
    a->halves[fill[a->pieces[k].to]++] = hi;
  }
  pfree(fill);
  for (uint32_t v = 0; v < nn; v++)
  {
    ArrHalf *h = &a->halves[a->hoff[v]];
    uint32_t deg = a->hoff[v + 1] - a->hoff[v];
    if (deg <= 2)
      continue;
    const ArrNode *nd = &a->nodes[v];
    if (nd->cross >= 0)
    {
      /* A crossing: the two pieces of each edge, ordered by the side the
       * second edge crosses the first from */
      const ArrCross *c = &a->cross[nd->cross];
      if (deg != 4)
        return ARR_TURN;
      ArrHalf ao = {0, true}, ai = ao, bo = ao, bi = ao;
      int found = 0;
      for (uint32_t k = 0; k < 4; k++)
      {
        uint32_t edge = a->pieces[h[k].piece].edge;
        if (edge == c->a)
        {
          if (h[k].out) ao = h[k]; else ai = h[k];
          found |= h[k].out ? 1 : 2;
        }
        else if (edge == c->b)
        {
          if (h[k].out) bo = h[k]; else bi = h[k];
          found |= h[k].out ? 4 : 8;
        }
      }
      if (found != 15)
        return ARR_TURN;
      if (c->sign > 0)
      {
        h[0] = ao; h[1] = bo; h[2] = ai; h[3] = bi;
      }
      else
      {
        h[0] = ao; h[1] = bi; h[2] = ai; h[3] = bo;
      }
      continue;
    }
    /* A vertex several edges pass through: by their directions */
    Ival *dx = palloc(sizeof(Ival) * deg), *dy = palloc(sizeof(Ival) * deg);
    for (uint32_t k = 0; k < deg; k++)
      arr_half_direction(a, &h[k], v, &dx[k], &dy[k]);
    for (uint32_t k = 1; k < deg; k++)
      for (uint32_t m = k; m > 0; m--)
      {
        int c = arr_half_cmp(a, v, &h[m - 1], dx[m - 1], dy[m - 1], &h[m],
          dx[m], dy[m]);
        if (c == 2)
        {
          pfree(dx); pfree(dy);
          return ARR_TURN;
        }
        if (c < 0)
          break;
        ArrHalf th = h[m]; h[m] = h[m - 1]; h[m - 1] = th;
        Ival tx = dx[m]; dx[m] = dx[m - 1]; dx[m - 1] = tx;
        Ival ty = dy[m]; dy[m] = dy[m - 1]; dy[m - 1] = ty;
      }
    pfree(dx); pfree(dy);
  }
  return ARR_ANSWERED;
}

/*
 * The winding number of a piece rises by one from its right to its left, the
 * side the curves wind around, so each piece is labelled by either side, and
 * a node passes its labels around it: turning counterclockwise past a half
 * leaving the node crosses its piece from right to left, past one arriving
 * from left to right.
 */

/* Label a piece by the winding number on its right; false where it carries
 * another */
static bool
arr_label(ArrPiece *p, int right)
{
  if (p->right != INT_MIN)
    return p->right == right;
  p->right = right;
  p->left = right + 1;
  return true;
}

/* The winding number of the sector after the k-th half of a node */
static inline int
arr_sector_after(const ArrPiece *p, const ArrHalf *h)
{
  return h->out ? p->left : p->right;
}

/**
 * @brief Pass the labels around a node from a piece of it that carries one
 */
static bool
arr_label_node(Arr *a, uint32_t v, uint32_t *queue, uint32_t *qn,
  bool *queued)
{
  ArrHalf *h = &a->halves[a->hoff[v]];
  uint32_t deg = a->hoff[v + 1] - a->hoff[v];
  uint32_t k0 = UINT32_MAX;
  for (uint32_t k = 0; k < deg && k0 == UINT32_MAX; k++)
    if (a->pieces[h[k].piece].right != INT_MIN)
      k0 = k;
  if (k0 == UINT32_MAX)
    return true;
  int w = arr_sector_after(&a->pieces[h[k0].piece], &h[k0]);
  for (uint32_t s = 1; s <= deg; s++)
  {
    const ArrHalf *hk = &h[(k0 + s) % deg];
    ArrPiece *p = &a->pieces[hk->piece];
    /* The sector before this half is w: its right where it leaves, its left
     * where it arrives */
    if (! arr_label(p, hk->out ? w : w - 1))
      return false;
    w = arr_sector_after(p, hk);
    uint32_t other = hk->out ? p->to : p->from;
    if (! queued[other])
    {
      queued[other] = true;
      queue[(*qn)++] = other;
    }
  }
  return true;
}

/*
 * The rightmost point of the pieces of a component, where the side facing
 * the positive x axis is the outside of the component: an input vertex, or
 * the point of an arc's circle furthest along x where the arc passes it.
 */
typedef struct
{
  bool on_arc;        /**< The rightmost point is interior to an arc */
  uint32_t node;      /**< Else the vertex */
  uint32_t piece;     /**< The arc piece holding it */
  Ival x;
} ArrExtreme;

/* Return 1 where the piece of an arc holds the point of its circle furthest
 * along x, 0 where it does not, 2 where the intervals cannot tell */
static int
arr_piece_holds_right(const Arr *a, const ArrPiece *p, Ival *x)
{
  uint32_t j = p->edge;
  const Edge *e = &a->e[j];
  Ival r = ival_sqrt(a->cr2[j]);
  Ival px = ival_add(a->ccx[j], r), py = a->ccy[j];
  *x = px;
  int on = arr_on_arc(a, j, px, py);
  if (on != 1)
    return on;
  /* Within the edge: between the positions of the piece's two ends */
  Ival pos = arr_arc_position(e, a->ccx[j], a->ccy[j], px, py);
  if (pos.lo > p->pfrom.hi && pos.hi < p->pto.lo)
    return 1;
  if (pos.hi < p->pfrom.lo || pos.lo > p->pto.hi)
    return 0;
  return 2;
}

/**
 * @brief Return the winding number of the region around a point, counted by
 * the labelled pieces a ray from it along the positive x axis crosses
 * @return false where the intervals cannot tell a crossing
 */
static bool
arr_winding_at(const Arr *a, Ival px, double py, int *w)
{
  *w = 0;
  for (uint32_t k = 0; k < a->npieces; k++)
  {
    const ArrPiece *p = &a->pieces[k];
    if (p->right == INT_MIN)
      continue;
    /* The ray must cross a piece within it: one through an end of it is
     * not counted by a half-open rule here, and the arrangement is left
     * unanswered */
    const Edge *e = &a->e[p->edge];
    if (! arr_is_arc(e))
    {
      /* The line of the segment meets the ray right of the point where the
       * point stands left of an upward segment or right of a downward one */
      if (e->y1 == e->y2)
      {
        if (e->y1 == py)
          return false;
        continue;
      }
      bool up = e->y2 > e->y1;
      if ((py < e->y1 && py < e->y2) || (py > e->y1 && py > e->y2))
        continue;
      Ival s = ival_cross(ival_sub(ival(e->x2), ival(e->x1)),
        ival_sub(ival(e->y2), ival(e->y1)), ival_sub(px, ival(e->x1)),
        ival_sub(ival(py), ival(e->y1)));
      int ss = ival_sign(s);
      if (ss == 2 || ss == 0)
        return false;
      if (up ? ss < 0 : ss > 0)
        continue;
      /* Within the piece: the parameter of the meeting along the segment
       * between those of the piece's ends */
      Ival t = ival_div(ival_sub(ival(py), ival(e->y1)),
        ival_sub(ival(e->y2), ival(e->y1)));
      if (t.hi < p->pfrom.lo || t.lo > p->pto.hi)
        continue;
      if (! (t.lo > p->pfrom.hi && t.hi < p->pto.lo))
        return false;
      *w += up ? 1 : -1;
      continue;
    }
    /* An arc: the points of its circle at height py, right of the point and
     * within the piece */
    uint32_t j = p->edge;
    Ival dy = ival_sub(ival(py), a->ccy[j]);
    Ival h2 = ival_sub(a->cr2[j], ival_mul(dy, dy));
    int sh = ival_sign(h2);
    if (sh == -1)
      continue;
    if (sh != 1)
      return false;
    Ival hx = ival_sqrt(h2);
    for (int side = -1; side <= 1; side += 2)
    {
      Ival qx = ival_add(a->ccx[j], side < 0 ? ival_neg(hx) : hx);
      int right = ival_sign(ival_sub(qx, px));
      if (right == 2 || right == 0)
        return false;
      if (right < 0)
        continue;
      /* Within the piece */
      Ival pos = arr_arc_position(e, a->ccx[j], a->ccy[j], qx, ival(py));
      int onarc = arr_on_arc(a, j, qx, ival(py));
      if (onarc == 2)
        return false;
      if (onarc == 0)
        continue;
      if (pos.hi < p->pfrom.lo || pos.lo > p->pto.hi)
        continue;
      if (! (pos.lo > p->pfrom.hi && pos.hi < p->pto.lo))
        return false;
      /* Upward on the right of a counterclockwise circle */
      bool up = (side > 0) == a->ccw[j];
      *w += up ? 1 : -1;
    }
  }
  return true;
}

/**
 * @brief Label every piece with the winding numbers on each side, component
 * by component, each from its rightmost point
 */
static ArrangeStatus
arr_label_all(Arr *a, uint32_t *outer_piece)
{
  uint32_t nn = a->nnodes;
  bool *queued = palloc0(sizeof(bool) * Max(nn, 1u));
  uint32_t *queue = palloc(sizeof(uint32_t) * Max(nn, 1u));
  ArrangeStatus st = ARR_ANSWERED;
  *outer_piece = UINT32_MAX;
  bool first = true;
  for (;;)
  {
    /* The component of the first piece not labelled yet */
    uint32_t seed = UINT32_MAX;
    for (uint32_t k = 0; k < a->npieces && seed == UINT32_MAX; k++)
      if (a->pieces[k].right == INT_MIN)
        seed = k;
    if (seed == UINT32_MAX)
      break;
    /* Its nodes and pieces, by walking it */
    uint32_t qn = 0;
    uint32_t *comp = palloc(sizeof(uint32_t) * Max(nn, 1u));
    bool *incomp = palloc0(sizeof(bool) * Max(nn, 1u));
    uint32_t cn = 0;
    comp[cn++] = a->pieces[seed].from;
    incomp[a->pieces[seed].from] = true;
    for (uint32_t c = 0; c < cn; c++)
    {
      uint32_t v = comp[c];
      for (uint32_t k = a->hoff[v]; k < a->hoff[v + 1]; k++)
      {
        const ArrPiece *p = &a->pieces[a->halves[k].piece];
        uint32_t o = a->halves[k].out ? p->to : p->from;
        if (! incomp[o])
        {
          incomp[o] = true;
          comp[cn++] = o;
        }
      }
    }
    /* Its rightmost point */
    ArrExtreme ex = {false, UINT32_MAX, UINT32_MAX, {-INFINITY, -INFINITY}};
    bool unsure = false;
    for (uint32_t c = 0; c < cn; c++)
    {
      const ArrNode *nd = &a->nodes[comp[c]];
      if (! nd->vertex)
        continue;
      if (ex.node == UINT32_MAX || nd->x > ex.x.lo)
      {
        ex.node = comp[c];
        ex.x = ival(nd->x);
      }
    }
    for (uint32_t c = 0; c < cn && ! unsure; c++)
    {
      uint32_t v = comp[c];
      for (uint32_t k = a->hoff[v]; k < a->hoff[v + 1]; k++)
      {
        if (! a->halves[k].out)
          continue;
        const ArrPiece *p = &a->pieces[a->halves[k].piece];
        if (! arr_is_arc(&a->e[p->edge]))
          continue;
        Ival x;
        int holds = arr_piece_holds_right(a, p, &x);
        if (holds == 0)
          continue;
        /* A vertex on the circle of the arc, an end of its edge, stands left
         * of the point of the circle furthest along x where the piece holds
         * that point strictly within it, however close the intervals put
         * them */
        const Edge *ae = &a->e[p->edge];
        const ArrNode *xn = &a->nodes[ex.node];
        if (holds == 1 && ! ex.on_arc &&
            (arr_same(xn->x, xn->y, ae->x1, ae->y1) ||
             arr_same(xn->x, xn->y, ae->x2, ae->y2)))
        {
          ex.on_arc = true;
          ex.piece = a->halves[k].piece;
          ex.x = x;
          continue;
        }
        if (holds == 2 || (x.hi >= ex.x.lo && x.lo <= ex.x.hi))
        {
          if (holds == 2 && x.hi < ex.x.lo)
            continue;
          unsure = true;
          break;
        }
        if (x.lo > ex.x.hi)
        {
          ex.on_arc = true;
          ex.piece = a->halves[k].piece;
          ex.x = x;
        }
      }
    }
    if (unsure)
    {
      st = ARR_DEPTH;
      pfree(comp); pfree(incomp);
      break;
    }
    /* The winding number outside the component: zero for the first, the one
     * the components labelled before give its rightmost point otherwise */
    int outside = 0;
    if (! first)
    {
      /* The ray counts crossings from a point it holds exactly: the
       * rightmost vertex, whose face of the components labelled before is
       * the one the whole component lies in, as it meets none of them */
      Ival px = ival(a->nodes[ex.node].x);
      double py = a->nodes[ex.node].y;
      if (! arr_winding_at(a, px, py, &outside))
      {
        st = ARR_DEPTH;
        pfree(comp); pfree(incomp);
        break;
      }
    }
    /* Seed the labels at the rightmost point */
    if (ex.on_arc)
    {
      /* At the rightmost point of its circle an arc runs upward where it
       * turns counterclockwise, so the outside is on its right */
      ArrPiece *p = &a->pieces[ex.piece];
      bool ccw = a->ccw[p->edge];
      arr_label(p, ccw ? outside : outside - 1);
      if (first)
        *outer_piece = ex.piece;
      uint32_t ends[2] = {p->from, p->to};
      for (int k = 0; k < 2; k++)
        if (! queued[ends[k]])
        {
          queued[ends[k]] = true;
          queue[qn++] = ends[k];
        }
    }
    else
    {
      /* The sector of the rightmost vertex holding the positive x axis */
      uint32_t v = ex.node;
      ArrHalf *h = &a->halves[a->hoff[v]];
      uint32_t deg = a->hoff[v + 1] - a->hoff[v];
      Ival ux = ival(1.0), uy = ival(0.0);
      uint32_t before = UINT32_MAX;
      for (uint32_t k = 0; k < deg; k++)
      {
        Ival dx, dy, ex2, ey2;
        arr_half_direction(a, &h[k], v, &dx, &dy);
        arr_half_direction(a, &h[(k + 1) % deg], v, &ex2, &ey2);
        const ArrHalf *hn = &h[(k + 1) % deg];
        int c1 = arr_half_cmp(a, v, &h[k], dx, dy, NULL, ux, uy);
        int c2 = arr_half_cmp(a, v, NULL, ux, uy, hn, ex2, ey2);
        int c3 = arr_half_cmp(a, v, &h[k], dx, dy, hn, ex2, ey2);
        if (c1 == 2 || c2 == 2 || (deg > 1 && c3 == 2))
        {
          before = UINT32_MAX;
          break;
        }
        /* The axis lies in the sector from half k to half k + 1 */
        bool in = (deg == 1) || (c3 < 0 ? (c1 < 0 && c2 < 0) :
          (c1 < 0 || c2 < 0));
        if (in)
        {
          before = k;
          break;
        }
      }
      if (before == UINT32_MAX)
      {
        st = ARR_DEPTH;
        pfree(comp); pfree(incomp);
        break;
      }
      ArrPiece *p = &a->pieces[h[before].piece];
      /* The sector after half k is its left where it leaves, its right
       * where it arrives */
      arr_label(p, h[before].out ? outside - 1 : outside);
      if (first)
        *outer_piece = h[before].piece;
      if (! queued[v])
      {
        queued[v] = true;
        queue[qn++] = v;
      }
    }
    for (uint32_t q = 0; q < qn && st == ARR_ANSWERED; q++)
      if (! arr_label_node(a, queue[q], queue, &qn, queued))
        st = ARR_DEPTH;
    pfree(comp); pfree(incomp);
    if (st != ARR_ANSWERED)
      break;
    first = false;
  }
  pfree(queued); pfree(queue);
  return st;
}

/* The piece of an edge between two nodes, as an Edge in the direction it is
 * walked */
static void
arr_piece_edge(const Arr *a, const ArrPiece *p, Edge *out)
{
  const Edge *e = &a->e[p->edge];
  const ArrNode *n1 = &a->nodes[p->flip ? p->to : p->from];
  const ArrNode *n2 = &a->nodes[p->flip ? p->from : p->to];
  memset(out, 0, sizeof(Edge));
  out->x1 = n1->x; out->y1 = n1->y;
  out->x2 = n2->x; out->y2 = n2->y;
  if (arr_is_arc(e))
  {
    out->etype = EDGE_POLYARC;
    out->cx = e->cx; out->cy = e->cy; out->radius = e->radius;
    out->theta0 = atan2(out->y1 - e->cy, out->x1 - e->cx);
    out->theta1 = atan2(out->y2 - e->cy, out->x2 - e->cx);
    out->ccw = p->flip ? ! a->ccw[p->edge] : a->ccw[p->edge];
  }
  else
    out->etype = EDGE_POLYSEG;
  return;
}

/**
 * @brief Link the pieces that bound the region into rings
 * @details A piece bounds the region where the winding number is at least
 * one on one side of it and not on the other, and is walked with the region
 * on its left. At a node the walk takes the first piece leaving it clockwise
 * from the one it arrives along, which keeps the region on its left and
 * closes the smallest ring
 */
static ArrangeStatus
arr_rings(Arr *a, int sgn, uint32_t outer_piece, MeosArray ***rings,
  uint32_t *nrings)
{
  for (uint32_t k = 0; k < a->npieces; k++)
  {
    ArrPiece *p = &a->pieces[k];
    bool inl = sgn * p->left >= 1, inr = sgn * p->right >= 1;
    p->result = inl != inr;
    p->flip = inr;
  }
  if (outer_piece == UINT32_MAX || ! a->pieces[outer_piece].result)
    return ARR_SHELLS;
  uint32_t cap = 4, n = 0;
  MeosArray **out = palloc(sizeof(MeosArray *) * cap);
  /* The ring through the outermost piece first: it is the shell */
  uint32_t order0 = outer_piece;
  for (uint32_t pass = 0; pass <= a->npieces; pass++)
  {
    uint32_t k = (pass == 0) ? order0 : pass - 1;
    if (k >= a->npieces || ! a->pieces[k].result || a->pieces[k].used)
      continue;
    MeosArray *ring = meos_array_create(sizeof(Edge));
    uint32_t cur = k;
    for (;;)
    {
      ArrPiece *p = &a->pieces[cur];
      p->used = true;
      Edge pe;
      arr_piece_edge(a, p, &pe);
      if (pe.x1 != pe.x2 || pe.y1 != pe.y2)
        meos_array_add(ring, &pe);
      uint32_t v = p->flip ? p->from : p->to;
      ArrHalf *h = &a->halves[a->hoff[v]];
      uint32_t deg = a->hoff[v + 1] - a->hoff[v];
      /* The half of the piece arriving here, then clockwise */
      uint32_t at = UINT32_MAX;
      for (uint32_t m = 0; m < deg && at == UINT32_MAX; m++)
        if (h[m].piece == cur && h[m].out == p->flip)
          at = m;
      uint32_t next = UINT32_MAX;
      for (uint32_t s = 1; s < deg && next == UINT32_MAX && at != UINT32_MAX;
           s++)
      {
        const ArrHalf *hm = &h[(at + deg - s) % deg];
        const ArrPiece *q = &a->pieces[hm->piece];
        if (q->result && hm->out != q->flip)
          next = hm->piece;
      }
      if (next == UINT32_MAX)
      {
        meos_array_destroy(ring);
        arrange_rings_free(out, n);
        return ARR_DEPTH;
      }
      if (next == k)
        break;
      if (a->pieces[next].used)
      {
        meos_array_destroy(ring);
        arrange_rings_free(out, n);
        return ARR_DEPTH;
      }
      cur = next;
    }
    /* A ring every piece of which has ends written at one point bounds
     * nothing the coordinates can state */
    if (ring->count == 0)
    {
      meos_array_destroy(ring);
      continue;
    }
    if (n == cap)
    {
      cap *= 2;
      out = repalloc(out, sizeof(MeosArray *) * cap);
    }
    out[n++] = ring;
  }
  *rings = out;
  *nrings = n;
  return ARR_ANSWERED;
}

/**
 * @brief Release the rings an arrangement answers
 */
void
arrange_rings_free(MeosArray **rings, uint32_t nrings)
{
  if (! rings)
    return;
  for (uint32_t i = 0; i < nrings; i++)
    meos_array_destroy(rings[i]);
  pfree(rings);
  return;
}

/**
 * @brief Return the rings bounding the region closed curves wind around
 * @details The curves are given as their edges, as #geom_extract_edges emits
 * those of the rings of a curve polygon: segments and arcs, each ring's edges
 * consecutive, each starting exactly where the one before ends, and the last
 * ending where the first starts. The region is the set of points the curves
 * wind around at least once, in the sense the outermost curve turns. The
 * first ring answered is the one bounding it from outside, turning
 * counterclockwise; the others are its holes, turning clockwise. Each ring is
 * an array of pieces (#Edge), segments and arcs, in the order walked.
 * @param[in] edges The edges of the curves
 * @param[out] rings,nrings The rings, released by #arrange_rings_free
 * @return #ARR_ANSWERED, or the question left unanswered
 */
ArrangeStatus
arrange_wound_boundary(const MeosArray *edges, MeosArray ***rings,
  uint32_t *nrings)
{
  assert(edges); assert(rings); assert(nrings);
  *rings = NULL;
  *nrings = 0;
  uint32_t n = edges->count;
  if (n == 0)
    return ARR_NOT_CLOSED;
  Arr a;
  memset(&a, 0, sizeof(Arr));
  a.e = (const Edge *) edges->elems;
  a.n = n;
  /* Every edge a segment of some length or an arc whose ends are its
   * defining points, and every ring closed on itself */
  for (uint32_t i = 0; i < n; i++)
  {
    const Edge *e = &a.e[i];
    if (arr_is_arc(e))
    {
      if (e->x1 != e->sx || e->y1 != e->sy || e->x2 != e->ex ||
          e->y2 != e->ey)
        return ARR_NOT_CLOSED;
    }
    else if (e->etype != EDGE_POLYSEG && e->etype != EDGE_LINESEG)
      return ARR_NOT_CLOSED;
    if (arr_same(e->x1, e->y1, e->x2, e->y2))
      return ARR_NOT_CLOSED;
  }
  /* The circles of the arcs */
  a.ccx = palloc(sizeof(Ival) * n); a.ccy = palloc(sizeof(Ival) * n);
  a.cr2 = palloc(sizeof(Ival) * n); a.ccw = palloc(sizeof(bool) * n);
  for (uint32_t i = 0; i < n; i++)
    if (arr_is_arc(&a.e[i]))
    {
      arr_circle(&a.e[i], &a.ccx[i], &a.ccy[i], &a.cr2[i]);
      a.ccw[i] = arr_arc_ccw(&a.e[i]);
    }
  /* The vertices */
  a.capnodes = 2 * n + 16;
  a.nodes = palloc(sizeof(ArrNode) * a.capnodes);
  a.vmask = 1;
  while (a.vmask < 4 * n)
    a.vmask <<= 1;
  a.vtab = palloc(sizeof(int32_t) * a.vmask);
  for (uint32_t h = 0; h < a.vmask; h++)
    a.vtab[h] = -1;
  a.vmask -= 1;
  a.start = palloc(sizeof(uint32_t) * n);
  a.end = palloc(sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; i++)
  {
    a.start[i] = arr_vertex(&a, a.e[i].x1, a.e[i].y1);
    a.end[i] = arr_vertex(&a, a.e[i].x2, a.e[i].y2);
  }
  a.cuts = palloc0(sizeof(ArrCuts) * n);
  /* Closed curves: as many edges start at every vertex as end there */
  ArrangeStatus st = ARR_ANSWERED;
  int32_t *balance = palloc0(sizeof(int32_t) * a.nnodes);
  for (uint32_t i = 0; i < n; i++)
  {
    balance[a.start[i]]++;
    balance[a.end[i]]--;
  }
  for (uint32_t v = 0; v < a.nnodes && st == ARR_ANSWERED; v++)
    if (balance[v] != 0)
      st = ARR_NOT_CLOSED;
  pfree(balance);

  if (st == ARR_ANSWERED)
    st = arr_node_all(&a);
  if (st == ARR_ANSWERED)
    st = arr_pieces(&a);
  if (st == ARR_ANSWERED)
    st = arr_order_nodes(&a);
  uint32_t outer = UINT32_MAX;
  if (st == ARR_ANSWERED)
    st = arr_label_all(&a, &outer);
  if (st == ARR_ANSWERED)
  {
    /* The sense of the region: the side of the outermost piece away from the
     * outside */
    const ArrPiece *p = &a.pieces[outer];
    int inner = (p->left == 0) ? p->right : p->left;
    int sgn = (inner > 0) ? 1 : -1;
    st = arr_rings(&a, sgn, outer, rings, nrings);
  }
  /* Release */
  for (uint32_t i = 0; i < n; i++)
    if (a.cuts[i].cuts)
      pfree(a.cuts[i].cuts);
  pfree(a.cuts); pfree(a.start); pfree(a.end); pfree(a.vtab);
  pfree(a.nodes);
  pfree(a.ccx); pfree(a.ccy); pfree(a.cr2); pfree(a.ccw);
  if (a.cross)
    pfree(a.cross);
  if (a.pieces)
    pfree(a.pieces);
  if (a.halves)
    pfree(a.halves);
  if (a.hoff)
    pfree(a.hoff);
  return st;
}
