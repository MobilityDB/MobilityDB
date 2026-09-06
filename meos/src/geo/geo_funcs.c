/*****************************************************************************
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
 * @brief Decompose a geometry into the edges a native implementation works on
 * @details An implementation that answers a question about a geometry without
 * calling GEOS works on the edges of that geometry: the segments and the
 * circular arcs its boundary is made of. The decomposition that produces them
 * lived inside the clipping engine, reachable only from there, so anything
 * else needing it had to build its own.
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include "postgres.h"
#include <utils/float.h>
#include <pgtypes.h>
/* PostGIS */
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
/* MEOS */
#include "meos.h"
#include "meos_internal_geo.h"
#include "geo/geo_funcs.h"
#include "geo/postgis_funcs.h"
#include "geo/meos_transform.h"

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
    e.xmin = Min(e.x1, e.x2); e.xmax = Max(e.x1, e.x2);
    e.ymin = Min(e.y1, e.y2); e.ymax = Max(e.y1, e.y2);
    e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
    e.length = e.dx * e.dx + e.dy * e.dy;
    e.etype = etype;
    edge_set_tolerance(&e);
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
  /* An empty point (e.g. a component of a multipoint or the boundary of a
   * closed trajectory) has no vertex to read; it contributes no edge. */
  if (! pt->point || pt->point->npoints < 1)
    return;
  POINT4D p;
  (void) getPoint4d_p(pt->point, 0, &p);
  Edge e;
  e.x1 = e.x2 = e.xmin = e.xmax = p.x;
  e.y1 = e.y2 = e.ymin = e.ymax = p.y;
  e.dx = e.dy = e.length = 0;
  e.etype = EDGE_POINT;
  edge_set_tolerance(&e);
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

/**
 * @brief Return true if a ring encloses no area
 * @details A ring that runs out and back along itself, or whose vertices are
 * collinear, bounds no region: the point set it draws is its own linework.
 * Real survey data carries them, so this is not a synthetic case.
 * @note The shoelace sum is an AREA, so what reads it as zero is an area too:
 * a distance times the extent the ring occupies. Reading it against a length
 * would make the answer depend on the unit the coordinates are expressed in.
 * #ptarray_signed_area accumulates DIFFERENCES of coordinates rather than the
 * coordinates themselves, so its rounding grows with that extent and not with
 * the distance of the ring from the origin.
 */
