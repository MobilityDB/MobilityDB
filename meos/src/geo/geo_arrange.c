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
 *
 * Segments running along each other are read as the stretches they share,
 * each weighted by the rise of the winding number all of them give across it
 * (#arr_merge_overlaps): every end of such a stretch is an input vertex, so
 * the stretches are decided exactly, and the curves may share any part of
 * their boundaries.
 *
 * The union of many surfaces bounded by segments is merged pairwise
 * (#arrange_union_surfaces): the boundary of each union reaches the next
 * arrangement as stretches of input segments between two ends, each an input
 * vertex or the crossing of two input segments (#ArrStretch). Every node of
 * every merge is therefore placed by the input doubles alone, every question
 * keeps the degree it has in one arrangement of all the rings, and only the
 * last merge rounds the crossings it writes.
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
#include "temporal/temporal.h"

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

/* Each bound moved outward by at least one step: |x| 2^-52 is no less than
 * the unit of the last place of x and is exact, 2^-1074 covers the zeros and
 * the subnormals, and the sum or difference rounds to nearest, which never
 * brings a value back past the double next to x. An infinity stays one */
static inline Ival
ival_out(double lo, double hi)
{
  Ival r = {lo - (fabs(lo) * 0x1p-52 + 0x1p-1074),
    hi + (fabs(hi) * 0x1p-52 + 0x1p-1074)};
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

/* The interval of four products or quotients: their extremes, rounded
 * outward by #ival_out, and the whole line where one is undefined, an
 * infinity times zero */
static inline Ival
ival_extremes(const double p[4])
{
  if (isnan(p[0]) || isnan(p[1]) || isnan(p[2]) || isnan(p[3]))
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  double lo = p[0], hi = p[0];
  for (int i = 1; i < 4; i++)
  {
    lo = (p[i] < lo) ? p[i] : lo;
    hi = (p[i] > hi) ? p[i] : hi;
  }
  return ival_out(lo, hi);
}

/* The product: where the sign of either interval is known, its extremes are
 * two of the four products of the bounds, which the signs name; where both
 * hold zero within them, all four are read (#ival_extremes) */
static pg_attribute_always_inline Ival
ival_mul(Ival a, Ival b)
{
  double lo, hi;
  if (a.lo >= 0.0 && b.lo >= 0.0)
  {
    lo = a.lo * b.lo; hi = a.hi * b.hi;
  }
  else if (a.lo >= 0.0 && b.hi <= 0.0)
  {
    lo = a.hi * b.lo; hi = a.lo * b.hi;
  }
  else if (a.lo >= 0.0)
  {
    lo = a.hi * b.lo; hi = a.hi * b.hi;
  }
  else if (a.hi <= 0.0 && b.lo >= 0.0)
  {
    lo = a.lo * b.hi; hi = a.hi * b.lo;
  }
  else if (a.hi <= 0.0 && b.hi <= 0.0)
  {
    lo = a.hi * b.hi; hi = a.lo * b.lo;
  }
  else if (a.hi <= 0.0)
  {
    lo = a.lo * b.hi; hi = a.lo * b.lo;
  }
  else if (b.lo >= 0.0)
  {
    lo = a.lo * b.hi; hi = a.hi * b.hi;
  }
  else if (b.hi <= 0.0)
  {
    lo = a.hi * b.lo; hi = a.lo * b.lo;
  }
  else
  {
    double p[4] = {a.lo * b.lo, a.lo * b.hi, a.hi * b.lo, a.hi * b.hi};
    return ival_extremes(p);
  }
  if (isnan(lo) || isnan(hi))
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  return ival_out(lo, hi);
}

/* A divisor holding zero bounds nothing */
static pg_attribute_always_inline Ival
ival_div(Ival a, Ival b)
{
  if (b.lo <= 0.0 && b.hi >= 0.0)
  {
    Ival r = {-INFINITY, INFINITY};
    return r;
  }
  double p[4] = {a.lo / b.lo, a.lo / b.hi, a.hi / b.lo, a.hi / b.hi};
  return ival_extremes(p);
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

static pg_attribute_always_inline Ival
ival_cross(Ival ax, Ival ay, Ival bx, Ival by)
{
  return ival_sub(ival_mul(ax, by), ival_mul(ay, bx));
}

static pg_attribute_always_inline Ival
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
 * size of the arc, as #emit_arc_edge forms the centre it carries. A centred
 * arc carries its circle as its centre and radius
 */
static void
arr_circle(const Edge *arc, Ival *cx, Ival *cy, Ival *r2)
{
  if (arc->centred)
  {
    *cx = ival(arc->cx);
    *cy = ival(arc->cy);
    *r2 = ival_mul(ival(arc->radius), ival(arc->radius));
    return;
  }
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
  if (arc->diameter || arc->centred)
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
 * after it, since every point asked about stands within the arc. A centred
 * arc starts at the point of its circle furthest along -x, the radius (-r, 0)
 */
static Ival
arr_arc_position(const Edge *arc, Ival cx, Ival cy, Ival px, Ival py)
{
  Ival sx = arc->centred ? ival(- arc->radius) : ival_sub(ival(arc->sx), cx);
  Ival sy = arc->centred ? ival(0.0) : ival_sub(ival(arc->sy), cy);
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

/* How the point of a cut arises, as #MeetPoint states it with the indices
 * of the edges */
typedef struct
{
  MeetKind kind;
  const Edge *p, *q;
  int sigma;
  double x, y;
  const TangentPoint *tp;
} ArrMeet;

/* A node: an input vertex, exact, or a crossing of two edges interior to
 * both, rounded once */
typedef struct
{
  double x, y;
  Ival bx, by;       /**< Intervals holding the point exactly: the point
                          itself for a vertex */
  bool vertex;
  int32_t cross;     /**< The crossing it is, or -1 */
  const Edge *xp, *xq; /**< For a crossing, the two edges whose crossing it
                          is, which place it exactly: input segments where
                          the edges are stretches (#ArrStretch) */
  int xsign;         /**< The sign of the cross product of their
                          directions */
  bool merged;       /**< Several edges cross at it, its cuts merged
                          (#arr_node_merge): ordered around by their
                          directions rather than as a crossing of two */
  bool tower;        /**< A point no double holds, placed in a tower of
                          square roots: every question about it reads
                          @p meet (#towerseg_meet) */
  ArrMeet meet;      /**< For such a node, the meeting it is */
  const Edge *origin; /**< The whole circle starting and ending at it, the
                          point of the circle furthest along -x, or NULL */
} ArrNode;

/* A cut of an edge at a node interior to it, with its position along the
 * edge as an interval and the point it is, exactly */
typedef struct
{
  uint32_t edge;
  uint32_t node;
  Ival pos;
  ArrMeet meet;
} ArrCut;

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

/* A stretch of the boundary of a region: the part of an input edge between
 * two ends, walked from the one to the other with the region on its left.
 * The edge is a segment, whose line holds the stretch, a whole circle
 * (#edges_add_circle) or a segment between tangent points
 * (#edges_add_tangent_segment). Each end is the meeting that places it
 * exactly: an input vertex, the crossing of two input segments, a tangent
 * point, a crossing of the circles and segments between tangent points, or
 * where a circle starts and ends (#MEET_ORIGIN), its coordinates rounded once
 * where it was found. The boundary of the union of surfaces is carried from
 * one arrangement to the next as its stretches (#arrange_union_surfaces), so
 * that every node of every arrangement is one of these meetings of input
 * edges, decided as exactly as those of the first */
typedef struct
{
  const Edge *s;       /**< The input edge it lies on */
  bool rev;            /**< Walked from the end of the segment towards its
                            start */
  ArrMeet from, to;
  Ival fbox[2], tbox[2]; /**< The intervals holding each end, as the
                            arrangement that wrote it placed it, empty where
                            none did */
} ArrStretch;

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
  int32_t *w;             /**< The weight of each edge, the curves running
                               along it counted by their sense
                               (#arr_merge_overlaps), or NULL where every
                               edge is one curve */
  Ival *ccx, *ccy, *cr2;  /**< The circle of each arc edge */
  bool *ccw;              /**< The sense each arc edge turns */
  ArrNode *nodes;
  uint32_t nnodes, capnodes;
  uint32_t *start;        /**< The node each edge starts at */
  uint32_t *end;          /**< The node each edge ends at */
  ArrCut *cuts;           /**< The cuts of all the edges, as they are made,
                               then grouped by edge (#arr_pieces) */
  uint32_t ncuts, capcuts;
  uint32_t *coff;         /**< First cut of each edge, one past the last */
  ArrCross *cross;
  uint32_t ncross, capcross;
  /* The vertex nodes by their coordinates */
  int32_t *vtab;
  uint32_t vmask;
  ArrPiece *pieces;
  uint32_t npieces;
  uint32_t *nrep;         /**< The node standing for each node, where cuts
                               of an edge at one point merged them
                               (#arr_pieces) */
  ArrHalf *halves;        /**< The halves of every node, node by node */
  uint32_t *hoff;         /**< First half of each node, one past the last */
  double (*box)[4];       /**< The box of every edge (#arr_box) */
  /* Where the edges are stretches (#ArrStretch), each the line of its input
   * segment, walked in its sense, between the nodes of its two ends */
  const Edge **sup;       /**< The input segment of each edge, or NULL */
  bool *erev;             /**< Whether the edge runs against it */
  Ival *tfrom, *tto;      /**< The positions of its ends along it */
  ArrMeet *mfrom, *mto;   /**< The meetings its ends are */
  int32_t *xtab;          /**< The crossing nodes by their segments */
  uint32_t xmask;
  uint32_t (*same)[2];    /**< Pairs of nodes found at one point, merged
                               once the cuts are grouped (#arr_pieces) */
  uint32_t nsame, capsame;
  /* The nodes of the tangent points ending segments between them, by the
   * point (#arr_tangent_node) */
  const TangentPoint **tptab;
  uint32_t *tpnode;
  uint32_t tpmask;
  uint32_t split;         /**< Where the edges are the stretches of two
                               boundaries, the first edge of the second, 0
                               otherwise: the stretches of one boundary meet
                               at their ends alone (#arr_meet_curved) */
  uint8_t *half;          /**< Where stretches of the two boundaries running
                               along each other are merged, the boundary each
                               edge is a stretch of, 2 for a stretch of both;
                               NULL where @p split tells it */
  /* Linework and points the union states beside its surfaces
   * (#arrange_union_surfaces_lines), as #ArrLinework gives them: NULL where
   * there are none */
  const Edge *lines;      /**< The segments of the linework */
  uint32_t nlines;
  MeosArray **lout;       /**< Where the pieces of the linework no surface
                               covers are written */
  const POINT2D *pts;     /**< The points */
  uint32_t npts;
  bool *pkeep;            /**< Whether each point lies outside the surfaces
                               and off the linework */
} Arr;

/* The edge a meeting or a crossing refers to: the input segment of a
 * stretch, which outlives the arrangement, or the edge itself */
static inline const Edge *
arr_edge_ref(const Arr *a, uint32_t i)
{
  return a->sup ? a->sup[i] : &a->e[i];
}

/* A meeting as #meets_order_exact reads it */
static MeetPoint
arr_meet_point(const ArrMeet *m)
{
  MeetPoint r;
  r.kind = m->kind;
  r.p = (m->kind == MEET_VERTEX) ? NULL : m->p;
  r.q = (m->kind == MEET_VERTEX) ? NULL : m->q;
  r.sigma = m->sigma;
  r.x = m->x; r.y = m->y;
  r.tp = m->tp;
  return r;
}

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
  nd->merged = false;
  nd->tower = false;
  nd->xp = nd->xq = NULL;
  nd->xsign = 0;
  nd->origin = NULL;
  nd->bx = ival(x); nd->by = ival(y);
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
  if (a->ncuts == a->capcuts)
  {
    a->capcuts = a->capcuts ? 2 * a->capcuts : 64;
    a->cuts = a->cuts ? repalloc(a->cuts, sizeof(ArrCut) * a->capcuts) :
      palloc(sizeof(ArrCut) * a->capcuts);
  }
  ArrCut *c = &a->cuts[a->ncuts++];
  c->edge = edge;
  c->node = node;
  c->pos = pos;
  c->meet = meet;
  return;
}

static inline bool arr_is_arc(const Edge *e);

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
  ArrNode *nd = &a->nodes[node];
  nd->xp = arr_edge_ref(a, ea);
  nd->xq = arr_edge_ref(a, eb);
  /* The edge of a stretch may run against its segment, which the sign of
   * the crossing read off the segments follows; a stretch of a circle or of
   * a segment between tangent points runs along its edge */
  bool lines = a->sup && ! arr_is_arc(nd->xp) && ! arr_is_arc(nd->xq) &&
    ! nd->xp->from && ! nd->xq->from;
  nd->xsign = lines ? cross_product_sign(nd->xp->x1, nd->xp->y1,
    nd->xp->x2, nd->xp->y2, nd->xq->x1, nd->xq->y1, nd->xq->x2, nd->xq->y2) :
    sign;
  return (int32_t) a->ncross++;
}

/* Whether an edge is an arc, the kind test #arr_weight sits beside */
static inline bool
arr_is_arc(const Edge *e)
{
  return e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC;
}

/* The weight of an edge: by how much the winding number rises from its right
 * to its left. An accessor of the arrangement's per-edge data, as
 * #arr_is_arc reads the kind of an edge */
static inline int32_t
arr_weight(const Arr *a, uint32_t edge)
{
  return a->w ? a->w[edge] : 1;
}

/* Whether two edges are stretches of one of the two boundaries an
 * arrangement of stretches merges, which meet at their ends alone: an
 * accessor of its per-edge data, as #arr_weight */
static inline bool
arr_same_half(const Arr *a, uint32_t i, uint32_t j)
{
  if (a->half)
    return a->half[i] < 2 && a->half[i] == a->half[j];
  return a->split && (i < a->split) == (j < a->split);
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
 * of its ends, which a centred arc does not have on its circle */
static void
arr_cut_vertex(Arr *a, uint32_t i, double x, double y)
{
  const Edge *e = &a->e[i];
  if (! e->centred &&
      (arr_same(x, y, e->x1, e->y1) || arr_same(x, y, e->x2, e->y2)))
    return;
  Ival pos = arr_is_arc(e) ?
    arr_arc_position(e, a->ccx[i], a->ccy[i], ival(x), ival(y)) :
    arr_segment_position(e, ival(x), ival(y));
  ArrMeet m = {MEET_VERTEX, NULL, NULL, 0, x, y, NULL};
  arr_cut(a, i, arr_vertex(a, x, y), pos, m);
  return;
}

/* A node placed in a tower of square roots: rounded once, its intervals
 * the box holding it exactly (#meet_point_bounds); UINT32_MAX where it
 * cannot be placed */
static uint32_t arr_tower_node_at(Arr *a, const ArrMeet *m, double x,
  double y, const double box[4]);

static uint32_t
arr_tower_node(Arr *a, const ArrMeet *m)
{
  MeetPoint mp = arr_meet_point(m);
  mp.p = m->p; mp.q = m->q;
  double x, y, box[4];
  if (! meet_point_bounds(&mp, &x, &y, box))
    return UINT32_MAX;
  return arr_tower_node_at(a, m, x, y, box);
}

/* The node of #arr_tower_node, its point already placed by
 * #meet_point_bounds */
static uint32_t
arr_tower_node_at(Arr *a, const ArrMeet *m, double x, double y,
  const double box[4])
{
  uint32_t node = arr_node_new(a, x, y, false, -1);
  ArrNode *nd = &a->nodes[node];
  nd->bx = ival_out(box[0], box[1]);
  nd->by = ival_out(box[2], box[3]);
  nd->tower = true;
  nd->meet = *m;
  nd->meet.x = x; nd->meet.y = y;
  return node;
}

/* The node of a meeting no double holds, placed by the intervals @p box
 * holding it where an arrangement before placed it, its coordinates then
 * those it wrote, or else anew (#arr_tower_node) */
static uint32_t
arr_end_tower_node(Arr *a, const ArrMeet *m, const Ival *box)
{
  if (! box || box[0].lo > box[0].hi)
    return arr_tower_node(a, m);
  double b[4] = {box[0].lo, box[0].hi, box[1].lo, box[1].hi};
  return arr_tower_node_at(a, m, m->x, m->y, b);
}

/* The node of a point where a tangent touches a circle, one per point,
 * found by the point that defines it; UINT32_MAX where it cannot be
 * placed. Two points of one value are made one node (#arr_arrange) */
static uint32_t
arr_tangent_node_at(Arr *a, const TangentPoint *tp, const ArrMeet *end,
  const Ival *box);

static uint32_t
arr_tangent_node(Arr *a, const TangentPoint *tp)
{
  return arr_tangent_node_at(a, tp, NULL, NULL);
}

/* The node of #arr_tangent_node, placed where the intervals @p box are
 * given at the coordinates of @p end, as an arrangement before placed it */
static uint32_t
arr_tangent_node_at(Arr *a, const TangentPoint *tp, const ArrMeet *end,
  const Ival *box)
{
  uintptr_t key = (uintptr_t) tp;
  uint64_t hk = (uint64_t) key * UINT64CONST(0x9E3779B97F4A7C15);
  uint32_t h = (uint32_t) (hk >> 32) & a->tpmask;
  while (a->tptab[h])
  {
    if (a->tptab[h] == tp)
      return a->tpnode[h];
    h = (h + 1) & a->tpmask;
  }
  ArrMeet m = {MEET_TANGENT, NULL, NULL, 0, end ? end->x : 0.0,
    end ? end->y : 0.0, tp};
  uint32_t node = arr_end_tower_node(a, &m, end ? box : NULL);
  if (node != UINT32_MAX)
  {
    a->tptab[h] = tp;
    a->tpnode[h] = node;
  }
  return node;
}

/* The position of a node of the tower along an edge, from the box holding
 * it: along a segment between tangent points by its rounded ends, which
 * orders the points of its line as the exact segment does */
static Ival
arr_tower_position(const Arr *a, uint32_t i, uint32_t node)
{
  const Edge *e = &a->e[i];
  const ArrNode *nd = &a->nodes[node];
  return arr_is_arc(e) ?
    arr_arc_position(e, a->ccx[i], a->ccy[i], nd->bx, nd->by) :
    arr_segment_position(e, nd->bx, nd->by);
}

/*
 * The cross product (B - A) x (D - C) of differences of four doubles, as an
 * interval: its value computed in doubles and the bound #cross_product_sign
 * reads, (3 + 16 eps) eps (|left| + |right|) with eps the machine epsilon,
 * widened for the rounding of the bound and of the ends of the interval and
 * for products that underflow
 */
static inline Ival
arr_cross_ival(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  double left = (bx - ax) * (dy - cy);
  double right = (by - ay) * (dx - cx);
  double det = left - right;
  double bound = (3.0 + 16.0 * DBL_EPSILON) * DBL_EPSILON *
    (fabs(left) + fabs(right)) * (1.0 + 4.0 * DBL_EPSILON) + 0x1p-1072;
  return ival_out(det - bound, det + bound);
}

/* The crossing of the lines of two segments P and Q, as the positions along
 * each, t = (C - A) x S / (R x S), every cross product of differences of the
 * four ends held by its value and the bound of #arr_cross_ival, and as the
 * intervals of its coordinates */
static void
arr_cross_ivals(const Edge *p, const Edge *q, Ival *tp, Ival *tq, Ival *bx,
  Ival *by)
{
  Ival den = arr_cross_ival(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1, q->x2,
    q->y2);
  *tp = ival_div(arr_cross_ival(p->x1, p->y1, q->x1, q->y1, q->x1, q->y1,
    q->x2, q->y2), den);
  *tq = ival_div(arr_cross_ival(p->x1, p->y1, q->x1, q->y1, p->x1, p->y1,
    p->x2, p->y2), den);
  double rx = p->x2 - p->x1, ry = p->y2 - p->y1;
  *bx = ival_add(ival(p->x1), ival_mul(*tp, ival_out(rx, rx)));
  *by = ival_add(ival(p->y1), ival_mul(*tp, ival_out(ry, ry)));
  return;
}

/**
 * @brief Node two segments where they meet
 */
static ArrangeStatus
arr_meet_segments(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  /* Two segments sharing an end and not on one line meet at that end only,
   * where both end already: their lines cross once */
  if ((arr_same(p->x1, p->y1, q->x1, q->y1) ||
       arr_same(p->x1, p->y1, q->x2, q->y2) ||
       arr_same(p->x2, p->y2, q->x1, q->y1) ||
       arr_same(p->x2, p->y2, q->x2, q->y2)) &&
      cross_product_sign(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1, q->x2,
        q->y2) != 0)
    return ARR_ANSWERED;
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
  Ival tp, tq;
  arr_cross_ivals(p, q, &tp, &tq, &a->nodes[node].bx, &a->nodes[node].by);
  ArrMeet m = {MEET_SEGSEG, arr_edge_ref(a, i), arr_edge_ref(a, j), 0,
    0.0, 0.0, NULL};
  arr_cut(a, i, node, tp, m);
  arr_cut(a, j, node, tq, m);
  return ARR_ANSWERED;
}

/*****************************************************************************
 * Stretches: the arrangement of the boundaries of two regions
 *****************************************************************************/

/* Record that two nodes are one point, merged once the cuts are grouped
 * (#arr_pieces) */
static void
arr_same_add(Arr *a, uint32_t v, uint32_t w)
{
  if (a->nsame == a->capsame)
  {
    a->capsame = a->capsame ? 2 * a->capsame : 16;
    a->same = a->same ? repalloc(a->same, sizeof(uint32_t[2]) * a->capsame) :
      palloc(sizeof(uint32_t[2]) * a->capsame);
  }
  a->same[a->nsame][0] = v;
  a->same[a->nsame++][1] = w;
  return;
}

/* Whether two meetings of input edges are one point by what defines them:
 * one description, the crossing sigma of circles p and q being the crossing
 * -sigma of q and p, as #arcarc_meet_exact places them on either side of
 * the line of the centres. Points of other descriptions are left to the
 * arithmetic (#meets_order_exact) */
static bool
arr_meets_same(const ArrMeet *m1, const ArrMeet *m2)
{
  if (m1->kind != m2->kind)
    return false;
  switch (m1->kind)
  {
    case MEET_TANGENT:
      return m1->tp == m2->tp;
    case MEET_LINES:
    case MEET_SEGSEG:
      return (m1->p == m2->p && m1->q == m2->q) ||
        (m1->p == m2->q && m1->q == m2->p);
    case MEET_ARCARC:
      if (! m1->p->centred || ! m1->q->centred)
        return false;
      return (m1->p == m2->p && m1->q == m2->q && m1->sigma == m2->sigma) ||
        (m1->p == m2->q && m1->q == m2->p && m1->sigma == - m2->sigma);
    case MEET_LINECIRCLE:
      return m1->p == m2->p && m1->q == m2->q && m1->sigma == m2->sigma;
    default:
      return false;
  }
}

/* Return how two points of the line of an edge are ordered along it, by
 * their positions and, where the intervals overlap, by #meets_order_exact:
 * -1, 1, 0 for one point, 2 where it cannot be told */
static int
arr_line_order(const Arr *a, uint32_t i, Ival t1, const ArrMeet *m1, Ival t2,
  const ArrMeet *m2)
{
  /* Where a circle starts, before every other point of it, and where it
   * ends, after every other: no other edge meets it there, as no edge
   * meets the start of a whole circle (#arr_arrange) */
  if (m1->kind == MEET_ORIGIN || m2->kind == MEET_ORIGIN)
  {
    if (m1->kind == m2->kind && m1->sigma == m2->sigma)
      return 0;
    if (m1->kind == MEET_ORIGIN)
      return m1->sigma ? 1 : -1;
    return m2->sigma ? -1 : 1;
  }
  if (arr_meets_same(m1, m2))
    return 0;
  if (t1.hi < t2.lo)
    return -1;
  if (t2.hi < t1.lo)
    return 1;
  MeetPoint p1 = arr_meet_point(m1), p2 = arr_meet_point(m2);
  return meets_order_exact(&a->e[i], a->ccw && a->ccw[i], &p1, &p2);
}

/* Where a point of the line of a stretch lies against it */
typedef enum
{
  ARR_OFF,        /**< Before its start or after its end */
  ARR_AT_FROM,    /**< At its start */
  ARR_AT_TO,      /**< At its end */
  ARR_WITHIN,     /**< Strictly between its ends */
  ARR_UNTOLD      /**< The order cannot be told */
} ArrWhere;

/* Whether a vertex is the point of a whole circle furthest along -x, where
 * it starts and ends, as #arr_node_circle_right tells a node at the point
 * furthest along x: y = cy and x + r = cx, the sum decided exactly by its
 * rounding error (two-sum) */
static inline bool
arr_vertex_at_origin(const Edge *e, double x, double y)
{
  if (y != e->cy)
    return false;
  double s = x + e->radius;
  double bv = s - x;
  double err = (x - (s - bv)) + (e->radius - bv);
  return s == e->cx && err == 0.0;
}

/* The doubles a meeting is where they hold it exactly: a vertex, or the
 * point where a tangent touches a circle of radius zero, its centre */
static inline bool
arr_meet_exact_xy(const ArrMeet *m, double *x, double *y)
{
  if (m->kind == MEET_VERTEX)
  {
    *x = m->x; *y = m->y;
    return true;
  }
  if (m->kind == MEET_TANGENT && m->tp &&
      (m->tp->k ? m->tp->r2 : m->tp->r1) == 0.0)
  {
    *x = m->tp->k ? m->tp->c2x : m->tp->c1x;
    *y = m->tp->k ? m->tp->c2y : m->tp->c1y;
    return true;
  }
  return false;
}

static ArrWhere
arr_stretch_where(const Arr *a, uint32_t i, Ival t, const ArrMeet *m)
{
  /* The point where a whole circle starts and ends is the end of the
   * stretch starting or ending there, never a point within it, which the
   * order along the circle (#arr_line_order) would make of it, putting its
   * start before and its end after every point. A meeting doubles hold is
   * there where they are the start; any other, as the tangent point of a
   * line x = cx - r, where #meets_same_point tells it is, asked only of a
   * meeting whose position may be the start, the start being held by
   * doubles where the difference cx - r is exact */
  const Edge *e = &a->e[i];
  double mx, my;
  int at = 0;
  if (e->centred && m->kind != MEET_ORIGIN)
  {
    if (arr_meet_exact_xy(m, &mx, &my))
      at = arr_vertex_at_origin(e, mx, my);
    else if ((t.lo <= 0.0 || t.hi >= 4.0) &&
        arr_vertex_at_origin(e, e->x1, e->y1))
    {
      MeetPoint mp = arr_meet_point(m);
      MeetPoint op = {0};
      op.kind = MEET_VERTEX;
      op.x = e->x1; op.y = e->y1;
      at = meets_same_point(&mp, &op);
      if (at == 2)
        return ARR_UNTOLD;
    }
  }
  if (at)
  {
    if (a->mfrom[i].kind == MEET_ORIGIN && a->mfrom[i].sigma == 0)
      return ARR_AT_FROM;
    if (a->mto[i].kind == MEET_ORIGIN && a->mto[i].sigma == 1)
      return ARR_AT_TO;
  }
  int c = arr_line_order(a, i, t, m, a->tfrom[i], &a->mfrom[i]);
  if (c == 2)
    return ARR_UNTOLD;
  if (c <= 0)
    return (c < 0) ? ARR_OFF : ARR_AT_FROM;
  c = arr_line_order(a, i, t, m, a->tto[i], &a->mto[i]);
  if (c == 2)
    return ARR_UNTOLD;
  if (c >= 0)
    return (c > 0) ? ARR_OFF : ARR_AT_TO;
  return ARR_WITHIN;
}

/* The node of an end of a stretch where a point lies at it */
static inline uint32_t
arr_stretch_end_node(const Arr *a, uint32_t i, ArrWhere w)
{
  return (w == ARR_AT_FROM) ? a->start[i] : a->end[i];
}

static inline const ArrMeet *
arr_stretch_end_meet(const Arr *a, uint32_t i, ArrWhere w)
{
  return (w == ARR_AT_FROM) ? &a->mfrom[i] : &a->mto[i];
}

/* The position of a node along the line of an edge holding it: 0 or 1 at
 * an end of the edge's segment, else read off the intervals holding it */
static Ival
arr_node_position(const Edge *e, const ArrNode *nd)
{
  if (nd->vertex)
  {
    if (arr_same(nd->x, nd->y, e->x1, e->y1))
      return ival(0.0);
    if (arr_same(nd->x, nd->y, e->x2, e->y2))
      return ival(1.0);
  }
  return arr_segment_position(e, nd->bx, nd->by);
}

/* The positions of the ends of stretch j along the line of stretch i, which
 * holds them */
static void
arr_stretch_on(const Arr *a, uint32_t i, uint32_t j, Ival *tf, Ival *tt)
{
  const ArrNode *nf = &a->nodes[a->start[j]], *nt = &a->nodes[a->end[j]];
  *tf = arr_node_position(&a->e[i], nf);
  *tt = arr_node_position(&a->e[i], nt);
  return;
}

/**
 * @brief Return whether two stretches on one line share a stretch of it: 1
 * where they do, 0 where they do not, 2 where it cannot be told; where they
 * only touch at an end, the nodes of the two ends are made one
 */
static int
arr_stretches_share(Arr *a, uint32_t i, uint32_t j)
{
  Ival tf, tt;
  arr_stretch_on(a, i, j, &tf, &tt);
  int dir = arr_line_order(a, i, tf, &a->mfrom[j], tt, &a->mto[j]);
  if (dir != -1 && dir != 1)
    return 2;
  /* The ends of j in the order of i */
  Ival tlo = (dir < 0) ? tf : tt, thi = (dir < 0) ? tt : tf;
  const ArrMeet *mlo = (dir < 0) ? &a->mfrom[j] : &a->mto[j];
  const ArrMeet *mhi = (dir < 0) ? &a->mto[j] : &a->mfrom[j];
  uint32_t nlo = (dir < 0) ? a->start[j] : a->end[j];
  uint32_t nhi = (dir < 0) ? a->end[j] : a->start[j];
  int c1 = arr_line_order(a, i, tlo, mlo, a->tto[i], &a->mto[i]);
  int c2 = arr_line_order(a, i, thi, mhi, a->tfrom[i], &a->mfrom[i]);
  if (c1 == 2 || c2 == 2)
    return 2;
  if (c1 > 0 || c2 < 0)
    return 0;
  if (c1 == 0)
  {
    if (nlo != a->end[i])
      arr_same_add(a, nlo, a->end[i]);
    return 0;
  }
  if (c2 == 0)
  {
    if (nhi != a->start[i])
      arr_same_add(a, nhi, a->start[i]);
    return 0;
  }
  return 1;
}

/* Whether the lines of two segments are one line */
static inline bool
arr_one_line(const Edge *p, const Edge *q)
{
  return cross_product_sign(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1, q->x2,
      q->y2) == 0 &&
    cross_product_sign(p->x1, p->y1, p->x2, p->y2, p->x1, p->y1, q->x1,
      q->y1) == 0;
}

/* The order of two segments by their coordinates, which makes the rounding
 * of their crossing independent of the order they are met in */
static int
arr_segment_cmp(const Edge *p, const Edge *q)
{
  const double a[4] = {p->x1, p->y1, p->x2, p->y2};
  const double b[4] = {q->x1, q->y1, q->x2, q->y2};
  for (int k = 0; k < 4; k++)
    if (a[k] != b[k])
      return (a[k] < b[k]) ? -1 : 1;
  return 0;
}

/**
 * @brief Node two stretches where they meet
 * @details Where the lines of their segments cross, the point is an input
 * vertex, where an end of either segment lies on the other, or else the
 * crossing of the two, and it is placed against the ends of each stretch
 * exactly (#arr_stretch_where). A point strictly within both is a node of
 * both; within one and at an end of the other, it is the node of that end,
 * which cuts the first; at an end of both, the two ends are one node. Two
 * stretches on one line that share a stretch of it leave it to the merge
 * that runs before (#arr_merge_stretches)
 */
static ArrangeStatus
arr_meet_stretches(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  int sign = cross_product_sign(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1,
    q->x2, q->y2);
  if (sign == 0)
  {
    if (! arr_one_line(p, q))
      return ARR_ANSWERED;
    int sh = arr_stretches_share(a, i, j);
    return (sh == 0) ? ARR_ANSWERED : (sh == 1 ? ARR_OVERLAP : ARR_ORDER);
  }
  /* Two stretches sharing an end meet at that end only */
  if (a->start[i] == a->start[j] || a->start[i] == a->end[j] ||
      a->end[i] == a->start[j] || a->end[i] == a->end[j])
    return ARR_ANSWERED;
  IntersectResult r = linesegm_intersect(p->x1, p->y1, p->x2, p->y2, q->x1,
    q->y1, q->x2, q->y2);
  if (r.type == INTERSECT_NONE)
    return ARR_ANSWERED;
  /* The point, as a meeting, and its position along each */
  const Edge *edges[2] = {p, q};
  bool vertex = false;
  double x = 0.0, y = 0.0;
  for (int k = 0; k < 4 && ! vertex; k++)
  {
    const Edge *own = edges[k / 2], *other = edges[1 - k / 2];
    x = (k % 2 == 0) ? own->x1 : own->x2;
    y = (k % 2 == 0) ? own->y1 : own->y2;
    vertex = point_on_segment_exact(x, y, other->x1, other->y1, other->x2,
      other->y2);
  }
  ArrMeet m;
  Ival ti, tj, bx, by;
  if (vertex)
  {
    m = (ArrMeet) {MEET_VERTEX, NULL, NULL, 0, x, y, NULL};
    ti = arr_segment_position(p, ival(x), ival(y));
    tj = arr_segment_position(q, ival(x), ival(y));
  }
  else
  {
    m = (ArrMeet) {MEET_SEGSEG, a->sup[i], a->sup[j], 0, 0.0, 0.0, NULL};
    arr_cross_ivals(p, q, &ti, &tj, &bx, &by);
  }
  ArrWhere wi = arr_stretch_where(a, i, ti, &m);
  ArrWhere wj = arr_stretch_where(a, j, tj, &m);
  if (wi == ARR_UNTOLD || wj == ARR_UNTOLD)
    return ARR_ORDER;
  if (wi == ARR_OFF || wj == ARR_OFF)
    return ARR_ANSWERED;
  if (wi == ARR_WITHIN && wj == ARR_WITHIN)
  {
    uint32_t node;
    if (vertex)
      node = arr_vertex(a, x, y);
    else
    {
      /* Rounded once, from the two segments in the order of their
       * coordinates */
      const Edge *s1 = a->sup[i], *s2 = a->sup[j];
      if (arr_segment_cmp(s1, s2) > 0)
      {
        const Edge *t = s1; s1 = s2; s2 = t;
      }
      IntersectResult rs = linesegm_intersect(s1->x1, s1->y1, s1->x2, s1->y2,
        s2->x1, s2->y1, s2->x2, s2->y2);
      node = arr_node_new(a, s1->x1 + rs.t0 * (s1->x2 - s1->x1),
        s1->y1 + rs.t0 * (s1->y2 - s1->y1), false, -1);
      a->nodes[node].cross = arr_cross_new(a, node, i, j, sign);
      a->nodes[node].bx = bx;
      a->nodes[node].by = by;
    }
    arr_cut(a, i, node, ti, m);
    arr_cut(a, j, node, tj, m);
    return ARR_ANSWERED;
  }
  if (wi == ARR_WITHIN)
  {
    arr_cut(a, i, arr_stretch_end_node(a, j, wj), ti,
      *arr_stretch_end_meet(a, j, wj));
    return ARR_ANSWERED;
  }
  if (wj == ARR_WITHIN)
  {
    arr_cut(a, j, arr_stretch_end_node(a, i, wi), tj,
      *arr_stretch_end_meet(a, i, wi));
    return ARR_ANSWERED;
  }
  uint32_t ni = arr_stretch_end_node(a, i, wi);
  uint32_t nj = arr_stretch_end_node(a, j, wj);
  if (ni != nj)
    arr_same_add(a, ni, nj);
  return ARR_ANSWERED;
}

/* The side of the line of its chord, from its start to its end, an arc
 * stands on: 1 to the left, -1 to the right, 0 for a centred arc, the whole
 * circle, which has no chord */
static inline int
arr_arc_side(const Arr *a, uint32_t j)
{
  const Edge *arc = &a->e[j];
  if (arc->centred)
    return 0;
  return arc->diameter ? (a->ccw[j] ? -1 : 1) :
    cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey, arc->sx, arc->sy,
      arc->mx, arc->my);
}

/**
 * @brief Node a segment and an arc where they meet
 */
static ArrangeStatus
arr_meet_segment_arc(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *s = &a->e[i], *arc = &a->e[j];
  /* A segment whose nearest point to the centre lies beyond the circle, or
   * whose two ends lie within it, meets none of its points: the disc is
   * convex. Decided in intervals, and asked exactly otherwise. A segment
   * sharing an end with the arc has that end on the circle, and goes to the
   * exact meeting directly. A centred arc has no end on its circle */
  bool s1 = ! arc->centred && (arr_same(s->x1, s->y1, arc->x1, arc->y1) ||
    arr_same(s->x1, s->y1, arc->x2, arc->y2));
  bool s2 = ! arc->centred && (arr_same(s->x2, s->y2, arc->x1, arc->y1) ||
    arr_same(s->x2, s->y2, arc->x2, arc->y2));
  bool shared = s1 || s2;
  if (shared && s1 != s2)
  {
    /* The arc is the part of its circle on one side of the line of its
     * chord, its ends on that line (#arr_arc_side). A segment sharing one
     * end with it has that end on the line and every other point strictly
     * on the side of its other end: where that end lies strictly on the
     * other side, the segment meets the arc at the shared end alone, which
     * both already end at, decided exactly. Offset segments leave the round
     * joins of a buffer this way, along their tangents */
    int want = arr_arc_side(a, j);
    double qx = s1 ? s->x2 : s->x1, qy = s1 ? s->y2 : s->y1;
    if (want != 0 && cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey,
          arc->sx, arc->sy, qx, qy) == - want)
      return ARR_ANSWERED;
  }
  if (! shared)
  {
    Ival ax = ival_sub(ival(s->x1), a->ccx[j]);
    Ival ay = ival_sub(ival(s->y1), a->ccy[j]);
    Ival bx = ival_sub(ival(s->x2), a->ccx[j]);
    Ival by = ival_sub(ival(s->y2), a->ccy[j]);
    Ival da = ival_sub(ival_dot(ax, ay, ax, ay), a->cr2[j]);
    Ival db = ival_sub(ival_dot(bx, by, bx, by), a->cr2[j]);
    if (da.hi < 0.0 && db.hi < 0.0)
      return ARR_ANSWERED;
    if (da.lo > 0.0 && db.lo > 0.0)
    {
      /* Both ends outside: the segment keeps outside where the centre
       * projects beyond an end, or lies farther from its line than r */
      Ival rx = ival_sub(bx, ax), ry = ival_sub(by, ay);
      Ival ta = ival_dot(ax, ay, rx, ry), tb = ival_dot(bx, by, rx, ry);
      if (ta.lo > 0.0 || tb.hi < 0.0)
        return ARR_ANSWERED;
      Ival cr = ival_cross(ax, ay, rx, ry);
      Ival beyond = ival_sub(ival_mul(cr, cr), ival_mul(a->cr2[j],
        ival_dot(rx, ry, rx, ry)));
      if (beyond.lo > 0.0)
        return ARR_ANSWERED;
    }
    /* The arc is the part of its circle on one side of the line of its
     * chord, its ends on that line: a segment with both ends strictly on
     * the other side meets none of it, decided exactly */
    int want = arr_arc_side(a, j);
    if (want != 0 && cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey,
          arc->sx, arc->sy, s->x1, s->y1) == - want &&
        cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey, arc->sx,
          arc->sy, s->x2, s->y2) == - want)
      return ARR_ANSWERED;
    /* The roots of |A + t R - C|^2 = r^2, t = (-b -+ sqrt(b^2 - qa qc)) / qa
     * with b = (A - C).R, held by intervals that hold the circle: where
     * each lies off the segment, or at a point on the side of the chord
     * the arc does not stand on, none meets the arc */
    Ival rx = ival_sub(bx, ax), ry = ival_sub(by, ay);
    Ival qa = ival_dot(rx, ry, rx, ry), qb = ival_dot(ax, ay, rx, ry);
    Ival disc = ival_sub(ival_mul(qb, qb), ival_mul(qa, da));
    if (disc.hi < 0.0)
      return ARR_ANSWERED;
    if (want != 0 && disc.lo > 0.0 && qa.lo > 0.0)
    {
      Ival sq = ival_sqrt(disc);
      Ival cx = ival_sub(ival(arc->ex), ival(arc->sx));
      Ival cy = ival_sub(ival(arc->ey), ival(arc->sy));
      int k;
      for (k = 0; k < 2; k++)
      {
        Ival t = ival_div(k == 0 ? ival_sub(ival_neg(qb), sq) :
          ival_add(ival_neg(qb), sq), qa);
        if (t.hi < 0.0 || t.lo > 1.0)
          continue;
        Ival px = ival_add(ival(s->x1), ival_mul(t, rx));
        Ival py = ival_add(ival(s->y1), ival_mul(t, ry));
        if (ival_sign(ival_cross(cx, cy, ival_sub(px, ival(arc->sx)),
              ival_sub(py, ival(arc->sy)))) != - want)
          break;
      }
      if (k == 2)
        return ARR_ANSWERED;
    }
  }
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
         * segment enters the circle from. A half circle on a diameter and a
         * centred arc read the quadratic |P - C|^2 - r^2, the other way round
         * when they turn counterclockwise */
        int st = ((arc->diameter || arc->centred) && a->ccw[j]) ?
          - m[k].sigma : m[k].sigma;
        uint32_t node = arr_node_new(a, x, y, false, -1);
        a->nodes[node].cross = arr_cross_new(a, node, i, j, - st);
        Ival px = ival_add(ax, ival_mul(t, rx)), py = ival_add(ay,
          ival_mul(t, ry));
        ArrMeet am = {MEET_SEGARC, &a->e[i], &a->e[j], m[k].sigma, 0.0,
          0.0, NULL};
        a->nodes[node].bx = px;
        a->nodes[node].by = py;
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
 * chord from its start to its end, against the side the arc stands on. Every
 * point of its circle lies within a centred arc */
static int
arr_on_arc(const Arr *a, uint32_t j, Ival px, Ival py)
{
  const Edge *arc = &a->e[j];
  if (arc->centred)
    return 1;
  Ival cx = ival_sub(ival(arc->ex), ival(arc->sx));
  Ival cy = ival_sub(ival(arc->ey), ival(arc->sy));
  int side = ival_sign(ival_cross(cx, cy, ival_sub(px, ival(arc->sx)),
    ival_sub(py, ival(arc->sy))));
  if (side == 2)
    return 2;
  return side == arr_arc_side(a, j) ? 1 : 0;
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
  a->nodes[node].bx = px;
  a->nodes[node].by = py;
  ArrMeet m = {MEET_ARCARC, &a->e[i], &a->e[j], sigma, 0.0, 0.0, NULL};
  /* Two centred circles cross at a point of one root, which the questions
   * about the points of segments between tangent points read too, and
   * which is rounded from its exact value rather than from the middle of
   * intervals that circles of nearly one centre leave wide
   * (#meet_point_bounds) */
  if (a->e[i].centred && a->e[j].centred)
  {
    ArrNode *nd = &a->nodes[node];
    nd->tower = true;
    nd->meet = m;
    MeetPoint mp = arr_meet_point(&m);
    double x, y, box[4];
    if (meet_point_bounds(&mp, &x, &y, box))
    {
      nd->x = nd->meet.x = x;
      nd->y = nd->meet.y = y;
      nd->bx = ival_out(box[0], box[1]);
      nd->by = ival_out(box[2], box[3]);
      px = nd->bx;
      py = nd->by;
    }
  }
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
  /* Centres the intervals cannot tell apart, as of two join arcs about one
   * vertex: the crossings are placed from the exact terms instead,
   * P = M1 + N1 / D1 + (G d' + sigma sqrt(H) rot(d')) / (2 Q L) */
  bool concentric = ival_sign(d2) != 1;
  Ival c1x = ival(0.0), c1y = c1x, ex = c1x, ey = c1x, eg = c1x, eh = c1x,
    eden = c1x;
  if (concentric && meet != 0)
  {
    double t[ARCARC_NTERMS][2];
    if (! arcarc_crossing_terms(&a->e[i], &a->e[j], t))
      return ARR_MEETING;
    Ival v[ARCARC_NTERMS];
    for (int k = 0; k < ARCARC_NTERMS; k++)
    {
      v[k].lo = t[k][0];
      v[k].hi = t[k][1];
    }
    c1x = ival_add(v[ARCARC_M1X], ival_div(v[ARCARC_N1X], v[ARCARC_D1]));
    c1y = ival_add(v[ARCARC_M1Y], ival_div(v[ARCARC_N1Y], v[ARCARC_D1]));
    ex = v[ARCARC_DX]; ey = v[ARCARC_DY]; eg = v[ARCARC_G];
    eh = ival_sqrt(v[ARCARC_H]);
    eden = ival_mul(ival(2.0), ival_mul(v[ARCARC_QQ], v[ARCARC_L]));
  }
  int c12 = (a->ccw[i] ? 1 : -1) * (a->ccw[j] ? 1 : -1);
  for (int r = 0; r < 2; r++)
  {
    if (! within[r][0] || ! within[r][1])
      continue;
    /* Circles touching within both arcs meet without crossing */
    if (meet == 0)
      return ARR_MEETING;
    int sigma = (r == 0) ? -1 : 1;
    Ival px, py;
    if (concentric)
    {
      px = ival_add(c1x, ival_div(ival_sub(ival_mul(eg, ex),
        ival_mul(ival(sigma), ival_mul(eh, ey))), eden));
      py = ival_add(c1y, ival_div(ival_add(ival_mul(eg, ey),
        ival_mul(ival(sigma), ival_mul(eh, ex))), eden));
    }
    else
    {
      px = ival_add(bx, ival_mul(ival(-sigma), ival_mul(f, dy)));
      py = ival_add(by, ival_mul(ival(sigma), ival_mul(f, dx)));
    }
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
    /* A centred arc has no end on its circle */
    if (((k < 2) ? p : q)->centred || ! point_on_arc_circle(other, x, y))
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
   * other and they run along each other, as a centred arc, its whole
   * circle, runs along every other arc of it */
  if (arc_same_circle(p, q))
    return (inner || p->centred || q->centred) ? ARR_OVERLAP : ARR_ANSWERED;
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
    /* An input vertex where a centred arc touches another circle: the
     * signs below read the chords of both */
    if (p->centred || q->centred)
      return ARR_MEETING;
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
    a->nodes[node].bx = qx;
    a->nodes[node].by = qy;
    ArrMeet m = {MEET_ARCARC_REFLECT, &a->e[i], &a->e[j], 0, jx, jy, NULL};
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

/**
 * @brief Node a segment between tangent points and another such segment or
 * a centred arc where they meet, decided exactly (#towerseg_meet)
 * @details A meeting at an end of a segment is the node of its tangent
 * point; any other is a node of its own, a crossing ordered around by the
 * sign of the cross product of the two directions there. Any other edge
 * meeting such a segment is not answered
 */
static ArrangeStatus
arr_meet_tower(Arr *a, uint32_t i, uint32_t j)
{
  uint32_t s = a->e[i].from ? i : j, o = a->e[i].from ? j : i;
  const Edge *p = &a->e[s], *q = &a->e[o];
  if (! q->from && ! q->centred)
    return ARR_MEETING;
  TowerMeet m[2];
  int n = towerseg_meet(p, q, m);
  if (n == -1)
    return ARR_OVERLAP;
  if (n < 0)
    return ARR_MEETING;
  for (int k = 0; k < n; k++)
  {
    /* A touching interior to the segment is not answered here */
    if (m[k].m.kind == MEET_LINECIRCLE && m[k].sign == 0)
      return ARR_MEETING;
    ArrMeet am = {m[k].m.kind, m[k].m.p, m[k].m.q, m[k].m.sigma, 0.0, 0.0,
      m[k].m.tp};
    uint32_t node;
    if (am.kind == MEET_TANGENT)
      node = arr_tangent_node(a, am.tp);
    else
    {
      node = arr_tower_node(a, &am);
      if (node != UINT32_MAX)
        a->nodes[node].cross = arr_cross_new(a, node, s, o, m[k].sign);
    }
    if (node == UINT32_MAX)
      return ARR_MEETING;
    am = a->nodes[node].meet;
    /* Each edge is cut where the meeting is interior to it, which it is
     * everywhere on a whole circle */
    if (m[k].end_p < 0)
      arr_cut(a, s, node, arr_tower_position(a, s, node), am);
    if (q->centred || m[k].end_q < 0)
      arr_cut(a, o, node, arr_tower_position(a, o, node), am);
  }
  return ARR_ANSWERED;
}

/* The input edge a meeting found between edges i and j of an arrangement of
 * stretches refers to: the edge of the stretch, which outlives the
 * arrangement, for either of the two */
static inline const Edge *
arr_meet_ref(const Arr *a, const Edge *e, uint32_t i, uint32_t j)
{
  return (e == &a->e[i]) ? a->sup[i] : ((e == &a->e[j]) ? a->sup[j] : e);
}

/* The root of the meeting of the edge of stretch s with a circle, sigma = 1
 * the farther along the edge, as a meeting referring to its input segment
 * reads it, along that segment (#arr_meet_ref, #towerseg_meet): the edge of a
 * stretch walking its segment backwards runs from its end (#arr_stretch_edge),
 * so the root farther along the edge is the nearer along the segment */
static inline int
arr_support_sigma(const Arr *a, uint32_t s, int sigma)
{
  return (a->erev && a->erev[s]) ? - sigma : sigma;
}

/* Whether a tangent point lies on the circle of a centred arc as the point
 * where its tangent touches that circle, the circle it was built on */
static inline bool
arr_tangent_on(const TangentPoint *tp, const Edge *c)
{
  return tp->k ? (tp->c2x == c->cx && tp->c2y == c->cy && tp->r2 == c->radius) :
    (tp->c1x == c->cx && tp->c1y == c->cy && tp->r1 == c->radius);
}

/**
 * @brief Return the meetings of segment s between tangent points with edge
 * o where the tangent points that build them tell them without arithmetic,
 * false where they do not
 * @details As #towerseg_meet answers them: a segment ending at a point where
 * its tangent touches the circle of @p o meets that circle there alone, a
 * touching, and a chord of the circle at both ends; two segments sharing an
 * end, not along one line, meet there alone
 */
static bool
arr_support_meets_known(const Arr *a, uint32_t s, uint32_t o, ArrMeet m[2],
  int sign[2], int *n)
{
  const Edge *p = &a->e[s], *q = &a->e[o];
  const TangentPoint *ends[2] = {p->from, p->to};
  *n = 0;
  if (q->centred)
  {
    const bool on[2] = {arr_tangent_on(ends[0], q),
      arr_tangent_on(ends[1], q)};
    if (! on[0] && ! on[1])
      return false;
    for (int k = 0; k < 2; k++)
      if (on[k])
      {
        m[*n] = (ArrMeet) {MEET_TANGENT, NULL, NULL, 0, 0.0, 0.0, ends[k]};
        sign[(*n)++] = 0;
      }
    return true;
  }
  /* The segments of one hull are told apart by their ends: a tangent and
   * a chord meet at one end, two tangents of a hull at none */
  for (int k = 0; k < 2; k++)
    if (ends[k] == q->from || ends[k] == q->to)
    {
      if (ends[1 - k] == q->from || ends[1 - k] == q->to)
        return false;
      m[0] = (ArrMeet) {MEET_TANGENT, NULL, NULL, 0, 0.0, 0.0, ends[k]};
      sign[0] = 0;
      *n = 1;
      return true;
    }
  return false;
}

/**
 * @brief Return the meetings of the whole circles and segments between
 * tangent points two stretches lie on, decided exactly, with the sign of the
 * cross product of the directions of the two at each, 0 for a touching
 * @details As #arr_meet_tower and #arr_meet_arcs_exact find them for the
 * edges themselves; the meetings refer to the input edges
 * @return #ARR_OVERLAP where the two run along each other, #ARR_MEETING
 * where a meeting is not told
 */
static ArrangeStatus
arr_support_meets(const Arr *a, uint32_t i, uint32_t j, ArrMeet m[2],
  int sign[2], int *n)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  *n = 0;
  if (p->centred && q->centred)
  {
    if (arc_same_circle(p, q))
      return ARR_OVERLAP;
    /* Circles the intervals keep apart, or one within the other */
    Ival dx = ival_sub(a->ccx[j], a->ccx[i]);
    Ival dy = ival_sub(a->ccy[j], a->ccy[i]);
    Ival d2 = ival_dot(dx, dy, dx, dy);
    Ival rs = ival_add(ival(p->radius), ival(q->radius));
    Ival rd = ival_sub(ival(p->radius), ival(q->radius));
    if (ival_sign(ival_sub(d2, ival_mul(rs, rs))) == 1 ||
        ival_sign(ival_sub(ival_mul(rd, rd), d2)) == 1)
      return ARR_ANSWERED;
    /* Two circles touching cross nothing: on either side of each, the
     * winding number the other gives is the same all along it, so the
     * touching is no node of the arrangement, as two circles apart */
    int meet, within[2][2];
    arcarc_meet_exact(p, q, &meet, within);
    if (meet <= 0)
      return ARR_ANSWERED;
    int c12 = (a->ccw[i] ? 1 : -1) * (a->ccw[j] ? 1 : -1);
    for (int r = 0; r < 2; r++)
    {
      int sigma = (r == 0) ? -1 : 1;
      m[r] = (ArrMeet) {MEET_ARCARC, a->sup[i], a->sup[j], sigma, 0.0, 0.0,
        NULL};
      sign[r] = sigma * c12;
    }
    *n = 2;
    return ARR_ANSWERED;
  }
  /* A segment first, one between tangent points before one of the input */
  bool swap = p->centred || (! p->from && q->from);
  uint32_t s = swap ? j : i, o = swap ? i : j;
  if (a->e[s].from && arr_support_meets_known(a, s, o, m, sign, n))
    return ARR_ANSWERED;
  TowerMeet tm[2];
  int c = towerseg_meet(&a->e[s], &a->e[o], tm);
  if (c == -1)
    return ARR_OVERLAP;
  if (c < 0)
    return ARR_MEETING;
  for (int k = 0; k < c; k++)
  {
    /* A vertex, the end of a segment of the input, is its coordinates; the
     * root of a meeting with a circle is read along the input segment */
    int sigma = (tm[k].m.kind == MEET_LINECIRCLE) ?
      arr_support_sigma(a, s, tm[k].m.sigma) : tm[k].m.sigma;
    m[k] = (ArrMeet) {tm[k].m.kind, arr_meet_ref(a, tm[k].m.p, i, j),
      arr_meet_ref(a, tm[k].m.q, i, j), sigma, tm[k].m.x,
      tm[k].m.y, tm[k].m.tp};
    sign[k] = swap ? - tm[k].sign : tm[k].sign;
  }
  *n = c;
  return ARR_ANSWERED;
}

static inline Ival arr_box_position(const Arr *a, uint32_t i, Ival bx,
  Ival by);

/* The sign of the cross product (B - A) x (P - A) of points the intervals
 * hold, 2 where they cannot tell */
static inline int
arr_orient_ival(const ArrNode *na, const ArrNode *nb, const ArrNode *np)
{
  return ival_sign(ival_cross(ival_sub(nb->bx, na->bx),
    ival_sub(nb->by, na->by), ival_sub(np->bx, na->bx),
    ival_sub(np->by, na->by)));
}

/* Whether the circle of the stretch of a circle c misses the stretch of a
 * segment s, the intervals holding its ends showing it within the disc or
 * clear of it */
static bool
arr_segment_misses_circle(const Arr *a, uint32_t s, uint32_t c)
{
  const ArrNode *na = &a->nodes[a->start[s]], *nb = &a->nodes[a->end[s]];
  Ival cx = a->ccx[c], cy = a->ccy[c], r2 = a->cr2[c];
  Ival ax = ival_sub(na->bx, cx), ay = ival_sub(na->by, cy);
  Ival bx = ival_sub(nb->bx, cx), by = ival_sub(nb->by, cy);
  int sa = ival_sign(ival_sub(ival_dot(ax, ay, ax, ay), r2));
  int sb = ival_sign(ival_sub(ival_dot(bx, by, bx, by), r2));
  /* Both ends within the disc: the segment lies within it */
  if (sa == -1 && sb == -1)
    return true;
  if (sa != 1 || sb != 1)
    return false;
  /* Both ends outside: clear of the disc where its line is, or where the
   * point of the segment nearest the centre is an end */
  Ival ux = ival_sub(bx, ax), uy = ival_sub(by, ay);
  Ival cr = ival_cross(ux, uy, ival_neg(ax), ival_neg(ay));
  if (ival_sign(ival_sub(ival_mul(cr, cr),
      ival_mul(r2, ival_dot(ux, uy, ux, uy)))) == 1)
    return true;
  return ival_sign(ival_dot(ival_neg(ax), ival_neg(ay), ux, uy)) == -1 ||
    ival_sign(ival_dot(ival_neg(bx), ival_neg(by), ux, uy)) == 1;
}

/* Whether every point where the line of the stretch of a segment s meets
 * the circle of the stretch of a circle c lies, as the intervals place it,
 * off the one stretch or the other: the roots of |A + t (B - A) - C|^2 = r^2
 * for the ends A and B of s, off s outside [0, 1], off c outside the
 * positions of its ends (#arr_crossing_off) */
static bool
arr_segment_circle_off(const Arr *a, uint32_t s, uint32_t c)
{
  const ArrNode *na = &a->nodes[a->start[s]], *nb = &a->nodes[a->end[s]];
  Ival ux = ival_sub(na->bx, a->ccx[c]), uy = ival_sub(na->by, a->ccy[c]);
  Ival vx = ival_sub(nb->bx, na->bx), vy = ival_sub(nb->by, na->by);
  Ival qa = ival_dot(vx, vy, vx, vy);
  Ival qb = ival_dot(vx, vy, ux, uy);
  Ival qc = ival_sub(ival_dot(ux, uy, ux, uy), a->cr2[c]);
  /* t = (-qb +- sqrt(qb^2 - qa qc)) / qa */
  Ival disc = ival_sub(ival_mul(qb, qb), ival_mul(qa, qc));
  if (ival_sign(qa) != 1 || ival_sign(disc) != 1)
    return false;
  Ival sq = ival_sqrt(disc);
  for (int r = -1; r <= 1; r += 2)
  {
    Ival t = ival_div(ival_add(ival_neg(qb), r < 0 ? ival_neg(sq) : sq), qa);
    if (! isfinite(t.lo) || ! isfinite(t.hi))
      return false;
    if (t.hi < 0.0 || t.lo > 1.0)
      continue;
    Ival px = ival_add(na->bx, ival_mul(t, vx));
    Ival py = ival_add(na->by, ival_mul(t, vy));
    Ival pc = arr_box_position(a, c, px, py);
    if (pc.hi < a->tfrom[c].lo || pc.lo > a->tto[c].hi)
      continue;
    return false;
  }
  return true;
}

/**
 * @brief Return whether two stretches on whole circles or segments between
 * tangent points cannot meet, as the intervals holding the ends of their
 * stretches show it, before the exact meetings of their edges are sought
 * (#arr_support_meets)
 * @details A meeting outside a stretch is passed over (#arr_meet_curved),
 * so the stretches decide: two segments each wholly on one side of the line
 * of the other, a segment within the disc of a circle or clear of it. Two
 * circles are kept apart by #arr_support_meets itself
 */
static bool
arr_stretches_apart(const Arr *a, uint32_t i, uint32_t j)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  if (! p->centred && ! q->centred)
  {
    const ArrNode *pa = &a->nodes[a->start[i]], *pb = &a->nodes[a->end[i]];
    const ArrNode *qa = &a->nodes[a->start[j]], *qb = &a->nodes[a->end[j]];
    int s1 = arr_orient_ival(pa, pb, qa), s2 = arr_orient_ival(pa, pb, qb);
    if (s1 != 2 && s1 != 0 && s1 == s2)
      return true;
    s1 = arr_orient_ival(qa, qb, pa);
    s2 = arr_orient_ival(qa, qb, pb);
    return s1 != 2 && s1 != 0 && s1 == s2;
  }
  if (! p->centred && q->centred)
    return arr_segment_misses_circle(a, i, j) ||
      arr_segment_circle_off(a, i, j);
  if (! q->centred && p->centred)
    return arr_segment_misses_circle(a, j, i) ||
      arr_segment_circle_off(a, j, i);
  return false;
}

/* The position along edge i of a point the intervals hold */
static inline Ival
arr_box_position(const Arr *a, uint32_t i, Ival bx, Ival by)
{
  const Edge *e = &a->e[i];
  return arr_is_arc(e) ? arr_arc_position(e, a->ccx[i], a->ccy[i], bx, by) :
    arr_segment_position(e, bx, by);
}

/* Whether the intervals place the crossing sigma of the circles of
 * stretches i and j, P = B + sigma f rot(d) as #arr_meet_arcs_exact reads it,
 * strictly before the start or after the end of either stretch */
static bool
arr_crossing_off(const Arr *a, uint32_t i, uint32_t j, int sigma)
{
  Ival dx = ival_sub(a->ccx[j], a->ccx[i]), dy = ival_sub(a->ccy[j],
    a->ccy[i]);
  Ival d2 = ival_dot(dx, dy, dx, dy);
  if (ival_sign(d2) != 1)
    return false;
  Ival u = ival_div(ival_add(d2, ival_sub(a->cr2[i], a->cr2[j])),
    ival_mul(ival(2.0), d2));
  Ival h2 = ival_sub(a->cr2[i], ival_mul(ival_mul(u, u), d2));
  if (ival_sign(h2) != 1)
    return false;
  Ival f = ival_sqrt(ival_div(h2, d2));
  Ival px = ival_add(ival_add(a->ccx[i], ival_mul(u, dx)),
    ival_mul(ival(-sigma), ival_mul(f, dy)));
  Ival py = ival_add(ival_add(a->ccy[i], ival_mul(u, dy)),
    ival_mul(ival(sigma), ival_mul(f, dx)));
  if (! isfinite(px.lo) || ! isfinite(px.hi) || ! isfinite(py.lo) ||
      ! isfinite(py.hi))
    return false;
  const uint32_t e[2] = {i, j};
  for (int k = 0; k < 2; k++)
  {
    Ival t = arr_box_position(a, e[k], px, py);
    if (t.hi < a->tfrom[e[k]].lo || t.lo > a->tto[e[k]].hi)
      return true;
  }
  return false;
}

/* Where the intervals place a point against a stretch: strictly off it (0),
 * strictly within it (1), or neither (2), its position @p t along the edge
 * against those of the ends */
static inline int
arr_ival_where(const Arr *a, uint32_t i, Ival t)
{
  if (t.hi < a->tfrom[i].lo || t.lo > a->tto[i].hi)
    return 0;
  if (t.lo > a->tfrom[i].hi && t.hi < a->tto[i].lo)
    return 1;
  return 2;
}

/* Node stretches i and j at a meeting strictly within both, placed by the
 * intervals @p px, @p py holding it, its coordinates their middles until the
 * rings are written (#arr_nodes_narrow). Intervals that hold it loosely, as
 * at the crossing of two nearly parallel segments, are narrowed exactly
 * first (#meet_point_bounds): the box is carried to the arrangements after
 * this one, whose filters and rays read it (#arr_point_clear) */
static ArrangeStatus
arr_ival_crossing(Arr *a, uint32_t i, uint32_t j, ArrMeet m, Ival px,
  Ival py, int sign)
{
  double box[4] = {px.lo, px.hi, py.lo, py.hi};
  double x = 0.5 * (px.lo + px.hi), y = 0.5 * (py.lo + py.hi);
  if (box[1] - box[0] > 0x1p-40 * fabs(x) ||
      box[3] - box[2] > 0x1p-40 * fabs(y))
  {
    MeetPoint mp = arr_meet_point(&m);
    double nx, ny, nb[4];
    if (meet_point_bounds(&mp, &nx, &ny, nb))
    {
      x = nx; y = ny;
      memcpy(box, nb, sizeof(box));
    }
  }
  uint32_t node = arr_tower_node_at(a, &m, x, y, box);
  a->nodes[node].cross = arr_cross_new(a, node, i, j, sign);
  ArrMeet am = a->nodes[node].meet;
  arr_cut(a, i, node, arr_box_position(a, i, px, py), am);
  arr_cut(a, j, node, arr_box_position(a, j, px, py), am);
  return ARR_ANSWERED;
}

/**
 * @brief Node two stretches on whole circles or segments between tangent
 * points where the intervals tell every meeting of their edges strictly off
 * either stretch or strictly within both, as #arr_meet_stretches tells the
 * crossing of two segments; false where they do not, which leaves the
 * meetings to the arithmetic of their towers (#arr_meet_curved)
 * @details Each point within both is the meeting that defines it, the
 * crossing of the two segments, the root sigma of a segment on a circle
 * (#towerseg_meet) or the crossing sigma of two circles
 * (#arcarc_meet_exact), placed by the intervals that told it
 */
static bool
arr_meet_ival(Arr *a, uint32_t i, uint32_t j)
{
  const Edge *p = &a->e[i], *q = &a->e[j];
  if (! p->centred && ! q->centred)
  {
    const ArrNode *na = &a->nodes[a->start[i]], *nb = &a->nodes[a->end[i]];
    const ArrNode *nc = &a->nodes[a->start[j]], *nd = &a->nodes[a->end[j]];
    Ival rx = ival_sub(nb->bx, na->bx), ry = ival_sub(nb->by, na->by);
    Ival sx = ival_sub(nd->bx, nc->bx), sy = ival_sub(nd->by, nc->by);
    Ival cx = ival_sub(nc->bx, na->bx), cy = ival_sub(nc->by, na->by);
    Ival den = ival_cross(rx, ry, sx, sy);
    int sd = ival_sign(den);
    if (sd != 1 && sd != -1)
      return false;
    Ival t = ival_div(ival_cross(cx, cy, sx, sy), den);
    Ival u = ival_div(ival_cross(cx, cy, rx, ry), den);
    if (! isfinite(t.lo) || ! isfinite(t.hi) || ! isfinite(u.lo) ||
        ! isfinite(u.hi))
      return false;
    if (t.hi < 0.0 || t.lo > 1.0 || u.hi < 0.0 || u.lo > 1.0)
      return true;
    if (! (t.lo > 0.0 && t.hi < 1.0 && u.lo > 0.0 && u.hi < 1.0))
      return false;
    ArrMeet m = {MEET_LINES, a->sup[i], a->sup[j], 0, 0.0, 0.0, NULL};
    arr_ival_crossing(a, i, j, m, ival_add(na->bx, ival_mul(t, rx)),
      ival_add(na->by, ival_mul(t, ry)), sd);
    return true;
  }
  if (p->centred && q->centred)
  {
    Ival dx = ival_sub(a->ccx[j], a->ccx[i]);
    Ival dy = ival_sub(a->ccy[j], a->ccy[i]);
    Ival d2 = ival_dot(dx, dy, dx, dy);
    if (ival_sign(d2) != 1)
      return false;
    Ival u = ival_div(ival_add(d2, ival_sub(a->cr2[i], a->cr2[j])),
      ival_mul(ival(2.0), d2));
    Ival h2 = ival_sub(a->cr2[i], ival_mul(ival_mul(u, u), d2));
    int sh = ival_sign(h2);
    if (sh == -1)
      return true;
    if (sh != 1)
      return false;
    Ival f = ival_sqrt(ival_div(h2, d2));
    Ival bx = ival_add(a->ccx[i], ival_mul(u, dx));
    Ival by = ival_add(a->ccy[i], ival_mul(u, dy));
    int c12 = (a->ccw[i] ? 1 : -1) * (a->ccw[j] ? 1 : -1);
    Ival px[2], py[2];
    int w[2];
    for (int r = 0; r < 2; r++)
    {
      int sigma = r ? 1 : -1;
      px[r] = ival_add(bx, ival_mul(ival(-sigma), ival_mul(f, dy)));
      py[r] = ival_add(by, ival_mul(ival(sigma), ival_mul(f, dx)));
      if (! isfinite(px[r].lo) || ! isfinite(px[r].hi) ||
          ! isfinite(py[r].lo) || ! isfinite(py[r].hi))
        return false;
      int wi = arr_ival_where(a, i, arr_box_position(a, i, px[r], py[r]));
      int wj = arr_ival_where(a, j, arr_box_position(a, j, px[r], py[r]));
      w[r] = (wi == 0 || wj == 0) ? 0 : ((wi == 1 && wj == 1) ? 1 : 2);
      if (w[r] == 2)
        return false;
    }
    for (int r = 0; r < 2; r++)
      if (w[r] == 1)
      {
        int sigma = r ? 1 : -1;
        ArrMeet m = {MEET_ARCARC, a->sup[i], a->sup[j], sigma, 0.0, 0.0,
          NULL};
        arr_ival_crossing(a, i, j, m, px[r], py[r], sigma * c12);
      }
    return true;
  }
  /* A segment s and a circle c */
  uint32_t s = p->centred ? j : i, c = p->centred ? i : j;
  const ArrNode *na = &a->nodes[a->start[s]], *nb = &a->nodes[a->end[s]];
  Ival ux = ival_sub(na->bx, a->ccx[c]), uy = ival_sub(na->by, a->ccy[c]);
  Ival vx = ival_sub(nb->bx, na->bx), vy = ival_sub(nb->by, na->by);
  Ival qa = ival_dot(vx, vy, vx, vy);
  Ival qb = ival_dot(vx, vy, ux, uy);
  Ival qc = ival_sub(ival_dot(ux, uy, ux, uy), a->cr2[c]);
  Ival disc = ival_sub(ival_mul(qb, qb), ival_mul(qa, qc));
  if (ival_sign(qa) != 1)
    return false;
  int sdisc = ival_sign(disc);
  if (sdisc == -1)
    return true;
  if (sdisc != 1)
    return false;
  Ival sq = ival_sqrt(disc);
  Ival px[2], py[2];
  int w[2];
  for (int r = 0; r < 2; r++)
  {
    /* The root sigma = 1 the farther along the segment, which the stretch
     * walks in its sense */
    Ival t = ival_div(ival_add(ival_neg(qb), r ? sq : ival_neg(sq)), qa);
    if (! isfinite(t.lo) || ! isfinite(t.hi))
      return false;
    px[r] = ival_add(na->bx, ival_mul(t, vx));
    py[r] = ival_add(na->by, ival_mul(t, vy));
    int ws = (t.hi < 0.0 || t.lo > 1.0) ? 0 :
      ((t.lo > 0.0 && t.hi < 1.0) ? 1 : 2);
    int wc = arr_ival_where(a, c, arr_box_position(a, c, px[r], py[r]));
    w[r] = (ws == 0 || wc == 0) ? 0 : ((ws == 1 && wc == 1) ? 1 : 2);
    if (w[r] == 2)
      return false;
  }
  for (int r = 0; r < 2; r++)
    if (w[r] == 1)
    {
      int sigma = r ? 1 : -1;
      ArrMeet m = {MEET_LINECIRCLE, a->sup[s], a->sup[c],
        arr_support_sigma(a, s, sigma), 0.0, 0.0, NULL};
      /* The segment leaves the disc at its farther root (#towerseg_meet) */
      int sign = sigma * (a->ccw[c] ? 1 : -1);
      arr_ival_crossing(a, i, j, m, px[r], py[r], (s == i) ? sign : - sign);
    }
  return true;
}

/**
 * @brief Node two stretches lying on whole circles or on segments between
 * tangent points where they meet
 * @details As #arr_meet_stretches nodes two stretches of segments: each
 * meeting of the two edges (#arr_support_meets) is placed against the ends
 * of either stretch exactly (#arr_stretch_where). A point strictly within
 * both is a node of both, the crossing of the two; within one and at an end
 * of the other, it is the node of that end, which cuts the first; at an end
 * of both, the two ends are one node. Two stretches of one edge, which the
 * arrangement that wrote them left apart, meet at their ends only, whose
 * nodes are one where they are one meeting (#arr_end_node). A stretch of a
 * segment meeting such a stretch is not answered
 */
static ArrangeStatus
arr_meet_curved(Arr *a, uint32_t i, uint32_t j)
{
  /* Two stretches of one boundary, which the arrangement that wrote it left
   * apart but at their ends, whose nodes are one where the ends are one
   * meeting (#arr_end_node) */
  if (a->sup[i] == a->sup[j] || arr_same_half(a, i, j))
    return ARR_ANSWERED;
  /* Whole circles and segments, between tangent points or of the input; an
   * arc of three points is not answered here */
  const Edge *p = &a->e[i], *q = &a->e[j];
  if ((arr_is_arc(p) && ! p->centred) || (arr_is_arc(q) && ! q->centred))
    return ARR_MEETING;
  /* Stretches the intervals keep apart, as #arr_meet_stretches passes over
   * segments that do not cross, or whose meetings they tell */
  if (arr_stretches_apart(a, i, j) || arr_meet_ival(a, i, j))
    return ARR_ANSWERED;
  ArrMeet m[2];
  int sign[2], n;
  ArrangeStatus st = arr_support_meets(a, i, j, m, sign, &n);
  /* Two segments between tangent points on one line: the stretches sharing
   * a stretch of it were merged before (#arr_merge_stretches), so these
   * meet at an end at most, as #arr_meet_stretches tells it of segments */
  if (st == ARR_OVERLAP && ! p->centred && ! q->centred)
  {
    int sh = arr_stretches_share(a, i, j);
    return (sh == 0) ? ARR_ANSWERED : (sh == 1 ? ARR_OVERLAP : ARR_ORDER);
  }
  if (st != ARR_ANSWERED)
    return st;
  for (int k = 0; k < n; k++)
  {
    /* A segment touching a circle within it crosses nothing, as two
     * circles touching (#arr_support_meets): no node */
    if (m[k].kind == MEET_LINECIRCLE && sign[k] == 0)
      continue;
    /* A crossing of two circles the intervals place off either stretch,
     * as #arr_meet_arcs_exact places it, is passed over before it is
     * placed exactly */
    if (m[k].kind == MEET_ARCARC && arr_crossing_off(a, i, j, m[k].sigma))
      continue;
    /* A tangent point is placed once, its node (#arr_tangent_node) */
    double x, y, box[4];
    if (m[k].kind == MEET_TANGENT)
    {
      uint32_t tn = arr_tangent_node(a, m[k].tp);
      if (tn == UINT32_MAX)
        return ARR_MEETING;
      const ArrNode *nd = &a->nodes[tn];
      x = nd->x; y = nd->y;
      box[0] = nd->bx.lo; box[1] = nd->bx.hi;
      box[2] = nd->by.lo; box[3] = nd->by.hi;
    }
    else
    {
      MeetPoint mp = arr_meet_point(&m[k]);
      if (! meet_point_bounds(&mp, &x, &y, box))
        return ARR_MEETING;
    }
    Ival bx = ival_out(box[0], box[1]), by = ival_out(box[2], box[3]);
    Ival ti = arr_box_position(a, i, bx, by);
    Ival tj = arr_box_position(a, j, bx, by);
    ArrWhere wi = arr_stretch_where(a, i, ti, &m[k]);
    ArrWhere wj = arr_stretch_where(a, j, tj, &m[k]);
    if (wi == ARR_UNTOLD || wj == ARR_UNTOLD)
      return ARR_ORDER;
    if (wi == ARR_OFF || wj == ARR_OFF)
      continue;
    if (wi == ARR_WITHIN && wj == ARR_WITHIN)
    {
      /* Two edges touching within both stretches meet without crossing */
      if (sign[k] == 0)
        return ARR_MEETING;
      uint32_t node = (m[k].kind == MEET_TANGENT) ?
        arr_tangent_node(a, m[k].tp) : arr_tower_node_at(a, &m[k], x, y, box);
      if (node == UINT32_MAX)
        return ARR_MEETING;
      if (m[k].kind != MEET_TANGENT)
        a->nodes[node].cross = arr_cross_new(a, node, i, j, sign[k]);
      ArrMeet am = a->nodes[node].meet;
      arr_cut(a, i, node, ti, am);
      arr_cut(a, j, node, tj, am);
      continue;
    }
    if (wi == ARR_WITHIN || wj == ARR_WITHIN)
    {
      /* An edge meeting the end of the other stretch within its own is cut
       * at that end; where a whole circle starts, which only the circle
       * reads (#arr_stretches_write), at the node of the meeting itself,
       * made one node with that start */
      uint32_t c = (wi == ARR_WITHIN) ? i : j, o = (c == i) ? j : i;
      ArrWhere wo = (c == i) ? wj : wi;
      uint32_t node = arr_stretch_end_node(a, o, wo);
      ArrMeet am = *arr_stretch_end_meet(a, o, wo);
      if (am.kind == MEET_ORIGIN)
      {
        uint32_t end = node;
        if (m[k].kind == MEET_VERTEX)
          node = arr_vertex(a, m[k].x, m[k].y);
        else if (m[k].kind == MEET_TANGENT)
          node = arr_tangent_node(a, m[k].tp);
        else
          node = arr_tower_node_at(a, &m[k], x, y, box);
        if (node == UINT32_MAX)
          return ARR_MEETING;
        am = (m[k].kind == MEET_VERTEX) ? m[k] : a->nodes[node].meet;
        arr_same_add(a, node, end);
      }
      arr_cut(a, c, node, (c == i) ? ti : tj, am);
      continue;
    }
    uint32_t ni = arr_stretch_end_node(a, i, wi);
    uint32_t nj = arr_stretch_end_node(a, j, wj);
    if (ni != nj)
      arr_same_add(a, ni, nj);
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
    /* A segment between tangent points carries the box holding it */
    box[0] = e->from ? e->xmin : Min(e->x1, e->x2);
    box[1] = e->from ? e->xmax : Max(e->x1, e->x2);
    box[2] = e->from ? e->ymin : Min(e->y1, e->y2);
    box[3] = e->from ? e->ymax : Max(e->y1, e->y2);
    return;
  }
  double pad = 16.0 * DBL_EPSILON * (fabs(e->cx) + fabs(e->cy) + e->radius);
  Ival x = ival_out(e->xmin - pad, e->xmax + pad);
  Ival y = ival_out(e->ymin - pad, e->ymax + pad);
  box[0] = x.lo; box[1] = x.hi;
  box[2] = y.lo; box[3] = y.hi;
  return;
}

typedef struct
{
  double xmin;
  uint32_t id;
} ArrSweep;

static inline int
arr_sweep_cmp(const ArrSweep *p, const ArrSweep *q)
{
  if (p->xmin < q->xmin)
    return -1;
  if (p->xmin > q->xmin)
    return 1;
  return (p->id < q->id) ? -1 : (p->id > q->id) ? 1 : 0;
}

/* Sort the sweep with its order written into the sort, as
 * #buffer_sweep_edge_sort does */
#define ST_SORT arr_sweep_sort
#define ST_ELEMENT_TYPE ArrSweep
#define ST_COMPARE(a, b) arr_sweep_cmp(a, b)
#define ST_SCOPE static
#define ST_DECLARE
#define ST_DEFINE
#include "port/sort_template.h"

/* A pair of edges whose boxes meet */
typedef struct
{
  uint32_t i, j;
} ArrPair;

/**
 * @brief Return the pairs of edges whose boxes meet, in the order a sweep
 * over the left ends of the boxes meets them
 * @details The boxes an edge's box meets follow it in the sweep until the
 * left end of one passes its right end, and none after that can. The pairs
 * are read once for the whole arrangement: #arr_merge_overlaps reads the
 * segments among them that run along each other, and #arr_node_all the
 * meetings of all of them (#arr_mapped_pairs)
 * @param[in] box The box of every edge (#arr_box)
 * @param[in] n Number of edges
 * @param[out] npairs Number of pairs
 */
static ArrPair *
arr_box_pairs(const double (*box)[4], uint32_t n, uint32_t *npairs)
{
  ArrSweep *sw = palloc(sizeof(ArrSweep) * Max(n, 1u));
  for (uint32_t i = 0; i < n; i++)
  {
    sw[i].xmin = box[i][0];
    sw[i].id = i;
  }
  arr_sweep_sort(sw, n);
  /* The boxes in the order of the sweep, which its inner loop reads one
   * after the other */
  double (*sb)[4] = palloc(sizeof(double[4]) * Max(n, 1u));
  for (uint32_t k = 0; k < n; k++)
    memcpy(sb[k], box[sw[k].id], sizeof(double[4]));
  uint32_t cap = Max(2 * n, 16u), count = 0;
  ArrPair *pairs = palloc(sizeof(ArrPair) * cap);
  for (uint32_t k = 0; k < n; k++)
  {
    const double *bi = sb[k];
    for (uint32_t l = k + 1; l < n; l++)
    {
      const double *bj = sb[l];
      if (bj[0] > bi[1])
        break;
      if (bj[2] > bi[3] || bi[2] > bj[3])
        continue;
      if (count == cap)
      {
        cap *= 2;
        pairs = repalloc(pairs, sizeof(ArrPair) * cap);
      }
      pairs[count].i = sw[k].id;
      pairs[count++].j = sw[l].id;
    }
  }
  pfree(sb);
  pfree(sw);
  *npairs = count;
  return pairs;
}

/**
 * @brief Node every pair of edges whose boxes meet (#arr_box_pairs)
 */
static ArrangeStatus
arr_node_all(Arr *a, const ArrPair *pairs, uint32_t npairs)
{
  ArrangeStatus st = ARR_ANSWERED;
  for (uint32_t k = 0; k < npairs && st == ARR_ANSWERED; k++)
  {
    uint32_t i = pairs[k].i, j = pairs[k].j;
    bool ai = arr_is_arc(&a->e[i]), aj = arr_is_arc(&a->e[j]);
    if (a->sup && ! ai && ! aj && ! a->e[i].from && ! a->e[j].from)
      st = arr_meet_stretches(a, i, j);
    else if (a->sup)
      st = arr_meet_curved(a, i, j);
    else if (a->e[i].from || a->e[j].from)
      st = arr_meet_tower(a, i, j);
    else if (! ai && ! aj)
      st = arr_meet_segments(a, i, j);
    else if (! ai)
      st = arr_meet_segment_arc(a, i, j);
    else if (! aj)
      st = arr_meet_segment_arc(a, j, i);
    else
      st = arr_meet_arcs(a, i, j);
  }
  return st;
}

static inline int
arr_cut_cmp(const ArrCut *p, const ArrCut *q)
{
  if (p->pos.lo < q->pos.lo)
    return -1;
  if (p->pos.lo > q->pos.lo)
    return 1;
  return (p->node < q->node) ? -1 : (p->node > q->node) ? 1 : 0;
}

/* Sort the cuts of an edge with their order written into the sort, as
 * #buffer_sweep_edge_sort does */
#define ST_SORT arr_cut_sort
#define ST_ELEMENT_TYPE ArrCut
#define ST_COMPARE(a, b) arr_cut_cmp(a, b)
#define ST_SCOPE static
#define ST_DECLARE
#define ST_DEFINE
#include "port/sort_template.h"

/* Return how two cuts of an edge are ordered along it: by their intervals,
 * and where those overlap by #meets_order_exact; 0 for one node, 3 for two
 * nodes at one point, 2 for an order that cannot be told */
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
  MeetPoint m1 = arr_meet_point(&c1->meet);
  MeetPoint m2 = arr_meet_point(&c2->meet);
  int c = meets_order_exact(&a->e[edge], a->ccw[edge], &m1, &m2);
  return (c == 0) ? 3 : c;
}

/* The node standing for a node, halving the path walked */
static uint32_t
arr_node_rep(Arr *a, uint32_t v)
{
  while (a->nrep[v] != v)
  {
    a->nrep[v] = a->nrep[a->nrep[v]];
    v = a->nrep[v];
  }
  return v;
}

/* Make one node of two at one point: an input vertex, whose coordinates are
 * exact, stands for both where one of them is, and the node is no crossing
 * of two edges any more but a node several edges pass through, ordered
 * around by their directions (#arr_order_nodes); a crossing standing for
 * both keeps the edges that place it exactly */
static void
arr_node_merge(Arr *a, uint32_t v, uint32_t w)
{
  v = arr_node_rep(a, v);
  w = arr_node_rep(a, w);
  if (v == w)
    return;
  /* Where a whole circle starts stands for the node only where nothing
   * else does: the end of a stretch of another edge is never that, which
   * only the circle reads (#arr_stretches_write) */
  if ((a->nodes[w].vertex && ! a->nodes[v].vertex) ||
      (! a->nodes[v].vertex && a->nodes[v].origin && ! a->nodes[w].origin))
  {
    uint32_t t = v; v = w; w = t;
  }
  a->nrep[w] = v;
  a->nodes[v].merged = true;
  return;
}

/**
 * @brief Cut every edge into pieces at its nodes, in their order along it
 * @details A node cut twice into one edge is one cut. Two cuts at distinct
 * nodes whose intervals overlap are ordered exactly (#meets_order_exact),
 * and two distinct nodes at one point, as where three edges cross at one
 * point, are made one node (#arr_node_merge); an order that cannot be told
 * leaves the arrangement unanswered. The cuts of every edge are put in order
 * first and the pieces cut after, so that a node merged while ordering a
 * later edge is one node in the pieces of the earlier ones too
 */
static ArrangeStatus
arr_pieces(Arr *a)
{
  /* The cuts grouped by edge, in the order they were made within each */
  uint32_t n = a->n;
  a->coff = palloc0(sizeof(uint32_t) * (n + 1));
  for (uint32_t k = 0; k < a->ncuts; k++)
    a->coff[a->cuts[k].edge + 1]++;
  for (uint32_t i = 0; i < n; i++)
    a->coff[i + 1] += a->coff[i];
  ArrCut *grouped = palloc(sizeof(ArrCut) * Max(a->ncuts, 1u));
  uint32_t *fill = palloc(sizeof(uint32_t) * Max(n, 1u));
  memcpy(fill, a->coff, sizeof(uint32_t) * n);
  for (uint32_t k = 0; k < a->ncuts; k++)
    grouped[fill[a->cuts[k].edge]++] = a->cuts[k];
  pfree(fill);
  if (a->cuts)
    pfree(a->cuts);
  a->cuts = grouped;
  a->nrep = palloc(sizeof(uint32_t) * Max(a->nnodes, 1u));
  for (uint32_t v = 0; v < a->nnodes; v++)
    a->nrep[v] = v;
  for (uint32_t k = 0; k < a->nsame; k++)
    arr_node_merge(a, a->same[k][0], a->same[k][1]);
  for (uint32_t i = 0; i < n; i++)
  {
    ArrCut *cuts = &a->cuts[a->coff[i]];
    uint32_t ncut = a->coff[i + 1] - a->coff[i];
    if (ncut > 1)
      arr_cut_sort(cuts, ncut);
    /* Cuts whose intervals overlap are put in their exact order: the sort
     * by the lower ends leaves each within its run of overlapping ones */
    for (uint32_t k = 1; k < ncut; k++)
      for (uint32_t m = k; m > 0; m--)
      {
        int o = arr_cut_order(a, i, &cuts[m - 1], &cuts[m]);
        if (o == 2)
          return ARR_ORDER;
        if (o == 3)
          arr_node_merge(a, cuts[m - 1].node, cuts[m].node);
        if (o <= 0 || o == 3)
          break;
        ArrCut t = cuts[m]; cuts[m] = cuts[m - 1];
        cuts[m - 1] = t;
      }
  }
  a->pieces = palloc(sizeof(ArrPiece) * Max(n + a->ncuts, 1u));
  a->npieces = 0;
  for (uint32_t i = 0; i < n; i++)
  {
    ArrCut *cuts = &a->cuts[a->coff[i]];
    uint32_t ncut = a->coff[i + 1] - a->coff[i];
    for (uint32_t k = 0; k < ncut; k++)
      cuts[k].node = arr_node_rep(a, cuts[k].node);
    uint32_t from = arr_node_rep(a, a->start[i]);
    uint32_t end = arr_node_rep(a, a->end[i]);
    /* A segment runs from 0 to 1 and an arc from 0 on, see
     * #arr_arc_position; a stretch between the positions of its ends */
    Ival pfrom = a->tfrom ? a->tfrom[i] : ival(0.0);
    Ival pend = a->tto ? a->tto[i] :
      ival(arr_is_arc(&a->e[i]) ? INFINITY : 1.0);
    /* A cut at the node a piece starts at, that of the previous cut or the
     * start of the edge made one node with a meeting there (#arr_node_merge),
     * cuts nothing; nor does a last cut at the node of the end */
    uint32_t first = a->npieces;
    for (uint32_t k = 0; k < ncut; k++)
    {
      const ArrCut *cut = &cuts[k];
      if (cut->node == from)
        continue;
      ArrPiece *p = &a->pieces[a->npieces++];
      p->edge = i; p->from = from; p->to = cut->node;
      p->pfrom = pfrom; p->pto = cut->pos;
      p->left = p->right = INT_MIN;
      p->result = p->flip = p->used = false;
      from = cut->node;
      pfrom = cut->pos;
    }
    if (a->npieces > first && from == end)
    {
      a->pieces[a->npieces - 1].pto = pend;
      continue;
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
  const ArrNode *nd = &a->nodes[node];
  if (arr_is_arc(e))
  {
    /* At a node no double holds, the box holding it */
    if (nd->tower)
      arr_arc_tangent(a, p->edge, nd->bx, nd->by, dx, dy);
    else
      arr_arc_tangent(a, p->edge, ival(nd->x), ival(nd->y), dx, dy);
  }
  else if (e->from)
  {
    /* A segment between tangent points: the boxes of its two ends */
    const ArrNode *n0 = &a->nodes[a->start[p->edge]];
    const ArrNode *n1 = &a->nodes[a->end[p->edge]];
    *dx = ival_sub(n1->bx, n0->bx);
    *dy = ival_sub(n1->by, n0->by);
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
  /* A node no double holds is read in its tower */
  if (nd->tower)
  {
    MeetPoint at = arr_meet_point(&nd->meet);
    at.p = nd->meet.p; at.q = nd->meet.q;
    return edges_turn_cmp_meet(h1 ? &a->e[e1] : NULL, h1 && a->ccw[e1],
      h1 && h1->out, h2 ? &a->e[e2] : NULL, h2 && a->ccw[e2],
      h2 && h2->out, &at);
  }
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
    if (nd->cross >= 0 && ! nd->merged)
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
 * The winding number of a piece rises by the weight of its edge from its
 * right to its left, the side the curves wind around -- by one where a single
 * curve runs along it (#arr_weight) -- so each piece is labelled by either
 * side, and a node passes its labels around it: turning counterclockwise past
 * a half leaving the node crosses its piece from right to left, past one
 * arriving from left to right.
 */

/* Label a piece by the winding number on its right; false where it carries
 * another */
static bool
arr_label(const Arr *a, ArrPiece *p, int right)
{
  if (p->right != INT_MIN)
    return p->right == right;
  p->right = right;
  p->left = right + arr_weight(a, p->edge);
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
    if (! arr_label(a, p, hk->out ? w : w - arr_weight(a, p->edge)))
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

/* Return how the points of the circles of two centred arcs furthest along x
 * compare, cx + r against cx + r, decided exactly: -1, 0 or 1 */
static int
arr_centred_right_cmp(const Edge *p, const Edge *q)
{
  const double t[4] = {p->cx, p->radius, - q->cx, - q->radius};
  ExactSum sum;
  exact_sum_set(&sum, t, 4);
  PolyTerm term = {1.0, 1, {0}};
  return polynomial_sign_exact(&sum, &term, 1);
}

/* Return the side of the centre C of the circle of an arc a height y lies
 * on, sign(C.y - y), decided exactly: the centre given for a centred arc,
 * the middle of the diameter for a half of a full circle, and for an arc of
 * three points S, M, E the circumcentre C = S + U with
 * U.y = (a.x |b|^2 - b.x |a|^2) / D, a = M - S, b = E - S,
 * D = 2 (a.x b.y - a.y b.x), so that C.y - y = N / D with
 * N = (S.y - y) D + a.x |b|^2 - b.x |a|^2, a polynomial in the differences
 * of the input doubles (#polynomial_sign_exact, as #arc_same_circle reads
 * them); 2 where three collinear points fix no circle */
static int
arr_arc_centre_y_side(const Edge *e, double y)
{
  if (e->centred)
    return (e->cy > y) - (e->cy < y);
  if (e->diameter)
  {
    const double t[3] = {e->sy, e->ey, -2.0 * y};
    ExactSum sum;
    exact_sum_set(&sum, t, 3);
    PolyTerm term = {1.0, 1, {0}};
    return polynomial_sign_exact(&sum, &term, 1);
  }
  const double t[5][2] = {{e->mx, - e->sx}, {e->my, - e->sy},
    {e->ex, - e->sx}, {e->ey, - e->sy}, {e->sy, - y}};
  ExactSum sums[5];
  for (int k = 0; k < 5; k++)
    exact_sum_set(&sums[k], t[k], 2);
  /* ax 0, ay 1, bx 2, by 3, S.y - y 4 */
  const PolyTerm dterms[2] = {{2.0, 2, {0, 3}}, {-2.0, 2, {1, 2}}};
  const PolyTerm nterms[6] = {{2.0, 3, {4, 0, 3}}, {-2.0, 3, {4, 1, 2}},
    {1.0, 3, {0, 2, 2}}, {1.0, 3, {0, 3, 3}}, {-1.0, 3, {2, 0, 0}},
    {-1.0, 3, {2, 1, 1}}};
  int sd = polynomial_sign_exact(sums, dterms, 2);
  if (sd == 0)
    return 2;
  return polynomial_sign_exact(sums, nterms, 6) * sd;
}

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
 * @brief Return the side of the horizontal line y = @p py a node lies on:
 * -1 below, 0 on it, 1 above, 2 where it is not told
 * @details A vertex is read off its coordinates. A crossing of segments P and
 * Q lies at P1 + t (P2 - P1) with t = ((Q1 - P1) x S) / (R x S), R = P2 - P1
 * and S = Q2 - Q1, so its height less @p py has the sign of R x S times
 * (P1y - py) (R x S) + Ry ((Q1 - P1) x S), a polynomial of degree three in
 * differences of the doubles of the two segments, whose sign is decided
 * exactly (#polynomial_sign_exact)
 */
static int
arr_node_side(const Arr *a, uint32_t v, double py)
{
  const ArrNode *nd = &a->nodes[v];
  if (nd->vertex)
    return (nd->y > py) - (nd->y < py);
  /* Clear of the line by the intervals holding it: a crossing with an arc,
   * whose height no polynomial below decides, as much as any other */
  if (nd->by.lo > py)
    return 1;
  if (nd->by.hi < py)
    return -1;
  if (nd->tower)
  {
    MeetPoint at = arr_meet_point(&nd->meet);
    at.p = nd->meet.p; at.q = nd->meet.q;
    return meet_point_y_side(&at, py);
  }
  const Edge *p = nd->xp, *q = nd->xq;
  if (! p || arr_is_arc(p) || arr_is_arc(q) || nd->xsign == 0)
    return 2;
  /* The exact sums: P1y - py, Rx, Ry, Sx, Sy, Q1x - P1x, Q1y - P1y */
  double t[7][2] = {
    {p->y1, -py}, {p->x2, -p->x1}, {p->y2, -p->y1}, {q->x2, -q->x1},
    {q->y2, -q->y1}, {q->x1, -p->x1}, {q->y1, -p->y1}};
  ExactSum sums[7];
  for (int k = 0; k < 7; k++)
    exact_sum_set(&sums[k], t[k], 2);
  /* (P1y - py) (Rx Sy - Ry Sx) + Ry ((Q1x - P1x) Sy - (Q1y - P1y) Sx) */
  PolyTerm terms[4] = {
    {1.0, 3, {0, 1, 4, 0}}, {-1.0, 3, {0, 2, 3, 0}},
    {1.0, 3, {2, 5, 4, 0}}, {-1.0, 3, {2, 6, 3, 0}}};
  return nd->xsign * polynomial_sign_exact(sums, terms, 4);
}

/**
 * @brief Count a crossing of a piece by the ray from a point along the
 * positive x axis
 * @details A crossing adds to @p w the weight of the piece's edge
 * (#arr_weight) where the piece runs upward in the sense of its edge, minus
 * it where it runs downward, and to @p c one. A segment piece is read by the
 * half-open rule: it is crossed where one of its ends lies above the ray and
 * the other does not (#arr_node_side), right of the point where the point
 * lies left of a rising edge or right of a falling one, all of it decided
 * exactly, so a ray through an end of a piece or along a piece is counted
 * like any other
 * @return false where a side is not told, or the point lies on the piece,
 * or the intervals cannot tell where the ray meets an arc
 */
static bool
arr_piece_ray(const Arr *a, const ArrPiece *p, Ival px, double py, int *w,
  int *c)
{
  /* A piece whose edge's box lies above or below the ray, or left of the
   * point, keeps clear of it: the box only admits the pieces asked. Nor
   * does linework, which winds around nothing (#arr_weight) */
  const double *b = a->box[p->edge];
  if (b[3] < py || b[2] > py || b[1] < px.lo || arr_weight(a, p->edge) == 0)
    return true;
  const Edge *e = &a->e[p->edge];
  if (! arr_is_arc(e))
  {
    if (px.lo != px.hi)
      return false;
    /* The piece runs from p->from to p->to in the sense of its edge */
    int sf = arr_node_side(a, p->from, py), st = arr_node_side(a, p->to, py);
    if (sf == 2 || st == 2)
      return false;
    bool below_from = sf <= 0, below_to = st <= 0;
    if (below_from == below_to)
      return true;
    bool up = below_from;
    int side = e->from ? towerseg_side(e, px.lo, py) :
      cross_product_sign(e->x1, e->y1, e->x2, e->y2, e->x1, e->y1, px.lo,
        py);
    if (side == 2)
      return false;
    if (side == 0)
      return false;
    if (up ? side < 0 : side > 0)
      return true;
    *w += up ? arr_weight(a, p->edge) : -arr_weight(a, p->edge);
    (*c)++;
    return true;
  }
  /* An arc: the points of its circle at height py, right of the point and
   * within the piece */
  uint32_t j = p->edge;
  Ival dy = ival_sub(ival(py), a->ccy[j]);
  Ival h2 = ival_sub(a->cr2[j], ival_mul(dy, dy));
  int sh = ival_sign(h2);
  /* For a centred arc the sign of r^2 - (py - cy)^2 is that of the side of
   * its circle (cx, py) lies on, decided exactly: a ray touching the circle
   * at its top or bottom crosses nothing */
  if (sh == 2 && e->centred)
    sh = - arc_circle_side(e, e->cx, py);
  if (sh == 0 && e->centred)
  {
    /* The ray touches the circle at its top or bottom, (cx, py). The
     * half-open rule reads the ray as passing just above a point on it, as
     * for the ends of a segment: above the top it misses the circle, and
     * just above the bottom it crosses the circle on either side of it, so
     * a piece holding the bottom within it is crossed twice, up and down,
     * which leaves the count even and the winding number as it is, while a
     * piece starting at the bottom is crossed once, rising from it, and one
     * ending there once, falling into it */
    if (py > e->cy)
      return true;
    /* The bottom stands a quarter of a turn from the start of a
     * counterclockwise circle and three quarters from that of a clockwise
     * one (#arr_stretch_box): an end whose position keeps clear of it is
     * not there */
    double bottom = a->ccw[p->edge] ? 1.0 : 3.0;
    if ((p->pfrom.hi < bottom || p->pfrom.lo > bottom) &&
        (p->pto.hi < bottom || p->pto.lo > bottom))
      return true;
    int sf = arr_node_side(a, p->from, py), st = arr_node_side(a, p->to, py);
    if (sf == 2 || st == 2)
      return false;
    if ((sf != 0) == (st != 0))
      return true;
    if (px.lo != px.hi)
      return false;
    if (e->cx == px.lo)
      return false;
    if (e->cx < px.lo)
      return true;
    bool up = (sf == 0);
    *w += up ? arr_weight(a, p->edge) : -arr_weight(a, p->edge);
    (*c)++;
    return true;
  }
  if (sh == -1)
    return true;
  if (sh != 1)
    return false;
  Ival hx = ival_sqrt(h2);
  for (int side = -1; side <= 1; side += 2)
  {
    Ival qx = ival_add(a->ccx[j], side < 0 ? ival_neg(hx) : hx);
    /* Within the piece, asked first: a point of the circle off the arc or
     * off the piece is passed over wherever it stands against the point,
     * as where the point lies on another piece of the edge */
    int onarc = arr_on_arc(a, j, qx, ival(py));
    if (onarc == 0)
      continue;
    /* The start of a centred arc, (cx - r, cy) exactly, met by a ray along
     * y = cy: a point of the circle like any other, which no edge but this
     * one passes through, read on the piece leaving it */
    bool start = e->centred && side < 0 && py == e->cy;
    if (start && a->nodes[p->from].origin != arr_edge_ref(a, j))
      continue;
    Ival pos = arr_arc_position(e, a->ccx[j], a->ccy[j], qx, ival(py));
    if (! start && onarc == 1 &&
        (pos.hi < p->pfrom.lo || pos.lo > p->pto.hi))
      continue;
    int right = ival_sign(ival_sub(qx, px));
    if (right == 2 || right == 0)
      return false;
    if (right < 0)
      continue;
    if (onarc == 2)
      return false;
    if (! start && ! (pos.lo > p->pfrom.hi && pos.hi < p->pto.lo))
      return false;
    /* Upward on the right of a counterclockwise circle */
    bool up = (side > 0) == a->ccw[j];
    *w += up ? arr_weight(a, p->edge) : -arr_weight(a, p->edge);
    (*c)++;
  }
  return true;
}

/**
 * @brief Return the winding number of the region around a point, counted by
 * the labelled pieces a ray from it along the positive x axis crosses
 * (#arr_piece_ray)
 * @return false where the intervals cannot tell a crossing
 */
static bool
arr_winding_at(const Arr *a, const uint32_t *cpieces, const uint32_t *coff,
  const double (*cbox)[4], uint32_t ncomp, Ival px, double py, int *w)
{
  *w = 0;
  int c = 0;
  for (uint32_t m = 0; m < ncomp; m++)
  {
    /* A component whose box the ray cannot meet is passed over whole */
    const double *cb = cbox[m];
    if (cb[3] < py || cb[2] > py || cb[1] < px.lo)
      continue;
    for (uint32_t k = coff[m]; k < coff[m + 1]; k++)
    {
      const ArrPiece *p = &a->pieces[cpieces[k]];
      /* The box test of #arr_piece_ray, read here before the call */
      const double *b = a->box[p->edge];
      if (b[3] < py || b[2] > py || b[1] < px.lo)
        continue;
      if (! arr_piece_ray(a, p, px, py, w, &c))
        return false;
    }
  }
  return true;
}

/* The meeting a node is, as #meets_order_exact reads it: its coordinates for
 * a vertex, the two edges placing it for a crossing */
static ArrMeet
arr_node_meet(const Arr *a, uint32_t v)
{
  const ArrNode *nd = &a->nodes[v];
  if (nd->tower)
    return nd->meet;
  ArrMeet m = {nd->vertex ? MEET_VERTEX : MEET_SEGSEG,
    nd->vertex ? NULL : nd->xp, nd->vertex ? NULL : nd->xq, 0, nd->x, nd->y,
    NULL};
  return m;
}

/* Return how the x coordinates of two nodes compare, exactly: their order
 * along a horizontal segment pointing along the positive x axis
 * (#meets_order_exact); 2 where it cannot be told */
static int
arr_node_x_cmp(const Arr *a, uint32_t v, uint32_t w)
{
  const ArrNode *nv = &a->nodes[v], *nw = &a->nodes[w];
  /* Points no double holds, read in their towers, as a vertex and the
   * crossing of two segments of the input are (#meet_tower_build) */
  bool tv = nv->tower || nv->vertex ||
    (nv->xp && ! arr_is_arc(nv->xp) && ! arr_is_arc(nv->xq));
  bool tw = nw->tower || nw->vertex ||
    (nw->xp && ! arr_is_arc(nw->xp) && ! arr_is_arc(nw->xq));
  if ((nv->tower || nw->tower) && tv && tw)
  {
    ArrMeet mv = arr_node_meet(a, v), mw = arr_node_meet(a, w);
    MeetPoint pv = arr_meet_point(&mv), pw = arr_meet_point(&mw);
    pv.p = mv.p; pv.q = mv.q;
    pw.p = mw.p; pw.q = mw.q;
    return meets_x_cmp(&pv, &pw);
  }
  if (! nv->vertex && (nv->tower || ! nv->xp || arr_is_arc(nv->xp)))
    return 2;
  if (! nw->vertex && (nw->tower || ! nw->xp || arr_is_arc(nw->xp)))
    return 2;
  Edge h;
  memset(&h, 0, sizeof(Edge));
  h.etype = EDGE_LINESEG;
  h.x1 = nv->x; h.y1 = h.y2 = nv->y;
  h.x2 = nv->x + fmax(1.0, fabs(nv->x));
  ArrMeet mv = arr_node_meet(a, v), mw = arr_node_meet(a, w);
  MeetPoint pv = arr_meet_point(&mv), pw = arr_meet_point(&mw);
  return meets_order_exact(&h, false, &pv, &pw);
}

/**
 * @brief Return the rightmost point of the pieces of a component, false where
 * the intervals cannot tell which point lies furthest right
 * @details A vertex, or the point of an arc's circle furthest along x where
 * the arc passes it (#arr_piece_holds_right)
 * @param[in] a Arrangement
 * @param[in] comp,cn The nodes of the component
 * @param[out] out Its rightmost point
 */
/* Whether node v, an end of a piece of the centred arc e, is the point of
 * its circle furthest along x: on the circle at the height of its centre,
 * right of it, the side of the line y = cy decided exactly as
 * #arr_node_side decides it */
static bool
arr_node_circle_right(const Arr *a, uint32_t v, const Edge *e)
{
  const ArrNode *nd = &a->nodes[v];
  if (nd->origin || ! (nd->bx.lo > e->cx))
    return false;
  if (nd->vertex)
    return nd->y == e->cy;
  if (! nd->tower)
    return false;
  MeetPoint at = arr_meet_point(&nd->meet);
  at.p = nd->meet.p; at.q = nd->meet.q;
  return meet_point_y_side(&at, e->cy) == 0;
}

/* The sign of the x of node v less that of the point of the circle of the
 * centred arc e furthest along x, cx + r: -1, 0 or 1, 2 where it is not
 * told; @p same tells whether the node is that point. A node no double
 * holds is read in its tower (#meet_point_x_side, #meet_point_y_side), a
 * vertex against the sum read with its rounding error (two-sum), as
 * #arr_node_circle_right reads the height */
static int
arr_node_right_x(const Arr *a, uint32_t v, const Edge *e, bool *same)
{
  const ArrNode *nd = &a->nodes[v];
  *same = false;
  if (nd->vertex)
  {
    double s = e->cx + e->radius;
    double bv = s - e->cx;
    double err = (e->cx - (s - bv)) + (e->radius - bv);
    int c = (nd->x > s) ? 1 : (nd->x < s) ? -1 :
      ((err > 0.0) ? -1 : (err < 0.0) ? 1 : 0);
    *same = (c == 0 && nd->y == e->cy);
    return c;
  }
  if (! nd->tower)
    return 2;
  MeetPoint at = arr_meet_point(&nd->meet);
  at.p = nd->meet.p; at.q = nd->meet.q;
  int c = meet_point_x_side(&at, e->cx, e->radius);
  if (c == 2)
    return 2;
  if (c == 0)
  {
    int y = meet_point_y_side(&at, e->cy);
    if (y == 2)
      return 2;
    *same = (y == 0);
  }
  return c;
}

/* Where node v of the circle of centred arc j stands against the point of
 * that circle furthest along x, (cx + r, cy), walked in the sense of the arc
 * from its start, the point furthest along -x: -1 before it, 1 after it, 0
 * at it, 2 where it is not told. Counterclockwise the walk runs below the
 * line y = cy before it and above after it, clockwise the other way, so the
 * side of that line decides, exactly as #arr_node_side decides it; the
 * start of the arc stands before every point and after it as its end */
static int
arr_node_right_order(const Arr *a, uint32_t j, uint32_t v, bool start)
{
  const ArrNode *nd = &a->nodes[v];
  const Edge *e = &a->e[j];
  if (nd->origin && nd->origin == arr_edge_ref(a, j))
    return start ? -1 : 1;
  int s;
  if (nd->vertex)
    s = (nd->y > e->cy) - (nd->y < e->cy);
  else if (nd->tower)
  {
    MeetPoint at = arr_meet_point(&nd->meet);
    at.p = nd->meet.p; at.q = nd->meet.q;
    s = meet_point_y_side(&at, e->cy);
    if (s == 2)
      return 2;
  }
  else
    return 2;
  /* On the line, a point of the circle other than its start is the point
   * furthest along x */
  if (s == 0)
    return (nd->bx.lo > e->cx) ? 0 : 2;
  return a->ccw[j] ? (s < 0 ? -1 : 1) : (s > 0 ? -1 : 1);
}

/* Whether the piece of a centred arc holds the point of its circle furthest
 * along x strictly within it, decided exactly by the order of its ends
 * against that point (#arr_node_right_order): 1, 0, or 2 where it is not
 * told */
static int
arr_piece_holds_right_exact(const Arr *a, const ArrPiece *p)
{
  int f = arr_node_right_order(a, p->edge, p->from, true);
  int t = arr_node_right_order(a, p->edge, p->to, false);
  if (f == 2 || t == 2)
    return 2;
  return (f < 0 && t > 0) ? 1 : 0;
}

static bool
arr_component_extreme(const Arr *a, const uint32_t *comp, uint32_t cn,
  ArrExtreme *out)
{
  ArrExtreme ex = {false, UINT32_MAX, UINT32_MAX, {-INFINITY, -INFINITY}};
  bool unsure = false;
  for (uint32_t c = 0; c < cn && ! unsure; c++)
  {
    const ArrNode *nd = &a->nodes[comp[c]];
    /* Stretches end at crossings as well, which are told apart from the
     * others by their intervals and exactly where those overlap; the start
     * of a circle is its point furthest along -x, never the rightmost */
    if ((! nd->vertex && ! a->sup) || nd->origin)
      continue;
    if (ex.node == UINT32_MAX || nd->bx.lo > ex.x.hi)
    {
      ex.node = comp[c];
      ex.x = nd->bx;
      continue;
    }
    if (nd->bx.hi < ex.x.lo || (nd->vertex &&
        a->nodes[ex.node].vertex && nd->x <= ex.x.lo))
      continue;
    int cmp = arr_node_x_cmp(a, comp[c], ex.node);
    if (cmp == 2)
      unsure = true;
    else if (cmp > 0)
    {
      ex.node = comp[c];
      ex.x = nd->bx;
    }
  }
  if (unsure)
  {
    *out = ex;
    return false;
  }
  Ival tie = {INFINITY, -INFINITY};
  bool tied = false;
  for (uint32_t c = 0; c < cn; c++)
  {
    uint32_t v = comp[c];
    for (uint32_t k = a->hoff[v]; k < a->hoff[v + 1]; k++)
    {
      if (! a->halves[k].out)
        continue;
      const ArrPiece *p = &a->pieces[a->halves[k].piece];
      if (! arr_is_arc(&a->e[p->edge]))
        continue;
      /* An arc whose box ends left of the rightmost point found holds no
       * point further right */
      if (a->box[p->edge][1] < ex.x.lo)
        continue;
      Ival x;
      int holds = arr_piece_holds_right(a, p, &x);
      /* Where the intervals cannot tell, as at an end within a few units in
       * the last place of that point, the order of the ends tells; only
       * among stretches, whose ends compete with the arcs, as a piece ending
       * at that point leaves it to the node there */
      /* An arc ending at a vertex the point of its circle furthest along x,
       * R, lies within the intervals of: an arc leaves its start A toward
       * +x, and so passes R, exactly where A lies below the centre when it
       * turns counterclockwise, above it when it turns clockwise, and
       * reaches its end B from +x exactly where B lies above the centre
       * counterclockwise, below it clockwise (#arr_arc_centre_y_side); at
       * the height of the centre the vertex is R itself, which the vertex
       * already stands for. As the round joins of a buffer start and end
       * where straight sides tangent to them leave them */
      if (holds == 2 && ! a->e[p->edge].centred)
      {
        const ArrNode *na = &a->nodes[p->from], *nb = &a->nodes[p->to];
        bool neara = na->vertex && na->x >= x.lo && na->x <= x.hi;
        bool nearb = nb->vertex && nb->x >= x.lo && nb->x <= x.hi;
        if (neara != nearb)
        {
          int side = arr_arc_centre_y_side(&a->e[p->edge],
            neara ? na->y : nb->y);
          if (side != 2)
          {
            bool ccw = a->ccw[p->edge];
            holds = (side == 0) ? 0 : (neara ?
              (ccw ? side > 0 : side < 0) : (ccw ? side < 0 : side > 0));
          }
        }
      }
      if (holds == 2 && a->sup && a->e[p->edge].centred)
      {
        int h = arr_piece_holds_right_exact(a, p);
        if (h != 2)
          holds = h;
      }
      if (holds == 0)
        continue;
      /* A vertex on the circle of the arc, an end of its edge, stands left
       * of the point of the circle furthest along x where the piece holds
       * that point strictly within it, however close the intervals put
       * them */
      const Edge *ae = &a->e[p->edge];
      const ArrNode *xn = (ex.node == UINT32_MAX) ? NULL :
        &a->nodes[ex.node];
      if (holds == 1 && ! ex.on_arc && xn && ! ae->centred &&
          (arr_same(xn->x, xn->y, ae->x1, ae->y1) ||
           arr_same(xn->x, xn->y, ae->x2, ae->y2)))
      {
        ex.on_arc = true;
        ex.piece = a->halves[k].piece;
        ex.x = x;
        continue;
      }
      if (holds == 1 && ex.on_arc && ae->centred &&
          a->e[a->pieces[ex.piece].edge].centred &&
          x.hi >= ex.x.lo && x.lo <= ex.x.hi)
      {
        /* Two centred arcs holding the points of their circles furthest
         * along x, cx + r, compared exactly: either of two at one x has
         * nothing to its right, but at one point, the centres at one y,
         * the two circles touch, which no node records
         * (#arr_support_meets), and the larger lies outside the smaller,
         * so that its piece is the one the outside faces */
        const Edge *xe = &a->e[a->pieces[ex.piece].edge];
        int order = arr_centred_right_cmp(ae, xe);
        if (order > 0 ||
            (order == 0 && ae->cy == xe->cy && ae->radius > xe->radius))
        {
          ex.piece = a->halves[k].piece;
          ex.x = x;
        }
        continue;
      }
      if (holds == 1 && ! ex.on_arc && ae->centred && xn && xn->tower &&
          x.hi >= ex.x.lo && x.lo <= ex.x.hi)
      {
        /* The point of the circle furthest along x, cx + r, against a node
         * no double holds, compared exactly: the node keeps it where it
         * lies at least as far right */
        MeetPoint at = arr_meet_point(&xn->meet);
        at.p = xn->meet.p; at.q = xn->meet.q;
        int side = meet_point_x_side(&at, ae->cx, ae->radius);
        if (side != 2)
        {
          if (side < 0)
          {
            ex.on_arc = true;
            ex.piece = a->halves[k].piece;
            ex.x = x;
          }
          continue;
        }
      }
      if (holds == 1 && ! ex.on_arc && ae->centred && xn && xn->vertex &&
          x.hi >= ex.x.lo && x.lo <= ex.x.hi)
      {
        /* The same against a vertex, as #arr_vertex_at_origin compares a
         * vertex with the point furthest along -x: cx + r against x, the
         * sum read with its rounding error (two-sum), so the sign of
         * (cx + r) - x is exact; the vertex keeps the point where it lies
         * at least as far right, nothing lying right of either */
        double s = ae->cx + ae->radius;
        double bv = s - ae->cx;
        double err = (ae->cx - (s - bv)) + (ae->radius - bv);
        if (s > xn->x || (s == xn->x && err > 0.0))
        {
          ex.on_arc = true;
          ex.piece = a->halves[k].piece;
          ex.x = x;
        }
        continue;
      }
      if (holds == 2 || (x.hi >= ex.x.lo && x.lo <= ex.x.hi))
      {
        if (holds == 2 && x.hi < ex.x.lo)
          continue;
        /* A centred arc ending at the point of its circle furthest along x
         * holds no point right of that node, which the nodes compete with
         * where they are stretches' ends; elsewhere a node no double holds
         * does not compete, so the arc stays */
        if (a->sup && ae->centred && (arr_node_circle_right(a, p->from, ae) ||
            arr_node_circle_right(a, p->to, ae)))
          continue;
        /* Not told from the point found so far, which a point further
         * right than both still settles */
        tie = tied ? ival_hull(tie, x) : x;
        tied = true;
        continue;
      }
      if (x.lo > ex.x.hi)
      {
        ex.on_arc = true;
        ex.piece = a->halves[k].piece;
        ex.x = x;
      }
    }
  }
  /* A point not told from the rightmost one */
  if (tied && tie.hi >= ex.x.lo)
    unsure = true;
  *out = ex;
  return ! unsure;
}

static bool arr_piece_clear(const Arr *a, const ArrPiece *p, double lox,
  double hix, double loy, double hiy);
static bool arr_point_clear(const Arr *a, double x, double y, Ival bx,
  Ival by, const uint32_t *lpieces, const uint32_t *loff,
  const double (*lbox)[4], uint32_t nl);

/**
 * @brief Return a point of a component the ray of #arr_winding_at may start
 * from, false where none can be given
 * @details The rightmost node where it is a vertex, else any vertex of the
 * component. A component of stretches may have none, every end of its
 * stretches a crossing, and neither may one of centred arcs, whose nodes are
 * crossings and the starts of the arcs: such a node is then read at its
 * coordinates, rounded once, which lie in the face of the pieces labelled
 * before the node lies in where none of them reaches the box holding both
 * (#arr_point_clear), as #arr_ring_within reads a ring
 * @param[in] a Arrangement
 * @param[in] comp,cn The nodes of the component
 * @param[in] node Its rightmost node
 * @param[in] lpieces,loff,lbox,nl The pieces of the components labelled
 * before, component by component, and the box of each
 * @param[out] px,py The point
 */
static bool
arr_component_ray_point(const Arr *a, const uint32_t *comp, uint32_t cn,
  uint32_t node, const uint32_t *lpieces, const uint32_t *loff,
  const double (*lbox)[4], uint32_t nl, double *px, double *py)
{
  /* A component of centred arcs alone holds no vertex, its rightmost point
   * interior to an arc */
  uint32_t v = (node == UINT32_MAX) ? comp[0] : node;
  for (uint32_t c = 0; c < cn && ! a->nodes[v].vertex; c++)
    if (a->nodes[comp[c]].vertex)
      v = comp[c];
  const ArrNode *nd = &a->nodes[v];
  *px = nd->x;
  *py = nd->y;
  if (nd->vertex)
    return true;
  if (arr_point_clear(a, nd->x, nd->y, nd->bx, nd->by, lpieces, loff, lbox,
      nl))
    return true;
  /* A node no double holds, whose box a piece before reaches: the box
   * narrowed exactly (#meet_point_bounds) may keep clear of it, which is
   * what tells a loop a few units in the last place across from the
   * components around it */
  if (! nd->tower)
    return false;
  MeetPoint at = arr_meet_point(&nd->meet);
  at.p = nd->meet.p; at.q = nd->meet.q;
  double x, y, box[4];
  if (! meet_point_bounds(&at, &x, &y, box))
    return false;
  if (! arr_point_clear(a, x, y, ival_out(box[0], box[1]),
      ival_out(box[2], box[3]), lpieces, loff, lbox, nl))
    return false;
  *px = x;
  *py = y;
  return true;
}

/**
 * @brief Return true where a point rounded once lies in the face of the
 * components labelled before that the exact point lies in: no piece of
 * theirs reaches the box holding both (#arr_piece_clear)
 * @param[in] a Arrangement
 * @param[in] x,y The point as rounded
 * @param[in] bx,by Intervals holding the exact point
 * @param[in] lpieces,loff,lbox,nl The pieces of the components labelled
 * before, component by component, and the box of each
 */
static bool
arr_point_clear(const Arr *a, double x, double y, Ival bx, Ival by,
  const uint32_t *lpieces, const uint32_t *loff, const double (*lbox)[4],
  uint32_t nl)
{
  double lox = fmin(bx.lo, x), hix = fmax(bx.hi, x);
  double loy = fmin(by.lo, y), hiy = fmax(by.hi, y);
  for (uint32_t m = 0; m < nl; m++)
  {
    /* A component whose box keeps clear of the box is passed over whole */
    const double *cb = lbox[m];
    if (cb[0] > hix || cb[1] < lox || cb[2] > hiy || cb[3] < loy)
      continue;
    for (uint32_t k = loff[m]; k < loff[m + 1]; k++)
    {
      const ArrPiece *p = &a->pieces[lpieces[k]];
      const double *b = a->box[p->edge];
      if (b[0] > hix || b[1] < lox || b[2] > hiy || b[3] < loy)
        continue;
      if (! arr_piece_clear(a, p, lox, hix, loy, hiy))
        return false;
    }
  }
  return true;
}

/**
 * @brief Label every piece with the winding numbers on each side, component
 * by component, each from its rightmost point
 * @details A component enclosing another lies further right than it: the ray
 * from the inner one's rightmost point along the positive x axis crosses the
 * outer one, whose rightmost point lies beyond that crossing. The components
 * are therefore labelled from the one whose rightmost point lies furthest
 * right, which lies within none of the others, so that the winding number
 * outside each is read off components already labelled, every one enclosing
 * it among them (#arr_winding_at). Two components whose rightmost points the
 * intervals do not order, and whose boxes do not show that neither lies
 * within the other, leave the labels unanswered
 */
static ArrangeStatus
arr_label_all(Arr *a, uint32_t *outer_piece)
{
  uint32_t nn = a->nnodes;
  bool *queued = palloc0(sizeof(bool) * Max(nn, 1u));
  uint32_t *queue = palloc(sizeof(uint32_t) * Max(nn, 1u));
  /* Every component: its nodes, walked once, its rightmost point and its
   * box */
  uint32_t *cnodes = palloc(sizeof(uint32_t) * Max(nn, 1u));
  bool *incomp = palloc0(sizeof(bool) * Max(nn, 1u));
  uint32_t ccap = 16, ncomp = 0, total = 0;
  uint32_t *cnoff = palloc(sizeof(uint32_t) * (ccap + 1));
  ArrExtreme *cex = palloc(sizeof(ArrExtreme) * ccap);
  double (*cbox)[4] = palloc(sizeof(double[4]) * ccap);
  cnoff[0] = 0;
  ArrangeStatus st = ARR_ANSWERED;
  *outer_piece = UINT32_MAX;
  for (uint32_t seed = 0; seed < a->npieces && st == ARR_ANSWERED; seed++)
  {
    uint32_t v0 = a->pieces[seed].from;
    if (incomp[v0])
      continue;
    uint32_t c0 = total;
    cnodes[total++] = v0;
    incomp[v0] = true;
    for (uint32_t c = c0; c < total; c++)
    {
      uint32_t v = cnodes[c];
      for (uint32_t k = a->hoff[v]; k < a->hoff[v + 1]; k++)
      {
        const ArrPiece *p = &a->pieces[a->halves[k].piece];
        uint32_t o = a->halves[k].out ? p->to : p->from;
        if (! incomp[o])
        {
          incomp[o] = true;
          cnodes[total++] = o;
        }
      }
    }
    if (ncomp == ccap)
    {
      ccap *= 2;
      cnoff = repalloc(cnoff, sizeof(uint32_t) * (ccap + 1));
      cex = repalloc(cex, sizeof(ArrExtreme) * ccap);
      cbox = repalloc(cbox, sizeof(double[4]) * ccap);
    }
    if (! arr_component_extreme(a, &cnodes[c0], total - c0, &cex[ncomp]))
      st = ARR_DEPTH;
    double *cb = cbox[ncomp];
    cb[0] = cb[2] = INFINITY;
    cb[1] = cb[3] = -INFINITY;
    for (uint32_t c = c0; c < total; c++)
      for (uint32_t k = a->hoff[cnodes[c]]; k < a->hoff[cnodes[c] + 1]; k++)
      {
        const double *b = a->box[a->pieces[a->halves[k].piece].edge];
        cb[0] = fmin(cb[0], b[0]); cb[1] = fmax(cb[1], b[1]);
        cb[2] = fmin(cb[2], b[2]); cb[3] = fmax(cb[3], b[3]);
      }
    cnoff[++ncomp] = total;
  }
  pfree(incomp);

  /* The components from the one whose rightmost point lies furthest right,
   * the sweep's records holding minus the lower bound of each point */
  ArrSweep *order = palloc(sizeof(ArrSweep) * Max(ncomp, 1u));
  for (uint32_t c = 0; c < ncomp; c++)
  {
    order[c].xmin = - cex[c].x.lo;
    order[c].id = c;
  }
  if (st == ARR_ANSWERED)
    arr_sweep_sort(order, ncomp);
  for (uint32_t k = 0; k < ncomp && st == ARR_ANSWERED; k++)
    for (uint32_t m = k + 1; m < ncomp && st == ARR_ANSWERED; m++)
    {
      const Ival *xk = &cex[order[k].id].x, *xm = &cex[order[m].id].x;
      const ArrExtreme *ek = &cex[order[k].id], *em = &cex[order[m].id];
      bool arcs = ek->on_arc && em->on_arc &&
        a->e[a->pieces[ek->piece].edge].centred &&
        a->e[a->pieces[em->piece].edge].centred;
      /* Rightmost points told apart, or equal vertices: a component
       * enclosing another lies strictly further right */
      if (xm->hi < xk->lo ||
          (! arcs && xk->lo == xk->hi && xm->lo == xm->hi))
        continue;
      /* Rightmost points on centred arcs, compared exactly: one further
       * right than the other is not enclosed by it. At one x they are one
       * point where the centres are at one y, and there the two circles
       * touch, which no node records (#arr_support_meets): the larger holds
       * the smaller, so neither encloses the component of the larger, which
       * is labelled first */
      if (arcs)
      {
        const Edge *pk = &a->e[a->pieces[ek->piece].edge];
        const Edge *pm = &a->e[a->pieces[em->piece].edge];
        int c = arr_centred_right_cmp(pk, pm);
        if (c == 0 && pk->cy == pm->cy)
        {
          if (pk->radius == pm->radius)
            st = ARR_DEPTH;
          else if (pk->radius < pm->radius)
          {
            ArrSweep t = order[k]; order[k] = order[m]; order[m] = t;
            /* The component now at k is compared with all after it anew */
            m = k;
          }
          continue;
        }
        if (c >= 0)
          continue;
      }
      /* The point of a centred arc furthest along x against a node, compared
       * exactly (#arr_node_right_x): the one further right is labelled
       * first, and at one x two points apart enclose neither */
      bool karc = ek->on_arc && a->e[a->pieces[ek->piece].edge].centred;
      bool marc = em->on_arc && a->e[a->pieces[em->piece].edge].centred;
      if (karc != marc && ! (karc ? em->on_arc : ek->on_arc))
      {
        const ArrExtreme *earc = karc ? ek : em, *enode = karc ? em : ek;
        bool same;
        int s = arr_node_right_x(a, enode->node,
          &a->e[a->pieces[earc->piece].edge], &same);
        /* The sign of the x of the component at m less that of the one
         * at k */
        int d = (s == 2) ? 2 : (karc ? s : - s);
        if (d < 0 || (d == 0 && ! same))
          continue;
        if (d > 0)
        {
          ArrSweep t = order[k]; order[k] = order[m]; order[m] = t;
          m = k;
          continue;
        }
      }
      const double *bk = cbox[order[k].id], *bm = cbox[order[m].id];
      bool kinm = bk[0] >= bm[0] && bk[1] <= bm[1] && bk[2] >= bm[2] &&
        bk[3] <= bm[3];
      bool mink = bm[0] >= bk[0] && bm[1] <= bk[1] && bm[2] >= bk[2] &&
        bm[3] <= bk[3];
      if (kinm || mink)
        st = ARR_DEPTH;
    }

  /* The pieces of every component labelled, component by component, and
   * the box of each, which the rays of the later ones read */
  uint32_t *cpieces = palloc(sizeof(uint32_t) * Max(a->npieces, 1u));
  uint32_t *coff = palloc(sizeof(uint32_t) * (ncomp + 1));
  double (*lbox)[4] = palloc(sizeof(double[4]) * Max(ncomp, 1u));
  coff[0] = 0;
  uint32_t nlab = 0;
  for (uint32_t ord = 0; ord < ncomp && st == ARR_ANSWERED; ord++)
  {
    uint32_t cid = order[ord].id;
    const uint32_t *comp = &cnodes[cnoff[cid]];
    uint32_t cn = cnoff[cid + 1] - cnoff[cid];
    ArrExtreme ex = cex[cid];
    bool first = (ord == 0);
    uint32_t qn = 0;
    /* The winding number outside the component: zero for the first, the one
     * the components labelled before give its rightmost point otherwise */
    int outside = 0;
    if (! first)
    {
      /* The ray counts crossings from a point it holds exactly: a vertex
       * of the component, the rightmost where it is one, whose face of the
       * components labelled before is the one the whole component lies in,
       * as it meets none of them */
      double px, py;
      bool told = arr_component_ray_point(a, comp, cn, ex.node, cpieces,
          coff, (const double (*)[4]) lbox, nlab, &px, &py) &&
        arr_winding_at(a, cpieces, coff, (const double (*)[4]) lbox, nlab,
          ival(px), py, &outside);
      /* Any point of the component gives the same winding number, as it
       * meets none of the components before: where the ray from the first
       * is not told, as through a node of theirs, from each node in turn */
      for (uint32_t c = 0; c < cn && ! told; c++)
        told = arr_component_ray_point(a, &comp[c], 1, comp[c], cpieces,
            coff, (const double (*)[4]) lbox, nlab, &px, &py) &&
          arr_winding_at(a, cpieces, coff, (const double (*)[4]) lbox, nlab,
            ival(px), py, &outside);
      /* And from the top and the bottom of the circle of each centred arc
       * leaving a node of it, (cx, cy + r) and (cx, cy - r) rounded once,
       * on rays of their own heights */
      for (uint32_t c = 0; c < cn && ! told; c++)
        for (uint32_t h = a->hoff[comp[c]]; h < a->hoff[comp[c] + 1] &&
             ! told; h++)
        {
          const ArrPiece *cp = &a->pieces[a->halves[h].piece];
          const Edge *ce = &a->e[cp->edge];
          if (! a->halves[h].out || ! ce->centred)
            continue;
          for (int sgn = -1; sgn <= 1 && ! told; sgn += 2)
          {
            /* On a stretch of the circle, a point the piece holds: the
             * bottom and the top a quarter and three quarters of a turn
             * from its start (#arr_stretch_box) */
            double at = ((sgn > 0) == ce->ccw) ? 3.0 : 1.0;
            if (a->sup && ! (cp->pfrom.hi < at && at < cp->pto.lo))
              continue;
            Ival ty = sgn > 0 ? ival_add(ival(ce->cy), ival(ce->radius)) :
              ival_sub(ival(ce->cy), ival(ce->radius));
            px = ce->cx;
            py = ce->cy + sgn * ce->radius;
            told = arr_point_clear(a, px, py, ival(px), ty, cpieces, coff,
                (const double (*)[4]) lbox, nlab) &&
              arr_winding_at(a, cpieces, coff, (const double (*)[4]) lbox,
                nlab, ival(px), py, &outside);
          }
        }
      /* And from the points of those circles half way between, an eighth
       * of a turn either side of their top and bottom, which a circle
       * touching two lines at its top and bottom keeps clear of them:
       * (cx +- r sqrt(1/2), cy +- r sqrt(1/2)), held by intervals and
       * rounded once, t quarters of a turn from the start of the circle,
       * at the angle pi + t pi / 2 counterclockwise, pi - t pi / 2
       * clockwise */
      for (uint32_t c = 0; c < cn && ! told; c++)
        for (uint32_t h = a->hoff[comp[c]]; h < a->hoff[comp[c] + 1] &&
             ! told; h++)
        {
          const ArrPiece *cp = &a->pieces[a->halves[h].piece];
          const Edge *ce = &a->e[cp->edge];
          if (! a->halves[h].out || ! ce->centred)
            continue;
          Ival hr = ival_mul(ival(ce->radius), ival_sqrt(ival(0.5)));
          for (int q = 0; q < 4 && ! told; q++)
          {
            double at = q + 0.5;
            if (a->sup && ! (cp->pfrom.hi < at && at < cp->pto.lo))
              continue;
            /* The signs of the cosine and the sine at that angle */
            static const int ccwx[4] = {-1, 1, 1, -1};
            static const int ccwy[4] = {-1, -1, 1, 1};
            int sx = ccwx[q], sy = ce->ccw ? ccwy[q] : - ccwy[q];
            Ival tx = ival_add(ival(ce->cx), sx > 0 ? hr : ival_neg(hr));
            Ival ty = ival_add(ival(ce->cy), sy > 0 ? hr : ival_neg(hr));
            px = 0.5 * (tx.lo + tx.hi);
            py = 0.5 * (ty.lo + ty.hi);
            told = arr_point_clear(a, px, py, tx, ty, cpieces, coff,
                (const double (*)[4]) lbox, nlab) &&
              arr_winding_at(a, cpieces, coff, (const double (*)[4]) lbox,
                nlab, ival(px), py, &outside);
          }
        }
      if (! told)
      {
        st = ARR_DEPTH;
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
      arr_label(a, p, ccw ? outside : outside - arr_weight(a, p->edge));
      if (first)
        *outer_piece = ex.piece;
      const uint32_t ends[2] = {p->from, p->to};
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
        break;
      }
      ArrPiece *p = &a->pieces[h[before].piece];
      /* The sector after half k is its left where it leaves, its right
       * where it arrives */
      arr_label(a, p, h[before].out ? outside - arr_weight(a, p->edge) :
        outside);
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
    if (st != ARR_ANSWERED)
      break;
    /* The component's pieces, each leaving one of its nodes once, and its
     * box, read by the rays of the components after it */
    uint32_t np = coff[nlab];
    for (uint32_t c = 0; c < cn; c++)
    {
      uint32_t v = comp[c];
      for (uint32_t h = a->hoff[v]; h < a->hoff[v + 1]; h++)
        if (a->halves[h].out)
          cpieces[np++] = a->halves[h].piece;
    }
    memcpy(lbox[nlab], cbox[cid], sizeof(double[4]));
    coff[++nlab] = np;
  }
  pfree(queued); pfree(queue); pfree(cnodes); pfree(cnoff); pfree(cex);
  pfree(cbox); pfree(order); pfree(cpieces); pfree(coff); pfree(lbox);
  return st;
}

/* The piece of an edge between two nodes, in the direction it is walked:
 * its ends and, for an arc, a middle (#ArrangePiece) */
static void
arr_piece_write(const Arr *a, const ArrPiece *p, ArrangePiece *out)
{
  const Edge *e = &a->e[p->edge];
  const ArrNode *n1 = &a->nodes[p->flip ? p->to : p->from];
  const ArrNode *n2 = &a->nodes[p->flip ? p->from : p->to];
  out->x1 = n1->x; out->y1 = n1->y;
  out->x2 = n2->x; out->y2 = n2->y;
  out->arc = arr_is_arc(e);
  out->mx = out->my = 0.0;
  if (! out->arc)
    return;
  /* The whole circle of a centred arc no node cuts: from the point furthest
   * along -x through the one opposite */
  if (e->centred && p->from == p->to)
  {
    out->mx = e->cx + e->radius; out->my = e->cy;
    return;
  }
  /* The edge's own middle where the piece is the whole of an arc of three
   * points, else the point of its circle on the bisector of its ends, on the
   * side the piece sweeps, written once */
  if (! e->diameter && ! e->centred && p->from == a->start[p->edge] &&
      p->to == a->end[p->edge])
  {
    out->mx = e->mx; out->my = e->my;
    return;
  }
  bool ccw = p->flip ? ! a->ccw[p->edge] : a->ccw[p->edge];
  if (! e->centred)
  {
    double ax = out->x1 - e->cx, ay = out->y1 - e->cy;
    double bx = out->x2 - e->cx, by = out->y2 - e->cy;
    double sx = ax + bx, sy = ay + by;
    double len = hypot(sx, sy);
    double dx, dy;
    if (len > 0.0)
    {
      /* Within half a turn the middle lies on the side of the sum */
      double z = ax * by - ay * bx;
      double k = ((ccw && z > 0.0) || (! ccw && z < 0.0)) ? 1.0 : -1.0;
      dx = k * sx / len; dy = k * sy / len;
    }
    else
    {
      /* A half circle: a quarter turn on from the start */
      double alen = hypot(ax, ay);
      dx = (ccw ? - ay : ay) / alen;
      dy = (ccw ? ax : - ax) / alen;
    }
    out->mx = e->cx + e->radius * dx;
    out->my = e->cy + e->radius * dy;
    return;
  }
  /* The middle of a centred arc lies on the perpendicular to its chord
   * through the centre, right of the chord where the arc turns
   * counterclockwise, for any sweep: read off the chord wherever it is long.
   * A short chord holds an arc of next to no sweep, written as the chord
   * below, or of next to a whole turn, whose middle lies opposite the sum of
   * the radii to its ends; the positions of the piece's ends along the arc
   * tell which, its rounded ends where they do not */
  double cx = out->x2 - out->x1, cy = out->y2 - out->y1;
  double clen = hypot(cx, cy);
  double dx, dy;
  if (clen >= 0.25 * e->radius)
  {
    dx = (ccw ? cy : - cy) / clen;
    dy = (ccw ? - cx : cx) / clen;
  }
  else
  {
    double ax = out->x1 - e->cx, ay = out->y1 - e->cy;
    double bx = out->x2 - e->cx, by = out->y2 - e->cy;
    double sx = ax + bx, sy = ay + by;
    double len = hypot(sx, sy);
    Ival sweep = ival_sub(isinf(p->pto.hi) ? ival(4.0) : p->pto, p->pfrom);
    int whole = (sweep.lo > 2.0) ? 1 : ((sweep.hi < 2.0) ? -1 : 0);
    if (whole == 0)
    {
      double z = ax * by - ay * bx;
      whole = ((ccw && z < 0.0) || (! ccw && z > 0.0)) ? 1 : -1;
    }
    double k = (whole > 0) ? -1.0 : 1.0;
    dx = k * sx / len; dy = k * sy / len;
  }
  out->mx = e->cx + e->radius * dx;
  out->my = e->cy + e->radius * dy;
  /* An arc whose middle the rounding of its written points puts on their
   * chord is written as the chord: three such points fix no circle, and the
   * region moves by less than their rounding */
  double sag = (clen > 0.0) ?
    fabs(cx * (out->my - out->y1) - cy * (out->mx - out->x1)) / clen : 0.0;
  double unit = DBL_EPSILON * (fabs(out->x1) + fabs(out->y1) +
    fabs(out->x2) + fabs(out->y2));
  if (clen > 0.0 && sag <= 8.0 * unit)
  {
    out->arc = false;
    out->mx = out->my = 0.0;
  }
  return;
}

/* Return true where two segments written one after the other retrace each
 * other. The boundary of the region never runs back along itself, so such a
 * pair is a stretch narrower than the doubles resolve, which writing its
 * nodes, each rounded once (#arr_piece_write), folds onto itself: it bounds
 * no area the coordinates can state, as a piece whose ends are written at
 * one point bounds none */
static inline bool
arr_pieces_retrace(const ArrangePiece *p, const ArrangePiece *q)
{
  return ! p->arc && ! q->arc && p->x1 == q->x2 && p->y1 == q->y2 &&
    p->x2 == q->x1 && p->y2 == q->y1;
}

/* The node a piece of the boundary is walked from, and the one it is walked
 * to */
static inline uint32_t
arr_walk_from(const ArrPiece *p)
{
  return p->flip ? p->to : p->from;
}

static inline uint32_t
arr_walk_to(const ArrPiece *p)
{
  return p->flip ? p->from : p->to;
}

/**
 * @brief Return a ring written from the pieces of a closed walk, or NULL
 * where it bounds nothing the coordinates can state
 * @details A piece whose ends are written at one point is left out, but the
 * whole circle of a centred arc, and two
 * segments that retrace each other are taken back together
 * (#arr_pieces_retrace), across the closing piece as well
 */
static MeosArray *
arr_ring_write(const Arr *a, const uint32_t *pieces, uint32_t m)
{
  MeosArray *ring = meos_array_create(sizeof(ArrangePiece));
  for (uint32_t t = 0; t < m; t++)
  {
    if (ring->count >= ring->capacity)
      edge_array_reserve(ring, 1);
    ArrangePiece *pe = &((ArrangePiece *) ring->elems)[ring->count];
    const ArrPiece *p = &a->pieces[pieces[t]];
    arr_piece_write(a, p, pe);
    /* A whole circle starts and ends at its one node */
    if (pe->x1 == pe->x2 && pe->y1 == pe->y2 && ! (pe->arc &&
        p->from == p->to))
      continue;
    if (ring->count > 0 && arr_pieces_retrace(pe - 1, pe))
      ring->count--;
    else
      ring->count++;
  }
  ArrangePiece *pr = (ArrangePiece *) ring->elems;
  while (ring->count >= 2 &&
         arr_pieces_retrace(&pr[ring->count - 1], &pr[0]))
  {
    memmove(&pr[0], &pr[1], sizeof(ArrangePiece) * (ring->count - 2));
    ring->count -= 2;
  }
  if (ring->count == 0)
  {
    meos_array_destroy(ring);
    return NULL;
  }
  return ring;
}

/**
 * @brief Split a closed walk at the nodes it passes more than once into the
 * simple loops it is made of, the loop through its first piece first
 * @details The region's boundary is walked as the boundary of a face, which
 * passes a node twice where the region touches itself there: a hole
 * touching its shell at a point, two parts of the region meeting at one.
 * Each loop the walk closes at a node already on it is cut off where it
 * closes, so that every loop passes each of its nodes once, and the loops
 * are then rings that meet at most at nodes, which their containment
 * classifies (#arr_surfaces)
 * @param[in] a Arrangement
 * @param[in,out] walk Pieces of the walk, written back loop by loop
 * @param[in] m Number of pieces
 * @param[in,out] onstack For every node, -1, as it is left
 * @param[out] scratch Room for twice @p m pieces
 * @param[out] ends End of each loop in @p walk, room for @p m
 * @return Number of loops
 */
static uint32_t
arr_walk_loops(const Arr *a, uint32_t *walk, uint32_t m, int32_t *onstack,
  uint32_t *scratch, uint32_t *ends)
{
  uint32_t *stack = scratch, *loops = scratch + m;
  uint32_t top = 0, nl = 0, lp = 0;
  uint32_t v0 = arr_walk_from(&a->pieces[walk[0]]);
  onstack[v0] = 0;
  for (uint32_t t = 0; t < m; t++)
  {
    stack[top++] = walk[t];
    uint32_t v = arr_walk_to(&a->pieces[walk[t]]);
    if (onstack[v] < 0)
    {
      onstack[v] = (int32_t) top;
      continue;
    }
    /* The pieces from the one leaving v on are a loop through v, which
     * stays on the stack as the node the walk goes on from */
    uint32_t d = (uint32_t) onstack[v];
    for (uint32_t q = d; q < top; q++)
    {
      loops[lp++] = stack[q];
      if (q > d)
        onstack[arr_walk_from(&a->pieces[stack[q]])] = -1;
    }
    ends[nl++] = lp;
    top = d;
  }
  onstack[v0] = -1;
  /* The walk closes at its first node, so the last loop cut off holds its
   * first piece: it is written first, the others after it in the order they
   * were cut off, their ends read into the stack's room, free by now */
  uint32_t *newends = stack;
  uint32_t last = (nl > 1) ? ends[nl - 2] : 0, w = 0, prev = 0;
  for (uint32_t q = last; q < lp; q++)
    walk[w++] = loops[q];
  newends[0] = w;
  for (uint32_t k = 0; k + 1 < nl; k++)
  {
    for (uint32_t q = prev; q < ends[k]; q++)
      walk[w++] = loops[q];
    prev = ends[k];
    newends[k + 1] = w;
  }
  memcpy(ends, newends, sizeof(uint32_t) * nl);
  return nl;
}

/**
 * @brief Link the pieces that bound the region into rings
 * @details A piece bounds the region where the winding number is at least
 * one on one side of it and not on the other, and is walked with the region
 * on its left. At a node the walk takes the first piece leaving it clockwise
 * from the one it arrives along, which keeps the region on its left and
 * closes the smallest ring. Where the rings are classified into surfaces
 * (@p ring_pieces given), a walk passing a node twice is split there into
 * the simple loops it is made of (#arr_walk_loops)
 */
static ArrangeStatus
arr_rings(Arr *a, int sgn, uint32_t outer_piece, MeosArray ***rings,
  uint32_t *nrings, uint32_t **ring_pieces, uint32_t **ring_off)
{
  for (uint32_t k = 0; k < a->npieces; k++)
  {
    ArrPiece *p = &a->pieces[k];
    bool inl = sgn * p->left >= 1, inr = sgn * p->right >= 1;
    p->result = inl != inr;
    p->flip = inr;
  }
  /* The outermost piece bounds the region, unless it is linework, which
   * bounds nothing (#arr_weight) */
  if (outer_piece == UINT32_MAX || (! a->pieces[outer_piece].result &&
      arr_weight(a, a->pieces[outer_piece].edge) != 0))
    return ARR_SHELLS;
  uint32_t cap = 4, n = 0;
  MeosArray **out = palloc(sizeof(MeosArray *) * cap);
  /* The pieces of every ring, in the order walked */
  uint32_t *rp = palloc(sizeof(uint32_t) * Max(a->npieces, 1u));
  uint32_t *roff = palloc(sizeof(uint32_t) * (cap + 1));
  uint32_t np = 0;
  roff[0] = 0;
  /* Where walks are split into loops, the stack of nodes the split reads */
  bool split = (ring_pieces != NULL);
  int32_t *onstack = NULL;
  uint32_t *scratch = NULL, *ends = NULL;
  if (split)
  {
    onstack = palloc(sizeof(int32_t) * Max(a->nnodes, 1u));
    for (uint32_t v = 0; v < a->nnodes; v++)
      onstack[v] = -1;
    scratch = palloc(sizeof(uint32_t) * 2 * Max(a->npieces, 1u));
    ends = palloc(sizeof(uint32_t) * Max(a->npieces, 1u));
  }
  /* The ring through the outermost piece first: it is the shell */
  uint32_t order0 = outer_piece;
  ArrangeStatus st = ARR_ANSWERED;
  for (uint32_t pass = 0; pass <= a->npieces && st == ARR_ANSWERED; pass++)
  {
    uint32_t k = (pass == 0) ? order0 : pass - 1;
    if (k >= a->npieces || ! a->pieces[k].result || a->pieces[k].used)
      continue;
    uint32_t cur = k, np0 = np;
    for (;;)
    {
      ArrPiece *p = &a->pieces[cur];
      p->used = true;
      rp[np++] = cur;
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
      if (next == UINT32_MAX || (next != k && a->pieces[next].used))
      {
        st = ARR_DEPTH;
        break;
      }
      if (next == k)
        break;
      cur = next;
    }
    if (st != ARR_ANSWERED)
      break;
    /* The walk, split into its loops where they are classified */
    uint32_t m = np - np0, nl = 1;
    const uint32_t whole[1] = {m};
    const uint32_t *lend = whole;
    if (split)
    {
      nl = arr_walk_loops(a, &rp[np0], m, onstack, scratch, ends);
      lend = ends;
    }
    /* Each loop written as a ring; one bounding nothing the coordinates can
     * state is left out with its pieces */
    uint32_t from = np0, keep = np0;
    for (uint32_t l = 0; l < nl; l++)
    {
      uint32_t to = np0 + lend[l];
      MeosArray *ring = arr_ring_write(a, &rp[from], to - from);
      if (ring)
      {
        memmove(&rp[keep], &rp[from], sizeof(uint32_t) * (to - from));
        keep += to - from;
        if (n == cap)
        {
          cap *= 2;
          out = repalloc(out, sizeof(MeosArray *) * cap);
          roff = repalloc(roff, sizeof(uint32_t) * (cap + 1));
        }
        out[n++] = ring;
        roff[n] = keep;
      }
      from = to;
    }
    np = keep;
  }
  if (split)
  {
    pfree(onstack); pfree(scratch); pfree(ends);
  }
  if (st != ARR_ANSWERED)
  {
    arrange_rings_free(out, n);
    pfree(rp); pfree(roff);
    return st;
  }
  *rings = out;
  *nrings = n;
  if (ring_pieces)
  {
    *ring_pieces = rp;
    *ring_off = roff;
  }
  else
  {
    pfree(rp); pfree(roff);
  }
  return ARR_ANSWERED;
}

/*
 * The box of a ring's pieces, an arc's taken as its circle's: a filter that
 * only admits the rings that may hold another (rule 5 of the exact lane)
 */
static void
arr_ring_box(const Arr *a, const uint32_t *rp, uint32_t from, uint32_t to,
  double box[4])
{
  box[0] = box[2] = INFINITY;
  box[1] = box[3] = -INFINITY;
  for (uint32_t k = from; k < to; k++)
  {
    const ArrPiece *p = &a->pieces[rp[k]];
    const Edge *e = &a->e[p->edge];
    if (arr_is_arc(e))
    {
      Ival r = ival_sqrt(a->cr2[p->edge]);
      box[0] = fmin(box[0], ival_sub(a->ccx[p->edge], r).lo);
      box[1] = fmax(box[1], ival_add(a->ccx[p->edge], r).hi);
      box[2] = fmin(box[2], ival_sub(a->ccy[p->edge], r).lo);
      box[3] = fmax(box[3], ival_add(a->ccy[p->edge], r).hi);
    }
    else
    {
      /* The piece's ends, the intervals holding a crossing: the edge of a
       * stretch is the whole of its segment */
      const ArrNode *ends[2] = {&a->nodes[p->from], &a->nodes[p->to]};
      for (int m = 0; m < 2; m++)
      {
        box[0] = fmin(box[0], ends[m]->bx.lo);
        box[1] = fmax(box[1], ends[m]->bx.hi);
        box[2] = fmin(box[2], ends[m]->by.lo);
        box[3] = fmax(box[3], ends[m]->by.hi);
      }
    }
  }
  return;
}

/*
 * Whether a piece keeps clear of a box: true where every corner of the box
 * stands strictly on one side of the piece's line or circle, the side of a
 * corner, a double, decided exactly by #cross_product_sign or by intervals;
 * or where the positions of the corners along the piece's edge all lie
 * before or all after the piece. False where neither can be told
 */
static bool
arr_piece_clear(const Arr *a, const ArrPiece *p, double lox, double hix,
  double loy, double hiy)
{
  const Edge *e = &a->e[p->edge];
  const double cx[4] = {lox, hix, lox, hix}, cy[4] = {loy, loy, hiy, hiy};
  int side[4];
  for (int k = 0; k < 4; k++)
  {
    if (arr_is_arc(e))
    {
      Ival dx = ival_sub(ival(cx[k]), a->ccx[p->edge]);
      Ival dy = ival_sub(ival(cy[k]), a->ccy[p->edge]);
      side[k] = ival_sign(ival_sub(ival_dot(dx, dy, dx, dy),
        a->cr2[p->edge]));
    }
    else if (e->from)
      side[k] = towerseg_side(e, cx[k], cy[k]);
    else
      side[k] = cross_product_sign(e->x1, e->y1, e->x2, e->y2, e->x1,
        e->y1, cx[k], cy[k]);
  }
  if ((side[0] == 1 || side[0] == -1) && side[1] == side[0] &&
      side[2] == side[0] && side[3] == side[0])
    return true;
  /* The positions along a segment between tangent points are read off its
   * rounded ends, which do not bound it; the piece, a segment between two
   * points the boxes of its end nodes hold, lies within the box of both */
  if (e->from)
  {
    const ArrNode *n0 = &a->nodes[p->from], *n1 = &a->nodes[p->to];
    return fmin(n0->bx.lo, n1->bx.lo) > hix ||
      fmax(n0->bx.hi, n1->bx.hi) < lox ||
      fmin(n0->by.lo, n1->by.lo) > hiy || fmax(n0->by.hi, n1->by.hi) < loy;
  }
  /* The positions, read only where the sides do not tell */
  Ival pos[4];
  for (int k = 0; k < 4; k++)
    pos[k] = arr_is_arc(e) ?
      arr_arc_position(e, a->ccx[p->edge], a->ccy[p->edge], ival(cx[k]),
        ival(cy[k])) :
      arr_segment_position(e, ival(cx[k]), ival(cy[k]));
  Ival h = ival_hull(ival_hull(pos[0], pos[1]), ival_hull(pos[2], pos[3]));
  return h.hi < p->pfrom.lo || h.lo > p->pto.hi;
}

/*
 * Whether ring x lies within ring y: 1, 0, or 2 where it cannot be told.
 * The rings bound one region, so they never cross, and x lies within y
 * where any of its points not on y does: a vertex node of x that y does not
 * pass through, whose coordinates are exact, and a vertex lying on a piece
 * of y would be a node of it. The ray from it crosses y an odd number of
 * times (#arr_piece_ray)
 */
static int
arr_ring_within(const Arr *a, const uint32_t *rp, const uint32_t *roff,
  uint32_t x, uint32_t y, bool *mark)
{
  for (uint32_t k = roff[y]; k < roff[y + 1]; k++)
  {
    const ArrPiece *p = &a->pieces[rp[k]];
    mark[p->from] = mark[p->to] = true;
  }
  /* Every such node in turn, until the ray from one is told. A crossing is
   * read at its coordinates, rounded once: they lie on the side of y the
   * crossing does where no piece of y reaches the box holding both
   * (#arr_piece_clear) */
  int result = 2;
  for (uint32_t k = roff[x]; k < roff[x + 1] && result == 2; k++)
  {
    uint32_t v = a->pieces[rp[k]].from;
    if (mark[v])
      continue;
    const ArrNode *nd = &a->nodes[v];
    if (! nd->vertex)
    {
      double lox = fmin(nd->bx.lo, nd->x), hix = fmax(nd->bx.hi, nd->x);
      double loy = fmin(nd->by.lo, nd->y), hiy = fmax(nd->by.hi, nd->y);
      bool clear = true;
      for (uint32_t m = roff[y]; m < roff[y + 1] && clear; m++)
        clear = arr_piece_clear(a, &a->pieces[rp[m]], lox, hix, loy, hiy);
      if (! clear)
        continue;
    }
    int w = 0, c = 0;
    bool told = true;
    double vx = a->nodes[v].x, vy = a->nodes[v].y;
    for (uint32_t m = roff[y]; m < roff[y + 1] && told; m++)
    {
      /* The box test of #arr_piece_ray, read here before the call */
      const double *b = a->box[a->pieces[rp[m]].edge];
      if (b[3] < vy || b[2] > vy || b[1] < vx)
        continue;
      told = arr_piece_ray(a, &a->pieces[rp[m]], ival(vx), vy, &w, &c);
    }
    if (told)
      result = c % 2;
  }
  for (uint32_t k = roff[y]; k < roff[y + 1]; k++)
  {
    const ArrPiece *p = &a->pieces[rp[k]];
    mark[p->from] = mark[p->to] = false;
  }
  return result;
}

/*
 * The surfaces the rings bound: a ring lying within an odd number of the
 * others is a hole of the innermost of them, which lies within one ring
 * fewer; every other ring is a shell
 */
static ArrangeStatus
arr_surfaces(const Arr *a, uint32_t nrings, const uint32_t *rp,
  const uint32_t *roff, int32_t *owner)
{
  double *box = palloc(sizeof(double) * 4 * Max(nrings, 1u));
  for (uint32_t i = 0; i < nrings; i++)
    arr_ring_box(a, rp, roff[i], roff[i + 1], &box[4 * i]);
  bool *mark = palloc0(sizeof(bool) * Max(a->nnodes, 1u));
  uint32_t *depth = palloc0(sizeof(uint32_t) * Max(nrings, 1u));
  /* The rings each lies within, as a list per ring */
  uint32_t cap = Max(nrings, 4u), nin = 0;
  uint32_t *in_x = palloc(sizeof(uint32_t) * cap);
  uint32_t *in_y = palloc(sizeof(uint32_t) * cap);
  ArrangeStatus st = ARR_ANSWERED;
  for (uint32_t x = 0; x < nrings && st == ARR_ANSWERED; x++)
    for (uint32_t y = 0; y < nrings && st == ARR_ANSWERED; y++)
    {
      if (x == y)
        continue;
      const double *bx = &box[4 * x], *by = &box[4 * y];
      if (bx[0] < by[0] || bx[1] > by[1] || bx[2] < by[2] || bx[3] > by[3])
        continue;
      int r = arr_ring_within(a, rp, roff, x, y, mark);
      if (r == 2)
        st = ARR_SHELLS;
      else if (r == 1)
      {
        depth[x]++;
        if (nin == cap)
        {
          cap *= 2;
          in_x = repalloc(in_x, sizeof(uint32_t) * cap);
          in_y = repalloc(in_y, sizeof(uint32_t) * cap);
        }
        in_x[nin] = x; in_y[nin] = y; nin++;
      }
    }
  for (uint32_t i = 0; i < nrings; i++)
    owner[i] = -1;
  for (uint32_t k = 0; k < nin && st == ARR_ANSWERED; k++)
  {
    uint32_t x = in_x[k], y = in_y[k];
    if (depth[x] % 2 == 1 && depth[y] + 1 == depth[x])
      owner[x] = (int32_t) y;
  }
  for (uint32_t i = 0; i < nrings && st == ARR_ANSWERED; i++)
    if (depth[i] % 2 == 1 && owner[i] < 0)
      st = ARR_SHELLS;
  pfree(box); pfree(mark); pfree(depth); pfree(in_x); pfree(in_y);
  return st;
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

/* The root of an edge's class of segments running along each other, halving
 * the path walked */
static uint32_t
arr_class_root(uint32_t *parent, uint32_t i)
{
  while (parent[i] != i)
  {
    parent[i] = parent[parent[i]];
    i = parent[i];
  }
  return i;
}

/* Points of one line in their order along it: by x, then by y, which on a
 * line that is not vertical is the order of x alone, and on a vertical one
 * that of y */
static inline int
arr_line_cmp(const POINT2D *p, const POINT2D *q)
{
  if (p->x != q->x)
    return (p->x < q->x) ? -1 : 1;
  if (p->y != q->y)
    return (p->y < q->y) ? -1 : 1;
  return 0;
}

/* Sort the points of a line with their order written into the sort, as
 * #arr_sweep_sort does */
#define ST_SORT arr_line_sort
#define ST_ELEMENT_TYPE POINT2D
#define ST_COMPARE(a, b) arr_line_cmp(a, b)
#define ST_SCOPE static
#define ST_DECLARE
#define ST_DEFINE
#include "port/sort_template.h"

/* The place of a point among the distinct sorted points of a line it is one
 * of */
static uint32_t
arr_line_find(const POINT2D *pts, uint32_t npts, const POINT2D *p)
{
  uint32_t lo = 0, hi = npts;
  while (hi - lo > 1)
  {
    uint32_t mid = lo + (hi - lo) / 2;
    if (arr_line_cmp(&pts[mid], p) <= 0)
      lo = mid;
    else
      hi = mid;
  }
  return lo;
}

/* Where the edges of an arrangement went when the segments running along
 * each other were merged (#arr_merge_overlaps): an edge in a class of one is
 * the edge at newof, and the edges of a larger class, whose root is root,
 * are the scount stretches from sfirst on of that root */
typedef struct
{
  uint32_t *root;
  uint32_t *newof;
  uint32_t *sfirst;
  uint32_t *scount;
} ArrMerge;

/**
 * @brief Replace the segments running along each other by the stretches
 * they share, each weighted by the segments running along it
 * @details Two segments run along each other where they lie on one line and
 * share a stretch of it (#linesegm_intersect, exact), and the segments
 * related that way, directly or through others, lie on one line. Every end
 * of the stretch two of them share is an end of one of them, an input
 * vertex, so the line is cut at the ends of the segments of the class alone,
 * ordered along it by their coordinates (#arr_line_cmp) and decided exactly.
 * Each stretch between two consecutive ends is written once, in the order
 * of the line, weighted by the segments covering it, each counted by its
 * sense: plus one where it runs in the order of the line, minus one where it
 * runs against it. That is the rise of the winding number across the stretch
 * all of them together give, so the stretch stands for them; a stretch
 * whose weight is zero bounds nothing and is left out. The pairs asked are
 * those whose boxes meet (#arr_box_pairs).
 * @param[in] edges The edges of the curves
 * @param[in] pairs,npairs The pairs of edges whose boxes meet
 * @param[out] merged The edges, every class written as its stretches
 * @param[out] weights The weight of each of them
 * @param[out] mg Where every edge went, released by the caller
 * @return false where no two segments run along each other, which leaves
 * the edges as they are
 */
static bool
arr_merge_overlaps(const MeosArray *edges, const ArrPair *pairs,
  uint32_t npairs, MeosArray **merged, int32_t **weights, ArrMerge *mg)
{
  uint32_t n = edges->count;
  const Edge *e = (const Edge *) edges->elems;
  uint32_t *parent = NULL;
  for (uint32_t k = 0; k < npairs; k++)
  {
    uint32_t i = pairs[k].i, j = pairs[k].j;
    if (arr_is_arc(&e[i]) || arr_is_arc(&e[j]) || e[i].from || e[j].from)
      continue;
    /* Segments whose directions are not parallel do not run along each
     * other, which the bound of the cross product of their directions shows
     * for most pairs without the meeting test (#arr_cross_ival) */
    Ival par = arr_cross_ival(e[i].x1, e[i].y1, e[i].x2, e[i].y2, e[j].x1,
      e[j].y1, e[j].x2, e[j].y2);
    if (par.lo > 0.0 || par.hi < 0.0)
      continue;
    IntersectResult r = linesegm_intersect(e[i].x1, e[i].y1, e[i].x2,
      e[i].y2, e[j].x1, e[j].y1, e[j].x2, e[j].y2);
    if (r.type != INTERSECT_OVERLAP)
      continue;
    if (! parent)
    {
      parent = palloc(sizeof(uint32_t) * n);
      for (uint32_t m = 0; m < n; m++)
        parent[m] = m;
    }
    uint32_t ri = arr_class_root(parent, i), rj = arr_class_root(parent, j);
    if (ri != rj)
      parent[Max(ri, rj)] = Min(ri, rj);
  }
  if (! parent)
    return false;

  /* The members of each class, class by class, in the order of the edges */
  uint32_t *root = palloc(sizeof(uint32_t) * n);
  uint32_t *moff = palloc0(sizeof(uint32_t) * (n + 1));
  for (uint32_t i = 0; i < n; i++)
  {
    root[i] = arr_class_root(parent, i);
    moff[root[i] + 1]++;
  }
  for (uint32_t i = 0; i < n; i++)
    moff[i + 1] += moff[i];
  uint32_t *members = palloc(sizeof(uint32_t) * n);
  uint32_t *fill = palloc(sizeof(uint32_t) * n);
  memcpy(fill, moff, sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; i++)
    members[fill[root[i]]++] = i;
  pfree(fill);

  /* A class of m segments writes at most 2m - 1 stretches */
  MeosArray *out = meos_array_create(sizeof(Edge));
  int32_t *w = palloc(sizeof(int32_t) * 2 * n);
  POINT2D *pts = palloc(sizeof(POINT2D) * 2 * n);
  int32_t *rise = palloc(sizeof(int32_t) * 2 * n);
  mg->root = root;
  mg->newof = palloc(sizeof(uint32_t) * n);
  mg->sfirst = palloc0(sizeof(uint32_t) * n);
  mg->scount = palloc0(sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; i++)
  {
    uint32_t r = root[i], m = moff[r + 1] - moff[r];
    if (m == 1)
    {
      mg->newof[i] = out->count;
      w[out->count] = 1;
      meos_array_add(out, (void *) &e[i]);
      continue;
    }
    mg->newof[i] = UINT32_MAX;
    /* A class is written where its first member stands */
    if (r != i)
      continue;
    mg->sfirst[r] = out->count;
    const uint32_t *mem = &members[moff[r]];
    uint32_t npts = 0;
    for (uint32_t k = 0; k < m; k++)
    {
      pts[npts].x = e[mem[k]].x1; pts[npts++].y = e[mem[k]].y1;
      pts[npts].x = e[mem[k]].x2; pts[npts++].y = e[mem[k]].y2;
    }
    arr_line_sort(pts, npts);
    uint32_t nd = 1;
    for (uint32_t k = 1; k < npts; k++)
      if (arr_line_cmp(&pts[k], &pts[nd - 1]) != 0)
        pts[nd++] = pts[k];
    memset(rise, 0, sizeof(int32_t) * nd);
    for (uint32_t k = 0; k < m; k++)
    {
      const Edge *s = &e[mem[k]];
      POINT2D p1 = {s->x1, s->y1}, p2 = {s->x2, s->y2};
      bool forward = arr_line_cmp(&p1, &p2) < 0;
      uint32_t lo = arr_line_find(pts, nd, forward ? &p1 : &p2);
      uint32_t hi = arr_line_find(pts, nd, forward ? &p2 : &p1);
      rise[lo] += forward ? 1 : -1;
      rise[hi] -= forward ? 1 : -1;
    }
    int32_t weight = 0;
    for (uint32_t k = 0; k + 1 < nd; k++)
    {
      weight += rise[k];
      if (weight == 0)
        continue;
      w[out->count] = weight;
      edges_add_segment(out, &pts[k], &pts[k + 1]);
    }
    mg->scount[r] = out->count - mg->sfirst[r];
  }
  pfree(pts); pfree(rise); pfree(members); pfree(moff);
  pfree(parent);
  *merged = out;
  *weights = w;
  return true;
}

/* Append a pair to an array of pairs, growing it as needed */
static inline void
arr_pair_add(ArrPair **pairs, uint32_t *count, uint32_t *cap, uint32_t i,
  uint32_t j)
{
  if (*count == *cap)
  {
    *cap *= 2;
    *pairs = repalloc(*pairs, sizeof(ArrPair) * (*cap));
  }
  (*pairs)[*count].i = i;
  (*pairs)[(*count)++].j = j;
}

/* Whether the boxes of two edges meet, as #arr_box_pairs reads them */
static inline bool
arr_boxes_meet(const double (*box)[4], uint32_t s, uint32_t t)
{
  return ! (box[t][0] > box[s][1] || box[s][0] > box[t][1] ||
    box[t][2] > box[s][3] || box[s][2] > box[t][3]);
}

/**
 * @brief Return the pairs of merged edges whose boxes meet, read off the
 * pairs of the edges they come from
 * @details An edge kept whole keeps its pairs. A stretch of a class lies on
 * a segment of the class that covers it, so its box lies within that
 * segment's, and a box it meets is met by the segment's: the pairs of the
 * stretches of a class are read off the edges its segments pair with, each
 * of them once, which a stamp of the class marks, as the meeting test asked
 * twice of one pair would add its crossing twice. Two classes are paired
 * once, from the one of the smaller root. Two stretches of one class lie on
 * one line and meet only at the ends they share, which are nodes already
 * @param[in] box The box of every merged edge
 * @param[in] pairs,npairs The pairs of the edges they come from
 * @param[in] n Number of the edges they come from
 * @param[in] mg Where every edge went (#arr_merge_overlaps)
 * @param[out] nout Number of pairs returned
 */
static ArrPair *
arr_mapped_pairs(const double (*box)[4], const ArrPair *pairs,
  uint32_t npairs, uint32_t n, const ArrMerge *mg, uint32_t *nout)
{
  uint32_t cap = Max(npairs, 16u), count = 0;
  ArrPair *out = palloc(sizeof(ArrPair) * cap);
  /* The edges each class's segments pair with, class by class */
  uint32_t *poff = palloc0(sizeof(uint32_t) * (n + 1));
  for (uint32_t k = 0; k < npairs; k++)
  {
    uint32_t i = pairs[k].i, j = pairs[k].j;
    uint32_t ni = mg->newof[i], nj = mg->newof[j];
    if (ni != UINT32_MAX && nj != UINT32_MAX)
    {
      arr_pair_add(&out, &count, &cap, ni, nj);
      continue;
    }
    if (ni == UINT32_MAX)
      poff[mg->root[i] + 1]++;
    if (nj == UINT32_MAX)
      poff[mg->root[j] + 1]++;
  }
  for (uint32_t r = 0; r < n; r++)
    poff[r + 1] += poff[r];
  uint32_t *partner = palloc(sizeof(uint32_t) * Max(poff[n], 1u));
  uint32_t *fill = palloc(sizeof(uint32_t) * Max(n, 1u));
  memcpy(fill, poff, sizeof(uint32_t) * n);
  for (uint32_t k = 0; k < npairs; k++)
  {
    uint32_t i = pairs[k].i, j = pairs[k].j;
    if (mg->newof[i] == UINT32_MAX)
      partner[fill[mg->root[i]]++] = j;
    if (mg->newof[j] == UINT32_MAX)
      partner[fill[mg->root[j]]++] = i;
  }
  pfree(fill);
  /* The edge kept whole, or the root of the class, already read for the
   * class being read */
  uint32_t *stamp = palloc0(sizeof(uint32_t) * Max(n, 1u));
  for (uint32_t r = 0; r < n; r++)
  {
    if (poff[r + 1] == poff[r])
      continue;
    uint32_t s0 = mg->sfirst[r], s1 = s0 + mg->scount[r];
    for (uint32_t k = poff[r]; k < poff[r + 1]; k++)
    {
      uint32_t x = partner[k];
      uint32_t key = (mg->newof[x] != UINT32_MAX) ? x : mg->root[x];
      if (stamp[key] == r + 1)
        continue;
      stamp[key] = r + 1;
      if (mg->newof[x] != UINT32_MAX)
      {
        for (uint32_t s = s0; s < s1; s++)
          if (arr_boxes_meet(box, s, mg->newof[x]))
            arr_pair_add(&out, &count, &cap, s, mg->newof[x]);
        continue;
      }
      uint32_t q = mg->root[x];
      if (q <= r)
        continue;
      uint32_t t0 = mg->sfirst[q], t1 = t0 + mg->scount[q];
      for (uint32_t s = s0; s < s1; s++)
        for (uint32_t t = t0; t < t1; t++)
          if (arr_boxes_meet(box, s, t))
            arr_pair_add(&out, &count, &cap, s, t);
    }
  }
  pfree(stamp); pfree(partner); pfree(poff);
  *nout = count;
  return out;
}

static ArrangeStatus arr_stretches_write(Arr *a, int sgn, uint32_t outer,
  ArrStretch **out, uint32_t *nout);

/* Write every node no double holds that the intervals placed, wider than a
 * few units in the last place (#arr_meet_ival), at its coordinates rounded
 * from the exact point (#meet_point_bounds), so that only the vertices
 * written are rounded and each once; one the bounds do not place keeps the
 * middle of its intervals */
static void
arr_nodes_narrow(Arr *a)
{
  for (uint32_t v = 0; v < a->nnodes; v++)
  {
    ArrNode *nd = &a->nodes[v];
    if (! nd->tower)
      continue;
    double tx = 8.0 * DBL_EPSILON * fabs(nd->x) + DBL_MIN;
    double ty = 8.0 * DBL_EPSILON * fabs(nd->y) + DBL_MIN;
    if (nd->bx.hi - nd->bx.lo <= 2.0 * tx && nd->by.hi - nd->by.lo <= 2.0 * ty)
      continue;
    MeetPoint mp = arr_meet_point(&nd->meet);
    mp.p = nd->meet.p; mp.q = nd->meet.q;
    double x, y, box[4];
    if (! meet_point_bounds(&mp, &x, &y, box))
      continue;
    nd->x = nd->meet.x = x;
    nd->y = nd->meet.y = y;
    nd->bx = ival_out(box[0], box[1]);
    nd->by = ival_out(box[2], box[3]);
  }
  return;
}

/**
 * @brief Answer the linework and the points a labelled arrangement holds
 * beside its region: the pieces of the linework where no surface winds,
 * and whether each point lies outside the surfaces and off the linework
 * @details A piece of linework winds as much on either side, as it adds
 * nothing to the winding number (#arr_weight), and lies outside the region
 * where that number is not one at least in the sense @p sgn. A point lies
 * on the linework where it lies on one of its segments, decided exactly
 * (#point_on_segment_exact), and within the region where the ray from it
 * crosses the surfaces' pieces to a winding number of one at least
 * (#arr_piece_ray); a point on a piece of the surfaces is not told
 * @param[in,out] a Arrangement, labelled, its nodes written
 * @param[in] sgn Sense of the region
 */
static ArrangeStatus
arr_lines_points(Arr *a, int sgn)
{
  if (a->lout)
  {
    *a->lout = meos_array_create(sizeof(ArrangePiece));
    for (uint32_t k = 0; k < a->npieces; k++)
    {
      const ArrPiece *p = &a->pieces[k];
      if (arr_weight(a, p->edge) != 0 || sgn * p->left >= 1)
        continue;
      const ArrNode *n1 = &a->nodes[p->from], *n2 = &a->nodes[p->to];
      ArrangePiece piece = {n1->x, n1->y, n2->x, n2->y, 0.0, 0.0, false};
      meos_array_add(*a->lout, &piece);
    }
  }
  for (uint32_t i = 0; i < a->npts; i++)
  {
    double x = a->pts[i].x, y = a->pts[i].y;
    bool covered = false;
    for (uint32_t k = 0; k < a->nlines && ! covered; k++)
    {
      const Edge *l = &a->lines[k];
      covered = point_on_segment_exact(x, y, l->x1, l->y1, l->x2, l->y2);
    }
    int w = 0, c = 0;
    for (uint32_t k = 0; k < a->npieces && ! covered; k++)
      if (! arr_piece_ray(a, &a->pieces[k], ival(x), y, &w, &c))
        return ARR_MEETING;
    a->pkeep[i] = ! covered && sgn * w < 1;
  }
  return ARR_ANSWERED;
}

/**
 * @brief Node, cut and label the edges of an arrangement set up by its
 * caller, and answer the region they wind around
 * @details As rings (#arr_rings), classified into surfaces where @p owner is
 * given, or as the stretches of its boundary where @p sout is given
 * (#arr_stretches_write). Every array the setup and the steps allocate is
 * released, but the edges, their weights and the arrays of stretches, which
 * the caller owns
 * @param[in,out] a Arrangement, its edges, nodes, ends and boxes set
 * @param[in] pairs,npairs The pairs of edges whose boxes meet, released
 */
static ArrangeStatus
arr_finish(Arr *a, ArrPair *pairs, uint32_t npairs, MeosArray ***rings,
  uint32_t *nrings, int32_t **owner, ArrStretch **sout, uint32_t *nsout)
{
  /* Room for a few cuts an edge, the doubling of #arr_cut taking over where
   * the edges cross more */
  uint32_t n = a->n;
  a->capcuts = 4 * n + 64;
  a->cuts = palloc(sizeof(ArrCut) * a->capcuts);
  a->ncuts = 0;
  a->coff = NULL;
  /* Closed curves: as many curves start at every vertex as end there, an
   * edge counting for the curves its weight stands for */
  ArrangeStatus st = ARR_ANSWERED;
  int32_t *balance = palloc0(sizeof(int32_t) * a->nnodes);
  /* Ends found at one point are one node (#arr_same_add) */
  uint32_t *root = NULL;
  if (a->nsame > 0)
  {
    root = palloc(sizeof(uint32_t) * a->nnodes);
    for (uint32_t v = 0; v < a->nnodes; v++)
      root[v] = v;
    for (uint32_t k = 0; k < a->nsame; k++)
    {
      uint32_t r1 = arr_class_root(root, a->same[k][0]);
      uint32_t r2 = arr_class_root(root, a->same[k][1]);
      if (r1 != r2)
        root[Max(r1, r2)] = Min(r1, r2);
    }
  }
  for (uint32_t i = 0; i < n; i++)
  {
    uint32_t s = root ? arr_class_root(root, a->start[i]) : a->start[i];
    uint32_t e = root ? arr_class_root(root, a->end[i]) : a->end[i];
    balance[s] += arr_weight(a, i);
    balance[e] -= arr_weight(a, i);
  }
  if (root)
    pfree(root);
  for (uint32_t v = 0; v < a->nnodes && st == ARR_ANSWERED; v++)
    if (balance[v] != 0)
      st = ARR_NOT_CLOSED;
  pfree(balance);

  if (st == ARR_ANSWERED)
    st = arr_node_all(a, pairs, npairs);
  pfree(pairs);
  if (st == ARR_ANSWERED)
    st = arr_pieces(a);
  if (st == ARR_ANSWERED)
    st = arr_order_nodes(a);
  uint32_t outer = UINT32_MAX;
  if (st == ARR_ANSWERED)
    st = arr_label_all(a, &outer);
  if (st == ARR_ANSWERED)
  {
    /* The sense of the region: the side of the outermost piece away from the
     * outside */
    const ArrPiece *p = &a->pieces[outer];
    int inner = (p->left == 0) ? p->right : p->left;
    int sgn = (inner > 0) ? 1 : -1;
    /* Linework outermost tells no sense: the surfaces of a union with
     * linework turn counterclockwise (#arrange_union_surfaces_lines) */
    if (arr_weight(a, p->edge) == 0)
      sgn = 1;
    uint32_t *rp = NULL, *roff = NULL;
    if (sout)
      st = arr_stretches_write(a, sgn, outer, sout, nsout);
    else
    {
      arr_nodes_narrow(a);
      st = arr_rings(a, sgn, outer, rings, nrings, owner ? &rp : NULL,
        owner ? &roff : NULL);
    }
    if (st == ARR_ANSWERED && ! sout && (a->lout || a->npts > 0))
      st = arr_lines_points(a, sgn);
    if (st == ARR_ANSWERED && owner)
    {
      *owner = palloc(sizeof(int32_t) * Max(*nrings, 1u));
      st = arr_surfaces(a, *nrings, rp, roff, *owner);
      if (st != ARR_ANSWERED)
      {
        pfree(*owner);
        *owner = NULL;
        arrange_rings_free(*rings, *nrings);
        *rings = NULL;
        *nrings = 0;
      }
    }
    if (rp)
    {
      pfree(rp); pfree(roff);
    }
  }
  /* Release */
  if (a->cuts)
    pfree(a->cuts);
  if (a->coff)
    pfree(a->coff);
  pfree(a->start); pfree(a->end); pfree(a->vtab);
  pfree(a->nodes);
  pfree(a->ccx); pfree(a->ccy); pfree(a->cr2); pfree(a->ccw);
  if (a->cross)
    pfree(a->cross);
  if (a->pieces)
    pfree(a->pieces);
  if (a->halves)
    pfree(a->halves);
  if (a->nrep)
    pfree(a->nrep);
  if (a->hoff)
    pfree(a->hoff);
  if (a->box)
    pfree(a->box);
  if (a->xtab)
    pfree(a->xtab);
  if (a->same)
    pfree(a->same);
  if (a->tptab)
  {
    pfree(a->tptab); pfree(a->tpnode);
  }
  return st;
}

/* The nodes of tangent points in the order of the left of their boxes */
typedef struct
{
  double lo;
  uint32_t node;
} ArrTangentBox;

static int
arr_tangent_box_cmp(const void *p, const void *q)
{
  const ArrTangentBox *a = p, *b = q;
  return (a->lo < b->lo) ? -1 : ((a->lo > b->lo) ? 1 :
    ((a->node < b->node) ? -1 : (a->node > b->node)));
}

/**
 * @brief Make one node of the nodes of tangent points of one value, decided
 * exactly where their boxes meet (#meets_same_point)
 * @return #ARR_MEETING where it is not told
 */
static ArrangeStatus
arr_same_tangents(Arr *a)
{
  uint32_t n = 0;
  ArrTangentBox *tb = palloc(sizeof(ArrTangentBox) * (a->tpmask + 1));
  for (uint32_t h = 0; h <= a->tpmask; h++)
    if (a->tptab[h])
    {
      tb[n].lo = a->nodes[a->tpnode[h]].bx.lo;
      tb[n++].node = a->tpnode[h];
    }
  qsort(tb, n, sizeof(ArrTangentBox), arr_tangent_box_cmp);
  ArrangeStatus st = ARR_ANSWERED;
  for (uint32_t k = 0; k < n && st == ARR_ANSWERED; k++)
  {
    const ArrNode *v = &a->nodes[tb[k].node];
    for (uint32_t m = k + 1; m < n && st == ARR_ANSWERED; m++)
    {
      const ArrNode *w = &a->nodes[tb[m].node];
      if (w->bx.lo > v->bx.hi)
        break;
      if (w->by.lo > v->by.hi || v->by.lo > w->by.hi)
        continue;
      MeetPoint mv = arr_meet_point(&v->meet), mw = arr_meet_point(&w->meet);
      int same = meets_same_point(&mv, &mw);
      if (same == 2)
        st = ARR_MEETING;
      else if (same == 1)
        arr_same_add(a, tb[k].node, tb[m].node);
    }
  }
  pfree(tb);
  return st;
}

/* A node of an end of a stretch at a point doubles hold exactly */
typedef struct
{
  double x, y;
  uint32_t node;
} ArrExactEnd;

static int
arr_exact_end_cmp(const void *p, const void *q)
{
  const ArrExactEnd *a = p, *b = q;
  if (a->x != b->x)
    return (a->x < b->x) ? -1 : 1;
  if (a->y != b->y)
    return (a->y < b->y) ? -1 : 1;
  return (a->node < b->node) ? -1 : (a->node > b->node);
}

/**
 * @brief Make one node of the nodes of the ends of stretches at one point
 * doubles hold exactly: where a whole circle starts, an input vertex, or a
 * tangent point of a circle of radius zero, its centre
 * @details A stretch of a whole circle keeps where it starts as such
 * (#arr_stretches_write) whatever else meets there, so the node of that end
 * is not the node of the other ends at its point (#arr_end_node), unlike
 * two tangent points of one value (#arr_same_tangents)
 * @return #ARR_MEETING where it is not told
 */
static ArrangeStatus
arr_same_exact_ends(Arr *a)
{
  ArrExactEnd *xe = palloc(sizeof(ArrExactEnd) * Max(a->nnodes, 1u));
  uint32_t n = 0;
  for (uint32_t v = 0; v < a->nnodes; v++)
  {
    const ArrNode *nd = &a->nodes[v];
    double x, y;
    /* Where a whole circle starts is (cx - r, cy) rounded once, a point the
     * doubles hold only where the difference is exact */
    if (nd->vertex ||
        (nd->origin && arr_vertex_at_origin(nd->origin, nd->x, nd->y)))
    {
      x = nd->x; y = nd->y;
    }
    else if (nd->origin || ! nd->tower ||
        ! arr_meet_exact_xy(&nd->meet, &x, &y))
      continue;
    xe[n].x = x; xe[n].y = y; xe[n++].node = v;
  }
  qsort(xe, n, sizeof(ArrExactEnd), arr_exact_end_cmp);
  for (uint32_t k = 1; k < n; k++)
    if (xe[k].x == xe[k - 1].x && xe[k].y == xe[k - 1].y)
      arr_same_add(a, xe[k - 1].node, xe[k].node);
  pfree(xe);

  /* A meeting no double holds, as the tangent point of a line x = cx - r,
   * is where a whole circle starts where #meets_same_point tells it is,
   * asked of the nodes whose boxes hold the start, in the order of the
   * left of their boxes (#arr_same_tangents) */
  ArrangeStatus st = ARR_ANSWERED;
  ArrTangentBox *tb = palloc(sizeof(ArrTangentBox) * Max(a->nnodes, 1u));
  uint32_t nt = 0;
  double wmax = 0.0;
  for (uint32_t v = 0; v < a->nnodes; v++)
  {
    const ArrNode *nd = &a->nodes[v];
    double x, y;
    if (! nd->tower || arr_meet_exact_xy(&nd->meet, &x, &y))
      continue;
    tb[nt].lo = nd->bx.lo;
    tb[nt++].node = v;
    wmax = Max(wmax, nd->bx.hi - nd->bx.lo);
  }
  qsort(tb, nt, sizeof(ArrTangentBox), arr_tangent_box_cmp);
  for (uint32_t v = 0; v < a->nnodes && nt > 0 && st == ARR_ANSWERED; v++)
  {
    const ArrNode *o = &a->nodes[v];
    if (! o->origin || ! arr_vertex_at_origin(o->origin, o->x, o->y))
      continue;
    MeetPoint op = {0};
    op.kind = MEET_VERTEX;
    op.x = o->x; op.y = o->y;
    /* The first box whose left may hold the start, the bound rounded down
     * so that it admits every box holding it */
    double from = nextafter(o->x - wmax, -INFINITY);
    uint32_t lo = 0, hi = nt;
    while (lo < hi)
    {
      uint32_t mid = lo + (hi - lo) / 2;
      if (tb[mid].lo < from)
        lo = mid + 1;
      else
        hi = mid;
    }
    for (uint32_t k = lo; k < nt && tb[k].lo <= o->x; k++)
    {
      const ArrNode *w = &a->nodes[tb[k].node];
      if (w->bx.hi < o->x || w->by.lo > o->y || w->by.hi < o->y)
        continue;
      MeetPoint mw = arr_meet_point(&w->meet);
      int same = meets_same_point(&mw, &op);
      if (same == 2)
      {
        st = ARR_MEETING;
        break;
      }
      if (same == 1)
        arr_same_add(a, v, tb[k].node);
    }
  }
  pfree(tb);
  return st;
}

/*
 * The arrangement of the curves and the rings bounding the region they wind
 * around: as #arrange_wound_boundary answers them where @p owner is NULL,
 * as #arrange_wound_surfaces answers them otherwise
 */
static ArrangeStatus
arr_arrange(const MeosArray *edges, MeosArray ***rings, uint32_t *nrings,
  int32_t **owner)
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
  /* Every edge a segment of some length, an arc whose ends are its
   * defining points or a centred arc, its whole circle, and every ring
   * closed on itself */
  for (uint32_t i = 0; i < n; i++)
  {
    const Edge *e = &a.e[i];
    if (arr_is_arc(e) && e->centred)
    {
      /* A whole circle, closed on itself */
      if (! (e->radius > 0.0) || ! isfinite(e->cx) || ! isfinite(e->cy) ||
          ! isfinite(e->radius))
        return ARR_NOT_CLOSED;
      continue;
    }
    /* A segment between tangent points, its ends told apart exactly where
     * their rounding meets */
    if (e->from)
    {
      if (! e->to || e->from == e->to)
        return ARR_NOT_CLOSED;
      continue;
    }
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
  /* The pairs of edges whose boxes meet, read once */
  a.box = palloc(sizeof(double[4]) * n);
  for (uint32_t i = 0; i < n; i++)
    arr_box(&a.e[i], a.box[i]);
  uint32_t npairs;
  ArrPair *pairs = arr_box_pairs((const double (*)[4]) a.box, n, &npairs);
  /* Segments running along each other are one stretch of the curves, read
   * once with the rise of the winding number they give across it */
  MeosArray *merged = NULL;
  ArrMerge mg;
  if (arr_merge_overlaps(edges, pairs, npairs, &merged, &a.w, &mg))
  {
    pfree(a.box);
    a.box = NULL;
    if (merged->count == 0)
    {
      meos_array_destroy(merged);
      pfree(a.w); pfree(pairs);
      pfree(mg.root); pfree(mg.newof); pfree(mg.sfirst); pfree(mg.scount);
      return ARR_NOT_CLOSED;
    }
    a.e = (const Edge *) merged->elems;
    a.n = n = merged->count;
    a.box = palloc(sizeof(double[4]) * n);
    for (uint32_t i = 0; i < n; i++)
      arr_box(&a.e[i], a.box[i]);
    uint32_t nmapped;
    ArrPair *mapped = arr_mapped_pairs((const double (*)[4]) a.box, pairs,
      npairs, edges->count, &mg, &nmapped);
    pfree(pairs);
    pairs = mapped;
    npairs = nmapped;
    pfree(mg.root); pfree(mg.newof); pfree(mg.sfirst); pfree(mg.scount);
  }
  /* The circles of the arcs */
  a.ccx = palloc(sizeof(Ival) * n); a.ccy = palloc(sizeof(Ival) * n);
  /* A segment turns neither way: its sense reads false where the order of
   * the edges around a node reads it (#arr_half_cmp) */
  a.cr2 = palloc(sizeof(Ival) * n); a.ccw = palloc0(sizeof(bool) * n);
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
  /* The tangent points ending segments between them, one node each */
  a.tpmask = 1;
  while (a.tpmask < 4 * n)
    a.tpmask <<= 1;
  a.tptab = palloc0(sizeof(TangentPoint *) * a.tpmask);
  a.tpnode = palloc(sizeof(uint32_t) * a.tpmask);
  a.tpmask -= 1;
  uint32_t ntangent = 0;
  for (uint32_t i = 0; i < n; i++)
  {
    const Edge *e = &a.e[i];
    if (e->from)
    {
      a.start[i] = arr_tangent_node(&a, e->from);
      a.end[i] = arr_tangent_node(&a, e->to);
      if (a.start[i] == UINT32_MAX || a.end[i] == UINT32_MAX)
      {
        pfree(a.start); pfree(a.end); pfree(a.vtab); pfree(a.nodes);
        pfree(a.ccx); pfree(a.ccy); pfree(a.cr2); pfree(a.ccw);
        pfree(a.box); pfree(pairs); pfree(a.tptab); pfree(a.tpnode);
        if (merged)
        {
          meos_array_destroy(merged);
          pfree(a.w);
        }
        return ARR_MEETING;
      }
      ntangent++;
      continue;
    }
    if (arr_is_arc(e) && e->centred)
    {
      /* A whole circle starts and ends at its point furthest along -x, a
       * node of its own and no input vertex: (cx - r, cy) exactly, written
       * rounded once */
      uint32_t v = arr_node_new(&a, e->x1, e->y1, false, -1);
      a.nodes[v].bx = ival_sub(ival(e->cx), ival(e->radius));
      a.nodes[v].origin = e;
      a.start[i] = a.end[i] = v;
      continue;
    }
    a.start[i] = arr_vertex(&a, e->x1, e->y1);
    a.end[i] = arr_vertex(&a, e->x2, e->y2);
  }
  /* Two tangent points of one value are one node: read where their boxes
   * meet, as where consecutive placements share a tangent */
  if (ntangent > 0 && arr_same_tangents(&a) != ARR_ANSWERED)
  {
    pfree(a.start); pfree(a.end); pfree(a.vtab); pfree(a.nodes);
    pfree(a.ccx); pfree(a.ccy); pfree(a.cr2); pfree(a.ccw);
    pfree(a.box); pfree(pairs); pfree(a.tptab); pfree(a.tpnode);
    if (a.same)
      pfree(a.same);
    if (merged)
    {
      meos_array_destroy(merged);
      pfree(a.w);
    }
    return ARR_MEETING;
  }
  ArrangeStatus st = arr_finish(&a, pairs, npairs, rings, nrings, owner,
    NULL, NULL);
  if (merged)
  {
    meos_array_destroy(merged);
    pfree(a.w);
  }
  return st;
}

/*****************************************************************************
 * The union of surfaces, merged pairwise as the stretches of their boundaries
 *****************************************************************************/

/* The edge of a stretch: its input segment, walked in its sense, or its
 * whole circle or segment between tangent points, which a stretch walks in
 * the sense of the edge, the edge #arr_arrange reads unchanged */
static void
arr_stretch_edge(const ArrStretch *st, Edge *e)
{
  const Edge *s = st->s;
  if (s->centred || s->from)
  {
    *e = *s;
    return;
  }
  memset(e, 0, sizeof(Edge));
  e->etype = EDGE_POLYSEG;
  e->x1 = st->rev ? s->x2 : s->x1; e->y1 = st->rev ? s->y2 : s->y1;
  e->x2 = st->rev ? s->x1 : s->x2; e->y2 = st->rev ? s->y1 : s->y2;
  e->xmin = Min(e->x1, e->x2); e->xmax = Max(e->x1, e->x2);
  e->ymin = Min(e->y1, e->y2); e->ymax = Max(e->y1, e->y2);
  return;
}

/* The box of a stretch: that of its ends, the intervals holding a crossing,
 * and for a stretch of a circle the points of the circle furthest along each
 * axis it passes, a quarter turn apart from its start at the positions 1, 2
 * and 3 (#arr_arc_position) */
static void
arr_stretch_box(Arr *a, uint32_t i)
{
  const ArrNode *f = &a->nodes[a->start[i]], *t = &a->nodes[a->end[i]];
  a->box[i][0] = fmin(f->bx.lo, t->bx.lo);
  a->box[i][1] = fmax(f->bx.hi, t->bx.hi);
  a->box[i][2] = fmin(f->by.lo, t->by.lo);
  a->box[i][3] = fmax(f->by.hi, t->by.hi);
  const Edge *e = &a->e[i];
  if (! e->centred)
    return;
  Ival r = ival(e->radius);
  /* Counterclockwise from (-r, 0): the bottom, the right, the top */
  const int ax[3] = {1, 0, 1}, sg[3] = {-1, 1, 1};
  for (int k = 0; k < 3; k++)
  {
    if (a->tfrom[i].lo > k + 1 || a->tto[i].hi < k + 1)
      continue;
    int s = (ax[k] == 1 && ! e->ccw) ? - sg[k] : sg[k];
    Ival c = ax[k] ? ival(e->cy) : ival(e->cx);
    Ival v = (s > 0) ? ival_add(c, r) : ival_sub(c, r);
    int lo = ax[k] ? 2 : 0;
    a->box[i][lo] = fmin(a->box[i][lo], v.lo);
    a->box[i][lo + 1] = fmax(a->box[i][lo + 1], v.hi);
  }
  return;
}

static uint32_t
arr_hash_pair(const Edge *p, const Edge *q)
{
  uint64_t hp = (uint64_t) (uintptr_t) p, hq = (uint64_t) (uintptr_t) q;
  uint64_t h = hp * UINT64CONST(0x9E3779B97F4A7C15) ^
    hq * UINT64CONST(0xC2B2AE3D27D4EB4F);
  h ^= h >> 29;
  h *= UINT64CONST(0xBF58476D1CE4E5B9);
  h ^= h >> 32;
  return (uint32_t) h;
}

/* Whether node v of an arrangement of stretches is the end a meeting
 * describes, as #arr_end_node finds it */
static bool
arr_node_is_end(const Arr *a, uint32_t v, const ArrMeet *end, const Edge *p,
  const Edge *q)
{
  const ArrNode *nd = &a->nodes[v];
  if (end->kind == MEET_ORIGIN)
    return nd->origin == p;
  if (end->kind == MEET_SEGSEG)
    return ! nd->tower && ! nd->origin && nd->xp == p && nd->xq == q;
  return nd->tower && nd->meet.kind == end->kind && nd->meet.p == p &&
    nd->meet.q == q && nd->meet.sigma == end->sigma;
}

/* The node of an end of a stretch: the one node of a vertex's coordinates
 * (#arr_vertex), of a tangent point (#arr_tangent_node), of the start of a
 * circle, or of the crossing of two input edges, whatever the order of two
 * segments, a point no double holds placed by the intervals @p box where
 * the arrangement that wrote the stretch gave them; UINT32_MAX where the
 * point cannot be placed */
static uint32_t
arr_end_node(Arr *a, const ArrMeet *end, const Ival *box)
{
  if (end->kind == MEET_VERTEX)
    return arr_vertex(a, end->x, end->y);
  if (end->kind == MEET_TANGENT)
    return arr_tangent_node_at(a, end->tp, end, box);
  const Edge *p = end->p, *q = end->q;
  if (end->kind == MEET_SEGSEG && (uintptr_t) p > (uintptr_t) q)
  {
    const Edge *t = p; p = q; q = t;
  }
  uint32_t h = arr_hash_pair(p, q) & a->xmask;
  while (a->xtab[h] >= 0)
  {
    if (arr_node_is_end(a, (uint32_t) a->xtab[h], end, p, q))
      return (uint32_t) a->xtab[h];
    h = (h + 1) & a->xmask;
  }
  uint32_t id;
  if (end->kind == MEET_ORIGIN)
  {
    /* (cx - r, cy) exactly, rounded once, as #arr_arrange places it */
    id = arr_node_new(a, p->x1, p->y1, false, -1);
    a->nodes[id].bx = ival_sub(ival(p->cx), ival(p->radius));
    a->nodes[id].origin = p;
  }
  else if (end->kind == MEET_SEGSEG)
  {
    id = arr_node_new(a, end->x, end->y, false, -1);
    ArrNode *nd = &a->nodes[id];
    nd->xp = p;
    nd->xq = q;
    nd->xsign = cross_product_sign(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1,
      q->x2, q->y2);
    Ival tp, tq;
    arr_cross_ivals(p, q, &tp, &tq, &nd->bx, &nd->by);
  }
  else
  {
    id = arr_end_tower_node(a, end, box);
    if (id == UINT32_MAX)
      return id;
  }
  a->xtab[h] = (int32_t) id;
  return id;
}

/* The end a node is, as a stretch carries it: where a circle starts and
 * ends, the one or the other as the end @p last of the stretch says */
static ArrMeet
arr_node_end(const Arr *a, uint32_t v, bool last)
{
  const ArrNode *nd = &a->nodes[v];
  if (nd->origin)
  {
    ArrMeet m = {MEET_ORIGIN, nd->origin, NULL, last ? 1 : 0, nd->x, nd->y,
      NULL};
    return m;
  }
  if (nd->tower)
    return nd->meet;
  ArrMeet m = {nd->vertex ? MEET_VERTEX : MEET_SEGSEG,
    nd->vertex ? NULL : nd->xp, nd->vertex ? NULL : nd->xq, 0, nd->x, nd->y,
    NULL};
  return m;
}

/* A point of the line of a class of stretches running along each other: an
 * end of one of them, at its position along the line */
typedef struct
{
  Ival t;
  ArrMeet m;
  uint32_t node;
  uint32_t member;
  bool to;
} ArrLinePoint;

/* The per-edge arrays of an arrangement of stretches, written anew where the
 * stretches running along each other are merged */
typedef struct
{
  Edge *e;
  const Edge **sup;
  bool *erev;
  uint32_t *start, *end;
  Ival *tfrom, *tto;
  ArrMeet *mfrom, *mto;
  int32_t *w;
  uint8_t *half;
} ArrStretchEdges;

static void
arr_stretch_edges_alloc(ArrStretchEdges *se, uint32_t n)
{
  n = Max(n, 1u);
  se->e = palloc(sizeof(Edge) * n);
  se->sup = palloc(sizeof(const Edge *) * n);
  se->erev = palloc(sizeof(bool) * n);
  se->start = palloc(sizeof(uint32_t) * n);
  se->end = palloc(sizeof(uint32_t) * n);
  se->tfrom = palloc(sizeof(Ival) * n);
  se->tto = palloc(sizeof(Ival) * n);
  se->mfrom = palloc(sizeof(ArrMeet) * n);
  se->mto = palloc(sizeof(ArrMeet) * n);
  se->w = palloc(sizeof(int32_t) * n);
  se->half = palloc(sizeof(uint8_t) * n);
  return;
}

/* Release the per-edge arrays an arrangement of stretches holds */
static void
arr_stretch_edges_free(Arr *a, Edge *e)
{
  pfree(e); pfree(a->sup); pfree(a->erev);
  pfree(a->tfrom); pfree(a->tto); pfree(a->mfrom); pfree(a->mto);
  if (a->w)
    pfree(a->w);
  if (a->half)
    pfree(a->half);
  return;
}

/**
 * @brief Replace the stretches running along each other by the stretches
 * they share, each weighted by the stretches running along it
 * @details As #arr_merge_overlaps merges segments, but the ends of a stretch
 * are input vertices or crossings, so the ends of a class are ordered along
 * its line exactly (#arr_line_order) rather than by their coordinates, and
 * ends at one point are one node. Each stretch written lies on the segment of
 * one of the stretches covering it, walked in the order of the line, so that
 * the segment of every edge holds the edge
 * @param[in,out] a Arrangement of stretches, its per-edge arrays replaced
 * where any two run along each other
 * @param[in] pairs,npairs The pairs of edges whose boxes meet
 * @param[in,out] ecopy The edges the arrangement owns
 * @param[out] done Whether any two ran along each other
 * @param[out] mg Where every edge went, set where @p done
 */
static ArrangeStatus
arr_merge_stretches(Arr *a, const ArrPair *pairs, uint32_t npairs,
  Edge **ecopy, bool *done, ArrMerge *mg)
{
  *done = false;
  uint32_t n = a->n;
  uint32_t *parent = NULL;
  for (uint32_t k = 0; k < npairs; k++)
  {
    uint32_t i = pairs[k].i, j = pairs[k].j;
    const Edge *p = &a->e[i], *q = &a->e[j];
    /* Stretches of circles running along each other are not answered
     * (#arr_meet_curved), nor a segment between tangent points along a
     * segment of the input bounding a surface; along linework it is */
    if (arr_is_arc(p) || arr_is_arc(q) ||
        ((p->from != NULL) != (q->from != NULL) && arr_weight(a, i) != 0 &&
         arr_weight(a, j) != 0))
      continue;
    if (p->from || q->from)
    {
      /* Two segments between tangent points are on one line, sharing more
       * than an end, where #towerseg_meet tells it, the ends of either not
       * kept off the line of the other by the intervals holding them; two
       * stretches of one boundary never run along each other */
      if (a->sup[i] == a->sup[j] || arr_same_half(a, i, j))
        continue;
      const ArrNode *pa = &a->nodes[a->start[i]], *pb = &a->nodes[a->end[i]];
      int s1 = arr_orient_ival(pa, pb, &a->nodes[a->start[j]]);
      int s2 = arr_orient_ival(pa, pb, &a->nodes[a->end[j]]);
      if ((s1 != 0 && s1 != 2) || (s2 != 0 && s2 != 2))
        continue;
      TowerMeet tm[2];
      if (towerseg_meet(p, q, tm) != -1)
        continue;
    }
    else
    {
      Ival par = arr_cross_ival(p->x1, p->y1, p->x2, p->y2, q->x1, q->y1,
        q->x2, q->y2);
      if (par.lo > 0.0 || par.hi < 0.0 || ! arr_one_line(p, q))
        continue;
    }
    int sh = arr_stretches_share(a, i, j);
    if (sh == 2)
    {
      if (parent)
        pfree(parent);
      return ARR_ORDER;
    }
    if (sh == 0)
      continue;
    if (! parent)
    {
      parent = palloc(sizeof(uint32_t) * n);
      for (uint32_t m = 0; m < n; m++)
        parent[m] = m;
    }
    uint32_t ri = arr_class_root(parent, i), rj = arr_class_root(parent, j);
    if (ri != rj)
      parent[Max(ri, rj)] = Min(ri, rj);
  }
  if (! parent)
    return ARR_ANSWERED;
  *done = true;

  /* The members of each class, class by class, in the order of the edges */
  uint32_t *root = palloc(sizeof(uint32_t) * n);
  uint32_t *moff = palloc0(sizeof(uint32_t) * (n + 1));
  for (uint32_t i = 0; i < n; i++)
  {
    root[i] = arr_class_root(parent, i);
    moff[root[i] + 1]++;
  }
  for (uint32_t i = 0; i < n; i++)
    moff[i + 1] += moff[i];
  uint32_t *members = palloc(sizeof(uint32_t) * n);
  uint32_t *fill = palloc(sizeof(uint32_t) * n);
  memcpy(fill, moff, sizeof(uint32_t) * n);
  for (uint32_t i = 0; i < n; i++)
    members[fill[root[i]]++] = i;
  pfree(fill);

  /* A class of m stretches writes at most 2m - 1 stretches */
  ArrStretchEdges se;
  arr_stretch_edges_alloc(&se, 2 * n);
  ArrLinePoint *pts = palloc(sizeof(ArrLinePoint) * 2 * n);
  uint32_t *rep = palloc(sizeof(uint32_t) * 2 * n);
  int32_t *rise = palloc(sizeof(int32_t) * 2 * n);
  int32_t *lrise = palloc(sizeof(int32_t) * 2 * n);
  uint32_t *lo = palloc(sizeof(uint32_t) * n), *hi = palloc(sizeof(uint32_t) * n);
  bool *fwd = palloc(sizeof(bool) * n);
  mg->root = root;
  mg->newof = palloc(sizeof(uint32_t) * n);
  mg->sfirst = palloc0(sizeof(uint32_t) * n);
  mg->scount = palloc0(sizeof(uint32_t) * n);
  ArrangeStatus st = ARR_ANSWERED;
  uint32_t c = 0;
  for (uint32_t i = 0; i < n && st == ARR_ANSWERED; i++)
  {
    uint32_t r = root[i], m = moff[r + 1] - moff[r];
    if (m == 1)
    {
      mg->newof[i] = c;
      se.e[c] = a->e[i]; se.sup[c] = a->sup[i]; se.erev[c] = a->erev[i];
      se.start[c] = a->start[i]; se.end[c] = a->end[i];
      se.tfrom[c] = a->tfrom[i]; se.tto[c] = a->tto[i];
      se.mfrom[c] = a->mfrom[i]; se.mto[c] = a->mto[i];
      se.half[c] = a->half ? a->half[i] :
        (a->split ? (uint8_t) (i >= a->split) : 2);
      se.w[c++] = arr_weight(a, i);
      continue;
    }
    mg->newof[i] = UINT32_MAX;
    /* A class is written where its first member stands */
    if (r != i)
      continue;
    mg->sfirst[r] = c;
    const uint32_t *mem = &members[moff[r]];
    /* The ends of the class in the order of the line of its root */
    uint32_t npts = 0;
    for (uint32_t k = 0; k < m; k++)
      for (int to = 0; to < 2; to++)
      {
        uint32_t node = to ? a->end[mem[k]] : a->start[mem[k]];
        const ArrNode *nd = &a->nodes[node];
        ArrLinePoint *lp = &pts[npts++];
        lp->t = arr_node_position(&a->e[r], nd);
        lp->m = to ? a->mto[mem[k]] : a->mfrom[mem[k]];
        lp->node = node;
        lp->member = k;
        lp->to = (to != 0);
      }
    for (uint32_t k = 1; k < npts && st == ARR_ANSWERED; k++)
      for (uint32_t q = k; q > 0; q--)
      {
        int o = arr_line_order(a, r, pts[q - 1].t, &pts[q - 1].m, pts[q].t,
          &pts[q].m);
        if (o == 2)
        {
          st = ARR_ORDER;
          break;
        }
        if (o <= 0)
          break;
        ArrLinePoint t = pts[q]; pts[q] = pts[q - 1]; pts[q - 1] = t;
      }
    if (st != ARR_ANSWERED)
      break;
    /* The distinct points, each standing for the ends at it, a vertex
     * where one of them is */
    uint32_t nd = 0;
    for (uint32_t q = 0; q < npts && st == ARR_ANSWERED; q++)
    {
      int o = (nd == 0) ? -1 : arr_line_order(a, r, pts[rep[nd - 1]].t,
        &pts[rep[nd - 1]].m, pts[q].t, &pts[q].m);
      if (o == 2)
        st = ARR_ORDER;
      else if (o != 0)
        rep[nd++] = q;
      else if (pts[q].node != pts[rep[nd - 1]].node)
      {
        arr_same_add(a, pts[rep[nd - 1]].node, pts[q].node);
        if (a->nodes[pts[q].node].vertex &&
            ! a->nodes[pts[rep[nd - 1]].node].vertex)
          rep[nd - 1] = q;
      }
      uint32_t d = nd - 1;
      if (pts[q].to)
        hi[pts[q].member] = d;
      else
        lo[pts[q].member] = d;
    }
    if (st != ARR_ANSWERED)
      break;
    /* The rise of the winding number across each stretch, and the members
     * of linework running along it, which wind around nothing */
    memset(rise, 0, sizeof(int32_t) * nd);
    memset(lrise, 0, sizeof(int32_t) * nd);
    for (uint32_t k = 0; k < m; k++)
    {
      fwd[k] = lo[k] < hi[k];
      if (! fwd[k])
      {
        uint32_t t = lo[k]; lo[k] = hi[k]; hi[k] = t;
      }
      int32_t wk = arr_weight(a, mem[k]);
      rise[lo[k]] += fwd[k] ? wk : -wk;
      rise[hi[k]] -= fwd[k] ? wk : -wk;
      if (wk == 0)
      {
        lrise[lo[k]]++;
        lrise[hi[k]]--;
      }
    }
    int32_t weight = 0, nline = 0;
    for (uint32_t d = 0; d + 1 < nd; d++)
    {
      weight += rise[d];
      nline += lrise[d];
      /* A stretch no surface bounds is written where linework runs along
       * it, as linework */
      if (weight == 0 && nline == 0)
        continue;
      bool line = (weight == 0);
      /* The segment of a member covering it, walked along the line; a
       * segment between tangent points runs in the sense of its edge
       * (#arr_stretches_write), so it is that of a member running the way
       * the winding number rises, which carries the rise as it is */
      uint32_t k = 0;
      while (k < m && ! (lo[k] <= d && hi[k] > d &&
          (line ? arr_weight(a, mem[k]) == 0 :
            (arr_weight(a, mem[k]) != 0 && (! a->e[mem[k]].from ||
              fwd[k] == (weight > 0))))))
        k++;
      if (k == m)
      {
        st = ARR_ORDER;
        break;
      }
      uint32_t x = mem[k];
      bool tower = (a->e[x].from != NULL);
      se.e[c] = a->e[x];
      se.sup[c] = a->sup[x];
      se.erev[c] = a->erev[x];
      const ArrLinePoint *p0 = &pts[rep[d]], *p1 = &pts[rep[d + 1]];
      if (! fwd[k])
      {
        if (tower)
        {
          const ArrLinePoint *t = p0; p0 = p1; p1 = t;
        }
        else
        {
          Edge *ex = &se.e[c];
          double t;
          t = ex->x1; ex->x1 = ex->x2; ex->x2 = t;
          t = ex->y1; ex->y1 = ex->y2; ex->y2 = t;
          se.erev[c] = ! se.erev[c];
        }
      }
      const ArrNode *n0 = &a->nodes[p0->node], *n1 = &a->nodes[p1->node];
      se.start[c] = p0->node; se.end[c] = p1->node;
      se.tfrom[c] = arr_node_position(&se.e[c], n0);
      se.tto[c] = arr_node_position(&se.e[c], n1);
      se.mfrom[c] = p0->m; se.mto[c] = p1->m;
      se.half[c] = 2;
      se.w[c++] = (tower && ! fwd[k]) ? - weight : weight;
    }
    mg->scount[r] = c - mg->sfirst[r];
  }
  pfree(pts); pfree(rep); pfree(rise); pfree(lrise); pfree(lo);
  pfree(hi); pfree(fwd);
  pfree(members); pfree(moff); pfree(parent);
  /* The arrangement takes the merged edges */
  pfree(a->start); pfree(a->end);
  arr_stretch_edges_free(a, *ecopy);
  *ecopy = se.e;
  a->e = se.e; a->n = c;
  a->sup = se.sup; a->erev = se.erev;
  a->start = se.start; a->end = se.end;
  a->tfrom = se.tfrom; a->tto = se.tto;
  a->mfrom = se.mfrom; a->mto = se.mto;
  a->w = se.w;
  a->half = se.half;
  if (st != ARR_ANSWERED)
  {
    pfree(mg->root); pfree(mg->newof); pfree(mg->sfirst); pfree(mg->scount);
    *done = false;
  }
  return st;
}

/**
 * @brief Write the boundary of the region as its stretches
 * @details A piece bounds the region where the winding number is at least
 * one on one side of it and not on the other, as #arr_rings reads it; it is
 * written with the region on its left, and the pieces of one edge that follow
 * each other along it, walked the same way, as one stretch
 */
static ArrangeStatus
arr_stretches_write(Arr *a, int sgn, uint32_t outer, ArrStretch **out,
  uint32_t *nout)
{
  for (uint32_t k = 0; k < a->npieces; k++)
  {
    ArrPiece *p = &a->pieces[k];
    bool inl = sgn * p->left >= 1, inr = sgn * p->right >= 1;
    p->result = inl != inr;
    p->flip = inr;
  }
  if (outer == UINT32_MAX || ! a->pieces[outer].result)
    return ARR_SHELLS;
  ArrStretch *res = palloc(sizeof(ArrStretch) * Max(a->npieces, 1u));
  uint32_t c = 0;
  for (uint32_t k = 0; k < a->npieces; )
  {
    const ArrPiece *p = &a->pieces[k];
    if (! p->result)
    {
      k++;
      continue;
    }
    uint32_t last = k;
    while (last + 1 < a->npieces)
    {
      const ArrPiece *q = &a->pieces[last + 1];
      if (q->edge != p->edge || ! q->result || q->flip != p->flip ||
          q->from != a->pieces[last].to)
        break;
      last++;
    }
    /* A stretch of a circle or of a segment between tangent points is
     * walked in the sense of its edge, the region on the left of every
     * surface bounded by them; one walked against it is not answered */
    const Edge *se = a->sup[p->edge];
    if (p->flip && (se->centred || se->from))
    {
      pfree(res);
      return ARR_OVERLAP;
    }
    ArrStretch *s = &res[c++];
    s->s = se;
    s->rev = (a->erev[p->edge] != p->flip);
    uint32_t nf = p->flip ? a->pieces[last].to : p->from;
    uint32_t nt = p->flip ? p->from : a->pieces[last].to;
    /* A stretch of a whole circle from where it starts, or to where it
     * ends, keeps that end as such (#arr_line_order reads it before and
     * after every point of the circle), the node of the edge before the
     * nodes at one point were made one (#arr_node_merge): the node standing
     * for them may be another meeting there, and a circle read from it to
     * itself runs nowhere */
    if (se->centred && ! p->flip)
    {
      if ((k == 0 || a->pieces[k - 1].edge != p->edge) &&
          a->nodes[a->start[p->edge]].origin)
        nf = a->start[p->edge];
      if ((last + 1 == a->npieces || a->pieces[last + 1].edge != p->edge) &&
          a->nodes[a->end[p->edge]].origin)
        nt = a->end[p->edge];
    }
    s->from = arr_node_end(a, nf, false);
    s->to = arr_node_end(a, nt, true);
    s->fbox[0] = a->nodes[nf].bx; s->fbox[1] = a->nodes[nf].by;
    s->tbox[0] = a->nodes[nt].bx; s->tbox[1] = a->nodes[nt].by;
    k = last + 1;
  }
  *out = res;
  *nout = c;
  return ARR_ANSWERED;
}

/* The circles of the arcs of an arrangement of stretches and the senses they
 * turn, as #arr_arrange reads them; a segment turns neither way */
static void
arr_stretch_circles(Arr *a)
{
  uint32_t n = Max(a->n, 1u);
  a->ccx = palloc(sizeof(Ival) * n); a->ccy = palloc(sizeof(Ival) * n);
  a->cr2 = palloc(sizeof(Ival) * n); a->ccw = palloc0(sizeof(bool) * n);
  for (uint32_t i = 0; i < a->n; i++)
    if (arr_is_arc(&a->e[i]))
    {
      arr_circle(&a->e[i], &a->ccx[i], &a->ccy[i], &a->cr2[i]);
      a->ccw[i] = arr_arc_ccw(&a->e[i]);
    }
  return;
}

/* The position of an end of stretch i along its edge: 0 where a circle
 * starts and after every other point where it ends, else read off the node,
 * as #arr_node_position reads it along a segment and #arr_tower_position
 * along a circle or a segment between tangent points */
static Ival
arr_end_position(const Arr *a, uint32_t i, uint32_t node, const ArrMeet *m)
{
  if (m->kind == MEET_ORIGIN)
    return ival(m->sigma ? INFINITY : 0.0);
  const Edge *e = &a->e[i];
  if (e->centred || e->from)
    return arr_tower_position(a, i, node);
  return arr_node_position(e, &a->nodes[node]);
}

/* Linework and points arranged beside the surfaces of a union
 * (#arrange_union_surfaces_lines) */
typedef struct
{
  const Edge *lines;      /**< The segments of the linework, the last
                               @p nlines stretches arranged */
  uint32_t nlines;
  MeosArray **lout;       /**< Where its pieces no surface covers go */
  const POINT2D *pts;     /**< Points */
  uint32_t npts;
  bool *pkeep;            /**< Whether each lies off the surfaces and the
                               linework */
} ArrLinework;

/**
 * @brief The arrangement of stretches and the region they wind around: as
 * #arr_arrange answers it, as rings or, where @p sout is given, as the
 * stretches of its boundary, and where @p lw is given the linework and the
 * points beside it (#arr_lines_points)
 */
static ArrangeStatus
arr_arrange_stretches(const ArrStretch *st, uint32_t n, uint32_t split,
  const ArrLinework *lw, MeosArray ***rings, uint32_t *nrings,
  int32_t **owner, ArrStretch **sout, uint32_t *nsout)
{
  if (n == 0)
    return ARR_NOT_CLOSED;
  /* A stretch of a circle or of a segment between tangent points runs in
   * the sense of its edge (#arr_stretches_write) */
  for (uint32_t i = 0; i < n; i++)
    if (st[i].rev && (st[i].s->centred || st[i].s->from))
      return ARR_OVERLAP;
  Arr a;
  memset(&a, 0, sizeof(Arr));
  Edge *ecopy = palloc(sizeof(Edge) * n);
  a.sup = palloc(sizeof(const Edge *) * n);
  a.erev = palloc(sizeof(bool) * n);
  for (uint32_t i = 0; i < n; i++)
  {
    arr_stretch_edge(&st[i], &ecopy[i]);
    a.sup[i] = st[i].s;
    a.erev[i] = st[i].rev;
  }
  a.e = ecopy;
  a.n = n;
  a.split = split;
  /* Linework winds around nothing, and meets every other edge */
  if (lw && (lw->nlines > 0 || lw->npts > 0))
  {
    a.lines = lw->lines; a.nlines = lw->nlines; a.lout = lw->lout;
    a.pts = lw->pts; a.npts = lw->npts; a.pkeep = lw->pkeep;
    a.w = palloc(sizeof(int32_t) * n);
    a.half = palloc(sizeof(uint8_t) * n);
    for (uint32_t i = 0; i < n; i++)
    {
      bool line = i + lw->nlines >= n;
      a.w[i] = line ? 0 : 1;
      a.half[i] = (line || ! split) ? 2 : (uint8_t) (i >= split);
    }
  }
  /* The circles of the stretches of circles, read by the positions of
   * their ends */
  arr_stretch_circles(&a);
  /* The nodes of the ends, each found once */
  a.capnodes = 2 * n + 16;
  a.nodes = palloc(sizeof(ArrNode) * a.capnodes);
  a.vmask = 1;
  while (a.vmask < 4 * n)
    a.vmask <<= 1;
  a.vtab = palloc(sizeof(int32_t) * a.vmask);
  a.xtab = palloc(sizeof(int32_t) * a.vmask);
  for (uint32_t h = 0; h < a.vmask; h++)
    a.vtab[h] = a.xtab[h] = -1;
  a.tptab = palloc0(sizeof(TangentPoint *) * a.vmask);
  a.tpnode = palloc(sizeof(uint32_t) * a.vmask);
  a.vmask -= 1;
  a.xmask = a.tpmask = a.vmask;
  a.start = palloc(sizeof(uint32_t) * n);
  a.end = palloc(sizeof(uint32_t) * n);
  a.mfrom = palloc(sizeof(ArrMeet) * n);
  a.mto = palloc(sizeof(ArrMeet) * n);
  a.tfrom = palloc(sizeof(Ival) * n);
  a.tto = palloc(sizeof(Ival) * n);
  a.box = palloc(sizeof(double[4]) * n);
  ArrangeStatus status = ARR_ANSWERED;
  bool tangent = false;
  for (uint32_t i = 0; i < n && status == ARR_ANSWERED; i++)
  {
    a.start[i] = arr_end_node(&a, &st[i].from, st[i].fbox);
    a.end[i] = arr_end_node(&a, &st[i].to, st[i].tbox);
    if (a.start[i] == UINT32_MAX || a.end[i] == UINT32_MAX)
    {
      status = ARR_MEETING;
      break;
    }
    a.mfrom[i] = st[i].from;
    a.mto[i] = st[i].to;
    tangent |= st[i].from.kind == MEET_TANGENT ||
      st[i].to.kind == MEET_TANGENT;
    a.tfrom[i] = arr_end_position(&a, i, a.start[i], &a.mfrom[i]);
    a.tto[i] = arr_end_position(&a, i, a.end[i], &a.mto[i]);
    arr_stretch_box(&a, i);
  }
  /* Two tangent points of one value are one node (#arr_arrange) */
  if (status == ARR_ANSWERED && tangent)
    status = arr_same_tangents(&a);
  if (status == ARR_ANSWERED)
    status = arr_same_exact_ends(&a);
  if (status != ARR_ANSWERED)
  {
    pfree(a.start); pfree(a.end); pfree(a.vtab); pfree(a.xtab);
    pfree(a.tptab); pfree(a.tpnode);
    pfree(a.nodes); pfree(a.box);
    pfree(a.ccx); pfree(a.ccy); pfree(a.cr2); pfree(a.ccw);
    if (a.same)
      pfree(a.same);
    arr_stretch_edges_free(&a, ecopy);
    return status;
  }
  uint32_t npairs;
  ArrPair *pairs = arr_box_pairs((const double (*)[4]) a.box, n, &npairs);
  /* Stretches running along each other are one stretch of the boundaries,
   * read once with the rise of the winding number they give across it */
  bool done;
  ArrMerge mg;
  status = arr_merge_stretches(&a, pairs, npairs, &ecopy, &done, &mg);
  if (status == ARR_ANSWERED && done)
  {
    /* The merged stretches are numbered anew */
    a.split = 0;
    pfree(a.ccx); pfree(a.ccy); pfree(a.cr2); pfree(a.ccw);
    arr_stretch_circles(&a);
    pfree(a.box);
    a.box = palloc(sizeof(double[4]) * Max(a.n, 1u));
    for (uint32_t i = 0; i < a.n; i++)
      arr_stretch_box(&a, i);
    uint32_t nmapped;
    ArrPair *mapped = arr_mapped_pairs((const double (*)[4]) a.box, pairs,
      npairs, n, &mg, &nmapped);
    pfree(pairs);
    pairs = mapped;
    npairs = nmapped;
    pfree(mg.root); pfree(mg.newof); pfree(mg.sfirst); pfree(mg.scount);
    if (a.n == 0)
      status = ARR_NOT_CLOSED;
  }
  if (status != ARR_ANSWERED)
  {
    pfree(pairs);
    pfree(a.start); pfree(a.end); pfree(a.vtab); pfree(a.xtab);
    pfree(a.tptab); pfree(a.tpnode);
    pfree(a.nodes); pfree(a.box);
    pfree(a.ccx); pfree(a.ccy); pfree(a.cr2); pfree(a.ccw);
    if (a.same)
      pfree(a.same);
    arr_stretch_edges_free(&a, ecopy);
    return status;
  }
  status = arr_finish(&a, pairs, npairs, rings, nrings, owner, sout, nsout);
  arr_stretch_edges_free(&a, ecopy);
  return status;
}

/**
 * @brief Return the rings bounding the union of surfaces bounded by
 * segments, as surfaces: each shell and the holes it owns
 * @details A point lies in the union where it lies in one of the surfaces,
 * which is where the rings of the surfaces of either half of them wind around
 * it at least once, so the union is merged pairwise: each surface is the
 * stretches of its rings, and two unions are merged in the arrangement of the
 * stretches of their boundaries (#arr_arrange_stretches), which answers the
 * stretches of the boundary of their union. Every end of a stretch is an
 * input vertex or the crossing of two input segments, whatever the depth of
 * the merge, so every question is the one the arrangement of all the rings
 * asks, decided as exactly, and only the last arrangement rounds the
 * crossings it writes. The edges an arrangement reads are those of two
 * boundaries, not those of every surface beneath them, which is what the
 * merge saves where the surfaces overlap
 * @param[in] edges The segments of the rings of the surfaces, the shells
 * turning counterclockwise and the holes clockwise, each ring's segments
 * consecutive and closed
 * @param[in] soff First segment of each surface, one past the last
 * @param[in] nsurf Number of surfaces
 * @param[out] rings,nrings The rings, released by #arrange_rings_free
 * @param[out] owner For every ring, the index of the shell owning it where it
 * is a hole, -1 where it is a shell; released by the caller
 * @return #ARR_ANSWERED, or the question left unanswered
 */
ArrangeStatus
arrange_union_surfaces(const MeosArray *edges, const uint32_t *soff,
  uint32_t nsurf, MeosArray ***rings, uint32_t *nrings, int32_t **owner)
{
  return arrange_union_surfaces_lines(edges, soff, nsurf, NULL, 0, NULL, 0,
    rings, nrings, owner, NULL, NULL);
}

/**
 * @brief Return the rings bounding the union of surfaces bounded by
 * segments and arcs, as surfaces, and the linework and the points beside
 * them that no surface covers
 * @details As #arrange_union_surfaces, the linework arranged with the two
 * unions of the last merge: a segment of it winds around nothing, so it
 * bounds no ring, and its pieces where no surface winds are answered; a
 * point is answered where it lies outside every surface and off every
 * segment of the linework (#arr_lines_points)
 * @param[in] edges,soff,nsurf The surfaces, as #arrange_union_surfaces
 * reads them, turning counterclockwise
 * @param[in] lines,nlines The segments of the linework, of the input
 * @param[in] pts,npts The points
 * @param[out] rings,nrings,owner The surfaces, as #arrange_union_surfaces
 * answers them
 * @param[out] lout Where @p lines is given, its pieces no surface covers, as
 * segments (#ArrangePiece); released by the caller
 * @param[out] pkeep Where @p pts is given, whether each point lies off the
 * surfaces and the linework
 * @return #ARR_ANSWERED, or the question left unanswered
 */
ArrangeStatus
arrange_union_surfaces_lines(const MeosArray *edges, const uint32_t *soff,
  uint32_t nsurf, const Edge *lines, uint32_t nlines, const POINT2D *pts,
  uint32_t npts, MeosArray ***rings, uint32_t *nrings, int32_t **owner,
  MeosArray **lout, bool *pkeep)
{
  assert(edges); assert(soff); assert(rings); assert(nrings); assert(owner);
  assert(! nlines || (lines && lout)); assert(! npts || (pts && pkeep));
  *rings = NULL;
  *nrings = 0;
  *owner = NULL;
  if (lout)
    *lout = NULL;
  const Edge *e = (const Edge *) edges->elems;
  /* The boundary of each surface, as its stretches */
  ArrStretch **part = palloc(sizeof(ArrStretch *) * Max(nsurf, 1u));
  uint32_t *count = palloc(sizeof(uint32_t) * Max(nsurf, 1u));
  uint32_t m = 0;
  for (uint32_t k = 0; k < nsurf; k++)
  {
    uint32_t ne = soff[k + 1] - soff[k];
    if (ne == 0)
      continue;
    part[m] = palloc(sizeof(ArrStretch) * ne);
    for (uint32_t i = 0; i < ne; i++)
    {
      const Edge *s = &e[soff[k] + i];
      ArrStretch *sk = &part[m][i];
      sk->s = s;
      sk->rev = false;
      /* No arrangement placed its ends yet */
      sk->fbox[0] = sk->fbox[1] = sk->tbox[0] = sk->tbox[1] =
        (Ival) {INFINITY, -INFINITY};
      if (s->centred)
      {
        /* A whole circle, from where it starts to where it ends */
        sk->from = (ArrMeet) {MEET_ORIGIN, s, NULL, 0, s->x1, s->y1, NULL};
        sk->to = (ArrMeet) {MEET_ORIGIN, s, NULL, 1, s->x2, s->y2, NULL};
      }
      else if (s->from)
      {
        sk->from = (ArrMeet) {MEET_TANGENT, NULL, NULL, 0, s->x1, s->y1,
          s->from};
        sk->to = (ArrMeet) {MEET_TANGENT, NULL, NULL, 0, s->x2, s->y2,
          s->to};
      }
      else
      {
        sk->from = (ArrMeet) {MEET_VERTEX, NULL, NULL, 0, s->x1, s->y1,
          NULL};
        sk->to = (ArrMeet) {MEET_VERTEX, NULL, NULL, 0, s->x2, s->y2, NULL};
      }
    }
    count[m++] = ne;
  }
  ArrangeStatus st = (m > 0 || nlines > 0) ? ARR_ANSWERED : ARR_NOT_CLOSED;
  /* Two unions merged at a time, level by level, until the last two */
  while (m > 2 && st == ARR_ANSWERED)
  {
    uint32_t k = 0;
    for (uint32_t g = 0; g < m; g += 2)
    {
      if (g + 1 == m || st != ARR_ANSWERED)
      {
        part[k] = part[g];
        count[k++] = count[g];
        if (g + 1 < m)
        {
          part[k] = part[g + 1];
          count[k++] = count[g + 1];
        }
        continue;
      }
      /* The union of many surfaces is long, so it can be stopped between two
       * merges */
      meos_check_for_interrupts();
      uint32_t nboth = count[g] + count[g + 1];
      ArrStretch *both = palloc(sizeof(ArrStretch) * nboth);
      memcpy(both, part[g], sizeof(ArrStretch) * count[g]);
      memcpy(both + count[g], part[g + 1], sizeof(ArrStretch) * count[g + 1]);
      pfree(part[g]); pfree(part[g + 1]);
      ArrStretch *out = NULL;
      uint32_t nout = 0;
      st = arr_arrange_stretches(both, nboth, count[g], NULL, NULL, NULL,
        NULL, &out, &nout);
      pfree(both);
      part[k] = out;
      count[k++] = (st == ARR_ANSWERED) ? nout : 0;
    }
    m = k;
  }
  if (st == ARR_ANSWERED)
  {
    uint32_t total = nlines;
    for (uint32_t g = 0; g < m; g++)
      total += count[g];
    ArrStretch *all = palloc(sizeof(ArrStretch) * Max(total, 1u));
    uint32_t at = 0;
    for (uint32_t g = 0; g < m; at += count[g], g++)
      memcpy(all + at, part[g], sizeof(ArrStretch) * count[g]);
    /* The linework last, each segment from its start to its end */
    for (uint32_t k = 0; k < nlines; k++)
    {
      const Edge *l = &lines[k];
      ArrStretch *sk = &all[at + k];
      sk->s = l;
      sk->rev = false;
      sk->fbox[0] = sk->fbox[1] = sk->tbox[0] = sk->tbox[1] =
        (Ival) {INFINITY, -INFINITY};
      sk->from = (ArrMeet) {MEET_VERTEX, NULL, NULL, 0, l->x1, l->y1, NULL};
      sk->to = (ArrMeet) {MEET_VERTEX, NULL, NULL, 0, l->x2, l->y2, NULL};
    }
    ArrLinework lw = {lines, nlines, lout, pts, npts, pkeep};
    st = arr_arrange_stretches(all, total, (m == 2) ? count[0] : 0,
      (nlines || npts) ? &lw : NULL, rings, nrings, owner, NULL, NULL);
    pfree(all);
  }
  for (uint32_t g = 0; g < m; g++)
    if (part[g])
      pfree(part[g]);
  pfree(part); pfree(count);
  return st;
}

/**
 * @brief Return the rings bounding the region closed curves wind around,
 * the first its shell and the others its holes (#arr_arrange)
 * @details The curves are given as their edges, as #geom_extract_edges emits
 * those of the rings of a curve polygon: segments and arcs, each ring's edges
 * consecutive, each starting exactly where the one before ends, and the last
 * ending where the first starts. The region is the set of points the curves
 * wind around at least once, in the sense the outermost curve turns. The
 * first ring answered is the one bounding it from outside, turning
 * counterclockwise; the others are its holes, turning clockwise. Each ring is
 * an array of pieces (#ArrangePiece), segments and arcs, in the order walked.
 * @param[in] edges The edges of the curves
 * @param[out] rings,nrings The rings, released by #arrange_rings_free
 * @return #ARR_ANSWERED, or the question left unanswered
 */
ArrangeStatus
arrange_wound_boundary(const MeosArray *edges, MeosArray ***rings,
  uint32_t *nrings)
{
  return arr_arrange(edges, rings, nrings, NULL);
}

/**
 * @brief Return the rings bounding the region closed curves wind around, as
 * surfaces: each shell and the holes it owns
 * @details As #arrange_wound_boundary, the region possibly in several
 * surfaces. A ring is a hole where it lies within an odd number of the
 * others, owned by the innermost of them, and a shell otherwise, which
 * containment between rings that never cross decides exactly
 * @param[in] edges The edges of the curves
 * @param[out] rings,nrings The rings, released by #arrange_rings_free
 * @param[out] owner For every ring, the index of the shell owning it where it
 * is a hole, -1 where it is a shell; released by the caller
 * @return #ARR_ANSWERED, or the question left unanswered
 */
ArrangeStatus
arrange_wound_surfaces(const MeosArray *edges, MeosArray ***rings,
  uint32_t *nrings, int32_t **owner)
{
  assert(owner);
  *owner = NULL;
  return arr_arrange(edges, rings, nrings, owner);
}
