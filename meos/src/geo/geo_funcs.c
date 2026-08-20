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
 * @brief Add to the dynamic array in the last argument the edges obtained
 * from a polygon
 */
static void
extract_poly(const LWPOLY *poly, MeosArray *edges)
{
  for (int r = 0; r < (int) poly->nrings; r++)
    emit_ring_edges(poly->rings[r], edges, EDGE_POLYSEG);
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
  emit_ring_edges(tri->points, edges, EDGE_POLYSEG);
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
MeosArray *
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
  double minq = FP_MIN(q1->x, q2->x);
  double maxq = FP_MAX(q1->x, q2->x);
  double minp = FP_MIN(p1->x, p2->x);
  double maxp = FP_MAX(p1->x, p2->x);

  if (FP_GT(minp, maxq) || FP_LT(maxp, minq))
    return false;

  minq = FP_MIN(q1->y, q2->y);
  maxq = FP_MAX(q1->y, q2->y);
  minp = FP_MIN(p1->y, p2->y);
  maxp = FP_MAX(p1->y, p2->y);

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
  if (fabs(e->x2 - e->x1) > FP_TOLERANCE || fabs(e->y2 - e->y1) > FP_TOLERANCE)
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
  /* The placement below reads the vertices, which are the whole of a
   * geometry only while every edge of it is a segment */
  assert(! lwgeom_has_arc(geom));

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
    if (hypot(dx, dy) <= FP_TOLERANCE)
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
   * that case too. */
  double best_area = DBL_MAX;
  POINT2D best_rect[5] = {0};
  for (uint32_t i = 0; i < nangles; i++)
  {
    double angle = angles[i];
    double ux = cos(angle);
    double uy = sin(angle);
    POINT2D rect[5];
    double area = mrr_rectangle_for_direction(hull, nhull, ux, uy, rect);
    if (i == 0 || area < best_area)
    {
      best_area = area;
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
geom_oriented_envelope_meos(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  /* A geometry carrying a circular arc has no native placement yet, and the
   * arc reaches past the points that define it, so it is answered by GEOS */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom_has_arc(lwgeom))
  {
    lwgeom_free(lwgeom);
    return geom_oriented_envelope(gs);
  }
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
  /* The placement below reads the vertices, which are the whole of a
   * geometry only while every edge of it is a segment */
  assert(! lwgeom_has_arc(geom));

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
 */
GSERIALIZED *
geom_convex_hull_meos(const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  /* Empty.ConvexHull() == Empty */
  if (gserialized_is_empty(gs))
    return geo_copy(gs);

  /* A geometry carrying a circular arc has no native hull yet, and the arc
   * reaches past the points that define it, so it is answered by GEOS */
  LWGEOM *lwgeom = lwgeom_from_gserialized(gs);
  if (lwgeom_has_arc(lwgeom))
  {
    lwgeom_free(lwgeom);
    return geom_convex_hull(gs);
  }
  LWGEOM *res = convex_hull(lwgeom);
  GSERIALIZED *result = geo_serialize(res);
  lwgeom_free(lwgeom); lwgeom_free(res);
  return result;
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
 * @brief Ensure that the spatiotemporal argument have the same type of
 * coordinates, either planar or geodetic
 */
bool
ensure_same_geodetic(int16 flags1, int16 flags2)
{
  if (MEOS_FLAGS_GET_X(flags1) && MEOS_FLAGS_GET_X(flags2) &&
    MEOS_FLAGS_GET_GEODETIC(flags1) != MEOS_FLAGS_GET_GEODETIC(flags2))
  {
    meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
      "Operation on mixed planar and geodetic coordinates");
    return false;
  }
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
 * @brief Ensure that the two spatial objects have the same SRID
 */
bool
ensure_same_srid(int32_t srid1, int32_t srid2)
{
  if (srid1 == srid2)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed SRID");
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
 * @brief Uses the exact line/arc engine
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
        if (point_on_segment(x, y, e->x1, e->y1, e->x2, e->y2))
          return true;
        break;
      case EDGE_POLYARC:
        if (point_on_arc(x, y, e))
          return true;
        break;
      default:
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
