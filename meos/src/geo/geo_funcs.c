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
/* PostGIS */
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
/* MEOS */
#include "meos.h"
#include "meos_internal_geo.h"
#include "temporal/temporal.h"
#include "geo/geo_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

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

