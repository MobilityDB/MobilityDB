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
 * @brief Decompose a geometry into the edges a native implementation works on
 * @details An implementation that answers a question about a geometry without
 * calling GEOS works on the edges of that geometry: the segments and the
 * circular arcs its boundary is made of. The decomposition that produces them
 * lived inside the clipping engine, reachable only from there, so anything
 * else needing it had to build its own.
 */

/* C */
#include <limits.h>
#include <math.h>
/* PostgreSQL */
#include "postgres.h"
#include <utils/float.h>
#include <utils/memutils.h>  /* TopMemoryContext */
#include <pgtypes.h>
/* PostGIS */
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
/* MEOS */
#include "meos.h"
#include "meos_internal_geo.h"
#include "meos_tls.h"
#include "geo/geo_funcs.h"
#include "geo/postgis_funcs.h"
#include "geo/meos_transform.h"
#include "temporal/type_util.h"

/*****************************************************************************
 * Extract edges from a geometry that can be of type point, line, polygon or
 * collection of these
 *****************************************************************************/

/**
 * @brief Make room in an edge array for @p n more edges at once
 * @details The array doubles as it fills, each doubling copying every edge
 * already read, while a ring or a circular string knows how many edges it
 * yields before it emits any. The room they need is taken in one step, at
 * least doubling so that many small rings still grow the array geometrically,
 * and an array holding no edge yet takes fresh room rather than copying slots
 * nothing was written to. The room is read in the array's own element size,
 * so an array of any fixed-size element takes it the same way
 */
void
edge_array_reserve(MeosArray *edges, size_t n)
{
  size_t need = edges->count + n;
  if (need <= edges->capacity)
    return;
  size_t cap = Max(need, 2 * edges->capacity);
  if (edges->count == 0)
  {
    pfree(edges->elems);
    edges->elems = palloc(cap * edges->elem_size);
  }
  else
    edges->elems = repalloc(edges->elems, cap * edges->elem_size);
  edges->capacity = cap;
  return;
}

/* Add an edge to an edge array, copied as an edge rather than as bytes of
 * the array's element size (#meos_array_add) */
static inline void
edge_array_add(MeosArray *edges, const Edge *e)
{
  if (edges->count >= edges->capacity)
    edge_array_reserve(edges, 1);
  ((Edge *) edges->elems)[edges->count++] = *e;
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a ring
 */
static void
emit_ring_edges(const POINTARRAY *pa, MeosArray *edges, EdgeType etype)
{
  /* A ring of n points yields n - 1 edges */
  edge_array_reserve(edges, pa->npoints);
  for (int i = 0; i < (int) pa->npoints - 1; i++)
  {
    const POINT2D *a = getPoint2d_cp(pa, i);
    const POINT2D *b = getPoint2d_cp(pa, i + 1);
    Edge e;
    e.centred = false;
    e.from = e.to = NULL;
    e.cut = false;
    e.x1 = a->x; e.y1 = a->y;
    e.x2 = b->x; e.y2 = b->y;
    e.xmin = Min(e.x1, e.x2); e.xmax = Max(e.x1, e.x2);
    e.ymin = Min(e.y1, e.y2); e.ymax = Max(e.y1, e.y2);
    e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
    e.length = e.dx * e.dx + e.dy * e.dy;
    e.etype = etype;
    edge_set_tolerance(&e);
    edge_array_add(edges, &e);
  }
  return;
}

/**
 * @brief Add to an edge array the straight edge of a surface's boundary from
 * @p a to @p b, as #geom_extract_edges reads one from a ring
 * @details The edge is decided exactly by an arrangement
 * (#arrange_wound_surfaces), which reads no tolerance of it, so none is
 * read for it: every field it does not set is left zero
 */
void
edges_add_segment(MeosArray *edges, const POINT2D *a, const POINT2D *b)
{
  assert(edges); assert(a); assert(b);
  Edge e = {0};
  e.x1 = a->x; e.y1 = a->y;
  e.x2 = b->x; e.y2 = b->y;
  e.xmin = Min(e.x1, e.x2); e.xmax = Max(e.x1, e.x2);
  e.ymin = Min(e.y1, e.y2); e.ymax = Max(e.y1, e.y2);
  e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
  e.length = e.dx * e.dx + e.dy * e.dy;
  e.etype = EDGE_POLYSEG;
  edge_array_add(edges, &e);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edge obtained
 * from a point
 */
static void
extract_point(const LWPOINT *pt, MeosArray *edges)
{
  /* An empty point (e.g. a component of a multipoint or the boundary of a
   * closed trajectory) has no vertex to read; it contributes no edge. */
  if (! pt->point || pt->point->npoints < 1)
    return;
  const POINT2D *p = getPoint2d_cp(pt->point, 0);
  Edge e;
  e.centred = false;
  e.from = e.to = NULL;
  e.cut = false;
  e.x1 = e.x2 = e.xmin = e.xmax = p->x;
  e.y1 = e.y2 = e.ymin = e.ymax = p->y;
  e.dx = e.dy = e.length = 0;
  e.etype = EDGE_POINT;
  edge_set_tolerance(&e);
  edge_array_add(edges, &e);
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
  emit_ring_edges(line->points, edges, EDGE_LINESEG);
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

static inline void two_product(double a, double b, double *x, double *y);
static inline int grow_expansion(int elen, const double *e, double b,
  double *h);

/**
 * @brief Return the sign of the area a ring encloses, decided exactly
 * @details The shoelace sum of x_i y_{i+1} - x_{i+1} y_i over the edges of a
 * ring is twice the signed area it encloses. The sum taken in floating point
 * answers wherever it stands clear of the bound its rounding reaches: the
 * rounding of each product and of each addition is at most a unit in the last
 * place of the running magnitude, and twice n + 1 of them bound it generously.
 * Within that bound every product is split into the two doubles that hold it
 * exactly (#two_product) and the parts are summed into an expansion
 * (#grow_expansion), whose largest component carries the exact sign.
 */
int
ring_area_sign(const POINTARRAY *pa)
{
  uint32_t n = pa->npoints;
  double sum = 0.0, mag = 0.0;
  for (uint32_t i = 0; i + 1 < n; i++)
  {
    const POINT2D *a = getPoint2d_cp(pa, i);
    const POINT2D *b = getPoint2d_cp(pa, i + 1);
    double l = a->x * b->y, r = b->x * a->y;
    sum += l - r;
    mag += fabs(l) + fabs(r);
  }
  double bound = 2.0 * ((double) n + 1.0) * DBL_EPSILON * mag;
  if (sum > bound)
    return 1;
  if (sum < - bound)
    return -1;
  size_t cap = 4 * (size_t) n + 2;
  double *e = palloc(sizeof(double) * cap);
  double *h = palloc(sizeof(double) * cap);
  double *swap;
  int elen = 0;
  for (uint32_t i = 0; i + 1 < n; i++)
  {
    const POINT2D *a = getPoint2d_cp(pa, i);
    const POINT2D *b = getPoint2d_cp(pa, i + 1);
    double x, y;
    two_product(a->x, b->y, &x, &y);
    elen = grow_expansion(elen, e, x, h);
    swap = e; e = h; h = swap;
    elen = grow_expansion(elen, e, y, h);
    swap = e; e = h; h = swap;
    two_product(b->x, a->y, &x, &y);
    elen = grow_expansion(elen, e, - x, h);
    swap = e; e = h; h = swap;
    elen = grow_expansion(elen, e, - y, h);
    swap = e; e = h; h = swap;
  }
  int result = (elen == 0) ? 0 :
    ((e[elen - 1] > 0.0) ? 1 : ((e[elen - 1] < 0.0) ? -1 : 0));
  pfree(e); pfree(h);
  return result;
}

/**
 * @brief Return true if a ring encloses no area
 * @details A ring that runs out and back along itself, or whose vertices are
 * collinear, bounds no region: the point set it draws is its own linework.
 * Real survey data carries them, so this is not a synthetic case.
 * @note Whether the ring encloses area is a question on its input vertices, so
 * it is the exact sign of its shoelace sum (#ring_area_sign) and not that sum
 * read against a band: a band of any size reads a ring enclosing an area below
 * it as linework, and which rings those are changes with the scale of the
 * coordinates.
 */
static bool
ring_encloses_no_area(const POINTARRAY *pa)
{
  if (! pa || pa->npoints < 3)
    return true;
  return ring_area_sign(pa) == 0;
}

/**
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a polygon
 * @details A ring enclosing no area bounds no region, so what it contributes
 * is the point set its own linework traces, one dimension lower. An outer ring
 * of no area therefore yields line edges and the surface is not areal at all,
 * while such a hole removes nothing from the surface it sits in and yields
 * none: its linework already lies in that surface.
 */
static void
extract_poly(const LWPOLY *poly, MeosArray *edges)
{
  if (poly->nrings == 0)
    return;
  if (ring_encloses_no_area(poly->rings[0]))
  {
    emit_ring_edges(poly->rings[0], edges, EDGE_LINESEG);
    return;
  }
  for (int r = 0; r < (int) poly->nrings; r++)
  {
    if (r > 0 && ring_encloses_no_area(poly->rings[r]))
      continue;
    emit_ring_edges(poly->rings[r], edges, EDGE_POLYSEG);
  }
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
  emit_ring_edges(tri->points, edges,
    ring_encloses_no_area(tri->points) ? EDGE_LINESEG : EDGE_POLYSEG);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the edge obtained from
 * three consecutive points of a circular string
 * @details The three points are the start, an intermediate point, and the end
 * of the arc. Three collinear points degenerate to straight segments and are
 * emitted as line edges. The angles of the ends are read where @p angles is
 * true, and left zero otherwise
 */
static void
emit_arc_edge(const POINT2D *pa, const POINT2D *pb, const POINT2D *pc,
  MeosArray *edges, EdgeType line_etype, EdgeType arc_etype, bool angles)
{
  /* The circumcentre below is read from the SQUARES of the coordinates, and
   * those squares then cancel down to a centre lying among the points
   * themselves. On projected data a coordinate of 6e6 squares to 4e13, whose
   * last representable bit is 8e-3, so the cancellation leaves an error of
   * that order in a centre that sits a metre or two away -- larger than the
   * radius of a buffer join arc, which then reads as lying off the boundary
   * and is dropped. Measuring the three points FROM ONE OF THEMSELVES makes
   * every square the size of the arc rather than of the coordinates, and the
   * centre is carried back at the end: on a radius-1 arc at 6.1e6 that is the
   * difference between an error of 3e-3 and one of 1.3e-10. The same
   * cancellation governs the collinearity test that shares the determinant */
  double ox = pb->x, oy = pb->y;
  double ax = pa->x - ox, ay = pa->y - oy;
  double bx = 0.0, by = 0.0;
  double cx = pc->x - ox, cy = pc->y - oy;
  /* Twice the signed area of the triangle; zero when the points are collinear */
  double d = 2 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));

  /* A circular string returning to the point it starts from is a full circle,
   * and the three points of one are collinear, so the circumcentre below
   * cannot be read from them. The middle point is the one opposite, and the
   * two points split the circle into the two half circles the arcs of an edge
   * hold exactly. This is the shape #lwcircle_make gives a circle */
  if (pa->x == pc->x && pa->y == pc->y && (pa->x != pb->x || pa->y != pb->y))
  {
    double mx = (ax + bx) / 2, my = (ay + by) / 2;
    double radius = hypot(ax - mx, ay - my);
    double theta_a = angles ? atan2(ay - my, ax - mx) : 0.0;
    double theta_b = angles ? atan2(by - my, bx - mx) : 0.0;
    /* Carried back to where the points came from */
    mx += ox; my += oy;
    /* The points given, as the ends of an arc are */
    const double sx[2] = {pa->x, pb->x}, sy[2] = {pa->y, pb->y};
    const double ex[2] = {pb->x, pa->x}, ey[2] = {pb->y, pa->y};
    const double t0[2] = {theta_a, theta_b}, t1[2] = {theta_b, theta_a};
    for (int i = 0; i < 2; i++)
    {
      Edge e;
      e.centred = false;
      e.from = e.to = NULL;
      e.cut = false;
      e.cx = mx; e.cy = my; e.radius = radius;
      /* Both halves lie on the circle on the diameter between the two input
       * points, which an exact question about it reads */
      e.sx = pa->x; e.sy = pa->y; e.ex = pb->x; e.ey = pb->y;
      e.mx = e.my = 0.0;
      e.diameter = true;
      e.x1 = sx[i]; e.y1 = sy[i];
      e.x2 = ex[i]; e.y2 = ey[i];
      e.theta0 = t0[i]; e.theta1 = t1[i];
      /* Both halves are traversed the same way, so together they cover the
       * circle rather than the same half twice */
      e.ccw = true;
      e.dx = e.dy = e.length = 0;
      e.etype = arc_etype;
      arc_set_bbox(&e);
      edge_set_tolerance(&e);
      edge_array_add(edges, &e);
    }
    return;
  }

  /* The turn of (start, mid, end): the exact sign of the cross product of
   * A -> B and A -> C. The three points are input vertices, read on the
   * doubles the geometry holds, so it answers the same at every scale */
  int turn = cross_product_sign(pa->x, pa->y, pb->x, pb->y, pa->x, pa->y,
    pc->x, pc->y);

  /* Collinear points: emit straight line edges. A triangle that is not flat
   * but whose area above rounds to zero has a centre no double holds, and is
   * emitted as the segments it approaches */
  if (turn == 0 || d == 0.0)
  {
    /* The points given, as the ends of an arc are */
    const double px[3] = {pa->x, pb->x, pc->x};
    const double py[3] = {pa->y, pb->y, pc->y};
    for (int i = 0; i < 2; i++)
    {
      Edge e;
      e.centred = false;
      e.from = e.to = NULL;
      e.cut = false;
      e.x1 = px[i]; e.y1 = py[i];
      e.x2 = px[i + 1]; e.y2 = py[i + 1];
      e.xmin = Min(e.x1, e.x2); e.xmax = Max(e.x1, e.x2);
      e.ymin = Min(e.y1, e.y2); e.ymax = Max(e.y1, e.y2);
      e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
      e.length = e.dx * e.dx + e.dy * e.dy;
      e.etype = line_etype;
      edge_set_tolerance(&e);
      edge_array_add(edges, &e);
    }
    return;
  }

  double a2 = ax * ax + ay * ay;
  double b2 = bx * bx + by * by;
  double c2 = cx * cx + cy * cy;
  Edge e;
  e.centred = false;
  e.from = e.to = NULL;
  e.cut = false;
  /* Circumcenter of the three points */
  e.cx = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
  e.cy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;
  e.radius = hypot(ax - e.cx, ay - e.cy);
  /* Carried back to where the points came from */
  e.cx += ox; e.cy += oy;
  /* The three input points fix the circle, and an exact question about it
   * reads them rather than the centre constructed from them */
  e.sx = pa->x; e.sy = pa->y; e.mx = pb->x; e.my = pb->y;
  e.ex = pc->x; e.ey = pc->y;
  e.diameter = false;
  /* The ends are the points given: carried back from the middle point, an
   * end lands a rounding step from itself wherever its coordinates and the
   * middle's lie in different binades, and the next edge of the string, which
   * starts where this one ends, then does not meet it */
  e.x1 = pa->x; e.y1 = pa->y;
  e.x2 = pc->x; e.y2 = pc->y;
  e.theta0 = angles ? atan2(e.y1 - e.cy, e.x1 - e.cx) : 0.0;
  e.theta1 = angles ? atan2(e.y2 - e.cy, e.x2 - e.cx) : 0.0;
  /* Traversal orientation: the turn of (start, mid, end) */
  e.ccw = (turn > 0);
  e.dx = e.dy = e.length = 0;
  e.etype = arc_etype;
  arc_set_bbox(&e);
  edge_set_tolerance(&e);
  edge_array_add(edges, &e);
  return;
}

/**
 * @brief Add to an edge array the arc of a surface's boundary through @p a,
 * @p m and @p b, as #geom_extract_edges reads one from a circular string:
 * three collinear points are the straight edges they lie on. The angles of
 * its ends, which an arrangement (#arrange_wound_surfaces) does not read, are
 * left zero
 */
void
edges_add_arc(MeosArray *edges, const POINT2D *a, const POINT2D *m,
  const POINT2D *b)
{
  assert(edges); assert(a); assert(m); assert(b);
  emit_arc_edge(a, m, b, edges, EDGE_POLYSEG, EDGE_POLYARC, false);
  return;
}

/**
 * @brief Add to an edge array the whole circle of a centre and a radius, the
 * boundary of the disc they bound, turning counterclockwise where @p ccw is
 * true
 * @details The circle is the one the three doubles fix exactly, and every
 * exact question about it reads them (#Edge, member centred): no three
 * points on it are doubles in general, so it is not handed to
 * #edges_add_arc. Its ends are the point of the circle furthest along -x,
 * rounded once, which an arrangement reads as the start of its positions
 * along the circle and never as a point on it (#arrange_wound_surfaces). An
 * arrangement decides exactly, so the edge carries no tolerance
 * @param[in,out] edges Edge array
 * @param[in] cx,cy Centre
 * @param[in] radius Radius, positive
 * @param[in] ccw Whether the circle turns counterclockwise
 */
void
edges_add_circle(MeosArray *edges, double cx, double cy, double radius,
  bool ccw)
{
  assert(edges); assert(radius > 0.0);
  Edge e;
  memset(&e, 0, sizeof(Edge));
  e.etype = EDGE_POLYARC;
  e.cx = cx; e.cy = cy; e.radius = radius;
  e.centred = true;
  e.ccw = ccw;
  e.x1 = e.x2 = cx - radius;
  e.y1 = e.y2 = cy;
  e.theta0 = e.theta1 = M_PI;
  e.xmin = cx - radius; e.xmax = cx + radius;
  e.ymin = cy - radius; e.ymax = cy + radius;
  edge_array_add(edges, &e);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the arc edges obtained
 * from a circular string, emitting them with the given line/arc edge types
 * @details Straight components (collinear point triples) are emitted with
 * @p line_etype and genuine arcs with @p arc_etype. A standalone circular
 * string uses the 1D types (#EDGE_LINESEG / #EDGE_LINEARC); a circular string that
 * bounds a curve polygon ring uses the region types (#EDGE_POLYSEG /
 * #EDGE_POLYARC)
 */
static void
emit_circstring_edges(const LWCIRCSTRING *circ, MeosArray *edges,
  EdgeType line_etype, EdgeType arc_etype)
{
  const POINTARRAY *pa = circ->points;
  int np = (int) pa->npoints;
  /* Each arc of the string yields at most two edges, so no more than it has
   * points */
  edge_array_reserve(edges, pa->npoints);
  for (int i = 0; i + 2 < np; i += 2)
  {
    emit_arc_edge(getPoint2d_cp(pa, i), getPoint2d_cp(pa, i + 1),
      getPoint2d_cp(pa, i + 2), edges, line_etype, arc_etype, true);
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
  emit_circstring_edges(circ, edges, EDGE_LINESEG, EDGE_LINEARC);
  return;
}

/**
 * @brief Add to the dynamic array in the last argument the region-boundary
 * edges obtained from a ring of a curve polygon
 * @details A ring is a line string, a circular string, or a compound curve
 * chaining both. Every edge is emitted with polygon (region) semantics
 * (#EDGE_POLYSEG / #EDGE_POLYARC) so that the even-odd containment test in
 * #point_in_polygon treats it as a boundary rather than a 1D feature
 */
static void
extract_curvepoly_ring(const LWGEOM *ring, MeosArray *edges)
{
  switch (ring->type)
  {
    case LINETYPE:
      emit_ring_edges(((const LWLINE *) ring)->points, edges, EDGE_POLYSEG);
      break;

    case CIRCSTRINGTYPE:
      emit_circstring_edges((const LWCIRCSTRING *) ring, edges, EDGE_POLYSEG,
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
  /* Skip empty (sub-)geometries: an empty component contributes no edges, and
   * extracting one would read vertex 0 of an empty point array. This covers
   * empty parts nested inside a multi-geometry or collection too. */
  if (! geom || lwgeom_is_empty(geom))
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
     * (collection of line/circular/compound curves), a multisurface
     * (collection of polygons/curve polygons), a TIN (collection of triangles)
     * and a polyhedral surface (collection of polygonal faces) all share the
     * collection memory layout, so their components are extracted the same way
     * as a collection */
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
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
MeosArray *
geom_extract_edges(const LWGEOM *geom)
{
  MeosArray *edges = meos_array_create(sizeof(Edge));
  geom_extract_edges_iter(geom, edges);
  return edges;
}

/**
 * @brief Return how far the native implementations cover a geometry
 * @details Mirrors the type dispatch of #geom_extract_edges_iter, which every
 * native implementation of a PostGIS function reads its geometry through, so
 * the answer holds for all of them: the clip engine, the DE-9IM matrix, the
 * convex hull, the oriented envelope and the buffer alike.
 *
 * THE THIRD ANSWER IS THE POINT. A geometry the kernels do not cover and a
 * geometry of a type this build has never heard of are different facts, and a
 * caller can act on the second only if it is told them apart: liblwgeom
 * numbers its types 1 to 15 today and PostGIS adds to that set, so a build
 * meets a type its own enumeration does not name the moment it is linked
 * against a newer liblwgeom. Answering "not covered" there states something
 * about a type nothing here has seen.
 *
 * @return 1 where the kernels answer for the geometry, 0 where a type they
 * know is one they do not answer for, and -1 where the type is not one this
 * build enumerates at all, which is reported as an unsupported type
 * @note Neither 0 nor -1 means unrelated: both are the absence of an answer
 * rather than a negative one. A caller either answers such a geometry another
 * way or reports that it is not supported. The two differ in who reports it:
 * a 0 is the caller's to report, naming the operation it has no answer for,
 * while a -1 is reported here, because a type this build does not enumerate
 * is a fact about the geometry rather than about the operation
 */
int
geom_meos_coverage(const LWGEOM *geom)
{
  if (! geom)
    return -1;
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
      return 1;
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    case COLLECTIONTYPE:
    {
      /* A multicurve/multisurface/TIN/polyhedral surface is supported when
       * every component is: its components are line/circular/compound curves,
       * polygons/curve polygons and triangles, each validated by the recursive
       * call */
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
      {
        /* A member carries the weaker answer outward: a member of an unknown
         * type makes the whole collection unknown, and an uncovered one makes
         * it uncovered */
        int member = geom_meos_coverage(col->geoms[i]);
        if (member != 1)
          return member;
      }
      return 1;
    }
    case CURVEPOLYTYPE:
    {
      /* Mirrors the ring dispatch of #extract_curvepoly_ring: a ring must be a
       * line string, a circular string, or a compound curve of those */
      const LWCURVEPOLY *cp = (const LWCURVEPOLY *) geom;
      for (uint32_t r = 0; r < cp->nrings; r++)
      {
        uint8_t rt = cp->rings[r]->type;
        /* A ring of a type the ring dispatch does not take is a type this
         * build knows, so the geometry is uncovered rather than unknown */
        if (rt != LINETYPE && rt != CIRCSTRINGTYPE && rt != COMPOUNDTYPE)
          return 0;
        if (rt == COMPOUNDTYPE)
        {
          int ring = geom_meos_coverage(cp->rings[r]);
          if (ring != 1)
            return ring;
        }
      }
      return 1;
    }
    /* Every type liblwgeom numbers has an arm above, so this one is reached
     * only by a type added after this code. Saying anything about it other
     * than "unknown" would be inventing an answer, so it is reported here
     * exactly as #geom_extract_edges_iter reports it one call deeper */
    default:
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "Unsupported geometry type");
      return -1;
  }
}

/**
 * @brief Build an R-tree from edges
 */
RTree *
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

/*****************************************************************************
 * The sign of a cross product where the filter cannot tell
 *****************************************************************************/

/**
 * @brief Split the difference of two doubles into its rounded value and the
 * error of that rounding, which together are the difference exactly
 */
static inline void
two_diff(double a, double b, double *x, double *y)
{
  *x = a - b;
  double bv = a - *x;
  double av = *x + bv;
  *y = (a - av) + (bv - b);
}

/**
 * @brief Split the product of two doubles into its rounded value and the
 * error of that rounding, which together are the product exactly
 * @note Exact where the product neither overflows nor underflows
 */
static inline void
two_product(double a, double b, double *x, double *y)
{
  *x = a * b;
  *y = fma(a, b, - *x);
}

/**
 * @brief Split the square of a double into its rounded value and the error of
 * that rounding, which together are the square exactly
 * @details The square of #two_product computed with no call: Dekker's product
 * on the halves of 26 bits that splitting by 2^27 + 1 gives, each of their
 * products exact
 * @note Exact where the square neither overflows nor underflows and the
 * value is at most 2^996 in magnitude
 */
static inline void
two_square(double a, double *x, double *y)
{
  double t = 134217729.0 * a;
  double ah = t - (t - a);
  double al = a - ah;
  *x = a * a;
  *y = ((ah * ah - *x) + 2.0 * ah * al) + al * al;
}

/**
 * @brief Add a double to an expansion, a sum of doubles that do not overlap,
 * held in increasing order of magnitude, and return its new length
 * @details The sum is exact, and a zero component is dropped, so the last
 * component carries the sign of the whole expansion
 */
static inline int
grow_expansion(int elen, const double *e, double b, double *h)
{
  double q = b;
  int hlen = 0;
  for (int i = 0; i < elen; i++)
  {
    double sum = q + e[i];
    double bv = sum - q;
    double av = sum - bv;
    double err = (q - av) + (e[i] - bv);
    q = sum;
    if (err != 0.0)
      h[hlen++] = err;
  }
  if (q != 0.0 || hlen == 0)
    h[hlen++] = q;
  return hlen;
}

/**
 * @brief Add the cross product of the vectors B - A and D - C, exactly, into
 * an expansion held in one of two buffers, and return its length
 * @details Each difference is its rounded value plus the error of the
 * rounding, and each product of two terms is its rounded value plus its
 * error, so the sixteen terms add up to the cross product exactly; a term
 * with a zero factor adds nothing and is skipped. The last component carries
 * the sign of the whole
 * @param[in] buf1,buf2 Room for 32 components each
 * @param[out] result The buffer holding the expansion
 * @return Number of components, 0 exactly where the vectors are parallel
 */
static int
cross_product_expansion(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy, double *buf1, double *buf2,
  const double **result)
{
  double d[4], t[4];
  two_diff(bx, ax, &d[0], &t[0]);
  two_diff(dy, cy, &d[1], &t[1]);
  two_diff(by, ay, &d[2], &t[2]);
  two_diff(dx, cx, &d[3], &t[3]);
  /* (d0 + t0) (d1 + t1) - (d2 + t2) (d3 + t3), term by term */
  const double lf[2] = {d[0], t[0]}, lg[2] = {d[1], t[1]};
  const double rf[2] = {d[2], t[2]}, rg[2] = {d[3], t[3]};
  double *e = buf1, *h = buf2, *swap;
  int elen = 0;
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 2; j++)
    {
      double x, y;
      if (lf[i] != 0.0 && lg[j] != 0.0)
      {
        two_product(lf[i], lg[j], &x, &y);
        elen = grow_expansion(elen, e, x, h);
        swap = e; e = h; h = swap;
        elen = grow_expansion(elen, e, y, h);
        swap = e; e = h; h = swap;
      }
      if (rf[i] != 0.0 && rg[j] != 0.0)
      {
        two_product(rf[i], rg[j], &x, &y);
        elen = grow_expansion(elen, e, - x, h);
        swap = e; e = h; h = swap;
        elen = grow_expansion(elen, e, - y, h);
        swap = e; e = h; h = swap;
      }
    }
  *result = e;
  return elen;
}

/**
 * @brief Return the sign of the cross product of the vectors B - A and D - C,
 * decided exactly
 * @details Each difference is its rounded value plus the error of the
 * rounding. Where the four differences are exact, which the calls the filter
 * of #cross_product_sign cannot sign mostly are, the cross product is a
 * difference of two products of doubles: rounding is monotone, so two rounded
 * products that differ order as the exact ones do, and two that are equal
 * leave the sign to the difference of their rounding errors, which are exact.
 * Otherwise each product of two terms is its rounded value plus its error,
 * and the sixteen terms are added into an expansion whose last component
 * carries the sign; a term with a zero factor adds nothing and is skipped.
 * #cross_product_sign calls it where its filter cannot tell, which keeps this
 * out of every call the filter decides
 * @note Exact where no product of coordinate differences overflows or
 * underflows
 * @return 1 or -1 for the two turns, 0 exactly where the vectors are parallel
 */
int
cross_product_sign_exact(double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy)
{
  double d[4], t[4];
  two_diff(bx, ax, &d[0], &t[0]);
  two_diff(dy, cy, &d[1], &t[1]);
  two_diff(by, ay, &d[2], &t[2]);
  two_diff(dx, cx, &d[3], &t[3]);
  if (t[0] == 0.0 && t[1] == 0.0 && t[2] == 0.0 && t[3] == 0.0)
  {
    double l = d[0] * d[1], r = d[2] * d[3];
    if (l != r)
      return (l > r) ? 1 : -1;
    double err = fma(d[0], d[1], - l) - fma(d[2], d[3], - r);
    return (err > 0.0) ? 1 : ((err < 0.0) ? -1 : 0);
  }
  double buf1[32], buf2[32];
  const double *e;
  int elen = cross_product_expansion(ax, ay, bx, by, cx, cy, dx, dy, buf1,
    buf2, &e);
  /* Every term with a zero factor: the cross product is zero */
  if (elen == 0)
    return 0;
  double top = e[elen - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the cross product of the vectors B - A and D - C, computed
 * exactly and rounded once
 * @details The cross product of rounded differences carries an error of the
 * order of a rounding of each of its two products, so where the vectors are
 * nearly parallel and the products nearly cancel, it keeps none of its
 * relative precision. The exact expansion of #cross_product_sign_exact,
 * summed from its smallest component up, keeps it, however small the cross
 * product is against the coordinates
 * @note Exact where no product of coordinate differences overflows or
 * underflows
 * @return The cross product, 0 exactly where the vectors are parallel
 */
double
cross_product_exact(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  double buf1[32], buf2[32];
  const double *e;
  int elen = cross_product_expansion(ax, ay, bx, by, cx, cy, dx, dy, buf1,
    buf2, &e);
  double sum = 0.0;
  for (int i = 0; i < elen; i++)
    sum += e[i];
  return sum;
}

/*****************************************************************************
 * The circle of an arc, decided exactly on its input points
 *****************************************************************************/

/**
 * @brief Split the sum of two doubles into its rounded value and the error of
 * that rounding, which together are the sum exactly
 */
static inline void
two_sum(double a, double b, double *x, double *y)
{
  *x = a + b;
  double bv = *x - a;
  double av = *x - bv;
  *y = (a - av) + (b - bv);
}

/**
 * @brief Set the product of two pairs of doubles as a pair, within
 * 2^-104 of the magnitude of the product of their larger parts and of the
 * cross terms
 */
static pg_attribute_always_inline void
pair_mul(double ah, double al, double bh, double bl, double *rh, double *rl)
{
  double p, e;
  two_product(ah, bh, &p, &e);
  e += ah * bl + al * bh;
  *rh = p + e;
  *rl = e - (*rh - p);
}

/**
 * @brief Set the sum of two pairs of doubles as a pair, within 2^-104 of the
 * sum of the magnitudes of the operands
 */
static pg_attribute_always_inline void
pair_add(double ah, double al, double bh, double bl, double *rh, double *rl)
{
  double s, e;
  two_sum(ah, bh, &s, &e);
  e += al + bl;
  *rh = s + e;
  *rl = e - (*rh - s);
}

/**
 * @brief Return the sign of `q - m^2 l` at a midpoint `m` given as a pair,
 * where its evaluation in pairs of doubles carries it, 0 otherwise
 * @details `q` and `l` are pairs within @p eq and @p el of the values they
 * stand for; the products and the difference each err by 2^-104 of the
 * magnitudes they combine, which `2^-96` covers, and a value below 2^-960,
 * whose products could underflow, is not told
 * @param[in] q,eq The value and a bound on its error
 * @param[in] l,el The factor of the squared midpoint and a bound on its error
 * @param[in] mh,ml The midpoint as a pair
 */
static pg_attribute_always_inline int
pair_mid_sign(const double *q, double eq, const double *l, double el,
  double mh, double ml)
{
  double m2h, m2l, ph, pl, vh, vl;
  pair_mul(mh, ml, mh, ml, &m2h, &m2l);
  pair_mul(m2h, m2l, l[0], l[1], &ph, &pl);
  pair_add(q[0], q[1], - ph, - pl, &vh, &vl);
  double m2 = fabs(m2h), aq = fabs(q[0]), al = fabs(l[0]);
  double bound = (eq + m2 * el + 0x1p-96 * (aq + m2 * al) + 0x1p-960) *
    (1.0 + 16.0 * DBL_EPSILON);
  if (! isfinite(vh) || ! isfinite(bound))
    return 0;
  return (vh > bound) ? 1 : ((vh < - bound) ? -1 : 0);
}

/**
 * @brief Multiply an expansion by a double exactly, and return the length of
 * the product
 * @details Each component times the double is its rounded value plus its
 * error (#two_product), and the terms are carried through in increasing order
 * of magnitude, so the product is again an expansion, its zero components
 * dropped and its last component carrying its sign
 * @param[out] h Room for 2 * elen components
 */
static int
scale_expansion(int elen, const double *e, double b, double *h)
{
  int hlen = 0;
  double q, hh;
  two_product(e[0], b, &q, &hh);
  if (hh != 0.0)
    h[hlen++] = hh;
  for (int i = 1; i < elen; i++)
  {
    double p1, p0, sum;
    two_product(e[i], b, &p1, &p0);
    two_sum(q, p0, &sum, &hh);
    if (hh != 0.0)
      h[hlen++] = hh;
    two_sum(p1, sum, &q, &hh);
    if (hh != 0.0)
      h[hlen++] = hh;
  }
  if (q != 0.0 || hlen == 0)
    h[hlen++] = q;
  return hlen;
}

/**
 * @brief Add two expansions exactly, and return the length of the sum
 * @details Each component of the second is added into the first
 * (#grow_expansion); the sum is left in @p h, which may be @p e itself
 * @param[out] h Room for elen + flen components
 * @param tmp Scratch of the same room
 */
static int
expansion_sum(int elen, const double *e, int flen, const double *f,
  double *h, double *tmp)
{
  double *cur = h, *nxt = tmp;
  if (cur != e)
    memcpy(cur, e, (size_t) elen * sizeof(double));
  int len = elen;
  for (int i = 0; i < flen; i++)
  {
    len = grow_expansion(len, cur, f[i], nxt);
    double *swap = cur; cur = nxt; nxt = swap;
  }
  if (cur != h)
    memcpy(h, cur, (size_t) len * sizeof(double));
  return len;
}

/**
 * @brief Multiply two expansions exactly, and return the length of the
 * product
 * @param[out] h Room for 2 * elen * flen components
 * @param tmp,part Scratch of that room and of 2 * elen components
 */
static int
expansion_product(int elen, const double *e, int flen, const double *f,
  double *h, double *tmp, double *part)
{
  int hlen = 0;
  for (int j = 0; j < flen; j++)
  {
    int plen = scale_expansion(elen, e, f[j], part);
    if (hlen == 0)
    {
      memcpy(h, part, (size_t) plen * sizeof(double));
      hlen = plen;
    }
    else
      hlen = expansion_sum(hlen, h, plen, part, h, tmp);
  }
  return hlen;
}

/*****************************************************************************
 * The sign of a polynomial in exact sums of doubles
 *****************************************************************************/

/**
 * @brief Compress an expansion into one of the same value with fewer
 * components, and return its length
 * @details The components are summed from the largest down and then from the
 * smallest up, each rounding error kept (Shewchuk's Compress), so the result
 * is again an expansion whose last component carries the sign of the whole
 * @param[out] h Room for elen components, distinct from @p e
 */
static int
compress_expansion(int elen, const double *e, double *h)
{
  int bottom = elen - 1;
  double q = e[bottom];
  for (int i = elen - 2; i >= 0; i--)
  {
    double qnew, err;
    two_sum(q, e[i], &qnew, &err);
    if (err != 0.0)
    {
      h[bottom--] = qnew;
      q = err;
    }
    else
      q = qnew;
  }
  h[bottom] = q;
  int top = 0;
  for (int i = bottom + 1; i < elen; i++)
  {
    double qnew, err;
    two_sum(h[i], q, &qnew, &err);
    if (err != 0.0)
      h[top++] = err;
    q = qnew;
  }
  h[top++] = q;
  return top;
}

/**
 * @brief Set an exact sum to the sum of doubles
 * @details The sum is held exactly as an expansion (#grow_expansion), with the
 * double nearest to it and a bound on the error of that double, which the
 * filter of #polynomial_sign_exact reads
 * @param[out] sum Exact sum
 * @param[in] terms Doubles to add, a subtraction given as a negated term
 * @param[in] nterms Number of terms, at most EXACT_SUM_MAXTERMS
 */
void
exact_sum_set(ExactSum *sum, const double *terms, int nterms)
{
  assert(nterms > 0 && nterms <= EXACT_SUM_MAXTERMS);
  int len = 0;
  if (nterms == 2)
  {
    /* The second step of #grow_expansion on the first term, with no copy:
     * the error of the sum where it is not zero, then the sum */
    double q = terms[1], e0 = terms[0];
    double s = q + e0, bv = s - q, av = s - bv;
    double err = (q - av) + (e0 - bv);
    if (err != 0.0)
      sum->e[len++] = err;
    if (s != 0.0 || len == 0)
      sum->e[len++] = s;
  }
  else
  {
    double buf[EXACT_SUM_MAXTERMS + 1];
    for (int i = 0; i < nterms; i++)
    {
      len = grow_expansion(len, sum->e, terms[i], buf);
      memcpy(sum->e, buf, (size_t) len * sizeof(double));
    }
  }
  sum->n = len;
  double approx = 0.0, mag = 0.0;
  for (int i = 0; i < len; i++)
  {
    approx += sum->e[i];
    mag += fabs(sum->e[i]);
  }
  sum->approx = approx;
  sum->err = len * DBL_EPSILON * mag;
  return;
}

/**
 * @brief Return the sign of a polynomial in exact sums where its evaluation
 * in pairs of doubles carries it, and 0 where it cannot tell
 * @details The second filter of #polynomial_sign_exact. Each exact sum is read
 * as a pair, its largest component and the rounded sum of the others, within
 * 2 u of those others, u being DBL_EPSILON / 2. Each term is a pair, its
 * coefficient times its sums: the product of two pairs is the exact product of
 * their larger parts (#two_product) plus the rounded cross terms, within
 * 8.1 u^2 of the product of the larger parts, besides the errors the factors
 * carry, which the bound carries through the product. The terms are added
 * into a pair (#two_sum), each rounding of the smaller part adding u of it.
 * Where the pair stands clear of twice the bound, every bound inflated for its
 * own rounding, its sign is the polynomial's. A product of the larger parts
 * outside 2^-900 to 2^900, which could lose its error to underflow, declines.
 * Mirrors #incircle_sign_filter
 * @param[in] sums Exact sums the terms refer to
 * @param[in] terms,nterms Terms of the polynomial and their number
 */
static int
polynomial_sign_pairs(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  const double u = DBL_EPSILON / 2.0, inflate = 1.0 + 16.0 * DBL_EPSILON;
  double H = 0.0, L = 0.0, E = 0.0;
  for (int i = 0; i < nterms; i++)
  {
    /* The term, starting from its coefficient, which is exact */
    double h = terms[i].coef, l = 0.0, e = 0.0;
    for (int k = 0; k < terms[i].deg; k++)
    {
      const ExactSum *s = &sums[terms[i].sum[k]];
      /* The sum as a pair */
      double rest = 0.0, restmag = 0.0;
      for (int j = 0; j < s->n - 1; j++)
      {
        rest += s->e[j];
        restmag += fabs(s->e[j]);
      }
      double sh, sl;
      two_sum(s->e[s->n - 1], rest, &sh, &sl);
      double se = 2.0 * u * restmag * inflate;
      /* The product of the two pairs */
      double p, pe;
      two_product(h, sh, &p, &pe);
      double ap = fabs(p);
      if (! isfinite(p) || (ap != 0.0 && (ap < 0x1p-900 || ap > 0x1p900)))
        return 0;
      double c = (pe + h * sl) + l * sh;
      double ne = (e * (fabs(sh) + fabs(sl)) + (fabs(h) + fabs(l)) * se +
        e * se + 8.1 * u * u * fabs(h) * fabs(sh)) * inflate;
      two_sum(p, c, &h, &l);
      e = ne;
    }
    /* The term added to the total */
    double x;
    two_sum(H, h, &H, &x);
    double l1 = L + x;
    L = l1 + l;
    E = (E + e + u * (fabs(l1) + fabs(L))) * inflate;
  }
  double v = H + L;
  double bound = 2.0 * (E + 8.0 * DBL_MIN * nterms * POLY_TERM_MAXDEG) *
    inflate;
  if (! isfinite(v) || ! isfinite(bound))
    return 0;
  if (v > bound)
    return 1;
  if (v < - bound)
    return -1;
  return 0;
}

/** Number of doubles of the arena in which #polynomial_sign_stack forms each
 * term, and of each of its two buffers of the total */
#define POLY_STACK_ARENA 2048
#define POLY_STACK_TOTAL 1024

/**
 * @brief Set in @p sign the exact sign of a polynomial in exact sums and
 * return true, evaluated in buffers on the stack, or return false where an
 * expansion outgrows them
 * @details The evaluation of #polynomial_sign_exact: each term is formed in an
 * arena that is reset for the next one, and the total alternates between two
 * buffers, so that no expansion is allocated
 */
static bool
polynomial_sign_stack(const ExactSum *sums, const PolyTerm *terms,
  int nterms, int *sign)
{
  double arena[POLY_STACK_ARENA], total[2][POLY_STACK_TOTAL];
  int tlen = 0, cur = 0;
  for (int i = 0; i < nterms; i++)
  {
    /* The term, starting from its coefficient */
    double *prod = arena;
    int plen = 1, top = 1;
    prod[0] = terms[i].coef;
    for (int k = 0; k < terms[i].deg; k++)
    {
      const ExactSum *s = &sums[terms[i].sum[k]];
      int cap = 2 * plen * s->n;
      if (top + 2 * cap + 2 * plen > POLY_STACK_ARENA)
        return false;
      double *h = arena + top, *tmp = h + cap, *part = tmp + cap;
      top += 2 * cap + 2 * plen;
      int hlen = expansion_product(plen, prod, s->n, s->e, h, tmp, part);
      prod = tmp;
      plen = compress_expansion(hlen, h, prod);
    }
    /* Add the term to the total */
    if (tlen + plen > POLY_STACK_TOTAL || top + tlen + plen > POLY_STACK_ARENA)
      return false;
    if (tlen == 0)
    {
      memcpy(total[cur], prod, sizeof(double) * plen);
      tlen = plen;
    }
    else
    {
      double *h = arena + top;
      int hlen = expansion_sum(tlen, total[cur], plen, prod, h,
        total[1 - cur]);
      cur = 1 - cur;
      tlen = compress_expansion(hlen, h, total[cur]);
    }
  }
  double last = tlen ? total[cur][tlen - 1] : 0.0;
  *sign = (last > 0.0) ? 1 : ((last < 0.0) ? -1 : 0);
  return true;
}

/**
 * @brief Return the sign of a polynomial in exact sums where its filters
 * carry it, and 0 where they cannot tell
 * @details The two filters of #polynomial_sign_exact, in doubles and else in
 * pairs of doubles (#polynomial_sign_pairs). The polynomial is
 * evaluated in doubles together with a bound on the error of every operation,
 * each rounding contributing at most DBL_EPSILON times its result; where the
 * value exceeds the bound its sign is the exact one. Neither filter answers a
 * value outside the doubles, so a caller holding an exact route of its own
 * reads it where this answers 0
 * @param[in] sums Exact sums the terms refer to
 * @param[in] terms,nterms Terms of the polynomial and their number
 * @return -1 or 1, or 0 where the filters cannot tell
 */
static inline int
polynomial_sign_filter(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  /* The value in doubles and a bound on its error */
  double val = 0.0, err = 0.0;
  for (int i = 0; i < nterms; i++)
  {
    double v = terms[i].coef, e = 0.0;
    for (int k = 0; k < terms[i].deg; k++)
    {
      const ExactSum *s = &sums[terms[i].sum[k]];
      double nv = v * s->approx;
      e = fabs(v) * s->err + fabs(s->approx) * e + e * s->err +
        DBL_EPSILON * fabs(nv);
      v = nv;
    }
    double nval = val + v;
    err += e + DBL_EPSILON * fabs(nval);
    val = nval;
  }
  /* The bound itself is computed in doubles, hence the margin; the absolute
   * term covers a product that underflows */
  double bound = err * (1.0 + 1e-6) + DBL_MIN;
  if (isfinite(val) && isfinite(bound) && fabs(val) > bound)
    return (val > 0.0) ? 1 : -1;
  return polynomial_sign_pairs(sums, terms, nterms);
}

/**
 * @brief Return the sign of a polynomial in exact sums, computed exactly
 * @details The polynomial is first evaluated in doubles together with a bound
 * on the error of every operation, each rounding contributing at most
 * DBL_EPSILON times its result; where the value exceeds the bound its sign is
 * the exact one. Otherwise every term is formed as an expansion, the product
 * of its coefficient and of its exact sums (#expansion_product), the terms are
 * added (#expansion_sum), and the expansions are compressed as they grow; the
 * last component of the total carries the exact sign. Between the two, the
 * evaluation in pairs of doubles of #polynomial_sign_pairs is read against
 * its own bound
 * @param[in] sums Exact sums the terms refer to
 * @param[in] terms,nterms Terms of the polynomial and their number
 * @return -1, 0, or 1
 * @note Exact where no product of the components overflows or underflows
 */
int
polynomial_sign_exact(const ExactSum *sums, const PolyTerm *terms,
  int nterms)
{
  int sign = polynomial_sign_filter(sums, terms, nterms);
  if (sign != 0)
    return sign;
  if (polynomial_sign_stack(sums, terms, nterms, &sign))
    return sign;

  /* Exact evaluation */
  int tlen = 0;
  double *total = NULL;
  for (int i = 0; i < nterms; i++)
  {
    /* The term, starting from its coefficient */
    int plen = 1;
    double *prod = palloc(sizeof(double));
    prod[0] = terms[i].coef;
    for (int k = 0; k < terms[i].deg; k++)
    {
      const ExactSum *s = &sums[terms[i].sum[k]];
      int cap = 2 * plen * s->n;
      double *h = palloc(sizeof(double) * cap);
      double *tmp = palloc(sizeof(double) * cap);
      double *part = palloc(sizeof(double) * 2 * plen);
      int hlen = expansion_product(plen, prod, s->n, s->e, h, tmp, part);
      pfree(prod); pfree(part);
      prod = tmp;
      plen = compress_expansion(hlen, h, prod);
      pfree(h);
    }
    /* Add the term to the total */
    if (! total)
    {
      total = prod;
      tlen = plen;
    }
    else
    {
      int cap = tlen + plen;
      double *h = palloc(sizeof(double) * cap);
      double *tmp = palloc(sizeof(double) * cap);
      int hlen = expansion_sum(tlen, total, plen, prod, h, tmp);
      pfree(total); pfree(prod);
      total = tmp;
      tlen = compress_expansion(hlen, h, total);
      pfree(h);
    }
  }
  double last = total ? total[tlen - 1] : 0.0;
  if (total)
    pfree(total);
  return (last > 0.0) ? 1 : ((last < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the dot product of two vectors, computed exactly
 * @details Each product of two coordinates is its rounded value plus its
 * error (#two_product), and the six terms are added into an expansion whose
 * last component carries the sign of the whole. #dot_product_sign calls it
 * where its filter cannot tell
 * @note Exact where no product of coordinates overflows or underflows
 * @return 1 or -1, 0 exactly where the vectors are orthogonal
 */
int
dot_product_sign_exact(const POINT3D *p, const POINT3D *q)
{
  const double pc[3] = {p->x, p->y, p->z}, qc[3] = {q->x, q->y, q->z};
  double buf1[6], buf2[6], *cur = buf1, *nxt = buf2;
  int len = 0;
  for (int k = 0; k < 3; k++)
  {
    double x, y;
    two_product(pc[k], qc[k], &x, &y);
    if (y != 0.0)
    {
      len = grow_expansion(len, cur, y, nxt);
      double *swap = cur; cur = nxt; nxt = swap;
    }
    if (x != 0.0)
    {
      len = grow_expansion(len, cur, x, nxt);
      double *swap = cur; cur = nxt; nxt = swap;
    }
  }
  if (len == 0)
    return 0;
  double top = cur[len - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return 2^k, for k from -1022 to 1023, the normal doubles that are
 * powers of two
 * @details Multiplying by it is exact wherever the product neither overflows
 * nor underflows, as #ldexp is, and costs a multiplication
 */
static inline double
pow2_double(int k)
{
  assert(k >= -1022 && k <= 1023);
  uint64_t bits = (uint64_t) (k + 1023) << 52;
  double result;
  memcpy(&result, &bits, sizeof(result));
  return result;
}

/**
 * @brief Set the products whose sum is the squared distance between two
 * points less the square of a sum of up to four doubles, each as two factors
 * and an exact multiplier of 1, -1, 2 or -2, and return their number
 * @details The squared distance is the sum over the coordinates of
 * `(d + e)^2 = d*d + 2*d*e + e*e`, where `d + e` is a coordinate difference
 * held exactly as its rounded value and the error of that rounding, and the
 * square of the sum is the sum of the squares of its terms and of twice the
 * product of each pair of them
 * @param[in] d,e Coordinate differences and the errors of their rounding
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] t,nt Terms of the sum and their number, at most 4
 * @param[out] left,right,factor Room for 19 products each
 */
static pg_attribute_always_inline int
point_distance_square_terms(const double *d, const double *e, int ndims,
  const double *t, int nt, double *left, double *right, double *factor)
{
  assert(nt >= 1 && nt <= 4);
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = d[i]; right[n] = d[i]; factor[n++] = 1.0;
    left[n] = d[i]; right[n] = e[i]; factor[n++] = 2.0;
    left[n] = e[i]; right[n] = e[i]; factor[n++] = 1.0;
  }
  for (int i = 0; i < nt; i++)
  {
    left[n] = t[i]; right[n] = t[i]; factor[n++] = -1.0;
    for (int j = i + 1; j < nt; j++)
    {
      left[n] = t[i]; right[n] = t[j]; factor[n++] = -2.0;
    }
  }
  return n;
}

/**
 * @brief Return the exponent of the lowest bit a finite double can carry: its
 * unit in the last place, which is 2^-1074 for a subnormal
 */
static inline int
double_lowest_bit(double a)
{
  uint64_t bits;
  memcpy(&bits, &a, sizeof(bits));
  int biased = (int) ((bits >> 52) & 0x7FF);
  return biased ? biased - 1075 : -1074;
}

/**
 * @brief Return the exponent of a power of two, the multiplier of a product
 */
static inline int
double_pow2_exponent(double f)
{
  uint64_t bits;
  memcpy(&bits, &f, sizeof(bits));
  return (int) ((bits >> 52) & 0x7FF) - 1023;
}

/**
 * @brief Return true if #square_products_expansion holds every product
 * exactly, as its rounded value and the error of that rounding, each times its
 * multiplier, and adds them without overflow
 * @details The error is a double exactly where the lowest bit the product
 * times its multiplier can carry, the sum of the lowest bits of its factors
 * and of the exponent of the multiplier, is not below 2^-1074, the lowest bit
 * of a double, and a product below 2^1016 leaves the sum of fifteen of them
 * finite; a sum of doubles is otherwise exact (#two_sum)
 */
static bool
square_products_exact(const double *left, const double *right,
  const double *factor, int n)
{
  for (int j = 0; j < n; j++)
  {
    if (left[j] == 0.0 || right[j] == 0.0)
      continue;
    int ef = double_pow2_exponent(factor[j]);
    int la = double_lowest_bit(left[j]), lb = double_lowest_bit(right[j]);
    if (la + lb + ef < -1074)
      return false;
    /* A double is below 2^53 times its lowest bit */
    if (la + lb + ef + 106 > 1016)
      return false;
  }
  return true;
}

/** Number of 64-bit limbs of #square_products_wide: they span the bits from
 * the lowest a product of two doubles times 1/4 carries past the sum of
 * fifteen products of the largest doubles times 2 */
#define WIDE_LIMBS 66
/** Exponent of the weight of the lowest bit of #square_products_wide, twice
 * the exponent of the lowest bit of a double, less 2 for a multiplier of 1/4 */
#define WIDE_LOWEST_BIT (2 * -1074 - 2)

/**
 * @brief Add a 64-bit integer times 2^pos to, or subtract it from, a two's
 * complement integer of #WIDE_LIMBS limbs
 */
static void
wide_add(uint64_t *acc, uint64_t v, int pos, bool negate)
{
  int limb = pos / 64, shift = pos % 64;
  uint64_t lo = v << shift;
  uint64_t hi = shift ? v >> (64 - shift) : 0;
  uint64_t carry = 0;
  for (int i = limb; i < WIDE_LIMBS; i++)
  {
    uint64_t term = (i == limb) ? lo : ((i == limb + 1) ? hi : 0);
    if (term == 0 && carry == 0)
    {
      if (i > limb + 1)
        break;
      continue;
    }
    uint64_t prev = acc[i];
    if (! negate)
    {
      uint64_t sum = prev + term;
      uint64_t c1 = sum < prev;
      acc[i] = sum + carry;
      carry = c1 + (acc[i] < sum);
    }
    else
    {
      uint64_t diff = prev - term;
      uint64_t b1 = prev < term;
      acc[i] = diff - carry;
      carry = b1 + (diff < carry);
    }
  }
}

/**
 * @brief Set in a two's complement integer of #WIDE_LIMBS limbs a sum of
 * products of doubles, each times a power of two from 1/4 to 2 or its
 * negation, computed exactly
 * @details A finite double is an integer mantissa of at most 53 bits times a
 * power of two, so a product of two is the product of their mantissas, split
 * in four products of at most 64 bits, times the product of their lowest bits
 * and of the multiplier. Each is added at its bit position, so the sum is
 * exact whatever the magnitudes of the products, where an expansion loses the
 * low bits of a product below 2^-1074 and overflows past the largest double.
 * #square_products_sign calls it where #square_products_exact says it must
 * @param[in] left,right,factor Products, as two factors and a multiplier
 * @param[in] n Number of products
 * @param[out] acc The sum, the weight of its lowest bit 2^#WIDE_LOWEST_BIT
 */
static void
square_products_wide(const double *left, const double *right,
  const double *factor, int n, uint64_t *acc)
{
  memset(acc, 0, WIDE_LIMBS * sizeof(uint64_t));
  for (int j = 0; j < n; j++)
  {
    if (left[j] == 0.0 || right[j] == 0.0)
      continue;
    uint64_t abits, bbits;
    memcpy(&abits, &left[j], sizeof(abits));
    memcpy(&bbits, &right[j], sizeof(bbits));
    uint64_t am = abits & 0xFFFFFFFFFFFFFULL, bm = bbits & 0xFFFFFFFFFFFFFULL;
    if ((abits >> 52) & 0x7FF)
      am |= 1ULL << 52;
    if ((bbits >> 52) & 0x7FF)
      bm |= 1ULL << 52;
    bool negate = ((abits >> 63) != (bbits >> 63)) != (factor[j] < 0.0);
    int pos = double_lowest_bit(left[j]) + double_lowest_bit(right[j]) -
      WIDE_LOWEST_BIT + double_pow2_exponent(factor[j]);
    /* The mantissas as 21 and 32 bits each, and their four products */
    uint64_t ah = am >> 32, al = am & 0xFFFFFFFFULL;
    uint64_t bh = bm >> 32, bl = bm & 0xFFFFFFFFULL;
    wide_add(acc, al * bl, pos, negate);
    wide_add(acc, ah * bl, pos + 32, negate);
    wide_add(acc, al * bh, pos + 32, negate);
    wide_add(acc, ah * bh, pos + 64, negate);
  }
  return;
}

/**
 * @brief Return the sign of an integer #square_products_wide sets
 */
static int
wide_sign(const uint64_t *acc)
{
  if (acc[WIDE_LIMBS - 1] >> 63)
    return -1;
  for (int i = 0; i < WIDE_LIMBS; i++)
  {
    if (acc[i])
      return 1;
  }
  return 0;
}

/**
 * @brief Return a positive integer #square_products_wide sets as a double
 * @p m and a power of two @p 2^exp whose product is within a relative 2^-63
 * of it, so that a caller scales the quotient of @p m rather than @p m, which
 * may lie below the smallest double
 */
static double
wide_frexp(const uint64_t *acc, int *exp)
{
  int i = WIDE_LIMBS - 1;
  while (i > 0 && acc[i] == 0)
    i--;
  double m = (double) acc[i];
  if (i > 0)
    m += ldexp((double) acc[i - 1], -64);
  *exp = 64 * i + WIDE_LOWEST_BIT;
  return m;
}

/**
 * @brief Return in an expansion a sum of products of doubles, each times 1,
 * -1, 2 or -2, and its length
 * @details Each product is its rounded value plus its error (#two_product),
 * doubling and negating a pair of doubles is exact, and the terms are added
 * into one expansion (#grow_expansion), held in increasing order of
 * magnitude, whose last component carries the sign of the whole
 * @note Exact where #square_products_exact holds
 */
static pg_attribute_always_inline int
square_products_expansion(const double *left, const double *right,
  const double *factor, int n, double *buf1, double *buf2,
  const double **result)
{
  double *cur = buf1, *nxt = buf2;
  int len = 0;
  for (int j = 0; j < n; j++)
  {
    if (left[j] == 0.0 || right[j] == 0.0)
      continue;
    double x, y;
    two_product(left[j], right[j], &x, &y);
    /* A multiplier of 1, -1, 2 or -2 scales both components exactly */
    x *= factor[j];
    y *= factor[j];
    if (y != 0.0)
    {
      len = grow_expansion(len, cur, y, nxt);
      double *swap = cur; cur = nxt; nxt = swap;
    }
    len = grow_expansion(len, cur, x, nxt);
    double *swap = cur; cur = nxt; nxt = swap;
  }
  *result = cur;
  return len;
}

/**
 * @brief Return the sign of a sum of products of doubles, each times a power
 * of two from 1/4 to 2 or its negation, computed exactly for any finite
 * doubles
 * @details The sign of the last component of #square_products_expansion where
 * #square_products_exact holds, and of the integer #square_products_wide sets
 * otherwise
 */
static int
square_products_sign(const double *left, const double *right,
  const double *factor, int n)
{
  if (! square_products_exact(left, right, factor, n))
  {
    uint64_t acc[WIDE_LIMBS];
    square_products_wide(left, right, factor, n, acc);
    return wide_sign(acc);
  }
  double buf1[40], buf2[40];
  const double *ex;
  int len = square_products_expansion(left, right, factor, n, buf1, buf2, &ex);
  if (len == 0)
    return 0;
  double top = ex[len - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the squared distance between two points in the
 * plane less the square of a sum of up to three doubles, computed exactly
 * @details The squared distance is the sum over the coordinates of
 * `q*q - 2*q*p + p*p` and the square of the sum that of the squares of its
 * terms and of twice the product of each pair, each a product of the input
 * doubles times 1, -1 or -2, as #point_distance_offset_side_sign sets them,
 * so #square_products_sign decides it for any finite doubles, where a product
 * underflows below the smallest double or overflows past the largest
 * @param[in] p,q Coordinates of the two points
 * @param[in] t,nt Terms of the sum and their number, from 1 to 3
 */
int
point_distance_square_sum_sign(const double *p, const double *q,
  const double *t, int nt)
{
  assert(nt >= 1 && nt <= 3);
  double left[12], right[12], factor[12];
  int n = 0;
  for (int i = 0; i < 2; i++)
  {
    left[n] = q[i]; right[n] = q[i]; factor[n++] = 1.0;
    left[n] = q[i]; right[n] = p[i]; factor[n++] = -2.0;
    left[n] = p[i]; right[n] = p[i]; factor[n++] = 1.0;
  }
  for (int i = 0; i < nt; i++)
  {
    left[n] = t[i]; right[n] = t[i]; factor[n++] = -1.0;
    for (int j = i + 1; j < nt; j++)
    {
      left[n] = t[i]; right[n] = t[j]; factor[n++] = -2.0;
    }
  }
  return square_products_sign(left, right, factor, n);
}

/**
 * @brief Widen a range of lowest bits by those of some doubles, zeros left
 * out
 */
static pg_attribute_always_inline void
lowest_bits_range(const double *v, int n, int *low, int *high)
{
  for (int i = 0; i < n; i++)
  {
    if (v[i] == 0.0)
      continue;
    int lb = double_lowest_bit(v[i]);
    *low = (lb < *low) ? lb : *low;
    *high = (lb > *high) ? lb : *high;
  }
}

/**
 * @brief Return true if an expansion holds exactly every product of two
 * doubles whose lowest bits lie in a range, times 1 or 2
 * @details As #square_products_exact asks: twice the lowest bit is not below
 * 2^-1074, and a double being below 2^53 times its lowest bit, twice the
 * highest bit plus 53 leaves the products, times 2, below 2^1016. An empty
 * range, every input zero, holds
 */
static pg_attribute_always_inline bool
lowest_bits_exact(int low, int high)
{
  return low == INT_MAX ||
    (2 * low >= -1074 && 2 * (high + 53) + 1 <= 1016);
}

/**
 * @brief Return true if the expansion holds every product of
 * #point_distance_square_terms exactly, read on the inputs rather than on
 * each product, as #lowest_bits_exact reads them
 */
static pg_attribute_always_inline bool
point_distance_square_inputs_exact(const double *d, const double *e,
  int ndims, const double *t, int nt)
{
  int low = INT_MAX, high = INT_MIN;
  lowest_bits_range(d, ndims, &low, &high);
  lowest_bits_range(e, ndims, &low, &high);
  lowest_bits_range(t, nt, &low, &high);
  return lowest_bits_exact(low, high);
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of a sum of up to four doubles, computed exactly for any finite
 * doubles
 * @details The sign of the last component of the expansion of the products of
 * #point_distance_square_terms where @p exact says the expansion holds them,
 * as #point_distance_square_inputs_exact reads it, and of their sum in a wide
 * integer (#square_products_wide) otherwise
 */
static pg_attribute_always_inline int
point_distance_square_sign_given(const double *d, const double *e, int ndims,
  const double *t, int nt, bool exact)
{
  double left[19], right[19], factor[19];
  int n = point_distance_square_terms(d, e, ndims, t, nt, left, right, factor);
  if (! exact)
  {
    uint64_t acc[WIDE_LIMBS];
    square_products_wide(left, right, factor, n, acc);
    return wide_sign(acc);
  }
  double buf1[40], buf2[40];
  const double *ex;
  int len = square_products_expansion(left, right, factor, n, buf1, buf2, &ex);
  if (len == 0)
    return 0;
  double top = ex[len - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of a sum of up to four doubles, computed exactly for any finite
 * doubles, as #point_distance_square_sign_given computes it once
 * #point_distance_square_inputs_exact has read its inputs.
 * #point_distance_midpoint_sign calls it where its filter cannot tell
 */
static pg_attribute_always_inline int
point_distance_square_sign_exact(const double *d, const double *e, int ndims,
  const double *t, int nt)
{
  return point_distance_square_sign_given(d, e, ndims, t, nt,
    point_distance_square_inputs_exact(d, e, ndims, t, nt));
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of `m + h`
 * @details The squared distance less `m*m` arrives as the double @p dm and a
 * bound @p dmerr on its error, which #point_distance_exact forms once for a
 * candidate `m` and both of its midpoints. Less `2*m*h + h*h`, with the
 * rounding of those two products and of the subtraction added to the bound,
 * the value gives the sign wherever it stands clear of the bound. Where it
 * does not, which a tie always is, #point_distance_square_sign_exact decides
 * the same quantity over the coordinate differences
 */
static inline int
point_distance_midpoint_sign(const double *d, const double *e, int ndims,
  double m, double h, double dm, double dmerr)
{
  double mh = 2.0 * m * h, hh = h * h;
  double value = dm - (mh + hh);
  double bound = dmerr + 4.0 * DBL_EPSILON * (fabs(mh) + hh + fabs(value)) +
    DBL_MIN;
  if (value > bound)
    return 1;
  if (value < - bound)
    return -1;
  const double t[2] = {m, h};
  return point_distance_square_sign_exact(d, e, ndims, t, 2);
}

/**
 * @brief Set the coordinate differences of two points, each as its rounded
 * value and the error of that rounding (#two_diff), and the largest of them
 * @return 0, or 1 where a difference is NaN, or 2 where one rounds to
 * infinity, which exceeds the largest double by at least half a unit in its
 * last place, and so does the distance
 */
static inline int
point_distance_diffs(const double *p, const double *q, int ndims, double *d,
  double *e, double *maxd)
{
  /* A difference less itself is 0, or NaN where it is NaN or infinite, which
   * the first such difference then tells apart */
  double z = 0.0;
  *maxd = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    two_diff(q[i], p[i], &d[i], &e[i]);
    z += d[i] - d[i];
    if (fabs(d[i]) > *maxd)
      *maxd = fabs(d[i]);
  }
  if (! isnan(z))
    return 0;
  for (int i = 0; i < ndims; i++)
  {
    if (isnan(d[i]))
      return 1;
    if (isinf(d[i]))
      return 2;
  }
  return 0;
}

/**
 * @brief Set the power of two 2^k that brings a positive double into [1, 2),
 * as two factors that are each a normal double and so is each inverse, and
 * the two factors of 2^-k
 * @details Multiplying by them is exact wherever the product neither
 * overflows nor underflows, as #ldexp is
 */
static inline void
point_distance_scale(double maxd, int *k, double *f1, double *f2, double *g1,
  double *g2)
{
  uint64_t maxbits;
  memcpy(&maxbits, &maxd, sizeof(maxbits));
  int maxexp = (int) ((maxbits >> 52) & 0x7FF);
  /* The exponent field of a normal double; a subnormal one asks #ilogb */
  *k = maxexp ? 1023 - maxexp : - ilogb(maxd);
  int k1 = (*k > 1022 || *k < -1022) ? *k / 2 : *k;
  *f1 = pow2_double(k1);
  *f2 = pow2_double(*k - k1);
  *g1 = pow2_double(- k1);
  *g2 = pow2_double(k1 - *k);
  return;
}

/**
 * @brief Return true if scaling the coordinate differences, the errors of
 * their rounding and a radius by 2^k keeps every one of them exact
 * @details A scaled value is exact where its lowest bit times 2^k is not below
 * 2^-1074, the lowest bit of a double; the largest difference scales into
 * [1, 2), so none overflows. Read before the scaling, since the lowest bits of
 * a scaled value cannot tell whether the scaling rounded it
 * (#lowest_bits_exact reads the scaled ones)
 * @param[in] d,e Coordinate differences and the errors of their rounding
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] k Exponent of the scaling (#point_distance_scale)
 * @param[in] r Radius, 0 for none
 */
static bool
point_distance_scale_exact(const double *d, const double *e, int ndims, int k,
  double r)
{
  for (int i = 0; i < ndims; i++)
  {
    if (d[i] != 0.0 && double_lowest_bit(d[i]) + k < -1074)
      return false;
    if (e[i] != 0.0 && double_lowest_bit(e[i]) + k < -1074)
      return false;
  }
  return r == 0.0 || double_lowest_bit(r) + k >= -1074;
}

static double point_distance_offset_inputs(const double *p, const double *q,
  int ndims, double r, double rlo);

/**
 * @brief Return the distance between two points as the double nearest it,
 * moving a candidate from the root of the rounded sum of the squares
 * @details The search of #point_distance_exact for a pair its first test does
 * not settle; the differences are scaled first where @p scale is true
 * @param[in] p,q Coordinates of the two points
 * @param[in,out] d,e Coordinate differences and the errors of their rounding
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] maxd Largest absolute difference
 * @param[in] scale Whether to scale by the power of two that brings the
 * largest difference into [1, 2)
 */
static pg_noinline double
point_distance_nearest(const double *p, const double *q, double *d,
  double *e, int ndims, double maxd, bool scale)
{
  int k = 0;
  double f1 = 1.0, f2 = 1.0, g1 = 1.0, g2 = 1.0;
  if (scale)
  {
    point_distance_scale(maxd, &k, &f1, &f2, &g1, &g2);
    /* Where the scaling rounds a difference or its error, every side is read
     * on the input coordinates */
    if (! point_distance_scale_exact(d, e, ndims, k, 0.0))
      return point_distance_offset_inputs(p, q, ndims, 0.0, 0.0);
    for (int i = 0; i < ndims; i++)
    {
      d[i] = d[i] * f1 * f2;
      e[i] = e[i] * f1 * f2;
    }
  }

  /* The squared distance as the sum s of the rounded squares of the
   * differences, the small terms lo0 that make it exact, and the magnitude
   * lomag0 of those terms, which no candidate changes; the first candidate is
   * the square root of s */
  double s = 0.0, lo0 = 0.0, lomag0 = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double x, y, t;
    two_product(d[i], d[i], &x, &y);
    two_sum(s, x, &s, &t);
    /* An exact difference, its error 0, adds no term of its own */
    if (e[i] == 0.0)
    {
      lo0 += t + y;
      lomag0 += fabs(t) + fabs(y);
      continue;
    }
    double de = 2.0 * d[i] * e[i], ee = e[i] * e[i];
    lo0 += (t + y) + (de + ee);
    lomag0 += fabs(t) + fabs(y) + fabs(de) + ee;
  }
  double c = sqrt(s) * g1 * g2;
  if (isinf(c))
    c = DBL_MAX;

  /* Move the candidate to the nearest double */
  while (true)
  {
    uint64_t bits;
    memcpy(&bits, &c, sizeof(bits));
    bool even = (bits & 1) == 0;
    double cs = c * f1 * f2;
    /* The squared distance less cs*cs, and a bound on its error */
    double x, y;
    two_product(cs, cs, &x, &y);
    double lo = lo0 - y;
    double lomag = lomag0 + fabs(y);
    double hi = s - x;
    bool hiexact = (s >= 0.5 * x && s <= 2.0 * x);
    double dm = hi + lo;
    double dmerr = (hiexact ? 0.0 : DBL_EPSILON * fabs(hi)) +
      16.0 * DBL_EPSILON * lomag + DBL_EPSILON * fabs(dm);
    /* With gu and gd the gaps from c to its upper and lower neighbours,
     * scaled, the squares of the two midpoints exceed cs*cs by
     * cs*gu + gu*gu/4 and fall short of it by cs*gd - gd*gd/4: a squared
     * distance between them rounds its root to c, and one beyond either is
     * nearer the neighbour, to which the candidate moves without reading the
     * midpoints. The factors cover the rounding of both sides of each test */
    if (c > 0.0 && c < DBL_MAX)
    {
      uint64_t ubits = bits + 1, dbits = bits - 1;
      double cu, cd;
      memcpy(&cu, &ubits, sizeof(cu));
      memcpy(&cd, &dbits, sizeof(cd));
      double gu = (cu - c) * f1 * f2, gd = (c - cd) * f1 * f2;
      double near = (fabs(dm) + dmerr) * (1.0 + 0x1p-50);
      if (near < gd * (cs - 0.25 * gd) * (1.0 - 0x1p-48) &&
          near < gu * (cs + 0.25 * gu) * (1.0 - 0x1p-48))
        return c;
      if ((dm - dmerr) * (1.0 - 0x1p-50) >
          gu * (cs + 0.25 * gu) * (1.0 + 0x1p-48))
      {
        c = cu;
        continue;
      }
      if (- (dm + dmerr) * (1.0 - 0x1p-50) > gd * cs * (1.0 + 0x1p-48))
      {
        c = cd;
        continue;
      }
    }
    /* The neighbours of a non-negative finite double are the next bit
     * patterns, the one after the largest double being infinity. Half the gap
     * to the upper neighbour is scaled with the rest; past the largest double
     * it is half the gap below it, 2^970 */
    uint64_t upbits = bits + 1;
    double up;
    memcpy(&up, &upbits, sizeof(up));
    double hu = isinf(up) ? ldexp(1.0, 970 + k) : (up - c) * f1 * f2 / 2.0;
    int su = point_distance_midpoint_sign(d, e, ndims, cs, hu, dm, dmerr);
    if (su > 0 || (su == 0 && ! even))
    {
      if (isinf(up))
        return INFINITY;
      c = up;
      continue;
    }
    if (c > 0.0)
    {
      uint64_t downbits = bits - 1;
      double down;
      memcpy(&down, &downbits, sizeof(down));
      double hd = (c - down) * f1 * f2 / 2.0;
      int sd = point_distance_midpoint_sign(d, e, ndims, cs, - hd, dm,
        dmerr);
      if (sd < 0 || (sd == 0 && ! even))
      {
        c = down;
        continue;
      }
    }
    return c;
  }
}

/**
 * @brief Return the distance between two points, computed exactly and rounded
 * once
 * @details The distance is the square root of the sum of the squared
 * coordinate differences, an exact rational of the coordinates, and the answer
 * is the double nearest that root, a tie going to the double whose last bit is
 * even. No root of a rounded value decides it. A double `c` is the nearest
 * exactly when the squared distance lies between the squares of the midpoints
 * from `c` to its two neighbours, and a midpoint of two doubles and its square
 * are exact, so #point_distance_midpoint_sign decides each side on the input
 * coordinates. The root of the rounded squared distance gives the first
 * candidate, and the candidate moves to a neighbour while the squared distance
 * lies beyond a midpoint.
 *
 * The squared distance less the square of the candidate is formed from pieces
 * that are exact or tiny: the squares of the differences and the square of
 * the candidate are their rounded values plus their errors (#two_square,
 * #two_product), the rounded squares are summed exactly (#two_sum), and the
 * two large terms that remain nearly cancel, so their difference is exact
 * wherever one is within a factor of two of the other (Sterbenz). Everything
 * else is far below a unit in the last place of the candidate. Where the
 * largest coordinate difference lies from 2^-400 to 2^400, the values are read
 * as they are and a fixed bound on that rest settles all but the squared
 * distances lying within about 2^-40 of a unit in the last place from a
 * midpoint, with at most one move; every other pair is searched by
 * #point_distance_nearest, whose filter leaves those to
 * #point_distance_midpoint_sign. Beyond that range every value is first scaled
 * by the power of two that brings the largest difference into [1, 2), which is
 * exact, so the squares neither overflow nor underflow where the distance
 * itself is a double: two points 1e200 or 1e-200 apart are answered like two
 * points a metre apart, and a distance beyond the largest double is
 * infinity.
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @note Correctly rounded for any finite coordinates: a side the filter cannot
 * tell is decided by #point_distance_square_sign_exact, exact for any finite
 * doubles
 */
double
point_distance_exact(const double *p, const double *q, int ndims)
{
  assert(ndims == 2 || ndims == 3);
  double d[3], e[3], maxd;
  int status = point_distance_diffs(p, q, ndims, d, e, &maxd);
  if (status)
    return (status == 1) ? NAN : INFINITY;
  if (maxd == 0.0)
    return 0.0;

  /* A largest difference from 2^-400 to 2^400 keeps every square between
   * 2^-1000 and 2^800, so the values are read as they are; beyond, they are
   * scaled by 2^k, exactly */
  uint64_t maxbits;
  memcpy(&maxbits, &maxd, sizeof(maxbits));
  int maxexp = (int) ((maxbits >> 52) & 0x7FF) - 1023;
  if (maxexp >= -400 && maxexp <= 400)
  {
    /* The root c of the rounded squared distance is the answer where the
     * squared distance lies strictly between the squares of the midpoints
     * below and above c, which exceed c*c by -gd*(c - gd/4) and
     * gu*(c + gu/4), gd <= gu the gaps to the neighbours. Its distance to
     * c*c is dm, formed from s - x, exact by Sterbenz since x, the rounded
     * square of c, is within a factor of two of s, and from the small terms:
     * the errors of the squares, of the sum and of c*c, and the terms of the
     * errors e of the differences, each |e| <= 2^-53 |d|. Their magnitudes
     * add to below 2^-48 s, so the rounding of dm, with at most an absolute
     * 2^-1060 where a term underflows, against s >= 2^-800, is below
     * 2^-90 s; every value is below 2^402, so #two_square is exact but for
     * those underflows. The root is within a unit in the last place of the
     * answer, so a squared distance beyond a midpoint moves c to that
     * neighbour, which is tested in turn */
    double s = 0.0, lo = 0.0, x, y, t;
    for (int i = 0; i < ndims; i++)
    {
      two_square(d[i], &x, &y);
      two_sum(s, x, &s, &t);
      lo += (t + y) + (2.0 * d[i] * e[i] + e[i] * e[i]);
    }
    double c = sqrt(s + lo), err = 0x1p-90 * s;
    for (int move = 0; move < 2; move++)
    {
      two_square(c, &x, &y);
      double dm = (s - x) + (lo - y);
      uint64_t cbits, ubits, dbits;
      memcpy(&cbits, &c, sizeof(cbits));
      ubits = cbits + 1;
      dbits = cbits - 1;
      double cu, cd;
      memcpy(&cu, &ubits, sizeof(cu));
      memcpy(&cd, &dbits, sizeof(cd));
      double gu = cu - c, gd = c - cd;
      if (fabs(dm) + err < gd * (c - 0.25 * gd) * (1.0 - 0x1p-48))
        return c;
      if (dm - err > gu * (c + 0.25 * gu) * (1.0 + 0x1p-48))
        c = cu;
      else if (- (dm + err) > gd * (c - 0.25 * gd) * (1.0 + 0x1p-48))
        c = cd;
      else
        break;
    }
  }
  return point_distance_nearest(p, q, d, e, ndims, maxd,
    maxexp < -400 || maxexp > 400);
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of `r + rlo + c + g/2`, computed exactly for any finite doubles
 * @details This is the side of a midpoint #point_distance_offset_exact
 * decides, `r + rlo` the radius, `c` a candidate and `g` the signed gap to its
 * neighbour, so that the midpoint is no double where `c` is subnormal. The
 * squared distance is the sum over the coordinates of `q*q - 2*q*p + p*p` and
 * the square the sum of the squares of the terms and of twice the product of
 * each pair, as #point_distance_square_terms sets them, each a product of the
 * input doubles times a power of two, so #square_products_sign decides it
 * where a coordinate difference overflows and where the products of a
 * candidate far below the radius underflow
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,rlo Radius as the sum of two doubles, @p rlo 0 for one double
 * @param[in] c Candidate
 * @param[in] g Gap from the candidate to its upper neighbour, or the negated
 * gap to its lower one
 */
static int
point_distance_offset_pair_side_sign(const double *p, const double *q,
  int ndims, double r, double rlo, double c, double g)
{
  double left[19], right[19], factor[19];
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = q[i]; right[n] = q[i]; factor[n++] = 1.0;
    left[n] = q[i]; right[n] = p[i]; factor[n++] = -2.0;
    left[n] = p[i]; right[n] = p[i]; factor[n++] = 1.0;
  }
  left[n] = r; right[n] = r; factor[n++] = -1.0;
  left[n] = c; right[n] = c; factor[n++] = -1.0;
  left[n] = g; right[n] = g; factor[n++] = -0.25;
  left[n] = r; right[n] = c; factor[n++] = -2.0;
  left[n] = r; right[n] = g; factor[n++] = -1.0;
  left[n] = c; right[n] = g; factor[n++] = -1.0;
  if (rlo != 0.0)
  {
    left[n] = rlo; right[n] = rlo; factor[n++] = -1.0;
    left[n] = r; right[n] = rlo; factor[n++] = -2.0;
    left[n] = rlo; right[n] = c; factor[n++] = -2.0;
    left[n] = rlo; right[n] = g; factor[n++] = -1.0;
  }
  return square_products_sign(left, right, factor, n);
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of `r + c + g/2`, computed exactly for any finite doubles, as
 * #point_distance_offset_pair_side_sign reads it for a radius of one double
 */
static inline int
point_distance_offset_side_sign(const double *p, const double *q, int ndims,
  double r, double c, double g)
{
  return point_distance_offset_pair_side_sign(p, q, ndims, r, 0.0, c, g);
}

/**
 * @brief Return the side of a midpoint #point_distance_offset_exact decides,
 * read on the scaled coordinate differences where the candidate, its half gap
 * and the radius scale exactly, and on the input coordinates by
 * #point_distance_offset_side_sign otherwise
 * @param[in] p,q Coordinates of the two points
 * @param[in] d,e Scaled coordinate differences and the errors of their rounding
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,rlo Radius as the sum of two doubles
 * @param[in] rs,rlos The two scaled
 * @param[in] c Candidate
 * @param[in] g Gap from the candidate to its upper neighbour, or the negated
 * gap to its lower one
 * @param[in] scaled True when the radius scales exactly
 * @param[in] k,f1,f2 Exponent of the scaling and its two factors
 * @param[in] low,high Range of the lowest bits of the scaled differences, of
 * their errors and of the scaled radius (#lowest_bits_range)
 * @param[in] cmin Least candidate read without checking its products
 */
static inline int
point_distance_offset_side(const double *p, const double *q, const double *d,
  const double *e, int ndims, double r, double rlo, double rs, double rlos,
  double c, double g, bool scaled, int k, double f1, double f2, int low,
  int high, double cmin)
{
  /* The twin of #point_distance_midpoint_sign for a radius. A candidate of at
   * least cmin and its half gap scale exactly and keep the products exact */
  int nt = (rlo != 0.0) ? 4 : 3;
  if (scaled && c >= cmin)
  {
    const double t[4] = {rs, c * f1 * f2, g * f1 * f2 / 2.0, rlos};
    return point_distance_square_sign_given(d, e, ndims, t, nt, true);
  }
  /* Scaling by 2^k, and halving, is exact where the lowest bit of the
   * result is not below 2^-1074, the scaled values below 2 */
  int lc = (c == 0.0) ? INT_MAX : double_lowest_bit(c) + k;
  int lg = double_lowest_bit(g) + k - 1;
  if (scaled && lc >= -1074 && lg >= -1074)
  {
    const double t[4] = {rs, c * f1 * f2, g * f1 * f2 / 2.0, rlos};
    if (lc != INT_MAX)
    {
      low = (lc < low) ? lc : low;
      high = (lc > high) ? lc : high;
    }
    low = (lg < low) ? lg : low;
    high = (lg > high) ? lg : high;
    return point_distance_square_sign_given(d, e, ndims, t, nt,
      lowest_bits_exact(low, high));
  }
  return point_distance_offset_pair_side_sign(p, q, ndims, r, rlo, c, g);
}

/**
 * @brief Return the distance between two points less a radius, not below
 * zero, as the double nearest it, every side read on the input coordinates
 * @details The path of #point_distance_exact and #point_distance_offset_exact
 * where scaling by 2^k would round a coordinate difference or its error
 * (#point_distance_scale_exact): the lowest bits of the scaled values cannot
 * show that rounding, so no decision reads them. Whether the points are
 * farther apart than the radius, and each side of every candidate, is the
 * sign #point_distance_offset_pair_side_sign reads on the input coordinates,
 * exact for any finite doubles. The first candidate is read from the integer
 * #square_products_wide sets for `S - r^2` on the input coordinates, a start
 * only, as #point_distance_offset_exact reads it where the difference lies
 * below the smallest double
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,rlo Radius as the sum of two doubles, not negative, both 0 for
 * the distance itself
 */
static double
point_distance_offset_inputs(const double *p, const double *q, int ndims,
  double r, double rlo)
{
  /* No farther apart than the radius: S - r^2 is not positive */
  if (point_distance_offset_pair_side_sign(p, q, ndims, r, rlo, 0.0, 0.0) <= 0)
    return 0.0;
  double left[12], right[12], factor[12];
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = q[i]; right[n] = q[i]; factor[n++] = 1.0;
    left[n] = q[i]; right[n] = p[i]; factor[n++] = -2.0;
    left[n] = p[i]; right[n] = p[i]; factor[n++] = 1.0;
  }
  left[n] = r; right[n] = r; factor[n++] = -1.0;
  left[n] = r; right[n] = rlo; factor[n++] = -2.0;
  left[n] = rlo; right[n] = rlo; factor[n++] = -1.0;
  uint64_t acc[WIDE_LIMBS];
  square_products_wide(left, right, factor, n, acc);
  /* (S - r^2) / (sqrt(S) + r), its numerator m * 2^exp and its denominator
   * read on the halves of the differences scaled by 2^-hexp, which brings the
   * largest into [1/2, 1), so that their squares neither overflow nor vanish;
   * the radius scales below 2, as sqrt(S) exceeds it */
  int exp, hexp;
  double m = wide_frexp(acc, &exp);
  double h[3], maxh = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    h[i] = q[i] * 0.5 - p[i] * 0.5;
    maxh = fmax(maxh, fabs(h[i]));
  }
  (void) frexp(maxh, &hexp);
  double sum = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double hs = ldexp(h[i], - hexp);
    sum += hs * hs;
  }
  double den = 2.0 * sqrt(sum) + ldexp(r, - hexp) + ldexp(rlo, - hexp);
  double c = (den > 0.0) ? ldexp(m / den, exp - hexp) : 0.0;
  if (! (c > 0.0))
    c = 0.0;
  if (isinf(c))
    c = DBL_MAX;

  /* Move the candidate to the nearest double */
  while (true)
  {
    uint64_t bits;
    memcpy(&bits, &c, sizeof(bits));
    bool even = (bits & 1) == 0;
    uint64_t upbits = bits + 1;
    double up;
    memcpy(&up, &upbits, sizeof(up));
    /* The gap to the upper neighbour, past the largest double the gap below
     * it, 2^971 */
    double gu = isinf(up) ? ldexp(1.0, 971) : up - c;
    int su = point_distance_offset_pair_side_sign(p, q, ndims, r, rlo, c, gu);
    if (su > 0 || (su == 0 && ! even))
    {
      if (isinf(up))
        return INFINITY;
      c = up;
      continue;
    }
    if (c > 0.0)
    {
      uint64_t downbits = bits - 1;
      double down;
      memcpy(&down, &downbits, sizeof(down));
      int sd = point_distance_offset_pair_side_sign(p, q, ndims, r, rlo, c,
        - (c - down));
      if (sd < 0 || (sd == 0 && ! even))
      {
        c = down;
        continue;
      }
    }
    return c;
  }
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of `a + b1 + b2 + b3`, or 0 where a filter on doubles cannot tell
 * @details The squared distance arrives as the sum `s` of the rounded squares
 * of the coordinate differences and the double `lo` of its small terms, whose
 * magnitudes add to @p lomag, as #point_distance_offset_filter forms them. The
 * square of `a` is split by #two_square, so the value is
 * `(s - x) + (lo - y - 2*a*b - b*b)`, `b` the rounded `b1 + b2 + b3`. The
 * first difference is exact where `s` is within a factor of two of `x`
 * (Sterbenz), and every other term carries its rounding into the bound: the
 * summing of `lo` (at most 15 operations on terms below @p lomag), the
 * rounding of `b` (`|b - b1 - b2 - b3| <= DBL_EPSILON bmag`, `bmag` the sum of
 * their magnitudes, which `2a` and `2 bmag` carry into the products), the
 * products and the three subtractions, each below `DBL_EPSILON` times the
 * magnitudes they read, and an absolute `DBL_MIN` for a square that underflows
 * @return 1 or -1 where the value stands clear of the bound, 0 otherwise,
 * never the sign of an exact tie, which the caller reads exactly
 */
static inline int
point_distance_offset_filter_sign(double s, double lo, double lomag, double a,
  double b1, double b2, double b3)
{
  double x, y;
  two_square(a, &x, &y);
  double hi = s - x;
  bool hiexact = (s >= 0.5 * x && s <= 2.0 * x);
  double b = (b1 + b2) + b3;
  double bmag = fabs(b1) + fabs(b2) + fabs(b3);
  double ab = 2.0 * a * b, bb = b * b;
  double value = hi + (((lo - y) - ab) - bb);
  double bound = (hiexact ? 0.0 : DBL_EPSILON * fabs(hi)) +
    32.0 * DBL_EPSILON * (lomag + fabs(y) + fabs(ab) + bb + fabs(value)) +
    4.0 * DBL_EPSILON * bmag * (fabs(a) + bmag) + 4.0 * DBL_MIN;
  if (value > bound)
    return 1;
  if (value < - bound)
    return -1;
  return 0;
}

/**
 * @brief Answer the distance between two points less a radius from its value
 * in two doubles where its rounding is certain
 * @details The squared distance `S` arrives as `s + lo`, within `delta` of it,
 * as #point_distance_offset_filter forms it. With `t` the rounded square root
 * of `s + lo`, `sqrt(S) = t + (S - t^2) / (2 t) - q` where
 * `0 <= q <= (S - t^2)^2 / (8 t^3)`: `S - t^2` is read as `(s - x) + (lo - y)`,
 * `t^2` split by #two_square into `x + y`, its first difference exact by
 * Sterbenz as `x` lies within a factor of two of `s`; `t - r` is split by
 * #two_sum into `h + l`, exactly, and `- rlo` and the correction added to
 * `l`. The value `v = h + l` then lies within `E` of `sqrt(S) - r - rlo`, `E`
 * the sum of the error of `S` carried through the division, the rounding of
 * the three additions and of the division, and `q`, each bounded by the
 * magnitudes it reads and the whole doubled. The answer is 0 where
 * `v + E < 0`, and the double `c` nearest `v` where `v` lies farther than `E`
 * from both midpoints between `c` and its neighbours, read on the exact
 * remainder `v - c` of a second #two_sum
 * @param[in] s,lo,delta The squared distance, the double of its small terms
 * and the bound of their error
 * @param[in] r,rlo Radius as the sum of two doubles, positive
 * @param[out] result The answer where the function returns true
 * @return False where the value lies within its bound of a midpoint, which
 * every tie does, and the caller's filter and exact search answer
 */
static inline bool
point_distance_offset_dd(double s, double lo, double delta, double r,
  double rlo, double *result)
{
  double t = sqrt(s + lo);
  double x, y;
  two_square(t, &x, &y);
  if (! (s >= 0.5 * x && s <= 2.0 * x))
    return false;
  double u = lo - y, w = (s - x) + u;
  /* Error of w as S - t^2: the error of S and the two roundings */
  double dw = delta + DBL_EPSILON * (fabs(u) + fabs(w));
  double corr = w / (2.0 * t);
  double h, l;
  two_sum(t, - r, &h, &l);
  double l0 = l - rlo;
  double l1 = l0 + corr;
  double big = fabs(w) + dw;
  double err = 2.0 * (dw / (2.0 * t) + DBL_EPSILON * fabs(corr) +
    DBL_EPSILON * (fabs(l0) + fabs(l1)) + big * big / (8.0 * t * t * t)) +
    4.0 * DBL_MIN;
  double c, rem;
  two_sum(h, l1, &c, &rem);
  if (c + rem + err < 0.0)
  {
    *result = 0.0;
    return true;
  }
  if (! (c >= 0x1p-900) || ! isfinite(c))
    return false;
  uint64_t bits;
  memcpy(&bits, &c, sizeof(bits));
  uint64_t ubits = bits + 1, dbits = bits - 1;
  double cu, cd;
  memcpy(&cu, &ubits, sizeof(cu));
  memcpy(&cd, &dbits, sizeof(cd));
  if (! isfinite(cu))
    return false;
  /* Each half gap is a power of two, exact for a candidate of at least
   * 2^-900 */
  if (rem + err < 0.5 * (cu - c) && rem - err > - 0.5 * (c - cd))
  {
    *result = c;
    return true;
  }
  return false;
}

/**
 * @brief Answer the distance between two points less a radius on doubles
 * where a filter settles it, returning false where it does not
 * @details The first test of #point_distance_offset_exact, the twin of the
 * one #point_distance_exact reads before its search. Where the largest
 * coordinate difference lies from 2^-400 to 2^400, and the radius, which is
 * below twice it, keeps every square between 2^-1000 and 2^804, the squared
 * distance `S` is `s + lo` up to the rounding of `lo`, which
 * #point_distance_offset_filter_sign bounds. The side of `S - r^2` is read
 * first, and the answer is 0 where the points are certainly no farther apart
 * than the radius. Otherwise the candidate is `(S - r^2) / (sqrt(S) + r)`,
 * and a double `c` is the answer exactly where `S` lies strictly between the
 * squares of `r + c` less half the gap to its lower neighbour and plus half
 * the gap to its upper one: `r + c` is split by #two_sum into `a + a1`, and
 * each half gap, a power of two, is exact for a candidate of at least
 * 2^-900, so each side is the sign of `S - (a + a1 + rlo + h)^2`, `rlo` the
 * second double of the radius. The candidate moves to a neighbour while a
 * side certainly says so, at most four times. Any side the filter cannot
 * tell, which every tie is, returns false, and the exact search of
 * #point_distance_offset_exact answers
 * @param[in] d,e Coordinate differences and the errors of their rounding
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,rlo Radius as the sum of two doubles, positive and @p r below
 * twice @p maxd
 * @param[in] maxd Largest absolute difference, positive
 * @param[out] result The answer where the function returns true
 */
static bool
point_distance_offset_filter(const double *d, const double *e, int ndims,
  double r, double rlo, double maxd, double *result)
{
  uint64_t maxbits;
  memcpy(&maxbits, &maxd, sizeof(maxbits));
  int maxexp = (int) ((maxbits >> 52) & 0x7FF) - 1023;
  if (maxexp < -400 || maxexp > 400)
    return false;
  double s = 0.0, lo = 0.0, lomag = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double x, y, t;
    two_square(d[i], &x, &y);
    two_sum(s, x, &s, &t);
    double de = 2.0 * d[i] * e[i], ee = e[i] * e[i];
    lo += (t + y) + (de + ee);
    lomag += fabs(t) + fabs(y) + fabs(de) + ee;
  }
  /* The value in two doubles settles every pair it leaves clear of a
   * midpoint; the summing of lo, at most 15 operations on terms below
   * lomag, bounds the error of S as #point_distance_offset_filter_sign
   * reads it */
  if (point_distance_offset_dd(s, lo, 32.0 * DBL_EPSILON * lomag, r, rlo,
      result))
    return true;
  /* The side of S - r^2 */
  int side = point_distance_offset_filter_sign(s, lo, lomag, r, rlo, 0.0,
    0.0);
  if (side == 0)
    return false;
  if (side < 0)
  {
    *result = 0.0;
    return true;
  }
  /* First candidate, (S - r^2) / (sqrt(S) + r) on doubles */
  double rx, ry;
  two_square(r, &rx, &ry);
  double c = ((s - rx) + (lo - ry - rlo * (2.0 * r + rlo))) /
    (sqrt(s + lo) + r + rlo);
  for (int move = 0; move < 4; move++)
  {
    if (! (c >= 0x1p-900) || ! isfinite(c))
      return false;
    uint64_t bits;
    memcpy(&bits, &c, sizeof(bits));
    uint64_t ubits = bits + 1, dbits = bits - 1;
    double cu, cd;
    memcpy(&cu, &ubits, sizeof(cu));
    memcpy(&cd, &dbits, sizeof(cd));
    if (! isfinite(cu))
      return false;
    double a, a1;
    two_sum(r, c, &a, &a1);
    int su = point_distance_offset_filter_sign(s, lo, lomag, a, a1, rlo,
      0.5 * (cu - c));
    if (su > 0)
    {
      c = cu;
      continue;
    }
    int sd = point_distance_offset_filter_sign(s, lo, lomag, a, a1, rlo,
      - 0.5 * (c - cd));
    if (sd < 0)
    {
      c = cd;
      continue;
    }
    if (su < 0 && sd > 0)
    {
      *result = c;
      return true;
    }
    return false;
  }
  return false;
}

/**
 * @brief Return the distance between two points less a radius, computed
 * exactly and rounded once, or 0 where the points are no farther apart than
 * the radius
 * @details This is the distance of a circular buffer to a point. With `S` the
 * squared distance and `r` the radius, the answer is the double nearest
 * `sqrt(S) - r`, a tie going to the double whose last bit is even, and a
 * double `c` is that one exactly when `S` lies between the squares of `r` plus
 * the midpoints from `c` to its two neighbours, each the square of a sum of
 * three doubles, so #point_distance_square_sign_exact decides each side on the
 * input coordinates. Subtracting the radius from the rounded distance rounds
 * twice, and where the distance barely exceeds the radius the subtraction
 * cancels, so its error is many units in the last place of the answer.
 *
 * Whether the points are farther apart than the radius is the sign of
 * `S - r^2`, decided exactly first. The first candidate is that difference,
 * summed from its exact expansion, over `sqrt(S) + r`, which keeps its
 * relative precision however much the two cancel, and it moves to a neighbour
 * while `S` lies beyond a midpoint. The values are scaled as in
 * #point_distance_exact, and a radius of at least twice the largest
 * coordinate difference exceeds the distance outright.
 *
 * The radius is the exact sum `r + rlo` of two doubles, so that the sum of
 * two radii, which rounds as a double, is read as it is: `rlo` enters every
 * side as one more term of the sum whose square is taken.
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,rlo Radius as the sum of two doubles, not negative, @p rlo 0
 * for a radius of one double
 * @note Correctly rounded for any finite coordinates and radius: whether the
 * points are farther apart than the radius is decided by
 * #square_products_sign, and each side of a candidate on the input
 * coordinates by #point_distance_offset_pair_side_sign, so neither a
 * coordinate difference beyond the largest double nor an answer far below the
 * radius leaves a double the decision cannot read
 */
static double
point_distance_offset_pair(const double *p, const double *q, int ndims,
  double r, double rlo)
{
  assert(ndims == 2 || ndims == 3);
  assert(r >= 0.0);
  if (r == 0.0 && rlo == 0.0)
    return point_distance_exact(p, q, ndims);
  double d[3], e[3], maxd;
  int status = point_distance_diffs(p, q, ndims, d, e, &maxd);
  if (status == 1)
    return NAN;
  /* The first test on doubles, which settles every pair but those near a
   * midpoint or a tie, as the first test of #point_distance_exact does */
  if (status == 0 && maxd > 0.0 && r < 2.0 * maxd)
  {
    double result;
    if (point_distance_offset_filter(d, e, ndims, r, rlo, maxd, &result))
      return result;
  }
  double c;
  if (status == 2)
  {
    /* A difference beyond the largest double puts the points farther apart
     * than a radius of one double, though not always than one of two; the
     * first candidate is four times a quarter of their distance, read on the
     * halves of the coordinates scaled by 2^-600, less a quarter of the
     * radius, so that neither the distance nor the candidate overflows
     * before the answer does, which a radius of two doubles below twice the
     * largest double leaves finite where half the distance is not */
    double sum = 0.0;
    for (int i = 0; i < ndims; i++)
    {
      double h = ldexp(q[i] * 0.5 - p[i] * 0.5, -600);
      sum += h * h;
    }
    c = 4.0 * (ldexp(sqrt(sum), 599) - r * 0.25 - rlo * 0.25);
  }
  /* The values scaled by 2^k, read where they are exact */
  int k = 0;
  double f1 = 1.0, f2 = 1.0, g1 = 1.0, g2 = 1.0, rs = 0.0, rlos = 0.0;
  bool scaled = false;
  int low = INT_MAX, high = INT_MIN;
  /* The least candidate whose scaled value and half gap keep every product
   * exact, with no check of its own (#point_distance_offset_side) */
  double cmin = INFINITY;
  if (status == 0)
  {
    /* The distance is below sqrt(3) times the largest difference, and below
     * twice it however the difference rounded */
    if (r > 2.0 * maxd || (r == 2.0 * maxd && rlo >= 0.0))
      return 0.0;
    point_distance_scale(maxd, &k, &f1, &f2, &g1, &g2);
    /* Where the scaling rounds a difference or its error, or the second
     * double of the radius, every side is read on the input coordinates; the
     * first double of the radius is read below */
    if (! point_distance_scale_exact(d, e, ndims, k, 0.0) ||
        (rlo != 0.0 && double_lowest_bit(rlo) + k < -1074))
      return point_distance_offset_inputs(p, q, ndims, r, rlo);
    /* Every scaled value is below 4, so its lowest bit is at most 2^-50 and
     * only the lowest one is tracked */
    double sum = 0.0;
    high = -50;
    for (int i = 0; i < ndims; i++)
    {
      d[i] = d[i] * f1 * f2;
      e[i] = e[i] * f1 * f2;
      sum += d[i] * d[i];
      if (d[i] != 0.0)
        low = Min(low, double_lowest_bit(d[i]));
      if (e[i] != 0.0)
        low = Min(low, double_lowest_bit(e[i]));
    }
    /* The radius scales exactly where its lowest bit stays above 2^-1074, and
     * otherwise lies far below the distance */
    rs = r * f1 * f2;
    int lr = double_lowest_bit(r) + k;
    scaled = (lr >= -1074);
    low = Min(low, lr);
    if (rlo != 0.0)
    {
      rlos = rlo * f1 * f2;
      low = Min(low, double_lowest_bit(rlo) + k);
    }

    /* No farther apart than the radius: S - r^2 is not positive. The radius
     * scaled is a double or below the squared distance however it rounds.
     * The first candidate is (S - r^2) / (sqrt(S) + r), the difference summed
     * from its smallest component up, or read with its power of two where it
     * lies below the smallest double */
    const double rt[2] = {rs, rlos};
    double left[15], right[15], factor[15];
    int n = point_distance_square_terms(d, e, ndims, rt, (rlo != 0.0) ? 2 : 1,
      left, right, factor);
    /* A candidate of at least 2^(-480-k) scales to a value whose lowest bit,
     * and that of its half gap, is above 2^-537, and every scaled candidate
     * is below 4, so its products stay exact wherever those of the inputs do */
    if (lowest_bits_exact(low, high))
    {
      /* 2^(-480-k), a normal power of two, a subnormal one, or zero where
       * every candidate is above it */
      int ce = -480 - k;
      if (ce >= -1022)
        cmin = pow2_double(ce);
      else if (ce >= -1074)
      {
        uint64_t cbits = 1ULL << (ce + 1074);
        memcpy(&cmin, &cbits, sizeof(cmin));
      }
      else
        cmin = 0.0;
      double buf1[40], buf2[40];
      const double *ex;
      int len = square_products_expansion(left, right, factor, n, buf1, buf2,
        &ex);
      if (len == 0 || ex[len - 1] <= 0.0)
        return 0.0;
      double diff = 0.0;
      for (int i = 0; i < len; i++)
        diff += ex[i];
      c = diff / (sqrt(sum) + rs + rlos) * g1 * g2;
    }
    else
    {
      uint64_t acc[WIDE_LIMBS];
      square_products_wide(left, right, factor, n, acc);
      if (wide_sign(acc) <= 0)
        return 0.0;
      int exp;
      double m = wide_frexp(acc, &exp);
      c = ldexp(m / (sqrt(sum) + rs + rlos), exp - k);
    }
  }
  if (! (c > 0.0))
    c = 0.0;
  if (isinf(c))
    c = DBL_MAX;

  /* Move the candidate to the nearest double */
  while (true)
  {
    uint64_t bits;
    memcpy(&bits, &c, sizeof(bits));
    bool even = (bits & 1) == 0;
    uint64_t upbits = bits + 1;
    double up;
    memcpy(&up, &upbits, sizeof(up));
    /* The gap to the upper neighbour, past the largest double the gap below
     * it, 2^971 */
    double gu = isinf(up) ? ldexp(1.0, 971) : up - c;
    int su = point_distance_offset_side(p, q, d, e, ndims, r, rlo, rs, rlos,
      c, gu, scaled, k, f1, f2, low, high, cmin);
    if (su > 0 || (su == 0 && ! even))
    {
      if (isinf(up))
        return INFINITY;
      c = up;
      continue;
    }
    if (c > 0.0)
    {
      uint64_t downbits = bits - 1;
      double down;
      memcpy(&down, &downbits, sizeof(down));
      int sd = point_distance_offset_side(p, q, d, e, ndims, r, rlo, rs,
        rlos, c, - (c - down), scaled, k, f1, f2, low, high, cmin);
      if (sd < 0 || (sd == 0 && ! even))
      {
        c = down;
        continue;
      }
    }
    return c;
  }
}

/**
 * @brief Return the distance between two points less a radius, computed
 * exactly and rounded once, or 0 where the points are no farther apart than
 * the radius
 * @details This is the distance of a circular buffer to a point, as
 * #point_distance_offset_pair answers it for a radius of one double
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius, not negative
 * @note Correctly rounded for any finite coordinates and radius
 */
double
point_distance_offset_exact(const double *p, const double *q, int ndims,
  double r)
{
  return point_distance_offset_pair(p, q, ndims, r, 0.0);
}

/**
 * @brief Return the distance between two points less the sum of two radii,
 * computed exactly and rounded once, or 0 where the points are no farther
 * apart than that sum
 * @details This is the distance between two circular buffers. The sum of the
 * radii rounds as a double, so it is split by #two_sum into the exact sum of
 * two doubles, which #point_distance_offset_pair reads as the radius. A sum
 * beyond the largest double is read as the two radii themselves, the larger
 * first, whose sum is exact all the same
 * @param[in] p,q Coordinates of the two points
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r1,r2 Radii, not negative
 * @note Correctly rounded for any finite coordinates and radii
 */
double
point_distance_offset_sum_exact(const double *p, const double *q, int ndims,
  double r1, double r2)
{
  assert(r1 >= 0.0 && r2 >= 0.0);
  double r, rlo;
  two_sum(r1, r2, &r, &rlo);
  if (isinf(r))
  {
    r = fmax(r1, r2);
    rlo = fmin(r1, r2);
  }
  return point_distance_offset_pair(p, q, ndims, r, rlo);
}

/**
 * @brief Return the sign of the squared distance between two points less the
 * square of a distance, computed exactly
 * @details The coordinate differences are held exactly as their rounded values
 * and the errors of that rounding (#point_distance_diffs). Where the largest
 * difference lies from 2^-400 to 2^400, they and the distance are read as they
 * are: the squared distance less the square of the distance, formed from
 * squares split by #two_square, gives the sign wherever it stands clear of a
 * bound of 2^-90 of the squares, and #point_distance_square_sign_exact
 * decides the rest. Beyond that range they are scaled by the power of two that
 * brings the largest difference into [1, 2), which is exact
 * (#point_distance_scale), so the squares neither overflow nor underflow where
 * the points and the distance are doubles. The sign is then that of
 * #point_distance_square_sign_exact with the distance as the one term. The distance is below twice the largest difference, so a
 * distance of at least that is not reached, and a distance whose scaled value
 * underflows is far below the squared distance. #point_within_distance_sign
 * calls it where its filter cannot tell
 * @note Exact for any finite coordinates and distance, as
 * #point_distance_square_sign_exact is
 * @return -1 where the points are nearer than the distance, 1 where they are
 * farther, 0 exactly where the distance is the one they are apart
 */
int
point_within_distance_sign_exact(double px, double py, double qx, double qy,
  double d)
{
  const double p[2] = {px, py}, q[2] = {qx, qy};
  double dd[2], e[2], maxd;
  int status = point_distance_diffs(p, q, 2, dd, e, &maxd);
  /* A difference beyond the largest double puts the points farther apart
   * than any finite distance */
  if (status == 2)
    return isinf(d) ? -1 : 1;
  if (status == 1)
    return 1;
  if (maxd == 0.0)
    return (d > 0.0) ? -1 : 0;
  if (d >= 2.0 * maxd)
    return -1;
  uint64_t maxbits;
  memcpy(&maxbits, &maxd, sizeof(maxbits));
  int maxexp = (int) ((maxbits >> 52) & 0x7FF) - 1023;
  if (maxexp >= -400 && maxexp <= 400)
  {
    /* A largest difference from 2^-400 to 2^400 and a distance below twice
     * it keep every square between 2^-1000 and 2^804, so the values are read
     * as they are, as #point_distance_exact reads them. S - d^2 is the sum s
     * of the rounded squares of the differences and its small terms lo, less
     * the square of d split by #two_square: the magnitudes of those terms add
     * to below 2^-48 (s + d^2) and their rounding, with at most an absolute
     * 2^-1060 where a term underflows, to below 2^-95 (s + d^2), and the two
     * subtractions and the final sum round by at most 2^-52 of the value, so
     * a value beyond that bound has the sign of S - d^2 */
    double s = 0.0, lo = 0.0, x, y, t;
    for (int i = 0; i < 2; i++)
    {
      two_square(dd[i], &x, &y);
      two_sum(s, x, &s, &t);
      lo += (t + y) + (2.0 * dd[i] * e[i] + e[i] * e[i]);
    }
    two_square(d, &x, &y);
    double value = (s - x) + (lo - y);
    double bound = 0x1p-90 * (s + x) + 0x1p-51 * fabs(value) + DBL_MIN;
    if (value > bound)
      return 1;
    if (value < - bound)
      return -1;
    /* The exact sign on the values as they are, which no scaling rounds */
    return point_distance_square_sign_exact(dd, e, 2, &d, 1);
  }
  int k;
  double f1, f2, g1, g2;
  point_distance_scale(maxd, &k, &f1, &f2, &g1, &g2);
  /* Where the scaling rounds a difference, its error or the distance, the
   * sign of S - d^2 is read on the input coordinates */
  if (! point_distance_scale_exact(dd, e, 2, k, d))
    return point_distance_offset_side_sign(p, q, 2, d, 0.0, 0.0);
  for (int i = 0; i < 2; i++)
  {
    dd[i] = dd[i] * f1 * f2;
    e[i] = e[i] * f1 * f2;
  }
  double ds = d * f1 * f2;
  return point_distance_square_sign_exact(dd, e, 2, &ds, 1);
}

/**
 * @brief Return true where the distance of a point to the interior of a
 * segment, less a radius, lies beyond the upper midpoint of a double
 * @details With `w` the point less the segment's start and `u` the segment,
 * the squared distance to the line is `(w x u)^2 / (u . u)`, so the distance
 * less `r` lies beyond `c + h`, the midpoint from `c` to its upper neighbour,
 * exactly when `(w x u)^2 - (r + c + h)^2 (u . u)` is positive, a polynomial
 * in exact sums of doubles that #polynomial_sign_exact decides. A tie is
 * beyond when the last bit of `c` is odd, so the double nearest the distance,
 * a tie going to the even one, is the smallest for which this is false
 * @param[in] sums Exact sums: the coordinates of `w`, then those of `u`, then
 * room for the sum of the radius, the candidate and the half gap
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,c,h Scaled radius, candidate and half gap to its upper neighbour
 * @param[in] odd Whether the last bit of the candidate is odd
 */
static bool
point_segment_beyond(ExactSum *sums, int ndims, double r, double c, double h,
  bool odd)
{
  const double m[3] = {r, c, h};
  int nm = 2 * ndims;
  exact_sum_set(&sums[nm], m, 3);
  /* |w x u|^2 - (r + c + h)^2 (u . u), the squared cross product the sum over
   * the planes of two coordinates of (w_i u_j - w_j u_i)^2 */
  PolyTerm terms[12];
  int n = 0;
  for (int i = 0; i < ndims; i++)
    for (int j = i + 1; j < ndims; j++)
    {
      terms[n++] = (PolyTerm) { 1.0, 4, {i, i, ndims + j, ndims + j}};
      terms[n++] = (PolyTerm) {-2.0, 4, {i, ndims + j, j, ndims + i}};
      terms[n++] = (PolyTerm) { 1.0, 4, {j, j, ndims + i, ndims + i}};
    }
  for (int k = 0; k < ndims; k++)
    terms[n++] = (PolyTerm) {-1.0, 4, {nm, nm, ndims + k, ndims + k}};
  int sign = polynomial_sign_exact(sums, terms, n);
  return sign > 0 || (sign == 0 && odd);
}

/**
 * @brief Set the magnitude of a two's complement integer of #WIDE_LIMBS limbs
 * and return its sign
 */
static int
wide_magnitude(const uint64_t *acc, uint64_t *mag)
{
  int sign = wide_sign(acc);
  memcpy(mag, acc, WIDE_LIMBS * sizeof(uint64_t));
  if (sign < 0)
  {
    /* Negate: invert every bit and add one */
    uint64_t carry = 1;
    for (int i = 0; i < WIDE_LIMBS; i++)
    {
      mag[i] = ~mag[i] + carry;
      carry = (carry && mag[i] == 0) ? 1 : 0;
    }
  }
  return sign;
}

/**
 * @brief Set the product of two magnitudes #wide_magnitude sets, exactly, in
 * 32-bit words, the lowest first
 * @details The schoolbook product of the 32-bit halves of the limbs, each
 * partial product and its carries held in 64 bits
 * @param[in] x,y Magnitudes of #WIDE_LIMBS limbs
 * @param[out] out Product of 4 * #WIDE_LIMBS words
 */
static void
wide_product(const uint64_t *x, const uint64_t *y, uint32_t *out)
{
  const int n = 2 * WIDE_LIMBS;
  uint32_t a[2 * WIDE_LIMBS], b[2 * WIDE_LIMBS];
  for (int i = 0; i < WIDE_LIMBS; i++)
  {
    a[2 * i] = (uint32_t) x[i]; a[2 * i + 1] = (uint32_t) (x[i] >> 32);
    b[2 * i] = (uint32_t) y[i]; b[2 * i + 1] = (uint32_t) (y[i] >> 32);
  }
  memset(out, 0, 2 * n * sizeof(uint32_t));
  for (int i = 0; i < n; i++)
  {
    if (a[i] == 0)
      continue;
    uint64_t carry = 0;
    for (int j = 0; j < n; j++)
    {
      uint64_t t = (uint64_t) a[i] * b[j] + out[i + j] + carry;
      out[i + j] = (uint32_t) t;
      carry = t >> 32;
    }
    /* No earlier row reaches past word i + n - 1 */
    out[i + n] = (uint32_t) carry;
  }
  return;
}

/** Number of 64-bit limbs of a #WideDyadic. The quantities
 * #point_motion_squares forms are products of up to six differences of
 * timestamps and four doubles, whose terms the exponents of the doubles spread
 * over at most about 9100 bits */
#define WIDE_DYADIC_LIMBS 160

/**
 * @brief A dyadic rational `sign * magnitude * 2^exp`, the magnitude held in
 * 64-bit limbs, the lowest first, as #square_products_wide holds its integer,
 * with exact sums and products of any two
 */
typedef struct
{
  int sign;         /**< -1, 0 or 1 */
  int len;          /**< Number of limbs of the magnitude */
  int exp;          /**< Weight of the lowest bit of the magnitude */
  uint64_t limb[WIDE_DYADIC_LIMBS];
} WideDyadic;

/**
 * @brief Drop the zero limbs at both ends of a #WideDyadic, those at the
 * bottom raising its exponent
 */
static void
wide_dyadic_normalize(WideDyadic *a)
{
  while (a->len > 0 && a->limb[a->len - 1] == 0)
    a->len--;
  if (a->len == 0)
  {
    a->sign = 0;
    a->exp = 0;
    return;
  }
  int z = 0;
  while (a->limb[z] == 0)
    z++;
  if (z > 0)
  {
    memmove(a->limb, a->limb + z, (size_t) (a->len - z) * sizeof(uint64_t));
    a->len -= z;
    a->exp += 64 * z;
  }
  return;
}

/**
 * @brief Set a #WideDyadic to a finite double, exactly: its integer mantissa
 * times the weight of its lowest bit
 */
static void
wide_dyadic_set_double(WideDyadic *a, double x)
{
  a->sign = 0;
  a->len = 0;
  a->exp = 0;
  if (x == 0.0)
    return;
  uint64_t bits;
  memcpy(&bits, &x, sizeof(bits));
  int e = (int) ((bits >> 52) & 0x7FF);
  uint64_t m = bits & 0xFFFFFFFFFFFFFULL;
  if (e)
  {
    m |= 1ULL << 52;
    a->exp = e - 1075;
  }
  else
    a->exp = -1074;
  a->sign = (bits >> 63) ? -1 : 1;
  a->limb[0] = m;
  a->len = 1;
  return;
}

/**
 * @brief Set a #WideDyadic to a 64-bit integer, exactly
 */
static void
wide_dyadic_set_int(WideDyadic *a, int64 v)
{
  a->exp = 0;
  if (v == 0)
  {
    a->sign = 0;
    a->len = 0;
    return;
  }
  a->sign = (v < 0) ? -1 : 1;
  a->limb[0] = (v < 0) ? (uint64_t) 0 - (uint64_t) v : (uint64_t) v;
  a->len = 1;
  return;
}

/**
 * @brief Set the product of two 64-bit integers as two 64-bit halves, from
 * their 32-bit halves as #wide_product forms its partial products
 */
static inline void
wide_mul64(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
  uint64_t a0 = a & 0xFFFFFFFFULL, a1 = a >> 32;
  uint64_t b0 = b & 0xFFFFFFFFULL, b1 = b >> 32;
  uint64_t p00 = a0 * b0, p01 = a0 * b1, p10 = a1 * b0, p11 = a1 * b1;
  uint64_t mid = (p00 >> 32) + (p01 & 0xFFFFFFFFULL) + (p10 & 0xFFFFFFFFULL);
  *lo = (p00 & 0xFFFFFFFFULL) | (mid << 32);
  *hi = p11 + (p01 >> 32) + (p10 >> 32) + (mid >> 32);
  return;
}

/**
 * @brief Return false, raising an internal error, where @p n limbs exceed the
 * capacity of a #WideDyadic
 */
static bool
wide_dyadic_fits(int n)
{
  if (n <= WIDE_DYADIC_LIMBS)
    return true;
  meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
    "An exact product exceeds %d limbs", WIDE_DYADIC_LIMBS);
  return false;
}

/**
 * @brief Set in @p out, of @p nx + @p ny limbs, the product of two
 * magnitudes of @p nx and @p ny limbs, the lowest first
 */
static void
wide_limbs_mul(const uint64_t *x, int nx, const uint64_t *y, int ny,
  uint64_t *out)
{
  memset(out, 0, (size_t) (nx + ny) * sizeof(uint64_t));
  for (int i = 0; i < nx; i++)
  {
    uint64_t carry = 0;
    for (int j = 0; j < ny; j++)
    {
      uint64_t hi, lo;
      wide_mul64(x[i], y[j], &hi, &lo);
      uint64_t s = out[i + j] + lo;
      hi += (s < lo);
      uint64_t s2 = s + carry;
      hi += (s2 < s);
      out[i + j] = s2;
      carry = hi;
    }
    /* No earlier row reaches past limb i + ny - 1 */
    out[i + ny] = carry;
  }
  return;
}

/**
 * @brief Set the product of two #WideDyadic, exactly
 * @note The result is not one of the operands
 */
static void
wide_dyadic_mul(const WideDyadic *a, const WideDyadic *b, WideDyadic *r)
{
  r->sign = 0;
  r->len = 0;
  r->exp = 0;
  if (a->sign == 0 || b->sign == 0 || ! wide_dyadic_fits(a->len + b->len))
    return;
  wide_limbs_mul(a->limb, a->len, b->limb, b->len, r->limb);
  r->len = a->len + b->len;
  r->sign = a->sign * b->sign;
  r->exp = a->exp + b->exp;
  wide_dyadic_normalize(r);
  return;
}

/** Number of limbs of a product of two #WideDyadic, and of its shift */
#define WIDE_PRODUCT_LIMBS (2 * WIDE_DYADIC_LIMBS + 2)

/**
 * @brief Return the sign of `a b - c d` for four #WideDyadic, exactly
 * @details Each product is formed in limbs of twice the capacity of a
 * #WideDyadic (#wide_limbs_mul), so a comparison whose four factors are held
 * exactly has its two products held exactly too: a sign of degree six in the
 * input doubles is read on factors of degree at most four, none of which
 * exceeds the capacity. The products are ordered by the weights of their
 * highest bits, and otherwise on their limbs aligned to the lower of their
 * lowest bits, as #wide_dyadic_cmp orders two values
 */
static int
wide_dyadic_product_cmp(const WideDyadic *a, const WideDyadic *b,
  const WideDyadic *c, const WideDyadic *d)
{
  int s1 = a->sign * b->sign, s2 = c->sign * d->sign;
  if (s1 != s2)
    return (s1 > s2) ? 1 : -1;
  if (s1 == 0)
    return 0;
  const WideDyadic *f[2][2] = {{a, b}, {c, d}};
  uint64_t prod[2][WIDE_PRODUCT_LIMBS];
  int len[2], exp[2], top[2];
  for (int k = 0; k < 2; k++)
  {
    wide_limbs_mul(f[k][0]->limb, f[k][0]->len, f[k][1]->limb, f[k][1]->len,
      prod[k]);
    len[k] = f[k][0]->len + f[k][1]->len;
    while (len[k] > 1 && prod[k][len[k] - 1] == 0)
      len[k]--;
    exp[k] = f[k][0]->exp + f[k][1]->exp;
    uint64_t hl = prod[k][len[k] - 1];
    int bits = 0;
    while (hl)
    {
      bits++;
      hl >>= 1;
    }
    top[k] = exp[k] + 64 * (len[k] - 1) + bits;
  }
  if (top[0] != top[1])
    return (top[0] > top[1]) ? s1 : - s1;
  /* The highest bits weigh the same, so each product shifted to the lower of
   * the two lowest bits spans at most its own limbs and one more */
  int e = (exp[0] < exp[1]) ? exp[0] : exp[1];
  uint64_t al[2][WIDE_PRODUCT_LIMBS];
  int n[2];
  for (int k = 0; k < 2; k++)
  {
    int sh = exp[k] - e, ls = sh / 64, bs = sh % 64;
    n[k] = len[k] + ls + 1;
    assert(n[k] <= WIDE_PRODUCT_LIMBS);
    memset(al[k], 0, (size_t) n[k] * sizeof(uint64_t));
    for (int i = 0; i < len[k]; i++)
    {
      al[k][i + ls] |= prod[k][i] << bs;
      if (bs)
        al[k][i + ls + 1] |= prod[k][i] >> (64 - bs);
    }
  }
  int m = (n[0] > n[1]) ? n[0] : n[1];
  for (int i = m - 1; i >= 0; i--)
  {
    uint64_t x = (i < n[0]) ? al[0][i] : 0, y = (i < n[1]) ? al[1][i] : 0;
    if (x != y)
      return (x > y) ? s1 : - s1;
  }
  return 0;
}

/**
 * @brief Set in @p out the magnitude of a #WideDyadic shifted to the weight
 * 2^e of its lowest bit, e not above its exponent, and return its number of
 * limbs, or -1 where it exceeds the capacity
 */
static int
wide_dyadic_aligned(const WideDyadic *a, int e, uint64_t *out)
{
  int s = a->exp - e, ls = s / 64, bs = s % 64;
  int n = a->len + ls + 1;
  if (! wide_dyadic_fits(n))
    return -1;
  memset(out, 0, (size_t) n * sizeof(uint64_t));
  for (int i = 0; i < a->len; i++)
  {
    out[i + ls] |= a->limb[i] << bs;
    if (bs)
      out[i + ls + 1] |= a->limb[i] >> (64 - bs);
  }
  return n;
}

/**
 * @brief Set the sum of two #WideDyadic, exactly: both magnitudes aligned to
 * the lower of the two lowest bits, then added or the smaller subtracted
 * @note The result is not one of the operands
 */
static void
wide_dyadic_add(const WideDyadic *a, const WideDyadic *b, WideDyadic *r)
{
  if (a->sign == 0)
  {
    *r = *b;
    return;
  }
  if (b->sign == 0)
  {
    *r = *a;
    return;
  }
  uint64_t x[WIDE_DYADIC_LIMBS], y[WIDE_DYADIC_LIMBS];
  int e = (a->exp < b->exp) ? a->exp : b->exp;
  int nx = wide_dyadic_aligned(a, e, x), ny = wide_dyadic_aligned(b, e, y);
  r->sign = 0;
  r->len = 0;
  r->exp = 0;
  if (nx < 0 || ny < 0)
    return;
  int n = (nx > ny) ? nx : ny;
  for (int i = nx; i < n; i++)
    x[i] = 0;
  for (int i = ny; i < n; i++)
    y[i] = 0;
  if (a->sign == b->sign)
  {
    if (! wide_dyadic_fits(n + 1))
      return;
    uint64_t carry = 0;
    for (int i = 0; i < n; i++)
    {
      uint64_t s = x[i] + y[i];
      uint64_t c1 = s < x[i];
      r->limb[i] = s + carry;
      carry = c1 + (r->limb[i] < s);
    }
    r->limb[n] = carry;
    r->len = n + 1;
    r->sign = a->sign;
  }
  else
  {
    /* The larger magnitude less the smaller, with the sign of the larger */
    int cmp = 0;
    for (int i = n - 1; i >= 0 && cmp == 0; i--)
    {
      if (x[i] != y[i])
        cmp = (x[i] > y[i]) ? 1 : -1;
    }
    if (cmp == 0)
      return;
    const uint64_t *big = (cmp > 0) ? x : y, *small = (cmp > 0) ? y : x;
    uint64_t borrow = 0;
    for (int i = 0; i < n; i++)
    {
      uint64_t d = big[i] - small[i];
      uint64_t b1 = big[i] < small[i];
      r->limb[i] = d - borrow;
      borrow = b1 + (d < borrow);
    }
    r->len = n;
    r->sign = (cmp > 0) ? a->sign : b->sign;
  }
  r->exp = e;
  wide_dyadic_normalize(r);
  return;
}

/**
 * @brief Return the sign of the difference of two #WideDyadic, exactly
 * @details Values of different signs are ordered by their signs, magnitudes
 * whose highest bits differ in weight by those weights, and the others on
 * their limbs aligned to the lower of their two lowest bits
 */
static int
wide_dyadic_cmp(const WideDyadic *a, const WideDyadic *b)
{
  if (a->sign != b->sign)
    return (a->sign > b->sign) ? 1 : -1;
  if (a->sign == 0)
    return 0;
  /* Same sign: the magnitude whose highest bit weighs more is the larger */
  int top[2];
  const WideDyadic *v[2] = {a, b};
  for (int k = 0; k < 2; k++)
  {
    uint64_t hl = v[k]->limb[v[k]->len - 1];
    int bits = 0;
    while (hl)
    {
      bits++;
      hl >>= 1;
    }
    top[k] = v[k]->exp + 64 * (v[k]->len - 1) + bits;
  }
  if (top[0] != top[1])
    return (top[0] > top[1]) ? a->sign : - a->sign;
  /* Otherwise the magnitudes are compared on the lower of their lowest bits */
  uint64_t x[WIDE_DYADIC_LIMBS], y[WIDE_DYADIC_LIMBS];
  int e = (a->exp < b->exp) ? a->exp : b->exp;
  int nx = wide_dyadic_aligned(a, e, x), ny = wide_dyadic_aligned(b, e, y);
  if (nx < 0 || ny < 0)
    return 0;
  int n = (nx > ny) ? nx : ny;
  for (int i = n - 1; i >= 0; i--)
  {
    uint64_t xi = (i < nx) ? x[i] : 0, yi = (i < ny) ? y[i] : 0;
    if (xi != yi)
      return (xi > yi) ? a->sign : - a->sign;
  }
  return 0;
}

/**
 * @brief Return a #WideDyadic as a double @p m and a power of two whose
 * product is within a relative 2^-63 of it, as #wide_frexp reads an integer
 * of #square_products_wide, so a caller divides two such values without
 * leaving the doubles
 */
static double
wide_dyadic_frexp(const WideDyadic *a, int *exp)
{
  if (a->sign == 0)
  {
    *exp = 0;
    return 0.0;
  }
  int i = a->len - 1;
  double m = (double) a->limb[i];
  if (i > 0)
    m += ldexp((double) a->limb[i - 1], -64);
  *exp = a->exp + 64 * i;
  return m;
}

/**
 * @brief Set the exact squared distances of two points moving linearly over
 * the interval [ta, tb], as quotients over a common denominator
 * @details Over the interval the first point lies on its segment from @p p0
 * at @p s0 to @p p1 at @p s1, and the second on its segment from @p q0 at
 * @p r0 to @p q1 at @p r1, so with `Dp = s1 - s0`, `Dq = r1 - r0`,
 * `a = ta - s0`, `b = ta - r0` and `t = ta + u`, the difference of their
 * positions times `Dp Dq` is `V0 + u E`, linear in the time, with
 *   `V0 = Dq (Dp - a) p0 + Dq a p1 - Dp (Dq - b) q0 - Dp b q1`,
 *   `E = Dq p1 - Dq p0 - Dp q1 + Dp q0`,
 * sums of products of integer timestamps and input doubles, held exactly. No
 * position between the instants of an operand is formed. The least squared
 * distance over the interval is that of the origin to the segment from `V0`
 * to `V0 + L E`, `L = tb - ta`: the squared cross product of `V0` and `E`
 * over `|E|^2` where the foot falls inside, an end otherwise, all over
 * `(Dp Dq)^2`.
 * @param[in] p0,p1,s0,s1 Segment of the first point and its timestamps
 * @param[in] q0,q1,r0,r1 Segment of the second point and its timestamps
 * @param[in] ta,tb Interval, within both segments
 * @param[out] nmin,dmin The least squared distance as @p nmin over @p dmin
 * @param[out] nend0,nend1,dend Squared distances at @p ta and at @p tb, over
 * @p dend
 * @pre `s0 < s1`, `r0 < r1`, `s0 <= ta <= tb <= s1`, `r0 <= ta <= tb <= r1`,
 * 2D points
 */
static void
point_motion_squares(const double *p0, const double *p1, int64 s0, int64 s1,
  const double *q0, const double *q1, int64 r0, int64 r1, int64 ta, int64 tb,
  WideDyadic *nmin, WideDyadic *dmin, WideDyadic *nend0, WideDyadic *nend1,
  WideDyadic *dend)
{
  int64 dp = s1 - s0, dq = r1 - r0, a = ta - s0, b = ta - r0;
  WideDyadic Dp, Dq, Ka, Kb, k[4], kneg, t1, t2, acc, sum;
  wide_dyadic_set_int(&Dp, dp);
  wide_dyadic_set_int(&Dq, dq);
  /* The coefficients of p0, p1, q0, q1 in V0 */
  wide_dyadic_set_int(&Ka, dp - a);
  wide_dyadic_mul(&Dq, &Ka, &k[0]);
  wide_dyadic_set_int(&Ka, a);
  wide_dyadic_mul(&Dq, &Ka, &k[1]);
  wide_dyadic_set_int(&Kb, b - dq);
  wide_dyadic_mul(&Dp, &Kb, &k[2]);
  wide_dyadic_set_int(&Kb, - b);
  wide_dyadic_mul(&Dp, &Kb, &k[3]);
  WideDyadic v[2], e[2];
  for (int c = 0; c < 2; c++)
  {
    const double val[4] = {p0[c], p1[c], q0[c], q1[c]};
    /* V0 */
    acc.sign = 0; acc.len = 0; acc.exp = 0;
    for (int i = 0; i < 4; i++)
    {
      wide_dyadic_set_double(&t1, val[i]);
      wide_dyadic_mul(&k[i], &t1, &t2);
      wide_dyadic_add(&acc, &t2, &sum);
      acc = sum;
    }
    v[c] = acc;
    /* E: Dq p1 - Dq p0 - Dp q1 + Dp q0 */
    acc.sign = 0; acc.len = 0; acc.exp = 0;
    for (int i = 0; i < 4; i++)
    {
      wide_dyadic_set_double(&t1, val[i]);
      const WideDyadic *f = (i < 2) ? &Dq : &Dp;
      wide_dyadic_mul(f, &t1, &t2);
      /* p0 and q1 enter negated */
      if (i == 0 || i == 3)
        t2.sign = - t2.sign;
      wide_dyadic_add(&acc, &t2, &sum);
      acc = sum;
    }
    e[c] = acc;
  }
  /* The common denominator of the squared distances at the ends */
  WideDyadic dpdq;
  wide_dyadic_mul(&Dp, &Dq, &dpdq);
  wide_dyadic_mul(&dpdq, &dpdq, dend);
  /* |V0|^2 and |V0 + L E|^2 */
  WideDyadic L, v1[2], sq0, sq1;
  wide_dyadic_set_int(&L, tb - ta);
  for (int c = 0; c < 2; c++)
  {
    wide_dyadic_mul(&L, &e[c], &t1);
    wide_dyadic_add(&v[c], &t1, &v1[c]);
  }
  wide_dyadic_mul(&v[0], &v[0], &sq0);
  wide_dyadic_mul(&v[1], &v[1], &sq1);
  wide_dyadic_add(&sq0, &sq1, nend0);
  wide_dyadic_mul(&v1[0], &v1[0], &sq0);
  wide_dyadic_mul(&v1[1], &v1[1], &sq1);
  wide_dyadic_add(&sq0, &sq1, nend1);
  /* |E|^2 and V0 . E */
  WideDyadic ee, ve;
  wide_dyadic_mul(&e[0], &e[0], &sq0);
  wide_dyadic_mul(&e[1], &e[1], &sq1);
  wide_dyadic_add(&sq0, &sq1, &ee);
  wide_dyadic_mul(&v[0], &e[0], &sq0);
  wide_dyadic_mul(&v[1], &e[1], &sq1);
  wide_dyadic_add(&sq0, &sq1, &ve);
  /* The foot lies inside where V0 . E < 0 and -V0 . E < L |E|^2 */
  bool inside = false;
  if (ee.sign > 0 && ve.sign < 0)
  {
    wide_dyadic_mul(&L, &ee, &t1);
    kneg = ve;
    kneg.sign = - kneg.sign;
    inside = wide_dyadic_cmp(&kneg, &t1) < 0;
  }
  if (inside)
  {
    WideDyadic cross;
    wide_dyadic_mul(&v[0], &e[1], &sq0);
    wide_dyadic_mul(&v[1], &e[0], &sq1);
    sq1.sign = - sq1.sign;
    wide_dyadic_add(&sq0, &sq1, &cross);
    wide_dyadic_mul(&cross, &cross, nmin);
    wide_dyadic_mul(&ee, dend, dmin);
  }
  else
  {
    *nmin = (wide_dyadic_cmp(nend0, nend1) <= 0) ? *nend0 : *nend1;
    *dmin = *dend;
  }
  return;
}

/**
 * @brief Return true where the root of `n / d` lies beyond the midpoint from
 * the double @p c to its upper neighbour @p up, infinity standing for 2^1024,
 * a tie being beyond where the last bit of @p c is odd
 */
static bool
wide_dyadic_root_beyond(const WideDyadic *n, const WideDyadic *d, double c,
  double up)
{
  WideDyadic wc, wu, mid, m2, rhs;
  wide_dyadic_set_double(&wc, c);
  if (isinf(up))
  {
    wu.sign = 1; wu.len = 1; wu.exp = 1024; wu.limb[0] = 1;
  }
  else
    wide_dyadic_set_double(&wu, up);
  wide_dyadic_add(&wc, &wu, &mid);
  if (mid.sign != 0)
    mid.exp -= 1;
  wide_dyadic_mul(&mid, &mid, &m2);
  wide_dyadic_mul(&m2, d, &rhs);
  int s = wide_dyadic_cmp(n, &rhs);
  uint64_t bits;
  memcpy(&bits, &c, sizeof(bits));
  return s > 0 || (s == 0 && (bits & 1) != 0);
}

/**
 * @brief Return the double nearest the square root of `n / d >= 0`, a tie
 * going to the even double
 * @details The answer is the smallest bit pattern whose upper midpoint the
 * root does not pass (#wide_dyadic_root_beyond): a gallop from a candidate
 * read on the leading limbs brackets it, then halving the bracket finds it, as
 * #point_segment_distance_offset_exact finds its answer
 */
static double
wide_dyadic_nearest_root(const WideDyadic *n, const WideDyadic *d)
{
  if (n->sign <= 0)
    return 0.0;
  int en, ed;
  double mn = wide_dyadic_frexp(n, &en), md = wide_dyadic_frexp(d, &ed);
  double ratio = mn / md;
  int ex = en - ed;
  if (ex % 2 != 0)
  {
    ratio *= 2.0;
    ex -= 1;
  }
  double c = ldexp(sqrt(ratio), ex / 2);
  if (! (c > 0.0))
    c = 0.0;
  if (isinf(c))
    c = DBL_MAX;
  uint64_t bits;
  memcpy(&bits, &c, sizeof(bits));
#define WD_BEYOND(bb, res) \
  do { \
    double cc, nn; \
    uint64_t b1 = (bb), b2 = (bb) + 1; \
    memcpy(&cc, &b1, sizeof(cc)); \
    memcpy(&nn, &b2, sizeof(nn)); \
    (res) = wide_dyadic_root_beyond(n, d, cc, nn); \
  } while (0)
  uint64_t lo, hi;
  bool beyond;
  WD_BEYOND(bits, beyond);
  if (beyond)
  {
    /* The answer is above: lo is beyond, hi is not */
    uint64_t step = 1;
    lo = bits;
    const uint64_t inf_bits = 0x7FF0000000000000ULL;
    while (true)
    {
      hi = (lo + step < inf_bits) ? lo + step : inf_bits;
      if (hi == inf_bits)
        break;
      WD_BEYOND(hi, beyond);
      if (! beyond)
        break;
      lo = hi;
      step *= 2;
    }
  }
  else
  {
    /* The answer is at or below: hi is not beyond, lo is beyond or zero */
    uint64_t step = 1;
    hi = bits;
    while (true)
    {
      if (hi == 0)
        return 0.0;
      lo = (hi > step) ? hi - step : 0;
      WD_BEYOND(lo, beyond);
      if (beyond)
        break;
      hi = lo;
      if (lo == 0)
        return 0.0;
      step *= 2;
    }
  }
  while (hi - lo > 1)
  {
    uint64_t mid = lo + (hi - lo) / 2;
    WD_BEYOND(mid, beyond);
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
#undef WD_BEYOND
  double result;
  memcpy(&result, &hi, sizeof(result));
  return result;
}

/*****************************************************************************
 * The nearest double of a value read exactly
 *****************************************************************************/

/* The sign of the value a search reads less @p mid */
typedef int (*NearestSide)(const WideDyadic *mid, const void *arg);

/* True where the value, negated where @p neg is true, lies beyond the
 * midpoint from the double of bit pattern @p bits to the next one, a tie
 * being beyond where the last bit of that double is odd, so a tie goes to
 * the even double, as #wide_dyadic_root_beyond reads it */
static bool
nearest_bits_beyond(uint64_t bits, bool neg, NearestSide side,
  const void *arg)
{
  double lo, up;
  uint64_t b2 = bits + 1;
  memcpy(&lo, &bits, sizeof(lo));
  memcpy(&up, &b2, sizeof(up));
  WideDyadic wl, wu, mid;
  wide_dyadic_set_double(&wl, lo);
  wide_dyadic_set_double(&wu, up);
  wide_dyadic_add(&wl, &wu, &mid);
  if (mid.sign != 0)
    mid.exp -= 1;
  /* -v beyond mid is v below -mid */
  if (neg)
    mid.sign = - mid.sign;
  int s = side(&mid, arg);
  if (neg)
    s = - s;
  return s > 0 || (s == 0 && (bits & 1) != 0);
}

/**
 * @brief Return the double nearest a value that @p side reads exactly, a tie
 * going to the even double
 * @details The sign of the value is read exactly at 0; a negative value is
 * the negation of the double nearest its magnitude. That double is the
 * smallest bit pattern whose upper midpoint the magnitude does not pass: a
 * gallop from the candidate @p c brackets it, then halving the bracket finds
 * it, as #wide_dyadic_nearest_root finds a root
 * @pre The value lies strictly between -DBL_MAX and DBL_MAX
 */
static double
nearest_double_exact(double c, NearestSide side, const void *arg)
{
  WideDyadic zero;
  zero.sign = 0; zero.len = 0; zero.exp = 0;
  int s0 = side(&zero, arg);
  if (s0 == 0)
    return 0.0;
  bool neg = s0 < 0;
  c = fabs(c);
  if (! isfinite(c))
    c = DBL_MAX;
  const uint64_t max_bits = 0x7FEFFFFFFFFFFFFFULL;
  uint64_t bits, lo, hi, step = 1;
  memcpy(&bits, &c, sizeof(bits));
  if (nearest_bits_beyond(bits, neg, side, arg))
  {
    /* The answer is above: lo is beyond, hi is not */
    lo = bits;
    while (true)
    {
      hi = (lo < max_bits - step) ? lo + step : max_bits;
      if (hi == max_bits || ! nearest_bits_beyond(hi, neg, side, arg))
        break;
      lo = hi;
      step *= 2;
    }
  }
  else
  {
    /* The answer is at or below: hi is not beyond, lo is beyond or zero */
    hi = bits;
    while (true)
    {
      lo = (hi > step) ? hi - step : 0;
      if (lo == 0 || nearest_bits_beyond(lo, neg, side, arg))
        break;
      hi = lo;
      step *= 2;
    }
    /* The magnitude lies below the upper midpoint of 0 */
    if (lo == 0 && ! nearest_bits_beyond(0, neg, side, arg))
      hi = 0;
  }
  while (hi - lo > 1)
  {
    uint64_t mid = lo + (hi - lo) / 2;
    if (nearest_bits_beyond(mid, neg, side, arg))
      lo = mid;
    else
      hi = mid;
  }
  double result;
  memcpy(&result, &hi, sizeof(result));
  return neg ? - result : result;
}

/* A quotient `n / d`, d > 0 */
typedef struct
{
  const WideDyadic *n, *d;
} NearestQuotient;

/* The sign of `n / d - mid`, that of `n - mid d` */
static int
nearest_quotient_side(const WideDyadic *mid, const void *arg)
{
  const NearestQuotient *q = (const NearestQuotient *) arg;
  WideDyadic md, diff;
  wide_dyadic_mul(mid, q->d, &md);
  md.sign = - md.sign;
  wide_dyadic_add(q->n, &md, &diff);
  return diff.sign;
}

/* A value `c + b / sqrt(s)`, s > 0 */
typedef struct
{
  const WideDyadic *c, *b, *s;
} NearestOffsetRoot;

/* The sign of `c + b / sqrt(s) - mid`, that of `b - a sqrt(s)` with
 * `a = mid - c`, read on squares where both sides have one sign */
static int
nearest_offset_root_side(const WideDyadic *mid, const void *arg)
{
  const NearestOffsetRoot *v = (const NearestOffsetRoot *) arg;
  WideDyadic nc = *v->c, a, a2, a2s, b2, diff;
  nc.sign = - nc.sign;
  wide_dyadic_add(mid, &nc, &a);
  int sa = a.sign, sb = v->b->sign;
  if (sa <= 0 && sb >= 0)
    return (sa == 0 && sb == 0) ? 0 : 1;
  if (sa >= 0 && sb <= 0)
    return -1;
  /* Both of one sign: compare b^2 with a^2 s */
  wide_dyadic_mul(&a, &a, &a2);
  wide_dyadic_mul(&a2, v->s, &a2s);
  wide_dyadic_mul(v->b, v->b, &b2);
  a2s.sign = - a2s.sign;
  wide_dyadic_add(&b2, &a2s, &diff);
  return (sb > 0) ? diff.sign : - diff.sign;
}

/* The double nearest `n / d`, d > 0 */
static double
wide_dyadic_nearest_quotient(const WideDyadic *n, const WideDyadic *d)
{
  int en, ed;
  double mn = wide_dyadic_frexp(n, &en), md = wide_dyadic_frexp(d, &ed);
  double c = ldexp(mn / md, en - ed);
  NearestQuotient q = {n, d};
  return nearest_double_exact(c, &nearest_quotient_side, &q);
}

/* The double nearest `c + b / sqrt(s)`, s > 0 */
static double
wide_dyadic_nearest_offset_root(double c, const WideDyadic *b,
  const WideDyadic *s)
{
  int eb, es;
  double mb = wide_dyadic_frexp(b, &eb), ms = wide_dyadic_frexp(s, &es);
  if (es % 2 != 0)
  {
    ms *= 2.0;
    es -= 1;
  }
  double guess = c + b->sign * ldexp(mb / sqrt(ms), eb - es / 2);
  WideDyadic wc;
  wide_dyadic_set_double(&wc, c);
  NearestOffsetRoot v = {&wc, b, s};
  return nearest_double_exact(guess, &nearest_offset_root_side, &v);
}

/**
 * @brief Set the shortest line from a point or a disc at rest to a segment,
 * each end the double nearest its exact value
 * @details The nearest point Q of the segment ab to the centre c is
 * `a + t (b - a)`, `t = N / D` with `N = (c - a).(b - a)` and
 * `D = |b - a|^2` clamped to [0, 1], so each coordinate of Q is the rational
 * `(a D + (b - a) N) / D`, rounded once to its nearest double. The line
 * starts at the centre for a point (@p r = 0), at Q for a disc meeting the
 * segment (`|Q - c| <= r`), and otherwise at the point of the circle towards
 * Q, `c + r (Q - c) / |Q - c|`. At an end `Q - c` is `a - c` or `b - c`; at
 * the foot it is `X (-uy, ux) / D`, with `X = w x u`, perpendicular to the
 * segment, so the point of the circle is `c + r sign(X) (-uy, ux) / |u|`.
 * Either way each coordinate is `c + b / sqrt(s)`, with `b` and `s` of degree
 * two in the input doubles, decided by exact signs of squares; no root is
 * taken in doubles. Every test has degree at most four, which #WideDyadic
 * holds for every finite input
 * @param[in] c Centre
 * @param[in] r Radius, 0 for a point
 * @param[in] a,b Ends of the segment
 * @param[out] sx,sy Start of the line
 * @param[out] ex,ey End of the line
 */
void
point_segment_shortestline_exact(const double *c, double r, const double *a,
  const double *b, double *sx, double *sy, double *ex, double *ey)
{
  WideDyadic wcx, wcy, wax, way, wbx, wby, ux, uy, wx, wy, t1, t2, n, d;
  wide_dyadic_set_double(&wcx, c[0]); wide_dyadic_set_double(&wcy, c[1]);
  wide_dyadic_set_double(&wax, a[0]); wide_dyadic_set_double(&way, a[1]);
  wide_dyadic_set_double(&wbx, b[0]); wide_dyadic_set_double(&wby, b[1]);
  /* u = b - a, w = c - a, N = w.u, D = u.u, all exact */
  WideDyadic nax = wax, nay = way;
  nax.sign = - nax.sign; nay.sign = - nay.sign;
  wide_dyadic_add(&wbx, &nax, &ux); wide_dyadic_add(&wby, &nay, &uy);
  wide_dyadic_add(&wcx, &nax, &wx); wide_dyadic_add(&wcy, &nay, &wy);
  wide_dyadic_mul(&wx, &ux, &t1); wide_dyadic_mul(&wy, &uy, &t2);
  wide_dyadic_add(&t1, &t2, &n);
  wide_dyadic_mul(&ux, &ux, &t1); wide_dyadic_mul(&uy, &uy, &t2);
  wide_dyadic_add(&t1, &t2, &d);
  /* Q = (qxn, qyn) / den */
  WideDyadic qxn, qyn, den;
  bool foot = false;
  if (d.sign == 0 || n.sign <= 0)
  {
    qxn = wax; qyn = way;
    wide_dyadic_set_double(&den, 1.0);
  }
  else if (wide_dyadic_cmp(&n, &d) >= 0)
  {
    qxn = wbx; qyn = wby;
    wide_dyadic_set_double(&den, 1.0);
  }
  else
  {
    wide_dyadic_mul(&wax, &d, &t1); wide_dyadic_mul(&ux, &n, &t2);
    wide_dyadic_add(&t1, &t2, &qxn);
    wide_dyadic_mul(&way, &d, &t1); wide_dyadic_mul(&uy, &n, &t2);
    wide_dyadic_add(&t1, &t2, &qyn);
    den = d;
    foot = true;
  }
  *ex = wide_dyadic_nearest_quotient(&qxn, &den);
  *ey = wide_dyadic_nearest_quotient(&qyn, &den);
  if (r == 0.0)
  {
    *sx = c[0]; *sy = c[1];
    return;
  }
  /* The direction from c to Q, (dx, dy) / sqrt(s), and the square it is
   * compared with: at the foot (-uy, ux) X / D, |Q - c|^2 = X^2 / D, against
   * r^2 as X^2 against r^2 D; at an end Q - c, against r^2 */
  WideDyadic dx, dy, s, lhs, rhs, wr, bx, by;
  wide_dyadic_set_double(&wr, r);
  wide_dyadic_mul(&wr, &wr, &t1);
  if (foot)
  {
    WideDyadic x, t3;
    wide_dyadic_mul(&wx, &uy, &t2); wide_dyadic_mul(&wy, &ux, &t3);
    t3.sign = - t3.sign;
    wide_dyadic_add(&t2, &t3, &x);
    dx = uy; dx.sign = - dx.sign * x.sign;
    dy = ux; dy.sign = dy.sign * x.sign;
    s = d;
    wide_dyadic_mul(&x, &x, &lhs);
    wide_dyadic_mul(&t1, &d, &rhs);
  }
  else
  {
    WideDyadic ncx = wcx, ncy = wcy;
    ncx.sign = - ncx.sign; ncy.sign = - ncy.sign;
    wide_dyadic_add(&qxn, &ncx, &dx); wide_dyadic_add(&qyn, &ncy, &dy);
    wide_dyadic_mul(&dx, &dx, &t2); wide_dyadic_mul(&dy, &dy, &lhs);
    wide_dyadic_add(&t2, &lhs, &s);
    lhs = s;
    rhs = t1;
  }
  if (wide_dyadic_cmp(&lhs, &rhs) <= 0)
  {
    /* The disc meets the segment: the line collapses to Q */
    *sx = *ex; *sy = *ey;
    return;
  }
  wide_dyadic_mul(&wr, &dx, &bx); wide_dyadic_mul(&wr, &dy, &by);
  *sx = wide_dyadic_nearest_offset_root(c[0], &bx, &s);
  *sy = wide_dyadic_nearest_offset_root(c[1], &by, &s);
  return;
}

/**
 * @brief Return the nearest approach of two points moving linearly over the
 * interval [ta, tb], the double nearest its exact value
 * @details Time is read as continuous: the least distance over the interval
 * is an exact rational of the input doubles and the integer timestamps
 * (#point_motion_squares), answered as the double nearest its root
 * @param[in] p0,p1,s0,s1 Segment of the first point and its timestamps
 * @param[in] q0,q1,r0,r1 Segment of the second point and its timestamps
 * @param[in] ta,tb Interval, within both segments
 * @pre `s0 < s1`, `r0 < r1`, the interval within both segments, 2D points
 */
double
point_motion_nad_exact(const double *p0, const double *p1, int64 s0,
  int64 s1, const double *q0, const double *q1, int64 r0, int64 r1, int64 ta,
  int64 tb)
{
  WideDyadic nmin, dmin, n0, n1, dend;
  point_motion_squares(p0, p1, s0, s1, q0, q1, r0, r1, ta, tb, &nmin, &dmin,
    &n0, &n1, &dend);
  return wide_dyadic_nearest_root(&nmin, &dmin);
}

/** Relative slack on either side of a comparison
 * #point_motion_dwithin_filter certifies, above the relative error of the at
 * most a few dozen roundings forming each side, about 2^-48 */
#define MOTION_FILTER_SLACK 0x1p-40
/** Absolute margin of a comparison #point_motion_dwithin_filter certifies,
 * above the error a gradual underflow adds to either side */
#define MOTION_FILTER_TINY 0x1p-1000
/** Greatest magnitude of a coordinate or a distance
 * #point_motion_dwithin_filter reads, so that none of its products overflows */
#define MOTION_FILTER_MAXMAG 0x1p250
/** Least magnitude of the coordinates #point_motion_dwithin_filter reads, so
 * that the error of the ends dominates every underflow */
#define MOTION_FILTER_MINMAG 0x1p-300

/**
 * @brief Return true if @p lo, a lower bound computed in doubles, certainly
 * exceeds @p hi, an upper bound computed in doubles, each widened by
 * #MOTION_FILTER_SLACK of its magnitude and the two by #MOTION_FILTER_TINY
 */
static inline bool
point_motion_filter_exceeds(double lo, double hi)
{
  return lo - MOTION_FILTER_SLACK * fabs(lo) >
    hi + MOTION_FILTER_SLACK * fabs(hi) + MOTION_FILTER_TINY;
}

/**
 * @brief Return 1 or 0 where two points moving linearly over the interval
 * [ta, tb] are certainly or certainly not within a distance at some instant
 * (@p ever) or at every instant of it, -1 where doubles cannot decide
 * @details The difference of the two positions is linear over the interval,
 * from `A` at @p ta to `C` at @p tb, so with `N0 = |A|^2`, `N1 = |C|^2`,
 * `G = A . C`, `X = A x C` and `D = |C - A|^2` the pair is ever within `d`
 * where `min(N0, N1) <= d^2`, or where the foot of the origin on the segment
 * from `A` to `C` falls inside it, `N0 > G` and `N1 > G`, and
 * `X^2 <= d^2 D`; it is not where `X^2 > d^2 D`, the line from `A` to `C`
 * being farther than `d`, or where both ends are farther than `d` and the
 * foot falls outside; it is always within where both ends are. Each is
 * decided on doubles with a bound on its error, and certified only where it
 * holds over the whole width of the bounds.
 *
 * The bounds, with `u = 2^-53` and `g_k = k u / (1 - k u)`, rounding to the
 * nearest, a fused multiply-add only lowering them:
 * - Each coordinate of a position is `p0 + (p1 - p0) l` with `l` the exact
 *   fraction of the segment's duration, in [0, 1]. Its computed form is
 *   `l (1 + t3)`, `|t3| <= g3`, two conversions and a division, so the
 *   product is within `g5 |p1 - p0| l <= 2 g5 M` of `(p1 - p0) l`, `M` the
 *   largest magnitude of the eight input coordinates, and the position, of
 *   magnitude at most `M`, within `2 g5 M + u (1 + 2 g5) M <= 3 g6 M`.
 * - The difference of the two positions is then within
 *   `6 g6 M + u (2 + 6 g6) M <= 7 g7 M <= 2^-47 M = e` of the exact `A`, per
 *   coordinate, and likewise `C`.
 * - A product of two such coordinates is within `e (|a| + |c| + e)` of the
 *   exact one, `a` and `c` the computed coordinates; a sum of two products
 *   is within `g2` of its terms' magnitudes, and a sum of two squares
 *   within `g3 <= 2^-51` of its computed value. So `N0` is within
 *   `e (2 |Ax| + 2 |Ay| + 2 e) + 2^-51 N0`, `X` and `G` within
 *   `e (|Ax| + |Ay| + |Cx| + |Cy| + 2 e)` plus `2^-51` of the magnitudes of
 *   their two products, each coordinate of `C - A` within
 *   `e' = 2 e + 2^-52 |C - A|`, `D` within `e' (2 |C - A| + e')` per
 *   coordinate plus `2^-51 D`, and `d^2` within `2^-52 d^2`.
 * Each bound is widened by #MOTION_FILTER_SLACK before it is added or
 * subtracted, covering the roundings that form it, and each comparison by
 * that slack and #MOTION_FILTER_TINY (#point_motion_filter_exceeds). The
 * filter answers -1 outside the magnitudes #MOTION_FILTER_MINMAG and
 * #MOTION_FILTER_MAXMAG bound.
 * @param[in] p0,p1,s0,s1 Segment of the first point and its timestamps
 * @param[in] q0,q1,r0,r1 Segment of the second point and its timestamps
 * @param[in] ta,tb Interval, within both segments
 * @param[in] d Distance, not negative
 * @param[in] ever True for the ever semantics, false for the always one
 * @pre `s0 < s1`, `r0 < r1`, the interval within both segments, 2D points
 */
static int
point_motion_dwithin_filter(const double *p0, const double *p1, int64 s0,
  int64 s1, const double *q0, const double *q1, int64 r0, int64 r1, int64 ta,
  int64 tb, double d, bool ever)
{
  const double *pts[4] = {p0, p1, q0, q1};
  double M = 0.0;
  for (int i = 0; i < 4; i++)
    for (int c = 0; c < 2; c++)
    {
      double m = fabs(pts[i][c]);
      /* The negation also refuses a NaN */
      if (! (m <= MOTION_FILTER_MAXMAG))
        return -1;
      if (m > M)
        M = m;
    }
  if (M < MOTION_FILTER_MINMAG || ! (d <= MOTION_FILTER_MAXMAG))
    return -1;
  const double sl = 1.0 + MOTION_FILTER_SLACK;
  const double e = ldexp(M, -47);
  /* The two ends of the difference of the positions */
  double dp = (double) (s1 - s0), dq = (double) (r1 - r0);
  double la = (double) (ta - s0) / dp, lb = (double) (tb - s0) / dp;
  double ma = (double) (ta - r0) / dq, mb = (double) (tb - r0) / dq;
  double A[2], C[2], B[2];
  for (int c = 0; c < 2; c++)
  {
    double up = p1[c] - p0[c], uq = q1[c] - q0[c];
    A[c] = (p0[c] + up * la) - (q0[c] + uq * ma);
    C[c] = (p0[c] + up * lb) - (q0[c] + uq * mb);
    B[c] = C[c] - A[c];
  }
  double ax = fabs(A[0]), ay = fabs(A[1]), cx = fabs(C[0]), cy = fabs(C[1]);
  double n0 = A[0] * A[0] + A[1] * A[1];
  double n1 = C[0] * C[0] + C[1] * C[1];
  double bn0 = sl * (e * (2.0 * (ax + ay) + 2.0 * e) + 0x1p-51 * n0);
  double bn1 = sl * (e * (2.0 * (cx + cy) + 2.0 * e) + 0x1p-51 * n1);
  double dd = d * d, bdd = sl * 0x1p-52 * dd;
  double dd_lo = dd - bdd, dd_hi = dd + bdd;
  if (! ever)
  {
    /* Always within where both ends are, not where either is not */
    if (point_motion_filter_exceeds(dd_lo, n0 + bn0) &&
        point_motion_filter_exceeds(dd_lo, n1 + bn1))
      return 1;
    if (point_motion_filter_exceeds(n0 - bn0, dd_hi) ||
        point_motion_filter_exceeds(n1 - bn1, dd_hi))
      return 0;
    return -1;
  }
  /* Ever within where an end is */
  if (point_motion_filter_exceeds(dd_lo, n0 + bn0) ||
      point_motion_filter_exceeds(dd_lo, n1 + bn1))
    return 1;
  double ecross = e * (ax + ay + cx + cy + 2.0 * e);
  double x = A[0] * C[1] - A[1] * C[0];
  double bx = sl * (ecross + 0x1p-51 * (ax * cy + ay * cx));
  double g = A[0] * C[0] + A[1] * C[1];
  double bg = sl * (ecross + 0x1p-51 * (ax * cx + ay * cy));
  double bd = 0x1p-51 * (B[0] * B[0] + B[1] * B[1]);
  double dlen = B[0] * B[0] + B[1] * B[1];
  for (int c = 0; c < 2; c++)
  {
    double eb = 2.0 * e + 0x1p-52 * fabs(B[c]);
    bd += eb * (2.0 * fabs(B[c]) + eb);
  }
  bd *= sl;
  double x_hi = fabs(x) + bx, x_lo = fabs(x) - bx;
  double dlen_lo = dlen - bd, dlen_hi = dlen + bd;
  /* Within where the foot falls inside and the line is within */
  if (point_motion_filter_exceeds(n0 - bn0, g + bg) &&
      point_motion_filter_exceeds(n1 - bn1, g + bg) &&
      dd_lo > 0.0 && dlen_lo > 0.0 &&
      point_motion_filter_exceeds(dd_lo * dlen_lo, x_hi * x_hi))
    return 1;
  /* Not within where the line is farther than the distance */
  if (x_lo > 0.0 && point_motion_filter_exceeds(x_lo * x_lo, dd_hi * dlen_hi))
    return 0;
  /* Not within where both ends are farther and the foot falls outside */
  if (point_motion_filter_exceeds(n0 - bn0, dd_hi) &&
      point_motion_filter_exceeds(n1 - bn1, dd_hi) &&
      (point_motion_filter_exceeds(g - bg, n0 + bn0) ||
       point_motion_filter_exceeds(g - bg, n1 + bn1)))
    return 0;
  return -1;
}

/**
 * @brief Return true if two points moving linearly over the interval
 * [ta, tb] are within a distance at some instant of it (@p ever) or at every
 * instant of it, decided exactly over continuous time
 * @details Ever within holds where the least squared distance over the
 * interval is at most `d^2`; always within where both ends are, the squared
 * distance being a convex quadratic in time (#point_motion_squares). The
 * filter #point_motion_dwithin_filter answers where doubles decide it.
 * @param[in] p0,p1,s0,s1 Segment of the first point and its timestamps
 * @param[in] q0,q1,r0,r1 Segment of the second point and its timestamps
 * @param[in] ta,tb Interval, within both segments
 * @param[in] d Distance, not negative
 * @param[in] ever True for the ever semantics, false for the always one
 * @pre `s0 < s1`, `r0 < r1`, the interval within both segments, 2D points
 */
bool
point_motion_dwithin_exact(const double *p0, const double *p1, int64 s0,
  int64 s1, const double *q0, const double *q1, int64 r0, int64 r1, int64 ta,
  int64 tb, double d, bool ever)
{
  int filter = point_motion_dwithin_filter(p0, p1, s0, s1, q0, q1, r0, r1, ta,
    tb, d, ever);
  if (filter >= 0)
    return filter == 1;
  WideDyadic nmin, dmin, n0, n1, dend, wd, dd, rhs;
  point_motion_squares(p0, p1, s0, s1, q0, q1, r0, r1, ta, tb, &nmin, &dmin,
    &n0, &n1, &dend);
  wide_dyadic_set_double(&wd, d);
  wide_dyadic_mul(&wd, &wd, &dd);
  if (ever)
  {
    wide_dyadic_mul(&dd, &dmin, &rhs);
    return wide_dyadic_cmp(&nmin, &rhs) <= 0;
  }
  wide_dyadic_mul(&dd, &dend, &rhs);
  return wide_dyadic_cmp(&n0, &rhs) <= 0 && wide_dyadic_cmp(&n1, &rhs) <= 0;
}

/**
 * @brief Return the least distance of two points moving linearly over the
 * interval [ta, tb], computed in doubles
 * @details An estimate, not the nearest double of the exact value: it only
 * chooses the bound #point_motion_nad_exceeds tests the intervals against, so
 * that #point_motion_nad_exact runs on the few intervals that may hold the
 * least distance. Its error decides no answer.
 * @param[in] p0,p1,s0,s1 Segment of the first point and its timestamps
 * @param[in] q0,q1,r0,r1 Segment of the second point and its timestamps
 * @param[in] ta,tb Interval, within both segments
 * @pre `s0 < s1`, `r0 < r1`, the interval within both segments, 2D points
 */
double
point_motion_nad_approx(const double *p0, const double *p1, int64 s0,
  int64 s1, const double *q0, const double *q1, int64 r0, int64 r1, int64 ta,
  int64 tb)
{
  double dp = (double) (s1 - s0), dq = (double) (r1 - r0);
  double la = (double) (ta - s0) / dp, lb = (double) (tb - s0) / dp;
  double ma = (double) (ta - r0) / dq, mb = (double) (tb - r0) / dq;
  double A[2], B[2];
  for (int c = 0; c < 2; c++)
  {
    double up = p1[c] - p0[c], uq = q1[c] - q0[c];
    A[c] = (p0[c] + up * la) - (q0[c] + uq * ma);
    B[c] = ((p0[c] + up * lb) - (q0[c] + uq * mb)) - A[c];
  }
  double dlen = B[0] * B[0] + B[1] * B[1];
  double g = - (A[0] * B[0] + A[1] * B[1]);
  double f = (dlen > 0.0 && g > 0.0) ? ((g < dlen) ? g / dlen : 1.0) : 0.0;
  return hypot(A[0] + f * B[0], A[1] + f * B[1]);
}

/**
 * @brief Return true if the least distance of two points moving linearly
 * over the interval [ta, tb] certainly exceeds a distance, false where it
 * does not or doubles cannot decide (#point_motion_dwithin_filter)
 * @param[in] p0,p1,s0,s1 Segment of the first point and its timestamps
 * @param[in] q0,q1,r0,r1 Segment of the second point and its timestamps
 * @param[in] ta,tb Interval, within both segments
 * @param[in] d Distance, not negative
 * @pre `s0 < s1`, `r0 < r1`, the interval within both segments, 2D points
 */
bool
point_motion_nad_exceeds(const double *p0, const double *p1, int64 s0,
  int64 s1, const double *q0, const double *q1, int64 r0, int64 r1, int64 ta,
  int64 tb, double d)
{
  return point_motion_dwithin_filter(p0, p1, s0, s1, q0, q1, r0, r1, ta, tb,
    d, true) == 0;
}

/*****************************************************************************/

/**
 * @brief Add @p x to the #WideDyadic @p acc, exactly
 */
static inline void
wide_dyadic_accumulate(WideDyadic *acc, const WideDyadic *x)
{
  WideDyadic sum;
  wide_dyadic_add(acc, x, &sum);
  *acc = sum;
  return;
}

/**
 * @brief Add the double @p x times @p sign to the #WideDyadic @p acc, exactly
 */
static inline void
wide_dyadic_accumulate_double(WideDyadic *acc, double x, int sign)
{
  WideDyadic t;
  wide_dyadic_set_double(&t, x);
  t.sign *= sign;
  wide_dyadic_accumulate(acc, &t);
  return;
}

/**
 * @brief Return the value of a #WideDyadic, rounded, or an infinity or zero
 * outside the doubles
 */
static double
wide_dyadic_approx(const WideDyadic *a)
{
  int e;
  double m = wide_dyadic_frexp(a, &e);
  return a->sign * ldexp(m, e);
}

/**
 * @brief Set the coefficients of the quadratic whose sign says whether two
 * discs moving linearly over a segment are within a distance, exactly
 * @details Over the fraction f in [0, 1] of the segment the centres stand
 * apart by `w(f) = w0 + f w1`, with `w0 = a0 - b0` and
 * `w1 = (a1 - a0) - (b1 - b0)`, and the discs are within @p d where
 * `|w(f)| <= L(f)`, `L(f) = L0 + f dL` the sum of the radii and the
 * distance, from `L0 = ra0 + rb0 + d` to `L1 = ra1 + rb1 + d`. The radii and
 * the distance are not negative, so neither is `L` over the segment, and the
 * discs are within where `q(f) = |w(f)|^2 - L(f)^2 = A f^2 + 2 B f + C` is not
 * positive, with `A = |w1|^2 - dL^2`, `B = w0 . w1 - L0 dL` and
 * `C = |w0|^2 - L0^2`, each a sum of products of the input doubles, held
 * exactly as #point_motion_squares holds its sums. The distance is the sum
 * @p d + @p dlo, exactly, so that the midpoint of two doubles can be one
 */
static void
point_motion_dwithin_coeffs(const double *a0, const double *a1,
  const double *b0, const double *b1, int ndims, double ra0, double ra1,
  double rb0, double rb1, double d, double dlo, WideDyadic *A, WideDyadic *B,
  WideDyadic *C)
{
  WideDyadic w0, w1, sq, l0, dl;
  A->sign = 0; A->len = 0; A->exp = 0;
  B->sign = 0; B->len = 0; B->exp = 0;
  C->sign = 0; C->len = 0; C->exp = 0;
  for (int c = 0; c < ndims; c++)
  {
    w0.sign = 0; w0.len = 0; w0.exp = 0;
    wide_dyadic_accumulate_double(&w0, a0[c], 1);
    wide_dyadic_accumulate_double(&w0, b0[c], -1);
    w1.sign = 0; w1.len = 0; w1.exp = 0;
    wide_dyadic_accumulate_double(&w1, a1[c], 1);
    wide_dyadic_accumulate_double(&w1, a0[c], -1);
    wide_dyadic_accumulate_double(&w1, b1[c], -1);
    wide_dyadic_accumulate_double(&w1, b0[c], 1);
    wide_dyadic_mul(&w1, &w1, &sq);
    wide_dyadic_accumulate(A, &sq);
    wide_dyadic_mul(&w0, &w1, &sq);
    wide_dyadic_accumulate(B, &sq);
    wide_dyadic_mul(&w0, &w0, &sq);
    wide_dyadic_accumulate(C, &sq);
  }
  l0.sign = 0; l0.len = 0; l0.exp = 0;
  wide_dyadic_accumulate_double(&l0, ra0, 1);
  wide_dyadic_accumulate_double(&l0, rb0, 1);
  wide_dyadic_accumulate_double(&l0, d, 1);
  wide_dyadic_accumulate_double(&l0, dlo, 1);
  dl.sign = 0; dl.len = 0; dl.exp = 0;
  wide_dyadic_accumulate_double(&dl, ra1, 1);
  wide_dyadic_accumulate_double(&dl, rb1, 1);
  wide_dyadic_accumulate_double(&dl, ra0, -1);
  wide_dyadic_accumulate_double(&dl, rb0, -1);
  wide_dyadic_mul(&dl, &dl, &sq);
  sq.sign = - sq.sign;
  wide_dyadic_accumulate(A, &sq);
  wide_dyadic_mul(&l0, &dl, &sq);
  sq.sign = - sq.sign;
  wide_dyadic_accumulate(B, &sq);
  wide_dyadic_mul(&l0, &l0, &sq);
  sq.sign = - sq.sign;
  wide_dyadic_accumulate(C, &sq);
  return;
}

/**
 * @brief The quadratic of #point_motion_dwithin_coeffs read at the whole
 * microseconds of a segment of `D` microseconds: `n` is within where
 * `A n^2 + 2 B D n + C D^2`, which is `D^2 q(n / D)`, is not positive
 */
typedef struct
{
  WideDyadic A;     /**< A */
  WideDyadic B2D;   /**< 2 B D */
  WideDyadic CD2;   /**< C D^2 */
  WideDyadic BD;    /**< B D, for the vertex test A n + B D */
} DWithinGrid;

/**
 * @brief Return the sign of `D^2 q(n / D)` (@p vertex false) or of
 * `A n + B D` (@p vertex true), exactly
 */
static int
dwithin_grid_sign(const DWithinGrid *g, int64 n, bool vertex)
{
  WideDyadic wn, t, acc;
  wide_dyadic_set_int(&wn, n);
  if (vertex)
  {
    wide_dyadic_mul(&g->A, &wn, &t);
    wide_dyadic_add(&t, &g->BD, &acc);
    return acc.sign;
  }
  WideDyadic n2;
  wide_dyadic_mul(&wn, &wn, &n2);
  wide_dyadic_mul(&g->A, &n2, &acc);
  wide_dyadic_mul(&g->B2D, &wn, &t);
  wide_dyadic_accumulate(&acc, &t);
  wide_dyadic_accumulate(&acc, &g->CD2);
  return acc.sign;
}

/**
 * @brief Return the microsecond strictly inside (@p lo, @p hi) that a search
 * starts from: the truncation of the estimate @p est where it falls there,
 * the middle otherwise
 */
static int64
dwithin_grid_start(double est, int64 lo, int64 hi)
{
  int64 start = lo + (hi - lo) / 2;
  if (isfinite(est) && est > (double) lo + 1.0 && est < (double) hi - 1.0)
    start = llrint(floor(est));
  return start;
}

/**
 * @brief Return the largest @p n in [@p lo, @p hi) at which the sign of
 * #dwithin_grid_sign is not positive, that sign being not positive at @p lo,
 * positive at @p hi and changing once in between
 * @details A gallop from the estimate @p est brackets the answer, then halving
 * the bracket finds it, as #wide_dyadic_nearest_root finds its answer
 */
static int64
dwithin_grid_last(const DWithinGrid *g, int64 lo, int64 hi, double est,
  bool vertex)
{
  if (hi - lo <= 1)
    return lo;
  int64 start = dwithin_grid_start(est, lo, hi);
  int64 step = 1;
  if (dwithin_grid_sign(g, start, vertex) <= 0)
  {
    lo = start;
    while (true)
    {
      int64 next = (hi - lo > step) ? lo + step : hi;
      if (next == hi || dwithin_grid_sign(g, next, vertex) > 0)
      {
        hi = next;
        break;
      }
      lo = next;
      step *= 2;
    }
  }
  else
  {
    hi = start;
    while (true)
    {
      int64 next = (hi - lo > step) ? hi - step : lo;
      if (next == lo || dwithin_grid_sign(g, next, vertex) <= 0)
      {
        lo = next;
        break;
      }
      hi = next;
      step *= 2;
    }
  }
  while (hi - lo > 1)
  {
    int64 mid = lo + (hi - lo) / 2;
    if (dwithin_grid_sign(g, mid, vertex) <= 0)
      lo = mid;
    else
      hi = mid;
  }
  return lo;
}

/**
 * @brief Return the twin of #dwithin_grid_last for a sign that is positive at
 * @p lo and not positive at @p hi: the smallest @p n in (@p lo, @p hi] at
 * which the sign of `D^2 q(n / D)` is not positive
 */
static int64
dwithin_grid_first(const DWithinGrid *g, int64 lo, int64 hi, double est)
{
  if (hi - lo <= 1)
    return hi;
  int64 start = dwithin_grid_start(est, lo, hi);
  int64 step = 1;
  if (dwithin_grid_sign(g, start, false) <= 0)
  {
    hi = start;
    while (true)
    {
      int64 next = (hi - lo > step) ? hi - step : lo;
      if (next == lo || dwithin_grid_sign(g, next, false) > 0)
      {
        lo = next;
        break;
      }
      hi = next;
      step *= 2;
    }
  }
  else
  {
    lo = start;
    while (true)
    {
      int64 next = (hi - lo > step) ? lo + step : hi;
      if (next == hi || dwithin_grid_sign(g, next, false) <= 0)
      {
        hi = next;
        break;
      }
      lo = next;
      step *= 2;
    }
  }
  while (hi - lo > 1)
  {
    int64 mid = lo + (hi - lo) / 2;
    if (dwithin_grid_sign(g, mid, false) <= 0)
      hi = mid;
    else
      lo = mid;
  }
  return hi;
}

/**
 * @brief Return the microsecond, from the start of a segment of @p D
 * microseconds, at which truncation dates a root of the quadratic of
 * #point_motion_dwithin_coeffs
 * @param[in] g The quadratic at the whole microseconds
 * @param[in] root A double estimate of the root, a fraction of the segment
 * @param[in] D Duration of the segment
 * @param[in] rising True for a root at which the quadratic turns positive,
 * false for one at which it stops being positive
 * @param[in] lo,hi Microseconds bracketing the root: the quadratic is not
 * positive at @p lo and positive at @p hi where @p rising, and the reverse
 * otherwise
 */
static int64
dwithin_grid_root(const DWithinGrid *g, double root, int64 D, bool rising,
  int64 lo, int64 hi)
{
  double est = root * (double) D;
  /* A root at which the quadratic turns positive lies in [n, n + 1) for the
   * last n at which it is not positive, so truncation dates it at n */
  if (rising)
    return dwithin_grid_last(g, lo, hi, est, false);
  /* A root at which it stops being positive lies in (m - 1, m] for the first
   * m at which it is not positive: at m where it is zero there, and inside
   * (m - 1, m) otherwise, which truncation dates at m - 1 */
  int64 m = dwithin_grid_first(g, lo, hi, est);
  return (dwithin_grid_sign(g, m, false) == 0) ? m : m - 1;
}

/**
 * @brief Return 1, 2 or 0 where two discs moving linearly over a segment are
 * certainly within a distance over the whole segment, certainly within it at
 * some instant, or certainly never within it, -1 where doubles cannot decide
 * @details The twin of #point_motion_dwithin_filter for the quadratic of
 * #point_motion_dwithin_coeffs, read on the rounded differences `w0`, `w1`,
 * `L0` and `dL`. With `M` the largest magnitude of the inputs, each rounded
 * difference is a sum of at most six of them formed in at most five
 * roundings, so it lies within `e = 2^-47 M` of the exact one; a product of
 * two of them is within `e (|x| + |y| + e)` of the exact one, and a sum of at
 * most four products within `2^-50` of the magnitudes of its terms. With `T`
 * the sum of the magnitudes of the rounded differences, every product reads
 * terms of at most `T` and each coefficient sums at most four of them, so one
 * bound `E = e (2 T + 4 e) + 2^-50 T^2` holds for `A`, `B` and `C` alike,
 * and `T` is a NaN or an infinity where an input is. The discs are within
 * throughout where `q(0)` and `q(1)` are certainly not positive, the gap
 * `|w| - L` being convex; never where both are certainly positive and so is
 * the least of `q` over the segment: at an end where the parabola is
 * certainly concave, or certainly convex with its vertex certainly outside,
 * and at the vertex where `A C - B^2` is certainly positive. Each bound is
 * widened by #MOTION_FILTER_SLACK and each comparison read by
 * #point_motion_filter_exceeds
 */
static int
point_motion_dwithin_period_filter(const double *a0, const double *a1,
  const double *b0, const double *b1, int ndims, double ra0, double ra1,
  double rb0, double rb1, double d)
{
  double A = 0.0, B = 0.0, C = 0.0, T = 0.0, M = 0.0, G = 0.0;
  for (int c = 0; c < ndims; c++)
  {
    double w0 = a0[c] - b0[c];
    double w1 = (a1[c] - a0[c]) - (b1[c] - b0[c]);
    A += w1 * w1;
    B += w0 * w1;
    C += w0 * w0;
    T += fabs(w0) + fabs(w1);
    /* The magnitudes are read with Max rather than fmax, a call: a NaN they
     * would drop is refused below through T */
    M = Max(M, Max(Max(fabs(a0[c]), fabs(a1[c])),
      Max(fabs(b0[c]), fabs(b1[c]))));
    /* The gap from 0 to the interval the coordinate of w(f) sweeps */
    double lo = Min(w0, w0 + w1), hi = Max(w0, w0 + w1);
    double gap = Max(Max(lo, - hi), 0.0);
    G += gap * gap;
  }
  double L0 = (ra0 + rb0) + d;
  double dL = (ra1 + rb1) - (ra0 + rb0);
  A -= dL * dL;
  B -= L0 * dL;
  C -= L0 * L0;
  T += fabs(L0) + fabs(dL);
  M = Max(M, Max(Max(Max(ra0, ra1), Max(rb0, rb1)), d));
  /* The negations also refuse a NaN, which T carries from any input */
  if (! (T <= MOTION_FILTER_MAXMAG) || ! (M <= MOTION_FILTER_MAXMAG) ||
      M < MOTION_FILTER_MINMAG)
    return -1;
  const double sl = 1.0 + MOTION_FILTER_SLACK;
  /* Exact, M lying above #MOTION_FILTER_MINMAG */
  const double e = M * 0x1p-47;
  /* Never within where the box w(f) sweeps lies farther from the origin than
   * the largest L. Each end of the interval a coordinate sweeps is within 3 e
   * of the exact one, so each gap is, and the squared distance of the box,
   * a sum of gaps of at most T in all, exceeds G less 8 e (T + 4 e); the
   * largest L is within 4 e of the exact one, and taken above it */
  double Lmax = Max(L0, L0 + dL) + 4.0 * e;
  if (point_motion_filter_exceeds(G - sl * 8.0 * e * (T + 4.0 * e),
      sl * Lmax * Lmax))
    return 0;
  const double E = sl * (e * (2.0 * T + 4.0 * e) + 0x1p-50 * T * T);
  double q1 = (A + 2.0 * B) + C;
  double e1 = sl * (4.0 * E + 0x1p-51 * (fabs(A) + 2.0 * fabs(B) +
    fabs(C)));
  bool in0 = point_motion_filter_exceeds(0.0, C + E);
  bool out0 = point_motion_filter_exceeds(C - E, 0.0);
  bool in1 = point_motion_filter_exceeds(0.0, q1 + e1);
  bool out1 = point_motion_filter_exceeds(q1 - e1, 0.0);
  if (in0 && in1)
    return 1;
  /* Within at an end, so within at some instant */
  if (! out0 || ! out1)
    return (in0 || in1) ? 2 : -1;
  /* A parabola lies at most A / 4 below the chord of its ends, so the least
   * is certainly positive where the lesser end exceeds that, which settles a
   * segment at rest, A = 0, and every one moving slowly against the gap */
  if (point_motion_filter_exceeds(fmin(C - E, q1 - e1),
      0.25 * (fabs(A) + E)))
    return 0;
  /* Certainly concave: the least is at an end */
  if (point_motion_filter_exceeds(0.0, A + E))
    return 0;
  if (! point_motion_filter_exceeds(A - E, 0.0))
    return -1;
  /* Certainly convex with the vertex -B / A at or before the start, B >= 0,
   * or at or after the end, A + B <= 0: the least is at an end */
  if (point_motion_filter_exceeds(B - E, 0.0) ||
      point_motion_filter_exceeds(- (A + B) - 2.0 * E, 0.0))
    return 0;
  /* The least at the vertex is (A C - B^2) / A */
  double ac = A * C, bb = B * B;
  double ed = sl * (E * (fabs(A) + fabs(C) + 2.0 * fabs(B) + 2.0 * E) +
    0x1p-51 * (fabs(ac) + bb));
  if (point_motion_filter_exceeds(ac - bb - ed, 0.0))
    return 0;
  /* Within at the vertex, certainly inside, so within at some instant */
  if (point_motion_filter_exceeds(0.0, B + E) &&
      point_motion_filter_exceeds(A + B - 2.0 * E, 0.0) &&
      point_motion_filter_exceeds(0.0, ac - bb + ed))
    return 2;
  return -1;
}

static int point_motion_dwithin_period_wide(const double *a0,
  const double *a1, const double *b0, const double *b1, int ndims,
  double ra0, double ra1, double rb0, double rb1, double d, int64 D,
  int64 *n1, int64 *n2);

/**
 * @brief Return 2, 1 or 0 where two discs moving linearly over a segment of
 * @p D microseconds are within a distance during a period of it, at a single
 * instant of it, or never, setting in @p n1 and @p n2 the microseconds from
 * the start of the segment that bound the period, decided exactly over
 * continuous time
 * @details The discs are within where the quadratic `q` of
 * #point_motion_dwithin_coeffs is not positive. The gap `|w(f)| - L(f)`
 * between them is convex in the fraction `f` of the segment, a norm of an
 * affine map less an affine map, and `L` is not negative, so the fractions at
 * which they are within form one interval of [0, 1], possibly a single
 * fraction or none. Each bound of it is an end of the segment or a root of
 * `q`, and is dated as MEOS dates a crossing, by truncating its exact offset
 * to a whole microsecond: a microsecond `n` lies at or after a bound at which
 * the discs come within, and at or before one at which they leave, exactly
 * where `D^2 q(n / D)` is not positive, an exact sign on the input doubles
 * and the integer @p D, so the dates are found by galloping searches on that
 * sign from a double estimate (#dwithin_grid_root), and no root of a rounded
 * value decides one. A period narrower than a microsecond holds no whole
 * microsecond, and both its bounds are dated at the truncation of its vertex,
 * a single instant. Points are discs of radius 0. The filter
 * #point_motion_dwithin_period_filter answers where doubles decide that the
 * discs are within over the whole segment or never.
 * @param[in] a0,a1 Centre of the first disc at the start and at the end
 * @param[in] b0,b1 Centre of the second disc at the start and at the end
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] ra0,ra1 Radius of the first disc at the start and at the end
 * @param[in] rb0,rb1 Radius of the second disc at the start and at the end
 * @param[in] d Distance
 * @param[in] D Duration of the segment in microseconds
 * @param[out] n1,n2 Bounds of the period in microseconds from the start
 * @pre The radii and the distance are finite and not negative, `D > 0`
 */
int
point_motion_dwithin_period_exact(const double *a0, const double *a1,
  const double *b0, const double *b1, int ndims, double ra0, double ra1,
  double rb0, double rb1, double d, int64 D, int64 *n1, int64 *n2)
{
  assert(D > 0); assert(ndims == 2 || ndims == 3);
  assert(ra0 >= 0.0 && ra1 >= 0.0 && rb0 >= 0.0 && rb1 >= 0.0 && d >= 0.0);
  int filter = point_motion_dwithin_period_filter(a0, a1, b0, b1, ndims, ra0,
    ra1, rb0, rb1, d);
  if (filter == 1)
  {
    *n1 = 0;
    *n2 = D;
    return 2;
  }
  if (filter == 0)
    return 0;
  return point_motion_dwithin_period_wide(a0, a1, b0, b1, ndims, ra0, ra1,
    rb0, rb1, d, D, n1, n2);
}

/**
 * @brief Return true where two discs moving linearly over a segment of @p D
 * microseconds are within a distance at the microsecond @p n of it, decided
 * exactly
 * @details The discs are within where `D^2 q(n / D)` is not positive, the
 * sign #point_motion_dwithin_period_exact searches on (#dwithin_grid_sign).
 * A bound of the period that it dates by truncation lies at a microsecond at
 * which the discs need not be within: the one before a root at which they
 * come within, unless the root is that microsecond
 * @param[in] a0,a1 Centre of the first disc at the start and at the end
 * @param[in] b0,b1 Centre of the second disc at the start and at the end
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] ra0,ra1 Radius of the first disc at the start and at the end
 * @param[in] rb0,rb1 Radius of the second disc at the start and at the end
 * @param[in] d Distance
 * @param[in] D Duration of the segment in microseconds
 * @param[in] n Microsecond from the start of the segment
 * @pre The radii and the distance are finite and not negative, `D > 0`
 */
bool
point_motion_dwithin_at_exact(const double *a0, const double *a1,
  const double *b0, const double *b1, int ndims, double ra0, double ra1,
  double rb0, double rb1, double d, int64 D, int64 n)
{
  assert(D > 0); assert(ndims == 2 || ndims == 3);
  DWithinGrid g;
  WideDyadic B, C, B2, t, wD;
  point_motion_dwithin_coeffs(a0, a1, b0, b1, ndims, ra0, ra1, rb0, rb1, d, 0.0,
    &g.A, &B, &C);
  wide_dyadic_set_int(&t, 2);
  wide_dyadic_mul(&B, &t, &B2);
  wide_dyadic_set_int(&wD, D);
  wide_dyadic_mul(&B2, &wD, &g.B2D);
  wide_dyadic_mul(&C, &wD, &t);
  wide_dyadic_mul(&t, &wD, &g.CD2);
  wide_dyadic_mul(&B, &wD, &g.BD);
  return dwithin_grid_sign(&g, n, false) <= 0;
}

/**
 * @brief Return the answer of #point_motion_dwithin_period_exact for a
 * segment its filter does not settle, on the exact quadratic
 * @details Kept out of line, as #point_distance_nearest is, so that the
 * exact values it holds do not enlarge the frame of every call the filter
 * answers
 */
static pg_noinline int
point_motion_dwithin_period_wide(const double *a0, const double *a1,
  const double *b0, const double *b1, int ndims, double ra0, double ra1,
  double rb0, double rb1, double d, int64 D, int64 *n1, int64 *n2)
{
  DWithinGrid g;
  WideDyadic B, C, B2, t, q1;
  point_motion_dwithin_coeffs(a0, a1, b0, b1, ndims, ra0, ra1, rb0, rb1, d, 0.0,
    &g.A, &B, &C);
  /* q(1) = A + 2 B + C */
  wide_dyadic_set_int(&t, 2);
  wide_dyadic_mul(&B, &t, &B2);
  wide_dyadic_add(&g.A, &B2, &q1);
  wide_dyadic_accumulate(&q1, &C);
  bool in0 = C.sign <= 0, in1 = q1.sign <= 0;
  /* Within at both ends, within throughout, the gap being convex */
  if (in0 && in1)
  {
    *n1 = 0;
    *n2 = D;
    return 2;
  }
  WideDyadic wD;
  wide_dyadic_set_int(&wD, D);
  wide_dyadic_mul(&B2, &wD, &g.B2D);
  wide_dyadic_mul(&C, &wD, &t);
  wide_dyadic_mul(&t, &wD, &g.CD2);
  wide_dyadic_mul(&B, &wD, &g.BD);
  /* Estimates of the roots of A x^2 + 2 B x + C, which only start the
   * searches, the one a difference of close values would form taken from
   * their product C / A */
  double a = wide_dyadic_approx(&g.A), b = wide_dyadic_approx(&B),
    c = wide_dyadic_approx(&C);
  double disc = b * b - a * c;
  double sq = (disc > 0.0) ? sqrt(disc) : 0.0;
  double qq = (b >= 0.0) ? -(b + sq) : -(b - sq);
  double ra = (a != 0.0) ? qq / a : ((b != 0.0) ? - c / (2.0 * b) : 0.5);
  double rb = (qq != 0.0) ? c / qq : ra;
  double rlo = fmin(ra, rb), rhi = fmax(ra, rb);

  /* Within at the start, not at the end: one root, at which q turns
   * positive */
  if (in0)
  {
    *n1 = 0;
    *n2 = dwithin_grid_root(&g, (rlo >= 0.0) ? rlo : rhi, D, true, 0, D);
    return (*n2 == 0) ? 1 : 2;
  }
  /* Within at the end, not at the start: one root, at which q stops being
   * positive */
  if (in1)
  {
    *n1 = dwithin_grid_root(&g, (rhi <= 1.0) ? rhi : rlo, D, false, 0, D);
    *n2 = D;
    return (*n1 == D) ? 1 : 2;
  }
  /* Within at neither end: within in between only where the parabola is
   * convex, its vertex -B / A lies inside, and its discriminant B^2 - A C is
   * not negative */
  WideDyadic bb, ac, dw, apb;
  wide_dyadic_mul(&B, &B, &bb);
  wide_dyadic_mul(&g.A, &C, &ac);
  ac.sign = - ac.sign;
  wide_dyadic_add(&bb, &ac, &dw);
  wide_dyadic_add(&g.A, &B, &apb);
  if (g.A.sign <= 0 || B.sign >= 0 || apb.sign <= 0 || dw.sign < 0)
    return 0;
  /* The microsecond the vertex truncates to, the last n with A n + B D <= 0,
   * which is 0 < -B D / A < D */
  double v = (a != 0.0) ? - b / a : 0.5;
  int64 nv = dwithin_grid_last(&g, 0, D, v * (double) D, true);
  /* A whole microsecond within the period, if any, lies next to the vertex */
  int64 inside = -1;
  if (dwithin_grid_sign(&g, nv, false) <= 0)
    inside = nv;
  else if (nv + 1 <= D && dwithin_grid_sign(&g, nv + 1, false) <= 0)
    inside = nv + 1;
  /* A period holding no whole microsecond lies inside (nv, nv + 1), and
   * truncation dates both its bounds at nv */
  if (inside < 0)
  {
    *n1 = *n2 = nv;
    return 1;
  }
  *n1 = dwithin_grid_root(&g, rlo, D, false, 0, inside);
  *n2 = dwithin_grid_root(&g, rhi, D, true, inside, D);
  return (*n1 == *n2) ? 1 : 2;
}

/**
 * @brief Return true if two discs moving linearly over a segment are within
 * a distance at one end of it and at no other fraction of it, decided exactly
 * @details The fractions at which they are within form one interval of
 * [0, 1] (#point_motion_dwithin_period_exact). It is the start alone where the
 * quadratic `q = A f^2 + 2 B f + C` of #point_motion_dwithin_coeffs is zero
 * there and turns positive after it, `C = 0` and `B > 0`, or `B = 0` and
 * `A > 0`; the end alone where `q(1) = 0` and `q` is positive before it,
 * `A + B < 0`, or `A + B = 0` and `A > 0`. An excluded end of a sequence is
 * the one case where a segment within at some fraction is within at no
 * instant of the sequence
 * @param[in] a0,a1,b0,b1,ndims,ra0,ra1,rb0,rb1,d As for
 * #point_motion_dwithin_period_exact
 * @param[in] start True for the start of the segment, false for its end
 */
bool
point_motion_dwithin_only_at_end(const double *a0, const double *a1,
  const double *b0, const double *b1, int ndims, double ra0, double ra1,
  double rb0, double rb1, double d, bool start)
{
  WideDyadic A, B, C, B2, t, q1, apb;
  point_motion_dwithin_coeffs(a0, a1, b0, b1, ndims, ra0, ra1, rb0, rb1, d, 0.0,
    &A, &B, &C);
  if (start)
    return C.sign == 0 && (B.sign > 0 || (B.sign == 0 && A.sign > 0));
  wide_dyadic_set_int(&t, 2);
  wide_dyadic_mul(&B, &t, &B2);
  wide_dyadic_add(&A, &B2, &q1);
  wide_dyadic_accumulate(&q1, &C);
  wide_dyadic_add(&A, &B, &apb);
  return q1.sign == 0 && (apb.sign < 0 || (apb.sign == 0 && A.sign > 0));
}

/**
 * @brief Return the sign of the least, over a segment, of the gap between a
 * disc moving linearly and a point, less a distance `d + dlo`, exactly
 * @details The gap `|w(f)| - L(f)` and the quadratic `q(f)` of
 * #point_motion_dwithin_coeffs have the same sign at every fraction, `L`
 * being not negative, so the sign of the least gap is that of the least of
 * `q` over [0, 1]: at an end, or at the vertex `-B / A` where the parabola is
 * convex and the vertex lies inside, there `(A C - B^2) / A`
 */
typedef struct
{
  WideDyadic A;     /**< A, which no distance changes */
  WideDyadic B0;    /**< B at the distance 0 */
  WideDyadic C0;    /**< C at the distance 0 */
  WideDyadic R1;    /**< The radius at the start, L0 at the distance 0 */
  WideDyadic DR;    /**< The change of the radius, dL */
} PointMotionGap;

/**
 * @brief Set the parts of the quadratic of #point_motion_dwithin_coeffs for a
 * disc moving against a point that no distance changes, so that each
 * distance a search asks costs two products (#point_motion_gap_sign)
 */
static void
point_motion_gap_init(PointMotionGap *g, const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2)
{
  point_motion_dwithin_coeffs(c1, c2, p, p, ndims, r1, r2, 0.0, 0.0, 0.0,
    0.0, &g->A, &g->B0, &g->C0);
  wide_dyadic_set_double(&g->R1, r1);
  g->DR = g->R1;
  g->DR.sign = - g->DR.sign;
  wide_dyadic_accumulate_double(&g->DR, r2, 1);
  return;
}

/**
 * @brief Return the sign of the least, over a segment, of the gap between a
 * disc moving linearly and a point, less a distance `d + dlo`, exactly, from
 * the parts of #point_motion_gap_init: with `L = d + dlo` the distance adds
 * `L` to `L0` and nothing to `dL`, so `B = B0 - L dL` and
 * `C = C0 - L (2 r1 + L)`
 * @details As #point_motion_gap_sign
 */
static int
point_motion_gap_sign_at(const PointMotionGap *g, double d, double dlo)
{
  WideDyadic L, t, t2, B, C, B2, q1, apb, bb, ac, v, two;
  wide_dyadic_set_double(&L, d);
  wide_dyadic_accumulate_double(&L, dlo, 1);
  wide_dyadic_mul(&L, &g->DR, &t);
  t.sign = - t.sign;
  wide_dyadic_add(&g->B0, &t, &B);
  wide_dyadic_set_int(&two, 2);
  wide_dyadic_mul(&g->R1, &two, &t);
  wide_dyadic_add(&t, &L, &t2);
  wide_dyadic_mul(&L, &t2, &t);
  t.sign = - t.sign;
  wide_dyadic_add(&g->C0, &t, &C);
  const WideDyadic *A = &g->A;
  wide_dyadic_mul(&B, &two, &B2);
  wide_dyadic_add(A, &B2, &q1);
  wide_dyadic_accumulate(&q1, &C);
  int sign = (C.sign < q1.sign) ? C.sign : q1.sign;
  wide_dyadic_add(A, &B, &apb);
  if (A->sign > 0 && B.sign < 0 && apb.sign > 0)
  {
    wide_dyadic_mul(A, &C, &ac);
    wide_dyadic_mul(&B, &B, &bb);
    bb.sign = - bb.sign;
    wide_dyadic_add(&ac, &bb, &v);
    if (v.sign < sign)
      sign = v.sign;
  }
  return sign;
}

static int
point_motion_gap_sign(const double *c1, const double *c2, const double *p,
  int ndims, double r1, double r2, double d, double dlo)
{
  WideDyadic A, B, C, B2, t, q1, apb, bb, ac, v;
  point_motion_dwithin_coeffs(c1, c2, p, p, ndims, r1, r2, 0.0, 0.0, d, dlo,
    &A, &B, &C);
  wide_dyadic_set_int(&t, 2);
  wide_dyadic_mul(&B, &t, &B2);
  wide_dyadic_add(&A, &B2, &q1);
  wide_dyadic_accumulate(&q1, &C);
  int sign = (C.sign < q1.sign) ? C.sign : q1.sign;
  wide_dyadic_add(&A, &B, &apb);
  if (A.sign > 0 && B.sign < 0 && apb.sign > 0)
  {
    wide_dyadic_mul(&A, &C, &ac);
    wide_dyadic_mul(&B, &B, &bb);
    bb.sign = - bb.sign;
    wide_dyadic_add(&ac, &bb, &v);
    if (v.sign < sign)
      sign = v.sign;
  }
  return sign;
}

/**
 * @brief Return the sign of #point_motion_gap_sign where the filters of
 * #polynomial_sign_filter carry it, and 2 where they cannot tell
 * @details The quadratic `q(f) = A f^2 + 2 B f + C` of
 * #point_motion_dwithin_coeffs, for a disc moving against a point, is read as
 * polynomials in exact sums of the input doubles: `u = c1 - p` and
 * `v = c2 - c1` in each coordinate, `L0 = r1 + d + dlo` and `dL = r2 - r1`,
 * with `A = v . v - dL^2`, `B = u . v - L0 dL` and `C = u . u - L0^2`. The
 * least of `q` is the least of `C` and `A + 2 B + C`, and of the value at the
 * vertex where `A > 0`, `B < 0` and `A + B > 0`, whose sign is that of
 * `A C - B^2 = |u|^2 |v|^2 - (u . v)^2 - L0^2 |v|^2 - dL^2 |u|^2 +
 * 2 L0 dL (u . v)`, the terms of the two first products in one coordinate
 * cancelling. Each sign is decided as #polynomial_sign_exact decides it but
 * for the expansions, so that the WideDyadic route of #point_motion_gap_sign
 * is read only for the signs the doubles and their pairs leave open
 */
static int
point_motion_gap_sign_filter(const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2, double d, double dlo)
{
  /* u: 0 .. ndims - 1, v: ndims .. 2 ndims - 1, L0: 2 ndims, dL: 2 ndims + 1 */
  ExactSum sums[8];
  for (int i = 0; i < ndims; i++)
  {
    const double tu[2] = {c1[i], - p[i]}, tv[2] = {c2[i], - c1[i]};
    exact_sum_set(&sums[i], tu, 2);
    exact_sum_set(&sums[ndims + i], tv, 2);
  }
  const int L = 2 * ndims, R = 2 * ndims + 1;
  const double tl[3] = {r1, d, dlo}, tr[2] = {r2, - r1};
  exact_sum_set(&sums[L], tl, 3);
  exact_sum_set(&sums[R], tr, 2);

  PolyTerm t[21];
  int n = 0;
  /* C = u . u - L0^2 */
  for (int i = 0; i < ndims; i++)
    t[n++] = (PolyTerm) {1.0, 2, {i, i}};
  t[n++] = (PolyTerm) {-1.0, 2, {L, L}};
  int sc = polynomial_sign_filter(sums, t, n);
  if (sc == 0)
    return 2;
  if (sc < 0)
    return -1;
  /* A + 2 B + C */
  n = 0;
  for (int i = 0; i < ndims; i++)
  {
    t[n++] = (PolyTerm) {1.0, 2, {ndims + i, ndims + i}};
    t[n++] = (PolyTerm) {2.0, 2, {i, ndims + i}};
    t[n++] = (PolyTerm) {1.0, 2, {i, i}};
  }
  t[n++] = (PolyTerm) {-1.0, 2, {R, R}};
  t[n++] = (PolyTerm) {-2.0, 2, {L, R}};
  t[n++] = (PolyTerm) {-1.0, 2, {L, L}};
  int s1 = polynomial_sign_filter(sums, t, n);
  if (s1 == 0)
    return 2;
  if (s1 < 0)
    return -1;
  /* Both ends are clear: the vertex decides where it lies inside */
  n = 0;
  for (int i = 0; i < ndims; i++)
    t[n++] = (PolyTerm) {1.0, 2, {ndims + i, ndims + i}};
  t[n++] = (PolyTerm) {-1.0, 2, {R, R}};
  int sa = polynomial_sign_filter(sums, t, n);
  if (sa == 0)
    return 2;
  if (sa < 0)
    return 1;
  n = 0;
  for (int i = 0; i < ndims; i++)
    t[n++] = (PolyTerm) {1.0, 2, {i, ndims + i}};
  t[n++] = (PolyTerm) {-1.0, 2, {L, R}};
  int sb = polynomial_sign_filter(sums, t, n);
  if (sb == 0)
    return 2;
  if (sb > 0)
    return 1;
  /* A + B */
  n = 0;
  for (int i = 0; i < ndims; i++)
  {
    t[n++] = (PolyTerm) {1.0, 2, {ndims + i, ndims + i}};
    t[n++] = (PolyTerm) {1.0, 2, {i, ndims + i}};
  }
  t[n++] = (PolyTerm) {-1.0, 2, {R, R}};
  t[n++] = (PolyTerm) {-1.0, 2, {L, R}};
  int sab = polynomial_sign_filter(sums, t, n);
  if (sab == 0)
    return 2;
  if (sab < 0)
    return 1;
  /* A C - B^2 */
  n = 0;
  for (int i = 0; i < ndims; i++)
    for (int j = 0; j < ndims; j++)
    {
      if (i == j)
        continue;
      t[n++] = (PolyTerm) {1.0, 4, {ndims + i, ndims + i, j, j}};
      t[n++] = (PolyTerm) {-1.0, 4, {i, ndims + i, j, ndims + j}};
    }
  for (int i = 0; i < ndims; i++)
  {
    t[n++] = (PolyTerm) {-1.0, 4, {L, L, ndims + i, ndims + i}};
    t[n++] = (PolyTerm) {-1.0, 4, {R, R, i, i}};
    t[n++] = (PolyTerm) {2.0, 4, {L, R, i, ndims + i}};
  }
  int sv = polynomial_sign_filter(sums, t, n);
  if (sv == 0)
    return 2;
  return (sv < 0) ? -1 : 1;
}

/**
 * @brief Return true if a disc moving linearly over a segment is certainly
 * never within a distance of a point, false where it is or doubles cannot
 * tell (#point_motion_dwithin_period_filter)
 */
bool
point_motion_dwithin_never(const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2, double d)
{
  return point_motion_dwithin_period_filter(c1, c2, p, p, ndims, r1, r2, 0.0,
    0.0, d) == 0;
}

/**
 * @brief Return 1 or 0 where a disc moving linearly over a segment is
 * certainly within a distance of a point at some instant or certainly never
 * within it, -1 where doubles cannot tell
 * (#point_motion_dwithin_period_filter)
 */
static int
point_motion_dwithin_ever_filter(const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2, double d)
{
  int filter = point_motion_dwithin_period_filter(c1, c2, p, p, ndims, r1,
    r2, 0.0, 0.0, d);
  return (filter == 1 || filter == 2) ? 1 : filter;
}

/**
 * @brief Return an estimate of the least distance of a disc moving linearly
 * to a point, less the radius, not below 0
 * @details The least of the ends and of the stationary point of
 * `|u + f v| - (r1 + f k)`, inside where `|k| < |v|`, read in extended
 * precision, so that a search starts within a unit or two in the last place
 * of the answer and the filter answers its steps but the last ones. An
 * estimate only: #point_motion_point_offset_exact and
 * #point_motion_point_offset_bounds decide from it by exact signs
 */
static double
point_motion_offset_estimate(const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2)
{
  long double uu = 0.0L, uv = 0.0L, vv = 0.0L, ee = 0.0L, cr = 0.0L;
  for (int i = 0; i < ndims; i++)
  {
    long double u = (long double) c1[i] - p[i];
    long double w = (long double) c2[i] - c1[i];
    long double e = (long double) c2[i] - p[i];
    uu += u * u; uv += u * w; vv += w * w; ee += e * e;
  }
  /* |u x v|, componentwise in the plane and by Lagrange's identity beyond */
  if (ndims == 2)
    cr = fabsl(((long double) c1[0] - p[0]) * ((long double) c2[1] - c1[1]) -
      ((long double) c1[1] - p[1]) * ((long double) c2[0] - c1[0]));
  else
    cr = sqrtl(fmaxl(uu * vv - uv * uv, 0.0L));
  long double k = (long double) r2 - r1;
  long double est = fminl(sqrtl(uu) - r1, sqrtl(ee) - r2);
  if (vv > k * k)
  {
    long double sd = sqrtl(vv - k * k);
    long double ts = (- uv + k * cr / sd) / vv;
    if (ts > 0.0L && ts < 1.0L)
      est = fminl(est, (cr * sd + k * uv) / vv - r1);
  }
  double c = (double) est;
  if (! (c > 0.0))
    c = 0.0;
  if (isinf(c))
    c = DBL_MAX;
  return c;
}

/**
 * @brief Bound in doubles the least distance of a disc moving linearly to a
 * point, less the radius
 * @details Where the disc stands clear of the point, the bounds bracket the
 * estimate of #point_motion_offset_estimate: the disc is within the upper
 * bound of the point at some instant and never within the lower one, each
 * decided by the filter of #point_motion_dwithin_period_filter, which answers
 * only where it is certain, so the exact least lies above the lower bound and
 * at most at the upper one, and the double nearest it, rounding being
 * monotone, between them. The bracket widens three times before it declines,
 * so that a search ranks the distance without its exact value
 * @param[in] c1,c2 Centre of the disc at the start and at the end
 * @param[in] p The point
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r1,r2 Radius at the start and at the end, not negative
 * @param[out] lo,hi The bounds, lo positive
 * @return false where the filter cannot certify that the disc stands clear of
 * the point or cannot certify a bracket, which leaves the distance to
 * #point_motion_point_offset_exact
 */
bool
point_motion_point_offset_bounds(const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2, double *lo, double *hi)
{
  if (point_motion_dwithin_ever_filter(c1, c2, p, ndims, r1, r2, 0.0) != 0)
    return false;
  double c = point_motion_offset_estimate(c1, c2, p, ndims, r1, r2);
  if (! (c > 0.0) || c == DBL_MAX)
    return false;
  double m = Max(r1, r2);
  for (int i = 0; i < ndims; i++)
    m = Max(m, Max(Max(fabs(c1[i]), fabs(c2[i])), fabs(p[i])));
  double delta = 0x1p-40 * (c + m);
  for (int k = 0; k < 3; k++, delta *= 0x1p8)
  {
    double l = c - delta, h = c + delta;
    if (l <= 0.0 ||
        point_motion_dwithin_ever_filter(c1, c2, p, ndims, r1, r2, h) != 1 ||
        point_motion_dwithin_ever_filter(c1, c2, p, ndims, r1, r2, l) != 0)
      continue;
    *lo = l;
    *hi = h;
    return true;
  }
  return false;
}

/**
 * @brief Return the least distance of a disc moving linearly to a point, less
 * the radius, the double nearest its exact value, or 0 where the disc reaches
 * the point
 * @details The disc moves from @p c1 with radius @p r1 to @p c2 with radius
 * @p r2. Its least distance less the radius exceeds a value `x` exactly where
 * it is never within `x` of the point, the sign of
 * #point_motion_gap_sign at the distance `x`, a quadratic in rationals of the
 * input doubles. The answer is the smallest double whose upper midpoint the
 * least does not pass, the midpoint being the sum of the double and half the
 * gap to the next one, and a tie going to the even double, found by a gallop
 * from the double estimate of its closed form and halving the bracket, as
 * #wide_dyadic_nearest_root finds its answer, so no root of a rounded value
 * decides it
 * @param[in] c1,c2 Centre of the disc at the start and at the end
 * @param[in] p The point
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r1,r2 Radius at the start and at the end, not negative
 */
double
point_motion_point_offset_exact(const double *c1, const double *c2,
  const double *p, int ndims, double r1, double r2)
{
  /* The disc reaches the point where it is ever within 0 of it, which the
   * filter tells but near a contact */
  int reach = point_motion_dwithin_ever_filter(c1, c2, p, ndims, r1, r2, 0.0);
  if (reach < 0)
  {
    int s = point_motion_gap_sign_filter(c1, c2, p, ndims, r1, r2, 0.0, 0.0);
    if (s == 2)
      s = point_motion_gap_sign(c1, c2, p, ndims, r1, r2, 0.0, 0.0);
    reach = (s <= 0) ? 1 : 0;
  }
  if (reach == 1)
    return 0.0;
  double c = point_motion_offset_estimate(c1, c2, p, ndims, r1, r2);
  uint64_t bits;
  memcpy(&bits, &c, sizeof(bits));
  /* The WideDyadic parts of the quadratic, set at the first side the filters
   * of #point_motion_gap_sign_filter leave open */
  PointMotionGap gap;
  bool gapset = false;
#define PMO_BEYOND(bb, res) \
  do { \
    double cc, nn; \
    uint64_t b1 = (bb), b2 = (bb) + 1; \
    memcpy(&cc, &b1, sizeof(cc)); \
    memcpy(&nn, &b2, sizeof(nn)); \
    /* Beyond where certainly never within the upper neighbour, and not \
     * where certainly within the double itself: the filter answers every \
     * step but those next to the answer */ \
    if (! isinf(nn) && point_motion_dwithin_ever_filter(c1, c2, p, ndims, \
        r1, r2, nn) == 0) \
    { \
      (res) = true; \
      break; \
    } \
    if (point_motion_dwithin_ever_filter(c1, c2, p, ndims, r1, r2, cc) == 1) \
    { \
      (res) = false; \
      break; \
    } \
    double hh = isinf(nn) ? ldexp(1.0, 970) : (nn - cc) / 2.0; \
    int s = point_motion_gap_sign_filter(c1, c2, p, ndims, r1, r2, cc, hh); \
    if (s == 2) \
    { \
      if (! gapset) \
      { \
        point_motion_gap_init(&gap, c1, c2, p, ndims, r1, r2); \
        gapset = true; \
      } \
      s = point_motion_gap_sign_at(&gap, cc, hh); \
    } \
    (res) = s > 0 || (s == 0 && (b1 & 1) != 0); \
  } while (0)
  uint64_t lo, hi;
  bool beyond;
  PMO_BEYOND(bits, beyond);
  if (beyond)
  {
    /* The answer is above: lo is beyond, hi is not */
    uint64_t step = 1;
    lo = bits;
    const uint64_t inf_bits = 0x7FF0000000000000ULL;
    while (true)
    {
      hi = (lo + step < inf_bits) ? lo + step : inf_bits;
      if (hi == inf_bits)
        break;
      PMO_BEYOND(hi, beyond);
      if (! beyond)
        break;
      lo = hi;
      step *= 2;
    }
  }
  else
  {
    /* The answer is at or below: hi is not beyond, lo is beyond or zero */
    uint64_t step = 1;
    hi = bits;
    while (true)
    {
      if (hi == 0)
        return 0.0;
      lo = (hi > step) ? hi - step : 0;
      PMO_BEYOND(lo, beyond);
      if (beyond)
        break;
      hi = lo;
      if (lo == 0)
        return 0.0;
      step *= 2;
    }
  }
  while (hi - lo > 1)
  {
    uint64_t mid = lo + (hi - lo) / 2;
    PMO_BEYOND(mid, beyond);
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
#undef PMO_BEYOND
  double result;
  memcpy(&result, &hi, sizeof(result));
  return result;
}

/**
 * @brief Return the sign of the squared distance of a point to the line of a
 * segment, times the squared length of the segment, less the square of
 * `r + c + g/2` times that squared length, computed exactly for any finite
 * doubles
 * @details This is the side of a midpoint #point_segment_distance_offset_exact
 * decides, `|w x u|^2 - m^2 (u . u)` with `w = p - a`, `u = b - a` and
 * `m = r + c + g/2`, read on the input coordinates where scaling them would
 * round a value or leave a product of four outside the doubles. By Lagrange's
 * identity `|w x u|^2 = (w . w)(u . u) - (w . u)^2`, in two coordinates as in
 * three, so the value is `(u . u)(w . w - m^2) - (w . u)^2`. The squared
 * length `u . u`, the difference `w . w - m^2` and the dot product `w . u`
 * are each a sum of products of two input doubles, an integer
 * #square_products_wide sets as #point_distance_offset_side_sign sets its
 * terms, and the two products are compared on their exact values
 * (#wide_product), the two having the weight of the same lowest bit
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius
 * @param[in] c Candidate
 * @param[in] g Gap from the candidate to its upper neighbour
 */
static int
point_segment_side_sign_inputs(const double *p, const double *a,
  const double *b, int ndims, double r, double c, double g)
{
  /* At most 15 products, each of a weight from 1/4 to 2, which the integer
   * of #square_products_wide holds */
  double left[15], right[15], factor[15];
  uint64_t wm[WIDE_LIMBS], len[WIDE_LIMBS], dot[WIDE_LIMBS];
  /* w . w - m^2 */
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = p[i]; right[n] = p[i]; factor[n++] = 1.0;
    left[n] = p[i]; right[n] = a[i]; factor[n++] = -2.0;
    left[n] = a[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  left[n] = r; right[n] = r; factor[n++] = -1.0;
  left[n] = c; right[n] = c; factor[n++] = -1.0;
  left[n] = g; right[n] = g; factor[n++] = -0.25;
  left[n] = r; right[n] = c; factor[n++] = -2.0;
  left[n] = r; right[n] = g; factor[n++] = -1.0;
  left[n] = c; right[n] = g; factor[n++] = -1.0;
  square_products_wide(left, right, factor, n, wm);
  /* u . u */
  n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = b[i]; right[n] = b[i]; factor[n++] = 1.0;
    left[n] = b[i]; right[n] = a[i]; factor[n++] = -2.0;
    left[n] = a[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  square_products_wide(left, right, factor, n, len);
  /* w . u */
  n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = p[i]; right[n] = b[i]; factor[n++] = 1.0;
    left[n] = p[i]; right[n] = a[i]; factor[n++] = -1.0;
    left[n] = a[i]; right[n] = b[i]; factor[n++] = -1.0;
    left[n] = a[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  square_products_wide(left, right, factor, n, dot);

  uint64_t mwm[WIDE_LIMBS], mlen[WIDE_LIMBS], mdot[WIDE_LIMBS];
  int swm = wide_magnitude(wm, mwm);
  int slen = wide_magnitude(len, mlen);
  int sdot = wide_magnitude(dot, mdot);
  /* The square of the dot product is not negative; the other product carries
   * the signs of its factors */
  int slhs = slen * swm;
  if (slhs < 0)
    return -1;
  if (slhs == 0)
    return (sdot != 0) ? -1 : 0;
  uint32_t lhs[4 * WIDE_LIMBS], rhs[4 * WIDE_LIMBS];
  wide_product(mlen, mwm, lhs);
  wide_product(mdot, mdot, rhs);
  for (int i = 4 * WIDE_LIMBS - 1; i >= 0; i--)
  {
    if (lhs[i] != rhs[i])
      return (lhs[i] > rhs[i]) ? 1 : -1;
  }
  return 0;
}

/**
 * @brief Return true where the distance of a point to the interior of a
 * segment, less a radius, lies beyond the upper midpoint of a double, read on
 * the input coordinates
 * @details The twin of #point_segment_beyond by
 * #point_segment_side_sign_inputs; past the largest double the gap is the one
 * below it, 2^971
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r,c,n Radius, candidate and its upper neighbour
 * @param[in] odd Whether the last bit of the candidate is odd
 */
static bool
point_segment_beyond_inputs(const double *p, const double *a, const double *b,
  int ndims, double r, double c, double n, bool odd)
{
  double g = isinf(n) ? ldexp(1.0, 971) : n - c;
  int sign = point_segment_side_sign_inputs(p, a, b, ndims, r, c, g);
  return sign > 0 || (sign == 0 && odd);
}

/**
 * @brief Return the sign of `(p - s) . (b - a)`, computed exactly for any
 * finite doubles
 * @details A sum of products of two input doubles (#square_products_sign):
 * with `s = a` it is the sign of `w . u`, with `s = b` that of
 * `w . u - u . u`, which decide whether the nearest point of the segment is an
 * end. As #ring_area_sign, the product taken in floating point, on the rounded
 * differences, answers wherever it stands clear of the bound its rounding
 * reaches: each difference is within a half unit in its last place, and the
 * products and their sum add as many of the running magnitude, which twice
 * DBL_EPSILON of it bounds generously, and DBL_MIN covers an underflow. A
 * difference rounds to 0 only where it is 0, so a product of one is exactly 0.
 * Within the bound, the sign is that of the exact sum
 */
static int
point_segment_dot_sign_inputs(const double *p, const double *s,
  const double *a, const double *b, int ndims)
{
  bool zero = true;
  double dot = 0.0, mag = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double w = p[i] - s[i], v = b[i] - a[i];
    if (w != 0.0 && v != 0.0)
      zero = false;
    double t = w * v;
    dot += t;
    mag += fabs(t);
  }
  if (zero)
    return 0;
  double bound = 4.0 * DBL_EPSILON * mag + DBL_MIN;
  if (isfinite(bound))
  {
    if (dot > bound)
      return 1;
    if (dot < - bound)
      return -1;
  }
  double left[12] = {0.0}, right[12] = {0.0}, factor[12] = {0.0};
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    left[n] = p[i]; right[n] = b[i]; factor[n++] = 1.0;
    left[n] = p[i]; right[n] = a[i]; factor[n++] = -1.0;
    left[n] = s[i]; right[n] = b[i]; factor[n++] = -1.0;
    left[n] = s[i]; right[n] = a[i]; factor[n++] = 1.0;
  }
  return square_products_sign(left, right, factor, n);
}

/**
 * @brief Return the side of the position of a point along a segment, as
 * #linesegm_param reads it: -1 where it reads 0, 1 where it reads 1, 0 inside
 */
static int
linesegm_param_side(const double *a, const double *b, const double *p)
{
  if (point_segment_dot_sign_inputs(p, a, a, b, 2) <= 0)
    return -1;
  if (point_segment_dot_sign_inputs(p, b, a, b, 2) >= 0)
    return 1;
  return 0;
}

/**
 * @brief Return the sign of the difference of the positions of two points
 * along the segment AB, the positions #linesegm_param reads, compared exactly
 * @details A position is the projection of the point on the segment over its
 * squared length, clamped to [0, 1]. It reads 0 exactly where
 * `(p - a) . (b - a) <= 0` and 1 where `(p - b) . (b - a) >= 0`, and two
 * positions inside compare as `(p - q) . (b - a)` does, since they share the
 * positive squared length; each is the sign of a sum of products of two input
 * doubles (#point_segment_dot_sign_inputs), exact for any finite doubles, so
 * no rounded quotient decides the order
 * @param[in] a,b The two ends of the segment, distinct
 * @param[in] p,q The two points
 * @return -1, 0 or 1 as the position of `p` is before, at or after that of
 * `q`
 */
int
linesegm_param_cmp(const double *a, const double *b, const double *p,
  const double *q)
{
  int sp = linesegm_param_side(a, b, p), sq = linesegm_param_side(a, b, q);
  if (sp != 0 || sq != 0)
    return (sp > sq) ? 1 : ((sp < sq) ? -1 : 0);
  return point_segment_dot_sign_inputs(p, q, a, b, 2);
}

/**
 * @brief Return the distance between a point and a segment less a radius, not
 * below zero, as the double nearest it, every sign read on the input
 * coordinates
 * @details The path of #point_segment_distance_offset_exact where scaling the
 * coordinates would round a value or leave a product of four of them outside
 * the doubles, or where a difference overflows and a quarter of a coordinate
 * would round. Whether the nearest point is an end is the sign
 * #point_segment_dot_sign_inputs reads, an end is a pair of points
 * (#point_distance_offset_exact), and for the foot the answer is the smallest
 * double whose upper midpoint the distance less the radius does not pass
 * (#point_segment_beyond_inputs), found by the galloping search of
 * #point_segment_distance_offset_exact from a rounded start
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius, not negative
 */
static double
point_segment_distance_offset_inputs(const double *p, const double *a,
  const double *b, int ndims, double r)
{
  if (point_segment_dot_sign_inputs(p, a, a, b, ndims) <= 0)
    return point_distance_offset_exact(p, a, ndims, r);
  if (point_segment_dot_sign_inputs(p, b, a, b, ndims) >= 0)
    return point_distance_offset_exact(p, b, ndims, r);

  /* First candidate, a start only, from the halves of the differences scaled
   * by 2^-hexp, which brings the largest into [1/2, 1), so that neither the
   * cross product nor the length overflows or vanishes */
  double hw[3], hu[3], maxh = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    hw[i] = p[i] * 0.5 - a[i] * 0.5;
    hu[i] = b[i] * 0.5 - a[i] * 0.5;
    maxh = fmax(maxh, fmax(fabs(hw[i]), fabs(hu[i])));
  }
  int hexp;
  (void) frexp(maxh, &hexp);
  double w[3], u[3], ul2 = 0.0, x2 = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    w[i] = ldexp(hw[i], - hexp);
    u[i] = ldexp(hu[i], - hexp);
    ul2 += u[i] * u[i];
  }
  for (int i = 0; i < ndims; i++)
    for (int j = i + 1; j < ndims; j++)
    {
      double x = w[i] * u[j] - w[j] * u[i];
      x2 += x * x;
    }
  double ul = sqrt(ul2);
  double c = (ul > 0.0) ? ldexp(sqrt(x2) / ul, 1 + hexp) - r : 0.0;
  if (! (c > 0.0))
    c = 0.0;
  if (isinf(c))
    c = DBL_MAX;
  uint64_t bits;
  memcpy(&bits, &c, sizeof(bits));

  /* The answer is the smallest bit pattern that is not beyond: gallop from the
   * candidate to bracket it, then halve the bracket */
#define PSD_BEYOND_INPUTS(bb, res) \
  do { \
    double cc, nn; \
    uint64_t b1 = (bb), b2 = (bb) + 1; \
    memcpy(&cc, &b1, sizeof(cc)); \
    memcpy(&nn, &b2, sizeof(nn)); \
    (res) = point_segment_beyond_inputs(p, a, b, ndims, r, cc, nn, \
      (b1 & 1) != 0); \
  } while (0)
  uint64_t lo, hi;
  bool beyond;
  PSD_BEYOND_INPUTS(bits, beyond);
  if (beyond)
  {
    /* The answer is above: lo is beyond, hi is not */
    uint64_t step = 1;
    lo = bits;
    const uint64_t inf_bits = 0x7FF0000000000000ULL;
    while (true)
    {
      hi = (lo + step < inf_bits) ? lo + step : inf_bits;
      if (hi == inf_bits)
        break;
      PSD_BEYOND_INPUTS(hi, beyond);
      if (! beyond)
        break;
      lo = hi;
      step *= 2;
    }
  }
  else
  {
    /* The answer is at or below: hi is not beyond, lo is beyond or zero */
    uint64_t step = 1;
    hi = bits;
    while (true)
    {
      if (hi == 0)
        return 0.0;
      lo = (hi > step) ? hi - step : 0;
      PSD_BEYOND_INPUTS(lo, beyond);
      if (beyond)
        break;
      hi = lo;
      if (lo == 0)
        return 0.0;
      step *= 2;
    }
  }
  /* lo is beyond, hi is not, and the answer is the least not beyond */
  while (hi - lo > 1)
  {
    uint64_t mid = lo + (hi - lo) / 2;
    PSD_BEYOND_INPUTS(mid, beyond);
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
#undef PSD_BEYOND_INPUTS
  double result;
  memcpy(&result, &hi, sizeof(result));
  return result;
}

/**
 * @brief Return true if the scaled values of
 * #point_segment_distance_offset_exact keep every product of four of them
 * exact
 * @details A product of four values whose lowest bits are at least `L` has its
 * lowest bit at least `4 L`, a double where that is not below 2^-1074, so
 * every value scaled by 2^k must keep its lowest bit at or above 2^-268. Every
 * scaled value is below 4, so no product of four overflows
 * @param[in] v Values before the scaling, zeros left out
 * @param[in] n Number of values
 * @param[in] k Exponent of the scaling (#point_distance_scale)
 */
static bool
point_segment_scale_exact(const double *v, int n, int k)
{
  for (int i = 0; i < n; i++)
  {
    if (v[i] != 0.0 && double_lowest_bit(v[i]) + k < -268)
      return false;
  }
  return true;
}

/**
 * @brief Return true if a quarter of a coordinate of the point or of the
 * segment, or of the radius, rounds, its lowest bit falling below 2^-1074
 * @details #point_segment_distance_offset_exact answers a difference beyond
 * the largest double on the quarters of its values, which is exact otherwise
 */
static bool
point_segment_quarter_rounds(const double *p, const double *a,
  const double *b, int ndims, double r)
{
  double v[10];
  int n = 0;
  for (int i = 0; i < ndims; i++)
  {
    v[n++] = p[i]; v[n++] = a[i]; v[n++] = b[i];
  }
  v[n++] = r;
  for (int i = 0; i < n; i++)
  {
    if (v[i] != 0.0 && double_lowest_bit(v[i]) - 2 < -1074)
      return true;
  }
  return false;
}

/**
 * @brief Return the sign of `Q - (a + b1 + b2)^2 L`, or 0 where a filter on
 * doubles cannot tell, setting in @p value the double it reads
 * @details The twin of #point_distance_offset_filter_sign for a point and a
 * segment: `Q` is the squared cross product of the point less the segment's
 * start and the segment, held as `qs + qlo` within @p eq, `qmag` the
 * magnitude of the terms of `qlo`, and `L` the squared length of the segment,
 * held as `ls + llo` within @p el. The square of `a` is split by #two_square
 * and `a*a*ls` by #two_product, so the value is
 * `(qs - q) + (qlo - qe - a^2 llo - P (ls + llo))`, with
 * `P = ya + 2ab + b^2` and `b` the rounded `b1 + b2`. The first difference is
 * exact where one term is within a factor of two of the other (Sterbenz).
 * The bound adds the error that `eq` and `el` carry, the rounding of `b`
 * (`|2(a + b)(b - b1 - b2)| <= 2 DBL_EPSILON (|ab| + b^2)` times `L`), and
 * the rounding of every product and sum, each below `DBL_EPSILON` times the
 * magnitudes it reads, with a factor of 32 over the count of operations, and
 * an absolute `4 DBL_MIN` for the products that underflow
 * @return 1 or -1 where the value stands clear of the bound, 0 otherwise,
 * never the sign of an exact tie, which the caller reads exactly
 */
static inline int
point_segment_offset_filter_sign(double qs, double qlo, double qmag,
  double eq, double ls, double llo, double el, double a, double b1, double b2,
  double *value)
{
  double xa, ya, q, qe;
  two_square(a, &xa, &ya);
  two_product(xa, ls, &q, &qe);
  double b = b1 + b2;
  double ab2 = 2.0 * a * b, bb = b * b;
  double pp = (ya + ab2) + bb;
  double t3 = xa * llo, t4 = pp * ls, t5 = pp * llo;
  double hi = qs - q;
  bool hiexact = (qs >= 0.5 * q && qs <= 2.0 * q);
  double v = hi + (((qlo - qe) - t3) - (t4 + t5));
  double lsum = ls + fabs(llo) + el;
  double bound = (hiexact ? 0.0 : DBL_EPSILON * fabs(hi)) + eq +
    (xa + fabs(pp)) * el * (1.0 + 4.0 * DBL_EPSILON) +
    4.0 * DBL_EPSILON * (fabs(ab2) + bb) * lsum +
    32.0 * DBL_EPSILON * (qmag + fabs(qe) + fabs(t3) +
      fabs(t4) + fabs(t5) + (fabs(ya) + fabs(ab2) + bb) * lsum + fabs(v)) +
    4.0 * DBL_MIN;
  *value = v;
  if (v > bound)
    return 1;
  if (v < - bound)
    return -1;
  return 0;
}

/**
 * @brief Set the cross product `w_i u_j - w_j u_i` of the plane of the
 * coordinates @p i and @p j as `xs + xlo` within @p ex
 * @details Each product is split by #two_product and the rounding errors of
 * the differences enter as their products with the other factor, within a
 * bound of 16 DBL_EPSILON of the magnitude of those terms
 */
static inline void
point_segment_cross_plane(const double *dw, const double *ew,
  const double *du, const double *eu, int i, int j, double *xs, double *xlo,
  double *ex)
{
  double p1, e1, p2, e2, t;
  two_product(dw[i], du[j], &p1, &e1);
  two_product(dw[j], du[i], &p2, &e2);
  two_sum(p1, - p2, xs, &t);
  double c1 = dw[i] * eu[j], c2 = ew[i] * du[j], c3 = dw[j] * eu[i],
    c4 = ew[j] * du[i], c5 = ew[i] * eu[j], c6 = ew[j] * eu[i];
  *xlo = (((t + e1) - e2) + ((c1 + c2) - (c3 + c4))) + (c5 - c6);
  *ex = 16.0 * DBL_EPSILON * (fabs(t) + fabs(e1) + fabs(e2) +
    fabs(c1) + fabs(c2) + fabs(c3) + fabs(c4) + fabs(c5) + fabs(c6)) +
    4.0 * DBL_MIN;
}

/**
 * @brief Answer the distance of a point to a segment less a radius on doubles
 * where a filter settles it, returning false where it does not
 * @details The first test of #point_segment_distance_offset_exact, the twin
 * of #point_distance_offset_filter. It reads the coordinate differences as
 * they are, where each largest one lies from 2^-120 to 2^120 and the radius
 * below 2^122, so that no product of four of them overflows. With `w` the
 * point less the segment's start and `u` the segment, each held exactly as a
 * rounded difference and its error, the signs of `w . u` and of
 * `w . u - u . u` are read in doubles against Shewchuk's bound, the error
 * terms of the differences adding at most 2^-52 of each product: an end is
 * answered by #point_distance_offset_exact, and the foot as follows. The
 * cross product of each plane of two coordinates (#point_segment_cross_plane)
 * and the squared length `L` are formed from #two_product and #two_square as
 * a double and the double of its small terms, and the squared cross product
 * `Q = |w x u|^2` is the square of the one plane in 2D, the sum of the squares
 * of the three in 3D, whose additions the bound of `Q` carries as `L`'s does.
 * The squared distance to the line is `Q / L`, so the answer is 0 where
 * `Q - r^2 L` is certainly not positive, and otherwise the double `c` for
 * which `Q - (r + c + h)^2 L` is positive at the lower midpoint and
 * negative at the upper one, each read by #point_segment_offset_filter_sign
 * with `r + c` split by #two_sum and `h` half a gap, exact for a candidate of
 * at least 2^-900. The candidate starts at `(Q - r^2 L) / (L (sqrt(Q) /
 * sqrt(L) + r))`, whose numerator keeps its precision however much the distance and
 * the radius cancel, and moves while a side certainly says so, at most four
 * times. Any sign the filter cannot tell, which every tie is, returns false,
 * and the exact search answers
 * @param[in] dw,ew,du,eu Differences of the point and of the segment's end
 * from its start, and the errors of their rounding
 * @param[in] maxw,maxu Largest absolute differences
 * @param[in] p,a,b The point and the two ends of the segment
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] r Radius, not negative
 * @param[out] result The answer where the function returns true
 */
static bool
point_segment_offset_filter(const double *dw, const double *ew,
  const double *du, const double *eu, double maxw, double maxu,
  const double *p, const double *a, const double *b, int ndims, double r,
  double *result)
{
  if (! (maxw >= 0x1p-120 && maxw <= 0x1p120 && maxu >= 0x1p-120 &&
      maxu <= 0x1p120 && r < 0x1p122))
    return false;
  /* The nearest point of the segment: the start, the end or the foot */
  double dmag = fabs(dw[0] * du[0]) + fabs(dw[1] * du[1]);
  double l = du[0] * du[0] + du[1] * du[1];
  double dot = dw[0] * du[0] + dw[1] * du[1];
  if (ndims == 3)
  {
    dmag += fabs(dw[2] * du[2]);
    l += du[2] * du[2];
    dot += dw[2] * du[2];
  }
  double dbound = 8.0 * DBL_EPSILON * dmag + 4.0 * DBL_MIN;
  if (dot < - dbound)
  {
    *result = point_distance_offset_exact(p, a, ndims, r);
    return true;
  }
  if (dot <= dbound)
    return false;
  double dl = dot - l;
  double dlbound = 8.0 * DBL_EPSILON * (dmag + l + fabs(dl)) + 4.0 * DBL_MIN;
  if (dl > dlbound)
  {
    *result = point_distance_offset_exact(p, b, ndims, r);
    return true;
  }
  if (dl >= - dlbound)
    return false;

  /* The foot. Q = |w x u|^2 as qs + qlo within eq, from the cross product
   * xs + xlo within ex of each plane */
  double xs, xlo, ex, qs, qlo, qmag, eq, xabs;
  point_segment_cross_plane(dw, ew, du, eu, 0, 1, &xs, &xlo, &ex);
  {
    double y, t1 = 2.0 * xs * xlo, t2 = xlo * xlo;
    two_square(xs, &qs, &y);
    qlo = (y + t1) + t2;
    qmag = (fabs(y) + fabs(t1)) + t2;
    eq = 2.0 * (fabs(xs) + fabs(xlo)) * ex + ex * ex;
    xabs = fabs(xs);
  }
  if (ndims == 3)
  {
    static const int plane[2][2] = {{0, 2}, {1, 2}};
    double qsum = 0.0;
    for (int k = 0; k < 2; k++)
    {
      point_segment_cross_plane(dw, ew, du, eu, plane[k][0], plane[k][1],
        &xs, &xlo, &ex);
      double x, y, s, t1 = 2.0 * xs * xlo, t2 = xlo * xlo;
      two_square(xs, &x, &y);
      two_sum(qs, x, &qs, &s);
      qlo += (s + y) + (t1 + t2);
      qmag += fabs(s) + fabs(y) + fabs(t1) + t2;
      eq += 2.0 * (fabs(xs) + fabs(xlo)) * ex + ex * ex;
      qsum += fabs(qlo);
    }
    eq += 32.0 * DBL_EPSILON * (qmag + qsum);
    xabs = sqrt(qs);
  }
  /* L = u . u as ls + llo within el */
  double ls = 0.0, llo = 0.0, lmag = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double x, y, s;
    two_square(du[i], &x, &y);
    two_sum(ls, x, &ls, &s);
    double de = 2.0 * du[i] * eu[i], ee = eu[i] * eu[i];
    llo += (s + y) + (de + ee);
    lmag += fabs(s) + fabs(y) + fabs(de) + ee;
  }
  double el = 32.0 * DBL_EPSILON * lmag + 4.0 * DBL_MIN;
  if (! (ls > 0.0))
    return false;

  /* The side of X^2 - r^2 L */
  double num;
  int side = point_segment_offset_filter_sign(qs, qlo, qmag, eq, ls, llo, el,
    r, 0.0, 0.0, &num);
  if (side == 0)
    return false;
  if (side < 0)
  {
    *result = 0.0;
    return true;
  }
  double c = num / (ls * (xabs / sqrt(ls) + r));
  for (int move = 0; move < 4; move++)
  {
    if (! (c >= 0x1p-900) || ! isfinite(c))
      return false;
    uint64_t bits;
    memcpy(&bits, &c, sizeof(bits));
    uint64_t ubits = bits + 1, dbits = bits - 1;
    double cu, cd;
    memcpy(&cu, &ubits, sizeof(cu));
    memcpy(&cd, &dbits, sizeof(cd));
    if (! isfinite(cu))
      return false;
    double m, m1e, v;
    two_sum(r, c, &m, &m1e);
    int su = point_segment_offset_filter_sign(qs, qlo, qmag, eq, ls, llo, el,
      m, m1e, 0.5 * (cu - c), &v);
    if (su > 0)
    {
      c = cu;
      continue;
    }
    int sd = point_segment_offset_filter_sign(qs, qlo, qmag, eq, ls, llo, el,
      m, m1e, - 0.5 * (c - cd), &v);
    if (sd < 0)
    {
      c = cd;
      continue;
    }
    if (su < 0 && sd > 0)
    {
      *result = c;
      return true;
    }
    return false;
  }
  return false;
}

/**
 * @brief A value held as the unevaluated sum of two doubles, with a bound on
 * its distance to the exact value it stands for, the filter of the 3D interior distance
 * @details The sum is formed by the error-free transforms #two_diff,
 * #two_product and #two_sum, so it carries about 104 bits on every platform
 * and under every emulator that rounds a double and a fused multiply-add
 * correctly
 */
typedef struct
{
  double hi;  /**< The value, rounded */
  double lo;  /**< What the rounding leaves out */
  double e;   /**< A bound on the error of hi + lo */
} DDBound;

/* The relative rounding of one operation on two-double values, with a wide
 * margin, and an absolute term for a product that underflows */
#define DDB_EPS 0x1p-100
#define DDB_TINY 0x1p-1060
/* The least magnitude of a product of two doubles that #two_product splits
 * exactly: below it the part that rounding leaves out may itself round */
#define DDB_EXACT_PRODUCT 0x1p-969

/**
 * @brief Set the three coordinates of `q - p`, exact as two doubles
 */
static pg_attribute_always_inline void
ddb_diff3(const double *q, const double *p, DDBound *r)
{
  for (int c = 0; c < 3; c++)
  {
    two_diff(q[c], p[c], &r[c].hi, &r[c].lo);
    r[c].e = 0.0;
  }
  return;
}

/**
 * @brief Return true if a bounded value is exactly zero: both doubles zero and
 * no error
 */
static pg_attribute_always_inline bool
ddb_is_zero(DDBound a)
{
  return a.hi == 0.0 && a.lo == 0.0 && a.e == 0.0;
}

/**
 * @brief Return true if a bounded value is a double held exactly
 */
static pg_attribute_always_inline bool
ddb_is_double(DDBound a)
{
  return a.lo == 0.0 && a.e == 0.0;
}

/**
 * @brief Return the product of two bounded values, with its bound
 * @details A product with an exact zero is an exact zero, and the product of
 * two doubles held exactly is exact as two doubles (#two_product)
 */
static pg_attribute_always_inline DDBound
ddb_mul(DDBound a, DDBound b)
{
  DDBound r;
  if (ddb_is_zero(a) || ddb_is_zero(b))
  {
    r.hi = r.lo = r.e = 0.0;
    return r;
  }
  if (ddb_is_double(a) && ddb_is_double(b))
  {
    two_product(a.hi, b.hi, &r.hi, &r.lo);
    /* A product that underflows is not split exactly */
    r.e = (fabs(r.hi) < DDB_EXACT_PRODUCT) ? DDB_TINY : 0.0;
    return r;
  }
  double p, pe;
  two_product(a.hi, b.hi, &p, &pe);
  pe += a.hi * b.lo + a.lo * b.hi;
  two_sum(p, pe, &r.hi, &r.lo);
  double ma = fabs(a.hi) + fabs(a.lo), mb = fabs(b.hi) + fabs(b.lo);
  r.e = ma * b.e + mb * a.e + a.e * b.e + fabs(a.lo * b.lo) +
    DDB_EPS * fabs(p) + DDB_TINY;
  return r;
}

/**
 * @brief Return the sum of two bounded values, with its bound
 */
static pg_attribute_always_inline DDBound
ddb_add(DDBound a, DDBound b)
{
  /* A sum with an exact zero is the other term, and the sum of two doubles
   * held exactly is exact as two doubles (#two_sum) */
  if (ddb_is_zero(b))
    return a;
  if (ddb_is_zero(a))
    return b;
  double s, se;
  two_sum(a.hi, b.hi, &s, &se);
  DDBound r;
  if (ddb_is_double(a) && ddb_is_double(b))
  {
    r.hi = s;
    r.lo = se;
    r.e = 0.0;
    return r;
  }
  se += a.lo + b.lo;
  two_sum(s, se, &r.hi, &r.lo);
  r.e = a.e + b.e + DDB_EPS * (fabs(a.hi) + fabs(b.hi)) + DDB_TINY;
  return r;
}

/**
 * @brief Return the difference of two bounded values, with its bound
 */
static pg_attribute_always_inline DDBound
ddb_sub(DDBound a, DDBound b)
{
  b.hi = - b.hi;
  b.lo = - b.lo;
  return ddb_add(a, b);
}

/**
 * @brief Return the dot product of two vectors of three bounded values
 * @details Where every component is a double held exactly, each product is
 * exact as two doubles (#two_product), as in #ddb_cross3, and only the sums
 * round
 */
static pg_attribute_always_inline DDBound
ddb_dot3(const DDBound *a, const DDBound *b)
{
  if (a[0].lo == 0.0 && a[0].e == 0.0 && a[1].lo == 0.0 && a[1].e == 0.0 &&
      a[2].lo == 0.0 && a[2].e == 0.0 && b[0].lo == 0.0 && b[0].e == 0.0 &&
      b[1].lo == 0.0 && b[1].e == 0.0 && b[2].lo == 0.0 && b[2].e == 0.0)
  {
    DDBound p[3];
    for (int c = 0; c < 3; c++)
    {
      two_product(a[c].hi, b[c].hi, &p[c].hi, &p[c].lo);
      p[c].e = (a[c].hi != 0.0 && b[c].hi != 0.0 &&
        fabs(p[c].hi) < DDB_EXACT_PRODUCT) ? DDB_TINY : 0.0;
    }
    return ddb_add(ddb_add(p[0], p[1]), p[2]);
  }
  return ddb_add(ddb_add(ddb_mul(a[0], b[0]), ddb_mul(a[1], b[1])),
    ddb_mul(a[2], b[2]));
}

/**
 * @brief Set the cross product of two vectors of three bounded values
 * @details Where every component is a double held exactly, as the difference
 * of two coordinates is wherever it does not round, each product is exact as
 * two doubles (#two_product) and only the sum of the two rounds, by the
 * bound #ddb_add gives it
 */
static pg_attribute_always_inline void
ddb_cross3(const DDBound *a, const DDBound *b, DDBound *r)
{
  bool exact = true;
  for (int c = 0; c < 3; c++)
    exact = exact && a[c].lo == 0.0 && a[c].e == 0.0 && b[c].lo == 0.0 &&
      b[c].e == 0.0;
  if (exact)
  {
    for (int c = 0; c < 3; c++)
    {
      int i = (c + 1) % 3, j = (c + 2) % 3;
      double p1, e1, p2, e2, s, se;
      two_product(a[i].hi, b[j].hi, &p1, &e1);
      two_product(a[j].hi, b[i].hi, &p2, &e2);
      /* Two exact products are equal where their pairs are, the pair of a
       * product being unique, and their difference is then an exact zero;
       * a product that underflows is not split exactly, so two of them say
       * nothing by being equal */
      bool under = (p1 != 0.0 && fabs(p1) < DDB_EXACT_PRODUCT) ||
        (p2 != 0.0 && fabs(p2) < DDB_EXACT_PRODUCT) ||
        (p1 == 0.0 && a[i].hi != 0.0 && b[j].hi != 0.0) ||
        (p2 == 0.0 && a[j].hi != 0.0 && b[i].hi != 0.0);
      if (! under && p1 == p2 && e1 == e2)
      {
        r[c].hi = r[c].lo = r[c].e = 0.0;
        continue;
      }
      two_diff(p1, p2, &s, &se);
      se += e1 - e2;
      two_sum(s, se, &r[c].hi, &r[c].lo);
      r[c].e = DDB_EPS * (fabs(p1) + fabs(p2)) + DDB_TINY;
    }
    return;
  }
  for (int c = 0; c < 3; c++)
  {
    int i = (c + 1) % 3, j = (c + 2) % 3;
    r[c] = ddb_sub(ddb_mul(a[i], b[j]), ddb_mul(a[j], b[i]));
  }
  return;
}

/**
 * @brief Set the triple product `(c - a) . ((b - a) x (d - c))` computed
 * exactly from the coordinates and rounded to two doubles
 * @details Each component of the cross product is the cross product of the two
 * vectors in a plane of two coordinates, whose exact expansion
 * #cross_product_exact sums; scaled by the two doubles of each difference
 * (#scale_expansion), added (#expansion_sum) and compressed
 * (#compress_expansion), the triple product keeps its precision relative to
 * itself where the segments nearly meet and the products of the rounded
 * differences of #ddb_cross3 cancel. The two largest components of the
 * compressed expansion, summed exactly, hold it to about twice the precision
 * of a double; an exact zero is held as one
 */
static void
ddb_triple3_points(const double *a, const double *b, const double *c,
  const double *d, DDBound *r)
{
  static const int plane[3][2] = {{1, 2}, {2, 0}, {0, 1}};
  double total[512], tmp[512], part[128], comp[512];
  int tlen = 0;
  for (int k = 0; k < 3; k++)
  {
    int i = plane[k][0], j = plane[k][1];
    double buf1[32], buf2[32];
    const double *n;
    int nlen = cross_product_expansion(a[i], a[j], b[i], b[j], c[i], c[j],
      d[i], d[j], buf1, buf2, &n);
    if (nlen == 0)
      continue;
    double eh, el;
    two_diff(c[k], a[k], &eh, &el);
    const double ev[2] = {eh, el};
    for (int h = 0; h < 2; h++)
    {
      if (ev[h] == 0.0)
        continue;
      int plen = scale_expansion(nlen, n, ev[h], part);
      if (tlen == 0)
      {
        memcpy(total, part, (size_t) plen * sizeof(double));
        tlen = plen;
      }
      else
        tlen = expansion_sum(tlen, total, plen, part, total, tmp);
    }
  }
  int clen = (tlen == 0) ? 0 : compress_expansion(tlen, total, comp);
  while (clen > 0 && comp[clen - 1] == 0.0)
    clen--;
  /* The expansions are exact where no product of three differences
   * underflows, which a difference below 2^-320 may make it do: there the
   * value holds the bound of an underflow, and no exact zero */
  double least = INFINITY;
  const double *from[3] = {a, c, a}, *to[3] = {b, d, c};
  for (int w = 0; w < 3; w++)
    for (int k = 0; k < 3; k++)
    {
      double dh, dl;
      two_diff(to[w][k], from[w][k], &dh, &dl);
      if (dh != 0.0)
        least = fmin(least, fabs(dh));
      if (dl != 0.0)
        least = fmin(least, fabs(dl));
    }
  double under = (least < 0x1p-320) ? DDB_TINY : 0.0;
  if (clen == 0)
  {
    r->hi = r->lo = 0.0;
    r->e = under;
    return;
  }
  if (clen == 1)
  {
    r->hi = comp[0];
    r->lo = 0.0;
    r->e = under;
    return;
  }
  double rest = 0.0;
  for (int i = 0; i < clen - 2; i++)
    rest += comp[i];
  two_sum(comp[clen - 1], comp[clen - 2], &r->hi, &r->lo);
  r->lo += rest;
  r->e = ((clen == 2) ? 0.0 : DDB_EPS * fabs(r->hi) + DDB_TINY) + under;
  return;
}

/**
 * @brief Return the sign of a bounded value where the bound decides it: 1 or
 * -1, or 0 where the value may lie on either side of zero
 * @details The bound is itself computed in doubles, hence the margin; a value
 * that is not finite decides nothing
 */
static pg_attribute_always_inline int
ddb_sign(DDBound a)
{
  double bound = a.e * (1.0 + 1e-6) + DDB_TINY;
  double mag = fabs(a.hi) - fabs(a.lo);
  if (! isfinite(a.hi) || ! isfinite(a.lo) || ! isfinite(bound))
    return 0;
  if (mag <= bound)
    return 0;
  return (a.hi > 0.0) ? 1 : -1;
}

/**
 * @brief Return true and write in the last argument the double nearest the
 * root of `n / d`, `d` positive, where the bounds place the root strictly
 * inside the rounding cell of one double
 * @details The residual search of #point_distance_nearest, with `d` scaling
 * the gaps: with `c` a candidate, the root lies inside its cell exactly where
 * the residual `n - c^2 d` falls short of `d gu (c + gu / 4)` and exceeds
 * `- d gd (c - gd / 4)`, `gu` and `gd` the gaps from `c` to its upper and
 * lower neighbours. The residual is formed once per candidate on two doubles
 * with a bound on its error (#ddb_mul, #ddb_sub), and a residual beyond
 * either gap moves the candidate to that neighbour. A tie, or a root the
 * bounds cannot place, decides nothing
 */
static bool
ddb_nearest_root(DDBound n, DDBound d, double *result)
{
  if (ddb_sign(d) <= 0)
    return false;
  /* An exact zero is the root of zero, which is zero */
  if (ddb_is_zero(n))
  {
    *result = 0.0;
    return true;
  }
  if (ddb_sign(n) <= 0)
    return false;
  double dv = d.hi + d.lo;
  /* The bound of d must leave the gaps their meaning */
  if (! (d.e <= 0x1p-60 * dv))
    return false;
  /* The quotient and the root in doubles round twice, so the candidate may
   * be a neighbour of the answer: a few steps towards the side the residual
   * names reach it */
  double c = sqrt((n.hi + n.lo) / dv);
  for (int step = 0; step < 4; step++)
  {
    if (! (c > 0.0 && c < DBL_MAX))
      return false;
    uint64_t bits, ubits, dbits;
    memcpy(&bits, &c, sizeof(bits));
    ubits = bits + 1;
    dbits = bits - 1;
    double cu, cd;
    memcpy(&cu, &ubits, sizeof(cu));
    memcpy(&cd, &dbits, sizeof(cd));
    if (! (cd > 0.0))
      return false;
    double gu = cu - c, gd = c - cd;
    /* The residual n - c^2 d and a bound on its error */
    DDBound cc;
    two_product(c, c, &cc.hi, &cc.lo);
    cc.e = (fabs(cc.hi) < DDB_EXACT_PRODUCT) ? DDB_TINY : 0.0;
    DDBound r = ddb_sub(n, ddb_mul(cc, d));
    if (! isfinite(r.hi) || ! isfinite(r.lo) || ! isfinite(r.e))
      return false;
    double rv = r.hi + r.lo;
    double rerr = r.e + DBL_EPSILON * fabs(rv);
    double up = gu * (c + 0.25 * gu) * dv, down = gd * (c - 0.25 * gd) * dv;
    double near = (fabs(rv) + rerr) * (1.0 + 0x1p-50);
    if (near < down * (1.0 - 0x1p-48) && near < up * (1.0 - 0x1p-48))
    {
      *result = c;
      return true;
    }
    if ((rv - rerr) * (1.0 - 0x1p-50) > up * (1.0 + 0x1p-48))
    {
      c = cu;
      continue;
    }
    if (- (rv + rerr) * (1.0 - 0x1p-50) > gd * c * dv * (1.0 + 0x1p-48))
    {
      c = cd;
      continue;
    }
    return false;
  }
  return false;
}

/**
 * @brief Return the distance of a point to a segment less a radius, computed
 * exactly and rounded once, or 0 where the point is no farther from the
 * segment than the radius
 * @details With `w` the point less the segment's start `a` and `u` the segment
 * `b - a`, the nearest point of the segment is the foot of the perpendicular
 * at the parameter `(w . u) / (u . u)` where it lies in [0, 1], and an end
 * otherwise. Which one is decided by the signs of `w . u` and of
 * `w . u - u . u`, exactly (#polynomial_sign_exact), and an end is a pair of
 * points (#point_distance_offset_exact). For the foot, the squared distance is
 * the rational `(w x u)^2 / (u . u)`, and the answer is the smallest double
 * whose upper midpoint the distance less the radius does not pass
 * (#point_segment_beyond), so no root of a rounded value decides it. A
 * galloping search from the rounded quotient finds it in a few exact signs
 * however much the distance and the radius cancel. The differences and the
 * radius are scaled by the power of two that brings the largest difference
 * into [1, 2), which is exact, as #point_distance_exact scales them
 * @param[in] p,a,b The point and the two ends of the segment, @p ndims
 * coordinates each
 * @param[in] ndims Number of coordinates, 2 or 3: in 3D the squared cross
 * product is the sum over the planes of two coordinates of the squares of
 * their cross products, `(w_i u_j - w_j u_i)^2`
 * @param[in] r Radius, not negative
 * @note Correctly rounded where, after the scaling, no product of the exact
 * sums underflows: the differences, their rounding errors, the radius and the
 * answer are zero or within a factor 2^240 of the largest difference
 */
double
point_segment_distance_offset_exact(const double *p, const double *a,
  const double *b, int ndims, double r)
{
  assert(r >= 0.0); assert(ndims == 2 || ndims == 3);
  double dw[3], ew[3], du[3], eu[3], maxw, maxu;
  int sw = point_distance_diffs(a, p, ndims, dw, ew, &maxw);
  int su = point_distance_diffs(a, b, ndims, du, eu, &maxu);
  if (sw == 1 || su == 1)
    return NAN;
  /* A degenerate segment is a point, exactly */
  bool degenerate = true;
  for (int i = 0; i < ndims; i++)
    degenerate = degenerate && du[i] == 0.0;
  if (degenerate)
    return point_distance_offset_exact(p, a, ndims, r);
  /* The first test on doubles, which settles every pair but those near a
   * midpoint, a tie or an end, as #point_distance_offset_exact does */
  if (sw == 0 && su == 0)
  {
    double result;
    if (point_segment_offset_filter(dw, ew, du, eu, maxw, maxu, p, a, b,
        ndims, r, &result))
      return result;
    /* With no radius, two doubles with a bound on their error, an exact zero
     * held as one (#DDBound), place the nearest point by the signs of w . u
     * and of w . u - u . u, and the distance to the foot is the root of
     * |w x u|^2 / (u . u), answered wherever they hold it clear of the
     * midpoints of its rounding (#ddb_nearest_root) */
    if (r == 0.0)
    {
      DDBound fw[3], fu[3];
      for (int i = 0; i < ndims; i++)
      {
        two_diff(p[i], a[i], &fw[i].hi, &fw[i].lo);
        two_diff(b[i], a[i], &fu[i].hi, &fu[i].lo);
        fw[i].e = fu[i].e = 0.0;
      }
      DDBound fdot = {0.0, 0.0, 0.0}, fl = {0.0, 0.0, 0.0},
        fq = {0.0, 0.0, 0.0};
      for (int i = 0; i < ndims; i++)
      {
        fdot = ddb_add(fdot, ddb_mul(fw[i], fu[i]));
        fl = ddb_add(fl, ddb_mul(fu[i], fu[i]));
      }
      int sdot = ddb_is_zero(fdot) ? 0 : ddb_sign(fdot);
      if (ddb_is_zero(fdot) || sdot < 0)
        return point_distance_offset_exact(p, a, ndims, r);
      DDBound fdl = ddb_sub(fdot, fl);
      if (ddb_is_zero(fdl) || ddb_sign(fdl) > 0)
        return point_distance_offset_exact(p, b, ndims, r);
      if (sdot > 0 && ddb_sign(fdl) < 0)
      {
        for (int i = 0; i < ndims; i++)
          for (int j = i + 1; j < ndims; j++)
          {
            DDBound x = ddb_sub(ddb_mul(fw[i], fu[j]),
              ddb_mul(fw[j], fu[i]));
            fq = ddb_add(fq, ddb_mul(x, x));
          }
        if (ddb_nearest_root(fq, fl, &result))
          return result;
      }
    }
  }
  /* A difference beyond the largest double where a quarter of a coordinate
   * or of the radius would round: every sign on the input coordinates */
  if ((sw == 2 || su == 2) && point_segment_quarter_rounds(p, a, b, ndims, r))
    return point_segment_distance_offset_inputs(p, a, b, ndims, r);
  /* A difference beyond the largest double: a quarter of every value is
   * exact and its differences are doubles, so solve there and scale back */
  if (sw == 2 || su == 2)
  {
    double p4[3] = {0.0}, a4[3] = {0.0}, b4[3] = {0.0};
    for (int i = 0; i < ndims; i++)
    {
      p4[i] = p[i] / 4.0;
      a4[i] = a[i] / 4.0;
      b4[i] = b[i] / 4.0;
    }
    return point_segment_distance_offset_exact(p4, a4, b4, ndims, r / 4.0) *
      4.0;
  }
  double maxd = (maxw > maxu) ? maxw : maxu;
  int k;
  double f1, f2, g1, g2;
  point_distance_scale(maxd, &k, &f1, &f2, &g1, &g2);
  /* Where the scaling rounds a value or leaves a product of four of them
   * outside the doubles, every sign on the input coordinates */
  {
    double v[13];
    int nv = 0;
    for (int i = 0; i < ndims; i++)
    {
      v[nv++] = dw[i]; v[nv++] = ew[i]; v[nv++] = du[i]; v[nv++] = eu[i];
    }
    v[nv++] = r;
    if (! point_segment_scale_exact(v, nv, k))
      return point_segment_distance_offset_inputs(p, a, b, ndims, r);
  }
  ExactSum sums[7];
  double t[2];
  for (int i = 0; i < ndims; i++)
  {
    t[0] = dw[i] * f1 * f2; t[1] = ew[i] * f1 * f2;
    exact_sum_set(&sums[i], t, 2);
    t[0] = du[i] * f1 * f2; t[1] = eu[i] * f1 * f2;
    exact_sum_set(&sums[ndims + i], t, 2);
  }
  /* Beyond the ends of the segment the nearest point is an end: the signs of
   * w . u and of w . u - u . u */
  PolyTerm dot[3], dotlen[6];
  int ndot = 0, ndotlen = 0;
  for (int i = 0; i < ndims; i++)
  {
    dot[ndot++] = dotlen[ndotlen++] = (PolyTerm) {1.0, 2, {i, ndims + i}};
  }
  for (int i = 0; i < ndims; i++)
    dotlen[ndotlen++] = (PolyTerm) {-1.0, 2, {ndims + i, ndims + i}};
  if (polynomial_sign_exact(sums, dot, ndot) <= 0)
    return point_distance_offset_exact(p, a, ndims, r);
  if (polynomial_sign_exact(sums, dotlen, ndotlen) >= 0)
    return point_distance_offset_exact(p, b, ndims, r);
  /* The distance to the line is below that to the start, which is below
   * twice the largest difference */
  if (r >= 2.0 * maxw)
    return 0.0;
  double rs = r * f1 * f2;

  /* The squared cross product Q and the squared length L in pairs of doubles,
   * each scaled sum two doubles and so a pair exactly, which tell every
   * midpoint the search asks but those nearest the answer
   * (#pair_mid_sign) */
  double Wp[3][2], Up[3][2], Q[2] = {0.0, 0.0}, L[2] = {0.0, 0.0};
  double magq = 0.0, magl = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    const ExactSum *sumw = &sums[i], *sumu = &sums[ndims + i];
    two_sum(sumw->e[sumw->n - 1], (sumw->n > 1) ? sumw->e[0] : 0.0, &Wp[i][0],
      &Wp[i][1]);
    two_sum(sumu->e[sumu->n - 1], (sumu->n > 1) ? sumu->e[0] : 0.0, &Up[i][0],
      &Up[i][1]);
    double th, tl;
    pair_mul(Up[i][0], Up[i][1], Up[i][0], Up[i][1], &th, &tl);
    pair_add(L[0], L[1], th, tl, &L[0], &L[1]);
    magl += fabs(th);
  }
  for (int i = 0; i < ndims; i++)
    for (int j = i + 1; j < ndims; j++)
    {
      double ph, pl, qh, ql, xh, xl;
      pair_mul(Wp[i][0], Wp[i][1], Up[j][0], Up[j][1], &ph, &pl);
      pair_mul(Wp[j][0], Wp[j][1], Up[i][0], Up[i][1], &qh, &ql);
      pair_add(ph, pl, - qh, - ql, &xh, &xl);
      double mx = fabs(ph) + fabs(qh);
      pair_mul(xh, xl, xh, xl, &ph, &pl);
      pair_add(Q[0], Q[1], ph, pl, &Q[0], &Q[1]);
      magq += mx * mx;
    }
  double eq = 0x1p-96 * magq, el = 0x1p-96 * magl;

  /* First candidate, from the rounded cross product over the rounded length */
  double cross2 = 0.0, len2 = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double ui = sums[ndims + i].approx;
    len2 += ui * ui;
    for (int j = i + 1; j < ndims; j++)
    {
      double x = sums[i].approx * sums[ndims + j].approx -
        sums[j].approx * ui;
      cross2 += x * x;
    }
  }
  double cross = (ndims == 2) ?
    fabs(sums[0].approx * sums[3].approx - sums[1].approx * sums[2].approx) :
    sqrt(cross2);
  double c = (cross / sqrt(len2) - rs) * g1 * g2;
  if (! (c > 0.0))
    c = 0.0;
  if (isinf(c))
    c = DBL_MAX;
  uint64_t bits;
  memcpy(&bits, &c, sizeof(bits));

  /* The answer is the smallest bit pattern that is not beyond: gallop from the
   * candidate to bracket it, then halve the bracket */
#define PSD_BEYOND(bb, res) \
  do { \
    double cc, nn; \
    uint64_t b1 = (bb), b2 = (bb) + 1; \
    memcpy(&cc, &b1, sizeof(cc)); \
    memcpy(&nn, &b2, sizeof(nn)); \
    /* A candidate or a half gap scaled below 2^-268: the input coordinates */ \
    if ((cc != 0.0 && double_lowest_bit(cc) + k < -268) || \
        (! isinf(nn) && double_lowest_bit(nn - cc) - 1 + k < -268)) \
    { \
      (res) = point_segment_beyond_inputs(p, a, b, ndims, r, cc, nn, \
        (b1 & 1) != 0); \
      break; \
    } \
    double hh = isinf(nn) ? ldexp(1.0, 970 + k) : (nn - cc) * f1 * f2 / 2.0; \
    /* The midpoint r + c + h as a pair, within 2^-104 of it */ \
    double s1, e1, mh, e2; \
    two_sum(rs, cc * f1 * f2, &s1, &e1); \
    two_sum(s1, hh, &mh, &e2); \
    int sign = pair_mid_sign(Q, eq, L, el, mh, e1 + e2); \
    (res) = (sign != 0) ? (sign > 0) : \
      point_segment_beyond(sums, ndims, rs, cc * f1 * f2, hh, \
        (b1 & 1) != 0); \
  } while (0)
  uint64_t lo, hi;
  bool beyond;
  PSD_BEYOND(bits, beyond);
  if (beyond)
  {
    /* The answer is above: lo is beyond, hi is not */
    uint64_t step = 1;
    lo = bits;
    const uint64_t inf_bits = 0x7FF0000000000000ULL;
    while (true)
    {
      hi = (lo + step < inf_bits) ? lo + step : inf_bits;
      if (hi == inf_bits)
        break;
      PSD_BEYOND(hi, beyond);
      if (! beyond)
        break;
      lo = hi;
      step *= 2;
    }
  }
  else
  {
    /* The answer is at or below: hi is not beyond, lo is beyond or zero */
    uint64_t step = 1;
    hi = bits;
    while (true)
    {
      if (hi == 0)
        return 0.0;
      lo = (hi > step) ? hi - step : 0;
      PSD_BEYOND(lo, beyond);
      if (beyond)
        break;
      hi = lo;
      if (lo == 0)
        return 0.0;
      step *= 2;
    }
  }
  /* lo is beyond, hi is not, and the answer is the least not beyond */
  while (hi - lo > 1)
  {
    uint64_t mid = lo + (hi - lo) / 2;
    PSD_BEYOND(mid, beyond);
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
#undef PSD_BEYOND
  double result;
  memcpy(&result, &hi, sizeof(result));
  return result;
}

/**
 * @brief Return the sign of the distance of a point to a segment less a
 * radius, computed exactly for any finite doubles
 * @details The sign #point_segment_distance_offset_exact rounds its answer
 * from, with no rounding: the nearest point of the segment is an end where
 * #point_segment_dot_sign_inputs says so, and the sign is then that of the
 * squared distance of the two points less the squared radius
 * (#point_distance_offset_side_sign); otherwise it is that of
 * `(w x u)^2 - r^2 (u . u)` (#point_segment_side_sign_inputs). Each is a sum
 * of products of the input doubles decided exactly, so the answer is 0
 * exactly where the disc of the radius is tangent to the segment
 * @param[in] p,a,b The point and the two ends of the segment, two coordinates
 * each
 * @param[in] r Radius, not negative
 * @return -1, 0 or 1
 */
int
point_segment_offset_sign(const double *p, const double *a, const double *b,
  double r)
{
  assert(r >= 0.0);
  if (point_segment_dot_sign_inputs(p, a, a, b, 2) <= 0)
    return point_distance_offset_side_sign(p, a, 2, r, 0.0, 0.0);
  if (point_segment_dot_sign_inputs(p, b, a, b, 2) >= 0)
    return point_distance_offset_side_sign(p, b, 2, r, 0.0, 0.0);
  return point_segment_side_sign_inputs(p, a, b, 2, r, 0.0, 0.0);
}

/**
 * @brief Set an edge to the circle of a disk as a centred arc, as
 * #edges_add_circle builds it
 */
static void
edge_disk_circle(const double *p, double r, Edge *c)
{
  memset(c, 0, sizeof(Edge));
  c->etype = EDGE_POLYARC;
  c->cx = p[0]; c->cy = p[1]; c->radius = r;
  c->centred = true;
  c->ccw = true;
  c->x1 = c->x2 = p[0] - r;
  c->y1 = c->y2 = p[1];
  c->theta0 = c->theta1 = M_PI;
  c->xmin = p[0] - r; c->xmax = p[0] + r;
  c->ymin = p[1] - r; c->ymax = p[1] + r;
  return;
}

/**
 * @brief Return the sign of the distance of a point to an arc less a radius,
 * computed exactly, as #point_segment_offset_sign reads it for a segment
 * @details The arc runs from S through M to E, the three input points that
 * fix its circle. A point of the arc lies strictly inside the disk of centre
 * @p p and radius @p r where one of the three does, which
 * #point_distance_offset_side_sign decides, or where the circle of the disk
 * crosses the arc between its ends, which #arcarc_meet_exact decides with the
 * circle of the disk as a centred arc (#edges_add_circle); the sign is then
 * -1. Otherwise no point of the open arc crosses the circle of the disk, so
 * the open arc lies in the disk or out of it as a whole, and M decides which.
 * The sign is then 0 where the arc touches the circle, at a tangency between
 * its ends, a double root that is no crossing, or at an end, and 1 otherwise.
 * Where S, M and E all lie on the circle of the disk the two circles are one
 * and the sign is 0.
 *
 * A half circle on a diameter runs between its ends A and B on the side of
 * the line AB it turns to, its right from A to B where it turns
 * counterclockwise. Where neither end lies inside the disk and @p p lies
 * strictly on that side, the point of the circle nearest @p p lies on the
 * open half, so the half crosses the circle of the disk, touches it or clears
 * it as the two circles cross, touch or keep apart, the sign of their meeting
 * (#arcarc_meet_exact); @p p off the line AB is not the centre of the half,
 * so the circles are distinct. Otherwise the distance to @p p along the half
 * grows from one end to the furthest point of the circle and falls to the
 * other end, so the ends decide
 * @param[in] p Centre of the disk, two coordinates
 * @param[in] e Arc of three input points or half circle on a diameter
 * @param[in] r Radius, not negative
 * @return -1, 0 or 1, or 2 for an arc this function does not decide: a
 * centred arc, whose circle no input point fixes, or a radius of 0
 */
int
point_arc_offset_sign(const double *p, const Edge *e, double r)
{
  assert(r >= 0.0);
  if (e->centred || r == 0.0)
    return 2;
  if (e->diameter)
  {
    const double a[2] = {e->x1, e->y1}, b[2] = {e->x2, e->y2};
    int sa = point_distance_offset_side_sign(p, a, 2, r, 0.0, 0.0);
    int sb = point_distance_offset_side_sign(p, b, 2, r, 0.0, 0.0);
    if (sa < 0 || sb < 0)
      return -1;
    /* The side of the line A B the half lies on, and the one @p p lies on */
    int side = e->ccw ? -1 : 1;
    if (cross_product_sign(a[0], a[1], b[0], b[1], a[0], a[1], p[0],
        p[1]) == side)
    {
      Edge c;
      edge_disk_circle(p, r, &c);
      int meet, within[2][2];
      arcarc_meet_exact(e, &c, &meet, within);
      return (meet > 0) ? -1 : ((meet == 0) ? 0 : 1);
    }
    return (sa == 0 || sb == 0) ? 0 : 1;
  }
  const double s[2] = {e->sx, e->sy}, m[2] = {e->mx, e->my},
    t[2] = {e->ex, e->ey};
  int ss = point_distance_offset_side_sign(p, s, 2, r, 0.0, 0.0);
  int sm = point_distance_offset_side_sign(p, m, 2, r, 0.0, 0.0);
  int st = point_distance_offset_side_sign(p, t, 2, r, 0.0, 0.0);
  if (ss < 0 || sm < 0 || st < 0)
    return -1;
  if (ss == 0 && sm == 0 && st == 0)
    return 0;
  Edge c;
  edge_disk_circle(p, r, &c);
  int meet, within[2][2];
  arcarc_meet_exact(e, &c, &meet, within);
  /* A meeting within the arc lies strictly between its ends */
  bool between = within[0][0] || within[1][0];
  if (meet > 0 && between)
    return -1;
  if ((meet == 0 && between) || ss == 0 || st == 0)
    return 0;
  return 1;
}

/**
 * @brief The square of the distance of a point to a segment as a quotient of
 * polynomials in exact sums, `num / den` (#segment_distance_square)
 */
typedef struct
{
  PolyTerm num[9];       /**< Terms of the numerator */
  int nnum;              /**< Number of terms of the numerator */
  PolyTerm den[3];       /**< Terms of the denominator */
  int nden;              /**< Number of terms of the denominator */
} SegmentSquare;

/**
 * @brief Return the sign of `x / y - u / v` for two squares of
 * #segment_distance_square, the sign of `x v - u y` since both denominators
 * are positive
 */
static int
segment_square_cmp(const ExactSum *sums, const SegmentSquare *s1,
  const SegmentSquare *s2)
{
  PolyTerm terms[54];
  int n = 0;
  for (int side = 0; side < 2; side++)
  {
    const SegmentSquare *x = side ? s2 : s1, *y = side ? s1 : s2;
    for (int i = 0; i < x->nnum; i++)
      for (int j = 0; j < y->nden; j++)
      {
        PolyTerm *t = &terms[n++];
        t->coef = (side ? -1.0 : 1.0) * x->num[i].coef * y->den[j].coef;
        t->deg = x->num[i].deg + y->den[j].deg;
        memcpy(t->sum, x->num[i].sum, sizeof(int) * x->num[i].deg);
        memcpy(t->sum + x->num[i].deg, y->den[j].sum,
          sizeof(int) * y->den[j].deg);
      }
  }
  return polynomial_sign_exact(sums, terms, n);
}

/**
 * @brief Return the least of the exact squares of the distances `want`
 * selects, as the mask of those that reach it (#segment_distance_square)
 * @details With `w` the end less the start of the other segment, `u` that
 * segment and `z` the end less the end of the other segment, the square is
 * `w . w` at the start, `z . z` at the end and `|w x u|^2 / (u . u)` at the
 * foot of the perpendicular, the squared cross product the sum over the
 * planes of two coordinates of `(w_i u_j - w_j u_i)^2`; every square is a
 * quotient of polynomials of
 * degree at most 4 in the coordinate differences, and two compare by the sign
 * of a polynomial of degree at most 6 (#polynomial_sign_exact). The
 * differences and the errors of their rounding are scaled by the power of two
 * that brings the largest into [1, 2), which changes no sign, as
 * #point_segment_distance_offset_exact scales them; a product of six scaled
 * values is exact where each has its lowest bit at or above 2^-179, and below
 * 2^6, so none overflows
 * @param[in] pt The point, the start and the end of the other segment, of
 * each of the four distances
 * @param[in] near Where the nearest point of each lies: 0 the start, 1 the
 * end, 2 the foot
 * @param[in] want The mask of the distances to compare
 * @param[in] ndims Number of coordinates, 2 or 3
 * @return The mask of those that reach the least, or -1 where a difference is
 * not finite, or where the scaling leaves a product of six outside the doubles
 */
static int
segment_square_least(const double *pt[4][3], const int *near, int want,
  int ndims)
{
  /* The differences w = p - s, u = e - s and z = p - e of each, and the
   * errors of their rounding */
  double dv[4][3][3], ev[4][3][3], maxd = 0.0;
  for (int i = 0; i < 4; i++)
  {
    if (! (want & (1 << i)))
      continue;
    const double *p = pt[i][0], *s = pt[i][1], *e = pt[i][2];
    double m[3];
    if (point_distance_diffs(s, p, ndims, dv[i][0], ev[i][0], &m[0]) ||
        point_distance_diffs(s, e, ndims, dv[i][1], ev[i][1], &m[1]) ||
        point_distance_diffs(e, p, ndims, dv[i][2], ev[i][2], &m[2]))
      return -1;
    for (int j = 0; j < 3; j++)
      if (m[j] > maxd)
        maxd = m[j];
  }
  if (maxd == 0.0)
    return -1;
  int k;
  double f1, f2, g1, g2;
  point_distance_scale(maxd, &k, &f1, &f2, &g1, &g2);
  /* The exact sums, sum (3 i + j) * ndims + c for coordinate c of difference
   * j of distance i */
  ExactSum sums[36];
  for (int i = 0; i < 4; i++)
  {
    if (! (want & (1 << i)))
      continue;
    for (int j = 0; j < 3; j++)
      for (int cc = 0; cc < ndims; cc++)
      {
        double v = dv[i][j][cc], e = ev[i][j][cc];
        if ((v != 0.0 && double_lowest_bit(v) + k < -179) ||
            (e != 0.0 && double_lowest_bit(e) + k < -179))
          return -1;
        const double t[2] = {v * f1 * f2, e * f1 * f2};
        exact_sum_set(&sums[(3 * i + j) * ndims + cc], t, 2);
      }
  }
  /* The square of each distance */
  SegmentSquare sq[4];
  for (int i = 0; i < 4; i++)
  {
    if (! (want & (1 << i)))
      continue;
    /* The first sum of w, of u and of z */
    int w = (3 * i) * ndims, u = w + ndims;
    SegmentSquare *s = &sq[i];
    s->nnum = s->nden = 0;
    if (near[i] < 2)
    {
      /* The start, w . w, or the end, z . z */
      int z = u + ndims;
      int x = near[i] ? z : w;
      for (int c = 0; c < ndims; c++)
        s->num[s->nnum++] = (PolyTerm) {1.0, 2, {x + c, x + c}};
      s->den[s->nden++] = (PolyTerm) {1.0, 0, {0}};
    }
    else
    {
      /* The foot: the sum over the planes of (w_a u_b - w_b u_a)^2, over
       * u . u */
      for (int a = 0; a < ndims; a++)
        for (int b = a + 1; b < ndims; b++)
        {
          s->num[s->nnum++] = (PolyTerm) {1.0, 4, {w + a, w + a, u + b,
            u + b}};
          s->num[s->nnum++] = (PolyTerm) {-2.0, 4, {w + a, w + b, u + a,
            u + b}};
          s->num[s->nnum++] = (PolyTerm) {1.0, 4, {w + b, w + b, u + a,
            u + a}};
        }
      for (int c = 0; c < ndims; c++)
        s->den[s->nden++] = (PolyTerm) {1.0, 2, {u + c, u + c}};
    }
  }
  int best = -1, mask = 0;
  for (int i = 0; i < 4; i++)
  {
    if (! (want & (1 << i)))
      continue;
    int cmp = (best < 0) ? -1 : segment_square_cmp(sums, &sq[i], &sq[best]);
    if (cmp < 0)
    {
      best = i;
      mask = 1 << i;
    }
    else if (cmp == 0)
      mask |= 1 << i;
  }
  return mask;
}

/**
 * @brief Return true where two pairs of points are the same pair, in either
 * order
 */
static inline bool
point_pair_eq(const double *p1, const double *q1, const double *p2,
  const double *q2, int ndims)
{
#define PT_EQ(x, y) ((x)[0] == (y)[0] && (x)[1] == (y)[1] && \
  (ndims == 2 || (x)[2] == (y)[2]))
  bool result = (PT_EQ(p1, p2) && PT_EQ(q1, q2)) ||
    (PT_EQ(p1, q2) && PT_EQ(q1, p2));
#undef PT_EQ
  return result;
}

/**
 * @brief Return which of the four distances of an end of one segment to the
 * other reach the least of them, decided on their squares exactly
 * @details The four distances are those of #segment_distance_exact: the two
 * ends of the first segment to the second, then the two ends of the second to
 * the first. Their order is that of their squares, so the least is found as
 * liblwgeom finds it, comparing the squares (`distance2d_sqr_pt_seg`), and only
 * that one needs its distance rounded. Each distance is first bounded in
 * doubles, at the clamped parameter of its foot, as #dist_rest_bounds bounds
 * it, which filters the distances: one whose lower bound exceeds the least
 * upper bound is not the least. Where more than one is left, where the nearest
 * point of each lies is decided by exact signs (#point_segment_dot_sign_inputs):
 * the start, the end, or the foot of the perpendicular. Two whose nearest
 * points are the same pair of input points, an end of each segment, are the
 * same distance; the others are compared on their exact squares
 * (#segment_square_least)
 * @param[in] a,b The two ends of the first segment, @p ndims coordinates each
 * @param[in] c,d The two ends of the second segment, @p ndims coordinates
 * each
 * @param[in] ndims Number of coordinates, 2 or 3
 * @return The mask of the distances that reach the least, bit `i` for the
 * `i`-th of the four; -1 where a coordinate is not finite, or where the exact
 * squares cannot be compared in the doubles, which leaves the four to be
 * rounded and compared by the caller
 */
int
segment_distance_square(const double *a, const double *b, const double *c,
  const double *d, int ndims)
{
  /* Each distance: the end, the start and the end of the other segment */
  const double *pt[4][3] = {{a, c, d}, {b, c, d}, {c, a, b}, {d, a, b}};
  for (int k = 0; k < ndims; k++)
    if (! isfinite(a[k]) || ! isfinite(b[k]) || ! isfinite(c[k]) ||
        ! isfinite(d[k]))
      return -1;
  /* The bounds of each distance, with `e` the unit roundoff: the computed
   * distance at the clamped rounded parameter is within
   * `8e (|w|_1 + |u|_1) + 3e D` of the distance `D` (#dist_rest_bounds), and
   * the bounds stand `32 DBL_EPSILON` times those magnitudes on either side,
   * where every product and every square of a bound is a normal double;
   * their squares bound the squares */
  double lo[4], hi[4], least = INFINITY;
  int side[4];
  for (int i = 0; i < 4; i++)
  {
    const double *p = pt[i][0], *s = pt[i][1], *e = pt[i][2];
    side[i] = 0;
    double w[3], u[3], w1 = 0.0, u1 = 0.0;
    for (int k = 0; k < ndims; k++)
    {
      w[k] = p[k] - s[k];
      u[k] = e[k] - s[k];
      w1 += fabs(w[k]);
      u1 += fabs(u[k]);
    }
    double m = w1 + u1;
    lo[i] = 0.0;
    hi[i] = INFINITY;
    if (! (m <= 0x1p480) || (m > 0.0 && m < 0x1p-400))
      continue;
    double q2 = 0.0;
    if (u1 > 0.0)
    {
      double wu = 0.0, uu = 0.0;
      for (int k = 0; k < ndims; k++)
      {
        wu += w[k] * u[k];
        uu += u[k] * u[k];
      }
      double t = wu / uu;
      side[i] = (t >= 1.0) ? 1 : 0;
      t = (t < 0.0) ? 0.0 : ((t > 1.0) ? 1.0 : t);
      for (int k = 0; k < ndims; k++)
      {
        double q = w[k] - t * u[k];
        q2 += q * q;
      }
    }
    else
      for (int k = 0; k < ndims; k++)
        q2 += w[k] * w[k];
    double dist = sqrt(q2);
    double delta = 32.0 * DBL_EPSILON * (m + dist);
    double l = dist - delta, h = dist + delta;
    lo[i] = (l > 0.0) ? l * l : 0.0;
    hi[i] = h * h;
    if (hi[i] < least)
      least = hi[i];
  }
  int left = 0;
  for (int i = 0; i < 4; i++)
    if (! (lo[i] > least))
      left |= 1 << i;
  if (! (left & (left - 1)))
    return left;
  /* Where the nearest point of each distance left lies, the end the rounded
   * parameter reaches asked first, and those left, each standing for those
   * equal to it as the same pair of input points */
  int near[4], group[4], want = 0;
  for (int i = 0; i < 4; i++)
  {
    group[i] = 0;
    if (! (left & (1 << i)))
      continue;
    const double *p = pt[i][0], *s = pt[i][1], *e = pt[i][2];
    if (side[i] == 1 &&
        point_segment_dot_sign_inputs(p, e, e, s, ndims) <= 0)
      near[i] = 1;
    else if (point_segment_dot_sign_inputs(p, s, s, e, ndims) <= 0)
      near[i] = 0;
    else if (side[i] != 1 &&
        point_segment_dot_sign_inputs(p, e, e, s, ndims) <= 0)
      near[i] = 1;
    else
      near[i] = 2;
    group[i] = 1 << i;
    for (int j = 0; j < i; j++)
    {
      if (! (want & (1 << j)) || near[i] == 2 || near[j] == 2)
        continue;
      const double *qi = (near[i] == 1) ? pt[i][2] : pt[i][1];
      const double *qj = (near[j] == 1) ? pt[j][2] : pt[j][1];
      if (point_pair_eq(pt[i][0], qi, pt[j][0], qj, ndims))
      {
        group[j] |= 1 << i;
        group[i] = 0;
        break;
      }
    }
    if (group[i])
      want |= 1 << i;
  }
  int least_mask = want;
  if (want & (want - 1))
  {
    least_mask = segment_square_least(pt, near, want, ndims);
    if (least_mask < 0)
      return -1;
  }
  int mask = 0;
  for (int i = 0; i < 4; i++)
    if (least_mask & (1 << i))
      mask |= group[i];
  return mask;
}


/*****************************************************************************
 * The closest parameters of two segments, ported from Geometric Tools
 *
 * The four functions below port DCPQuery<T, Segment, Segment>::ComputeRobust
 * and its helpers GetClampedRoot, ComputeIntersection and
 * ComputeMinimumParameters of GTE/Mathematics/DistSegmentSegment.h, file
 * version 8.0.2025.05.10:
 *   David Eberly, Geometric Tools, Redmond WA 98052
 *   Copyright (c) 1998-2026
 *   Distributed under the Boost Software License, Version 1.0.
 *   https://www.boost.org/LICENSE_1_0.txt
 * The search is a conjugate gradient search for the minimum of the squared
 * distance on [0,1]^2, robust for nearly parallel segments; see
 * https://www.geometrictools.com/Documentation/DistanceLine3Line3.pdf
 *****************************************************************************/

/**
 * @brief Return the root of `h0 + slope z` clamped to [0, 1], GTE's
 * GetClampedRoot
 */
static pg_attribute_always_inline double
gte_clamped_root(double slope, double h0, double h1)
{
  if (h0 < 0.0)
  {
    if (h1 > 0.0)
    {
      double r = - h0 / slope;
      return (r > 1.0) ? 0.5 : r;
    }
    return 1.0;
  }
  return 0.0;
}

/**
 * @brief Set the ends of the segment where the line dR/ds = 0 meets
 * [0,1]^2 and the edges they lie on, GTE's ComputeIntersection
 */
static pg_attribute_always_inline void
gte_compute_intersection(const double *svalue, const int *classify, double b,
  double f00, double f10, int *edge, double end[2][2])
{
  if (classify[0] < 0)
  {
    edge[0] = 0;
    end[0][0] = 0.0;
    end[0][1] = f00 / b;
    if (end[0][1] < 0.0 || end[0][1] > 1.0)
      end[0][1] = 0.5;
    if (classify[1] == 0)
    {
      edge[1] = 3;
      end[1][0] = svalue[1];
      end[1][1] = 1.0;
    }
    else
    {
      edge[1] = 1;
      end[1][0] = 1.0;
      end[1][1] = f10 / b;
      if (end[1][1] < 0.0 || end[1][1] > 1.0)
        end[1][1] = 0.5;
    }
  }
  else if (classify[0] == 0)
  {
    edge[0] = 2;
    end[0][0] = svalue[0];
    end[0][1] = 0.0;
    if (classify[1] < 0)
    {
      edge[1] = 0;
      end[1][0] = 0.0;
      end[1][1] = f00 / b;
      if (end[1][1] < 0.0 || end[1][1] > 1.0)
        end[1][1] = 0.5;
    }
    else if (classify[1] == 0)
    {
      edge[1] = 3;
      end[1][0] = svalue[1];
      end[1][1] = 1.0;
    }
    else
    {
      edge[1] = 1;
      end[1][0] = 1.0;
      end[1][1] = f10 / b;
      if (end[1][1] < 0.0 || end[1][1] > 1.0)
        end[1][1] = 0.5;
    }
  }
  else
  {
    edge[0] = 1;
    end[0][0] = 1.0;
    end[0][1] = f10 / b;
    if (end[0][1] < 0.0 || end[0][1] > 1.0)
      end[0][1] = 0.5;
    if (classify[1] == 0)
    {
      edge[1] = 3;
      end[1][0] = svalue[1];
      end[1][1] = 1.0;
    }
    else
    {
      edge[1] = 0;
      end[1][0] = 0.0;
      end[1][1] = f00 / b;
      if (end[1][1] < 0.0 || end[1][1] > 1.0)
        end[1][1] = 0.5;
    }
  }
  return;
}

/**
 * @brief Set the parameters of the minimum of R on the segment where the line
 * dR/ds = 0 meets [0,1]^2, GTE's ComputeMinimumParameters
 */
static pg_attribute_always_inline void
gte_compute_minimum_parameters(const int *edge, double end[2][2], double b,
  double c, double e, double g00, double g10, double g01, double g11,
  double *param)
{
  double delta = end[1][1] - end[0][1];
  double h0 = delta * (- b * end[0][0] + c * end[0][1] - e);
  if (h0 >= 0.0)
  {
    if (edge[0] == 0)
    {
      param[0] = 0.0;
      param[1] = gte_clamped_root(c, g00, g01);
    }
    else if (edge[0] == 1)
    {
      param[0] = 1.0;
      param[1] = gte_clamped_root(c, g10, g11);
    }
    else
    {
      param[0] = end[0][0];
      param[1] = end[0][1];
    }
    return;
  }
  double h1 = delta * (- b * end[1][0] + c * end[1][1] - e);
  if (h1 <= 0.0)
  {
    if (edge[1] == 0)
    {
      param[0] = 0.0;
      param[1] = gte_clamped_root(c, g00, g01);
    }
    else if (edge[1] == 1)
    {
      param[0] = 1.0;
      param[1] = gte_clamped_root(c, g10, g11);
    }
    else
    {
      param[0] = end[1][0];
      param[1] = end[1][1];
    }
    return;
  }
  double z = h0 / (h0 - h1);
  z = (z < 0.0) ? 0.0 : ((z > 1.0) ? 1.0 : z);
  param[0] = (1.0 - z) * end[0][0] + z * end[1][0];
  param[1] = (1.0 - z) * end[0][1] + z * end[1][1];
  return;
}

/**
 * @brief Set the parameters of the closest points of two segments on their
 * coefficients, GTE's ComputeRobust: the squared distance is
 * `R(s,t) = a s^2 - 2 b s t + c t^2 + 2 d s - 2 e t + f` on [0,1]^2
 */
static pg_attribute_always_inline void
gte_segment_closest(double a, double b, double c, double d, double e,
  double *param)
{
  double f00 = d, f10 = f00 + a;
  double g00 = - e, g01 = g00 + c;
  if (a > 0.0 && c > 0.0)
  {
    double f01 = f00 - b, f11 = f10 - b, g10 = g00 - b, g11 = g10 + c;
    const double svalue[2] = {gte_clamped_root(a, f00, f10),
      gte_clamped_root(a, f01, f11)};
    int classify[2];
    for (int i = 0; i < 2; i++)
      classify[i] = (svalue[i] <= 0.0) ? -1 : ((svalue[i] >= 1.0) ? 1 : 0);
    if (classify[0] == -1 && classify[1] == -1)
    {
      param[0] = 0.0;
      param[1] = gte_clamped_root(c, g00, g01);
    }
    else if (classify[0] == 1 && classify[1] == 1)
    {
      param[0] = 1.0;
      param[1] = gte_clamped_root(c, g10, g11);
    }
    else
    {
      int edge[2] = {0, 0};
      double end[2][2];
      gte_compute_intersection(svalue, classify, b, f00, f10, edge, end);
      gte_compute_minimum_parameters(edge, end, b, c, e, g00, g10, g01, g11,
        param);
    }
  }
  else if (a > 0.0)
  {
    param[0] = gte_clamped_root(a, f00, f10);
    param[1] = 0.0;
  }
  else if (c > 0.0)
  {
    param[0] = 0.0;
    param[1] = gte_clamped_root(c, g00, g01);
  }
  else
  {
    param[0] = 0.0;
    param[1] = 0.0;
  }
  return;
}

/**
 * @brief Return the minimum over x in [0, 1] of `g (y - x)` for y in {0, 1},
 * the term of one parameter in the Frank-Wolfe gap
 */
static pg_attribute_always_inline double
segment_fw_term(double g, double x)
{
  return fmin(- g * x, g * (1.0 - x));
}


/**
 * @brief Write in the last two arguments an interval holding the squared
 * distance of two segments, and return false where the doubles leave it, for
 * a number of coordinates the compiler knows (#segment_distance_range)
 */
static pg_attribute_always_inline bool
segment_distance_range_dims(const double *p0, const double *u, double a,
  const double *q0, const double *v, double c, int ndims, double big,
  double *lo, double *hi)
{
  const double eps = 8.0 * DBL_EPSILON, tiny = 0x1p-1000;
  double w[3];
  for (int k = 0; k < ndims; k++)
    w[k] = p0[k] - q0[k];
  /* The margin, where the bound on the differences allows it */
  if (big >= 0x1p-400 && big <= 0x1p400)
  {
    double b = 0.0, d = 0.0, e = 0.0, f = 0.0;
    for (int k = 0; k < ndims; k++)
    {
      b += u[k] * v[k];
      d += u[k] * w[k];
      e += v[k] * w[k];
      f += w[k] * w[k];
    }
    double x[2];
    gte_segment_closest(a, b, c, d, e, x);
    double s = (x[0] < 0.0) ? 0.0 : ((x[0] > 1.0) ? 1.0 : x[0]);
    double t = (x[1] < 0.0) ? 0.0 : ((x[1] > 1.0) ? 1.0 : x[1]);
    double r = a * s * s - 2.0 * b * s * t + c * t * t + 2.0 * d * s -
      2.0 * e * t + f;
    double gs = 2.0 * (a * s - b * t + d), gt = 2.0 * (- b * s + c * t - e);
    double margin = 0x1p-40 * big * big;
    *hi = r + margin;
    *lo = r + segment_fw_term(gs, s) + segment_fw_term(gt, t) - margin;
    if (*lo < 0.0)
      *lo = 0.0;
    return true;
  }
  double b = 0.0, d = 0.0, e = 0.0, f = 0.0;
  double mb = 0.0, md = 0.0, me = 0.0;
  for (int k = 0; k < ndims; k++)
  {
    b += u[k] * v[k];
    d += u[k] * w[k];
    e += v[k] * w[k];
    f += w[k] * w[k];
    mb += fabs(u[k] * v[k]);
    md += fabs(u[k] * w[k]);
    me += fabs(v[k] * w[k]);
  }
  double ea = eps * a + tiny, eb = eps * mb + tiny, ec = eps * c + tiny;
  double ed = eps * md + tiny, ee = eps * me + tiny, ef = eps * f + tiny;
  double x[2];
  gte_segment_closest(a, b, c, d, e, x);
  double s = (x[0] < 0.0) ? 0.0 : ((x[0] > 1.0) ? 1.0 : x[0]);
  double t = (x[1] < 0.0) ? 0.0 : ((x[1] > 1.0) ? 1.0 : x[1]);
  double r = a * s * s - 2.0 * b * s * t + c * t * t + 2.0 * d * s -
    2.0 * e * t + f;
  double mr = a * s * s + 2.0 * fabs(b) * s * t + c * t * t +
    2.0 * fabs(d) * s + 2.0 * fabs(e) * t + f;
  double er = ea * s * s + 2.0 * eb * s * t + ec * t * t + 2.0 * ed * s +
    2.0 * ee * t + ef + eps * mr + tiny;
  double gs = 2.0 * (a * s - b * t + d), gt = 2.0 * (- b * s + c * t - e);
  double egs = 2.0 * (ea * s + eb * t + ed) +
    eps * 2.0 * (a * s + fabs(b) * t + fabs(d)) + tiny;
  double egt = 2.0 * (eb * s + ec * t + ee) +
    eps * 2.0 * (fabs(b) * s + c * t + fabs(e)) + tiny;
  double gap = fmin(segment_fw_term(gs - egs, s),
    segment_fw_term(gs + egs, s)) +
    fmin(segment_fw_term(gt - egt, t), segment_fw_term(gt + egt, t));
  *hi = (r + er) * (1.0 + eps);
  *lo = (r - er + gap * (1.0 + eps)) * (1.0 - eps);
  if (*lo < 0.0)
    *lo = 0.0;
  return isfinite(*hi) && isfinite(*lo) && isfinite(gap);
}

/**
 * @brief Write in the last two arguments an interval holding the squared
 * distance of two segments in 2D, and return false where the doubles leave
 * it (#segment_distance_range), kept out of the 3D entry the distance walk
 * calls
 */
static pg_noinline bool
segment_distance_range_2d(const double *p0, const double *u, double a,
  const double *q0, const double *v, double c, double big, double *lo,
  double *hi)
{
  return segment_distance_range_dims(p0, u, a, q0, v, c, 2, big, lo, hi);
}

/**
 * @brief Write in the last two arguments an interval holding the squared
 * distance of two segments, and return false where the doubles leave it
 * @details The parameters of GTE's search (#gte_segment_closest) are a pair of
 * points of the two segments, so the squared distance there bounds the least
 * from above. The squared distance `R` is convex on [0,1]^2, its Hessian
 * `[a -b; -b c]` having `a c >= b^2`, so at any point `x` of the square the
 * least is at least `R(x) + sum_i min(g_i (0 - x_i), g_i (1 - x_i))` with
 * `g` the gradient there: the Frank-Wolfe duality gap. Each coefficient,
 * value and gradient carries a bound on its rounding in doubles, at most
 * 8 DBL_EPSILON of the magnitudes it combines, and the gap's term is concave
 * in `g`, so its least over the bounded gradient lies at an end of that
 * range. Where a bound on the differences of the coordinates lies within a
 * factor 2^400 of 1, the evaluation in doubles errs far less than 2^-40 of its
 * square, the margin #geom_distance_walk allows its own evaluation, and the
 * interval is read with that margin instead. It decides no answer: the walk
 * reads it to keep the pairs that may be the nearest, which
 * #segment_distance_exact then answers
 * @param[in] p0,u The start of the first segment and its end less its start,
 * rounded, @p ndims coordinates each
 * @param[in] a The rounded squared length of `u`
 * @param[in] q0,v The start of the second segment and its end less its start,
 * rounded, @p ndims coordinates each
 * @param[in] c The rounded squared length of `v`
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] big A bound on the magnitude of every difference of two of their
 * coordinates, or 0 where none is known
 * @param[out] lo,hi The interval
 */
bool
segment_distance_range(const double *p0, const double *u, double a,
  const double *q0, const double *v, double c, int ndims, double big,
  double *lo, double *hi)
{
  assert(ndims == 2 || ndims == 3);
  if (ndims == 2)
    return segment_distance_range_2d(p0, u, a, q0, v, c, big, lo, hi);
  return segment_distance_range_dims(p0, u, a, q0, v, c, 3, big, lo, hi);
}

/* The sums of segment_interior_distance: u at 0 to 2, v at 3 to 5, e at 6 to 8
 * and the midpoint at 9 */
#define SI_U 0
#define SI_V 3
#define SI_E 6
#define SI_M 9

/**
 * @brief Set the terms of `n . n` (@p what 0), `(e x v) . n` (1) or
 * `(e x u) . n` (2), `n = u x v`, on the sums of #segment_interior_distance,
 * each times @p coef, and return their number
 * @details Component `k` of a cross product `p x q` is `p_i q_j - p_j q_i`
 * with `i = k + 1` and `j = k + 2` modulo 3, so each product of two components
 * is four terms of degree four, and the square of one is three
 */
static int
segment_interior_terms(int what, double coef, PolyTerm *t)
{
  int n = 0;
  for (int k = 0; k < 3; k++)
  {
    int i = (k + 1) % 3, j = (k + 2) % 3;
    if (what == 0)
    {
      t[n++] = (PolyTerm) {coef, 4, {SI_U + i, SI_U + i, SI_V + j, SI_V + j}};
      t[n++] = (PolyTerm) {-2.0 * coef, 4, {SI_U + i, SI_V + j, SI_U + j,
        SI_V + i}};
      t[n++] = (PolyTerm) {coef, 4, {SI_U + j, SI_U + j, SI_V + i, SI_V + i}};
    }
    else
    {
      int q = (what == 1) ? SI_V : SI_U;
      t[n++] = (PolyTerm) {coef, 4, {SI_E + i, q + j, SI_U + i, SI_V + j}};
      t[n++] = (PolyTerm) {-coef, 4, {SI_E + i, q + j, SI_U + j, SI_V + i}};
      t[n++] = (PolyTerm) {-coef, 4, {SI_E + j, q + i, SI_U + i, SI_V + j}};
      t[n++] = (PolyTerm) {coef, 4, {SI_E + j, q + i, SI_U + j, SI_V + i}};
    }
  }
  return n;
}


/**
 * @brief Return true where the nearest points of the lines of two segments
 * lie strictly inside both, the signs read by @p sign on `n . n`,
 * `(e x v) . n` and `(e x u) . n` and their differences
 * @details With `n` not zero the parameters of the nearest points are
 * `s = ((e x v) . n) / (n . n)` along the first segment and
 * `t = ((e x u) . n) / (n . n)` along the second; a parameter at 0 or 1
 * places the nearest point at an end, which the ends answer
 */
#define SEGMENT_INSIDE(snn, ss, ss1, st, st1) \
  ((snn) > 0 && (ss) > 0 && (ss1) < 0 && (st) > 0 && (st1) < 0)


/**
 * @brief Write the signs that place the nearest points of the lines of two
 * segments, 3D, where a bound on their rounding in doubles decides them, 0
 * where it does not
 * @details With `u = b - a`, `v = d - c`, `e = c - a` and `n = u x v`, the
 * signs are those of `(e x v) . n`, of `(e x v) . n - n . n`, of
 * `(e x u) . n` and of `(e x u) . n - n . n`: the nearest points lie inside
 * both segments exactly where they read 1, -1, 1, -1. Every difference,
 * product and sum errs by at most 4 DBL_EPSILON of the magnitudes it
 * combines, carried through the cross products as
 * #cross_product_sign carries the error of a cross product
 * The same evaluation gives `e . n` and `n . n` with a bound on the error of
 * each, from which #segment_interior_certify and #segment_interior_lower
 * bound the distance of the lines
 * @param[in] a,b The two ends of the first segment, three coordinates each
 * @param[in] c,d The two ends of the second segment, three coordinates each
 * @param[out] sign The four signs
 * @param[out] est `e . n`, a bound on its error, `n . n` and a bound on its
 * error
 */
static pg_attribute_always_inline void
segment_interior_signs(const double *a, const double *b, const double *c,
  const double *d, int *sign, double *est)
{
  const double eps = 4.0 * DBL_EPSILON, tiny = 0x1p-1000;
  const double u[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
  const double v[3] = {d[0] - c[0], d[1] - c[1], d[2] - c[2]};
  const double e[3] = {c[0] - a[0], c[1] - a[1], c[2] - a[2]};
  double n[3], en[3], ev[3], eev[3], eu[3], eeu[3];
  for (int k = 0; k < 3; k++)
  {
    int i = (k + 1) % 3, j = (k + 2) % 3;
    double l = u[i] * v[j], r = u[j] * v[i];
    n[k] = l - r;
    en[k] = eps * (fabs(l) + fabs(r)) + tiny;
    l = e[i] * v[j]; r = e[j] * v[i];
    ev[k] = l - r;
    eev[k] = eps * (fabs(l) + fabs(r)) + tiny;
    l = e[i] * u[j]; r = e[j] * u[i];
    eu[k] = l - r;
    eeu[k] = eps * (fabs(l) + fabs(r)) + tiny;
  }
  double nn = 0.0, ns = 0.0, nt = 0.0, enn = tiny, ens = tiny, ent = tiny;
  double dot = 0.0, edot = tiny;
  for (int k = 0; k < 3; k++)
  {
    dot += e[k] * n[k];
    edot += fabs(e[k]) * en[k] + eps * fabs(e[k] * n[k]) +
      eps * fabs(e[k]) * en[k];
    nn += n[k] * n[k];
    ns += ev[k] * n[k];
    nt += eu[k] * n[k];
    enn += 2.0 * fabs(n[k]) * en[k] + en[k] * en[k] + eps * n[k] * n[k];
    ens += fabs(ev[k]) * en[k] + fabs(n[k]) * eev[k] + eev[k] * en[k] +
      eps * fabs(ev[k] * n[k]);
    ent += fabs(eu[k]) * en[k] + fabs(n[k]) * eeu[k] + eeu[k] * en[k] +
      eps * fabs(eu[k] * n[k]);
  }
  const double val[4] = {ns, ns - nn, nt, nt - nn};
  const double err[4] = {ens, ens + enn + eps * fabs(ns - nn), ent,
    ent + enn + eps * fabs(nt - nn)};
  for (int i = 0; i < 4; i++)
  {
    double bound = err[i] * (1.0 + 1e-6) + tiny;
    sign[i] = (! isfinite(val[i]) || ! isfinite(bound) ||
      fabs(val[i]) <= bound) ? 0 : ((val[i] > 0.0) ? 1 : -1);
  }
  est[0] = dot;
  est[1] = edot * (1.0 + 1e-6) + tiny;
  est[2] = nn;
  est[3] = enn * (1.0 + 1e-6) + tiny;
  return;
}

/**
 * @brief Set in @p lo and @p hi bounds of the distance of the lines of two
 * segments, `|e . n| / sqrt(n . n)`, from the evaluation of
 * #segment_interior_signs, and return false where it bounds nothing
 * @details Each value lies within its error bound of the one read, and each
 * of the four operations forming a bound errs by half a unit in the last
 * place, which the factors `1 -+ 2^-50` cover
 */
static pg_attribute_always_inline bool
segment_interior_bounds(const double *est, double *lo, double *hi)
{
  double x = fabs(est[0]), ex = est[1], nn = est[2], enn = est[3];
  if (! (nn - enn > 0.0) || ! isfinite(x + ex) || ! isfinite(nn + enn))
    return false;
  *lo = (x - ex) / sqrt(nn + enn) * (1.0 - 0x1p-50);
  *hi = (x + ex) / sqrt(nn - enn) * (1.0 + 0x1p-50);
  return isfinite(*hi);
}

/**
 * @brief Return true where the double nearest the distance of the lines of
 * two segments is certified by the evaluation of #segment_interior_signs,
 * setting it in @p result
 * @details The distance lies within the bounds of #segment_interior_bounds;
 * where both lie strictly between the midpoints of a double and its two
 * neighbours, that double is the nearest. A midpoint is exact where the double
 * is above 2^-960, half its gap to a neighbour then being a double
 */
static bool
segment_interior_certify(const double *est, double *result)
{
  double lo, hi;
  if (! segment_interior_bounds(est, &lo, &hi) || ! (lo > 0x1p-960))
    return false;
  double c = lo + (hi - lo) / 2.0;
  double cd = nextafter(c, 0.0), cu = nextafter(c, INFINITY);
  if (! isfinite(cu))
    return false;
  if (lo > c - (c - cd) / 2.0 && hi < c + (cu - c) / 2.0)
  {
    *result = c;
    return true;
  }
  return false;
}

/**
 * @brief Return 1 where the nearest points of the lines of two segments in 3D
 * certainly lie inside both, setting in @p lo and @p hi bounds of their
 * distance, -1 where they certainly do not, and 0 where the evaluation in
 * doubles of #segment_interior_signs cannot tell
 * @details A walk over pairs of segments reads it to bound a pair: inside,
 * the distance is that of the lines; outside, an end of one of the segments
 * is nearest
 * @param[in] a,b The two ends of the first segment, three coordinates each
 * @param[in] c,d The two ends of the second segment, three coordinates each
 * @param[out] lo,hi Bounds of the distance where the function returns 1
 */
int
segment_interior_range(const double *a, const double *b, const double *c,
  const double *d, double *lo, double *hi)
{
  int q[4];
  double est[4];
  segment_interior_signs(a, b, c, d, q, est);
  if (q[0] < 0 || q[1] > 0 || q[2] < 0 || q[3] > 0)
    return -1;
  if (! SEGMENT_INSIDE(1, q[0], q[1], q[2], q[3]) ||
      ! segment_interior_bounds(est, lo, hi))
    return 0;
  if (*lo < 0.0)
    *lo = 0.0;
  return 1;
}

/* The value `|e . n| / sqrt(n . n)` that #nearest_double_exact searches */
typedef struct
{
  const WideDyadic *en, *nn;
} NearestLines;

/* The sign of `|e . n| / sqrt(n . n) - mid`, that of
 * `(e . n)^2 - mid^2 (n . n)` for a midpoint not negative */
static int
nearest_lines_side(const WideDyadic *mid, const void *arg)
{
  const NearestLines *v = (const NearestLines *) arg;
  if (mid->sign < 0)
    return 1;
  WideDyadic m2;
  wide_dyadic_mul(mid, mid, &m2);
  return wide_dyadic_product_cmp(v->en, v->en, &m2, v->nn);
}

/**
 * @brief Return true where the nearest points of two segments in 3D lie
 * inside both, setting in @p result the double nearest their distance, read
 * on #WideDyadic for any finite coordinates
 * @details The differences, `n = u x v` and the dot products have degree at
 * most four in the input doubles, which a #WideDyadic holds for every finite
 * input, and the rounding compares `(e . n)^2` with `mid^2 (n . n)`, degree
 * six, as a comparison of two products (#wide_dyadic_product_cmp)
 */
static bool
segment_interior_wide(const double *a, const double *b, const double *c,
  const double *d, double *result)
{
  WideDyadic u[3], v[3], e[3], n[3], x, y, t1, t2;
  for (int k = 0; k < 3; k++)
  {
    WideDyadic pa, pb, pc, pd;
    wide_dyadic_set_double(&pa, a[k]); wide_dyadic_set_double(&pb, b[k]);
    wide_dyadic_set_double(&pc, c[k]); wide_dyadic_set_double(&pd, d[k]);
    pa.sign = - pa.sign; pc.sign = - pc.sign;
    wide_dyadic_add(&pb, &pa, &u[k]);
    wide_dyadic_add(&pd, &pc, &v[k]);
    pc.sign = - pc.sign;
    wide_dyadic_add(&pc, &pa, &e[k]);
  }
  /* n = u x v, and the cross products of e with v and with u */
  WideDyadic ev[3], eu[3];
  const WideDyadic *lhs[3] = {u, e, e}, *rhs[3] = {v, v, u};
  WideDyadic *out[3] = {n, ev, eu};
  for (int w = 0; w < 3; w++)
    for (int k = 0; k < 3; k++)
    {
      int i = (k + 1) % 3, j = (k + 2) % 3;
      wide_dyadic_mul(&lhs[w][i], &rhs[w][j], &t1);
      wide_dyadic_mul(&lhs[w][j], &rhs[w][i], &t2);
      t2.sign = - t2.sign;
      wide_dyadic_add(&t1, &t2, &out[w][k]);
    }
  /* The dot products of n with itself, with e x v, with e x u and with e */
  WideDyadic dot[4];
  const WideDyadic *with[4] = {n, ev, eu, e};
  for (int w = 0; w < 4; w++)
  {
    dot[w].sign = 0; dot[w].len = 0; dot[w].exp = 0;
    for (int k = 0; k < 3; k++)
    {
      wide_dyadic_mul(&n[k], &with[w][k], &x);
      wide_dyadic_add(&dot[w], &x, &y);
      dot[w] = y;
    }
  }
  WideDyadic *nn = &dot[0], *en = &dot[3];
  int ss1 = wide_dyadic_cmp(&dot[1], nn), st1 = wide_dyadic_cmp(&dot[2], nn);
  if (! SEGMENT_INSIDE(nn->sign, dot[1].sign, ss1, dot[2].sign, st1))
    return false;
  if (en->sign == 0)
  {
    *result = 0.0;
    return true;
  }
  int ee, en2;
  double me = wide_dyadic_frexp(en, &ee), mn = wide_dyadic_frexp(nn, &en2);
  double ratio = me * me / mn;
  int ex = 2 * ee - en2;
  if (ex % 2 != 0)
  {
    ratio *= 2.0;
    ex -= 1;
  }
  double guess = ldexp(sqrt(ratio), ex / 2);
  NearestLines lines = {en, nn};
  *result = nearest_double_exact(guess, &nearest_lines_side, &lines);
  return true;
}


/**
 * @brief Return the sign of `(e . n)^2 - m^2 (n . n)` at a midpoint `m` given
 * as a pair, where its evaluation in pairs of doubles carries it, 0 otherwise
 * @param[in] en,een `e . n` as a pair and a bound on its error
 * @param[in] nn,enn `n . n` as a pair and a bound on its error
 * @param[in] mh,ml The midpoint as a pair
 */
static pg_attribute_always_inline int
segment_interior_mid_sign(const double *en, double een, const double *nn,
  double enn, double mh, double ml)
{
  double e2h, e2l, m2h, m2l, qh, ql, vh, vl;
  pair_mul(en[0], en[1], en[0], en[1], &e2h, &e2l);
  pair_mul(mh, ml, mh, ml, &m2h, &m2l);
  pair_mul(m2h, m2l, nn[0], nn[1], &qh, &ql);
  pair_add(e2h, e2l, - qh, - ql, &vh, &vl);
  double ae = fabs(en[0]), m2 = fabs(m2h), an = fabs(nn[0]);
  /* Values whose products of the pairs could underflow are not told */
  if (ae * ae < 0x1p-900 || m2 * an < 0x1p-900)
    return 0;
  double bound = (2.0 * ae * een + een * een + m2 * enn +
    0x1p-96 * (ae * ae + m2 * an)) * (1.0 + 16.0 * DBL_EPSILON);
  if (! isfinite(vh) || ! isfinite(bound))
    return 0;
  return (vh > bound) ? 1 : ((vh < - bound) ? -1 : 0);
}

/* The values of #segment_interior_distance in pairs of doubles, each with a
 * bound on its error: e . n, n . n, (e x v) . n and (e x u) . n */
typedef struct
{
  double en[2], nn[2], ns[2], nt[2];
  double een, enn, ens, ent;
} SegmentPairs;

/**
 * @brief Evaluate in pairs of doubles the values that place and round the
 * distance of the lines of two segments, from the differences `u`, `v` and
 * `e`, each a pair
 * @details The cross products
 * `n = u x v`, `e x v` and `e x u` and the dot products are formed as pairs,
 * each product and sum within 2^-104 of the magnitudes it combines, which the
 * bounds `2^-96` times the magnitude of each value cover with room for the
 * depth of its evaluation
 */
static void
segment_interior_pairs_from(double U[3][2], double V[3][2], double Ev[3][2],
  bool signs, SegmentPairs *pv)
{
  /* The cross products of u and v, and where the signs are asked, of e and v
   * and of e and u */
  const double (*lhs[3])[2] = {U, Ev, Ev}, (*rhs[3])[2] = {V, V, U};
  double X[3][3][2], mag[3][3];
  int nw = signs ? 3 : 1;
  for (int w = 0; w < nw; w++)
    for (int k = 0; k < 3; k++)
    {
      int i = (k + 1) % 3, j = (k + 2) % 3;
      double ph, pl, qh, ql;
      pair_mul(lhs[w][i][0], lhs[w][i][1], rhs[w][j][0], rhs[w][j][1], &ph,
        &pl);
      pair_mul(lhs[w][j][0], lhs[w][j][1], rhs[w][i][0], rhs[w][i][1], &qh,
        &ql);
      pair_add(ph, pl, - qh, - ql, &X[w][k][0], &X[w][k][1]);
      mag[w][k] = fabs(ph) + fabs(qh);
    }
  /* The dot products of n with e, with itself, and where the signs are asked,
   * with e x v and with e x u */
  double *out[4] = {pv->en, pv->nn, pv->ns, pv->nt};
  double *err[4] = {&pv->een, &pv->enn, &pv->ens, &pv->ent};
  int nd = signs ? 4 : 2;
  for (int d = 0; d < nd; d++)
  {
    double m = 0.0;
    out[d][0] = out[d][1] = 0.0;
    for (int k = 0; k < 3; k++)
    {
      const double *x = (d == 0) ? Ev[k] : (d == 1) ? X[0][k] : X[d - 1][k];
      double mx = (d == 0) ? fabs(Ev[k][0]) : (d == 1) ? mag[0][k] :
        mag[d - 1][k];
      double ph, pl;
      pair_mul(x[0], x[1], X[0][k][0], X[0][k][1], &ph, &pl);
      pair_add(out[d][0], out[d][1], ph, pl, &out[d][0], &out[d][1]);
      m += mx * mag[0][k];
    }
    *err[d] = 0x1p-96 * m;
  }
  return;
}

/**
 * @brief Evaluate the values of #segment_interior_pairs_from on the scaled
 * sums of #segment_interior_distance, each two doubles, a pair exactly
 */
static void
segment_interior_pairs(const ExactSum *sums, SegmentPairs *pv)
{
  double U[3][2], V[3][2], Ev[3][2];
  for (int i = 0; i < 3; i++)
  {
    const ExactSum *su = &sums[SI_U + i], *sv = &sums[SI_V + i],
      *se = &sums[SI_E + i];
    two_sum(su->e[su->n - 1], (su->n > 1) ? su->e[0] : 0.0, &U[i][0],
      &U[i][1]);
    two_sum(sv->e[sv->n - 1], (sv->n > 1) ? sv->e[0] : 0.0, &V[i][0],
      &V[i][1]);
    two_sum(se->e[se->n - 1], (se->n > 1) ? se->e[0] : 0.0, &Ev[i][0],
      &Ev[i][1]);
  }
  segment_interior_pairs_from(U, V, Ev, true, pv);
  return;
}

/* The sign of a value of #segment_interior_pairs where its bound tells it, 0
 * otherwise */
static inline int
segment_pair_sign(const double *v, double err)
{
  if (! isfinite(v[0]) || ! isfinite(err))
    return 0;
  return (v[0] > err) ? 1 : ((v[0] < - err) ? -1 : 0);
}

/* The sign of the difference of two values of #segment_interior_pairs where
 * its bound tells it, 0 otherwise */
static inline int
segment_pair_diff_sign(const double *a, double ea, const double *b,
  double eb)
{
  double h, l;
  pair_add(a[0], a[1], - b[0], - b[1], &h, &l);
  double bound = (ea + eb + 0x1p-100 * (fabs(a[0]) + fabs(b[0]))) *
    (1.0 + 16.0 * DBL_EPSILON);
  const double v[2] = {h, l};
  return segment_pair_sign(v, bound);
}

/**
 * @brief Return true where the double nearest the distance of the lines of
 * two segments, `|e . n| / sqrt(n . n)`, is certified by the evaluation in
 * pairs of doubles of #segment_interior_pairs, setting it in @p result
 * @details The candidate is the rounded quotient; it is the nearest double
 * where `(e . n)^2 - m^2 (n . n)` is certainly positive at its lower midpoint
 * and certainly negative at its upper one (#segment_interior_mid_sign), each
 * midpoint the pair of the scaled candidate and half its scaled gap. Values
 * below 2^-400, whose products of the pairs could underflow, decline
 * @param[in] pv The values in pairs of doubles
 * @param[in] f,g The scale and its inverse, powers of two
 * @param[out] result The distance where the function returns true
 */
static bool
segment_interior_certify_pairs(const SegmentPairs *pv, double f, double g,
  double *result)
{
  /* A scale that is no double, beyond 2^1023, leaves it to the search */
  if (! (f > 0.0) || ! isfinite(f) || ! (g > 0.0) || ! isfinite(g))
    return false;
  const double *en = pv->en, *nn = pv->nn;
  double een = pv->een, enn = pv->enn;
  if (! (fabs(en[0]) > 0x1p-400) || ! (nn[0] > 0x1p-400) ||
      ! (fabs(en[0]) > 2.0 * een) || ! (nn[0] > 2.0 * enn))
    return false;
  double c = fabs(en[0]) / sqrt(nn[0]) * g;
  if (! (c > 0x1p-900) || ! isfinite(c))
    return false;
  double cd = nextafter(c, 0.0), cu = nextafter(c, INFINITY);
  if (! isfinite(cu))
    return false;
  double cs = c * f;
  if (segment_interior_mid_sign(en, een, nn, enn, cs, - (c - cd) / 2.0 * f)
        > 0 &&
      segment_interior_mid_sign(en, een, nn, enn, cs, (cu - c) / 2.0 * f) < 0)
  {
    *result = c;
    return true;
  }
  return false;
}

/**
 * @brief Return true where the nearest points of two segments in 3D lie
 * inside both, setting in @p result the double nearest their distance
 * @details With `u = b - a`, `v = d - c`, `e = c - a` and `n = u x v` not
 * zero, the nearest points of the lines are at the parameters
 * `s = ((e x v) . n) / (n . n)` along the first and
 * `t = ((e x u) . n) / (n . n)` along the second; where both lie strictly
 * between 0 and 1 the squared distance is `(e . n)^2 / (n . n)`, the least
 * over both segments, since the squared distance is convex in the parameters
 * and that is its stationary point. The differences and the errors of their
 * rounding are scaled as #segment_square_least scales them, each sign is that
 * of a polynomial in exact sums (#polynomial_sign_exact), and the answer is
 * the smallest double whose upper midpoint `m` has `(e . n)^2 - m^2 (n . n)`
 * not positive, a polynomial of degree six, found by a gallop from the
 * rounded quotient, as #point_segment_distance_offset_exact finds its answer.
 * Where a difference overflows or the scaling leaves a product of six
 * outside the doubles, the values are read on #WideDyadic
 * (#segment_interior_wide)
 * @param[in] a,b The two ends of the first segment, three coordinates each
 * @param[in] c,d The two ends of the second segment, three coordinates each
 * @param[out] result The distance where the function returns true
 */
static bool
segment_interior_distance(const double *a, const double *b, const double *c,
  const double *d, double *result)
{
  /* The signs read in doubles settle all but the pairs near a boundary of
   * the interior: outside, the ends answer */
  int q[4];
  double est[4];
  segment_interior_signs(a, b, c, d, q, est);
  if (q[0] < 0 || q[1] > 0 || q[2] < 0 || q[3] > 0)
    return false;
  bool inside = SEGMENT_INSIDE(1, q[0], q[1], q[2], q[3]);
  /* Inside, the evaluation in doubles answers wherever it places the distance
   * clear of every midpoint */
  if (inside && segment_interior_certify(est, result))
    return true;
  /* On two doubles with a bound on their error, an exact zero held as one
   * (#DDBound): the cross product n, n . n and, where the signs in doubles
   * do not tell, the parameters' numerators; inside, the root of
   * (e . n)^2 / (n . n), from e . n read as the triple product exactly where
   * its rounded factors cancel (#ddb_triple3_points) */
  {
    DDBound fu[3], fv[3], fe[3], fn[3];
    ddb_diff3(b, a, fu);
    ddb_diff3(d, c, fv);
    ddb_diff3(c, a, fe);
    ddb_cross3(fu, fv, fn);
    DDBound fnn = ddb_dot3(fn, fn);
    if (! inside)
    {
      /* Parallel lines, their cross product an exact zero, are nearest at an
       * end of one of the segments */
      if (ddb_is_zero(fnn))
        return false;
      if (ddb_sign(fnn) > 0)
      {
        DDBound fev[3], feu[3];
        ddb_cross3(fe, fv, fev);
        ddb_cross3(fe, fu, feu);
        DDBound fns = ddb_dot3(fev, fn), fnt = ddb_dot3(feu, fn);
        DDBound fns1 = ddb_sub(fns, fnn), fnt1 = ddb_sub(fnt, fnn);
        const int sg[4] = {ddb_sign(fns), ddb_sign(fns1), ddb_sign(fnt),
          ddb_sign(fnt1)};
        /* A parameter at 0 or 1 exactly places the nearest point of the
         * lines at an end of a segment, which the ends answer */
        if (sg[0] < 0 || sg[1] > 0 || sg[2] < 0 || sg[3] > 0 ||
            ddb_is_zero(fns) || ddb_is_zero(fns1) || ddb_is_zero(fnt) ||
            ddb_is_zero(fnt1))
          return false;
        inside = SEGMENT_INSIDE(1, sg[0], sg[1], sg[2], sg[3]);
      }
    }
    if (inside)
    {
      DDBound fen = ddb_dot3(fe, fn);
      if (ddb_nearest_root(ddb_mul(fen, fen), fnn, result))
        return true;
      ddb_triple3_points(a, b, c, d, &fen);
      if (ddb_is_zero(fen))
      {
        *result = 0.0;
        return true;
      }
      if (ddb_nearest_root(ddb_mul(fen, fen), fnn, result))
        return true;
    }
  }
  /* Where every difference lies from 2^-100 to 2^100 in magnitude, each is a
   * pair exactly (#two_diff) and no product of six of them leaves the
   * doubles, so the evaluation in pairs reads them as they are */
  SegmentPairs fv;
  bool have_fv = false;
  {
    double U[3][2], V[3][2], Ev[3][2];
    const double *from[3] = {a, c, a}, *to[3] = {b, d, c};
    double (*dst[3])[2] = {U, V, Ev};
    bool ok = true;
    for (int w = 0; w < 3 && ok; w++)
      for (int i = 0; i < 3 && ok; i++)
      {
        two_diff(to[w][i], from[w][i], &dst[w][i][0], &dst[w][i][1]);
        double m = fabs(dst[w][i][0]);
        if (! (m <= 0x1p100) || (m != 0.0 && m < 0x1p-100))
          ok = false;
      }
    if (ok)
    {
      segment_interior_pairs_from(U, V, Ev, ! inside, &fv);
      have_fv = true;
      if (! inside)
      {
        const int pq[5] = {segment_pair_sign(fv.nn, fv.enn),
          segment_pair_sign(fv.ns, fv.ens),
          segment_pair_diff_sign(fv.ns, fv.ens, fv.nn, fv.enn),
          segment_pair_sign(fv.nt, fv.ent),
          segment_pair_diff_sign(fv.nt, fv.ent, fv.nn, fv.enn)};
        if (pq[0] < 0 || pq[1] < 0 || pq[2] > 0 || pq[3] < 0 || pq[4] > 0)
          return false;
        inside = SEGMENT_INSIDE(pq[0], pq[1], pq[2], pq[3], pq[4]);
      }
      if (inside && segment_interior_certify_pairs(&fv, 1.0, 1.0, result))
        return true;
    }
  }
  double dv[3][3], ev[3][3], m[3], maxd = 0.0;
  int st[3];
  st[0] = point_distance_diffs(a, b, 3, dv[0], ev[0], &m[0]);
  st[1] = point_distance_diffs(c, d, 3, dv[1], ev[1], &m[1]);
  st[2] = point_distance_diffs(a, c, 3, dv[2], ev[2], &m[2]);
  for (int j = 0; j < 3; j++)
  {
    if (st[j] == 1)
      return false;
    if (st[j] == 2)
      return segment_interior_wide(a, b, c, d, result);
    if (m[j] > maxd)
      maxd = m[j];
  }
  if (maxd == 0.0)
    return false;
  int k;
  double f1, f2, g1, g2;
  point_distance_scale(maxd, &k, &f1, &f2, &g1, &g2);
  ExactSum sums[10];
  for (int j = 0; j < 3; j++)
    for (int i = 0; i < 3; i++)
    {
      double x = dv[j][i], y = ev[j][i];
      if ((x != 0.0 && double_lowest_bit(x) + k < -179) ||
          (y != 0.0 && double_lowest_bit(y) + k < -179))
        return segment_interior_wide(a, b, c, d, result);
      const double t[2] = {x * f1 * f2, y * f1 * f2};
      exact_sum_set(&sums[3 * j + i], t, 2);
    }
  /* Whether the nearest points of the lines lie inside both segments, where
   * the signs in doubles do not tell: in pairs of doubles, the evaluation of
   * the differences as they are where it was read, at the scale @p pscale
   * of its values, then exactly */
  SegmentPairs pv;
  double pscale = 1.0;
  if (have_fv)
    pv = fv;
  else
  {
    segment_interior_pairs(sums, &pv);
    pscale = f1 * f2;
    if (! inside)
    {
      const int pq[5] = {segment_pair_sign(pv.nn, pv.enn),
        segment_pair_sign(pv.ns, pv.ens),
        segment_pair_diff_sign(pv.ns, pv.ens, pv.nn, pv.enn),
        segment_pair_sign(pv.nt, pv.ent),
        segment_pair_diff_sign(pv.nt, pv.ent, pv.nn, pv.enn)};
      if (pq[0] < 0 || pq[1] < 0 || pq[2] > 0 || pq[3] < 0 || pq[4] > 0)
        return false;
      inside = SEGMENT_INSIDE(pq[0], pq[1], pq[2], pq[3], pq[4]);
    }
  }
  PolyTerm terms[45];
  int nt;
  if (! inside)
  {
    /* n . n is positive exactly where a component of n = u x v is not zero,
     * each a difference of two products, of degree two */
    int snn = 0;
    for (int kk = 0; kk < 3 && snn == 0; kk++)
    {
      int i = (kk + 1) % 3, j = (kk + 2) % 3;
      const PolyTerm nk[2] = {{1.0, 2, {SI_U + i, SI_V + j}},
        {-1.0, 2, {SI_U + j, SI_V + i}}};
      snn = (polynomial_sign_exact(sums, nk, 2) != 0) ? 1 : 0;
    }
    if (snn <= 0)
      return false;
    int sgn[4];
    for (int w = 1; w <= 2; w++)
    {
      nt = segment_interior_terms(w, 1.0, terms);
      sgn[2 * (w - 1)] = polynomial_sign_exact(sums, terms, nt);
      nt += segment_interior_terms(0, -1.0, terms + nt);
      sgn[2 * (w - 1) + 1] = polynomial_sign_exact(sums, terms, nt);
    }
    if (! SEGMENT_INSIDE(snn, sgn[0], sgn[1], sgn[2], sgn[3]))
      return false;
  }

  /* The evaluation in pairs of doubles answers wherever it places the
   * distance clear of the midpoints of the candidate */
  if (! have_fv &&
      segment_interior_certify_pairs(&pv, f1 * f2, g1 * g2, result))
    return true;

  /* e . n, six terms of degree three, and (e . n)^2 - m^2 (n . n) */
  PolyTerm en[6];
  int ne = 0;
  for (int kk = 0; kk < 3; kk++)
  {
    int i = (kk + 1) % 3, j = (kk + 2) % 3;
    en[ne++] = (PolyTerm) {1.0, 3, {SI_E + kk, SI_U + i, SI_V + j}};
    en[ne++] = (PolyTerm) {-1.0, 3, {SI_E + kk, SI_U + j, SI_V + i}};
  }
  if (polynomial_sign_exact(sums, en, ne) == 0)
  {
    *result = 0.0;
    return true;
  }
  nt = 0;
  for (int i = 0; i < ne; i++)
    for (int j = 0; j < ne; j++)
    {
      PolyTerm *t = &terms[nt++];
      t->coef = en[i].coef * en[j].coef;
      t->deg = 6;
      memcpy(t->sum, en[i].sum, 3 * sizeof(int));
      memcpy(t->sum + 3, en[j].sum, 3 * sizeof(int));
    }
  PolyTerm nn[9];
  int nnn = segment_interior_terms(0, -1.0, nn);
  for (int i = 0; i < nnn; i++)
  {
    PolyTerm *t = &terms[nt++];
    t->coef = nn[i].coef;
    t->deg = 6;
    t->sum[0] = t->sum[1] = SI_M;
    memcpy(t->sum + 2, nn[i].sum, 4 * sizeof(int));
  }

  /* First candidate, from the rounded quotient */
  double cx[3], dot = 0.0, len2 = 0.0;
  for (int kk = 0; kk < 3; kk++)
  {
    int i = (kk + 1) % 3, j = (kk + 2) % 3;
    cx[kk] = sums[SI_U + i].approx * sums[SI_V + j].approx -
      sums[SI_U + j].approx * sums[SI_V + i].approx;
    len2 += cx[kk] * cx[kk];
    dot += sums[SI_E + kk].approx * cx[kk];
  }
  double cand = fabs(dot) / sqrt(len2) * g1 * g2;
  if (! (cand > 0.0))
    cand = 0.0;
  if (isinf(cand))
    cand = DBL_MAX;
  uint64_t bits;
  memcpy(&bits, &cand, sizeof(bits));

  /* The answer is the smallest bit pattern that is not beyond: gallop from the
   * candidate to bracket it, then halve the bracket. A candidate or a half
   * gap scaled below 2^-179 leaves a product of six outside the doubles, and
   * the values are then read on #WideDyadic */
  bool wide = false;
#define SI_BEYOND(bb, res) \
  do { \
    double cc, nx; \
    uint64_t b1 = (bb), b2 = (bb) + 1; \
    memcpy(&cc, &b1, sizeof(cc)); \
    memcpy(&nx, &b2, sizeof(nx)); \
    if ((cc != 0.0 && double_lowest_bit(cc) + k < -179) || \
        (! isinf(nx) && double_lowest_bit(nx - cc) - 1 + k < -179)) \
    { \
      wide = true; \
      (res) = false; \
      break; \
    } \
    double hh = isinf(nx) ? ldexp(1.0, 970 + k) : (nx - cc) * f1 * f2 / 2.0; \
    /* The evaluation in pairs of doubles first, which tells every midpoint \
     * clear of the distance by more than its bound */ \
    double hp = isinf(nx) ? ldexp(1.0, 970) * pscale : \
      (nx - cc) / 2.0 * pscale; \
    int sign = segment_interior_mid_sign(pv.en, pv.een, pv.nn, pv.enn, \
      cc * pscale, hp); \
    if (sign == 0) \
    { \
      const double mt[2] = {cc * f1 * f2, hh}; \
      exact_sum_set(&sums[SI_M], mt, 2); \
      sign = polynomial_sign_exact(sums, terms, nt); \
    } \
    (res) = sign > 0 || (sign == 0 && (b1 & 1) != 0); \
  } while (0)
  uint64_t lo, hi;
  bool beyond;
  SI_BEYOND(bits, beyond);
  if (beyond)
  {
    /* The answer is above: lo is beyond, hi is not */
    uint64_t step = 1;
    lo = bits;
    const uint64_t inf_bits = 0x7FF0000000000000ULL;
    while (! wide)
    {
      hi = (lo + step < inf_bits) ? lo + step : inf_bits;
      if (hi == inf_bits)
        break;
      SI_BEYOND(hi, beyond);
      if (! beyond)
        break;
      lo = hi;
      step *= 2;
    }
  }
  else
  {
    /* The answer is at or below: hi is not beyond, lo is beyond or zero */
    uint64_t step = 1;
    hi = bits;
    lo = 0;
    while (! wide && hi > 0)
    {
      lo = (hi > step) ? hi - step : 0;
      SI_BEYOND(lo, beyond);
      if (beyond)
        break;
      hi = lo;
      step *= 2;
    }
    if (! wide && hi == 0)
    {
      *result = 0.0;
      return true;
    }
  }
  /* lo is beyond, hi is not, and the answer is the least not beyond */
  while (! wide && hi - lo > 1)
  {
    uint64_t mid = lo + (hi - lo) / 2;
    SI_BEYOND(mid, beyond);
    if (beyond)
      lo = mid;
    else
      hi = mid;
  }
#undef SI_BEYOND
  if (wide)
    return segment_interior_wide(a, b, c, d, result);
  memcpy(result, &hi, sizeof(double));
  return true;
}

/**
 * @brief Return the distance of a point to a segment in doubles, which orders
 * the ends #segment_distance_exact rounds and decides no answer
 */
static pg_attribute_always_inline double
point_segment_distance_approx(const double *p, const double *a,
  const double *b, int ndims)
{
  double u[3], w[3], l2 = 0.0, wu = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    u[i] = b[i] - a[i];
    w[i] = p[i] - a[i];
    l2 += u[i] * u[i];
    wu += w[i] * u[i];
  }
  double t = (l2 > 0.0) ? wu / l2 : 0.0;
  t = (t < 0.0) ? 0.0 : ((t > 1.0) ? 1.0 : t);
  double result = 0.0;
  for (int i = 0; i < ndims; i++)
  {
    double dd = w[i] - t * u[i];
    result += dd * dd;
  }
  return sqrt(result);
}

/**
 * @brief Return the double nearest the distance of two segments, read on the
 * coordinates as they are (#segment_distance_exact)
 * @param[in] big The largest magnitude of their coordinates
 */
static double
segment_distance_unscaled(const double *a, const double *b, const double *c,
  const double *d, int ndims, double big)
{
  if (ndims == 2)
  {
    IntersectResult meet = linesegm_intersect(a[0], a[1], b[0], b[1], c[0],
      c[1], d[0], d[1]);
    if (meet.type != INTERSECT_NONE)
      return 0.0;
  }
  else
  {
    double inner;
    if (segment_interior_distance(a, b, c, d, &inner))
      return inner;
  }
  /* The four ends, in the order of their distance in doubles, each rounded
   * once while it may be nearer than the least found: where the coordinates
   * lie within a factor 2^400 of 1, that distance errs far less than 2^-40 of
   * the largest coordinate, so an end it places farther than the least by
   * that margin cannot answer less; rounding is monotone */
  const double *pt[4][3] = {{a, c, d}, {b, c, d}, {c, a, b}, {d, a, b}};
  bool filter = big >= 0x1p-400 && big <= 0x1p400;
  double margin = 0x1p-40 * big;
  double approx[4];
  int order[4] = {0, 1, 2, 3};
  for (int i = 0; i < 4; i++)
    approx[i] = point_segment_distance_approx(pt[i][0], pt[i][1], pt[i][2],
      ndims);
  for (int i = 1; i < 4; i++)
    for (int j = i; j > 0 && approx[order[j]] < approx[order[j - 1]]; j--)
    {
      int t = order[j];
      order[j] = order[j - 1];
      order[j - 1] = t;
    }
  double best = INFINITY;
  for (int i = 0; i < 4; i++)
  {
    int k = order[i];
    if (filter && approx[k] > best + margin)
      break;
    best = fmin(best, point_segment_distance_offset_exact(pt[k][0], pt[k][1],
      pt[k][2], ndims, 0.0));
  }
  return best;
}

/**
 * @brief Return the power of two of the largest coordinate of the ends of two
 * segments where scaling them by its inverse is exact and needed, 0
 * otherwise
 * @details Scaling is not needed where the largest coordinate lies within
 * 2^100 of 1, and not exact where it rounds the smallest nonzero coordinate
 * below the normal range
 * @param[in] pts The four ends, @p ndims coordinates each
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] big The largest magnitude of their coordinates
 */
static inline int
segment_distance_scale_exponent(const double *const *pts, int ndims,
  double big)
{
  /* The largest coordinate alone decides the common case, in range */
  if (big == 0.0 || (big >= 0x1p-100 && big <= 0x1p100) || ! isfinite(big))
    return 0;
  double small = INFINITY;
  for (int i = 0; i < 4; i++)
    for (int k = 0; k < ndims; k++)
    {
      double v = fabs(pts[i][k]);
      if (isnan(v))
        return 0;
      if (v > 0.0)
        small = fmin(small, v);
    }
  int e = ilogb(big);
  if (ilogb(small) - e < DBL_MIN_EXP - 1)
    return 0;
  return e;
}

/**
 * @brief Return the double nearest the distance of two segments
 * @details Two segments meet or not by exact orientations
 * (#linesegm_intersect), and where they meet the distance is 0. In 3D two
 * segments whose nearest points lie inside both answer that distance
 * (#segment_interior_distance), 0 where they cross. Otherwise two segments
 * are nearest at an end of one of them, so the distance is
 * the least of the distances of the two ends of each to the other: which one,
 * by their squares (#segment_distance_square), and that one rounded once to
 * the double nearest it (#point_segment_distance_offset_exact). Where the
 * squares are not compared, each of the four is rounded, and rounding is
 * monotone, so the least of them is the double nearest the least distance. A
 * segment of no length is a point: it meets nothing in #linesegm_intersect,
 * and its distance to the other is one of the four. The bounded values of
 * the 3D kernels (#DDBound) read products of up to the sixth degree of the
 * coordinates, which underflow or overflow far from 1, and a distance scales
 * exactly with a power of two, so the kernels read the coordinates brought
 * near 1 (#segment_distance_scale_exponent)
 * @param[in] a,b The two ends of the first segment, @p ndims coordinates each
 * @param[in] c,d The two ends of the second segment, @p ndims coordinates
 * each
 * @param[in] ndims Number of coordinates, 2 or 3
 */
double
segment_distance_exact(const double *a, const double *b, const double *c,
  const double *d, int ndims)
{
  assert(ndims == 2 || ndims == 3);
  const double *pts[4] = {a, b, c, d};
  /* A NaN compares false, so it leaves the largest coordinate as fmax does */
  double big = 0.0;
  for (int i = 0; i < 4; i++)
    for (int k = 0; k < ndims; k++)
    {
      double v = fabs(pts[i][k]);
      if (v > big)
        big = v;
    }
  int e = segment_distance_scale_exponent(pts, ndims, big);
  if (e != 0)
  {
    double sa[3] = {0.0}, sb[3] = {0.0}, sc[3] = {0.0}, sd[3] = {0.0};
    for (int k = 0; k < ndims; k++)
    {
      sa[k] = ldexp(a[k], -e);
      sb[k] = ldexp(b[k], -e);
      sc[k] = ldexp(c[k], -e);
      sd[k] = ldexp(d[k], -e);
    }
    double r = segment_distance_unscaled(sa, sb, sc, sd, ndims,
      ldexp(big, -e));
    double dist = ldexp(r, e);
    /* Scaled back exactly where it lies in the normal range */
    if (r == 0.0 || (isfinite(dist) && dist >= DBL_MIN))
      return dist;
  }
  return segment_distance_unscaled(a, b, c, d, ndims, big);
}

/**
 * @brief Return the double nearest the distance of two segments, given a
 * bound on the magnitudes of their coordinates (#segment_distance_exact)
 * @details A bound within 2^100 of 1 leaves no coordinate to scale, which
 * spares the walk of two geometries reading the largest coordinate of every
 * pair; a bound above the largest coordinate widens the margin of the ends,
 * which stays an upper bound of their error
 * @param[in] a,b The two ends of the first segment, @p ndims coordinates each
 * @param[in] c,d The two ends of the second segment, @p ndims coordinates
 * each
 * @param[in] ndims Number of coordinates, 2 or 3
 * @param[in] big A bound on the magnitudes of their coordinates
 */
double
segment_distance_exact_bounded(const double *a, const double *b,
  const double *c, const double *d, int ndims, double big)
{
  assert(ndims == 2 || ndims == 3);
  if (big >= 0x1p-100 && big <= 0x1p100)
    return segment_distance_unscaled(a, b, c, d, ndims, big);
  return segment_distance_exact(a, b, c, d, ndims);
}

/**
 * @brief Return the sign of the triple product `<p, q x r>` of three vectors,
 * computed exactly
 * @details The triple product is the determinant of the three vectors, a sum
 * of six products of three coordinates. Each product of two of them is its
 * rounded value plus its error (#two_product), that pair times the third is
 * again an expansion (#scale_expansion), and the six are added into one whose
 * last component carries the sign of the whole. #triple_product_sign calls it
 * where its filter cannot tell
 * @note Exact where no product of coordinates overflows or underflows
 * @return 1 where `r` lies on the side of the plane of `p` and `q` that their
 * cross product points to, -1 on the other, 0 exactly where the three vectors
 * are coplanar
 */
int
triple_product_sign_exact(const POINT3D *p, const POINT3D *q,
  const POINT3D *r)
{
  /* The six terms, each a coordinate of `p` times a product of a coordinate
   * of `q` and one of `r`, with its sign */
  const double f[6] = {p->x, - p->x, p->y, - p->y, p->z, - p->z};
  const double g[6] = {q->y, q->z, q->z, q->x, q->x, q->y};
  const double h[6] = {r->z, r->y, r->x, r->z, r->y, r->x};
  double acc[24], tmp[24];
  int len = 0;
  for (int k = 0; k < 6; k++)
  {
    double x, y, e[2], term[4];
    two_product(g[k], h[k], &x, &y);
    int elen = 0;
    if (y != 0.0)
      e[elen++] = y;
    if (x != 0.0)
      e[elen++] = x;
    if (elen == 0 || f[k] == 0.0)
      continue;
    int tlen = scale_expansion(elen, e, f[k], term);
    if (len == 0)
    {
      memcpy(acc, term, (size_t) tlen * sizeof(double));
      len = tlen;
    }
    else
      len = expansion_sum(len, acc, tlen, term, acc, tmp);
  }
  if (len == 0)
    return 0;
  double top = acc[len - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Return the sign of the incircle determinant of four points, computed
 * exactly from their coordinates
 * @details Each coordinate difference is its rounded value plus the error of
 * the rounding (#two_diff), the three lifts and the three cross products are
 * formed from those as expansions, and the three terms are added, so the last
 * component of the sum carries the sign of the determinant
 * @note Exact where no product of coordinate differences overflows or
 * underflows
 */
static int
incircle_sign_exact(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  /* The differences A - D, B - D and C - D, x then y */
  const double from[6] = {ax, ay, bx, by, cx, cy};
  const double to[6] = {dx, dy, dx, dy, dx, dy};
  double v[6][2];
  int vl[6];
  for (int i = 0; i < 6; i++)
  {
    double x, y;
    two_diff(from[i], to[i], &x, &y);
    vl[i] = 0;
    if (y != 0.0)
      v[i][vl[i]++] = y;
    if (x != 0.0 || vl[i] == 0)
      v[i][vl[i]++] = x;
  }
  double pa[8], pb[8], scratch[16], part[4];
  /* The lifts |A - D|^2, |B - D|^2, |C - D|^2 */
  double lift[3][16];
  int liftl[3];
  for (int k = 0; k < 3; k++)
  {
    const double *x = v[2 * k], *y = v[2 * k + 1];
    int lx = vl[2 * k], ly = vl[2 * k + 1];
    int l1 = expansion_product(lx, x, lx, x, pa, scratch, part);
    int l2 = expansion_product(ly, y, ly, y, pb, scratch, part);
    liftl[k] = expansion_sum(l1, pa, l2, pb, lift[k], scratch);
  }
  /* The cross products of (B, C), (C, A) and (A, B), each point taken from D */
  static const int cu[3] = {2, 4, 0}, cv[3] = {4, 0, 2};
  double cross[3][16];
  int crossl[3];
  for (int k = 0; k < 3; k++)
  {
    int u = cu[k], w = cv[k];
    int l1 = expansion_product(vl[u], v[u], vl[w + 1], v[w + 1], pa, scratch,
      part);
    int l2 = expansion_product(vl[w], v[w], vl[u + 1], v[u + 1], pb, scratch,
      part);
    for (int i = 0; i < l2; i++)
      pb[i] = - pb[i];
    crossl[k] = expansion_sum(l1, pa, l2, pb, cross[k], scratch);
  }
  /* The determinant, the sum of each lift times the cross product of the
   * other two points */
  double term[512], tmp[512], part2[32], acc[1536], acc2[1536];
  int accl = 0;
  for (int k = 0; k < 3; k++)
  {
    int tl = expansion_product(liftl[k], lift[k], crossl[k], cross[k], term,
      tmp, part2);
    if (accl == 0)
    {
      memcpy(acc, term, (size_t) tl * sizeof(double));
      accl = tl;
    }
    else
      accl = expansion_sum(accl, acc, tl, term, acc, acc2);
  }
  double top = acc[accl - 1];
  return (top > 0.0) ? 1 : ((top < 0.0) ? -1 : 0);
}

/**
 * @brief Set the sum of two products of doubles as a pair of doubles, its
 * rounded value and the rest, and the sum of the magnitudes of the products
 * @details The products are exact as their rounded values and their errors
 * (#two_product), the rounded values are added exactly (#two_sum), and the
 * errors are added to the error of that sum, two roundings that together lie
 * within 3.01 u^2 times the magnitude, u being DBL_EPSILON / 2
 */
static inline void
incircle_pair(double a, double b, double c, double d, double *h, double *l,
  double *mag)
{
  double p, pe, q, qe, s, se;
  two_product(a, b, &p, &pe);
  two_product(c, d, &q, &qe);
  two_sum(p, q, &s, &se);
  two_sum(s, se + (pe + qe), h, l);
  *mag = fabs(p) + fabs(q);
}

/**
 * @brief Return the sign of the incircle determinant of four points where its
 * evaluation in pairs of doubles carries it, and 0 where it cannot tell
 * @details The second filter between the one of #incircle_sign and
 * #incircle_sign_exact, for four points whose coordinate differences from D
 * are exact doubles, as those of nearby points are (Sterbenz). Each lift and
 * each cross product is a pair of doubles within 3.01 u^2 of its magnitude
 * (#incircle_pair), u being DBL_EPSILON / 2; each term, the product of two
 * pairs, is a pair within 14.1 u^2 of the product of their magnitudes; and
 * the sum of the three terms is a pair within 26.2 u^2 of the permanent, the
 * sum of those products. Where the pair stands clear of 128 u^2 times the
 * permanent as computed, its sign is the determinant's. Where a difference
 * is not exact, or lies outside 2^-200 to 2^200, which keeps every product
 * and every error of a product a normal double, it declines. Mirrors
 * #point_motion_dwithin_filter, the second filter of a distance
 */
static int
incircle_sign_filter(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  /* The differences A - D, B - D and C - D, x then y */
  const double from[6] = {ax, ay, bx, by, cx, cy};
  const double to[6] = {dx, dy, dx, dy, dx, dy};
  double v[6];
  for (int i = 0; i < 6; i++)
  {
    double err;
    two_diff(from[i], to[i], &v[i], &err);
    double m = fabs(v[i]);
    if (err != 0.0 || (m != 0.0 && (m < 0x1p-200 || m > 0x1p200)))
      return 0;
  }
  /* Each lift times the cross product of the other two points */
  static const int cu[3] = {2, 4, 0}, cv[3] = {4, 0, 2};
  double th[3], tl[3], permanent = 0.0;
  for (int k = 0; k < 3; k++)
  {
    double lh, ll, lmag, ch, cl, cmag;
    incircle_pair(v[2 * k], v[2 * k], v[2 * k + 1], v[2 * k + 1], &lh, &ll,
      &lmag);
    int u = cu[k], w = cv[k];
    incircle_pair(v[u], v[w + 1], - v[w], v[u + 1], &ch, &cl, &cmag);
    double p, pe;
    two_product(lh, ch, &p, &pe);
    two_sum(p, (pe + lh * cl) + ll * ch, &th[k], &tl[k]);
    permanent += lmag * cmag;
  }
  /* The sum of the three terms */
  double s1, e1, s2, e2;
  two_sum(th[0], th[1], &s1, &e1);
  two_sum(s1, th[2], &s2, &e2);
  double det = s2 + ((((e1 + e2) + tl[0]) + tl[1]) + tl[2]);
  double bound = 32.0 * DBL_EPSILON * DBL_EPSILON * permanent;
  if (det > bound)
    return 1;
  if (det < - bound)
    return -1;
  return 0;
}

/**
 * @brief Return the sign of the incircle determinant of four points, decided
 * exactly
 * @details Positive where D lies inside the circle through A, B and C taken
 * counterclockwise, negative where it lies outside, and 0 exactly where the
 * four points lie on one circle. The determinant is evaluated in double and
 * read against Shewchuk's bound for it, formed from its own terms; where it
 * cannot tell, the evaluation in pairs of doubles of #incircle_sign_filter
 * is read against its own bound. Both are filters, never tolerances, since
 * where neither can tell the exact sign is computed (#incircle_sign_exact)
 */
int
incircle_sign(double ax, double ay, double bx, double by, double cx,
  double cy, double dx, double dy)
{
  double adx = ax - dx, ady = ay - dy, bdx = bx - dx, bdy = by - dy;
  double cdx = cx - dx, cdy = cy - dy;
  double bdxcdy = bdx * cdy, cdxbdy = cdx * bdy;
  double cdxady = cdx * ady, adxcdy = adx * cdy;
  double adxbdy = adx * bdy, bdxady = bdx * ady;
  double alift = adx * adx + ady * ady;
  double blift = bdx * bdx + bdy * bdy;
  double clift = cdx * cdx + cdy * cdy;
  double det = alift * (bdxcdy - cdxbdy) + blift * (cdxady - adxcdy) +
    clift * (adxbdy - bdxady);
  double permanent = (fabs(bdxcdy) + fabs(cdxbdy)) * alift +
    (fabs(cdxady) + fabs(adxcdy)) * blift +
    (fabs(adxbdy) + fabs(bdxady)) * clift;
  double bound = (10.0 + 96.0 * DBL_EPSILON) * DBL_EPSILON * permanent;
  if (det > bound)
    return 1;
  if (det < - bound)
    return -1;
  int sign = incircle_sign_filter(ax, ay, bx, by, cx, cy, dx, dy);
  if (sign != 0)
    return sign;
  return incircle_sign_exact(ax, ay, bx, by, cx, cy, dx, dy);
}

/**
 * @brief Return true if a point lies exactly on the circle of an arc
 * @details The circle is the one the arc's input points fix: its start, middle
 * and end, or the two ends of the diameter of a half of a full circle. A point
 * lies on the first where it is cocircular with the three (#incircle_sign),
 * and on the second where it sees the diameter at a right angle, the vectors
 * to its two ends then having a zero dot product (Thales), which is the sign
 * of a cross product (#cross_product_sign). Either way an input point is put
 * on the circle or off it exactly, at every scale, which the radial distance
 * from a constructed centre cannot do. The circle of a centred arc is its
 * centre and radius, and a point lies on it where |Q - C|^2 = r^2 exactly
 */
/* The side of the circle of a centred arc a point lies on, decided
 * exactly: the sign of |Q - C|^2 - r^2, a polynomial of degree two in exact
 * sums of the doubles (#polynomial_sign_exact) */
static int
centred_circle_side(const Edge *e, double qx, double qy)
{
  double t[3][2] = {{qx, -e->cx}, {qy, -e->cy}, {e->radius, 0.0}};
  ExactSum sums[3];
  for (int k = 0; k < 3; k++)
    exact_sum_set(&sums[k], t[k], 2);
  PolyTerm terms[3] = {
    {1.0, 2, {0, 0}}, {1.0, 2, {1, 1}}, {-1.0, 2, {2, 2}}};
  return polynomial_sign_exact(sums, terms, 3);
}

bool
point_on_arc_circle(const Edge *e, double qx, double qy)
{
  if (e->centred)
    return centred_circle_side(e, qx, qy) == 0;
  if ((qx == e->sx && qy == e->sy) || (qx == e->ex && qy == e->ey) ||
      (! e->diameter && qx == e->mx && qy == e->my))
    return true;
  if (e->diameter)
    /* (q - s) . (q - e), written as the cross product of q - s with the
     * vector (e.y - q.y, q.x - e.x) */
    return cross_product_sign(e->sx, e->sy, qx, qy, qy, e->ex, e->ey, qx) ==
      0;
  return incircle_sign(e->sx, e->sy, e->mx, e->my, e->ex, e->ey, qx, qy) == 0;
}

/**
 * @brief Return the side of the circle of an arc a point lies on, decided
 * exactly: -1 inside it, 0 on it, 1 outside it
 * @details The circle is the one #point_on_arc_circle reads. For a half of a
 * full circle the side is the sign of the dot product of the vectors from the
 * point to the two ends of the diameter, negative where the point sees the
 * diameter at an obtuse angle, which is inside (Thales). For an arc given by
 * three points it is the sign of #incircle_sign, positive inside the circle
 * of three points turning counterclockwise, read against the turn they make.
 * For a centred arc it is the sign of |Q - C|^2 - r^2
 */
int
arc_circle_side(const Edge *e, double qx, double qy)
{
  if (e->centred)
    return centred_circle_side(e, qx, qy);
  if (e->diameter)
    return cross_product_sign(e->sx, e->sy, qx, qy, qy, e->ex, e->ey, qx);
  int turn = cross_product_sign(e->sx, e->sy, e->mx, e->my, e->sx, e->sy,
    e->ex, e->ey);
  return - turn * incircle_sign(e->sx, e->sy, e->mx, e->my, e->ex, e->ey,
    qx, qy);
}

/**
 * @brief Return true if two arcs lie on one circle, decided exactly
 * @details Three input points fix a circle, so an arc defined by three lies on
 * the circle of another exactly where its three points do
 * (#point_on_arc_circle). Two halves of full circles lie on one circle where
 * the ends of each diameter lie on the other. The portions of one arc carry
 * its input points unchanged, which answers them without arithmetic. Two
 * centred arcs lie on one circle where they carry one centre and one radius,
 * and an arc of input points lies on a centred one where its points do, the
 * ends of a diameter where one of them does and their middle is the centre
 */
bool
arc_same_circle(const Edge *a, const Edge *b)
{
  if (a->centred && b->centred)
    return a->cx == b->cx && a->cy == b->cy && a->radius == b->radius;
  if (a->centred || b->centred)
  {
    /* The input points of the other fix its circle: three on the centred
     * one, or the two ends of a diameter on it about its centre */
    const Edge *c = a->centred ? a : b, *o = a->centred ? b : a;
    if (! o->diameter)
      return point_on_arc_circle(c, o->sx, o->sy) &&
        point_on_arc_circle(c, o->mx, o->my) &&
        point_on_arc_circle(c, o->ex, o->ey);
    double t[2][3] = {{o->sx, o->ex, -2.0 * c->cx},
      {o->sy, o->ey, -2.0 * c->cy}};
    ExactSum sums[2];
    for (int k = 0; k < 2; k++)
      exact_sum_set(&sums[k], t[k], 3);
    PolyTerm tx = {1.0, 1, {0}}, ty = {1.0, 1, {1}};
    return point_on_arc_circle(c, o->sx, o->sy) &&
      polynomial_sign_exact(sums, &tx, 1) == 0 &&
      polynomial_sign_exact(sums, &ty, 1) == 0;
  }
  if (a->diameter == b->diameter && a->sx == b->sx && a->sy == b->sy &&
      a->ex == b->ex && a->ey == b->ey &&
      (a->diameter || (a->mx == b->mx && a->my == b->my)))
    return true;
  const Edge *t = b->diameter ? a : b, *o = b->diameter ? b : a;
  if (! t->diameter)
    return point_on_arc_circle(o, t->sx, t->sy) &&
      point_on_arc_circle(o, t->mx, t->my) &&
      point_on_arc_circle(o, t->ex, t->ey);
  return point_on_arc_circle(o, t->sx, t->sy) &&
    point_on_arc_circle(o, t->ex, t->ey) &&
    point_on_arc_circle(t, o->sx, o->sy) &&
    point_on_arc_circle(t, o->ex, o->ey);
}

/*****************************************************************************
 * Where a straight segment meets a circular arc, decided exactly
 *****************************************************************************/

/*
 * The meeting of a segment AB and the circle of an arc is a quadratic in the
 * position t along the segment, P(t) = A + t (B - A). For an arc given by its
 * start S, middle M and end E the quadratic is the incircle determinant of
 * S, M, E and P(t), zero where P(t) lies on their circle; for half of a full
 * circle given by the ends S and E of a diameter it is (P - S).(P - E), zero
 * where P sees the diameter at a right angle (Thales). Its coefficients are
 * polynomials in the coordinates of the input points. A point of the circle
 * lies on the arc exactly where it stands strictly on the arc's side of the
 * chord S -> E -- the side the middle stands on, or for a half circle the
 * right of the chord when it turns counterclockwise -- which holds for an arc
 * of any sweep, and a point of the circle on the chord line is one of its two
 * ends. Every question the meeting asks is therefore the sign of a polynomial
 * in the input coordinates, or of U + W sqrt(D) with U, W and D such
 * polynomials, and each is decided exactly.
 *
 * Each is first read in doubles carrying a bound on their error, the filter
 * #polynomial_sign_exact applies: each operation adds DBL_EPSILON times its
 * result, and a value larger than its bound has the sign of the exact one.
 * Where a filter cannot tell, the same polynomials are formed again as
 * expansions (#expansion_sum, #expansion_product), whose last component
 * carries the exact sign.
 */

/* A value in doubles and a bound on the error of it */
typedef struct
{
  double v;
  double e;
} ArcsegFilter;

/* The doubles one exact evaluation takes at most without a block of its
 * own: the expansions of a meeting of a segment and an arc of three points */
#define ARCSEG_ARENA_SIZE 4096

/* The memory one exact evaluation takes, released in one step: taken in
 * order from a buffer on the stack, and from blocks of its own once the
 * buffer is used up */
typedef struct
{
  double buf[ARCSEG_ARENA_SIZE];
  int used;
  double **blocks;
  int n;
  int cap;
  bool unsafe;      /**< A product may have left the range where the
                         arithmetic of expansions is exact */
} ArcsegArena;

/* An exact value: an expansion whose last component carries its sign, and a
 * bound on how far it may lie from the value, 0 unless a product's error
 * term underflowed */
typedef struct
{
  int n;
  double *e;
  double err;
} ArcsegExact;

static double *
arcseg_arena_alloc(ArcsegArena *arena, int ndoubles)
{
  ndoubles = Max(ndoubles, 1);
  if (arena->used + ndoubles <= ARCSEG_ARENA_SIZE)
  {
    double *block = arena->buf + arena->used;
    arena->used += ndoubles;
    return block;
  }
  if (arena->n == arena->cap)
  {
    arena->cap = arena->cap ? 2 * arena->cap : 64;
    arena->blocks = arena->blocks ?
      repalloc(arena->blocks, sizeof(double *) * arena->cap) :
      palloc(sizeof(double *) * arena->cap);
  }
  double *block = palloc(sizeof(double) * ndoubles);
  arena->blocks[arena->n++] = block;
  return block;
}

static void
arcseg_arena_free(ArcsegArena *arena)
{
  for (int i = 0; i < arena->n; i++)
    pfree(arena->blocks[i]);
  if (arena->blocks)
    pfree(arena->blocks);
  arena->blocks = NULL;
  arena->n = arena->cap = arena->used = 0;
  arena->unsafe = false;
  return;
}

static inline ArcsegFilter
arcseg_f_diff(double a, double b)
{
  ArcsegFilter r;
  r.v = a - b;
  r.e = DBL_EPSILON * fabs(r.v);
  return r;
}

static inline ArcsegFilter
arcseg_f_add(ArcsegFilter a, ArcsegFilter b)
{
  ArcsegFilter r;
  r.v = a.v + b.v;
  r.e = a.e + b.e + DBL_EPSILON * fabs(r.v);
  return r;
}

static inline ArcsegFilter
arcseg_f_sub(ArcsegFilter a, ArcsegFilter b)
{
  ArcsegFilter r;
  r.v = a.v - b.v;
  r.e = a.e + b.e + DBL_EPSILON * fabs(r.v);
  return r;
}

static inline ArcsegFilter
arcseg_f_mul(ArcsegFilter a, ArcsegFilter b)
{
  ArcsegFilter r;
  r.v = a.v * b.v;
  r.e = fabs(a.v) * b.e + fabs(b.v) * a.e + a.e * b.e +
    DBL_EPSILON * fabs(r.v);
  return r;
}

/* Multiplied by a power of two, which scales the error without a rounding */
static inline ArcsegFilter
arcseg_f_scale(ArcsegFilter a, double k)
{
  ArcsegFilter r;
  r.v = a.v * k;
  r.e = a.e * fabs(k);
  return r;
}

/* The sign a filtered value carries, or 2 where its bound cannot tell. The
 * bound is computed in doubles, hence the margin, as #polynomial_sign_exact
 * reads its own */
static inline int
arcseg_f_sign(ArcsegFilter a)
{
  double bound = a.e * (1.0 + 1e-6) + DBL_MIN;
  if (! isfinite(a.v) || ! isfinite(bound) || fabs(a.v) <= bound)
    return 2;
  return (a.v > 0.0) ? 1 : -1;
}

static ArcsegExact
arcseg_x_diff(ArcsegArena *arena, double a, double b)
{
  ArcsegExact r;
  r.e = arcseg_arena_alloc(arena, 2);
  double x, y;
  two_sum(a, - b, &x, &y);
  r.err = 0.0;
  r.n = 0;
  if (y != 0.0)
    r.e[r.n++] = y;
  r.e[r.n++] = x;
  return r;
}

static ArcsegExact
arcseg_x_add(ArcsegArena *arena, ArcsegExact a, ArcsegExact b)
{
  int cap = a.n + b.n;
  double *h = arcseg_arena_alloc(arena, cap);
  ArcsegExact r;
  r.e = arcseg_arena_alloc(arena, cap);
  int hlen = expansion_sum(a.n, a.e, b.n, b.e, h, r.e);
  r.n = compress_expansion(hlen, h, r.e);
  r.err = (a.err + b.err) * (1.0 + 0x1p-50);
  return r;
}

static ArcsegExact
arcseg_x_scale(ArcsegArena *arena, ArcsegExact a, double k)
{
  /* A power of two scales every component exactly */
  ArcsegExact r;
  r.e = arcseg_arena_alloc(arena, a.n);
  for (int i = 0; i < a.n; i++)
    r.e[i] = a.e[i] * k;
  r.n = a.n;
  r.err = a.err * fabs(k);
  return r;
}

static ArcsegExact
arcseg_x_sub(ArcsegArena *arena, ArcsegExact a, ArcsegExact b)
{
  return arcseg_x_add(arena, a, arcseg_x_scale(arena, b, -1.0));
}

/* The sum of the magnitudes of the components of an expansion, read as
 * #arcseg_x_sign reads its last one */
static double
arcseg_x_mag(ArcsegExact a)
{
  double m = 0.0;
  for (int i = 0; i < a.n; i++)
    m += fabs(a.e[i]);
  return m * (1.0 + 0x1p-50);
}

static ArcsegExact
arcseg_x_mul(ArcsegArena *arena, ArcsegExact a, ArcsegExact b)
{
  /* The product of two components is exact while its error term is a
   * double: the units of their last places multiply to at least the
   * smallest subnormal, 2^-1074, which holds where the product is at least
   * 2^-968. Below it each of the products of two components may lose up to
   * 2^-1074, which the bound of the result carries; above the largest double
   * nothing is kept. The components grow in magnitude */
  double under = 0.0;
  if (a.n > 0 && b.n > 0)
  {
    double lo = fabs(a.e[0]) * fabs(b.e[0]);
    double hi = fabs(a.e[a.n - 1]) * fabs(b.e[b.n - 1]);
    if (hi > 0x1p+1000)
      arena->unsafe = true;
    /* A product of two components that are not zero may itself round to
     * zero, which is underflow and not a zero component */
    if (a.e[0] != 0.0 && b.e[0] != 0.0 && lo < 0x1p-968)
      under = (double) a.n * (double) b.n * 0x1p-1072;
  }
  int cap = 2 * a.n * b.n;
  double *h = arcseg_arena_alloc(arena, cap);
  double *part = arcseg_arena_alloc(arena, 2 * a.n);
  ArcsegExact r;
  r.e = arcseg_arena_alloc(arena, cap);
  int hlen = expansion_product(a.n, a.e, b.n, b.e, h, r.e, part);
  r.n = compress_expansion(hlen, h, r.e);
  r.err = 0.0;
  /* The bound is computed in doubles, which below the normal range round
   * to the nearest multiple of 2^-1074 and so may round it to zero: each of
   * its three products and two sums loses at most half of that */
  if (a.err > 0.0 || b.err > 0.0)
    r.err = (arcseg_x_mag(a) * b.err + arcseg_x_mag(b) * a.err +
      a.err * b.err) * (1.0 + 0x1p-50) + 0x1p-1071;
  r.err += under;
  return r;
}

static inline int
arcseg_x_sign(ArcsegExact a)
{
  double last = a.n > 0 ? a.e[a.n - 1] : 0.0;
  return (last > 0.0) ? 1 : ((last < 0.0) ? -1 : 0);
}

/* The sign of #arcseg_x_sign, recording in the arena where the bound of the
 * expansion does not let the value's sign be told: the value is at least
 * its last component less the others, which together are smaller than it */
static int
arcseg_x_sign_checked(ArcsegArena *arena, ArcsegExact a)
{
  if (a.err > 0.0)
  {
    double low = (a.n > 0) ? fabs(a.e[a.n - 1]) : 0.0;
    for (int i = 0; i + 1 < a.n; i++)
      low -= fabs(a.e[i]);
    if (low * (1.0 - 0x1p-50) <= a.err)
      arena->unsafe = true;
  }
  return arcseg_x_sign(a);
}

/*
 * The polynomials of a segment meeting an arc, in one arithmetic: the
 * quadratic qa t^2 + qb t + qc, the side h1 t + h0 of the chord S -> E that
 * P(t) stands on, and for an arc of three points the side its middle stands
 * on. The vectors are taken from A, so every factor is a difference of input
 * coordinates. A centred arc is its whole circle, so every root of its
 * quadratic stands on it: h1 is zero and h0 has the arc's side.
 */
#define ARCSEG_POLYNOMIALS(T, DIFF, ADD, SUB, MUL, SCALE, ZERO)               \
{                                                                            \
  T rx = DIFF(bx, ax), ry = DIFF(by, ay);                                    \
  T rr = ADD(MUL(rx, rx), MUL(ry, ry));                                      \
  if (arc->centred)                                                          \
  {                                                                          \
    /* |A + t R - C|^2 - r^2 = t^2 rr - 2 t R.(C - A) + |C - A|^2 - r^2,     \
     * every point of the whole circle on the arc: no chord, h1 = 0 and the  \
     * side h0 and the arc's side both positive */                           \
    T kx = DIFF(arc->cx, ax), ky = DIFF(arc->cy, ay);                        \
    T kr = DIFF(arc->radius, 0.0);                                           \
    *qa = rr;                                                                \
    *qb = SCALE(ADD(MUL(rx, kx), MUL(ry, ky)), -2.0);                        \
    *qc = SUB(ADD(MUL(kx, kx), MUL(ky, ky)), MUL(kr, kr));                   \
    *h1 = ZERO;                                                              \
    *h0 = rr;                                                                \
    *side = rr;                                                              \
  }                                                                          \
  else                                                                       \
  {                                                                          \
    T sx = DIFF(arc->sx, ax), sy = DIFF(arc->sy, ay);                        \
    T ex = DIFF(arc->ex, ax), ey = DIFF(arc->ey, ay);                        \
    T cx = SUB(ex, sx), cy = SUB(ey, sy);                                    \
    /* The chord crossed with P(t) - S = t R - S */                          \
    *h1 = SUB(MUL(cx, ry), MUL(cy, rx));                                     \
    *h0 = SCALE(SUB(MUL(cx, sy), MUL(cy, sx)), -1.0);                        \
    if (arc->diameter)                                                       \
    {                                                                        \
      /* (P - S).(P - E) = t^2 rr - t R.(S + E) + S.E */                     \
      T px = ADD(sx, ex), py = ADD(sy, ey);                                  \
      *qa = rr;                                                              \
      *qb = SCALE(ADD(MUL(rx, px), MUL(ry, py)), -1.0);                      \
      *qc = ADD(MUL(sx, ex), MUL(sy, ey));                                   \
      *side = rr;                                                            \
    }                                                                        \
    else                                                                     \
    {                                                                        \
      T mx = DIFF(arc->mx, ax), my = DIFF(arc->my, ay);                      \
      T pts[3][2] = {{sx, sy}, {mx, my}, {ex, ey}};                          \
      *qa = ZERO; *qb = ZERO; *qc = ZERO;                                    \
      for (int k = 0; k < 3; k++)                                            \
      {                                                                      \
        const T *x = pts[k], *y = pts[(k + 1) % 3], *z = pts[(k + 2) % 3];   \
        T lift = ADD(MUL(x[0], x[0]), MUL(x[1], x[1]));                      \
        T xr = ADD(MUL(x[0], rx), MUL(x[1], ry));                            \
        T yz = SUB(MUL(y[0], z[1]), MUL(y[1], z[0]));                        \
        T rd = SUB(MUL(rx, SUB(z[1], y[1])), MUL(ry, SUB(z[0], y[0])));      \
        /* t^2: rr (Y x Z) + 2 (X.R) (R x (Z - Y)) */                        \
        T ta = ADD(MUL(rr, yz), SCALE(MUL(xr, rd), 2.0));                    \
        /* t: - |X|^2 (R x (Z - Y)) - 2 (X.R) (Y x Z) */                     \
        T tb = SCALE(ADD(MUL(lift, rd), SCALE(MUL(xr, yz), 2.0)), -1.0);     \
        /* 1: |X|^2 (Y x Z) */                                               \
        T tc = MUL(lift, yz);                                                \
        *qa = ADD(*qa, ta);                                                  \
        *qb = ADD(*qb, tb);                                                  \
        *qc = ADD(*qc, tc);                                                  \
      }                                                                      \
      *side = SUB(MUL(cx, SUB(my, sy)), MUL(cy, SUB(mx, sx)));               \
    }                                                                        \
  }                                                                          \
}

static void
arcseg_polynomials_filter(double ax, double ay, double bx, double by,
  const Edge *arc, ArcsegFilter *qa, ArcsegFilter *qb, ArcsegFilter *qc,
  ArcsegFilter *h1, ArcsegFilter *h0, ArcsegFilter *side)
{
  const ArcsegFilter zero = {0.0, 0.0};
  ARCSEG_POLYNOMIALS(ArcsegFilter, arcseg_f_diff, arcseg_f_add, arcseg_f_sub,
    arcseg_f_mul, arcseg_f_scale, zero)
}

static void
arcseg_polynomials_exact(ArcsegArena *arena, double ax, double ay, double bx,
  double by, const Edge *arc, ArcsegExact *qa, ArcsegExact *qb,
  ArcsegExact *qc, ArcsegExact *h1, ArcsegExact *h0, ArcsegExact *side)
{
#define XDIFF(a, b) arcseg_x_diff(arena, a, b)
#define XADD(a, b) arcseg_x_add(arena, a, b)
#define XSUB(a, b) arcseg_x_sub(arena, a, b)
#define XMUL(a, b) arcseg_x_mul(arena, a, b)
#define XSCALE(a, k) arcseg_x_scale(arena, a, k)
  ArcsegExact zero;
  zero.e = arcseg_arena_alloc(arena, 1);
  zero.e[0] = 0.0;
  zero.n = 1;
  zero.err = 0.0;
  ARCSEG_POLYNOMIALS(ArcsegExact, XDIFF, XADD, XSUB, XMUL, XSCALE, zero)
#undef XDIFF
#undef XADD
#undef XSUB
#undef XMUL
#undef XSCALE
}

/*
 * The signs every decision of one meeting reads, in one arithmetic, with
 * D = qb^2 - 4 qa qc, U0 = -qb, U1 = -qb - 2 qa and UH = 2 qa h0 - h1 qb: a
 * root t = (-qb + sigma sqrt(D)) / (2 qa) stands against the position k of
 * the segment as U_k + sigma sqrt(D) against 0 in the sign of qa, and on the
 * side of the chord h1 t + h0 as UH + sigma h1 sqrt(D) in the same sign. The
 * filter writes 2 for a sign it cannot tell.
 */
typedef struct
{
  int qa;     /**< Sign of qa */
  int disc;   /**< Sign of D */
  int side;   /**< Side of the chord the arc stands on */
  int u0;     /**< Sign of U0 */
  int u1;     /**< Sign of U1 */
  int uh;     /**< Sign of UH */
  int h1;     /**< Sign of h1 */
  int q0;     /**< Sign of U0^2 - D */
  int q1;     /**< Sign of U1^2 - D */
  int qh;     /**< Sign of UH^2 - h1^2 D */
  int order;  /**< Sign of (E - S).R, the order of the arc's ends along AB */
} ArcsegSigns;

#define ARCSEG_SIGNS(T, SIGN, ADD, SUB, MUL, SCALE, DIFF)                     \
{                                                                            \
  T disc = SUB(MUL(qb, qb), SCALE(MUL(qa, qc), 4.0));                        \
  T u0 = SCALE(qb, -1.0);                                                    \
  T u1 = SUB(u0, SCALE(qa, 2.0));                                            \
  T uh = SUB(SCALE(MUL(qa, h0), 2.0), MUL(h1, qb));                          \
  T order = ADD(MUL(SUB(DIFF(arc->ex, ax), DIFF(arc->sx, ax)), DIFF(bx, ax)),\
    MUL(SUB(DIFF(arc->ey, ay), DIFF(arc->sy, ay)), DIFF(by, ay)));           \
  sg->qa = SIGN(qa);                                                         \
  sg->disc = SIGN(disc);                                                     \
  sg->side = arc->diameter ? (arc->ccw ? -1 : 1) : SIGN(side);               \
  sg->u0 = SIGN(u0);                                                         \
  sg->u1 = SIGN(u1);                                                         \
  sg->uh = SIGN(uh);                                                         \
  sg->h1 = SIGN(h1);                                                         \
  sg->q0 = SIGN(SUB(MUL(u0, u0), disc));                                     \
  sg->q1 = SIGN(SUB(MUL(u1, u1), disc));                                     \
  sg->qh = SIGN(SUB(MUL(uh, uh), MUL(MUL(h1, h1), disc)));                   \
  sg->order = SIGN(order);                                                   \
}

/* The signs in doubles, with the roots approximated for a caller that
 * constructs a crossing; false where a filter cannot tell one of them */
static bool
arcseg_signs_filter(double ax, double ay, double bx, double by,
  const Edge *arc, ArcsegSigns *sg, double roots[2])
{
  ArcsegFilter qa, qb, qc, h1, h0, side;
  arcseg_polynomials_filter(ax, ay, bx, by, arc, &qa, &qb, &qc, &h1, &h0,
    &side);
  ARCSEG_SIGNS(ArcsegFilter, arcseg_f_sign, arcseg_f_add, arcseg_f_sub,
    arcseg_f_mul, arcseg_f_scale, arcseg_f_diff)
  /* The two roots in the stable form of the quadratic formula, the one of
   * sigma = -1 first */
  double d = qb.v * qb.v - 4.0 * qa.v * qc.v;
  double sq = sqrt(Max(d, 0.0));
  double q = -0.5 * (qb.v + ((qb.v >= 0.0) ? sq : -sq));
  double ra = (q != 0.0) ? q / qa.v : 0.0;
  double rb = (q != 0.0) ? qc.v / q : -qb.v / (2.0 * qa.v);
  roots[0] = (qb.v >= 0.0) ? ra : rb;
  roots[1] = (qb.v >= 0.0) ? rb : ra;
  return sg->qa != 2 && sg->disc != 2 && sg->side != 2 && sg->u0 != 2 &&
    sg->u1 != 2 && sg->uh != 2 && sg->h1 != 2 && sg->q0 != 2 &&
    sg->q1 != 2 && sg->qh != 2 && sg->order != 2;
}

/* The same signs computed exactly */
static void
arcseg_signs_exact(double ax, double ay, double bx, double by,
  const Edge *arc, ArcsegSigns *sg)
{
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  ArcsegExact qa, qb, qc, h1, h0, side;
  arcseg_polynomials_exact(arena, ax, ay, bx, by, arc, &qa, &qb, &qc, &h1,
    &h0, &side);
#define XDIFF(a, b) arcseg_x_diff(arena, a, b)
#define XADD(a, b) arcseg_x_add(arena, a, b)
#define XSUB(a, b) arcseg_x_sub(arena, a, b)
#define XMUL(a, b) arcseg_x_mul(arena, a, b)
#define XSCALE(a, k) arcseg_x_scale(arena, a, k)
  ARCSEG_SIGNS(ArcsegExact, arcseg_x_sign, XADD, XSUB, XMUL, XSCALE, XDIFF)
#undef XDIFF
#undef XADD
#undef XSUB
#undef XMUL
#undef XSCALE
  arcseg_arena_free(arena);
  return;
}

/* The sign of U + W sqrt(D) for D > 0, from the signs of U and W and of
 * U^2 - W^2 D: the larger of the two terms carries it */
static inline int
arcseg_root_sign(int su, int sw, int sq)
{
  if (sw == 0)
    return su;
  if (su == 0 || su == sw)
    return sw;
  return su * sq;
}

/*
 * The discriminant of the quadratic where the line through A and B meets the
 * circle of an arc, in one arithmetic. The vectors are taken from a point of
 * the arc's own, its start S or the centre of a whole circle, so the
 * coefficients are of the size of the arc and of its distance to the line
 * however far the line lies, which a filter then tells. About that origin
 * the circle is D |X|^2 - X.K - L = 0: through S, M and E with m = M - S and
 * n = E - S, D = m x n and K = (|m|^2 n_y - |n|^2 m_y, |n|^2 m_x - |m|^2 n_x),
 * L = 0; on the diameter from S to E, D = 1, K = n, L = 0; a whole circle of
 * radius r, D = 1, K = 0, L = r^2. On X = P + t (B - A), P = A less the
 * origin, it is a t^2 + b t + c = 0 with a = D |B - A|^2,
 * b = 2 D P.(B - A) - K.(B - A), c = D |P|^2 - P.K - L
 */
#define ARCSEG_LINE_DISC(T, DIFF, ADD, SUB, MUL, SCALE, ONE)                 \
{                                                                            \
  double ox = arc->centred ? arc->cx : arc->sx;                              \
  double oy = arc->centred ? arc->cy : arc->sy;                              \
  T ex = DIFF(bx, ax), ey = DIFF(by, ay);                                    \
  T px = DIFF(ax, ox), py = DIFF(ay, oy);                                    \
  T d = ONE, kx = ONE, ky = ONE, l = ONE;                                    \
  bool round = false;                                                        \
  if (arc->centred)                                                          \
  {                                                                          \
    T r = DIFF(arc->radius, 0.0);                                            \
    l = MUL(r, r);                                                           \
    round = true;                                                            \
  }                                                                          \
  else if (arc->diameter)                                                    \
  {                                                                          \
    kx = DIFF(arc->ex, ox);                                                  \
    ky = DIFF(arc->ey, oy);                                                  \
  }                                                                          \
  else                                                                       \
  {                                                                          \
    T mx = DIFF(arc->mx, ox), my = DIFF(arc->my, oy);                        \
    T nx = DIFF(arc->ex, ox), ny = DIFF(arc->ey, oy);                        \
    T mm = ADD(MUL(mx, mx), MUL(my, my));                                    \
    T nn = ADD(MUL(nx, nx), MUL(ny, ny));                                    \
    d = SUB(MUL(mx, ny), MUL(my, nx));                                       \
    kx = SUB(MUL(mm, ny), MUL(nn, my));                                      \
    ky = SUB(MUL(nn, mx), MUL(mm, nx));                                      \
  }                                                                          \
  T ee = ADD(MUL(ex, ex), MUL(ey, ey));                                      \
  T pe = ADD(MUL(px, ex), MUL(py, ey));                                      \
  T pp = ADD(MUL(px, px), MUL(py, py));                                      \
  T qa = round ? ee : MUL(d, ee);                                            \
  T qb = round ? SCALE(pe, 2.0) : SUB(SCALE(MUL(d, pe), 2.0),                \
    ADD(MUL(kx, ex), MUL(ky, ey)));                                          \
  T qc = round ? SUB(pp, l) : SUB(MUL(d, pp), ADD(MUL(px, kx),               \
    MUL(py, ky)));                                                           \
  disc = SUB(MUL(qb, qb), SCALE(MUL(qa, qc), 4.0));                          \
}

/**
 * @brief Return the sign of the discriminant of the quadratic where the line
 * through A and B meets the circle of an arc, decided exactly: negative where
 * the line misses the circle, zero where it touches it, positive where it
 * crosses it
 * @details The discriminant (#ARCSEG_LINE_DISC) is read in doubles with a
 * bound on its error and formed again as expansions where the bound cannot
 * tell, as #arcseg_signs_exact forms its own
 */
static int
arcseg_line_disc_sign(double ax, double ay, double bx, double by,
  const Edge *arc)
{
  const ArcsegFilter one = {1.0, 0.0};
  int result;
  {
    ArcsegFilter disc;
    ARCSEG_LINE_DISC(ArcsegFilter, arcseg_f_diff, arcseg_f_add, arcseg_f_sub,
      arcseg_f_mul, arcseg_f_scale, one)
    result = arcseg_f_sign(disc);
  }
  if (result != 2)
    return result;
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  ArcsegExact xdisc, xone;
  xone.e = arcseg_arena_alloc(arena, 1);
  xone.e[0] = 1.0;
  xone.n = 1;
  xone.err = 0.0;
#define XDIFF(a, b) arcseg_x_diff(arena, a, b)
#define XADD(a, b) arcseg_x_add(arena, a, b)
#define XSUB(a, b) arcseg_x_sub(arena, a, b)
#define XMUL(a, b) arcseg_x_mul(arena, a, b)
#define XSCALE(a, k) arcseg_x_scale(arena, a, k)
  {
    ArcsegExact disc;
    ARCSEG_LINE_DISC(ArcsegExact, XDIFF, XADD, XSUB, XMUL, XSCALE, xone)
    xdisc = disc;
  }
#undef XDIFF
#undef XADD
#undef XSUB
#undef XMUL
#undef XSCALE
  result = arcseg_x_sign(xdisc);
  arcseg_arena_free(arena);
  return result;
}

/*
 * The signs of a meeting where an end of the segment is an end of the arc,
 * read in one arithmetic: the quadratic then has a root at that end exactly,
 * and the other is a quotient of its coefficients. With A shared, g(t) =
 * t (qa t + qb); with B shared, g(t) = (t - 1) (qa t + qa + qb). The shared
 * end lies on the chord, so the side of it is h1 times t, or h1 times t - 1
 */
static void arcarc_centre_exact(ArcsegArena *arena, const Edge *e,
  ArcsegExact *mx, ArcsegExact *my, ArcsegExact *nx, ArcsegExact *ny,
  ArcsegExact *d);

#define ARCSEG_SHARED_SIGNS(T, SIGN, ADD, SUB, MUL, SCALE)                    \
{                                                                            \
  *sqa = SIGN(qa);                                                           \
  *sqb = SIGN(qb);                                                           \
  *sab = SIGN(ADD(qa, qb));                                                  \
  *s2ab = SIGN(ADD(SCALE(qa, 2.0), qb));                                     \
  *sh1 = SIGN(h1);                                                           \
  *sside = arc->diameter ? (arc->ccw ? -1 : 1) : SIGN(side);                 \
}

static void
arcseg_shared_signs(double ax, double ay, double bx, double by,
  const Edge *arc, bool exact, int *sqa, int *sqb, int *sab, int *s2ab,
  int *sh1, int *sside)
{
  ArcsegFilter qa, qb, qc, h1, h0, side;
  arcseg_polynomials_filter(ax, ay, bx, by, arc, &qa, &qb, &qc, &h1, &h0,
    &side);
  ARCSEG_SHARED_SIGNS(ArcsegFilter, arcseg_f_sign, arcseg_f_add,
    arcseg_f_sub, arcseg_f_mul, arcseg_f_scale)
  if (! exact || (*sqa != 2 && *sqb != 2 && *sab != 2 && *s2ab != 2 &&
      *sh1 != 2 && *sside != 2))
    return;
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  /* The quadratic is k (|P(t) - C|^2 - r^2) with k of the sign of the
   * side of the arc's middle, 1 for a half circle on a diameter, so with
   * R = B - A: qa = k |R|^2, qb = 2 k (A - C).R, qa + qb = k R.(A + B - 2C)
   * and 2 qa + qb = 2 k R.(B - C). With the centre C = M + N / D of
   * #arcarc_centre_exact, each dot product is that of (V - M) D - N with R,
   * over D, for V = A, B or (A + B) / 2: degree four in the coordinates,
   * where the coefficients of the quadratic are of degree six */
  ArcsegExact rx = arcseg_x_diff(arena, bx, ax);
  ArcsegExact ry = arcseg_x_diff(arena, by, ay);
  ArcsegExact cx = arcseg_x_diff(arena, arc->ex, arc->sx);
  ArcsegExact cy = arcseg_x_diff(arena, arc->ey, arc->sy);
  *sh1 = arcseg_x_sign(arcseg_x_sub(arena, arcseg_x_mul(arena, cx, ry),
    arcseg_x_mul(arena, cy, rx)));
  int k;
  if (arc->diameter)
  {
    *sside = arc->ccw ? -1 : 1;
    k = 1;
  }
  else
  {
    *sside = arcseg_x_sign(arcseg_x_sub(arena, arcseg_x_mul(arena, cx,
      arcseg_x_diff(arena, arc->my, arc->sy)), arcseg_x_mul(arena, cy,
      arcseg_x_diff(arena, arc->mx, arc->sx))));
    k = *sside;
  }
  ArcsegExact mx, my, nx, ny, d;
  arcarc_centre_exact(arena, arc, &mx, &my, &nx, &ny, &d);
  int sd = arcseg_x_sign(d);
  /* ((V - M) D - N).R for V = A and V = B */
  ArcsegExact va[2];
  const double vx[2] = {ax, bx}, vy[2] = {ay, by};
  for (int i = 0; i < 2; i++)
  {
    ArcsegExact wx = arcseg_x_sub(arena, arcseg_x_mul(arena,
      arcseg_x_sub(arena, arcseg_x_diff(arena, vx[i], 0.0), mx), d), nx);
    ArcsegExact wy = arcseg_x_sub(arena, arcseg_x_mul(arena,
      arcseg_x_sub(arena, arcseg_x_diff(arena, vy[i], 0.0), my), d), ny);
    va[i] = arcseg_x_add(arena, arcseg_x_mul(arena, wx, rx),
      arcseg_x_mul(arena, wy, ry));
  }
  *sqa = k;
  *sqb = k * sd * arcseg_x_sign(va[0]);
  *sab = k * sd * arcseg_x_sign(arcseg_x_add(arena, va[0], va[1]));
  *s2ab = k * sd * arcseg_x_sign(va[1]);
  arcseg_arena_free(arena);
  return;
}

/*
 * A segment sharing an end with an arc: the shared end is one meeting, the
 * arc's end, and the other root of the quadratic is the only other one. Its
 * position against the ends of the segment and its side of the chord are
 * signs of qa, qb, qa + qb and h1; the sign of the derivative of the
 * quadratic there, which orients the crossing, is that of -qb with A shared
 * and of -(2 qa + qb) with B shared. Return -1 where neither end is shared
 */
static int
arcseg_meet_shared(double ax, double ay, double bx, double by,
  const Edge *arc, ArcsegMeet out[2])
{
  bool a_start = ax == arc->x1 && ay == arc->y1;
  bool a_end = ax == arc->x2 && ay == arc->y2;
  bool b_start = bx == arc->x1 && by == arc->y1;
  bool b_end = bx == arc->x2 && by == arc->y2;
  bool ashared = a_start || a_end, bshared = b_start || b_end;
  if (! ashared && ! bshared)
    return -1;
  /* The two signs the shortcut below reads, the side h1 of the chord the
   * segment runs to and the side of the arc's middle, are those of cross
   * products of input points: asked first, at the cost of an orientation */
  if (! (ashared && bshared))
  {
    int ch1 = cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey, ax, ay,
      bx, by);
    int cside = arc->diameter ? (arc->ccw ? -1 : 1) :
      cross_product_sign(arc->sx, arc->sy, arc->ex, arc->ey, arc->sx,
        arc->sy, arc->mx, arc->my);
    if (cside != 0 && ch1 != 0 && (ashared ? ch1 : - ch1) != cside)
    {
      out[0].kind = (a_start || b_start) ? ARCSEG_AT_ARC_START :
        ARCSEG_AT_ARC_END;
      out[0].t = ashared ? 0.0 : 1.0;
      out[0].sigma = 0;
      out[0].smaller = false;
      return 1;
    }
  }
  int sqa, sqb, sab, s2ab, sh1, sside;
  arcseg_shared_signs(ax, ay, bx, by, arc, false, &sqa, &sqb, &sab, &s2ab,
    &sh1, &sside);
  if (sside == 0)
    return -2;
  /* A segment leaving the arc's end where the arc runs on, as an offset
   * segment leaves its join arc: a second root strictly within the segment
   * stands on the side h1 of the chord with A shared, -h1 with B shared,
   * and one at the shared end is double. Where that side is not the arc's,
   * the shared end is the only meeting, whatever the sign the filter
   * cannot tell, which is that of a value within rounding of zero */
  if (! (ashared && bshared) && sh1 != 2 && sh1 != 0 && sside != 2 &&
      (ashared ? sh1 : - sh1) != sside)
  {
    out[0].kind = (a_start || b_start) ? ARCSEG_AT_ARC_START :
      ARCSEG_AT_ARC_END;
    out[0].t = ashared ? 0.0 : 1.0;
    out[0].sigma = 0;
    out[0].smaller = false;
    return 1;
  }
  if (sqa == 2 || sqb == 2 || sab == 2 || s2ab == 2 || sh1 == 2 ||
      sside == 2)
    arcseg_shared_signs(ax, ay, bx, by, arc, true, &sqa, &sqb, &sab, &s2ab,
      &sh1, &sside);
  if (sqa == 0 || sside == 0)
    return -2;
  /* Each meeting is classified as #arcsegm_meet_exact classifies a root; the
   * smaller root has the derivative of sign -qa */
  if (ashared && bshared)
  {
    /* The segment is the chord: g(t) = qa t (t - 1), whose derivative is
     * -qa at A and qa at B, and nothing else meets it */
    out[0].kind = a_start ? ARCSEG_AT_ARC_START : ARCSEG_AT_ARC_END;
    out[0].t = 0.0; out[0].sigma = 0; out[0].smaller = false;
    out[1].kind = b_start ? ARCSEG_AT_ARC_START : ARCSEG_AT_ARC_END;
    out[1].t = 1.0; out[1].sigma = 0; out[1].smaller = false;
    return 2;
  }
  /* The other root: t2 = -qb / qa with A shared, -(qa + qb) / qa with B
   * shared. The derivative of the quadratic is qb at A and 2 qa + qb at B,
   * and its negation at t2: zero is a double root at the shared end */
  int s0, s1, sh, sigma;
  if (ashared)
  {
    s0 = - sqb * sqa;                 /* t2 */
    s1 = - sab * sqa;                 /* t2 - 1 = -(qa + qb) / qa */
    sh = sh1 * s0;                    /* h1 t2 */
    sigma = - sqb;
  }
  else
  {
    s0 = - sab * sqa;                 /* t2 = -(qa + qb) / qa */
    s1 = - s2ab * sqa;                /* t2 - 1 = -(2 qa + qb) / qa */
    sh = sh1 * s1;                    /* h1 (t2 - 1) */
    sigma = - s2ab;
  }
  ArcsegMeet shared;
  shared.kind = (a_start || b_start) ? ARCSEG_AT_ARC_START :
    ARCSEG_AT_ARC_END;
  shared.t = ashared ? 0.0 : 1.0;
  shared.sigma = 0;
  shared.smaller = false;
  ArcsegMeet m;
  m.kind = ARCSEG_INTERIOR;
  m.sigma = sigma;
  m.smaller = sigma * sqa < 0;
  /* A double root, or the other root outside the segment */
  bool other = sigma != 0 && s0 >= 0 && s1 <= 0;
  if (other)
  {
    if (sh == 0)
      /* On the circle and on the chord line: the other end of the arc */
      m.kind = (shared.kind == ARCSEG_AT_ARC_START) ? ARCSEG_AT_ARC_END :
        ARCSEG_AT_ARC_START;
    else if (sh != sside)
      /* On the circle beyond the arc */
      other = false;
    else if (s0 == 0)
      m.kind = ARCSEG_AT_SEGMENT_START;
    else if (s1 == 0)
      m.kind = ARCSEG_AT_SEGMENT_END;
  }
  if (other)
  {
    ArcsegFilter fqa, fqb, fqc, fh1, fh0, fside;
    arcseg_polynomials_filter(ax, ay, bx, by, arc, &fqa, &fqb, &fqc, &fh1,
      &fh0, &fside);
    double t = ashared ? - fqb.v / fqa.v : - (fqa.v + fqb.v) / fqa.v;
    /* Its position is rounded once, and a rounding that leaves the segment
     * stands at its end */
    m.t = (m.kind == ARCSEG_AT_SEGMENT_START) ? 0.0 :
      (m.kind == ARCSEG_AT_SEGMENT_END) ? 1.0 : Min(Max(t, 0.0), 1.0);
  }
  /* A meeting at an end of either carries no root sign */
  if (other && m.kind != ARCSEG_INTERIOR)
  {
    m.sigma = 0;
    m.smaller = false;
  }
  int n = 0;
  if (ashared)
    out[n++] = shared;
  if (other)
    out[n++] = m;
  if (bshared)
    out[n++] = shared;
  return n;
}

/**
 * @brief Return where a straight segment meets a circular arc, decided
 * exactly
 * @details Every meeting is classified exactly (see the comment above
 * #ArcsegFilter): at the start or the end of the segment, at an end of the
 * arc, or interior to both, where only its position along the segment is
 * computed, rounded once. The arc is one #geom_extract_edges emits, whose
 * ends are its first and last defining points, or a centred one, the whole
 * circle (#edges_add_circle); any other is not answered
 * @param[in] ax,ay,bx,by Start and end of the segment, distinct
 * @param[in] arc Arc edge
 * @param[out] out The meetings, in increasing position along the segment
 * @return Number of meetings, 0 to 2, or -1 where the arc is not answered
 */
int
arcsegm_meet_exact(double ax, double ay, double bx, double by,
  const Edge *arc, ArcsegMeet out[2])
{
  assert(arc); assert(out);
  if ((ax == bx && ay == by) || (! arc->centred && (arc->x1 != arc->sx ||
      arc->y1 != arc->sy || arc->x2 != arc->ex || arc->y2 != arc->ey)))
    return -1;
  /* A segment sharing an end with the arc: that end is one root exactly. A
   * centred arc has no end on its circle */
  int nshared = arc->centred ? -1 :
    arcseg_meet_shared(ax, ay, bx, by, arc, out);
  if (nshared == -2)
    return -1;
  if (nshared >= 0)
    return nshared;
  ArcsegSigns sg;
  double roots[2];
  if (! arcseg_signs_filter(ax, ay, bx, by, arc, &sg, roots))
    arcseg_signs_exact(ax, ay, bx, by, arc, &sg);
  /* No circle, or an arc of three points on one line */
  if (sg.qa == 0 || sg.side == 0)
    return -1;
  if (sg.disc < 0)
    return 0;
  int nroots = (sg.disc == 0) ? 1 : 2, n = 0;
  for (int r = 0; r < nroots; r++)
  {
    /* sigma = -1 first, which is the smaller root where qa is positive */
    int sigma = (nroots == 1) ? 0 : ((r == 0) ? -1 : 1);
    int s0, s1, sh;
    if (sigma == 0)
    {
      s0 = sg.u0; s1 = sg.u1; sh = sg.uh;
    }
    else
    {
      s0 = arcseg_root_sign(sg.u0, sigma, sg.q0);
      s1 = arcseg_root_sign(sg.u1, sigma, sg.q1);
      sh = arcseg_root_sign(sg.uh, sigma * sg.h1, sg.qh);
    }
    s0 *= sg.qa; s1 *= sg.qa; sh *= sg.qa;
    /* Before the start or past the end of the segment */
    if (s0 < 0 || s1 > 0)
      continue;
    ArcsegMeet m;
    m.t = (nroots == 1) ? roots[0] : roots[r];
    m.sigma = sigma;
    m.smaller = (nroots == 1) || ((r == 0) == (sg.qa > 0));
    if (sh == 0)
    {
      /* On the circle and on the chord line: an end of the arc. Both ends
       * meet the segment only where it runs along the chord, and then in the
       * order (E - S).R gives them */
      bool start_on = point_on_segment_exact(arc->x1, arc->y1, ax, ay, bx,
        by);
      bool end_on = point_on_segment_exact(arc->x2, arc->y2, ax, ay, bx, by);
      if (start_on && end_on)
      {
        bool first = (r == 0) == (sg.qa > 0);
        m.kind = (first == (sg.order > 0)) ? ARCSEG_AT_ARC_START :
          ARCSEG_AT_ARC_END;
      }
      else if (start_on)
        m.kind = ARCSEG_AT_ARC_START;
      else if (end_on)
        m.kind = ARCSEG_AT_ARC_END;
      else
        return -1;
    }
    else if (sh != sg.side)
      /* On the circle beyond the arc */
      continue;
    else if (s0 == 0)
      m.kind = ARCSEG_AT_SEGMENT_START;
    else if (s1 == 0)
      m.kind = ARCSEG_AT_SEGMENT_END;
    else
    {
      m.kind = ARCSEG_INTERIOR;
      /* Interior to the segment exactly; its position is rounded once, and
       * a rounding that leaves the segment stands at its end */
      m.t = Min(Max(m.t, 0.0), 1.0);
    }
    /* A meeting at an end of either carries no root sign */
    if (m.kind != ARCSEG_INTERIOR)
    {
      m.sigma = 0;
      m.smaller = false;
    }
    out[n++] = m;
  }
  /* Increasing along the segment: the roots come in increasing order where
   * qa is positive */
  if (n == 2 && sg.qa < 0)
  {
    ArcsegMeet tmp = out[0];
    out[0] = out[1];
    out[1] = tmp;
  }
  return n;
}

/*
 * The centre of the circle of an arc as M + N / D, every term formed exactly:
 * for an arc of three points M is its middle and, with A = S - M and
 * B = E - M, N = (|A|^2 By - |B|^2 Ay, |B|^2 Ax - |A|^2 Bx) and D = 2 A x B,
 * the circumcentre #emit_arc_edge forms from the same points; for a half
 * circle on a diameter M is its start, N = E - S and D = 2; for a centred
 * arc M is its centre, N = 0 and D = 1.
 */
static void
arcarc_centre_exact(ArcsegArena *arena, const Edge *e, ArcsegExact *mx,
  ArcsegExact *my, ArcsegExact *nx, ArcsegExact *ny, ArcsegExact *d)
{
  if (e->centred)
  {
    *mx = arcseg_x_diff(arena, e->cx, 0.0);
    *my = arcseg_x_diff(arena, e->cy, 0.0);
    *nx = arcseg_x_diff(arena, 0.0, 0.0);
    *ny = arcseg_x_diff(arena, 0.0, 0.0);
    *d = arcseg_x_diff(arena, 1.0, 0.0);
    return;
  }
  if (e->diameter)
  {
    *mx = arcseg_x_diff(arena, e->sx, 0.0);
    *my = arcseg_x_diff(arena, e->sy, 0.0);
    *nx = arcseg_x_diff(arena, e->ex, e->sx);
    *ny = arcseg_x_diff(arena, e->ey, e->sy);
    *d = arcseg_x_diff(arena, 2.0, 0.0);
    return;
  }
  *mx = arcseg_x_diff(arena, e->mx, 0.0);
  *my = arcseg_x_diff(arena, e->my, 0.0);
  ArcsegExact ax = arcseg_x_diff(arena, e->sx, e->mx);
  ArcsegExact ay = arcseg_x_diff(arena, e->sy, e->my);
  ArcsegExact bx = arcseg_x_diff(arena, e->ex, e->mx);
  ArcsegExact by = arcseg_x_diff(arena, e->ey, e->my);
  ArcsegExact a2 = arcseg_x_add(arena, arcseg_x_mul(arena, ax, ax),
    arcseg_x_mul(arena, ay, ay));
  ArcsegExact b2 = arcseg_x_add(arena, arcseg_x_mul(arena, bx, bx),
    arcseg_x_mul(arena, by, by));
  *nx = arcseg_x_sub(arena, arcseg_x_mul(arena, a2, by),
    arcseg_x_mul(arena, b2, ay));
  *ny = arcseg_x_sub(arena, arcseg_x_mul(arena, b2, ax),
    arcseg_x_mul(arena, a2, bx));
  *d = arcseg_x_scale(arena, arcseg_x_sub(arena, arcseg_x_mul(arena, ax, by),
    arcseg_x_mul(arena, ay, bx)), 2.0);
  return;
}

/* The square of the radius of the circle of an arc times D^2, exactly, with
 * the centre M + N / D of #arcarc_centre_exact: |D (S - M) - N|^2 from its
 * start S, or r^2 for a centred arc, whose D is 1 */
static ArcsegExact
arcarc_radius2_exact(ArcsegArena *arena, const Edge *e, ArcsegExact mx,
  ArcsegExact my, ArcsegExact nx, ArcsegExact ny, ArcsegExact d)
{
  if (e->centred)
  {
    ArcsegExact r = arcseg_x_diff(arena, e->radius, 0.0);
    return arcseg_x_mul(arena, r, r);
  }
  ArcsegExact sx = arcseg_x_sub(arena, arcseg_x_mul(arena, d,
    arcseg_x_sub(arena, arcseg_x_diff(arena, e->sx, 0.0), mx)), nx);
  ArcsegExact sy = arcseg_x_sub(arena, arcseg_x_mul(arena, d,
    arcseg_x_sub(arena, arcseg_x_diff(arena, e->sy, 0.0), my)), ny);
  return arcseg_x_add(arena, arcseg_x_mul(arena, sx, sx),
    arcseg_x_mul(arena, sy, sy));
}

/**
 * @brief Return the signs that place the second meeting of the circles of
 * two arcs sharing a point, decided exactly
 * @details Two distinct circles through a point J meet again at the
 * reflection Q of J across the line of their centres C1 and C2, and at J
 * alone where they touch there. With d = C2 - C1:
 * - @p s1, the sign of d x (J - C1), is 0 exactly where the circles touch at
 *   J, and gives the sense the two cross in: their tangents at J cross with
 *   that sign, at Q with the other one, each turned by the sense its arc
 *   turns;
 * - @p sp and @p sq, the signs of (E - S) . d for the chord S -> E of each
 *   arc, place Q on a side of the chord of an arc ending at J: Q - J is
 *   -2 (d x (J - C1)) / |d|^2 rot(d), and the chord crossed with rot(d) is
 *   the chord dotted with d, so the side is -s1 sp.
 * The centres are rational in the input coordinates (#arcarc_centre_exact),
 * so each sign is that of a polynomial in them once the denominators are
 * cleared, and each is computed exactly as expansions
 * @param[in] p,q Arcs, of two distinct circles, both holding J
 * @param[in] jx,jy The point they share, an input vertex
 * @param[out] s1,sp,sq The signs
 */
void
arcarc_shared_point_signs(const Edge *p, const Edge *q, double jx, double jy,
  int *s1, int *sp, int *sq)
{
  assert(p); assert(q); assert(s1); assert(sp); assert(sq);
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  ArcsegExact m1x, m1y, n1x, n1y, d1, m2x, m2y, n2x, n2y, d2;
  arcarc_centre_exact(arena, p, &m1x, &m1y, &n1x, &n1y, &d1);
  arcarc_centre_exact(arena, q, &m2x, &m2y, &n2x, &n2y, &d2);
  /* D1 D2 d = D1 D2 (M2 - M1) + D1 N2 - D2 N1 */
  ArcsegExact d12 = arcseg_x_mul(arena, d1, d2);
  ArcsegExact dx = arcseg_x_add(arena,
    arcseg_x_mul(arena, d12, arcseg_x_sub(arena, m2x, m1x)),
    arcseg_x_sub(arena, arcseg_x_mul(arena, d1, n2x),
      arcseg_x_mul(arena, d2, n1x)));
  ArcsegExact dy = arcseg_x_add(arena,
    arcseg_x_mul(arena, d12, arcseg_x_sub(arena, m2y, m1y)),
    arcseg_x_sub(arena, arcseg_x_mul(arena, d1, n2y),
      arcseg_x_mul(arena, d2, n1y)));
  /* D1 (J - C1) = D1 (J - M1) - N1 */
  ArcsegExact jx1 = arcseg_x_add(arena, arcseg_x_diff(arena, jx, 0.0),
    arcseg_x_scale(arena, m1x, -1.0));
  ArcsegExact jy1 = arcseg_x_add(arena, arcseg_x_diff(arena, jy, 0.0),
    arcseg_x_scale(arena, m1y, -1.0));
  ArcsegExact gx = arcseg_x_sub(arena, arcseg_x_mul(arena, d1, jx1), n1x);
  ArcsegExact gy = arcseg_x_sub(arena, arcseg_x_mul(arena, d1, jy1), n1y);
  /* (D1 D2 d) x (D1 (J - C1)) = D1^2 D2 (d x (J - C1)) */
  int sd2 = arcseg_x_sign(d2), sd12 = arcseg_x_sign(d12);
  *s1 = arcseg_x_sign(arcseg_x_sub(arena, arcseg_x_mul(arena, dx, gy),
    arcseg_x_mul(arena, dy, gx))) * sd2;
  const Edge *arcs[2] = {p, q};
  int *out[2] = {sp, sq};
  for (int k = 0; k < 2; k++)
  {
    ArcsegExact cx = arcseg_x_diff(arena, arcs[k]->ex, arcs[k]->sx);
    ArcsegExact cy = arcseg_x_diff(arena, arcs[k]->ey, arcs[k]->sy);
    *out[k] = arcseg_x_sign(arcseg_x_add(arena, arcseg_x_mul(arena, cx, dx),
      arcseg_x_mul(arena, cy, dy))) * sd12;
  }
  arcseg_arena_free(arena);
  return;
}

/* The two circles of a pair of arcs, exactly, in the terms of
 * #arcarc_meet_exact: the centre of the first as M1 + N1 / D1, D2,
 * L = D1 D2, d' = L (C2 - C1), Q = |d'|^2, G = Q + R1' - R2' and
 * H = 4 R1' Q - G^2 */
typedef struct
{
  ArcsegExact m1x, m1y, n1x, n1y, d1, d2, l, dx, dy, qq, g, h;
} ArcarcCircles;

static void
arcarc_circles_exact(ArcsegArena *arena, const Edge *p, const Edge *q,
  ArcarcCircles *c)
{
  ArcsegExact m1x, m1y, n1x, n1y, d1, m2x, m2y, n2x, n2y, d2;
  arcarc_centre_exact(arena, p, &m1x, &m1y, &n1x, &n1y, &d1);
  arcarc_centre_exact(arena, q, &m2x, &m2y, &n2x, &n2y, &d2);
  ArcsegExact l = arcseg_x_mul(arena, d1, d2);
  ArcsegExact dx = arcseg_x_add(arena,
    arcseg_x_mul(arena, l, arcseg_x_sub(arena, m2x, m1x)),
    arcseg_x_sub(arena, arcseg_x_mul(arena, d1, n2x),
      arcseg_x_mul(arena, d2, n1x)));
  ArcsegExact dy = arcseg_x_add(arena,
    arcseg_x_mul(arena, l, arcseg_x_sub(arena, m2y, m1y)),
    arcseg_x_sub(arena, arcseg_x_mul(arena, d1, n2y),
      arcseg_x_mul(arena, d2, n1y)));
  ArcsegExact qq = arcseg_x_add(arena, arcseg_x_mul(arena, dx, dx),
    arcseg_x_mul(arena, dy, dy));
  /* The squared radii, times L^2 */
  ArcsegExact r1 = arcseg_x_mul(arena, arcarc_radius2_exact(arena, p, m1x,
    m1y, n1x, n1y, d1), arcseg_x_mul(arena, d2, d2));
  ArcsegExact r2 = arcseg_x_mul(arena, arcarc_radius2_exact(arena, q, m2x,
    m2y, n2x, n2y, d2), arcseg_x_mul(arena, d1, d1));
  ArcsegExact g = arcseg_x_sub(arena, arcseg_x_add(arena, qq, r1), r2);
  c->m1x = m1x; c->m1y = m1y; c->n1x = n1x; c->n1y = n1y;
  c->d1 = d1; c->d2 = d2; c->l = l; c->dx = dx; c->dy = dy; c->qq = qq;
  c->g = g;
  c->h = arcseg_x_sub(arena,
    arcseg_x_scale(arena, arcseg_x_mul(arena, r1, qq), 4.0),
    arcseg_x_mul(arena, g, g));
  return;
}

/* An interval holding an exact value: the sum of its components, which
 * errs by at most the number of additions times half an epsilon of the sum
 * of their magnitudes, widened by the bound on how far the expansion lies
 * from the value and by a step outward for the rounding of its ends. False
 * where the sum is not finite */
static bool
arcseg_x_bounds(ArcsegExact a, double out[2])
{
  double sum = 0.0, mag = 0.0;
  for (int i = 0; i < a.n; i++)
  {
    sum += a.e[i];
    mag += fabs(a.e[i]);
  }
  double b = ((double) a.n * DBL_EPSILON * mag + a.err) *
    (1.0 + 4.0 * DBL_EPSILON) + 0x1p-1070;
  out[0] = sum - b;
  out[1] = sum + b;
  out[0] -= fabs(out[0]) * 0x1p-52 + 0x1p-1074;
  out[1] += fabs(out[1]) * 0x1p-52 + 0x1p-1074;
  return isfinite(out[0]) && isfinite(out[1]);
}

/**
 * @brief Return the terms placing where the circles of two arcs cross, each
 * as an interval holding its exact value
 * @details The terms of #arcarc_meet_exact: the meetings lie at
 * P = M1 + N1 / D1 + (G d' + sigma sqrt(H) rot(d')) / (2 Q L), rot(x, y) =
 * (-y, x), which hold however near the two centres lie, where the
 * difference of centres read in intervals holds zero
 * @param[in] p,q Arcs of two distinct circles
 * @param[out] terms The intervals, indexed by #ArcarcTerm
 * @return False where an interval is not finite or the arithmetic of
 * expansions left its exact range
 */
bool
arcarc_crossing_terms(const Edge *p, const Edge *q,
  double terms[ARCARC_NTERMS][2])
{
  assert(p); assert(q); assert(terms);
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  ArcarcCircles c;
  arcarc_circles_exact(arena, p, q, &c);
  const ArcsegExact *x[ARCARC_NTERMS] = {&c.m1x, &c.m1y, &c.n1x, &c.n1y,
    &c.d1, &c.l, &c.dx, &c.dy, &c.qq, &c.g, &c.h};
  bool ok = ! arena->unsafe;
  for (int k = 0; k < ARCARC_NTERMS && ok; k++)
    ok = arcseg_x_bounds(*x[k], terms[k]);
  arcseg_arena_free(arena);
  return ok;
}

/**
 * @brief Return where the circles of two arcs meet and whether each meeting
 * lies within both arcs, decided exactly
 * @details With the centres C = M + N / D of #arcarc_centre_exact and
 * L = D1 D2, the vector d = C2 - C1 times L is d' = L (M2 - M1) + D1 N2 -
 * D2 N1, Q = |d'|^2, and the squared radii times L^2 are R1' = |s1|^2 D2^2
 * and R2' = |s2|^2 D1^2, sk = Dk (Sk - Mk) - Nk. The circles meet where
 * H = 4 R1' Q - (Q + R1' - R2')^2 is not negative, at
 * P = B + sigma f rot(d), B on the line of the centres. A point of the circle
 * of an arc lies within it where it stands on the arc's side of its chord
 * c = E - S; for P that is the sign of U'' + sigma W'' sqrt(H) times the sign
 * of L, with U'' = 2 Q D2 (c x e') + (Q + R1' - R2') (c x d'),
 * e' = D1 (M1 - S) + N1, and W'' = c . d', which #arcseg_root_sign reads from
 * the signs of U'', W'' and U''^2 - W''^2 H; every point of the circle of a
 * centred arc lies within it. The tangents of the two circles
 * at P cross with the sign sigma times the senses the arcs turn
 * @param[in] p,q Arcs of two distinct circles
 * @param[out] meet The sign of H; for each sigma = -1, 1 in turn, whether P
 * lies within @p p and within @p q (1 or 0)
 */
void
arcarc_meet_exact(const Edge *p, const Edge *q, int *meet, int within[2][2])
{
  assert(p); assert(q); assert(meet); assert(within);
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  ArcarcCircles cc;
  arcarc_circles_exact(arena, p, q, &cc);
  ArcsegExact m1x = cc.m1x, m1y = cc.m1y, n1x = cc.n1x, n1y = cc.n1y,
    d1 = cc.d1, d2 = cc.d2, dx = cc.dx, dy = cc.dy, qq = cc.qq, g = cc.g,
    h = cc.h;
  int sl = arcseg_x_sign(cc.l);
  *meet = arcseg_x_sign(h);
  const Edge *arcs[2] = {p, q};
  for (int k = 0; k < 2; k++)
  {
    const Edge *e = arcs[k];
    /* A centred arc is its whole circle */
    if (e->centred)
    {
      within[0][k] = within[1][k] = 1;
      continue;
    }
    ArcsegExact cx = arcseg_x_diff(arena, e->ex, e->sx);
    ArcsegExact cy = arcseg_x_diff(arena, e->ey, e->sy);
    /* e' = D1 (M1 - S) + N1 */
    ArcsegExact ex = arcseg_x_add(arena, arcseg_x_mul(arena, d1,
      arcseg_x_sub(arena, m1x, arcseg_x_diff(arena, e->sx, 0.0))), n1x);
    ArcsegExact ey = arcseg_x_add(arena, arcseg_x_mul(arena, d1,
      arcseg_x_sub(arena, m1y, arcseg_x_diff(arena, e->sy, 0.0))), n1y);
    ArcsegExact ce = arcseg_x_sub(arena, arcseg_x_mul(arena, cx, ey),
      arcseg_x_mul(arena, cy, ex));
    ArcsegExact cd = arcseg_x_sub(arena, arcseg_x_mul(arena, cx, dy),
      arcseg_x_mul(arena, cy, dx));
    ArcsegExact u = arcseg_x_add(arena, arcseg_x_scale(arena,
      arcseg_x_mul(arena, arcseg_x_mul(arena, qq, d2), ce), 2.0),
      arcseg_x_mul(arena, g, cd));
    ArcsegExact w = arcseg_x_add(arena, arcseg_x_mul(arena, cx, dx),
      arcseg_x_mul(arena, cy, dy));
    int su = arcseg_x_sign(u), sw = arcseg_x_sign(w);
    int sq = arcseg_x_sign(arcseg_x_sub(arena, arcseg_x_mul(arena, u, u),
      arcseg_x_mul(arena, arcseg_x_mul(arena, w, w), h)));
    int want = e->diameter ? (e->ccw ? -1 : 1) :
      cross_product_sign(e->sx, e->sy, e->ex, e->ey, e->sx, e->sy, e->mx,
        e->my);
    for (int r = 0; r < 2; r++)
    {
      int sigma = (r == 0) ? -1 : 1;
      int side = (*meet == 0) ? su : arcseg_root_sign(su, sigma * sw, sq);
      within[r][k] = (side * sl == want) ? 1 : 0;
    }
  }
  arcseg_arena_free(arena);
  return;
}
/* The direction an edge leaves a point of it in, scaled by a positive
 * factor, exactly: the segment itself, or the radius of an arc to the point
 * turned a quarter in the sense it turns, the radius V - C being
 * ((V - M) D - N) / D with the centre of #arcarc_centre_exact; negated where
 * the edge arrives at the point rather than leaves it */
static void
edge_direction_exact(ArcsegArena *arena, const Edge *e, bool ccw, bool out,
  double vx, double vy, ArcsegExact *dx, ArcsegExact *dy)
{
  if (e->etype != EDGE_POLYARC && e->etype != EDGE_LINEARC)
  {
    *dx = arcseg_x_diff(arena, e->x2, e->x1);
    *dy = arcseg_x_diff(arena, e->y2, e->y1);
  }
  else
  {
    ArcsegExact mx, my, nx, ny, d;
    arcarc_centre_exact(arena, e, &mx, &my, &nx, &ny, &d);
    ArcsegExact wx = arcseg_x_sub(arena, arcseg_x_mul(arena,
      arcseg_x_sub(arena, arcseg_x_diff(arena, vx, 0.0), mx), d), nx);
    ArcsegExact wy = arcseg_x_sub(arena, arcseg_x_mul(arena,
      arcseg_x_sub(arena, arcseg_x_diff(arena, vy, 0.0), my), d), ny);
    if (arcseg_x_sign(d) < 0)
    {
      wx = arcseg_x_scale(arena, wx, -1.0);
      wy = arcseg_x_scale(arena, wy, -1.0);
    }
    /* Counterclockwise the radius (x, y) turns to (-y, x) */
    *dx = ccw ? arcseg_x_scale(arena, wy, -1.0) : wy;
    *dy = ccw ? wx : arcseg_x_scale(arena, wx, -1.0);
  }
  if (! out)
  {
    *dx = arcseg_x_scale(arena, *dx, -1.0);
    *dy = arcseg_x_scale(arena, *dy, -1.0);
  }
  return;
}

/* The half-plane of a direction from its exact signs: 0 for an angle in
 * [0, pi), 1 for [pi, 2pi), 2 for no direction */
static inline int
edge_direction_half_plane(int sx, int sy)
{
  if (sy != 0)
    return sy > 0 ? 0 : 1;
  return sx > 0 ? 0 : (sx < 0 ? 1 : 2);
}

/**
 * @brief Return how two edges leaving a point of both are ordered
 * counterclockwise from the positive x axis, decided exactly
 * @details Each direction is that of #edge_direction_exact. Two edges
 * leaving in one direction are ordered by how they bend from it: a segment
 * runs straight, and an arc bends toward its centre, which lies on its left
 * where it leaves the point counterclockwise or arrives at it clockwise, so
 * that it comes after a segment counterclockwise. The ordering mirrors that
 * of the intervals the arrangement reads first (#arcarc_meet_exact computes
 * on the same centres)
 * @param[in] e1,e2 Edges holding the point; NULL for the positive x axis @nullable
 * @param[in] ccw1,ccw2 Whether each arc turns counterclockwise
 * @param[in] out1,out2 Whether each edge leaves the point rather than
 * arrives at it
 * @param[in] vx,vy The point, an end of each edge or a vertex on it
 * @return -1 where the first comes before the second, 1 where after, 2 where
 * the two leave alike: two segments, two arcs or an edge along the axis
 */
int
edges_turn_cmp_exact(const Edge *e1, bool ccw1, bool out1, const Edge *e2,
  bool ccw2, bool out2, double vx, double vy)
{
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  ArcsegExact ax, ay, bx, by;
  if (e1)
    edge_direction_exact(arena, e1, ccw1, out1, vx, vy, &ax, &ay);
  else
  {
    ax = arcseg_x_diff(arena, 1.0, 0.0);
    ay = arcseg_x_diff(arena, 0.0, 0.0);
  }
  if (e2)
    edge_direction_exact(arena, e2, ccw2, out2, vx, vy, &bx, &by);
  else
  {
    bx = arcseg_x_diff(arena, 1.0, 0.0);
    by = arcseg_x_diff(arena, 0.0, 0.0);
  }
  int ha = edge_direction_half_plane(arcseg_x_sign(ax), arcseg_x_sign(ay));
  int hb = edge_direction_half_plane(arcseg_x_sign(bx), arcseg_x_sign(by));
  int result = 2;
  if (ha != 2 && hb != 2)
  {
    if (ha != hb)
      result = ha < hb ? -1 : 1;
    else
    {
      int s = arcseg_x_sign(arcseg_x_sub(arena, arcseg_x_mul(arena, ax, by),
        arcseg_x_mul(arena, ay, bx)));
      if (s != 0)
        result = s > 0 ? -1 : 1;
      else if (e1 && e2)
      {
        bool arc1 = e1->etype == EDGE_POLYARC || e1->etype == EDGE_LINEARC;
        bool arc2 = e2->etype == EDGE_POLYARC || e2->etype == EDGE_LINEARC;
        if (arc1 != arc2)
        {
          /* The arc bends left of the direction where it leaves the point
           * counterclockwise or arrives at it clockwise */
          bool left = arc1 ? (ccw1 == out1) : (ccw2 == out2);
          result = (left == arc1) ? 1 : -1;
        }
        else if (e1->centred && e2->centred && e1->radius != e2->radius)
        {
          /* Two whole circles touching at the point: the one bending
           * further left, the larger of the curvatures 1/r signed by the
           * side each bends to, comes after (#edges_turn_cmp_meet) */
          bool left1 = (ccw1 == out1), left2 = (ccw2 == out2);
          result = (left1 != left2) ? (left1 ? 1 : -1) :
            (((e1->radius < e2->radius) == left1) ? 1 : -1);
        }
      }
    }
  }
  arcseg_arena_free(arena);
  return result;
}

/* A point of the plane P = (X0 + X1 sqrt(H), Y0 + Y1 sqrt(H)) / W, every
 * term exact, H not negative: where two edges meet */
typedef struct
{
  ArcsegExact x0, x1, y0, y1, w, h;
} MeetExact;

/* The sign of U + W sqrt(H), H not negative */
static int
meet_root_sign(ArcsegArena *arena, ArcsegExact u, ArcsegExact w,
  ArcsegExact h)
{
  int su = arcseg_x_sign_checked(arena, u);
  int sw = arcseg_x_sign_checked(arena, w);
  if (sw == 0 || arcseg_x_sign_checked(arena, h) == 0)
    return su;
  int sq = arcseg_x_sign_checked(arena, arcseg_x_sub(arena,
    arcseg_x_mul(arena, u, u), arcseg_x_mul(arena, arcseg_x_mul(arena, w, w),
    h)));
  return arcseg_root_sign(su, sw, sq);
}

/* The sign of X + Y sqrt(H1) + (Z + T sqrt(H1)) sqrt(H2): that of
 * A + B sqrt(H2) with A and B in one radical, the larger term carrying it,
 * which the sign of A^2 - B^2 H2 = (X^2 + Y^2 H1 - (Z^2 + T^2 H1) H2) +
 * 2 (X Y - Z T H2) sqrt(H1) tells */
static int
meet_two_roots_sign(ArcsegArena *arena, ArcsegExact x, ArcsegExact y,
  ArcsegExact h1, ArcsegExact z, ArcsegExact t, ArcsegExact h2)
{
  int sa = meet_root_sign(arena, x, y, h1);
  if (arcseg_x_sign_checked(arena, h2) == 0)
    return sa;
  int sb = meet_root_sign(arena, z, t, h1);
  if (sb == 0)
    return sa;
  if (sa == 0 || sa == sb)
    return sb;
  ArcsegExact p0 = arcseg_x_sub(arena, arcseg_x_add(arena,
    arcseg_x_mul(arena, x, x), arcseg_x_mul(arena, arcseg_x_mul(arena, y, y),
    h1)), arcseg_x_mul(arena, arcseg_x_add(arena, arcseg_x_mul(arena, z, z),
    arcseg_x_mul(arena, arcseg_x_mul(arena, t, t), h1)), h2));
  ArcsegExact p1 = arcseg_x_scale(arena, arcseg_x_sub(arena,
    arcseg_x_mul(arena, x, y), arcseg_x_mul(arena, arcseg_x_mul(arena, z, t),
    h2)), 2.0);
  return sa * meet_root_sign(arena, p0, p1, h1);
}

/* An exact value of a double */
static inline ArcsegExact
meet_val(ArcsegArena *arena, double v)
{
  return arcseg_x_diff(arena, v, 0.0);
}

/*****************************************************************************
 * Numbers of a tower of square roots, their signs decided exactly
 *****************************************************************************/

/*
 * Where a segment tangent to two circles meets a circle or another such
 * segment, the coordinates lie in a tower of square roots Q(sqrt h_1)
 * (sqrt h_2) ..., each h_i a number of the tower below it and not negative.
 * A number of a tower of k roots is a + b sqrt(h_k), a and b numbers of the
 * k - 1 roots below, held as the 2^k exact coefficients of the products of
 * the roots: c[m] multiplies the product of sqrt(h_i) over the bits i of m,
 * so that a is the first half of the coefficients and b the second. Its sign
 * is that of a or of b sqrt(h_k) where they agree, and otherwise that of the
 * larger, which the sign of a^2 - b^2 h_k, a number of the tower below,
 * tells: the recursion #meet_root_sign and #meet_two_roots_sign take one and
 * two steps of. The roots of two towers are independent symbols, so two
 * numbers are compared in the tower of the roots of both, a root of one whose
 * number is one of the other, decided exactly, being read once
 * (#tower_merge).
 *
 * Each sign is read first in doubles carrying a bound on their error
 * (#ArcsegFilter), then in pairs of doubles carrying one (#DDBound), then
 * exactly as expansions, and where a product of
 * expansions leaves the range where they are exact -- their components span
 * the exponents of doubles, which the squarings of the recursion outgrow --
 * as #WideDyadic, the points of the question built again from what defines
 * them (#TowerCtx).
 */
#define TOWER_MAXDEPTH 4
#define TOWER_SIZE (1 << TOWER_MAXDEPTH)
#define TOWER_WIDE_BLOCK 32

/* The arithmetic the numbers of a tower are computed in: expansions in
 * @p arena, or #WideDyadic where @p wide is set, taken from blocks of
 * #TOWER_WIDE_BLOCK */
typedef struct
{
  ArcsegArena arena;
  bool wide;
  bool filter;         /**< Doubles carrying a bound on their error, a sign
                            the bound cannot tell marking the arena unsafe */
  bool dd;             /**< Pairs of doubles carrying a bound on their error
                            (#DDBound), as @p filter */
  WideDyadic **blocks;
  int nblocks;
  int capblocks;
  int used;            /**< Taken from the last block */
} TowerCtx;

/* A value of the arithmetic of a #TowerCtx, the whole union zeroed where it
 * is formed so that a copy reads no byte left unset */
typedef union
{
  ArcsegExact x;
  WideDyadic *w;
  ArcsegFilter f;
  DDBound d;
} TowerX;

/* A number of a tower, its coefficients by the products of the roots */
typedef struct
{
  TowerX c[TOWER_SIZE];
} TowerNum;

/* A tower: the numbers whose square roots it adds, h[i] a number of the i
 * roots below it */
typedef struct
{
  int depth;
  TowerNum h[TOWER_MAXDEPTH];
  const TangentPoint *key[TOWER_MAXDEPTH]; /**< For a root of the tangent of
                            two circles, a point of it, which tells the root
                            of another tangent of the same two circles without
                            arithmetic; NULL for any other root */
} Tower;

static void
tower_ctx_init(TowerCtx *ctx, bool wide)
{
  ctx->arena.used = ctx->arena.n = ctx->arena.cap = 0;
  ctx->arena.unsafe = false;
  ctx->arena.blocks = NULL;
  ctx->wide = wide;
  ctx->filter = false;
  ctx->dd = false;
  ctx->blocks = NULL;
  ctx->nblocks = ctx->capblocks = 0;
  ctx->used = TOWER_WIDE_BLOCK;
  return;
}

static void
tower_ctx_free(TowerCtx *ctx)
{
  arcseg_arena_free(&ctx->arena);
  for (int i = 0; i < ctx->nblocks; i++)
    pfree(ctx->blocks[i]);
  if (ctx->blocks)
    pfree(ctx->blocks);
  ctx->blocks = NULL;
  ctx->nblocks = ctx->capblocks = 0;
  ctx->used = TOWER_WIDE_BLOCK;
  return;
}

/* Whether the answers of the arithmetic of the expansions may not be exact */
static inline bool
tower_ctx_unsafe(const TowerCtx *ctx)
{
  return ! ctx->wide && ctx->arena.unsafe;
}

static WideDyadic *
tower_wide_alloc(TowerCtx *ctx)
{
  if (ctx->used == TOWER_WIDE_BLOCK)
  {
    if (ctx->nblocks == ctx->capblocks)
    {
      ctx->capblocks = ctx->capblocks ? 2 * ctx->capblocks : 16;
      ctx->blocks = ctx->blocks ?
        repalloc(ctx->blocks, sizeof(WideDyadic *) * ctx->capblocks) :
        palloc(sizeof(WideDyadic *) * ctx->capblocks);
    }
    ctx->blocks[ctx->nblocks++] =
      palloc(sizeof(WideDyadic) * TOWER_WIDE_BLOCK);
    ctx->used = 0;
  }
  return &ctx->blocks[ctx->nblocks - 1][ctx->used++];
}

/* The difference of two doubles, exactly */
static pg_attribute_always_inline TowerX
tx_diff(TowerCtx *ctx, double a, double b)
{
  TowerX r = {0};
  if (ctx->filter)
  {
    /* The rounding error of the difference, exactly (two-sum) */
    double v = a - b, bv = v - a;
    r.f.v = v;
    r.f.e = fabs((a - (v - bv)) + (- b - bv));
    return r;
  }
  if (ctx->dd)
  {
    two_diff(a, b, &r.d.hi, &r.d.lo);
    r.d.e = 0.0;
    return r;
  }
  if (! ctx->wide)
  {
    r.x = arcseg_x_diff(&ctx->arena, a, b);
    return r;
  }
  WideDyadic da, db;
  wide_dyadic_set_double(&da, a);
  wide_dyadic_set_double(&db, - b);
  r.w = tower_wide_alloc(ctx);
  wide_dyadic_add(&da, &db, r.w);
  return r;
}

static inline TowerX
tx_val(TowerCtx *ctx, double v)
{
  return tx_diff(ctx, v, 0.0);
}

/* Whether the expansions of a tower have left their exact range, so that
 * nothing more is computed in them: every answer of a context that is
 * unsafe is discarded (#tower_ctx_unsafe, #tower_pred_run), and an
 * expansion holding a component that overflowed keeps every component of
 * the values it is added to or multiplied by, compression merging none of
 * them, so that each later sum or product costs more than the last */
static inline bool
tx_spent(const TowerCtx *ctx)
{
  return ! ctx->filter && ! ctx->dd && tower_ctx_unsafe(ctx);
}

static pg_attribute_always_inline TowerX
tx_add(TowerCtx *ctx, TowerX a, TowerX b)
{
  TowerX r = {0};
  if (ctx->filter)
  {
    r.f = arcseg_f_add(a.f, b.f);
    return r;
  }
  if (ctx->dd)
  {
    r.d = ddb_add(a.d, b.d);
    return r;
  }
  if (! ctx->wide)
  {
    if (tx_spent(ctx))
      return a;
    r.x = arcseg_x_add(&ctx->arena, a.x, b.x);
    return r;
  }
  r.w = tower_wide_alloc(ctx);
  wide_dyadic_add(a.w, b.w, r.w);
  return r;
}

/* Multiplied by a power of two, possibly negative, exactly */
static pg_attribute_always_inline TowerX
tx_scale(TowerCtx *ctx, TowerX a, double k)
{
  TowerX r = {0};
  if (ctx->filter)
  {
    r.f = arcseg_f_scale(a.f, k);
    return r;
  }
  if (ctx->dd)
  {
    r.d.hi = a.d.hi * k;
    r.d.lo = a.d.lo * k;
    r.d.e = a.d.e * fabs(k);
    return r;
  }
  if (! ctx->wide)
  {
    r.x = arcseg_x_scale(&ctx->arena, a.x, k);
    return r;
  }
  int e;
  frexp(k, &e);
  r.w = tower_wide_alloc(ctx);
  *r.w = *a.w;
  if (r.w->sign != 0)
  {
    r.w->exp += e - 1;
    r.w->sign *= (k < 0.0) ? -1 : 1;
  }
  return r;
}

static pg_attribute_always_inline TowerX
tx_sub(TowerCtx *ctx, TowerX a, TowerX b)
{
  return tx_add(ctx, a, tx_scale(ctx, b, -1.0));
}

static pg_attribute_always_inline TowerX
tx_mul(TowerCtx *ctx, TowerX a, TowerX b)
{
  TowerX r = {0};
  if (ctx->filter)
  {
    r.f = arcseg_f_mul(a.f, b.f);
    return r;
  }
  if (ctx->dd)
  {
    r.d = ddb_mul(a.d, b.d);
    return r;
  }
  if (! ctx->wide)
  {
    if (tx_spent(ctx))
      return a;
    r.x = arcseg_x_mul(&ctx->arena, a.x, b.x);
    return r;
  }
  r.w = tower_wide_alloc(ctx);
  wide_dyadic_mul(a.w, b.w, r.w);
  return r;
}

/* Whether a value is zero, read off its representation */
static pg_attribute_always_inline bool
tx_zero(const TowerCtx *ctx, TowerX a)
{
  if (ctx->filter)
    return a.f.v == 0.0 && a.f.e == 0.0;
  if (ctx->dd)
    return ddb_is_zero(a.d);
  if (ctx->wide)
    return a.w->sign == 0;
  for (int i = 0; i < a.x.n; i++)
    if (a.x.e[i] != 0.0)
      return false;
  return a.x.err == 0.0;
}

/* The sign of a value, the arena recording where an expansion may not
 * carry it (#arcseg_x_sign_checked) */
static int
tx_sign(TowerCtx *ctx, TowerX a)
{
  if (ctx->filter)
  {
    /* An exact zero, as the coefficients a root does not reach, or a sign
     * the bound tells; any other leaves the question to the expansions */
    if (a.f.v == 0.0 && a.f.e == 0.0)
      return 0;
    int s = arcseg_f_sign(a.f);
    if (s == 2)
    {
      ctx->arena.unsafe = true;
      return 0;
    }
    return s;
  }
  if (ctx->dd)
  {
    if (ddb_is_zero(a.d))
      return 0;
    int s = ddb_sign(a.d);
    if (s == 0)
      ctx->arena.unsafe = true;
    return s;
  }
  if (ctx->wide)
    return a.w->sign;
  return arcseg_x_sign_checked(&ctx->arena, a.x);
}

/* A value as a double and a bound on its error */
static ArcsegFilter
tx_filter(const TowerCtx *ctx, TowerX a)
{
  ArcsegFilter r;
  if (ctx->filter)
    return a.f;
  if (ctx->dd)
  {
    /* The pair rounds once, its rounding error exactly (fast two-sum) */
    r.v = a.d.hi + a.d.lo;
    r.e = a.d.e + fabs(a.d.lo - (r.v - a.d.hi));
    return r;
  }
  if (ctx->wide)
  {
    int e;
    double m = wide_dyadic_frexp(a.w, &e);
    r.v = ldexp(m, e) * (double) a.w->sign;
    /* The two limbs read hold the value to 2^-60 of it, and ldexp rounds
     * once */
    r.e = fabs(r.v) * 0x1p-50;
    return r;
  }
  double s = 0.0, mag = 0.0;
  for (int i = 0; i < a.x.n; i++)
  {
    s += a.x.e[i];
    mag += fabs(a.x.e[i]);
  }
  r.v = s;
  r.e = (double) a.x.n * DBL_EPSILON * mag + a.x.err;
  return r;
}

/* Whether a number of k roots is zero in every coefficient */
static bool
tower_zero_k(const TowerCtx *ctx, const TowerX *x, int k)
{
  for (int m = 0; m < (1 << k); m++)
    if (! tx_zero(ctx, x[m]))
      return false;
  return true;
}

/* A number of k roots that is the value v */
static void
tower_set(TowerCtx *ctx, TowerX v, int k, TowerX *out)
{
  out[0] = v;
  for (int m = 1; m < (1 << k); m++)
    out[m] = tx_val(ctx, 0.0);
  return;
}

static void
tower_add_k(TowerCtx *ctx, const TowerX *x, const TowerX *y, int k,
  TowerX *out)
{
  for (int m = 0; m < (1 << k); m++)
    out[m] = tx_add(ctx, x[m], y[m]);
  return;
}

static void
tower_sub_k(TowerCtx *ctx, const TowerX *x, const TowerX *y, int k,
  TowerX *out)
{
  for (int m = 0; m < (1 << k); m++)
    out[m] = tx_sub(ctx, x[m], y[m]);
  return;
}

/* Multiplied by a power of two, possibly negative */
static void
tower_scale_k(TowerCtx *ctx, const TowerX *x, double s, int k, TowerX *out)
{
  for (int m = 0; m < (1 << k); m++)
    out[m] = tx_scale(ctx, x[m], s);
  return;
}

/* Multiplied by a value, the first coefficient written before the others
 * as #tower_set writes it */
static void
tower_mulx_k(TowerCtx *ctx, const TowerX *x, TowerX v, int k, TowerX *out)
{
  out[0] = tx_mul(ctx, x[0], v);
  for (int m = 1; m < (1 << k); m++)
    out[m] = tx_mul(ctx, x[m], v);
  return;
}

/* The product of two numbers of k roots of a tower:
 * (a1 + b1 r)(a2 + b2 r) = (a1 a2 + b1 b2 h) + (a1 b2 + b1 a2) r */
static void
tower_mul_k(TowerCtx *ctx, const Tower *t, const TowerX *x, const TowerX *y,
  int k, TowerX *out)
{
  if (k == 0)
  {
    out[0] = tx_mul(ctx, x[0], y[0]);
    return;
  }
  int half = 1 << (k - 1);
  bool xb = ! tower_zero_k(ctx, x + half, k - 1);
  bool yb = ! tower_zero_k(ctx, y + half, k - 1);
  TowerX p[TOWER_SIZE / 2], q[TOWER_SIZE / 2], r1[TOWER_SIZE / 2],
    r2[TOWER_SIZE / 2];
  tower_mul_k(ctx, t, x, y, k - 1, p);
  if (xb && yb)
  {
    TowerX bb[TOWER_SIZE / 2];
    tower_mul_k(ctx, t, x + half, y + half, k - 1, bb);
    tower_mul_k(ctx, t, bb, t->h[k - 1].c, k - 1, q);
    tower_add_k(ctx, p, q, k - 1, p);
  }
  if (yb)
    tower_mul_k(ctx, t, x, y + half, k - 1, r1);
  if (xb)
    tower_mul_k(ctx, t, x + half, y, k - 1, r2);
  if (xb && yb)
    tower_add_k(ctx, r1, r2, k - 1, r1);
  else if (xb)
    memcpy(r1, r2, sizeof(TowerX) * half);
  else if (! yb)
    tower_set(ctx, tx_val(ctx, 0.0), k - 1, r1);
  memcpy(out, p, sizeof(TowerX) * half);
  memcpy(out + half, r1, sizeof(TowerX) * half);
  return;
}

/* The square root of a filtered value not negative: within the error
 * bound e of v, sqrt moves by at most e / (sqrt(v) + sqrt(v - e)), and by at
 * most sqrt(v + e) where v - e may be negative */
static ArcsegFilter
tower_f_sqrt(ArcsegFilter h)
{
  ArcsegFilter r;
  double s = sqrt(fmax(h.v, 0.0));
  double lo = h.v - h.e;
  r.v = s;
  r.e = (lo > 0.0) ? h.e / (s + sqrt(lo)) : sqrt(fmax(h.v + h.e, 0.0));
  r.e = r.e * (1.0 + 4.0 * DBL_EPSILON) + 2.0 * DBL_EPSILON * s;
  return r;
}

/* The square root of a pair of doubles not negative: one Newton step from
 * the root of its larger part, which lands within 2^-102 of the root, and
 * within the error bound e of the pair the root moves as in #tower_f_sqrt */
static DDBound
tower_dd_sqrt(DDBound h)
{
  DDBound r;
  double v = h.hi + h.lo;
  if (! (h.hi > 0.0))
  {
    r.hi = r.lo = 0.0;
    r.e = sqrt(fmax(v + h.e, 0.0)) * (1.0 + 4.0 * DBL_EPSILON) + DDB_TINY;
    return r;
  }
  double s = sqrt(h.hi), p, pe;
  two_product(s, s, &p, &pe);
  two_sum(s, ((h.hi - p) - pe + h.lo) / (2.0 * s), &r.hi, &r.lo);
  double lo = v - h.e;
  double moved = (h.e == 0.0) ? 0.0 :
    ((lo > 0.0) ? h.e / (s + sqrt(lo)) : sqrt(fmax(v + h.e, 0.0)));
  r.e = moved * (1.0 + 4.0 * DBL_EPSILON) + DDB_EPS * s + DDB_TINY;
  return r;
}

/* A number of k roots of a tower as a pair of doubles and a bound on its
 * error, in the arithmetic of a context whose values are such pairs */
static DDBound
tower_dd_k(const TowerCtx *ctx, const Tower *t, const TowerX *x, int k)
{
  if (k == 0)
    return x[0].d;
  int half = 1 << (k - 1);
  DDBound a = tower_dd_k(ctx, t, x, k - 1);
  if (tower_zero_k(ctx, x + half, k - 1))
    return a;
  DDBound b = tower_dd_k(ctx, t, x + half, k - 1);
  DDBound h = tower_dd_k(ctx, t, t->h[k - 1].c, k - 1);
  return ddb_add(a, ddb_mul(b, tower_dd_sqrt(h)));
}

/* A number of k roots of a tower as a double and a bound on its error */
static ArcsegFilter
tower_filter_k(const TowerCtx *ctx, const Tower *t, const TowerX *x, int k)
{
  if (k == 0)
    return tx_filter(ctx, x[0]);
  int half = 1 << (k - 1);
  ArcsegFilter a = tower_filter_k(ctx, t, x, k - 1);
  if (tower_zero_k(ctx, x + half, k - 1))
    return a;
  ArcsegFilter b = tower_filter_k(ctx, t, x + half, k - 1);
  ArcsegFilter h = tower_filter_k(ctx, t, t->h[k - 1].c, k - 1);
  return arcseg_f_add(a, arcseg_f_mul(b, tower_f_sqrt(h)));
}

/* The sign of a number of k roots: of its filter where the bound tells it,
 * and otherwise of a + b sqrt(h) from the signs of a, b, h and
 * a^2 - b^2 h, each a number of the roots below */
static int
tower_sign_k(TowerCtx *ctx, const Tower *t, const TowerX *x, int k)
{
  if (k == 0)
    return tx_sign(ctx, x[0]);
  int s;
  if (ctx->dd)
  {
    s = ddb_sign(tower_dd_k(ctx, t, x, k));
    if (s != 0)
      return s;
  }
  else
  {
    s = arcseg_f_sign(tower_filter_k(ctx, t, x, k));
    if (s != 2)
      return s;
  }
  int half = 1 << (k - 1);
  int sa = tower_sign_k(ctx, t, x, k - 1);
  if (tower_zero_k(ctx, x + half, k - 1))
    return sa;
  int sb = tower_sign_k(ctx, t, x + half, k - 1);
  int sh = tower_sign_k(ctx, t, t->h[k - 1].c, k - 1);
  /* A root of a negative number is no real number */
  if (sh < 0)
    ctx->arena.unsafe = true;
  if (sb == 0 || sh <= 0)
    return sa;
  if (sa == 0 || sa == sb)
    return sb;
  TowerX aa[TOWER_SIZE / 2], bb[TOWER_SIZE / 2];
  tower_mul_k(ctx, t, x, x, k - 1, aa);
  tower_mul_k(ctx, t, x + half, x + half, k - 1, bb);
  tower_mul_k(ctx, t, bb, t->h[k - 1].c, k - 1, bb);
  tower_sub_k(ctx, aa, bb, k - 1, aa);
  return sa * tower_sign_k(ctx, t, aa, k - 1);
}

/* A number of k roots of one tower read in another, its root i the root
 * map[i] there: its coefficient m moves to the product of the roots map[i]
 * over the bits i of m */
static void
tower_map(TowerCtx *ctx, const TowerX *x, int k, const int *map, int depth,
  TowerX *out)
{
  TowerX tmp[TOWER_SIZE];
  for (int m = 0; m < (1 << depth); m++)
    tmp[m] = tx_val(ctx, 0.0);
  for (int m = 0; m < (1 << k); m++)
  {
    int mm = 0;
    for (int i = 0; i < k; i++)
      if (m & (1 << i))
        mm |= 1 << map[i];
    tmp[mm] = x[m];
  }
  memcpy(out, tmp, sizeof(TowerX) * (1 << depth));
  return;
}

/* Whether two numbers of k roots carry equal coefficients, decided
 * exactly: then they are one number */
static bool
tower_same_k(TowerCtx *ctx, const TowerX *x, const TowerX *y, int k)
{
  for (int m = 0; m < (1 << k); m++)
    if (tx_sign(ctx, tx_sub(ctx, x[m], y[m])) != 0)
      return false;
  return true;
}

/* Whether two tangent points belong to tangents of one pair of circles, in
 * either order, whose roots sqrt(E), E = |C2 - C1|^2 - (r1 - r2)^2, are
 * then one, as their doubles say without arithmetic (#tower_point_tangent) */
static bool
tangent_same_pair(const TangentPoint *a, const TangentPoint *b)
{
  if (a->c1x == b->c1x && a->c1y == b->c1y && a->r1 == b->r1 &&
      a->c2x == b->c2x && a->c2y == b->c2y && a->r2 == b->r2)
    return true;
  return a->c1x == b->c2x && a->c1y == b->c2y && a->r1 == b->r2 &&
    a->c2x == b->c1x && a->c2y == b->c1y && a->r2 == b->r1;
}

/* Merge the roots of tower @p t2 into @p t, a root of @p t2 whose number is
 * one of @p t, once read there, being that root: map[i] is where root i of
 * @p t2 stands in @p t. False where @p t would hold more than
 * #TOWER_MAXDEPTH roots */
static bool
tower_merge(TowerCtx *ctx, Tower *t, const Tower *t2, int *map)
{
  static const int ident[TOWER_MAXDEPTH] = {0, 1, 2, 3};
  for (int i = 0; i < t2->depth; i++)
  {
    TowerX h[TOWER_SIZE];
    tower_map(ctx, t2->h[i].c, i, map, t->depth, h);
    int j = 0;
    for (; j < t->depth; j++)
    {
      /* The roots of two tangents of one pair of circles */
      if (t->key[j] && t2->key[i] && tangent_same_pair(t->key[j], t2->key[i]))
        break;
      TowerX hj[TOWER_SIZE];
      tower_map(ctx, t->h[j].c, j, ident, t->depth, hj);
      if (tower_same_k(ctx, h, hj, t->depth))
        break;
    }
    if (j == t->depth)
    {
      if (t->depth == TOWER_MAXDEPTH)
        return false;
      memcpy(t->h[j].c, h, sizeof(TowerX) * (1 << j));
      t->key[j] = t2->key[i];
      t->depth++;
    }
    map[i] = j;
  }
  return true;
}

/*****************************************************************************
 * Points of a tower of square roots
 *****************************************************************************/

/* A point (x / w, y / w) of a tower, taken from an origin, w not zero */
typedef struct
{
  Tower t;
  TowerNum x, y, w;
} TowerPoint;

/* A point given by doubles */
static void
tower_point_xy(TowerCtx *ctx, double px, double py, double ox, double oy,
  TowerPoint *out)
{
  out->t.depth = 0;
  out->x.c[0] = tx_diff(ctx, px, ox);
  out->y.c[0] = tx_diff(ctx, py, oy);
  out->w.c[0] = tx_val(ctx, 1.0);
  return;
}

/**
 * @brief Set a point where the common tangent of two circles touches one of
 * them, exactly
 * @details For circles (C1, r1) and (C2, r2) with d = C2 - C1 and
 * E = |d|^2 - (r1 - r2)^2 > 0, the tangent on the side @p side of C1 -> C2,
 * 1 its left, touches circle k at Ck + rk u with the unit normal
 * u = ((r1 - r2) d + side sqrt(E) rot(d)) / |d|^2, rot(x, y) = (-y, x):
 * the points #trapezoid_make constructs by angles, Ta for side 1. The tower
 * has the one root sqrt(E)
 */
static void
tower_point_tangent(TowerCtx *ctx, const TangentPoint *tp, double ox,
  double oy, TowerPoint *out)
{
  TowerX dx = tx_diff(ctx, tp->c2x, tp->c1x);
  TowerX dy = tx_diff(ctx, tp->c2y, tp->c1y);
  TowerX dr = tx_diff(ctx, tp->r1, tp->r2);
  TowerX d2 = tx_add(ctx, tx_mul(ctx, dx, dx), tx_mul(ctx, dy, dy));
  TowerX rk = tx_val(ctx, tp->k ? tp->r2 : tp->r1);
  TowerX rkdr = tx_mul(ctx, rk, dr);
  TowerX srk = tx_scale(ctx, rk, (double) tp->side);
  out->t.depth = 1;
  out->t.key[0] = tp;
  out->t.h[0].c[0] = tx_sub(ctx, d2, tx_mul(ctx, dr, dr));
  /* Ck d^2 + rk (r1 - r2) d + side rk sqrt(E) rot(d), over d^2 */
  out->x.c[0] = tx_add(ctx, tx_mul(ctx,
    tx_diff(ctx, tp->k ? tp->c2x : tp->c1x, ox), d2), tx_mul(ctx, rkdr, dx));
  out->x.c[1] = tx_scale(ctx, tx_mul(ctx, srk, dy), -1.0);
  out->y.c[0] = tx_add(ctx, tx_mul(ctx,
    tx_diff(ctx, tp->k ? tp->c2y : tp->c1y, oy), d2), tx_mul(ctx, rkdr, dy));
  out->y.c[1] = tx_mul(ctx, srk, dx);
  tower_set(ctx, d2, 1, out->w.c);
  return;
}

/**
 * @brief Set a point where two circles cross, the meeting sigma, and return
 * the sign of the number under its root: none where it is negative, a
 * touching where it is zero
 * @details With d = C2 - C1, D = |d|^2 and g = D + r1^2 - r2^2 the
 * meetings are C1 + (g d + sigma sqrt(4 r1^2 D - g^2) rot(d)) / (2 D), the
 * points #arcarc_meet_exact places for two centred arcs
 */
static int
tower_point_circles(TowerCtx *ctx, double c1x, double c1y, double r1,
  double c2x, double c2y, double r2, int sigma, double ox, double oy,
  TowerPoint *out)
{
  TowerX dx = tx_diff(ctx, c2x, c1x), dy = tx_diff(ctx, c2y, c1y);
  TowerX dd = tx_add(ctx, tx_mul(ctx, dx, dx), tx_mul(ctx, dy, dy));
  TowerX a = tx_val(ctx, r1), b = tx_val(ctx, r2);
  TowerX g = tx_sub(ctx, tx_add(ctx, dd, tx_mul(ctx, a, a)),
    tx_mul(ctx, b, b));
  TowerX h = tx_sub(ctx, tx_scale(ctx, tx_mul(ctx, tx_mul(ctx, a, a), dd),
    4.0), tx_mul(ctx, g, g));
  int sh = tx_sign(ctx, h);
  if (sh < 0)
    return sh;
  TowerX w = tx_scale(ctx, dd, 2.0);
  out->t.depth = 1;
  out->t.key[0] = NULL;
  out->t.h[0].c[0] = h;
  out->x.c[0] = tx_add(ctx, tx_mul(ctx, tx_diff(ctx, c1x, ox), w),
    tx_mul(ctx, g, dx));
  out->x.c[1] = tx_scale(ctx, dy, (double) - sigma);
  out->y.c[0] = tx_add(ctx, tx_mul(ctx, tx_diff(ctx, c1y, oy), w),
    tx_mul(ctx, g, dy));
  out->y.c[1] = tx_scale(ctx, dx, (double) sigma);
  tower_set(ctx, w, 1, out->w.c);
  return sh;
}

/* The coordinates of several points read in one tower, the tower of all
 * their roots: xyw[i][0..2] the x, y and w of point i. False where it would
 * hold more than #TOWER_MAXDEPTH roots */
static bool
tower_points_read(TowerCtx *ctx, const TowerPoint *const *pts, int n,
  Tower *t, TowerX (*xyw)[3][TOWER_SIZE])
{
  int map[8][TOWER_MAXDEPTH];
  assert(n <= 8);
  t->depth = 0;
  for (int i = 0; i < n; i++)
    if (! tower_merge(ctx, t, &pts[i]->t, map[i]))
      return false;
  for (int i = 0; i < n; i++)
  {
    int k = pts[i]->t.depth;
    tower_map(ctx, pts[i]->x.c, k, map[i], t->depth, xyw[i][0]);
    tower_map(ctx, pts[i]->y.c, k, map[i], t->depth, xyw[i][1]);
    tower_map(ctx, pts[i]->w.c, k, map[i], t->depth, xyw[i][2]);
  }
  return true;
}

/* The point where the line through P0 and P1 crosses the line through Q0 and
 * Q1: the cross product of the two lines in homogeneous coordinates. False
 * where its tower would hold too many roots */
static bool
tower_point_lines(TowerCtx *ctx, const TowerPoint *p0, const TowerPoint *p1,
  const TowerPoint *q0, const TowerPoint *q1, TowerPoint *out)
{
  const TowerPoint *pts[4] = {p0, p1, q0, q1};
  TowerX c[4][3][TOWER_SIZE];
  if (! tower_points_read(ctx, pts, 4, &out->t, c))
    return false;
  const Tower *t = &out->t;
  int k = t->depth;
  TowerX l[2][3][TOWER_SIZE], u[TOWER_SIZE], v[TOWER_SIZE];
  for (int j = 0; j < 2; j++)
    for (int r = 0; r < 3; r++)
    {
      /* (a x b)[r] = a[r + 1] b[r + 2] - a[r + 2] b[r + 1] */
      int r1 = (r + 1) % 3, r2 = (r + 2) % 3;
      tower_mul_k(ctx, t, c[2 * j][r1], c[2 * j + 1][r2], k, u);
      tower_mul_k(ctx, t, c[2 * j][r2], c[2 * j + 1][r1], k, v);
      tower_sub_k(ctx, u, v, k, l[j][r]);
    }
  TowerX *o[3] = {out->x.c, out->y.c, out->w.c};
  for (int r = 0; r < 3; r++)
  {
    int r1 = (r + 1) % 3, r2 = (r + 2) % 3;
    tower_mul_k(ctx, t, l[0][r1], l[1][r2], k, u);
    tower_mul_k(ctx, t, l[0][r2], l[1][r1], k, v);
    tower_sub_k(ctx, u, v, k, o[r]);
  }
  return true;
}

/**
 * @brief Set a point where the line through P0 and P1 meets a circle, the
 * root sigma along the line, and return the sign of the number under its
 * root: none where it is negative, a touching where it is zero, 2 where the
 * point is not built
 * @details With U = P0 - C w0 and V = P1 w0 - P0 w1 the line is
 * P0 / w0 + t V / (w0 w1), and it meets the circle at
 * t = (-(V.U) + sigma sw1 sqrt(D)) w1 / (V.V), sw1 the sign of w1 and
 * D = (V.U)^2 - (V.V) (U.U - r^2 w0^2): the point
 * (P0 (V.V) + (-(V.U) + sigma sw1 sqrt(D)) V) / (w0 (V.V)), in the tower of
 * the line with sqrt(D) above it. Sigma 1 is the farther along P0 -> P1
 */
static int
tower_point_line_circle(TowerCtx *ctx, const TowerPoint *p0,
  const TowerPoint *p1, double cx, double cy, double r, int sigma, double ox,
  double oy, TowerPoint *out)
{
  const TowerPoint *pts[2] = {p0, p1};
  TowerX c[2][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, 2, &t, c) || t.depth == TOWER_MAXDEPTH)
    return 2;
  int k = t.depth;
  TowerX ux[TOWER_SIZE], uy[TOWER_SIZE], vx[TOWER_SIZE], vy[TOWER_SIZE],
    a[TOWER_SIZE], b[TOWER_SIZE];
  /* U = P0 - C w0 */
  tower_mulx_k(ctx, c[0][2], tx_diff(ctx, cx, ox), k, a);
  tower_sub_k(ctx, c[0][0], a, k, ux);
  tower_mulx_k(ctx, c[0][2], tx_diff(ctx, cy, oy), k, a);
  tower_sub_k(ctx, c[0][1], a, k, uy);
  /* V = P1 w0 - P0 w1 */
  tower_mul_k(ctx, &t, c[1][0], c[0][2], k, a);
  tower_mul_k(ctx, &t, c[0][0], c[1][2], k, b);
  tower_sub_k(ctx, a, b, k, vx);
  tower_mul_k(ctx, &t, c[1][1], c[0][2], k, a);
  tower_mul_k(ctx, &t, c[0][1], c[1][2], k, b);
  tower_sub_k(ctx, a, b, k, vy);
  TowerX vv[TOWER_SIZE], vu[TOWER_SIZE], uu[TOWER_SIZE], d[TOWER_SIZE];
  tower_mul_k(ctx, &t, vx, vx, k, a);
  tower_mul_k(ctx, &t, vy, vy, k, b);
  tower_add_k(ctx, a, b, k, vv);
  tower_mul_k(ctx, &t, vx, ux, k, a);
  tower_mul_k(ctx, &t, vy, uy, k, b);
  tower_add_k(ctx, a, b, k, vu);
  tower_mul_k(ctx, &t, ux, ux, k, a);
  tower_mul_k(ctx, &t, uy, uy, k, b);
  tower_add_k(ctx, a, b, k, uu);
  /* U.U - r^2 w0^2 */
  TowerX rr = tx_val(ctx, r);
  tower_mul_k(ctx, &t, c[0][2], c[0][2], k, a);
  tower_mulx_k(ctx, a, tx_mul(ctx, rr, rr), k, a);
  tower_sub_k(ctx, uu, a, k, uu);
  tower_mul_k(ctx, &t, vu, vu, k, a);
  tower_mul_k(ctx, &t, vv, uu, k, b);
  tower_sub_k(ctx, a, b, k, d);
  int disc = tower_sign_k(ctx, &t, d, k);
  int sw1 = tower_sign_k(ctx, &t, c[1][2], k);
  if (tower_ctx_unsafe(ctx) || sw1 == 0)
    return 2;
  if (disc < 0)
    return disc;
  out->t = t;
  memcpy(out->t.h[k].c, d, sizeof(TowerX) * (1 << k));
  out->t.key[k] = NULL;
  out->t.depth = k + 1;
  int half = 1 << k;
  double s = (double) (sigma * sw1);
  /* x = x0 (V.V) - (V.U) Vx + s Vx sqrt(D) */
  TowerX *o[2] = {out->x.c, out->y.c};
  TowerX *vc[2] = {vx, vy};
  for (int j = 0; j < 2; j++)
  {
    tower_mul_k(ctx, &t, c[0][j], vv, k, a);
    tower_mul_k(ctx, &t, vu, vc[j], k, b);
    tower_sub_k(ctx, a, b, k, o[j]);
    tower_scale_k(ctx, vc[j], s, k, o[j] + half);
  }
  tower_mul_k(ctx, &t, c[0][2], vv, k, out->w.c);
  tower_set(ctx, tx_val(ctx, 0.0), k, out->w.c + half);
  return disc;
}

/* The sign of the orientation of three points, 1 where they turn
 * counterclockwise: the determinant of their homogeneous coordinates times
 * the signs of their w; 2 where it is not told */
static int
tower_orient(TowerCtx *ctx, const TowerPoint *p, const TowerPoint *q,
  const TowerPoint *r)
{
  const TowerPoint *pts[3] = {p, q, r};
  TowerX c[3][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, 3, &t, c))
    return 2;
  int k = t.depth;
  TowerX det[TOWER_SIZE], u[TOWER_SIZE], v[TOWER_SIZE], m[TOWER_SIZE];
  tower_set(ctx, tx_val(ctx, 0.0), k, det);
  for (int i = 0; i < 3; i++)
  {
    /* p . (q x r) */
    int i1 = (i + 1) % 3, i2 = (i + 2) % 3;
    tower_mul_k(ctx, &t, c[1][i1], c[2][i2], k, u);
    tower_mul_k(ctx, &t, c[1][i2], c[2][i1], k, v);
    tower_sub_k(ctx, u, v, k, m);
    tower_mul_k(ctx, &t, c[0][i], m, k, m);
    tower_add_k(ctx, det, m, k, det);
  }
  int s = tower_sign_k(ctx, &t, det, k);
  for (int i = 0; i < 3 && s != 0; i++)
    s *= tower_sign_k(ctx, &t, c[i][2], k);
  return tower_ctx_unsafe(ctx) ? 2 : s;
}

/*****************************************************************************
 * Segments between tangent points, and the meetings they make
 *****************************************************************************/

/* A tangent point in doubles, from the formula #tower_point_tangent places
 * it by exactly */
static void
tangent_point_round(const TangentPoint *tp, double *x, double *y)
{
  double dx = tp->c2x - tp->c1x, dy = tp->c2y - tp->c1y;
  double d2 = dx * dx + dy * dy, dr = tp->r1 - tp->r2;
  double se = tp->side * sqrt(fmax(d2 - dr * dr, 0.0));
  double ux = (dr * dx - se * dy) / d2, uy = (dr * dy + se * dx) / d2;
  double rk = tp->k ? tp->r2 : tp->r1;
  *x = (tp->k ? tp->c2x : tp->c1x) + rk * ux;
  *y = (tp->k ? tp->c2y : tp->c1y) + rk * uy;
  return;
}

/* A question about meetings, answered in the arithmetic of @p ctx: its
 * answer, or 2 where it is not told there */
typedef int (*TowerPred)(TowerCtx *ctx, const void *arg);

/* The answer of a question in expansions, and in #WideDyadic where those
 * leave their exact range, the question building its points again */
static int
tower_pred_run(TowerPred fn, const void *arg)
{
  TowerCtx ctx;
  /* In doubles, then in pairs of doubles, carrying a bound on their error
   * first */
  int r = 2;
  bool again = true;
  for (int pass = 0; pass < 2 && again; pass++)
  {
    tower_ctx_init(&ctx, false);
    ctx.filter = (pass == 0);
    ctx.dd = (pass == 1);
    r = fn(&ctx, arg);
    again = (r == 2) || ctx.arena.unsafe;
    tower_ctx_free(&ctx);
  }
  if (! again)
    return r;
  tower_ctx_init(&ctx, false);
  r = fn(&ctx, arg);
  again = (r == 2) || ctx.arena.unsafe;
  tower_ctx_free(&ctx);
  if (! again)
    return r;
  tower_ctx_init(&ctx, true);
  r = fn(&ctx, arg);
  tower_ctx_free(&ctx);
  return r;
}

/* The two ends of a segment between tangent points */
static void
towerseg_ends(TowerCtx *ctx, const Edge *s, double ox, double oy,
  TowerPoint *p0, TowerPoint *p1)
{
  if (s->from)
  {
    tower_point_tangent(ctx, s->from, ox, oy, p0);
    tower_point_tangent(ctx, s->to, ox, oy, p1);
  }
  else
  {
    tower_point_xy(ctx, s->x1, s->y1, ox, oy, p0);
    tower_point_xy(ctx, s->x2, s->y2, ox, oy, p1);
  }
  return;
}

/**
 * @brief Set the point of a meeting in the arithmetic of @p ctx, taken from
 * an origin; return 1, 0 where the kind is not one the tower places, 2 where
 * it is not told
 * @details A vertex, a tangent point, the crossing of two segments, between
 * tangent points or of the input (#towerseg_ends), or of two centred arcs,
 * and the meeting of such a segment with the circle of a centred arc
 */
static int
meet_tower_build(TowerCtx *ctx, const MeetPoint *m, double ox, double oy,
  TowerPoint *out)
{
  switch (m->kind)
  {
    case MEET_VERTEX:
      tower_point_xy(ctx, m->x, m->y, ox, oy, out);
      return 1;
    case MEET_TANGENT:
      tower_point_tangent(ctx, m->tp, ox, oy, out);
      return 1;
    case MEET_SEGSEG:
    case MEET_LINES:
    {
      TowerPoint p0, p1, q0, q1;
      towerseg_ends(ctx, m->p, ox, oy, &p0, &p1);
      towerseg_ends(ctx, m->q, ox, oy, &q0, &q1);
      return tower_point_lines(ctx, &p0, &p1, &q0, &q1, out) ? 1 : 2;
    }
    case MEET_LINECIRCLE:
    {
      TowerPoint p0, p1;
      towerseg_ends(ctx, m->p, ox, oy, &p0, &p1);
      int disc = tower_point_line_circle(ctx, &p0, &p1, m->q->cx, m->q->cy,
        m->q->radius, m->sigma, ox, oy, out);
      return (disc == 2) ? 2 : ((disc < 0) ? 0 : 1);
    }
    case MEET_ARCARC:
      if (! m->p->centred || ! m->q->centred)
        return 0;
      return tower_point_circles(ctx, m->p->cx, m->p->cy, m->p->radius,
        m->q->cx, m->q->cy, m->q->radius, m->sigma, ox, oy, out) < 0 ? 0 : 1;
    default:
      return 0;
  }
}

/* The sign of (B - A) . (D - C), the points of one tower question */
static int
tower_dot(TowerCtx *ctx, const TowerPoint *a, const TowerPoint *b,
  const TowerPoint *c, const TowerPoint *d)
{
  const TowerPoint *pts[4] = {a, b, c, d};
  TowerX v[4][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, 4, &t, v))
    return 2;
  int k = t.depth;
  TowerX s[TOWER_SIZE], u[TOWER_SIZE], w[TOWER_SIZE], z[TOWER_SIZE];
  tower_set(ctx, tx_val(ctx, 0.0), k, s);
  for (int j = 0; j < 2; j++)
  {
    /* (B wA - A wB) and (D wC - C wD), the vectors times wA wB and wC wD */
    tower_mul_k(ctx, &t, v[1][j], v[0][2], k, u);
    tower_mul_k(ctx, &t, v[0][j], v[1][2], k, w);
    tower_sub_k(ctx, u, w, k, u);
    tower_mul_k(ctx, &t, v[3][j], v[2][2], k, z);
    tower_mul_k(ctx, &t, v[2][j], v[3][2], k, w);
    tower_sub_k(ctx, z, w, k, z);
    tower_mul_k(ctx, &t, u, z, k, u);
    tower_add_k(ctx, s, u, k, s);
  }
  int sign = tower_sign_k(ctx, &t, s, k);
  for (int i = 0; i < 4 && sign != 0; i++)
    sign *= tower_sign_k(ctx, &t, v[i][2], k);
  return tower_ctx_unsafe(ctx) ? 2 : sign;
}

/* Whether two points of tower questions are one point: 1, 0, or 2 where it
 * is not told */
static int
tower_same_point(TowerCtx *ctx, const TowerPoint *a, const TowerPoint *b)
{
  const TowerPoint *pts[2] = {a, b};
  TowerX v[2][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, 2, &t, v))
    return 2;
  int k = t.depth;
  TowerX u[TOWER_SIZE], w[TOWER_SIZE];
  for (int j = 0; j < 2; j++)
  {
    tower_mul_k(ctx, &t, v[0][j], v[1][2], k, u);
    tower_mul_k(ctx, &t, v[1][j], v[0][2], k, w);
    tower_sub_k(ctx, u, w, k, u);
    int s = tower_sign_k(ctx, &t, u, k);
    if (tower_ctx_unsafe(ctx))
      return 2;
    if (s != 0)
      return 0;
  }
  return 1;
}

/* Where a point of the line of a segment stands along it: 0 at its start,
 * 1 at its end, -1 strictly between them, -2 beyond them, 2 where it is not
 * told */
static int
tower_on_segment(TowerCtx *ctx, const TowerPoint *p0, const TowerPoint *p1,
  const TowerPoint *x)
{
  int s0 = tower_dot(ctx, p0, x, p0, p1);
  int s1 = tower_dot(ctx, p1, x, p1, p0);
  if (s0 == 2 || s1 == 2)
    return 2;
  if (s0 == 0)
    return 0;
  if (s1 == 0)
    return 1;
  return (s0 > 0 && s1 > 0) ? -1 : -2;
}

typedef struct
{
  const Edge *p, *q;
  TowerMeet *out;
} TowersegMeetArg;

/* The meeting an end of a segment is: the point where its tangent touches
 * a circle for a segment between tangent points, the vertex its double
 * holds for a segment of the input */
static inline void
towerseg_end_meet(MeetPoint *m, const Edge *e, int end)
{
  if (e->from)
  {
    m->kind = MEET_TANGENT;
    m->tp = end ? e->to : e->from;
  }
  else
  {
    m->kind = MEET_VERTEX;
    m->x = end ? e->x2 : e->x1;
    m->y = end ? e->y2 : e->y1;
  }
  return;
}

/* Two segments between tangent points */
static int
towerseg_meet_segs(TowerCtx *ctx, const Edge *p, const Edge *q,
  TowerMeet *out)
{
  double ox = p->x1, oy = p->y1;
  TowerPoint pe[2], qe[2];
  towerseg_ends(ctx, p, ox, oy, &pe[0], &pe[1]);
  towerseg_ends(ctx, q, ox, oy, &qe[0], &qe[1]);
  const int o[4] = {tower_orient(ctx, &pe[0], &pe[1], &qe[0]),
    tower_orient(ctx, &pe[0], &pe[1], &qe[1]),
    tower_orient(ctx, &qe[0], &qe[1], &pe[0]),
    tower_orient(ctx, &qe[0], &qe[1], &pe[1])};
  for (int i = 0; i < 4; i++)
    if (o[i] == 2)
      return -2;
  if (o[0] * o[1] < 0 && o[2] * o[3] < 0)
  {
    /* A crossing interior to both: (P1 - P0) x (Q1 - Q0) is the orientation
     * of P0, P1, Q1 less that of P0, P1, Q0, of the sign of the former */
    memset(&out[0], 0, sizeof(TowerMeet));
    out[0].m.kind = MEET_LINES;
    out[0].m.p = p; out[0].m.q = q;
    out[0].end_p = out[0].end_q = -1;
    out[0].sign = o[1];
    return 1;
  }
  if (o[0] == 0 && o[1] == 0)
  {
    /* On one line: they run along each other unless at most an end of each
     * meets */
    int a = tower_on_segment(ctx, &pe[0], &pe[1], &qe[0]);
    int b = tower_on_segment(ctx, &pe[0], &pe[1], &qe[1]);
    int c = tower_on_segment(ctx, &qe[0], &qe[1], &pe[0]);
    int d = tower_on_segment(ctx, &qe[0], &qe[1], &pe[1]);
    if (a == 2 || b == 2 || c == 2 || d == 2)
      return -2;
    if (a == -1 || b == -1 || c == -1 || d == -1 ||
        (a >= 0 && b >= 0))
      return -1;
    int n = 0;
    for (int j = 0; j < 2; j++)
    {
      int at = j ? b : a;
      if (at < 0)
        continue;
      memset(&out[n], 0, sizeof(TowerMeet));
      towerseg_end_meet(&out[n].m, q, j);
      out[n].end_p = at;
      out[n].end_q = j;
      n++;
    }
    return n;
  }
  /* An end of one on the other */
  int n = 0;
  for (int j = 0; j < 4 && n < 2; j++)
  {
    if (o[j] != 0)
      continue;
    const TowerPoint *x = (j < 2) ? &qe[j] : &pe[j - 2];
    int at = (j < 2) ? tower_on_segment(ctx, &pe[0], &pe[1], x) :
      tower_on_segment(ctx, &qe[0], &qe[1], x);
    if (at == 2)
      return -2;
    if (at == -2)
      continue;
    /* An end of each at one point is found from both */
    bool seen = false;
    for (int i = 0; i < n; i++)
      seen = seen || ((j < 2) ? out[i].end_q == j :
        out[i].end_p == j - 2);
    if (seen)
      continue;
    memset(&out[n], 0, sizeof(TowerMeet));
    if (j < 2)
    {
      towerseg_end_meet(&out[n].m, q, j);
      out[n].end_q = j;
      out[n].end_p = at;
    }
    else
    {
      towerseg_end_meet(&out[n].m, p, j - 2);
      out[n].end_p = j - 2;
      out[n].end_q = at;
    }
    n++;
  }
  return n;
}

/* A segment between tangent points and the circle of a centred arc */
static int
towerseg_meet_circle(TowerCtx *ctx, const Edge *p, const Edge *q,
  TowerMeet *out)
{
  double ox = p->x1, oy = p->y1;
  TowerPoint pe[2], y[2];
  towerseg_ends(ctx, p, ox, oy, &pe[0], &pe[1]);
  int disc = 0;
  for (int r = 0; r < 2; r++)
  {
    disc = tower_point_line_circle(ctx, &pe[0], &pe[1], q->cx, q->cy,
      q->radius, r ? 1 : -1, ox, oy, &y[r]);
    if (disc == 2)
      return -2;
    if (disc < 0)
      return 0;
  }
  int n = 0;
  for (int r = 0; r < (disc == 0 ? 1 : 2); r++)
  {
    int at = tower_on_segment(ctx, &pe[0], &pe[1], &y[r]);
    if (at == 2)
      return -2;
    if (at == -2)
      continue;
    memset(&out[n], 0, sizeof(TowerMeet));
    out[n].end_q = -1;
    out[n].end_p = at;
    if (at >= 0)
      towerseg_end_meet(&out[n].m, p, at);
    else
    {
      out[n].m.kind = MEET_LINECIRCLE;
      out[n].m.p = p; out[n].m.q = q;
      out[n].m.sigma = r ? 1 : -1;
      /* The segment leaves the disc at its farther root: its direction
       * crossed with the tangent of a counterclockwise circle has the sign
       * of the root; a touching, the one root of the line, crosses nothing,
       * its sign 0 */
      out[n].sign = (disc == 0) ? 0 : (r ? 1 : -1) * (q->ccw ? 1 : -1);
    }
    n++;
  }
  return n;
}

static int
towerseg_meet_pred(TowerCtx *ctx, const void *arg)
{
  const TowersegMeetArg *a = arg;
  int n = a->q->centred ? towerseg_meet_circle(ctx, a->p, a->q, a->out) :
    towerseg_meet_segs(ctx, a->p, a->q, a->out);
  /* A count is 0, 1 or 2, so -2, not told, reads as 2 for the tiers */
  return (n == -2) ? 2 : ((n == 2) ? 3 : n);
}

/**
 * @brief Return how a segment between tangent points meets another segment
 * or the circle of a centred arc, decided exactly
 * @details Each meeting is an end of either segment, a crossing of the two
 * segments interior to both, or a meeting of the segment with the circle,
 * the root sigma along the segment, interior to it. The points lie in towers
 * of square roots (#TowerPoint): the two ends of a segment in that of the
 * root of their tangent, the crossings in that of both, the meetings with a
 * circle one root above
 * @param[in] p A segment between tangent points (#edges_add_tangent_segment)
 * @param[in] q Another such segment, or a centred arc (#edges_add_circle)
 * @param[out] out The meetings, in their order along @p p, a touching of the
 * circle interior to the segment of sign 0
 * @return Their number, -1 where the two segments run along each other, -2
 * where a meeting is not told
 */
int
towerseg_meet(const Edge *p, const Edge *q, TowerMeet out[2])
{
  assert(p); assert(q); assert(out);
  TowersegMeetArg arg = {p, q, out};
  int r = tower_pred_run(towerseg_meet_pred, &arg);
  return (r == 2) ? -2 : ((r == 3) ? 2 : r);
}

typedef struct
{
  const Edge *e;
  bool ccw;
  const MeetPoint *a, *b;
} TowerOrderArg;

static int
meets_order_tower_pred(TowerCtx *ctx, const void *arg)
{
  const TowerOrderArg *o = arg;
  const Edge *e = o->e;
  double ox = e->x1, oy = e->y1;
  TowerPoint pa, pb;
  int ra = meet_tower_build(ctx, o->a, ox, oy, &pa);
  int rb = meet_tower_build(ctx, o->b, ox, oy, &pb);
  if (ra != 1 || rb != 1)
    return 2;
  if (! e->centred)
  {
    TowerPoint e0, e1;
    towerseg_ends(ctx, e, ox, oy, &e0, &e1);
    int s = tower_dot(ctx, &pa, &pb, &e0, &e1);
    return (s == 2) ? 2 : -s;
  }
  /* Around the circle from the point furthest along -x: the half turn of
   * each, the side of the radius (-1, 0) its radius stands on, then the cross
   * product of the two radii, in the sense the circle turns */
  const TowerPoint *pts[2] = {&pa, &pb};
  TowerX v[2][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, 2, &t, v))
    return 2;
  int k = t.depth, turn = o->ccw ? 1 : -1, cls[2];
  TowerX rx[2][TOWER_SIZE], ry[2][TOWER_SIZE], u[TOWER_SIZE];
  for (int i = 0; i < 2; i++)
  {
    /* The radius times w: P - C w */
    tower_mulx_k(ctx, v[i][2], tx_diff(ctx, e->cx, ox), k, u);
    tower_sub_k(ctx, v[i][0], u, k, rx[i]);
    tower_mulx_k(ctx, v[i][2], tx_diff(ctx, e->cy, oy), k, u);
    tower_sub_k(ctx, v[i][1], u, k, ry[i]);
    int sw = tower_sign_k(ctx, &t, v[i][2], k);
    /* (-1, 0) x V = -Vy */
    int side = - tower_sign_k(ctx, &t, ry[i], k) * sw * turn;
    if (side == 0)
      cls[i] = (tower_sign_k(ctx, &t, rx[i], k) * sw < 0) ? 0 : 1;
    else
      cls[i] = (side > 0) ? 0 : 1;
  }
  if (tower_ctx_unsafe(ctx))
    return 2;
  if (cls[0] != cls[1])
    return (cls[0] < cls[1]) ? -1 : 1;
  TowerX a[TOWER_SIZE], b[TOWER_SIZE];
  tower_mul_k(ctx, &t, rx[0], ry[1], k, a);
  tower_mul_k(ctx, &t, ry[0], rx[1], k, b);
  tower_sub_k(ctx, a, b, k, a);
  int s = tower_sign_k(ctx, &t, a, k) * tower_sign_k(ctx, &t, v[0][2], k) *
    tower_sign_k(ctx, &t, v[1][2], k) * turn;
  if (tower_ctx_unsafe(ctx))
    return 2;
  return (s > 0) ? -1 : ((s < 0) ? 1 : 0);
}

/**
 * @brief Return the order of two meetings along a segment between tangent
 * points or around a centred arc, decided exactly
 * @details Along a segment by the projection of their difference on its
 * direction; around a centred arc from the point of its circle furthest
 * along -x, by the half turn each lies in and then by the cross product of
 * their radii, as #meets_order_exact orders meetings along an arc
 * @return -1 where @p a comes first, 1 where @p b does, 0 where they are one
 * point, 2 where it is not told
 */
int
meets_order_tower(const Edge *e, bool ccw, const MeetPoint *a,
  const MeetPoint *b)
{
  assert(e); assert(a); assert(b);
  TowerOrderArg arg = {e, ccw, a, b};
  return tower_pred_run(meets_order_tower_pred, &arg);
}

typedef struct
{
  const Edge *s;
  double px, py;
} TowerSideArg;

static int
towerseg_side_pred(TowerCtx *ctx, const void *arg)
{
  const TowerSideArg *o = arg;
  TowerPoint e0, e1, p;
  towerseg_ends(ctx, o->s, o->px, o->py, &e0, &e1);
  tower_point_xy(ctx, o->px, o->py, o->px, o->py, &p);
  return tower_orient(ctx, &e0, &e1, &p);
}

static inline bool towerseg_offset(const Edge *e, int *s);

/**
 * @brief Return the side of a segment between tangent points a point stands
 * on, decided exactly: 1 to its left, -1 to its right, 0 on its line
 * @details The offset by r to the side s of the segment from C1 to C2
 * between the centres of two circles of one radius (#towerseg_offset) is
 * the line d x (X - C1) = s r |d|, d = C2 - C1, so a point P stands on the
 * side of the sign of d x (P - C1) - s r |d| of the segment from its point
 * on the first circle, read off the doubles with a bound on its error; the
 * tower decides what the bound does not
 */
int
towerseg_side(const Edge *s, double px, double py)
{
  assert(s);
  int so;
  if (towerseg_offset(s, &so))
  {
    const TangentPoint *t = s->from;
    ArcsegFilter dx = arcseg_f_diff(t->c2x, t->c1x);
    ArcsegFilter dy = arcseg_f_diff(t->c2y, t->c1y);
    ArcsegFilter qx = arcseg_f_diff(px, t->c1x);
    ArcsegFilter qy = arcseg_f_diff(py, t->c1y);
    ArcsegFilter r = {t->r1, 0.0};
    ArcsegFilter len = tower_f_sqrt(arcseg_f_add(arcseg_f_mul(dx, dx),
      arcseg_f_mul(dy, dy)));
    ArcsegFilter v = arcseg_f_sub(arcseg_f_sub(arcseg_f_mul(dx, qy),
      arcseg_f_mul(dy, qx)), arcseg_f_scale(arcseg_f_mul(r, len),
      (double) so));
    int sg = arcseg_f_sign(v);
    if (sg != 2)
      return (t->k == 0) ? sg : - sg;
  }
  TowerSideArg arg = {s, px, py};
  return tower_pred_run(towerseg_side_pred, &arg);
}

typedef struct
{
  const MeetPoint *m;
  double py;
} TowerYArg;

static int
meet_point_y_side_pred(TowerCtx *ctx, const void *arg)
{
  const TowerYArg *o = arg;
  TowerPoint p;
  if (meet_tower_build(ctx, o->m, 0.0, o->py, &p) != 1)
    return 2;
  int s = tower_sign_k(ctx, &p.t, p.y.c, p.t.depth) *
    tower_sign_k(ctx, &p.t, p.w.c, p.t.depth);
  return tower_ctx_unsafe(ctx) ? 2 : s;
}

/**
 * @brief Return the side of the horizontal line y = @p py the point of a
 * meeting lies on, decided exactly: 1 above, -1 below, 0 on it, 2 where it is
 * not told
 */
int
meet_point_y_side(const MeetPoint *m, double py)
{
  assert(m);
  TowerYArg arg = {m, py};
  return tower_pred_run(meet_point_y_side_pred, &arg);
}

typedef struct
{
  const MeetPoint *m;
  double ox, d;
} TowerXSideArg;

static int
meet_point_x_side_pred(TowerCtx *ctx, const void *arg)
{
  const TowerXSideArg *o = arg;
  TowerPoint p;
  if (meet_tower_build(ctx, o->m, o->ox, 0.0, &p) != 1)
    return 2;
  /* (X - d w) w, X taken from ox */
  int k = p.t.depth;
  TowerX u[TOWER_SIZE];
  tower_mulx_k(ctx, p.w.c, tx_diff(ctx, o->d, 0.0), k, u);
  tower_sub_k(ctx, p.x.c, u, k, u);
  int s = tower_sign_k(ctx, &p.t, u, k) * tower_sign_k(ctx, &p.t, p.w.c, k);
  return tower_ctx_unsafe(ctx) ? 2 : s;
}

/**
 * @brief Return the side of the vertical line x = @p ox + @p d the point of
 * a meeting lies on, decided exactly: 1 right of it, -1 left, 0 on it, 2
 * where it is not told
 * @details As #meet_point_y_side reads a horizontal line; the line of the
 * points of circles of centre @p ox and radius @p d furthest along x
 */
int
meet_point_x_side(const MeetPoint *m, double ox, double d)
{
  assert(m);
  TowerXSideArg arg = {m, ox, d};
  return tower_pred_run(meet_point_x_side_pred, &arg);
}

typedef struct
{
  const MeetPoint *a, *b;
} TowerXArg;

static int
meets_x_cmp_pred(TowerCtx *ctx, const void *arg)
{
  const TowerXArg *o = arg;
  TowerPoint pa, pb;
  double ox = o->a->x, oy = o->a->y;
  if (meet_tower_build(ctx, o->a, ox, oy, &pa) != 1 ||
      meet_tower_build(ctx, o->b, ox, oy, &pb) != 1)
    return 2;
  /* Xa wb - Xb wa, read in the tower of both, as #meets_order_tower reads
   * the cross product of two radii */
  const TowerPoint *pts[2] = {&pa, &pb};
  TowerX v[2][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, 2, &t, v))
    return 2;
  int k = t.depth;
  TowerX u[TOWER_SIZE], w[TOWER_SIZE];
  tower_mul_k(ctx, &t, v[0][0], v[1][2], k, u);
  tower_mul_k(ctx, &t, v[1][0], v[0][2], k, w);
  tower_sub_k(ctx, u, w, k, u);
  int s = tower_sign_k(ctx, &t, u, k) * tower_sign_k(ctx, &t, v[0][2], k) *
    tower_sign_k(ctx, &t, v[1][2], k);
  return tower_ctx_unsafe(ctx) ? 2 : s;
}

/**
 * @brief Return how the x coordinates of the points of two meetings
 * compare, decided exactly: -1 where the first is smaller, 1 where it is
 * larger, 0 where they are equal, 2 where it is not told
 * @details The points lie in towers of square roots (#meet_point_y_side
 * reads one against a line)
 */
int
meets_x_cmp(const MeetPoint *a, const MeetPoint *b)
{
  assert(a); assert(b);
  TowerXArg arg = {a, b};
  return tower_pred_run(meets_x_cmp_pred, &arg);
}

/* A quotient of two filtered values, and a bound on its error, the divisor
 * kept away from zero by its bound; false where it is not */
static bool
tower_f_div(ArcsegFilter a, ArcsegFilter b, double *v, double *e)
{
  double m = fabs(b.v) - b.e;
  if (! (m > 0.0) || ! isfinite(a.v) || ! isfinite(b.v))
    return false;
  *v = a.v / b.v;
  *e = ((a.e + fabs(*v) * b.e) / m) * (1.0 + 4.0 * DBL_EPSILON) +
    2.0 * DBL_EPSILON * fabs(*v) + DBL_MIN;
  return isfinite(*v) && isfinite(*e);
}

typedef struct
{
  const MeetPoint *m;
  double *x, *y, *box;
} TowerBoundsArg;

/* A double as an unsigned integer whose order is that of the doubles */
static inline uint64_t
tower_double_key(double d)
{
  uint64_t u;
  memcpy(&u, &d, sizeof(u));
  return (u & UINT64CONST(0x8000000000000000)) ? ~u :
    (u | UINT64CONST(0x8000000000000000));
}

static inline double
tower_key_double(uint64_t key)
{
  uint64_t u = (key & UINT64CONST(0x8000000000000000)) ?
    (key & ~UINT64CONST(0x8000000000000000)) : ~key;
  double d;
  memcpy(&d, &u, sizeof(d));
  return d;
}

/* Narrow [lo, hi], two doubles holding org + num / den of a tower, to two
 * consecutive doubles, or to one where it is a double: bisected in the order
 * of the doubles, each step decided exactly by the sign of
 * num - (m - org) den. False where a sign is not told */
static bool
tower_bisect(TowerCtx *ctx, const Tower *t, const TowerX *num,
  const TowerX *den, int k, double org, double *lo, double *hi)
{
  int sd = tower_sign_k(ctx, t, den, k);
  if (sd == 0 || tower_ctx_unsafe(ctx))
    return false;
  uint64_t klo = tower_double_key(*lo), khi = tower_double_key(*hi);
  TowerX md[TOWER_SIZE];
  while (khi - klo > 1)
  {
    uint64_t kmid = klo + (khi - klo) / 2;
    double m = tower_key_double(kmid);
    tower_mulx_k(ctx, den, tx_diff(ctx, m, org), k, md);
    tower_sub_k(ctx, num, md, k, md);
    int s = tower_sign_k(ctx, t, md, k) * sd;
    if (tower_ctx_unsafe(ctx))
      return false;
    if (s == 0)
    {
      klo = khi = kmid;
      break;
    }
    if (s > 0)
      klo = kmid;
    else
      khi = kmid;
  }
  *lo = tower_key_double(klo);
  *hi = tower_key_double(khi);
  return true;
}

/* A double near the point of a meeting, which the point is read from so
 * that the bound on the error of its offset is small beside its coordinates:
 * the centre of a circle it lies on, or the start of a segment */
static void
meet_point_origin(const MeetPoint *m, double *ox, double *oy)
{
  switch (m->kind)
  {
    case MEET_TANGENT:
      *ox = m->tp->k ? m->tp->c2x : m->tp->c1x;
      *oy = m->tp->k ? m->tp->c2y : m->tp->c1y;
      return;
    case MEET_LINES:
    case MEET_LINECIRCLE:
      *ox = m->p->x1; *oy = m->p->y1;
      return;
    case MEET_ARCARC:
      *ox = m->p->cx; *oy = m->p->cy;
      return;
    default:
      *ox = m->x; *oy = m->y;
      return;
  }
}

static int
meet_point_bounds_pred(TowerCtx *ctx, const void *arg)
{
  const TowerBoundsArg *o = arg;
  TowerPoint p;
  double org[2];
  meet_point_origin(o->m, &org[0], &org[1]);
  if (! isfinite(org[0]) || ! isfinite(org[1]))
    org[0] = org[1] = 0.0;
  if (meet_tower_build(ctx, o->m, org[0], org[1], &p) != 1 ||
      tower_ctx_unsafe(ctx))
    return 2;
  int k = p.t.depth;
  ArcsegFilter fw = tower_filter_k(ctx, &p.t, p.w.c, k);
  const TowerX *num[2] = {p.x.c, p.y.c};
  double *val[2] = {o->x, o->y};
  for (int j = 0; j < 2; j++)
  {
    ArcsegFilter f = tower_filter_k(ctx, &p.t, num[j], k);
    double v, e;
    bool told = tower_f_div(f, fw, &v, &e);
    /* The coordinate is the origin plus the offset, the sum rounded once,
     * which adds half a unit in its last place to the bound */
    double c = 0.0, ce = 0.0;
    if (told)
    {
      c = org[j] + v;
      ce = e + DBL_EPSILON * fabs(c) + DBL_MIN;
      told = isfinite(c) && isfinite(ce);
    }
    /* A coordinate the filter holds to a few units in its last place is
     * written as it reads; any other is narrowed exactly, from the box of
     * the filter or from the whole range of the doubles where the filter
     * bounds nothing, as at the crossing of two nearly parallel segments */
    if (! told || ce > 8.0 * DBL_EPSILON * fabs(c) + DBL_MIN)
    {
      double lo = told ? c - ce : - DBL_MAX, hi = told ? c + ce : DBL_MAX;
      if (! tower_bisect(ctx, &p.t, num[j], p.w.c, k, org[j], &lo, &hi))
        return 2;
      c = lo;
      o->box[2 * j] = lo; o->box[2 * j + 1] = hi;
    }
    else
    {
      o->box[2 * j] = c - ce; o->box[2 * j + 1] = c + ce;
    }
    *val[j] = c;
  }
  return 1;
}

/* A point where the common tangent of two circles of one radius touches one
 * of them, Ck + side r rot(d) / |d| as #tangent_point_round reads it, and a
 * box holding it exactly: each operation rounds to nearest, so the offset is
 * off by less than 12 units of the machine epsilon of its size and the sum
 * by half a unit of its own, a bound taken four times over. False for two
 * radii, or where the squares leave the range the bound holds in, which
 * #meet_point_bounds_pred places in the tower instead */
static bool
tangent_point_bounds(const TangentPoint *tp, double *x, double *y,
  double box[4])
{
  if (tp->r1 != tp->r2)
    return false;
  double dx = tp->c2x - tp->c1x, dy = tp->c2y - tp->c1y;
  double d2 = dx * dx + dy * dy;
  if (! (d2 > 0x1p-900 && d2 < 0x1p900))
    return false;
  double f = tp->side * tp->r1 / sqrt(d2);
  double ox = - f * dy, oy = f * dx;
  double cx = tp->k ? tp->c2x : tp->c1x, cy = tp->k ? tp->c2y : tp->c1y;
  *x = cx + ox;
  *y = cy + oy;
  double ex = 48.0 * DBL_EPSILON * fabs(ox) + 2.0 * DBL_EPSILON * fabs(*x) +
    4.0 * DBL_MIN;
  double ey = 48.0 * DBL_EPSILON * fabs(oy) + 2.0 * DBL_EPSILON * fabs(*y) +
    4.0 * DBL_MIN;
  if (! isfinite(*x) || ! isfinite(*y) || ! isfinite(ex) || ! isfinite(ey))
    return false;
  box[0] = *x - ex; box[1] = *x + ex;
  box[2] = *y - ey; box[3] = *y + ey;
  return true;
}

/**
 * @brief Set a box holding exactly a point where a tangent touches one of
 * two circles of one radius, given the point as #tangent_point_bounds
 * rounds it, as the ends of a segment between tangent points carry it
 * (#edges_add_tangent_segment): the bound of #tangent_point_bounds with the
 * offset, whose length the rounding keeps below twice the radius, taken at
 * twice the radius
 * @return False where #tangent_point_bounds does not read the point off the
 * doubles
 */
bool
tangent_point_box(const TangentPoint *tp, double x, double y, double box[4])
{
  assert(tp); assert(box);
  if (tp->r1 != tp->r2)
    return false;
  double dx = tp->c2x - tp->c1x, dy = tp->c2y - tp->c1y;
  double d2 = dx * dx + dy * dy;
  if (! (d2 > 0x1p-900 && d2 < 0x1p900))
    return false;
  /* Far within the range of the doubles, where the point of
   * #tangent_point_bounds is finite and is the one given */
  if (! (fabs(x) < 0x1p1000 && fabs(y) < 0x1p1000))
    return false;
  double o = 96.0 * DBL_EPSILON * fabs(tp->r1);
  double ex = o + 2.0 * DBL_EPSILON * fabs(x) + 4.0 * DBL_MIN;
  double ey = o + 2.0 * DBL_EPSILON * fabs(y) + 4.0 * DBL_MIN;
  if (! isfinite(x) || ! isfinite(y) || ! isfinite(ex) || ! isfinite(ey))
    return false;
  box[0] = x - ex; box[1] = x + ex;
  box[2] = y - ey; box[3] = y + ey;
  return true;
}

/* Whether a segment between tangent points lies on the offset of the
 * segment between the centres of one pair of circles of one radius, the
 * common tangent of both its ends, the side @p s of C1 -> C2 */
static inline bool
towerseg_offset(const Edge *e, int *s)
{
  const TangentPoint *a = e->from, *b = e->to;
  if (! a || ! b || a->r1 != a->r2 || a->side != b->side || a->k == b->k ||
      a->c1x != b->c1x || a->c1y != b->c1y || a->c2x != b->c2x ||
      a->c2y != b->c2y || a->r1 != b->r1)
    return false;
  *s = a->side;
  return true;
}

/* The coordinates of a point taken from an origin, each an offset read
 * with a bound on its error, written where the bound holds the coordinate
 * to a few units in its last place, as #meet_point_bounds_pred writes them */
static bool
filter_point_bounds(double ox, double oy, ArcsegFilter fx, ArcsegFilter fy,
  double *x, double *y, double box[4])
{
  const double org[2] = {ox, oy};
  const ArcsegFilter off[2] = {fx, fy};
  double *val[2] = {x, y};
  for (int j = 0; j < 2; j++)
  {
    double c = org[j] + off[j].v;
    double ce = off[j].e + DBL_EPSILON * fabs(c) + DBL_MIN;
    if (! isfinite(c) || ! isfinite(off[j].e) ||
        ! (ce <= 8.0 * DBL_EPSILON * fabs(c) + DBL_MIN))
      return false;
    box[2 * j] = c - ce; box[2 * j + 1] = c + ce;
    *val[j] = c;
  }
  return true;
}

/* A quotient of filtered values as a filtered value, false where the
 * divisor's bound holds zero (#tower_f_div) */
static inline bool
filter_div(ArcsegFilter a, ArcsegFilter b, ArcsegFilter *r)
{
  return tower_f_div(a, b, &r->v, &r->e);
}

/* Whether a difference of the doubles placing an offset is zero or within
 * 2^-300 and 2^300 of its size, where no product of up to three of them
 * leaves the range of normal doubles */
static inline bool
offset_term_in_range(double d)
{
  double a = fabs(d);
  return a == 0.0 || (a >= 0x1p-300 && a <= 0x1p300);
}

/* One coordinate o + num / den of a point placed in plain doubles, and the
 * interval holding it exactly, from the bounds ne of the numerator and de
 * of the denominator, as #tower_f_div bounds the quotient and
 * #filter_point_bounds the point; false where the denominator's bound
 * holds zero or the interval is wider than a few units in the last place */
static inline bool
plain_quotient_point(double o, double num, double ne, double den, double de,
  double *val, double *lo, double *hi)
{
  double m = fabs(den) - de;
  if (! (m > 0.0))
    return false;
  double v = num / den;
  double e = ((ne + fabs(v) * de) / m) * (1.0 + 4.0 * DBL_EPSILON) +
    2.0 * DBL_EPSILON * fabs(v) + DBL_MIN;
  double c = o + v;
  double ce = e + DBL_EPSILON * fabs(c) + DBL_MIN;
  if (! isfinite(c) || ! isfinite(e) ||
      ! (ce <= 8.0 * DBL_EPSILON * fabs(c) + DBL_MIN))
    return false;
  *lo = c - ce; *hi = c + ce; *val = c;
  return true;
}

/* The crossing of two offsets sharing a centre V of #offset_lines_bounds,
 * the same values in plain doubles, each computed as its filter computes
 * it, and a bound on each coordinate that bounds the one the filters carry
 * from their rules (#arcseg_f_mul, #arcseg_f_add, #tower_f_sqrt): with
 * every difference zero or within 2^-300 and 2^300 of its size, no product
 * falls below the normal doubles but r times a numerator, and to first
 * order in eps the length lp is off by 4 eps lp, the numerator by
 * 8 eps S with S = lq |dpy| + lp |dqy| (lq |dpx| + lp |dqx| for y), its
 * product with r by 9 eps |r| S plus 4 DBL_MIN, the denominator by
 * 17 eps lp lq, as |dp . dq| <= lp lq; 5, 11 and 18 cover the second order.
 * The quotient and the point take them as #tower_f_div and
 * #filter_point_bounds take theirs. False where the bound is wider than
 * theirs allows, which the filters then place */
static bool
offset_lines_shared_bounds(const TangentPoint *p, const TangentPoint *q,
  int sp, int sq, double vx, double vy, double *x, double *y, double box[4])
{
  double dpx = p->c2x - p->c1x, dpy = p->c2y - p->c1y;
  double dqx = q->c2x - q->c1x, dqy = q->c2y - q->c1y;
  double r = p->r1;
  if (! offset_term_in_range(dpx) || ! offset_term_in_range(dpy) ||
      ! offset_term_in_range(dqx) || ! offset_term_in_range(dqy) ||
      r == 0.0 || ! offset_term_in_range(r))
    return false;
  double lp = sqrt(dpx * dpx + dpy * dpy), lq = sqrt(dqx * dqx + dqy * dqy);
  if (lp == 0.0 || lq == 0.0)
    return false;
  /* With the unit normals s rot(d) / |d| multiplied through by |dp| |dq|:
   * r (sp |dq| rot(dp) + sq |dp| rot(dq)) / (|dp| |dq| + sp sq dp . dq) */
  double wp = lq * (double) sp, wq = lp * (double) sq;
  const double num[2] = {r * ((wp * dpy + wq * dqy) * -1.0),
    r * (wp * dpx + wq * dqx)};
  const double sum[2] = {lq * fabs(dpy) + lp * fabs(dqy),
    lq * fabs(dpx) + lp * fabs(dqx)};
  double den = lp * lq + (dpx * dqx + dpy * dqy) * (double) (sp * sq);
  double de = 18.0 * DBL_EPSILON * (lp * lq);
  return plain_quotient_point(vx, num[0],
      11.0 * DBL_EPSILON * fabs(r) * sum[0] + 4.0 * DBL_MIN, den, de, x,
      &box[0], &box[1]) &&
    plain_quotient_point(vy, num[1],
      11.0 * DBL_EPSILON * fabs(r) * sum[1] + 4.0 * DBL_MIN, den, de, y,
      &box[2], &box[3]);
}

/* The crossing of two offsets sharing no centre of #offset_lines_bounds,
 * O + (hp dq - hq dp) / (dp x dq), the same values in plain doubles, each
 * computed as its filter computes it, and a bound on each coordinate that
 * bounds the one the filters carry from their rules: with every difference
 * and both distances zero or within 2^-300 and 2^300 of their size, no
 * product of up to three of them falls below the normal doubles, and to
 * first order in eps, with P = |dpx dqy| + |dpy dqx| and
 * C = |dqx ay| + |dqy ax|, the denominator is off by 4 eps P, hp by
 * 6 eps |rp| lp, hq by 5 eps C + 7 eps |rq| lq, and the numerator of x by
 * 10 eps |rp| lp |dqx| + |dpx| (9 eps C + 11 eps |rq| lq), that of y alike;
 * 5, 11, 10 and 12 cover the second order, 8 DBL_MIN a product of a sum
 * that cancels to below the normal doubles. False where the bound is wider
 * than the filters allow, which the filters then place */
static bool
offset_lines_cross_bounds(const TangentPoint *p, const TangentPoint *q,
  int sp, int sq, double *x, double *y, double box[4])
{
  double dpx = p->c2x - p->c1x, dpy = p->c2y - p->c1y;
  double dqx = q->c2x - q->c1x, dqy = q->c2y - q->c1y;
  double ox = p->c1x, oy = p->c1y;
  double ax = q->c1x - ox, ay = q->c1y - oy;
  double rp = p->r1, rq = q->r1;
  if (! offset_term_in_range(dpx) || ! offset_term_in_range(dpy) ||
      ! offset_term_in_range(dqx) || ! offset_term_in_range(dqy) ||
      ! offset_term_in_range(ax) || ! offset_term_in_range(ay) ||
      rp == 0.0 || ! offset_term_in_range(rp) || rq == 0.0 ||
      ! offset_term_in_range(rq))
    return false;
  double lp = sqrt(dpx * dpx + dpy * dpy), lq = sqrt(dqx * dqx + dqy * dqy);
  double hp = (rp * lp) * (double) sp;
  double hq = (dqx * ay - dqy * ax) + (rq * lq) * (double) sq;
  double den = dpx * dqy - dpy * dqx;
  double de = 5.0 * DBL_EPSILON * (fabs(dpx * dqy) + fabs(dpy * dqx));
  double hpl = fabs(rp) * lp, c = fabs(dqx * ay) + fabs(dqy * ax);
  double hql = 10.0 * c + 12.0 * fabs(rq) * lq;
  double nex = DBL_EPSILON * (11.0 * hpl * fabs(dqx) + fabs(dpx) * hql) +
    8.0 * DBL_MIN;
  double ney = DBL_EPSILON * (11.0 * hpl * fabs(dqy) + fabs(dpy) * hql) +
    8.0 * DBL_MIN;
  return plain_quotient_point(ox, hp * dqx - hq * dpx, nex, den, de, x,
      &box[0], &box[1]) &&
    plain_quotient_point(oy, hp * dqy - hq * dpy, ney, den, de, y,
      &box[2], &box[3]);
}

/* The crossing of two segments between tangent points that are offsets of
 * the segments between the centres of pairs of circles of one radius
 * (#towerseg_offset), and a box holding it exactly, read off the doubles
 * with a bound on their error as #meet_point_bounds_pred reads a tower. Two
 * offsets of segments sharing a centre V are tangent to the circle about V
 * on the sides of their unit normals m1 and m2 from it, so they cross at
 * V + r (m1 + m2) / (1 + m1 . m2), which no cancellation loses however
 * little the segments turn; any two others at O + (hp dq - hq dp) /
 * (dp x dq), the offset of C1 -> C2 on the side s at the distance r being
 * the line d x (X - C1) = s r |d|, d = C2 - C1, and hp and hq its right
 * sides taken from the first centre O of the first. False where a bound is
 * wider than a few units in the last place, as where the lines are nearly
 * parallel, which the tower places instead */
static bool
offset_lines_bounds(const MeetPoint *m, double *x, double *y, double box[4])
{
  int sp, sq;
  if (! m->p || ! m->q || ! towerseg_offset(m->p, &sp) ||
      ! towerseg_offset(m->q, &sq))
    return false;
  const TangentPoint *p = m->p->from, *q = m->q->from;
  /* A centre both segments share, and one radius: in plain doubles first
   * (#offset_lines_shared_bounds) */
  bool vp = (p->c1x == q->c1x && p->c1y == q->c1y) ||
    (p->c1x == q->c2x && p->c1y == q->c2y);
  bool vq = (p->c2x == q->c1x && p->c2y == q->c1y) ||
    (p->c2x == q->c2x && p->c2y == q->c2y);
  bool shared = p->r1 == q->r1 && (vp || vq);
  double vx = vp ? p->c1x : p->c2x, vy = vp ? p->c1y : p->c2y;
  if (shared ? offset_lines_shared_bounds(p, q, sp, sq, vx, vy, x, y, box) :
      offset_lines_cross_bounds(p, q, sp, sq, x, y, box))
    return true;
  ArcsegFilter dpx = arcseg_f_diff(p->c2x, p->c1x);
  ArcsegFilter dpy = arcseg_f_diff(p->c2y, p->c1y);
  ArcsegFilter dqx = arcseg_f_diff(q->c2x, q->c1x);
  ArcsegFilter dqy = arcseg_f_diff(q->c2y, q->c1y);
  ArcsegFilter rp = {p->r1, 0.0}, rq = {q->r1, 0.0};
  ArcsegFilter lp = tower_f_sqrt(arcseg_f_add(arcseg_f_mul(dpx, dpx),
    arcseg_f_mul(dpy, dpy)));
  ArcsegFilter lq = tower_f_sqrt(arcseg_f_add(arcseg_f_mul(dqx, dqx),
    arcseg_f_mul(dqy, dqy)));
  if (shared)
  {
    /* With the unit normals s rot(d) / |d| multiplied through by |dp| |dq|:
     * r (sp |dq| rot(dp) + sq |dp| rot(dq)) / (|dp| |dq| + sp sq dp . dq) */
    ArcsegFilter wp = arcseg_f_scale(lq, (double) sp);
    ArcsegFilter wq = arcseg_f_scale(lp, (double) sq);
    ArcsegFilter nx = arcseg_f_scale(arcseg_f_add(arcseg_f_mul(wp, dpy),
      arcseg_f_mul(wq, dqy)), -1.0);
    ArcsegFilter ny = arcseg_f_add(arcseg_f_mul(wp, dpx),
      arcseg_f_mul(wq, dqx));
    ArcsegFilter den = arcseg_f_add(arcseg_f_mul(lp, lq),
      arcseg_f_scale(arcseg_f_add(arcseg_f_mul(dpx, dqx),
        arcseg_f_mul(dpy, dqy)), (double) (sp * sq)));
    ArcsegFilter fx, fy;
    if (! filter_div(arcseg_f_mul(rp, nx), den, &fx) ||
        ! filter_div(arcseg_f_mul(rp, ny), den, &fy))
      return false;
    return filter_point_bounds(vx, vy, fx, fy, x, y, box);
  }
  double ox = p->c1x, oy = p->c1y;
  ArcsegFilter ax = arcseg_f_diff(q->c1x, ox), ay = arcseg_f_diff(q->c1y, oy);
  ArcsegFilter hp = arcseg_f_scale(arcseg_f_mul(rp, lp), (double) sp);
  ArcsegFilter hq = arcseg_f_add(arcseg_f_sub(arcseg_f_mul(dqx, ay),
    arcseg_f_mul(dqy, ax)), arcseg_f_scale(arcseg_f_mul(rq, lq),
    (double) sq));
  ArcsegFilter den = arcseg_f_sub(arcseg_f_mul(dpx, dqy),
    arcseg_f_mul(dpy, dqx));
  ArcsegFilter fx, fy;
  if (! filter_div(arcseg_f_sub(arcseg_f_mul(hp, dqx),
        arcseg_f_mul(hq, dpx)), den, &fx) ||
      ! filter_div(arcseg_f_sub(arcseg_f_mul(hp, dqy),
        arcseg_f_mul(hq, dpy)), den, &fy))
    return false;
  return filter_point_bounds(ox, oy, fx, fy, x, y, box);
}

/* The meeting of #offset_line_circle_bounds, the same values in plain
 * doubles, each computed as its filter computes it, and a bound on each
 * coordinate that bounds the one the filters carry from their rules: with
 * every difference and both radii zero or within 2^-300 and 2^300 of their
 * size, to first order in eps, with Cc = |dx ay| + |dy ax|, the length l is
 * off by 4 eps l, the cross product by 4 eps Cc, the quotient q by
 * eps (4 Cc / l + 7 |q|) and delta by that and eps |delta|; h = R^2 -
 * delta^2 carries its rules' bound he as they compute it, its root sqrt(h)
 * moves by at most he / sqrt(h), and the numerator of x by
 * |dy| (2 eps |delta| + delta's) + |dx| (2 eps sqrt(h) + the root's) plus
 * eps of its size, that of y alike; 5, 8, 4 and the factor 1 + 16 eps
 * cover the second order, 8 DBL_MIN a product below the normal doubles.
 * False where the bound is wider than the filters allow, which the filters
 * then place */
static bool
offset_line_circle_plain(const MeetPoint *m, int s, double *x, double *y,
  double box[4])
{
  const TangentPoint *t = m->p->from;
  double u = (m->p->to->k == 1) ? 1.0 : -1.0;
  double dx = t->c2x - t->c1x, dy = t->c2y - t->c1y;
  double ax = m->q->cx - t->c1x, ay = m->q->cy - t->c1y;
  double r = t->r1, rr = m->q->radius;
  if (! offset_term_in_range(dx) || ! offset_term_in_range(dy) ||
      ! offset_term_in_range(ax) || ! offset_term_in_range(ay) ||
      r == 0.0 || ! offset_term_in_range(r) || rr == 0.0 ||
      ! offset_term_in_range(rr))
    return false;
  double l = sqrt(dx * dx + dy * dy);
  if (l == 0.0)
    return false;
  double le = 5.0 * DBL_EPSILON * l;
  double cc = fabs(dx * ay) + fabs(dy * ax);
  double q = (dx * ay - dy * ax) / l;
  double qe = DBL_EPSILON * (5.0 * (cc / l) + 8.0 * fabs(q)) + DBL_MIN;
  double delta = r * (double) s - q;
  double dle = qe * (1.0 + 16.0 * DBL_EPSILON) + DBL_EPSILON * fabs(delta);
  double h = rr * rr - delta * delta;
  double he = (DBL_EPSILON * (rr * rr + delta * delta + fabs(h)) +
    2.0 * fabs(delta) * dle + dle * dle) * (1.0 + 16.0 * DBL_EPSILON) +
    8.0 * DBL_MIN;
  if (! (h - he > 0.0))
    return false;
  double sr = sqrt(h);
  double se = (he / sr) * (1.0 + 4.0 * DBL_EPSILON) + 2.0 * DBL_EPSILON * sr;
  double sg = sr * ((double) m->sigma * u);
  double nx = (delta * dy) * -1.0 + sg * dx, ny = delta * dx + sg * dy;
  double ad = fabs(delta);
  double nex = (fabs(dy) * (2.0 * DBL_EPSILON * ad + dle) +
    fabs(dx) * (2.0 * DBL_EPSILON * sr + se) + DBL_EPSILON * fabs(nx)) *
    (1.0 + 16.0 * DBL_EPSILON) + 8.0 * DBL_MIN;
  double ney = (fabs(dx) * (2.0 * DBL_EPSILON * ad + dle) +
    fabs(dy) * (2.0 * DBL_EPSILON * sr + se) + DBL_EPSILON * fabs(ny)) *
    (1.0 + 16.0 * DBL_EPSILON) + 8.0 * DBL_MIN;
  return plain_quotient_point(m->q->cx, nx, nex, l, le, x, &box[0],
      &box[1]) &&
    plain_quotient_point(m->q->cy, ny, ney, l, le, y, &box[2], &box[3]);
}

/* The meeting of an offset of the segment between the centres of a pair of
 * circles of one radius (#towerseg_offset) with the circle of a centred
 * arc, the root sigma along the segment as #tower_point_line_circle places
 * it, read off the doubles with a bound on their error. The offset on the
 * side s of d = C2 - C1 at the distance r is the line rot(d) . (X - C1) =
 * s r |d|, so from the centre C of the circle of radius R the meeting is
 * (delta rot(d) + sigma u sqrt(R^2 - delta^2) d) / |d| with
 * delta = s r - d x (C - C1) / |d| and u the sense the segment runs along
 * d. False where a bound is wide, as where the line nearly touches the
 * circle, which the tower places instead */
static bool
offset_line_circle_bounds(const MeetPoint *m, double *x, double *y,
  double box[4])
{
  int s;
  if (! m->p || ! m->q || ! m->q->centred || ! towerseg_offset(m->p, &s))
    return false;
  if (offset_line_circle_plain(m, s, x, y, box))
    return true;
  const TangentPoint *t = m->p->from;
  double u = (m->p->to->k == 1) ? 1.0 : -1.0;
  ArcsegFilter dx = arcseg_f_diff(t->c2x, t->c1x);
  ArcsegFilter dy = arcseg_f_diff(t->c2y, t->c1y);
  ArcsegFilter ax = arcseg_f_diff(m->q->cx, t->c1x);
  ArcsegFilter ay = arcseg_f_diff(m->q->cy, t->c1y);
  ArcsegFilter r = {t->r1, 0.0}, rr = {m->q->radius, 0.0};
  ArcsegFilter l = tower_f_sqrt(arcseg_f_add(arcseg_f_mul(dx, dx),
    arcseg_f_mul(dy, dy)));
  ArcsegFilter cr = arcseg_f_sub(arcseg_f_mul(dx, ay), arcseg_f_mul(dy, ax));
  ArcsegFilter q, delta, h, fx, fy;
  if (! filter_div(cr, l, &q))
    return false;
  delta = arcseg_f_sub(arcseg_f_scale(r, (double) s), q);
  h = arcseg_f_sub(arcseg_f_mul(rr, rr), arcseg_f_mul(delta, delta));
  if (arcseg_f_sign(h) != 1)
    return false;
  ArcsegFilter sq = arcseg_f_scale(tower_f_sqrt(h), (double) m->sigma * u);
  if (! filter_div(arcseg_f_add(arcseg_f_scale(arcseg_f_mul(delta, dy),
        -1.0), arcseg_f_mul(sq, dx)), l, &fx) ||
      ! filter_div(arcseg_f_add(arcseg_f_mul(delta, dx),
        arcseg_f_mul(sq, dy)), l, &fy))
    return false;
  return filter_point_bounds(m->q->cx, m->q->cy, fx, fy, x, y, box);
}

/* The crossing of #equal_circles_bounds, the same values in plain doubles,
 * each computed as its filter computes it, and a bound on each coordinate
 * that bounds the one the filters carry from their rules: with both
 * differences and the radius zero or within 2^-300 and 2^300 of their
 * size, to first order in eps, D is off by 4 eps D, h = 4 r^2 - D by
 * eps (4 r^2 + |h|) and D's, its root sqrt(h) by at most that over
 * sqrt(h) and 2 eps sqrt(h), the root of D by 4.5 eps sqrt(D), and the
 * coordinate offset d / 2 - f rot(d) by eps |d| / 2, 2 eps |f| |d| and |d|
 * times the bound of f, plus eps of its size; 5, 5 and the factor
 * 1 + 16 eps cover the second order, 8 DBL_MIN a product below the normal
 * doubles. The quotient and the point take them as #tower_f_div and
 * #filter_point_bounds take theirs. False where the bound is wider than
 * the filters allow, which the filters then place */
static bool
equal_circles_plain(const MeetPoint *m, double *x, double *y, double box[4])
{
  double dx = m->q->cx - m->p->cx, dy = m->q->cy - m->p->cy;
  double r = m->p->radius;
  if (! offset_term_in_range(dx) || ! offset_term_in_range(dy) ||
      r == 0.0 || ! offset_term_in_range(r))
    return false;
  double dd = dx * dx + dy * dy;
  if (! (dd > 0.0))
    return false;
  double dde = 5.0 * DBL_EPSILON * dd;
  double r4 = (r * r) * 4.0;
  double h = r4 - dd;
  double he = (DBL_EPSILON * (r4 + fabs(h)) + dde) *
    (1.0 + 16.0 * DBL_EPSILON) + 8.0 * DBL_MIN;
  if (! (h - he > 0.0))
    return false;
  double sh = sqrt(h), sd = sqrt(dd);
  ArcsegFilter num = {sh,
    (he / sh) * (1.0 + 4.0 * DBL_EPSILON) + 2.0 * DBL_EPSILON * sh};
  ArcsegFilter den = {sd * 2.0, 10.0 * DBL_EPSILON * sd};
  ArcsegFilter f, fx, fy;
  if (! tower_f_div(num, den, &f.v, &f.e))
    return false;
  f.v = f.v * (double) m->sigma;
  double af = fabs(f.v), ax = fabs(dx), ay = fabs(dy);
  fx.v = dx * 0.5 - f.v * dy;
  fy.v = dy * 0.5 + f.v * dx;
  fx.e = (DBL_EPSILON * (0.5 * ax + 2.0 * af * ay + fabs(fx.v)) +
    ay * f.e) * (1.0 + 16.0 * DBL_EPSILON) + 8.0 * DBL_MIN;
  fy.e = (DBL_EPSILON * (0.5 * ay + 2.0 * af * ax + fabs(fy.v)) +
    ax * f.e) * (1.0 + 16.0 * DBL_EPSILON) + 8.0 * DBL_MIN;
  return filter_point_bounds(m->p->cx, m->p->cy, fx, fy, x, y, box);
}

/* The crossing sigma of the circles of two centred arcs of one radius, as
 * #tower_point_circles places it, read off the doubles with a bound on
 * their error: with d = C2 - C1 and D = |d|^2 it is
 * C1 + d / 2 + sigma sqrt(4 r^2 - D) rot(d) / (2 sqrt(D)), in plain
 * doubles first (#equal_circles_plain), kept out of #meet_point_bounds,
 * which places the far more frequent meetings. False where a bound is wide,
 * as where the circles nearly touch */
static pg_noinline bool
equal_circles_bounds(const MeetPoint *m, double *x, double *y, double box[4])
{
  if (! m->p || ! m->q || ! m->p->centred || ! m->q->centred ||
      m->p->radius != m->q->radius)
    return false;
  if (equal_circles_plain(m, x, y, box))
    return true;
  ArcsegFilter dx = arcseg_f_diff(m->q->cx, m->p->cx);
  ArcsegFilter dy = arcseg_f_diff(m->q->cy, m->p->cy);
  ArcsegFilter r = {m->p->radius, 0.0};
  ArcsegFilter dd = arcseg_f_add(arcseg_f_mul(dx, dx), arcseg_f_mul(dy, dy));
  ArcsegFilter h = arcseg_f_sub(arcseg_f_scale(arcseg_f_mul(r, r), 4.0), dd);
  if (arcseg_f_sign(h) != 1 || arcseg_f_sign(dd) != 1)
    return false;
  ArcsegFilter f;
  if (! filter_div(tower_f_sqrt(h), arcseg_f_scale(tower_f_sqrt(dd), 2.0),
      &f))
    return false;
  f = arcseg_f_scale(f, (double) m->sigma);
  ArcsegFilter fx = arcseg_f_sub(arcseg_f_scale(dx, 0.5), arcseg_f_mul(f, dy));
  ArcsegFilter fy = arcseg_f_add(arcseg_f_scale(dy, 0.5), arcseg_f_mul(f, dx));
  return filter_point_bounds(m->p->cx, m->p->cy, fx, fy, x, y, box);
}

/**
 * @brief Set the point of a meeting rounded once and a box holding it
 * exactly; false where it cannot be placed
 * @details A point where a tangent touches one of two circles of one radius
 * is read off the doubles with a bound on their error
 * (#tangent_point_bounds), and so, where the bound is tight, are the
 * crossing of two offsets of segments between the centres of such circles
 * (#offset_lines_bounds), the meeting of such an offset with a circle
 * (#offset_line_circle_bounds) and the crossing of two circles of one
 * radius (#equal_circles_bounds); any other is placed in its tower
 */
bool
meet_point_bounds(const MeetPoint *m, double *x, double *y, double box[4])
{
  assert(m); assert(x); assert(y); assert(box);
  if (m->kind == MEET_TANGENT && tangent_point_bounds(m->tp, x, y, box))
    return true;
  if (m->kind == MEET_LINES && offset_lines_bounds(m, x, y, box))
    return true;
  if (m->kind == MEET_LINECIRCLE && offset_line_circle_bounds(m, x, y, box))
    return true;
  if (m->kind == MEET_ARCARC && equal_circles_bounds(m, x, y, box))
    return true;
  TowerBoundsArg arg = {m, x, y, box};
  return tower_pred_run(meet_point_bounds_pred, &arg) == 1;
}

typedef struct
{
  const MeetPoint *a, *b;
} TowerSameArg;

static int
meets_same_point_pred(TowerCtx *ctx, const void *arg)
{
  const TowerSameArg *o = arg;
  TowerPoint pa, pb;
  if (meet_tower_build(ctx, o->a, 0.0, 0.0, &pa) != 1 ||
      meet_tower_build(ctx, o->b, 0.0, 0.0, &pb) != 1)
    return 2;
  return tower_same_point(ctx, &pa, &pb);
}

/**
 * @brief Return whether two meetings are one point, decided exactly: 1 or
 * 0, 2 where it is not told
 */
int
meets_same_point(const MeetPoint *a, const MeetPoint *b)
{
  assert(a); assert(b);
  TowerSameArg arg = {a, b};
  return tower_pred_run(meets_same_point_pred, &arg);
}

/* The direction a centred arc leaves a point of it in, as a vector of the
 * tower of the point times a positive factor: its radius turned a quarter in
 * the sense it turns, or the positive x axis for no edge; negated where the
 * arc arrives at the point. A segment's own is read off its ends
 * (#edges_turn_cmp_meet) */
static void
tower_edge_direction(TowerCtx *ctx, const Edge *e, bool ccw, bool out,
  const Tower *t, const TowerX (*pc)[TOWER_SIZE], double ox, double oy,
  TowerX *dx, TowerX *dy)
{
  int k = t->depth;
  TowerX u[TOWER_SIZE];
  if (! e)
  {
    tower_set(ctx, tx_val(ctx, 1.0), k, dx);
    tower_set(ctx, tx_val(ctx, 0.0), k, dy);
    return;
  }
  {
    /* (P - C w) times the sign of w */
    TowerX vx[TOWER_SIZE], vy[TOWER_SIZE];
    tower_mulx_k(ctx, pc[2], tx_diff(ctx, e->cx, ox), k, u);
    tower_sub_k(ctx, pc[0], u, k, vx);
    tower_mulx_k(ctx, pc[2], tx_diff(ctx, e->cy, oy), k, u);
    tower_sub_k(ctx, pc[1], u, k, vy);
    double sw = (double) tower_sign_k(ctx, t, pc[2], k);
    /* Counterclockwise the radius (x, y) turns to (-y, x) */
    tower_scale_k(ctx, vy, ccw ? - sw : sw, k, dx);
    tower_scale_k(ctx, vx, ccw ? sw : - sw, k, dy);
  }
  if (! out)
  {
    tower_scale_k(ctx, dx, -1.0, k, dx);
    tower_scale_k(ctx, dy, -1.0, k, dy);
  }
  return;
}

typedef struct
{
  const Edge *e1, *e2;
  bool ccw1, out1, ccw2, out2;
  const MeetPoint *at;
} TowerTurnArg;

static int
edges_turn_cmp_meet_pred(TowerCtx *ctx, const void *arg)
{
  const TowerTurnArg *o = arg;
  double ox = o->e1 ? o->e1->x1 : (o->e2 ? o->e2->x1 : 0.0);
  double oy = o->e1 ? o->e1->y1 : (o->e2 ? o->e2->y1 : 0.0);
  TowerPoint p, s0[2], s1[2];
  if (meet_tower_build(ctx, o->at, ox, oy, &p) != 1)
    return 2;
  /* The point and the ends of the segments, in one tower */
  const Edge *es[2] = {o->e1, o->e2};
  const TowerPoint *pts[5];
  int npts = 0, segat[2] = {-1, -1};
  pts[npts++] = &p;
  for (int i = 0; i < 2; i++)
    if (es[i] && ! es[i]->centred)
    {
      towerseg_ends(ctx, es[i], ox, oy, &s0[i], &s1[i]);
      segat[i] = npts;
      pts[npts++] = &s0[i];
      pts[npts++] = &s1[i];
    }
  TowerX v[5][3][TOWER_SIZE];
  Tower t;
  if (! tower_points_read(ctx, pts, npts, &t, v))
    return 2;
  int k = t.depth;
  TowerX dx[2][TOWER_SIZE], dy[2][TOWER_SIZE];
  bool ccw[2] = {o->ccw1, o->ccw2}, out[2] = {o->out1, o->out2};
  for (int i = 0; i < 2; i++)
  {
    if (segat[i] >= 0)
    {
      /* (E1 w0 - E0 w1) times the sign of w0 w1 */
      TowerX a[TOWER_SIZE], b[TOWER_SIZE];
      const TowerX (*e0)[TOWER_SIZE] = v[segat[i]];
      const TowerX (*e1)[TOWER_SIZE] = v[segat[i] + 1];
      double sw = (double) (tower_sign_k(ctx, &t, e0[2], k) *
        tower_sign_k(ctx, &t, e1[2], k));
      for (int j = 0; j < 2; j++)
      {
        tower_mul_k(ctx, &t, e1[j], e0[2], k, a);
        tower_mul_k(ctx, &t, e0[j], e1[2], k, b);
        tower_sub_k(ctx, a, b, k, a);
        tower_scale_k(ctx, a, out[i] ? sw : - sw, k, j ? dy[i] : dx[i]);
      }
    }
    else
      tower_edge_direction(ctx, es[i], ccw[i], out[i], &t,
        (const TowerX (*)[TOWER_SIZE]) v[0], ox, oy, dx[i], dy[i]);
  }
  int sx[2], sy[2], h[2];
  for (int i = 0; i < 2; i++)
  {
    sx[i] = tower_sign_k(ctx, &t, dx[i], k);
    sy[i] = tower_sign_k(ctx, &t, dy[i], k);
    h[i] = edge_direction_half_plane(sx[i], sy[i]);
  }
  if (tower_ctx_unsafe(ctx) || h[0] == 2 || h[1] == 2)
    return 2;
  if (h[0] != h[1])
    return h[0] < h[1] ? -1 : 1;
  TowerX a[TOWER_SIZE], b[TOWER_SIZE];
  tower_mul_k(ctx, &t, dx[0], dy[1], k, a);
  tower_mul_k(ctx, &t, dy[0], dx[1], k, b);
  tower_sub_k(ctx, a, b, k, a);
  int s = tower_sign_k(ctx, &t, a, k);
  if (tower_ctx_unsafe(ctx))
    return 2;
  if (s != 0)
    return s > 0 ? -1 : 1;
  if (o->e1 && o->e2)
  {
    bool arc1 = o->e1->centred, arc2 = o->e2->centred;
    if (arc1 != arc2)
    {
      /* The arc bends left of the direction where it leaves the point
       * counterclockwise or arrives at it clockwise (#edges_turn_cmp_exact) */
      bool left = arc1 ? (o->ccw1 == o->out1) : (o->ccw2 == o->out2);
      return (left == arc1) ? 1 : -1;
    }
    /* Both arcs, or neither */
    if (arc1)
    {
      /* Two circles touching at the point: the one bending further left,
       * the larger of the curvatures 1/r signed by the side each bends
       * to, comes after, as an arc after a segment above */
      bool left1 = (o->ccw1 == o->out1), left2 = (o->ccw2 == o->out2);
      if (left1 != left2)
        return left1 ? 1 : -1;
      double r1 = o->e1->radius, r2 = o->e2->radius;
      if (r1 == r2)
        return 2;
      return ((r1 < r2) == left1) ? 1 : -1;
    }
  }
  return 2;
}

/**
 * @brief Return how two edges leaving a meeting are ordered
 * counterclockwise from the positive x axis, decided exactly: the
 * #edges_turn_cmp_exact of a point no double holds
 * @param[in] e1,e2 Segments between tangent points or centred arcs holding
 * the point; NULL for the positive x axis @nullable
 * @param[in] ccw1,ccw2 Whether each arc turns counterclockwise
 * @param[in] out1,out2 Whether each edge leaves the point
 * @param[in] at The point
 * @return -1 where the first comes before the second, 1 where after, 2
 * where they leave alike or it is not told
 */
int
edges_turn_cmp_meet(const Edge *e1, bool ccw1, bool out1, const Edge *e2,
  bool ccw2, bool out2, const MeetPoint *at)
{
  assert(at);
  TowerTurnArg arg = {e1, e2, ccw1, out1, ccw2, out2, at};
  return tower_pred_run(edges_turn_cmp_meet_pred, &arg);
}

/**
 * @brief Add to an edge array the segment between two points where tangents
 * touch circles, which no double holds (#TangentPoint): the edge of the
 * convex hull of two discs along their common tangent, or the chord between
 * the points where the two tangents touch one circle
 * @details Its ends are those points rounded once, and its box holds the
 * exact segment, so that a box filter only admits candidates; every exact
 * question about it reads the points (#towerseg_meet)
 * @param[in,out] edges Edge array
 * @param[in] from,to The ends, of one root sqrt(E) (#tower_point_tangent);
 * they outlive the edge array
 */
void
edges_add_tangent_segment(MeosArray *edges, const TangentPoint *from,
  const TangentPoint *to)
{
  assert(edges); assert(from); assert(to);
  /* The two ends of a tangent of one pair of circles of one radius are
   * its centres plus the one offset f rot(d), f = side r / |d|
   * (#tangent_point_bounds): read once, the same doubles, the edge written
   * in its place in the array */
  if (from->r1 == from->r2 && to->r1 == to->r2 && from->r1 == to->r1 &&
      from->side == to->side && from->k != to->k &&
      from->c1x == to->c1x && from->c1y == to->c1y &&
      from->c2x == to->c2x && from->c2y == to->c2y)
  {
    double dx = from->c2x - from->c1x, dy = from->c2y - from->c1y;
    double d2 = dx * dx + dy * dy;
    if (d2 > 0x1p-900 && d2 < 0x1p900)
    {
      double f = from->side * from->r1 / sqrt(d2);
      double ox = - f * dy, oy = f * dx;
      double eo = 48.0 * DBL_EPSILON;
      double fx = from->k ? from->c2x : from->c1x;
      double fy = from->k ? from->c2y : from->c1y;
      double tx = to->k ? to->c2x : to->c1x, ty = to->k ? to->c2y : to->c1y;
      double x1 = fx + ox, y1 = fy + oy, x2 = tx + ox, y2 = ty + oy;
      double eox = eo * fabs(ox), eoy = eo * fabs(oy);
      double ex1 = eox + 2.0 * DBL_EPSILON * fabs(x1) + 4.0 * DBL_MIN;
      double ey1 = eoy + 2.0 * DBL_EPSILON * fabs(y1) + 4.0 * DBL_MIN;
      double ex2 = eox + 2.0 * DBL_EPSILON * fabs(x2) + 4.0 * DBL_MIN;
      double ey2 = eoy + 2.0 * DBL_EPSILON * fabs(y2) + 4.0 * DBL_MIN;
      if (isfinite(x1) && isfinite(y1) && isfinite(x2) && isfinite(y2) &&
          isfinite(ex1) && isfinite(ey1) && isfinite(ex2) && isfinite(ey2))
      {
        if (edges->count >= edges->capacity)
          edge_array_reserve(edges, 1);
        Edge *e = &((Edge *) edges->elems)[edges->count];
        memset(e, 0, sizeof(Edge));
        e->etype = EDGE_POLYSEG;
        e->from = from; e->to = to;
        e->x1 = x1; e->y1 = y1; e->x2 = x2; e->y2 = y2;
        e->xmin = Min(x1 - ex1, x2 - ex2); e->xmax = Max(x1 + ex1, x2 + ex2);
        e->ymin = Min(y1 - ey1, y2 - ey2); e->ymax = Max(y1 + ey1, y2 + ey2);
        e->dx = x2 - x1; e->dy = y2 - y1;
        e->length = e->dx * e->dx + e->dy * e->dy;
        edges->count++;
        return;
      }
    }
  }
  Edge e;
  memset(&e, 0, sizeof(Edge));
  e.etype = EDGE_POLYSEG;
  e.from = from; e.to = to;
  double box0[4], box1[4];
  MeetPoint m0, m1;
  memset(&m0, 0, sizeof(MeetPoint));
  memset(&m1, 0, sizeof(MeetPoint));
  m0.kind = m1.kind = MEET_TANGENT;
  m0.tp = from; m1.tp = to;
  /* Each end bounded where the doubles bound it, else rounded alone */
  if (! meet_point_bounds(&m0, &e.x1, &e.y1, box0))
  {
    tangent_point_round(from, &e.x1, &e.y1);
    box0[0] = box0[1] = e.x1; box0[2] = box0[3] = e.y1;
  }
  if (! meet_point_bounds(&m1, &e.x2, &e.y2, box1))
  {
    tangent_point_round(to, &e.x2, &e.y2);
    box1[0] = box1[1] = e.x2; box1[2] = box1[3] = e.y2;
  }
  e.xmin = Min(box0[0], box1[0]); e.xmax = Max(box0[1], box1[1]);
  e.ymin = Min(box0[2], box1[2]); e.ymax = Max(box0[3], box1[3]);
  e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
  e.length = e.dx * e.dx + e.dy * e.dy;
  edge_array_add(edges, &e);
  return;
}

/**
 * @brief Add to an edge array the arc of the circle of a centre and a radius
 * between two points where tangents touch that circle, which no double holds
 * (#TangentPoint): the round join of a buffer about a vertex, between the
 * offsets of the two edges meeting there
 * @details The circle is the one the three doubles fix exactly, carried as
 * #edges_add_circle carries it, and the arc is its part from @p from to
 * @p to in the sense it turns; both points lie on it as the points where
 * their tangents touch it, the circle they are built on. Every exact
 * question about the arc reads the circle and the two points
 * (#arrange_wound_tangent_surfaces), so the arc built about a vertex is
 * that vertex's circle exactly, whatever the coordinates of its ends round to
 * @param[in,out] edges Edge array
 * @param[in] cx,cy Centre
 * @param[in] radius Radius, positive
 * @param[in] ccw Whether the arc turns counterclockwise
 * @param[in] from,to The ends, points of the circle; they outlive the edge
 * array
 */
void
edges_add_tangent_arc(MeosArray *edges, double cx, double cy, double radius,
  bool ccw, const TangentPoint *from, const TangentPoint *to)
{
  assert(edges); assert(radius > 0.0); assert(from); assert(to);
  edges_add_circle(edges, cx, cy, radius, ccw);
  Edge *e = &((Edge *) edges->elems)[edges->count - 1];
  e->from = from; e->to = to;
  return;
}

/* The point of a meeting, exactly: a vertex; two segments crossing at
 * A + t R, t = ((C - A) x S) / (R x S); a segment crossing an arc at the
 * root t = (-qb + sigma sqrt(qb^2 - 4 qa qc)) / (2 qa) of the quadratic of
 * #arcsegm_meet_exact; two arcs at P = B + sigma f rot(d) of
 * #arcarc_meet_exact, which with its terms is
 * (M1 W + 2 Q L N1 + D1 (G d' + sigma sqrt(H) rot(d'))) / W, W = 2 Q L D1;
 * or two arcs through a point J at its reflection J - 2 (d' x g) rot(d') /
 * (D1 Q), g = D1 (J - M1) - N1, of #arcarc_shared_point_signs. Every point
 * is taken from the origin O, so that no term carries the magnitude of the
 * coordinates themselves, only that of their differences */
static void
meet_point_exact(ArcsegArena *arena, const MeetPoint *m, double ox,
  double oy, MeetExact *r)
{
  ArcsegExact zero = meet_val(arena, 0.0), one = meet_val(arena, 1.0);
  r->x1 = r->y1 = r->h = zero;
  r->w = one;
  switch (m->kind)
  {
    case MEET_VERTEX:
      r->x0 = arcseg_x_diff(arena, m->x, ox);
      r->y0 = arcseg_x_diff(arena, m->y, oy);
      return;
    case MEET_SEGSEG:
    {
      const Edge *p = m->p, *q = m->q;
      ArcsegExact rx = arcseg_x_diff(arena, p->x2, p->x1);
      ArcsegExact ry = arcseg_x_diff(arena, p->y2, p->y1);
      ArcsegExact sx = arcseg_x_diff(arena, q->x2, q->x1);
      ArcsegExact sy = arcseg_x_diff(arena, q->y2, q->y1);
      ArcsegExact cx = arcseg_x_diff(arena, q->x1, p->x1);
      ArcsegExact cy = arcseg_x_diff(arena, q->y1, p->y1);
      ArcsegExact den = arcseg_x_sub(arena, arcseg_x_mul(arena, rx, sy),
        arcseg_x_mul(arena, ry, sx));
      ArcsegExact num = arcseg_x_sub(arena, arcseg_x_mul(arena, cx, sy),
        arcseg_x_mul(arena, cy, sx));
      r->w = den;
      r->x0 = arcseg_x_add(arena, arcseg_x_mul(arena, arcseg_x_diff(arena,
        p->x1, ox),
        den), arcseg_x_mul(arena, rx, num));
      r->y0 = arcseg_x_add(arena, arcseg_x_mul(arena, arcseg_x_diff(arena,
        p->y1, oy),
        den), arcseg_x_mul(arena, ry, num));
      return;
    }
    case MEET_SEGARC:
    {
      const Edge *p = m->p;
      ArcsegExact qa, qb, qc, h1, h0, side;
      arcseg_polynomials_exact(arena, p->x1, p->y1, p->x2, p->y2, m->q, &qa,
        &qb, &qc, &h1, &h0, &side);
      ArcsegExact rx = arcseg_x_diff(arena, p->x2, p->x1);
      ArcsegExact ry = arcseg_x_diff(arena, p->y2, p->y1);
      r->w = arcseg_x_scale(arena, qa, 2.0);
      r->h = arcseg_x_sub(arena, arcseg_x_mul(arena, qb, qb),
        arcseg_x_scale(arena, arcseg_x_mul(arena, qa, qc), 4.0));
      r->x0 = arcseg_x_sub(arena, arcseg_x_mul(arena, r->w,
        arcseg_x_diff(arena, p->x1, ox)), arcseg_x_mul(arena, qb, rx));
      r->y0 = arcseg_x_sub(arena, arcseg_x_mul(arena, r->w,
        arcseg_x_diff(arena, p->y1, oy)), arcseg_x_mul(arena, qb, ry));
      r->x1 = arcseg_x_scale(arena, rx, (double) m->sigma);
      r->y1 = arcseg_x_scale(arena, ry, (double) m->sigma);
      return;
    }
    default:
    {
      ArcsegExact m1x, m1y, n1x, n1y, d1, m2x, m2y, n2x, n2y, d2;
      arcarc_centre_exact(arena, m->p, &m1x, &m1y, &n1x, &n1y, &d1);
      arcarc_centre_exact(arena, m->q, &m2x, &m2y, &n2x, &n2y, &d2);
      ArcsegExact l = arcseg_x_mul(arena, d1, d2);
      ArcsegExact dx = arcseg_x_add(arena,
        arcseg_x_mul(arena, l, arcseg_x_sub(arena, m2x, m1x)),
        arcseg_x_sub(arena, arcseg_x_mul(arena, d1, n2x),
          arcseg_x_mul(arena, d2, n1x)));
      ArcsegExact dy = arcseg_x_add(arena,
        arcseg_x_mul(arena, l, arcseg_x_sub(arena, m2y, m1y)),
        arcseg_x_sub(arena, arcseg_x_mul(arena, d1, n2y),
          arcseg_x_mul(arena, d2, n1y)));
      ArcsegExact qq = arcseg_x_add(arena, arcseg_x_mul(arena, dx, dx),
        arcseg_x_mul(arena, dy, dy));
      if (m->kind == MEET_ARCARC_REFLECT)
      {
        ArcsegExact jx = meet_val(arena, m->x), jy = meet_val(arena, m->y);
        ArcsegExact gx = arcseg_x_sub(arena, arcseg_x_mul(arena, d1,
          arcseg_x_sub(arena, jx, m1x)), n1x);
        ArcsegExact gy = arcseg_x_sub(arena, arcseg_x_mul(arena, d1,
          arcseg_x_sub(arena, jy, m1y)), n1y);
        ArcsegExact k = arcseg_x_scale(arena, arcseg_x_sub(arena,
          arcseg_x_mul(arena, dx, gy), arcseg_x_mul(arena, dy, gx)), 2.0);
        r->w = arcseg_x_mul(arena, d1, qq);
        /* J W - k rot(d'), rot(d') = (-dy, dx) */
        r->x0 = arcseg_x_add(arena, arcseg_x_mul(arena,
          arcseg_x_diff(arena, m->x, ox), r->w),
          arcseg_x_mul(arena, k, dy));
        r->y0 = arcseg_x_sub(arena, arcseg_x_mul(arena,
          arcseg_x_diff(arena, m->y, oy), r->w),
          arcseg_x_mul(arena, k, dx));
        return;
      }
      ArcsegExact r1 = arcseg_x_mul(arena, arcarc_radius2_exact(arena, m->p,
        m1x, m1y, n1x, n1y, d1), arcseg_x_mul(arena, d2, d2));
      ArcsegExact r2 = arcseg_x_mul(arena, arcarc_radius2_exact(arena, m->q,
        m2x, m2y, n2x, n2y, d2), arcseg_x_mul(arena, d1, d1));
      ArcsegExact g = arcseg_x_sub(arena, arcseg_x_add(arena, qq, r1), r2);
      r->h = arcseg_x_sub(arena,
        arcseg_x_scale(arena, arcseg_x_mul(arena, r1, qq), 4.0),
        arcseg_x_mul(arena, g, g));
      ArcsegExact ql2 = arcseg_x_scale(arena, arcseg_x_mul(arena, qq, l),
        2.0);
      r->w = arcseg_x_mul(arena, ql2, d1);
      r->x0 = arcseg_x_add(arena, arcseg_x_add(arena,
        arcseg_x_mul(arena, arcseg_x_sub(arena, m1x, meet_val(arena, ox)),
        r->w), arcseg_x_mul(arena, ql2, n1x)),
        arcseg_x_mul(arena, arcseg_x_mul(arena, g, d1), dx));
      r->y0 = arcseg_x_add(arena, arcseg_x_add(arena,
        arcseg_x_mul(arena, arcseg_x_sub(arena, m1y, meet_val(arena, oy)),
        r->w), arcseg_x_mul(arena, ql2, n1y)),
        arcseg_x_mul(arena, arcseg_x_mul(arena, g, d1), dy));
      ArcsegExact sd1 = arcseg_x_scale(arena, d1, (double) m->sigma);
      r->x1 = arcseg_x_scale(arena, arcseg_x_mul(arena, sd1, dy), -1.0);
      r->y1 = arcseg_x_mul(arena, sd1, dx);
      return;
    }
  }
}

/**
 * @brief Return the order of two meetings along an edge holding both,
 * decided exactly
 * @details Each meeting is a point (X0 + X1 sqrt(H), Y0 + Y1 sqrt(H)) / W,
 * exact (#MeetPoint). Along a segment A -> B two points are ordered by
 * (P2 - P1) . (B - A); along an arc of centre C = M + N / D from its start S
 * by the half turn each lies in, the side of S - C the radius P - C stands
 * on in the sense the arc turns, then by the cross product of their radii.
 * Every one of these is the sign of X + Y sqrt(H1) + (Z + T sqrt(H1))
 * sqrt(H2) with exact terms, which two squarings decide
 * @param[in] e Edge holding both meetings, strictly within it
 * @param[in] ccw Whether an arc turns counterclockwise
 * @param[in] a,b The meetings
 * @return -1 where @p a comes first along the edge, 1 where @p b does, 0
 * where they are one point, 2 where a product of the coordinates would leave
 * the range of doubles, scaled as they are by a power of two
 */
/* Multiply a coordinate by the two factors f of a power of two of
 * #point_distance_scale; false where the product is not exact, which its
 * inverse g tells */
static inline bool
coord_scale(double *v, double f1, double f2, double g1, double g2)
{
  double w = *v * f1 * f2;
  bool exact = isfinite(w) && w * g1 * g2 == *v;
  *v = w;
  return exact;
}

/* An edge with every input coordinate scaled by #coord_scale; false where
 * one is not scaled exactly */
static bool
edge_scale(const Edge *e, double f1, double f2, double g1, double g2,
  Edge *out)
{
  *out = *e;
  double *v[] = {&out->x1, &out->y1, &out->x2, &out->y2, &out->sx, &out->sy,
    &out->mx, &out->my, &out->ex, &out->ey};
  /* A segment carries only its ends */
  size_t n = (e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC) ?
    sizeof(v) / sizeof(v[0]) : 4;
  bool exact = true;
  for (size_t i = 0; i < n; i++)
    exact = coord_scale(v[i], f1, f2, g1, g2) && exact;
  /* A centred arc carries its circle as its centre and radius */
  if (e->centred)
  {
    exact = coord_scale(&out->cx, f1, f2, g1, g2) && exact;
    exact = coord_scale(&out->cy, f1, f2, g1, g2) && exact;
    exact = coord_scale(&out->radius, f1, f2, g1, g2) && exact;
  }
  return exact;
}

/* Widen a box by the points of an edge, as #edge_scale reads them: the box
 * of its circle for a centred arc */
static void
edge_extent(const Edge *e, double box[4])
{
  if (e->centred)
  {
    box[0] = Min(box[0], e->cx - e->radius);
    box[1] = Max(box[1], e->cx + e->radius);
    box[2] = Min(box[2], e->cy - e->radius);
    box[3] = Max(box[3], e->cy + e->radius);
    return;
  }
  double x[5] = {e->x1, e->x2, e->sx, e->mx, e->ex};
  double y[5] = {e->y1, e->y2, e->sy, e->my, e->ey};
  int n = (e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC) ? 5 : 2;
  for (int i = 0; i < n; i++)
  {
    box[0] = Min(box[0], x[i]); box[1] = Max(box[1], x[i]);
    box[2] = Min(box[2], y[i]); box[3] = Max(box[3], y[i]);
  }
  return;
}

static int meets_order_scaled(const Edge *e, bool ccw, const MeetPoint *a,
  const MeetPoint *b);

int
meets_order_exact(const Edge *e, bool ccw, const MeetPoint *a,
  const MeetPoint *b)
{
  /* Meetings no double holds, and the segments between them, are ordered
   * in their towers of square roots (#meets_order_tower) */
  if (e->from || a->kind >= MEET_TANGENT || b->kind >= MEET_TANGENT)
    return meets_order_tower(e, ccw, a, b);
  /* The terms are products of many differences of coordinates, exact while
   * none leaves the range of doubles: the edges are read scaled by the power
   * of two that brings their extent into [1, 2), which changes no sign */
  double box[4] = {INFINITY, -INFINITY, INFINITY, -INFINITY};
  edge_extent(e, box);
  const MeetPoint *m[2] = {a, b};
  for (int i = 0; i < 2; i++)
  {
    if (m[i]->p)
      edge_extent(m[i]->p, box);
    if (m[i]->q)
      edge_extent(m[i]->q, box);
  }
  double extent = Max(box[1] - box[0], box[3] - box[2]);
  if (! (extent > 0.0) || ! isfinite(extent))
    return 2;
  int k;
  double f1, f2, g1, g2;
  point_distance_scale(extent, &k, &f1, &f2, &g1, &g2);
  Edge se, sp[2], sq[2];
  MeetPoint sm[2];
  bool exact = edge_scale(e, f1, f2, g1, g2, &se);
  for (int i = 0; i < 2; i++)
  {
    sm[i] = *m[i];
    exact = coord_scale(&sm[i].x, f1, f2, g1, g2) && exact;
    exact = coord_scale(&sm[i].y, f1, f2, g1, g2) && exact;
    if (m[i]->p)
    {
      exact = edge_scale(m[i]->p, f1, f2, g1, g2, &sp[i]) && exact;
      sm[i].p = &sp[i];
    }
    if (m[i]->q)
    {
      exact = edge_scale(m[i]->q, f1, f2, g1, g2, &sq[i]) && exact;
      sm[i].q = &sq[i];
    }
  }
  /* A coordinate the scaling does not keep, a subnormal one, is not read */
  if (! exact)
    return 2;
  return meets_order_scaled(&se, ccw, &sm[0], &sm[1]);
}

/* The order of #meets_order_exact, on edges of an extent near one */
static int
meets_order_scaled(const Edge *e, bool ccw, const MeetPoint *a,
  const MeetPoint *b)
{
  ArcsegArena arena_, *arena = &arena_;
  arena_.used = arena_.n = arena_.cap = 0;
  arena_.unsafe = false;
  arena_.blocks = NULL;
  MeetExact pa, pb;
  double ox = e->x1, oy = e->y1;
  meet_point_exact(arena, a, ox, oy, &pa);
  meet_point_exact(arena, b, ox, oy, &pb);
  int swa = arcseg_x_sign_checked(arena, pa.w);
  int swb = arcseg_x_sign_checked(arena, pb.w);
  int result;
  if (e->etype != EDGE_POLYARC && e->etype != EDGE_LINEARC)
  {
    /* (Wa Xb - Wb Xa) . R over Wa Wb */
    ArcsegExact rx = arcseg_x_diff(arena, e->x2, e->x1);
    ArcsegExact ry = arcseg_x_diff(arena, e->y2, e->y1);
#define MEET_DOT(ux, uy) arcseg_x_add(arena, arcseg_x_mul(arena, ux, rx), \
  arcseg_x_mul(arena, uy, ry))
    ArcsegExact x = arcseg_x_sub(arena,
      arcseg_x_mul(arena, pa.w, MEET_DOT(pb.x0, pb.y0)),
      arcseg_x_mul(arena, pb.w, MEET_DOT(pa.x0, pa.y0)));
    ArcsegExact y = arcseg_x_scale(arena, arcseg_x_mul(arena, pb.w,
      MEET_DOT(pa.x1, pa.y1)), -1.0);
    ArcsegExact z = arcseg_x_mul(arena, pa.w, MEET_DOT(pb.x1, pb.y1));
#undef MEET_DOT
    int s = meet_two_roots_sign(arena, x, y, pa.h, z, meet_val(arena, 0.0),
      pb.h) * swa * swb;
    result = (s > 0) ? -1 : ((s < 0) ? 1 : 0);
  }
  else
  {
    ArcsegExact mx, my, nx, ny, d;
    arcarc_centre_exact(arena, e, &mx, &my, &nx, &ny, &d);
    /* (P - C) W D = D (X0 + X1 sqrt(H)) - W (M D + N), and (S - C) D, all
     * from the origin */
    ArcsegExact cx = arcseg_x_add(arena, arcseg_x_mul(arena,
      arcseg_x_sub(arena, mx, meet_val(arena, ox)), d), nx);
    ArcsegExact cy = arcseg_x_add(arena, arcseg_x_mul(arena,
      arcseg_x_sub(arena, my, meet_val(arena, oy)), d), ny);
    /* A centred arc starts at the point of its circle furthest along -x,
     * the radius (-r, 0) */
    ArcsegExact ux = e->centred ? meet_val(arena, - e->radius) :
      arcseg_x_sub(arena, arcseg_x_mul(arena, arcseg_x_diff(arena, e->x1,
        ox), d), cx);
    ArcsegExact uy = e->centred ? meet_val(arena, 0.0) :
      arcseg_x_sub(arena, arcseg_x_mul(arena, arcseg_x_diff(arena, e->y1,
        oy), d), cy);
    const MeetExact *pts[2] = {&pa, &pb};
    ArcsegExact vx0[2], vy0[2], vx1[2], vy1[2];
    int cls[2], turn = ccw ? 1 : -1;
    for (int k = 0; k < 2; k++)
    {
      const MeetExact *pk = pts[k];
      vx0[k] = arcseg_x_sub(arena, arcseg_x_mul(arena, d, pk->x0),
        arcseg_x_mul(arena, pk->w, cx));
      vy0[k] = arcseg_x_sub(arena, arcseg_x_mul(arena, d, pk->y0),
        arcseg_x_mul(arena, pk->w, cy));
      vx1[k] = arcseg_x_mul(arena, d, pk->x1);
      vy1[k] = arcseg_x_mul(arena, d, pk->y1);
      /* The side of S - C: its cross product with P - C, over W D^2 */
      int side = meet_root_sign(arena, arcseg_x_sub(arena,
        arcseg_x_mul(arena, ux, vy0[k]), arcseg_x_mul(arena, uy, vx0[k])),
        arcseg_x_sub(arena, arcseg_x_mul(arena, ux, vy1[k]),
        arcseg_x_mul(arena, uy, vx1[k])), pk->h) *
        arcseg_x_sign_checked(arena, pk->w) * turn;
      if (side == 0)
      {
        /* On the line of S and C: at S, or half a turn on */
        int dot = meet_root_sign(arena, arcseg_x_add(arena,
          arcseg_x_mul(arena, ux, vx0[k]), arcseg_x_mul(arena, uy, vy0[k])),
          arcseg_x_add(arena, arcseg_x_mul(arena, ux, vx1[k]),
          arcseg_x_mul(arena, uy, vy1[k])), pk->h) *
          arcseg_x_sign_checked(arena, pk->w);
        cls[k] = (dot > 0) ? 0 : 1;
      }
      else
        cls[k] = (side > 0) ? 0 : 1;
    }
    if (cls[0] != cls[1])
      result = (cls[0] < cls[1]) ? -1 : 1;
    else
    {
      /* Va x Vb = (Xa + Ya rHa) x (Xb + Yb rHb) over Wa Wb D^2 */
#define MEET_CROSS(ax, ay, bx, by) arcseg_x_sub(arena, \
  arcseg_x_mul(arena, ax, by), arcseg_x_mul(arena, ay, bx))
      ArcsegExact x = MEET_CROSS(vx0[0], vy0[0], vx0[1], vy0[1]);
      ArcsegExact y = MEET_CROSS(vx1[0], vy1[0], vx0[1], vy0[1]);
      ArcsegExact z = MEET_CROSS(vx0[0], vy0[0], vx1[1], vy1[1]);
      ArcsegExact t = MEET_CROSS(vx1[0], vy1[0], vx1[1], vy1[1]);
#undef MEET_CROSS
      int s = meet_two_roots_sign(arena, x, y, pa.h, z, t, pb.h) * swa *
        swb * turn;
      result = (s > 0) ? -1 : ((s < 0) ? 1 : 0);
    }
  }
  if (arena->unsafe)
    result = 2;
  arcseg_arena_free(arena);
  return result;
}

/*****************************************************************************
 * Functions computing the intersection of two segments derived from PostGIS
 * The seg2d_intersection function is a modified version of the PostGIS
 * lw_segment_intersects function and also returns the intersection point
 * in case the two segments intersect at equal endpoints.
 * The intersection point is required in #pointarr_find_splits only for this
 * intersection type (MEOS_SEG_TOUCH_END).
 *****************************************************************************/

/*
 * The possible ways a pair of segments can interact.
 * Returned by the function seg2d_intersection
 */
enum
{
  MEOS_SEG_NO_INTERSECTION,  /* Segments do not intersect */
  MEOS_SEG_OVERLAP,          /* Segments overlap */
  MEOS_SEG_CROSS,            /* Segments cross */
  MEOS_SEG_TOUCH_END,        /* Segments touch in two equal enpoints */
  MEOS_SEG_TOUCH,            /* Segments touch without equal enpoints */
} MEOS_SEG_INTER_TYPE;

/**
 * @brief Find the *unique* intersection point @p p between two closed
 * collinear segments @p ab and @p cd
 * @details Return @p p and a @p MEOS_SEG_INTER_TYPE value.
 * @note If the segments overlap no point is returned since they
 * can be an infinite number of them.
 * @pre This function is called after verifying that the points are
 * collinear and that their bounding boxes intersect.
 */
static int
parseg2d_intersection(const POINT2D *a, const POINT2D *b, const POINT2D *c,
  const POINT2D *d, POINT2D *p)
{
  /* Compute the intersection of the bounding boxes */
  double xmin = Max(Min(a->x, b->x), Min(c->x, d->x));
  double xmax = Min(Max(a->x, b->x), Max(c->x, d->x));
  double ymin = Max(Min(a->y, b->y), Min(c->y, d->y));
  double ymax = Min(Max(a->y, b->y), Max(c->y, d->y));
  /* If the intersection of the bounding boxes is not a point */
  if (xmin < xmax || ymin < ymax )
    return MEOS_SEG_OVERLAP;
  /* We are sure that the segments touch each other */
  if ((b->x == c->x && b->y == c->y) ||
      (b->x == d->x && b->y == d->y))
  {
    p->x = b->x;
    p->y = b->y;
    return MEOS_SEG_TOUCH_END;
  }
  if ((a->x == c->x && a->y == c->y) ||
      (a->x == d->x && a->y == d->y))
  {
    p->x = a->x;
    p->y = a->y;
    return MEOS_SEG_TOUCH_END;
  }
  /* We should never arrive here since this function is called after verifying
   * that the bounding boxes of the segments intersect */
  return MEOS_SEG_NO_INTERSECTION;
}

/**
 * @brief Determines the side of segment P where Q lies
 * @details
 * - Return -1  if point Q is left of segment P
 * - Return  1  if point Q is right of segment P
 * - Return  0  if point Q in on segment P
 * @note Function adapted from @p lw_segment_side() to take into account
 * precision errors
 */
static int
seg2d_side(const POINT2D *p1, const POINT2D *p2, const POINT2D *q)
{
  double side = ( (q->x - p1->x) * (p2->y - p1->y) -
    (p2->x - p1->x) * (q->y - p1->y) );
  if (fabs(side) < MEOS_EPSILON)
    return 0;
  else
    return SIGNUM(side);
}

/**
 * @brief Function derived from file @p lwalgorithm.c since it is declared
 * static
 */
static bool
lw_seg_interact(const POINT2D *p1, const POINT2D *p2, const POINT2D *q1,
  const POINT2D *q2)
{
  double minq = Min(q1->x, q2->x);
  double maxq = Max(q1->x, q2->x);
  double minp = Min(p1->x, p2->x);
  double maxp = Max(p1->x, p2->x);

  if (FP_GT(minp, maxq) || FP_LT(maxp, minq))
    return false;

  minq = Min(q1->y, q2->y);
  maxq = Max(q1->y, q2->y);
  minp = Min(p1->y, p2->y);
  maxp = Max(p1->y, p2->y);

  if (FP_GT(minp,maxq) || FP_LT(maxp,minq))
    return false;

  return true;
}

/**
 * @brief Find the *unique* intersection point @p p between two closed segments
 * @p ab and @p cd
 * @details Return @p p and a @p MEOS_SEG_INTER_TYPE value.
 * @note Currently, the function only computes @p p if the result value is
 * @p MEOS_SEG_TOUCH_END, since the return value is never used in other cases.
 * @note If the segments overlap no point is returned since they can be an
 * infinite number of them.
 */
static int
seg2d_intersection(const POINT2D *a, const POINT2D *b, const POINT2D *c,
  const POINT2D *d, POINT2D *p)
{
  /* assume the following names: p = Segment(a, b), q = Segment(c, d) */
  int pq1, pq2, qp1, qp2;

  /* No envelope interaction => we are done. */
  if (! lw_seg_interact(a, b, c, d))
    return MEOS_SEG_NO_INTERSECTION;

  /* Are the start and end points of q on the same side of p? */
  pq1 = seg2d_side(a, b, c);
  pq2 = seg2d_side(a, b, d);
  if ((pq1 > 0 && pq2 > 0) || (pq1 < 0 && pq2 < 0))
    return MEOS_SEG_NO_INTERSECTION;

  /* Are the start and end points of p on the same side of q? */
  qp1 = seg2d_side(c, d, a);
  qp2 = seg2d_side(c, d, b);
  if ((qp1 > 0 && qp2 > 0) || (qp1 < 0 && qp2 < 0))
    return MEOS_SEG_NO_INTERSECTION;

  /* Nobody is on one side or another? Must be colinear. */
  if (pq1 == 0 && pq2 == 0 && qp1 == 0 && qp2 == 0)
    return parseg2d_intersection(a, b, c, d, p);

  /* Check if the intersection is an endpoint */
  if (pq1 == 0 || pq2 == 0 || qp1 == 0 || qp2 == 0)
  {
    /* Check for two equal endpoints */
    if ((b->x == c->x && b->y == c->y) ||
        (b->x == d->x && b->y == d->y))
    {
      p->x = b->x;
      p->y = b->y;
      return MEOS_SEG_TOUCH_END;
    }
    if ((a->x == c->x && a->y == c->y) ||
        (a->x == d->x && a->y == d->y))
    {
      p->x = a->x;
      p->y = a->y;
      return MEOS_SEG_TOUCH_END;
    }

    /* The intersection is inside one of the segments
     * note: p is not compute for this type of intersection */
    return MEOS_SEG_TOUCH;
  }

  /* Crossing
   * note: p is not compute for this type of intersection */
  return MEOS_SEG_CROSS;
}

/**
 * @brief Initialize a GBOX with a point
 */
static void gbox_init_point2d(const POINT2D *p, GBOX *gbox)
{
  gbox->xmin = gbox->xmax = p->x;
  gbox->ymin = gbox->ymax = p->y;
}

/**
 * @brief Enlarge a GBOX with a point
 */
static void gbox_merge_point2d(const POINT2D *p, GBOX *gbox)
{
  if ( gbox->xmin > p->x ) gbox->xmin = p->x;
  if ( gbox->ymin > p->y ) gbox->ymin = p->y;
  if ( gbox->xmax < p->x ) gbox->xmax = p->x;
  if ( gbox->ymax < p->y ) gbox->ymax = p->y;
}

/**
 * @brief Return the positions at which a sequence of points must be cut into
 * polylines that neither cross themselves nor repeat a point
 * @details The polyline joining the points is cut wherever two of its segments
 * meet outside the endpoint two consecutive segments share, and wherever a
 * point repeats the one before it. Cutting at a returned position keeps that
 * point in both fragments, so the fragments cover the whole polyline.
 * @note The function works only on 2D even if the input points are in 3D
 * @param[in] points Array of points
 * @param[in] npoints Number of elements in the array of points
 * @param[out] count Number of positions at which the array must be cut
 * @return Boolean array determining the positions at which the array of points
 * must be cut
 * @pre The array has at least two points
 */
bool *
pointarr_find_splits(const POINT2D **points, int npoints, int *count)
{
  assert(points); assert(count); assert(npoints >= 2);
  /* bitarr is an array of bool for collecting the splits */
  bool *bitarr = palloc0(sizeof(bool) * npoints);
  int numsplits = 0;
  for (int i = 1; i < npoints; i++)
  {
    /* If stationary segment we need to split the sequence */
    if (points[i - 1]->x == points[i]->x && points[i - 1]->y == points[i]->y)
    {
      if (i > 1 && ! bitarr[i - 1])
      {
        bitarr[i - 1] = true;
        numsplits++;
      }
      if (i < npoints - 1)
      {
        bitarr[i] = true;
        numsplits++;
      }
    }
  }

  /* Loop for every split due to stationary segments while adding
   * additional splits due to intersecting segments */
  int start = 0;
  while (start < npoints - 2)
  {
    int end = start + 1;
    while (end < npoints - 1 && ! bitarr[end])
      end++;
    if (end == start + 1)
    {
      start = end;
      continue;
    }
    /* Find intersections in the piece defined by start and end in a
     * breadth-first search */
    int i = start, j = start + 1;
    GBOX box;
    gbox_init_point2d(points[i], &box);
    gbox_merge_point2d(points[j], &box);
    while (j < end)
    {
      /* Candidate for intersection */
      POINT2D p = { 0 }; /* make compiler quiet */
      int intertype = seg2d_intersection(points[i], points[i + 1],
        points[j], points[j + 1], &p);
      if (intertype > 0 &&
        /* Exclude the case when two consecutive segments that
         * necessarily touch each other in their common point */
        (intertype != MEOS_SEG_TOUCH_END || j != i + 1 ||
         p.x != points[j]->x || p.y != points[j]->y))
      {
        /* Set the new end */
        end = j;
        bitarr[end] = true;
        numsplits++;
        break;
      }
      if (i < j - 1)
        i++;
      else
      {
        j++;
        i = start;

        /* Shortcut */
        if (!gbox_contains_point2d(&box, points[j]))
        {
          while (j < end) {
            bool out = false;
            if ( box.xmin > points[j]->x )
            {
              box.xmin = points[j]->x;
              if ( box.xmin > points[j+1]->x )
                out = true;
            }
            else if ( box.xmax < points[j]->x )
            {
              box.xmax = points[j]->x;
              if ( box.xmax < points[j+1]->x )
                out = true;
            }
            if ( box.ymin > points[j]->y )
            {
              box.ymin = points[j]->y;
              if ( box.ymin > points[j+1]->y )
                out = true;
            }
            else if ( box.ymax < points[j]->y )
            {
              box.ymax = points[j]->y;
              if ( box.ymax < points[j+1]->y )
                out = true;
            }
            if ( !out )
              break;
            j++;
          }
        }
      }
    }
    /* Process the next split */
    start = end;
  }
  *count = numsplits;
  return bitarr;
}

/*****************************************************************************
 * Oriented envelope (a.k.a minimum rotated rectangle) and convex hull of a
 * geometry whose every edge is a segment.
 *
 * Both are placed on the vertices of the geometry, which for such a geometry
 * are the whole of it: a convex hull has a vertex of the input at every corner,
 * and a rectangle of minimum area has a side flush with an edge of that hull,
 * so trying the direction of each hull edge finds it exactly.
 *
 * A geometry carrying a circular arc is declined rather than answered, since
 * the arc reaches past the points that define it and a region placed on those
 * points does not enclose it.
 *****************************************************************************/

/**
 * @brief Compare two 2D points lexicographically
 */
static inline int
point2d_cmp(const POINT2D *pa, const POINT2D *pb)
{
  if (pa->x < pb->x)
    return -1;
  if (pa->x > pb->x)
    return 1;
  if (pa->y < pb->y)
    return -1;
  if (pa->y > pb->y)
    return 1;
  return 0;
}

/* Sort points lexicographically with their order written into the sort, as
 * #edge_key_sort sorts the edges of an index */
#define ST_SORT point2d_sort
#define ST_ELEMENT_TYPE POINT2D
#define ST_COMPARE(a, b) point2d_cmp(a, b)
#define ST_SCOPE static
#define ST_DECLARE
#define ST_DEFINE
#include "port/sort_template.h"

/**
 * @brief Cross product of AB and AC
 */
static inline double
cross_product(const POINT2D *a, const POINT2D *b, const POINT2D *c)
{
  return (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
}

/**
 * @brief Add a point to an array
 */
static inline void
add_point(POINT2D *points, uint32_t *npoints, double x, double y)
{
  points[*npoints].x = x;
  points[*npoints].y = y;
  (*npoints)++;
}

/**
 * @brief Add the two end points of an edge
 * @details The end points that repeat are removed by #convex_hull_points
 */
static void
add_edge_points(const Edge *e, POINT2D *points, uint32_t *npoints)
{
  assert(e); assert(points); assert(npoints);
  add_point(points, npoints, e->x1, e->y1);
  add_point(points, npoints, e->x2, e->y2);
}

/**
 * @brief Compute the convex hull of a set of 2D points
 * @details The returned hull is not closed, counterclockwise, and has no three
 * vertices on one line. The implementation uses Andrew's monotone-chain
 * algorithm, each turn read as the exact sign of a cross product
 * (#cross_product_sign).
 */
static uint32_t
convex_hull_points(const POINT2D *points, uint32_t npoints, POINT2D **hull)
{
  assert(points); assert(hull);
  *hull = NULL;
  if (npoints == 0)
    return 0;

  /* Sort a copy of the points */
  POINT2D *sorted = palloc(sizeof(POINT2D) * npoints);
  memcpy(sorted, points, sizeof(POINT2D) * npoints);
  point2d_sort(sorted, npoints);

  /* Remove duplicate points */
  uint32_t n = 0;
  for (uint32_t i = 0; i < npoints; i++)
  {
    if (n == 0 || sorted[i].x != sorted[n - 1].x ||
        sorted[i].y != sorted[n - 1].y)
    {
      sorted[n++] = sorted[i];
    }
  }

  /* Point or line */
  if (n <= 2)
  {
    *hull = sorted;
    return n;
  }

  /* Maximum size is 2*n */
  POINT2D *h = palloc(sizeof(POINT2D) * (2 * n));
  uint32_t nhull = 0;

  /* Lower hull */
  for (uint32_t i = 0; i < n; i++)
  {
    while (nhull >= 2 &&
      cross_product_sign(h[nhull - 2].x, h[nhull - 2].y, h[nhull - 1].x,
        h[nhull - 1].y, h[nhull - 2].x, h[nhull - 2].y, sorted[i].x,
        sorted[i].y) <= 0)
    {
      nhull--;
    }
    h[nhull++] = sorted[i];
  }

  /* Upper hull */
  uint32_t lower = nhull;
  for (int i = (int) n - 2; i >= 0; i--)
  {
    while (nhull > lower &&
        cross_product_sign(h[nhull - 2].x, h[nhull - 2].y, h[nhull - 1].x,
        h[nhull - 1].y, h[nhull - 2].x, h[nhull - 2].y, sorted[i].x,
        sorted[i].y) <= 0)
      nhull--;
    h[nhull++] = sorted[i];
  }

  /* Last point duplicates the first */
  nhull--;
  pfree(sorted);

  *hull = palloc(sizeof(POINT2D) * nhull);
  memcpy(*hull, h, sizeof(POINT2D) * nhull);
  pfree(h);
  return nhull;
}

/**
 * @brief Compute the rectangle defined by a given orientation
 * @details The rectangle axes are:
 *   u = (ux,uy)
 *   v = (-uy,ux)
 * The function computes the bounding rectangle of the supplied
 * convex-hull points in that coordinate system.
 */
static double
mrr_rectangle_for_direction(const POINT2D *points, uint32_t npoints,
  double ux, double uy, POINT2D rect[5])
{
  double vx = -uy;
  double vy = ux;
  double min_u = DBL_MAX;
  double max_u = -DBL_MAX;
  double min_v = DBL_MAX;
  double max_v = -DBL_MAX;
  for (uint32_t i = 0; i < npoints; i++)
  {
    double u = points[i].x * ux + points[i].y * uy;
    double v = points[i].x * vx + points[i].y * vy;
    if (u < min_u)
      min_u = u;
    if (u > max_u)
      max_u = u;
    if (v < min_v)
      min_v = v;
    if (v > max_v)
      max_v = v;
  }

  double width = max_u - min_u;
  double height = max_v - min_v;
  /* Convert the corners back to XY */
  rect[0].x = min_u * ux + min_v * vx;
  rect[0].y = min_u * uy + min_v * vy;
  rect[1].x = max_u * ux + min_v * vx;
  rect[1].y = max_u * uy + min_v * vy;
  rect[2].x = max_u * ux + max_v * vx;
  rect[2].y = max_u * uy + max_v * vy;
  rect[3].x = min_u * ux + max_v * vx;
  rect[3].y = min_u * uy + max_v * vy;
  rect[4] = rect[0];
  return width * height;
}

/**
 * @brief Add candidate rectangle directions generated by an edge
 * @details For a straight edge, the rectangle orientation only needs the edge
 * direction. For a circular arc, its tangent direction varies continuously.
 * The cardinal directions of the supporting circle delimit the intervals over
 * which the support point changes continuously. We therefore add:
 *   - the arc endpoint tangent directions
 *   - the four cardinal tangent directions when they occur on the arc
 * This keeps the calculation entirely analytic and avoids polygonizing the arc
 */
static void
mrr_add_edge_directions(const Edge *e, double *angles, uint32_t *nangles)
{
  if (e->etype != EDGE_LINEARC && e->etype != EDGE_POLYARC)
  {
    /* Straight edge */
    double angle = atan2(e->y2 - e->y1, e->x2 - e->x1);
    angles[(*nangles)++] = angle;
    return;
  }

  /* Circular arc: For CCW traversal the tangent direction at angle theta is:
   *   (-sin(theta), cos(theta))
   * We only need orientations, so adding pi is equivalent. */
  double theta[6];
  int ntheta = 0;
  theta[ntheta++] = e->theta0;
  theta[ntheta++] = e->theta1;

  /* Cardinal points split the circle into analytically simple
   * support-function intervals */
  const double cardinal[4] = { 0.0, M_PI_2, M_PI, -M_PI_2 };
  for (int i = 0; i < 4; i++)
  {
    if (arc_contains_angle(e, cardinal[i]))
      theta[ntheta++] = cardinal[i];
  }
  for (int i = 0; i < ntheta; i++)
  {
    double angle = theta[i] + (e->ccw ? M_PI_2 : -M_PI_2);
    /* Rectangle orientation has period pi */
    angle = fmod(angle, M_PI);
    if (angle < 0)
      angle += M_PI;
    angles[(*nangles)++] = angle;
  }
}

/**
 * @brief Construct a POINT/LINESTRING/POLYGON from a set of points
 */
static LWGEOM *
make_geometry_points(int32_t srid, const POINT2D *points, uint32_t nhull)
{
  /* Point */
  if (nhull == 1)
    return lwpoint_as_lwgeom(lwpoint_make2d(srid, points[0].x, points[0].y));

  /* Line */
  if (nhull == 2)
  {
    POINTARRAY *pa = ptarray_construct_empty(0, 0, 2);
    POINT4D p;
    p.z = 0.0;
    p.m = 0.0;
    p.x = points[0].x;
    p.y = points[0].y;
    ptarray_append_point(pa, &p, LW_TRUE);
    p.x = points[1].x;
    p.y = points[1].y;
    ptarray_append_point(pa, &p, LW_TRUE);
    return lwline_as_lwgeom(lwline_construct(srid, NULL, pa));
  }

  /* Polygon */
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 5);
  POINT4D p;
  p.z = 0.0;
  p.m = 0.0;
  for (int i = 0; i < 5; i++)
  {
    p.x = points[i].x;
    p.y = points[i].y;
    ptarray_append_point(pa, &p, LW_TRUE);
  }
  LWPOLY *poly = lwpoly_construct_empty(srid, 0, 0);
  lwpoly_add_ring(poly, pa);
  return lwpoly_as_lwgeom(poly);
}

/**
 * @brief Return the convex hull of the end points of the edges of a geometry
 * @details The hull is not closed. When @p points is not NULL it receives the
 * end points in the order of the edges, and the caller frees them
 */
static uint32_t
edges_points_hull(const MeosArray *edges, POINT2D **points, uint32_t *npoints,
  POINT2D **hull)
{
  uint32_t nedges = edges->count;
  POINT2D *pts = palloc(sizeof(POINT2D) * Max(2 * nedges, 1));
  uint32_t n = 0;
  for (uint32_t i = 0; i < nedges; i++)
    add_edge_points((Edge *) meos_array_get_intl(edges, i), pts, &n);
  uint32_t nhull = convex_hull_points(pts, n, hull);
  if (points)
  {
    *points = pts;
    *npoints = n;
  }
  else
    pfree(pts);
  return nhull;
}

/**
 * @brief Order the two vertices of a degenerate hull as they appear in the
 * geometry
 * @details PostGIS function @p ST_ConvexHull() reports a two-vertex hull in
 * the order the vertices occur in its argument, which the sort performed by
 * #convex_hull_points() loses.
 */
static void
hull_order_as_input(const POINT2D *points, uint32_t npoints, POINT2D *hull)
{
  for (uint32_t i = 0; i < npoints; i++)
  {
    if (points[i].x == hull[0].x && points[i].y == hull[0].y)
      return;
    if (points[i].x == hull[1].x && points[i].y == hull[1].y)
    {
      POINT2D tmp = hull[0];
      hull[0] = hull[1];
      hull[1] = tmp;
      return;
    }
  }
}

/**
 * @brief Construct a POINT/LINESTRING/POLYGON from the vertices of a convex
 * hull
 * @details The hull computed by #convex_hull_points() is not closed, so the
 * polygon ring repeats the first vertex. One and two vertices give a POINT and
 * a LINESTRING, as PostGIS function @p ST_ConvexHull() does.
 */
static LWGEOM *
make_geometry_hull(int32_t srid, const POINT2D *hull, uint32_t nhull)
{
  /* Point and line */
  if (nhull <= 2)
    return make_geometry_points(srid, hull, nhull);

  /* Polygon.
   * #convex_hull_points() delivers the vertices counterclockwise starting at
   * an arbitrary vertex. PostGIS function @p ST_ConvexHull() reports the ring
   * clockwise starting at its lowest vertex, so the ring is emitted in that
   * order to keep both functions textually interchangeable. */
  uint32_t start = 0;
  for (uint32_t i = 1; i < nhull; i++)
  {
    if (hull[i].y < hull[start].y ||
        (hull[i].y == hull[start].y && hull[i].x < hull[start].x))
      start = i;
  }
  POINTARRAY *pa = ptarray_construct_empty(0, 0, nhull + 1);
  POINT4D p;
  p.z = 0.0;
  p.m = 0.0;
  for (uint32_t i = 0; i <= nhull; i++)
  {
    /* Walking the counterclockwise vertices backwards gives the clockwise
     * ring, and the last vertex closes it on the first one */
    const POINT2D *v = &hull[(start + nhull - i % nhull) % nhull];
    p.x = v->x;
    p.y = v->y;
    ptarray_append_point(pa, &p, LW_TRUE);
  }
  LWPOLY *poly = lwpoly_construct_empty(srid, 0, 0);
  lwpoly_add_ring(poly, pa);
  return lwpoly_as_lwgeom(poly);
}

/*****************************************************************************
 * The convex hull of a geometry carrying circular arcs
 * The hull of a geometry whose every edge is a segment has a vertex of the
 * input at every corner, so it is read from the vertices. An arc reaches past
 * the points that define it, so a hull placed on those points leaves the arc
 * outside itself. What an arc does carry is a support: in a direction its
 * angular span holds, the point of its circle furthest that way belongs to
 * the hull. The hull is walked along its boundary from one supporting feature
 * to the next (#hull_walk), and the rectangle of minimum area enclosing it is
 * read through its support function
 *****************************************************************************/

/**
 * @brief A feature the convex hull of a geometry is supported by, either a
 * point or the arc of a circle
 */
typedef struct
{
  double x, y;           /**< The point, or the centre of the circle */
  double radius;         /**< Zero for a point */
  double theta0, theta1; /**< Angular span of the arc */
  bool ccw;              /**< The way the arc is traversed */
  bool is_arc;           /**< Whether the feature is an arc */
} HullFeature;

/**
 * @brief Return true if an angle lies within the angular span of an arc
 * feature
 */
static bool
hull_arc_holds(const HullFeature *f, double phi)
{
  double sweep = f->ccw ?
    angle_normalize(f->theta1 - f->theta0) :
    angle_normalize(f->theta0 - f->theta1);
  double off = f->ccw ?
    angle_normalize(phi - f->theta0) :
    angle_normalize(f->theta0 - phi);
  return off <= sweep + MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return how far a geometry reaches in a direction, and the point of
 * it that reaches that far
 * @details The support of a point is its projection on the direction; that of
 * an arc, in a direction its span holds, is the projection of its centre plus
 * its radius, reached at the point of the circle that way. A direction the
 * span does not hold is answered by the ends of the arc, which are features of
 * their own
 * @param[in] feats,nfeats Features supporting the hull
 * @param[in] phi Direction
 * @param[out] px,py Point reaching that far, ignored when null @nullable
 * @param[out] which Feature reaching that far, ignored when null @nullable
 */
static double
hull_support(const HullFeature *feats, int nfeats, double phi, double *px,
  double *py, int *which)
{
  double ux = cos(phi), uy = sin(phi);
  double result = -DBL_MAX;
  for (int i = 0; i < nfeats; i++)
  {
    const HullFeature *f = &feats[i];
    double s = f->x * ux + f->y * uy;
    double cx = f->x, cy = f->y;
    if (f->is_arc)
    {
      if (! hull_arc_holds(f, phi))
        continue;
      s += f->radius;
      cx = f->x + f->radius * ux;
      cy = f->y + f->radius * uy;
    }
    if (s > result)
    {
      result = s;
      if (px) *px = cx;
      if (py) *py = cy;
      if (which) *which = i;
    }
  }
  return result;
}

/**
 * @brief Add a direction to an array of directions
 */
static void
hull_add_direction(double phi, double *dirs, int *ndirs)
{
  dirs[(*ndirs)++] = angle_normalize(phi);
  return;
}

/**
 * @brief Comparator of two directions
 */
static int
hull_direction_cmp(const void *a, const void *b)
{
  double x = *(const double *) a, y = *(const double *) b;
  return (x < y) ? -1 : ((x > y) ? 1 : 0);
}

/**
 * @brief Return the directions in which the feature supporting the hull can
 * change
 * @details One feature gives way to another where the two reach equally far,
 * which for features at @p (A,B) apart and of radii differing by @p C is where
 * `A cos(phi) + B sin(phi) = C`, and an arc enters and leaves the contest at
 * the ends of its span. Between two consecutive such directions one feature
 * supports the hull throughout
 */
static int
hull_directions(const HullFeature *feats, int nfeats, double **result)
{
  int maxdirs = nfeats * nfeats + 2 * nfeats + 4;
  double *dirs = palloc(sizeof(double) * maxdirs);
  int ndirs = 0;
  for (int i = 0; i < nfeats; i++)
  {
    if (feats[i].is_arc)
    {
      hull_add_direction(feats[i].theta0, dirs, &ndirs);
      hull_add_direction(feats[i].theta1, dirs, &ndirs);
    }
    for (int j = i + 1; j < nfeats; j++)
    {
      double a = feats[i].x - feats[j].x;
      double b = feats[i].y - feats[j].y;
      double c = (feats[j].is_arc ? feats[j].radius : 0.0) -
        (feats[i].is_arc ? feats[i].radius : 0.0);
      double h = hypot(a, b);
      if (h <= MEOS_GEOM_TOLERANCE || fabs(c) > h)
        continue;
      double ratio = c / h;
      /* The end of an arc lies on its own circle, so the two features reach
       * equally far in exactly one direction, the radial one. Reading that
       * direction from an arc cosine of a ratio a rounding away from one
       * moves it far enough to leave a sliver between the arc and the point
       * that ends it, so the ratio is read as the one it is */
      double base = atan2(b, a);
      double off = (ratio >= 1.0 - MEOS_GEOM_TOLERANCE) ? 0.0 :
        ((ratio <= -1.0 + MEOS_GEOM_TOLERANCE) ? M_PI : acos(ratio));
      hull_add_direction(base + off, dirs, &ndirs);
      hull_add_direction(base - off, dirs, &ndirs);
    }
  }
  if (ndirs == 0)
    hull_add_direction(0.0, dirs, &ndirs);
  qsort(dirs, ndirs, sizeof(double), hull_direction_cmp);
  /* Remove the directions that repeat the one before them */
  int nuniq = 0;
  for (int i = 0; i < ndirs; i++)
    if (nuniq == 0 || dirs[i] - dirs[nuniq - 1] > MEOS_GEOM_TOLERANCE)
      dirs[nuniq++] = dirs[i];
  *result = dirs;
  return nuniq;
}

/**
 * @brief A feature of the hull of a geometry carrying arcs, a point or an
 * arc
 */
typedef struct
{
  const Edge *arc;       /**< The arc, NULL for a point */
  double x, y;           /**< The point, or for a direction of an end of an
                              arc that end */
} HullAt;

/**
 * @brief Return the point of a feature its exact numbers are taken from: the
 * point, the start of an arc, or the centre of a whole circle
 */
static void
hull_at_origin(const HullAt *f, double *ox, double *oy)
{
  if (! f->arc)
  {
    *ox = f->x; *oy = f->y;
  }
  else if (f->arc->centred)
  {
    *ox = f->arc->cx; *oy = f->arc->cy;
  }
  else
  {
    *ox = f->arc->sx; *oy = f->arc->sy;
  }
  return;
}

/**
 * @brief The centre of a feature and the square of its radius, exactly: the
 * centre is (px / w, py / w) from an origin, w positive, and rw is the square
 * of the radius times w^2, zero for a point
 */
typedef struct
{
  TowerX px, py, w, rw;
} HullExact;

/**
 * @brief Set the centre and the square of the radius of a feature, exactly
 * @details The circle through S, M and E, with m = M - S, n = E - S and
 * D = m x n, has its centre at S + K / (2 D) for
 * K = (|m|^2 n_y - |n|^2 m_y, |n|^2 m_x - |m|^2 n_x), so that w = 2 |D| and
 * rw = |K|^2, the sign of D being the exact turn of the three points
 * (#cross_product_sign). The circle on the diameter from S to E has its
 * centre at S + n / 2, and a whole circle the centre and the radius it is
 * given
 */
static void
hull_exact(TowerCtx *ctx, const HullAt *f, double ox, double oy,
  HullExact *out)
{
  const Edge *e = f->arc;
  if (! e)
  {
    out->px = tx_diff(ctx, f->x, ox);
    out->py = tx_diff(ctx, f->y, oy);
    out->w = tx_val(ctx, 1.0);
    out->rw = tx_val(ctx, 0.0);
    return;
  }
  if (e->centred)
  {
    TowerX r = tx_val(ctx, e->radius);
    out->px = tx_diff(ctx, e->cx, ox);
    out->py = tx_diff(ctx, e->cy, oy);
    out->w = tx_val(ctx, 1.0);
    out->rw = tx_mul(ctx, r, r);
    return;
  }
  TowerX nx = tx_diff(ctx, e->ex, e->sx), ny = tx_diff(ctx, e->ey, e->sy);
  TowerX kx = nx, ky = ny;
  if (e->diameter)
    out->w = tx_val(ctx, 2.0);
  else
  {
    TowerX mx = tx_diff(ctx, e->mx, e->sx), my = tx_diff(ctx, e->my, e->sy);
    TowerX mm = tx_add(ctx, tx_mul(ctx, mx, mx), tx_mul(ctx, my, my));
    TowerX nn = tx_add(ctx, tx_mul(ctx, nx, nx), tx_mul(ctx, ny, ny));
    double turn = (double) cross_product_sign(e->sx, e->sy, e->mx, e->my,
      e->sx, e->sy, e->ex, e->ey);
    kx = tx_scale(ctx, tx_sub(ctx, tx_mul(ctx, mm, ny), tx_mul(ctx, nn, my)),
      turn);
    ky = tx_scale(ctx, tx_sub(ctx, tx_mul(ctx, nn, mx), tx_mul(ctx, mm, nx)),
      turn);
    out->w = tx_scale(ctx, tx_sub(ctx, tx_mul(ctx, mx, ny),
      tx_mul(ctx, my, nx)), 2.0 * turn);
  }
  out->px = tx_add(ctx, tx_mul(ctx, tx_diff(ctx, e->sx, ox), out->w), kx);
  out->py = tx_add(ctx, tx_mul(ctx, tx_diff(ctx, e->sy, oy), out->w), ky);
  out->rw = tx_add(ctx, tx_mul(ctx, kx, kx), tx_mul(ctx, ky, ky));
  return;
}

/**
 * @brief A feature of the hull in doubles with a bound on the error of each
 * (#ArcsegFilter), from its own origin (#hull_at_origin): its centre
 * (px / w, py / w), w positive, and r, its radius times w (#hull_exact)
 */
typedef struct
{
  ArcsegFilter px, py, w, r;
  double ox, oy;         /**< The origin */
} HullFilter;

/**
 * @brief Set a feature of the hull in doubles with bounds on their errors
 */
static void
hull_filter_set(const HullAt *f, HullFilter *out)
{
  hull_at_origin(f, &out->ox, &out->oy);
  if (! f->arc)
  {
    out->px.v = out->px.e = out->py.v = out->py.e = 0.0;
    out->w.v = 1.0; out->w.e = 0.0;
    out->r.v = out->r.e = 0.0;
    return;
  }
  TowerCtx ctx;
  tower_ctx_init(&ctx, false);
  ctx.filter = true;
  HullExact e;
  hull_exact(&ctx, f, out->ox, out->oy, &e);
  out->px = e.px.f;
  out->py = e.py.f;
  out->w = e.w.f;
  out->r = tower_f_sqrt(e.rw.f);
  tower_ctx_free(&ctx);
  return;
}

/**
 * @brief Set the centre of a feature in doubles from another origin, times
 * its w: its own numbers moved by the difference of the two origins, which
 * the bounds then size to the distance between them rather than to the
 * coordinates
 */
static void
hull_filter_from(const HullFilter *f, double ox, double oy, ArcsegFilter *px,
  ArcsegFilter *py)
{
  *px = arcseg_f_add(f->px, arcseg_f_mul(arcseg_f_diff(f->ox, ox), f->w));
  *py = arcseg_f_add(f->py, arcseg_f_mul(arcseg_f_diff(f->oy, oy), f->w));
  return;
}

/**
 * @brief Return the sign of a value in doubles with a bound on its error, an
 * exact zero being zero (#tx_sign), or 2 where the bound does not tell it
 */
static int
hull_filter_sign(ArcsegFilter a)
{
  return (a.v == 0.0 && a.e == 0.0) ? 0 : arcseg_f_sign(a);
}

/**
 * @brief An edge from A to B of the hull of a set of points in doubles with
 * bounds on their errors, as #HullFilter holds a feature
 * @details It holds B - A, its length, and k = (B - A) x (A - G) from the
 * origin G, the first vertex of the hull
 */
typedef struct
{
  ArcsegFilter ex, ey, len, k;
} HullEdgeFilter;

/**
 * @brief The largest of each quantity of #HullEdgeFilter over the edges of a
 * hull and the largest bound on its error, which bound the error of a sum
 * over any edge read in doubles alone (#hull_circle_inside)
 */
typedef struct
{
  double e, ee;          /**< |B - A| along either axis, and its error */
  double k, ke;
  double len, lene;
} HullEdgeMax;

/**
 * @brief Return the edges of the hull of a set of points in doubles with
 * bounds on their errors (#HullEdgeFilter), and set their largest values
 */
static HullEdgeFilter *
hull_edge_filters(const POINT2D *hull, uint32_t nhull, HullEdgeMax *max)
{
  HullEdgeFilter *result = palloc(sizeof(HullEdgeFilter) * Max(nhull, 1));
  memset(max, 0, sizeof(HullEdgeMax));
  for (uint32_t i = 0; i < nhull; i++)
  {
    const POINT2D *a = &hull[i], *b = &hull[(i + 1) % nhull];
    HullEdgeFilter *e = &result[i];
    e->ex = arcseg_f_diff(b->x, a->x);
    e->ey = arcseg_f_diff(b->y, a->y);
    e->len = tower_f_sqrt(arcseg_f_add(arcseg_f_mul(e->ex, e->ex),
      arcseg_f_mul(e->ey, e->ey)));
    e->k = arcseg_f_sub(
      arcseg_f_mul(e->ex, arcseg_f_diff(a->y, hull[0].y)),
      arcseg_f_mul(e->ey, arcseg_f_diff(a->x, hull[0].x)));
    max->e = fmax(max->e, fmax(fabs(e->ex.v), fabs(e->ey.v)));
    max->ee = fmax(max->ee, fmax(e->ex.e, e->ey.e));
    max->k = fmax(max->k, fabs(e->k.v));
    max->ke = fmax(max->ke, e->k.e);
    max->len = fmax(max->len, e->len.v);
    max->lene = fmax(max->lene, e->len.e);
  }
  return result;
}

/**
 * @brief Return true when a circle lies inside the hull of a set of points,
 * clear of its boundary
 * @details An arc of such a circle supports the hull in no direction. The
 * hull is counterclockwise with no three vertices collinear
 * (#convex_hull_points). The circle lies inside where the line of every edge
 * misses it (#arcseg_line_disc_sign) and a point of it lies strictly left of
 * that edge (#cross_product_sign): its start, an input point on it, or for a
 * whole circle its centre. The end of an arc lies on its circle too, so an
 * end that does not lie strictly left of an edge tells, without the
 * discriminant, that the line meets the circle, as it does where an edge of
 * the hull leaves an arc along its tangent. Each sign is decided exactly on
 * the input points that fix the circle, never on the centre and radius
 * constructed from them. Most edges are told first in doubles carrying bounds
 * on their errors (#hull_filter_set): the centre lies left of the edge by
 * more than the radius where S = (B - A) x (C - A) w is positive and
 * S - r w |B - A| too. From the origin G of #HullEdgeFilter,
 * S = (B - A) x (C - G) w - w (B - A) x (A - G), the second cross product
 * read once for every circle
 * @param[in] hull,nhull Vertices of the hull
 * @param[in] ef Edges of the hull in doubles (#hull_edge_filters)
 * @param[in] max Their largest values (#HullEdgeMax), which tell most edges
 * from doubles alone
 * @param[in] arc Arc edge
 */
static bool
hull_circle_inside(const POINT2D *hull, uint32_t nhull,
  const HullEdgeFilter *ef, const HullEdgeMax *max, const Edge *arc)
{
  if (nhull < 3)
    return false;
  HullAt f = {arc, 0.0, 0.0};
  HullFilter c;
  hull_filter_set(&f, &c);
  ArcsegFilter gx, gy;
  hull_filter_from(&c, hull[0].x, hull[0].y, &gx, &gy);
  double qx = arc->centred ? arc->cx : arc->sx;
  double qy = arc->centred ? arc->cy : arc->sy;
  /* A bound on the error of S - r w |B - A| read in doubles alone over any
   * edge: seven roundings of the sum of the magnitudes of its four products,
   * and the errors of their factors, each factor taken at its largest */
  double gs = fabs(gx.v) + fabs(gy.v), ge = gx.e + gy.e;
  double wv = fabs(c.w.v), rv = fabs(c.r.v);
  double bound = 8.0 * DBL_EPSILON *
      (max->e * gs + wv * max->k + rv * max->len) +
    max->e * ge + max->ee * (gs + ge) +
    wv * max->ke + c.w.e * (max->k + max->ke) +
    rv * max->lene + c.r.e * (max->len + max->lene);
  bound = bound * (1.0 + 1e-6) + DBL_MIN;
  for (uint32_t i = 0; i < nhull; i++)
  {
    const POINT2D *a = &hull[i], *b = &hull[(i + 1) % nhull];
    const HullEdgeFilter *e = &ef[i];
    /* The circle clears the edge where S - r w |B - A| exceeds the bound */
    if (e->ex.v * gy.v - e->ey.v * gx.v - c.w.v * e->k.v -
        c.r.v * e->len.v > bound)
      continue;
    ArcsegFilter sa = arcseg_f_sub(arcseg_f_sub(arcseg_f_mul(e->ex, gy),
      arcseg_f_mul(e->ey, gx)), arcseg_f_mul(c.w, e->k));
    int side = hull_filter_sign(sa);
    int clear = hull_filter_sign(arcseg_f_sub(sa,
      arcseg_f_mul(c.r, e->len)));
    if (side == 1 && clear == 1)
      continue;
    if (side == 0 || side == -1 || (side == 1 && clear != 2))
      return false;
    if (cross_product_sign(a->x, a->y, b->x, b->y, a->x, a->y, qx, qy) <= 0 ||
        (! arc->centred && cross_product_sign(a->x, a->y, b->x, b->y, a->x,
          a->y, arc->ex, arc->ey) <= 0) ||
        arcseg_line_disc_sign(a->x, a->y, b->x, b->y, arc) >= 0)
      return false;
  }
  return true;
}

/**
 * @brief Return the arcs of a geometry reaching past the hull of the end
 * points of its edges
 * @details An arc whose circle lies inside that hull never supports the hull
 * of the geometry, and is left out (#hull_circle_inside)
 * @param[in] edges Edges of the geometry
 * @param[in] hull,nhull Vertices of the hull of the end points of the edges,
 * counterclockwise (#edges_points_hull)
 * @param[out] narcs Number of arcs returned
 */
static const Edge **
hull_points_arcs(const MeosArray *edges, const POINT2D *hull, uint32_t nhull,
  int *narcs)
{
  int nedges = (int) edges->count;
  const Edge **result = palloc(sizeof(Edge *) * Max(nedges, 1));
  HullEdgeMax max;
  HullEdgeFilter *ef = hull_edge_filters(hull, nhull, &max);
  int n = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = (Edge *) meos_array_get_intl(edges, i);
    if ((e->etype == EDGE_LINEARC || e->etype == EDGE_POLYARC) &&
        ! hull_circle_inside(hull, nhull, ef, &max, e))
      result[n++] = e;
  }
  pfree(ef);
  *narcs = n;
  return result;
}

/**
 * @brief Collect the features supporting the convex hull of a geometry: the
 * vertices of the hull of the end points of its edges and the circles of the
 * arcs reaching past that hull (#hull_points_arcs)
 * @details A point within the hull of the other points never supports the
 * hull, so reading the vertices of that hull rather than every end point
 * keeps the number of features small
 */
static HullFeature *
hull_features(const MeosArray *edges, const POINT2D *hull, uint32_t nhull,
  int *nfeats)
{
  int narcs = 0;
  const Edge **arcs = hull_points_arcs(edges, hull, nhull, &narcs);
  HullFeature *result = palloc(sizeof(HullFeature) *
    Max((int) nhull + narcs, 1));
  int n = 0;
  for (uint32_t i = 0; i < nhull; i++)
  {
    result[n].x = hull[i].x; result[n].y = hull[i].y;
    result[n].radius = 0; result[n].theta0 = result[n].theta1 = 0;
    result[n].ccw = true; result[n].is_arc = false;
    n++;
  }
  for (int i = 0; i < narcs; i++)
  {
    const Edge *e = arcs[i];
    result[n].x = e->cx; result[n].y = e->cy; result[n].radius = e->radius;
    result[n].theta0 = e->theta0; result[n].theta1 = e->theta1;
    result[n].ccw = e->ccw; result[n].is_arc = true;
    n++;
  }
  pfree(arcs);
  *nfeats = n;
  return result;
}

/**
 * @brief One piece of the boundary of the convex hull, a segment or an arc
 */
typedef struct
{
  double x1, y1;         /**< Point the piece starts from */
  double xm, ym;         /**< Point halfway along an arc piece */
  double x2, y2;         /**< Point the piece ends on */
  bool is_arc;           /**< Whether the piece is an arc */
} HullBoundary;

/*****************************************************************************
 * The walk of the boundary of the convex hull of a geometry carrying arcs
 *
 * The boundary is walked counterclockwise by its outward normal, once around
 * from the direction (0, -1). The directions where the hull of the points
 * passes from one vertex to the next, where the span of the normals of an arc
 * starts and ends, and the four axes cut the turn into windows of less than a
 * quarter of it. Within a window the hull is supported by the vertex of the
 * hull of the points supporting that hull there or by an arc whose span holds
 * the window: every other point lies inside the hull of the points. The
 * feature supporting the hull at the start of a window supports it until
 * another reaches past it, which first happens where the outer common tangent
 * of the two touches both, and the walk follows that tangent to the feature.
 *
 * Every decision is the exact sign of a number of a tower of square roots
 * (#TowerNum), read in doubles carrying a bound on their error and formed again
 * exactly where the bound cannot tell (#tower_pred_run). The circle through
 * three points has a rational centre and a rational square of its radius, so
 * its radius is the root of a rational, and the outer common tangent of two
 * circles adds the root of E = |C2 - C1|^2 - (r1 - r2)^2. No decision reads
 * the centre or the radius an arc edge carries in doubles: those build the
 * points the boundary passes through, each rounded once, an input point being
 * its own double.
 *****************************************************************************/

/**
 * @brief The kinds of a direction of the walk
 */
typedef enum
{
  HULL_DIR_AXIS,         /**< The unit vector (a.x, a.y) */
  HULL_DIR_EDGE,         /**< The outward normal of the edge from the point a
                              to the point b of a counterclockwise ring */
  HULL_DIR_END,          /**< The outward normal of the arc a at its end
                              (a.x, a.y) */
  HULL_DIR_TANGENT,      /**< The outward normal of the outer common tangent
                              walked counterclockwise from the feature a to
                              the feature b */
} HullDirKind;

/**
 * @brief A direction of the walk, given by what defines it
 */
typedef struct
{
  HullDirKind kind;
  HullAt a, b;
} HullDir;

/**
 * @brief Return true if two features are one
 */
static bool
hull_at_same(const HullAt *f, const HullAt *g)
{
  if (f->arc || g->arc)
    return f->arc == g->arc;
  return f->x == g->x && f->y == g->y;
}

/**
 * @brief Add to a tower the root of a number of the roots it holds, and
 * return where the root stands
 */
static int
hull_tower_root(Tower *t, const TowerX *h)
{
  int k = t->depth;
  assert(k < TOWER_MAXDEPTH);
  memcpy(t->h[k].c, h, sizeof(TowerX) * (1 << k));
  t->key[k] = NULL;
  t->depth = k + 1;
  return k;
}

/**
 * @brief Add to a tower the root of the square of the radius of a feature
 * times w^2, and return where it stands, -1 for a point
 */
static int
hull_tower_radius(TowerCtx *ctx, Tower *t, const HullAt *f,
  const HullExact *e)
{
  if (! f->arc)
    return -1;
  TowerX h[TOWER_SIZE];
  tower_set(ctx, e->rw, t->depth, h);
  return hull_tower_root(t, h);
}

/**
 * @brief Set the vector of a direction given by rationals: an axis, the
 * normal of an edge, or the end of an arc less its centre, times w
 */
static void
hull_dir_vector(TowerCtx *ctx, const HullDir *d, TowerX *vx, TowerX *vy)
{
  if (d->kind == HULL_DIR_AXIS)
  {
    *vx = tx_val(ctx, d->a.x);
    *vy = tx_val(ctx, d->a.y);
  }
  else if (d->kind == HULL_DIR_EDGE)
  {
    *vx = tx_diff(ctx, d->b.y, d->a.y);
    *vy = tx_diff(ctx, d->a.x, d->b.x);
  }
  else
  {
    /* The centre from the end of the arc is (px, py) / w */
    HullExact e;
    hull_exact(ctx, &d->a, d->a.x, d->a.y, &e);
    *vx = tx_scale(ctx, e.px, -1.0);
    *vy = tx_scale(ctx, e.py, -1.0);
  }
  return;
}

/**
 * @brief Set the vector of a direction in a tower, adding the roots it needs
 * @details The vector of an axis, of the normal of an edge or of the end of
 * an arc is rational. That of the tangent from the feature A to the feature
 * B, with D = (C_B - C_A) w_A w_B and rho = (r_A - r_B) w_A w_B, is
 * n = rho D - sqrt(E) rot(D), rot(x, y) = (-y, x) and E = |D|^2 - rho^2, of
 * length |D|^2: the normal of the tangent that leaves both features on its
 * left, the one #tower_point_tangent places for the side -1. Its roots are
 * the radius of A, that of B, then sqrt(E)
 * @param[in] ctx Arithmetic
 * @param[in] d Direction
 * @param[in] ox,oy Origin of the exact numbers
 * @param[in,out] t Tower
 * @param[out] nx,ny Vector
 * @param[out] len Where not NULL, the length of the vector, adding its root
 * for the normal of an edge or of the end of an arc @nullable
 * @param[out] ra Where not NULL, where the root of the radius of A stands, -1
 * for none @nullable
 */
static void
hull_dir_tower(TowerCtx *ctx, const HullDir *d, double ox, double oy,
  Tower *t, TowerX *nx, TowerX *ny, TowerX *len, int *ra)
{
  TowerX zero = tx_val(ctx, 0.0);
  TowerX h[TOWER_SIZE];
  tower_set(ctx, zero, TOWER_MAXDEPTH, nx);
  tower_set(ctx, zero, TOWER_MAXDEPTH, ny);
  if (len)
    tower_set(ctx, zero, TOWER_MAXDEPTH, len);
  if (ra)
    *ra = -1;
  if (d->kind != HULL_DIR_TANGENT)
  {
    hull_dir_vector(ctx, d, &nx[0], &ny[0]);
    if (! len)
      return;
    if (d->kind == HULL_DIR_AXIS)
    {
      len[0] = tx_val(ctx, 1.0);
      return;
    }
    tower_set(ctx, tx_add(ctx, tx_mul(ctx, nx[0], nx[0]),
      tx_mul(ctx, ny[0], ny[0])), t->depth, h);
    len[1 << hull_tower_root(t, h)] = tx_val(ctx, 1.0);
    return;
  }
  HullExact ea, eb;
  hull_exact(ctx, &d->a, ox, oy, &ea);
  hull_exact(ctx, &d->b, ox, oy, &eb);
  TowerX rho[TOWER_SIZE];
  tower_set(ctx, zero, TOWER_MAXDEPTH, rho);
  int i = hull_tower_radius(ctx, t, &d->a, &ea);
  if (i >= 0)
    rho[1 << i] = eb.w;
  if (ra)
    *ra = i;
  i = hull_tower_radius(ctx, t, &d->b, &eb);
  if (i >= 0)
    rho[1 << i] = tx_scale(ctx, ea.w, -1.0);
  TowerX dx = tx_sub(ctx, tx_mul(ctx, eb.px, ea.w), tx_mul(ctx, ea.px, eb.w));
  TowerX dy = tx_sub(ctx, tx_mul(ctx, eb.py, ea.w), tx_mul(ctx, ea.py, eb.w));
  TowerX dd = tx_add(ctx, tx_mul(ctx, dx, dx), tx_mul(ctx, dy, dy));
  int k = t->depth;
  TowerX rr[TOWER_SIZE];
  tower_mul_k(ctx, t, rho, rho, k, rr);
  tower_set(ctx, dd, k, h);
  tower_sub_k(ctx, h, rr, k, h);
  int ie = hull_tower_root(t, h);
  k = t->depth;
  tower_mulx_k(ctx, rho, dx, k, nx);
  tower_mulx_k(ctx, rho, dy, k, ny);
  nx[1 << ie] = dy;
  ny[1 << ie] = tx_scale(ctx, dx, -1.0);
  if (len)
    len[0] = dd;
  return;
}

/**
 * @brief Return true where an arc defined by input points carries a point
 * among them, which lies exactly on its circle
 */
static bool
hull_arc_carries(const Edge *e, double x, double y)
{
  if (e->centred)
    return false;
  return (e->sx == x && e->sy == y) || (e->ex == x && e->ey == y) ||
    (! e->diameter && e->mx == x && e->my == y);
}

/**
 * @brief Return true where the features X and F are two arcs carrying one
 * input point, and set it
 * @details The point is the end of X or of F where the direction stands
 * where both carry it, otherwise one carried by both
 */
static bool
hull_shared_point(const HullAt *x, const HullAt *f, const HullDir *d,
  double *px, double *py)
{
  const Edge *a = x->arc, *b = f->arc;
  if (! a || ! b || a->centred || b->centred)
    return false;
  if (d->kind == HULL_DIR_END && (d->a.arc == a || d->a.arc == b) &&
      hull_arc_carries(a, d->a.x, d->a.y) &&
      hull_arc_carries(b, d->a.x, d->a.y))
  {
    *px = d->a.x; *py = d->a.y;
    return true;
  }
  double pts[3][2] = {{a->sx, a->sy}, {a->ex, a->ey}, {a->mx, a->my}};
  for (int k = 0; k < (a->diameter ? 2 : 3); k++)
    if (hull_arc_carries(b, pts[k][0], pts[k][1]))
    {
      *px = pts[k][0]; *py = pts[k][1];
      return true;
    }
  return false;
}

/**
 * @brief Return true where the circles of two arcs through the point P touch
 * there from one side, their centres C_X and C_F on one ray from P
 * @details With c = C - P, the radius of a circle through P is |c|, and two
 * distinct circles through P touch there where c_X x c_F is zero, from one
 * side where c_X . c_F is positive too: two signs of the input points, with no
 * root (#hull_exact)
 */
static bool
hull_shared_tangent(TowerCtx *ctx, const HullExact *ex, const HullExact *ef)
{
  TowerX cross = tx_sub(ctx, tx_mul(ctx, ex->px, ef->py),
    tx_mul(ctx, ex->py, ef->px));
  if (tx_sign(ctx, cross) != 0)
    return false;
  TowerX dot = tx_add(ctx, tx_mul(ctx, ex->px, ef->px),
    tx_mul(ctx, ex->py, ef->py));
  return tx_sign(ctx, dot) > 0;
}

/**
 * @brief Return the sign of how much further than the feature X the feature
 * F reaches in a direction, (C_F - C_X) . n + (r_F - r_X) |n| times
 * w_X w_F, or 2 where the arithmetic does not tell it
 * @details Where X and F are two arcs through an input point P and the
 * direction is the normal of one of them at P, the sign is told with no
 * root: with c = C - P, F reaches past X along the normal -c_X / |c_X| of X
 * by |c_F| - c_F . c_X / |c_X|, which is not negative and is zero where the
 * circles touch at P from one side (#hull_shared_tangent), and X reaches past
 * F along the normal of F likewise. The arcs of one circle given in pieces,
 * their circles a rounding apart, ask this of every end they share. Along
 * the normal of an edge of the hull of the points whose ends the features
 * carry, how far each reaches past the edge's line is formed without the
 * cancellation of an edge nearly touching an arc, as the hull of a buffer
 * leaves one arc for the next along their common tangent
 */
static int
hull_reach_sign(TowerCtx *ctx, const HullAt *x, const HullAt *f,
  const HullDir *d)
{
  double qx, qy;
  if (d->kind == HULL_DIR_END && (d->a.arc == x->arc || d->a.arc == f->arc) &&
      hull_shared_point(x, f, d, &qx, &qy) && d->a.x == qx && d->a.y == qy)
  {
    HullExact cx, cf;
    hull_exact(ctx, x, qx, qy, &cx);
    hull_exact(ctx, f, qx, qy, &cf);
    int result = hull_shared_tangent(ctx, &cx, &cf) ? 0 :
      ((d->a.arc == x->arc) ? 1 : -1);
    return tower_ctx_unsafe(ctx) ? 2 : result;
  }
  /* A point Q and an arc carrying the input point P, along the normal n of
   * the edge from one to the other: Q . n = P . n, which the circle reaches
   * past unless the edge touches it at P, (P - C) x n zero and (P - C) . n
   * positive */
  if (d->kind == HULL_DIR_EDGE && (x->arc != NULL) != (f->arc != NULL))
  {
    const HullAt *q = x->arc ? f : x, *arc = x->arc ? x : f;
    const HullAt *pe = (d->a.x == q->x && d->a.y == q->y) ? &d->b :
      ((d->b.x == q->x && d->b.y == q->y) ? &d->a : NULL);
    if (pe && hull_arc_carries(arc->arc, pe->x, pe->y))
    {
      HullExact ca;
      hull_exact(ctx, arc, pe->x, pe->y, &ca);
      TowerX nx, ny;
      hull_dir_vector(ctx, d, &nx, &ny);
      /* (px, py) / w is C - P */
      bool touch = tx_sign(ctx, tx_sub(ctx, tx_mul(ctx, ca.px, ny),
        tx_mul(ctx, ca.py, nx))) == 0 &&
        tx_sign(ctx, tx_add(ctx, tx_mul(ctx, ca.px, nx),
          tx_mul(ctx, ca.py, ny))) < 0;
      int result = touch ? 0 : (x->arc ? -1 : 1);
      return tower_ctx_unsafe(ctx) ? 2 : result;
    }
  }
  /* Two arcs carrying the ends P_X and P_F of the edge, along its normal n:
   * P_X . n = P_F . n, so that F reaches past X by delta_F - delta_X, where
   * delta = |c| |n| - c . n with c = P - C is how far the circle reaches past
   * P. An edge nearly touching both arcs cancels each delta, and
   * delta = (c x n)^2 / (|c| |n| + c . n) does not where c . n is positive:
   * with (p_x, p_y) / w = C - P, the sign is that of
   * (p_F x n)^2 w_X D_X - (p_X x n)^2 w_F D_F, D = sqrt(r^2 w^2) |n| - p . n */
  if (d->kind == HULL_DIR_EDGE && x->arc && f->arc)
  {
    const HullAt *ends[2] = {&d->a, &d->b}, *pxe = NULL, *pfe = NULL;
    for (int k = 0; k < 2; k++)
    {
      if (! pxe && hull_arc_carries(x->arc, ends[k]->x, ends[k]->y))
        pxe = ends[k];
      if (! pfe && hull_arc_carries(f->arc, ends[k]->x, ends[k]->y))
        pfe = ends[k];
    }
    if (pxe && pfe)
    {
      HullExact cx, cf;
      hull_exact(ctx, x, pxe->x, pxe->y, &cx);
      hull_exact(ctx, f, pfe->x, pfe->y, &cf);
      TowerX nx, ny;
      hull_dir_vector(ctx, d, &nx, &ny);
      TowerX dotx = tx_add(ctx, tx_mul(ctx, cx.px, nx),
        tx_mul(ctx, cx.py, ny));
      TowerX dotf = tx_add(ctx, tx_mul(ctx, cf.px, nx),
        tx_mul(ctx, cf.py, ny));
      if (tx_sign(ctx, dotx) < 0 && tx_sign(ctx, dotf) < 0)
      {
        TowerX crx = tx_sub(ctx, tx_mul(ctx, cx.px, ny),
          tx_mul(ctx, cx.py, nx));
        TowerX crf = tx_sub(ctx, tx_mul(ctx, cf.px, ny),
          tx_mul(ctx, cf.py, nx));
        TowerX a = tx_mul(ctx, tx_mul(ctx, crf, crf), cx.w);
        TowerX b = tx_mul(ctx, tx_mul(ctx, crx, crx), cf.w);
        Tower t;
        t.depth = 0;
        TowerX h[TOWER_SIZE], u[TOWER_SIZE];
        tower_set(ctx, cx.rw, t.depth, h);
        int ix = hull_tower_root(&t, h);
        tower_set(ctx, cf.rw, t.depth, h);
        int i = hull_tower_root(&t, h);
        tower_set(ctx, tx_add(ctx, tx_mul(ctx, nx, nx), tx_mul(ctx, ny, ny)),
          t.depth, h);
        int in = hull_tower_root(&t, h);
        tower_set(ctx, tx_sub(ctx, tx_mul(ctx, b, dotf),
          tx_mul(ctx, a, dotx)), TOWER_MAXDEPTH, u);
        u[(1 << ix) | (1 << in)] = a;
        u[(1 << i) | (1 << in)] = tx_scale(ctx, b, -1.0);
        int result = tower_sign_k(ctx, &t, u, t.depth);
        return tower_ctx_unsafe(ctx) ? 2 : result;
      }
    }
  }

  double ox, oy;
  hull_at_origin(x, &ox, &oy);
  HullExact ex, ef;
  hull_exact(ctx, x, ox, oy, &ex);
  hull_exact(ctx, f, ox, oy, &ef);
  TowerX dx = tx_sub(ctx, tx_mul(ctx, ef.px, ex.w), tx_mul(ctx, ex.px, ef.w));
  TowerX dy = tx_sub(ctx, tx_mul(ctx, ef.py, ex.w), tx_mul(ctx, ex.py, ef.w));
  Tower t;
  t.depth = 0;
  TowerX nx[TOWER_SIZE], ny[TOWER_SIZE], len[TOWER_SIZE], rho[TOWER_SIZE],
    u[TOWER_SIZE], v[TOWER_SIZE];
  int ix;
  hull_dir_tower(ctx, d, ox, oy, &t, nx, ny, len, &ix);
  /* The root of the radius of X is that of the tangent leaving it */
  if (d->kind != HULL_DIR_TANGENT || ! hull_at_same(&d->a, x))
    ix = hull_tower_radius(ctx, &t, x, &ex);
  int i = hull_tower_radius(ctx, &t, f, &ef);
  int k = t.depth;
  tower_set(ctx, tx_val(ctx, 0.0), TOWER_MAXDEPTH, rho);
  if (i >= 0)
    rho[1 << i] = ex.w;
  if (ix >= 0)
    rho[1 << ix] = tx_scale(ctx, ef.w, -1.0);
  tower_mulx_k(ctx, nx, dx, k, u);
  tower_mulx_k(ctx, ny, dy, k, v);
  tower_add_k(ctx, u, v, k, u);
  tower_mul_k(ctx, &t, rho, len, k, v);
  tower_add_k(ctx, u, v, k, u);
  int result = tower_sign_k(ctx, &t, u, k);
  return tower_ctx_unsafe(ctx) ? 2 : result;
}

/**
 * @brief Arguments of #hull_reach_pred
 */
typedef struct
{
  const HullAt *x, *f;
  const HullDir *d;
} HullReachArg;

static int
hull_reach_pred(TowerCtx *ctx, const void *arg)
{
  const HullReachArg *a = arg;
  return hull_reach_sign(ctx, a->x, a->f, a->d);
}

/**
 * @brief Arguments of #hull_passes_pred
 */
typedef struct
{
  const HullAt *x, *f;
  const HullDir *from, *to;
} HullPassArg;

/**
 * @brief Return 1 where the centre of the interval J of #hull_passes_pred
 * lies strictly between its two directions, 0 where not, 2 where not told
 * @details The centre is the direction of D = C_F - C_X, strictly after the
 * first direction and before the second where n x D for the first and
 * D x n for the second are positive. Both must be, so the one whose direction
 * needs no root is read first: the normal of an end of one of two arcs given
 * in pieces tells the answer from the input points alone where the tangent
 * the walk arrived by would add three roots
 * @param[in] ctx Arithmetic
 * @param[in] a Question
 * @param[in] ox,oy Origin of the exact numbers
 * @param[in] dx,dy D times w_X w_F
 */
static int
hull_passes_centre(TowerCtx *ctx, const HullPassArg *a, double ox, double oy,
  TowerX dx, TowerX dy)
{
  Tower t;
  TowerX nx[TOWER_SIZE], ny[TOWER_SIZE], u[TOWER_SIZE], v[TOWER_SIZE];
  int first = (a->from->kind == HULL_DIR_TANGENT &&
    a->to->kind != HULL_DIR_TANGENT) ? 1 : 0;
  for (int i = 0; i < 2; i++)
  {
    int j = first ^ i;
    t.depth = 0;
    hull_dir_tower(ctx, j ? a->to : a->from, ox, oy, &t, nx, ny, NULL, NULL);
    int k = t.depth;
    tower_mulx_k(ctx, nx, dy, k, u);
    tower_mulx_k(ctx, ny, dx, k, v);
    tower_sub_k(ctx, u, v, k, u);
    int s = tower_sign_k(ctx, &t, u, k) * (j ? -1 : 1);
    if (s <= 0)
      return tower_ctx_unsafe(ctx) ? 2 : 0;
  }
  return tower_ctx_unsafe(ctx) ? 2 : 1;
}

/**
 * @brief Return 1 where the feature F reaches past the feature X at a
 * direction strictly between two others less than a half turn apart, X
 * reaching furthest of all at the first, 0 where it does not, and 2 where the
 * arithmetic does not tell
 * @details The directions where F reaches past X form one open interval J
 * centred on the direction of C_F - C_X, which is not empty where
 * r_F > r_X or |C_F - C_X| > r_X - r_F, and which does not hold the first
 * direction. J meets the interval between the two directions where it holds
 * the second (#hull_reach_sign), and otherwise where it lies within it, so
 * where it is not empty and its centre lies between the two, each a cross
 * product
 */
static int
hull_passes_pred(TowerCtx *ctx, const void *arg)
{
  const HullPassArg *a = arg;
  int s = hull_reach_sign(ctx, a->x, a->f, a->to);
  if (s != -1 && s != 0)
    return (s == 1) ? 1 : 2;
  /* J ends where the walk left F for X along their tangent, so that its
   * centre lies before that direction */
  if (a->from->kind == HULL_DIR_TANGENT && hull_at_same(&a->from->a, a->f) &&
      hull_at_same(&a->from->b, a->x))
    return tower_ctx_unsafe(ctx) ? 2 : 0;

  double ox, oy;
  hull_at_origin(a->x, &ox, &oy);
  HullExact ex, ef;
  hull_exact(ctx, a->x, ox, oy, &ex);
  hull_exact(ctx, a->f, ox, oy, &ef);
  TowerX dx = tx_sub(ctx, tx_mul(ctx, ef.px, ex.w), tx_mul(ctx, ex.px, ef.w));
  TowerX dy = tx_sub(ctx, tx_mul(ctx, ef.py, ex.w), tx_mul(ctx, ex.py, ef.w));
  Tower t;
  TowerX h[TOWER_SIZE], u[TOWER_SIZE], v[TOWER_SIZE];

  /* J is not empty: rho = (r_X - r_F) w_X w_F is negative, or
   * |D|^2 - rho^2 is positive. Two distinct circles through an input point
   * P cross there, so that |D| > |r_X - r_F|, unless they touch there from
   * one side (#hull_shared_tangent), where |D| = |r_X - r_F| and J is empty
   * where r_X^2 >= r_F^2, all signs with no root */
  double qx, qy;
  if (hull_shared_point(a->x, a->f, a->to, &qx, &qy))
  {
    HullExact cx, cf;
    hull_exact(ctx, a->x, qx, qy, &cx);
    hull_exact(ctx, a->f, qx, qy, &cf);
    if (hull_shared_tangent(ctx, &cx, &cf) &&
        tx_sign(ctx, tx_sub(ctx, tx_mul(ctx, cx.rw, tx_mul(ctx, cf.w, cf.w)),
          tx_mul(ctx, cf.rw, tx_mul(ctx, cx.w, cx.w)))) >= 0)
      return tower_ctx_unsafe(ctx) ? 2 : 0;
    return hull_passes_centre(ctx, a, ox, oy, dx, dy);
  }
  t.depth = 0;
  int ix = hull_tower_radius(ctx, &t, a->x, &ex);
  int i = hull_tower_radius(ctx, &t, a->f, &ef);
  int k = t.depth;
  tower_set(ctx, tx_val(ctx, 0.0), TOWER_MAXDEPTH, h);
  if (ix >= 0)
    h[1 << ix] = ef.w;
  if (i >= 0)
    h[1 << i] = tx_scale(ctx, ex.w, -1.0);
  if (tower_sign_k(ctx, &t, h, k) >= 0)
  {
    tower_mul_k(ctx, &t, h, h, k, u);
    tower_set(ctx, tx_add(ctx, tx_mul(ctx, dx, dx), tx_mul(ctx, dy, dy)), k,
      v);
    tower_sub_k(ctx, v, u, k, v);
    if (tower_sign_k(ctx, &t, v, k) <= 0)
      return tower_ctx_unsafe(ctx) ? 2 : 0;
  }

  return hull_passes_centre(ctx, a, ox, oy, dx, dy);
}

/**
 * @brief A direction of the walk in doubles with bounds on their errors: the
 * vector of an axis, an edge or the end of an arc and its length, or the two
 * features of a tangent
 */
typedef struct
{
  bool tangent;
  ArcsegFilter vx, vy, len;
  HullFilter a, b;
} HullDirFilter;

/**
 * @brief Set the vector of a direction in doubles with bounds on their
 * errors, and its length: that of the tangent from A to B is
 * rho D - sqrt(E) rot(D), of length |D|^2 (#hull_dir_tower)
 */
static void
hull_dir_filter(const HullDirFilter *d, ArcsegFilter *nx, ArcsegFilter *ny,
  ArcsegFilter *len)
{
  if (! d->tangent)
  {
    *nx = d->vx; *ny = d->vy; *len = d->len;
    return;
  }
  ArcsegFilter bx, by;
  hull_filter_from(&d->b, d->a.ox, d->a.oy, &bx, &by);
  ArcsegFilter dx = arcseg_f_sub(arcseg_f_mul(bx, d->a.w),
    arcseg_f_mul(d->a.px, d->b.w));
  ArcsegFilter dy = arcseg_f_sub(arcseg_f_mul(by, d->a.w),
    arcseg_f_mul(d->a.py, d->b.w));
  ArcsegFilter dd = arcseg_f_add(arcseg_f_mul(dx, dx), arcseg_f_mul(dy, dy));
  ArcsegFilter rho = arcseg_f_sub(arcseg_f_mul(d->a.r, d->b.w),
    arcseg_f_mul(d->b.r, d->a.w));
  ArcsegFilter s = tower_f_sqrt(arcseg_f_sub(dd, arcseg_f_mul(rho, rho)));
  *nx = arcseg_f_add(arcseg_f_mul(rho, dx), arcseg_f_mul(s, dy));
  *ny = arcseg_f_sub(arcseg_f_mul(rho, dy), arcseg_f_mul(s, dx));
  *len = dd;
  return;
}

/**
 * @brief Return the answer of #hull_passes_pred from doubles carrying bounds
 * on their errors, 2 where the bounds do not tell it, and 3 where they leave
 * untold only whether F reaches past X at the second direction
 * @details The four signs it reads are each true, false or not told, and
 * the answer is told where they tell it whatever the signs not told are
 */
static int
hull_passes_filter(const HullFilter *x, const HullFilter *f,
  const HullDirFilter *from, const HullDirFilter *to)
{
  ArcsegFilter fx, fy;
  hull_filter_from(f, x->ox, x->oy, &fx, &fy);
  ArcsegFilter dx = arcseg_f_sub(arcseg_f_mul(fx, x->w),
    arcseg_f_mul(x->px, f->w));
  ArcsegFilter dy = arcseg_f_sub(arcseg_f_mul(fy, x->w),
    arcseg_f_mul(x->py, f->w));
  ArcsegFilter nx, ny, len;
  hull_dir_filter(to, &nx, &ny, &len);
  /* F reaches past X at the second direction */
  ArcsegFilter rho = arcseg_f_sub(arcseg_f_mul(f->r, x->w),
    arcseg_f_mul(x->r, f->w));
  int s = hull_filter_sign(arcseg_f_add(arcseg_f_add(arcseg_f_mul(dx, nx),
    arcseg_f_mul(dy, ny)), arcseg_f_mul(rho, len)));
  if (s == 1)
    return 1;
  int reach = (s == 2) ? 2 : 0;
  /* J is not empty: r_X - r_F is negative or |D|^2 - (r_X - r_F)^2 is
   * positive, all times w_X w_F */
  int sr = hull_filter_sign(rho);
  int se = hull_filter_sign(arcseg_f_sub(arcseg_f_add(arcseg_f_mul(dx, dx),
    arcseg_f_mul(dy, dy)), arcseg_f_mul(rho, rho)));
  int nested = (sr == 1 || se == 1) ? 1 :
    ((sr == 2 || se == 2) ? 2 : 0);
  /* The centre of J is strictly after the first direction and before the
   * second */
  if (nested != 0)
  {
    ArcsegFilter mx, my, mlen;
    hull_dir_filter(from, &mx, &my, &mlen);
    int c0 = hull_filter_sign(arcseg_f_sub(arcseg_f_mul(mx, dy),
      arcseg_f_mul(my, dx)));
    int c1 = hull_filter_sign(arcseg_f_sub(arcseg_f_mul(dx, ny),
      arcseg_f_mul(dy, nx)));
    if ((c0 != 1 && c0 != 2) || (c1 != 1 && c1 != 2))
      nested = 0;
    else if (c0 == 2 || c1 == 2)
      nested = 2;
  }
  if (nested == 1)
    return 1;
  if (nested == 0)
    return (reach == 0) ? 0 : 3;
  return 2;
}

/**
 * @brief Return the half turn from (0, -1) a vector lies in: 0 from (0, -1)
 * up to (0, 1) excluded, 1 for the rest
 */
static int
hull_dir_half(TowerCtx *ctx, TowerX vx, TowerX vy)
{
  int s = tx_sign(ctx, vx);
  if (s != 0)
    return (s > 0) ? 0 : 1;
  return (tx_sign(ctx, vy) < 0) ? 0 : 1;
}

static int
hull_dir_order_pred(TowerCtx *ctx, const void *arg)
{
  const HullDir *const *d = arg;
  TowerX ux, uy, vx, vy;
  hull_dir_vector(ctx, d[0], &ux, &uy);
  hull_dir_vector(ctx, d[1], &vx, &vy);
  int hu = hull_dir_half(ctx, ux, uy), hv = hull_dir_half(ctx, vx, vy);
  int result = (hu != hv) ? ((hu < hv) ? -1 : 1) :
    - tx_sign(ctx, tx_sub(ctx, tx_mul(ctx, ux, vy), tx_mul(ctx, uy, vx)));
  return tower_ctx_unsafe(ctx) ? 2 : result;
}

/**
 * @brief Return -1, 0 or 1 as a direction given by rationals comes before, at
 * or after another, turning counterclockwise from (0, -1), decided exactly
 */
static int
hull_dir_order(const HullDir *u, const HullDir *v)
{
  const HullDir *d[2] = {u, v};
  return tower_pred_run(hull_dir_order_pred, d);
}

/**
 * @brief An event of the walk, a direction where the features that may
 * support the hull change
 */
typedef struct
{
  HullDir dir;
  int kind;              /**< 0 the end of the span of an arc, 1 an edge of
                              the hull of the points, 2 the start of the span
                              of an arc, 3 an axis: the order of the events of
                              one direction */
  int index;             /**< The arc or the edge */
  ArcsegFilter vx, vy;   /**< The vector of the direction in doubles, with a
                              bound on its error */
  ArcsegFilter len;      /**< Its length */
} HullEvent;

/**
 * @brief Set the vector of the direction of an event in doubles, with a
 * bound on its error (#hull_dir_vector)
 */
static void
hull_event_filter(HullEvent *e)
{
  TowerCtx ctx;
  tower_ctx_init(&ctx, false);
  ctx.filter = true;
  TowerX vx, vy;
  hull_dir_vector(&ctx, &e->dir, &vx, &vy);
  e->vx = vx.f;
  e->vy = vy.f;
  if (e->dir.kind == HULL_DIR_AXIS)
  {
    e->len.v = 1.0;
    e->len.e = 0.0;
  }
  else
    e->len = tower_f_sqrt(arcseg_f_add(arcseg_f_mul(e->vx, e->vx),
      arcseg_f_mul(e->vy, e->vy)));
  tower_ctx_free(&ctx);
  return;
}

/**
 * @brief Set the direction of an event in doubles with bounds on their errors
 */
static void
hull_event_dir_filter(const HullEvent *e, HullDirFilter *out)
{
  out->tangent = false;
  out->vx = e->vx;
  out->vy = e->vy;
  out->len = e->len;
  return;
}

/**
 * @brief Return the half turn from (0, -1) a vector in doubles lies in
 * (#hull_dir_half), or 2 where its bound does not tell it
 */
static int
hull_filter_half(ArcsegFilter vx, ArcsegFilter vy)
{
  int s = hull_filter_sign(vx);
  if (s == 2)
    return 2;
  if (s != 0)
    return (s > 0) ? 0 : 1;
  s = hull_filter_sign(vy);
  return (s == 2) ? 2 : ((s < 0) ? 0 : 1);
}

/**
 * @brief Comparator of two events: their directions in doubles where the
 * bounds tell them apart, exactly otherwise (#hull_dir_order)
 */
static inline int
hull_event_cmp(const HullEvent *ea, const HullEvent *eb)
{
  int result = 2;
  int ha = hull_filter_half(ea->vx, ea->vy);
  int hb = hull_filter_half(eb->vx, eb->vy);
  if (ha != 2 && hb != 2)
  {
    if (ha != hb)
      result = (ha < hb) ? -1 : 1;
    else
    {
      int s = hull_filter_sign(arcseg_f_sub(arcseg_f_mul(ea->vx, eb->vy),
        arcseg_f_mul(ea->vy, eb->vx)));
      if (s != 2)
        result = - s;
    }
  }
  if (result == 2)
    result = hull_dir_order(&ea->dir, &eb->dir);
  if (result != 0)
    return result;
  if (ea->kind != eb->kind)
    return (ea->kind < eb->kind) ? -1 : 1;
  return (ea->index < eb->index) ? -1 : ((ea->index > eb->index) ? 1 : 0);
}

/* Sort the events with their order written into the sort, as #point2d_sort
 * sorts the points */
#define ST_SORT hull_event_sort
#define ST_ELEMENT_TYPE HullEvent
#define ST_COMPARE(a, b) hull_event_cmp(a, b)
#define ST_SCOPE static
#define ST_DECLARE
#define ST_DEFINE
#include "port/sort_template.h"

/**
 * @brief Return a value as a double, setting @p loose where it is read in
 * doubles or pairs of doubles carrying a bound on its error and that bound
 * exceeds 64 epsilons of it
 * @details A value formed exactly is rounded once (#tx_filter); one read
 * within 64 epsilons builds a point of the boundary to a few roundings, as
 * the exact one does
 */
static double
hull_read(const TowerCtx *ctx, TowerX x, bool *loose)
{
  ArcsegFilter f = tx_filter(ctx, x);
  if ((ctx->filter || ctx->dd) && ! (f.e <= 64.0 * DBL_EPSILON * fabs(f.v)))
    *loose = true;
  return f.v;
}

/**
 * @brief Set the unit vector of #hull_tangent_unit in the arithmetic of a
 * context, and return true where a value it reads is loose (#hull_read)
 */
static bool
hull_tangent_unit_in(TowerCtx *ctx, const HullDir *d, bool shared, double ox,
  double oy, double *ux, double *uy)
{
  bool loose = false;
  HullExact ea, eb;
  hull_exact(ctx, &d->a, ox, oy, &ea);
  hull_exact(ctx, &d->b, ox, oy, &eb);
  TowerX dx = tx_sub(ctx, tx_mul(ctx, eb.px, ea.w),
    tx_mul(ctx, ea.px, eb.w));
  TowerX dy = tx_sub(ctx, tx_mul(ctx, eb.py, ea.w),
    tx_mul(ctx, ea.py, eb.w));
  TowerX ua = tx_mul(ctx, ea.rw, tx_mul(ctx, eb.w, eb.w));
  TowerX ub = tx_mul(ctx, eb.rw, tx_mul(ctx, ea.w, ea.w));
  double va = hull_read(ctx, ua, &loose), vb = hull_read(ctx, ub, &loose);
  double vx = hull_read(ctx, dx, &loose), vy = hull_read(ctx, dy, &loose);
  double den = sqrt(va) + sqrt(vb);
  /* rho enters the vector as rho D beside sqrt(E) rot(D), |D| the larger,
   * so that it needs its error small against |D| rather than against
   * itself, as two arcs of nearly one radius make it */
  double rho = 0.0;
  if (den > 0.0)
  {
    ArcsegFilter q = tx_filter(ctx, tx_sub(ctx, ua, ub));
    if ((ctx->filter || ctx->dd) &&
        ! (q.e <= 64.0 * DBL_EPSILON * den * hypot(vx, vy)))
      loose = true;
    rho = q.v / den;
  }
  double a, e, s = 2.0 * sqrt(va) * sqrt(vb);
  if (shared)
  {
    TowerX ww = tx_mul(ctx, ea.w, eb.w);
    a = -2.0 * hull_read(ctx, tx_mul(ctx, ww, tx_add(ctx,
      tx_mul(ctx, ea.px, eb.px), tx_mul(ctx, ea.py, eb.py))), &loose);
    double c = hull_read(ctx, tx_mul(ctx, ww, tx_sub(ctx,
      tx_mul(ctx, ea.px, eb.py), tx_mul(ctx, ea.py, eb.px))), &loose);
    e = (a >= 0.0) ? a + s : -4.0 * c * c / (a - s);
  }
  else
  {
    TowerX ta = tx_sub(ctx, tx_add(ctx, tx_mul(ctx, dx, dx),
      tx_mul(ctx, dy, dy)), tx_add(ctx, ua, ub));
    a = hull_read(ctx, ta, &loose);
    e = (a >= 0.0) ? a + s :
      hull_read(ctx, tx_sub(ctx, tx_mul(ctx, ta, ta),
        tx_scale(ctx, tx_mul(ctx, ua, ub), 4.0)), &loose) / (a - s);
  }
  double se = sqrt(fmax(e, 0.0));
  double nx = rho * vx + se * vy, ny = rho * vy - se * vx;
  double len = hypot(nx, ny);
  *ux = nx / len;
  *uy = ny / len;
  return loose;
}

/**
 * @brief Return in doubles the unit vector of the tangent from one feature to
 * another (#hull_dir_tower), each quantity that cancels formed exactly and
 * rounded once
 * @details The centres of two arcs of nearly one circle, as the arcs of a
 * circle given in pieces, lie far less than a rounding of their coordinates
 * apart, so a tangent read from the centres in doubles turns by as much as it
 * likes. Formed exactly, D is the difference the input points fix. So are
 * the numerators of rho = (u_A - u_B) / (sqrt(u_A) + sqrt(u_B)) and, where
 * A = |D|^2 - u_A - u_B is negative,
 * E = A + 2 sqrt(u_A u_B) = (A^2 - 4 u_A u_B) / (A - 2 sqrt(u_A u_B)), with
 * u_A = r_A^2 w_A^2 w_B^2 and u_B = r_B^2 w_B^2 w_A^2: no denominator
 * cancels, and the vector is then read to a few roundings. Two arcs through
 * an input point P, from which their centres are a / w_A and b / w_B, have
 * A = -2 w_A w_B a . b and A^2 - 4 u_A u_B = -4 (w_A w_B a x b)^2, the
 * products of a lower degree that #hull_shared_tangent reads. Each quantity
 * is read first in doubles, then in pairs of doubles, carrying a bound on its
 * error, and formed exactly only where that bound exceeds a few roundings of
 * it (#hull_read)
 */
static void
hull_tangent_unit(const HullDir *d, double *ux, double *uy)
{
  double ox, oy;
  bool shared = hull_shared_point(&d->a, &d->b, d, &ox, &oy);
  if (! shared)
    hull_at_origin(&d->a, &ox, &oy);
  TowerCtx ctx;
  for (int pass = 0; pass < 3; pass++)
  {
    tower_ctx_init(&ctx, false);
    ctx.filter = (pass == 0);
    ctx.dd = (pass == 1);
    bool loose = hull_tangent_unit_in(&ctx, d, shared, ox, oy, ux, uy);
    tower_ctx_free(&ctx);
    if (! loose)
      break;
  }
  return;
}

/**
 * @brief Return in doubles the centre and the radius of the circle of an
 * arc, each formed exactly and rounded once
 * @details The circle an arc edge carries is read from its three points in
 * doubles, which for three points nearly on one line, a circle far larger
 * than they lie apart, errs by far more than a rounding, and a point of the
 * boundary built from it far from those points carries that error. The
 * numerators and the denominator of the centre (#hull_exact) are exact and
 * rounded once before the division, as #mec_circle forms the circle of three
 * points, which goes on to the nearest doubles
 */
static void
hull_circle_round(const Edge *arc, double *cx, double *cy, double *r)
{
  HullAt f = {arc, 0.0, 0.0};
  double ox, oy;
  hull_at_origin(&f, &ox, &oy);
  TowerCtx ctx;
  /* In doubles, then in pairs of doubles, carrying bounds on their errors
   * first (#hull_read) */
  for (int pass = 0; pass < 3; pass++)
  {
    tower_ctx_init(&ctx, false);
    ctx.filter = (pass == 0);
    ctx.dd = (pass == 1);
    bool loose = false;
    HullExact e;
    hull_exact(&ctx, &f, ox, oy, &e);
    double w = hull_read(&ctx, e.w, &loose);
    *cx = ox + hull_read(&ctx, e.px, &loose) / w;
    *cy = oy + hull_read(&ctx, e.py, &loose) / w;
    *r = sqrt(hull_read(&ctx, e.rw, &loose)) / w;
    tower_ctx_free(&ctx);
    if (! loose)
      break;
  }
  return;
}

/**
 * @brief The circles of the arcs of a walk in doubles (#hull_circle_round),
 * each formed the first time it is read
 */
typedef struct
{
  const Edge **arcs;
  int narcs;
  double (*c)[3];        /**< The centre and the radius */
  bool *set;
} HullCircles;

/**
 * @brief Return in doubles the centre and the radius of the circle of an arc
 * of a walk, formed once (#hull_circle_round)
 */
static void
hull_circle_get(HullCircles *hc, const Edge *arc, double *cx, double *cy,
  double *r)
{
  int j = 0;
  while (j < hc->narcs && hc->arcs[j] != arc)
    j++;
  assert(j < hc->narcs);
  if (! hc->set[j])
  {
    hull_circle_round(arc, &hc->c[j][0], &hc->c[j][1], &hc->c[j][2]);
    hc->set[j] = true;
  }
  *cx = hc->c[j][0]; *cy = hc->c[j][1]; *r = hc->c[j][2];
  return;
}

/**
 * @brief Return in doubles the point where a feature touches the line of a
 * unit normal, on the circle of #hull_circle_round
 */
static void
hull_touch(HullCircles *hc, const HullAt *f, double ux, double uy, double *x,
  double *y)
{
  if (! f->arc)
  {
    *x = f->x; *y = f->y;
    return;
  }
  double cx, cy, r;
  hull_circle_get(hc, f->arc, &cx, &cy, &r);
  *x = cx + r * ux;
  *y = cy + r * uy;
  return;
}

/**
 * @brief A piece of the boundary the walk collects, with the arc it lies on
 * and its start and sweep about the centre
 */
typedef struct
{
  HullBoundary b;
  const Edge *arc;
  double cx, cy, r;      /**< The circle of the arc (#hull_circle_round) */
  double theta, sweep;
} HullPiece;

/**
 * @brief The pieces the walk has collected
 */
typedef struct
{
  HullPiece *p;
  int n, cap;
} HullPieces;

static HullPiece *
hull_piece_add(HullPieces *ps, double x1, double y1, double x2, double y2)
{
  if (ps->n == ps->cap)
  {
    ps->cap = ps->cap ? 2 * ps->cap : 16;
    ps->p = ps->p ? repalloc(ps->p, sizeof(HullPiece) * ps->cap) :
      palloc(sizeof(HullPiece) * ps->cap);
  }
  HullPiece *p = &ps->p[ps->n++];
  p->b.x1 = x1; p->b.y1 = y1;
  p->b.x2 = x2; p->b.y2 = y2;
  p->b.xm = p->b.ym = 0.0;
  p->b.is_arc = false;
  p->arc = NULL;
  p->cx = p->cy = p->r = 0.0;
  p->theta = p->sweep = 0.0;
  return p;
}

/**
 * @brief Add the segment between two points, unless they are one
 */
static void
hull_piece_segment(HullPieces *ps, double x1, double y1, double x2, double y2)
{
  if (x1 != x2 || y1 != y2)
    (void) hull_piece_add(ps, x1, y1, x2, y2);
  return;
}

/**
 * @brief Add the piece of an arc between two of its points, walked
 * counterclockwise across @p naxes axes, unless they are one
 * @details The sweep in doubles is read about the centre of the arc
 * (#hull_circle_get), within a half turn of the quarters of the axes
 * crossed, which the true sweep lies within a quarter turn of
 */
static void
hull_piece_arc(HullPieces *ps, HullCircles *hc, const Edge *arc, double x1,
  double y1, double x2, double y2, int naxes)
{
  if (x1 == x2 && y1 == y2)
    return;
  HullPiece *p = hull_piece_add(ps, x1, y1, x2, y2);
  p->b.is_arc = true;
  p->arc = arc;
  hull_circle_get(hc, arc, &p->cx, &p->cy, &p->r);
  p->theta = atan2(y1 - p->cy, x1 - p->cx);
  double sweep = atan2(y2 - p->cy, x2 - p->cx) - p->theta;
  double base = naxes * M_PI_2;
  while (sweep <= base - M_PI)
    sweep += 2 * M_PI;
  while (sweep > base + M_PI)
    sweep -= 2 * M_PI;
  p->sweep = sweep;
  return;
}

/**
 * @brief Hand the hull from a point to an arc whose span holds the window and
 * which starts at that point, as the start of its span does (#hull_walk)
 * @details The arc reaches past its first end at every direction of its span
 * after the one of that end, so the point supports the hull no further. The
 * walk reaches the point there at the direction the arc starts or along a
 * tangent touching both, where the two are one
 */
static void
hull_walk_handover(HullAt *x, HullFilter *xf, const HullAt *first,
  const Edge **arcs, const HullFilter *arcf, const bool *active, int narcs,
  int *naxes)
{
  if (x->arc)
    return;
  for (int j = 0; j < narcs; j++)
    if (active[j] && first[j].x == x->x && first[j].y == x->y)
    {
      x->arc = arcs[j];
      *xf = arcf[j];
      *naxes = 0;
      return;
    }
  return;
}

/**
 * @brief Return the boundary of the convex hull of points and arcs, walked
 * counterclockwise
 * @param[in] hull,nhull Vertices of the hull of the points, counterclockwise,
 * at least one
 * @param[in] arcs,narcs Arcs reaching past that hull, their ends among the
 * points
 * @param[out] npieces Number of pieces
 */
static HullBoundary *
hull_walk(const POINT2D *hull, uint32_t nhull, const Edge **arcs, int narcs,
  int *npieces)
{
  /* The ends of every arc, first and last in the order of its normals: the
   * order of the arc where it turns counterclockwise */
  HullAt *first = palloc(sizeof(HullAt) * narcs);
  HullAt *last = palloc(sizeof(HullAt) * narcs);
  for (int j = 0; j < narcs; j++)
  {
    const Edge *e = arcs[j];
    bool ccw = (e->centred || e->diameter) ? e->ccw :
      cross_product_sign(e->sx, e->sy, e->mx, e->my, e->sx, e->sy, e->ex,
        e->ey) > 0;
    first[j].arc = last[j].arc = e;
    first[j].x = ccw ? e->x1 : e->x2; first[j].y = ccw ? e->y1 : e->y2;
    last[j].x = ccw ? e->x2 : e->x1; last[j].y = ccw ? e->y2 : e->y1;
  }

  /* The events, in their order from (0, -1) */
  static const double axes[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
  int nedges = (nhull >= 2) ? (int) nhull : 0;
  int nev = 4 + nedges + 2 * narcs;
  HullEvent *ev = palloc(sizeof(HullEvent) * nev);
  int n = 0;
  for (int i = 0; i < 4; i++)
  {
    ev[n].dir.kind = HULL_DIR_AXIS;
    ev[n].dir.a.arc = NULL;
    ev[n].dir.a.x = axes[i][0]; ev[n].dir.a.y = axes[i][1];
    ev[n].kind = 3; ev[n].index = i;
    n++;
  }
  for (int i = 0; i < nedges; i++)
  {
    const POINT2D *p = &hull[i], *q = &hull[(i + 1) % nhull];
    ev[n].dir.kind = HULL_DIR_EDGE;
    ev[n].dir.a.arc = ev[n].dir.b.arc = NULL;
    ev[n].dir.a.x = p->x; ev[n].dir.a.y = p->y;
    ev[n].dir.b.x = q->x; ev[n].dir.b.y = q->y;
    ev[n].kind = 1; ev[n].index = i;
    n++;
  }
  for (int j = 0; j < narcs; j++)
  {
    ev[n].dir.kind = HULL_DIR_END;
    ev[n].dir.a = first[j];
    ev[n].kind = 2; ev[n].index = j;
    n++;
    ev[n].dir.kind = HULL_DIR_END;
    ev[n].dir.a = last[j];
    ev[n].kind = 0; ev[n].index = j;
    n++;
  }
  for (int i = 0; i < nev; i++)
    hull_event_filter(&ev[i]);
  hull_event_sort(ev, nev);

  /* The features in doubles with bounds on their errors, which decide most
   * questions before the towers are built (#hull_passes_filter) */
  HullFilter *arcf = palloc(sizeof(HullFilter) * narcs);
  for (int j = 0; j < narcs; j++)
  {
    HullAt f = {arcs[j], 0.0, 0.0};
    hull_filter_set(&f, &arcf[j]);
  }

  /* The arcs whose span holds (0, -1): those whose span wraps past it, the
   * events at (0, -1) starting or ending the others */
  bool *active = palloc(sizeof(bool) * narcs);
  for (int j = 0; j < narcs; j++)
  {
    HullDir s = {HULL_DIR_END, first[j], first[j]};
    HullDir e = {HULL_DIR_END, last[j], last[j]};
    active[j] = hull_dir_order(&s, &e) >= 0;
  }

  /* The feature reaching furthest along (0, -1): the vertex of the hull of
   * the points supporting it just before, the lowest then leftmost, or an
   * arc reaching further */
  uint32_t vi = 0;
  for (uint32_t i = 1; i < nhull; i++)
    if (hull[i].y < hull[vi].y ||
        (hull[i].y == hull[vi].y && hull[i].x < hull[vi].x))
      vi = i;
  HullAt x = {NULL, hull[vi].x, hull[vi].y};
  HullDir down = ev[0].dir;
  HullDirFilter downf;
  hull_event_dir_filter(&ev[0], &downf);
  for (int i = 0; i < nev; i++)
    if (ev[i].kind == 3 && ev[i].index == 0)
    {
      down = ev[i].dir;
      hull_event_dir_filter(&ev[i], &downf);
    }
  for (int j = 0; j < narcs; j++)
  {
    if (! active[j])
      continue;
    HullAt f = {arcs[j], 0.0, 0.0};
    HullReachArg ra = {&x, &f, &down};
    if (tower_pred_run(hull_reach_pred, &ra) > 0)
      x = f;
  }
  double px = x.arc ? x.arc->cx : x.x;
  double py = x.arc ? x.arc->cy - x.arc->radius : x.y;
  double startx = px, starty = py;
  bool start_arc = (x.arc != NULL);

  HullPieces ps = {NULL, 0, 0};
  HullCircles hc = {arcs, narcs, palloc(sizeof(double[3]) * Max(narcs, 1)),
    palloc0(sizeof(bool) * Max(narcs, 1))};
  HullDir from = down;
  HullFilter xf;
  hull_filter_set(&x, &xf);
  HullDirFilter fromf = downf;
  int naxes = 0;
  /* Every step leaves a feature for another at a later direction, and a
   * feature supports the hull along at most as many pieces as there are
   * features */
  int maxsteps = 4 * (nev + narcs + (int) nhull) * (narcs + 2);
  int nsteps = 0;
  for (int i = 0; i <= nev; i++)
  {
    const HullDir *to = (i < nev) ? &ev[i].dir : &down;
    HullDirFilter tof = downf;
    if (i < nev)
      hull_event_dir_filter(&ev[i], &tof);
    /* The feature first reaching past X before the end of the window */
    while (true)
    {
      HullAt best = x, v = {NULL, hull[vi].x, hull[vi].y};
      HullFilter vf, bestf = xf;
      hull_filter_set(&v, &vf);
      HullDir tangent;
      HullDirFilter tangentf;
      bool found = false;
      for (int j = -1; j < narcs; j++)
      {
        HullAt f = v;
        const HullFilter *ff = &vf;
        if (j >= 0)
        {
          if (! active[j])
            continue;
          f.arc = arcs[j];
          ff = &arcf[j];
        }
        if (hull_at_same(&f, &x))
          continue;
        /* A point of the circle of X reaches past it in no direction
         * (#point_on_arc_circle), which an end of the arc tells at no cost */
        if (x.arc && ! f.arc && point_on_arc_circle(x.arc, f.x, f.y))
          continue;
        int r = hull_passes_filter(&xf, ff, &fromf, found ? &tangentf : &tof);
        if (r == 3)
        {
          /* Only how far F reaches at the end of the interval is untold */
          HullReachArg ra = {&x, &f, found ? &tangent : to};
          r = (tower_pred_run(hull_reach_pred, &ra) > 0) ? 1 : 0;
        }
        else if (r == 2)
        {
          HullPassArg pa = {&x, &f, &from, found ? &tangent : to};
          r = tower_pred_run(hull_passes_pred, &pa);
        }
        if (r == 1)
        {
          best = f;
          bestf = *ff;
          tangent.kind = HULL_DIR_TANGENT;
          tangent.a = x;
          tangent.b = f;
          tangentf.tangent = true;
          tangentf.a = xf;
          tangentf.b = *ff;
          found = true;
        }
      }
      if (! found)
        break;
      if (++nsteps > maxsteps)
      {
        pfree(first); pfree(last); pfree(ev); pfree(active); pfree(arcf);
        pfree(hc.c); pfree(hc.set);
        if (ps.p)
          pfree(ps.p);
        *npieces = 0;
        meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
          "The walk of the boundary of a convex hull does not end");
        return NULL;
      }
      /* Follow the tangent from X to the feature */
      double ux, uy, tx, ty, fx, fy;
      hull_tangent_unit(&tangent, &ux, &uy);
      hull_touch(&hc, &x, ux, uy, &tx, &ty);
      hull_touch(&hc, &best, ux, uy, &fx, &fy);
      if (x.arc)
        hull_piece_arc(&ps, &hc, x.arc, px, py, tx, ty, naxes);
      else
      {
        tx = px; ty = py;
      }
      hull_piece_segment(&ps, tx, ty, fx, fy);
      px = fx; py = fy;
      x = best;
      xf = bestf;
      from = tangent;
      fromf = tangentf;
      naxes = 0;
      hull_walk_handover(&x, &xf, first, arcs, arcf, active, narcs, &naxes);
    }
    if (i == nev)
      break;

    const HullEvent *e = &ev[i];
    int j = e->index;
    if (e->kind == 0)
    {
      /* The span of the arc ends, where it hands the hull to its end */
      active[j] = false;
      if (x.arc == arcs[j])
      {
        hull_piece_arc(&ps, &hc, x.arc, px, py, last[j].x, last[j].y, naxes);
        px = last[j].x; py = last[j].y;
        x.arc = NULL; x.x = px; x.y = py;
        hull_filter_set(&x, &xf);
        hull_walk_handover(&x, &xf, first, arcs, arcf, active, narcs, &naxes);
      }
    }
    else if (e->kind == 1)
    {
      /* The hull of the points passes to its next vertex */
      vi = (uint32_t) (j + 1) % nhull;
      if (! x.arc)
      {
        hull_piece_segment(&ps, px, py, hull[vi].x, hull[vi].y);
        px = hull[vi].x; py = hull[vi].y;
        x.x = px; x.y = py;
        hull_filter_set(&x, &xf);
        hull_walk_handover(&x, &xf, first, arcs, arcf, active, narcs, &naxes);
      }
    }
    else if (e->kind == 2)
    {
      /* The span of the arc starts, where its first end supports the hull */
      active[j] = true;
      hull_walk_handover(&x, &xf, first, arcs, arcf, active, narcs, &naxes);
    }
    else if (x.arc)
      naxes++;
    from = e->dir;
    hull_event_dir_filter(e, &fromf);
  }
  /* Back where the walk started */
  if (x.arc)
    hull_piece_arc(&ps, &hc, x.arc, px, py, startx, starty, naxes);
  else
    hull_piece_segment(&ps, px, py, startx, starty);
  /* A walk starting on an arc splits it where it starts, and its last piece
   * and its first are one */
  if (start_arc && ps.n >= 2 && ps.p[0].arc &&
      ps.p[0].arc == ps.p[ps.n - 1].arc)
  {
    HullPiece *a = &ps.p[ps.n - 1], *b = &ps.p[0];
    b->b.x1 = a->b.x1; b->b.y1 = a->b.y1;
    b->theta = a->theta;
    b->sweep += a->sweep;
    ps.n--;
  }

  HullBoundary *result = palloc(sizeof(HullBoundary) * Max(ps.n, 1));
  for (int k = 0; k < ps.n; k++)
  {
    result[k] = ps.p[k].b;
    const Edge *arc = ps.p[k].arc;
    if (! arc)
      continue;
    /* A piece that is a whole arc of three input points passes through the
     * middle one */
    const HullBoundary *b = &ps.p[k].b;
    if (! arc->centred && ! arc->diameter &&
        ((b->x1 == arc->x1 && b->y1 == arc->y1 && b->x2 == arc->x2 &&
          b->y2 == arc->y2) ||
         (b->x1 == arc->x2 && b->y1 == arc->y2 && b->x2 == arc->x1 &&
          b->y2 == arc->y1)))
    {
      result[k].xm = arc->mx;
      result[k].ym = arc->my;
      continue;
    }
    double mid = ps.p[k].theta + ps.p[k].sweep / 2;
    result[k].xm = ps.p[k].cx + ps.p[k].r * cos(mid);
    result[k].ym = ps.p[k].cy + ps.p[k].r * sin(mid);
  }
  pfree(first); pfree(last); pfree(ev); pfree(active); pfree(arcf);
  pfree(hc.c); pfree(hc.set);
  if (ps.p)
    pfree(ps.p);
  *npieces = ps.n;
  return result;
}

/**
 * @brief Return the geometry of a hull boundary, walked clockwise
 * @details The pieces are walked backwards and each of them reversed, because
 * the boundary is collected counterclockwise while the ring of a hull is
 * reported clockwise, the order #make_geometry_hull reports the hull of a
 * geometry of segments in. Consecutive segments are gathered into one line so
 * that the ring alternates between a line and an arc
 */
static LWGEOM *
hull_boundary_geometry(int32_t srid, const HullBoundary *pieces, int npieces)
{
  LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);
  POINTARRAY *line = NULL;
  POINT4D p;
  p.z = p.m = 0.0;
  for (int k = npieces - 1; k >= 0; k--)
  {
    const HullBoundary *piece = &pieces[k];
    /* Reversed, the piece runs from the point it ends on */
    if (! piece->is_arc)
    {
      if (! line)
      {
        line = ptarray_construct_empty(0, 0, 4);
        p.x = piece->x2; p.y = piece->y2;
        ptarray_append_point(line, &p, LW_TRUE);
      }
      p.x = piece->x1; p.y = piece->y1;
      ptarray_append_point(line, &p, LW_TRUE);
      continue;
    }
    if (line)
    {
      lwcompound_add_lwgeom(ring, lwline_as_lwgeom(lwline_construct(srid,
        NULL, line)));
      line = NULL;
    }
    POINTARRAY *arc = ptarray_construct_empty(0, 0, 3);
    p.x = piece->x2; p.y = piece->y2;
    ptarray_append_point(arc, &p, LW_TRUE);
    p.x = piece->xm; p.y = piece->ym;
    ptarray_append_point(arc, &p, LW_TRUE);
    p.x = piece->x1; p.y = piece->y1;
    ptarray_append_point(arc, &p, LW_TRUE);
    lwcompound_add_lwgeom(ring, lwcircstring_as_lwgeom(
      lwcircstring_construct(srid, NULL, arc)));
  }
  if (line)
    lwcompound_add_lwgeom(ring, lwline_as_lwgeom(lwline_construct(srid, NULL,
      line)));

  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(result, lwcompound_as_lwgeom(ring));
  return lwcurvepoly_as_lwgeom(result);
}

/**
 * @brief Return the area of the rectangle of a given direction enclosing a
 * geometry, and its corners
 * @details The rectangle holds every point reaching no further than the
 * geometry does in each of the four directions, so its sides are the four
 * supports and its corners the points where they meet
 */
static double
mrr_arc_rectangle(const HullFeature *feats, int nfeats, double phi,
  POINT2D *rect)
{
  double h0 = hull_support(feats, nfeats, phi, NULL, NULL, NULL);
  double h1 = hull_support(feats, nfeats, phi + M_PI_2, NULL, NULL, NULL);
  double h2 = hull_support(feats, nfeats, phi + M_PI, NULL, NULL, NULL);
  double h3 = hull_support(feats, nfeats, phi + 3 * M_PI_2, NULL, NULL, NULL);
  double width = h0 + h2, height = h1 + h3;
  if (rect)
  {
    double ux = cos(phi), uy = sin(phi);
    double vx = -uy, vy = ux;
    const double a[4] = {h0, -h2, -h2, h0};
    const double b[4] = {h1, h1, -h3, -h3};
    for (int i = 0; i < 4; i++)
    {
      rect[i].x = a[i] * ux + b[i] * vx;
      rect[i].y = a[i] * uy + b[i] * vy;
    }
    rect[4] = rect[0];
  }
  return width * height;
}

/**
 * @brief Return the rectangle of minimum area enclosing a geometry carrying a
 * circular arc
 * @details The area of the enclosing rectangle turns with its direction and
 * repeats every quarter turn, and it is smooth between two directions in which
 * the feature supporting the hull changes. Every such direction is tried, and
 * the smooth stretch after each of them is searched for the direction where
 * the area is least
 */
static LWGEOM *
mrr_arc(const LWGEOM *geom, const MeosArray *edges)
{
  POINT2D *hull = NULL;
  uint32_t nhull = edges_points_hull(edges, NULL, NULL, &hull);
  int nfeats = 0;
  HullFeature *feats = hull_features(edges, hull, nhull, &nfeats);
  if (hull)
    pfree(hull);
  double *dirs;
  int ndirs = hull_directions(feats, nfeats, &dirs);

  /* The directions the support changes in, brought into the quarter turn the
   * area repeats over, with its two ends */
  double *cand = palloc(sizeof(double) * (4 * ndirs + 2));
  int ncand = 0;
  cand[ncand++] = 0.0;
  cand[ncand++] = M_PI_2;
  for (int i = 0; i < ndirs; i++)
    cand[ncand++] = fmod(dirs[i], M_PI_2);
  pfree(dirs);
  qsort(cand, ncand, sizeof(double), hull_direction_cmp);

  double best_area = DBL_MAX, best_phi = 0.0;
  for (int i = 0; i < ncand; i++)
  {
    double area = mrr_arc_rectangle(feats, nfeats, cand[i], NULL);
    if (area < best_area)
    {
      best_area = area;
      best_phi = cand[i];
    }
    if (i + 1 >= ncand || cand[i + 1] - cand[i] <= MEOS_GEOM_TOLERANCE)
      continue;
    /* The stretch between two consecutive directions carries no change of
     * support, so the area is smooth over it and is searched by narrowing the
     * interval it is least on */
    double lo = cand[i], hi = cand[i + 1];
    const double invphi = 0.6180339887498949;
    double c = hi - (hi - lo) * invphi, d = lo + (hi - lo) * invphi;
    double fc = mrr_arc_rectangle(feats, nfeats, c, NULL);
    double fd = mrr_arc_rectangle(feats, nfeats, d, NULL);
    for (int k = 0; k < 80 && hi - lo > 1e-15; k++)
    {
      if (fc < fd)
      {
        hi = d; d = c; fd = fc;
        c = hi - (hi - lo) * invphi;
        fc = mrr_arc_rectangle(feats, nfeats, c, NULL);
      }
      else
      {
        lo = c; c = d; fc = fd;
        d = lo + (hi - lo) * invphi;
        fd = mrr_arc_rectangle(feats, nfeats, d, NULL);
      }
    }
    double phi = (lo + hi) / 2;
    area = mrr_arc_rectangle(feats, nfeats, phi, NULL);
    /* The minimum is attained over a whole stretch of directions whenever the
     * hull is as wide every way, a circle being the plainest case, so a
     * direction the support changes in is kept over one the search lands on
     * unless it is genuinely smaller */
    if (area < best_area - fabs(best_area) * MEOS_GEOM_TOLERANCE)
    {
      best_area = area;
      best_phi = phi;
    }
  }

  POINT2D rect[5];
  (void) mrr_arc_rectangle(feats, nfeats, best_phi, rect);
  pfree(cand); pfree(feats);
  return make_geometry_points(geom->srid, rect, 4);
}

/**
 * @brief Return the oriented envelop (a.k.a. minimum-area rotated rectangle)
 * of a geometry
 * @details Works directly on the exact circular arcs represented by the Edge
 * structure.
 */
LWGEOM *
meos_oriented_envelope(const LWGEOM *geom)
{
  assert(geom);

  /* Empty input */
  if (lwgeom_is_empty(geom))
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));

  /* Extract the geometry */
  MeosArray *edge_array = geom_extract_edges(geom);
  uint32_t nedges = edge_array->count;
  if (nedges == 0)
  {
    meos_array_destroy(edge_array);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));
  }

  /* The placement below reads the vertices, which are the whole of a geometry
   * only while every edge of it is a segment. An arc reaches past the points
   * that define it, and is enclosed by reading how far its circle reaches */
  if (lwgeom_has_arc(geom))
  {
    LWGEOM *result = mrr_arc(geom, edge_array);
    meos_array_destroy(edge_array);
    return result;
  }

  /* The hull of the end points of the edges */
  POINT2D *hull = NULL;
  uint32_t nhull = edges_points_hull(edge_array, NULL, NULL, &hull);
  meos_array_destroy(edge_array);
  if (nhull == 0)
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));

  /* Degenerate cases */
  if (nhull == 1)
  {
    POINT2D rect[5];
    for (int i = 0; i < 5; i++)
      rect[i] = hull[0];
    LWGEOM *result = make_geometry_points(geom->srid, rect, nhull);
    pfree(hull);
    return result;
  }

  if (nhull == 2)
  {
    POINT2D rect[5];
    rect[0] = hull[0];
    rect[1] = hull[1];
    rect[2] = hull[1];
    rect[3] = hull[0];
    rect[4] = hull[0];
    LWGEOM *result = make_geometry_points(geom->srid, rect, nhull);
    pfree(hull);
    return result;
  }

  /*
   * Generate candidate orientations.
   * - For a polygonal convex hull these are simply the hull-edge
   *   orientations.
   * - For an arc, its tangent direction changes continuously. We therefore
   *   inspect the analytically significant tangent directions of the arc.
   * Because the rectangle orientation is periodic modulo pi, all angles are
   * normalized to [0,pi).
   */

  /*
   * Maximum number of candidate directions. Every hull edge contributes one
   * direction. The actual Edge array may have more entries than the hull, but
   * using a generous bound keeps this implementation simple.
   */
  uint32_t maxangles = 8 * nedges + 8 * nhull;
  double *angles = palloc(sizeof(double) * maxangles);
  uint32_t nangles = 0;

  /*
   * Re-extract the edges. This is inexpensive compared with the convex-hull
   * computation and keeps the code independent from any Edge pointer retained
   * after the first array was freed.
   */
  edge_array = geom_extract_edges(geom);
  nedges = edge_array->count;
  for (uint32_t i = 0; i < nedges; i++)
  {
    const Edge *e = (Edge *) meos_array_get_intl(edge_array, i);
    /* Only directions which can define a support side need to be considered */
    mrr_add_edge_directions(e, angles, &nangles);
  }
  meos_array_destroy(edge_array);

  /* Also add all convex-hull edge directions.
   * This is important because the hull itself is the object whose
   * bounding rectangle is being computed. */
  for (uint32_t i = 0; i < nhull; i++)
  {
    const POINT2D *a = &hull[i];
    const POINT2D *b = &hull[(i + 1) % nhull];
    double dx = b->x - a->x;
    double dy = b->y - a->y;
    if (hypot(dx, dy) <= MEOS_GEOM_TOLERANCE)
      continue;
    double angle = atan2(dy, dx);
    angle = fmod(angle, M_PI);
    if (angle < 0)
      angle += M_PI;
    angles[nangles++] = angle;
  }

  /* No candidate direction: the hull has no support side to align with */
  if (nangles == 0)
  {
    pfree(angles); pfree(hull);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));
  }

  /* Find the minimum-area rectangle. The rectangle of the first direction
   * seeds the result: a comparison against it is false for every direction
   * when a coordinate is not a number, and the rectangle must be defined in
   * that case too.
   * The minimum area does not name ONE rectangle. Every side of a triangle
   * carries a rectangle of area twice the triangle's, so the three tie exactly
   * and any point set whose hull has parallel support directions ties as well.
   * The tied rectangles differ, and they differ in the diagonal a caller reads
   * as the size of the region, so the shorter diagonal decides the tie: among
   * the rectangles of least area it is the one that encloses the points most
   * tightly, and it makes the answer a function of the points rather than of
   * the order the directions happen to be visited in. */
  double best_area = DBL_MAX, best_diag = DBL_MAX;
  POINT2D best_rect[5] = {0};
  for (uint32_t i = 0; i < nangles; i++)
  {
    double angle = angles[i];
    double ux = cos(angle);
    double uy = sin(angle);
    POINT2D rect[5];
    double area = mrr_rectangle_for_direction(hull, nhull, ux, uy, rect);
    double diag = hypot(rect[2].x - rect[0].x, rect[2].y - rect[0].y);
    /* Two areas within a part in a million of each other are one area here:
     * the rounding of a rotated projection is orders of magnitude smaller than
     * that, and a set whose rectangles differ by so little is one whose
     * tightest rectangle is the better answer either way */
    bool tied = fabs(area - best_area) <=
      MEOS_EPSILON * fmax(fabs(area), fabs(best_area));
    if (i == 0 || (tied ? diag < best_diag : area < best_area))
    {
      best_area = area;
      best_diag = diag;
      for (int j = 0; j < 5; j++)
        best_rect[j] = rect[j];
    }
  }

  /* Clean up and return */
  pfree(angles); pfree(hull);
  return make_geometry_points(geom->srid, best_rect, 4);
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the oriented envelope (a.k.a. minimum-area rotated rectangle)
 * of a geometry
 * @param[in] gs Geometry
 * @note PostGIS function: @p ST_OrientedEnvelope(PG_FUNCTION_ARGS).
 * @csqlfn #Geom_oriented_envelope()
 */
GSERIALIZED *
geom_oriented_envelope(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWGEOM *res = meos_oriented_envelope(lwgeom);
  GSERIALIZED *result = geo_serialize(res);
  lwgeom_free(lwgeom); lwgeom_free(res);
  return result;
}

/******************************************************************************/

/**
 * @brief Return the convex hull of a geometry
 * @details Works directly on the exact circular arcs represented by the Edge
 * structure.
 */
LWGEOM *
convex_hull(const LWGEOM *geom)
{
  assert(geom);

  /* Empty input */
  if (lwgeom_is_empty(geom))
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));

  /* Extract the geometry */
  MeosArray *edge_array = geom_extract_edges(geom);
  uint32_t nedges = edge_array->count;
  if (nedges == 0)
  {
    meos_array_destroy(edge_array);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));
  }

  /* The hull of the end points of the edges */
  POINT2D *points, *hull = NULL;
  uint32_t npoints;
  uint32_t nhull = edges_points_hull(edge_array, &points, &npoints, &hull);

  /* An arc reaches past the points that define it, and belongs to the hull as
   * the piece of its circle that no other feature reaches past: the boundary
   * is walked (#hull_walk) where an arc reaches past the hull of the points */
  LWGEOM *result = NULL;
  bool declined = false;
  if (nhull > 0 && lwgeom_has_arc(geom))
  {
    int narcs = 0, npieces = 0;
    const Edge **arcs = hull_points_arcs(edge_array, hull, nhull, &narcs);
    HullBoundary *pieces = (narcs > 0) ?
      hull_walk(hull, nhull, arcs, narcs, &npieces) : NULL;
    declined = (narcs > 0 && ! pieces);
    for (int i = 0; i < npieces && ! result; i++)
    {
      if (pieces[i].is_arc)
        result = hull_boundary_geometry(geom->srid, pieces, npieces);
    }
    if (pieces)
      pfree(pieces);
    pfree(arcs);
  }
  meos_array_destroy(edge_array);

  /* No arc reaches past the points, so the hull is the one they place */
  if (! result && ! declined)
  {
    if (nhull == 2)
      hull_order_as_input(points, npoints, hull);
    result = (nhull == 0) ?
      lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0)) :
      make_geometry_hull(geom->srid, hull, nhull);
  }
  pfree(points);
  if (hull)
    pfree(hull);
  return result;
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return the convex hull of a geometry
 * @param[in] gs Geometry
 * @note PostGIS function: @p ST_ConvexHull(PG_FUNCTION_ARGS). With respect to
 * the original function we do not use the @p prec argument.
 * @csqlfn #Geom_convex_hull()
 */
GSERIALIZED *
geom_convex_hull(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  /* Empty.ConvexHull() == Empty */
  if (gserialized_is_empty(gs))
    return geo_copy(gs);

  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  LWGEOM *res = convex_hull(lwgeom);
  GSERIALIZED *result = geo_serialize(res);
  lwgeom_free(lwgeom); lwgeom_free(res);
  return result;
}

/*****************************************************************************
 * Simple geometries
 * Implementation of the PostGIS function ST_IsSimple improving its
 * performance and answering it without GEOS
 * A geometry is simple when it has no anomalous point, which is a point at
 * which it crosses or touches itself. A point is always simple, a multipoint
 * is simple when it repeats no point, a line is simple when it meets itself
 * only where two of its segments follow one another and, when it closes, at
 * the point where it closes, and an areal geometry is simple when each of its
 * rings is. The components of a multipart geometry are answered one by one,
 * except that the lines of a multiline may meet only at a point that ends
 * both of them.
 *****************************************************************************/

/**
 * @brief Collect the points of a point array, dropping a point that repeats
 * the one before it
 * @details A repeated point contributes a segment of no length, which the
 * standard does not read as the geometry meeting itself
 * @param[in] pa Point array
 * @param[out] points Array receiving the points, of at least @p pa->npoints
 * elements
 * @return Number of points collected
 */
static int
pointarr_collect(const POINTARRAY *pa, const POINT2D **points)
{
  assert(pa); assert(points);
  int npoints = 0;
  for (uint32_t i = 0; i < pa->npoints; i++)
  {
    const POINT2D *point = getPoint2d_cp(pa, i);
    if (npoints == 0 ||
        point->x != points[npoints - 1]->x ||
        point->y != points[npoints - 1]->y)
      points[npoints++] = point;
  }
  return npoints;
}

/**
 * @brief Return true if a sequence of points closes on itself
 */
static bool
pointarr_is_closed(const POINT2D **points, int npoints)
{
  assert(points);
  return npoints > 1 && points[0]->x == points[npoints - 1]->x &&
    points[0]->y == points[npoints - 1]->y;
}

/**
 * @brief Return true if the polyline joining a sequence of points meets itself
 * nowhere it is not allowed to
 * @details Two segments that follow one another meet at the point they share,
 * and the first and the last segment of a closed polyline meet at the point
 * that closes it. Meeting anywhere else, meeting at more than a point, or an
 * end of one segment falling inside another, is the geometry crossing or
 * touching itself.
 * @param[in] points Array of points, holding no point that repeats the one
 * before it
 * @param[in] npoints Number of elements in the array of points
 */
static bool
pointarr_is_simple(const POINT2D **points, int npoints)
{
  assert(points);
  /* A single point and a single segment cannot meet themselves */
  if (npoints < 3)
    return true;
  const bool closed = pointarr_is_closed(points, npoints);
  const int nsegs = npoints - 1;
  for (int i = 0; i < nsegs; i++)
  {
    for (int j = i + 1; j < nsegs; j++)
    {
      POINT2D p = { 0 }; /* make compiler quiet */
      int intertype = seg2d_intersection(points[i], points[i + 1],
        points[j], points[j + 1], &p);
      if (intertype == MEOS_SEG_NO_INTERSECTION)
        continue;
      /* Two segments that follow one another meet at the point they share */
      if (intertype == MEOS_SEG_TOUCH_END && j == i + 1 &&
          p.x == points[j]->x && p.y == points[j]->y)
        continue;
      /* The two ends of a closed polyline meet at the point that closes it */
      if (intertype == MEOS_SEG_TOUCH_END && closed && i == 0 &&
          j == nsegs - 1 && p.x == points[0]->x && p.y == points[0]->y)
        continue;
      return false;
    }
  }
  return true;
}

/**
 * @brief Return true if two polylines meet only at a point that ends both of
 * them
 * @details A closed polyline has no end, so a closed one meeting another
 * polyline anywhere is the pair touching itself.
 * @param[in] points1,points2 Arrays of points
 * @param[in] npoints1,npoints2 Number of elements in the arrays of points
 */
static bool
pointarrs_meet_at_ends(const POINT2D **points1, int npoints1,
  const POINT2D **points2, int npoints2)
{
  assert(points1); assert(points2);
  if (npoints1 < 2 || npoints2 < 2)
    return true;
  const bool closed1 = pointarr_is_closed(points1, npoints1);
  const bool closed2 = pointarr_is_closed(points2, npoints2);
  for (int i = 0; i < npoints1 - 1; i++)
  {
    for (int j = 0; j < npoints2 - 1; j++)
    {
      POINT2D p = { 0 }; /* make compiler quiet */
      int intertype = seg2d_intersection(points1[i], points1[i + 1],
        points2[j], points2[j + 1], &p);
      if (intertype == MEOS_SEG_NO_INTERSECTION)
        continue;
      /* Meeting along a stretch, or at a point inside either segment, is not
       * the two ends meeting */
      if (intertype != MEOS_SEG_TOUCH_END || closed1 || closed2)
        return false;
      /* The point ends both polylines, not only the two segments carrying it */
      if (! ((p.x == points1[0]->x && p.y == points1[0]->y) ||
             (p.x == points1[npoints1 - 1]->x &&
              p.y == points1[npoints1 - 1]->y)))
        return false;
      if (! ((p.x == points2[0]->x && p.y == points2[0]->y) ||
             (p.x == points2[npoints2 - 1]->x &&
              p.y == points2[npoints2 - 1]->y)))
        return false;
    }
  }
  return true;
}

/**
 * @brief Return true if a point array is simple as a polyline of its own
 */
static bool
ptarray_is_simple(const POINTARRAY *pa)
{
  assert(pa);
  if (pa->npoints == 0)
    return true;
  const POINT2D **points = palloc(sizeof(POINT2D *) * pa->npoints);
  int npoints = pointarr_collect(pa, points);
  bool result = pointarr_is_simple(points, npoints);
  pfree(points);
  return result;
}

/**
 * @brief Return true if the points of a multipoint are all distinct
 */
static bool
lwmpoint_is_simple(const LWMPOINT *mpoint)
{
  assert(mpoint);
  for (uint32_t i = 0; i < mpoint->ngeoms; i++)
  {
    const LWPOINT *point1 = mpoint->geoms[i];
    if (! point1 || lwpoint_is_empty(point1))
      continue;
    const POINT2D *p1 = getPoint2d_cp(point1->point, 0);
    for (uint32_t j = i + 1; j < mpoint->ngeoms; j++)
    {
      const LWPOINT *point2 = mpoint->geoms[j];
      if (! point2 || lwpoint_is_empty(point2))
        continue;
      const POINT2D *p2 = getPoint2d_cp(point2->point, 0);
      if (p1->x == p2->x && p1->y == p2->y)
        return false;
    }
  }
  return true;
}

/**
 * @brief Return true if the lines of a multiline are each simple and meet one
 * another only where they end
 */
static bool
lwmline_is_simple(const LWMLINE *mline)
{
  assert(mline);
  const POINT2D ***points = palloc0(sizeof(POINT2D **) * mline->ngeoms);
  int *npoints = palloc0(sizeof(int) * mline->ngeoms);
  bool result = true;
  for (uint32_t i = 0; i < mline->ngeoms && result; i++)
  {
    const LWLINE *line = mline->geoms[i];
    if (! line || ! line->points || line->points->npoints == 0)
      continue;
    points[i] = palloc(sizeof(POINT2D *) * line->points->npoints);
    npoints[i] = pointarr_collect(line->points, points[i]);
    result = pointarr_is_simple(points[i], npoints[i]);
  }
  for (uint32_t i = 0; i < mline->ngeoms && result; i++)
  {
    if (! points[i])
      continue;
    for (uint32_t j = i + 1; j < mline->ngeoms && result; j++)
    {
      if (! points[j])
        continue;
      result = pointarrs_meet_at_ends(points[i], npoints[i], points[j],
        npoints[j]);
    }
  }
  for (uint32_t i = 0; i < mline->ngeoms; i++)
    if (points[i])
      pfree(points[i]);
  pfree(points); pfree(npoints);
  return result;
}

/**
 * @brief Return true if every ring of an areal geometry is simple
 */
static bool
lwpoly_is_simple(const LWPOLY *poly)
{
  assert(poly);
  for (uint32_t i = 0; i < poly->nrings; i++)
    if (poly->rings[i] && ! ptarray_is_simple(poly->rings[i]))
      return false;
  return true;
}

static bool relate_edges_meet(const Edge *a, const Edge *b);
static int relate_any_edge_intersection(const Edge *a, const Edge *b,
  double ix[2], double iy[2]);
static bool relate_same_point(double x1, double y1, double x2, double y2);

/**
 * @brief Return true if the edges one curve draws meet only where consecutive
 * edges share their endpoint
 * @details A curve is simple when it passes through no point twice, the two
 * ends of a closed one excepted. Consecutive edges share a vertex by
 * construction, so for them the question is whether they meet ANYWHERE ELSE:
 * a second isolated point, or a span they run along together. Every other
 * pair meets nowhere at all. #relate_edges_meet reads the intersection type
 * rather than a count, so two arcs of one circle sharing a span answer here,
 * which is what a segment intersection cannot see
 */
static bool
meos_curve_edges_simple(Edge **edges, int nedges)
{
  /* The two ends are input vertices, so the curve is closed exactly when they
   * are the same point */
  bool closed = nedges > 1 &&
    edges[0]->x1 == edges[nedges - 1]->x2 &&
    edges[0]->y1 == edges[nedges - 1]->y2;
  for (int i = 0; i < nedges; i++)
    for (int j = i + 1; j < nedges; j++)
    {
      /* The vertices the two legitimately share. Consecutive edges share the
       * one between them, and the two ends of a closed curve share where it
       * closes -- a closed curve of two edges is BOTH, and shares two */
      double sx[2], sy[2];
      int nshared = 0;
      if (j == i + 1)
      {
        sx[nshared] = edges[i]->x2; sy[nshared++] = edges[i]->y2;
      }
      if (closed && i == 0 && j == nedges - 1)
      {
        sx[nshared] = edges[0]->x1; sy[nshared++] = edges[0]->y1;
      }
      if (nshared == 0)
      {
        if (relate_edges_meet(edges[i], edges[j]))
          return false;
        continue;
      }
      double ix[2], iy[2];
      int n = relate_any_edge_intersection(edges[i], edges[j], ix, iy);
      if (n == 0)
      {
        /* Running along one another leaves no isolated point to read, so a
         * meeting the points do not account for is a span they share */
        if (relate_edges_meet(edges[i], edges[j]))
          return false;
        continue;
      }
      for (int k = 0; k < n; k++)
      {
        bool shared = false;
        for (int m = 0; m < nshared && ! shared; m++)
          shared = relate_same_point(ix[k], iy[k], sx[m], sy[m]);
        if (! shared)
          return false;
      }
    }
  return true;
}

/**
 * @brief Return true if the curves two members draw meet only at points that
 * end them both
 * @details A multi-curve is simple when each member is and the members meet
 * only on their boundaries, which is the rule #lwmline_is_simple applies to
 * straight members through #pointarrs_meet_at_ends. Two members running along
 * one another share a span rather than a point, and report no isolated point
 * to read, so that case is asked of #relate_edges_meet
 */
static bool
meos_curves_meet_at_ends(Edge **ea, int na, Edge **eb, int nb)
{
  if (na == 0 || nb == 0)
    return true;
  double ax0 = ea[0]->x1, ay0 = ea[0]->y1;
  double ax1 = ea[na - 1]->x2, ay1 = ea[na - 1]->y2;
  double bx0 = eb[0]->x1, by0 = eb[0]->y1;
  double bx1 = eb[nb - 1]->x2, by1 = eb[nb - 1]->y2;
  for (int i = 0; i < na; i++)
    for (int j = 0; j < nb; j++)
    {
      double ix[2], iy[2];
      int n = relate_any_edge_intersection(ea[i], eb[j], ix, iy);
      if (n == 0)
      {
        if (relate_edges_meet(ea[i], eb[j]))
          return false;
        continue;
      }
      for (int k = 0; k < n; k++)
      {
        bool ends_a = relate_same_point(ix[k], iy[k], ax0, ay0) ||
          relate_same_point(ix[k], iy[k], ax1, ay1);
        bool ends_b = relate_same_point(ix[k], iy[k], bx0, by0) ||
          relate_same_point(ix[k], iy[k], bx1, by1);
        if (! ends_a || ! ends_b)
          return false;
      }
    }
  return true;
}

/**
 * @brief Return true if the members of a multi-curve meet only at their ends
 */
static bool
meos_multicurve_members_meet_at_ends(const LWCOLLECTION *coll, bool *result)
{
  MeosArray **arrs = palloc0(sizeof(MeosArray *) * Max(coll->ngeoms, 1));
  Edge ***edges = palloc0(sizeof(Edge **) * Max(coll->ngeoms, 1));
  int *nedges = palloc0(sizeof(int) * Max(coll->ngeoms, 1));
  bool ok = true;
  for (uint32_t i = 0; i < coll->ngeoms; i++)
  {
    if (! coll->geoms[i] || lwgeom_is_empty(coll->geoms[i]))
      continue;
    arrs[i] = geom_extract_edges(coll->geoms[i]);
    nedges[i] = (int) arrs[i]->count;
    edges[i] = palloc(sizeof(Edge *) * Max(nedges[i], 1));
    for (int k = 0; k < nedges[i]; k++)
      edges[i][k] = (Edge *) meos_array_get_intl(arrs[i], k);
  }
  for (uint32_t i = 0; i < coll->ngeoms && ok; i++)
    for (uint32_t j = i + 1; j < coll->ngeoms && ok; j++)
      if (edges[i] && edges[j])
        ok = meos_curves_meet_at_ends(edges[i], nedges[i], edges[j],
          nedges[j]);
  for (uint32_t i = 0; i < coll->ngeoms; i++)
  {
    if (edges[i])
      pfree(edges[i]);
    if (arrs[i])
      meos_array_destroy(arrs[i]);
  }
  pfree(arrs); pfree(edges); pfree(nedges);
  *result = ok;
  return true;
}

/**
 * @brief Return true if every curve a geometry carrying arcs draws is simple
 * @details The components are walked rather than the edges of the whole,
 * since a ring and a member each draw their own curve, and two of them
 * meeting is a question about the geometry rather than about one curve
 */
static bool
meos_curved_is_simple(const LWGEOM *geom, bool *result)
{
  MeosArray *arr = geom_extract_edges(geom);
  if (! arr)
    return false;
  int n = (int) arr->count;
  Edge **edges = palloc(sizeof(Edge *) * Max(n, 1));
  for (int i = 0; i < n; i++)
    edges[i] = (Edge *) meos_array_get_intl(arr, i);
  *result = meos_curve_edges_simple(edges, n);
  pfree(edges);
  meos_array_destroy(arr);
  return true;
}

/**
 * @brief Return true if a geometry has no anomalous point
 * @details The result is reported in the last argument, the function itself
 * reporting whether the geometry is covered. A geometry holding a circular
 * arc is not covered.
 * @param[in] geom Geometry
 * @param[out] result True if the geometry is simple
 * @return True if the geometry is covered
 */
bool
meos_is_simple(const LWGEOM *geom, bool *result)
{
  assert(geom); assert(result);
  if (lwgeom_is_empty(geom))
  {
    *result = true;
    return true;
  }
  switch (geom->type)
  {
    case POINTTYPE:
      *result = true;
      return true;
    case MULTIPOINTTYPE:
      *result = lwmpoint_is_simple((const LWMPOINT *) geom);
      return true;
    case LINETYPE:
      *result = ptarray_is_simple(((const LWLINE *) geom)->points);
      return true;
    case MULTILINETYPE:
      *result = lwmline_is_simple((const LWMLINE *) geom);
      return true;
    case TRIANGLETYPE:
      *result = ptarray_is_simple(((const LWTRIANGLE *) geom)->points);
      return true;
    case POLYGONTYPE:
      *result = lwpoly_is_simple((const LWPOLY *) geom);
      return true;
    case MULTIPOLYGONTYPE:
    {
      const LWMPOLY *mpoly = (const LWMPOLY *) geom;
      for (uint32_t i = 0; i < mpoly->ngeoms; i++)
        if (mpoly->geoms[i] && ! lwpoly_is_simple(mpoly->geoms[i]))
        {
          *result = false;
          return true;
        }
      *result = true;
      return true;
    }
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
      {
        bool component;
        if (! coll->geoms[i])
          continue;
        if (! meos_is_simple(coll->geoms[i], &component))
          return false;
        if (! component)
        {
          *result = false;
          return true;
        }
      }
      *result = true;
      return true;
    }
    case CIRCSTRINGTYPE:
    case COMPOUNDTYPE:
      /* One curve, whose arcs meet one another along arcs as well as at
       * points, which is what #meos_curve_edges_simple reads */
      return meos_curved_is_simple(geom, result);
    case CURVEPOLYTYPE:
    {
      /* A surface is simple when each of its rings is, exactly as a polygon
       * of straight rings is */
      const LWCURVEPOLY *cpoly = (const LWCURVEPOLY *) geom;
      for (uint32_t i = 0; i < cpoly->nrings; i++)
      {
        bool ring;
        if (! cpoly->rings[i])
          continue;
        if (! meos_is_simple(cpoly->rings[i], &ring))
          return false;
        if (! ring)
        {
          *result = false;
          return true;
        }
      }
      *result = true;
      return true;
    }
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    {
      /* Every member answers for itself, as the straight multi-geometries do */
      const LWCOLLECTION *mcoll = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < mcoll->ngeoms; i++)
      {
        bool component;
        if (! mcoll->geoms[i])
          continue;
        if (! meos_is_simple(mcoll->geoms[i], &component))
          return false;
        if (! component)
        {
          *result = false;
          return true;
        }
      }
      /* A multi-curve asks one thing more, which its straight twin asks
       * through #pointarrs_meet_at_ends: the members meet only on their
       * boundaries. A multi-surface asks nothing more, exactly as a
       * multi-polygon of straight rings does not */
      if (geom->type == MULTICURVETYPE)
        return meos_multicurve_members_meet_at_ends(mcoll, result);
      *result = true;
      return true;
    }
    default:
      return false;
  }
}

/*****************************************************************************
 * Points, and the geometry a value serializes to
 *****************************************************************************/

/**
 * @brief Return -1, 0, or 1 depending on whether the first point is less than,
 * equal to, or greater than the second one
 */
int
geopoint_cmp(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    if (float8_lt(point1->x, point2->x))
      return -1;
    if (float8_gt(point1->x, point2->x))
      return 1;
    if (float8_lt(point1->y, point2->y))
      return -1;
    if (float8_gt(point1->y, point2->y))
      return 1;
    if (float8_lt(point1->z, point2->z))
      return -1;
    if (float8_gt(point1->z, point2->z))
      return 1;
    return 0;
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    if (float8_lt(point1->x, point2->x))
      return -1;
    if (float8_gt(point1->x, point2->x))
      return 1;
    if (float8_lt(point1->y, point2->y))
      return -1;
    if (float8_gt(point1->y, point2->y))
      return 1;
    return 0;
  }
}

/**
 * @brief Return the number of bytes of the sort key of a point
 * @param[in] gs Point
 */
size_t
geopoint_sortkey_size(const GSERIALIZED *gs)
{
  return (FLAGS_GET_Z(gs->gflags) ? 3 : 2) * datum_sortkey_size(T_FLOAT8);
}

/**
 * @brief Write the sort key of a point
 * @details The key is the coordinates in the order #geopoint_cmp compares
 * them, each written as a double
 * @param[in] gs Point
 * @param[out] out Buffer of #geopoint_sortkey_size bytes
 * @return The first byte after the key
 */
uint8_t *
geopoint_sortkey_put(const GSERIALIZED *gs, uint8_t *out)
{
  double coords[3];
  int ncoords;
  if (FLAGS_GET_Z(gs->gflags))
  {
    const POINT3DZ *point = GSERIALIZED_POINT3DZ_P(gs);
    coords[0] = point->x; coords[1] = point->y; coords[2] = point->z;
    ncoords = 3;
  }
  else
  {
    const POINT2D *point = GSERIALIZED_POINT2D_P(gs);
    coords[0] = point->x; coords[1] = point->y;
    ncoords = 2;
  }
  size_t n = datum_sortkey_size(T_FLOAT8);
  for (int i = 0; i < ncoords; i++)
  {
    datum_sortkey_put(Float8GetDatum(coords[i]), T_FLOAT8, out);
    out += n;
  }
  return out;
}

/**
 * @brief Return true if the points are equal
 * @note This function is called in the iterations over sequences where we
 * are sure that their SRID and GEODETIC are equal. The function accepts
 * mixed 2D/3D arguments
 */
bool
geopoint_eq(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  // TODO: Currently, activating these lines break tests
  // assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));
  // assert(FLAGS_GET_GEODETIC(gs1->gflags) == FLAGS_GET_GEODETIC(gs2->gflags));
  if (FLAGS_GET_Z(gs1->gflags) && FLAGS_GET_Z(gs2->gflags) )
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    return float8_eq(point1->x, point2->x) &&
      float8_eq(point1->y, point2->y) && float8_eq(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    return float8_eq(point1->x, point2->x) && float8_eq(point1->y, point2->y);
  }
}
/**
 * @brief Return true if the points are equal taking into account floating
 * point imprecision
 * @note This function is called in the iterations over sequences where we
 * are sure that their SRID, Z, and GEODETIC are equal
 */
bool
geopoint_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gserialized_get_srid(gs1) == gserialized_get_srid(gs2));
  assert(FLAGS_GET_Z(gs1->gflags) == FLAGS_GET_Z(gs2->gflags));
  assert(FLAGS_GET_GEODETIC(gs1->gflags) == FLAGS_GET_GEODETIC(gs2->gflags));
  if (FLAGS_GET_Z(gs1->gflags))
  {
    const POINT3DZ *point1 = GSERIALIZED_POINT3DZ_P(gs1);
    const POINT3DZ *point2 = GSERIALIZED_POINT3DZ_P(gs2);
    return MEOS_FP_EQ(point1->x, point2->x) &&
      MEOS_FP_EQ(point1->y, point2->y) && MEOS_FP_EQ(point1->z, point2->z);
  }
  else
  {
    const POINT2D *point1 = GSERIALIZED_POINT2D_P(gs1);
    const POINT2D *point2 = GSERIALIZED_POINT2D_P(gs2);
    return MEOS_FP_EQ(point1->x, point2->x) &&
      MEOS_FP_EQ(point1->y, point2->y);
  }
}
/**
 * @brief Return true if the two temporal points have the same spatial
 * dimensionality as given by their flags
 */
bool
same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) == MEOS_FLAGS_GET_X(flags2) &&
      MEOS_FLAGS_GET_Z(flags1) == MEOS_FLAGS_GET_Z(flags2))
    return true;
  return false;
}
/**
 * @brief Ensure that the geometry/geography is a (multi)line
 */
bool
mline_type(const GSERIALIZED *gs)
{
  uint32_t geotype = gserialized_get_type(gs);
  if (geotype == LINETYPE || geotype == MULTILINETYPE)
    return true;
  return false;
}
/**
 * @brief Return a point created from the arguments
 */
GSERIALIZED *
geopoint_make(double x, double y, double z, bool hasz, bool geodetic,
  int32_t srid)
{
  /* The coordinate list, the point array, and the point are in the stack,
   * since only the serialized result outlives the function */
  double coords[3] = {x, y, z};
  lwflags_t flags = 0;
  FLAGS_SET_Z(flags, hasz);
  FLAGS_SET_GEODETIC(flags, geodetic);
  POINTARRAY pa;
  pa.npoints = pa.maxpoints = 1;
  pa.flags = flags;
  pa.serialized_pointlist = (uint8_t *) coords;
  LWPOINT point;
  point.bbox = NULL;
  point.point = &pa;
  point.srid = srid;
  point.flags = flags;
  point.type = POINTTYPE;
  return geo_serialize((LWGEOM *) &point);
}
/**
 * @brief Extract the first-level elements of a gemetry collection
 */
GSERIALIZED **
geo_extract_elements(const GSERIALIZED *gs, int *count)
{
  assert(gs); assert(count);
  /* Extract the elements of the arguments, if they are collections */
  LWCOLLECTION *coll;
  GSERIALIZED **result = NULL;
  /* An empty geometry holds no element, and it is neither unitary --
   * #geo_is_unitary refuses an empty one deliberately -- nor necessarily a
   * collection, so without this it reaches the collection arm below and
   * #lwgeom_as_lwcollection answers NULL for the POINT, LINESTRING or POLYGON
   * it is. The array is still allocated because every caller releases it with
   * #pfree_array, which asserts the pointer it is given */
  if (gserialized_is_empty(gs))
  {
    *count = 0;
    return palloc(sizeof(GSERIALIZED *));
  }
  if (geo_is_unitary(gs))
  {
    *count = 1;
    result = palloc(sizeof(LWGEOM *));
    result[0] = geo_copy(gs);
  }
  else
  {
    LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
    coll = lwgeom_as_lwcollection(lwgeom);
    *count = coll->ngeoms;
    result = palloc(sizeof(LWGEOM *) * coll->ngeoms);
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      result[i] = geo_serialize(coll->geoms[i]);
    lwgeom_free(lwgeom);
  }
  return result;
}
/**
 * @brief Serialize a geometry/geography
 * @pre It is supposed that the flags such as Z and geodetic have been
 * set up before by the calling function
 */
GSERIALIZED *
geo_serialize(const LWGEOM *geom)
{
  GSERIALIZED *result = FLAGS_GET_GEODETIC(geom->flags) ?
    geog_serialize((LWGEOM *) geom) : geom_serialize((LWGEOM *) geom);
  return result;
}

/*****************************************************************************
 * Preconditions a geometry is read under
 *****************************************************************************/

/**
 * @brief Ensure that an SRID is geodetic
 */
bool
ensure_srid_is_latlong(int32_t srid)
{
  if (srid_is_latlong(srid))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only lon/lat coordinate systems are supported");
  return false;
}
/**
 * @brief Ensure that the geometry has geodetic coordinates
 */
bool
ensure_geodetic_geo(const GSERIALIZED *gs)
{
  if (FLAGS_GET_GEODETIC(gs->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only geodetic coordinates supported");
  return false;
}
/**
 * @brief Ensure that the geometry has planar coordinates
 */
bool
ensure_not_geodetic_geo(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_GEODETIC(gs->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only planar coordinates supported");
  return false;
}
/**
 * @brief Ensure that the spatiotemporal argument has geodetic coordinates
 */
bool
ensure_geodetic(int16 flags)
{
  if ((MEOS_FLAGS_GET_X(flags) || MEOS_FLAGS_GET_Z(flags)) && 
    MEOS_FLAGS_GET_GEODETIC(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only geodetic coordinates supported");
  return false;
}
/**
 * @brief Ensure that the spatiotemporal argument has planar coordinates
 */
bool
ensure_not_geodetic(int16 flags)
{
  if ((MEOS_FLAGS_GET_X(flags) || MEOS_FLAGS_GET_Z(flags)) && 
    ! MEOS_FLAGS_GET_GEODETIC(flags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only planar coordinates supported");
  return false;
}

/**
 * @brief Ensure that every member of an array of geometries carries the SRID
 * the first one does
 * @param[in] geoms Geometries
 * @param[in] count Number of elements in the array
 */
bool
ensure_same_srid_geoarr(const GSERIALIZED **geoms, int count)
{
  assert(geoms);
  int32_t srid = gserialized_get_srid(geoms[0]);
  for (int i = 1; i < count; i++)
    if (! ensure_same_srid(srid, gserialized_get_srid(geoms[i])))
      return false;
  return true;
}

/**
 * @brief Ensure that two geometries/geographies are both planar or both
 * geodetic
 */
bool
ensure_same_geodetic_geo(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_GEODETIC(gs1->gflags) == FLAGS_GET_GEODETIC(gs2->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed planar and geodetic coordinates");
  return false;
}

/**
 * @brief Ensure that two geometries/geographies have the same dimensionality,
 * as #ensure_same_dimensionality_tspatial_geo requires it of a
 * spatiotemporal value and a geometry/geography
 */
bool
ensure_same_dimensionality_geo(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  if (FLAGS_GET_Z(gs1->gflags) == FLAGS_GET_Z(gs2->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed 2D/3D dimensions");
  return false;
}
/**
 * @brief Ensure that the SRID is known
 */
bool
ensure_srid_known(int32_t srid)
{
  if (srid != SRID_UNKNOWN)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The SRID cannot be unknown");
  return false;
}
/**
 * @brief Ensure that the SRID is valid
 */
bool
ensure_srid_valid(int32_t srid)
{
  if (srid != SRID_INVALID)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The SRID is invalid");
  return false;
}
/**
 * @brief Reconcile the SRID of two spatial components: copy the known SRID onto
 * the one that is unknown, and ensure that two known SRIDs are equal
 * @details This is the single construction-time SRID resolution used by the
 * parsers and the constructors: an unknown (`SRID_UNKNOWN`) component adopts the
 * SRID of the other, while two known but different SRIDs are rejected. By
 * convention @p srid1 is the geometry SRID and @p srid2 the temporal type SRID.
 * @param[in] srid1,srid2 SRIDs to reconcile
 * @param[out] result Common SRID (the known one, or `SRID_UNKNOWN` if both are
 * unknown)
 * @return True when the SRIDs reconcile and the result is written
 * @errval false
 */
bool
ensure_srid_reconcile(int32_t srid1, int32_t srid2, int32_t *result)
{
  if (srid1 == SRID_UNKNOWN)
    *result = srid2;
  else if (srid2 == SRID_UNKNOWN || srid2 == srid1)
    *result = srid1;
  else
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "SRID of geometry (%d) and temporal type (%d) must correspond", srid1,
      srid2);
    return false;
  }
  return true;
}
/**
 * @brief Ensure that two temporal points have the same dimensionality as given
 * by their flags
 */
bool
ensure_same_dimensionality(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) == MEOS_FLAGS_GET_X(flags2) &&
      MEOS_FLAGS_GET_Z(flags1) == MEOS_FLAGS_GET_Z(flags2) &&
      MEOS_FLAGS_GET_T(flags1) == MEOS_FLAGS_GET_T(flags2))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The arguments must be of the same dimensionality");
  return false;
}
/**
 * @brief Ensure that two temporal points have the same spatial dimensionality
 * as given by their flags
 */
bool
ensure_same_spatial_dimensionality(int16 flags1, int16 flags2)
{
  if (same_spatial_dimensionality(flags1, flags2))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed 2D/3D dimensions");
  return false;
}
/**
 * @brief Ensure that the geometry/geography has not Z dimension
 */
bool
ensure_has_Z_geo(const GSERIALIZED *gs)
{
  if (FLAGS_GET_Z(gs->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The geometry must have Z dimension");
  return false;
}
/**
 * @brief Ensure that the geometry/geography has not Z dimension
 */
bool
ensure_has_not_Z_geo(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_Z(gs->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The geometry cannot have Z dimension");
  return false;
}
/**
 * @brief Ensure that the geometry/geography has M dimension
 */
bool
ensure_has_M_geo(const GSERIALIZED *gs)
{
  if (FLAGS_GET_M(gs->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The geometry must have M dimension");
  return false;
}
/**
 * @brief Ensure that the geometry/geography has not M dimension
 */
bool
ensure_has_not_M_geo(const GSERIALIZED *gs)
{
  if (! FLAGS_GET_M(gs->gflags))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The geometry cannot have M dimension");
  return false;
}
/**
 * @brief Ensure that the geometry/geography is a point
 */
bool
ensure_point_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) == POINTTYPE)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only point geometries accepted");
  return false;
}
/**
 * @brief Ensure that the geometry/geography is a (multi)line
 */
bool
ensure_mline_type(const GSERIALIZED *gs)
{
  if (mline_type(gs))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only (multi)line geometries accepted");
  return false;
}

/**
 * @brief Ensure that the geometry/geography is not empty
 * @details A point, the value of every temporal point instant, is read in
 * place: its number of points is the word before its coordinates, so the test
 * a temporal instant makes does not walk the geometry
 */
bool
ensure_not_empty(const GSERIALIZED *gs)
{
  const uint32_t *words = (const uint32_t *) (GS_POINT_PTR(gs) - 8);
  if ((words[0] == POINTTYPE) ? words[1] != 0 : ! gserialized_is_empty(gs))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only non-empty geometries accepted");
  return false;
}

/**
 * @brief Return true if a coordinate of a geometry/geography is NaN, walking
 * its points as #lwgeom_isfinite walks them
 * @details Kept out of line: inlined, it makes the caller save the registers
 * it uses on every call, which the test of a point never needs
 */
static pg_noinline bool
geo_walk_has_nan(const GSERIALIZED *gs)
{
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  bool hasz = lwgeom_has_z(geom), hasm = lwgeom_has_m(geom);
  bool nan = false;
  LWPOINTITERATOR *it = lwpointiterator_create(geom);
  while (! nan && lwpointiterator_has_next(it))
  {
    POINT4D p;
    lwpointiterator_next(it, &p);
    nan = isnan(p.x) || isnan(p.y) || (hasz && isnan(p.z)) ||
      (hasm && isnan(p.m));
  }
  lwpointiterator_destroy(it);
  lwgeom_free(geom);
  return nan;
}

/**
 * @brief Ensure that no coordinate of a geometry/geography is NaN
 * @details A NaN coordinate has no position, so the point holding it cannot
 * be bounded, compared or indexed. An infinite coordinate, which
 * #lwgeom_isfinite also refuses, is accepted
 */
bool
ensure_not_nan_geo(const GSERIALIZED *gs)
{
  /* A point, the value of every temporal point instant, is read in place: a
   * temporal instant is made by every operation that produces one, so the
   * test neither deserializes the geometry nor calls into it. The type and
   * the number of points of a serialized geometry are the two words before
   * the coordinates of its first point, and an empty point has none */
  const uint32_t *words = (const uint32_t *) (GS_POINT_PTR(gs) - 8);
  bool nan;
  if (words[0] == POINTTYPE)
  {
    const POINT2D *pt = GSERIALIZED_POINT2D_P(gs);
    bool z = FLAGS_GET_Z(gs->gflags), m = FLAGS_GET_M(gs->gflags);
    nan = words[1] != 0 && (isnan(pt->x) || isnan(pt->y) ||
      (z && isnan(GSERIALIZED_POINT3DZ_P(gs)->z)) ||
      (m && isnan(z ? GSERIALIZED_POINT4D_P(gs)->m :
        GSERIALIZED_POINT3DM_P(gs)->m)));
  }
  else
    nan = geo_walk_has_nan(gs);
  if (! nan)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "The coordinates of a geometry cannot be NaN");
  return false;
}

/*****************************************************************************
 * Segments and arrays of points
 *****************************************************************************/

/**
 * @brief Return -1, 0, or 1 depending on whether the first @p LWPOINT
 * is less than, equal to, or greater than the second one
 * @pre The points are not empty and are of the same dimensionality
 */
static int
lwpoint_cmp(const LWPOINT *p, const LWPOINT *q)
{
  assert(FLAGS_GET_ZM(p->flags) == FLAGS_GET_ZM(q->flags));
  POINT4D p4d, q4d;
  /* We are sure the points are not empty */
  lwpoint_getPoint4d_p(p, &p4d);
  lwpoint_getPoint4d_p(q, &q4d);
  int cmp = pg_float8_cmp(p4d.x, q4d.x);
  if (cmp != 0)
    return cmp;
  cmp = pg_float8_cmp(p4d.y, q4d.y);
  if (cmp != 0)
    return cmp;
  if (FLAGS_GET_Z(p->flags))
  {
    cmp = pg_float8_cmp(p4d.z, q4d.z);
    if (cmp != 0)
      return cmp;
  }
  if (FLAGS_GET_M(p->flags))
  {
    cmp = pg_float8_cmp(p4d.m, q4d.m);
    if (cmp != 0)
      return cmp;
  }
  return 0;
}

/**
 * @brief Comparator function for lwpoints
 */
static int
lwpoint_sort_cmp(const LWPOINT **l, const LWPOINT **r)
{
  return lwpoint_cmp(*l, *r);
}
/**
 * @brief Sort function for lwpoints
 */
static void
lwpointarr_sort(LWPOINT **points, int count)
{
  qsort(points, (size_t) count, sizeof(LWPOINT *),
    (qsort_comparator) &lwpoint_sort_cmp);
  return;
}

/**
 * @brief Return the location of the closest point on a 2D segment to a point,
 * as a fraction of the total segment length
 * @details The location is a long double between 0 and 1.
 * @note Function derived from the PostGIS function @p closest_point_on_segment
 */
long double
closest_point2d_on_segment_ratio(const POINT2D *p, const POINT2D *A,
  const POINT2D *B, POINT2D *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y))
  {
    if (closest)
      *closest = *A;
    return 0.0;
  }

  /*
   * We use comp.graphics.algorithms Frequently Asked Questions method
   *
   * (1)          AC dot AB
   *         r = ----------
   *              ||AB||^2
   *  r has the following meaning:
   *  r=0 P = A
   *  r=1 P = B
   *  r<0 P is on the backward extension of AB
   *  r>1 P is on the forward extension of AB
   *  0<r<1 P is interior to AB
   *
   */
  long double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) );

  if (r < 0)
  {
    if (closest)
      *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    if (closest)
      *closest = *B;
    return 1.0;
  }

  if (closest)
  {
    closest->x = (double) (A->x + ( (B->x - A->x) * r ));
    closest->y = (double) (A->y + ( (B->y - A->y) * r ));
  }
  return r;
}

/**
 * @brief Return the location of the closest point on a 3D segment to a point
 * @details The result is a long double between 0 and 1 representing the
 * location as a fraction of total segment length.
 * @note Function derived from the PostGIS function @p closest_point_on_segment
 */
long double
closest_point3dz_on_segment_ratio(const POINT3DZ *p, const POINT3DZ *A,
  const POINT3DZ *B, POINT3DZ *closest)
{
  if (FP_EQUALS(A->x, B->x) && FP_EQUALS(A->y, B->y) && FP_EQUALS(A->z, B->z))
  {
    *closest = *A;
    return 0.0;
  }

  /* Function #closest_point2d_on_segment_ratio explains how r is computed */
  long double r = ( (p->x-A->x) * (B->x-A->x) + (p->y-A->y) * (B->y-A->y) +
      (p->z-A->z) * (B->z-A->z) ) /
    ( (B->x-A->x) * (B->x-A->x) + (B->y-A->y) * (B->y-A->y) +
      (B->z-A->z) * (B->z-A->z) );

  if (r < 0)
  {
    *closest = *A;
    return 0.0;
  }
  if (r > 1)
  {
    *closest = *B;
    return 1.0;
  }

  closest->x = (double) (A->x + ( (B->x - A->x) * r ));
  closest->y = (double) (A->y + ( (B->y - A->y) * r ));
  closest->z = (double) (A->z + ( (B->z - A->z) * r ));
  return r;
}

/**
 * @brief Remove duplicates from an array of LWGEOM points
 */
LWGEOM **
lwpointarr_remove_duplicates(LWGEOM **points, int count, int *newcount)
{
  assert(count > 0);
  LWGEOM **newpoints = palloc(sizeof(LWGEOM *) * count);
  memcpy(newpoints, points, sizeof(LWGEOM *) * count);
  lwpointarr_sort((LWPOINT **) newpoints, count);
  int count1 = 0;
  for (int i = 1; i < count; i++)
    if (! lwpoint_same((LWPOINT *) newpoints[count1], (LWPOINT *) newpoints[i]))
      newpoints[++ count1] = newpoints[i];
  *newcount = count1 + 1;
  return newpoints;
}

/**
 * @brief Return a trajectory from a set of points
 * @details The result is either a linestring or a multipoint depending on
 * whether the interpolation is step/discrete or linear.
 * @param[in] points Array of points
 * @param[in] count Number of elements in the input array
 * @param[in] interp Interpolation
 * @note The function does not remove duplicate points, that is, repeated
 * points in a multipoint or consecutive equal points in a line string
 */
LWGEOM *
lwpointarr_make_trajectory(LWGEOM **points, int count, interpType interp)
{
  assert(points); assert(count > 0);
  if (count == 1)
    return lwpoint_as_lwgeom(lwpoint_clone(lwgeom_as_lwpoint(points[0])));

  LWGEOM *result = (interp == LINEAR) ?
    (LWGEOM *) lwline_from_lwgeom_array(points[0]->srid, (uint32_t) count,
      points) :
    (LWGEOM *) lwcollection_construct(MULTIPOINTTYPE, points[0]->srid,
      NULL, (uint32_t) count, points);
  FLAGS_SET_Z(result->flags, FLAGS_GET_Z(points[0]->flags));
  FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(points[0]->flags));
  return result;
}

/**
 * @brief Ensure the validity of two temporal points
 */
bool
ensure_valid_geo_geo(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs1, false); VALIDATE_NOT_NULL(gs2, false);
  if (! ensure_same_srid(gserialized_get_srid(gs1),
        gserialized_get_srid(gs2)) ||
      ! ensure_same_geodetic_geo(gs1, gs2))
    return false;
  return true;
}

/*****************************************************************************
 * Create a circle
 *****************************************************************************/

/* The following function is not exported in PostGIS */
extern LWCIRCSTRING *lwcircstring_from_lwpointarray(int32_t srid,
  uint32_t npoints, LWPOINT **points);

/**
 * @brief Return a circle created from a central point and a radius
 */
LWGEOM *
lwcircle_make(double x, double y, double radius, int32_t srid)
{
  assert(radius > 0);
  LWPOINT *points[3];
  /* Shift the X coordinate of the point by +- radius */
  points[0] = lwpoint_make2d(srid, x - radius, y);
  points[1] = lwpoint_make2d(srid, x + radius, y);
  points[2] = lwpoint_make2d(srid, x - radius, y);
  /* Construct the circle */
  LWGEOM *ring = lwcircstring_as_lwgeom(
    lwcircstring_from_lwpointarray(srid, 3, points));
  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(result, ring);
  /* Clean up and return */
  lwpoint_free(points[0]); lwpoint_free(points[1]); lwpoint_free(points[2]);
  /* We cannot lwgeom_free(ring); */
  return lwcurvepoly_as_lwgeom(result);
}

/**
 * @brief Return a circle created from a central point and a radius
 */
GSERIALIZED *
geocircle_make(double x, double y, double radius, int32_t srid)
{
  LWGEOM *res = lwcircle_make(x, y, radius, srid);
  GSERIALIZED *result = geo_serialize(res);
  lwgeom_free(res);
  return result;
}

/**
 * @brief Return true if a geometry is a circle
 * @details A circle is a curve polygon of a single ring, that ring a closed
 * circular string of three points: the two ends, which are the same point,
 * and the point opposite them. A ring of any other kind belongs to a curve
 * polygon that bounds some other shape, and the members it carries are not
 * the ones a circular string carries, so its kind is read before it is
 * addressed as one.
 */
bool
circle_type(const GSERIALIZED *gs)
{
  if (gserialized_get_type(gs) != CURVEPOLYTYPE)
    return false;
  LWGEOM *geo = lwgeom_from_gserialized(gs);
  bool result = false;
  if (lwgeom_count_rings(geo) == 1)
  {
    LWGEOM *ring = ((LWCURVEPOLY *) geo)->rings[0];
    if (ring->type == CIRCSTRINGTYPE)
    {
      const POINTARRAY *points = ((LWCIRCSTRING *) ring)->points;
      result = points->npoints == 3 && ptarray_is_closed(points);
    }
  }
  lwgeom_free(geo);
  return result;
}

/**
 * @brief Ensure that the geometry/geography is a circle
 */
bool
ensure_circle_type(const GSERIALIZED *gs)
{
  if (! circle_type(gs))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Only circle polygons accepted");
    return false;
  }
  return true;
}

/*****************************************************************************
 * Point classification
 *****************************************************************************/

/**
 * @brief Scan the boundary edges of an array for one that carries a point
 * @details An input vertex lies on a boundary segment or on the circle of a
 * boundary arc exactly where its coordinates say so (#point_on_segment_exact,
 * #point_on_arc_circle); a constructed point is read within the rounding its
 * edge carries
 * @param[in] x,y Coordinates of the point
 * @param[in] edges,nedges Edge array
 * @param[in] vertex True if the point is an input vertex
 */
static pg_attribute_always_inline bool
relate_point_on_boundary_scan(double x, double y, Edge **edges, int nedges,
  bool vertex)
{
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    switch (e->etype)
    {
      case EDGE_POLYSEG:
        if (vertex ?
            point_on_segment_exact(x, y, e->x1, e->y1, e->x2, e->y2) :
            point_on_segment_within(x, y, e->x1, e->y1, e->x2, e->y2,
              e->tol))
          return true;
        break;
      case EDGE_POLYARC:
        if (vertex ?
            (point_on_arc_circle(e, x, y) &&
              arc_contains_angle(e, atan2(y - e->cy, x - e->cx))) :
            point_on_arc(x, y, e))
          return true;
        break;
      case EDGE_POINT:
      case EDGE_LINESEG:
      case EDGE_LINEARC:
        break;
    }
  }
  return false;
}

/**
 * @brief Return true if a point lies on the boundary of a geometry
 * @details The scan over the edges (#relate_point_on_boundary_scan) is
 * compiled once for an input vertex and once for a constructed point, and the
 * one asked is chosen here, so neither loop tests which kind of point it reads
 * @param[in] x,y Coordinates of the point
 * @param[in] edges,nedges Edge array
 * @param[in] vertex True if the point is an input vertex
 */
bool
relate_point_on_boundary(double x, double y, Edge **edges, int nedges,
  bool vertex)
{
  return vertex ?
    relate_point_on_boundary_scan(x, y, edges, nedges, true) :
    relate_point_on_boundary_scan(x, y, edges, nedges, false);
}

/**
 * @brief Classify a point with respect to an areal geometry
 * @details Return:
 *   0 = interior
 *   1 = boundary
 *   2 = exterior
 * @param[in] vertex True if the point is an input vertex
 * (#relate_point_on_boundary)
 */
int
relate_point_in_area(double x, double y, Edge **edges, int nedges,
  bool vertex)
{
  if (relate_point_on_boundary(x, y, edges, nedges, vertex))
    return 1;
  return (vertex ? point_in_polygon_vertex(x, y, edges, nedges) :
    point_in_polygon(x, y, edges, nedges)) ? 0 : 2;
}

/* Indexing one edge array costs one insertion per edge, while walking it once
 * for every edge of the other costs their PRODUCT. The index is built where
 * that product dwarfs the pass, and a pair small enough for the walk to be the
 * cheaper of the two keeps it */

/**
 * @brief An edge read by an index, with the side of its box it is ordered by
 */
typedef struct
{
  double key;       /**< Bottom or left of the box of the edge */
  uint32_t id;      /**< Position of the edge in its array */
} EdgeKey;

/**
 * @brief Order the edges of an index by the side of their box, then by their
 * position, which makes the order total
 */
static inline int
edge_key_cmp(const EdgeKey *a, const EdgeKey *b)
{
  if (a->key < b->key)
    return -1;
  if (a->key > b->key)
    return 1;
  return (a->id < b->id) ? -1 : (a->id > b->id) ? 1 : 0;
}

/* Sort the edges of an index with their order written into the sort, as
 * #buffer_sweep_edge_sort sorts the edges of a sweep */
#define ST_SORT edge_key_sort
#define ST_ELEMENT_TYPE EdgeKey
#define ST_COMPARE(a, b) edge_key_cmp(a, b)
#define ST_SCOPE static
#define ST_DECLARE
#define ST_DEFINE
#include "port/sort_template.h"

/**
 * @brief Order the edges of an index along one axis
 * @details An edge whose extent along the axis is more than eight times that
 * of three edges in four is kept apart, in @p apart, and the rest stand in
 * @p ids by the low side of their box, with that side in @p lo and the
 * greatest extent among them in @p reach. Eight times the third quartile
 * leaves the ordinary edges of a ring together and sets apart only the few a
 * polygon carries across its whole extent; where three edges in four have no
 * extent at all, as the horizontal or vertical edges of a grid, the extent of
 * the whole array over the square root of their number takes its place
 */
static void
edge_index_axis(Edge **edges, int nedges, bool alongy, uint32_t **ids,
  double **lo, int *nids, double *reach, uint32_t **apart, int *napart)
{
  double *ext = palloc(sizeof(double) * nedges);
  double amin = DBL_MAX, amax = -DBL_MAX;
  for (int i = 0; i < nedges; i++)
  {
    double l = alongy ? edges[i]->ymin : edges[i]->xmin;
    double h = alongy ? edges[i]->ymax : edges[i]->xmax;
    ext[i] = h - l;
    amin = Min(amin, l);
    amax = Max(amax, h);
  }
  EdgeKey *keys = palloc(sizeof(EdgeKey) * nedges);
  for (int i = 0; i < nedges; i++)
  {
    keys[i].key = ext[i];
    keys[i].id = (uint32_t) i;
  }
  edge_key_sort(keys, (size_t) nedges);
  double cut = 8.0 * keys[(3 * (nedges - 1)) / 4].key;
  if (cut <= 0.0)
    cut = (amax - amin) / sqrt((double) nedges);
  int n = 0, na = 0;
  *reach = 0.0;
  for (int i = 0; i < nedges; i++)
  {
    if (ext[i] > cut)
    {
      na++;
      continue;
    }
    keys[n].key = alongy ? edges[i]->ymin : edges[i]->xmin;
    keys[n].id = (uint32_t) i;
    n++;
    *reach = Max(*reach, ext[i]);
  }
  edge_key_sort(keys, (size_t) n);
  *ids = palloc(sizeof(uint32_t) * Max(n, 1));
  *lo = palloc(sizeof(double) * Max(n, 1));
  for (int k = 0; k < n; k++)
  {
    (*ids)[k] = keys[k].id;
    (*lo)[k] = keys[k].key;
  }
  *nids = n;
  *apart = palloc(sizeof(uint32_t) * Max(na, 1));
  *napart = 0;
  for (int i = 0; i < nedges; i++)
    if (ext[i] > cut)
      (*apart)[(*napart)++] = (uint32_t) i;
  pfree(keys); pfree(ext);
  return;
}

/**
 * @brief Return an index over the boxes of an edge array
 * @details See #EdgeIndex
 * @param[in] edges,nedges The edges, which the index names by position
 */
EdgeIndex *
edge_index_make(Edge **edges, int nedges)
{
  assert(edges); assert(nedges > 0);
  EdgeIndex *index = palloc(sizeof(EdgeIndex));
  index->nedges = nedges;
  edge_index_axis(edges, nedges, true, &index->byy, &index->ylo,
    &index->nbyy, &index->tallest, &index->tall, &index->ntall);
  edge_index_axis(edges, nedges, false, &index->byx, &index->xlo,
    &index->nbyx, &index->widest, &index->wide, &index->nwide);
  return index;
}

/**
 * @brief Release an index over the boxes of an edge array
 */
void
edge_index_free(EdgeIndex *index)
{
  if (! index)
    return;
  pfree(index->byy); pfree(index->ylo); pfree(index->tall);
  pfree(index->byx); pfree(index->xlo); pfree(index->wide);
  pfree(index);
  return;
}

/**
 * @brief Return the first place in an ordered array of sides holding a side
 * no lower than a value
 */
static int
edge_index_first(const double *lo, int n, double value)
{
  int a = 0, b = n;
  while (a < b)
  {
    int m = a + (b - a) / 2;
    if (lo[m] < value)
      a = m + 1;
    else
      b = m;
  }
  return a;
}

/**
 * @brief Collect into an array the edges whose box meets a box, closed on both
 * axes, read out of an index over them
 * @details The edges answered are those an R-tree of the same boxes answers
 * for the same query, each once, in the order the index reads them
 * @param[in] index Index over @p edges
 * @param[in] edges The edges the index was built over
 * @param[in] xmin,xmax,ymin,ymax Box
 * @param[out] result Array the positions of the edges are collected into, as
 * #index_result_create makes one, emptied first
 * @return The number of edges collected
 */
int
edge_index_query(const EdgeIndex *index, Edge **edges, double xmin,
  double xmax, double ymin, double ymax, MeosArray *result)
{
  assert(index); assert(edges); assert(result);
  meos_array_reset(result);
  /* A box wider than it is tall leaves few edges whose bottom lies within
   * its height and many whose left lies within its width, and the other way
   * round */
  bool alongy = (xmax - xmin) >= (ymax - ymin);
  const uint32_t *ids = alongy ? index->byy : index->byx;
  const double *lo = alongy ? index->ylo : index->xlo;
  int nids = alongy ? index->nbyy : index->nbyx;
  double reach = alongy ? index->tallest : index->widest;
  double qlo = alongy ? ymin : xmin, qhi = alongy ? ymax : xmax;
  /* An edge meeting the box has its low side at most the high side of the
   * box, and its high side at least the low side of the box, which with no
   * edge reaching further than @p reach puts its low side at least that far
   * below the box */
  for (int k = edge_index_first(lo, nids, qlo - reach);
       k < nids && lo[k] <= qhi; k++)
  {
    const Edge *e = edges[ids[k]];
    if (e->xmax < xmin || e->xmin > xmax || e->ymax < ymin || e->ymin > ymax)
      continue;
    int64 id = (int64) ids[k];
    meos_array_add(result, &id);
  }
  const uint32_t *apart = alongy ? index->tall : index->wide;
  int napart = alongy ? index->ntall : index->nwide;
  for (int k = 0; k < napart; k++)
  {
    const Edge *e = edges[apart[k]];
    if (e->xmax < xmin || e->xmin > xmax || e->ymax < ymin || e->ymin > ymax)
      continue;
    int64 id = (int64) apart[k];
    meos_array_add(result, &id);
  }
  return (int) result->count;
}

/**
 * @brief Return an index over the bounding boxes of an edge array
 * @details See #edge_index_make
 */
static EdgeIndex *
relate_edges_index(Edge **edges, int nedges)
{
  assert(edges);
  return nedges > 0 ? edge_index_make(edges, nedges) : NULL;
}

/**
 * @brief Fill in an edge array together with what reading it selectively needs
 * @param[out] re Structure to fill in
 * @param[in] edges,nedges The edges
 * @param[in] index True to index them, which the caller decides from the size
 * of BOTH arrays, since it is their product the index removes
 */
void
relate_edges_init(RelateEdges *re, Edge **edges, int nedges, bool index)
{
  assert(edges);
  re->edges = edges;
  re->nedges = nedges;
  /* The extent is the union of the edge boxes the index holds, so a question
   * it leaves standing apart is one the index answers with no edge at all.
   * Its greatest x is also how far a ray cast from a point to its right runs
   * before it leaves the array behind.
   * An index query has to admit every edge the scan it replaces would accept,
   * and the on-edge tests read the tolerance the EDGE carries, which grows
   * with the size of its coordinates. Padding a query by MEOS_GEOM_TOLERANCE
   * would therefore drop, at projected coordinates, edges whose own tolerance
   * is thousands of times that. The widest tolerance in the array is what
   * makes the index answer what the scan answers at every scale */
  re->xmin = re->ymin = DBL_MAX;
  re->xmax = re->ymax = -DBL_MAX;
  re->tol = 0.0;
  re->reach = 0.0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->xmin < re->xmin) re->xmin = e->xmin;
    if (e->xmax > re->xmax) re->xmax = e->xmax;
    if (e->ymin < re->ymin) re->ymin = e->ymin;
    if (e->ymax > re->ymax) re->ymax = e->ymax;
    if (e->tol > re->tol)
      re->tol = e->tol;
    /* The reach a point location reads an end of the edge within, as
     * #point_in_polygon_impl reads it */
    double reach = e->tol;
    if (e->etype == EDGE_POLYARC)
      reach = fmax(reach, MEOS_GEOM_TOLERANCE * e->radius);
    if (reach > re->reach)
      re->reach = reach;
  }
  re->index = index ? relate_edges_index(edges, nedges) : NULL;
  re->results = index ? index_result_create() : NULL;
  return;
}

/**
 * @brief Build the index an edge array set up without one
 * @details The answers read through it are those the scan gives, so a caller
 * builds it on the first question it answers
 */
static void
relate_edges_index_build(RelateEdges *re)
{
  if (re->index)
    return;
  re->index = relate_edges_index(re->edges, re->nedges);
  re->results = index_result_create();
  return;
}

/**
 * @brief Free the index an edge array carries, if it carries one
 */
void
relate_edges_clear(RelateEdges *re)
{
  if (re->index)
    edge_index_free(re->index);
  if (re->results)
    meos_array_destroy(re->results);
  re->index = NULL;
  re->results = NULL;
  return;
}

/**
 * @brief Return true if a point lies on the boundary an edge array draws,
 * reading the edges that can carry it out of an index
 * @details Only an edge whose box holds the point can carry it, and the box is
 * grown by the tolerance the on-edge tests allow, so the edges the index
 * answers are the ones the scan finds
 */
static bool
relate_point_on_boundary_index(double x, double y, const RelateEdges *re,
  bool vertex)
{
  if (! re->index)
    return relate_point_on_boundary(x, y, re->edges, re->nedges, vertex);
  int nc = edge_index_query(re->index, re->edges, x - re->tol, x + re->tol,
    y - re->tol, y + re->tol, re->results);
  bool result = false;
  for (int c = 0; c < nc && ! result; c++)
  {
    Edge *one = re->edges[INDEX_RESULT_ID_N(re->results, c)];
    result = relate_point_on_boundary(x, y, &one, 1, vertex);
  }
  return result;
}

/**
 * @brief Return true if a box grown by a pad stands apart from the extent of
 * an edge array
 * @details The extent is the union of the boxes the index of the array holds,
 * so a query the test separates is one the index answers with no edge, and
 * with the pad an index query is grown by, one the scan answers with none
 * either
 * @param[in] re Edge array
 * @param[in] xmin,xmax,ymin,ymax Box
 * @param[in] pad Distance the box is grown by
 */
static inline bool
relate_edges_apart(const RelateEdges *re, double xmin, double xmax,
  double ymin, double ymax, double pad)
{
  return xmax + pad < re->xmin || xmin - pad > re->xmax ||
    ymax + pad < re->ymin || ymin - pad > re->ymax;
}

/**
 * @brief Return where a point stands with respect to the area an edge array
 * bounds, reading the edges each question needs out of an index
 * @details The answer is the one #relate_point_in_area() gives: the index only
 * decides which edges are asked, and one it leaves out neither carries the
 * point nor crosses the ray cast from it
 */
int
relate_point_in_area_index(double x, double y, const RelateEdges *re,
  bool vertex)
{
  /* A point apart from the extent is carried by no edge, and a ray cast from
   * it crosses the closed rings the array draws an even number of times */
  if (relate_edges_apart(re, x, x, y, y, re->tol))
    return 2;
  if (relate_point_on_boundary_index(x, y, re, vertex))
    return 1;
  if (! re->index)
    return (vertex ? point_in_polygon_vertex(x, y, re->edges, re->nedges) :
      point_in_polygon(x, y, re->edges, re->nedges)) ? 0 : 2;
  return point_in_polygon_index_into(x, y, re->edges, re->nedges, re->index,
    re->xmax, re->reach, re->results, vertex) ? 0 : 2;
}

/**
 * @brief Return whether a point is inside or outside the area an edge array
 * bounds, for a caller that already knows the point is not on its boundary
 * @details #relate_point_in_area_index answers ON THE BOUNDARY first, and that
 * question is the one a tolerance decides: a point within the band of an edge
 * reads as carried by it. A caller holding a point that PROVABLY misses the
 * boundary gets a wrong answer from that test and only a wrong one, so this
 * reads the even-odd parity alone -- the same parity, over the same edges, with
 * the band that cannot apply left out
 * @return 0 for the interior and 2 for the exterior, as #relate_point_in_area
 * reports them
 */
static int
relate_point_in_area_parity(double x, double y, const RelateEdges *re)
{
  if (relate_edges_apart(re, x, x, y, y, 0.0))
    return 2;
  if (! re->index)
    return point_in_polygon(x, y, re->edges, re->nedges) ? 0 : 2;
  return point_in_polygon_index_into(x, y, re->edges, re->nedges, re->index,
    re->xmax, re->reach, re->results, false) ? 0 : 2;
}

/*****************************************************************************
 * DE-9IM / ST_Relate
 *****************************************************************************/

/*
 * @brief DE-9IM cell dimensions:
 *   -1 = F (empty)
 *    0 = point
 *    1 = line
 *    2 = area
 */
typedef struct
{
  int8_t ii;
  int8_t ib;
  int8_t ie;
  int8_t bi;
  int8_t bb;
  int8_t be;
  int8_t ei;
  int8_t eb;
  int8_t ee;
} MeosDE9IM;

/**
 * @brief The nine cells of a DE-9IM matrix as bits, in the order of its
 * string, so a caller names the cells it reads
 * @details A pattern reads the cells it does not mark `*`: `T********` reads
 * the interiors alone, and a cell nobody reads needs no computing
 */
#define DE9IM_II  (1 << 0)
#define DE9IM_IB  (1 << 1)
#define DE9IM_IE  (1 << 2)
#define DE9IM_BI  (1 << 3)
#define DE9IM_BB  (1 << 4)
#define DE9IM_BE  (1 << 5)
#define DE9IM_EI  (1 << 6)
#define DE9IM_EB  (1 << 7)
#define DE9IM_EE  (1 << 8)
#define DE9IM_ALL 0x1FF

/**
 * @brief Return the cells of the matrix of the reversed pair that answer the
 * cells named for the pair
 * @details Row @p r column @p c of the matrix is row @p c column @p r of the
 * matrix of the reversed pair, which is its transpose
 * @param[in] want Cells, as the #DE9IM_II bits
 */
static inline int
de9im_transpose_cells(int want)
{
  int result = 0;
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      if (want & (1 << (3 * r + c)))
        result |= 1 << (3 * c + r);
  return result;
}

/**
 * @brief The question a caller asks of a DE-9IM matrix: one or more patterns,
 * any of which matching answers it
 * @details A relationship reads a handful of the nine cells, and the patterns
 * deciding it spell out which: `contains` is `T*****FF*`, `touches` holds when
 * any of `FT*******`, `F**T*****` or `F***T****` does. The engine computes the
 * cells the patterns name and stops once the answer is known, as the GEOS
 * relate engine does, instead of computing all nine cells whatever the question
 */
typedef struct
{
  int count;              /**< Number of patterns */
  char pattern[4][10];    /**< The patterns, any of which answers the question */
  int cells;              /**< Cells some pattern reads, as #DE9IM_II bits */
  int fcells[4];          /**< Cells each pattern reads as `F` */
  int tcells[4];          /**< Cells each pattern reads as `T` */
  int dcells[4];          /**< Cells each pattern reads as a dimension */
} RelateQuery;

/**
 * @brief Set the cell masks of the patterns of a query
 */
static void
relate_query_masks(RelateQuery *q)
{
  q->cells = 0;
  for (int k = 0; k < q->count; k++)
  {
    q->fcells[k] = q->tcells[k] = q->dcells[k] = 0;
    for (int i = 0; i < 9; i++)
    {
      char c = q->pattern[k][i];
      if (c == '*')
        continue;
      q->cells |= 1 << i;
      if (c == 'F')
        q->fcells[k] |= 1 << i;
      else if (c == 'T')
        q->tcells[k] |= 1 << i;
      else
        q->dcells[k] |= 1 << i;
    }
  }
  return;
}

/**
 * @brief Set a query from its patterns
 * @param[out] q Query
 * @param[in] patterns,count Patterns, at most four
 */
static void
relate_query_init(RelateQuery *q, const char *const *patterns, int count)
{
  assert(count > 0 && count <= 4);
  q->count = count;
  for (int k = 0; k < count; k++)
  {
    memcpy(q->pattern[k], patterns[k], 9);
    q->pattern[k][9] = '\0';
  }
  relate_query_masks(q);
  return;
}

/**
 * @brief Return the cells a query reads, every cell where there is no query
 */
static inline int
relate_query_cells(const RelateQuery *q)
{
  return q ? q->cells : DE9IM_ALL;
}

/**
 * @brief Set the query of the reversed pair, whose matrix is the transpose
 */
static void
relate_query_transpose(const RelateQuery *q, RelateQuery *result)
{
  result->count = q->count;
  for (int k = 0; k < q->count; k++)
  {
    for (int r = 0; r < 3; r++)
      for (int c = 0; c < 3; c++)
        result->pattern[k][3 * c + r] = q->pattern[k][3 * r + c];
    result->pattern[k][9] = '\0';
  }
  relate_query_masks(result);
  return;
}

/**
 * @brief Return true if no pattern of a query reads a cell of an exterior
 */
static bool
relate_query_reads_no_exterior(const RelateQuery *q)
{
  const int exterior = DE9IM_IE | DE9IM_BE | DE9IM_EI | DE9IM_EB | DE9IM_EE;
  return q && ! (q->cells & exterior);
}

/**
 * @brief Return true if the matrix being filled already answers a query
 * @details A cell only ever rises (#de9im_add), so a pattern is known FALSE
 * once a cell it reads as `F` holds a dimension or a cell exceeds the dimension
 * it names, and a pattern reading only `T` and `*` is known TRUE once every
 * cell it reads as `T` holds one. The query is answered when a
 * pattern is known true or every pattern is known false. A matrix left
 * unfinished at that point matches every pattern exactly as the finished one
 * would, which is what lets a kernel stop there
 * @param[in] q Query, NULL where the whole matrix is asked for
 * @param[in] m Matrix being filled
 */
static bool
relate_query_decided(const RelateQuery *q, const MeosDE9IM *m)
{
  if (! q)
    return false;
  const int8_t cell[9] = {m->ii, m->ib, m->ie, m->bi, m->bb, m->be, m->ei,
    m->eb, m->ee};
  int filled = 0;
  for (int i = 0; i < 9; i++)
    if (cell[i] != -1)
      filled |= 1 << i;
  int known_false = 0;
  for (int k = 0; k < q->count; k++)
  {
    bool is_false = (q->fcells[k] & filled) != 0;
    for (int i = 0; i < 9 && ! is_false; i++)
      if (q->dcells[k] & (1 << i))
        is_false = (cell[i] > q->pattern[k][i] - '0');
    if (is_false)
      known_false++;
    else if (! q->fcells[k] && ! q->dcells[k] &&
        (q->tcells[k] & ~filled) == 0)
      return true;
  }
  return known_false == q->count;
}

/**
 * @brief Set all cells of a DE-9IM matrix to F
 */
static inline void
de9im_init(MeosDE9IM *m)
{
  m->ii = -1;
  m->ib = -1;
  m->ie = -1;
  m->bi = -1;
  m->bb = -1;
  m->be = -1;
  m->ei = -1;
  m->eb = -1;
  m->ee = -1;
  return;
}

static POINT2D *relate_linear_boundary_points(Edge **edges, int nedges,
  int *count);
static int relate_edges_candidates(const RelateEdges *re, double xmin,
  double xmax, double ymin, double ymax, MeosArray *candidates);
static MeosArray *relate_extract_edges(const LWGEOM *geom);
static bool relate_reads_union(const LWGEOM *geom);
static MeosArray *relate_union_edges(const LWGEOM *geom, MeosArray *all);
static bool relate_area_boundary_edge(const Edge *e);
static void relate_area_edge_point(const Edge *e, double t, double *x,
  double *y);
static double relate_area_edge_parameter(const Edge *e, double x, double y);
static void relate_area_add_parameter(double t, double *params, int *nparams,
  int maxparams);
static int relate_area_parameter_cmp(const void *a, const void *b);
static int relate_area_edge_intersection(const Edge *a, const Edge *b,
  double ix[2], double iy[2], bool *overlap);
static bool relate_point_on_edge(double x, double y, const Edge *e,
  bool vertex);

/**
 * @brief One operand of a relationship, together with the edges it draws
 */
typedef struct
{
  const LWGEOM *geom;  /**< Geometry the edges are those of */
  MeosArray *arr;      /**< Edges its interior and boundary are read from, for
                            a value #relate_reads_union names NULL until a
                            question reads them; read through
                            #relate_operand_arr, which keeps them in
                            #kept */
  MeosArray *own;      /**< Its own edges, ending at the input vertices: the
                            array #arr is cut from for a value
                            #relate_reads_union names, #arr itself otherwise;
                            NULL until a question reads them, through
                            #relate_operand_own, which keeps them in
                            #ownkept */
  MeosArray **kept;    /**< Where #arr is kept once read, as #index keeps the
                            index: the operand's own #arr, or that of the
                            context that lends it, so that a copy of the
                            operand reads and fills the same one */
  MeosArray **ownkept; /**< Where #own is kept once read, as #kept keeps
                            #arr */
  EdgeIndex **index;   /**< Where the index over #arr is kept for the calls
                            that follow, NULL for an operand read for one
                            call, which indexes by the size of the pair */
} RelateOperand;

/**
 * @brief Return the edges an operand's interior and boundary are read from,
 * reading the union of its members the first time a question asks for it
 * @details Whether two geometries share a point is read from their own edges
 * (#meos_intersects), and it is the question a relationship mostly asks, so
 * the union of a collection's members, what a call on such a value mostly
 * costs, is built only where a question reads it, and kept for the questions
 * that follow
 */
static MeosArray *
relate_operand_own(const RelateOperand *op)
{
  if (! *op->ownkept)
  {
    /* The edges of an operand a context lends live as long as the context,
     * so they are read into the memory that keeps the context, never into
     * the caller's, as #relate_edges_init_kept reads the index */
#if ! MEOS
    MemoryContext oldctx = op->index ?
      MemoryContextSwitchTo(TopMemoryContext) : NULL;
#endif
    *op->ownkept = geom_extract_edges(op->geom);
#if ! MEOS
    if (op->index)
      MemoryContextSwitchTo(oldctx);
#endif
  }
  return *op->ownkept;
}

static MeosArray *
relate_operand_arr(const RelateOperand *op)
{
  if (! *op->kept)
  {
    MeosArray *own = relate_operand_own(op);
    /* The union of an operand a context lends lives as long as the context,
     * as its own edges do (#relate_operand_own) */
#if ! MEOS
    MemoryContext oldctx = op->index ?
      MemoryContextSwitchTo(TopMemoryContext) : NULL;
#endif
    *op->kept = relate_reads_union(op->geom) ?
      relate_union_edges(op->geom, own) : own;
#if ! MEOS
    if (op->index)
      MemoryContextSwitchTo(oldctx);
#endif
  }
  return *op->kept;
}

/**
 * @brief The two operands of a relationship, each carrying its edges
 * @details Every step of a relationship reads the edges of the same two
 * geometries, and reading the edges of a multi-surface as those of the union
 * of its members is what a call on such a geometry mostly costs. The two
 * operands are therefore extracted ONCE where the relationship begins, and
 * each step borrows the edges from here rather than extracting them again
 */
typedef struct
{
  RelateOperand op[2];  /**< The two operands, in the order asked about */
} RelateOperands;

/**
 * @brief Extract the edges of both operands of a relationship
 */
static void
relate_operands_init(RelateOperands *ops, const LWGEOM *g1, const LWGEOM *g2)
{
  const LWGEOM *g[2] = {g1, g2};
  for (int k = 0; k < 2; k++)
  {
    /* The union is cut from the geometry's own edges, which are kept for the
     * questions they answer exactly (#meos_intersects); both are read the
     * first time a question asks for them (#relate_operand_own,
     * #relate_operand_arr) */
    ops->op[k].geom = g[k];
    ops->op[k].own = ops->op[k].arr = NULL;
    ops->op[k].ownkept = &ops->op[k].own;
    ops->op[k].kept = &ops->op[k].arr;
    ops->op[k].index = NULL;
  }
  return;
}

/**
 * @brief Release the edges of both operands of a relationship
 * @details As #relate_ctx_free releases those of a context: the own edges
 * where the union was cut from them, and the union where a question read it
 */
static void
relate_operands_free(RelateOperands *ops)
{
  for (int k = 0; k < 2; k++)
  {
    if (ops->op[k].own && ops->op[k].own != ops->op[k].arr)
      meos_array_destroy(ops->op[k].own);
    if (ops->op[k].arr)
      meos_array_destroy(ops->op[k].arr);
  }
  return;
}

/**
 * @brief Return the edges of a geometry, reading the ones a relationship has
 * already extracted where the geometry is one of its two operands
 * @param[in] ops Operands of the relationship, NULL where the caller stands
 * outside one, in which case the edges are extracted as before
 * @param[in] geom Geometry
 * @return The edges, which the caller gives back with #relate_return_edges
 * rather than releasing itself
 */
static MeosArray *
relate_borrow_edges(const RelateOperands *ops, const LWGEOM *geom)
{
  if (ops)
    for (int i = 0; i < 2; i++)
      if (ops->op[i].geom == geom)
        return relate_operand_arr(&ops->op[i]);
  return relate_extract_edges(geom);
}

/**
 * @brief Give back edges #relate_borrow_edges answered
 * @details Only edges it extracted for this caller are released: the ones the
 * relationship owns are released once, where it ends
 */
static void
relate_return_edges(const RelateOperands *ops, MeosArray *arr)
{
  if (ops)
    for (int i = 0; i < 2; i++)
      if (*ops->op[i].kept == arr)
        return;
  meos_array_destroy(arr);
  return;
}

/**
 * @brief Accumulate a dimension into a DE-9IM cell
 * @details A cell records the @b maximum dimension of the corresponding
 * intersection, so a contribution can only raise it. Assigning instead of
 * accumulating lets a later zero-dimensional contribution overwrite an
 * earlier one-dimensional one, which makes the matrix depend on the order in
 * which the edge pairs happen to be visited
 */
static inline void
de9im_add(int8_t *cell, int8_t dim)
{
  if (dim > *cell)
    *cell = dim;
  return;
}

/**
 * @brief Convert a DE-9IM dimension to its character representation
 */
static inline char
de9im_dim_char(int8_t dim)
{
  switch (dim)
  {
    case -1: return 'F';
    case  0: return '0';
    case  1: return '1';
    case  2: return '2';
    default: return 'F';
  }
}

/**
 * @brief Convert a DE-9IM matrix to its 9-character representation
 */
static void
de9im_to_string(const MeosDE9IM *m, char result[10])
{
  result[0] = de9im_dim_char(m->ii);
  result[1] = de9im_dim_char(m->ib);
  result[2] = de9im_dim_char(m->ie);

  result[3] = de9im_dim_char(m->bi);
  result[4] = de9im_dim_char(m->bb);
  result[5] = de9im_dim_char(m->be);

  result[6] = de9im_dim_char(m->ei);
  result[7] = de9im_dim_char(m->eb);
  result[8] = de9im_dim_char(m->ee);

  result[9] = '\0';
  return;
}

/**
 * @brief Return true if a DE-9IM matrix satisfies a pattern
 * @details The characters of a pattern are:
 * - T, any non-empty intersection
 * - F, an empty intersection
 * - 0, a point
 * - 1, a line
 * - 2, an area
 * - *, any intersection
 */
bool
de9im_match(const char matrix[10], const char pattern[10])
{
  for (int i = 0; i < 9; i++)
  {
    char p = pattern[i];
    if (p == '*')
      continue;
    if (p == 'T')
    {
      if (matrix[i] == 'F')
        return false;
      continue;
    }
    if (p == 'F')
    {
      if (matrix[i] != 'F')
        return false;
      continue;
    }
    if (p != matrix[i])
      return false;
  }
  return true;
}

/*****************************************************************************
 * Geometry classification
 *****************************************************************************/

/**
 * @brief Return true if a geometry contains a 2-dimensional region
 */
bool
relate_is_areal(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return false;

  switch (geom->type)
  {
    case POLYGONTYPE:
    case MULTIPOLYGONTYPE:
    case TRIANGLETYPE:
    case CURVEPOLYTYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
      return true;
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
      {
        if (relate_is_areal(col->geoms[i]))
          return true;
      }
      return false;
    }
    default:
      return false;
  }
}


/**
 * @brief Return true if a geometry is a point geometry
 */
static bool
relate_is_point(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return false;
  return geom->type == POINTTYPE || geom->type == MULTIPOINTTYPE;
}

/**
 * @brief Return true if a geometry contains 1-dimensional features
 */
static bool
relate_is_linear(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return false;

  switch (geom->type)
  {
    case LINETYPE:
    case MULTILINETYPE:
    case CIRCSTRINGTYPE:
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
      return true;
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
      {
        if (relate_is_linear(col->geoms[i]))
          return true;
      }
      return false;
    }
    default:
      return false;
  }
}

/**
 * @brief Return true if the vertices of a point array are all one point
 * @details The vertices are input values, so they coincide exactly where their
 * coordinates are equal
 */
static bool
relate_ptarray_is_point(const POINTARRAY *pa)
{
  const POINT2D *p0 = getPoint2d_cp(pa, 0);
  for (uint32_t i = 1; i < pa->npoints; i++)
  {
    const POINT2D *p = getPoint2d_cp(pa, i);
    if (p->x != p0->x || p->y != p0->y)
      return false;
  }
  return true;
}

/**
 * @brief Return true if a linear geometry draws points rather than curves
 * @details A curve whose vertices all coincide draws the one point they
 * denote, whatever the number of vertices writing it, so a linear geometry
 * every component of which is such a curve is a point geometry to the
 * relation: its interior is that set of points and its boundary is empty. A
 * collection draws points when each of its members is a point geometry or
 * draws points
 */
static bool
relate_linear_draws_points(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return false;
  switch (geom->type)
  {
    case LINETYPE:
      return relate_ptarray_is_point(((const LWLINE *) geom)->points);
    case CIRCSTRINGTYPE:
      return relate_ptarray_is_point(((const LWCIRCSTRING *) geom)->points);
    case COMPOUNDTYPE:
    case MULTILINETYPE:
    case MULTICURVETYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
      {
        const LWGEOM *member = col->geoms[i];
        if (lwgeom_is_empty(member) ||
            (geom->type == COLLECTIONTYPE && relate_is_point(member)))
          continue;
        if (! relate_linear_draws_points(member))
          return false;
      }
      return true;
    }
    default:
      return false;
  }
}

/**
 * @brief Return the topological dimension of a geometry
 */
static int
relate_dimension(const LWGEOM *geom)
{
  if (relate_is_areal(geom))
    return 2;
  if (relate_is_linear(geom))
    return relate_linear_draws_points(geom) ? 0 : 1;
  if (relate_is_point(geom))
    return 0;
  return -1;
}

/*****************************************************************************
 * Point / Point
 *****************************************************************************/

/*****************************************************************************
 * Linear geometry boundary handling
 *****************************************************************************/

/**
 * @brief Return true if two coordinates of constructed points differ by at
 * most #coordinate_rounding
 * @details That rounding grows with the larger magnitude, which is at most the
 * magnitude of the first coordinate plus their difference. A difference within
 * the rounding of the first coordinate is therefore one coordinate, and a
 * difference above twice that rounding is two. Only the narrow range between
 * reads the magnitude of the second, so the answer to most calls, two
 * coordinates far apart, costs a comparison with a value of the first alone
 */
static inline bool
relate_same_coordinate(double c1, double c2)
{
  double d = fabs(c1 - c2);
  double r1 = coordinate_rounding(c1, c1);
  if (d > 2.0 * r1)
    return false;
  if (d <= r1)
    return true;
  return d <= coordinate_rounding(c1, c2);
}

/**
 * @brief Return true if two points the engine constructed are one point
 * @details A point constructed from input coordinates, such as the end of a
 * boundary portion or an intersection, misses where it should land by the
 * rounding of its coordinates (#coordinate_rounding), so two constructions of
 * one point agree within that distance. The distance scales with the
 * coordinates: an absolute one reads two distinct points as one once they
 * stand closer than it, whatever the size of the geometry around them, and
 * the same pair scaled by a power of two answers differently at two sizes.
 * Two input vertices are compared exactly (#relate_points_equal).
 */
static inline bool
relate_same_point(double x1, double y1, double x2, double y2)
{
  return relate_same_coordinate(x1, x2) && relate_same_coordinate(y1, y2);
}

/**
 * @brief Return true if an edge has non-zero length
 * @details The ends of an edge are input vertices, so it has no length
 * exactly where they are equal
 */
static inline bool
relate_edge_nonempty(const Edge *e)
{
  return e->x1 != e->x2 || e->y1 != e->y2;
}

/**
 * @brief Return true if two points are one point
 * @details Two input vertices are one point exactly where their coordinates
 * are equal. A constructed point is rounded, and reads as another point
 * within the rounding of their coordinates (#relate_same_point)
 * @param[in] x1,y1,x2,y2 Coordinates of the two points
 * @param[in] vertex True if both points are input vertices
 */
static inline bool
relate_points_equal(double x1, double y1, double x2, double y2, bool vertex)
{
  return vertex ? (x1 == x2 && y1 == y2) : relate_same_point(x1, y1, x2, y2);
}

/**
 * @brief Return true if a point is an endpoint of an edge
 */
static inline bool
relate_point_is_edge_endpoint(double x, double y, const Edge *e)
{
  return relate_same_point(x, y, e->x1, e->y1) ||
         relate_same_point(x, y, e->x2, e->y2);
}

/**
 * @brief Return true if a point occurs an odd number of times among the
 * endpoints of a linear geometry
 * @details This implements the endpoint parity rule used for the boundary of
 * linear geometries:
 * - odd number of occurrences -> boundary
 * - even number of occurrences -> interior
 * Closed lines consequently have an empty boundary.
 * The function works on the extracted Edge representation, so circular arcs
 * remain exact.
 * @param[in] x,y Coordinates of the point
 * @param[in] edges,nedges Edges of the linear geometry
 * @param[in] vertex True if the point is an input vertex, compared with the
 * endpoints exactly (#relate_points_equal)
 */
static bool
relate_point_on_linear_boundary(double x, double y, Edge **edges, int nedges,
  bool vertex)
{
  int count = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINESEG && e->etype != EDGE_LINEARC)
      continue;
    if (!relate_edge_nonempty(e))
      continue;
    if (relate_points_equal(x, y, e->x1, e->y1, vertex))
      count++;
    if (relate_points_equal(x, y, e->x2, e->y2, vertex))
      count++;
  }
  return (count & 1) != 0;
}

/**
 * @brief The endpoints of the curves of a linear geometry, sorted by their
 * coordinates
 * @details The sort lets the endpoints a point equals be found by a binary
 * search rather than by a scan of every edge. The endpoints are those
 * #relate_point_on_linear_boundary counts: both ends of every non-empty
 * straight or circular linear edge. An endpoint with a coordinate that is not
 * a number equals no point, so it is left out
 */
typedef struct
{
  Edge **edges;     /**< Edges of the linear geometry */
  int nedges;       /**< Number of edges */
  POINT2D *points;  /**< Endpoints, sorted by x then y, NULL until asked */
  int count;        /**< Number of endpoints */
} RelateLinearEnds;

/**
 * @brief Order two points by x, then by y
 */
static int
relate_point2d_cmp(const void *a, const void *b)
{
  const POINT2D *p = (const POINT2D *) a, *q = (const POINT2D *) b;
  if (p->x < q->x)
    return -1;
  if (p->x > q->x)
    return 1;
  if (p->y < q->y)
    return -1;
  if (p->y > q->y)
    return 1;
  return 0;
}

/**
 * @brief Set the endpoints of a linear edge array, collected and sorted on the
 * first question asked of them
 * @param[out] ends Endpoints
 * @param[in] edges,nedges Edges of the linear geometry
 */
static void
relate_linear_ends_init(RelateLinearEnds *ends, Edge **edges, int nedges)
{
  ends->edges = edges;
  ends->nedges = nedges;
  ends->points = NULL;
  ends->count = 0;
  return;
}

/**
 * @brief Collect and sort the endpoints of a linear edge array
 */
static void
relate_linear_ends_sort(RelateLinearEnds *ends)
{
  Edge **edges = ends->edges;
  int nedges = ends->nedges;
  ends->points = palloc(sizeof(POINT2D) * (size_t) (2 * nedges + 1));
  ends->count = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINESEG && e->etype != EDGE_LINEARC)
      continue;
    if (! relate_edge_nonempty(e))
      continue;
    if (! isnan(e->x1) && ! isnan(e->y1))
    {
      ends->points[ends->count].x = e->x1;
      ends->points[ends->count++].y = e->y1;
    }
    if (! isnan(e->x2) && ! isnan(e->y2))
    {
      ends->points[ends->count].x = e->x2;
      ends->points[ends->count++].y = e->y2;
    }
  }
  qsort(ends->points, (size_t) ends->count, sizeof(POINT2D),
    relate_point2d_cmp);
  return;
}

/**
 * @brief Return true if a point is on the Mod-2 boundary of a linear geometry,
 * read from its sorted endpoints
 * @details The answer is the one #relate_point_on_linear_boundary gives: the
 * endpoints the point equals are counted, exactly for an input vertex and
 * within the MEOS tolerance on each coordinate for a constructed point, and
 * those endpoints lie in the run of x values the tolerance admits
 * @param[in] ends Sorted endpoints
 * @param[in] x,y Coordinates of the point
 * @param[in] vertex True if the point is an input vertex
 */
static bool
relate_linear_ends_boundary(RelateLinearEnds *ends, double x, double y,
  bool vertex)
{
  if (! ends->points)
    relate_linear_ends_sort(ends);
  double tol = vertex ? 0.0 : MEOS_GEOM_TOLERANCE;
  int lo = 0, hi = ends->count;
  while (lo < hi)
  {
    int mid = lo + (hi - lo) / 2;
    if (ends->points[mid].x < x - tol)
      lo = mid + 1;
    else
      hi = mid;
  }
  int count = 0;
  for (int i = lo; i < ends->count && ends->points[i].x <= x + tol; i++)
    if (relate_points_equal(x, y, ends->points[i].x, ends->points[i].y,
          vertex))
      count++;
  return (count & 1) != 0;
}

/**
 * @brief Classify a point with respect to a complete linear geometry
 * @details Return:
 *   0 = interior
 *   1 = boundary
 *   2 = exterior
 * Note that an endpoint of an individual edge does NOT automatically make the
 * point part of the geometry boundary. Endpoint parity is evaluated over the
 * complete linear geometry.
 *
 * Whether an input vertex lies on a straight edge is a question on three
 * input vertices, decided exactly (#point_on_segment_exact); a constructed
 * point is rounded and is located within the tolerance its coordinates call
 * for (#point_on_segment)
 * @param[in] x,y Coordinates of the point
 * @param[in] edges,nedges Edges of the linear geometry
 * @param[in] vertex True if the point is an input vertex rather than a point
 * the engine constructs
 */
static int
relate_point_in_linear(double x, double y, Edge **edges, int nedges,
  bool vertex)
{
  bool found = false;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINESEG && e->etype != EDGE_LINEARC)
      continue;
    bool on = false;
    if (e->etype == EDGE_LINESEG)
      on = vertex ? point_on_segment_exact(x, y, e->x1, e->y1, e->x2, e->y2) :
        point_on_segment(x, y, e->x1, e->y1, e->x2, e->y2);
    else if (e->etype == EDGE_LINEARC)
      on = point_on_arc(x, y, e);
    if (on)
    {
      found = true;
      break;
    }
  }

  if (! found)
    return 2;
  if (relate_point_on_linear_boundary(x, y, edges, nedges, vertex))
    return 1;
  return 0;
}

/**
 * @brief Return the points of a point geometry
 * @details A multipoint and a collection share the collection memory layout,
 * so a point geometry is walked to any depth the same way #geom_extract_edges
 * walks one. Reading the components of a collection as points instead reads a
 * nested multipoint as a point
 */
static int
relate_count_points(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return 0;
  if (geom->type == POINTTYPE)
    return 1;
  /* A curve whose vertices all coincide contributes the one point it draws */
  if (geom->type == LINETYPE || geom->type == CIRCSTRINGTYPE)
    return relate_linear_draws_points(geom) ? 1 : 0;
  if (geom->type != MULTIPOINTTYPE && geom->type != COLLECTIONTYPE &&
      geom->type != COMPOUNDTYPE && geom->type != MULTILINETYPE &&
      geom->type != MULTICURVETYPE)
    return 0;
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  int result = 0;
  for (uint32_t i = 0; i < col->ngeoms; i++)
    result += relate_count_points(col->geoms[i]);
  return result;
}

/**
 * @brief Append the points of a point geometry to an array
 */
static void
relate_extract_points_iter(const LWGEOM *geom, POINT2D *result, int *count)
{
  if (! geom || lwgeom_is_empty(geom))
    return;
  if (geom->type == POINTTYPE)
  {
    result[*count] = *getPoint2d_cp(((const LWPOINT *) geom)->point, 0);
    (*count)++;
    return;
  }
  /* A curve whose vertices all coincide contributes the one point it draws */
  if (geom->type == LINETYPE || geom->type == CIRCSTRINGTYPE)
  {
    if (relate_linear_draws_points(geom))
    {
      const POINTARRAY *pa = (geom->type == LINETYPE) ?
        ((const LWLINE *) geom)->points :
        ((const LWCIRCSTRING *) geom)->points;
      result[*count] = *getPoint2d_cp(pa, 0);
      (*count)++;
    }
    return;
  }
  if (geom->type != MULTIPOINTTYPE && geom->type != COLLECTIONTYPE &&
      geom->type != COMPOUNDTYPE && geom->type != MULTILINETYPE &&
      geom->type != MULTICURVETYPE)
    return;
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  for (uint32_t i = 0; i < col->ngeoms; i++)
    relate_extract_points_iter(col->geoms[i], result, count);
  return;
}

/**
 * @brief Return the points of a point geometry
 * @details A POINT and a MULTIPOINT are the same kind of set to the relation,
 * one of them holding a single element, so both are related by the same code.
 * So is a linear geometry whose curves each draw a single point, which
 * contributes those points
 * @param[in] geom Point geometry
 * @param[out] count Number of points, zero for an empty geometry
 */
static POINT2D *
relate_extract_points(const LWGEOM *geom, int *count)
{
  POINT2D *result = palloc(sizeof(POINT2D) *
    (size_t) (relate_count_points(geom) + 1));
  *count = 0;
  relate_extract_points_iter(geom, result, count);
  return result;
}

/**
 * @brief Return true if a point belongs to a set of points
 * @details The point and the set are input vertices, so the point belongs to
 * the set exactly where its coordinates are those of one of its points
 */
static bool
relate_point_in_points(double x, double y, const POINT2D *points, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (relate_points_equal(x, y, points[i].x, points[i].y, true))
      return true;
  }
  return false;
}

/**
 * @brief Compute the DE-9IM matrix for two point geometries
 * @details A point geometry is its own interior and has an empty boundary, so
 * the boundary row and the boundary column stay F and the two interiors are
 * compared element by element
 */
static void
relate_point_point(const LWGEOM *g1, const LWGEOM *g2, MeosDE9IM *m)
{
  int n1, n2;
  POINT2D *p1 = relate_extract_points(g1, &n1);
  POINT2D *p2 = relate_extract_points(g2, &n2);

  for (int i = 0; i < n1; i++)
  {
    if (relate_point_in_points(p1[i].x, p1[i].y, p2, n2))
      de9im_add(&m->ii, 0);
    else
      de9im_add(&m->ie, 0);
  }
  for (int i = 0; i < n2; i++)
  {
    if (! relate_point_in_points(p2[i].x, p2[i].y, p1, n1))
      de9im_add(&m->ei, 0);
  }

  de9im_add(&m->ee, 2);
  pfree(p1); pfree(p2);
  return;
}

/**
 * @brief Compute the DE-9IM matrix for a point geometry and a linear geometry
 */
static void
relate_point_linear(const LWGEOM *point_geom, const LWGEOM *line_geom,
  const RelateOperands *ops, MeosDE9IM *m)
{
  int np;
  POINT2D *points = relate_extract_points(point_geom, &np);
  MeosArray *arr = relate_borrow_edges(ops, line_geom);
  int nedges = (int) arr->count;
  Edge **edges = palloc(sizeof(Edge *) * (size_t) (nedges + 1));
  for (int i = 0; i < nedges; i++)
    edges[i] = (Edge *) meos_array_get_intl(arr, i);

  /* Each point lies in the interior of the linear geometry, on its Mod-2
   * boundary, or outside it. A point geometry has an empty boundary, so its
   * boundary row stays F */
  for (int i = 0; i < np; i++)
  {
    switch (relate_point_in_linear(points[i].x, points[i].y, edges, nedges,
      true))
    {
      case 0:
        de9im_add(&m->ii, 0);
        break;
      case 1:
        de9im_add(&m->ib, 0);
        break;
      default:
        de9im_add(&m->ie, 0);
        break;
    }
  }

  /* Removing a finite set of points from a linear geometry leaves a
   * 1-dimensional part of its interior outside them */
  de9im_add(&m->ei, 1);

  /* Each Mod-2 boundary point of the linear geometry that is none of the
   * points lies in their exterior */
  int nb;
  POINT2D *bpts = relate_linear_boundary_points(edges, nedges, &nb);
  for (int i = 0; i < nb; i++)
  {
    if (relate_point_in_points(bpts[i].x, bpts[i].y, points, np))
      continue;
    de9im_add(&m->eb, 0);
    break;
  }

  de9im_add(&m->ee, 2);
  pfree(bpts); pfree(points); pfree(edges); relate_return_edges(ops, arr);
  return;
}

/**
 * @brief Compute the DE-9IM matrix for a point geometry and an areal geometry
 */
static void
relate_point_area(const LWGEOM *point_geom, const LWGEOM *area_geom,
  const RelateOperands *ops, MeosDE9IM *m)
{
  int np;
  POINT2D *points = relate_extract_points(point_geom, &np);
  MeosArray *arr = relate_borrow_edges(ops, area_geom);
  int nedges = (int) arr->count;
  Edge **edges = palloc(sizeof(Edge *) * (size_t) (nedges + 1));
  for (int i = 0; i < nedges; i++)
    edges[i] = (Edge *) meos_array_get_intl(arr, i);

  /* Each point lies in the interior of the area, on its boundary, or outside
   * it. A point geometry has an empty boundary, so its boundary row stays F */
  for (int i = 0; i < np; i++)
  {
    switch (relate_point_in_area(points[i].x, points[i].y, edges, nedges,
      true))
    {
      case 0:
        de9im_add(&m->ii, 0);
        break;
      case 1:
        de9im_add(&m->ib, 0);
        break;
      default:
        de9im_add(&m->ie, 0);
        break;
    }
  }

  /* The interior of an areal geometry is two-dimensional and its boundary is
   * one-dimensional, and a finite set of points covers neither */
  de9im_add(&m->ei, 2);
  de9im_add(&m->eb, 1);
  de9im_add(&m->ee, 2);

  pfree(points); pfree(edges); relate_return_edges(ops, arr);
  return;
}

/*****************************************************************************
 * Point / Linear
 *****************************************************************************/

/*****************************************************************************
 * Linear / Point
 *****************************************************************************/

/**
 * @brief Set the DE-9IM matrix of a linear geometry and a point
 */
static void
relate_linear_point(const LWGEOM *line_geom, const LWGEOM *point_geom,
  const RelateOperands *ops, MeosDE9IM *m)
{
  MeosDE9IM tmp;
  de9im_init(&tmp);
  relate_point_linear(point_geom, line_geom, ops, &tmp);

  /*
   * Transpose:
   *   II -> II
   *   IB -> BI
   *   IE -> EI
   *   BI -> IB
   *   BB -> BB
   *   BE -> EB
   *   EI -> IE
   *   EB -> BE
   *   EE -> EE
   */
  m->ii = tmp.ii;
  m->ib = tmp.bi;
  m->ie = tmp.ei;

  m->bi = tmp.ib;
  m->bb = tmp.bb;
  m->be = tmp.eb;

  m->ei = tmp.ie;
  m->eb = tmp.be;
  m->ee = tmp.ee;

  return;
}

/*****************************************************************************
 * Linear / Linear
 *****************************************************************************/

/**
 * @brief Return true if an intersection between two linear edges contains
 * a one-dimensional portion, and report the covered parameter interval
 * @details The function preserves the exact line/arc intersection
 * @param[in] a,b Edges to intersect
 * @param[out] t0,t1 Interval of @p a covered by @p b, only set on success
 */
static bool
relate_linear_edges_overlap(const Edge *a, const Edge *b, double *t0,
  double *t1)
{
  if (a->etype == EDGE_LINESEG && b->etype == EDGE_LINESEG)
  {
    IntersectResult r = linesegm_intersect(a->x1, a->y1, a->x2, a->y2,
        b->x1, b->y1, b->x2, b->y2);
    if (r.type != INTERSECT_OVERLAP)
      return false;
    /* The parameters are expressed on the first edge */
    *t0 = r.t0;
    *t1 = r.t1;
    return true;
  }

  /* A line and a circular arc can intersect only in points unless
   * the line is degenerate, which is excluded here. */
  return false;
}

/*****************************************************************************
 * Point / Area
 *****************************************************************************/

/*****************************************************************************
 * Area / Point
 *****************************************************************************/

/**
 * @brief Set the DE-9IM matrix of an areal geometry and a point
 */
static void
relate_area_point(const LWGEOM *area_geom, const LWGEOM *point_geom,
  const RelateOperands *ops, MeosDE9IM *m)
{
  MeosDE9IM tmp;
  de9im_init(&tmp);
  relate_point_area(point_geom, area_geom, ops, &tmp);

  /* Transpose the matrix */
  m->ii = tmp.ii;
  m->ib = tmp.bi;
  m->ie = tmp.ei;

  m->bi = tmp.ib;
  m->bb = tmp.bb;
  m->be = tmp.eb;

  m->ei = tmp.ie;
  m->eb = tmp.be;
  m->ee = tmp.ee;
  return;
}

/*****************************************************************************
 * Linear / Area
 *****************************************************************************/

/**
 * @brief Compute the parameter of a point on an arc
 * @details The returned value is in [0,1], where 0 corresponds to theta0 and
 * 1 corresponds to theta1 following the orientation of the arc.
 */
static double
relate_arc_parameter(const Edge *e, double x, double y)
{
  double phi = angle_normalize(atan2(y - e->cy, x - e->cx));
  double sweep;
  double off;
  if (e->ccw)
  {
    sweep = angle_normalize(e->theta1 - e->theta0);
    off = angle_normalize(phi - e->theta0);
  }
  else
  {
    sweep = angle_normalize(e->theta0 - e->theta1);
    off = angle_normalize(e->theta0 - phi);
  }
  if (sweep < MEOS_GEOM_TOLERANCE)
    return 0.0;
  double t = off / sweep;
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;
  return t;
}

/**
 * @brief Compute the point at parameter t on an edge
 * @details This preserves the exact circular representation of an arc.
 */
static void
relate_edge_point(const Edge *e, double t, double *x, double *y)
{
  if (e->etype == EDGE_LINESEG || e->etype == EDGE_POLYSEG)
  {
    *x = e->x1 + t * (e->x2 - e->x1);
    *y = e->y1 + t * (e->y2 - e->y1);
    return;
  }
  /* Circular arc */
  double sweep = e->ccw ?
    angle_normalize(e->theta1 - e->theta0) :
    angle_normalize(e->theta0 - e->theta1);
  double theta = e->ccw ?
    e->theta0 + t * sweep :
    e->theta0 - t * sweep;
  *x = e->cx + e->radius * cos(theta);
  *y = e->cy + e->radius * sin(theta);
  return;
}

/**
 * @brief Add an intersection parameter to an array
 */
static void
relate_add_parameter(double t, double *params, int *nparams, int maxparams)
{
  if (t < -MEOS_GEOM_TOLERANCE || t > 1.0 + MEOS_GEOM_TOLERANCE)
    return;
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;

  /* Avoid inserting the same intersection several times. This is
   * particularly important at polygon vertices where two boundary
   * edges meet. */
  for (int i = 0; i < *nparams; i++)
  {
    if (fabs(params[i] - t) <= MEOS_GEOM_TOLERANCE)
      return;
  }
  if (*nparams < maxparams)
    params[(*nparams)++] = t;
}


/**
 * @brief Return true if two circular arcs overlap in a non-zero-length portion
 * @details  The arcs must lie on the same supporting circle.
 * We use the endpoints of both arcs as candidate split points and test
 * whether an interval between consecutive candidates belongs to both
 * arcs. No polygonization is involved.
 */
static bool
relate_arcs_overlap(const Edge *a, const Edge *b)
{
  if (! arc_same_circle(a, b))
    return false;

  /* Collect the four endpoint parameters of b with respect to a.
   * If an endpoint is inside a, it provides a candidate boundary of
   * the overlap. */
  double p[4];
  int np = 0;
  double t;
  if (point_on_arc(b->x1, b->y1, a))
  {
    t = relate_arc_parameter(a, b->x1, b->y1);
    p[np++] = t;
  }
  if (point_on_arc(b->x2, b->y2, a))
  {
    t = relate_arc_parameter(a, b->x2, b->y2);
    p[np++] = t;
  }
  if (point_on_arc(a->x1, a->y1, b))
  {
    t = relate_arc_parameter(a, a->x1, a->y1);
    p[np++] = t;
  }
  if (point_on_arc(a->x2, a->y2, b))
  {
    t = relate_arc_parameter(a, a->x2, a->y2);
    p[np++] = t;
  }

  /* The arcs can overlap without having an endpoint strictly inside
   * the other arc only when they are effectively coincident. In that
   * case an endpoint of one arc is necessarily on the other arc. */
  if (np < 2)
    return false;

  /* Remove duplicate parameters. */
  for (int i = 0; i < np; i++)
  {
    for (int j = i + 1; j < np;)
    {
      if (fabs(p[i] - p[j]) <= MEOS_GEOM_TOLERANCE)
      {
        for (int k = j; k < np - 1; k++)
          p[k] = p[k + 1];
        np--;
      }
      else
        j++;
    }
  }

  /* Test intervals between all candidate points. */
  for (int i = 0; i < np; i++)
  {
    for (int j = i + 1; j < np; j++)
    {
      if (fabs(p[j] - p[i]) <= MEOS_GEOM_TOLERANCE)
        continue;
      double tm = (p[i] + p[j]) * 0.5;
      double x, y;
      relate_edge_point(a, tm, &x, &y);
      if (point_on_arc(x, y, b))
        return true;
    }
  }
  return false;
}

/**
 * @brief Add the intersection points of two circular arcs
 * @details Returns the number of point intersections added to x/y.
 * If the arcs overlap over a non-zero-length portion, overlap is set
 * to true and no point is required for the one-dimensional component.
 */
int
relate_arc_arc_points(const Edge *a, const Edge *b, double x[2], double y[2],
  bool *overlap)
{
  *overlap = false;
  /* One supporting circle */
  if (arc_same_circle(a, b))
  {
    if (relate_arcs_overlap(a, b))
    {
      *overlap = true;
      return 0;
    }

    /* They may touch at one or more common endpoints without having
     * a one-dimensional overlap. */
    int n = 0;
    if (point_on_arc(a->x1, a->y1, b))
    {
      x[n] = a->x1;
      y[n++] = a->y1;
    }
    if (n < 2 && point_on_arc(a->x2, a->y2, b) &&
        ! relate_same_point(a->x1, a->y1, a->x2, a->y2))
    {
      x[n] = a->x2;
      y[n++] = a->y2;
    }
    return n;
  }

  double dx = b->cx - a->cx;
  double dy = b->cy - a->cy;
  double d = hypot(dx, dy);
  /* Two circles about one centre never meet */
  if (d == 0.0)
    return 0;
  double aa = (d * d + a->radius * a->radius - b->radius * b->radius) /
    (2.0 * d);
  double h2 = a->radius * a->radius - aa * aa;
  /* The square of the half-chord read against the rounding of its own terms:
   * below it the circles are separate or nested, within it tangent */
  double bound = arc_half_chord_rounding(d, a->radius, b->radius, aa);
  if (h2 < - bound)
    return 0;
  bool tangent = h2 <= bound;
  double h = tangent ? 0.0 : sqrt(h2);
  double ux = dx / d;
  double uy = dy / d;
  double mx = a->cx + aa * ux;
  double my = a->cy + aa * uy;
  int n = 0;
  for (int k = 0; k < 2; k++)
  {
    double px = mx + (k ? h : -h) * (-uy);
    double py = my + (k ? h : -h) * ux;
    /* The candidate is placed on both supporting circles by the solution
     * above, so only its angular position decides whether it belongs to the
     * two arcs. Asking whether it lies on them by its distance to the centres
     * instead measures the rounding of that solution against a tolerance the
     * radius does not enter, which rejects a genuine intersection of two arcs
     * of large radius. This is what #arcarc_intersect asks */
    if (! arc_contains_angle(a, atan2(py - a->cy, px - a->cx)) ||
        ! arc_contains_angle(b, atan2(py - b->cy, px - b->cx)))
      continue;
    /* Avoid duplicating a tangency point */
    if (n > 0 && relate_same_point(px, py, x[0], y[0]))
      continue;
    x[n] = px;
    y[n] = py;
    n++;
    if (tangent)
      break;
  }
  return n;
}

/**
 * @brief Set a segment of the given ends in the form the exact order of
 * meetings reads (#meets_order_exact), as #buffer_piece_from_edge sets one
 */
static void
relate_input_segment(double x1, double y1, double x2, double y2, Edge *out)
{
  memset(out, 0, sizeof(Edge));
  out->x1 = x1; out->y1 = y1;
  out->x2 = x2; out->y2 = y2;
  out->dx = x2 - x1; out->dy = y2 - y1;
  out->etype = EDGE_POLYSEG;
  return;
}

/* Where a meeting lies along a piece of an input segment
 * (#relate_piece_locate) */
typedef enum
{
  RELATE_PIECE_UNDECIDED,
  RELATE_PIECE_OUTSIDE,
  RELATE_PIECE_START,
  RELATE_PIECE_INSIDE,
  RELATE_PIECE_END
} RelatePiecePos;

/**
 * @brief Return where a meeting on an input segment lies against a piece of
 * it, ordering it exactly against the two ends of the piece
 * @details An end of the piece is the point its coordinates hold, or the
 * crossing of the input segment with the one recorded for it, which no double
 * holds; #meets_order_exact orders a meeting of either kind along the input
 * segment, exactly, and leaves undecided an order it cannot read
 * @param[in] piece Piece, running along @p line from its start to its end
 * @param[in] line Input segment of the piece
 * @param[in] m Meeting on @p line
 */
static RelatePiecePos
relate_piece_locate(const Edge *piece, const Edge *line, const MeetPoint *m)
{
  Edge crosser[2];
  MeetPoint ends[2];
  memset(ends, 0, sizeof(ends));
  for (int k = 0; k < 2; k++)
  {
    if (piece->cut && piece->crossed[k])
    {
      relate_input_segment(piece->crosser[k][0], piece->crosser[k][1],
        piece->crosser[k][2], piece->crosser[k][3], &crosser[k]);
      ends[k].kind = MEET_SEGSEG;
      ends[k].p = line;
      ends[k].q = &crosser[k];
    }
    else
    {
      ends[k].kind = MEET_VERTEX;
      ends[k].x = k ? piece->x2 : piece->x1;
      ends[k].y = k ? piece->y2 : piece->y1;
    }
  }
  int s = meets_order_exact(line, false, &ends[0], m);
  if (s == 2)
    return RELATE_PIECE_UNDECIDED;
  if (s >= 0)
    return s == 0 ? RELATE_PIECE_START : RELATE_PIECE_OUTSIDE;
  int t = meets_order_exact(line, false, m, &ends[1]);
  if (t == 2)
    return RELATE_PIECE_UNDECIDED;
  if (t >= 0)
    return t == 0 ? RELATE_PIECE_END : RELATE_PIECE_OUTSIDE;
  return RELATE_PIECE_INSIDE;
}

/**
 * @brief Return how two segments meet, as #linesegm_intersect answers it,
 * reading a piece of the boundary of a union on the input segment it is cut
 * from
 * @details An end of a piece is constructed on its input segment and rounded
 * once (#relate_union_edges), so it falls off that segment's line and to
 * either side of the segment that cut it: read off its ends, a piece running
 * along a segment of the other geometry crosses it or misses it, and a piece
 * ending on a segment crosses it a rounding step before its end or misses it.
 * Every question is therefore asked of the input segments. Whether the two
 * lie on one line is a sign of cross products of input coordinates
 * (#cross_product_sign), and the stretch they share runs between their own
 * ends. Otherwise the input segments meet at one point or nowhere
 * (#linesegm_intersect), and that point is ordered exactly against the ends
 * of each piece (#relate_piece_locate): it lies outside a piece, at one of
 * its ends -- the crossing it was cut at -- or inside it. An order the exact
 * reading leaves undecided is read on the rounded ends
 */
static IntersectResult
relate_segments_intersect(const Edge *a, const Edge *b)
{
  if (! a->cut && ! b->cut)
    return linesegm_intersect(a->x1, a->y1, a->x2, a->y2, b->x1, b->y1,
      b->x2, b->y2);
  Edge la, lb;
  relate_input_segment(a->cut ? a->sx : a->x1, a->cut ? a->sy : a->y1,
    a->cut ? a->ex : a->x2, a->cut ? a->ey : a->y2, &la);
  relate_input_segment(b->cut ? b->sx : b->x1, b->cut ? b->sy : b->y1,
    b->cut ? b->ex : b->x2, b->cut ? b->ey : b->y2, &lb);
  int oc = cross_product_sign(la.x1, la.y1, la.x2, la.y2, la.x1, la.y1,
    lb.x1, lb.y1);
  int od = cross_product_sign(la.x1, la.y1, la.x2, la.y2, la.x1, la.y1,
    lb.x2, lb.y2);
  if (oc == 0 && od == 0)
    return linesegm_intersect_collinear(a->x1, a->y1, a->x2, a->y2, b->x1,
      b->y1, b->x2, b->y2);

  /* Where the input segments meet, exactly: an input vertex lying on the
   * other segment, or the crossing of the two */
  IntersectResult none = {INTERSECT_NONE, 0, 0};
  IntersectResult ri = linesegm_intersect(la.x1, la.y1, la.x2, la.y2, lb.x1,
    lb.y1, lb.x2, lb.y2);
  if (ri.type == INTERSECT_NONE)
    return none;
  int oa = cross_product_sign(lb.x1, lb.y1, lb.x2, lb.y2, lb.x1, lb.y1,
    la.x1, la.y1);
  int ob = cross_product_sign(lb.x1, lb.y1, lb.x2, lb.y2, lb.x1, lb.y1,
    la.x2, la.y2);
  MeetPoint m;
  memset(&m, 0, sizeof(MeetPoint));
  m.kind = MEET_VERTEX;
  if (oa == 0)
  {
    m.x = la.x1; m.y = la.y1;
  }
  else if (ob == 0)
  {
    m.x = la.x2; m.y = la.y2;
  }
  else if (oc == 0)
  {
    m.x = lb.x1; m.y = lb.y1;
  }
  else if (od == 0)
  {
    m.x = lb.x2; m.y = lb.y2;
  }
  else
  {
    m.kind = MEET_SEGSEG;
    m.p = &la;
    m.q = &lb;
  }

  /* The meeting is one of the pieces only where it lies on both */
  RelatePiecePos pa = a->cut ? relate_piece_locate(a, &la, &m) :
    RELATE_PIECE_INSIDE;
  RelatePiecePos pb = b->cut ? relate_piece_locate(b, &lb, &m) :
    RELATE_PIECE_INSIDE;
  if (pa == RELATE_PIECE_UNDECIDED || pb == RELATE_PIECE_UNDECIDED)
  {
    IntersectResult r = linesegm_intersect(a->x1, a->y1, a->x2, a->y2, b->x1,
      b->y1, b->x2, b->y2);
    if (r.type == INTERSECT_OVERLAP)
      r.type = INTERSECT_POINT;
    return r;
  }
  if (pa == RELATE_PIECE_OUTSIDE || pb == RELATE_PIECE_OUTSIDE)
    return none;
  IntersectResult r = {INTERSECT_POINT, 0, 0};
  if (pa == RELATE_PIECE_START)
    r.t0 = 0.0;
  else if (pa == RELATE_PIECE_END)
    r.t0 = 1.0;
  else if (! a->cut)
    r.t0 = ri.t0;
  else
  {
    /* Inside the piece: its position, read on the rounded ends */
    double px = la.x1 + ri.t0 * la.dx, py = la.y1 + ri.t0 * la.dy;
    double t = (fabs(a->dx) >= fabs(a->dy)) ? (px - a->x1) / a->dx :
      (py - a->y1) / a->dy;
    r.t0 = (t < 0.0) ? 0.0 : ((t > 1.0) ? 1.0 : t);
  }
  return r;
}

/**
 * @brief Process the intersection between one linear edge and one polygon
 * boundary edge
 * @details Updates the DE-9IM cells corresponding to the intersection between
 * the linear geometry and the polygon boundary. The parameter array is
 * populated with all points at which the linear edge must be split.
 */
static void
relate_linear_area_edge_intersection(const Edge *line, const Edge *boundary,
  Edge **all_lines, int nlines, MeosDE9IM *m, double *params, int *nparams,
  int maxparams)
{
  /* Line / Poly */
  if (line->etype == EDGE_LINESEG && boundary->etype == EDGE_POLYSEG)
  {
    IntersectResult r = relate_segments_intersect(line, boundary);
    if (r.type == INTERSECT_NONE)
      return;
    if (r.type == INTERSECT_OVERLAP)
    {
      /* The interior of the linear geometry overlaps the polygon
       * boundary over a one-dimensional portion */
      m->ib = 1;
      relate_add_parameter(r.t0, params, nparams, maxparams);
      relate_add_parameter(r.t1, params, nparams, maxparams);
      return;
    }

    /* Point intersection */
    double x = line->x1 + r.t0 * line->dx;
    double y = line->y1 + r.t0 * line->dy;
    int lloc = relate_point_in_linear(x, y, all_lines, nlines, false);
    if (lloc == 0)
      de9im_add(&m->ib, 0);
    else if (lloc == 1)
      m->bb = 0;
    relate_add_parameter(r.t0, params, nparams, maxparams);
    return;
  }

  /* Line / PolyArc */
  if (line->etype == EDGE_LINESEG && boundary->etype == EDGE_POLYARC)
  {
    double roots[2];
    int n = arcsegm_intersect(line->x1, line->y1, line->dx, line->dy,
      boundary, roots);
    for (int i = 0; i < n; i++)
    {
      double t = roots[i];
      double x = line->x1 + t * line->dx;
      double y = line->y1 + t * line->dy;
      int lloc = relate_point_in_linear(x, y, all_lines, nlines, false);
      if (lloc == 0)
        de9im_add(&m->ib, 0);
      else if (lloc == 1)
        m->bb = 0;
      relate_add_parameter(t, params, nparams, maxparams);
    }
    return;
  }

  /*
   * Arc / Poly
   * arcsegm_intersect() parametrizes the straight segment, therefore
   * we call it with the polygon edge as the trajectory and convert
   * the resulting point to the parameter of the linear arc.
   */
  if (line->etype == EDGE_LINEARC && boundary->etype == EDGE_POLYSEG)
  {
    double roots[2];
    int n = arcsegm_intersect(boundary->x1, boundary->y1,
      boundary->dx, boundary->dy, line, roots);
    for (int i = 0; i < n; i++)
    {
      double x = boundary->x1 + roots[i] * boundary->dx;
      double y = boundary->y1 + roots[i] * boundary->dy;
      double t = relate_arc_parameter(line, x, y);
      int lloc = relate_point_in_linear(x, y, all_lines, nlines, false);
      if (lloc == 0)
        de9im_add(&m->ib, 0);
      else if (lloc == 1)
        m->bb = 0;
      relate_add_parameter(t, params, nparams, maxparams);
    }
    return;
  }

  /* Arc / PolyArc */
  if (line->etype == EDGE_LINEARC && boundary->etype == EDGE_POLYARC)
  {
    double ix[2], iy[2];
    bool overlap = false;
    int n = relate_arc_arc_points(line, boundary, ix, iy, &overlap);
    if (overlap)
    {
      /* A non-zero-length common arc is a one-dimensional
       * intersection between the line interior and polygon
       * boundary */
      m->ib = 1;
      /* Split at the endpoints of both arcs. This is sufficient
       * to classify the remaining portions of the linear edge. */
      if (point_on_arc(boundary->x1, boundary->y1, line))
        relate_add_parameter(
          relate_arc_parameter(line, boundary->x1, boundary->y1),
          params, nparams, maxparams);
      if (point_on_arc(boundary->x2, boundary->y2, line))
        relate_add_parameter(
          relate_arc_parameter(line, boundary->x2, boundary->y2),
          params, nparams, maxparams);
      if (point_on_arc(line->x1, line->y1, boundary))
        relate_add_parameter(0.0, params, nparams, maxparams);
      if (point_on_arc(line->x2, line->y2, boundary))
        relate_add_parameter(1.0, params, nparams, maxparams);
      return;
    }

    for (int i = 0; i < n; i++)
    {
      int lloc = relate_point_in_linear(ix[i], iy[i], all_lines, nlines,
        false);
      if (lloc == 0)
        de9im_add(&m->ib, 0);
      else if (lloc == 1)
        m->bb = 0;
      double t = relate_arc_parameter(line, ix[i], iy[i]);
      relate_add_parameter(t, params, nparams, maxparams);
    }
    return;
  }
  return;
}

/**
 * @brief Classify the open portion of a linear edge between two consecutive
 * parameters
 * @details Since the portion contains no intersection with the polygon
 * boundary, one representative point is sufficient to determine whether the
 * complete open portion belongs to the polygon interior or exterior.
 */
static void
relate_linear_area_interval(const Edge *line, double t0, double t1,
  const RelateEdges *ra, MeosDE9IM *m)
{
  if (t1 - t0 <= MEOS_GEOM_TOLERANCE)
    return;
  double tm = (t0 + t1) * 0.5;
  double x, y;
  relate_edge_point(line, tm, &x, &y);
  int loc = relate_point_in_area_index(x, y, ra, false);
  switch (loc)
  {
    case 0:
      /* A non-zero open portion of the linear geometry is inside
       * the area: I/I has dimension 1. */
      de9im_add(&m->ii, 1);
      break;
    case 1:
      /* A non-zero open portion coincides with the area boundary. */
      de9im_add(&m->ib, 1);
      break;
    case 2:
      /* A non-zero open portion is outside the area. */
      de9im_add(&m->ie, 1);
      break;
  }
  return;
}

/**
 * @brief Sort an array of parameters in increasing order
 */
static int
relate_parameter_cmp(const void *a, const void *b)
{
  const double da = *(const double *) a;
  const double db = *(const double *) b;
  if (da < db)
    return -1;
  if (da > db)
    return 1;
  return 0;
}

/*****************************************************************************
 * Linear / Linear
 *****************************************************************************/

/**
 * @brief Structure keeping a parameter interval of an edge
 */
typedef struct
{
  double t0;    /**< Start parameter, in [0,1] */
  double t1;    /**< End parameter, in [0,1], never less than t0 */
} RelateInterval;

/**
 * @brief Comparator ordering parameter intervals by their start
 */
static int
relate_interval_cmp(const void *a, const void *b)
{
  const RelateInterval *i1 = (const RelateInterval *) a;
  const RelateInterval *i2 = (const RelateInterval *) b;
  if (i1->t0 < i2->t0)
    return -1;
  if (i1->t0 > i2->t0)
    return 1;
  return 0;
}

/**
 * @brief Return true if a set of parameter intervals covers the whole
 * parameter range of an edge
 * @details The intervals are sorted in place and swept once, tracking the
 * furthest parameter reached without a gap. A gap of positive length is
 * a part of the edge that no interval covers
 * @param[in,out] intervals Intervals to test, reordered by the function
 * @param[in] count Number of intervals
 */
static bool
relate_intervals_cover(RelateInterval *intervals, int count)
{
  if (count == 0)
    return false;
  qsort(intervals, (size_t) count, sizeof(RelateInterval),
    relate_interval_cmp);
  if (intervals[0].t0 > MEOS_GEOM_TOLERANCE)
    return false;
  double reach = intervals[0].t1;
  for (int i = 1; i < count; i++)
  {
    if (reach >= 1.0 - MEOS_GEOM_TOLERANCE)
      break;
    if (intervals[i].t0 > reach + MEOS_GEOM_TOLERANCE)
      return false;
    if (intervals[i].t1 > reach)
      reach = intervals[i].t1;
  }
  return reach >= 1.0 - MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return the parameter intervals of an arc covered by another arc
 * @details Two arcs of the same circle can overlap in two disjoint pieces,
 * for example when both sweep more than half of the circle, so every piece
 * is reported separately instead of being merged into one enclosing interval
 * @param[in] a,b Arc edges
 * @param[out] out Covered intervals, expressed on @p a, at most three
 * @return Number of intervals reported
 */
static int
relate_arc_overlap_ranges(const Edge *a, const Edge *b, RelateInterval *out)
{
  if (! arc_same_circle(a, b))
    return 0;

  /* Candidate interval bounds: the ends of a, plus the ends of b that lie
   * on a, all expressed in the parameter space of a */
  double p[4];
  int np = 0;
  p[np++] = 0.0;
  p[np++] = 1.0;
  if (point_on_arc(b->x1, b->y1, a))
    p[np++] = relate_arc_parameter(a, b->x1, b->y1);
  if (point_on_arc(b->x2, b->y2, a))
    p[np++] = relate_arc_parameter(a, b->x2, b->y2);
  qsort(p, (size_t) np, sizeof(double), relate_parameter_cmp);

  int count = 0;
  for (int i = 0; i + 1 < np; i++)
  {
    if (p[i + 1] - p[i] <= MEOS_GEOM_TOLERANCE)
      continue;
    /* A piece is covered when its midpoint is on the other arc */
    double x, y;
    relate_edge_point(a, (p[i] + p[i + 1]) * 0.5, &x, &y);
    if (! point_on_arc(x, y, b))
      continue;
    out[count].t0 = p[i];
    out[count].t1 = p[i + 1];
    count++;
  }
  return count;
}

/**
 * @brief Return true if every edge of a linear geometry is entirely covered
 * by the edges of another linear geometry
 * @details An uncovered part of a linear geometry is one-dimensional, so the
 * answer decides the interior/exterior cell of the DE-9IM matrix. Only the
 * one-dimensional intersections cover anything: a line and an arc, and two
 * arcs of different circles, meet in isolated points that cover no length
 */
static bool
relate_linear_covered(Edge **edges, int nedges, Edge **others, int nothers)
{
  /* Two arcs contribute at most three pieces, a line pair exactly one */
  RelateInterval *intervals = palloc(sizeof(RelateInterval) *
    (size_t) (3 * nothers + 1));
  bool result = true;
  for (int i = 0; i < nedges && result; i++)
  {
    const Edge *a = edges[i];
    if (a->etype != EDGE_LINESEG && a->etype != EDGE_LINEARC)
      continue;
    if (! relate_edge_nonempty(a))
      continue;
    int count = 0;
    for (int j = 0; j < nothers; j++)
    {
      const Edge *b = others[j];
      if (b->etype != EDGE_LINESEG && b->etype != EDGE_LINEARC)
        continue;
      if (a->etype == EDGE_LINEARC && b->etype == EDGE_LINEARC)
        count += relate_arc_overlap_ranges(a, b, intervals + count);
      else if (relate_linear_edges_overlap(a, b, &intervals[count].t0,
          &intervals[count].t1))
        count++;
    }
    if (! relate_intervals_cover(intervals, count))
      result = false;
  }
  pfree(intervals);
  return result;
}

/**
 * @brief Return the boundary points of a linear geometry
 * @details Under the OGC Mod-2 rule a point belongs to the boundary of a
 * linear geometry when it is an endpoint of an odd number of the component
 * curves. Counting edge endpoints gives the same parity, because an interior
 * vertex of a chain is shared by exactly two edges and therefore cancels
 * @param[in] edges,nedges Edges of the geometry
 * @param[out] count Number of boundary points, possibly zero for a geometry
 * made of closed components
 */
static POINT2D *
relate_linear_boundary_points(Edge **edges, int nedges, int *count)
{
  /* Equal endpoints sit side by side once sorted, so each distinct endpoint is
   * one run, on the boundary exactly where the run is odd */
  RelateLinearEnds ends;
  relate_linear_ends_init(&ends, edges, nedges);
  relate_linear_ends_sort(&ends);
  POINT2D *result = palloc(sizeof(POINT2D) * (size_t) (2 * nedges + 1));
  *count = 0;
  for (int i = 0; i < ends.count; )
  {
    int j = i + 1;
    while (j < ends.count && ends.points[j].x == ends.points[i].x &&
        ends.points[j].y == ends.points[i].y)
      j++;
    if (((j - i) & 1) != 0)
      result[(*count)++] = ends.points[i];
    i = j;
  }
  pfree(ends.points);
  return result;
}

/**
 * @brief Append the isolated intersection points of two linear edges
 * @param[in] a,b Edges to intersect
 * @param[out] out Intersection points, at most two
 * @return Number of points appended
 */
static int
relate_linear_edge_points(const Edge *a, const Edge *b, POINT2D *out)
{
  int count = 0;
  if (a->etype == EDGE_LINESEG && b->etype == EDGE_LINESEG)
  {
    IntersectResult r = linesegm_intersect(a->x1, a->y1, a->x2, a->y2,
      b->x1, b->y1, b->x2, b->y2);
    if (r.type == INTERSECT_POINT)
    {
      relate_edge_point(a, r.t0, &out[count].x, &out[count].y);
      count++;
    }
  }
  else if (a->etype == EDGE_LINESEG && b->etype == EDGE_LINEARC)
  {
    double roots[2];
    int n = arcsegm_intersect(a->x1, a->y1, a->dx, a->dy, b, roots);
    for (int k = 0; k < n; k++)
    {
      relate_edge_point(a, roots[k], &out[count].x, &out[count].y);
      count++;
    }
  }
  else if (a->etype == EDGE_LINEARC && b->etype == EDGE_LINESEG)
  {
    double roots[2];
    int n = arcsegm_intersect(b->x1, b->y1, b->dx, b->dy, a, roots);
    for (int k = 0; k < n; k++)
    {
      relate_edge_point(b, roots[k], &out[count].x, &out[count].y);
      count++;
    }
  }
  else if (a->etype == EDGE_LINEARC && b->etype == EDGE_LINEARC)
  {
    double x[2], y[2];
    bool overlap = false;
    int n = relate_arc_arc_points(a, b, x, y, &overlap);
    for (int k = 0; k < n; k++)
    {
      out[count].x = x[k];
      out[count].y = y[k];
      count++;
    }
  }
  return count;
}

/**
 * @brief Return true if two straight edges meet at one point lying on neither
 * Mod-2 boundary
 * @details Where the two edges meet at one point is a question on their four
 * input vertices, answered exactly: at an end of either edge where that end
 * lies on the other (#point_on_segment_exact), and otherwise at a crossing
 * interior to both, which the rounded parameter of the crossing cannot tell
 * from a meeting at an end. An end is a boundary point where its coordinates are those of one.
 * A crossing interior to both edges is one only where a boundary point lies
 * on both edges: two segments of different directions share one point at
 * most, so that boundary point is the crossing
 * @param[in] a,b Straight edges
 * @param[in] b1,nb1,b2,nb2 Mod-2 boundary points of the two geometries
 */
static bool
relate_segments_meet_inside(const Edge *a, const Edge *b, const POINT2D *b1,
  int nb1, const POINT2D *b2, int nb2)
{
  IntersectResult r = linesegm_intersect(a->x1, a->y1, a->x2, a->y2,
    b->x1, b->y1, b->x2, b->y2);
  if (r.type != INTERSECT_POINT)
    return false;
  bool vertex = true;
  double x = 0.0, y = 0.0;
  if (point_on_segment_exact(a->x1, a->y1, b->x1, b->y1, b->x2, b->y2))
  {
    x = a->x1; y = a->y1;
  }
  else if (point_on_segment_exact(a->x2, a->y2, b->x1, b->y1, b->x2, b->y2))
  {
    x = a->x2; y = a->y2;
  }
  else if (point_on_segment_exact(b->x1, b->y1, a->x1, a->y1, a->x2, a->y2))
  {
    x = b->x1; y = b->y1;
  }
  else if (point_on_segment_exact(b->x2, b->y2, a->x1, a->y1, a->x2, a->y2))
  {
    x = b->x2; y = b->y2;
  }
  else
    vertex = false;
  for (int k = 0; k < 2; k++)
  {
    const POINT2D *bp = k ? b2 : b1;
    int nb = k ? nb2 : nb1;
    for (int p = 0; p < nb; p++)
    {
      if (vertex ? (bp[p].x == x && bp[p].y == y) :
          (point_on_segment_exact(bp[p].x, bp[p].y, a->x1, a->y1, a->x2,
             a->y2) &&
           point_on_segment_exact(bp[p].x, bp[p].y, b->x1, b->y1, b->x2,
             b->y2)))
        return false;
    }
  }
  return true;
}

/**
 * @brief Compute the DE-9IM matrix for two linear geometries
 * @details Every cell has exactly one source, so no cell can be attributed
 * twice or left to the visiting order of the edge pairs:
 * - II comes from the one-dimensional overlaps and from the intersection
 *   points that are interior to both geometries
 * - the boundary row of @p g1 comes from classifying each Mod-2 boundary
 *   point of @p g1 against @p g2, and symmetrically for @p g2
 * - IE and EI come from whether one geometry covers the other entirely,
 *   an uncovered part of a linear geometry being one-dimensional
 * The line/arc and arc/arc intersections stay exact, so a circular string
 * is related without being stroked into segments first
 */
static void
relate_linear_linear(const LWGEOM *g1, const LWGEOM *g2,
  const RelateOperands *ops, MeosDE9IM *m)
{
  MeosArray *a1 = relate_borrow_edges(ops, g1);
  MeosArray *a2 = relate_borrow_edges(ops, g2);
  int n1 = (int) a1->count;
  int n2 = (int) a2->count;
  Edge **e1 = palloc(sizeof(Edge *) * (size_t) (n1 + 1));
  Edge **e2 = palloc(sizeof(Edge *) * (size_t) (n2 + 1));
  for (int i = 0; i < n1; i++)
    e1[i] = (Edge *) meos_array_get_intl(a1, i);
  for (int i = 0; i < n2; i++)
    e2[i] = (Edge *) meos_array_get_intl(a2, i);

  /* The Mod-2 boundary of each operand, empty for closed components */
  int nb1, nb2;
  POINT2D *b1 = relate_linear_boundary_points(e1, n1, &nb1);
  POINT2D *b2 = relate_linear_boundary_points(e2, n2, &nb2);

  /* Interior/interior. A positive-length overlap of any edge pair is a
   * one-dimensional intersection of the two interiors. An intersection point
   * that is on neither Mod-2 boundary is a zero-dimensional one; a point on
   * a boundary is accounted for by the boundary row or column below */
  POINT2D points[2];
  for (int i = 0; i < n1; i++)
  {
    const Edge *a = e1[i];
    if (a->etype != EDGE_LINESEG && a->etype != EDGE_LINEARC)
      continue;
    for (int j = 0; j < n2; j++)
    {
      const Edge *b = e2[j];
      if (b->etype != EDGE_LINESEG && b->etype != EDGE_LINEARC)
        continue;
      double t0, t1;
      if (a->etype == EDGE_LINEARC && b->etype == EDGE_LINEARC)
      {
        RelateInterval iv[3];
        if (relate_arc_overlap_ranges(a, b, iv) > 0)
          de9im_add(&m->ii, 1);
      }
      else if (relate_linear_edges_overlap(a, b, &t0, &t1))
        de9im_add(&m->ii, 1);

      /* Two straight edges meet at a point decided on their input vertices */
      if (a->etype == EDGE_LINESEG && b->etype == EDGE_LINESEG)
      {
        if (relate_segments_meet_inside(a, b, b1, nb1, b2, nb2))
          de9im_add(&m->ii, 0);
        continue;
      }
      int np = relate_linear_edge_points(a, b, points);
      for (int k = 0; k < np; k++)
      {
        bool on_b1 = false, on_b2 = false;
        for (int p = 0; p < nb1 && ! on_b1; p++)
          on_b1 = relate_same_point(points[k].x, points[k].y, b1[p].x,
            b1[p].y);
        for (int p = 0; p < nb2 && ! on_b2; p++)
          on_b2 = relate_same_point(points[k].x, points[k].y, b2[p].x,
            b2[p].y);
        if (! on_b1 && ! on_b2)
          de9im_add(&m->ii, 0);
      }
    }
  }

  /* Boundary row of g1: each of its boundary points lies on the boundary of
   * g2, in the interior of g2, or outside g2 altogether */
  for (int p = 0; p < nb1; p++)
  {
    int loc = relate_point_in_linear(b1[p].x, b1[p].y, e2, n2, true);
    if (loc == 1)
      de9im_add(&m->bb, 0);
    else if (loc == 0)
      de9im_add(&m->bi, 0);
    else
      de9im_add(&m->be, 0);
  }

  /* Boundary column of g2, symmetrically */
  for (int p = 0; p < nb2; p++)
  {
    int loc = relate_point_in_linear(b2[p].x, b2[p].y, e1, n1, true);
    if (loc == 1)
      de9im_add(&m->bb, 0);
    else if (loc == 0)
      de9im_add(&m->ib, 0);
    else
      de9im_add(&m->eb, 0);
  }

  /* Interior/exterior. Whatever of g1 the edges of g2 do not cover is a
   * one-dimensional part of the interior of g1 lying outside g2 */
  if (! relate_linear_covered(e1, n1, e2, n2))
    de9im_add(&m->ie, 1);
  if (! relate_linear_covered(e2, n2, e1, n1))
    de9im_add(&m->ei, 1);

  /* The exterior of a bounded planar geometry is two-dimensional */
  de9im_add(&m->ee, 2);

  pfree(b1); pfree(b2); pfree(e1); pfree(e2);
  relate_return_edges(ops, a1); relate_return_edges(ops, a2);
  return;
}

/**
 * @brief Return true if every cell a caller reads of the matrix of a linear
 * geometry and an areal one already holds the largest dimension it can
 * @details The interior of a linear geometry meets anything in at most one
 * dimension and its boundary, a set of points, in at most zero. A cell only
 * ever rises, so one holding its largest dimension is final
 * @param[in] m Matrix being filled
 * @param[in] want Cells the caller reads, as the #DE9IM_II bits
 */
static bool
relate_linear_area_settled(const MeosDE9IM *m, int want)
{
  return (! (want & DE9IM_II) || m->ii == 1) &&
    (! (want & DE9IM_IB) || m->ib == 1) &&
    (! (want & DE9IM_IE) || m->ie == 1) &&
    (! (want & DE9IM_BI) || m->bi == 0) &&
    (! (want & DE9IM_BB) || m->bb == 0) &&
    (! (want & DE9IM_BE) || m->be == 0);
}

/**
 * @brief Return true if the walk over the linear edges can stop: every cell the
 * query reads is settled, or the query is answered
 * @details Both answers depend on the cells alone, so they are read again only
 * where a cell has changed since the last reading, which a walk over many
 * edges contributing nothing new never repeats
 * @param[in] m Matrix being filled
 * @param[in] want,q Cells and query the matrix answers
 * @param[in,out] seen Cells at the last reading, all 127 before the first
 * @param[in,out] stop Answer at the last reading
 */
static bool
relate_linear_area_stop(const MeosDE9IM *m, int want, const RelateQuery *q,
  MeosDE9IM *seen, bool *stop)
{
  if (memcmp(m, seen, sizeof(MeosDE9IM)) == 0)
    return *stop;
  *seen = *m;
  *stop = relate_linear_area_settled(m, want) || relate_query_decided(q, m);
  return *stop;
}

/**
 * @brief Compute the DE-9IM matrix for a linear geometry and an areal geometry
 * @details Only the cells named in @p want are answered, and the walk over the
 * edges of the linear geometry stops once each of them holds the largest
 * dimension it can reach, so a pattern asking whether the interiors meet stops
 * at the first stretch of the line inside the area
 * @param[in] line_geom,area_geom Geometries
 * @param[in] ops Operands of the relationship, NULL where there is none
 * @param[in] q Query the matrix answers, NULL for the whole matrix @nullable
 * @param[in,out] m Matrix
 */
static void
relate_linear_area(const LWGEOM *line_geom, const LWGEOM *area_geom,
  const RelateOperands *ops, const RelateQuery *q, MeosDE9IM *m)
{
  int want = relate_query_cells(q);
  MeosArray *la = relate_borrow_edges(ops, line_geom);
  MeosArray *aa = relate_borrow_edges(ops, area_geom);
  int nl = (int) la->count;
  int na = (int) aa->count;
  Edge **lines = palloc(sizeof(Edge *) * nl);
  Edge **area_edges = palloc(sizeof(Edge *) * na);
  for (int i = 0; i < nl; i++)
    lines[i] = (Edge *) meos_array_get_intl(la, i);
  for (int i = 0; i < na; i++)
    area_edges[i] = (Edge *) meos_array_get_intl(aa, i);
  /* Whether an endpoint of a linear edge is on the boundary of the linear
   * geometry is asked for every endpoint, and read from the endpoints sorted
   * once rather than from a scan of every edge each time */
  RelateLinearEnds ends;
  relate_linear_ends_init(&ends, lines, nl);
  /* Where a point stands against the area, and which boundary edges a linear
   * edge meets, each read the areal edge array once per linear edge, so the
   * array is indexed where that product dwarfs a pass over it, the gate the
   * areal kernel reads */
  RelateEdges ra;
  bool index = ((int64) nl * (int64) na >= RELATE_INDEX_MIN_PAIRS);
  /* The index is built on the first question it answers, so a linear geometry
   * every edge of which lies apart from the areal extent builds none */
  relate_edges_init(&ra, area_edges, na, false);
  bool area_arcs = false;
  for (int j = 0; j < na && ! area_arcs; j++)
    area_arcs = (area_edges[j]->etype == EDGE_POLYARC);
  MeosArray *candidates = NULL;
  /* The extent of a straight areal geometry, grown by the widest tolerance any
   * of its edges reads a point on it within */
  double axmin = DBL_MAX, axmax = -DBL_MAX, aymin = DBL_MAX, aymax = -DBL_MAX;
  for (int j = 0; j < na; j++)
  {
    axmin = fmin(axmin, area_edges[j]->xmin);
    axmax = fmax(axmax, area_edges[j]->xmax);
    aymin = fmin(aymin, area_edges[j]->ymin);
    aymax = fmax(aymax, area_edges[j]->ymax);
  }
  double apad = ra.tol + MEOS_GEOM_TOLERANCE;

  /* Maximum number of split parameters:
   * - every area edge can contribute at most two intersection parameters;
   * - 2 is added for the two endpoints of the linear edge.
   * This is deliberately allocated per linear edge. */
  const int maxparams = 2 * na + 2;
  double *params = palloc(sizeof(double) * maxparams);

  /* The area interior has dimension 2 whenever the area is non-empty, a
   * linear geometry covering no two-dimensional set */
  m->ei = 2;
  /* A finite ordinary area geometry has a 2-dimensional exterior. */
  m->ee = 2;

  /* The boundary of the area lies outside the linear geometry except where
   * that geometry runs along it, so every boundary edge is split where the
   * linear edges meet it and where they end, and the cell is answered by the
   * first portion found off them. Reading the whole boundary as outside
   * would report a geometry drawn over it as leaving a part uncovered */
  if (want & DE9IM_EB)
  {
    int maxbparams = 2 * nl + 2;
    double *bparams = palloc(sizeof(double) * maxbparams);
    for (int j = 0; j < na && m->eb == -1 && ! relate_query_decided(q, m); j++)
    {
      const Edge *boundary = area_edges[j];
      if (! relate_area_boundary_edge(boundary))
        continue;
      int nbparams = 0;
      bparams[nbparams++] = 0.0;
      bparams[nbparams++] = 1.0;
      for (int i = 0; i < nl; i++)
      {
        const Edge *line = lines[i];
        if (line->etype != EDGE_LINESEG && line->etype != EDGE_LINEARC)
          continue;
        double ix[2], iy[2];
        bool overlap;
        int n = relate_area_edge_intersection(boundary, line, ix, iy,
          &overlap);
        for (int k = 0; k < n; k++)
          relate_area_add_parameter(relate_area_edge_parameter(boundary,
            ix[k], iy[k]), bparams, &nbparams, maxbparams);
        /* A linear edge running along the boundary edge ends somewhere on
         * it, and the boundary is split there as well */
        for (int endpoint = 0; endpoint < 2; endpoint++)
        {
          double x = endpoint == 0 ? line->x1 : line->x2;
          double y = endpoint == 0 ? line->y1 : line->y2;
          bool on = boundary->etype == EDGE_POLYARC ?
            point_on_arc(x, y, boundary) :
            point_on_segment(x, y, boundary->x1, boundary->y1, boundary->x2,
              boundary->y2);
          if (on)
            relate_area_add_parameter(relate_area_edge_parameter(boundary, x,
              y), bparams, &nbparams, maxbparams);
        }
      }
      qsort(bparams, nbparams, sizeof(double), relate_area_parameter_cmp);
      for (int k = 0; k < nbparams - 1; k++)
      {
        if (bparams[k + 1] - bparams[k] <= MEOS_GEOM_TOLERANCE)
          continue;
        double x, y;
        relate_area_edge_point(boundary, (bparams[k] + bparams[k + 1]) * 0.5,
          &x, &y);
        if (relate_point_in_linear(x, y, lines, nl, false) == 2)
        {
          m->eb = 1;
          break;
        }
      }
    }
    pfree(bparams);
  }
  MeosDE9IM seen;
  memset(&seen, 127, sizeof(MeosDE9IM));
  bool stop = false;
  for (int i = 0; i < nl && ! relate_linear_area_stop(m, want, q, &seen,
       &stop); i++)
  {
    const Edge *line = lines[i];
    if (line->etype != EDGE_LINESEG && line->etype != EDGE_LINEARC)
      continue;
    /* A straight edge whose box lies apart from the extent of a straight
     * areal geometry meets none of its boundary edges, and every point of it
     * lies in the exterior of a closed ring set, so the edge contributes what
     * the steps below compute for it: its one open portion in the exterior,
     * and an endpoint on the linear boundary in the exterior as well */
    bool edge_apart = ! area_arcs && line->etype == EDGE_LINESEG &&
      (line->xmax < axmin - apad || line->xmin > axmax + apad ||
       line->ymax < aymin - apad || line->ymin > aymax + apad);
    /* Its endpoints are read by parity only where they are input vertices the
     * edge carries, the case the endpoint step below reads the same way */
    bool ends_read = ! (want & DE9IM_BE) || (relate_edge_nonempty(line) &&
      isfinite(line->x1) && isfinite(line->y1) && isfinite(line->x2) &&
      isfinite(line->y2));
    if (edge_apart && ends_read)
    {
      de9im_add(&m->ie, 1);
      if ((want & DE9IM_BE) && ! relate_query_decided(q, m) &&
          (relate_linear_ends_boundary(&ends, line->x1, line->y1, true) ||
           relate_linear_ends_boundary(&ends, line->x2, line->y2, true)))
        de9im_add(&m->be, 0);
      continue;
    }
    if (index && ! ra.index)
    {
      relate_edges_index_build(&ra);
      candidates = index_result_create();
    }
    int nparams = 0;
    /* The edge endpoints delimit the complete edge. */
    params[nparams++] = 0.0;
    params[nparams++] = 1.0;
    /* Intersect this linear edge with the area boundary edges. Two straight
     * edges meet only where their boxes meet, as the exact segment kernel
     * decides, so for a straight edge against straight boundary edges the ones
     * the index answers are the ones the scan finds meeting it; the parameters
     * are sorted below, so the order they are found in reads nothing */
    int nc = (candidates && line->etype == EDGE_LINESEG && ! area_arcs) ?
      relate_edges_candidates(&ra, line->xmin, line->xmax, line->ymin,
        line->ymax, candidates) : INT_MAX;
    if (nc != INT_MAX)
    {
      for (int c = 0; c < nc; c++)
      {
        const Edge *boundary = area_edges[INDEX_RESULT_ID_N(candidates, c)];
        if (boundary->etype != EDGE_POLYSEG)
          continue;
        relate_linear_area_edge_intersection(line, boundary, lines, nl, m,
          params, &nparams, maxparams);
      }
    }
    else
    {
      for (int j = 0; j < na; j++)
      {
        const Edge *boundary = area_edges[j];
        if (boundary->etype != EDGE_POLYSEG && boundary->etype != EDGE_POLYARC)
          continue;
        relate_linear_area_edge_intersection(line, boundary, lines, nl, m,
          params, &nparams, maxparams);
      }
    }

    /* Sort and remove duplicate parameters. */
    qsort(params, nparams, sizeof(double), relate_parameter_cmp);
    int nuniq = 0;
    for (int j = 0; j < nparams; j++)
    {
      if (nuniq == 0 || fabs(params[j] - params[nuniq - 1]) > MEOS_GEOM_TOLERANCE)
      {
        params[nuniq++] = params[j];
      }
    }

    /* Classify every open portion between consecutive boundary
     * intersections. */
    for (int j = 0; j < nuniq - 1; j++)
    {
      relate_linear_area_interval(line, params[j], params[j + 1], &ra, m);
    }

    /* Classify the two endpoints of the linear edge.
     * These belong to the boundary or interior of the complete
     * linear geometry according to the endpoint parity established
     * by relate_point_in_linear(). */
    for (int endpoint = 0; endpoint < 2; endpoint++)
    {
      double x = endpoint == 0 ? line->x1 : line->x2;
      double y = endpoint == 0 ? line->y1 : line->y2;
      /* Where the endpoint stands against the area names the two cells it can
       * feed, one for the interior and one for the boundary of the linear
       * geometry, and its linear location is read only where the query reads
       * either of them */
      int aloc = relate_point_in_area_index(x, y, &ra, true);
      static const int icell[3] = {DE9IM_II, DE9IM_IB, DE9IM_IE};
      static const int bcell[3] = {DE9IM_BI, DE9IM_BB, DE9IM_BE};
      if (! (want & (icell[aloc] | bcell[aloc])))
        continue;
      /* An input vertex lies on the straight edge it ends, exactly, so it is
       * in the linear geometry and only its boundary parity is read */
      int lloc = (line->etype == EDGE_LINESEG && relate_edge_nonempty(line) &&
          isfinite(x) && isfinite(y)) ?
        (relate_linear_ends_boundary(&ends, x, y, true) ? 1 : 0) :
        relate_point_in_linear(x, y, lines, nl, true);
      if (lloc == 2)
        continue;
      if (lloc == 0)
      {
        /* Linear interior ∩ area. An endpoint contributes dimension 0, which
         * must not demote the dimension 1 an open portion of the same edge
         * has already contributed to the very same cell */
        if (aloc == 0)
          de9im_add(&m->ii, 0);
        else if (aloc == 1)
          de9im_add(&m->ib, 0);
        else
          de9im_add(&m->ie, 0);
      }
      else
      {
        /* Linear boundary ∩ area */
        if (aloc == 0)
          de9im_add(&m->bi, 0);
        else if (aloc == 1)
          de9im_add(&m->bb, 0);
        else
          de9im_add(&m->be, 0);
      }
    }
  }

  /* If the linear geometry has a non-empty boundary, its boundary
   * is zero-dimensional. We therefore also need to account for the
   * boundary's intersection with the area exterior when the boundary
   * is not completely contained in the area closure. A caller not reading
   * that cell, or a cell already holding the one dimension it can reach, is
   * spared the search */
  bool has_boundary = false;
  for (int i = 0; (want & DE9IM_BE) && m->be != 0 &&
       ! relate_query_decided(q, m) && i < nl && !has_boundary; i++)
  {
    const Edge *line = lines[i];
    if (line->etype != EDGE_LINESEG && line->etype != EDGE_LINEARC)
      continue;
    if (! relate_edge_nonempty(line))
      continue;
    if (relate_linear_ends_boundary(&ends, line->x1, line->y1, true) ||
        relate_linear_ends_boundary(&ends, line->x2, line->y2, true))
      has_boundary = true;
  }

  if (has_boundary)
  {
    if (index && ! ra.index)
      relate_edges_index_build(&ra);
    /*
     * BE is present whenever a linear boundary point lies outside
     * the area.
     * BI and BB have already been set above for boundary points
     * that lie in the area interior or on its boundary.
     */
    for (int i = 0; i < nl; i++)
    {
      const Edge *line = lines[i];
      if (line->etype != EDGE_LINESEG && line->etype != EDGE_LINEARC)
        continue;
      const double x[2] = {line->x1, line->x2};
      const double y[2] = {line->y1, line->y2};
      for (int k = 0; k < 2; k++)
      {
        if (! relate_linear_ends_boundary(&ends, x[k], y[k], true))
          continue;
        int aloc = relate_point_in_area_index(x[k], y[k], &ra, true);
        if (aloc == 2)
          m->be = 0;
      }
    }
  }

  relate_edges_clear(&ra);
  if (candidates)
    meos_array_destroy(candidates);
  pfree(params); pfree(lines); pfree(area_edges);
  if (ends.points)
    pfree(ends.points);
  relate_return_edges(ops, la); relate_return_edges(ops, aa);
  return;
}

/*****************************************************************************
 * Area / Linear
 *****************************************************************************/

/**
 * @brief Set the DE-9IM matrix of an areal geometry and a linear one
 */
static void
relate_area_linear(const LWGEOM *area_geom, const LWGEOM *line_geom,
  const RelateOperands *ops, const RelateQuery *q, MeosDE9IM *m)
{
  MeosDE9IM tmp;
  de9im_init(&tmp);
  RelateQuery tq;
  if (q)
    relate_query_transpose(q, &tq);
  relate_linear_area(line_geom, area_geom, ops, q ? &tq : NULL, &tmp);

  m->ii = tmp.ii;
  m->ib = tmp.bi;
  m->ie = tmp.ei;

  m->bi = tmp.ib;
  m->bb = tmp.bb;
  m->be = tmp.eb;

  m->ei = tmp.ie;
  m->eb = tmp.be;
  m->ee = tmp.ee;
  return;
}

/*****************************************************************************
 * Area / Area
 *****************************************************************************/

/**
 * @brief Return true if an edge is a polygon boundary edge
 */
static inline bool
relate_area_boundary_edge(const Edge *e)
{
  return e->etype == EDGE_POLYSEG || e->etype == EDGE_POLYARC;
}

/**
 * @brief Return which side of the line through two points a third lies on, or
 * zero where the double evaluation cannot be trusted to carry the sign
 * @details The three points are INPUT VERTICES, so the question is the sign of
 * one determinant and needs no tolerance: a band here would not protect the
 * answer, it would decide it. What the sign does need is a guarantee that
 * rounding has not eaten it, and that guarantee is a FILTER rather than a
 * band. The bound is computed FROM THE OPERANDS and decides only whether the
 * double is trustworthy, never what the answer is -- a wrong tolerance returns
 * a WRONG answer where a wrong filter returns NO answer, and it is that
 * asymmetry that makes the filter safe where a band is not. It is Shewchuk's
 * bound, written in terms of @p DBL_EPSILON so that it is arithmetic rather
 * than a constant anyone is invited to tune
 * @return 1 or -1 for the two sides, 0 for collinear or unable to tell
 */
static int
relate_orientation(double ax, double ay, double bx, double by, double cx,
  double cy)
{
  double left = (bx - ax) * (cy - ay);
  double right = (by - ay) * (cx - ax);
  double det = left - right;
  double bound = (3.0 + 16.0 * DBL_EPSILON) * DBL_EPSILON *
    (fabs(left) + fabs(right));
  if (det > bound)
    return 1;
  if (det < - bound)
    return -1;
  return 0;
}

/**
 * @brief Return true if two straight edges properly cross
 * @details Proper means each edge carries the endpoints of the other STRICTLY
 * on either side of it, so the two meet at a single point interior to both.
 * Four determinants on the four input vertices decide it, with no point
 * constructed and no tolerance anywhere. An endpoint that merely touches, a
 * collinear pair, and a sign the filter declines to carry all read as NOT
 * crossing, which leaves each of those to the routes that already answer them
 */
static bool
relate_edges_cross(const Edge *e1, const Edge *e2)
{
  int d1 = relate_orientation(e1->x1, e1->y1, e1->x2, e1->y2, e2->x1, e2->y1);
  int d2 = relate_orientation(e1->x1, e1->y1, e1->x2, e1->y2, e2->x2, e2->y2);
  if (d1 == 0 || d2 == 0 || d1 == d2)
    return false;
  int d3 = relate_orientation(e2->x1, e2->y1, e2->x2, e2->y2, e1->x1, e1->y1);
  if (d3 == 0)
    return false;
  int d4 = relate_orientation(e2->x1, e2->y1, e2->x2, e2->y2, e1->x2, e1->y2);
  return d4 != 0 && d3 != d4;
}

/**
 * @brief Return true if two areal boundary edges properly cross
 * @details Two straight edges are decided by four determinants on their own
 * vertices (#relate_edges_cross). A straight edge against a CIRCULAR one is
 * decided by the quadratic that meets a line with a circle (#arcsegm_cross),
 * which is equally exact: an arc edge carries the centre, the radius and the
 * angular span, so the curve between its endpoints is determined and nothing
 * has to be inferred from those endpoints.
 * TWO CIRCULAR edges are decided by the half-chord of their supporting
 * circles (#arcarc_cross), which vanishes exactly at a tangency, so the same
 * question is answered for every pair of boundary edges the engine builds
 */
static bool
relate_area_edges_cross(const Edge *e1, const Edge *e2)
{
  if (e1->etype == EDGE_POLYSEG)
  {
    if (e2->etype == EDGE_POLYSEG)
      return relate_edges_cross(e1, e2);
    if (e2->etype == EDGE_POLYARC)
      return arcsegm_cross(e1->x1, e1->y1, e1->dx, e1->dy, e2);
    return false;
  }
  if (e1->etype == EDGE_POLYARC)
  {
    if (e2->etype == EDGE_POLYSEG)
      return arcsegm_cross(e2->x1, e2->y1, e2->dx, e2->dy, e1);
    if (e2->etype == EDGE_POLYARC)
      return arcarc_cross(e1, e2);
  }
  return false;
}

/**
 * @brief Compute a point on an areal boundary edge
 * @details 
 *   t = 0 -> first endpoint
 *   t = 1 -> second endpoint
 * Circular arcs are evaluated exactly.
 */
static void
relate_area_edge_point(const Edge *e, double t, double *x, double *y)
{
  if (e->etype == EDGE_POLYSEG)
  {
    *x = e->x1 + t * (e->x2 - e->x1);
    *y = e->y1 + t * (e->y2 - e->y1);
    return;
  }

  /* Circular polygon boundary */
  double sweep = e->ccw ?
    angle_normalize(e->theta1 - e->theta0) :
    angle_normalize(e->theta0 - e->theta1);
  double theta = e->ccw ?
    e->theta0 + t * sweep :
    e->theta0 - t * sweep;
  *x = e->cx + e->radius * cos(theta);
  *y = e->cy + e->radius * sin(theta);
  return;
}

/**
 * @brief Return the parameter of a point on a polygon boundary edge
 * @details The parameter is read along the coordinate the edge advances most
 * along. The ends of a segment are input vertices, so it has no direction
 * exactly where they are equal, and any other segment, however short, gives
 * each point along it a parameter of its own
 */
static double
relate_area_edge_parameter(const Edge *e, double x, double y)
{
  if (e->etype == EDGE_POLYSEG)
  {
    double dx = e->x2 - e->x1;
    double dy = e->y2 - e->y1;
    if (fabs(dx) >= fabs(dy))
      return (dx == 0.0) ? 0.0 : (x - e->x1) / dx;
    return (y - e->y1) / dy;
  }
  return relate_arc_parameter(e, x, y);
}

/**
 * @brief Add an intersection parameter to an array
 */
static void
relate_area_add_parameter(double t, double *params, int *nparams,
  int maxparams)
{
  if (t < -MEOS_GEOM_TOLERANCE || t > 1.0 + MEOS_GEOM_TOLERANCE)
    return;
  if (t < 0.0)
    t = 0.0;
  if (t > 1.0)
    t = 1.0;
  for (int i = 0; i < *nparams; i++)
  {
    if (fabs(params[i] - t) <= MEOS_GEOM_TOLERANCE)
      return;
  }
  if (*nparams < maxparams)
    params[(*nparams)++] = t;
  return;
}

/**
 * @brief Sort area-edge parameters
 */
static int
relate_area_parameter_cmp(const void *a, const void *b)
{
  const double da = *(const double *) a;
  const double db = *(const double *) b;
  if (da < db)
    return -1;
  if (da > db)
    return 1;
  return 0;
}

/**
 * @brief Determine intersections between two polygon boundary edges
 * @details Return the number of point intersections, written in x/y, and set
 * @p overlap when the two edges share a one-dimensional portion. The two
 * answers are separate because a shared portion carries no discrete
 * intersection point: reporting it as a count would name coordinates the
 * function never wrote. This mirrors #relate_arc_arc_points, which the arc
 * case delegates to.
 */
static int
relate_area_edge_intersection(const Edge *a, const Edge *b, double ix[2],
  double iy[2], bool *overlap)
{
  *overlap = false;
  /* Line / Line */
  if (a->etype == EDGE_POLYSEG && b->etype == EDGE_POLYSEG)
  {
    IntersectResult r = relate_segments_intersect(a, b);
    if (r.type == INTERSECT_NONE)
      return 0;
    if (r.type == INTERSECT_OVERLAP)
    {
      *overlap = true;
      return 0;
    }
    if (r.type == INTERSECT_POINT)
    {
      ix[0] = a->x1 + r.t0 * a->dx;
      iy[0] = a->y1 + r.t0 * a->dy;
      return 1;
    }
    return 0;
  }

  /* Line / Arc */
  if (a->etype == EDGE_POLYSEG && b->etype == EDGE_POLYARC)
  {
    double roots[2];
    int n = arcsegm_intersect(a->x1, a->y1, a->dx, a->dy, b, roots);
    if (n <= 0)
      return 0;
    int count = 0;
    for (int i = 0; i < n && count < 2; i++)
    {
      ix[count] = a->x1 + roots[i] * a->dx;
      iy[count] = a->y1 + roots[i] * a->dy;
      count++;
    }
    return count;
  }

  /* Arc / Line */
  if (a->etype == EDGE_POLYARC && b->etype == EDGE_POLYSEG)
  {
    double roots[2];
    int n = arcsegm_intersect(b->x1, b->y1, b->dx, b->dy, a, roots);
    if (n <= 0)
      return 0;
    int count = 0;
    for (int i = 0; i < n && count < 2; i++)
    {
      ix[count] = b->x1 + roots[i] * b->dx;
      iy[count] = b->y1 + roots[i] * b->dy;
      count++;
    }
    return count;
  }

  /* Arc / Arc */
  if (a->etype == EDGE_POLYARC && b->etype == EDGE_POLYARC)
    return relate_arc_arc_points(a, b, ix, iy, overlap);
  return 0;
}

/**
 * @brief Test whether an open portion of one area boundary lies in the interior
 * of another area
 * @details This is the key operation for detecting:
 *   BI = boundary(A) / interior(B)
 *   IB = interior(A) / boundary(B)
 * as one-dimensional intersections.
 */
static bool
relate_area_edge_inside_area(const Edge *edge, const RelateEdges *area)
{
  double x, y;
  /* Midpoint of a straight boundary edge */
  if (edge->etype == EDGE_POLYSEG)
  {
    x = (edge->x1 + edge->x2) * 0.5;
    y = (edge->y1 + edge->y2) * 0.5;
  }
  else
  {
    /* Midpoint of the circular arc */
    relate_area_edge_point(edge, 0.5, &x, &y);
  }
  return relate_point_in_area_index(x, y, area, false) == 0;
}

/* Split parameters an edge holds on the stack, which covers an edge meeting up
 * to 31 candidate edges of the other boundary */
#define RELATE_INTERVAL_PARAMS 64

/**
 * @brief Classify the open portions of one polygon boundary edge
 * @details The edge is split at every intersection with the other polygon
 * boundary. Since there is no boundary intersection inside an open
 * interval, one representative point is sufficient.
 */
static void
relate_area_edge_intervals(const Edge *edge, const RelateEdges *other,
  MeosDE9IM *m, bool first)
{
  /* An edge apart from the extent of the other boundary meets none of its
   * edges, so it is one open interval, and its midpoint, lying in the box of
   * the edge, stands apart from the other area as well */
  if (relate_edges_apart(other, edge->xmin, edge->xmax, edge->ymin,
        edge->ymax, fmax(other->tol, edge->tol)))
  {
    if (first)
      m->be = 1;
    else
      m->eb = 1;
    return;
  }
  /* The edges this one can meet are those whose box meets its own, and an index
   * answers them in the place of a pass over the whole array. A boundary of a
   * few thousand edges leaves every pair but a handful standing apart, so the
   * pass is what the walk costs and the index is what removes it */
  MeosArray *candidates = NULL;
  int ncand = other->nedges;
  if (other->index)
  {
    double pad = fmax(other->tol, edge->tol);
    /* The ids are collected into the array the edges carry for their index,
     * which nothing else reads while this loop runs, rather than into one
     * made and released for every edge */
    candidates = other->results;
    ncand = edge_index_query(other->index, other->edges, edge->xmin - pad,
      edge->xmax + pad, edge->ymin - pad, edge->ymax + pad, candidates);
  }

  /* Maximum number of intersections between one edge and one
   * circular/linear boundary edge is two, and only a candidate meets this
   * one, so the parameters are bounded by the candidates rather than by the
   * whole other boundary: a handful, held on the stack rather than allocated
   * for every edge */
  int maxparams = 2 * ncand + 2;
  double pstack[RELATE_INTERVAL_PARAMS], lostack[RELATE_INTERVAL_PARAMS],
    histack[RELATE_INTERVAL_PARAMS];
  bool heap = maxparams > RELATE_INTERVAL_PARAMS;
  double *params = heap ? palloc(sizeof(double) * maxparams) : pstack;
  int nparams = 0;
  params[nparams++] = 0.0;
  params[nparams++] = 1.0;
  /* The stretches along which this edge RUNS ON the other boundary rather than
   * meeting it at a point. Every other part of the edge misses that boundary
   * entirely, which is what lets the classification below drop a tolerance */
  double *shlo = heap ? palloc(sizeof(double) * maxparams) : lostack;
  double *shhi = heap ? palloc(sizeof(double) * maxparams) : histack;
  int nshared = 0;
  for (int c = 0; c < ncand; c++)
  {
    int j = candidates ? (int) INDEX_RESULT_ID_N(candidates, c) : c;
    const Edge *oedge = other->edges[j];
    if (! relate_area_boundary_edge(oedge))
      continue;
    double ix[2], iy[2];
    bool overlap;
    int n = relate_area_edge_intersection(edge, oedge, ix, iy, &overlap);
    if (overlap)
    {
      /* The two boundary edges share a one-dimensional portion. Add the
       * endpoints of that portion as split parameters. This is mainly needed
       * for the classification of the remaining portions. */
      if (relate_point_on_edge(oedge->x1, oedge->y1, edge, false))
      {
        relate_area_add_parameter(relate_area_edge_parameter(edge, oedge->x1,
          oedge->y1), params, &nparams, maxparams);
      }

      if (relate_point_on_edge(oedge->x2, oedge->y2, edge, false))
      {
        relate_area_add_parameter(relate_area_edge_parameter(edge, oedge->x2,
          oedge->y2), params, &nparams, maxparams);
      }
      /* Keep the stretch itself, not only its ends: a point of the edge inside
       * it lies ON the other boundary, and no parity test can say so */
      if (nshared < maxparams)
      {
        double ta = relate_area_edge_parameter(edge, oedge->x1, oedge->y1);
        double tb = relate_area_edge_parameter(edge, oedge->x2, oedge->y2);
        double lo = fmax(0.0, fmin(ta, tb));
        double hi = fmin(1.0, fmax(ta, tb));
        if (hi > lo)
        {
          shlo[nshared] = lo;
          shhi[nshared] = hi;
          nshared++;
        }
      }
      continue;
    }
    for (int k = 0; k < n; k++)
    {
      double t = relate_area_edge_parameter(edge, ix[k], iy[k]);
      relate_area_add_parameter(t, params, &nparams, maxparams);
    }
  }

  qsort(params, nparams, sizeof(double), relate_area_parameter_cmp);
  /* Remove duplicates */
  int nuniq = 0;
  for (int i = 0; i < nparams; i++)
  {
    if (nuniq == 0 || fabs(params[i] - params[nuniq - 1]) > MEOS_GEOM_TOLERANCE)
    {
      params[nuniq++] = params[i];
    }
  }

  /* Classify each open interval */
  for (int i = 0; i < nuniq - 1; i++)
  {
    if (params[i + 1] - params[i] <= MEOS_GEOM_TOLERANCE)
      continue;
    double t = (params[i] + params[i + 1]) * 0.5;
    double x, y;
    relate_area_edge_point(edge, t, &x, &y);
    /* Every point at which this edge meets the other boundary is one of the
     * parameters it was split at, so the OPEN interval between two consecutive
     * ones touches that boundary nowhere -- unless the edge runs along it,
     * which the shared stretches record. Off those stretches the point is
     * therefore known to miss the boundary before it is classified, and asking
     * whether it lies ON the boundary can only answer wrongly: the midpoint is
     * a CONSTRUCTED point, so where the interval is narrower than the band it
     * reads as carried by an edge it provably misses, and a boundary reading
     * takes the interval out of both BI and BE. Reading the parity alone
     * settles it with no tolerance in the path */
    bool on_shared = false;
    for (int k = 0; k < nshared && ! on_shared; k++)
      if (t >= shlo[k] && t <= shhi[k])
        on_shared = true;
    int loc = on_shared ? 1 : relate_point_in_area_parity(x, y, other);
    /* A boundary point of A lying in the interior of B carries a whole
     * neighbourhood of itself inside B, and every neighbourhood of a boundary
     * point of A meets the interior of A, so the two interiors meet in a
     * two-dimensional set. The interiors are therefore what settles this
     * branch, and #relate_area_interiors_intersect has already answered them
     * from the operands' own vertices before this walk runs.
     *
     * Reading that answer is what keeps the branch sound where the midpoint
     * cannot: the midpoint is CONSTRUCTED from two split parameters that are
     * themselves solved from the coordinates, so where the two boundaries run
     * within their own rounding of one another it lands on either side of the
     * other boundary by the residue of that arithmetic, and the parity below
     * reports whichever side it landed on. Where the interiors provably miss,
     * so does the boundary, whatever the midpoint reads.
     *
     * The crossing route decides every pair of boundary edges the engine
     * builds -- two straight ones, one of each, and two arcs -- so the
     * interiors answer is exact for every areal pair and this branch reads it
     * rather than the midpoint in all of them */
    if (loc == 0 && m->ii == 2)
    {
      /* Boundary(A) ∩ Interior(B) or Interior(A) ∩ Boundary(B) */
      if (first)
        m->bi = 1;
      else
        m->ib = 1;
      /* A non-zero boundary portion of A inside B carries a neighbourhood of
       * itself inside B, and that neighbourhood meets the interior of A */
      m->ii = 2;
    }
    else if (loc == 2)
    {
      /* Boundary(A) ∩ Exterior(B) */
      if (first)
        m->be = 1;
      else
        m->eb = 1;
    }
  }
  if (heap)
  {
    pfree(params);
    pfree(shlo);
    pfree(shhi);
  }
  return;
}

/**
 * @brief Process all point intersections between two area boundaries
 */
static void
relate_area_boundary_points(const RelateEdges *are, const RelateEdges *bre,
  MeosDE9IM *m)
{
  Edge **aedges = are->edges, **bedges = bre->edges;
  int na = are->nedges, nb = bre->nedges;
  MeosArray *candidates = bre->index ? index_result_create() : NULL;
  for (int i = 0; i < na; i++)
  {
    const Edge *a = aedges[i];
    if (!relate_area_boundary_edge(a))
      continue;
    /* Two edges whose boxes stand apart meet nowhere, so the edges worth
     * solving against this one are those the index answers for its box. The
     * bound the query is grown by is the widest tolerance the array asks for,
     * which is what makes the index admit every edge the pass would test */
    int ncand = nb;
    if (bre->index)
    {
      double pad = fmax(bre->tol, a->tol);
      ncand = edge_index_query(bre->index, bre->edges, a->xmin - pad,
        a->xmax + pad, a->ymin - pad, a->ymax + pad, candidates);
    }
    for (int c = 0; c < ncand; c++)
    {
      int j = candidates ? (int) INDEX_RESULT_ID_N(candidates, c) : c;
      const Edge *b = bedges[j];
      if (!relate_area_boundary_edge(b))
        continue;
      double ix[2], iy[2];
      bool overlap;
      int n = relate_area_edge_intersection(a, b, ix, iy, &overlap);

      /* A one-dimensional overlap, whether the two edges are segments or
       * coincident arcs */
      if (overlap)
      {
        de9im_add(&m->bb, 1);
        continue;
      }

      /* Point intersections. The cell keeps the largest dimension found over
       * all the edge pairs, so a shared boundary segment is not demoted by a
       * later pair that meets in a single point */
      if (n > 0)
        de9im_add(&m->bb, 0);
    }
  }
  if (candidates)
    meos_array_destroy(candidates);
  return;
}

/**
 * @brief Determine whether an area has an interior point in the interior of
 * another area
 * @details We use boundary vertices first. If a vertex of A lies in the
 * interior of B, then a neighbourhood of that vertex inside A is
 * also inside B, proving II = 2. The rings of an area are closed, so the end
 * of each boundary edge is the start of the one following it, and locating
 * the starts locates every vertex once rather than twice.
 */
static bool
relate_area_has_vertex_interior(const RelateEdges *self,
  const RelateEdges *other)
{
  for (int i = 0; i < self->nedges; i++)
  {
    const Edge *e = self->edges[i];
    if (!relate_area_boundary_edge(e))
      continue;
    if (relate_point_in_area_index(e->x1, e->y1, other, false) == 0)
      return true;
  }
  return false;
}

/**
 * @brief Find an interior point of an areal geometry
 * @details The function first tries boundary-edge midpoints with a small
 * perturbation to either side. This avoids requiring a centroid
 * implementation and works directly with circular boundaries.
 */
static bool
relate_area_edge_interior_point(const Edge *e, const RelateEdges *self,
  double *x, double *y)
{
  {
    if (! relate_area_boundary_edge(e))
      return false;
    /* Take the midpoint of the edge */
    double px, py;
    relate_area_edge_point(e, 0.5, &px, &py);
    /* Estimate a local tangent, and read the size of the edge the witness
     * steps off: its length, or the length of the arc */
    double tx, ty, size;
    if (e->etype == EDGE_POLYSEG)
    {
      tx = e->x2 - e->x1;
      ty = e->y2 - e->y1;
      size = hypot(tx, ty);
    }
    else
    {
      /* Tangent to a circular arc at its midpoint */
      double sweep = e->ccw ?
        angle_normalize(e->theta1 - e->theta0) :
        angle_normalize(e->theta0 - e->theta1);
      size = e->radius * sweep;
      double theta = e->ccw ?
        e->theta0 + 0.5 * sweep :
        e->theta0 - 0.5 * sweep;
      if (e->ccw)
      {
        tx = -sin(theta);
        ty =  cos(theta);
      }
      else
      {
        tx =  sin(theta);
        ty = -cos(theta);
      }
    }
    double len = hypot(tx, ty);
    if (len == 0.0)
      return false;
    tx /= len;
    ty /= len;

    /* Two possible normal directions. The actual polygon orientation
     * is irrelevant because we test both sides. */
    double nx = -ty;
    double ny =  tx;

    /* The witness steps off the edge far enough to leave the band within
     * which an edge of the geometry reads a point as lying on it, since
     * #relate_point_in_area answers boundary before it answers interior and a
     * witness inside the band would be taken for a boundary point and the
     * interior would be reported empty. Ten times the widest band of the
     * geometry leaves the margin, and the step is sized from the edge itself,
     * a billionth of its length, so it scales with the geometry and cannot
     * cross to the far side of it. */
    double eps = fmax(size * 1e-9, 10.0 * self->tol);
    double qx = px + eps * nx;
    double qy = py + eps * ny;
    if (relate_point_in_area_index(qx, qy, self, false) == 0)
    {
      *x = qx;
      *y = qy;
      return true;
    }
    qx = px - eps * nx;
    qy = py - eps * ny;
    if (relate_point_in_area_index(qx, qy, self, false) == 0)
    {
      *x = qx;
      *y = qy;
      return true;
    }
  }
  return false;
}

/**
 * @brief Return true if an areal geometry holds an interior point standing to
 * another areal geometry in a given location
 * @details A single witness does not answer a geometry of several components:
 * the one the search returns first belongs to the first component, and a
 * component further on may stand differently. Two multipolygons sharing a
 * component exactly are the case that shows it, the shared component making
 * the interiors meet while the witness of the first component lies outside.
 * Every boundary edge therefore contributes its own witness.
 * @param[in] edges,nedges Edges of the geometry the witness comes from
 * @param[in] other,nother Edges of the geometry the witness is located in
 * @param[in] location Location to look for, as #relate_point_in_area reports
 * it: 0 for the interior and 2 for the exterior
 */
static bool
relate_area_interior_point_located(const RelateEdges *self,
  const RelateEdges *other, int location)
{
  for (int i = 0; i < self->nedges; i++)
  {
    double x, y;
    if (! relate_area_edge_interior_point(self->edges[i], self, &x, &y))
      continue;
    if (relate_point_in_area_index(x, y, other, false) == location)
      return true;
  }
  return false;
}

/**
 * @brief Return true if an areal geometry holds an interior point standing to
 * another areal geometry in a given location, for a pair whose boundary of
 * the other does not meet the interior of the first
 * @details A connected piece of the interior of the first geometry that the
 * boundary of the other does not meet lies wholly inside the other or wholly
 * outside it, since passing from one to the other crosses that boundary. A
 * piece is bounded by rings of the first geometry, so one witness stepped off
 * one edge of each ring answers for every edge of that ring, where
 * #relate_area_interior_point_located asks every edge. A witness read as lying
 * on the boundary of the other answers nothing, and the next edge of the ring
 * is asked. The edges of a ring follow one another in the array, and a ring
 * ends at the edge that returns to the point it starts from
 * @param[in] self Edges of the geometry the witnesses come from
 * @param[in] other Edges of the geometry the witnesses are located in, whose
 * boundary does not meet the interior of @p self
 * @param[in] location Location to look for, as #relate_point_in_area reports
 * it: 0 for the interior and 2 for the exterior
 */
static bool
relate_area_ring_point_located(const RelateEdges *self,
  const RelateEdges *other, int location)
{
  bool open = false, located = false;
  double sx = 0.0, sy = 0.0;
  for (int i = 0; i < self->nedges; i++)
  {
    const Edge *e = self->edges[i];
    if (! open)
    {
      sx = e->x1;
      sy = e->y1;
      open = true;
      located = false;
      /* A witness looked for INSIDE the other geometry is never found on a
       * ring standing apart from its extent. A witness lies within the box of
       * its edge grown by its step, which is a billionth of the length of the
       * edge or ten times the band of the geometry, and a point apart from the
       * extent of the other geometry by more than the band of the other is
       * located in its exterior (#relate_point_in_area_index). An edge, or the
       * arc it draws, is no longer than four times the diagonal of the box of
       * its ring, which bounds the step of every witness the ring yields */
      if (location == 0)
      {
        double xmin = e->xmin, xmax = e->xmax, ymin = e->ymin,
          ymax = e->ymax;
        int end = i;
        while (end < self->nedges - 1 &&
            ! (self->edges[end]->x2 == sx && self->edges[end]->y2 == sy))
        {
          const Edge *f = self->edges[++end];
          xmin = fmin(xmin, f->xmin); xmax = fmax(xmax, f->xmax);
          ymin = fmin(ymin, f->ymin); ymax = fmax(ymax, f->ymax);
        }
        double step = fmax(4e-9 * ((xmax - xmin) + (ymax - ymin)),
          10.0 * self->tol);
        if (relate_edges_apart(other, xmin, xmax, ymin, ymax,
              step + other->tol))
        {
          i = end;
          open = false;
          continue;
        }
      }
    }
    double x, y;
    if (! located && relate_area_edge_interior_point(e, self, &x, &y))
    {
      int loc = relate_point_in_area_index(x, y, other, false);
      if (loc == location)
        return true;
      located = (loc != 1);
    }
    if (e->x2 == sx && e->y2 == sy)
      open = false;
  }
  return false;
}

/**
 * @brief Return true if a boundary edge of one areal geometry properly crosses
 * a boundary edge of the other
 * @details Where two boundaries cross transversally, the four sectors around
 * the crossing point are the four combinations of inside and outside, so one
 * of them lies interior to BOTH and the interiors share a two-dimensional set.
 * The whole proof is the sign of four determinants on four INPUT VERTICES.
 * Nothing is constructed, so nothing has to be classified within a tolerance,
 * and that is what lets this see an overlap too thin for a sampled witness to
 * land in: a sliver a couple of units in the last place wide is narrower than
 * the band every point-location route reads, yet its crossing vertices are
 * exactly the ones the determinants are taken on.
 * A CIRCULAR edge is decided too, and by the same kind of argument rather
 * than by the determinants: an arc carries its centre, radius and angular
 * span, so the curve between its endpoints is determined and the crossing is
 * solved exactly, against a straight edge and against another arc alike.
 * #relate_area_edges_cross dispatches the three cases
 * @param[in] a,b Edges of the two geometries, the second read out of its index
 * where it carries one
 */
static bool
relate_area_boundaries_cross(const RelateEdges *a, const RelateEdges *b)
{
  for (int i = 0; i < a->nedges; i++)
  {
    const Edge *ea = a->edges[i];
    if (! b->index)
    {
      for (int j = 0; j < b->nedges; j++)
      {
        const Edge *eb = b->edges[j];
        if (relate_area_edges_cross(ea, eb))
          return true;
      }
      continue;
    }
    /* Two edges that cross meet at a point lying on both, so that point is
     * in both boxes and the boxes overlap -- an arc's box being tight about
     * the curve and not merely about its endpoints. The query therefore needs
     * NO padding: an unpadded box already admits every crossing there is, and
     * unlike the point-location queries the tests behind it only ever REFUSE
     * a crossing near a boundary of either edge, so none of them reaches for a
     * point a pad would have to admit */
    MeosArray *candidates = index_result_create();
    int nc = edge_index_query(b->index, b->edges, ea->xmin, ea->xmax,
      ea->ymin, ea->ymax, candidates);
    bool result = false;
    for (int c = 0; c < nc && ! result; c++)
    {
      const Edge *eb = b->edges[INDEX_RESULT_ID_N(candidates, c)];
      result = relate_area_edges_cross(ea, eb);
    }
    meos_array_destroy(candidates);
    if (result)
      return true;
  }
  return false;
}

/**
 * @brief Determine whether the interiors of two areal geometries intersect in
 * dimension 2
 */
static bool
relate_area_interiors_intersect(const RelateEdges *a, const RelateEdges *b)
{
  /* A vertex of either geometry inside the other geometry proves
   * a two-dimensional interior/interior intersection */
  if (relate_area_has_vertex_interior(a, b))
    return true;
  if (relate_area_has_vertex_interior(b, a))
    return true;

  /* If a boundary portion of either geometry lies in the interior
   * of the other, the interiors necessarily overlap in area */
  for (int i = 0; i < a->nedges; i++)
  {
    const Edge *e = a->edges[i];
    if (!relate_area_boundary_edge(e))
      continue;
    if (relate_area_edge_inside_area(e, b))
      return true;
  }
  for (int i = 0; i < b->nedges; i++)
  {
    const Edge *e = b->edges[i];
    if (!relate_area_boundary_edge(e))
      continue;
    if (relate_area_edge_inside_area(e, a))
      return true;
  }

  /* Handle coincident boundaries / complete containment
   * where every tested vertex may lie on the other boundary.
   * An interior witness of either geometry inside the other answers it */
  if (relate_area_ring_point_located(a, b, 0))
    return true;
  if (relate_area_ring_point_located(b, a, 0))
    return true;

  /* Every route above answers by LOCATING A POINT, and each one a tolerance
   * decides: a vertex of one geometry, a point sampled along a boundary edge,
   * an interior witness stepped off the boundary. Where the two interiors
   * share only a sliver thinner than that tolerance, no such point can be
   * placed inside it -- the sliver's own corners are intersection points
   * belonging to neither vertex set, and any witness constructed within it
   * falls inside the band of both boundaries. All of them therefore decline
   * while the interiors do share area.
   * A crossing of the two boundaries proves that area without locating
   * anything, so it is asked last: it costs an index probe per boundary edge
   * and answers only what the routes above have already given up on */
  if (relate_area_boundaries_cross(a, b))
    return true;
  return false;
}

/**
 * @brief Set up a matrix operand's edges through the index its context keeps
 * @details A matrix reads each array once per edge of the other, so an index
 * pays wherever the call runs long, and loses where the query is answered
 * after a few edges: building it is a pass over the whole array, paid in full
 * in front of a walk that stops almost at once. An index a context keeps is
 * built once for the geometry, by the first call that needs it, and every
 * call after reads it for nothing, as GEOS reads the index of a prepared
 * geometry (RelateNG). An operand read for one call has no context and
 * indexes by the size of the pair, as before.
 * @return True if the index is the context's, which the caller must not free
 */
static bool
relate_edges_init_kept(RelateEdges *re, Edge **edges, int nedges,
  const RelateOperands *ops, const MeosArray *arr, bool index)
{
  EdgeIndex **kept = NULL;
  if (ops)
    for (int k = 0; k < 2 && ! kept; k++)
      if (*ops->op[k].kept == arr)
        kept = ops->op[k].index;
  if (! kept)
  {
    relate_edges_init(re, edges, nedges, index);
    return false;
  }
  relate_edges_init(re, edges, nedges, false);
  if (! *kept)
  {
    /* The index lives as long as the context, so it is read into the memory
     * that keeps the context, never into the caller's */
#if ! MEOS
    MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
    *kept = relate_edges_index(edges, nedges);
#if ! MEOS
    MemoryContextSwitchTo(oldctx);
#endif
  }
  re->index = *kept;
  re->results = index_result_create();
  return true;
}

/**
 * @brief Compute the DE-9IM matrix for two areal geometries
 */
static void
relate_area_area(const LWGEOM *g1, const LWGEOM *g2,
  const RelateOperands *ops, const RelateQuery *q, MeosDE9IM *m)
{
  MeosArray *a1 = relate_borrow_edges(ops, g1);
  MeosArray *a2 = relate_borrow_edges(ops, g2);
  int n1 = (int) a1->count;
  int n2 = (int) a2->count;
  Edge **e1 = palloc(sizeof(Edge *) * n1);
  Edge **e2 = palloc(sizeof(Edge *) * n2);
  for (int i = 0; i < n1; i++)
    e1[i] = (Edge *) meos_array_get_intl(a1, i);
  for (int i = 0; i < n2; i++)
    e2[i] = (Edge *) meos_array_get_intl(a2, i);

  /* Every question this matrix asks about one point reads one of the two edge
   * arrays, and asks it once per edge of the other, so an unindexed walk costs
   * their PRODUCT while indexing each array costs one pass over it. The two
   * indexes are therefore built ONCE here and read by every step below */
  RelateEdges re1, re2;
  bool index = ((int64) n1 * (int64) n2 >= RELATE_INDEX_MIN_PAIRS);
  bool kept1 = relate_edges_init_kept(&re1, e1, n1, ops, a1, index);
  bool kept2 = relate_edges_init_kept(&re2, e2, n2, ops, a2, index);

  /* Two-dimensional interior/interior intersection. */
  if (relate_area_interiors_intersect(&re1, &re2))
  {
    m->ii = 2;
  }

  /* Every pattern of TOUCHES reads the interiors as not meeting, so once they
   * meet the query is answered and no other cell is read */
  if (relate_query_decided(q, m))
    goto done;

  /* Boundary(A) / Interior(B) and Interior(A) / Boundary(B) are determined by
   * splitting every boundary edge at the intersections with the other
   * boundary. Where the step above has settled the interiors exactly, each
   * portion is read against that answer */
  /* A cell only ever rises, so the query is asked again after every edge: a
   * pattern a cell has already contradicted is answered there, as GEOS stops
   * at the first cell that contradicts it (RelateNG, IMPatternMatcher), and
   * the edges left are never split */
  /* The boundary of A fills the cells BI and BE, that of B the cells IB and
   * EB, so a query reading only the latter -- `contains` is `T*****FF*` -- is
   * answered by walking B first, and never by walking A at all */
  bool b_first = q && (q->cells & (DE9IM_IB | DE9IM_EB)) &&
    ! (q->cells & (DE9IM_BI | DE9IM_BE));
  for (int pass = 0; pass < 2; pass++)
  {
    bool first = (pass == 0) != b_first;
    Edge **edges = first ? e1 : e2;
    int nedges = first ? n1 : n2;
    const RelateEdges *other = first ? &re2 : &re1;
    for (int i = 0; i < nedges; i++)
    {
      if (!relate_area_boundary_edge(edges[i]))
        continue;
      /* The query reads the cells alone, so it is asked again only where an
       * edge has raised one: a matrix left as it stood is still undecided */
      MeosDE9IM before = *m;
      relate_area_edge_intervals(edges[i], other, m, first);
      if (memcmp(&before, m, sizeof(MeosDE9IM)) != 0 &&
          relate_query_decided(q, m))
        goto done;
    }
  }

  /* Boundary / Boundary.
   * Point intersections give dimension 0.
   * Coincident/overlapping boundary portions give dimension 1. */
  relate_area_boundary_points(&re1, &re2, m);
  if (relate_query_decided(q, m))
    goto done;

  /* Interior / Exterior, of dimension 2 because a non-empty open region is
   * two-dimensional. Two independent sources answer it, and either one alone
   * misses cases the other catches:
   * - a piece of the boundary of A running outside B, which
   *   #relate_area_edge_intervals has already recorded in BE, has a
   *   neighbourhood inside A that is outside B as well. This covers every
   *   partial overlap, where an interior witness can land inside B
   * - an interior point of A outside B covers the case where the whole
   *   boundary of A lies within B while a hole of B falls inside A */
  if (m->be == 1)
    de9im_add(&m->ie, 2);
  if (m->eb == 1)
    de9im_add(&m->ei, 2);

  /* A third source, the one that answers a hole of A covered by B. A point
   * where the boundary of A meets the interior of B has a neighbourhood
   * inside B, and the far side of that boundary is the exterior of A, so the
   * two meet in a two-dimensional set. Neither source above sees it when B
   * covers a hole of A: the boundary of B stays clear of the exterior of A,
   * and the interior witness of B lands in the body of A rather than in the
   * hole */
  if (m->bi != -1)
    de9im_add(&m->ei, 2);
  if (m->ib != -1)
    de9im_add(&m->ie, 2);

  /* Where the boundary of the other geometry meets no interior point of one,
   * a witness per ring of the one answers what a witness per edge does */
  if (m->ie != 2 && (m->ib == -1 ?
        relate_area_ring_point_located(&re1, &re2, 2) :
        relate_area_interior_point_located(&re1, &re2, 2)))
    de9im_add(&m->ie, 2);
  if (m->ei != 2 && (m->bi == -1 ?
        relate_area_ring_point_located(&re2, &re1, 2) :
        relate_area_interior_point_located(&re2, &re1, 2)))
    de9im_add(&m->ei, 2);

  /* Exterior / Exterior.
   * Two ordinary finite areal geometries always have a
   * two-dimensional common exterior. */
  de9im_add(&m->ee, 2);

done:
  /* A kept index belongs to its context and outlives this call */
  if (kept1)
    re1.index = NULL;
  if (kept2)
    re2.index = NULL;
  relate_edges_clear(&re1); relate_edges_clear(&re2);
  pfree(e1); pfree(e2);
  relate_return_edges(ops, a1); relate_return_edges(ops, a2);
  return;
}

/*****************************************************************************
 * The topology of a collection is that of the union of its components
 *****************************************************************************/

/**
 * @brief The edges of one areal component of a geometry
 * @details The components are kept apart from one another because the
 * interior of their union is not the union of their interiors: a point on an
 * edge two components share is interior to the union, while it lies on the
 * boundary of each component taken alone
 */
typedef struct
{
  MeosArray *arr;  /**< Edges of the component, owning their memory */
  Edge **edges;    /**< Pointers into the array above */
  int nedges;      /**< Number of edges */
  RelateEdges re;  /**< The same edges, with what reading them selectively
                        needs, so a point is located against a component
                        without walking it whole */
  double xmin, ymin, xmax, ymax; /**< Extent of the component, read from the
                        boxes its edges already carry */
} RelateComp;

/**
 * @brief Collect the areal components of a geometry, one entry per surface
 * @details A polygon carries its holes, so a surface and its rings are one
 * component; a multi-geometry or a collection contributes its components,
 * recursively. A component of another dimension bounds no area and is skipped
 */
static void
relate_area_comps_iter(const LWGEOM *geom, RelateComp **comps, int *ncomp,
  int *maxcomp, bool index)
{
  if (! geom || lwgeom_is_empty(geom))
    return;
  switch (geom->type)
  {
    case MULTIPOLYGONTYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < col->ngeoms; i++)
        relate_area_comps_iter(col->geoms[i], comps, ncomp, maxcomp, index);
      return;
    }
    case POLYGONTYPE:
    case TRIANGLETYPE:
    case CURVEPOLYTYPE:
      break;
    default:
      return;
  }
  if (*ncomp == *maxcomp)
  {
    *maxcomp *= 2;
    *comps = repalloc(*comps, sizeof(RelateComp) * (*maxcomp));
  }
  RelateComp *c = &(*comps)[(*ncomp)++];
  c->arr = geom_extract_edges(geom);
  c->nedges = (int) c->arr->count;
  c->edges = palloc(sizeof(Edge *) * Max(c->nedges, 1));
  for (int i = 0; i < c->nedges; i++)
    c->edges[i] = (Edge *) meos_array_get_intl(c->arr, i);
  relate_edges_init(&c->re, c->edges, c->nedges, index);
  c->xmin = c->ymin = DBL_MAX;
  c->xmax = c->ymax = -DBL_MAX;
  for (int i = 0; i < c->nedges; i++)
  {
    const Edge *e = c->edges[i];
    c->xmin = fmin(c->xmin, e->xmin); c->xmax = fmax(c->xmax, e->xmax);
    c->ymin = fmin(c->ymin, e->ymin); c->ymax = fmax(c->ymax, e->ymax);
  }
  return;
}

/**
 * @brief Free the areal components of a geometry
 */
static void
relate_comps_free(RelateComp *comps, int ncomp)
{
  for (int i = 0; i < ncomp; i++)
  {
    relate_edges_clear(&comps[i].re);
    pfree(comps[i].edges);
    meos_array_destroy(comps[i].arr);
  }
  pfree(comps);
  return;
}

/**
 * @brief Return true if a point lies in the closure of the union of the
 * areal components
 */
static bool
relate_in_area_union(double x, double y, const RelateComp *comps, int ncomp)
{
  for (int i = 0; i < ncomp; i++)
    if (relate_point_in_area_index(x, y, &comps[i].re, false) != 2)
      return true;
  return false;
}

/**
 * @brief Return the squared distance from a point to a segment
 * @details The nearest point of the segment is the projection clamped to it.
 * A segment of no length is its start, which is exactly where the squared
 * length is zero; a squared length too small for a double is zero too. The
 * segment case of #point_edge_dist2
 */
double
point_seg_dist2(double px, double py, double x1, double y1, double x2,
  double y2)
{
  const double ux = x2 - x1, uy = y2 - y1;
  const double l2 = ux * ux + uy * uy;
  if (l2 == 0.0)
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
 * is the difference to the supporting circle, which takes the root of the
 * squared distance to the centre before it is squared, otherwise it is the
 * distance to the nearer arc endpoint. The arc case of #point_edge_dist2
 */
double
point_arc_dist2(double px, double py, const Edge *e)
{
  const double dxc = px - e->cx, dyc = py - e->cy;
  if (arc_contains_angle(e, atan2(dyc, dxc)))
  {
    const double dd = sqrt(dxc * dxc + dyc * dyc) - e->radius;
    return dd * dd;
  }
  const double d0x = px - e->x1, d0y = py - e->y1;
  const double d1x = px - e->x2, d1y = py - e->y2;
  const double d0 = d0x * d0x + d0y * d0y;
  const double d1 = d1x * d1x + d1y * d1y;
  return Min(d0, d1);
}

/**
 * @brief Return the squared distance from a point to a single edge
 * @details The geometry layer's one reading of it, the square of what
 * #buffer_point_edge_distance answers for the buffer: the clip engine of a
 * moving point and the clearance search of a union (#relate_clearance) both
 * compare these squares, and none of them takes a root but the one an arc's
 * circle needs
 */
double
point_edge_dist2(double px, double py, const Edge *e)
{
  switch (e->etype)
  {
    case EDGE_POINT:
    {
      const double dx = px - e->x1, dy = py - e->y1;
      return dx * dx + dy * dy;
    }
    case EDGE_LINESEG:
    case EDGE_POLYSEG:
      return point_seg_dist2(px, py, e->x1, e->y1, e->x2, e->y2);
    case EDGE_LINEARC:
    case EDGE_POLYARC:
      return point_arc_dist2(px, py, e);
  }
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Unknown edge type: %d", e->etype);
  return DBL_MAX;
}

/**
 * @brief Return how far a point can be moved before it reaches a feature it
 * does not already lie on
 * @details The two sides of a boundary edge are told apart by reading a point
 * placed off the edge. This answers how far off it may be placed while still
 * reading the neighbourhood of that edge alone
 */
static double
relate_clearance(double x, double y, const RelateComp *comps, int ncomp,
  double seed)
{
  /* The nearest feature is found by growing a box about the point until one
   * falls inside it. An edge whose box stands outside a box of half-width r
   * has every one of its points outside it, so it lies further than r, and a
   * feature found no further than r is therefore the nearest there is. Where
   * a component carries no index the walk below answers, and it answers if
   * the growth ever runs out, so the two paths agree by construction */
  /* The search compares squared distances (#point_edge_dist2) and takes one
   * root, of the clearance it answers, a length its caller places points at.
   * A seed of no extent, a piece of no length, grows no box: the walk below
   * answers it */
  bool indexed = (ncomp > 0 && seed > 0.0);
  for (int i = 0; i < ncomp && indexed; i++)
    indexed = (comps[i].re.index != NULL);
  if (indexed)
  {
    MeosArray *candidates = index_result_create();
    double r = seed;
    for (int round = 0; round < 64; round++)
    {
      double best2 = -1;
      for (int i = 0; i < ncomp; i++)
      {
        int nc = edge_index_query(comps[i].re.index, comps[i].re.edges,
          x - r, x + r, y - r, y + r, candidates);
        for (int c = 0; c < nc; c++)
        {
          int j = (int) INDEX_RESULT_ID_N(candidates, c);
          const Edge *e = comps[i].edges[j];
          double d2 = point_edge_dist2(x, y, e);
          /* An edge the point LIES ON is no clearance from it. What the
           * distance misses the edge by is a property of the arithmetic --
           * a few units in the last place of the largest coordinate -- so an
           * absolute floor leaves a point on its own edge reading as a
           * feature a rounding step away */
          if (d2 <= e->tol * e->tol)
            continue;
          if (best2 < 0 || d2 < best2)
            best2 = d2;
        }
      }
      if (best2 >= 0 && best2 <= r * r)
      {
        meos_array_destroy(candidates);
        return sqrt(best2);
      }
      r *= 4;
    }
    meos_array_destroy(candidates);
  }
  double result2 = -1;
  for (int i = 0; i < ncomp; i++)
    for (int j = 0; j < comps[i].nedges; j++)
    {
      const Edge *e = comps[i].edges[j];
      double d2 = point_edge_dist2(x, y, e);
      /* An edge the point LIES ON is no clearance from it. What the
       * distance misses the edge by is a property of the arithmetic --
       * a few units in the last place of the largest coordinate -- so an
       * absolute floor leaves a point on its own edge reading as a
       * feature a rounding step away */
      if (d2 <= e->tol * e->tol)
        continue;
      if (result2 < 0 || d2 < result2)
        result2 = d2;
    }
  return (result2 < 0) ? -1 : sqrt(result2);
}

/**
 * @brief Return the direction of an edge at one of its parameters
 */
static void
relate_edge_tangent(const Edge *e, double t, double *tx, double *ty)
{
  if (e->etype == EDGE_POLYSEG || e->etype == EDGE_LINESEG)
  {
    double len = hypot(e->dx, e->dy);
    *tx = len > 0 ? e->dx / len : 1;
    *ty = len > 0 ? e->dy / len : 0;
    return;
  }
  /* The tangent of an arc is perpendicular to its radius, pointing the way
   * the arc is traversed */
  double sweep = e->ccw ?
    angle_normalize(e->theta1 - e->theta0) :
    angle_normalize(e->theta0 - e->theta1);
  double theta = e->ccw ? e->theta0 + t * sweep : e->theta0 - t * sweep;
  *tx = e->ccw ? -sin(theta) : sin(theta);
  *ty = e->ccw ? cos(theta) : -cos(theta);
  return;
}

/**
 * @brief Return true if a portion of a boundary edge lies in the interior of
 * the union of the areal components rather than on its boundary
 * @details The portion carries the interior of its own component on one side.
 * It is interior to the union when the union covers the other side as well,
 * which is read from a point placed off the portion, within the clearance so
 * that no other feature is reached
 */
static bool
relate_portion_inside_union(const Edge *e, double t, const RelateComp *comps,
  int ncomp)
{
  double x, y;
  relate_area_edge_point(e, t, &x, &y);
  /* A portion is interior to the union only where components MEET: the point
   * lies on the boundary of the component it belongs to, so some OTHER
   * component has to cover a neighbourhood of it, and that component's extent
   * must then contain it. Reading the extents settles every portion no second
   * component reaches, which is most of them on real multi-part data, and
   * costs one comparison per component against two point locations */
  {
    int reaching = 0;
    for (int i = 0; i < ncomp && reaching < 2; i++)
    {
      double pad = comps[i].re.tol;
      if (x >= comps[i].xmin - pad && x <= comps[i].xmax + pad &&
          y >= comps[i].ymin - pad && y <= comps[i].ymax + pad)
        reaching++;
    }
    if (reaching < 2)
      return false;
  }
  /* The portion's own edge sets the scale the search starts at, a length: the
   * radius of an arc, and a sixteenth of the larger extent of a segment's box,
   * which no root reaches and which a power of two keeps exact under scaling.
   * The box of the first round then holds the features nearest a point of a
   * boundary, normally the next ones along it, and few others */
  double delta = relate_clearance(x, y, comps, ncomp,
    e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC ?
      e->radius : 0.0625 * Max(fabs(e->dx), fabs(e->dy)));
  if (delta <= 0)
    return false;
  delta *= 0.5;
  double tx, ty;
  relate_edge_tangent(e, t, &tx, &ty);
  /* The two sides of the portion, along the normal to its direction */
  return relate_in_area_union(x - delta * ty, y + delta * tx, comps, ncomp) &&
    relate_in_area_union(x + delta * ty, y - delta * tx, comps, ncomp);
}

/**
 * @brief Return true if two boundary edge portions are the same portion
 * @details Two components whose boundaries run along one another on the same
 * side each contribute that portion, and the union bounds it once. A portion
 * kept twice is crossed twice by a ray, so the parity that locates a point
 * would read the inside of the union as its outside
 */
static bool
relate_same_portion(const Edge *a, const Edge *b)
{
  if (a->etype != b->etype)
    return false;
  if (a->etype == EDGE_POLYARC)
  {
    if (! arc_same_circle(a, b))
      return false;
    /* Two endpoints do not determine an arc of a circle: the two arcs a full
     * circle is read as carry the same pair and bow to opposite sides, so a
     * portion is told from its complement by a point strictly inside it */
    double amx, amy, bmx, bmy;
    relate_area_edge_point(a, 0.5, &amx, &amy);
    relate_area_edge_point(b, 0.5, &bmx, &bmy);
    if (! relate_same_point(amx, amy, bmx, bmy))
      return false;
  }
  /* The ends of a portion are constructed on the edge it comes from, so two
   * ends are one point within the rounding those edges carry
   * (#edge_set_tolerance). The two components may traverse the portion in
   * opposite directions */
  double tol = (a->tol > b->tol) ? a->tol : b->tol;
  if (fabs(a->x1 - b->x1) <= tol && fabs(a->y1 - b->y1) <= tol &&
      fabs(a->x2 - b->x2) <= tol && fabs(a->y2 - b->y2) <= tol)
    return true;
  return fabs(a->x1 - b->x2) <= tol && fabs(a->y1 - b->y2) <= tol &&
    fabs(a->x2 - b->x1) <= tol && fabs(a->y2 - b->y1) <= tol;
}

/**
 * @brief An areal member of a collection, read in place from its rings
 */
typedef struct
{
  POINTARRAY * const *rings; /**< Rings of the member, the first its shell */
  uint32_t nrings;           /**< Number of rings */
  int orient;                /**< Orientation of the shell, 1 or -1 */
  double xmin, ymin, xmax, ymax; /**< Extent of the vertices of the member */
} RelateMember;

/**
 * @brief Return 1 if a ring runs counterclockwise, -1 if it runs clockwise,
 * and 0 where the sum of doubles cannot be trusted to carry the sign
 * @details The sign of the shoelace sum over the input vertices, each term
 * read relative to the first vertex. A term carries a rounding error of at
 * most 4 DBL_EPSILON times the magnitudes of its two products, and each
 * addition at most DBL_EPSILON times the partial sum, so the sign holds
 * wherever the sum exceeds (n + 8) DBL_EPSILON times the magnitudes of all
 * the products
 */
static int
ring_orientation_sign(const POINTARRAY *pa)
{
  if (pa->npoints < 4)
    return 0;
  const POINT2D *p0 = getPoint2d_cp(pa, 0);
  double sum = 0.0, mag = 0.0;
  for (uint32_t i = 1; i + 2 < pa->npoints; i++)
  {
    const POINT2D *p = getPoint2d_cp(pa, i);
    const POINT2D *q = getPoint2d_cp(pa, i + 1);
    double left = (p->x - p0->x) * (q->y - p0->y);
    double right = (p->y - p0->y) * (q->x - p0->x);
    sum += left - right;
    mag += fabs(left) + fabs(right);
  }
  double bound = ((double) pa->npoints + 8.0) * DBL_EPSILON * mag;
  return (sum > bound) - (sum < - bound);
}

/**
 * @brief Return 1 if an input vertex lies in the interior of the area some
 * rings bound, 0 if it lies outside it, and -1 if it lies on one of the rings
 * @details A ray cast to the right at the vertex's own height crosses an edge
 * by the half-open rule, and the side of the edge the vertex lies on is the
 * sign #cross_product_sign reads exactly, so the answer holds at every scale
 */
static int
point_in_rings_exact(const POINT2D *pt, POINTARRAY * const *rings,
  uint32_t nrings)
{
  bool inside = false;
  for (uint32_t r = 0; r < nrings; r++)
  {
    const POINTARRAY *pa = rings[r];
    for (uint32_t i = 0; i + 1 < pa->npoints; i++)
    {
      const POINT2D *a = getPoint2d_cp(pa, i);
      const POINT2D *b = getPoint2d_cp(pa, i + 1);
      bool up = a->y <= pt->y && pt->y < b->y;
      bool down = b->y <= pt->y && pt->y < a->y;
      if (! up && ! down)
      {
        /* An edge the ray does not cross carries the vertex at its start or,
         * lying at the vertex's height, between its two ends */
        if ((a->x == pt->x && a->y == pt->y) ||
            (a->y == pt->y && b->y == pt->y && Min(a->x, b->x) <= pt->x &&
              pt->x <= Max(a->x, b->x)))
          return -1;
        continue;
      }
      int side = cross_product_sign(a->x, a->y, b->x, b->y, a->x, a->y,
        pt->x, pt->y);
      if (side == 0)
        return -1;
      if ((up && side > 0) || (down && side < 0))
        inside = ! inside;
    }
  }
  return inside ? 1 : 0;
}

/**
 * @brief Return the sign of the turn from the ray v->u to the ray v->d
 */
static inline int
ray_turn(const POINT2D *v, const POINT2D *u, const POINT2D *d)
{
  return cross_product_sign(v->x, v->y, u->x, u->y, v->x, v->y, d->x, d->y);
}

/**
 * @brief Return true if two rays of one line from a vertex point the same way
 * @note Read only where #ray_turn is exactly 0: the two products then share
 * their sign, so their sum carries it
 */
static inline bool
ray_same(const POINT2D *v, const POINT2D *u, const POINT2D *d)
{
  return (u->x - v->x) * (d->x - v->x) + (u->y - v->y) * (d->y - v->y) > 0;
}

/**
 * @brief Return whether the ray from a vertex through a point lies inside a
 * wedge
 * @details The wedge is swept counterclockwise from the ray through @p u to
 * the ray through @p w. The function returns 1 if the ray lies strictly inside
 * the wedge, 0 if it lies strictly outside it, and -1 if it runs along one of
 * the two rays or the wedge has no angle.
 */
static int
ray_in_wedge(const POINT2D *v, const POINT2D *u, const POINT2D *w,
  const POINT2D *d)
{
  int ud = ray_turn(v, u, d), dw = ray_turn(v, d, w), uw = ray_turn(v, u, w);
  if ((ud == 0 && ray_same(v, u, d)) || (dw == 0 && ray_same(v, d, w)) ||
      (uw == 0 && ray_same(v, u, w)))
    return -1;
  /* A wedge narrower than a half-plane holds what turns into it from both
   * rays; a wider one holds what either ray turns into; a half-plane holds
   * what turns left of its first ray */
  if (uw > 0)
    return ud > 0 && dw > 0;
  if (uw < 0)
    return ud > 0 || dw > 0;
  return ud > 0;
}

/**
 * @brief Read the interior wedge of a member at a vertex of one of its rings
 * @details The interior lies to the left of a shell running counterclockwise
 * and to the right of one running clockwise, and the other way round for a
 * hole, so it is swept counterclockwise from one neighbour of the vertex to
 * the other
 * @return False where the wedge cannot be read: a ring whose orientation the
 * filter does not settle, or a neighbour repeating the vertex
 */
static bool
relate_member_wedge(const RelateMember *m, uint32_t r, uint32_t k,
  const POINT2D **v, const POINT2D **u, const POINT2D **w)
{
  const POINTARRAY *pa = m->rings[r];
  uint32_t n = pa->npoints;
  /* The last point of a ring repeats its first */
  if (k == n - 1)
    k = 0;
  const POINT2D *prev = getPoint2d_cp(pa, k == 0 ? n - 2 : k - 1);
  const POINT2D *next = getPoint2d_cp(pa, k + 1);
  *v = getPoint2d_cp(pa, k);
  if ((prev->x == (*v)->x && prev->y == (*v)->y) ||
      (next->x == (*v)->x && next->y == (*v)->y))
    return false;
  int orient = (r == 0) ? m->orient : - ring_orientation_sign(pa);
  if (orient == 0)
    return false;
  *u = orient > 0 ? next : prev;
  *w = orient > 0 ? prev : next;
  return true;
}

/**
 * @brief Return true if the interiors of two members stand apart about a
 * vertex they share
 * @details Two wedges about one vertex overlap exactly where a ray bounding
 * one of them lies inside the other, so the four rays settle it
 */
static bool
relate_members_wedges_apart(const RelateMember *a, uint32_t ra, uint32_t ia,
  const RelateMember *b, uint32_t rb, uint32_t ib)
{
  const POINT2D *v, *vb, *au, *aw, *bu, *bw;
  if (! relate_member_wedge(a, ra, ia, &v, &au, &aw) ||
      ! relate_member_wedge(b, rb, ib, &vb, &bu, &bw))
    return false;
  return ray_in_wedge(v, au, aw, bu) == 0 && ray_in_wedge(v, au, aw, bw) == 0 &&
    ray_in_wedge(v, bu, bw, au) == 0 && ray_in_wedge(v, bu, bw, aw) == 0;
}

/**
 * @brief Return true if no ring of a member lies in the interior of another
 * @details Where two members meet nowhere but at vertices they share, and
 * their interiors stand apart about each of them, every ring of one lies
 * outside the other except at those vertices, or inside it whole. A vertex of
 * the ring off the boundary of the other member says which. A vertex on that
 * boundary is one the two share, every other contact being refused before
 * this is read, and the wedges about it read apart, so both edges of the ring
 * leave it outside the other member: a ring every vertex of which is shared
 * lies outside it
 */
static bool
relate_member_rings_outside(const RelateMember *a, const RelateMember *b,
  int64 *budget)
{
  for (uint32_t r = 0; r < a->nrings; r++)
  {
    const POINTARRAY *pa = a->rings[r];
    for (uint32_t k = 0; k + 1 < pa->npoints; k++)
    {
      const POINT2D *p = getPoint2d_cp(pa, k);
      if (p->x < b->xmin || p->x > b->xmax || p->y < b->ymin || p->y > b->ymax)
        break;
      for (uint32_t q = 0; q < b->nrings; q++)
        *budget -= b->rings[q]->npoints;
      if (*budget < 0)
        return false;
      int in = point_in_rings_exact(p, b->rings, b->nrings);
      if (in == 1)
        return false;
      if (in == 0)
        break;
    }
  }
  return true;
}

/**
 * @brief Return true if a segment of one member and a segment of another are
 * known not to make the two meet but at a vertex they share, with their
 * interiors apart about it
 * @details The tests #relate_members_pair_apart() reads each pair of segments
 * by
 */
static bool
relate_segments_apart(const RelateMember *a, uint32_t ra, uint32_t i,
  const RelateMember *b, uint32_t rb, uint32_t j, double tol)
{
  const POINT2D *p = getPoint2d_cp(a->rings[ra], i);
  const POINT2D *q = getPoint2d_cp(a->rings[ra], i + 1);
  const POINT2D *r = getPoint2d_cp(b->rings[rb], j);
  const POINT2D *s = getPoint2d_cp(b->rings[rb], j + 1);
    bool pr = p->x == r->x && p->y == r->y;
    bool ps = p->x == s->x && p->y == s->y;
    bool qr = q->x == r->x && q->y == r->y;
    bool qs = q->x == s->x && q->y == s->y;
    /* An edge the two members carry alike */
    if ((pr && qs) || (ps && qr))
      return false;
    if ((! pr && ! ps &&
          point_on_segment_within(p->x, p->y, r->x, r->y, s->x, s->y, tol)) ||
        (! qr && ! qs &&
          point_on_segment_within(q->x, q->y, r->x, r->y, s->x, s->y, tol)) ||
        (! pr && ! qr &&
          point_on_segment_within(r->x, r->y, p->x, p->y, q->x, q->y, tol)) ||
        (! ps && ! qs &&
          point_on_segment_within(s->x, s->y, p->x, p->y, q->x, q->y, tol)))
      return false;
    /* A crossing, each segment's ends strictly on the two sides of the
     * other */
    int o1 = cross_product_sign(p->x, p->y, q->x, q->y, p->x, p->y, r->x, r->y);
    int o2 = cross_product_sign(p->x, p->y, q->x, q->y, p->x, p->y, s->x, s->y);
    if (o1 * o2 < 0)
    {
      int o3 = cross_product_sign(r->x, r->y, s->x, s->y, r->x, r->y,
        p->x, p->y);
      int o4 = cross_product_sign(r->x, r->y, s->x, s->y, r->x, r->y,
        q->x, q->y);
      if (o3 * o4 < 0)
        return false;
    }
    /* A vertex the two share ends two segments of each, so it turns up
     * in four pairs of them, and the wedges about it are the same in
     * each: they are read in the one pair both segments start at it */
    if (pr && ! relate_members_wedges_apart(a, ra, i, b, rb, j))
      return false;
  return true;
}

/**
 * @brief A segment of a member read by the sweep of two members
 */
typedef struct
{
  double xmin, xmax, ymin, ymax; /**< Box of the segment */
  uint32_t r;                    /**< Ring of the segment */
  uint32_t i;                    /**< Its first vertex in the ring */
  bool second;                   /**< True for a segment of the second member */
} RelateSeg;

/**
 * @brief Order the segments of a sweep by the left end of their box, then by
 * member, ring and vertex, which makes the order total
 */
static int
relate_seg_cmp(const void *x, const void *y)
{
  const RelateSeg *a = (const RelateSeg *) x, *b = (const RelateSeg *) y;
  if (a->xmin != b->xmin)
    return (a->xmin > b->xmin) - (a->xmin < b->xmin);
  if (a->second != b->second)
    return (int) a->second - (int) b->second;
  if (a->r != b->r)
    return (a->r > b->r) - (a->r < b->r);
  return (a->i > b->i) - (a->i < b->i);
}

/**
 * @brief Add to a sweep the segments of a member within a box
 */
static void
relate_member_segs(const RelateMember *m, bool second, double ox0, double ox1,
  double oy0, double oy1, RelateSeg *segs, uint32_t *ns)
{
  for (uint32_t r = 0; r < m->nrings; r++)
  {
    const POINTARRAY *pa = m->rings[r];
    for (uint32_t i = 0; i + 1 < pa->npoints; i++)
    {
      const POINT2D *p = getPoint2d_cp(pa, i);
      const POINT2D *q = getPoint2d_cp(pa, i + 1);
      if (Max(p->x, q->x) < ox0 || Min(p->x, q->x) > ox1 ||
          Max(p->y, q->y) < oy0 || Min(p->y, q->y) > oy1 ||
          (p->x == q->x && p->y == q->y))
        continue;
      RelateSeg *g = &segs[(*ns)++];
      g->xmin = Min(p->x, q->x); g->xmax = Max(p->x, q->x);
      g->ymin = Min(p->y, q->y); g->ymax = Max(p->y, q->y);
      g->r = r; g->i = i; g->second = second;
    }
  }
}

/**
 * @brief Return true if two members are known to meet nowhere but at vertices
 * they share, with their interiors apart there and everywhere else
 * @details Only the segments within the overlap of the two extents can meet.
 * An end of one segment within the tolerance of the other is a contact the
 * union has to read unless it is a vertex the two carry alike, and a crossing
 * or an edge the two carry alike is one as well. The segments of both members
 * within the overlap are swept together by the left end of their box, so only
 * a segment of each whose boxes meet within the tolerance is read
 * (#relate_segments_apart); whether every pair passes does not depend on the
 * order they are read in
 */
static bool
relate_members_pair_apart(const RelateMember *a, const RelateMember *b,
  double tol, int64 *budget)
{
  double ox0 = Max(a->xmin, b->xmin) - tol, ox1 = Min(a->xmax, b->xmax) + tol;
  double oy0 = Max(a->ymin, b->ymin) - tol, oy1 = Min(a->ymax, b->ymax) + tol;
  uint32_t na = 0, nb = 0;
  for (uint32_t r = 0; r < a->nrings; r++)
    na += a->rings[r]->npoints;
  for (uint32_t r = 0; r < b->nrings; r++)
    nb += b->rings[r]->npoints;
  RelateSeg *segs = palloc(sizeof(RelateSeg) * Max(na + nb, 1u));
  uint32_t ns = 0;
  relate_member_segs(a, false, ox0, ox1, oy0, oy1, segs, &ns);
  relate_member_segs(b, true, ox0, ox1, oy0, oy1, segs, &ns);
  qsort(segs, ns, sizeof(RelateSeg), relate_seg_cmp);

  bool result = false;
  for (uint32_t k = 0; k < ns; k++)
  {
    for (uint32_t l = k + 1; l < ns; l++)
    {
      if (segs[l].xmin > segs[k].xmax + tol)
        break;
      if (segs[k].second == segs[l].second ||
          segs[k].ymax + tol < segs[l].ymin ||
          segs[l].ymax + tol < segs[k].ymin)
        continue;
      if (--(*budget) < 0)
        goto done;
      const RelateSeg *ga = segs[k].second ? &segs[l] : &segs[k];
      const RelateSeg *gb = segs[k].second ? &segs[k] : &segs[l];
      if (! relate_segments_apart(a, ga->r, ga->i, b, gb->r, gb->i, tol))
        goto done;
    }
  }
  result = relate_member_rings_outside(a, b, budget) &&
    relate_member_rings_outside(b, a, budget);

done:
  pfree(segs);
  return result;
}

/**
 * @brief Compare two members by the left side of their extents
 */
static int
relate_member_xmin_cmp(const void *x, const void *y)
{
  double a = ((const RelateMember *) x)->xmin;
  double b = ((const RelateMember *) y)->xmin;
  return (a > b) - (a < b);
}

/**
 * @brief Read an areal member of a collection in place
 * @return False where the member is not one the check reads: a curved or
 * nested surface, a ring too short to bound anything, or a shell whose
 * orientation the filter does not settle or which encloses no area
 */
static bool
relate_member_read(const LWGEOM *g, RelateMember *c)
{
  switch (g->type)
  {
    case POLYGONTYPE:
      c->rings = ((const LWPOLY *) g)->rings;
      c->nrings = ((const LWPOLY *) g)->nrings;
      break;
    case TRIANGLETYPE:
      c->rings = &((const LWTRIANGLE *) g)->points;
      c->nrings = 1;
      break;
    default:
      return false;
  }
  c->xmin = c->ymin = DBL_MAX;
  c->xmax = c->ymax = -DBL_MAX;
  for (uint32_t r = 0; r < c->nrings; r++)
  {
    const POINTARRAY *pa = c->rings[r];
    if (pa->npoints < 4)
      return false;
    for (uint32_t k = 0; k < pa->npoints; k++)
    {
      const POINT2D *p = getPoint2d_cp(pa, k);
      c->xmin = Min(c->xmin, p->x); c->xmax = Max(c->xmax, p->x);
      c->ymin = Min(c->ymin, p->y); c->ymax = Max(c->ymax, p->y);
    }
  }
  c->orient = ring_orientation_sign(c->rings[0]);
  return c->orient != 0 && ! ring_encloses_no_area(c->rings[0]);
}

/**
 * @brief Return true if the areal members of a collection are known to meet
 * nowhere but at vertices they share and to cover no part of one another
 * @details The union of such members bounds exactly what they bound, so the
 * edges of the collection are the edges of the union. That is what a valid
 * multipolygon is under the simple feature rule, whose members' interiors do
 * not meet and whose boundaries touch at finitely many points, and GEOS reads
 * the rings of a multipolygon as they are for that reason. Here it is shown
 * rather than assumed, from the input vertices read in place: the members are
 * swept by their extents, so only two whose extents meet are compared, then
 * their segments within the overlap of the extents, the interior wedges about
 * each vertex they share, and one vertex of each ring against the other.
 * @param[in] geom Collection
 * @param[in] budget The work it may spend, counted in pairs of segments and
 * points of rings read, after which it answers false
 * @return False where any of this is not shown: a curved, nested or
 * degenerate member, a contact the tests do not settle, or more work than the
 * budget. The union is then computed
 */
bool
relate_members_apart_within(const LWGEOM *geom, int64 budget)
{
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  RelateMember *m = palloc(sizeof(RelateMember) * Max(col->ngeoms, 1));
  int n = 0;
  bool result = false;
  double extent = 0.0;
  for (uint32_t i = 0; i < col->ngeoms; i++)
  {
    const LWGEOM *g = col->geoms[i];
    /* A member of another dimension bounds no area of the union */
    if (lwgeom_is_empty(g) || lwgeom_dimension(g) < 2)
      continue;
    RelateMember *c = &m[n];
    if (! relate_member_read(g, c))
      goto done;
    extent = Max(extent, Max(Max(fabs(c->xmin), fabs(c->xmax)),
      Max(fabs(c->ymin), fabs(c->ymax))));
    n++;
  }

  /* Twice what an edge of the collection reads two points within, so no
   * contact the union reads falls outside it */
  double tol = 2.0 * coordinate_tolerance(extent, extent);
  qsort(m, n, sizeof(RelateMember), relate_member_xmin_cmp);
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n && m[j].xmin <= m[i].xmax + tol; j++)
    {
      if (m[j].ymin > m[i].ymax + tol || m[i].ymin > m[j].ymax + tol)
        continue;
      if (! relate_members_pair_apart(&m[i], &m[j], tol, &budget))
        goto done;
    }
  result = true;

done:
  pfree(m);
  return result;
}

/**
 * @brief Return true if the areal members of a collection are known to meet
 * nowhere but at vertices they share and to cover no part of one another
 * @details #relate_members_apart_within(), spending no more than an index
 * would be worth (#RELATE_INDEX_MIN_PAIRS)
 */
bool
relate_members_apart(const LWGEOM *geom)
{
  return relate_members_apart_within(geom, RELATE_INDEX_MIN_PAIRS);
}

/* What fixes a parameter an edge of a union is cut at (#relate_union_edges):
 * an end of the edge or a meeting held only rounded, the crossing of a
 * segment, or a vertex of the edge running along it */
typedef enum
{
  RELATE_CUT_ROUNDED,
  RELATE_CUT_CROSSING,
  RELATE_CUT_VERTEX1,
  RELATE_CUT_VERTEX2
} RelateCutKind;

/* A parameter an edge of a union is cut at, with the edge that cuts it */
typedef struct
{
  double t;
  RelateCutKind kind;
  const Edge *by;
} RelateCut;

/**
 * @brief Sort the cuts of an edge by their parameter, as
 * #relate_area_parameter_cmp sorts the parameters
 */
static int
relate_cut_cmp(const void *a, const void *b)
{
  return relate_area_parameter_cmp(&((const RelateCut *) a)->t,
    &((const RelateCut *) b)->t);
}

/**
 * @brief Add a parameter to the cuts of an edge, as #relate_area_add_parameter
 * adds it, recording what cuts the edge there where it is added
 */
static void
relate_union_add_cut(double t, RelateCutKind kind, const Edge *by,
  double *params, RelateCut *cuts, int *nparams, int maxparams)
{
  int n = *nparams;
  relate_area_add_parameter(t, params, nparams, maxparams);
  if (*nparams > n)
  {
    cuts[n].t = params[n];
    cuts[n].kind = kind;
    cuts[n].by = by;
  }
  return;
}

/**
 * @brief Set an end of a piece of an edge of a union to the point it is cut
 * at, keeping what fixes that point exactly
 * @details A vertex of an edge running along this one is that input vertex. A
 * crossing of two segments is a point no double holds: the piece holds it
 * rounded once and keeps the input segment crossing it, so that every
 * question about that end reads the crossing (#relate_segments_intersect)
 */
static void
relate_union_piece_end(const Edge *e, const RelateCut *cut, int end,
  Edge *piece)
{
  double *x = end ? &piece->x2 : &piece->x1;
  double *y = end ? &piece->y2 : &piece->y1;
  switch (cut->kind)
  {
    case RELATE_CUT_VERTEX1:
      *x = cut->by->x1; *y = cut->by->y1;
      return;
    case RELATE_CUT_VERTEX2:
      *x = cut->by->x2; *y = cut->by->y2;
      return;
    case RELATE_CUT_CROSSING:
      piece->crossed[end] = true;
      piece->crosser[end][0] = cut->by->x1;
      piece->crosser[end][1] = cut->by->y1;
      piece->crosser[end][2] = cut->by->x2;
      piece->crosser[end][3] = cut->by->y2;
      relate_area_edge_point(e, cut->t, x, y);
      return;
    case RELATE_CUT_ROUNDED:
      relate_area_edge_point(e, cut->t, x, y);
      return;
  }
  return;
}

/**
 * @brief Return the edges of a collection, carrying the boundary of the union
 * of its areal components in place of the boundaries of the components
 * @details Two components of a collection may share an edge or overlap, and
 * such an edge lies in the interior of the union rather than on its boundary.
 * Every boundary edge is therefore split at its intersections with the other
 * components, and each portion the union covers on both sides is dropped,
 * which leaves the boundary of the union. What remains is a geometry whose
 * surfaces no longer overlap, so the cells are computed on it exactly as for
 * a multi-geometry
 */
static MeosArray *
relate_union_edges(const LWGEOM *geom, MeosArray *all)
{
  /* Members meeting nowhere but at vertices they share bound together exactly
   * what they bound apart, so their own edges are those of the union */
  if (relate_members_apart(geom))
    return all;

  int nall = (int) all->count;
  /* Every edge is located against every component, so a component is read
   * once for each edge and the two indexes below are worth what the same
   * threshold is worth anywhere else in this file */
  bool index = ((int64) nall * (int64) nall >= RELATE_INDEX_MIN_PAIRS);

  int ncomp = 0, maxcomp = 8;
  RelateComp *comps = palloc(sizeof(RelateComp) * maxcomp);
  relate_area_comps_iter(geom, &comps, &ncomp, &maxcomp, index);

  /* A collection holding at most one surface has no overlap to resolve */
  if (ncomp < 2)
  {
    relate_comps_free(comps, ncomp);
    return all;
  }

  MeosArray *result = meos_array_create(sizeof(Edge));
  int maxparams = 2 * nall + 2;
  double *params = palloc(sizeof(double) * maxparams);
  RelateCut *cuts = palloc(sizeof(RelateCut) * maxparams);
  /* Every edge is asked against every other, so the walk costs the SQUARE of
   * the array while indexing it costs one pass. Two edges that meet have
   * boxes that meet, so the index answers the pairs the walk would split at
   * and leaves out the ones it would only reject */
  Edge **edges = palloc(sizeof(Edge *) * Max(nall, 1));
  for (int i = 0; i < nall; i++)
    edges[i] = (Edge *) meos_array_get_intl(all, i);
  RelateEdges re;
  relate_edges_init(&re, edges, nall, index);
  MeosArray *candidates = re.index ? index_result_create() : NULL;
  for (int i = 0; i < nall; i++)
  {
    Edge *e = edges[i];
    if (! relate_area_boundary_edge(e))
    {
      /* An edge of another dimension bounds no area of the union */
      meos_array_add(result, e);
      continue;
    }
    /* Split the edge at every intersection with another boundary edge */
    int nparams = 0;
    params[nparams++] = 0.0;
    params[nparams++] = 1.0;
    cuts[0] = (RelateCut) {0.0, RELATE_CUT_ROUNDED, NULL};
    cuts[1] = (RelateCut) {1.0, RELATE_CUT_ROUNDED, NULL};
    /* The query is grown by the widest tolerance the array asks for, which is
     * what makes it admit every edge the walk would have solved against */
    int ncand = nall;
    if (re.index)
    {
      double pad = fmax(re.tol, e->tol);
      ncand = edge_index_query(re.index, re.edges, e->xmin - pad,
        e->xmax + pad, e->ymin - pad, e->ymax + pad, candidates);
    }
    for (int c = 0; c < ncand; c++)
    {
      int j = candidates ? (int) INDEX_RESULT_ID_N(candidates, c) : c;
      const Edge *other = edges[j];
      if (j == i || ! relate_area_boundary_edge(other))
        continue;
      double ix[2], iy[2];
      bool overlap;
      int n = relate_area_edge_intersection(e, other, ix, iy, &overlap);
      if (overlap)
      {
        /* The two boundary edges run along one another. What splits this one
         * is where the other one starts and ends, not the two points an
         * intersection reports for a pair that merely crosses */
        if (relate_point_on_edge(other->x1, other->y1, e, false))
          relate_union_add_cut(relate_area_edge_parameter(e, other->x1,
            other->y1), RELATE_CUT_VERTEX1, other, params, cuts, &nparams,
            maxparams);
        if (relate_point_on_edge(other->x2, other->y2, e, false))
          relate_union_add_cut(relate_area_edge_parameter(e, other->x2,
            other->y2), RELATE_CUT_VERTEX2, other, params, cuts, &nparams,
            maxparams);
        continue;
      }
      /* Two segments cross at one point, which the input segment of the
       * other one fixes; an arc's meeting is held rounded */
      RelateCutKind kind = (n == 1 && e->etype == EDGE_POLYSEG &&
        other->etype == EDGE_POLYSEG && ! other->cut) ?
        RELATE_CUT_CROSSING : RELATE_CUT_ROUNDED;
      for (int k = 0; k < n; k++)
        relate_union_add_cut(relate_area_edge_parameter(e, ix[k], iy[k]),
          kind, other, params, cuts, &nparams, maxparams);
    }
    qsort(cuts, nparams, sizeof(RelateCut), relate_cut_cmp);

    for (int k = 0; k < nparams - 1; k++)
    {
      double t0 = cuts[k].t, t1 = cuts[k + 1].t;
      if (t1 - t0 <= MEOS_GEOM_TOLERANCE)
        continue;
      if (relate_portion_inside_union(e, (t0 + t1) * 0.5, comps, ncomp))
        continue;
      /* The portion lies on the boundary of the union, so it is kept. An end
       * of the edge is the input vertex itself; any other end is constructed
       * on the edge and rounded once */
      Edge piece = *e;
      piece.crossed[0] = piece.crossed[1] = false;
      if (t0 > 0.0)
        relate_union_piece_end(e, &cuts[k], 0, &piece);
      if (t1 < 1.0)
        relate_union_piece_end(e, &cuts[k + 1], 1, &piece);
      if (piece.etype == EDGE_POLYARC)
      {
        piece.theta0 = atan2(piece.y1 - piece.cy, piece.x1 - piece.cx);
        piece.theta1 = atan2(piece.y2 - piece.cy, piece.x2 - piece.cx);
        arc_set_bbox(&piece);
      }
      else
      {
        /* A constructed end falls off the line of the input segment, which
         * the piece keeps, so that whether it runs along a segment of another
         * geometry is read on that line (#relate_segments_intersect) */
        if ((t0 > 0.0 || t1 < 1.0) && ! e->cut)
        {
          piece.sx = e->x1; piece.sy = e->y1;
          piece.ex = e->x2; piece.ey = e->y2;
          piece.cut = true;
        }
        piece.dx = piece.x2 - piece.x1;
        piece.dy = piece.y2 - piece.y1;
        piece.length = piece.dx * piece.dx + piece.dy * piece.dy;
        piece.xmin = Min(piece.x1, piece.x2);
        piece.xmax = Max(piece.x1, piece.x2);
        piece.ymin = Min(piece.y1, piece.y2);
        piece.ymax = Max(piece.y1, piece.y2);
      }
      /* A portion two components bound alike is bounded once by the union */
      bool seen = false;
      for (uint32_t q = 0; q < result->count && ! seen; q++)
        seen = relate_same_portion((Edge *) meos_array_get_intl(result, q),
          &piece);
      if (! seen)
        meos_array_add(result, &piece);
    }
  }
  pfree(params);
  pfree(cuts);
  if (candidates)
    meos_array_destroy(candidates);
  relate_edges_clear(&re);
  pfree(edges);

  relate_comps_free(comps, ncomp);
  return result;
}

/**
 * @brief Return true if the interior and boundary of a geometry are those of
 * the union of its members rather than of the members themselves
 * @details Every geometry answers with its own edges, except a value holding
 * several components, whose topology is that of their union. Two members of a
 * multipolygon may share a boundary edge -- edge-adjacent polygons are a valid
 * multipolygon -- and that edge lies in the interior of what they cover
 * together, so reading the members' own edges reports it as boundary
 */
static bool
relate_reads_union(const LWGEOM *geom)
{
  switch (geom->type)
  {
    case MULTIPOLYGONTYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    case COLLECTIONTYPE:
      return true;
    default:
      return false;
  }
}

/**
 * @brief Return the edges a DE-9IM cell is computed on: those of the union of
 * the members where #relate_reads_union says so, and a geometry's own otherwise
 */
static MeosArray *
relate_extract_edges(const LWGEOM *geom)
{
  MeosArray *own = geom_extract_edges(geom);
  if (! relate_reads_union(geom))
    return own;
  MeosArray *result = relate_union_edges(geom, own);
  if (result != own)
    meos_array_destroy(own);
  return result;
}

/*****************************************************************************
 * Main dispatcher
 *****************************************************************************/

/**
 * @brief Return the bitmask of the topological dimensions occurring in a
 * geometry
 * @details Bit @p k is set when the geometry has a component of dimension
 * @p k, so a collection mixing dimensions sets more than one bit. Testing
 * each dimension with a separate predicate cannot distinguish such a
 * collection from a homogeneous one, and routes it to whichever predicate is
 * tried first
 */
static int
relate_dim_mask(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return 0;
  switch (geom->type)
  {
    case POINTTYPE:
    case MULTIPOINTTYPE:
      return 1;
    case LINETYPE:
    case MULTILINETYPE:
    case CIRCSTRINGTYPE:
    case COMPOUNDTYPE:
    case MULTICURVETYPE:
      /* A curve whose vertices all coincide draws a point, so the dimension
       * follows what it draws, as it does for a ring enclosing no area */
      return relate_linear_draws_points(geom) ? 1 : 2;
    case POLYGONTYPE:
      /* A ring enclosing no area bounds no region, so what the surface draws
       * is its own linework and the dimension follows what it draws */
      return ring_encloses_no_area(((const LWPOLY *) geom)->rings[0]) ? 2 : 4;
    case TRIANGLETYPE:
      return ring_encloses_no_area(((const LWTRIANGLE *) geom)->points) ?
        2 : 4;
    case CURVEPOLYTYPE:
    case MULTISURFACETYPE:
      return 4;
    case MULTIPOLYGONTYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    {
      /* A multipolygon, a TIN or a polyhedral surface carrying any surface is
       * areal: a member enclosing no area draws linework the surfaces already
       * account for, and reporting both dimensions would send a value every
       * member of which is a polygon down the mixed-dimension path.  A face of
       * a closed surface standing perpendicular to the plane is exactly such a
       * member, so this is the ordinary case for a watertight solid rather than
       * an edge case */
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      int mask = 0;
      for (uint32_t i = 0; i < col->ngeoms; i++)
        mask |= relate_dim_mask(col->geoms[i]);
      return (mask & 4) ? 4 : mask;
    }
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      int mask = 0;
      for (uint32_t i = 0; i < col->ngeoms; i++)
        mask |= relate_dim_mask(col->geoms[i]);
      return mask;
    }
    default:
      return 0;
  }
}

/**
 * @brief Return the topological dimension of the boundary of a geometry
 * @details Following the OGC rules a point geometry has an empty boundary,
 * an areal geometry has a one-dimensional boundary, and a linear geometry has
 * a zero-dimensional boundary that is empty when every component is closed,
 * as for a linear ring
 * @return The dimension, or -1 when the boundary is empty
 */
static int8_t
relate_boundary_dimension(const LWGEOM *geom)
{
  if (relate_is_areal(geom))
    return 1;
  if (! relate_is_linear(geom))
    return -1;
  MeosArray *arr = relate_extract_edges(geom);
  int nedges = (int) arr->count;
  Edge **edges = palloc(sizeof(Edge *) * (size_t) (nedges + 1));
  for (int i = 0; i < nedges; i++)
    edges[i] = (Edge *) meos_array_get_intl(arr, i);
  int count;
  POINT2D *points = relate_linear_boundary_points(edges, nedges, &count);
  pfree(points); pfree(edges); meos_array_destroy(arr);
  return (count > 0) ? 0 : -1;
}

/*****************************************************************************
 * A collection mixing dimensions
 *****************************************************************************/

/**
 * @brief Collect the components of a geometry having a given dimension
 */
static void
relate_stratum_iter(const LWGEOM *geom, int dim, LWGEOM **comps, int *ncomp)
{
  if (! geom || lwgeom_is_empty(geom))
    return;
  if (geom->type == COLLECTIONTYPE)
  {
    const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < col->ngeoms; i++)
      relate_stratum_iter(col->geoms[i], dim, comps, ncomp);
    return;
  }
  if (relate_dim_mask(geom) == (1 << dim))
    comps[(*ncomp)++] = (LWGEOM *) geom;
  return;
}

/**
 * @brief Return the parameter at which a point lies on an edge of any type
 * @details The area helpers read every edge that is not a straight boundary
 * as an arc, so a linear edge reaches them with no arc to read
 */
static double
relate_any_edge_parameter(const Edge *e, double x, double y)
{
  if (e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC)
    return relate_arc_parameter(e, x, y);
  double dx = e->x2 - e->x1, dy = e->y2 - e->y1;
  if (fabs(dx) >= fabs(dy))
    return fabs(dx) <= MEOS_GEOM_TOLERANCE ? 0.0 : (x - e->x1) / dx;
  return fabs(dy) <= MEOS_GEOM_TOLERANCE ? 0.0 : (y - e->y1) / dy;
}

/**
 * @brief Return the points at which two edges of any type meet
 * @details Where two edges meet is a question about their geometry and not
 * about which part of a geometry they bound, so each is read here as the
 * segment or the arc it is
 */
static int
relate_any_edge_intersection(const Edge *a, const Edge *b, double ix[2],
  double iy[2])
{
  Edge ea = *a, eb = *b;
  ea.etype = (a->etype == EDGE_POLYARC || a->etype == EDGE_LINEARC) ?
    EDGE_LINEARC : EDGE_LINESEG;
  eb.etype = (b->etype == EDGE_POLYARC || b->etype == EDGE_LINEARC) ?
    EDGE_LINEARC : EDGE_LINESEG;
  POINT2D out[2];
  int n = relate_linear_edge_points(&ea, &eb, out);
  if (n > 2)
    n = 2;
  for (int i = 0; i < n; i++)
  {
    ix[i] = out[i].x;
    iy[i] = out[i].y;
  }
  return n;
}

/**
 * @brief Return true if a component of a geometry lies wholly within another
 * component of a larger dimension
 * @details The interior of a collection is not the union of the interiors of
 * its components: a point placed on the boundary of a surface of the same
 * collection is on the boundary of their union, and a line drawn over a
 * surface adds nothing to it. Such a component is answered by the larger one
 * it lies in, so it is left out of its own stratum rather than counted twice
 */
static bool
relate_comp_covered(const LWGEOM *comp, const RelateComp *comps, int ncomp,
  Edge **ledges, int nledges)
{
  MeosArray *arr = geom_extract_edges(comp);
  int n = (int) arr->count;
  bool result = (n > 0);
  for (int i = 0; i < n && result; i++)
  {
    const Edge *e = (const Edge *) meos_array_get_intl(arr, i);
    if (e->etype == EDGE_POINT)
    {
      result = relate_in_area_union(e->x1, e->y1, comps, ncomp) ||
        (nledges > 0 && relate_point_in_linear(e->x1, e->y1, ledges,
          nledges, true) != 2);
      continue;
    }
    /* A linear edge is covered when every portion of it left by the areal
     * boundaries it meets lies within the surfaces */
    int maxparams = 2 * ncomp + 2;
    for (int c = 0; c < ncomp; c++)
      maxparams += 2 * comps[c].nedges;
    double *params = palloc(sizeof(double) * maxparams);
    int nparams = 0;
    params[nparams++] = 0.0;
    params[nparams++] = 1.0;
    for (int c = 0; c < ncomp; c++)
      for (int j = 0; j < comps[c].nedges; j++)
      {
        double ix[2], iy[2];
        int nint = relate_any_edge_intersection(e, comps[c].edges[j], ix, iy);
        for (int k = 0; k < nint; k++)
          relate_area_add_parameter(relate_any_edge_parameter(e, ix[k],
            iy[k]), params, &nparams, maxparams);
      }
    qsort(params, nparams, sizeof(double), relate_area_parameter_cmp);
    for (int k = 0; k < nparams - 1 && result; k++)
    {
      if (params[k + 1] - params[k] <= MEOS_GEOM_TOLERANCE)
        continue;
      double x, y;
      relate_edge_point(e, (params[k] + params[k + 1]) * 0.5, &x, &y);
      result = relate_in_area_union(x, y, comps, ncomp);
    }
    pfree(params);
  }
  meos_array_destroy(arr);
  return result;
}

/**
 * @brief Return the components of a geometry having a given dimension, as a
 * geometry of that dimension alone
 * @details The result borrows the components of its argument, so it is
 * released with #relate_stratum_free rather than with @p lwgeom_free
 * @return @p NULL when the geometry has no component of that dimension
 */
static LWGEOM *
relate_stratum(const LWGEOM *geom, int dim, int ncomps, const LWGEOM *area,
  const LWGEOM *line)
{
  LWGEOM **comps = palloc(sizeof(LWGEOM *) * ncomps);
  int ncomp = 0;
  relate_stratum_iter(geom, dim, comps, &ncomp);

  /* Leave out every component a stratum of a larger dimension already
   * answers for */
  if (ncomp > 0 && (area || line))
  {
    int nareal = 0, maxareal = 8;
    RelateComp *areal = palloc(sizeof(RelateComp) * maxareal);
    if (area)
      relate_area_comps_iter(area, &areal, &nareal, &maxareal, false);
    MeosArray *larr = line ? geom_extract_edges(line) : NULL;
    int nledges = larr ? (int) larr->count : 0;
    Edge **ledges = palloc(sizeof(Edge *) * Max(nledges, 1));
    for (int i = 0; i < nledges; i++)
      ledges[i] = (Edge *) meos_array_get_intl(larr, i);
    int nkept = 0;
    for (int i = 0; i < ncomp; i++)
      if (! relate_comp_covered(comps[i], areal, nareal, ledges, nledges))
        comps[nkept++] = comps[i];
    ncomp = nkept;
    pfree(ledges);
    if (larr)
      meos_array_destroy(larr);
    relate_comps_free(areal, nareal);
  }

  if (ncomp == 0)
  {
    pfree(comps);
    return NULL;
  }
  if (ncomp == 1)
  {
    LWGEOM *result = comps[0];
    pfree(comps);
    return result;
  }
  /* The components are gathered in a collection rather than in a
   * multi-geometry because they may share an edge or overlap, which a
   * multi-geometry excludes and a collection resolves */
  return (LWGEOM *) lwcollection_construct(COLLECTIONTYPE, geom->srid, NULL,
    (uint32_t) ncomp, comps);
}

/**
 * @brief Release a stratum without releasing the components it borrows
 */
static void
relate_stratum_free(LWGEOM *stratum, const LWGEOM *geom)
{
  if (! stratum || stratum == geom)
    return;
  if (stratum->type == COLLECTIONTYPE)
  {
    LWCOLLECTION *col = (LWCOLLECTION *) stratum;
    /* Only the collection built above is released: its components belong to
     * the geometry it was taken from */
    if (col->bbox)
      lwfree(col->bbox);
    lwfree(col->geoms);
    lwfree(col);
  }
  return;
}

/**
 * @brief Return the number of components of a geometry
 */
static int
relate_count_comps(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return 0;
  if (geom->type != COLLECTIONTYPE)
    return 1;
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  int result = 0;
  for (uint32_t i = 0; i < col->ngeoms; i++)
    result += relate_count_comps(col->geoms[i]);
  return result;
}

/**
 * @brief Compute the DE-9IM matrix of two geometries of one dimension each
 */
static void
relate_simple(const LWGEOM *g1, const LWGEOM *g2, int mask1, int mask2,
  const RelateOperands *ops, const RelateQuery *q, MeosDE9IM *m)
{
  if (mask1 == 1 && mask2 == 1)
    relate_point_point(g1, g2, m);
  else if (mask1 == 1 && mask2 == 2)
    relate_point_linear(g1, g2, ops, m);
  else if (mask1 == 2 && mask2 == 1)
    relate_linear_point(g1, g2, ops, m);
  else if (mask1 == 1 && mask2 == 4)
    relate_point_area(g1, g2, ops, m);
  else if (mask1 == 4 && mask2 == 1)
    relate_area_point(g1, g2, ops, m);
  else if (mask1 == 2 && mask2 == 2)
    relate_linear_linear(g1, g2, ops, m);
  else if (mask1 == 2 && mask2 == 4)
    relate_linear_area(g1, g2, ops, q, m);
  else if (mask1 == 4 && mask2 == 2)
    relate_area_linear(g1, g2, ops, q, m);
  else
    relate_area_area(g1, g2, ops, q, m);
  return;
}

/**
 * @brief Compute the DE-9IM matrix of two geometries, either of which may
 * mix dimensions
 * @details A collection mixing dimensions is answered from its strata, the
 * components of each dimension taken together, because every cell function
 * answers about one dimension. The interior of the geometry is the union of
 * the interiors of its strata, and so is its boundary, so a cell reading
 * either of those takes the @b largest dimension any stratum contributes.
 * Its exterior is instead the @b intersection of the exteriors of the strata,
 * a point being outside the geometry only when it is outside every stratum,
 * so a cell reading an exterior takes the smallest.
 */
static void
relate_dispatch(const LWGEOM *g1, const LWGEOM *g2, int mask1, int mask2,
  const RelateOperands *ops, const RelateQuery *q, MeosDE9IM *m)
{
  /* A single dimension on each side is answered directly */
  if ((mask1 & (mask1 - 1)) == 0 && (mask2 & (mask2 - 1)) == 0)
  {
    relate_simple(g1, g2, mask1, mask2, ops, q, m);
    return;
  }

  int ncomp1 = relate_count_comps(g1), ncomp2 = relate_count_comps(g2);
  LWGEOM *s1[3] = {NULL, NULL, NULL}, *s2[3] = {NULL, NULL, NULL};
  MeosDE9IM cells[3][3];
  bool present[3][3];
  /* Built from the largest dimension down, each stratum leaving out what the
   * larger ones already answer for */
  for (int j = 2; j >= 0; j--)
  {
    s1[j] = (mask1 & (1 << j)) ?
      relate_stratum(g1, j, ncomp1, s1[2], j == 0 ? s1[1] : NULL) : NULL;
    s2[j] = (mask2 & (1 << j)) ?
      relate_stratum(g2, j, ncomp2, s2[2], j == 0 ? s2[1] : NULL) : NULL;
  }
  /* Leaving a stratum out may leave a single dimension on a side */
  mask1 = (s1[0] ? 1 : 0) | (s1[1] ? 2 : 0) | (s1[2] ? 4 : 0);
  mask2 = (s2[0] ? 1 : 0) | (s2[1] ? 2 : 0) | (s2[2] ? 4 : 0);
  for (int j = 0; j < 3; j++)
    for (int k = 0; k < 3; k++)
    {
      present[j][k] = (s1[j] && s2[k]);
      if (present[j][k])
      {
        de9im_init(&cells[j][k]);
        /* The strata merge their interior and boundary cells by the largest
         * dimension and their exterior cells by the smallest, so a stratum
         * may stop early only for a query reading no exterior: a cell it
         * leaves short then lowers no merged cell a pattern reads */
        relate_simple(s1[j], s2[k], 1 << j, 1 << k, ops,
          relate_query_reads_no_exterior(q) ? q : NULL, &cells[j][k]);
      }
    }

  /* A cell reading two interiors or two boundaries is the largest any pair
   * of strata contributes */
  for (int j = 0; j < 3; j++)
    for (int k = 0; k < 3; k++)
    {
      if (! present[j][k])
        continue;
      de9im_add(&m->ii, cells[j][k].ii);
      de9im_add(&m->ib, cells[j][k].ib);
      de9im_add(&m->bi, cells[j][k].bi);
      de9im_add(&m->bb, cells[j][k].bb);
    }

  /* A cell reading the exterior of the second geometry is bounded by every
   * stratum of it, so the smallest over those strata answers, and the
   * largest over the strata of the first geometry then answers for it */
  for (int j = 0; j < 3; j++)
  {
    if (! s1[j])
      continue;
    int8_t ie = 2, be = 2;
    for (int k = 0; k < 3; k++)
    {
      if (! present[j][k])
        continue;
      if (cells[j][k].ie < ie)
        ie = cells[j][k].ie;
      if (cells[j][k].be < be)
        be = cells[j][k].be;
    }
    de9im_add(&m->ie, ie);
    de9im_add(&m->be, be);
  }
  for (int k = 0; k < 3; k++)
  {
    if (! s2[k])
      continue;
    int8_t ei = 2, eb = 2;
    for (int j = 0; j < 3; j++)
    {
      if (! present[j][k])
        continue;
      if (cells[j][k].ei < ei)
        ei = cells[j][k].ei;
      if (cells[j][k].eb < eb)
        eb = cells[j][k].eb;
    }
    de9im_add(&m->ei, ei);
    de9im_add(&m->eb, eb);
  }
  de9im_add(&m->ee, 2);

  for (int j = 0; j < 3; j++)
  {
    relate_stratum_free(s1[j], g1);
    relate_stratum_free(s2[j], g2);
  }
  return;
}

/**
 * @brief Set the cells of the DE-9IM matrix of two geometries that share no
 * point
 * @details Neither geometry meets the other, so the interior and the boundary
 * of each fall entirely in the exterior of the other, each keeping its own
 * dimension, and the two exteriors meet in dimension 2. An empty geometry
 * contributes nothing. Only the cells the query reads are set, and the two
 * boundary cells last and only where the others leave the query undecided: the
 * dimension of the boundary of a linear geometry reads the parity of all its
 * endpoints, which a query answered by the interiors does not pay
 * @param[in] g1,g2 Geometries
 * @param[in] q Query, NULL for the whole matrix @nullable
 * @param[in,out] m Matrix, initialized by the caller
 */
static void
relate_matrix_apart(const LWGEOM *g1, const LWGEOM *g2, const RelateQuery *q,
  MeosDE9IM *m)
{
  int want = relate_query_cells(q);
  bool empty1 = lwgeom_is_empty(g1), empty2 = lwgeom_is_empty(g2);
  de9im_add(&m->ee, 2);
  if (! empty2 && (want & DE9IM_EI))
    de9im_add(&m->ei, (int8_t) relate_dimension(g2));
  if (! empty1 && (want & DE9IM_IE))
    de9im_add(&m->ie, (int8_t) relate_dimension(g1));
  if (relate_query_decided(q, m))
    return;
  /* Every cell but the two boundary ones is final here, so a query none of
   * whose patterns can match whatever those two hold is answered already */
  if (q)
  {
    char matrix[10];
    de9im_to_string(m, matrix);
    bool possible = false;
    for (int k = 0; k < q->count && ! possible; k++)
    {
      char pattern[10];
      memcpy(pattern, q->pattern[k], 10);
      pattern[5] = pattern[7] = '*';
      possible = de9im_match(matrix, pattern);
    }
    if (! possible)
      return;
  }
  if (! empty2 && (want & DE9IM_EB))
    de9im_add(&m->eb, relate_boundary_dimension(g2));
  if (! empty1 && (want & DE9IM_BE))
    de9im_add(&m->be, relate_boundary_dimension(g1));
  return;
}

/**
 * @brief Return true if the extents of two geometries lie apart by more than
 * any tolerance the engine reads a meeting within
 * @details The extent of a geometry drawn with straight edges is the minimum
 * and maximum of its input coordinates, which are exact. The engine reads two
 * points within the MEOS tolerance, and a point within the rounding of an
 * edge's own coordinates, as meeting, so the extents are compared with a
 * margin a thousand times the larger of the two: a pair the test separates is
 * a pair the engine reads as sharing no point. An arc is read on its own
 * circle within a tolerance that grows with its radius, which its extent does
 * not bound, so a geometry carrying one is never separated here
 * @param[in] g1,g2 Geometries, neither empty
 */
static bool
relate_extents_apart(const LWGEOM *g1, const LWGEOM *g2)
{
  if (lwgeom_has_arc(g1) || lwgeom_has_arc(g2))
    return false;
  /* The matrix of two geometries apart reads the dimension of each from
   * #relate_dimension, while the kernels read it from what the geometry draws
   * (#relate_dim_mask): a ring enclosing no area is a surface to the first and
   * the linework it traces to the second. Only a geometry both read alike is
   * answered here */
  const LWGEOM *g[2] = {g1, g2};
  for (int k = 0; k < 2; k++)
  {
    int mask = relate_dim_mask(g[k]);
    int drawn = (mask & 4) ? 2 : (mask & 2) ? 1 : (mask & 1) ? 0 : -1;
    if (drawn != relate_dimension(g[k]))
      return false;
  }
  GBOX box1, box2;
  if (lwgeom_calculate_gbox_cartesian(g1, &box1) != LW_SUCCESS ||
      lwgeom_calculate_gbox_cartesian(g2, &box2) != LW_SUCCESS)
    return false;
  double extent = fmax(fmax(fabs(box1.xmin), fabs(box1.xmax)),
    fmax(fabs(box1.ymin), fabs(box1.ymax)));
  extent = fmax(extent, fmax(fmax(fabs(box2.xmin), fabs(box2.xmax)),
    fmax(fabs(box2.ymin), fabs(box2.ymax))));
  double margin = 1000.0 * coordinate_tolerance(extent, extent);
  return box1.xmax + margin < box2.xmin || box2.xmax + margin < box1.xmin ||
    box1.ymax + margin < box2.ymin || box2.ymax + margin < box1.ymin;
}

/**
 * @brief Compute the DE-9IM intersection matrix, reading the edges a
 * relationship has already extracted where it has any
 * @param[in] g1,g2 Geometries
 * @param[in] ops Operands of the relationship the matrix answers a step of,
 * NULL where the matrix is asked for on its own
 * @param[in] q Query the matrix answers, NULL for the whole matrix @nullable
 * @param[in] meet True where the caller has found that the geometries share a
 * point, which settles that neither is empty and that their extents are not
 * apart
 * @param[out] result The matrix, which matches every pattern of @p q exactly
 * as the whole matrix does; where @p q is NULL it is the whole matrix
 * @return true if the geometry pair is supported, which is what
 * #geom_meos_coverage answers 1 for each geometry
 */
static bool
relate_matrix(const LWGEOM *g1, const LWGEOM *g2, const RelateOperands *ops,
  const RelateQuery *q, bool meet, char result[10])
{
  assert(g1); assert(g2); assert(result);

  /* Every native implementation reads the same predicate: the engine answers
   * a geometry the edge decomposition reaches, and nothing else */
  if (geom_meos_coverage(g1) != 1 || geom_meos_coverage(g2) != 1)
    return false;

  MeosDE9IM m;
  de9im_init(&m);

  /* An empty operand meets nothing, and neither do two geometries whose
   * extents lie apart, which a handful of comparisons settles before a single
   * edge is extracted. Two geometries sharing a point are within the band of
   * one another, a thousandth of the margin the extents are compared by, so a
   * caller that has found them meeting skips the test */
  if (! meet && (lwgeom_is_empty(g1) || lwgeom_is_empty(g2) ||
      relate_extents_apart(g1, g2)))
  {
    relate_matrix_apart(g1, g2, q, &m);
    de9im_to_string(&m, result);
    return true;
  }

  int mask1 = relate_dim_mask(g1);
  int mask2 = relate_dim_mask(g2);
  relate_dispatch(g1, g2, mask1, mask2, ops, q, &m);

  de9im_to_string(&m, result);
  return true;
}

/**
 * @brief Compute the DE-9IM intersection matrix
 * @details This is the native counterpart of PostGIS @p ST_Relate
 * @return true if the geometry pair is supported, which is what
 * #geom_meos_coverage answers 1 for each geometry. A false return means the
 * pair is outside that coverage, @b not that the geometries are unrelated, so
 * a caller must answer it another way rather than read @p result
 */
bool
meos_relate(const LWGEOM *g1, const LWGEOM *g2, char result[10])
{
  return relate_matrix(g1, g2, NULL, NULL, false, result);
}

/**
 * @brief Return true if a point lies on the curve an edge draws, whatever
 * part of a geometry that edge bounds
 * @details #relate_point_on_boundary answers the same question of an areal
 * boundary alone, and a shared point is a question about the curves. An input
 * vertex lies on a point, a segment or the circle of an arc exactly where its
 * coordinates say so (#relate_points_equal, #point_on_segment_exact,
 * #point_on_arc_circle), and a constructed point
 * within the rounding of its coordinates. The ends of an edge that another one
 * runs along are read with the rounding the overlap was found with, as a
 * constructed point is
 * @param[in] x,y Coordinates of the point
 * @param[in] e Edge
 * @param[in] vertex True if the point is an input vertex
 */
static bool
relate_point_on_edge(double x, double y, const Edge *e, bool vertex)
{
  switch (e->etype)
  {
    case EDGE_POINT:
      return relate_points_equal(x, y, e->x1, e->y1, vertex);
    case EDGE_LINEARC:
    case EDGE_POLYARC:
      /* Where on the circle an input vertex stands, the span decides */
      return vertex ?
        (point_on_arc_circle(e, x, y) &&
          arc_contains_angle(e, atan2(y - e->cy, x - e->cx))) :
        point_on_arc(x, y, e);
    case EDGE_LINESEG:
    case EDGE_POLYSEG:
      return vertex ?
        point_on_segment_exact(x, y, e->x1, e->y1, e->x2, e->y2) :
        point_on_segment(x, y, e->x1, e->y1, e->x2, e->y2);
  }
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Unknown edge type: %d", e->etype);
  return false;
}

/**
 * @brief Return the bounding box of what an edge array draws
 */
static void
relate_edges_box(Edge **edges, int nedges, double box[4])
{
  box[0] = box[1] = DBL_MAX;
  box[2] = box[3] = -DBL_MAX;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->xmin < box[0]) box[0] = e->xmin;
    if (e->ymin < box[1]) box[1] = e->ymin;
    if (e->xmax > box[2]) box[2] = e->xmax;
    if (e->ymax > box[3]) box[3] = e->ymax;
  }
  return;
}

/**
 * @brief Return true if the bounding boxes of two edge arrays overlap
 * @details Folding the edge boxes of each array costs a pass over each, where
 * the pair of loops it stands in front of costs a pass over one for every
 * edge of the other
 */
static bool
relate_edges_boxes_overlap(Edge **e1, int n1, Edge **e2, int n2)
{
  if (n1 <= 0 || n2 <= 0)
    return false;
  double b1[4], b2[4];
  relate_edges_box(e1, n1, b1);
  relate_edges_box(e2, n2, b2);
  return ! (b1[2] < b2[0] - MEOS_GEOM_TOLERANCE ||
            b2[2] < b1[0] - MEOS_GEOM_TOLERANCE ||
            b1[3] < b2[1] - MEOS_GEOM_TOLERANCE ||
            b2[3] < b1[1] - MEOS_GEOM_TOLERANCE);
}

/**
 * @brief Return true if the curves two edges draw meet
 * @details #relate_any_edge_intersection answers the ISOLATED points at which
 * they meet, which is nothing where the two run along each other: two equal
 * segments and two arcs of one circle sharing a span both report no point.
 * Meeting is the weaker question, so it reads the intersection type rather
 * than a count
 */
static bool
relate_edges_meet(const Edge *a, const Edge *b)
{
  Edge ea = *a, eb = *b;
  ea.etype = (a->etype == EDGE_POLYARC || a->etype == EDGE_LINEARC) ?
    EDGE_LINEARC : EDGE_LINESEG;
  eb.etype = (b->etype == EDGE_POLYARC || b->etype == EDGE_LINEARC) ?
    EDGE_LINEARC : EDGE_LINESEG;
  if (ea.etype == EDGE_LINESEG && eb.etype == EDGE_LINESEG)
  {
    IntersectResult r = linesegm_intersect(ea.x1, ea.y1, ea.x2, ea.y2,
      eb.x1, eb.y1, eb.x2, eb.y2);
    return r.type != INTERSECT_NONE;
  }
  if (ea.etype == EDGE_LINEARC && eb.etype == EDGE_LINEARC)
  {
    double x[2], y[2];
    bool overlap = false;
    int n = relate_arc_arc_points(&ea, &eb, x, y, &overlap);
    return n > 0 || overlap;
  }
  /* A straight segment and a circular arc share no curve, so the isolated
   * points answer the whole of it */
  double ix[2], iy[2];
  return relate_any_edge_intersection(a, b, ix, iy) > 0;
}

/**
 * @brief Return true if an edge draws a single point
 * @details A straight segment whose ends coincide draws the one point they
 * denote, so it meets another geometry exactly where that point does. The
 * segment kernel answers about positions along its first segment and a
 * segment of no length has none, so the point is what is asked about
 */
static bool
relate_edge_is_point(const Edge *e)
{
  if (e->etype == EDGE_POINT)
    return true;
  return (e->etype == EDGE_LINESEG || e->etype == EDGE_POLYSEG) &&
    e->x1 == e->x2 && e->y1 == e->y2;
}

/**
 * @brief Return true if one point of an edge lies inside the surfaces bounded
 * by another geometry's edges
 * @details Read only where no curve of either geometry meets a curve of the
 * other, which is what makes one point answer for the whole edge: an edge
 * crossing into a surface would have met its boundary. #point_in_polygon
 * passes over every edge that bounds no surface, so the array it reads may
 * mix dimensions
 */
static bool
relate_edge_inside_area(const Edge *e, const RelateEdges *other)
{
  /* The point read lies in the box of the edge, and a point apart from the
   * extent of the other geometry lies inside none of its surfaces */
  if (relate_edges_apart(other, e->xmin, e->xmax, e->ymin, e->ymax, 0.0))
    return false;
  double x, y;
  if (e->etype == EDGE_POINT)
  {
    x = e->x1;
    y = e->y1;
  }
  else
    relate_edge_point(e, 0.5, &x, &y);
  /* A point edge, and a segment of no length, whose middle is its vertex, is
   * an input vertex, which lies in a surface exactly where its coordinates
   * say so (#point_in_polygon_vertex); the middle of any other edge is a
   * constructed point */
  bool vertex = relate_edge_is_point(e);
  if (! other->index)
    return (vertex ?
      point_in_polygon_vertex(x, y, other->edges, other->nedges) :
      point_in_polygon(x, y, other->edges, other->nedges)) != 0;
  return point_in_polygon_index_into(x, y, other->edges, other->nedges,
    other->index, other->xmax, other->reach, other->results, vertex) != 0;
}

/**
 * @brief Return the edges of an array whose box can meet a given box, read
 * out of the array's index
 * @details The query is grown by the widest tolerance the array asks for, so
 * every edge the unindexed scan tests is among the ones it answers
 */
static int
relate_edges_candidates(const RelateEdges *re, double xmin, double xmax,
  double ymin, double ymax, MeosArray *candidates)
{
  return edge_index_query(re->index, re->edges, xmin - re->tol,
    xmax + re->tol, ymin - re->tol, ymax + re->tol, candidates);
}

/**
 * @brief Return true if a curve meets any curve of an edge array, reading the
 * edges whose box can meet its own out of the array's index
 */
static bool
relate_edges_meet_any(const Edge *a, const RelateEdges *other)
{
  /* The scan rejects a pair by the MEOS tolerance and the index by the widest
   * one the array asks for, so a curve apart from the extent by both meets
   * none of the curves either would solve */
  if (relate_edges_apart(other, a->xmin, a->xmax, a->ymin, a->ymax,
        fmax(other->tol, MEOS_GEOM_TOLERANCE)))
    return false;
  if (! other->index)
  {
    for (int j = 0; j < other->nedges; j++)
    {
      const Edge *b = other->edges[j];
      if (b->etype == EDGE_POINT)
        continue;
      if (a->xmax < b->xmin - MEOS_GEOM_TOLERANCE ||
          b->xmax < a->xmin - MEOS_GEOM_TOLERANCE ||
          a->ymax < b->ymin - MEOS_GEOM_TOLERANCE ||
          b->ymax < a->ymin - MEOS_GEOM_TOLERANCE)
        continue;
      if (relate_edges_meet(a, b))
        return true;
    }
    return false;
  }
  MeosArray *candidates = index_result_create();
  int nc = relate_edges_candidates(other, a->xmin, a->xmax, a->ymin, a->ymax,
    candidates);
  bool result = false;
  for (int c = 0; c < nc && ! result; c++)
  {
    const Edge *b = other->edges[INDEX_RESULT_ID_N(candidates, c)];
    if (b->etype == EDGE_POINT)
      continue;
    /* The index query is grown by the widest tolerance the array asks for,
     * which is wider than the bound the scan rejects a pair by. Applying the
     * scan's own test to what the index answers is what keeps the pairs
     * solved here the pairs the scan solves, so the answer does not depend on
     * whether the index was built */
    if (a->xmax < b->xmin - MEOS_GEOM_TOLERANCE ||
        b->xmax < a->xmin - MEOS_GEOM_TOLERANCE ||
        a->ymax < b->ymin - MEOS_GEOM_TOLERANCE ||
        b->ymax < a->ymin - MEOS_GEOM_TOLERANCE)
      continue;
    result = relate_edges_meet(a, b);
  }
  meos_array_destroy(candidates);
  return result;
}

/**
 * @brief Return true if an input vertex stands on any edge of an array,
 * reading the edges that can carry it out of the array's index
 */
static bool
relate_point_on_any_edge(double x, double y, const RelateEdges *other)
{
  if (! other->index)
  {
    for (int j = 0; j < other->nedges; j++)
      if (relate_point_on_edge(x, y, other->edges[j], true))
        return true;
    return false;
  }
  MeosArray *candidates = index_result_create();
  int nc = relate_edges_candidates(other, x, x, y, y, candidates);
  bool result = false;
  for (int c = 0; c < nc && ! result; c++)
    result = relate_point_on_edge(x, y,
      other->edges[INDEX_RESULT_ID_N(candidates, c)], true);
  meos_array_destroy(candidates);
  return result;
}

/**
 * @brief Answer the first two of the three questions of whether two
 * geometries share a point, read from their edges
 * @details Sharing a point is the pattern `FF*FF****` failing, and the matrix
 * is not needed to decide it. Three questions answer it, each stopping at the
 * first witness it finds: a point of one geometry standing on a curve or a
 * point of the other; a curve of one meeting a curve of the other; and, once
 * no curves meet at all, an edge lying within a surface of the other, where
 * one point of that edge answers for the whole of it. Each question reads the
 * other array through its index where one is built: stopping at the first
 * witness is what makes a pair that DOES meet cheap, but a pair that does not
 * holds no witness to stop at, so every question runs to the end and costs the
 * product of the two arrays. That is the case the index removes, and it is the
 * common one, a pair whose boxes overlap while the geometries keep apart.
 *
 * The first two questions read each geometry's OWN edges: a point of the union
 * of a collection's members is a point of one of them, so where the geometries
 * meet does not depend on the union, and the own edges end at the input
 * vertices where the union's are cut at constructed points. The third reads
 * the edges of the union, which bound each stretch of a surface once, as the
 * parity locating a point needs
 * @param[in] re1,re2 The geometries' own edges
 * @return 1 where they share a point, -1 where their bounding boxes stand
 * apart so that they share none, and 0 where no curve of either meets the
 * other, which leaves the third question to #relate_edges_within
 */
static int
relate_edges_share_point(const RelateEdges *re1, const RelateEdges *re2)
{
  Edge **e1 = re1->edges, **e2 = re2->edges;
  int n1 = re1->nedges, n2 = re2->nedges;

  /* Geometries whose bounding boxes stand apart share no point, and reading
   * that first is what keeps a pair that does not meet from costing a pass
   * over every edge of one against every edge of the other */
  if (! relate_edges_boxes_overlap(e1, n1, e2, n2))
    return -1;

  /* A point of one geometry standing on the other, a segment of no length
   * being the point it draws */
  for (int i = 0; i < n1; i++)
  {
    if (! relate_edge_is_point(e1[i]))
      continue;
    if (relate_point_on_any_edge(e1[i]->x1, e1[i]->y1, re2))
      return 1;
  }
  for (int j = 0; j < n2; j++)
  {
    if (! relate_edge_is_point(e2[j]))
      continue;
    if (relate_point_on_any_edge(e2[j]->x1, e2[j]->y1, re1))
      return 1;
  }

  /* Two curves meeting, the bounding boxes deciding which pairs are worth
   * solving */
  for (int i = 0; i < n1; i++)
  {
    if (relate_edge_is_point(e1[i]))
      continue;
    if (relate_edges_meet_any(e1[i], re2))
      return 1;
  }
  return 0;
}

/**
 * @brief Return true if an edge of one geometry lies within a surface of the
 * other, for two geometries no curve of which meets the other
 * @details Each edge then lies wholly inside or wholly outside the surfaces of
 * the other geometry, so one point of it answers for the whole
 * @param[in] re1,re2 The geometries' own edges
 * @param[in] area1,area2 The edges their surfaces are read from, those of
 * #relate_extract_edges
 */
static bool
relate_edges_within(const RelateEdges *re1, const RelateEdges *re2,
  const RelateEdges *area1, const RelateEdges *area2)
{
  for (int i = 0; i < re1->nedges; i++)
    if (relate_edge_inside_area(re1->edges[i], area2))
      return true;
  for (int j = 0; j < re2->nedges; j++)
    if (relate_edge_inside_area(re2->edges[j], area1))
      return true;
  return false;
}

/**
 * @brief Return true if a point lies in the point set an edge array draws
 * @details That is, inside a surface the array bounds, or on one of its
 * curves or points. #point_in_polygon passes over every edge bounding no
 * surface, so an array mixing dimensions is read correctly
 * @param[in] x,y Coordinates of the point
 * @param[in] re Edge array, read through its index where it carries one
 * @param[in] vertex True if the point is an input vertex (#relate_point_on_edge)
 */
static bool
relate_point_in_edges(double x, double y, const RelateEdges *re, bool vertex)
{
  if (! re->index)
  {
    if (point_in_polygon(x, y, re->edges, re->nedges) != 0)
      return true;
    for (int i = 0; i < re->nedges; i++)
      if (relate_point_on_edge(x, y, re->edges[i], vertex))
        return true;
    return false;
  }
  if (point_in_polygon_index_into(x, y, re->edges, re->nedges, re->index,
        re->xmax, re->reach, re->results, false))
    return true;
  int nc = relate_edges_candidates(re, x, x, y, y, re->results);
  for (int c = 0; c < nc; c++)
    if (relate_point_on_edge(x, y, re->edges[INDEX_RESULT_ID_N(re->results, c)],
          vertex))
      return true;
  return false;
}

/**
 * @brief Return true if every point an edge array draws lies in another one
 * @details Each edge is split where the other geometry's edges meet it, which
 * leaves every piece wholly inside or wholly outside, so one point of a piece
 * answers for the piece. The parameters an edge is split at come from the
 * isolated intersection points and from the ends of the other edges that lie
 * on it, the second of which is what bounds a stretch two edges run along
 * together, where there is no isolated point to find
 * ⛔ The edges of an areal geometry draw its BOUNDARY, not its interior, so a
 * false answer is conclusive while a true one is not: a hole of the first
 * geometry lying within the second is a piece of the second outside the first
 * that no edge of the second visits. The caller reads it as a rejection only
 * The edges of the first geometry an edge of the second can meet are those
 * whose box meets its own, which the index of the first answers where it
 * carries one, as it answers the location of each piece
 * @param[in] r1 Edges of the first geometry
 * @param[in] e2,n2 Edges of the second geometry
 * @return False if @p e2 draws a point outside what @p r1 draws, which
 * settles that the second geometry is not covered by the first
 */
static bool
relate_edges_cover(const RelateEdges *r1, Edge **e2, int n2)
{
  Edge **e1 = r1->edges;
  int n1 = r1->nedges;
  /* Nothing lies within a geometry whose box it stands outside of */
  if (! relate_edges_boxes_overlap(e1, n1, e2, n2))
    return false;

  int maxparams = 2 + 4 * n1;
  double *params = palloc(sizeof(double) * maxparams);
  bool result = true;
  for (int j = 0; j < n2 && result; j++)
  {
    const Edge *e = e2[j];
    if (e->etype == EDGE_POINT)
    {
      result = relate_point_in_edges(e->x1, e->y1, r1, true);
      continue;
    }
    int nparams = 0;
    params[nparams++] = 0.0;
    params[nparams++] = 1.0;
    /* The index query is grown by the widest tolerance the array asks for,
     * which is wider than the bound the scan rejects a pair by, and the scan's
     * own test is applied to what it answers, so the pairs solved are those
     * the scan solves. The parameters are sorted below, so the order the
     * index answers them in does not reach the result */
    int ncand = r1->index ? relate_edges_candidates(r1, e->xmin, e->xmax,
      e->ymin, e->ymax, r1->results) : n1;
    for (int c = 0; c < ncand; c++)
    {
      const Edge *o = r1->index ?
        e1[INDEX_RESULT_ID_N(r1->results, c)] : e1[c];
      if (e->xmax < o->xmin - MEOS_GEOM_TOLERANCE ||
          o->xmax < e->xmin - MEOS_GEOM_TOLERANCE ||
          e->ymax < o->ymin - MEOS_GEOM_TOLERANCE ||
          o->ymax < e->ymin - MEOS_GEOM_TOLERANCE)
        continue;
      double ix[2], iy[2];
      int nint = relate_any_edge_intersection(e, o, ix, iy);
      for (int k = 0; k < nint; k++)
        relate_area_add_parameter(relate_any_edge_parameter(e, ix[k], iy[k]),
          params, &nparams, maxparams);
      /* Where the two run along each other no isolated point is found, and
       * the stretch they share ends at an end of the other edge */
      if (o->etype != EDGE_POINT)
      {
        if (relate_point_on_edge(o->x1, o->y1, e, false))
          relate_area_add_parameter(relate_any_edge_parameter(e, o->x1, o->y1),
            params, &nparams, maxparams);
        if (relate_point_on_edge(o->x2, o->y2, e, false))
          relate_area_add_parameter(relate_any_edge_parameter(e, o->x2, o->y2),
            params, &nparams, maxparams);
      }
    }
    qsort(params, nparams, sizeof(double), relate_area_parameter_cmp);
    for (int k = 0; k < nparams - 1 && result; k++)
    {
      if (params[k + 1] - params[k] <= MEOS_GEOM_TOLERANCE)
        continue;
      double x, y;
      relate_edge_point(e, (params[k] + params[k + 1]) * 0.5, &x, &y);
      result = relate_point_in_edges(x, y, r1, false);
    }
  }
  pfree(params);
  return result;
}

/**
 * @brief Return the pointers into an edge array that #relate_edges_init reads
 * @details The edges lie one after another in the array's own storage, so
 * they are read in place rather than through #meos_array_get, which checks
 * every index against the count
 */
static Edge **
relate_edge_pointers(const MeosArray *arr)
{
  assert(arr->elem_size == sizeof(Edge) && ! arr->varlength);
  int n = (int) arr->count;
  Edge **result = palloc(sizeof(Edge *) * (n ? n : 1));
  Edge *edges = (Edge *) arr->elems;
  for (int i = 0; i < n; i++)
    result[i] = &edges[i];
  return result;
}

/**
 * @brief Return true if a point lies in or on a geometry of straight edges,
 * read on its input coordinates
 * @details Each member is read alone, so members that overlap need no union:
 * a point in or on any member is a point of the geometry, as GEOS locates a
 * point against the polygons of a collection one by one
 * (`RelatePointLocator::locateOnPolygons`). A curve is not read, and a point
 * it alone would hold answers false, which a caller reads as nothing known
 */
static bool
relate_geom_holds_vertex(const LWGEOM *g, double x, double y)
{
  if (lwgeom_is_empty(g))
    return false;
  /* The box read from a serialized value is rounded outward, so a point
   * outside it is outside the geometry */
  if (g->bbox && (x < g->bbox->xmin || x > g->bbox->xmax ||
      y < g->bbox->ymin || y > g->bbox->ymax))
    return false;
  switch (g->type)
  {
    case POINTTYPE:
    {
      const POINT2D *p = getPoint2d_cp(((const LWPOINT *) g)->point, 0);
      return p->x == x && p->y == y;
    }
    case LINETYPE:
    {
      const POINTARRAY *pa = ((const LWLINE *) g)->points;
      for (uint32_t i = 0; i + 1 < pa->npoints; i++)
      {
        const POINT2D *a = getPoint2d_cp(pa, i);
        const POINT2D *b = getPoint2d_cp(pa, i + 1);
        if (point_on_segment_exact(x, y, a->x, a->y, b->x, b->y))
          return true;
      }
      return false;
    }
    /* An input vertex in a polygon or on one of its rings, read exactly by
     * #point_in_rings_exact, which answers 0 only outside */
    case POLYGONTYPE:
    {
      const LWPOLY *poly = (const LWPOLY *) g;
      const POINT2D pt = {x, y};
      return point_in_rings_exact(&pt, poly->rings, poly->nrings) != 0;
    }
    case TRIANGLETYPE:
    {
      POINTARRAY *ring = ((const LWTRIANGLE *) g)->points;
      const POINT2D pt = {x, y};
      return point_in_rings_exact(&pt, &ring, 1) != 0;
    }
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case POLYHEDRALSURFACETYPE:
    case TINTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) g;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
        if (relate_geom_holds_vertex(coll->geoms[i], x, y))
          return true;
      return false;
    }
    /* What a curve holds is not read here, and the edges answer it */
    case CIRCSTRINGTYPE:
    case COMPOUNDTYPE:
    case CURVEPOLYTYPE:
      return false;
    default:
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "Unsupported geometry type");
      return false;
  }
}

/**
 * @brief Return in the last arguments the first input vertex of a geometry,
 * false where it has none
 * @details As #lwgeom_startpoint reads it, for every type a geometry of
 * the engine carries: that function raises on a multisurface or a
 * multicurve, where a question that only looks for a witness must answer
 * nothing. A collection, a compound curve and a curve polygon are read
 * through their members (#lwgeom_is_collection)
 */
static bool
relate_geom_first_vertex(const LWGEOM *g, double *x, double *y)
{
  if (lwgeom_is_empty(g))
    return false;
  const POINTARRAY *pa = NULL;
  switch (g->type)
  {
    case POINTTYPE:
      pa = ((const LWPOINT *) g)->point;
      break;
    case LINETYPE:
    case CIRCSTRINGTYPE:
    case TRIANGLETYPE:
      pa = ((const LWLINE *) g)->points;
      break;
    case POLYGONTYPE:
      pa = ((const LWPOLY *) g)->nrings > 0 ? ((const LWPOLY *) g)->rings[0] :
        NULL;
      break;
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COMPOUNDTYPE:
    case CURVEPOLYTYPE:
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case POLYHEDRALSURFACETYPE:
    case TINTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) g;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
        if (relate_geom_first_vertex(coll->geoms[i], x, y))
          return true;
      return false;
    }
    default:
      meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
        "Unsupported geometry type");
      return false;
  }
  if (! pa || pa->npoints == 0)
    return false;
  const POINT2D *p = getPoint2d_cp(pa, 0);
  *x = p->x;
  *y = p->y;
  return true;
}

/**
 * @brief Return true if the first vertex of one geometry lies in or on the
 * other, read on the input coordinates of both, with no edge extracted
 * @details As GEOS RelateNG locates a vertex before it reads an edge
 * (`RelateNG::computeAreaVertex`), a pair that meets mostly shows it at the
 * first vertex of one or the other, and extracting the edges is then what
 * the answer would have cost. The distance walk reads the same first vertex
 * (#geom_distance_first_point) against the polygons of the other. A false
 * answer says nothing, and the caller reads the edges
 */
static bool
relate_vertex_witness(const LWGEOM *from, const LWGEOM *in)
{
  double x, y;
  if (! relate_geom_first_vertex(from, &x, &y))
    return false;
  return relate_geom_holds_vertex(in, x, y);
}

/**
 * @brief Return whether two geometries share a point
 * @details This is the `INTERSECTS` relationship, which #meos_spatialrel
 * reads from the DE-9IM matrix as the pattern `FF*FF****` failing. Answering
 * it from the edges instead stops at the first witness, where the matrix
 * answers all nine cells whatever the question was
 * @param[in] ops Operands of the relationship, carrying the edges of each
 * @param[out] result True if the geometries share a point
 */
static void
meos_intersects(const RelateOperands *ops, bool *result)
{
  assert(ops); assert(result);
  /* Where the geometries meet is read from each one's own edges
   * (#relate_edges_share_point), and whether an edge lies within a surface from the
   * edges of the union, which only a pair no curve of which meets the other
   * asks (#relate_edges_within). The two are one array except for a
   * collection #relate_reads_union names */
  /* The first vertex of either geometry in or on the other is a point they
   * share, read before any edge of either (#relate_vertex_witness) */
  if (relate_vertex_witness(ops->op[1].geom, ops->op[0].geom) ||
      relate_vertex_witness(ops->op[0].geom, ops->op[1].geom))
  {
    *result = true;
    return;
  }
  MeosArray *own[2] = {relate_operand_own(&ops->op[0]),
    relate_operand_own(&ops->op[1])};
  Edge **eo[2] = {relate_edge_pointers(own[0]), relate_edge_pointers(own[1])};

  /* Every question this kernel asks reads one of the two arrays once per edge
   * of the other, so an unindexed walk costs their PRODUCT wherever no witness
   * ends it early. The gate is the one the matrix uses, and below it the walk
   * is the cheaper of the two and the index is not built */
  RelateEdges ro[2];
  bool index = (double) own[0]->count * (double) own[1]->count >=
    RELATE_INDEX_MIN_PAIRS;
  for (int k = 0; k < 2; k++)
    relate_edges_init(&ro[k], eo[k], (int) own[k]->count, index);

  int meet = relate_edges_share_point(&ro[0], &ro[1]);
  if (meet != 0)
    *result = (meet > 0);
  else
  {
    MeosArray *area[2] = {relate_operand_arr(&ops->op[0]),
      relate_operand_arr(&ops->op[1])};
    bool aindex = (double) area[0]->count * (double) area[1]->count >=
      RELATE_INDEX_MIN_PAIRS;
    RelateEdges ra[2];
    Edge **ea[2];
    for (int k = 0; k < 2; k++)
    {
      /* An operand whose two roles are one array, read with the same index,
       * is read once for both */
      if (area[k] == own[k] && aindex == index)
      {
        ea[k] = NULL;
        ra[k] = ro[k];
      }
      else
      {
        ea[k] = relate_edge_pointers(area[k]);
        relate_edges_init(&ra[k], ea[k], (int) area[k]->count, aindex);
      }
    }
    *result = relate_edges_within(&ro[0], &ro[1], &ra[0], &ra[1]);
    for (int k = 0; k < 2; k++)
      if (ea[k])
      {
        relate_edges_clear(&ra[k]);
        pfree(ea[k]);
      }
  }

  for (int k = 0; k < 2; k++)
  {
    relate_edges_clear(&ro[k]);
    pfree(eo[k]);
  }
  return;
}

/**
 * @brief Return whether the second geometry draws a point outside the first
 * @details `COVERS` and `CONTAINS` both ask that nothing of the second
 * geometry falls outside the first, and a boundary of the second running
 * outside the first settles that it does. Reading the edges rejects such a
 * pair without a matrix, and the pairs that survive are few enough that the
 * matrix answers them
 * @param[in] ops Operands of the relationship, carrying the edges of each
 * @param[out] result False where the second geometry draws a point outside
 * the first, which is conclusive; true where it draws none, which is not
 */
static void
meos_covers_possible(const RelateOperands *ops, bool *result)
{
  assert(ops); assert(result);
  const MeosArray *a1 = relate_operand_arr(&ops->op[0]);
  const MeosArray *a2 = relate_operand_arr(&ops->op[1]);
  Edge **e1 = relate_edge_pointers(a1);
  Edge **e2 = relate_edge_pointers(a2);

  /* Every edge of the second geometry is split against, and located in, the
   * first, so the first is read through the index its context keeps, or
   * through one built for the call where the pair is large enough to pay it,
   * by the gate #meos_intersects reads */
  RelateEdges re1;
  bool index = (double) a1->count * (double) a2->count >=
    RELATE_INDEX_MIN_PAIRS;
  bool kept = relate_edges_init_kept(&re1, e1, (int) a1->count, ops, a1,
    index);
  *result = relate_edges_cover(&re1, e2, (int) a2->count);

  /* A kept index belongs to its context and outlives this call */
  if (kept)
    re1.index = NULL;
  relate_edges_clear(&re1);
  pfree(e1); pfree(e2);
  return;
}

/**
 * @brief Return whether two geometries stand in one of the spatial
 * relationships MEOS asks for
 * @details Each relationship is the pattern the standard gives it, matched
 * against the DE-9IM matrix, so they share the one engine and hold the
 * pattern in one place. Two geometries intersect where they share a point,
 * which #meos_intersects reads from the edges directly;
 * one contains another where their interiors meet and nothing of the other
 * falls outside; they touch where their interiors do not meet while a boundary
 * meets the other geometry; and one covers another where nothing of the other
 * falls outside, whichever part of the first it meets.
 * @param[in] g1,g2 Geometries
 * @param[in] rel Relationship asked for
 * @param[out] result True if the geometries stand in the relationship
 * @return True if the pair is covered
 */
static bool
relate_spatialrel_ops(const RelateOperands *opsp, spatialRel rel, bool *result)
{
  const RelateOperands ops = *opsp;
  const LWGEOM *g1 = ops.op[0].geom;
  const LWGEOM *g2 = ops.op[1].geom;

  bool covered = true;

  /* Sharing a point is settled by the first witness found, so it is answered
   * from the edges rather than from a matrix computed whole */
  bool meet;
  meos_intersects(&ops, &meet);
  if (rel == INTERSECTS)
    *result = meet;

  /* Each of the other three relationships asks for a cell of the interior and
   * boundary rows, so each holds only where the geometries share a point.
   * Reading that first leaves the matrix to be computed only for a pair that
   * meets at all */
  else if (! meet)
    *result = false;
  else
  {
    /* Covering and containing both need the second geometry to keep clear of
     * the exterior of the first, and a boundary of it running outside settles
     * that it does not. Few pairs survive that, so the matrix answers those */
    bool possible = true;
    if (rel == COVERS || rel == CONTAINS)
      meos_covers_possible(&ops, &possible);
    if (! possible)
      *result = false;
    else
    {
      static const char *const contains[] = {"T*****FF*"};
      static const char *const touches[] = {"FT*******", "F**T*****",
        "F***T****"};
      static const char *const covers[] = {"T*****FF*", "*T****FF*",
        "***T**FF*", "****T*FF*"};
      RelateQuery q;
      if (rel == CONTAINS)
        relate_query_init(&q, contains, 1);
      else if (rel == TOUCHES)
        relate_query_init(&q, touches, 3);
      else
        relate_query_init(&q, covers, 4);
      char m[10];
      covered = relate_matrix(g1, g2, &ops, &q, true, m);
      if (covered)
        switch (rel)
        {
          case CONTAINS:
            *result = de9im_match(m, "T*****FF*");
            break;
          case TOUCHES:
            *result = de9im_match(m, "FT*******") ||
              de9im_match(m, "F**T*****") || de9im_match(m, "F***T****");
            break;
          case COVERS:
            *result = de9im_match(m, "T*****FF*") ||
              de9im_match(m, "*T****FF*") || de9im_match(m, "***T**FF*") ||
              de9im_match(m, "****T*FF*");
            break;
          default:
            covered = false;
            break;
        }
    }
  }

  return covered;
}

/**
 * @brief The edges of one geometry, kept for every relationship asked about it
 */
struct RelateCtx
{
  RelateOperand op;  /**< The geometry and the edges it draws */
  EdgeIndex *index;  /**< Index over the edges #op reads, built by the first
                          call that needs it and kept with them */
};

/**
 * @brief Read the edges of a geometry once, for a caller that asks about it
 * more than once
 * @param[in] geom Geometry
 * @return The context, or NULL where the geometry is one the engine does not
 * cover, which #meos_spatialrel_ctx then answers as uncovered
 * @note Building the context is what a relationship does at its own entry, so
 * a caller holding one pays the extraction once instead of once per pair
 */
void *
relate_ctx_make(const LWGEOM *geom)
{
  assert(geom);
  /* The same predicate a relationship reads at its entry. Answering NULL here
   * rather than extracting keeps an uncovered geometry on the path it already
   * takes: a relationship over it reports itself uncovered and raises nothing */
  if (geom_meos_coverage(geom) != 1)
    return NULL;
  struct RelateCtx *ctx = palloc(sizeof(struct RelateCtx));
  ctx->op.geom = geom;
  /* The edges and the union of a collection's members are read the first
   * time a question asks for them (#relate_operand_own,
   * #relate_operand_arr), and kept here for every call that borrows the
   * context */
  ctx->op.own = ctx->op.arr = NULL;
  ctx->op.ownkept = &ctx->op.own;
  ctx->op.kept = &ctx->op.arr;
  ctx->index = NULL;
  ctx->op.index = &ctx->index;
  return ctx;
}

/*****************************************************************************
 * The edges of a geometry, kept for the calls that follow
 *
 * A geometry asked about many times -- every composing value of a temporal
 * value against one region, every row of a scan against one constant -- is
 * read as edges once per question unless what that reading produced is kept.
 * The reading is a property of the geometry alone, so an entry holds it and
 * the calls that follow read it.
 *
 * A geometry is recognised the way the vendored PostGIS cache recognises one
 * (postgis/libpgcommon/shared_gserialized.c): the SAME serialized value
 * answers at once, a different size answers no, and only equal sizes are
 * compared byte for byte. Reading the serialized form is what makes the
 * comparison flat, where walking two geometries structurally costs a
 * substantial part of the call it is meant to save.
 *****************************************************************************/

/**
 * @brief An entry holding the edges one geometry is read as
 */
typedef struct
{
  GSERIALIZED *key;  /**< Copy of the serialized geometry, which recognises it */
  LWGEOM *geom;      /**< Copy of the geometry, which the context reads: a
                          context states the edges of the geometry it was made
                          from and does not outlive it */
  void *ctx;         /**< Context holding the edges it is read as */
  uint64_t sig;      /**< Signature of the key, which tells a value that
                          differs from it apart without comparing either */
  uint64_t hits;     /**< Questions answered from this entry */
  int busy;          /**< Calls reading these edges right now */
} RelateCtxCacheEntry;

/* Entries the cache holds. The working set of a walk is the DISTINCT
 * geometries it asks about, so the count is sized to hold one: a corpus of
 * 1444 areal pairs asks about 38 of them, a real join of 670 natural pairs
 * about 255. An entry holds a geometry only while it keeps being asked about,
 * so the cost of a spare slot is one that stays empty.
 * THERE IS NO SIZE FLOOR: what decides whether keeping a reading pays is
 * whether the geometry COMES BACK, never how large it is, and a floor refuses
 * exactly the small-geometry corpora whose reuse is highest. */
#define RELATE_CTX_CACHE_SIZE 256

/* AN ENTRY IS FILLED BY THE SECOND QUESTION ABOUT A VALUE, NEVER THE FIRST.
 * What a kept reading saves is the readings of the calls that FOLLOW it, so a
 * value asked about once pays the copy, the reading and the entry for nothing,
 * and a caller asking about a new value every call -- the lifted walk, which
 * asks about the point of each instant -- fills the cache with values nothing
 * asks for again and scans them on every call after. The signatures below
 * record a value the FIRST time it is asked about and admit it the next, so
 * such a walk fills no entry and scans none, while a value that comes back is
 * kept as it is without them, one question later. The table is read at the
 * signature itself, so it costs one slot whatever it holds */
#define RELATE_CTX_SEEN 4096

/**
 * @brief A value the cache has been asked about, and the entry holding it
 * @details A slot is read at the signature of the value, so a question costs
 * ONE slot whatever the cache holds: a walk asking about thousands of values
 * pays no scan for them
 */
typedef struct
{
  uint64_t sig;  /**< Signature of the value the slot stands for */
  int32_t at;    /**< Entry holding it, -1 where the value is asked about once
                      and no entry holds it yet */
} RelateCtxSeen;

/**
 * @brief The edges kept for the geometries asked about
 */
typedef struct
{
  /* The geometries the cache holds, keyed by their SERIALIZED value, so a hit
   * is decided without reading anything out of them */
  RelateCtxCacheEntry entries[RELATE_CTX_CACHE_SIZE];
  uint32_t count;
  /* The entry a full cache gives way at next, which walks round the entries so
   * that admitting a value costs one of them rather than a pass over all */
  uint32_t next;
  /* Where a value is looked up and where it is admitted from */
  RelateCtxSeen seen[RELATE_CTX_SEEN];
} RelateCtxCache;

/* Global variable to hold the contexts cache */
static MEOS_TLS RelateCtxCache *MEOS_RELATE_CTX_CACHE = NULL;

/**
 * @brief Return true if two serialized geometries are the same one
 * @details The same value answers at once, a different size answers no, and
 * equal sizes are compared byte for byte
 */
static bool
relate_ctx_same(const GSERIALIZED *g1, const GSERIALIZED *g2)
{
  if (g1 == g2)
    return true;
  if (VARSIZE(g1) != VARSIZE(g2))
    return false;
  return memcmp(g1, g2, VARSIZE(g1)) == 0;
}

/**
 * @brief Return a value two different serialized geometries rarely share
 * @details The size and three words of the value, read at places every
 * geometry carries: the head, which holds its box where it has one, the
 * middle and the end of its coordinates. Two values with different signatures
 * are different values, so a signature answers most of the pairs a scan would
 * compare byte for byte; the pairs it leaves standing are decided by
 * #relate_ctx_same, which is what the answer rests on either way
 * @param[in] gs Serialized geometry
 */
static uint64_t
relate_ctx_signature(const GSERIALIZED *gs)
{
  size_t size = VARSIZE(gs);
  uint64_t sig = (uint64_t) size;
  size_t word = sizeof(uint64_t);
  if (size >= VARHDRSZ + word)
  {
    const size_t at[3] = { VARHDRSZ, VARHDRSZ + (size - VARHDRSZ - word) / 2,
      size - word };
    for (int i = 0; i < 3; i++)
    {
      uint64_t w;
      memcpy(&w, (const char *) gs + at[i], word);
      sig = sig * UINT64CONST(31) + w;
    }
  }
  return sig;
}

/**
 * @brief Return the entry holding a value, where the cache holds it
 * @details The slot the signature names says which entry to ask, so the
 * question costs one slot and one comparison of the value against that entry.
 * A slot two values share answers the wrong entry, which #relate_ctx_same
 * refuses, and the question reads as a first sighting
 * @param[in] cache Cache of the geometries asked about, NULL where none is
 * built yet
 * @param[in] gs Serialized geometry
 * @param[in] sig Its signature
 */
static RelateCtxCacheEntry *
relate_ctx_held(RelateCtxCache *cache, const GSERIALIZED *gs, uint64_t sig)
{
  if (! cache)
    return NULL;
  RelateCtxSeen *slot = &cache->seen[sig % RELATE_CTX_SEEN];
  if (slot->sig != sig || slot->at < 0)
    return NULL;
  RelateCtxCacheEntry *e = &cache->entries[slot->at];
  if (! e->key || e->sig != sig || ! relate_ctx_same(e->key, gs))
    return NULL;
  return e;
}

/**
 * @brief Return true if a value is one the cache has been asked about before
 * @details A value the table does not hold is recorded there and answers
 * false, so the entry it would fill waits for the question that comes back.
 * A slot two values share admits the second of them one question early, which
 * keeps a reading asked for once; it never keeps a wrong one, since the entry
 * is recognised by #relate_ctx_same as any other
 * @param[in] cache Cache of the geometries asked about
 * @param[in] sig Signature of the value
 */
static bool
relate_ctx_returns(RelateCtxCache *cache, uint64_t sig)
{
  RelateCtxSeen *slot = &cache->seen[sig % RELATE_CTX_SEEN];
  if (slot->sig == sig)
    return true;
  slot->sig = sig;
  slot->at = -1;
  return false;
}

/**
 * @brief Return the entry a value is filled into, or NULL where every one of
 * them is being read
 * @details An empty entry is taken while the cache has one. Once it is full
 * the entries give way in turn, each admission costing the one the turn
 * reaches rather than a pass over all of them; an entry a call is READING is
 * never displaced, and the turn moves on to the next
 * @param[in] cache Cache of the geometries asked about
 */
static RelateCtxCacheEntry *
relate_ctx_victim(RelateCtxCache *cache)
{
  if (cache->count < RELATE_CTX_CACHE_SIZE)
    return &cache->entries[cache->count++];
  for (uint32_t i = 0; i < RELATE_CTX_CACHE_SIZE; i++)
  {
    RelateCtxCacheEntry *e = &cache->entries[cache->next];
    cache->next = (cache->next + 1) % RELATE_CTX_CACHE_SIZE;
    if (! e->busy)
      return e;
  }
  return NULL;
}

/**
 * @brief Point the slot of a value at the entry that holds it
 * @param[in] cache Cache of the geometries asked about
 * @param[in] e Entry, which carries the signature of the value it holds
 */
static void
relate_ctx_hold(RelateCtxCache *cache, RelateCtxCacheEntry *e)
{
  RelateCtxSeen *slot = &cache->seen[e->sig % RELATE_CTX_SEEN];
  slot->sig = e->sig;
  slot->at = (int32_t) (e - cache->entries);
  return;
}

/**
 * @brief Release the context and the key an entry holds
 */
static void
relate_ctx_cache_clear_entry(RelateCtxCacheEntry *e)
{
  if (e->ctx)
    relate_ctx_free(e->ctx);
  if (e->geom)
    lwgeom_free(e->geom);
  if (e->key)
    pfree(e->key);
  e->ctx = NULL;
  e->geom = NULL;
  e->key = NULL;
  e->hits = 0;
  e->busy = 0;
  return;
}

/**
 * @ingroup meos_setup
 * @brief Destroy the cache holding the edges of the geometries asked about
 */
void
meos_finalize_relate(void)
{
  /* Idempotency: only destroy a live cache, and null the slot so a second
   * call does not free the entries again */
  RelateCtxCache *cache = MEOS_RELATE_CTX_CACHE;
  if (! cache)
    return;
  for (uint32_t i = 0; i < cache->count; i++)
    relate_ctx_cache_clear_entry(&cache->entries[i]);
  pfree(cache);
  MEOS_RELATE_CTX_CACHE = NULL;
  return;
}

/**
 * @brief Return the context holding the edges of a serialized geometry, and
 * the entry it belongs to where one holds it
 * @details The entry is filled where the FIRST question about a geometry is
 * asked and never ahead of one, so a pair the extents separate is answered
 * before any edge is read. A geometry small enough that reading it is not
 * what a call costs is not kept.
 * @param[in] gs Serialized geometry
 * @param[in] geom Its geometry, which the context is built from
 * @param[out] entry Entry holding the context, NULL where none does and the
 * caller owns what it is given
 */
void *
relate_ctx_borrow(const GSERIALIZED *gs, void **entry, LWGEOM **made)
{
  *entry = NULL;
  *made = NULL;

  /* THE CACHE IS ASKED ON THE SERIALIZED VALUE, BEFORE ANYTHING IS READ OUT OF
   * IT. The edges of a geometry are what a relate call spends itself on, and
   * they do not change between the calls that ask about it, so they are read
   * ONCE and answered from thereafter. A caller that deserializes its operands
   * first pays that reading on every call the cache exists to save. */
  RelateCtxCache *cache = MEOS_RELATE_CTX_CACHE;
  uint64_t sig = relate_ctx_signature(gs);
  RelateCtxCacheEntry *seen = relate_ctx_held(cache, gs, sig);
  if (seen && seen->ctx)
  {
    seen->hits++;
    seen->busy++;
    *entry = seen;
    return seen->ctx;
  }

  /* An entry may hold the GEOMETRY without its edges, where a distance
   * predicate asked for the geometry alone. The edges are read into that same
   * entry, from the geometry it already holds, so nothing is read or copied a
   * second time. */
  if (seen && seen->geom && ! seen->busy)
  {
#if ! MEOS
    MemoryContext oldc = MemoryContextSwitchTo(TopMemoryContext);
#endif
    void *c = seen->ctx = relate_ctx_make(seen->geom);
#if ! MEOS
    MemoryContextSwitchTo(oldc);
#endif
    if (c)
    {
      seen->hits++;
      seen->busy++;
      *entry = seen;
      return c;
    }
  }

#if ! MEOS
  MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
  if (! cache)
  {
    cache = palloc0(sizeof(RelateCtxCache));
    MEOS_RELATE_CTX_CACHE = cache;
  }

  /* FILL FOR THE QUESTION THAT COMES BACK. The edges are read to answer THIS
   * call whatever the cache does, so keeping what the reading produced costs
   * nothing beyond the entry: one reading per geometry rather than one per
   * pair it appears in. The entry is filled once the value is asked about
   * again, so a walk asking about each value once keeps none of them.
   * This is not the eager cache that loses 16.8x on well-separated data --
   * that one built every geometry's context UP FRONT, outside the call path,
   * so the bounding-box test in front of the relationship saved nothing. The
   * box short-circuit sits BEFORE this borrow, so a pair that never computes
   * never reaches it, and filling here stays at or below the extraction count
   * of a cache-less engine in every regime. */
  RelateCtxCacheEntry *e = seen;
  if (! e && relate_ctx_returns(cache, sig))
  {
    e = relate_ctx_victim(cache);
    if (e && (e->key || e->ctx || e->geom))
      relate_ctx_cache_clear_entry(e);
  }

  void *ctx = NULL;
  if (e && ! e->busy)
  {
    /* THE ENTRY READS ITS OWN COPY OF THE SERIALIZED VALUE, and that is what
     * makes one copy enough. #lwgeom_from_gserialized does not copy the
     * coordinates: a ring is `ptarray_construct_reference_data(..., ptr)`, a
     * VIEW over the bytes it was read from. A geometry read from the CALLER'S
     * value is therefore a view over memory the caller reuses on its next row,
     * and keeping it hands the edges a buffer that changes underneath them --
     * measured, 13 of 1444 areal answers and 3 of 54 adversarial ones.
     * The entry already keeps the serialized value, to recognise the geometry
     * by it. Reading from THAT copy makes the view point into memory the entry
     * owns and releases together with it, so the deep clone this used to take
     * is unnecessary: one copy, not two, and no per-ring copy at all. */
    if (! e->key)
    {
      e->key = palloc(VARSIZE(gs));
      memcpy(e->key, gs, VARSIZE(gs));
      e->sig = sig;
      relate_ctx_hold(cache, e);
    }
    if (! e->geom)
      e->geom = lwgeom_from_gserialized(e->key);
    ctx = e->ctx = e->geom ? relate_ctx_make(e->geom) : NULL;
    if (ctx)
    {
      e->hits++;
      e->busy++;
      *entry = e;
    }
    else
      relate_ctx_cache_clear_entry(e);
  }
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif

  if (*entry)
    return ctx;
  /* No entry holds this reading, so the caller is handed its own and releases
   * it; that one may view the caller's value, which outlives this call */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  if (! geom)
    return NULL;
  *made = geom;
  return relate_ctx_make(geom);
}

/**
 * @brief Borrow the GEOMETRY a serialized value reads as, without reading its
 * edges
 * @details A distance predicate -- dwithin, its 3D form, the geodetic pair --
 * hands an LWGEOM to a liblwgeom routine and never looks at an edge, so the
 * edges an entry may hold are of no use to it while the READING is. The entry
 * keeps the geometry either way, so a caller that wants the geometry alone
 * takes it and leaves the edges unbuilt; a later relationship on the same
 * geometry reads them into the entry it already has.
 * THE CALLER READS IT UNDER THE CONTEXT THAT KEEPS IT, as #relate_ctx_borrow
 * builds the edges: liblwgeom attaches boxes to a geometry it measures and to
 * its parts (`measures.c` #lw_dist2d_recursive, `lwgeodetic.c`
 * #lwcollection_calculate_gbox_geodetic), and a box read into the per-tuple
 * context is freed under the entry that still points at it.
 * @param[in] gs Serialized geometry
 * @param[out] entry Entry holding it, NULL where none does
 * @param[out] made Geometry read for this call, which the caller releases
 * @return The geometry, or NULL where it cannot be read
 */
const LWGEOM *
relate_ctx_borrow_geom(const GSERIALIZED *gs, void **entry, LWGEOM **made)
{
  *entry = NULL;
  *made = NULL;

  RelateCtxCache *cache = MEOS_RELATE_CTX_CACHE;
  uint64_t sig = relate_ctx_signature(gs);
  RelateCtxCacheEntry *held = relate_ctx_held(cache, gs, sig);
  if (held && held->geom)
  {
    held->hits++;
    held->busy++;
    *entry = held;
    return held->geom;
  }

#if ! MEOS
  MemoryContext oldctx = MemoryContextSwitchTo(TopMemoryContext);
#endif
  if (! cache)
  {
    cache = palloc0(sizeof(RelateCtxCache));
    MEOS_RELATE_CTX_CACHE = cache;
  }
  /* THE VALUE IS READ ONCE, out of whichever copy the reading is a view over:
   * the entry's where one keeps it, the caller's where none does. A value
   * asked about for the first time is read for this call alone, so nothing is
   * copied and nothing is read twice */
  RelateCtxCacheEntry *e = relate_ctx_returns(cache, sig) ?
    relate_ctx_victim(cache) : NULL;
  if (e)
  {
    /* Read from the entry's OWN copy, for the reason #relate_ctx_borrow
     * states: a geometry is a view over the bytes it was read from */
    if (e->key || e->ctx || e->geom)
      relate_ctx_cache_clear_entry(e);
    e->key = palloc(VARSIZE(gs));
    memcpy(e->key, gs, VARSIZE(gs));
    e->sig = sig;
    relate_ctx_hold(cache, e);
    e->geom = lwgeom_from_gserialized(e->key);
    if (! e->geom)
    {
      relate_ctx_cache_clear_entry(e);
#if ! MEOS
      MemoryContextSwitchTo(oldctx);
#endif
      return NULL;
    }
    e->hits = 1;
    e->busy = 1;
    *entry = e;
#if ! MEOS
    /* EVERY exit restores the caller's context. The one that did not left the
     * backend allocating into TopMemoryContext for the rest of the query, so
     * the next per-tuple reset freed chunks that were never in it */
    MemoryContextSwitchTo(oldctx);
#endif
    return e->geom;
  }
  /* No entry holds this value, so the caller is handed its own reading and
   * releases it. It is read under the context the kept ones are read under,
   * since liblwgeom attaches boxes to what it measures */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
#if ! MEOS
  MemoryContextSwitchTo(oldctx);
#endif
  *made = geom;
  return geom;
}

/**
 * @brief Give back the geometry #relate_ctx_borrow_geom answered
 * @details An entry holds its geometry for the calls that follow; a geometry
 * no entry holds belongs to the caller, which releases it
 */
void
relate_ctx_return_geom(void *entry)
{
  if (entry)
    ((RelateCtxCacheEntry *) entry)->busy--;
  return;
}

/**
 * @brief Give back a context #relate_ctx_borrow answered
 * @details An entry keeps its context for the calls that follow; a context no
 * entry holds belongs to the caller, which releases it
 */
void
relate_ctx_return(void *ctx, void *entry)
{
  if (entry)
  {
    ((RelateCtxCacheEntry *) entry)->busy--;
    return;
  }
  relate_ctx_free(ctx);
  return;
}

/**
 * @brief Release a context #relate_ctx_make answered
 */
void
relate_ctx_free(void *ctxv)
{
  struct RelateCtx *ctx = (struct RelateCtx *) ctxv;
  if (! ctx)
    return;
  if (ctx->index)
    edge_index_free(ctx->index);
  if (ctx->op.own && ctx->op.own != ctx->op.arr)
    meos_array_destroy(ctx->op.own);
  if (ctx->op.arr)
    meos_array_destroy(ctx->op.arr);
  pfree(ctx);
  return;
}

/**
 * @brief Return whether two geometries stand in a spatial relationship,
 * reading edges each context has already extracted
 * @param[in] ctx1,ctx2 Contexts of the two geometries, in the order asked
 * about
 * @param[in] rel Relationship asked for
 * @param[out] result True if the geometries stand in the relationship
 * @return True if the pair is covered
 * @note The contexts keep owning their edges: this borrows them for the call
 * and releases nothing, so one context serves as many relationships as the
 * caller asks
 */
bool
meos_spatialrel_ctx(const void *ctx1, const void *ctx2, spatialRel rel,
  bool *result)
{
  assert(result);
  const struct RelateCtx *c1 = (const struct RelateCtx *) ctx1;
  const struct RelateCtx *c2 = (const struct RelateCtx *) ctx2;

  /* A geometry the engine does not cover carries no context, and the pair is
   * uncovered exactly as #meos_spatialrel reports it */
  if (! c1 || ! c2)
    return false;

  /* An empty geometry holds no point to share with another, and none of the
   * four patterns admits an empty operand */
  if (lwgeom_is_empty(c1->op.geom) || lwgeom_is_empty(c2->op.geom))
  {
    *result = false;
    return true;
  }

  RelateOperands ops;
  ops.op[0] = c1->op;
  ops.op[1] = c2->op;
  return relate_spatialrel_ops(&ops, rel, result);
}

/**
 * @brief Return whether two geometries stand in a spatial relationship
 * @param[in] g1,g2 Geometries
 * @param[in] rel Relationship asked for
 * @param[out] result True if the geometries stand in the relationship
 * @return True if the pair is covered
 */
bool
meos_spatialrel(const LWGEOM *g1, const LWGEOM *g2, spatialRel rel,
  bool *result)
{
  assert(g1); assert(g2); assert(result);

  /* Every native implementation reads the same predicate: the engine answers
   * a geometry the edge decomposition reaches, and nothing else */
  if (geom_meos_coverage(g1) != 1 || geom_meos_coverage(g2) != 1)
    return false;

  /* An empty geometry holds no point to share with another, and none of the
   * four patterns admits an empty operand */
  if (lwgeom_is_empty(g1) || lwgeom_is_empty(g2))
  {
    *result = false;
    return true;
  }

  /* Each step below reads the edges of the same two geometries, so both are
   * extracted ONCE here and every step borrows the answer. Reading the edges
   * of a multi-surface as those of the union of its members is what a call on
   * such a geometry mostly costs, and it was paid once per step */
  RelateOperands ops;
  relate_operands_init(&ops, g1, g2);
  bool covered = relate_spatialrel_ops(&ops, rel, result);
  relate_operands_free(&ops);
  return covered;
}

/**
 * @brief Return whether two geometries satisfy a DE-9IM pattern
 * @param[in] g1,g2 Geometries
 * @param[in] pattern DE-9IM pattern, in the alphabet #de9im_match reads
 * @param[out] result True if the geometries satisfy the pattern
 * @return True if the pair is covered, which is what
 * #geom_meos_coverage answers 1 for each geometry. A false return means the pair is outside that
 * coverage, @b not that the pattern fails, so a caller must answer it another
 * way rather than read @p result
 */
bool
meos_relate_pattern(const LWGEOM *g1, const LWGEOM *g2, const char *pattern,
  bool *result)
{
  assert(g1); assert(g2); assert(pattern); assert(result);
  /* A pattern reads the cells it does not mark `*`, and those are the cells
   * the matrix is computed for */
  RelateQuery q;
  relate_query_init(&q, &pattern, 1);
  char matrix[10];
  if (! relate_matrix(g1, g2, NULL, &q, false, matrix))
    return false;
  *result = de9im_match(matrix, pattern);
  return true;
}

/*****************************************************************************
 * Union of the linear and point components of a geometry
 *****************************************************************************/

/**
 * @brief Compute the DE-9IM matrix of two geometries from the edges already
 * read for them
 * @details PostGIS caches no prepared geometry for `ST_Relate`, and its
 * reason does not carry over: what it keeps is a prepared INDEX, and GEOS
 * prepares predicates only -- there is no prepared form that answers a
 * matrix, so there is nothing for its matrix entry to reuse. What MEOS keeps
 * is the EDGES, which the matrix reads exactly as a predicate does. A join
 * asking for the matrix of every pair reads one geometry once per pair it
 * appears in without this.
 * @param[in] ctx1,ctx2 Contexts of the two geometries
 * @param[out] result The matrix
 * @return true if the pair is one the engine covers
 */
bool
meos_relate_ctx(const void *ctx1, const void *ctx2, char result[10])
{
  assert(result);
  const struct RelateCtx *c1 = (const struct RelateCtx *) ctx1;
  const struct RelateCtx *c2 = (const struct RelateCtx *) ctx2;
  if (! c1 || ! c2)
    return false;
  RelateOperands ops;
  ops.op[0] = c1->op;
  ops.op[1] = c2->op;
  return relate_matrix(c1->op.geom, c2->op.geom, &ops, NULL, false, result);
}

/**
 * @brief Compute the DE-9IM matrix of a geometry with itself, from the
 * context kept for it
 * @details The interior and the boundary of a geometry are disjoint, and
 * neither meets its exterior, so the matrix of a geometry with itself holds
 * the dimension of its interior, the dimension of its boundary and the two
 * exteriors meeting in dimension 2, and nothing else. The dimensions are the
 * ones the matrix of two geometries apart reads (#relate_matrix_apart), and
 * only for a geometry whose own dimension and the one its edges draw agree,
 * as #relate_extents_apart requires of it
 * @param[in] ctx Context of the geometry
 * @param[out] result The matrix
 * @return true if the matrix is answered here, false where the caller reads
 * it from the edges of the two operands
 */
bool
meos_relate_self_ctx(const void *ctx, char result[10])
{
  assert(result);
  const struct RelateCtx *c = (const struct RelateCtx *) ctx;
  if (! c || lwgeom_is_empty(c->op.geom))
    return false;
  const LWGEOM *geom = c->op.geom;
  int mask = relate_dim_mask(geom);
  int drawn = (mask & 4) ? 2 : (mask & 2) ? 1 : (mask & 1) ? 0 : -1;
  if ((mask & (mask - 1)) != 0 || drawn != relate_dimension(geom))
    return false;
  MeosDE9IM m;
  de9im_init(&m);
  m.ii = (int8_t) drawn;
  m.bb = relate_boundary_dimension(geom);
  m.ee = 2;
  de9im_to_string(&m, result);
  return true;
}

/**
 * @brief Return whether a DE-9IM pattern holds of two geometries, from the
 * edges already read for them
 * @details The twin of #meos_spatialrel_ctx for the pattern entry. PostGIS
 * caches the prepared geometry of `relate_pattern` as it does of the
 * predicates (`GetPrepGeomCache(fcinfo, shared_geom1, shared_geom2)`), so the
 * entry that answers a pattern reads the kept edges rather than building its
 * own. `ST_Relate` returning the whole matrix caches in neither engine: it
 * asks for every cell, so there is no step it answers on behalf of.
 * @param[in] ctx1,ctx2 Contexts of the two geometries
 * @param[in] pattern DE-9IM pattern
 * @param[out] result Whether the pattern holds
 * @return true if the pair is one the engine covers
 */
bool
meos_relate_pattern_ctx(const void *ctx1, const void *ctx2,
  const char *pattern, bool *result)
{
  assert(pattern); assert(result);
  const struct RelateCtx *c1 = (const struct RelateCtx *) ctx1;
  const struct RelateCtx *c2 = (const struct RelateCtx *) ctx2;

  /* A geometry the engine does not cover carries no context, and the pair is
   * uncovered exactly as #meos_relate_pattern reports it */
  if (! c1 || ! c2)
    return false;

  RelateQuery q;
  relate_query_init(&q, &pattern, 1);
  RelateOperands ops;
  ops.op[0] = c1->op;
  ops.op[1] = c2->op;
  char matrix[10];
  if (! relate_matrix(c1->op.geom, c2->op.geom, &ops, &q, false,
        matrix))
    return false;
  *result = de9im_match(matrix, pattern);
  return true;
}

/**
 * @brief Return true if a type is one the linear union answers for
 * @details A surface is answered by #meos_areal_union() instead, and a type
 * carrying no linework at all is answered by neither
 */
static bool
linear_union_type(uint8_t type)
{
  return type == POINTTYPE || type == MULTIPOINTTYPE || type == LINETYPE ||
    type == MULTILINETYPE || type == CIRCSTRINGTYPE || type == COMPOUNDTYPE ||
    type == MULTICURVETYPE;
}

/**
 * @brief Collect the components of a geometry that carry no collection of
 * their own, in the order the geometry gives them
 * @details A multipoint, a multiline and a multicurve contribute their members;
 * a compound curve does NOT, its pieces being one curve rather than several
 * @return False where a component is of a type the linear union does not answer
 */
static bool
linear_union_collect(const LWGEOM *geom, const LWGEOM ***comps, int *ncomp,
  int *maxcomp)
{
  if (! geom || lwgeom_is_empty(geom))
    return true;
  uint8_t type = geom->type;
  if (type == MULTIPOINTTYPE || type == MULTILINETYPE ||
      type == MULTICURVETYPE || type == COLLECTIONTYPE)
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      if (! linear_union_collect(coll->geoms[i], comps, ncomp, maxcomp))
        return false;
    return true;
  }
  if (! linear_union_type(type))
    return false;
  if (*ncomp == *maxcomp)
  {
    *maxcomp *= 2;
    *comps = repalloc(*comps, sizeof(LWGEOM *) * (*maxcomp));
  }
  (*comps)[(*ncomp)++] = geom;
  return true;
}

/**
 * @brief Return true if a DE-9IM matrix says the first geometry of the ordered
 * pair it was read from covers the second
 */
static bool
linear_union_covers(const char matrix[10])
{
  return de9im_match(matrix, "T*****FF*") ||
    de9im_match(matrix, "*T****FF*") || de9im_match(matrix, "***T**FF*") ||
    de9im_match(matrix, "****T*FF*");
}

/**
 * @brief Write the DE-9IM matrix of the reversed pair
 * @details Row @p r column @p c of a DE-9IM matrix holds the dimension of what
 * the @p r-th part of the first geometry shares with the @p c-th part of the
 * second, and what the two share does not depend on the order they are named
 * in.  The matrix of the reversed pair is therefore the TRANSPOSE of this one,
 * so the reversed question is answered by reading the matrix already computed
 * rather than by a second walk over the two geometries
 */
static void
linear_union_transpose(const char matrix[10], char result[10])
{
  for (int r = 0; r < 3; r++)
    for (int c = 0; c < 3; c++)
      result[3 * c + r] = matrix[3 * r + c];
  result[9] = '\0';
}

/**
 * @brief A stretch of an edge that another edge covers
 * @details Both ends are INPUT VERTICES lying on the edge -- an end of the
 * covering edge, or an end of the covered one where the cover runs past it --
 * so a piece cut between two of them is drawn with coordinates the input
 * carries, never with a point constructed at a parameter
 */
typedef struct
{
  double lo;         /**< Position of the start along the edge */
  double hi;         /**< Position of the end along the edge */
  POINT2D plo;       /**< Vertex at the start */
  POINT2D phi;       /**< Vertex at the end */
} LinearStretch;

/**
 * @brief Return the position of a point along the line an edge spans
 * @details Points lying on that line are ordered along it by the coordinate
 * the edge advances most in, oriented to grow from the start of the edge to
 * its end. That is the parameter of the point on the edge up to an increasing
 * affine map, read off the coordinate itself: comparing two positions
 * compares two input coordinates, so the order is exact where a parameter
 * computed as @p (p - start) . d / |d|^2 would be a rounding of it
 */
static inline double
linear_union_position(const Edge *edge, double x, double y)
{
  if (fabs(edge->dx) >= fabs(edge->dy))
    return (edge->dx > 0.0) ? x : -x;
  return (edge->dy > 0.0) ? y : -y;
}

/**
 * @brief Return true if another edge covers a stretch of an edge, and that
 * stretch in the last argument
 * @details The other edge shares more than a point with the edge only where
 * both its ends lie on the line the edge spans, a question on four input
 * vertices that two exact determinant signs answer (#cross_product_sign).
 * The stretch is then the span of the other edge's ends clipped to the
 * edge's own, compared by their position along it (#linear_union_position).
 * A pair meeting at a point leaves the edge whole
 */
static bool
linear_union_cover(const Edge *b, const Edge *a, LinearStretch *s)
{
  assert(b); assert(a); assert(s);
  if (cross_product_sign(b->x1, b->y1, b->x2, b->y2, b->x1, b->y1,
        a->x1, a->y1) != 0 ||
      cross_product_sign(b->x1, b->y1, b->x2, b->y2, b->x1, b->y1,
        a->x2, a->y2) != 0)
    return false;
  double pa1 = linear_union_position(b, a->x1, a->y1);
  double pa2 = linear_union_position(b, a->x2, a->y2);
  bool forward = pa1 <= pa2;
  s->lo = forward ? pa1 : pa2;
  s->hi = forward ? pa2 : pa1;
  s->plo.x = forward ? a->x1 : a->x2; s->plo.y = forward ? a->y1 : a->y2;
  s->phi.x = forward ? a->x2 : a->x1; s->phi.y = forward ? a->y2 : a->y1;
  double pb1 = linear_union_position(b, b->x1, b->y1);
  double pb2 = linear_union_position(b, b->x2, b->y2);
  if (s->lo < pb1)
  {
    s->lo = pb1; s->plo.x = b->x1; s->plo.y = b->y1;
  }
  if (s->hi > pb2)
  {
    s->hi = pb2; s->phi.x = b->x2; s->phi.y = b->y2;
  }
  return s->lo < s->hi;
}

/**
 * @brief Return the segment joining two input vertices
 */
static LWGEOM *
linear_union_piece(const POINT2D *p1, const POINT2D *p2, int32_t srid)
{
  assert(p1); assert(p2);
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 2);
  POINT4D p;
  memset(&p, 0, sizeof(POINT4D));
  p.x = p1->x; p.y = p1->y;
  ptarray_append_point(pa, &p, LW_TRUE);
  p.x = p2->x; p.y = p2->y;
  ptarray_append_point(pa, &p, LW_TRUE);
  return lwline_as_lwgeom(lwline_construct(srid, NULL, pa));
}

/**
 * @brief Add to an array the pieces of an edge that no edge of a set covers
 * @details The stretches the set covers are read in the order the edge is
 * walked, and a piece runs from where one stretch ends to where the next one
 * starts. Every end is an input vertex compared by position, so two
 * stretches meeting at a shared vertex leave nothing between them and an edge
 * of any length keeps what is uncovered of it, however short. An edge of zero
 * length draws a point, which a line already walks, and adds no piece.
 * The stretches are sorted by where they start, so the pieces do not depend
 * on the order the covering edges are given in
 * @param[in] b Edge
 * @param[in] edges Edges that may cover it
 * @param[in] which,count Positions in @p edges of the edges to read, or
 * @p NULL for the first @p count of them
 * @param[out] cover Room for @p count stretches
 * @param[out] pieces Array the pieces are appended to, with room for
 * @p count + 1 of them
 * @param[in] srid Spatial reference identifier
 * @return Number of pieces added
 */
static int
linear_union_uncovered(const Edge *b, const MeosArray *edges,
  const int *which, int count, LinearStretch *cover, LWGEOM **pieces,
  int32_t srid)
{
  assert(b); assert(edges); assert(cover); assert(pieces);
  if (b->dx == 0.0 && b->dy == 0.0)
    return 0;
  int ncover = 0;
  for (int k = 0; k < count; k++)
  {
    LinearStretch s;
    int j = which ? which[k] : k;
    if (! linear_union_cover(b, (const Edge *) meos_array_get_intl(edges, j),
        &s))
      continue;
    int q = ncover - 1;
    while (q >= 0 && cover[q].lo > s.lo)
    {
      cover[q + 1] = cover[q];
      q--;
    }
    cover[q + 1] = s;
    ncover++;
  }
  int npieces = 0;
  POINT2D cur = { b->x1, b->y1 };
  double pcur = linear_union_position(b, b->x1, b->y1);
  for (int p = 0; p < ncover; p++)
  {
    if (cover[p].lo > pcur)
      pieces[npieces++] = linear_union_piece(&cur, &cover[p].plo, srid);
    if (cover[p].hi > pcur)
    {
      pcur = cover[p].hi;
      cur = cover[p].phi;
    }
  }
  if (linear_union_position(b, b->x2, b->y2) > pcur)
  {
    POINT2D end = { b->x2, b->y2 };
    pieces[npieces++] = linear_union_piece(&cur, &end, srid);
  }
  return npieces;
}

/**
 * @brief Return true if every edge of a geometry is a straight line segment
 */
static bool
linear_union_straight(const MeosArray *edges)
{
  assert(edges);
  if (edges->count == 0)
    return false;
  for (int i = 0; i < (int) edges->count; i++)
    if (((const Edge *) meos_array_get_intl(edges, i))->etype != EDGE_LINESEG)
      return false;
  return true;
}

/**
 * @brief Return true if two points are the same point
 * @details Every end of a piece is an input vertex, copied as the input
 * holds it, so two ends are the same point exactly where their coordinates
 * are equal: two distinct doubles are two distinct points, however near
 */
static bool
linear_union_same_point(const POINT2D *p1, const POINT2D *p2)
{
  assert(p1); assert(p2);
  return p1->x == p2->x && p1->y == p2->y;
}

/**
 * @brief A chain of points a curve is assembled into, grown at its end
 */
typedef struct
{
  POINT2D *points;   /**< Points of the chain, in the order it is walked */
  int count;         /**< Number of points */
  int maxcount;      /**< Number of points the array has room for */
} LinearChain;

/**
 * @brief Add a point to the end of a chain
 */
static void
linear_chain_append(LinearChain *chain, const POINT2D *p)
{
  assert(chain); assert(p);
  if (chain->count == chain->maxcount)
  {
    chain->maxcount *= 2;
    chain->points = repalloc(chain->points,
      sizeof(POINT2D) * (size_t) chain->maxcount);
  }
  chain->points[chain->count++] = *p;
}

/**
 * @brief Walk a chain the other way
 * @details A curve and the same curve reversed draw the same points, so a
 * piece continuing the START of the chain is attached by turning the chain
 * around and appending to it, which leaves one growth direction to maintain
 */
static void
linear_chain_reverse(LinearChain *chain)
{
  assert(chain);
  for (int i = 0, j = chain->count - 1; i < j; i++, j--)
  {
    POINT2D tmp = chain->points[i];
    chain->points[i] = chain->points[j];
    chain->points[j] = tmp;
  }
}

/**
 * @brief One end of a piece, as the endpoint index holds it
 */
typedef struct
{
  uint32_t piece;  /**< Piece the end belongs to */
  int32_t next;    /**< Next end in the same bucket, -1 at the end of a chain */
  double x;        /**< The end itself */
  double y;
} LinearEnd;

/**
 * @brief The ends of a set of pieces, read by the point they sit at
 * @details Sewing asks, once per attachment, which unused piece continues the
 * chain at one of its two ends, and answering it by walking every piece makes
 * the sewing quadratic in the pieces.
 *
 * The ends are read out of a hash of their coordinates instead. THE HASH IS
 * EXACT, not a filter over a tolerance: #linear_union_same_point accepts two
 * points only where both coordinates are EQUAL, so two ends that attach carry
 * the same bits and fall in the same bucket. Nothing outside a bucket can
 * attach, and a bucket's own entries are still compared, so a collision costs
 * a comparison and never an answer.
 */
typedef struct
{
  LinearEnd *ends;   /**< Two per piece, its first point and its last */
  int32_t *buckets;  /**< First end of a bucket, -1 where it holds none */
  uint32_t nbuckets; /**< Buckets, a power of two so the hash masks */
  uint32_t nends;    /**< Ends held */
} LinearEndIndex;

/**
 * @brief Return the bucket a point chains from
 * @details The coordinates are read as the bit patterns they are, which is
 * what makes the lookup exact
 */
static uint32_t
linear_end_bucket(const LinearEndIndex *ix, double x, double y)
{
  uint64_t hx, hy;
  memcpy(&hx, &x, sizeof(uint64_t));
  memcpy(&hy, &y, sizeof(uint64_t));
  /* A zero carries a sign in its bits while #linear_union_same_point reads the
   * two zeroes as ONE point, so the negative one is hashed as the positive.
   * The bits are what is corrected: the two doubles compare equal already, so
   * a comparison between them can express nothing */
  if (hx == UINT64CONST(0x8000000000000000)) hx = 0;
  if (hy == UINT64CONST(0x8000000000000000)) hy = 0;
  uint64_t h = hx * UINT64CONST(0x9E3779B97F4A7C15) ^
    hy * UINT64CONST(0xC2B2AE3D27D4EB4F);
  h ^= h >> 29;
  h *= UINT64CONST(0xBF58476D1CE4E5B9);
  h ^= h >> 32;
  return (uint32_t) (h & (uint64_t) (ix->nbuckets - 1));
}

/**
 * @brief Build the endpoint index of a set of pieces
 * @details A piece drawing no line attaches nothing, so it is marked taken
 * here and never enters the index, which is where the walk this replaces
 * marked it the first time it reached it
 * @param[out] ix Index to build
 * @param[in] pieces,npieces Pieces to index
 * @param[in,out] used Which pieces are taken
 */
static void
linear_end_index_make(LinearEndIndex *ix, LWGEOM **pieces, int npieces,
  bool *used)
{
  assert(ix); assert(pieces); assert(used);
  ix->nbuckets = 16;
  while (ix->nbuckets < 4 * (uint32_t) npieces)
    ix->nbuckets *= 2;
  ix->buckets = palloc(sizeof(int32_t) * ix->nbuckets);
  for (uint32_t i = 0; i < ix->nbuckets; i++)
    ix->buckets[i] = -1;
  ix->ends = palloc(sizeof(LinearEnd) * 2 *
    (size_t) (npieces > 0 ? npieces : 1));
  ix->nends = 0;
  for (int p = 0; p < npieces; p++)
  {
    const POINTARRAY *pa = ((const LWLINE *) pieces[p])->points;
    if (pieces[p]->type != LINETYPE || pa->npoints < 2)
    {
      used[p] = true;
      continue;
    }
    const POINT2D *ends[2] = { getPoint2d_cp(pa, 0),
      getPoint2d_cp(pa, pa->npoints - 1) };
    for (int e = 0; e < 2; e++)
    {
      uint32_t at = ix->nends++;
      ix->ends[at].piece = (uint32_t) p;
      ix->ends[at].x = ends[e]->x;
      ix->ends[at].y = ends[e]->y;
      uint32_t b = linear_end_bucket(ix, ends[e]->x, ends[e]->y);
      ix->ends[at].next = ix->buckets[b];
      ix->buckets[b] = (int32_t) at;
    }
  }
  return;
}

/**
 * @brief Release what an endpoint index holds
 */
static void
linear_end_index_free(LinearEndIndex *ix)
{
  assert(ix);
  pfree(ix->buckets); pfree(ix->ends);
  ix->buckets = NULL; ix->ends = NULL;
  return;
}

/**
 * @brief Return the lowest-numbered unused piece meeting a point, or -1
 * @details The walk this replaces takes the pieces in order and stops at the
 * first that attaches, so the piece it reaches is the lowest-numbered one
 * meeting either end of the chain. Reading that same piece out of the index
 * keeps the curve the sewing draws
 */
static int
linear_end_index_at(const LinearEndIndex *ix, const bool *used, POINT2D p)
{
  assert(ix); assert(used);
  int best = -1;
  for (int32_t at = ix->buckets[linear_end_bucket(ix, p.x, p.y)]; at >= 0;
       at = ix->ends[at].next)
  {
    const LinearEnd *end = &ix->ends[at];
    if (used[end->piece] || (best >= 0 && (int) end->piece > best))
      continue;
    if (end->x == p.x && end->y == p.y)
      best = (int) end->piece;
  }
  return best;
}

/**
 * @brief Sew a set of pieces into the curves they draw
 * @details Each piece continues a chain at one of its two ends, in either
 * direction, and the piece that does is read out of an index of the ends, the
 * enlarged chain reaching further than it did. A piece continuing neither end
 * of the chain under construction starts a chain of its own, so a set drawing
 * a fork is answered as the several curves it takes to walk it rather than
 * refused
 * @param[in] pieces,npieces Pieces to sew, which stay the caller's
 * @param[in] srid Spatial reference identifier
 * @param[out] result Curves drawn, allocated here and owned by the caller
 * @return Number of curves, or 0 where there is nothing to sew
 */
static int
linear_union_chain_all(LWGEOM **pieces, int npieces, int32_t srid,
  LWGEOM ***result)
{
  assert(result);
  *result = NULL;
  if (npieces <= 0)
    return 0;
  bool *used = palloc0(sizeof(bool) * (size_t) npieces);
  LWGEOM **out = palloc(sizeof(LWGEOM *) * (size_t) npieces);
  int nout = 0;
  /* Every chain is sewn from the same pieces, so their ends are indexed once
   * for all of them rather than walked once per attachment */
  LinearEndIndex ix;
  linear_end_index_make(&ix, pieces, npieces, used);

  for (int seed = 0; seed < npieces; seed++)
  {
    if (used[seed])
      continue;
    const POINTARRAY *spa = ((const LWLINE *) pieces[seed])->points;
    if (pieces[seed]->type != LINETYPE || spa->npoints < 2)
    {
      used[seed] = true;
      continue;
    }
    used[seed] = true;
    LinearChain chain;
    chain.maxcount = (int) spa->npoints + 2 * npieces + 2;
    chain.count = 0;
    chain.points = palloc(sizeof(POINT2D) * (size_t) chain.maxcount);
    for (uint32_t i = 0; i < spa->npoints; i++)
      linear_chain_append(&chain, getPoint2d_cp(spa, i));

    bool attached = true;
    while (attached)
    {
      attached = false;
      /* The two ends are COPIED: appending may move the chain in memory */
      POINT2D head = chain.points[0], tail = chain.points[chain.count - 1];
      /* The walk took the lowest-numbered piece meeting EITHER end and tried
       * the head first, so the index is asked at both and the lower answer
       * wins, the head keeping it where the two are the same piece */
      int at_head = linear_end_index_at(&ix, used, head);
      int at_tail = linear_end_index_at(&ix, used, tail);
      int p;
      if (at_head >= 0 && (at_tail < 0 || at_head <= at_tail))
        p = at_head;
      else
        p = at_tail;
      if (p < 0)
        break;
      const POINTARRAY *ppa = ((const LWLINE *) pieces[p])->points;
      const POINT2D *first = getPoint2d_cp(ppa, 0);
      const POINT2D *last = getPoint2d_cp(ppa, ppa->npoints - 1);
      if (linear_union_same_point(&head, first) ||
          linear_union_same_point(&head, last))
        linear_chain_reverse(&chain);
      POINT2D end = chain.points[chain.count - 1];
      if (linear_union_same_point(&end, first))
        for (uint32_t i = 1; i < ppa->npoints; i++)
          linear_chain_append(&chain, getPoint2d_cp(ppa, i));
      else
        for (int i = (int) ppa->npoints - 2; i >= 0; i--)
          linear_chain_append(&chain, getPoint2d_cp(ppa, i));
      used[p] = true; attached = true;
    }

    POINTARRAY *pa = ptarray_construct_empty(0, 0, (uint32_t) chain.count);
    POINT4D p4;
    memset(&p4, 0, sizeof(POINT4D));
    for (int i = 0; i < chain.count; i++)
    {
      p4.x = chain.points[i].x;
      p4.y = chain.points[i].y;
      ptarray_append_point(pa, &p4, LW_TRUE);
    }
    pfree(chain.points);
    out[nout++] = lwline_as_lwgeom(lwline_construct(srid, NULL, pa));
  }
  pfree(used);
  linear_end_index_free(&ix);
  if (nout == 0)
  {
    pfree(out);
    return 0;
  }
  *result = out;
  return nout;
}

/**
 * @brief Return the union of two geometries whose linework coincides over a
 * curve, as the single line it draws
 * @details Two surfaces whose interiors meet become ONE surface under
 * #meos_areal_union(), and a pair of curves sharing more than a finite set of
 * points is the same question one dimension down. The union is read as the
 * first geometry plus the parts of the second it does not already cover: each
 * segment of @p geom2 keeps the parameters no segment of @p geom1 overlaps,
 * and #linear_union_chain() sews what remains onto the curve it
 * continues
 * @param[in] geom1,geom2 Geometries
 * @param[out] curves The curves the union draws, which the caller owns
 * @return How many curves it takes, MORE THAN ONE where the two fork -- a
 * stretch they share with a branch leaving it is two curves and no single one,
 * and a component stands for more than one where it needs to, as the dissolve
 * of a doubled walk already does -- or 0 where either geometry carries an edge
 * that is not a straight segment, which this does not answer
 */
static int
linear_union_merge(const LWGEOM *geom1, const LWGEOM *geom2,
  LWGEOM ***curves)
{
  assert(geom1); assert(geom2); assert(curves);
  *curves = NULL;
  MeosArray *e1 = geom_extract_edges(geom1);
  MeosArray *e2 = geom_extract_edges(geom2);
  if (! e1 || ! e2 || ! linear_union_straight(e1) || ! linear_union_straight(e2))
  {
    if (e1) meos_array_destroy(e1);
    if (e2) meos_array_destroy(e2);
    return 0;
  }
  int n1 = (int) e1->count, n2 = (int) e2->count;
  int32_t srid = lwgeom_get_srid(geom1);

  /* Every segment of the second geometry contributes the parts of itself the
   * first does not cover, so at most one piece more than the overlaps it
   * carries */
  LWGEOM **pieces = palloc(sizeof(LWGEOM *) * (size_t) (n2 * (n1 + 1) + 1));
  LinearStretch *cover = palloc(sizeof(LinearStretch) * (size_t) (n1 + 1));
  int npieces = 0;
  for (int j = 0; j < n2; j++)
    npieces += linear_union_uncovered((const Edge *) meos_array_get_intl(e2, j),
      e1, NULL, n1, cover, &pieces[npieces], srid);
  pfree(cover);
  meos_array_destroy(e1); meos_array_destroy(e2);

  /* The first curve seeds the sewing, so the pieces attach to it in the order
   * it is walked */
  LWGEOM **all = palloc(sizeof(LWGEOM *) * (size_t) (npieces + 1));
  all[0] = lwgeom_clone_deep(geom1);
  for (int p = 0; p < npieces; p++)
    all[p + 1] = pieces[p];
  pfree(pieces);
  int ncurves = linear_union_chain_all(all, npieces + 1, srid, curves);
  for (int p = 0; p <= npieces; p++)
    lwgeom_free(all[p]);
  pfree(all);
  /* What the two draw together is however many curves the sewing takes: one
   * where they continue into each other, and several where they fork */
  return ncurves;
}

/**
 * @brief Return the curves a sequence of straight edges draws, walking each of
 * their points once
 * @details A union is a POINT SET, so the measure of the answer is the measure
 * of that set: a line walking a stretch twice -- @p LINESTRING(6 5,6 3,6 4)
 * traverses @p (6 3)-(6 4) on the way down and again on the way back -- reports
 * a length larger than the set it covers, and so do two lines drawing one
 * stretch between them. Each edge keeps only what no EARLIER edge of the
 * sequence already covers (#linear_union_uncovered), and what remains is sewn
 * into the curves it draws (#linear_union_chain_all), several where it forks.
 * Every end of a piece is an input vertex, so the answer holds no constructed
 * coordinate
 * @param[in] edges Straight edges, in the order they are read
 * @param[in] srid Spatial reference identifier
 * @param[out] curves Curves drawn, allocated here and owned by the caller
 * @return Number of curves
 */
static int
linear_union_dissolve_edges(const MeosArray *edges, int32_t srid,
  LWGEOM ***curves)
{
  assert(edges); assert(curves);
  int nedges = (int) edges->count;
  /* An edge covers a stretch of another only where their boxes meet, so the
   * edges read for each one are the EARLIER ones an index over the boxes
   * answers, rather than all of them */
  RTree *rtree = rtree_create_stbox();
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = (const Edge *) meos_array_get_intl(edges, i);
    STBox box;
    stbox_set(true, false, false, 0, e->xmin, e->xmax, e->ymin, e->ymax, 0, 0,
      NULL, &box);
    rtree_insert(rtree, &box, i);
  }
  MeosArray *found = index_result_create();
  int *which = palloc(sizeof(int) * (size_t) (nedges + 1));
  LinearStretch *cover = palloc(sizeof(LinearStretch) * (size_t) (nedges + 1));
  int maxpieces = nedges + 1, npieces = 0;
  LWGEOM **pieces = palloc(sizeof(LWGEOM *) * (size_t) maxpieces);
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = (const Edge *) meos_array_get_intl(edges, i);
    STBox query;
    stbox_set(true, false, false, 0, e->xmin, e->xmax, e->ymin, e->ymax, 0, 0,
      NULL, &query);
    meos_array_reset(found);
    int nfound = rtree_search_intl(rtree, INDEX_OVERLAPS, &query, found);
    int nwhich = 0;
    for (int f = 0; f < nfound; f++)
    {
      int j = (int) INDEX_RESULT_ID_N(found, (uint32_t) f);
      if (j < i)
        which[nwhich++] = j;
    }
    /* An edge adds at most one piece more than the stretches covering it */
    if (npieces + nwhich + 1 > maxpieces)
    {
      maxpieces = Max(2 * maxpieces, npieces + nwhich + 1);
      pieces = repalloc(pieces, sizeof(LWGEOM *) * (size_t) maxpieces);
    }
    npieces += linear_union_uncovered(e, edges, which, nwhich, cover,
      &pieces[npieces], srid);
  }
  pfree(cover); pfree(which);
  meos_array_destroy(found);
  rtree_free(rtree);

  int ncurves = linear_union_chain_all(pieces, npieces, srid, curves);
  for (int p = 0; p < npieces; p++)
    lwgeom_free(pieces[p]);
  pfree(pieces);
  return ncurves;
}

/**
 * @ingroup meos_internal_geo_base_spatial
 * @brief Return the union of the linear and point components of a geometry
 * @details The components are read as the point set they cover: a component
 * another one covers is left out, and what remains is the union.  The rule is
 * the one #meos_areal_union() applies to surfaces -- a pair sharing more than a
 * finite set of points becomes one, a pair meeting at points only stays apart
 * -- so a circular arc is kept on its own circle where a linearization would
 * replace it by the chain of segments approximating it
 * @param[in] geom Geometry
 * @return The union, or @p NULL where the geometry carries a component that is
 * not linear or a point, and where a coincident pair draws a fork rather than
 * a single curve, which leaves the caller to answer it another way
 */
LWGEOM *
meos_linear_union(const LWGEOM *geom)
{
  assert(geom);
  if (lwgeom_is_empty(geom))
    return NULL;

  /* A point is its own union */
  if (geom->type == POINTTYPE)
    return lwgeom_clone_deep(geom);
  if (! linear_union_type(geom->type) && geom->type != COLLECTIONTYPE)
    return NULL;

  int maxcomp = 8, ncomp = 0;
  const LWGEOM **comps = palloc(sizeof(LWGEOM *) * maxcomp);
  if (! linear_union_collect(geom, &comps, &ncomp, &maxcomp) || ncomp == 0)
  {
    pfree(comps);
    return NULL;
  }
  /* A single component is its own union, except a line of straight segments,
   * which may walk a stretch of itself twice and is dissolved below like the
   * lines of several components */
  if (ncomp == 1 && comps[0]->type != LINETYPE)
  {
    LWGEOM *result = lwgeom_clone_deep(comps[0]);
    pfree(comps);
    return result;
  }

  /* The live component of each slot: the one collected, or the curve a merge
   * put in its place, which this function owns and frees */
  /* A component walking a stretch of itself twice covers it once, and the
   * measure of the answer is the measure of what it covers. Dropping the
   * doubled stretch can leave what remains in SEVERAL curves, so a component
   * may stand for more than one, and the list grows to hold them */
  /* The lines of straight segments are dissolved TOGETHER, as one sequence of
   * edges read component after component: each edge keeps what no earlier edge
   * of ANY line covers. Where two lines share a stretch that is what their
   * union draws, however it forks -- a fork is several curves, not a refusal --
   * and it is decided on input vertices alone. A line with no segment of any
   * length, a point and a curve carrying an arc are kept as they stand */
  int maxcur = ncomp + 1;
  const LWGEOM **cur = palloc(sizeof(LWGEOM *) * maxcur);
  LWGEOM **owned = palloc0(sizeof(LWGEOM *) * maxcur);
  bool *together = palloc0(sizeof(bool) * maxcur);
  int ncur = 0;
  MeosArray *lineedges = meos_array_create(sizeof(Edge));
  for (int i = 0; i < ncomp; i++)
  {
    MeosArray *edges = (comps[i]->type == LINETYPE) ?
      geom_extract_edges(comps[i]) : NULL;
    bool straight = edges && linear_union_straight(edges);
    bool haslength = false;
    for (int e = 0; straight && e < (int) edges->count && ! haslength; e++)
    {
      const Edge *edge = (const Edge *) meos_array_get_intl(edges, e);
      haslength = edge->dx != 0.0 || edge->dy != 0.0;
    }
    if (straight && haslength)
    {
      for (int e = 0; e < (int) edges->count; e++)
        meos_array_add(lineedges, meos_array_get_intl(edges, e));
      meos_array_destroy(edges);
      continue;
    }
    if (edges)
      meos_array_destroy(edges);
    owned[ncur] = NULL;
    cur[ncur++] = comps[i];
  }
  if (lineedges->count > 0)
  {
    LWGEOM **curves = NULL;
    int ncurves = linear_union_dissolve_edges(lineedges,
      lwgeom_get_srid(geom), &curves);
    for (int c = 0; c < ncurves; c++)
    {
      if (ncur == maxcur)
      {
        maxcur *= 2;
        cur = repalloc(cur, sizeof(LWGEOM *) * (size_t) maxcur);
        owned = repalloc(owned, sizeof(LWGEOM *) * (size_t) maxcur);
        together = repalloc(together, sizeof(bool) * (size_t) maxcur);
      }
      owned[ncur] = curves[c];
      together[ncur] = true;
      cur[ncur++] = curves[c];
    }
    if (curves)
      pfree(curves);
  }
  meos_array_destroy(lineedges);
  ncomp = ncur;
  GBOX *boxes = palloc(sizeof(GBOX) * ncomp);
  bool *hasbox = palloc(sizeof(bool) * ncomp);
  bool *dropped = palloc0(sizeof(bool) * ncomp);
  bool declined = false;

  /* A pair whose linework coincides over a curve becomes ONE component, the
   * rule #meos_areal_union() applies to two surfaces whose interiors meet.
   * The merged curve may in turn meet a component already passed, so the scan
   * restarts on it -- and it terminates, every merge retiring one component */
  bool again = true;
  while (again && ! declined)
  {
    again = false;
    /* The extent of each live component, read once per scan. Two components
     * whose extents do not meet share nothing at all, so neither covers the
     * other and the two cannot coincide over a curve. The DE-9IM walk is
     * quadratic in the vertices of the pair while the extent test is a handful
     * of comparisons, so the pairs the extents separate are never walked */
    for (int i = 0; i < ncomp; i++)
      hasbox[i] = dropped[i] ? false :
        (lwgeom_calculate_gbox(cur[i], &boxes[i]) == LW_SUCCESS);

    for (int i = 0; i < ncomp && ! again && ! declined; i++)
    {
      if (dropped[i])
        continue;
      for (int j = i + 1; j < ncomp; j++)
      {
        if (dropped[j])
          continue;
        /* Two curves of the one dissolve walk each of their points once
         * between them, so neither covers the other nor shares a stretch */
        if (together[i] && together[j])
          continue;
        if (hasbox[i] && hasbox[j] &&
            gbox_overlaps_2d(&boxes[i], &boxes[j]) == LW_FALSE)
          continue;
        /* ONE matrix answers all three questions the pair raises: the ordered
         * pair, its reverse (the transpose) and what the two interiors share */
        char matrix[10], reversed[10];
        if (! meos_relate(cur[i], cur[j], matrix))
          continue;
        /* A component another one covers contributes nothing of its own */
        if (linear_union_covers(matrix))
        {
          dropped[j] = true;
          continue;
        }
        linear_union_transpose(matrix, reversed);
        if (linear_union_covers(reversed))
        {
          dropped[i] = true;
          break;
        }
        /* The interior/interior cell is the dimension of what the two share:
         * @p 1 where they share a curve, @p 0 where they share only points */
        if (matrix[0] == '1')
        {
          LWGEOM **merged = NULL;
          int nmerged = linear_union_merge(cur[i], cur[j], &merged);
          if (nmerged == 0)
          {
            declined = true;
            break;
          }
          /* The pair becomes the curves their union draws: the first takes
           * the slot they merged from, and a fork's other curves take slots
           * of their own, which is what the dissolve of a doubled walk does
           * with the curves it leaves */
          if (owned[i])
            lwgeom_free(owned[i]);
          owned[i] = merged[0];
          cur[i] = merged[0];
          together[i] = false;
          dropped[j] = true;
          for (int m = 1; m < nmerged; m++)
          {
            if (ncomp == maxcur)
            {
              maxcur *= 2;
              cur = repalloc(cur, sizeof(LWGEOM *) * (size_t) maxcur);
              owned = repalloc(owned, sizeof(LWGEOM *) * (size_t) maxcur);
              together = repalloc(together, sizeof(bool) * (size_t) maxcur);
              boxes = repalloc(boxes, sizeof(GBOX) * (size_t) maxcur);
              hasbox = repalloc(hasbox, sizeof(bool) * (size_t) maxcur);
              dropped = repalloc(dropped, sizeof(bool) * (size_t) maxcur);
            }
            owned[ncomp] = merged[m];
            cur[ncomp] = merged[m];
            together[ncomp] = false;
            hasbox[ncomp] = false;
            dropped[ncomp] = false;
            ncomp++;
          }
          pfree(merged);
          again = true;
          break;
        }
      }
    }
  }

  if (declined)
  {
    for (int i = 0; i < ncomp; i++)
      if (owned[i])
        lwgeom_free(owned[i]);
    pfree(owned); pfree(cur); pfree(together); pfree(boxes); pfree(hasbox);
    pfree(dropped);
    pfree(comps);
    return NULL;
  }
  pfree(boxes); pfree(hasbox);

  int nkept = 0;
  for (int i = 0; i < ncomp; i++)
    if (! dropped[i])
      nkept++;
  if (nkept == 1)
  {
    LWGEOM *result = NULL;
    for (int i = 0; i < ncomp; i++)
      if (! dropped[i])
        result = lwgeom_clone_deep(cur[i]);
    for (int i = 0; i < ncomp; i++)
      if (owned[i])
        lwgeom_free(owned[i]);
    pfree(owned); pfree(cur); pfree(together); pfree(dropped); pfree(comps);
    return result;
  }

  /* The collection carries the type its members do: a set of points is a
   * multipoint, a set of line strings a multiline, a set carrying a circular
   * arc a multicurve -- a multiline cannot hold one -- and a mixture the
   * general collection.  A mixture lists its points before its curves, the
   * order a collection is read in */
  bool allpoint = true, allline = true, allcurve = true;
  for (int i = 0; i < ncomp; i++)
  {
    if (dropped[i])
      continue;
    uint8_t comp = cur[i]->type;
    if (comp != POINTTYPE)
      allpoint = false;
    if (comp != LINETYPE)
      allline = false;
    if (comp == POINTTYPE)
      allcurve = false;
  }
  LWGEOM **kept = palloc(sizeof(LWGEOM *) * nkept);
  int k = 0;
  /* A curve the dissolve builds is read from EDGES, which carry no ordinate,
   * so it draws the plane where a component kept as it stands carries the
   * ordinates it came with. The answer is ONE figure, of one dimension
   * throughout: where the two meet it is the planar one, and the ordinates
   * its inputs determine are read back onto it by #meos_lift_ordinates(),
   * the post-pass every caller of this applies */
  bool onplane = false, withordinates = false;
  for (int i = 0; i < ncomp; i++)
  {
    if (dropped[i])
      continue;
    if (FLAGS_GET_Z(cur[i]->flags) || FLAGS_GET_M(cur[i]->flags))
      withordinates = true;
    else
      onplane = true;
  }
  bool toplane = onplane && withordinates;
  for (int pass = 0; pass < 2; pass++)
    for (int i = 0; i < ncomp; i++)
    {
      if (dropped[i])
        continue;
      bool ispoint = cur[i]->type == POINTTYPE;
      if ((pass == 0) != ispoint)
        continue;
      kept[k++] = toplane ? lwgeom_force_2d(cur[i]) :
        lwgeom_clone_deep(cur[i]);
    }
  for (int i = 0; i < ncomp; i++)
    if (owned[i])
      lwgeom_free(owned[i]);
  pfree(owned); pfree(cur); pfree(together); pfree(dropped); pfree(comps);
  uint8_t colltype = allpoint ? MULTIPOINTTYPE :
    (allline ? MULTILINETYPE :
      (allcurve ? MULTICURVETYPE : COLLECTIONTYPE));
  /* #lwcollection_construct() takes ownership of the array it is given */
  LWCOLLECTION *result = lwcollection_construct(colltype,
    lwgeom_get_srid(geom), NULL, (uint32_t) nkept, kept);
  return lwcollection_as_lwgeom(result);
}

/*****************************************************************************
 * Lifting a planar answer back to the ordinates its inputs carry
 *****************************************************************************/

/**
 * @brief Structure keeping what the inputs of an overlay determine at one
 * point of its answer
 */
typedef struct
{
  bool found;      /**< An input determines the ordinates there */
  bool zconflict;  /**< Two inputs determine different elevations there */
  bool mconflict;  /**< Two inputs determine different measures there */
  double z;        /**< Elevation the inputs determine */
  double m;        /**< Measure the inputs determine */
} LiftPoint;

/**
 * @brief Return the angle swept from one angle to another in a direction
 * @param[in] theta0,theta1 Angles of the ends of the sweep
 * @param[in] ccw True where the sweep runs counterclockwise
 */
static double
arc_angle_swept(double theta0, double theta1, bool ccw)
{
  return angle_normalize(ccw ? theta1 - theta0 : theta0 - theta1);
}

/**
 * @brief Return the ordinates an arc determines at a point of it
 * @details A geometry carrying a Z or an M is a planar figure together with a
 * LIFT: the ordinates are given at the vertices and read between them along
 * the curve joining them, which for an arc is read by ANGLE rather than by the
 * chord. The middle control point carries its own ordinates, so the lift is
 * linear over each of the two halves of the sweep
 * @param[in] p0,p1,p2 Control points of the arc
 * @param[in] px,py Point
 * @param[out] z,m Ordinates the arc determines there
 * @return True if the point lies on the arc, which is what determines them
 */
static bool
arc_ordinates_at(const POINT4D *p0, const POINT4D *p1, const POINT4D *p2,
  double px, double py, double *z, double *m)
{
  assert(p0); assert(p1); assert(p2); assert(z); assert(m);
  POINT2D a = { p0->x, p0->y }, b = { p1->x, p1->y }, c = { p2->x, p2->y };
  POINT2D centre;
  double radius = lw_arc_center(&a, &b, &c, &centre);
  /* Three collinear points draw a segment, which the caller reads as one */
  if (radius < 0)
    return false;

  /* The point lies on the circle */
  double tol = fmax(
    coordinate_tolerance(centre.x - radius, centre.x + radius),
    coordinate_tolerance(centre.y - radius, centre.y + radius));
  if (fabs(hypot(px - centre.x, py - centre.y) - radius) > tol)
    return false;

  /* The sweep runs from the first control point through the second, which is
   * the direction the three of them turn in */
  bool ccw = cross_product(&a, &b, &c) > 0;
  double t0 = atan2(a.y - centre.y, a.x - centre.x);
  double total = arc_angle_swept(t0,
    atan2(c.y - centre.y, c.x - centre.x), ccw);
  double mid = arc_angle_swept(t0,
    atan2(b.y - centre.y, b.x - centre.x), ccw);
  double here = arc_angle_swept(t0, atan2(py - centre.y, px - centre.x), ccw);
  /* A full circle sweeps from a point back to itself, which reads as no sweep
   * at all: its two halves are what the middle control point separates */
  if (total <= 0)
    total = 2 * M_PI;
  /* The point is on the circle but outside the span the arc occupies */
  if (here > total + MEOS_GEOM_TOLERANCE)
    return false;

  double f = here / total, fmid = mid / total;
  if (fmid <= 0 || fmid >= 1)
  {
    /* The middle point determines nothing of its own, so the lift runs from
     * one end of the arc to the other */
    *z = p0->z + f * (p2->z - p0->z);
    *m = p0->m + f * (p2->m - p0->m);
    return true;
  }
  if (f <= fmid)
  {
    double g = f / fmid;
    *z = p0->z + g * (p1->z - p0->z);
    *m = p0->m + g * (p1->m - p0->m);
  }
  else
  {
    double g = (f - fmid) / (1 - fmid);
    *z = p1->z + g * (p2->z - p1->z);
    *m = p1->m + g * (p2->m - p1->m);
  }
  return true;
}

/**
 * @brief Read one pair of ordinates determined at a point into what is
 * already determined there
 * @details The first pair read gives the ordinates, and a pair differing from
 * them reports the conflict, which is separate for each of the two: two
 * surfaces meeting at one elevation may carry different measures
 * @param[in,out] state Ordinates determined at the point
 * @param[in] z,m Ordinates read
 */
static void
lift_point_take(LiftPoint *state, double z, double m)
{
  if (! state->found)
  {
    state->found = true;
    state->z = z; state->m = m;
    return;
  }
  if (fabs(z - state->z) > coordinate_tolerance(z, state->z))
    state->zconflict = true;
  if (fabs(m - state->m) > coordinate_tolerance(m, state->m))
    state->mconflict = true;
  return;
}

/**
 * @brief Read what a point array determines at a point into what the arrays
 * already read determine there
 * @details A vertex of the array determines its own ordinates, and a point
 * between two of them reads them along the curve that joins them. EVERY
 * vertex and every piece of the array passing through the point is read: a
 * curve may pass through one point of the plane twice at two elevations, and
 * it then determines no single one there
 * @param[in] pa Point array
 * @param[in] circular True where the array reads as arcs rather than segments
 * @param[in] px,py Point
 * @param[in,out] state Ordinates determined at the point
 */
static void
lift_point_read(const POINTARRAY *pa, bool circular, double px, double py,
  LiftPoint *state)
{
  assert(pa); assert(state);
  for (uint32_t i = 0; i < pa->npoints; i++)
  {
    POINT4D p;
    getPoint4d_p(pa, i, &p);
    if (fabs(p.x - px) <= coordinate_tolerance(p.x, px) &&
        fabs(p.y - py) <= coordinate_tolerance(p.y, py))
      lift_point_take(state, p.z, p.m);
  }

  double z, m;
  if (circular)
  {
    /* An arc is read from three points, the last of one being the first of the
     * next */
    for (uint32_t i = 0; i + 2 < pa->npoints; i += 2)
    {
      POINT4D p0, p1, p2;
      getPoint4d_p(pa, i, &p0);
      getPoint4d_p(pa, i + 1, &p1);
      getPoint4d_p(pa, i + 2, &p2);
      if (arc_ordinates_at(&p0, &p1, &p2, px, py, &z, &m))
        lift_point_take(state, z, m);
    }
    return;
  }

  for (uint32_t i = 0; i + 1 < pa->npoints; i++)
  {
    POINT4D p0, p1;
    getPoint4d_p(pa, i, &p0);
    getPoint4d_p(pa, i + 1, &p1);
    if (! point_on_segment(px, py, p0.x, p0.y, p1.x, p1.y))
      continue;
    POINT2D q = { px, py }, a = { p0.x, p0.y }, b = { p1.x, p1.y }, closest;
    double f = (double) closest_point2d_on_segment_ratio(&q, &a, &b, &closest);
    lift_point_take(state, p0.z + f * (p1.z - p0.z), p0.m + f * (p1.m - p0.m));
  }
  return;
}

/**
 * @brief Read the ordinates a geometry determines at a point into what the
 * geometries read before it determined
 * @details Where two of them determine DIFFERENT ordinates at one point --
 * two surfaces crossing at different elevations -- the point has none that the
 * inputs give, and a value chosen between them would be one no input carries.
 * The conflict is reported rather than resolved
 * @param[in] geom Geometry
 * @param[in] px,py Point
 * @param[in,out] state Ordinates the geometries already read determine there,
 * which this one adds what it determines to
 */
static void
geom_ordinates_at(const LWGEOM *geom, double px, double py,
  LiftPoint *state)
{
  assert(geom); assert(state);
  const POINTARRAY *pa = NULL;
  bool circular = false;
  switch (geom->type)
  {
    case POINTTYPE:
      pa = ((const LWPOINT *) geom)->point;
      break;
    case LINETYPE:
      pa = ((const LWLINE *) geom)->points;
      break;
    case CIRCSTRINGTYPE:
      pa = ((const LWCIRCSTRING *) geom)->points;
      circular = true;
      break;
    case TRIANGLETYPE:
      pa = ((const LWTRIANGLE *) geom)->points;
      break;
    case POLYGONTYPE:
    {
      const LWPOLY *poly = (const LWPOLY *) geom;
      for (uint32_t i = 0; i < poly->nrings; i++)
        lift_point_read(poly->rings[i], false, px, py, state);
      return;
    }
    /* ⛔ A CURVEPOLYGON and a COMPOUNDCURVE hold their rings and their pieces
     * in @p geoms, and each of those is a geometry of its own that says
     * whether it reads as arcs. They are walked HERE rather than through
     * #lwgeom_is_collection(), which answers true for both */
    case CURVEPOLYTYPE:
    case COMPOUNDTYPE:
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTICURVETYPE:
    case MULTIPOLYGONTYPE:
    case MULTISURFACETYPE:
    case POLYHEDRALSURFACETYPE:
    case TINTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
        geom_ordinates_at(coll->geoms[i], px, py, state);
      return;
    }
    default:
      return;
  }
  lift_point_read(pa, circular, px, py, state);
  return;
}

/**
 * @brief Collect the point arrays of a geometry in the order a walk of it
 * visits them
 * @details Two geometries drawing the same figure list the same arrays with
 * the same number of points in the same order, which is what lets the
 * ordinates read for the vertices of one be written to the other
 * @param[in] geom Geometry
 * @param[in,out] arrays Array of point arrays
 */
static void
geom_ptarrays(const LWGEOM *geom, MeosArray *arrays)
{
  assert(geom); assert(arrays);
  POINTARRAY *pa = NULL;
  switch (geom->type)
  {
    case POINTTYPE:
      pa = ((const LWPOINT *) geom)->point;
      break;
    case LINETYPE:
      pa = ((const LWLINE *) geom)->points;
      break;
    case CIRCSTRINGTYPE:
      pa = ((const LWCIRCSTRING *) geom)->points;
      break;
    case TRIANGLETYPE:
      pa = ((const LWTRIANGLE *) geom)->points;
      break;
    case POLYGONTYPE:
    {
      const LWPOLY *poly = (const LWPOLY *) geom;
      for (uint32_t i = 0; i < poly->nrings; i++)
        meos_array_add(arrays, &poly->rings[i]);
      return;
    }
    /* ⛔ The rings of a CURVEPOLYGON and the pieces of a COMPOUNDCURVE are
     * geometries in @p geoms, walked here rather than through
     * #lwgeom_is_collection(), which answers true for both */
    case CURVEPOLYTYPE:
    case COMPOUNDTYPE:
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTICURVETYPE:
    case MULTIPOLYGONTYPE:
    case MULTISURFACETYPE:
    case POLYHEDRALSURFACETYPE:
    case TINTYPE:
    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
      for (uint32_t i = 0; i < coll->ngeoms; i++)
        geom_ptarrays(coll->geoms[i], arrays);
      return;
    }
    default:
      return;
  }
  meos_array_add(arrays, &pa);
  return;
}

/**
 * @ingroup meos_internal_geo_base_spatial
 * @brief Return a planar answer carrying the ordinates its inputs determine
 * @details A union, a difference or an intersection is computed on the PLANE,
 * while the Z and M its inputs carry are a lift of that plane: the ordinates
 * are given at the vertices of the inputs and read between them along the
 * curve joining them. Every vertex of such an answer lies on the boundary of
 * an input -- the boundary of a union is part of the boundaries it is built
 * from -- so the inputs determine an ordinate there, EXCEPT where two of them
 * determine different ones, as two surfaces crossing at different elevations
 * do. The answer therefore carries an ordinate exactly where the lift is
 * unique, and is the plain planar figure where it is not: a value chosen
 * between two the inputs carry, such as the mean, is one no input holds.
 * @param[in] geom Answer of a planar overlay
 * @param[in] geoms Geometries the answer is read from
 * @param[in] count Number of geometries, at least one
 * @return A new geometry carrying the ordinates the inputs determine
 * @note The two ordinates are decided one by one: an answer whose elevations
 * agree keeps them where its measures conflict
 */
LWGEOM *
meos_lift_ordinates(const LWGEOM *geom, const LWGEOM **geoms, int count)
{
  assert(geom); assert(geoms); assert(count > 0);
  /* The answer carries what the inputs do, and the inputs share it */
  bool hasz = FLAGS_GET_Z(geoms[0]->flags);
  bool hasm = FLAGS_GET_M(geoms[0]->flags);
  if (! hasz && ! hasm)
    return lwgeom_force_2d(geom);

  /* The vertices of the answer, read in the order a walk of it visits them */
  MeosArray *arrays = meos_array_create(sizeof(POINTARRAY *));
  geom_ptarrays(geom, arrays);
  int nvertices = 0;
  for (int i = 0; i < meos_array_count(arrays); i++)
    nvertices +=
      (int) (*(POINTARRAY **) meos_array_get_intl(arrays, i))->npoints;

  /* The extent of each geometry, read once for the whole walk. A vertex
   * outside one lies on nothing it draws, so the geometries a handful of
   * comparisons rule out are never walked for it */
  GBOX *boxes = palloc(sizeof(GBOX) * count);
  bool *hasbox = palloc(sizeof(bool) * count);
  for (int i = 0; i < count; i++)
    hasbox[i] = (lwgeom_calculate_gbox(geoms[i], &boxes[i]) == LW_SUCCESS);

  LiftPoint *lifted = palloc(sizeof(LiftPoint) * Max(nvertices, 1));
  bool zok = hasz, mok = hasm;
  int nvertex = 0;
  for (int i = 0; i < meos_array_count(arrays); i++)
  {
    const POINTARRAY *pa = *(POINTARRAY **) meos_array_get_intl(arrays, i);
    for (uint32_t j = 0; j < pa->npoints; j++)
    {
      POINT4D p;
      getPoint4d_p(pa, j, &p);
      LiftPoint *state = &lifted[nvertex++];
      state->found = false; state->zconflict = false; state->mconflict = false;
      state->z = state->m = 0;
      for (int k = 0; k < count; k++)
      {
        /* The extent is grown by the tolerance the coordinates call for, so a
         * vertex ON the boundary of one is read rather than ruled out */
        if (hasbox[k])
        {
          double tol = fmax(coordinate_tolerance(boxes[k].xmin, boxes[k].xmax),
            coordinate_tolerance(boxes[k].ymin, boxes[k].ymax));
          if (p.x < boxes[k].xmin - tol || p.x > boxes[k].xmax + tol ||
              p.y < boxes[k].ymin - tol || p.y > boxes[k].ymax + tol)
            continue;
        }
        geom_ordinates_at(geoms[k], p.x, p.y, state);
      }
      /* A vertex no input determines an ordinate for leaves the answer with
       * none, as a conflict does: the answer is a figure of one dimension or
       * of another, never of one at some of its vertices */
      if (! state->found || state->zconflict)
        zok = false;
      if (! state->found || state->mconflict)
        mok = false;
    }
  }
  meos_array_destroy(arrays);
  pfree(boxes); pfree(hasbox);

  LWGEOM *result = lwgeom_force_dims(geom, zok ? 1 : 0, mok ? 1 : 0, 0, 0);
  if (! zok && ! mok)
  {
    pfree(lifted);
    return result;
  }

  /* The forced answer draws the same figure, so its vertices are visited in
   * the same order and take the ordinates read for them */
  MeosArray *outarrays = meos_array_create(sizeof(POINTARRAY *));
  geom_ptarrays(result, outarrays);
  nvertex = 0;
  for (int i = 0; i < meos_array_count(outarrays); i++)
  {
    POINTARRAY *pa = *(POINTARRAY **) meos_array_get_intl(outarrays, i);
    for (uint32_t j = 0; j < pa->npoints; j++)
    {
      POINT4D p;
      getPoint4d_p(pa, j, &p);
      if (zok)
        p.z = lifted[nvertex].z;
      if (mok)
        p.m = lifted[nvertex].m;
      ptarray_set_point4d(pa, j, &p);
      nvertex++;
    }
  }
  meos_array_destroy(outarrays);
  pfree(lifted);
  return result;
}

/*****************************************************************************/

/*****************************************************************************
 * The centroid of a geometry
 *
 * The centroid is a weighted mean of the points a geometry is made of, so it
 * needs no overlay engine: an area, a length and a count are the whole of it.
 * Routing it through GEOS costs the arcs their exactness -- GEOS reads a
 * circular string by stroking it -- and costs the answer its ordinates,
 * because the centroid GEOS returns is a bare point in the plane.
 *
 * The weights are the measure of the piece each mean is taken over, and the
 * piece is chosen by dimension: a geometry holding any area is the mean of
 * its areas, one holding only curves is the mean of their lengths, and one
 * holding only points is the mean of the points. A piece of zero measure
 * carries no weight, so a shape that encloses nothing falls back to the mean
 * of what it does hold.
 *****************************************************************************/

/**
 * @brief The running mean a centroid is accumulated into
 * @details Every ordinate is summed with the SAME weight, so the answer is the
 * centroid of the point set in as many dimensions as the input states. The
 * weight is an area, a length or a count according to what is being measured.
 */
typedef struct
{
  double weight;    /**< total measure accumulated */
  double x, y;      /**< coordinates, each weighted by @p weight */
  double z, m;      /**< ordinates, weighted the same way */
} CentroidAcc;

/**
 * @brief Add a piece of the given measure, centred at the given point
 */
static void
centroid_add(CentroidAcc *acc, double weight, const POINT4D *p)
{
  acc->weight += weight;
  acc->x += weight * p->x;
  acc->y += weight * p->y;
  acc->z += weight * p->z;
  acc->m += weight * p->m;
  return;
}

/**
 * @brief Return the mean accumulated so far, false when nothing carries weight
 */
static bool
centroid_mean(const CentroidAcc *acc, POINT4D *result)
{
  if (acc->weight == 0.0)
    return false;
  result->x = acc->x / acc->weight;
  result->y = acc->y / acc->weight;
  result->z = acc->z / acc->weight;
  result->m = acc->m / acc->weight;
  return true;
}

/**
 * @brief Return the centre and the angles an arc through three points spans
 * @details The circumcentre is measured FROM THE MIDDLE POINT for the reason
 * #emit_arc_edge states: reading it from the squares of projected coordinates
 * loses the arc in the cancellation. Answers false when the three points are
 * collinear, which is a straight segment rather than an arc.
 */
static bool
arc_geometry(const POINT4D *p1, const POINT4D *p2, const POINT4D *p3,
  double *cx, double *cy, double *radius, double *sweep)
{
  /* Ends at one point: the whole circle through the middle, whose diameter
   * joins the two, as PostGIS reads such an arc; a full turn has no sense
   * of its own, and a hole is told by its place in the surface */
  if (p1->x == p3->x && p1->y == p3->y)
  {
    *cx = 0.5 * (p1->x + p2->x);
    *cy = 0.5 * (p1->y + p2->y);
    *radius = 0.5 * hypot(p2->x - p1->x, p2->y - p1->y);
    *sweep = 2 * M_PI;
    return *radius > 0.0;
  }
  /* The arc turns counterclockwise where the three points turn left, and
   * three points on one line, both decided exactly, are a straight segment */
  int turn = cross_product_sign(p1->x, p1->y, p2->x, p2->y, p1->x, p1->y,
    p3->x, p3->y);
  if (turn == 0)
    return false;
  double ox = p2->x, oy = p2->y;
  double ax = p1->x - ox, ay = p1->y - oy;
  double bx = p3->x - ox, by = p3->y - oy;
  double d = 2 * (ax * (0 - by) + 0 * (by - ay) + bx * (ay - 0));
  /* Points so nearly on one line that the doubles place no centre draw a
   * circle wider than they can state, its segment no area they can either */
  if (d == 0.0)
    return false;

  double asq = ax * ax + ay * ay, bsq = bx * bx + by * by;
  double ux = (ay * bsq - by * asq) / d;
  double uy = (bx * asq - ax * bsq) / d;
  *cx = ux + ox;
  *cy = uy + oy;
  *radius = hypot(ux, uy);
  if (! isfinite(*radius) || *radius == 0.0)
    return false;

  /* The angle from the radius to p1 to the one to p3, read off their cross
   * and dot products, which keeps an arc of next to no sweep as small as it
   * is, then taken the way round the arc turns */
  double rx1 = p1->x - *cx, ry1 = p1->y - *cy;
  double rx3 = p3->x - *cx, ry3 = p3->y - *cy;
  double phi = atan2(rx1 * ry3 - ry1 * rx3, rx1 * rx3 + ry1 * ry3);
  if (turn > 0)
    *sweep = (phi > 0.0) ? phi : phi + 2 * M_PI;
  else
    *sweep = (phi < 0.0) ? phi : phi - 2 * M_PI;
  return true;
}

/**
 * @brief Add the length and the centre of mass of the arc through three points
 * @details The centre of mass of a circular arc lies on the bisector of the
 * angle it spans, at @p r*sin(h)/h from the centre for a half angle @p h.
 * The ordinates are read BY ANGLE, as [[the lift of an arc]] is everywhere in
 * this engine, so their mean over the arc is the mean of the two ends.
 */
static void
centroid_add_arc(CentroidAcc *acc, const POINT4D *p1, const POINT4D *p2,
  const POINT4D *p3)
{
  double cx, cy, radius, sweep;
  if (! arc_geometry(p1, p2, p3, &cx, &cy, &radius, &sweep))
  {
    /* Collinear: the two segments the three points draw */
    const POINT4D *pts[3] = {p1, p2, p3};
    for (int i = 0; i < 2; i++)
    {
      double len = hypot(pts[i + 1]->x - pts[i]->x, pts[i + 1]->y - pts[i]->y);
      POINT4D mid;
      mid.x = (pts[i]->x + pts[i + 1]->x) / 2;
      mid.y = (pts[i]->y + pts[i + 1]->y) / 2;
      mid.z = (pts[i]->z + pts[i + 1]->z) / 2;
      mid.m = (pts[i]->m + pts[i + 1]->m) / 2;
      centroid_add(acc, len, &mid);
    }
    return;
  }

  double half = fabs(sweep) / 2;
  double t1 = atan2(p1->y - cy, p1->x - cx);
  double bisector = t1 + sweep / 2;
  double dist = (half < MEOS_EPSILON) ? radius : radius * sin(half) / half;
  POINT4D com;
  com.x = cx + dist * cos(bisector);
  com.y = cy + dist * sin(bisector);
  com.z = (p1->z + p3->z) / 2;
  com.m = (p1->m + p3->m) / 2;
  centroid_add(acc, radius * fabs(sweep), &com);
  return;
}

/**
 * @brief Add every piece of a point array read as a curve
 * @param[in,out] acc Accumulator of lengths, NULL where the lengths are not
 * read (#meos_centroid)
 * @param[in] pa Point array
 * @param[in] arcs True when consecutive triples of points draw circular arcs
 */
static void
centroid_add_ptarray_line(CentroidAcc *acc, const POINTARRAY *pa, bool arcs)
{
  if (! acc)
    return;
  if (arcs)
  {
    for (uint32_t i = 0; i + 2 < pa->npoints; i += 2)
    {
      POINT4D p1, p2, p3;
      getPoint4d_p(pa, i, &p1);
      getPoint4d_p(pa, i + 1, &p2);
      getPoint4d_p(pa, i + 2, &p3);
      centroid_add_arc(acc, &p1, &p2, &p3);
    }
    return;
  }
  for (uint32_t i = 0; i + 1 < pa->npoints; i++)
  {
    POINT4D p1, p2, mid;
    getPoint4d_p(pa, i, &p1);
    getPoint4d_p(pa, i + 1, &p2);
    double len = hypot(p2.x - p1.x, p2.y - p1.y);
    mid.x = (p1.x + p2.x) / 2;
    mid.y = (p1.y + p2.y) / 2;
    mid.z = (p1.z + p2.z) / 2;
    mid.m = (p1.m + p2.m) / 2;
    centroid_add(acc, len, &mid);
  }
  return;
}

/**
 * @brief Add every point of a point array, each weighing one, unless the
 * accumulator is NULL, where the points are not read (#meos_centroid)
 */
static void
centroid_add_ptarray_points(CentroidAcc *acc, const POINTARRAY *pa)
{
  if (! acc)
    return;
  for (uint32_t i = 0; i < pa->npoints; i++)
  {
    POINT4D p;
    getPoint4d_p(pa, i, &p);
    centroid_add(acc, 1.0, &p);
  }
  return;
}

/**
 * @brief Add the area a ring encloses and the centre of mass of that area
 * @details The ring is read as a fan of triangles from its first point, which
 * keeps every coordinate the size of the ring rather than of the projection it
 * sits in. A triangle weighs its SIGNED area, so a hole -- wound the other way
 * -- subtracts both its area and its moment, and the ordinates ride the same
 * weights as the coordinates. An arc contributes the triangle its chord draws
 * plus the circular segment standing on that chord.
 * @param[in,out] acc Accumulator of areas
 * @param[in] pa Point array of the ring
 * @param[in] arcs True when consecutive triples of points draw circular arcs
 */
static void
centroid_add_ring(CentroidAcc *acc, const POINTARRAY *pa, bool arcs,
  const POINT4D *origin)
{
  if (pa->npoints < 2)
    return;
  POINT4D p0 = *origin;

  /* A straight ring in two dimensions reads its points in place
   * (#getPoint2d_cp) rather than copying four ordinates of each, the same
   * products summed in the same order */
  if (! arcs && ! FLAGS_GET_Z(pa->flags) && ! FLAGS_GET_M(pa->flags))
  {
    const POINT2D *pp = getPoint2d_cp(pa, 0);
    for (uint32_t i = 0; i + 1 < pa->npoints; i++)
    {
      const POINT2D *pn = getPoint2d_cp(pa, i + 1);
      double cross = (pp->x - p0.x) * (pn->y - p0.y) -
        (pn->x - p0.x) * (pp->y - p0.y);
      double w = cross / 2;
      acc->weight += w;
      acc->x += w * ((p0.x + pp->x + pn->x) / 3);
      acc->y += w * ((p0.y + pp->y + pn->y) / 3);
      pp = pn;
    }
    return;
  }

  /* The chord polygon, in which each arc stands as the segment joining its ends.
   * The fan runs from an origin the CALLER owns, so a ring assembled from
   * several pieces -- a compound curve -- fans from one point throughout
   * instead of once per piece, which is what makes its area come out whole */
  uint32_t step = arcs ? 2 : 1;
  for (uint32_t i = 0; i + step < pa->npoints; i += step)
  {
    POINT4D pa1, pa2;
    getPoint4d_p(pa, i, &pa1);
    getPoint4d_p(pa, i + step, &pa2);
    double cross = (pa1.x - p0.x) * (pa2.y - p0.y) -
      (pa2.x - p0.x) * (pa1.y - p0.y);
    POINT4D com;
    com.x = (p0.x + pa1.x + pa2.x) / 3;
    com.y = (p0.y + pa1.y + pa2.y) / 3;
    com.z = (p0.z + pa1.z + pa2.z) / 3;
    com.m = (p0.m + pa1.m + pa2.m) / 3;
    centroid_add(acc, cross / 2, &com);
  }
  if (! arcs)
    return;

  /* The circular segment each arc adds to, or takes from, its chord */
  for (uint32_t i = 0; i + 2 < pa->npoints; i += 2)
  {
    POINT4D p1, p2, p3;
    getPoint4d_p(pa, i, &p1);
    getPoint4d_p(pa, i + 1, &p2);
    getPoint4d_p(pa, i + 2, &p3);
    double cx, cy, radius, sweep;
    if (! arc_geometry(&p1, &p2, &p3, &cx, &cy, &radius, &sweep))
      continue;
    /* theta - sin(theta), which cancels for a small sweep, from its series
     * there; every arc adds its segment, however small */
    double theta = fabs(sweep);
    double t2 = theta * theta;
    double denom = (theta < 1e-2) ?
      theta * t2 * (1.0 / 6 - t2 * (1.0 / 120 - t2 / 5040)) :
      theta - sin(theta);
    if (denom == 0.0)
      continue;
    double area = radius * radius * denom / 2;
    double half = theta / 2;
    double dist = 4 * radius * sin(half) * sin(half) * sin(half) / (3 * denom);
    double t1 = atan2(p1.y - cy, p1.x - cx);
    double bisector = t1 + sweep / 2;
    POINT4D com;
    com.x = cx + dist * cos(bisector);
    com.y = cy + dist * sin(bisector);
    com.z = (p1.z + p3.z) / 2;
    com.m = (p1.m + p3.m) / 2;
    /* A segment standing on the left of its chord adds; one on the right,
     * which is the arc bulging into the ring, takes away */
    centroid_add(acc, (sweep > 0) ? area : -area, &com);
  }
  return;
}

/**
 * @brief Read the first point of a ring, which may be assembled from pieces
 */
static bool
ring_first_point(const LWGEOM *geom, POINT4D *result)
{
  if (! geom || lwgeom_is_empty(geom))
    return false;
  if (geom->type == LINETYPE)
  {
    getPoint4d_p(((const LWLINE *) geom)->points, 0, result);
    return true;
  }
  if (geom->type == CIRCSTRINGTYPE)
  {
    getPoint4d_p(((const LWCIRCSTRING *) geom)->points, 0, result);
    return true;
  }
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      if (ring_first_point(coll->geoms[i], result))
        return true;
  }
  return false;
}

/**
 * @brief Add the area a ring encloses, the ring being a line, a circular
 * string, or a compound curve assembled from both
 */
static void
centroid_add_curve_ring(CentroidAcc *acc, const LWGEOM *geom,
  const POINT4D *origin)
{
  if (! geom || lwgeom_is_empty(geom))
    return;
  if (geom->type == LINETYPE)
  {
    centroid_add_ring(acc, ((const LWLINE *) geom)->points, false, origin);
    return;
  }
  if (geom->type == CIRCSTRINGTYPE)
  {
    centroid_add_ring(acc, ((const LWCIRCSTRING *) geom)->points, true, origin);
    return;
  }
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      centroid_add_curve_ring(acc, coll->geoms[i], origin);
  }
  return;
}

/**
 * @brief Add every piece of a ring read as a curve, the ring being a line, a
 * circular string, or a compound curve assembled from both
 * @details Both accumulators NULL read nothing, as a NULL one reads nothing in
 * #centroid_add_ptarray_line and #centroid_add_ptarray_points
 */
static void
centroid_add_curve_line(CentroidAcc *lineal, CentroidAcc *punctual,
  const LWGEOM *geom)
{
  if ((! lineal && ! punctual) || ! geom || lwgeom_is_empty(geom))
    return;
  if (geom->type == LINETYPE)
  {
    centroid_add_ptarray_line(lineal, ((const LWLINE *) geom)->points, false);
    centroid_add_ptarray_points(punctual, ((const LWLINE *) geom)->points);
    return;
  }
  if (geom->type == CIRCSTRINGTYPE)
  {
    centroid_add_ptarray_line(lineal, ((const LWCIRCSTRING *) geom)->points,
      true);
    centroid_add_ptarray_points(punctual,
      ((const LWCIRCSTRING *) geom)->points);
    return;
  }
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      centroid_add_curve_line(lineal, punctual, coll->geoms[i]);
  }
  return;
}

/**
 * @brief Fold a ring's own accumulator into the areal one, with the given sign
 * @details A ring is first made to weigh POSITIVELY, because the winding of a
 * ring does not say whether it bounds the surface or a hole in it -- both
 * rings of `POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 2,2 2,2 1,1 1))` run the same
 * way round. What says it is the ring's POSITION: the first bounds the
 * surface and every later one is a hole, so the hole is what carries the
 * negative sign here rather than whatever winding it happens to have.
 */
static void
centroid_fold_ring(CentroidAcc *areal, const CentroidAcc *ring, bool hole)
{
  if (ring->weight == 0.0)
    return;
  double sign = (ring->weight < 0) ? -1.0 : 1.0;
  if (hole)
    sign = -sign;
  areal->weight += sign * ring->weight;
  areal->x += sign * ring->x;
  areal->y += sign * ring->y;
  areal->z += sign * ring->z;
  areal->m += sign * ring->m;
  return;
}

/**
 * @brief Add a geometry to the accumulator of its own dimension
 * @details Each of the three accumulators is filled independently, and the
 * answer is read from the highest one that carries weight. That is what lets a
 * shape enclosing no area fall back to the curve it draws, and a curve of no
 * length to the point it stands at, without a separate pass.
 */
static void
centroid_add_geo(const LWGEOM *geom, CentroidAcc *areal, CentroidAcc *lineal,
  CentroidAcc *punctual)
{
  if (! geom || lwgeom_is_empty(geom))
    return;
  switch (geom->type)
  {
    case POINTTYPE:
      centroid_add_ptarray_points(punctual, ((const LWPOINT *) geom)->point);
      return;
    case LINETYPE:
      centroid_add_ptarray_line(lineal, ((const LWLINE *) geom)->points, false);
      centroid_add_ptarray_points(punctual, ((const LWLINE *) geom)->points);
      return;
    case CIRCSTRINGTYPE:
      centroid_add_ptarray_line(lineal, ((const LWCIRCSTRING *) geom)->points,
        true);
      centroid_add_ptarray_points(punctual,
        ((const LWCIRCSTRING *) geom)->points);
      return;
    case TRIANGLETYPE:
    {
      const LWTRIANGLE *tri = (const LWTRIANGLE *) geom;
      POINT4D origin;
      getPoint4d_p(tri->points, 0, &origin);
      CentroidAcc ring = {0};
      centroid_add_ring(&ring, tri->points, false, &origin);
      centroid_fold_ring(areal, &ring, false);
      centroid_add_ptarray_line(lineal, tri->points, false);
      centroid_add_ptarray_points(punctual, tri->points);
      return;
    }
    case POLYGONTYPE:
    {
      const LWPOLY *poly = (const LWPOLY *) geom;
      for (uint32_t i = 0; i < poly->nrings; i++)
      {
        if (poly->rings[i]->npoints == 0)
          continue;
        POINT4D origin;
        getPoint4d_p(poly->rings[i], 0, &origin);
        CentroidAcc ring = {0};
        centroid_add_ring(&ring, poly->rings[i], false, &origin);
        centroid_fold_ring(areal, &ring, i > 0);
        centroid_add_ptarray_line(lineal, poly->rings[i], false);
        centroid_add_ptarray_points(punctual, poly->rings[i]);
      }
      return;
    }
    case CURVEPOLYTYPE:
    {
      const LWCURVEPOLY *cp = (const LWCURVEPOLY *) geom;
      for (uint32_t i = 0; i < cp->nrings; i++)
      {
        const LWGEOM *ring = cp->rings[i];
        POINT4D origin;
        if (! ring_first_point(ring, &origin))
          continue;
        CentroidAcc acc = {0};
        centroid_add_curve_ring(&acc, ring, &origin);
        centroid_fold_ring(areal, &acc, i > 0);
        centroid_add_curve_line(lineal, punctual, ring);
      }
      return;
    }
    default:
      break;
  }
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      centroid_add_geo(coll->geoms[i], areal, lineal, punctual);
    return;
  }
  return;
}

/**
 * @brief Return true when a curve ring encloses an area, decided exactly
 * @details A ring of segments encloses an area exactly where its shoelace sum
 * is not zero (#ring_encloses_no_area). A ring carrying a circular arc whose
 * three points do not lie on one line (#cross_product_sign) encloses the
 * circular segment standing on its chord; a ring whose every arc is straight
 * is read through its vertices, joined in order across the members of a
 * compound curve as one ring, never member by member
 */
static bool
centroid_ring_holds_area(const LWGEOM *ring)
{
  if (! ring || lwgeom_is_empty(ring))
    return false;
  if (ring->type == LINETYPE)
    return ! ring_encloses_no_area(((const LWLINE *) ring)->points);
  const LWGEOM *one[1] = {ring};
  const LWGEOM **members = one;
  uint32_t n = 1;
  if (ring->type == COMPOUNDTYPE)
  {
    members = (const LWGEOM **) ((const LWCOMPOUND *) ring)->geoms;
    n = ((const LWCOMPOUND *) ring)->ngeoms;
  }
  POINTARRAY *pa = ptarray_construct_empty(FLAGS_GET_Z(ring->flags),
    FLAGS_GET_M(ring->flags), 8);
  bool arc = false;
  for (uint32_t i = 0; i < n && ! arc; i++)
  {
    const POINTARRAY *mp = (members[i]->type == CIRCSTRINGTYPE) ?
      ((const LWCIRCSTRING *) members[i])->points :
      ((const LWLINE *) members[i])->points;
    if (members[i]->type == CIRCSTRINGTYPE)
      for (uint32_t j = 0; j + 2 < mp->npoints && ! arc; j += 2)
      {
        const POINT2D *p1 = getPoint2d_cp(mp, j);
        const POINT2D *p2 = getPoint2d_cp(mp, j + 1);
        const POINT2D *p3 = getPoint2d_cp(mp, j + 2);
        arc = cross_product_sign(p1->x, p1->y, p3->x, p3->y, p1->x, p1->y,
          p2->x, p2->y) != 0;
      }
    for (uint32_t j = 0; j < mp->npoints; j++)
    {
      POINT4D p;
      getPoint4d_p(mp, j, &p);
      ptarray_append_point(pa, &p, LW_TRUE);
    }
  }
  bool result = arc || ! ring_encloses_no_area(pa);
  ptarray_free(pa);
  return result;
}

/**
 * @brief Return true when a geometry holds an area, decided exactly
 * @details Some ring of a polygon, a triangle or a curve polygon encloses an
 * area (#ring_encloses_no_area, #centroid_ring_holds_area); the members of a
 * collection are asked in turn
 */
static bool
centroid_holds_area(const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
    return false;
  switch (geom->type)
  {
    case TRIANGLETYPE:
      return ! ring_encloses_no_area(((const LWTRIANGLE *) geom)->points);
    case POLYGONTYPE:
    {
      const LWPOLY *poly = (const LWPOLY *) geom;
      for (uint32_t i = 0; i < poly->nrings; i++)
        if (! ring_encloses_no_area(poly->rings[i]))
          return true;
      return false;
    }
    case CURVEPOLYTYPE:
    {
      const LWCURVEPOLY *cp = (const LWCURVEPOLY *) geom;
      for (uint32_t i = 0; i < cp->nrings; i++)
        if (centroid_ring_holds_area(cp->rings[i]))
          return true;
      return false;
    }
    default:
      break;
  }
  if (lwgeom_is_collection(geom))
  {
    const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
    for (uint32_t i = 0; i < coll->ngeoms; i++)
      if (centroid_holds_area(coll->geoms[i]))
        return true;
  }
  return false;
}

/**
 * @brief Return the centroid of a geometry
 * @details The centroid is the mean of the highest-dimensional pieces the
 * geometry holds, weighted by their measure, and it is taken in every
 * dimension the geometry states. Works directly on the exact circular arcs,
 * which it answers by their closed forms rather than by stroking them.
 */
LWGEOM *
meos_centroid(const LWGEOM *geom)
{
  assert(geom);
  bool hasz = (bool) FLAGS_GET_Z(geom->flags);
  bool hasm = (bool) FLAGS_GET_M(geom->flags);
  int32_t srid = geom->srid;

  if (lwgeom_is_empty(geom))
    return lwpoint_as_lwgeom(lwpoint_construct_empty(srid, hasz, hasm));

  /* Which pieces weigh is decided exactly: the areas where the geometry holds
   * one (#centroid_holds_area), and only otherwise the lengths and then the
   * points, which are read only then. Every ring is folded in weighing
   * positively, so a surface that holds any area holds a positive total of
   * it; a total of exactly zero, a ring whose lobes cancel, falls through */
  CentroidAcc areal = {0}, lineal = {0}, punctual = {0};
  POINT4D mean = {0, 0, 0, 0};
  if (centroid_holds_area(geom))
  {
    centroid_add_geo(geom, &areal, NULL, NULL);
    if (centroid_mean(&areal, &mean))
      return lwpoint_as_lwgeom(lwpoint_make(srid, hasz, hasm, &mean));
  }
  CentroidAcc scratch = {0};
  centroid_add_geo(geom, &scratch, &lineal, &punctual);
  /* A total length is zero exactly where every piece has its two ends equal */
  if (! centroid_mean(&lineal, &mean) && ! centroid_mean(&punctual, &mean))
    return lwpoint_as_lwgeom(lwpoint_construct_empty(srid, hasz, hasm));
  return lwpoint_as_lwgeom(lwpoint_make(srid, hasz, hasm, &mean));
}

/**
 * @brief Return the area of a geometry, on its circular arcs
 * @details The area is the weight #meos_centroid accumulates for the surfaces
 * of the geometry: each ring is read as the polygon of its chords, an arc
 * standing as the segment joining its ends, plus the circular segment
 * @p r^2 (theta - sin theta) / 2 each arc adds to, or takes from, its chord
 * (#centroid_add_ring). The first ring of a surface adds and every later one,
 * a hole, takes away, whatever their winding (#centroid_fold_ring), and a
 * point or a curve encloses nothing. Where @p lwgeom_area replaces each
 * quarter circle by 32 segments, the area of a disc of radius 1 is @p pi.
 * @param[in] geom Geometry
 */
double
meos_area(const LWGEOM *geom)
{
  assert(geom);
  if (lwgeom_is_empty(geom))
    return 0.0;
  CentroidAcc areal = {0}, lineal = {0}, punctual = {0};
  centroid_add_geo(geom, &areal, &lineal, &punctual);
  return areal.weight;
}

/*****************************************************************************/