static bool
ring_encloses_no_area(const POINTARRAY *pa)
{
  if (! pa || pa->npoints < 3)
    return true;
  double xmin, xmax, ymin, ymax;
  const POINT2D *p = getPoint2d_cp(pa, 0);
  xmin = xmax = p->x; ymin = ymax = p->y;
  for (uint32_t i = 1; i < pa->npoints; i++)
  {
    p = getPoint2d_cp(pa, i);
    xmin = Min(xmin, p->x); xmax = Max(xmax, p->x);
    ymin = Min(ymin, p->y); ymax = Max(ymax, p->y);
  }
  double extent = Max(xmax - xmin, ymax - ymin);
  double tol = (MEOS_GEOM_TOLERANCE +
    4.0 * DBL_EPSILON * (double) pa->npoints * extent) * extent;
  return fabs(ptarray_signed_area(pa)) <= tol;
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
 * emitted as line edges
 */
static void
emit_arc_edge(const POINT4D *pa, const POINT4D *pb, const POINT4D *pc,
  MeosArray *edges, EdgeType line_etype, EdgeType arc_etype)
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
  if (fabs(ax - cx) <= MEOS_GEOM_TOLERANCE && fabs(ay - cy) <= MEOS_GEOM_TOLERANCE &&
      (fabs(ax - bx) > MEOS_GEOM_TOLERANCE || fabs(ay - by) > MEOS_GEOM_TOLERANCE))
  {
    double mx = (ax + bx) / 2, my = (ay + by) / 2;
    double radius = hypot(ax - mx, ay - my);
    double theta_a = atan2(ay - my, ax - mx);
    double theta_b = atan2(by - my, bx - mx);
    /* Carried back to where the points came from */
    mx += ox; my += oy;
    const double sx[2] = {ax + ox, bx + ox}, sy[2] = {ay + oy, by + oy};
    const double ex[2] = {bx + ox, ax + ox}, ey[2] = {by + oy, ay + oy};
    const double t0[2] = {theta_a, theta_b}, t1[2] = {theta_b, theta_a};
    for (int i = 0; i < 2; i++)
    {
      Edge e;
      e.cx = mx; e.cy = my; e.radius = radius;
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
      meos_array_add(edges, &e);
    }
    return;
  }

  /* Collinear points: emit straight line edges */
  if (fabs(d) < MEOS_GEOM_TOLERANCE)
  {
    const double px[3] = {ax + ox, bx + ox, cx + ox};
    const double py[3] = {ay + oy, by + oy, cy + oy};
    for (int i = 0; i < 2; i++)
    {
      Edge e;
      e.x1 = px[i]; e.y1 = py[i];
      e.x2 = px[i + 1]; e.y2 = py[i + 1];
      e.xmin = Min(e.x1, e.x2); e.xmax = Max(e.x1, e.x2);
      e.ymin = Min(e.y1, e.y2); e.ymax = Max(e.y1, e.y2);
      e.dx = e.x2 - e.x1; e.dy = e.y2 - e.y1;
      e.length = e.dx * e.dx + e.dy * e.dy;
      e.etype = line_etype;
      edge_set_tolerance(&e);
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
  /* Carried back to where the points came from */
  e.cx += ox; e.cy += oy;
  e.x1 = ax + ox; e.y1 = ay + oy;
  e.x2 = cx + ox; e.y2 = cy + oy;
  e.theta0 = atan2(e.y1 - e.cy, e.x1 - e.cx);
  e.theta1 = atan2(e.y2 - e.cy, e.x2 - e.cx);
  /* Traversal orientation from the signed area of (start, mid, end) */
  e.ccw = ((bx - ax) * (cy - ay) - (by - ay) * (cx - ax)) > 0;
  e.dx = e.dy = e.length = 0;
  e.etype = arc_etype;
  if (getenv("ARC_DIAG"))
    fprintf(stderr, "[arc] reconstructed radius %.12f centre (%.6f,%.6f)\n",
      e.radius, e.cx, e.cy);
  arc_set_bbox(&e);
  edge_set_tolerance(&e);
  meos_array_add(edges, &e);
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
 * @brief Return true if a geometry is composed solely of the types the native
 * implementations can extract into edges
 * @details Mirrors the type dispatch of #geom_extract_edges_iter, which every
 * native implementation of a PostGIS function reads its geometry through, so
 * the predicate answers for all of them: the clip engine, the DE-9IM matrix,
 * the convex hull, the oriented envelope and the buffer alike. A geometry
 * holding any other type, a TIN or a polyhedral surface, is uncovered and
 * belongs to the caller, which either answers it another way or reports that
 * it is not supported
 * @note Uncovered never means unrelated: a @p false is the absence of an
 * answer, not a negative one
 */
bool
geom_meos_supported(const LWGEOM *geom)
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
        if (! geom_meos_supported(col->geoms[i]))
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
        if (rt == COMPOUNDTYPE && ! geom_meos_supported(cp->rings[r]))
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
static int
point2d_cmp(const void *a, const void *b)
{
  const POINT2D *pa = (const POINT2D *) a;
  const POINT2D *pb = (const POINT2D *) b;
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
 * @brief Add the geometrically relevant points of an edge
 * @details For a line, only the two endpoints are needed. For an arc, the two
 * endpoints and every cardinal point of the supporting circle lying on the arc
 * are added. The latter are essential: an arc may attain its X/Y extrema in
 * its interior.
 */
static void
add_edge_points(const Edge *e, POINT2D *points, uint32_t *npoints)
{
  assert(e); assert(points); assert(npoints);
  add_point(points, npoints, e->x1, e->y1);
  if (fabs(e->x2 - e->x1) > MEOS_GEOM_TOLERANCE || fabs(e->y2 - e->y1) > MEOS_GEOM_TOLERANCE)
    add_point(points, npoints, e->x2, e->y2);
}

/**
 * @brief Compute the convex hull of a set of 2D points
 * @details The returned hull is not closed.
 * The implementation uses Andrew's monotone-chain algorithm.
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
  qsort(sorted, npoints, sizeof(POINT2D), point2d_cmp);

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
      cross_product(&h[nhull - 2], &h[nhull - 1], &sorted[i]) <= 0.0)
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
        cross_product(&h[nhull - 2], &h[nhull - 1], &sorted[i]) <= 0.0)
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
 * the hull. Reading the hull through its support function answers both the
 * hull and the rectangle of minimum area enclosing it, exactly and for arcs
 * as much as for segments
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
 * @param[out] px,py Point reaching that far, ignored when null
 * @param[out] which Feature reaching that far, ignored when null
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
 * @brief Collect the features supporting the convex hull of a geometry
 * @details Every edge contributes its two end points and an arc edge
 * contributes its circle. A point within the hull of the other points never
 * supports the hull, so the points are reduced to their own hull first, which
 * is what keeps the number of features small
 */
static HullFeature *
hull_features(const MeosArray *edges, int *nfeats)
{
  int nedges = (int) edges->count;
  uint32_t maxpoints = 2 * (uint32_t) nedges;
  POINT2D *points = palloc(sizeof(POINT2D) * maxpoints);
  uint32_t npoints = 0;
  int narcs = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = (Edge *) meos_array_get(edges, i);
    add_edge_points(e, points, &npoints);
    if (e->etype == EDGE_LINEARC || e->etype == EDGE_POLYARC)
      narcs++;
  }

  POINT2D *hull = NULL;
  uint32_t nhull = convex_hull_points(points, npoints, &hull);
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
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = (Edge *) meos_array_get(edges, i);
    if (e->etype != EDGE_LINEARC && e->etype != EDGE_POLYARC)
      continue;
    result[n].x = e->cx; result[n].y = e->cy; result[n].radius = e->radius;
    result[n].theta0 = e->theta0; result[n].theta1 = e->theta1;
    result[n].ccw = e->ccw; result[n].is_arc = true;
    n++;
  }
  if (hull)
    pfree(hull);
  pfree(points);
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

/**
 * @brief Move a point onto the vertex of the geometry it stands for
 * @details The end of an arc piece is read from the centre and the radius of
 * its circle, which names the vertex the arc ends on to within the rounding of
 * that arithmetic. The vertex itself is the point the geometry carries, so the
 * answer is given in the coordinates of the input rather than in a rounding of
 * them
 */
static void
hull_snap_to_vertex(const HullFeature *feats, int nfeats, double *x, double *y)
{
  for (int i = 0; i < nfeats; i++)
    if (! feats[i].is_arc &&
        fabs(feats[i].x - *x) <= MEOS_GEOM_TOLERANCE &&
        fabs(feats[i].y - *y) <= MEOS_GEOM_TOLERANCE)
    {
      *x = feats[i].x;
      *y = feats[i].y;
      return;
    }
  return;
}

/**
 * @brief Return the boundary of the convex hull of a geometry, walked
 * counterclockwise
 * @details Between two consecutive directions one feature supports the hull.
 * A point feature supporting a range of directions is one vertex of the hull;
 * an arc feature supporting one is the piece of its circle over that range.
 * Consecutive supports are joined by the segment between the point where one
 * stops touching the hull and the point where the next starts
 */
static HullBoundary *
hull_boundary(const HullFeature *feats, int nfeats, int *npieces)
{
  double *dirs;
  int ndirs = hull_directions(feats, nfeats, &dirs);

  /* The feature supporting the hull between each direction and the next */
  int *which = palloc(sizeof(int) * ndirs);
  for (int k = 0; k < ndirs; k++)
  {
    double a = dirs[k];
    double b = (k + 1 < ndirs) ? dirs[k + 1] : dirs[0] + 2 * M_PI;
    which[k] = -1;
    (void) hull_support(feats, nfeats, (a + b) / 2, NULL, NULL, &which[k]);
  }

  /* One feature may support the hull over several consecutive directions, and
   * it touches the hull along one piece of it rather than along one piece per
   * direction, so the walk starts where the support changes */
  int start = 0;
  for (int k = 0; k < ndirs; k++)
    if (which[k] != which[(k + ndirs - 1) % ndirs])
    {
      start = k;
      break;
    }

  HullBoundary *result = palloc(sizeof(HullBoundary) * (2 * ndirs + 2));
  int n = 0;
  double firstx = 0, firsty = 0, prevx = 0, prevy = 0;
  bool started = false;
  int k = 0;
  while (k < ndirs)
  {
    int idx = (start + k) % ndirs;
    if (which[idx] < 0)
    {
      k++;
      continue;
    }
    /* The whole run of directions this feature supports */
    int len = 1;
    while (k + len < ndirs && which[(start + k + len) % ndirs] == which[idx])
      len++;
    double a = dirs[idx];
    double b = dirs[(start + k + len) % ndirs];
    double sweep = angle_normalize(b - a);
    if (sweep <= MEOS_GEOM_TOLERANCE)
      sweep = 2 * M_PI;
    const HullFeature *f = &feats[which[idx]];
    double sx, sy, ex, ey, mx = 0, my = 0;
    if (! f->is_arc)
    {
      sx = ex = f->x;
      sy = ey = f->y;
    }
    else
    {
      sx = f->x + f->radius * cos(a);
      sy = f->y + f->radius * sin(a);
      ex = f->x + f->radius * cos(a + sweep);
      ey = f->y + f->radius * sin(a + sweep);
      mx = f->x + f->radius * cos(a + sweep / 2);
      my = f->y + f->radius * sin(a + sweep / 2);
      hull_snap_to_vertex(feats, nfeats, &sx, &sy);
      hull_snap_to_vertex(feats, nfeats, &ex, &ey);
    }

    if (! started)
    {
      firstx = sx; firsty = sy;
      started = true;
    }
    else if (hypot(sx - prevx, sy - prevy) > MEOS_GEOM_TOLERANCE)
    {
      /* The segment joining the previous support to this one */
      result[n].x1 = prevx; result[n].y1 = prevy;
      result[n].x2 = sx; result[n].y2 = sy;
      result[n].is_arc = false;
      n++;
    }
    else
    {
      /* The two meet, so the piece starts on the point already reached and
       * the ring closes on itself rather than within the rounding of two
       * ways of naming one point */
      sx = prevx; sy = prevy;
    }
    if (f->is_arc && hypot(ex - sx, ey - sy) > MEOS_GEOM_TOLERANCE)
    {
      result[n].x1 = sx; result[n].y1 = sy;
      result[n].xm = mx; result[n].ym = my;
      result[n].x2 = ex; result[n].y2 = ey;
      result[n].is_arc = true;
      n++;
    }
    prevx = ex; prevy = ey;
    k += len;
  }

  /* Close the ring on the point it starts from */
  if (started && n > 0)
  {
    if (hypot(firstx - prevx, firsty - prevy) > MEOS_GEOM_TOLERANCE)
    {
      result[n].x1 = prevx; result[n].y1 = prevy;
      result[n].x2 = firstx; result[n].y2 = firsty;
      result[n].is_arc = false;
      n++;
    }
    else
    {
      /* The ring ends where it starts up to the rounding of two ways of
       * naming one point, and is closed on that point */
      result[n - 1].x2 = firstx;
      result[n - 1].y2 = firsty;
    }
  }
  /* Every piece starts on the point the one before it ends on, so that the
   * ring is continuous however the two ways of naming that point round */
  for (int i = 1; i < n; i++)
  {
    result[i].x1 = result[i - 1].x2;
    result[i].y1 = result[i - 1].y2;
  }
  if (n > 0)
  {
    result[0].x1 = result[n - 1].x2;
    result[0].y1 = result[n - 1].y2;
  }
  pfree(which); pfree(dirs);
  *npieces = n;
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
 * @brief Return the convex hull of a geometry carrying a circular arc
 */
static LWGEOM *
convex_hull_arc(const LWGEOM *geom, const MeosArray *edges)
{
  int nfeats = 0;
  HullFeature *feats = hull_features(edges, &nfeats);
  int npieces = 0;
  HullBoundary *pieces = hull_boundary(feats, nfeats, &npieces);

  /* No arc reaches past the points, so the hull is the one they place */
  bool curved = false;
  for (int i = 0; i < npieces && ! curved; i++)
    curved = pieces[i].is_arc;
  if (! curved)
  {
    POINT2D *points = palloc(sizeof(POINT2D) * Max(nfeats, 1));
    uint32_t npoints = 0;
    for (int i = 0; i < nfeats; i++)
      if (! feats[i].is_arc)
      {
        points[npoints].x = feats[i].x;
        points[npoints].y = feats[i].y;
        npoints++;
      }
    POINT2D *hull = NULL;
    uint32_t nhull = convex_hull_points(points, npoints, &hull);
    LWGEOM *result = (nhull == 0) ?
      lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0)) :
      make_geometry_hull(geom->srid, hull, nhull);
    if (hull)
      pfree(hull);
    pfree(points); pfree(pieces); pfree(feats);
    return result;
  }

  LWGEOM *result = hull_boundary_geometry(geom->srid, pieces, npieces);
  pfree(pieces); pfree(feats);
  return result;
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
  int nfeats = 0;
  HullFeature *feats = hull_features(edges, &nfeats);
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

  /* Every edge contributes its two end points */
  uint32_t maxpoints = 2 * nedges;
  POINT2D *points = palloc(sizeof(POINT2D) * maxpoints);
  uint32_t npoints = 0;

  /* Extract the exact extremal points */
  for (uint32_t i = 0; i < nedges; i++)
  {
    const Edge *e = (Edge *) meos_array_get(edge_array, i);
    add_edge_points(e, points, &npoints);
  }

  /* We no longer need the edge array for the convex hull */
  meos_array_destroy(edge_array);

  /* Convex hull */
  POINT2D *hull = NULL;
  uint32_t nhull = convex_hull_points(points, npoints, &hull);
  pfree(points);
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
    const Edge *e = (Edge *) meos_array_get(edge_array, i);
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

  /* The hull below is placed on the vertices, which are the whole of a
   * geometry only while every edge of it is a segment. An arc reaches past
   * the points that define it, and belongs to the hull as the piece of its
   * circle that no other feature reaches past */
  if (lwgeom_has_arc(geom))
  {
    LWGEOM *result = convex_hull_arc(geom, edge_array);
    meos_array_destroy(edge_array);
    return result;
  }

  /* Every edge contributes its two end points */
  uint32_t maxpoints = 2 * nedges;
  POINT2D *points = palloc(sizeof(POINT2D) * maxpoints);
  uint32_t npoints = 0;

  /* Extract the exact extremal points */
  for (uint32_t i = 0; i < nedges; i++)
  {
    const Edge *e = (Edge *) meos_array_get(edge_array, i);
    add_edge_points(e, points, &npoints);
  }

  /* We no longer need the edge array for the convex hull */
  meos_array_destroy(edge_array);

  /* Convex hull */
  POINT2D *hull = NULL;
  uint32_t nhull = convex_hull_points(points, npoints, &hull);
  if (nhull == 0)
  {
    pfree(points);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(geom->srid, 0, 0));
  }
  if (nhull == 2)
    hull_order_as_input(points, npoints, hull);
  pfree(points);

  LWGEOM *result = make_geometry_hull(geom->srid, hull, nhull);
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
    POINT2D p1;
    lwpoint_getPoint2d_p(point1, &p1);
    for (uint32_t j = i + 1; j < mpoint->ngeoms; j++)
    {
      const LWPOINT *point2 = mpoint->geoms[j];
      if (! point2 || lwpoint_is_empty(point2))
        continue;
      POINT2D p2;
      lwpoint_getPoint2d_p(point2, &p2);
      if (p1.x == p2.x && p1.y == p2.y)
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
  bool closed = nedges > 1 &&
    relate_same_point(edges[0]->x1, edges[0]->y1,
      edges[nedges - 1]->x2, edges[nedges - 1]->y2);
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
      edges[i][k] = (Edge *) meos_array_get(arrs[i], k);
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
    edges[i] = (Edge *) meos_array_get(arr, i);
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
  meos_error(ERROR, MEOS_ERR_TEXT_INPUT,
    "Only lon/lat coordinate systems are supported in geography");
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
 * @brief Ensure that two geometries/geographies have the same dimensionality
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
 * @brief Reconcile the SRID of two spatial components: copy the known SRID onto
 * the one that is unknown, and ensure that two known SRIDs are equal
 * @details This is the single construction-time SRID resolution used by the
 * parsers and the constructors: an unknown (`SRID_UNKNOWN`) component adopts the
 * SRID of the other, while two known but different SRIDs are rejected. By
 * convention @p srid1 is the geometry SRID and @p srid2 the temporal type SRID.
 * @param[in] srid1,srid2 SRIDs to reconcile
 * @param[out] result Common SRID (the known one, or `SRID_UNKNOWN` if both are
 * unknown)
 * @return On error (two different known SRIDs) return false
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
 */
bool
ensure_not_empty(const GSERIALIZED *gs)
{
  if (! gserialized_is_empty(gs))
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Only non-empty geometries accepted");
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
 * @brief Return a long double between 0 and 1 representing the location of the
 * closest point on the 2D segment to the given point, as a fraction of total
 * segment length
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
 * @brief Return a long double between 0 and 1 representing the location of the
 * closest point on the 3D segment to the given point, as a fraction of total
 * segment length
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
 * @brief Return true if a point lies on the boundary of a geometry
 * @details Uses the exact line/arc engine
 */
bool
relate_point_on_boundary(double x, double y, Edge **edges, int nedges)
{
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    switch (e->etype)
    {
      case EDGE_POLYSEG:
        if (point_on_segment_within(x, y, e->x1, e->y1, e->x2, e->y2,
              e->tol))
          return true;
        break;
      case EDGE_POLYARC:
        if (point_on_arc(x, y, e))
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
 * @brief Classify a point with respect to an areal geometry
 * @details Return:
 *   0 = interior
 *   1 = boundary
 *   2 = exterior
 */
int
relate_point_in_area(double x, double y, Edge **edges, int nedges)
{
  if (relate_point_on_boundary(x, y, edges, nedges))
    return 1;
  return point_in_polygon(x, y, edges, nedges) ? 0 : 2;
}

/* Indexing one edge array costs one insertion per edge, while walking it once
 * for every edge of the other costs their PRODUCT. The index is built where
 * that product dwarfs the pass, and a pair small enough for the walk to be the
 * cheaper of the two keeps it */

/**
 * @brief Return the greatest x an edge array reaches, which is how far a ray
 * cast from a point to its right runs before it leaves the array behind
 */
static double
relate_edges_xmax(Edge **edges, int nedges)
{
  assert(edges);
  double result = -DBL_MAX;
  for (int i = 0; i < nedges; i++)
    if (edges[i]->xmax > result)
      result = edges[i]->xmax;
  return result;
}

/**
 * @brief Return an index over the bounding boxes of an edge array
 * @details The boxes carry no SRID of their own: each is compared against
 * another built the same way, so the value only has to be the same everywhere
 */
static RTree *
relate_edges_index(Edge **edges, int nedges)
{
  assert(edges);
  RTree *rtree = rtree_create_stbox();
  for (int i = 0; i < nedges; i++)
  {
    STBox box;
    stbox_set(true, false, false, 0, edges[i]->xmin, edges[i]->xmax,
      edges[i]->ymin, edges[i]->ymax, 0, 0, NULL, &box);
    rtree_insert(rtree, &box, i);
  }
  return rtree;
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
  re->edges = edges;
  re->nedges = nedges;
  re->xmax = relate_edges_xmax(edges, nedges);
  /* An index query has to admit every edge the scan it replaces would accept,
   * and the on-edge tests read the tolerance the EDGE carries, which grows
   * with the size of its coordinates. Padding a query by MEOS_GEOM_TOLERANCE
   * would therefore drop, at projected coordinates, edges whose own tolerance
   * is thousands of times that. The widest tolerance in the array is what
   * makes the index answer what the scan answers at every scale */
  re->tol = MEOS_GEOM_TOLERANCE;
  for (int i = 0; i < nedges; i++)
    if (edges[i]->tol > re->tol)
      re->tol = edges[i]->tol;
  re->index = index ? relate_edges_index(edges, nedges) : NULL;
  return;
}

/**
 * @brief Free the index an edge array carries, if it carries one
 */
void
relate_edges_clear(RelateEdges *re)
{
  if (re->index)
    rtree_free(re->index);
  re->index = NULL;
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
relate_point_on_boundary_index(double x, double y, const RelateEdges *re)
{
  if (! re->index)
    return relate_point_on_boundary(x, y, re->edges, re->nedges);
  STBox query;
  stbox_set(true, false, false, 0, x - re->tol, x + re->tol, y - re->tol,
    y + re->tol, 0, 0, NULL, &query);
  MeosArray *candidates = meos_array_create(sizeof(int64));
  int nc = rtree_search(re->index, INDEX_OVERLAPS, &query, candidates);
  bool result = false;
  for (int c = 0; c < nc && ! result; c++)
  {
    Edge *one = re->edges[*(int64 *) meos_array_get(candidates, c)];
    result = relate_point_on_boundary(x, y, &one, 1);
  }
  meos_array_destroy(candidates);
  return result;
}

/**
 * @brief Return where a point stands with respect to the area an edge array
 * bounds, reading the edges each question needs out of an index
 * @details The answer is the one #relate_point_in_area() gives: the index only
 * decides which edges are asked, and one it leaves out neither carries the
 * point nor crosses the ray cast from it
 */
int
relate_point_in_area_index(double x, double y, const RelateEdges *re)
{
  if (relate_point_on_boundary_index(x, y, re))
    return 1;
  if (! re->index)
    return point_in_polygon(x, y, re->edges, re->nedges) ? 0 : 2;
  return point_in_polygon_index(x, y, re->edges, re->nedges, re->index,
    re->xmax) ? 0 : 2;
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
static MeosArray *relate_extract_edges(const LWGEOM *geom);
static bool relate_area_boundary_edge(const Edge *e);
static void relate_area_edge_point(const Edge *e, double t, double *x,
  double *y);
static double relate_area_edge_parameter(const Edge *e, double x, double y);
static void relate_area_add_parameter(double t, double *params, int *nparams,
  int maxparams);
static int relate_area_parameter_cmp(const void *a, const void *b);
static int relate_area_edge_intersection(const Edge *a, const Edge *b,
  double ix[2], double iy[2], bool *overlap);
static bool relate_point_on_edge(double x, double y, const Edge *e);

/**
 * @brief One operand of a relationship, together with the edges it draws
 */
typedef struct
{
  const LWGEOM *geom;  /**< Geometry the edges are those of */
  MeosArray *arr;      /**< Edges of that geometry */
} RelateOperand;

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
  ops->op[0].geom = g1;
  ops->op[0].arr = relate_extract_edges(g1);
  ops->op[1].geom = g2;
  ops->op[1].arr = relate_extract_edges(g2);
  return;
}

/**
 * @brief Release the edges of both operands of a relationship
 */
static void
relate_operands_free(RelateOperands *ops)
{
  meos_array_destroy(ops->op[0].arr);
  meos_array_destroy(ops->op[1].arr);
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
        return ops->op[i].arr;
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
      if (ops->op[i].arr == arr)
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
 * @details Pattern characters:
 *   T = any non-empty intersection
 *   F = empty intersection
 *   0 = point
 *   1 = line
 *   2 = area
 *   * = don't care
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
 * @brief Return the topological dimension of a geometry
 */
static int
relate_dimension(const LWGEOM *geom)
{
  if (relate_is_areal(geom))
    return 2;
  if (relate_is_linear(geom))
    return 1;
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
 * @brief Return true if two points are equal within the MEOS tolerance.
 */
static inline bool
relate_same_point(double x1, double y1, double x2, double y2)
{
  return fabs(x1 - x2) <= MEOS_GEOM_TOLERANCE && fabs(y1 - y2) <= MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return true if an edge has non-zero length.
 */
static inline bool
relate_edge_nonempty(const Edge *e)
{
  return !relate_same_point(e->x1, e->y1, e->x2, e->y2);
}

/**
 * @brief Return true if a point is an endpoint of an edge.
 */
static inline bool
relate_point_is_edge_endpoint(double x, double y, const Edge *e)
{
  return relate_same_point(x, y, e->x1, e->y1) ||
         relate_same_point(x, y, e->x2, e->y2);
}

/**
 * @brief Return true if a point occurs an odd number of times among the
 * endpoints of a linear geometry.
 * @details This implements the endpoint parity rule used for the boundary of
 * linear geometries:
 * - odd number of occurrences -> boundary
 * - even number of occurrences -> interior
 * Closed lines consequently have an empty boundary.
 * The function works on the extracted Edge representation, so circular arcs
 * remain exact.
 */
static bool
relate_point_on_linear_boundary(double x, double y, Edge **edges, int nedges)
{
  int count = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINESEG && e->etype != EDGE_LINEARC)
      continue;
    if (!relate_edge_nonempty(e))
      continue;
    if (relate_same_point(x, y, e->x1, e->y1))
      count++;
    if (relate_same_point(x, y, e->x2, e->y2))
      count++;
  }
  return (count & 1) != 0;
}

/**
 * @brief Classify a point with respect to a complete linear geometry.
 * @details Return:
 *   0 = interior
 *   1 = boundary
 *   2 = exterior
 * Note that an endpoint of an individual edge does NOT automatically make the
 * point part of the geometry boundary. Endpoint parity is evaluated over the
 * complete linear geometry.
 */
static int
relate_point_in_linear(double x, double y, Edge **edges, int nedges)
{
  bool found = false;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINESEG && e->etype != EDGE_LINEARC)
      continue;
    bool on = false;
    if (e->etype == EDGE_LINESEG)
      on = point_on_segment(x, y, e->x1, e->y1, e->x2, e->y2);
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
  if (relate_point_on_linear_boundary(x, y, edges, nedges))
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
  if (geom->type != MULTIPOINTTYPE && geom->type != COLLECTIONTYPE)
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
    POINT4D p;
    getPoint4d_p(((const LWPOINT *) geom)->point, 0, &p);
    result[*count].x = p.x;
    result[*count].y = p.y;
    (*count)++;
    return;
  }
  if (geom->type != MULTIPOINTTYPE && geom->type != COLLECTIONTYPE)
    return;
  const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
  for (uint32_t i = 0; i < col->ngeoms; i++)
    relate_extract_points_iter(col->geoms[i], result, count);
  return;
}

/**
 * @brief Return the points of a point geometry
 * @details A POINT and a MULTIPOINT are the same kind of set to the relation,
 * one of them holding a single element, so both are related by the same code
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
 */
static bool
relate_point_in_points(double x, double y, const POINT2D *points, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (relate_same_point(x, y, points[i].x, points[i].y))
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
    edges[i] = (Edge *) meos_array_get(arr, i);

  /* Each point lies in the interior of the linear geometry, on its Mod-2
   * boundary, or outside it. A point geometry has an empty boundary, so its
   * boundary row stays F */
  for (int i = 0; i < np; i++)
  {
    switch (relate_point_in_linear(points[i].x, points[i].y, edges, nedges))
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
    edges[i] = (Edge *) meos_array_get(arr, i);

  /* Each point lies in the interior of the area, on its boundary, or outside
   * it. A point geometry has an empty boundary, so its boundary row stays F */
  for (int i = 0; i < np; i++)
  {
    switch (relate_point_in_area(points[i].x, points[i].y, edges, nedges))
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
    IntersectResult r = linesegm_intersect(a->x1, a->y1, a->dx, a->dy,
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
 * @brief Compute the parameter of a point on an arc.
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
 * @brief Compute the point at parameter t on an edge.
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
 * @brief Add an intersection parameter to an array.
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
 * @brief Return true if two arcs lie on the same supporting circle.
 */
static bool
relate_same_circle(const Edge *a, const Edge *b)
{
  return fabs(a->cx - b->cx) <= MEOS_GEOM_TOLERANCE &&
         fabs(a->cy - b->cy) <= MEOS_GEOM_TOLERANCE &&
         fabs(a->radius - b->radius) <= MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return true if two circular arcs overlap in a non-zero-length
 * portion.
 * @details  The arcs must lie on the same supporting circle.
 * We use the endpoints of both arcs as candidate split points and test
 * whether an interval between consecutive candidates belongs to both
 * arcs. No polygonization is involved.
 */
static bool
relate_arcs_overlap(const Edge *a, const Edge *b)
{
  if (!relate_same_circle(a, b))
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
 * @brief Add the intersection points of two circular arcs.
 * @details Returns the number of point intersections added to x/y.
 * If the arcs overlap over a non-zero-length portion, overlap is set
 * to true and no point is required for the one-dimensional component.
 */
int
relate_arc_arc_points(const Edge *a, const Edge *b, double x[2], double y[2],
  bool *overlap)
{
  *overlap = false;
  double dx = b->cx - a->cx;
  double dy = b->cy - a->cy;
  double d = hypot(dx, dy);

  /* Coincident supporting circles */
  if (d <= MEOS_GEOM_TOLERANCE)
  {
    if (fabs(a->radius - b->radius) > MEOS_GEOM_TOLERANCE)
      return 0;
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

  /* Disjoint supporting circles */
  if (d > a->radius + b->radius + MEOS_GEOM_TOLERANCE ||
      d < fabs(a->radius - b->radius) - MEOS_GEOM_TOLERANCE)
    return 0;
  double aa = (d * d + a->radius * a->radius - b->radius * b->radius) /
    (2.0 * d);
  double h2 = a->radius * a->radius - aa * aa;
  if (h2 < 0.0)
    h2 = 0.0;
  double h = sqrt(h2);
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
    if (h <= MEOS_GEOM_TOLERANCE)
      break;
  }
  return n;
}

/**
 * @brief Process the intersection between one linear edge and one
 * polygon boundary edge.
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
    IntersectResult r =  linesegm_intersect(line->x1, line->y1, line->dx,
      line->dy, boundary->x1, boundary->y1, boundary->x2, boundary->y2);
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
    int lloc = relate_point_in_linear(x, y, all_lines, nlines);
    if (lloc == 0)
      m->ib = 0;
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
      int lloc = relate_point_in_linear(x, y, all_lines, nlines);
      if (lloc == 0)
        m->ib = 0;
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
      int lloc = relate_point_in_linear(x, y, all_lines, nlines);
      if (lloc == 0)
        m->ib = 0;
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
      int lloc = relate_point_in_linear(ix[i], iy[i], all_lines, nlines);
      if (lloc == 0)
        m->ib = 0;
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
 * @brief Classify the open portion of a linear edge between two
 * consecutive parameters.
 * @details Since the portion contains no intersection with the polygon
 * boundary, one representative point is sufficient to determine whether the
 * complete open portion belongs to the polygon interior or exterior.
 */
static void
relate_linear_area_interval(const Edge *line, double t0, double t1,
  Edge **area_edges, int narea, MeosDE9IM *m)
{
  if (t1 - t0 <= MEOS_GEOM_TOLERANCE)
    return;
  double tm = (t0 + t1) * 0.5;
  double x, y;
  relate_edge_point(line, tm, &x, &y);
  int loc = relate_point_in_area(x, y, area_edges, narea);
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
 * @brief Sort an array of parameters in increasing order.
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
  if (! relate_same_circle(a, b))
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
  POINT2D *result = palloc(sizeof(POINT2D) * (size_t) (2 * nedges + 1));
  *count = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINESEG && e->etype != EDGE_LINEARC)
      continue;
    if (! relate_edge_nonempty(e))
      continue;
    for (int k = 0; k < 2; k++)
    {
      double x = (k == 0) ? e->x1 : e->x2;
      double y = (k == 0) ? e->y1 : e->y2;
      /* Keep a single entry per distinct point */
      bool seen = false;
      for (int j = 0; j < *count && ! seen; j++)
        seen = relate_same_point(x, y, result[j].x, result[j].y);
      if (seen)
        continue;
      if (! relate_point_on_linear_boundary(x, y, edges, nedges))
        continue;
      result[*count].x = x;
      result[*count].y = y;
      (*count)++;
    }
  }
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
    IntersectResult r = linesegm_intersect(a->x1, a->y1, a->dx, a->dy,
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
    e1[i] = (Edge *) meos_array_get(a1, i);
  for (int i = 0; i < n2; i++)
    e2[i] = (Edge *) meos_array_get(a2, i);

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
    int loc = relate_point_in_linear(b1[p].x, b1[p].y, e2, n2);
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
    int loc = relate_point_in_linear(b2[p].x, b2[p].y, e1, n1);
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
 * @brief Compute the DE-9IM matrix for a linear geometry and an
 * areal geometry.
 */
static void
relate_linear_area(const LWGEOM *line_geom, const LWGEOM *area_geom,
  const RelateOperands *ops, MeosDE9IM *m)
{
  MeosArray *la = relate_borrow_edges(ops, line_geom);
  MeosArray *aa = relate_borrow_edges(ops, area_geom);
  int nl = (int) la->count;
  int na = (int) aa->count;
  Edge **lines = palloc(sizeof(Edge *) * nl);
  Edge **area_edges = palloc(sizeof(Edge *) * na);
  for (int i = 0; i < nl; i++)
    lines[i] = (Edge *) meos_array_get(la, i);
  for (int i = 0; i < na; i++)
    area_edges[i] = (Edge *) meos_array_get(aa, i);

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
  {
    int maxbparams = 2 * nl + 2;
    double *bparams = palloc(sizeof(double) * maxbparams);
    for (int j = 0; j < na && m->eb == -1; j++)
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
        if (relate_point_in_linear(x, y, lines, nl) == 2)
        {
          m->eb = 1;
          break;
        }
      }
    }
    pfree(bparams);
  }
  for (int i = 0; i < nl; i++)
  {
    const Edge *line = lines[i];
    if (line->etype != EDGE_LINESEG && line->etype != EDGE_LINEARC)
      continue;
    int nparams = 0;
    /* The edge endpoints delimit the complete edge. */
    params[nparams++] = 0.0;
    params[nparams++] = 1.0;
    /* Intersect this linear edge with every area boundary edge. */
    for (int j = 0; j < na; j++)
    {
      const Edge *boundary = area_edges[j];
      if (boundary->etype != EDGE_POLYSEG && boundary->etype != EDGE_POLYARC)
        continue;
      relate_linear_area_edge_intersection(line, boundary, lines, nl, m,
        params, &nparams, maxparams);
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
      relate_linear_area_interval(line, params[j], params[j + 1], area_edges,
        na, m);
    }

    /* Classify the two endpoints of the linear edge.
     * These belong to the boundary or interior of the complete
     * linear geometry according to the endpoint parity established
     * by relate_point_in_linear(). */
    for (int endpoint = 0; endpoint < 2; endpoint++)
    {
      double x = endpoint == 0 ? line->x1 : line->x2;
      double y = endpoint == 0 ? line->y1 : line->y2;
      int lloc = relate_point_in_linear(x, y, lines, nl);
      if (lloc == 2)
        continue;
      int aloc = relate_point_in_area(x, y, area_edges, na);
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
   * is not completely contained in the area closure. */
  bool has_boundary = false;
  for (int i = 0; i < nl && !has_boundary; i++)
  {
    const Edge *line = lines[i];
    if (line->etype != EDGE_LINESEG && line->etype != EDGE_LINEARC)
      continue;
    if (! relate_edge_nonempty(line))
      continue;
    if (relate_point_on_linear_boundary(line->x1, line->y1, lines, nl) ||
        relate_point_on_linear_boundary(line->x2, line->y2, lines, nl))
      has_boundary = true;
  }

  if (has_boundary)
  {
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
        if (!relate_point_on_linear_boundary(x[k], y[k], lines, nl))
          continue;
        int aloc = relate_point_in_area(x[k], y[k], area_edges, na);
        if (aloc == 2)
          m->be = 0;
      }
    }
  }

  pfree(params); pfree(lines); pfree(area_edges);
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
  const RelateOperands *ops, MeosDE9IM *m)
{
  MeosDE9IM tmp;
  de9im_init(&tmp);
  relate_linear_area(line_geom, area_geom, ops, &tmp);

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
 * @brief Return true if an edge is a polygon boundary edge.
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
 * @brief Compute a point on an areal boundary edge.
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
 * @brief Return the parameter of a point on a polygon boundary edge.
 */
static double
relate_area_edge_parameter(const Edge *e, double x, double y)
{
  if (e->etype == EDGE_POLYSEG)
  {
    double dx = e->x2 - e->x1;
    double dy = e->y2 - e->y1;
    if (fabs(dx) >= fabs(dy))
    {
      if (fabs(dx) <= MEOS_GEOM_TOLERANCE)
        return 0.0;
      return (x - e->x1) / dx;
    }
    else
    {
      if (fabs(dy) <= MEOS_GEOM_TOLERANCE)
        return 0.0;
      return (y - e->y1) / dy;
    }
  }
  return relate_arc_parameter(e, x, y);
}

/**
 * @brief Add an intersection parameter to an array.
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
 * @brief Sort area-edge parameters.
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
 * @brief Determine intersections between two polygon boundary edges.
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
    IntersectResult r = linesegm_intersect(a->x1, a->y1, a->dx, a->dy,
      b->x1, b->y1, b->x2, b->y2);
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
 * @brief Test whether an open portion of one area boundary lies in the
 * interior of another area.
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
  return relate_point_in_area_index(x, y, area) == 0;
}

/**
 * @brief Classify the open portions of one polygon boundary edge.
 * @details The edge is split at every intersection with the other polygon
 * boundary. Since there is no boundary intersection inside an open
 * interval, one representative point is sufficient.
 */
static void
relate_area_edge_intervals(const Edge *edge, const RelateEdges *other,
  MeosDE9IM *m, bool first)
{
  /* Maximum number of intersections between one edge and one
   * circular/linear boundary edge is two */
  int maxparams = 2 * other->nedges + 2;
  double *params = palloc(sizeof(double) * maxparams);
  int nparams = 0;
  params[nparams++] = 0.0;
  params[nparams++] = 1.0;
  /* The edges this one can meet are those whose box meets its own, and an index
   * answers them in the place of a pass over the whole array. A boundary of a
   * few thousand edges leaves every pair but a handful standing apart, so the
   * pass is what the walk costs and the index is what removes it */
  MeosArray *candidates = NULL;
  int ncand = other->nedges;
  if (other->index)
  {
    STBox query;
    double pad = fmax(other->tol, edge->tol);
    stbox_set(true, false, false, 0, edge->xmin - pad, edge->xmax + pad,
      edge->ymin - pad, edge->ymax + pad, 0, 0, NULL, &query);
    candidates = meos_array_create(sizeof(int64));
    ncand = rtree_search(other->index, INDEX_OVERLAPS, &query, candidates);
  }
  for (int c = 0; c < ncand; c++)
  {
    int j = candidates ? (int) *(int64 *) meos_array_get(candidates, c) : c;
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
      if (relate_point_on_edge(oedge->x1, oedge->y1, edge))
      {
        relate_area_add_parameter(relate_area_edge_parameter(edge, oedge->x1,
          oedge->y1), params, &nparams, maxparams);
      }

      if (relate_point_on_edge(oedge->x2, oedge->y2, edge))
      {
        relate_area_add_parameter(relate_area_edge_parameter(edge, oedge->x2,
          oedge->y2), params, &nparams, maxparams);
      }
      continue;
    }
    for (int k = 0; k < n; k++)
    {
      double t = relate_area_edge_parameter(edge, ix[k], iy[k]);
      relate_area_add_parameter(t, params, &nparams, maxparams);
    }
  }

  if (candidates)
    meos_array_destroy(candidates);

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
    int loc = relate_point_in_area_index(x, y, other);
    if (loc == 0)
    {
      /* Boundary(A) ∩ Interior(B) or Interior(A) ∩ Boundary(B) */
      if (first)
        m->bi = 1;
      else
        m->ib = 1;
      /* If a non-zero boundary portion of A lies inside B,
       * then the interiors of A and B also intersect in a
       * two-dimensional neighbourhood of that portion. */
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
  pfree(params);
  return;
}

/**
 * @brief Process all point intersections between two area boundaries.
 */
static void
relate_area_boundary_points(const RelateEdges *are, const RelateEdges *bre,
  MeosDE9IM *m)
{
  Edge **aedges = are->edges, **bedges = bre->edges;
  int na = are->nedges, nb = bre->nedges;
  MeosArray *candidates = bre->index ? meos_array_create(sizeof(int64)) : NULL;
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
      STBox query;
      double pad = fmax(bre->tol, a->tol);
      stbox_set(true, false, false, 0, a->xmin - pad, a->xmax + pad,
        a->ymin - pad, a->ymax + pad, 0, 0, NULL, &query);
      ncand = rtree_search(bre->index, INDEX_OVERLAPS, &query, candidates);
    }
    for (int c = 0; c < ncand; c++)
    {
      int j = candidates ? (int) *(int64 *) meos_array_get(candidates, c) : c;
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
 * @brief Determine whether an area has an interior point in the
 * interior of another area.
 * @details We use boundary vertices first. If a vertex of A lies in the
 * interior of B, then a neighbourhood of that vertex inside A is
 * also inside B, proving II = 2.
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
    if (relate_point_in_area_index(e->x1, e->y1, other) == 0)
      return true;
    if (relate_point_in_area_index(e->x2, e->y2, other) == 0)
      return true;
  }
  return false;
}

/**
 * @brief Find an interior point of an areal geometry.
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
    /* Estimate a local tangent */
    double tx, ty;
    if (e->etype == EDGE_POLYSEG)
    {
      tx = e->x2 - e->x1;
      ty = e->y2 - e->y1;
    }
    else
    {
      /* Tangent to a circular arc at its midpoint */
      double sweep = e->ccw ?
        angle_normalize(e->theta1 - e->theta0) :
        angle_normalize(e->theta0 - e->theta1);
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
    if (len <= MEOS_GEOM_TOLERANCE)
      return false;
    tx /= len;
    ty /= len;

    /* Two possible normal directions. The actual polygon orientation
     * is irrelevant because we test both sides. */
    double nx = -ty;
    double ny =  tx;

    /* The witness steps off the edge far enough to leave the band within
     * which #point_on_segment reads a point as lying on that edge, since
     * #relate_point_in_area answers boundary before it answers interior and a
     * witness inside the band would be taken for a boundary point and the
     * interior would be reported empty. Ten times the band leaves the margin,
     * and the step stays small relative to the edge so the witness cannot
     * cross to the far side of the geometry. */
    double eps = fmax(MEOS_GEOM_TOLERANCE * 10.0, len * 1e-9);
    eps = fmax(eps, 10.0 * coordinate_tolerance(px, py));
    double qx = px + eps * nx;
    double qy = py + eps * ny;
    if (relate_point_in_area_index(qx, qy, self) == 0)
    {
      *x = qx;
      *y = qy;
      return true;
    }
    qx = px - eps * nx;
    qy = py - eps * ny;
    if (relate_point_in_area_index(qx, qy, self) == 0)
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
    if (relate_point_in_area_index(x, y, other) == location)
      return true;
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
 * Arcs are left to the other routes. The endpoints of an arc do not determine
 * the curve between them, so four signs taken on them say nothing about
 * whether the arcs meet
 * @param[in] a,b Edges of the two geometries, the second read out of its index
 * where it carries one
 */
static bool
relate_area_boundaries_cross(const RelateEdges *a, const RelateEdges *b)
{
  for (int i = 0; i < a->nedges; i++)
  {
    const Edge *ea = a->edges[i];
    if (ea->etype != EDGE_POLYSEG)
      continue;
    if (! b->index)
    {
      for (int j = 0; j < b->nedges; j++)
      {
        const Edge *eb = b->edges[j];
        if (eb->etype == EDGE_POLYSEG && relate_edges_cross(ea, eb))
          return true;
      }
      continue;
    }
    /* Two segments that cross meet at a point lying on both, so that point is
     * in both boxes and the boxes overlap. The query therefore needs NO
     * padding: an unpadded box already admits every crossing there is, and
     * unlike the point-location queries the test behind it reads no tolerance
     * that a pad would have to match */
    STBox query;
    stbox_set(true, false, false, 0, ea->xmin, ea->xmax, ea->ymin, ea->ymax,
      0, 0, NULL, &query);
    MeosArray *candidates = meos_array_create(sizeof(int64));
    int nc = rtree_search(b->index, INDEX_OVERLAPS, &query, candidates);
    bool result = false;
    for (int c = 0; c < nc && ! result; c++)
    {
      const Edge *eb = b->edges[*(int64 *) meos_array_get(candidates, c)];
      if (eb->etype == EDGE_POLYSEG)
        result = relate_edges_cross(ea, eb);
    }
    meos_array_destroy(candidates);
    if (result)
      return true;
  }
  return false;
}

/**
 * @brief Determine whether the interiors of two areal geometries
 * intersect in dimension 2.
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
  if (relate_area_interior_point_located(a, b, 0))
    return true;
  if (relate_area_interior_point_located(b, a, 0))
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
 * @brief Compute the DE-9IM matrix for two areal geometries.
 */
static void
relate_area_area(const LWGEOM *g1, const LWGEOM *g2,
  const RelateOperands *ops, MeosDE9IM *m)
{
  MeosArray *a1 = relate_borrow_edges(ops, g1);
  MeosArray *a2 = relate_borrow_edges(ops, g2);
  int n1 = (int) a1->count;
  int n2 = (int) a2->count;
  Edge **e1 = palloc(sizeof(Edge *) * n1);
  Edge **e2 = palloc(sizeof(Edge *) * n2);
  for (int i = 0; i < n1; i++)
    e1[i] = (Edge *) meos_array_get(a1, i);
  for (int i = 0; i < n2; i++)
    e2[i] = (Edge *) meos_array_get(a2, i);

  /* Every question this matrix asks about one point reads one of the two edge
   * arrays, and asks it once per edge of the other, so an unindexed walk costs
   * their PRODUCT while indexing each array costs one pass over it. The two
   * indexes are therefore built ONCE here and read by every step below */
  RelateEdges re1, re2;
  bool index = ((int64) n1 * (int64) n2 >= RELATE_INDEX_MIN_PAIRS);
  relate_edges_init(&re1, e1, n1, index);
  relate_edges_init(&re2, e2, n2, index);

  /* Two-dimensional interior/interior intersection. */
  if (relate_area_interiors_intersect(&re1, &re2))
  {
    m->ii = 2;
  }

  /* Boundary(A) / Interior(B) and Interior(A) / Boundary(B) are determined by
   * splitting every boundary edge at the intersections with the other
   * boundary. */
  for (int i = 0; i < n1; i++)
  {
    if (!relate_area_boundary_edge(e1[i]))
      continue;
    relate_area_edge_intervals(e1[i], &re2, m, true);
  }
  for (int i = 0; i < n2; i++)
  {
    if (!relate_area_boundary_edge(e2[i]))
      continue;
    relate_area_edge_intervals(e2[i], &re1, m, false);
  }

  /* Boundary / Boundary.
   * Point intersections give dimension 0.
   * Coincident/overlapping boundary portions give dimension 1. */
  relate_area_boundary_points(&re1, &re2, m);

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

  if (relate_area_interior_point_located(&re1, &re2, 2))
    de9im_add(&m->ie, 2);
  if (relate_area_interior_point_located(&re2, &re1, 2))
    de9im_add(&m->ei, 2);

  /* Exterior / Exterior.
   * Two ordinary finite areal geometries always have a
   * two-dimensional common exterior. */
  de9im_add(&m->ee, 2);

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
    c->edges[i] = (Edge *) meos_array_get(c->arr, i);
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
    if (relate_point_in_area_index(x, y, &comps[i].re) != 2)
      return true;
  return false;
}

/**
 * @brief Return the distance from a point to an edge
 */
static double
relate_edge_distance(double x, double y, const Edge *e)
{
  double d1 = hypot(x - e->x1, y - e->y1);
  double d2 = hypot(x - e->x2, y - e->y2);
  double result = Min(d1, d2);
  if (e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC)
  {
    if (arc_contains_angle(e, atan2(y - e->cy, x - e->cx)))
      result = Min(result, fabs(hypot(x - e->cx, y - e->cy) - e->radius));
    return result;
  }
  /* The nearest point of a segment is its projection where that falls within
   * the segment, and an endpoint otherwise */
  double len2 = e->dx * e->dx + e->dy * e->dy;
  if (len2 > 0)
  {
    double t = ((x - e->x1) * e->dx + (y - e->y1) * e->dy) / len2;
    if (t > 0 && t < 1)
      result = Min(result, fabs((x - e->x1) * e->dy - (y - e->y1) * e->dx) /
        sqrt(len2));
  }
  return result;
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
  bool indexed = (ncomp > 0);
  for (int i = 0; i < ncomp && indexed; i++)
    indexed = (comps[i].re.index != NULL);
  if (indexed)
  {
    MeosArray *candidates = meos_array_create(sizeof(int64));
    double r = seed > MEOS_GEOM_TOLERANCE ? seed : MEOS_GEOM_TOLERANCE;
    for (int round = 0; round < 64; round++)
    {
      double best = -1;
      for (int i = 0; i < ncomp; i++)
      {
        STBox query;
        stbox_set(true, false, false, 0, x - r, x + r, y - r, y + r, 0, 0,
          NULL, &query);
        int nc = rtree_search(comps[i].re.index, INDEX_OVERLAPS, &query,
          candidates);
        for (int c = 0; c < nc; c++)
        {
          int j = (int) *(int64 *) meos_array_get(candidates, c);
          double d = relate_edge_distance(x, y, comps[i].edges[j]);
          /* An edge the point LIES ON is no clearance from it. What the
           * distance misses the edge by is a property of the arithmetic --
           * a few units in the last place of the largest coordinate -- so an
           * absolute floor leaves a point on its own edge reading as a
           * feature a rounding step away */
          if (d <= fmax(comps[i].edges[j]->tol, MEOS_GEOM_TOLERANCE))
            continue;
          if (best < 0 || d < best)
            best = d;
        }
      }
      if (best >= 0 && best <= r)
      {
        meos_array_destroy(candidates);
        return best;
      }
      r *= 4;
    }
    meos_array_destroy(candidates);
  }
  double result = -1;
  for (int i = 0; i < ncomp; i++)
    for (int j = 0; j < comps[i].nedges; j++)
    {
      double d = relate_edge_distance(x, y, comps[i].edges[j]);
      /* An edge the point LIES ON is no clearance from it. What the
       * distance misses the edge by is a property of the arithmetic --
       * a few units in the last place of the largest coordinate -- so an
       * absolute floor leaves a point on its own edge reading as a
       * feature a rounding step away */
      if (d <= fmax(comps[i].edges[j]->tol, MEOS_GEOM_TOLERANCE))
        continue;
      if (result < 0 || d < result)
        result = d;
    }
  return result;
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
  /* The portion's own edge sets the scale the search starts at: the feature
   * nearest a point of a boundary is normally the next one along it */
  double delta = relate_clearance(x, y, comps, ncomp,
    e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC ?
      e->radius : e->length);
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
    if (fabs(a->cx - b->cx) > MEOS_GEOM_TOLERANCE ||
        fabs(a->cy - b->cy) > MEOS_GEOM_TOLERANCE ||
        fabs(a->radius - b->radius) > MEOS_GEOM_TOLERANCE)
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
  /* The two components may traverse the portion in opposite directions */
  return (relate_same_point(a->x1, a->y1, b->x1, b->y1) &&
      relate_same_point(a->x2, a->y2, b->x2, b->y2)) ||
    (relate_same_point(a->x1, a->y1, b->x2, b->y2) &&
      relate_same_point(a->x2, a->y2, b->x1, b->y1));
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
relate_union_edges(const LWGEOM *geom)
{
  MeosArray *all = geom_extract_edges(geom);
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
  /* Every edge is asked against every other, so the walk costs the SQUARE of
   * the array while indexing it costs one pass. Two edges that meet have
   * boxes that meet, so the index answers the pairs the walk would split at
   * and leaves out the ones it would only reject */
  Edge **edges = palloc(sizeof(Edge *) * Max(nall, 1));
  for (int i = 0; i < nall; i++)
    edges[i] = (Edge *) meos_array_get(all, i);
  RelateEdges re;
  relate_edges_init(&re, edges, nall, index);
  MeosArray *candidates = re.index ? meos_array_create(sizeof(int64)) : NULL;
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
    /* The query is grown by the widest tolerance the array asks for, which is
     * what makes it admit every edge the walk would have solved against */
    int ncand = nall;
    if (re.index)
    {
      STBox query;
      double pad = fmax(re.tol, e->tol);
      stbox_set(true, false, false, 0, e->xmin - pad, e->xmax + pad,
        e->ymin - pad, e->ymax + pad, 0, 0, NULL, &query);
      ncand = rtree_search(re.index, INDEX_OVERLAPS, &query, candidates);
    }
    for (int c = 0; c < ncand; c++)
    {
      int j = candidates ? (int) *(int64 *) meos_array_get(candidates, c) : c;
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
        if (relate_point_on_edge(other->x1, other->y1, e))
          relate_area_add_parameter(relate_area_edge_parameter(e, other->x1,
            other->y1), params, &nparams, maxparams);
        if (relate_point_on_edge(other->x2, other->y2, e))
          relate_area_add_parameter(relate_area_edge_parameter(e, other->x2,
            other->y2), params, &nparams, maxparams);
        continue;
      }
      for (int k = 0; k < n; k++)
        relate_area_add_parameter(relate_area_edge_parameter(e, ix[k], iy[k]),
          params, &nparams, maxparams);
    }
    qsort(params, nparams, sizeof(double), relate_area_parameter_cmp);

    for (int k = 0; k < nparams - 1; k++)
    {
      double t0 = params[k], t1 = params[k + 1];
      if (t1 - t0 <= MEOS_GEOM_TOLERANCE)
        continue;
      if (relate_portion_inside_union(e, (t0 + t1) * 0.5, comps, ncomp))
        continue;
      /* The portion lies on the boundary of the union, so it is kept */
      Edge piece = *e;
      relate_area_edge_point(e, t0, &piece.x1, &piece.y1);
      relate_area_edge_point(e, t1, &piece.x2, &piece.y2);
      if (piece.etype == EDGE_POLYARC)
      {
        piece.theta0 = atan2(piece.y1 - piece.cy, piece.x1 - piece.cx);
        piece.theta1 = atan2(piece.y2 - piece.cy, piece.x2 - piece.cx);
        arc_set_bbox(&piece);
      }
      else
      {
        piece.dx = piece.x2 - piece.x1;
        piece.dy = piece.y2 - piece.y1;
        piece.length = hypot(piece.dx, piece.dy);
        piece.xmin = Min(piece.x1, piece.x2);
        piece.xmax = Max(piece.x1, piece.x2);
        piece.ymin = Min(piece.y1, piece.y2);
        piece.ymax = Max(piece.y1, piece.y2);
      }
      /* A portion two components bound alike is bounded once by the union */
      bool seen = false;
      for (uint32_t q = 0; q < result->count && ! seen; q++)
        seen = relate_same_portion((Edge *) meos_array_get(result, q), &piece);
      if (! seen)
        meos_array_add(result, &piece);
    }
  }
  pfree(params);
  if (candidates)
    meos_array_destroy(candidates);
  relate_edges_clear(&re);
  pfree(edges);

  relate_comps_free(comps, ncomp);
  meos_array_destroy(all);
  return result;
}

/**
 * @brief Return the edges a DE-9IM cell is computed on
 * @details Every geometry answers with its own edges, except a value holding
 * several components, whose topology is that of their union. Two members of a
 * multipolygon may share a boundary edge -- edge-adjacent polygons are a valid
 * multipolygon -- and that edge lies in the interior of what they cover
 * together, so reading the members' own edges reports it as boundary
 */
static MeosArray *
relate_extract_edges(const LWGEOM *geom)
{
  switch (geom->type)
  {
    case MULTIPOLYGONTYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    case COLLECTIONTYPE:
      return relate_union_edges(geom);
    default:
      return geom_extract_edges(geom);
  }
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
      return 2;
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
    edges[i] = (Edge *) meos_array_get(arr, i);
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
    Edge *e = (Edge *) meos_array_get(arr, i);
    if (e->etype == EDGE_POINT)
    {
      result = relate_in_area_union(e->x1, e->y1, comps, ncomp) ||
        (nledges > 0 && relate_point_in_linear(e->x1, e->y1, ledges,
          nledges) != 2);
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
      ledges[i] = (Edge *) meos_array_get(larr, i);
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
  const RelateOperands *ops, MeosDE9IM *m)
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
    relate_linear_area(g1, g2, ops, m);
  else if (mask1 == 4 && mask2 == 2)
    relate_area_linear(g1, g2, ops, m);
  else
    relate_area_area(g1, g2, ops, m);
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
  const RelateOperands *ops, MeosDE9IM *m)
{
  /* A single dimension on each side is answered directly */
  if ((mask1 & (mask1 - 1)) == 0 && (mask2 & (mask2 - 1)) == 0)
  {
    relate_simple(g1, g2, mask1, mask2, ops, m);
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
        relate_simple(s1[j], s2[k], 1 << j, 1 << k, ops, &cells[j][k]);
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
 * @brief Compute the DE-9IM intersection matrix, reading the edges a
 * relationship has already extracted where it has any
 * @param[in] g1,g2 Geometries
 * @param[in] ops Operands of the relationship the matrix answers a step of,
 * NULL where the matrix is asked for on its own
 * @param[out] result The matrix
 * @return true if the geometry pair is supported, which is what
 * #geom_meos_supported answers of each geometry
 */
static bool
relate_matrix(const LWGEOM *g1, const LWGEOM *g2, const RelateOperands *ops,
  char result[10])
{
  assert(g1); assert(g2); assert(result);

  /* Every native implementation reads the same predicate: the engine answers
   * a geometry the edge decomposition reaches, and nothing else */
  if (! geom_meos_supported(g1) || ! geom_meos_supported(g2))
    return false;

  MeosDE9IM m;
  de9im_init(&m);

  /* An empty operand meets nothing, so the interior and the boundary of the
   * other operand fall entirely in its exterior, each keeping its own
   * dimension. The two exteriors meet in dimension 2 */
  bool empty1 = lwgeom_is_empty(g1);
  bool empty2 = lwgeom_is_empty(g2);
  if (empty1 || empty2)
  {
    if (! empty2)
    {
      de9im_add(&m.ei, (int8_t) relate_dimension(g2));
      de9im_add(&m.eb, relate_boundary_dimension(g2));
    }
    if (! empty1)
    {
      de9im_add(&m.ie, (int8_t) relate_dimension(g1));
      de9im_add(&m.be, relate_boundary_dimension(g1));
    }
    de9im_add(&m.ee, 2);
    de9im_to_string(&m, result);
    return true;
  }

  int mask1 = relate_dim_mask(g1);
  int mask2 = relate_dim_mask(g2);
  relate_dispatch(g1, g2, mask1, mask2, ops, &m);

  de9im_to_string(&m, result);
  return true;
}

/**
 * @brief Compute the DE-9IM intersection matrix
 * @details This is the native counterpart of PostGIS @p ST_Relate
 * @return true if the geometry pair is supported, which is what
 * #geom_meos_supported answers of each geometry. A false return means the
 * pair is outside that coverage, @b not that the geometries are unrelated, so
 * a caller must answer it another way rather than read @p result
 */
bool
meos_relate(const LWGEOM *g1, const LWGEOM *g2, char result[10])
{
  return relate_matrix(g1, g2, NULL, result);
}

/**
 * @brief Return true if a point lies on the curve an edge draws, whatever
 * part of a geometry that edge bounds
 * @details #relate_point_on_boundary answers the same question of an areal
 * boundary alone, and a shared point is a question about the curves
 */
static bool
relate_point_on_edge(double x, double y, const Edge *e)
{
  switch (e->etype)
  {
    case EDGE_POINT:
      return fabs(x - e->x1) <= MEOS_GEOM_TOLERANCE &&
        fabs(y - e->y1) <= MEOS_GEOM_TOLERANCE;
    case EDGE_LINEARC:
    case EDGE_POLYARC:
      return point_on_arc(x, y, e);
    case EDGE_LINESEG:
    case EDGE_POLYSEG:
      return point_on_segment(x, y, e->x1, e->y1, e->x2, e->y2);
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
    IntersectResult r = linesegm_intersect(ea.x1, ea.y1, ea.dx, ea.dy,
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
  double x, y;
  if (e->etype == EDGE_POINT)
  {
    x = e->x1;
    y = e->y1;
  }
  else
    relate_edge_point(e, 0.5, &x, &y);
  if (! other->index)
    return point_in_polygon(x, y, other->edges, other->nedges) != 0;
  return point_in_polygon_index(x, y, other->edges, other->nedges,
    other->index, other->xmax) != 0;
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
  STBox query;
  stbox_set(true, false, false, 0, xmin - re->tol, xmax + re->tol,
    ymin - re->tol, ymax + re->tol, 0, 0, NULL, &query);
  return rtree_search(re->index, INDEX_OVERLAPS, &query, candidates);
}

/**
 * @brief Return true if a curve meets any curve of an edge array, reading the
 * edges whose box can meet its own out of the array's index
 */
static bool
relate_edges_meet_any(const Edge *a, const RelateEdges *other)
{
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
  MeosArray *candidates = meos_array_create(sizeof(int64));
  int nc = relate_edges_candidates(other, a->xmin, a->xmax, a->ymin, a->ymax,
    candidates);
  bool result = false;
  for (int c = 0; c < nc && ! result; c++)
  {
    const Edge *b = other->edges[*(int64 *) meos_array_get(candidates, c)];
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
 * @brief Return true if a point stands on any edge of an array, reading the
 * edges that can carry it out of the array's index
 */
static bool
relate_point_on_any_edge(double x, double y, const RelateEdges *other)
{
  if (! other->index)
  {
    for (int j = 0; j < other->nedges; j++)
      if (relate_point_on_edge(x, y, other->edges[j]))
        return true;
    return false;
  }
  MeosArray *candidates = meos_array_create(sizeof(int64));
  int nc = relate_edges_candidates(other, x, x, y, y, candidates);
  bool result = false;
  for (int c = 0; c < nc && ! result; c++)
    result = relate_point_on_edge(x, y,
      other->edges[*(int64 *) meos_array_get(candidates, c)]);
  meos_array_destroy(candidates);
  return result;
}

/**
 * @brief Return true if two geometries share a point, read from their edges
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
 * common one, a pair whose boxes overlap while the geometries keep apart
 */
static bool
relate_edges_intersect(const RelateEdges *re1, const RelateEdges *re2)
{
  Edge **e1 = re1->edges, **e2 = re2->edges;
  int n1 = re1->nedges, n2 = re2->nedges;

  /* Geometries whose bounding boxes stand apart share no point, and reading
   * that first is what keeps a pair that does not meet from costing a pass
   * over every edge of one against every edge of the other */
  if (! relate_edges_boxes_overlap(e1, n1, e2, n2))
    return false;

  /* A point of one geometry standing on the other */
  for (int i = 0; i < n1; i++)
  {
    if (e1[i]->etype != EDGE_POINT)
      continue;
    if (relate_point_on_any_edge(e1[i]->x1, e1[i]->y1, re2))
      return true;
  }
  for (int j = 0; j < n2; j++)
  {
    if (e2[j]->etype != EDGE_POINT)
      continue;
    if (relate_point_on_any_edge(e2[j]->x1, e2[j]->y1, re1))
      return true;
  }

  /* Two curves meeting, the bounding boxes deciding which pairs are worth
   * solving */
  for (int i = 0; i < n1; i++)
  {
    if (e1[i]->etype == EDGE_POINT)
      continue;
    if (relate_edges_meet_any(e1[i], re2))
      return true;
  }

  /* No curve of either geometry meets the other, so each edge lies wholly
   * inside or wholly outside the surfaces of the other geometry */
  for (int i = 0; i < n1; i++)
    if (relate_edge_inside_area(e1[i], re2))
      return true;
  for (int j = 0; j < n2; j++)
    if (relate_edge_inside_area(e2[j], re1))
      return true;
  return false;
}

/**
 * @brief Return true if a point lies in the point set an edge array draws
 * @details That is, inside a surface the array bounds, or on one of its
 * curves or points. #point_in_polygon passes over every edge bounding no
 * surface, so an array mixing dimensions is read correctly
 */
static bool
relate_point_in_edges(double x, double y, Edge **edges, int nedges)
{
  if (point_in_polygon(x, y, edges, nedges) != 0)
    return true;
  for (int i = 0; i < nedges; i++)
    if (relate_point_on_edge(x, y, edges[i]))
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
 * @return False if @p e2 draws a point outside what @p e1 draws, which settles
 * that the second geometry is not covered by the first
 */
static bool
relate_edges_cover(Edge **e1, int n1, Edge **e2, int n2)
{
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
      result = relate_point_in_edges(e->x1, e->y1, e1, n1);
      continue;
    }
    int nparams = 0;
    params[nparams++] = 0.0;
    params[nparams++] = 1.0;
    for (int i = 0; i < n1; i++)
    {
      const Edge *o = e1[i];
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
        if (relate_point_on_edge(o->x1, o->y1, e))
          relate_area_add_parameter(relate_any_edge_parameter(e, o->x1, o->y1),
            params, &nparams, maxparams);
        if (relate_point_on_edge(o->x2, o->y2, e))
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
      result = relate_point_in_edges(x, y, e1, n1);
    }
  }
  pfree(params);
  return result;
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
  MeosArray *a1 = ops->op[0].arr, *a2 = ops->op[1].arr;
  int n1 = (int) a1->count, n2 = (int) a2->count;
  Edge **e1 = palloc(sizeof(Edge *) * (n1 ? n1 : 1));
  Edge **e2 = palloc(sizeof(Edge *) * (n2 ? n2 : 1));
  for (int i = 0; i < n1; i++)
    e1[i] = (Edge *) meos_array_get(a1, i);
  for (int j = 0; j < n2; j++)
    e2[j] = (Edge *) meos_array_get(a2, j);

  /* Every question this kernel asks reads one of the two arrays once per edge
   * of the other, so an unindexed walk costs their PRODUCT wherever no witness
   * ends it early. The gate is the one the matrix uses, and below it the walk
   * is the cheaper of the two and the index is not built */
  RelateEdges re1, re2;
  bool index = ((int64) n1 * (int64) n2 >= RELATE_INDEX_MIN_PAIRS);
  relate_edges_init(&re1, e1, n1, index);
  relate_edges_init(&re2, e2, n2, index);

  *result = relate_edges_intersect(&re1, &re2);

  relate_edges_clear(&re1); relate_edges_clear(&re2);
  pfree(e1); pfree(e2);
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
  MeosArray *a1 = ops->op[0].arr, *a2 = ops->op[1].arr;
  int n1 = (int) a1->count, n2 = (int) a2->count;
  Edge **e1 = palloc(sizeof(Edge *) * (n1 ? n1 : 1));
  Edge **e2 = palloc(sizeof(Edge *) * (n2 ? n2 : 1));
  for (int i = 0; i < n1; i++)
    e1[i] = (Edge *) meos_array_get(a1, i);
  for (int j = 0; j < n2; j++)
    e2[j] = (Edge *) meos_array_get(a2, j);

  *result = relate_edges_cover(e1, n1, e2, n2);

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
      char m[10];
      covered = relate_matrix(g1, g2, &ops, m);
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
  if (! geom_meos_supported(geom))
    return NULL;
  struct RelateCtx *ctx = palloc(sizeof(struct RelateCtx));
  ctx->op.geom = geom;
  ctx->op.arr = relate_extract_edges(geom);
  return ctx;
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
  if (! geom_meos_supported(g1) || ! geom_meos_supported(g2))
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
 * @return True if the pair is covered, which is what #geom_meos_supported
 * answers of each geometry. A false return means the pair is outside that
 * coverage, @b not that the pattern fails, so a caller must answer it another
 * way rather than read @p result
 */
bool
meos_relate_pattern(const LWGEOM *g1, const LWGEOM *g2, const char *pattern,
  bool *result)
{
  assert(result);
  char matrix[10];
  if (! meos_relate(g1, g2, matrix))
    return false;
  *result = de9im_match(matrix, pattern);
  return true;
}

/*****************************************************************************
 * Union of the linear and point components of a geometry
 *****************************************************************************/

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
 * @brief Return the sub-segment an edge draws between two of its parameters
 */
static LWGEOM *
linear_union_subsegment(const Edge *edge, double t0, double t1, int32_t srid)
{
  assert(edge);
  POINTARRAY *pa = ptarray_construct_empty(0, 0, 2);
  POINT4D p;
  memset(&p, 0, sizeof(POINT4D));
  /* An end of the edge is written as the edge writes it: @p x1 + 1.0 * dx is
   * not @p x2 in floating point, and a piece spanning the whole edge has to
   * reproduce the coordinates it was read from rather than a rounding of them */
  if (t0 <= 0.0)
  {
    p.x = edge->x1; p.y = edge->y1;
  }
  else
  {
    p.x = edge->x1 + t0 * edge->dx;
    p.y = edge->y1 + t0 * edge->dy;
  }
  ptarray_append_point(pa, &p, LW_TRUE);
  if (t1 >= 1.0)
  {
    p.x = edge->x2; p.y = edge->y2;
  }
  else
  {
    p.x = edge->x1 + t1 * edge->dx;
    p.y = edge->y1 + t1 * edge->dy;
  }
  ptarray_append_point(pa, &p, LW_TRUE);
  return lwline_as_lwgeom(lwline_construct(srid, NULL, pa));
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
    if (((const Edge *) meos_array_get(edges, i))->etype != EDGE_LINESEG)
      return false;
  return true;
}

/**
 * @brief Return true if two points are the same to what their coordinates
 * resolve
 */
static bool
linear_union_same_point(const POINT2D *p1, const POINT2D *p2)
{
  assert(p1); assert(p2);
  return fabs(p1->x - p2->x) <= coordinate_tolerance(p1->x, p2->x) &&
    fabs(p1->y - p2->y) <= coordinate_tolerance(p1->y, p2->y);
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
 * @brief Sew a set of pieces into the curves they draw
 * @details Each piece continues a chain at one of its two ends, in either
 * direction, and the scan restarts whenever one attaches, the enlarged chain
 * reaching further than it did. A piece continuing neither end of the chain
 * under construction starts a chain of its own, so a set drawing a fork is
 * answered as the several curves it takes to walk it rather than refused
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
      for (int p = 0; p < npieces && ! attached; p++)
      {
        if (used[p])
          continue;
        const POINTARRAY *ppa = ((const LWLINE *) pieces[p])->points;
        if (pieces[p]->type != LINETYPE || ppa->npoints < 2)
        {
          used[p] = true;
          continue;
        }
        /* The two ends are COPIED: appending may move the chain in memory */
        POINT2D head = chain.points[0], tail = chain.points[chain.count - 1];
        const POINT2D *first = getPoint2d_cp(ppa, 0);
        const POINT2D *last = getPoint2d_cp(ppa, ppa->npoints - 1);
        if (linear_union_same_point(&head, first) ||
            linear_union_same_point(&head, last))
          linear_chain_reverse(&chain);
        else if (! linear_union_same_point(&tail, first) &&
                 ! linear_union_same_point(&tail, last))
          continue;
        POINT2D end = chain.points[chain.count - 1];
        if (linear_union_same_point(&end, first))
          for (uint32_t i = 1; i < ppa->npoints; i++)
            linear_chain_append(&chain, getPoint2d_cp(ppa, i));
        else
          for (int i = (int) ppa->npoints - 2; i >= 0; i--)
            linear_chain_append(&chain, getPoint2d_cp(ppa, i));
        used[p] = true; attached = true;
      }
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
 * @return The union as a single line, or @p NULL where either geometry carries
 * an edge that is not a straight segment, and where what the two draw together
 * is more than one line -- a fork cannot be walked as one curve, and a caller
 * that has another way to answer may take it
 */
static LWGEOM *
linear_union_merge(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  MeosArray *e1 = geom_extract_edges(geom1);
  MeosArray *e2 = geom_extract_edges(geom2);
  if (! e1 || ! e2 || ! linear_union_straight(e1) || ! linear_union_straight(e2))
  {
    if (e1) meos_array_destroy(e1);
    if (e2) meos_array_destroy(e2);
    return NULL;
  }
  int n1 = (int) e1->count, n2 = (int) e2->count;
  int32_t srid = lwgeom_get_srid(geom1);

  /* Every segment of the second geometry contributes the parts of itself the
   * first does not cover, so at most one piece more than the overlaps it
   * carries */
  LWGEOM **pieces = palloc(sizeof(LWGEOM *) * (size_t) (n2 * (n1 + 1) + 1));
  int npieces = 0;
  double *lo = palloc(sizeof(double) * (size_t) (n1 + 1));
  double *hi = palloc(sizeof(double) * (size_t) (n1 + 1));

  for (int j = 0; j < n2; j++)
  {
    const Edge *b = (const Edge *) meos_array_get(e2, j);
    int nover = 0;
    for (int i = 0; i < n1; i++)
    {
      const Edge *a = (const Edge *) meos_array_get(e1, i);
      IntersectResult r = linesegm_intersect(b->x1, b->y1, b->dx, b->dy,
        a->x1, a->y1, a->x2, a->y2);
      /* A pair meeting at a point leaves the segment whole; only a shared
       * stretch removes anything from it */
      if (r.type != INTERSECT_OVERLAP)
        continue;
      lo[nover] = (r.t0 < r.t1) ? r.t0 : r.t1;
      hi[nover] = (r.t0 < r.t1) ? r.t1 : r.t0;
      nover++;
    }
    /* The overlaps read in the order the segment is walked */
    for (int p = 1; p < nover; p++)
    {
      double l = lo[p], h = hi[p];
      int q = p - 1;
      while (q >= 0 && lo[q] > l)
      {
        lo[q + 1] = lo[q]; hi[q + 1] = hi[q]; q--;
      }
      lo[q + 1] = l; hi[q + 1] = h;
    }
    /* A piece shorter than what the segment's own coordinates resolve is not
     * linework, it is the rounding of the parameter that produced it */
    double mint = (b->length > 0.0) ? b->tol / b->length : 1.0;
    double cur = 0.0;
    for (int p = 0; p < nover; p++)
    {
      if (lo[p] > cur + mint)
        pieces[npieces++] = linear_union_subsegment(b, cur, lo[p], srid);
      if (hi[p] > cur)
        cur = hi[p];
    }
    if (1.0 > cur + mint)
      pieces[npieces++] = linear_union_subsegment(b, cur, 1.0, srid);
  }
  pfree(lo); pfree(hi);
  meos_array_destroy(e1); meos_array_destroy(e2);

  /* The first curve seeds the sewing, so the pieces attach to it in the order
   * it is walked */
  LWGEOM **all = palloc(sizeof(LWGEOM *) * (size_t) (npieces + 1));
  all[0] = lwgeom_clone_deep(geom1);
  for (int p = 0; p < npieces; p++)
    all[p + 1] = pieces[p];
  pfree(pieces);
  LWGEOM **curves = NULL;
  int ncurves = linear_union_chain_all(all, npieces + 1, srid, &curves);
  for (int p = 0; p <= npieces; p++)
    lwgeom_free(all[p]);
  pfree(all);
  /* What the two draw together is a single curve, or it is a fork this entry
   * does not answer */
  LWGEOM *merged = NULL;
  if (ncurves == 1)
    merged = curves[0];
  else
    for (int c = 0; c < ncurves; c++)
      lwgeom_free(curves[c]);
  if (curves)
    pfree(curves);
  return merged;
}

/**
 * @brief Return a line that walks its own points once
 * @details A union is a POINT SET, so the measure of the answer is the measure
 * of that set: a line walking a stretch twice -- @p LINESTRING(6 5,6 3,6 4)
 * traverses @p (6 3)-(6 4) on the way down and again on the way back -- reports
 * a length larger than the set it covers.  Each segment keeps only what no
 * EARLIER segment of the same line already covers, and what remains is sewn
 * back into the curve it draws
 * @param[in] geom Geometry
 * @param[out] curves Curves the line draws walking each of its points once,
 * allocated here and owned by the caller
 * @return Number of curves, or 0 where the geometry is not a line of straight
 * segments -- the caller then keeps the line as it stands. Dropping a doubled
 * stretch can leave what remains in SEVERAL pieces, so a line does not always
 * walk its own points as one curve
 */
static int
linear_union_dissolve(const LWGEOM *geom, LWGEOM ***curves)
{
  assert(geom); assert(curves);
  *curves = NULL;
  if (geom->type != LINETYPE)
    return 0;
  MeosArray *edges = geom_extract_edges(geom);
  if (! edges || ! linear_union_straight(edges))
  {
    if (edges) meos_array_destroy(edges);
    return 0;
  }
  int nedges = (int) edges->count;
  if (nedges < 2)
  {
    meos_array_destroy(edges);
    return 0;
  }
  int32_t srid = lwgeom_get_srid(geom);

  LWGEOM **pieces = palloc(sizeof(LWGEOM *) * (size_t) (nedges * nedges + 1));
  int npieces = 0;
  double *lo = palloc(sizeof(double) * (size_t) (nedges + 1));
  double *hi = palloc(sizeof(double) * (size_t) (nedges + 1));
  for (int i = 0; i < nedges; i++)
  {
    const Edge *b = (const Edge *) meos_array_get(edges, i);
    int nover = 0;
    for (int j = 0; j < i; j++)
    {
      const Edge *a = (const Edge *) meos_array_get(edges, j);
      IntersectResult r = linesegm_intersect(b->x1, b->y1, b->dx, b->dy,
        a->x1, a->y1, a->x2, a->y2);
      if (r.type != INTERSECT_OVERLAP)
        continue;
      lo[nover] = (r.t0 < r.t1) ? r.t0 : r.t1;
      hi[nover] = (r.t0 < r.t1) ? r.t1 : r.t0;
      nover++;
    }
    for (int p = 1; p < nover; p++)
    {
      double l = lo[p], h = hi[p];
      int q = p - 1;
      while (q >= 0 && lo[q] > l)
      {
        lo[q + 1] = lo[q]; hi[q + 1] = hi[q]; q--;
      }
      lo[q + 1] = l; hi[q + 1] = h;
    }
    double mint = (b->length > 0.0) ? b->tol / b->length : 1.0;
    double cur = 0.0;
    for (int p = 0; p < nover; p++)
    {
      if (lo[p] > cur + mint)
        pieces[npieces++] = linear_union_subsegment(b, cur, lo[p], srid);
      if (hi[p] > cur)
        cur = hi[p];
    }
    if (1.0 > cur + mint)
      pieces[npieces++] = linear_union_subsegment(b, cur, 1.0, srid);
  }
  pfree(lo); pfree(hi);
  meos_array_destroy(edges);

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

  /* One component is its own union */
  if (linear_union_type(geom->type))
    return lwgeom_clone_deep(geom);
  if (geom->type != MULTIPOINTTYPE && geom->type != MULTILINETYPE &&
      geom->type != MULTICURVETYPE && geom->type != COLLECTIONTYPE)
    return NULL;

  int maxcomp = 8, ncomp = 0;
  const LWGEOM **comps = palloc(sizeof(LWGEOM *) * maxcomp);
  if (! linear_union_collect(geom, &comps, &ncomp, &maxcomp) || ncomp == 0)
  {
    pfree(comps);
    return NULL;
  }
  if (ncomp == 1)
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
  int maxcur = ncomp;
  const LWGEOM **cur = palloc(sizeof(LWGEOM *) * maxcur);
  LWGEOM **owned = palloc0(sizeof(LWGEOM *) * maxcur);
  int ncur = 0;
  for (int i = 0; i < ncomp; i++)
  {
    LWGEOM **curves = NULL;
    int ncurves = linear_union_dissolve(comps[i], &curves);
    if (ncurves <= 0)
    {
      if (ncur == maxcur)
      {
        maxcur *= 2;
        cur = repalloc(cur, sizeof(LWGEOM *) * (size_t) maxcur);
        owned = repalloc(owned, sizeof(LWGEOM *) * (size_t) maxcur);
      }
      owned[ncur] = NULL;
      cur[ncur++] = comps[i];
      continue;
    }
    for (int c = 0; c < ncurves; c++)
    {
      if (ncur == maxcur)
      {
        maxcur *= 2;
        cur = repalloc(cur, sizeof(LWGEOM *) * (size_t) maxcur);
        owned = repalloc(owned, sizeof(LWGEOM *) * (size_t) maxcur);
      }
      owned[ncur] = curves[c];
      cur[ncur++] = curves[c];
    }
    pfree(curves);
  }
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
          LWGEOM *merged = linear_union_merge(cur[i], cur[j]);
          if (! merged)
          {
            declined = true;
            break;
          }
          if (owned[i])
            lwgeom_free(owned[i]);
          owned[i] = merged;
          cur[i] = merged;
          dropped[j] = true;
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
    pfree(owned); pfree(cur); pfree(boxes); pfree(hasbox); pfree(dropped);
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
    pfree(owned); pfree(cur); pfree(dropped); pfree(comps);
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
  for (int pass = 0; pass < 2; pass++)
    for (int i = 0; i < ncomp; i++)
    {
      if (dropped[i])
        continue;
      bool ispoint = cur[i]->type == POINTTYPE;
      if ((pass == 0) != ispoint)
        continue;
      kept[k++] = lwgeom_clone_deep(cur[i]);
    }
  for (int i = 0; i < ncomp; i++)
    if (owned[i])
      lwgeom_free(owned[i]);
  pfree(owned); pfree(cur); pfree(dropped); pfree(comps);
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
 * @brief Return the ordinates a point array determines at a point
 * @details A vertex of the array determines its own ordinates, and a point
 * between two of them reads them along the curve that joins them
 * @param[in] pa Point array
 * @param[in] circular True where the array reads as arcs rather than segments
 * @param[in] px,py Point
 * @param[out] z,m Ordinates the array determines there
 * @return True if the point lies on what the array draws
 */
static bool
ptarray_ordinates_at(const POINTARRAY *pa, bool circular, double px, double py,
  double *z, double *m)
{
  assert(pa); assert(z); assert(m);
  /* A vertex carries its ordinates rather than reading them from a curve, and
   * an answer keeps the vertices of its inputs, so this is where the walk ends
   * for all but the nodes an overlay adds */
  for (uint32_t i = 0; i < pa->npoints; i++)
  {
    POINT4D p;
    getPoint4d_p(pa, i, &p);
    if (fabs(p.x - px) <= coordinate_tolerance(p.x, px) &&
        fabs(p.y - py) <= coordinate_tolerance(p.y, py))
    {
      *z = p.z; *m = p.m;
      return true;
    }
  }

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
      if (arc_ordinates_at(&p0, &p1, &p2, px, py, z, m))
        return true;
    }
    return false;
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
    *z = p0.z + f * (p1.z - p0.z);
    *m = p0.m + f * (p1.m - p0.m);
    return true;
  }
  return false;
}

/**
 * @brief Read what a point array determines at a point into what the arrays
 * already read determine there
 * @details The first array to determine the ordinates gives them, and one
 * determining others reports the conflict, which is separate for each of the
 * two: two surfaces meeting at one elevation may carry different measures
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
  double z, m;
  if (! ptarray_ordinates_at(pa, circular, px, py, &z, &m))
    return;
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
    nvertices += (int) (*(POINTARRAY **) meos_array_get(arrays, i))->npoints;

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
    const POINTARRAY *pa = *(POINTARRAY **) meos_array_get(arrays, i);
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
    POINTARRAY *pa = *(POINTARRAY **) meos_array_get(outarrays, i);
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
  double ox = p2->x, oy = p2->y;
  double ax = p1->x - ox, ay = p1->y - oy;
  double bx = p3->x - ox, by = p3->y - oy;
  double d = 2 * (ax * (0 - by) + 0 * (by - ay) + bx * (ay - 0));
  if (fabs(d) < MEOS_GEOM_TOLERANCE)
    return false;

  double asq = ax * ax + ay * ay, bsq = bx * bx + by * by;
  double ux = (ay * bsq - by * asq) / d;
  double uy = (bx * asq - ax * bsq) / d;
  *cx = ux + ox;
  *cy = uy + oy;
  *radius = hypot(ux, uy);
  if (*radius < MEOS_GEOM_TOLERANCE)
    return false;

  double t1 = atan2(p1->y - *cy, p1->x - *cx);
  double t2 = atan2(p2->y - *cy, p2->x - *cx);
  double t3 = atan2(p3->y - *cy, p3->x - *cx);
  /* The arc runs from p1 to p3 the way round that passes through p2 */
  bool ccw = angle_normalize(t2 - t1) < angle_normalize(t3 - t1) ||
    angle_normalize(t3 - t1) < MEOS_EPSILON;
  *sweep = ccw ? angle_normalize(t3 - t1) : -angle_normalize(t1 - t3);
  if (fabs(*sweep) < MEOS_EPSILON)
    *sweep = ccw ? 2 * M_PI : -2 * M_PI;
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
 * @param[in,out] acc Accumulator of lengths
 * @param[in] pa Point array
 * @param[in] arcs True when consecutive triples of points draw circular arcs
 */
static void
centroid_add_ptarray_line(CentroidAcc *acc, const POINTARRAY *pa, bool arcs)
{
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
 * @brief Add every point of a point array, each weighing one
 */
static void
centroid_add_ptarray_points(CentroidAcc *acc, const POINTARRAY *pa)
{
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
    double theta = fabs(sweep);
    double denom = theta - sin(theta);
    if (fabs(denom) < MEOS_EPSILON)
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
 */
static void
centroid_add_curve_line(CentroidAcc *lineal, CentroidAcc *punctual,
  const LWGEOM *geom)
{
  if (! geom || lwgeom_is_empty(geom))
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

  CentroidAcc areal = {0}, lineal = {0}, punctual = {0};
  centroid_add_geo(geom, &areal, &lineal, &punctual);

  POINT4D mean = {0, 0, 0, 0};
  /* Every ring is folded in weighing positively, so a surface that holds any
   * area holds a positive total of it */
  if (areal.weight > MEOS_EPSILON)
    centroid_mean(&areal, &mean);
  else if (lineal.weight > MEOS_EPSILON)
    centroid_mean(&lineal, &mean);
  else if (! centroid_mean(&punctual, &mean))
    return lwpoint_as_lwgeom(lwpoint_construct_empty(srid, hasz, hasm));

  return lwpoint_as_lwgeom(lwpoint_make(srid, hasz, hasm, &mean));
}

/*****************************************************************************/
