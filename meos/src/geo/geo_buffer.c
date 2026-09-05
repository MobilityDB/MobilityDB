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
 * @brief Native implementation of PostGIS function @p ST_Buffer() that do not
 * polygonizes arc segments
 * @details This is not yet a complete implementation
 */

/* C */
#include <math.h>
#include <stdlib.h>
/* PostgreSQL */
#include "postgres.h"
#include <utils/float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
/* MEOS */
#include "meos.h"
#include "meos_internal_geo.h"
#include "geo/geo_funcs.h"
#include "geo/geo_poly_clip.h"
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/**
 * @brief Join style for line buffering
 */
typedef enum
{
  JOIN_ROUND = 1,
  JOIN_MITRE = 2,
  JOIN_BEVEL = 3
} JoinStyle;

/**
 * @brief End-cap style for line buffering
 */
typedef enum
{
  ENDCAP_ROUND = 1,
  ENDCAP_FLAT = 2,
  ENDCAP_SQUARE = 3
} EndCapStyle;

/**
 * @brief Type of a buffer boundary piece.
 */
typedef enum
{
  BUFFER_SEGMENT,
  BUFFER_ARC
} BufferPieceType;

/**
 * @brief A piece of a buffer boundary.
 */
typedef struct
{
  BufferPieceType type;
  double x1;
  double y1;
  double x2;
  double y2;
  /* Parameters used only for circular arcs */
  double cx;
  double cy;
  double radius;
  double theta1;
  double theta2;
  bool ccw;
} BufferPiece;

/**
 * @brief Add a point to a local parameterized node array.
 */
typedef struct
{
  POINT2D point;
  double parameter;
} BufferSplitPoint;

/**
 * @brief Classification of a split buffer boundary piece with respect to
 * another buffer.
 */
typedef enum
{
  BUFFER_PIECE_EXTERIOR = 0,
  BUFFER_PIECE_INTERIOR = 1,
  BUFFER_PIECE_BOUNDARY = 2
} BufferPieceLocation;

/**
 * @brief Topological classification of a closed buffer boundary ring.
 * @details
 * - ring Boundary ring
 * - pieces The ordered pieces used to construct the ring. The ring owns the
 *   geometric representation, while this array contains copies of the
 *   BufferPiece descriptors needed by later topology stages.
 * - parent is the index of the immediately containing ring, or -1 when
 *   the ring has no containing ring.
 * - depth is the number of containing rings between the ring and the
 *   exterior. Even depth means shell, odd depth means hole.
 * - shell identifies the shell which directly owns the ring when the
 *   ring is a hole. It is -1 for shells.
 */
typedef struct
{
  LWCOMPOUND *ring;
  MeosArray *pieces;
  double x;
  double y;
  int32_t parent;
  uint32_t depth;
  int32_t shell;
} BufferRingInfo;
/* Temporary forward declaration */
extern LWGEOM *
meos_buffer(const LWGEOM *geom, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit);

/*****************************************************************************
 * Buffer utilities
 *****************************************************************************/

/**
 * @brief Return the 2D cross product of two vectors
 */
static inline double
buffer_cross(double ax, double ay, double bx, double by)
{
  return ax * by - ay * bx;
}

/**
 * @brief Return a point displaced from another point
 */
static inline POINT2D
buffer_point_offset(double x, double y, double nx, double ny, double distance)
{
  POINT2D result;
  result.x = x + nx * distance;
  result.y = y + ny * distance;
  return result;
}

/**
 * @brief Append a 2D point to a point array
 */
static void
buffer_append_point(POINTARRAY *pa, double x, double y)
{
  POINT4D point;
  point.x = x;
  point.y = y;
  point.z = 0.0;
  point.m = 0.0;
  ptarray_append_point(pa, &point, LW_TRUE);
}

/**
 * @brief Compute the intersection of two infinite lines
 * @details The first line is P + tR and the second line is Q + uS.
 */
static bool
buffer_line_intersection(POINT2D p, double rx, double ry, POINT2D q,
  double sx, double sy, POINT2D *result)
{
  assert(result);
  double denominator = buffer_cross(rx, ry, sx, sy);
  if (fabs(denominator) <= MEOS_GEOM_TOLERANCE)
    return false;
  double qpx = q.x - p.x;
  double qpy = q.y - p.y;
  double t = buffer_cross(qpx, qpy, sx, sy) / denominator;
  result->x = p.x + t * rx;
  result->y = p.y + t * ry;
  return true;
}

/**
 * @brief Construct a 2D LINESTRING containing two points
 */
static LWLINE *
buffer_make_segment(int32_t srid, POINT2D p1, POINT2D p2)
{
  POINTARRAY *points = ptarray_construct_empty(LW_FALSE, LW_FALSE, 2);
  buffer_append_point(points, p1.x, p1.y);
  buffer_append_point(points, p2.x, p2.y);
  return lwline_construct(srid, NULL, points);
}

/**
 * @brief Construct a 3-point circular arc
 */
static LWCIRCSTRING *
buffer_make_arc(int32_t srid, double cx, double cy, double radius,
  double start_angle, double end_angle, bool ccw, const POINT2D *start,
  const POINT2D *end)
{
  double sweep, middle_angle;
  POINTARRAY *points;
  if (ccw)
    sweep = angle_normalize(end_angle - start_angle);
  else
    sweep = angle_normalize(start_angle - end_angle);
  if (sweep <= MEOS_GEOM_TOLERANCE)
    return NULL;

  /* A CIRCSTRING arc is represented by three points.
   * Split arcs larger than PI into several pieces in the caller.
   * An endpoint the caller knows exactly is taken from it: recomputing it
   * from its angle moves it by a rounding step, which leaves the ring the
   * arcs belong to unclosed. */
  middle_angle = ccw ? start_angle + sweep * 0.5 : start_angle - sweep * 0.5;
  points = ptarray_construct_empty(LW_FALSE, LW_FALSE, 3);
  if (start)
    buffer_append_point(points, start->x, start->y);
  else
    buffer_append_point(points, cx + radius * cos(start_angle),
      cy + radius * sin(start_angle));
  buffer_append_point(points, cx + radius * cos(middle_angle),
    cy + radius * sin(middle_angle));
  if (end)
    buffer_append_point(points, end->x, end->y);
  else
    buffer_append_point(points, cx + radius * cos(end_angle),
      cy + radius * sin(end_angle));
  return lwcircstring_construct(srid, NULL, points);
}

/**
 * @brief Add a straight segment to a compound curve
 */
/* Defined with the ring walk that shares its question of node identity */
static bool buffer_points_equal(POINT2D p1, POINT2D p2);

/**
 * @brief Write a piece's start as the point the curve already ends at
 * @details The walk decides that two pieces meet by #buffer_points_equal, at
 * the tolerance the buffer places a node to, and #lwcompound_add_lwgeom
 * decides it AGAIN by liblwgeom's own `FP_EQUALS` -- an ABSOLUTE 1e-12 on a
 * coordinate, a thousand times finer than the last bit of a projected 6.4e6.
 * Two computations of one node therefore join for the walk and not for the
 * compound, which REFUSES the component; the refusal is a return value the
 * callers drop, so the piece leaves the ring without a word and the boundary
 * closes across the gap it left. Writing the shared node ONCE -- the
 * coordinates the curve already carries -- makes the two agree by
 * construction, which is what one node means
 */
static void
buffer_snap_to_curve_end(const LWCOMPOUND *curve, POINT2D *point)
{
  assert(curve); assert(point);
  if (curve->ngeoms == 0)
    return;
  /* A compound carries lines and circular strings, which hold their points
   * the same way, and this reads only the last of them */
  const LWLINE *prev = (const LWLINE *) curve->geoms[curve->ngeoms - 1];
  if (! prev || ! prev->points || prev->points->npoints == 0)
    return;
  POINT4D last;
  getPoint4d_p(prev->points, prev->points->npoints - 1, &last);
  POINT2D end;
  end.x = last.x;
  end.y = last.y;
  if (buffer_points_equal(end, *point))
    *point = end;
  return;
}

static void
buffer_add_segment(LWCOMPOUND *curve, int32_t srid, POINT2D p1, POINT2D p2)
{
  assert(curve);
  buffer_snap_to_curve_end(curve, &p1);
  if (hypot(p2.x - p1.x, p2.y - p1.y) <= MEOS_GEOM_TOLERANCE)
    return;
  LWLINE *line = buffer_make_segment(srid, p1, p2);
  if (lwcompound_add_lwgeom(curve, lwline_as_lwgeom(line)) != LW_SUCCESS)
    lwline_free(line);
}

/**
 * @brief Add circular arcs to a compound curve
 */
static void
buffer_add_arc(LWCOMPOUND *curve, int32_t srid, double cx, double cy,
  double radius, double start_angle, double end_angle, bool ccw,
  const POINT2D *start, const POINT2D *end)
{
  assert(curve);
  double sweep = ccw ? 
    angle_normalize(end_angle - start_angle) :
    angle_normalize(start_angle - end_angle);
  if (sweep <= MEOS_GEOM_TOLERANCE)
    return;

  /* PostGIS circular strings use three points per arc and an individual
   * arc must not exceed 180 degrees */
  int count = (int) ceil(sweep / M_PI);
  if (count < 1)
    count = 1;
  double delta = sweep / (double) count;
  POINT2D first;
  const POINT2D *snapped = start;
  if (start)
  {
    first = *start;
    buffer_snap_to_curve_end(curve, &first);
    snapped = &first;
  }
  for (int i = 0; i < count; i++)
  {
    double a0 = ccw ? start_angle + delta * i : start_angle - delta * i;
    double a1 = ccw ? start_angle + delta * (i + 1) : 
      start_angle - delta * (i + 1);
    LWCIRCSTRING *arc = buffer_make_arc(srid, cx, cy, radius, a0, a1, ccw,
      i == 0 ? snapped : NULL, i == count - 1 ? end : NULL);
    if (arc &&
        lwcompound_add_lwgeom(curve, lwcircstring_as_lwgeom(arc)) !=
          LW_SUCCESS)
      lwcircstring_free(arc);
  }
}

/**
 * @brief Add a round join
 */
static void
buffer_add_round_join(LWCOMPOUND *curve, int32_t srid, POINT2D vertex,
  POINT2D p1, POINT2D p2, double radius, bool ccw)
{
  assert(curve);
  double start_angle = atan2(p1.y - vertex.y, p1.x - vertex.x);
  double end_angle = atan2(p2.y - vertex.y, p2.x - vertex.x);
  buffer_add_arc(curve, srid, vertex.x, vertex.y, radius, start_angle,
    end_angle, ccw, &p1, &p2);
}

/**
 * @brief Add a bevel join
 */
static void
buffer_add_bevel_join(LWCOMPOUND *curve, int32_t srid, POINT2D p1, POINT2D p2)
{
  assert(curve);
  buffer_add_segment(curve, srid, p1, p2);
}

/**
 * @brief Add a mitre join
 */
static bool
buffer_add_mitre_join(LWCOMPOUND *curve, int32_t srid, POINT2D vertex,
  POINT2D p1, POINT2D p2, double radius, double mitre_limit)
{
  assert(curve);
  double r1x = p1.x - vertex.x;
  double r1y = p1.y - vertex.y;
  double r2x = p2.x - vertex.x;
  double r2y = p2.y - vertex.y;
  double l1 = hypot(r1x, r1y);
  double l2 = hypot(r2x, r2y);
  if (l1 <= MEOS_GEOM_TOLERANCE || l2 <= MEOS_GEOM_TOLERANCE)
    return false;
  r1x /= l1;
  r1y /= l1;
  r2x /= l2;
  r2y /= l2;
  POINT2D intersection;
  if (! buffer_line_intersection(p1, r1x, r1y, p2, r2x, r2y, &intersection))
    return false;
  double length = hypot(intersection.x - vertex.x, intersection.y - vertex.y);
  if (length > radius * mitre_limit + MEOS_GEOM_TOLERANCE)
    return false;
  buffer_add_segment(curve, srid, p1, intersection);
  buffer_add_segment(curve, srid, intersection, p2);
  return true;
}

/**
 * @brief Add a join between two offset segments
 */
static void
buffer_add_join(LWCOMPOUND *curve, int32_t srid, POINT2D vertex, POINT2D p1,
  POINT2D p2, double radius, JoinStyle join_style, double mitre_limit,
  bool outer)
{
  assert(curve);
  /* The inner side of a turn is always joined by the intersection
   * of the two offset lines */
  if (! outer)
  {
    double r1x = p1.x - vertex.x;
    double r1y = p1.y - vertex.y;
    double r2x = p2.x - vertex.x;
    double r2y = p2.y - vertex.y;
    POINT2D intersection;
    if (buffer_line_intersection(p1, r1x, r1y, p2, r2x, r2y, &intersection))
    {
      buffer_add_segment(curve, srid, p1, intersection);
      buffer_add_segment(curve, srid, intersection, p2);
    }
    else
      buffer_add_segment(curve, srid, p1, p2);
    return;
  }

  switch (join_style)
  {
    case JOIN_ROUND:
    {
      double cross = buffer_cross(p1.x - vertex.x, p1.y - vertex.y,
        p2.x - vertex.x, p2.y - vertex.y);
      buffer_add_round_join(curve, srid, vertex, p1, p2, radius, cross > 0.0);
      break;
    }
    case JOIN_MITRE:
      if (! buffer_add_mitre_join(curve, srid, vertex, p1, p2, radius,
          mitre_limit))
        buffer_add_bevel_join(curve, srid, p1, p2);
      break;
    case JOIN_BEVEL:
      buffer_add_bevel_join(curve, srid, p1, p2);
      break;
  }
}

/**
 * @brief Add a round cap
 */
static void
buffer_add_round_cap(LWCOMPOUND *curve, int32_t srid, POINT2D center,
  POINT2D p1, POINT2D p2, double radius, bool ccw)
{
  assert(curve);
  double start_angle = atan2(p1.y - center.y, p1.x - center.x);
  double end_angle = atan2( p2.y - center.y, p2.x - center.x);
  buffer_add_arc(curve, srid, center.x, center.y, radius, start_angle,
    end_angle, ccw, &p1, &p2);
}

/**
 * @brief Return the points of a piece of a compound curve
 */
static POINTARRAY *
buffer_curve_points(const LWGEOM *geom)
{
  assert(geom);
  switch (geom->type)
  {
    case LINETYPE:
      return ((const LWLINE *) geom)->points;
    case CIRCSTRINGTYPE:
      return ((const LWCIRCSTRING *) geom)->points;
    default:
      return NULL;
  }
}

/**
 * @brief Weld every piece of a ring to the one before it, and the ring to
 * itself
 * @details Every piece of a ring is placed from the geometry it offsets, the
 * arcs from the trigonometry of their centre and radius, so the point one
 * piece ends on is the point the next one starts from only up to the rounding
 * of the arithmetic that places it. Two pieces meeting a rounding step apart
 * are not one boundary, and a ring whose ends differ by such a step is not a
 * ring: the reader of a linear ring compares its endpoints exactly and
 * refuses one that misses its start at all, and a joint that misses by the
 * same residue reads as the boundary crossing itself there.
 *
 * The point a piece starts from is therefore taken from the piece before it
 * rather than computed a second time, and the point the ring closes on from
 * the piece it starts with.
 * @param[in,out] ring Ring to weld
 */
static void
buffer_ring_weld(LWCOMPOUND *ring)
{
  assert(ring);
  if (ring->ngeoms == 0)
    return;
  POINTARRAY *first = buffer_curve_points(ring->geoms[0]);
  if (! first || first->npoints == 0)
    return;
  /* Every piece starts where the one before it ends */
  POINTARRAY *prev = first;
  for (uint32_t i = 1; i < ring->ngeoms; i++)
  {
    POINTARRAY *points = buffer_curve_points(ring->geoms[i]);
    if (! points || points->npoints == 0)
      return;
    POINT4D end;
    getPoint4d_p(prev, prev->npoints - 1, &end);
    ptarray_set_point4d(points, 0, &end);
    prev = points;
  }
  /* And the ring ends where it starts */
  POINT4D start;
  getPoint4d_p(first, 0, &start);
  ptarray_set_point4d(prev, prev->npoints - 1, &start);
}

/**
 * @brief Add a ring to a curve polygon, closed on the point it starts from
 * @param[in,out] poly Curve polygon the ring is added to
 * @param[in,out] ring Ring to add
 */
static void
buffer_curvepoly_add_ring(LWCURVEPOLY *poly, LWCOMPOUND *ring)
{
  assert(poly); assert(ring);
  buffer_ring_weld(ring);
  lwcurvepoly_add_ring(poly, lwcompound_as_lwgeom(ring));
}

/*****************************************************************************
 * Buffer intersection
 *****************************************************************************/

/**
 * @brief Return true if two buffer geometries have overlapping interiors.
 * @details This is used to determine whether individual line buffers can
 * be returned independently or whether an overlay/union operation is needed.
 *
 * This first implementation performs a boundary intersection test.
 */
static bool
buffer_geometries_intersect(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  MeosArray *a1 = geom_extract_edges(geom1);
  MeosArray *a2 = geom_extract_edges(geom2);
  int n1 = (int) a1->count;
  int n2 = (int) a2->count;
  Edge **e1 = palloc(sizeof(Edge *) * n1);
  Edge **e2 = palloc(sizeof(Edge *) * n2);
  for (int i = 0; i < n1; i++)
    e1[i] = (Edge *) meos_array_get(a1, i);
  for (int i = 0; i < n2; i++)
    e2[i] = (Edge *) meos_array_get(a2, i);
  bool result = false;
  for (int i = 0; i < n1 && ! result; i++)
  {
    const Edge *a = e1[i];
    if (a->etype != EDGE_POLYSEG && a->etype != EDGE_POLYARC &&
        a->etype != EDGE_LINEARC && a->etype != EDGE_LINESEG)
      continue;
    for (int j = 0; j < n2; j++)
    {
      const Edge *b = e2[j];
      if (b->etype != EDGE_POLYSEG && b->etype != EDGE_POLYARC &&
          b->etype != EDGE_LINEARC && b->etype != EDGE_LINESEG)
        continue;

      /* Line / line */
      if (a->etype == EDGE_LINESEG && b->etype == EDGE_LINESEG)
      {
        IntersectResult r = linesegm_intersect(a->x1, a->y1, a->dx, a->dy,
          b->x1, b->y1, b->x2, b->y2);
        if (r.type != INTERSECT_NONE)
        {
          result = true;
          break;
        }
      }

      /* Line / arc */
      else if (a->etype == EDGE_LINESEG && b->etype == EDGE_LINEARC)
      {
        double roots[2];
        int n = arcsegm_intersect(a->x1, a->y1, a->dx, a->dy, b, roots);
        if (n > 0)
        {
          result = true;
          break;
        }
      }

      /* Arc / line */
      else if (a->etype == EDGE_LINEARC && b->etype == EDGE_LINESEG)
      {
        double roots[2];
        int n = arcsegm_intersect(b->x1, b->y1, b->dx, b->dy, a, roots);
        if (n > 0)
        {
          result = true;
          break;
        }
      }

      /* Arc / arc */
      else if (a->etype == EDGE_LINEARC && b->etype == EDGE_LINEARC)
      {
        if (arcarc_intersect(a, b))
        {
          result = true;
          break;
        }
      }

      /* Polygon boundary / polygon boundary.
       * The existing edge engine represents polygon straight boundaries as
       * EDGE_POLYSEG and curved boundaries as EDGE_POLYARC. */
      else if (a->etype == EDGE_POLYSEG && b->etype == EDGE_POLYSEG)
      {
        IntersectResult r = linesegm_intersect(a->x1, a->y1, a->dx, a->dy,
          b->x1, b->y1, b->x2, b->y2);
        if (r.type != INTERSECT_NONE)
        {
          result = true;
          break;
        }
      }
      else if (a->etype == EDGE_POLYSEG && b->etype == EDGE_POLYARC)
      {
        double roots[2];
        int n = arcsegm_intersect(a->x1, a->y1, a->dx, a->dy, b, roots);
        if (n > 0)
        {
          result = true;
          break;
        }
      }
      else if (a->etype == EDGE_POLYARC && b->etype == EDGE_POLYSEG)
      {
        double roots[2];
        int n = arcsegm_intersect(b->x1, b->y1, b->dx, b->dy, a, roots);
        if (n > 0)
        {
          result = true;
          break;
        }
      }
      else if (a->etype == EDGE_POLYARC && b->etype == EDGE_POLYARC)
      {
        if (arcarc_intersect(a, b))
        {
          result = true;
          break;
        }
      }
    }
  }
  /* Clean up and return */
  pfree(e1); pfree(e2); meos_array_destroy(a1); meos_array_destroy(a2);
  return result;
}

/*****************************************************************************
 * Buffer boundary intersection
 *****************************************************************************/

/**
 * @brief Return true if two buffer boundary edges intersect.
 * @details Uses the line and circular-arc intersection engines.
 */
static bool
buffer_edges_intersect(const Edge *e1, const Edge *e2)
{
  if (! e1 || ! e2)
    return false;

  /* Line / Line */
  if (e1->etype == EDGE_POLYSEG && e2->etype == EDGE_POLYSEG)
  {
    IntersectResult r = linesegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy,
      e2->x1, e2->y1, e2->x2, e2->y2);
    return r.type != INTERSECT_NONE;
  }

  /* Line / Arc */
  if (e1->etype == EDGE_POLYSEG && e2->etype == EDGE_POLYARC)
  {
    double roots[2];
    int n = arcsegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy, e2, roots);
    return n > 0;
  }

  /* Arc / Line */
  if (e1->etype == EDGE_POLYARC && e2->etype == EDGE_POLYSEG)
  {
    double roots[2];
    int n = arcsegm_intersect(e2->x1, e2->y1, e2->dx, e2->dy, e1, roots);
    return n > 0;
  }

  /* Arc / Arc */
  if (e1->etype == EDGE_POLYARC && e2->etype == EDGE_POLYARC)
    return arcarc_intersect(e1, e2);

  return false;
}

/**
 * @brief Return true if two sets of buffer edges intersect.
 * @details The edges are extracted from the two geometries and tested pairwise
 * using the line/arc intersection functions.
 */
static bool
buffer_boundaries_intersect(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  MeosArray *a1 = geom_extract_edges(geom1);
  MeosArray *a2 = geom_extract_edges(geom2);
  uint32_t n1 = a1->count;
  uint32_t n2 = a2->count;
  for (uint32_t i = 0; i < n1; i++)
  {
    const Edge *e1 = (const Edge *) meos_array_get(a1, i);
    if (! e1)
      continue;
    for (uint32_t j = 0; j < n2; j++)
    {
      const Edge *e2 = (const Edge *) meos_array_get(a2, j);
      if (! e2)
        continue;
      if (buffer_edges_intersect(e1, e2))
      {
        meos_array_destroy(a1); meos_array_destroy(a2);
        return true;
      }
    }
  }
  meos_array_destroy(a1); meos_array_destroy(a2);
  return false;
}

/*****************************************************************************
 * Buffer areal union
 *****************************************************************************/

/**
 * @brief Return a representative point of an areal geometry.
 * @details The returned point is taken from the first boundary edge.
 * It is only used for containment tests and is therefore subsequently
 * verified against the complete geometry.
 */
static bool
buffer_areal_representative_point(const LWGEOM *geom, double *x, double *y)
{
  assert(geom); assert(x); assert(y);
  MeosArray *arr = geom_extract_edges(geom);
  if (arr->count == 0)
  {
    meos_array_destroy(arr);
    return false;
  }
  const Edge *edge = (const Edge *) meos_array_get(arr, 0);
  if (edge->etype == EDGE_POLYSEG || edge->etype == EDGE_LINESEG)
  {
    *x = (edge->x1 + edge->x2) * 0.5;
    *y = (edge->y1 + edge->y2) * 0.5;
  }
  else if (edge->etype == EDGE_POLYARC || edge->etype == EDGE_LINEARC)
  {
    double sweep = edge->ccw ? angle_normalize(edge->theta1 - edge->theta0) :
      angle_normalize(edge->theta0 - edge->theta1);
    double theta = edge->ccw ? edge->theta0 + sweep * 0.5 :
      edge->theta0 - sweep * 0.5;
    *x = edge->cx + edge->radius * cos(theta);
    *y = edge->cy + edge->radius * sin(theta);
  }
  else
  {
    meos_array_destroy(arr);
    return false;
  }
  meos_array_destroy(arr);
  return true;
}

/**
 * @brief Return true if an areal geometry contains a point in its interior.
 * @details This function deliberately treats boundary points as not being
 * interior. It is therefore suitable for determining strict containment.
 */
static bool
buffer_areal_contains_point(const LWGEOM *geom, double x, double y)
{
  assert(geom);
  MeosArray *arr = geom_extract_edges(geom);
  int nedges = (int) arr->count;
  if (nedges == 0)
  {
    meos_array_destroy(arr);
    return false;
  }
  Edge **edges = palloc(sizeof(Edge *) * nedges);
  for (int i = 0; i < nedges; i++)
    edges[i] = (Edge *) meos_array_get(arr, i);
  if (relate_point_on_boundary(x, y, edges, nedges))
  {
    pfree(edges);
    meos_array_destroy(arr);
    return false;
  }
  bool result = point_in_polygon(x, y, edges, nedges);
  pfree(edges); meos_array_destroy(arr);
  return result;
}

/**
 * @brief Return true if an areal geometry completely contains another
 * areal geometry.
 * @details This is an exact containment test for the supported geometry
 * representation. The boundary intersection test is performed first so
 * that touching geometries are not classified as containment.
 */
static bool
buffer_areal_contains(const LWGEOM *outer, const LWGEOM *inner)
{
  assert(outer); assert(inner);
  double x, y;
  if (! buffer_areal_representative_point(inner, &x, &y))
    return false;
  /* If the representative point is not in the interior, the inner
   * geometry cannot be strictly contained */
  if (! buffer_areal_contains_point(outer, x, y))
    return false;
  /* Verify that no boundary of the inner geometry intersects the
   * boundary of the outer geometry */
  if (buffer_geometries_intersect(outer, inner))
    return false;
  return true;
}

/**
 * @brief Construct a MULTISURFACE from two disjoint areal geometries.
 */
static LWGEOM *
buffer_areal_collection(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  int32_t srid = lwgeom_get_srid(geom1);
  LWGEOM **geoms = palloc(sizeof(LWGEOM *) * 2);
  /* The caller releases both operands once it holds the answer, and
   * lwgeom_clone shares their point arrays rather than copying them */
  geoms[0] = lwgeom_clone_deep(geom1);
  geoms[1] = lwgeom_clone_deep(geom2);
  LWCOLLECTION *result = lwcollection_construct(MULTISURFACETYPE, srid,
    NULL, 2, geoms);
  return lwcollection_as_lwgeom(result);
}

/**
 * @brief Union two crossing buffer geometries while preserving circular arcs.
 * @details The implementation is defined later in this file.
 */
static LWGEOM *
buffer_union_crossing(const LWGEOM *geom1, const LWGEOM *geom2,
  bool *touching);

/**
 * @brief Return whether a geometry covers a hole of another one
 * @details A hole is not part of the surface carrying it, so a geometry that
 * covers one adds area to it. Where the two boundaries do not cross, one point
 * of the hole's own ring says where the whole ring lies, and a hole the inner
 * geometry reaches at all is a hole it contains entirely.
 */
static bool
buffer_covers_hole(const LWGEOM *hole, const LWGEOM *inner)
{
  assert(hole); assert(inner);
  double hx, hy;
  if (! buffer_areal_representative_point(hole, &hx, &hy))
    return false;
  return buffer_areal_contains_point(inner, hx, hy);
}

/**
 * @brief Return a surface with every hole another geometry fills removed
 * @details The union of a surface with one that lies inside it is the surface
 * itself, except where the inner one covers a hole: the hole belongs to
 * neither until then, and covering it joins it to the surface around it. The
 * inner geometry's boundary does not cross the outer one's, so it contains
 * every hole it reaches, and dropping that hole's ring is the whole of the
 * union.
 * @return A deep copy of @p outer carrying only the holes @p inner leaves
 * empty, and NULL where @p outer is not a surface this understands
 */
static LWGEOM *
buffer_outer_without_filled_holes(const LWGEOM *outer, const LWGEOM *inner,
  int32_t srid)
{
  assert(outer); assert(inner);
  if (outer->type == MULTISURFACETYPE || outer->type == COLLECTIONTYPE)
  {
    const LWCOLLECTION *col = (const LWCOLLECTION *) outer;
    LWCOLLECTION *result = lwcollection_construct_empty(outer->type, srid,
      0, 0);
    for (uint32_t i = 0; i < col->ngeoms; i++)
    {
      LWGEOM *part = col->geoms[i] ?
        buffer_outer_without_filled_holes(col->geoms[i], inner, srid) : NULL;
      if (! part)
      {
        lwgeom_free(lwcollection_as_lwgeom(result));
        return NULL;
      }
      lwcollection_add_lwgeom(result, part);
    }
    return lwcollection_as_lwgeom(result);
  }
  if (outer->type == CURVEPOLYTYPE)
  {
    const LWCURVEPOLY *poly = (const LWCURVEPOLY *) outer;
    if (poly->nrings == 0)
      return lwgeom_clone_deep(outer);
    LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
    lwcurvepoly_add_ring(result, lwgeom_clone_deep(poly->rings[0]));
    for (uint32_t i = 1; i < poly->nrings; i++)
    {
      if (! poly->rings[i] || buffer_covers_hole(poly->rings[i], inner))
        continue;
      lwcurvepoly_add_ring(result, lwgeom_clone_deep(poly->rings[i]));
    }
    return lwcurvepoly_as_lwgeom(result);
  }
  return NULL;
}

/**
 * @brief Return the union of two areal geometries for the
 * disjoint/containment cases.
 * @details The following cases are handled exactly:
 *   A disjoint B: MULTISURFACE(A, B)
 *   A contains B: A
 *   B contains A: B
 * Boundary-touching and boundary-crossing cases are deliberately not
 * handled by this slice because they require boundary noding and face
 * extraction.
 */
static LWGEOM *
buffer_areal_union_simple(const LWGEOM *geom1, const LWGEOM *geom2,
  bool *touching)
{
  assert(geom1); assert(geom2); assert(touching);
  *touching = false;

  /* One inside the other: the answer is the outer surface, and the holes of
   * it that the inner one covers are joined to the surface around them */
  int32_t srid = lwgeom_get_srid(geom1);
  if (buffer_areal_contains(geom1, geom2))
  {
    LWGEOM *filled = buffer_outer_without_filled_holes(geom1, geom2, srid);
    return filled ? filled : lwgeom_clone_deep(geom1);
  }
  if (buffer_areal_contains(geom2, geom1))
  {
    LWGEOM *filled = buffer_outer_without_filled_holes(geom2, geom1, srid);
    return filled ? filled : lwgeom_clone_deep(geom2);
  }

  /* If the boundaries do not intersect, the geometries are disjoint */
  if (! buffer_geometries_intersect(geom1, geom2))
    return buffer_areal_collection(geom1, geom2);

  /* The boundaries intersect. Try the curved-boundary union.
   * At this stage #buffer_union_crossing() handles the case where
   * the boundaries cross at discrete intersection points and the
   * resulting exterior boundary consists of one connected component.
   * More difficult cases, such as coincident boundaries, touching
   * boundaries, and multiple resulting rings, return @p NULL
   * and will be handled by subsequent overlay slices. */
  return buffer_union_crossing(geom1, geom2, touching);
}

/*****************************************************************************
 * Buffer overlay - boundary intersection detection
 *****************************************************************************/

/**
 * @brief Return true if an edge belongs to a buffer boundary.
 * @details Buffer boundaries can contain both straight segments and exact
 * circular arcs. The existing buffer implementation represents round joins
 * and caps using CircularStrings, which are exposed by geom_extract_edges()
 * as EDGE_POLYSEG and EDGE_POLYARC edges.
 */
static bool
buffer_is_boundary_edge(const Edge *edge)
{
  assert(edge);
  return edge->etype == EDGE_POLYSEG || edge->etype == EDGE_POLYARC;
}

/**
 * @brief Return the dimension of an intersection between two buffer edges.
 * @return Return 0 if the intersection is a point, 1 if the intersection is a
 * curve, -1 if there is no intersection.
 */
static int
buffer_boundary_intersection(const Edge *e1, const Edge *e2)
{
  assert(e1); assert(e2);

  /* Straight segment / straight segment */
  if (e1->etype == EDGE_POLYSEG && e2->etype == EDGE_POLYSEG)
  {
    IntersectResult result = linesegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy,
      e2->x1, e2->y1, e2->x2, e2->y2);
    if (result.type == INTERSECT_OVERLAP)
      return 1;
    if (result.type == INTERSECT_POINT)
      return 0;
    return -1;
  }

  /* Straight segment / circular arc */
  if (e1->etype == EDGE_POLYSEG && e2->etype == EDGE_POLYARC)
  {
    double roots[2];
    int n = arcsegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy, e2, roots);
    return n > 0 ? 0 : -1;
  }

  /* Circular arc / straight segment */
  if (e1->etype == EDGE_POLYARC && e2->etype == EDGE_POLYSEG)
  {
    double roots[2];
    int n = arcsegm_intersect(e2->x1, e2->y1, e2->dx, e2->dy, e1, roots);
    return n > 0 ? 0 : -1;
  }

  /* Circular arc / circular arc */
  if (e1->etype == EDGE_POLYARC && e2->etype == EDGE_POLYARC)
  {
    return arcarc_intersect(e1, e2) ? 0 : -1;
  }

  return -1;
}


/**
 * @brief Return true if two coordinate pairs are the same topological node
 */
static inline bool
buffer_nodes_equal(double x1, double y1, double x2, double y2)
{
  return fabs(x1 - x2) <= MEOS_GEOM_TOLERANCE && fabs(y1 - y2) <= MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return true if the boundary of a geometry crosses itself
 * @details Two edges of one ring that are not consecutive must not meet: an
 * offset ring that crosses itself does not bound a surface, and resolving it
 * into the surfaces it does bound needs the boundary overlay. Consecutive
 * edges meet at the node they share by construction and are skipped.
 */
static bool
buffer_boundary_self_intersects(const LWGEOM *geom)
{
  assert(geom);
  MeosArray *edges = geom_extract_edges(geom);
  if (! edges)
    return false;
  bool result = false;
  for (uint32_t i = 0; i < edges->count && ! result; i++)
  {
    const Edge *e1 = (const Edge *) meos_array_get(edges, i);
    if (! e1 || ! buffer_is_boundary_edge(e1))
      continue;
    for (uint32_t j = i + 1; j < edges->count; j++)
    {
      const Edge *e2 = (const Edge *) meos_array_get(edges, j);
      if (! e2 || ! buffer_is_boundary_edge(e2))
        continue;
      /* Only two consecutive edges meet by construction, which is where the
       * end of one is the start of the other. Two that share a start or an
       * end point instead meet where the boundary touches itself. */
      if (buffer_nodes_equal(e1->x2, e1->y2, e2->x1, e2->y1) ||
          buffer_nodes_equal(e2->x2, e2->y2, e1->x1, e1->y1))
        continue;
      if (buffer_boundary_intersection(e1, e2) >= 0)
      {
        result = true;
        break;
      }
    }
  }
  meos_array_destroy(edges);
  return result;
}

/*****************************************************************************
 * Buffer overlay - buffer piece geometry
 *****************************************************************************/

/**
 * @brief Return the parametric position of a point on a straight piece.
 * @details The returned parameter is approximately in [0,1].
 */
static double
buffer_segment_parameter(const BufferPiece *piece, double x, double y)
{
  assert(piece); assert(piece->type == BUFFER_SEGMENT);
  double dx = piece->x2 - piece->x1;
  double dy = piece->y2 - piece->y1;
  if (fabs(dx) >= fabs(dy))
  {
    if (fabs(dx) <= MEOS_GEOM_TOLERANCE)
      return 0.0;
    return (x - piece->x1) / dx;
  }
  if (fabs(dy) <= MEOS_GEOM_TOLERANCE)
    return 0.0;
  return (y - piece->y1) / dy;
}

/*****************************************************************************
 * Buffer overlay - containment detection
 *****************************************************************************/

/**
 * @brief Return a representative point on a buffer boundary.
 * @details The returned point is guaranteed to lie on the first usable
 * boundary edge. For a circular arc, the midpoint of the arc is used.
 * For a straight segment, its midpoint is used.
 * @note The point is intentionally obtained from the edge
 * representation.
 */
static bool
buffer_boundary_representative_point(const LWGEOM *geom, double *x, double *y)
{
  assert(geom); assert(x); assert(y);
  MeosArray *edges = geom_extract_edges(geom);
  for (uint32_t i = 0; i < edges->count; i++)
  {
    const Edge *edge = (const Edge *) meos_array_get(edges, i);
    if (! buffer_is_boundary_edge(edge))
      continue;

    if (edge->etype == EDGE_POLYSEG)
    {
      *x = (edge->x1 + edge->x2) * 0.5;
      *y = (edge->y1 + edge->y2) * 0.5;
      meos_array_destroy(edges);
      return true;
    }

    if (edge->etype == EDGE_POLYARC)
    {
      double sweep;
      if (edge->ccw)
        sweep = angle_normalize(edge->theta1 - edge->theta0);
      else
        sweep = angle_normalize(edge->theta0 - edge->theta1);
      if (sweep <= MEOS_GEOM_TOLERANCE)
        continue;
      double theta;
      if (edge->ccw)
        theta = edge->theta0 + sweep * 0.5;
      else
        theta = edge->theta0 - sweep * 0.5;
      *x = edge->cx + edge->radius * cos(theta);
      *y = edge->cy + edge->radius * sin(theta);
      meos_array_destroy(edges);
      return true;
    }
  }

  meos_array_destroy(edges);
  return false;
}

/**
 * @brief Return a small displacement suitable for probing around a boundary.
 */
static double
buffer_containment_epsilon(const LWGEOM *geom)
{
  assert(geom);
  const GBOX *box = lwgeom_get_bbox(geom);
  if (box)
  {
    double dx = box->xmax - box->xmin;
    double dy = box->ymax - box->ymin;
    double scale = fmax(dx, dy);
    if (scale > MEOS_GEOM_TOLERANCE)
      return fmax(scale * MEOS_EPSILON, MEOS_GEOM_TOLERANCE * 10.0);
  }
  return MEOS_GEOM_TOLERANCE * 10.0;
}

/**
 * @brief Return the normal direction of a buffer boundary edge.
 * @details The function returns both possible normals because the
 * orientation of an arbitrary buffer boundary cannot be assumed here.
 */
static bool
buffer_edge_normals(const Edge *edge, double *nx, double *ny)
{
  assert(edge); assert(nx); assert(ny);

  if (edge->etype == EDGE_POLYSEG)
  {
    double dx = edge->x2 - edge->x1;
    double dy = edge->y2 - edge->y1;
    double length = hypot(dx, dy);
    if (length <= MEOS_GEOM_TOLERANCE)
      return false;
    *nx = -dy / length;
    *ny = dx / length;
    return true;
  }

  if (edge->etype == EDGE_POLYARC)
  {
    /* Use the radial direction at the middle of the arc */
    double sweep;
    if (edge->ccw)
      sweep = angle_normalize(edge->theta1 - edge->theta0);
    else
      sweep = angle_normalize(edge->theta0 - edge->theta1);
    if (sweep <= MEOS_GEOM_TOLERANCE)
      return false;
    double theta;
    if (edge->ccw)
      theta = edge->theta0 + sweep * 0.5;
    else
      theta = edge->theta0 - sweep * 0.5;
    double rx = cos(theta);
    double ry = sin(theta);
    /* Radial direction is normal to the circular arc */
    *nx = rx;
    *ny = ry;
    return true;
  }
  return false;
}

/**
 * @brief Return the location of a point with respect to a buffer.
 * @return
 *   0 = interior
 *   1 = boundary
 *   2 = exterior
 */
static int
buffer_point_location(const LWGEOM *geom, double x, double y)
{
  assert(geom);
  MeosArray *arr = geom_extract_edges(geom);
  int nedges = (int) arr->count;
  if (nedges == 0)
  {
    meos_array_destroy(arr);
    return 2;
  }
  Edge **edges = palloc(sizeof(Edge *) * nedges);
  for (int i = 0; i < nedges; i++)
    edges[i] = (Edge *) meos_array_get(arr, i);
  int result = relate_point_in_area(x, y, edges, nedges);
  /* Clean up and return */
  pfree(edges); meos_array_destroy(arr);
  return result;
}

/**
 * @brief Determine whether one buffer is completely contained in another.
 * @details This function assumes that the two buffer boundaries do not
 * intersect.
 * Since the boundary of a connected buffer is a closed curve, if its
 * boundary does not intersect the boundary of the other buffer, testing
 * one point immediately inside the boundary is sufficient to determine
 * whether the complete buffer lies inside the other buffer.
 */
static bool
buffer_is_contained(const LWGEOM *inner, const LWGEOM *outer)
{
  assert(inner); assert(outer);
  /* First obtain a point on the boundary of the candidate inner buffer */
  double x, y;
  if (! buffer_boundary_representative_point(inner, &x, &y))
    return false;
  /* Locate the corresponding edge again so that we can determine
   * its local normal */
  MeosArray *edges = geom_extract_edges(inner);
  const Edge *representative = NULL;
  for (uint32_t i = 0; i < edges->count; i++)
  {
    const Edge *edge = (const Edge *) meos_array_get(edges, i);
    if (! buffer_is_boundary_edge(edge))
      continue;
    representative = edge;
    break;
  }
  if (! representative)
  {
    meos_array_destroy(edges);
    return false;
  }
  double nx, ny;
  if (! buffer_edge_normals(representative, &nx, &ny))
  {
    meos_array_destroy(edges);
    return false;
  }
  meos_array_destroy(edges);

  /* Move slightly to either side of the boundary. We test both sides because
   * we do not want to depend on the orientation of the curve ring. */
  double epsilon = buffer_containment_epsilon(inner);
  double x1 = x + nx * epsilon;
  double y1 = y + ny * epsilon;
  double x2 = x - nx * epsilon;
  double y2 = y - ny * epsilon;
  int loc1 = buffer_point_location(outer, x1, y1);
  int loc2 = buffer_point_location(outer, x2, y2);

  /* If either side of the boundary is inside the outer buffer, the candidate
   * buffer is contained in it, provided the boundaries are known not to
   * intersect */
  if (loc1 == 0 || loc2 == 0)
    return true;
  return false;
}

/**
 * @brief Classify the relationship between two buffer surfaces.
 * @return
 *   0 = disjoint
 *   1 = boundary intersection
 *   2 = first buffer contained in second
 *   3 = second buffer contained in first
 *   4 = coincident/degenerate relationship
 */
static int
buffer_components_relation(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  /* Boundary intersection has priority */
  if (buffer_boundaries_intersect(geom1, geom2))
    return 1;
  /* Boundaries are disjoint. Therefore containment can be tested */
  if (buffer_is_contained(geom1, geom2))
    return 2;
  if (buffer_is_contained(geom2, geom1))
    return 3;
  return 0;
}

/*****************************************************************************
 * Buffer overlay - containment detection
 *****************************************************************************/

/*****************************************************************************
 * Buffer overlay - intersection point collection
 *****************************************************************************/

/**
 * @brief Return how far apart two computations of ONE boundary node may lie
 * @details Where two boundaries meet at a shallow angle their crossing is
 * ill-conditioned: moving either curve by the rounding of its own arithmetic
 * moves the crossing by the SQUARE ROOT of that rounding, so two routes to the
 * same node — one boundary's arc against the other's offset, and the mirror of
 * it — answer points that far apart. Deduplicating a node set at the tolerance
 * a coordinate is STORED to therefore keeps one node twice, and the piece
 * between the two copies is a sliver belonging to no boundary.
 */
static double
buffer_node_tolerance(double x, double y)
{
  /* The square root belongs to the ROUNDING the coordinates carry, not to the
   * epsilon alone with the coordinates multiplied in afterwards: an
   * ill-conditioned crossing moves by the square root of the rounding of its
   * inputs, and that rounding is `DBL_EPSILON * scale`. Taking the root of the
   * epsilon and scaling the result multiplies two unrelated magnitudes
   * together, which at a projected 6.4e6 bounds one node by 0.318 metres --
   * a third of a buffer of radius 1, so that two crossings a centimetre apart
   * read as one node and the ring closes around the wrong pieces. The two
   * forms agree at unit coordinates, where the distinction does not arise */
  double scale = Max(fabs(x), fabs(y));
  double tol = sqrt(4.0 * DBL_EPSILON * Max(scale, 1.0));
  return Max(tol, MEOS_GEOM_TOLERANCE);
}

/**
 * @brief Add an intersection point to an array.
 * @details Duplicate points are ignored. This is important because
 * adjacent buffer segments may report the same topological node.
 */
static void
buffer_intersections_add(MeosArray *array, double x, double y)
{
  assert(array);
  /* Avoid inserting the same node more than once */
  for (uint32_t i = 0; i < array->count; i++)
  {
    const POINT2D *point = (POINT2D *) meos_array_get(array, i);
    double tol = buffer_node_tolerance(x, y);
    if (fabs(point->x - x) <= tol && fabs(point->y - y) <= tol)
      return;
  }
  POINT2D new;
  new.x = x;
  new.y = y;
  meos_array_add(array, &new);
}

/**
 * @brief Add an intersection point to an array if it is not already present.
 */
static void
buffer_add_intersection_point(MeosArray *points, double x, double y)
{
  assert(points);
  for (uint32_t i = 0; i < points->count; i++)
  {
    const POINT2D *p = (const POINT2D *) meos_array_get(points, i);
    double tol = buffer_node_tolerance(x, y);
    if (fabs(p->x - x) <= tol && fabs(p->y - y) <= tol)
      return;
  }
  POINT2D point;
  point.x = x;
  point.y = y;
  meos_array_add(points, &point);
}

/**
 * @brief Collect intersections between two straight buffer edges.
 */
static void
buffer_collect_line_line_intersections(const Edge *e1, const Edge *e2,
  MeosArray *points)
{
  assert(e1); assert(e2); assert(points);
  IntersectResult result = linesegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy,
    e2->x1, e2->y1, e2->x2, e2->y2);
  if (result.type == INTERSECT_POINT)
  {
    double x = e1->x1 + result.t0 * e1->dx;
    double y = e1->y1 + result.t0 * e1->dy;
    buffer_add_intersection_point(points, x, y);
  }
  else if (result.type == INTERSECT_OVERLAP)
  {
    /* For an overlapping segment there are two nodes */
    double x0 = e1->x1 + result.t0 * e1->dx;
    double y0 = e1->y1 + result.t0 * e1->dy;
    double x1 = e1->x1 + result.t1 * e1->dx;
    double y1 = e1->y1 + result.t1 * e1->dy;
    buffer_add_intersection_point(points, x0, y0);
    buffer_add_intersection_point(points, x1, y1);
  }
}

/**
 * @brief Collect intersections between a straight edge and an arc.
 * @details arcsegm_intersect() returns parameters along the straight segment.
 * Therefore the exact intersection coordinates can be reconstructed
 * directly without approximating the circular arc.
 */
static void
buffer_collect_line_arc_intersections(const Edge *line, const Edge *arc,
  MeosArray *points)
{
  assert(line); assert(arc); assert(points);
  double roots[2];
  int n = arcsegm_intersect(line->x1, line->y1, line->dx, line->dy, arc,
    roots);
  for (int i = 0; i < n; i++)
  {
    double x = line->x1 + roots[i] * line->dx;
    double y = line->y1 + roots[i] * line->dy;
    buffer_add_intersection_point(points, x, y);
  }
}

/**
 * @brief Collect intersections between two circular arcs.
 * @details This is the same geometric construction already used by
 * #arcarc_intersect(), but instead of returning only a Boolean,
 * this function records the actual intersection coordinates.
 */
static void
buffer_collect_arc_arc_intersections(const Edge *e1, const Edge *e2,
  MeosArray *points)
{
  assert(e1); assert(e2); assert(points);
  double dx = e2->cx - e1->cx;
  double dy = e2->cy - e1->cy;
  double d = hypot(dx, dy);
  double r1 = e1->radius;
  double r2 = e2->radius;

  /* Concentric circles.
   * Coincident arcs are not split here. Their common endpoints are
   * already existing boundary nodes and will be handled separately. */
  if (d < MEOS_GEOM_TOLERANCE)
  {
    if (fabs(r1 - r2) > MEOS_GEOM_TOLERANCE)
      return;

    /* Same supporting circle. Add common endpoints. */
    const double angles1[2] = {e1->theta0, e1->theta1};
    const double angles2[2] = {e2->theta0, e2->theta1};
    for (int i = 0; i < 2; i++)
    {
      double x = e1->cx + r1 * cos(angles1[i]);
      double y = e1->cy + r1 * sin(angles1[i]);
      if (point_on_arc(x, y, e2))
        buffer_add_intersection_point(points, x, y);
    }
    for (int i = 0; i < 2; i++)
    {
      double x = e2->cx + r2 * cos(angles2[i]);
      double y = e2->cy + r2 * sin(angles2[i]);
      if (point_on_arc(x, y, e1))
        buffer_add_intersection_point(points, x, y);
    }
    return;
  }

  /* Two circles that are not the same circle meet where
   * #relate_arc_arc_points() places them: the arithmetic that solves them and
   * keeps the solutions both angular spans hold is one computation, and the
   * overlay reads the points it answers. The coincident case above stays here,
   * since what the two engines want of it differs -- the overlay splits at the
   * ends of the shared stretch, the relate engine reads the stretch itself */
  double x[2], y[2];
  bool overlap = false;
  int n = relate_arc_arc_points(e1, e2, x, y, &overlap);
  for (int i = 0; i < n; i++)
    buffer_add_intersection_point(points, x[i], y[i]);
}

/**
 * @brief Test whether a point lies on a circular buffer arc.
 * @details The radial distance is the caller's question, since what it is read
 * against depends on the coordinates the point is built from; this answers the
 * angular half through #arc_span_contains(), the one statement of it
 */
static bool
buffer_point_on_arc(const BufferPiece *arc, double x, double y)
{
  assert(arc); assert(arc->type == BUFFER_ARC);
  return arc_span_contains(arc->theta1, arc->theta2, arc->ccw,
    atan2(y - arc->cy, x - arc->cx));
}

/*****************************************************************************
 * Buffer overlay - coincident piece equivalence
 *****************************************************************************/

/**
 * @brief Return true if two scalar values are equal within the geometric
 * tolerance used by the buffer overlay.
 */
static bool
buffer_values_equal(double a, double b)
{
  return fabs(a - b) <= MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return true if two points are equal within the geometric tolerance.
 */
static bool
buffer_piece_points_equal(double x1, double y1, double x2, double y2)
{
  return buffer_values_equal(x1, x2) && buffer_values_equal(y1, y2);
}

/**
 * @brief Return true if two straight buffer pieces represent the same
 * geometric segment, independently of orientation.
 */
static bool
buffer_segments_equal(const BufferPiece *a, const BufferPiece *b)
{
  assert(a); assert(b);
  if (a->type != BUFFER_SEGMENT || b->type != BUFFER_SEGMENT)
    return false;
  if (buffer_piece_points_equal(a->x1, a->y1, b->x1, b->y1) &&
      buffer_piece_points_equal(a->x2, a->y2, b->x2, b->y2))
    return true;
  return
    buffer_piece_points_equal(a->x1, a->y1, b->x2, b->y2) &&
    buffer_piece_points_equal(a->x2, a->y2, b->x1, b->y1);
}

/**
 * @brief Return true if two circular buffer pieces lie on the same circle.
 */
static bool
buffer_arcs_same_circle(const BufferPiece *a, const BufferPiece *b)
{
  assert(a); assert(b);
  if (a->type != BUFFER_ARC || b->type != BUFFER_ARC)
    return false;
  return
    buffer_values_equal(a->cx, b->cx) &&
    buffer_values_equal(a->cy, b->cy) &&
    buffer_values_equal(a->radius, b->radius);
}

/**
 * @brief Return true if two circular arcs represent the same geometric
 * arc, independently of traversal direction.
 */
static bool
buffer_arcs_equal(const BufferPiece *a, const BufferPiece *b)
{
  assert(a); assert(b);
  if (! buffer_arcs_same_circle(a, b))
    return false;

  /* The two endpoints must be the same geometric points. 
   * Orientation is deliberately ignored. */
  bool same_endpoints =
    buffer_piece_points_equal(a->x1, a->y1, b->x1, b->y1) &&
    buffer_piece_points_equal(a->x2, a->y2, b->x2, b->y2);
  bool reversed_endpoints =
    buffer_piece_points_equal(a->x1, a->y1, b->x2, b->y2) &&
    buffer_piece_points_equal(a->x2, a->y2, b->x1, b->y1);
  if (! same_endpoints && ! reversed_endpoints)
    return false;

  /* Equal endpoints alone are not enough for circles because there are two
   * possible arcs between two points. Check the midpoint of A  and verify
   * that it lies on B. */
  double sweep = a->ccw ?
    angle_normalize(a->theta2 - a->theta1) :
    angle_normalize(a->theta1 - a->theta2);
  double mid = a->ccw ?
    a->theta1 + sweep * 0.5 :
    a->theta1 - sweep * 0.5;
  return arc_span_contains(b->theta1, b->theta2, b->ccw, mid);
}

/**
 * @brief Return true if two buffer pieces represent the same
 * geometric locus.
 * @details The orientation of the pieces is ignored.
 */
static bool
buffer_pieces_equal(const BufferPiece *a, const BufferPiece *b)
{
  assert(a); assert(b);
  if (a->type != b->type)
    return false;
  if (a->type == BUFFER_SEGMENT)
    return buffer_segments_equal(a, b);
  if (a->type == BUFFER_ARC)
    return buffer_arcs_equal(a, b);
  return false;
}

/**
 * @brief Return true if a piece is already present in an array.
 */
static bool
buffer_piece_array_contains(const MeosArray *pieces, const BufferPiece *piece)
{
  assert(pieces); assert(piece);
  for (uint32_t i = 0; i < pieces->count; i++)
  {
    const BufferPiece *piece_i = (BufferPiece *) meos_array_get(pieces, i);
    if (buffer_pieces_equal(piece_i, piece))
      return true;
  }
  return false;
}

/*****************************************************************************
 * Native buffer boundary splitting
 *****************************************************************************/

/**
 * @brief Add a piece to an array unless an equivalent geometric piece is
 * already present.
 */
static void
buffer_pieces_add_unique(MeosArray *pieces, BufferPiece *piece)
{
  assert(pieces); assert(piece);
  if (! buffer_piece_array_contains(pieces, piece))
    meos_array_add(pieces, piece);
}

/**
 * @brief Return the directed angular parameter of a point on an arc.
 * @details The returned value is an angular distance from the start of the 
 * arc, measured in the direction of travel.
 * - For a CCW arc: parameter = angle - theta1
 * - For a CW arc:  parameter = theta1 - angle
 * The result is normalized to [0, 2*pi).
 */
static double
buffer_arc_parameter(const BufferPiece *piece, POINT2D *point)
{
  assert(piece); assert(point); assert(piece->type == BUFFER_ARC);
  double theta = atan2(point->y - piece->cy, point->x - piece->cx);
  if (piece->ccw)
    return angle_normalize(theta - piece->theta1);
  return angle_normalize(piece->theta1 - theta);
}

/**
 * @brief Return the total angular sweep of an arc.
 */
static double
buffer_arc_sweep(const BufferPiece *piece)
{
  assert(piece); assert(piece->type == BUFFER_ARC);
  if (piece->ccw)
    return angle_normalize(piece->theta2 - piece->theta1);
  return angle_normalize(piece->theta1 - piece->theta2);
}

/**
 * @brief Return true if a point belongs to a buffer piece.
 * @details This is a geometric test rather than an intersection test.
 * It is used to determine which collected nodes belong to a particular
 * boundary piece before splitting it.
 */
static bool
buffer_piece_contains_point(const BufferPiece *piece, POINT2D *point)
{
  assert(piece); assert(point);
  if (piece->type == BUFFER_SEGMENT)
    /* The node the splitter asks about was placed by the arithmetic of the
     * boundary that produced it, so it lies off the piece by the rounding of
     * ITS OWN COORDINATES rather than by the size of the piece. Bounding the
     * collinearity by the piece's local lengths asks a node at a projected
     * 6.4e6 to be collinear to 1e-12 of a metre, which is a thousand times
     * finer than the last bit of the coordinates it is built from */
    return point_on_segment(point->x, point->y, piece->x1, piece->y1,
      piece->x2, piece->y2);

  if (piece->type == BUFFER_ARC)
  {
    double dx = point->x - piece->cx;
    double dy = point->y - piece->cy;
    double distance = hypot(dx, dy);
    /* The radial distance is read from the coordinates of the point and of
     * the centre, so it carries their rounding and not the arc's radius */
    if (fabs(distance - piece->radius) >
        fmax(coordinate_tolerance(point->x, piece->cx),
          coordinate_tolerance(point->y, piece->cy)))
      return false;
    /* Then check that its angle lies within the finite arc */
    return buffer_point_on_arc(piece, point->x, point->y);
  }
  return false;
}

/**
 * @brief Sort split points according to their position on a boundary piece.
 */
static int
buffer_split_point_cmp(const void *a, const void *b)
{
  assert(a); assert(b);
  const BufferSplitPoint *p1 = (const BufferSplitPoint *) a;
  const BufferSplitPoint *p2 = (const BufferSplitPoint *) b;
  if (p1->parameter < p2->parameter)
    return -1;
  if (p1->parameter > p2->parameter)
    return 1;
  return 0;
}

/**
 * @brief Add a split point if it is not already present.
 */
static void
buffer_split_point_add(BufferSplitPoint *points, uint32_t *count,
  uint32_t capacity, POINT2D *point, double parameter)
{
  assert(points); assert(count); assert(point);
  /* Duplicate parameters correspond either to duplicate intersection
   * nodes or to an intersection occurring at an existing endpoint.
   * The parameter is normalised along the piece, so the SAME node reads a
   * parameter that differs by the distance between the two copies divided by
   * the piece's length, which is far above the tolerance a coordinate is
   * stored to. The points are therefore compared where they lie, at the
   * tolerance one boundary node is placed to. The piece's own ends are added
   * first, so a node that is one of them keeps the piece's own geometry and
   * splits nothing off it */
  double tol = buffer_node_tolerance(point->x, point->y);
  for (uint32_t i = 0; i < *count; i++)
  {
    if (fabs(points[i].parameter - parameter) <= MEOS_GEOM_TOLERANCE ||
        (fabs(points[i].point.x - point->x) <= tol &&
         fabs(points[i].point.y - point->y) <= tol))
      return;
  }
  if (*count >= capacity)
    return;
  points[*count].point = *point;
  points[*count].parameter = parameter;
  (*count)++;
}

/**
 * @brief Split a linear buffer piece at the supplied intersection nodes.
 */
static void
buffer_split_segment(const BufferPiece *piece, const MeosArray *intersections,
  MeosArray *result)
{
  assert(piece); assert(intersections); assert(result);
  assert(piece->type == BUFFER_SEGMENT);
  /* At most all intersection points plus the two endpoints */
  uint32_t capacity = intersections->count + 2;
  BufferSplitPoint *points = palloc(sizeof(BufferSplitPoint) * capacity);
  uint32_t count = 0;
  POINT2D start = {piece->x1, piece->y1};
  POINT2D end = {piece->x2, piece->y2};
  buffer_split_point_add(points, &count, capacity, &start, 0.0);
  buffer_split_point_add(points, &count, capacity, &end, 1.0);
  for (uint32_t i = 0; i < intersections->count; i++)
  {
    POINT2D *point = (POINT2D *) meos_array_get(intersections, i);
    if (! buffer_piece_contains_point(piece, point))
      continue;
    double parameter = buffer_segment_parameter(piece, point->x, point->y);
    /* Ignore nodes outside the segment due to numerical noise */
    if (parameter < -MEOS_GEOM_TOLERANCE || parameter > 1.0 + MEOS_GEOM_TOLERANCE)
      continue;
    if (parameter < 0.0)
      parameter = 0.0;
    else if (parameter > 1.0)
      parameter = 1.0;
    buffer_split_point_add(points, &count, capacity, point, parameter);
  }
  qsort(points, count, sizeof(BufferSplitPoint), buffer_split_point_cmp);

  /* Generate one segment between every pair of consecutive nodes */
  for (uint32_t i = 0; i + 1 < count; i++)
  {
    POINT2D p1 = points[i].point;
    POINT2D p2 = points[i + 1].point;
    if (hypot(p2.x - p1.x, p2.y - p1.y) <= MEOS_GEOM_TOLERANCE)
      continue;
    BufferPiece split;
    memset(&split, 0, sizeof(BufferPiece));
    split.type = BUFFER_SEGMENT;
    split.x1 = p1.x;
    split.y1 = p1.y;
    split.x2 = p2.x;
    split.y2 = p2.y;
    meos_array_add(result, &split);
  }
  pfree(points);
}

/**
 * @brief Split a circular buffer piece at supplied intersection nodes.
 */
static void
buffer_split_arc(const BufferPiece *piece, const MeosArray *intersections,
  MeosArray *result)
{
  assert(piece); assert(intersections); assert(result);
  assert(piece->type == BUFFER_ARC);
  uint32_t capacity = intersections->count + 2;
  BufferSplitPoint *points = palloc(sizeof(BufferSplitPoint) * capacity);
  uint32_t count = 0;
  POINT2D start = {piece->x1, piece->y1};
  POINT2D end = {piece->x2, piece->y2};
  double sweep = buffer_arc_sweep(piece);
  buffer_split_point_add(points, &count, capacity, &start, 0.0);
  buffer_split_point_add(points, &count, capacity, &end, sweep);
  for (uint32_t i = 0; i < intersections->count; i++)
  {
    POINT2D *point = (POINT2D *) meos_array_get(intersections, i);
    if (! buffer_piece_contains_point(piece, point))
      continue;
    double parameter = buffer_arc_parameter(piece, point);
    /* Ignore nodes outside the finite arc */
    if (parameter < -MEOS_GEOM_TOLERANCE || parameter > sweep + MEOS_GEOM_TOLERANCE)
      continue;
    if (parameter < 0.0)
      parameter = 0.0;
    else if (parameter > sweep)
      parameter = sweep;
    buffer_split_point_add(points, &count, capacity, point, parameter);
  }
  qsort(points, count, sizeof(BufferSplitPoint), buffer_split_point_cmp);
  /* Generate one circular arc between every consecutive pair of nodes */
  for (uint32_t i = 0; i + 1 < count; i++)
  {
    double theta_start, theta_end;
    if (piece->ccw)
    {
      theta_start = piece->theta1 + points[i].parameter;
      theta_end = piece->theta1 + points[i + 1].parameter;
    }
    else
    {
      theta_start = piece->theta1 - points[i].parameter;
      theta_end = piece->theta1 - points[i + 1].parameter;
    }
    double sub_sweep = points[i + 1].parameter - points[i].parameter;
    if (sub_sweep <= MEOS_GEOM_TOLERANCE)
      continue;

    /* Use the actual sorted intersection coordinates as endpoints.
     * This avoids unnecessarily recomputing the coordinates and preserves
     * the exact node shared with another boundary piece. */
    POINT2D p1 = points[i].point;
    POINT2D p2 = points[i + 1].point;
    BufferPiece split;
    memset(&split, 0, sizeof(BufferPiece));
    split.type = BUFFER_ARC;
    split.x1 = p1.x;
    split.y1 = p1.y;
    split.x2 = p2.x;
    split.y2 = p2.y;
    split.cx = piece->cx;
    split.cy = piece->cy;
    split.radius = piece->radius;
    split.theta1 = theta_start;
    split.theta2 = theta_end;
    split.ccw = piece->ccw;
    meos_array_add(result, &split);
  }
  pfree(points);
}

/**
 * @brief Collect the boundary pieces of an areal geometry
 * @details #geom_extract_edges() decomposes every geometry the clip engine
 * supports into straight and circular edges, whatever its ring count, its
 * component count, and whether a ring is a line string, a circular string or a
 * compound curve of both. An edge carries exactly what a piece holds, so
 * reading the boundary through it is what lets the overlay run on a geometry
 * that is not one circular ring: the buffer of a line string is a compound
 * curve, and the union of two buffers has holes and several surfaces.
 */
static void
buffer_piece_from_edge(const Edge *edge, BufferPiece *piece)
{
  assert(edge); assert(piece);
  memset(piece, 0, sizeof(BufferPiece));
  piece->x1 = edge->x1;
  piece->y1 = edge->y1;
  piece->x2 = edge->x2;
  piece->y2 = edge->y2;
  if (edge->etype == EDGE_POLYARC || edge->etype == EDGE_LINEARC)
  {
    piece->type = BUFFER_ARC;
    piece->cx = edge->cx;
    piece->cy = edge->cy;
    piece->radius = edge->radius;
    /* An Edge names the arc angles theta0/theta1 and a piece theta1/theta2 */
    piece->theta1 = edge->theta0;
    piece->theta2 = edge->theta1;
    piece->ccw = edge->ccw;
  }
  else
    piece->type = BUFFER_SEGMENT;
}

/**
 * @brief Collect the boundary pieces of an areal geometry
 * @details See #buffer_piece_from_edge()
 */
static bool
buffer_pieces_from_geometry(const LWGEOM *geom, MeosArray *pieces)
{
  assert(geom); assert(pieces);
  MeosArray *edges = geom_extract_edges(geom);
  if (! edges)
    return false;
  for (uint32_t i = 0; i < edges->count; i++)
  {
    const Edge *edge = (const Edge *) meos_array_get(edges, i);
    if (! edge || ! buffer_is_boundary_edge(edge))
      continue;
    BufferPiece piece;
    buffer_piece_from_edge(edge, &piece);
    meos_array_add(pieces, &piece);
  }
  meos_array_destroy(edges);
  return meos_array_count(pieces) > 0;
}

/**
 * @brief Split all pieces of a buffer boundary at intersection nodes.
 * @details Every resulting piece is either a straight segment or an exact
 * circular arc. A circular arc stays an arc, never a chord approximating it.
 */
static void
buffer_split_pieces(const MeosArray *pieces, const MeosArray *intersections,
  MeosArray *result)
{
  assert(pieces); assert(intersections); assert(result);
  for (uint32_t i = 0; i < pieces->count; i++)
  {
    const BufferPiece *piece = (const BufferPiece *) meos_array_get(pieces, i);
    if (! piece)
      continue;
    if (piece->type == BUFFER_SEGMENT)
      buffer_split_segment(piece, intersections, result);
    else if (piece->type == BUFFER_ARC)
      buffer_split_arc(piece, intersections, result);
  }
}

/*****************************************************************************
 * Native buffer boundary classification
 *****************************************************************************/

/**
 * @brief Compute a representative point in the interior of a buffer piece.
 * @details For a segment the midpoint is used. For a circular arc the angular
 * midpoint is used. The returned point is therefore exactly on the supporting
 * circle and does not approximate the arc by a chord.
 */
static bool
buffer_piece_midpoint(const BufferPiece *piece, POINT2D *point)
{
  assert(piece); assert(point);
  if (piece->type == BUFFER_SEGMENT)
  {
    point->x = (piece->x1 + piece->x2) * 0.5;
    point->y = (piece->y1 + piece->y2) * 0.5;
    return true;
  }
  if (piece->type == BUFFER_ARC)
  {
    double sweep = buffer_arc_sweep(piece);
    if (sweep <= MEOS_GEOM_TOLERANCE)
      return false;
    double theta;
    if (piece->ccw)
      theta = piece->theta1 + sweep * 0.5;
    else
      theta = piece->theta1 - sweep * 0.5;
    point->x = piece->cx + piece->radius * cos(theta);
    point->y = piece->cy + piece->radius * sin(theta);
    return true;
  }
  return false;
}

/**
 * @brief Classify one split boundary piece with respect to a geometry.
 * @details The representative point is located against the complete other
 * buffer. This is a classification of the boundary itself:
 * - EXTERIOR  -> the piece lies outside the other buffer and may belong
 *   to the exterior union boundary.
 * - INTERIOR  -> the piece lies inside the other buffer and must not
 *   belong to the exterior union boundary.
 * - BOUNDARY  -> the piece coincides with, or touches, the other boundary.
 *   This case is retained for the later coincident-boundary handling.
 */
static BufferPieceLocation
buffer_classify_piece(const BufferPiece *piece, const LWGEOM *other)
{
  assert(piece); assert(other);
  POINT2D midpoint;
  if (! buffer_piece_midpoint(piece, &midpoint))
    return BUFFER_PIECE_BOUNDARY;
  int location = buffer_point_location(other, midpoint.x, midpoint.y);
  switch (location)
  {
    case 0:
      return BUFFER_PIECE_INTERIOR;
    case 1:
      return BUFFER_PIECE_BOUNDARY;
    case 2:
      return BUFFER_PIECE_EXTERIOR;
    default:
      return BUFFER_PIECE_BOUNDARY;
  }
}

/*****************************************************************************
 * Buffer overlay - coincident boundary classification
 *****************************************************************************/

/**
 * @brief Compute a point on the left/right side of a buffer piece.
 * @details The piece orientation determines the tangent direction.
 * For a circular arc, the tangent is evaluated at the angular midpoint.
 * The returned points are very close to the boundary. They are used only to
 * determine which side of a coincident boundary the other buffer occupies.
 */
static bool
buffer_piece_side_points(const BufferPiece *piece, double epsilon,
  POINT2D *left, POINT2D *right)
{
  assert(piece); assert(left); assert(right);
  POINT2D midpoint;
  if (! buffer_piece_midpoint(piece, &midpoint))
    return false;
  double tx, ty;
  if (piece->type == BUFFER_SEGMENT)
  {
    tx = piece->x2 - piece->x1;
    ty = piece->y2 - piece->y1;
  }
  else if (piece->type == BUFFER_ARC)
  {
    double sweep = buffer_arc_sweep(piece);
    if (sweep <= MEOS_GEOM_TOLERANCE)
      return false;
    double theta;
    if (piece->ccw)
      theta = piece->theta1 + sweep * 0.5;
    else
      theta = piece->theta1 - sweep * 0.5;
    /* Tangent to a circle */
    if (piece->ccw)
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
  else
  {
    return false;
  }
  double length = hypot(tx, ty);
  if (length <= MEOS_GEOM_TOLERANCE)
    return false;
  tx /= length;
  ty /= length;
  /* Left-hand normal */
  double nx = -ty;
  double ny =  tx;
  left->x  = midpoint.x + epsilon * nx;
  left->y  = midpoint.y + epsilon * ny;
  right->x = midpoint.x - epsilon * nx;
  right->y = midpoint.y - epsilon * ny;
  return true;
}

/**
 * @brief Determine which side of a boundary piece is inside a geometry.
 * @return Return values:
 *   -1 : neither side is interior
 *    0 : left side is interior
 *    1 : right side is interior
 *    2 : both sides are interior
 * Boundary classification is treated conservatively: if an offset point
 * still falls on the boundary, the offset distance is increased.
 */
static int
buffer_piece_interior_side(const BufferPiece *piece, const LWGEOM *geom)
{
  assert(piece); assert(geom);
  /* Start with a small displacement relative to the piece itself */
  double scale = 1.0;
  double dx = piece->x2 - piece->x1;
  double dy = piece->y2 - piece->y1;
  if (piece->type == BUFFER_ARC)
    scale = piece->radius;
  else
    scale = hypot(dx, dy);
  if (scale <= MEOS_GEOM_TOLERANCE)
    scale = 1.0;
  double epsilon = fmax(MEOS_GEOM_TOLERANCE * 100.0, scale * 1.0e-8);
  for (int attempt = 0; attempt < 4; attempt++)
  {
    POINT2D left, right;
    if (! buffer_piece_side_points(piece, epsilon, &left, &right))
      return -1;
    int left_loc = buffer_point_location(geom, left.x, left.y);
    int right_loc = buffer_point_location(geom, right.x, right.y);

    /* buffer_point_location():
     *   0 = interior
     *   1 = boundary
     *   2 = exterior */
    if (left_loc != 1 && right_loc != 1)
    {
      bool left_inside = left_loc == 0;
      bool right_inside = right_loc == 0;
      if (left_inside && right_inside)
        return 2;
      if (left_inside)
        return 0;
      if (right_inside)
        return 1;
      return -1;
    }
    epsilon *= 10.0;
  }
  return -1;
}

/**
 * @brief Determine whether a coincident boundary piece is an exterior
 * boundary of the union.
 * @details This is based on the side occupied by the other geometry.
 * - If the other geometry occupies the same side as this buffer, the common
 *   boundary is external to the union.
 * - If the other geometry occupies the opposite side, the common boundary is
 *   internal to the union.
 */
static int
buffer_classify_coincident_piece(const BufferPiece *piece, const LWGEOM *owner,
  const LWGEOM *other)
{
  assert(piece); assert(owner); assert(other);
  int owner_side = buffer_piece_interior_side(piece, owner);
  int other_side = buffer_piece_interior_side(piece, other);
  /* We need to know the side occupied by both geometries. */
  if (owner_side < 0 || other_side < 0)
    return -1;
  /* Both geometries occupy the same side of the boundary.
   * The boundary is external to their union. */
  if (owner_side == other_side)
    return 1;
  /* The geometries occupy opposite sides.
   * The coincident boundary lies inside the union. */
  if ((owner_side == 0 && other_side == 1) ||
      (owner_side == 1 && other_side == 0))
    return 0;
  /* Degenerate/tangential situation. Defer it. */
  return -1;
}

/**
 * @brief Resolve coincident pieces belonging to one buffer.
 * @details If the coincident piece is external to the union, it is retained.
 * If it is internal, it is discarded.
 * @note The caller is responsible for preventing the same geometric piece
 * from being inserted twice when processing the second buffer.
 */
static bool
buffer_resolve_coincident_piece(BufferPiece *piece, const LWGEOM *owner,
  const LWGEOM *other, ClipOper oper, MeosArray *result)
{
  assert(piece); assert(owner); assert(other); assert(result);
  int classification = buffer_classify_coincident_piece(piece, owner, other);
  /* Unknown / degenerate topology */
  if (classification < 0)
    return false;

  /* Which side each interior lies on is the whole of what the operation needs,
   * and the two answers it can give divide the three operations in one place:
   *
   *   THE SAME SIDE -- the two overlap along here, so a union and an
   *   intersection are both bounded by the piece, while a difference has
   *   nothing to bound: where both are present, the first less the second
   *   covers nothing.
   *
   *   OPPOSITE SIDES -- the interiors meet along the piece without overlapping,
   *   so a difference is bounded by it, the first geometry lying on its own
   *   side; a union reads it as interior, and an intersection as bounding a
   *   region that is not there.
   */
  bool same_side = (classification == 1);
  if (same_side == (oper != CL_DIFFERENCE))
    buffer_pieces_add_unique(result, piece);
  return true;
}

/**
 * @brief Collect all exact boundary intersection nodes into the intersection
 * array.
 * @details The existing low-level intersection routines operate on MeosArray.
 * We therefore use a temporary MeosArray for each edge pair and transfer
 * the resulting points to MeosArray.
 */
static bool
buffer_collect_boundary_intersections(const LWGEOM *geom1, const LWGEOM *geom2,
  MeosArray *intersections)
{
  assert(geom1); assert(geom2); assert(intersections);
  MeosArray *a1 = geom_extract_edges(geom1);
  MeosArray *a2 = geom_extract_edges(geom2);
  if (! a1 || ! a2)
  {
    if (a1)
      meos_array_destroy(a1);
    if (a2)
      meos_array_destroy(a2);
    return false;
  }
  for (uint32_t i = 0; i < a1->count; i++)
  {
    const Edge *e1 = (const Edge *) meos_array_get(a1, i);
    if (! e1 || ! buffer_is_boundary_edge(e1))
      continue;
    for (uint32_t j = 0; j < a2->count; j++)
    {
      const Edge *e2 = (const Edge *) meos_array_get(a2, j);
      if (! e2 || ! buffer_is_boundary_edge(e2))
        continue;
      /* The existing intersection collectors expect MeosArray. */
      MeosArray *points = meos_array_create(sizeof(POINT2D));
      if (! points)
      {
        meos_array_destroy(a1); meos_array_destroy(a2);
        return false;
      }

      if (e1->etype == EDGE_POLYSEG && e2->etype == EDGE_POLYSEG)
        buffer_collect_line_line_intersections(e1, e2, points);
      else if (e1->etype == EDGE_POLYSEG && e2->etype == EDGE_POLYARC)
        buffer_collect_line_arc_intersections(e1, e2, points);
      else if (e1->etype == EDGE_POLYARC && e2->etype == EDGE_POLYSEG)
        buffer_collect_line_arc_intersections(e2, e1, points);
      else if (e1->etype == EDGE_POLYARC && e2->etype == EDGE_POLYARC)
        buffer_collect_arc_arc_intersections(e1, e2, points);

      /* Transfer the points to the intersection array */
      for (uint32_t k = 0; k < points->count; k++)
      {
        const POINT2D *point = (const POINT2D *) meos_array_get(points, k);
        if (point)
          buffer_intersections_add(intersections, point->x, point->y);
      }
      meos_array_destroy(points);
    }
  }
  meos_array_destroy(a1); meos_array_destroy(a2);
  return true;
}

/*****************************************************************************
 * Buffer overlay - union relation dispatch
 *****************************************************************************/

/*****************************************************************************
 * Buffer overlay - union boundary selection
 *****************************************************************************/

/**
 * @brief Select the pieces of two boundaries that bound the overlay
 * @details A piece of one boundary bounds the answer when it lies on the side
 * of the other geometry that the operation keeps, and the operation is the
 * whole of what tells the three apart:
 *
 *     union         exterior(A wrt B) + exterior(B wrt A)
 *     intersection  interior(A wrt B) + interior(B wrt A)
 *     difference    exterior(A wrt B) + interior(B wrt A)
 *
 * so nothing about the two shapes enters beyond where each piece sits.
 * A piece lying ON the other boundary is coincident, and
 * #buffer_resolve_coincident_piece places it from the side each interior
 * occupies. One it cannot place is returned in @p boundary, which the caller
 * reads as a pair this does not answer.
 * @param[out] coincident Set when either boundary carries a piece lying ON the
 * other, so that a caller keeping no piece can tell a meeting at isolated
 * NODES from one along a whole STRETCH. A resolved coincident piece the
 * operation drops leaves neither @p result nor @p boundary anything, so it is
 * the only report of it
 */
static void
buffer_select_overlay_boundary(const MeosArray *pieces_a, const LWGEOM *geom_b,
  const MeosArray *pieces_b, const LWGEOM *geom_a, ClipOper oper,
  MeosArray *result, MeosArray *boundary, bool *coincident)
{
  assert(pieces_a); assert(geom_b); assert(pieces_b); assert(geom_a);
  assert(result); assert(boundary); assert(coincident);
  *coincident = false;
  /* The side of the other geometry each of the two boundaries contributes */
  BufferPieceLocation keep_a = (oper == CL_INTERSECTION) ?
    BUFFER_PIECE_INTERIOR : BUFFER_PIECE_EXTERIOR;
  BufferPieceLocation keep_b = (oper == CL_UNION) ?
    BUFFER_PIECE_EXTERIOR : BUFFER_PIECE_INTERIOR;
  /* Pieces belonging to A */
  for (uint32_t i = 0; i < pieces_a->count; i++)
  {
    BufferPiece *piece = (BufferPiece *) meos_array_get(pieces_a, i);
    BufferPieceLocation location = buffer_classify_piece(piece, geom_b);
    if (location == keep_a)
      meos_array_add(result, piece);
    else if (location == BUFFER_PIECE_BOUNDARY)
    {
      *coincident = true;
      if (! buffer_resolve_coincident_piece(piece, geom_a, geom_b, oper,
            result))
        meos_array_add(boundary, piece);
    }
  }
  /* Pieces belonging to B */
  for (uint32_t i = 0; i < pieces_b->count; i++)
  {
    BufferPiece *piece = (BufferPiece *) meos_array_get(pieces_b, i);
    BufferPieceLocation location = buffer_classify_piece(piece, geom_a);
    if (location == keep_b)
      meos_array_add(result, piece);
    else if (location == BUFFER_PIECE_BOUNDARY)
    {
      *coincident = true;
      if (! buffer_resolve_coincident_piece(piece, geom_b, geom_a, oper,
            result))
        meos_array_add(boundary, piece);
    }
  }
}

/*****************************************************************************
 * Buffer overlay - boundary piece chaining
 *****************************************************************************/

/**
 * @brief Test whether two points represent the same topological node.
 */
static bool
buffer_points_equal(POINT2D p1, POINT2D p2)
{
  /* The two pieces meeting at a node each compute it, and the two results are
   * one node of the boundary however far apart the arithmetic leaves them.
   * That is the question #buffer_node_tolerance answers, and the collection of
   * the intersections and the splitting of the pieces already ask it that way;
   * chaining asks the same question, so it reads the same tolerance. Bounding
   * by a coordinate's own storage tolerance instead reads the two copies of
   * one node as two nodes, and a ring whose pieces are all present fails to
   * close */
  double tol = Max(buffer_node_tolerance(p1.x, p1.y),
    buffer_node_tolerance(p2.x, p2.y));
  return fabs(p1.x - p2.x) <= tol && fabs(p1.y - p2.y) <= tol;
}

/**
 * @brief Return the start point of a buffer piece.
 */
static POINT2D
buffer_piece_start(const BufferPiece *piece)
{
  assert(piece);
  POINT2D result;
  result.x = piece->x1;
  result.y = piece->y1;
  return result;
}

/**
 * @brief Return the end point of a buffer piece.
 */
static POINT2D
buffer_piece_end(const BufferPiece *piece)
{
  assert(piece);
  POINT2D result;
  result.x = piece->x2;
  result.y = piece->y2;
  return result;
}

/**
 * @brief Reverse the orientation of a buffer piece.
 * @details Reversing an arc also reverses its direction of traversal.
 * Therefore theta1/theta2 are exchanged and ccw is inverted.
 */
static void
buffer_piece_reverse(BufferPiece *piece)
{
  assert(piece);
  double tmp;
  tmp = piece->x1;
  piece->x1 = piece->x2;
  piece->x2 = tmp;
  tmp = piece->y1;
  piece->y1 = piece->y2;
  piece->y2 = tmp;
  if (piece->type == BUFFER_ARC)
  {
    tmp = piece->theta1;
    piece->theta1 = piece->theta2;
    piece->theta2 = tmp;
    piece->ccw = ! piece->ccw;
  }
}

/**
 * @brief Append one buffer piece to a compound curve.
 * @details The orientation of the piece is assumed to be the desired traversal
 * direction.
 */
static void
buffer_append_piece_to_curve(LWCOMPOUND *curve, int32_t srid,
  const BufferPiece *piece)
{
  assert(curve); assert(piece);
  POINT2D p1 = buffer_piece_start(piece);
  POINT2D p2 = buffer_piece_end(piece);
  if (piece->type == BUFFER_SEGMENT)
    buffer_add_segment(curve, srid, p1, p2);
  else if (piece->type == BUFFER_ARC)
    buffer_add_arc(curve, srid, piece->cx, piece->cy, piece->radius,
      piece->theta1, piece->theta2, piece->ccw, &p1, &p2);
}

/**
 * @brief Find an unused piece whose endpoint is connected to point.
 * @details If reverse is returned true, the piece must be reversed before it
 * is appended to the boundary.
 */
/**
 * @brief Return the direction a piece leaves one of its ends in
 * @details The tangent of a segment is the segment; the tangent of an arc is
 * the sine and cosine at the angle of the end it leaves, turned the way the
 * arc sweeps. Reading it AT AN END rather than at the midpoint is what tells
 * two pieces apart where they meet: their midpoints say nothing about the node
 * they share. See #buffer_piece_side_points(), which reads the same tangent
 * at the middle to find the sides of a piece
 * @param[in] piece Boundary piece
 * @param[in] at_start Read the direction it leaves its START in, rather than
 * the direction it arrives at its END from
 * @param[out] dx,dy Unit direction
 */
static bool
buffer_piece_end_direction(const BufferPiece *piece, bool at_start,
  double *dx, double *dy)
{
  assert(piece); assert(dx); assert(dy);
  double tx, ty;
  if (piece->type == BUFFER_SEGMENT)
  {
    tx = piece->x2 - piece->x1;
    ty = piece->y2 - piece->y1;
  }
  else if (piece->type == BUFFER_ARC)
  {
    double sweep = buffer_arc_sweep(piece);
    if (sweep <= MEOS_GEOM_TOLERANCE)
      return false;
    double theta = at_start ? piece->theta1 :
      (piece->ccw ? piece->theta1 + sweep : piece->theta1 - sweep);
    if (piece->ccw)
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
  else
    return false;
  double length = hypot(tx, ty);
  if (length <= MEOS_GEOM_TOLERANCE)
    return false;
  /* Leaving the END means travelling back along the piece */
  double sign = at_start ? 1.0 : -1.0;
  *dx = sign * tx / length;
  *dy = sign * ty / length;
  return true;
}

/**
 * @brief Return the piece a boundary continues into at a node, and whether it
 * is traversed in reverse
 * @details WHERE MORE THAN TWO PIECE-ENDS MEET AT ONE NODE the boundary pinches
 * -- two regions of the answer touch there at a single point -- and which piece
 * the walk takes decides whether it closes the two rings that are really there
 * or ONE ring that visits the node twice. The ring is chosen by turning as
 * sharply as the node allows, clockwise from the direction the walk arrives in,
 * which is the traversal that keeps each face of the boundary to itself.
 * A node where exactly two ends meet has one candidate and the rule does not
 * arise
 * @param[in] pieces Boundary pieces
 * @param[in] used Which of them the walk has taken
 * @param[in] point The node the walk stands on
 * @param[in] from_dx,from_dy The direction the walk arrived in, zero for the
 * first piece of a ring, which has no direction to turn from
 * @param[out] reverse Set when the piece is traversed from its end
 */
static int
buffer_find_connected_piece(const MeosArray *pieces, const bool *used,
  POINT2D point, double from_dx, double from_dy, bool *reverse)
{
  assert(pieces); assert(used); assert(reverse);
  int best = -1;
  bool best_reverse = false;
  double best_turn = 0.0;
  bool have_direction = (from_dx != 0.0 || from_dy != 0.0);
  for (uint32_t i = 0; i < pieces->count; i++)
  {
    if (used[i])
      continue;
    const BufferPiece *piece = (BufferPiece *) meos_array_get(pieces, i);
    POINT2D start = buffer_piece_start(piece);
    POINT2D end = buffer_piece_end(piece);
    bool rev;
    if (buffer_points_equal(start, point))
      rev = false;
    else if (buffer_points_equal(end, point))
      rev = true;
    else
      continue;
    /* The first piece of a ring, and a node with one candidate, need no turn */
    if (! have_direction)
    {
      *reverse = rev;
      return (int) i;
    }
    double dx, dy;
    if (! buffer_piece_end_direction(piece, ! rev, &dx, &dy))
    {
      /* A direction this cannot read orders nothing, so it is taken only where
       * nothing else offers itself */
      if (best < 0)
      {
        best = (int) i;
        best_reverse = rev;
        best_turn = 10.0;
      }
      continue;
    }
    /* The turn from the arriving direction to the leaving one, measured
     * clockwise so that the sharpest right turn scores highest. The walk
     * arrives travelling along (from_dx,from_dy) and leaves along (dx,dy) */
    double cross = from_dx * dy - from_dy * dx;
    double dot = from_dx * dx + from_dy * dy;
    double turn = -atan2(cross, dot);
    /* Read the turn as a full circle so that the FIRST piece round from the
     * direction the walk came back along is the smallest, which is the one
     * that keeps the walk on the face it is tracing rather than crossing to
     * the other side of the node */
    if (turn <= 0.0)
      turn += 2.0 * M_PI;
    if (best < 0 || turn < best_turn)
    {
      best = (int) i;
      best_reverse = rev;
      best_turn = turn;
    }
  }
  if (best < 0)
    return -1;
  *reverse = best_reverse;
  return best;
}

/**
 * @brief Chain one connected boundary component and retain its ordered pieces.
 * @details The returned piece array contains copies of the pieces in exactly
 * the traversal order used to construct the compound curve.
 * The input array is never modified.
 */
static LWCOMPOUND *
buffer_chain_ring_with_pieces(const MeosArray *pieces, bool *used,
  uint32_t start_index, int32_t srid, MeosArray *ordered)
{
  assert(pieces); assert(used); assert(ordered);
  assert(start_index < pieces->count);
  const BufferPiece *first = (const BufferPiece *) meos_array_get(pieces,
    start_index);
  if (! first)
    return NULL;
  LWCOMPOUND *curve = lwcompound_construct_empty(srid, 0, 0);
  if (! curve)
    return NULL;
  BufferPiece oriented = *first;
  buffer_append_piece_to_curve(curve, srid, &oriented);
  meos_array_add(ordered, &oriented);
  used[start_index] = true;
  POINT2D start = buffer_piece_start(&oriented);
  POINT2D current = buffer_piece_end(&oriented);
  /* The direction the walk arrives in, which is what decides its turn at a
   * node several pieces share. It is read from the piece just taken, in the
   * orientation it was taken in */
  double from_dx = 0.0, from_dy = 0.0;
  if (! buffer_piece_end_direction(&oriented, false, &from_dx, &from_dy))
    from_dx = from_dy = 0.0;
  while (! buffer_points_equal(current, start))
  {
    bool reverse = false;
    int index = buffer_find_connected_piece(pieces, used, current, -from_dx,
      -from_dy, &reverse);
    if (index < 0)
    {
      lwgeom_free(lwcompound_as_lwgeom(curve));
      return NULL;
    }
    const BufferPiece *piece = (const BufferPiece *) meos_array_get(pieces,
      (uint32_t) index);
    if (! piece)
    {
      lwgeom_free(lwcompound_as_lwgeom(curve));
      return NULL;
    }
    oriented = *piece;
    /* Reverse only the local copy. The input pieces must remain unchanged. */
    if (reverse)
      buffer_piece_reverse(&oriented);
    buffer_append_piece_to_curve(curve, srid, &oriented);
    meos_array_add(ordered, &oriented);
    used[(uint32_t) index] = true;
    current = buffer_piece_end(&oriented);
    if (! buffer_piece_end_direction(&oriented, false, &from_dx, &from_dy))
      from_dx = from_dy = 0.0;
  }
  return curve;
}

/**
 * @brief Chain all selected boundary pieces into closed rings.
 * @details The resulting BufferRingInfo objects retain both the geometric
 * ring and the ordered boundary pieces used to construct it.
 */
static bool
buffer_chain_ring_infos(const MeosArray *pieces, int32_t srid,
  MeosArray *rings)
{
  assert(pieces); assert(rings);
  if (pieces->count == 0)
    return true;
  bool *used = palloc0(sizeof(bool) * pieces->count);
  uint32_t used_count = 0;
  while (used_count < pieces->count)
  {
    uint32_t start_index = UINT32_MAX;
    for (uint32_t i = 0; i < pieces->count; i++)
    {
      if (! used[i])
      {
        start_index = i;
        break;
      }
    }
    if (start_index == UINT32_MAX)
      break;
    MeosArray *ordered = meos_array_create(sizeof(BufferPiece));
    if (! ordered)
    {
      pfree(used);
      return false;
    }
    LWCOMPOUND *ring = buffer_chain_ring_with_pieces(pieces, used, start_index,
      srid, ordered);
    if (! ring)
    {
      meos_array_destroy(ordered); pfree(used);
      return false;
    }
    /* Count the pieces consumed by this ring */
    uint32_t new_used_count = 0;
    for (uint32_t i = 0; i < pieces->count; i++)
    {
      if (used[i])
        new_used_count++;
    }
    if (new_used_count == used_count)
    {
      lwgeom_free(lwcompound_as_lwgeom(ring));
      meos_array_destroy(ordered); pfree(used);
      return false;
    }
    used_count = new_used_count;
    BufferRingInfo info;
    memset(&info, 0, sizeof(BufferRingInfo));
    info.ring = ring;
    info.pieces = ordered;
    info.parent = -1;
    info.depth = 0;
    info.shell = -1;
    meos_array_add(rings, &info);
  }
  pfree(used);
  return used_count == pieces->count;
}

/**
 * @brief Construct a temporary CURVEPOLYGON containing one ring.
 * @details The returned geometry owns the supplied ring.
 */
static LWGEOM *
buffer_make_single_ring_polygon(LWCOMPOUND *ring, int32_t srid)
{
  assert(ring);
  LWCURVEPOLY *polygon = lwcurvepoly_construct_empty(srid, 0, 0);
  if (! polygon)
    return NULL;
  buffer_curvepoly_add_ring(polygon, ring);
  return lwcurvepoly_as_lwgeom(polygon);
}

/**
 * @brief Find a point strictly inside a closed boundary ring.
 * @details A candidate point is generated close to the midpoint of a
 * boundary edge and displaced toward the interior. The candidate is
 * verified with the existing strict point-in-areal test.
 *
 * The displacement is progressively reduced if the initial candidate
 * is not suitable. This is important for narrow buffer regions where a
 * fixed displacement could cross the opposite boundary.
 */
static bool
buffer_ring_find_interior_point(const LWCOMPOUND *ring, int32_t srid,
  double *x, double *y)
{
  assert(ring); assert(x); assert(y);
  MeosArray *arr = geom_extract_edges(lwcompound_as_lwgeom(ring));
  if (! arr || arr->count == 0)
  {
    if (arr)
      meos_array_destroy(arr);
    return false;
  }
  for (uint32_t i = 0; i < arr->count; i++)
  {
    Edge *edge = (Edge *) meos_array_get(arr, i);
    if (! edge)
      continue;

    double mx, my;
    double nx, ny;
    double scale;
    /* Straight edges */
    if (edge->etype == EDGE_POLYSEG || edge->etype == EDGE_LINESEG)
    {
      mx = (edge->x1 + edge->x2) * 0.5;
      my = (edge->y1 + edge->y2) * 0.5;
      double length = hypot(edge->x2 - edge->x1, edge->y2 - edge->y1);
      if (length <= 0.0)
        continue;
      /* Unit normal of the directed edge */
      nx = -(edge->y2 - edge->y1) / length;
      ny =  (edge->x2 - edge->x1) / length;
      scale = length;
    }
    /* Circular arcs */
    else if (edge->etype == EDGE_POLYARC || edge->etype == EDGE_LINEARC)
    {
      double sweep = edge->ccw ?
        angle_normalize(edge->theta1 - edge->theta0) :
        angle_normalize(edge->theta0 - edge->theta1);
      if (sweep <= 0.0 || edge->radius <= 0.0)
        continue;
      double theta = edge->ccw ?
        edge->theta0 + sweep * 0.5 : edge->theta0 - sweep * 0.5;
      mx = edge->cx + edge->radius * cos(theta);
      my = edge->cy + edge->radius * sin(theta);
      /* The radial direction, which is the normal of the arc there */
      nx = (mx - edge->cx) / edge->radius;
      ny = (my - edge->cy) / edge->radius;
      scale = edge->radius;
    }
    else
      continue;

    /* Estimate a suitable displacement from the edge length or radius. The
     * progressively smaller ones keep the test usable in a narrow region, and
     * both sides are tried because which one the ring encloses depends on its
     * orientation. */
    double offsets[] = {
      scale * 1.0e-3,
      scale * 1.0e-4,
      scale * 1.0e-5,
      scale * 1.0e-6,
      scale * 1.0e-7
    };
    for (size_t k = 0; k < sizeof(offsets) / sizeof(offsets[0]); k++)
    {
      for (int side = 0; side < 2; side++)
      {
        double sign = side ? -1.0 : 1.0;
        double cx = mx + sign * nx * offsets[k];
        double cy = my + sign * ny * offsets[k];
        /* The temporary areal geometry serves only the containment test */
        LWCOMPOUND *copy = (LWCOMPOUND *) lwgeom_clone(
            lwcompound_as_lwgeom(ring));
        LWGEOM *polygon = buffer_make_single_ring_polygon(copy, srid);
        if (! polygon)
        {
          meos_array_destroy(arr);
          return false;
        }
        bool inside = buffer_areal_contains_point(polygon, cx, cy);
        lwgeom_free(polygon);
        if (inside)
        {
          *x = cx;
          *y = cy;
          meos_array_destroy(arr);
          return true;
        }
      }
    }
  }
  meos_array_destroy(arr);
  return false;
}

/**
 * @brief Compute a representative point strictly inside a boundary ring.
 * @details The point is obtained from a boundary edge and verified using
 * strict interior containment.
 */
static bool
buffer_ring_representative_point(LWCOMPOUND *ring, int32_t srid,
  double *x, double *y)
{
  assert(ring); assert(x); assert(y);
  return buffer_ring_find_interior_point(ring, srid, x, y);
}

/**
 * @brief Test whether one boundary ring contains another ring.
 * @details The representative point of the inner ring is tested against
 * the areal region bounded by the outer ring. The boundary is excluded
 * from the interior test.
 */
static bool
buffer_ring_contains_ring(const BufferRingInfo *outer,
  const BufferRingInfo *inner, int32_t srid)
{
  assert(outer); assert(inner);
  LWCOMPOUND *ring_copy = (LWCOMPOUND *) lwgeom_clone(
    lwcompound_as_lwgeom(outer->ring));
  if (! ring_copy)
    return false;
  LWGEOM *polygon = buffer_make_single_ring_polygon(ring_copy, srid);
  if (! polygon)
    return false;
  bool result = buffer_areal_contains_point(polygon, inner->x, inner->y);
  lwgeom_free(polygon);
  return result;
}

/**
 * @brief Release the ring and the pieces a boundary ring holds
 * @details A #BufferRingInfo owns the compound curve of its ring and the
 * array of pieces the ring is chained from, and neither is reachable from
 * anything else once the entry holding them goes. An entry whose ring has
 * moved to another array holds none, so releasing it again releases nothing
 * @param[in,out] info Boundary ring
 */
static void
buffer_ring_info_release(BufferRingInfo *info)
{
  assert(info);
  if (info->ring)
  {
    lwgeom_free(lwcompound_as_lwgeom(info->ring));
    info->ring = NULL;
  }
  if (info->pieces)
  {
    meos_array_destroy(info->pieces);
    info->pieces = NULL;
  }
  return;
}

/**
 * @brief Destroy an array of boundary rings together with what its entries
 * hold
 * @param[in] rings Array of boundary rings, owned by this function
 */
static void
buffer_ring_infos_destroy(MeosArray *rings)
{
  assert(rings);
  for (uint32_t i = 0; i < rings->count; i++)
  {
    BufferRingInfo *info = (BufferRingInfo *) meos_array_get(rings, i);
    if (info)
      buffer_ring_info_release(info);
  }
  meos_array_destroy(rings);
  return;
}

/**
 * @brief Release the boundary rings a classification under construction holds
 * @param[in] infos Boundary rings, owned by this function
 * @param[in] count Number of rings the classification has taken
 */
static void
buffer_ring_infos_free(BufferRingInfo *infos, uint32_t count)
{
  assert(infos);
  for (uint32_t i = 0; i < count; i++)
    buffer_ring_info_release(&infos[i]);
  pfree(infos);
  return;
}

/**
 * @brief Build the containment hierarchy of closed boundary rings.
 * @details For every ring, the immediate containing ring is identified
 * using the containment relation between rings. No ring orientation or
 * area calculation is required.
 * If A contains B and B contains C, then B is the parent of C even
 * though A also contains C. The depth is the number of ancestors:
 *   depth 0 -> shell
 *   depth 1 -> hole
 *   depth 2 -> shell
 *   depth 3 -> hole
 *   ...
 */
static bool
buffer_classify_rings(MeosArray *rings, int32_t srid,
  MeosArray *classified)
{
  assert(rings); assert(classified);
  uint32_t count = rings->count;
  if (count == 0)
    return true;
  BufferRingInfo *info = palloc0(sizeof(BufferRingInfo) * count);

  /* Initialize the ring information and compute one representative
   * point strictly inside every ring */
  uint32_t kept = 0;
  for (uint32_t i = 0; i < count; i++)
  {
    BufferRingInfo *ring_info = (BufferRingInfo *) meos_array_get(rings, i);
    if (! ring_info || ! ring_info->ring || ! ring_info->pieces)
    {
      buffer_ring_infos_free(info, kept);
      return false;
    }
    info[kept] = *ring_info;
    info[kept].parent = -1;
    info[kept].depth = 0;
    info[kept].shell = -1;
    /* A ring holding no point at all holds no area, which is what a boundary
     * pinched to zero width leaves behind: where a hole closes exactly, the
     * ring that bounded it survives the split with its two sides coincident.
     * Such a ring contributes no surface and no hole, so it is dropped and
     * the rest of the classification stands. Refusing it instead loses the
     * whole buffer over a ring that bounds nothing */
    if (! buffer_ring_representative_point(info[kept].ring, srid,
        &info[kept].x, &info[kept].y))
      continue;
    /* The ring and its pieces are held by the classification from here, and
     * the chain they are taken from gives them up: one owner releases them */
    ring_info->ring = NULL;
    ring_info->pieces = NULL;
    kept++;
  }
  count = kept;
  if (count == 0)
  {
    pfree(info);
    return true;
  }

  /* First determine all pairwise containment relationships.
   * contains[i * count + j] means that ring i strictly contains
   * the representative point of ring j. */
  bool *contains = palloc0(sizeof(bool) * count * count);
  for (uint32_t i = 0; i < count; i++)
  {
    for (uint32_t j = 0; j < count; j++)
    {
      if (i == j)
        continue;
      contains[i * count + j] = buffer_ring_contains_ring(&info[i], &info[j],
        srid);
    }
  }

  /* Determine the immediate parent.
   * Candidate j contains ring i. It is the immediate parent if there
   * is no third ring k such that:
   *   j contains k
   *   k contains i
   * In other words, no containing ring may lie strictly between j and i
   * in the containment hierarchy. */
  for (uint32_t i = 0; i < count; i++)
  {
    int32_t parent = -1;
    for (uint32_t j = 0; j < count; j++)
    {
      if (i == j)
        continue;
      if (! contains[j * count + i])
        continue;
      bool is_immediate = true;
      for (uint32_t k = 0; k < count; k++)
      {
        if (k == i || k == j)
          continue;

        /* k must be between j and i:
         *     j contains k
         *     k contains i */
        if (contains[j * count + k] && contains[k * count + i])
        {
          is_immediate = false;
          break;
        }
      }
      if (is_immediate)
      {
        /* There must be at most one immediate parent in a valid
         * non-intersecting ring hierarchy */
        if (parent >= 0)
        {
          pfree(contains); buffer_ring_infos_free(info, count);
          return false;
        }
        parent = (int32_t) j;
      }
    }
    info[i].parent = parent;
  }

  /* Derive the depth from the parent hierarchy */
  for (uint32_t i = 0; i < count; i++)
  {
    uint32_t depth = 0;
    int32_t current = info[i].parent;
    uint32_t steps = 0;
    while (current >= 0)
    {
      if ((uint32_t) current >= count)
      {
        pfree(contains); buffer_ring_infos_free(info, count);
        return false;
      }
      depth++;
      current = info[current].parent;
      /* A valid containment hierarchy is acyclic */
      if (++steps > count)
      {
        pfree(contains); buffer_ring_infos_free(info, count);
        return false;
      }
    }
    info[i].depth = depth;
  }

  /* Assign every hole to its immediate containing shell.
   * Because depth alternates between shells and holes, the parent
   * of every odd-depth ring must be an even-depth ring. */
  for (uint32_t i = 0; i < count; i++)
  {
    if ((info[i].depth & 1) == 0)
      continue;
    int32_t parent = info[i].parent;
    if (parent < 0 || (uint32_t) parent >= count)
    {
      pfree(contains); buffer_ring_infos_free(info, count);
      return false;
    }
    if ((info[parent].depth & 1) != 0)
    {
      pfree(contains); buffer_ring_infos_free(info, count);
      return false;
    }
    info[i].shell = parent;
  }
  /* Transfer the classification information into the generic array */
  for (uint32_t i = 0; i < count; i++)
    meos_array_add(classified, &info[i]);
  pfree(contains); pfree(info);
  return true;
}

/**
 * @brief Compute the signed area contribution of a straight buffer piece.
 * @details The contribution is one half of the line integral
 *   x dy - y dx
 * along the segment.
 */
static double
buffer_segment_signed_area(const BufferPiece *piece)
{
  assert(piece);
  assert(piece->type == BUFFER_SEGMENT);
  return 0.5 * (piece->x1 * piece->y2 - piece->x2 * piece->y1);
}

/**
 * @brief Compute the signed area contribution of a circular buffer arc.
 * @details The arc contribution is obtained from the line integral
 *   1/2 * integral(x dy - y dx)
 * along the directed circular arc. The sign of the angular sweep follows
 * the traversal direction: positive for CCW and negative for CW.
 */
static double
buffer_arc_signed_area(const BufferPiece *piece)
{
  assert(piece); assert(piece->type == BUFFER_ARC);
  double theta1 = piece->theta1;
  double theta2 = piece->theta2;
  /* Directed angular sweep.
   * Unlike #buffer_arc_sweep(), the sign is retained because it
   * determines the orientation of the complete ring. */
  double delta = piece->ccw ?
    angle_normalize(theta2 - theta1) : -angle_normalize(theta1 - theta2);
  /* Integral of x dy - y dx for
   *   x = cx + r cos(theta)
   *   y = cy + r sin(theta)
   * is
   *   r*cx*sin(theta)
   * - r*cy*cos(theta)
   * + r^2*theta */
  double contribution = piece->radius * piece->cx * 
      (sin(theta2) - sin(theta1)) +
    piece->radius * piece->cy * (cos(theta1) - cos(theta2)) +
    piece->radius * piece->radius * delta;
  return 0.5 * contribution;
}

/**
 * @brief Compute the signed area of an ordered buffer ring.
 * @details The ring may contain both straight segments and exact circular
 * arcs. A positive value means counter-clockwise traversal and a negative
 * value means clockwise traversal.
 */
static double
buffer_ring_signed_area(const MeosArray *pieces)
{
  assert(pieces);
  double area = 0.0;
  for (uint32_t i = 0; i < pieces->count; i++)
  {
    const BufferPiece *piece = (const BufferPiece *) meos_array_get(pieces, i);
    if (! piece)
      continue;
    if (piece->type == BUFFER_SEGMENT)
      area += buffer_segment_signed_area(piece);
    else if (piece->type == BUFFER_ARC)
      area += buffer_arc_signed_area(piece);
  }
  return area;
}

/**
 * @brief Reverse the traversal direction of a complete boundary ring.
 * @details The piece order is reversed and every individual piece is
 * reversed. The resulting sequence represents exactly the same geometric
 * ring with the opposite orientation.
 */
static MeosArray *
buffer_reverse_ring_pieces(const MeosArray *pieces)
{
  assert(pieces);
  MeosArray *reversed = meos_array_create(sizeof(BufferPiece));
  if (! reversed)
    return NULL;
  for (uint32_t i = pieces->count; i > 0; i--)
  {
    const BufferPiece *source = 
      (const BufferPiece *) meos_array_get(pieces, i - 1);
    if (! source)
    {
      meos_array_destroy(reversed);
      return NULL;
    }
    BufferPiece piece = *source;
    buffer_piece_reverse(&piece);
    meos_array_add(reversed, &piece);
  }
  return reversed;
}

/**
 * @brief Construct a compound curve from an ordered buffer-piece sequence.
 */
static LWCOMPOUND *
buffer_build_ring_from_pieces(const MeosArray *pieces, int32_t srid)
{
  assert(pieces);
  if (pieces->count == 0)
    return NULL;
  LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);
  if (! ring)
    return NULL;
  for (uint32_t i = 0; i < pieces->count; i++)
  {
    const BufferPiece *piece = (const BufferPiece *) meos_array_get(pieces, i);
    if (! piece)
    {
      lwgeom_free(lwcompound_as_lwgeom(ring));
      return NULL;
    }
    buffer_append_piece_to_curve(ring, srid, piece);
  }
  return ring;
}

/**
 * @brief Normalize the orientation of one classified boundary ring.
 * @details Shells are normalized to counter-clockwise traversal and holes
 * to clockwise traversal.
 * If the current orientation already matches the desired orientation,
 * the existing ring and piece sequence are retained.
 */
static bool
buffer_normalize_ring_orientation(BufferRingInfo *info, int32_t srid)
{
  assert(info); assert(info->ring); assert(info->pieces);
  double area =
    buffer_ring_signed_area(info->pieces);
  if (fabs(area) <= MEOS_GEOM_TOLERANCE)
    return false;
  /* Even depth = shell = CCW
   * Odd depth  = hole  = CW */
  bool want_ccw = (info->depth & 1) == 0;
  bool is_ccw = area > 0.0;
  if (is_ccw == want_ccw)
    return true;
  /* Reverse both the ordered pieces and the compound curve */
  MeosArray *reversed = buffer_reverse_ring_pieces(info->pieces);
  if (! reversed)
    return false;
  LWCOMPOUND *ring = buffer_build_ring_from_pieces(reversed, srid);
  if (! ring)
  {
    meos_array_destroy(reversed);
    return false;
  }
  lwgeom_free(lwcompound_as_lwgeom(info->ring));
  meos_array_destroy(info->pieces);
  info->ring = ring;
  info->pieces = reversed;
  /* Reversing a closed ring does not change its interior or its
   * containment relationship, so x/y, parent, depth and shell remain
   * unchanged. */
  return true;
}

/**
 * @brief Normalize the orientation of all classified boundary rings.
 */
static bool
buffer_normalize_ring_orientations(MeosArray *classified, int32_t srid)
{
  assert(classified);
  for (uint32_t i = 0; i < classified->count; i++)
  {
    BufferRingInfo *info = (BufferRingInfo *) meos_array_get(classified, i);
    if (! info)
      return false; 
    if (! buffer_normalize_ring_orientation(info, srid))
      return false;
  }
  return true;
}

/**
 * @brief Construct polygonal surfaces from classified boundary rings.
 * @details The topology stage has already classified every ring as either
 * a shell or a hole and assigned every hole to its immediate shell.
 * This function therefore performs no further geometric reasoning.
 * It simply constructs one CURVEPOLYGON for every shell and attaches the
 * holes belonging directly to that shell.
 * Shells and holes are already oriented by the preceding topology stage.
 */
static LWGEOM *
buffer_build_surfaces_from_classified_rings(MeosArray *classified,
  int32_t srid)
{
  assert(classified);
  uint32_t nshells = 0;
  /* Count the shells */
  for (uint32_t i = 0; i < classified->count; i++)
  {
    const BufferRingInfo *info =
      (const BufferRingInfo *) meos_array_get(classified, i);
    if (! info)
      return NULL;
    if ((info->depth & 1) == 0)
      nshells++;
  }
  /* No shells means that there is no areal result */
  if (nshells == 0)
    return NULL;
  LWGEOM **surfaces = palloc0(sizeof(LWGEOM *) * nshells);
  uint32_t surface_count = 0;
  /* Construct one CURVEPOLYGON for every shell */
  for (uint32_t i = 0; i < classified->count; i++)
  {
    BufferRingInfo *shell = (BufferRingInfo *) meos_array_get(classified, i);
    if (! shell)
      goto fail;
    /* Odd depth means hole */
    if ((shell->depth & 1) != 0)
      continue;
    LWCURVEPOLY *polygon = lwcurvepoly_construct_empty(srid, 0, 0);
    if (! polygon)
      goto fail;
    /* Add the shell. A ring bounds exactly one surface, as a shell or as the
     * hole of one shell, so the surface TAKES the ring rather than copying it
     * and the classification gives it up. #lwgeom_clone() would share the
     * point arrays of the ring rather than copy them, leaving the surface
     * reading what releasing the classification releases */
    LWCOMPOUND *shell_ring = shell->ring;
    if (! shell_ring)
    {
      lwgeom_free(lwcurvepoly_as_lwgeom(polygon));
      goto fail;
    }
    shell->ring = NULL;
    buffer_curvepoly_add_ring(polygon, shell_ring);
    /* Attach only the holes whose immediate shell is this shell */
    for (uint32_t j = 0; j < classified->count; j++)
    {
      BufferRingInfo *hole = (BufferRingInfo *) meos_array_get(classified, j);
      if (! hole)
      {
        lwgeom_free(lwcurvepoly_as_lwgeom(polygon));
        goto fail;
      }
      if ((hole->depth & 1) == 0)
        continue;
      if (hole->shell != (int32_t) i)
        continue;
      LWCOMPOUND *hole_ring = hole->ring;
      if (! hole_ring)
      {
        lwgeom_free(lwcurvepoly_as_lwgeom(polygon));
        goto fail;
      }
      hole->ring = NULL;
      buffer_curvepoly_add_ring(polygon, hole_ring);
    }
    surfaces[surface_count++] = lwcurvepoly_as_lwgeom(polygon);
  }

  /* Exactly one shell gives a single CURVEPOLYGON */
  if (surface_count == 1)
  {
    LWGEOM *result = surfaces[0];
    pfree(surfaces);
    return result;
  }
  /* Several independent shells give a MULTISURFACE. lwcollection_construct()
   * takes ownership of the geometry array, so surfaces must NOT be freed
   * after this call. */
  LWCOLLECTION *collection = lwcollection_construct(MULTISURFACETYPE, srid,
    NULL, surface_count, surfaces);
  if (! collection)
    goto fail;
  return lwcollection_as_lwgeom(collection);

fail:
  for (uint32_t i = 0; i < surface_count; i++)
    lwgeom_free(surfaces[i]);
  pfree(surfaces);
  return NULL;
}

/**
 * @brief Construct polygonal surfaces from closed boundary rings.
 * @details Rings are classified according to their containment depth.
 * Even-depth rings are shells and odd-depth rings are holes.
 * Each hole is assigned to its immediate containing shell.
 */
static LWGEOM *
buffer_build_surfaces_from_rings(MeosArray *rings, int32_t srid)
{
  assert(rings);
  if (rings->count == 0)
  {
    LWCURVEPOLY *empty = lwcurvepoly_construct_empty(srid, 0, 0);
    return lwcurvepoly_as_lwgeom(empty);
  }
  /* The classification array contains one BufferRingInfo for each
   * closed boundary ring, and holds the ring and the pieces of every one it
   * takes from @p rings */
  MeosArray *classified = meos_array_create(sizeof(BufferRingInfo));
  if (! classified)
    return NULL;
  if (! buffer_classify_rings(rings, srid, classified))
  {
    buffer_ring_infos_destroy(classified);
    return NULL;
  }
  /* Normalize shell/hole orientation only after the containment
   * hierarchy has been established */
  if (! buffer_normalize_ring_orientations(classified, srid))
  {
    buffer_ring_infos_destroy(classified);
    return NULL;
  }
  /* The surfaces are built from copies of the rings, so the classification
   * still holds the rings themselves and releases them */
  LWGEOM *result = buffer_build_surfaces_from_classified_rings(classified,
    srid);
  buffer_ring_infos_destroy(classified);
  return result;
}

/**
 * @brief Construct a CURVEPOLYGON from selected boundary pieces.
 * @details The selected pieces may form several disconnected boundary
 * components. Each component is first chained into a closed ring.
 * Shell/hole classification is handled by a later topology stage.
 */
static LWGEOM *
buffer_make_surfaces_from_pieces(const MeosArray *pieces, int32_t srid)
{
  assert(pieces);
  MeosArray *rings = meos_array_create(sizeof(BufferRingInfo));
  if (! rings)
    return NULL;
  if (! buffer_chain_ring_infos(pieces, srid, rings))
  {
    buffer_ring_infos_destroy(rings);
    return NULL;
  }
  LWGEOM *result = buffer_build_surfaces_from_rings(rings, srid);
  /* The classification holds the rings it takes, and what stays here is what
   * it leaves: a ring bounding no area, which it drops */
  buffer_ring_infos_destroy(rings);
  return result;
}

/*****************************************************************************
 * Buffer overlay - topology classification
 *****************************************************************************/

/**
 * @brief Determine whether two buffer boundaries meet at nodes the overlay can
 * split them at
 * @details Two boundaries meet either at isolated POINTS or along a CURVE, and
 * both give the overlay something to work with: a point is a node, and a
 * coincident stretch is bounded by two of them, which
 * #buffer_collect_line_line_intersections() emits from the parameters of the
 * overlap. What follows splits both boundaries at those nodes and classifies
 * each piece, and a piece lying ON the other boundary is resolved by the side
 * each geometry occupies -- opposite sides put the stretch INSIDE the union,
 * so it is dropped, and the two surfaces come out as one.
 * A point-touch without interior overlap answers true here as well; it is the
 * boundary SELECTION that leaves such a pair as two surfaces, because every
 * piece of both boundaries survives it.
 */
static bool
buffer_boundaries_cross(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  MeosArray *a1 = geom_extract_edges(geom1);
  MeosArray *a2 = geom_extract_edges(geom2);
  if (! a1 || ! a2)
  {
    if (a1)
      meos_array_destroy(a1);
    if (a2)
      meos_array_destroy(a2);
    return false;
  }

  bool point_intersection = false;
  for (uint32_t i = 0; i < a1->count && ! point_intersection; i++)
  {
    const Edge *e1 = (const Edge *) meos_array_get(a1, i);
    if (! e1 || ! buffer_is_boundary_edge(e1))
      continue;
    for (uint32_t j = 0; j < a2->count; j++)
    {
      const Edge *e2 = (const Edge *) meos_array_get(a2, j);
      if (! e2 || ! buffer_is_boundary_edge(e2))
        continue;
      int dimension = buffer_boundary_intersection(e1, e2);

      /* A curve the two boundaries share is bounded by two nodes, so it splits
       * like any other meeting and the stretch between them becomes a piece of
       * its own for the classification to place */
      if (dimension >= 0)
        point_intersection = true;
    }
  }

  meos_array_destroy(a1); meos_array_destroy(a2);
  return point_intersection;
}

/*****************************************************************************
 * Buffer overlay - first complete two-buffer union
 *****************************************************************************/

/**
 * @brief Read a set of nodes as the geometry it draws
 * @details One node draws a point and several draw a multipoint, which is the
 * shape #geo_points_covered gives the points it keeps of a point set.
 * @param[in] nodes Nodes, as #POINT2D
 * @param[in] srid Spatial reference identifier the answer carries
 */
static LWGEOM *
buffer_nodes_geometry(const MeosArray *nodes, int32_t srid)
{
  assert(nodes);
  uint32_t count = meos_array_count(nodes);
  if (count == 0)
    return lwpoint_as_lwgeom(lwpoint_construct_empty(srid, 0, 0));
  if (count == 1)
  {
    const POINT2D *p = (const POINT2D *) meos_array_get(nodes, 0);
    return lwpoint_as_lwgeom(lwpoint_make2d(srid, p->x, p->y));
  }
  LWGEOM **points = palloc(sizeof(LWGEOM *) * count);
  for (uint32_t i = 0; i < count; i++)
  {
    const POINT2D *p = (const POINT2D *) meos_array_get(nodes, i);
    points[i] = lwpoint_as_lwgeom(lwpoint_make2d(srid, p->x, p->y));
  }
  /* #lwcollection_construct keeps the array it is given */
  return lwcollection_as_lwgeom(lwcollection_construct(MULTIPOINTTYPE, srid,
    NULL, count, points));
}

/**
 * @brief Answer a Boolean operation on two areal geometries while preserving
 * circular arcs
 * @details One mechanism answers the three operations, and the boundary of the
 * answer is always built the same way: the two boundaries are cut at the nodes
 * where they meet, every piece is placed inside or outside the other geometry
 * by its own midpoint, the operation says which side it keeps
 * (#buffer_select_overlay_boundary), and the kept pieces are chained back into
 * rings. An arc is cut into arcs of its own circle throughout, so the answer
 * carries the circles its operands do rather than the chords a linearization
 * would put in their place.
 *
 * Nothing here asks how the two geometries lie relative to one another. Where
 * their boundaries stay apart there is nothing to cut and every piece is
 * wholly inside or wholly outside, which is the containment and the disjoint
 * case answered by the same selection; and a geometry of several components is
 * read piece by piece, so one component inside and another outside need no
 * separate treatment.
 * @param[in] geom1,geom2 Areal geometries
 * @param[in] oper Operation, one of @p CL_UNION, @p CL_INTERSECTION and
 * @p CL_DIFFERENCE
 * @param[out] touching Set when the two boundaries meet without their
 * interiors overlapping, which a union answers as two surfaces
 * A stretch the two boundaries SHARE is bounded by the nodes at its ends, so
 * it splits into a piece of its own and is placed like any other -- by the
 * side each interior occupies rather than by a midpoint that lies on both.
 * @return The overlay, an EMPTY geometry where it covers nothing, the NODES an
 * intersection meets at where the two share no area, or @p NULL for a pair
 * this does not answer -- a shared piece whose sides do not resolve, a meeting
 * that is not a crossing, and an intersection meeting along a stretch rather
 * than at nodes
 */
static LWGEOM *
buffer_areal_overlay(const LWGEOM *geom1, const LWGEOM *geom2, ClipOper oper,
  bool *touching)
{
  assert(geom1); assert(geom2); assert(touching);
  *touching = false;
  bool crossing = buffer_boundaries_intersect(geom1, geom2);

  /* A point intersection by itself does not imply an overlapping union
   * boundary. In particular, two buffers may merely touch at one point.
   * Such components should remain separate surfaces. */
  if (crossing && ! buffer_boundaries_cross(geom1, geom2))
    return NULL;

  /* Collect the exact intersection nodes. Boundaries that stay apart have
   * none, and the split below then leaves every piece whole */
  MeosArray *intersections = meos_array_create(sizeof(POINT2D));
  if (crossing)
  {
    if (! buffer_collect_boundary_intersections(geom1, geom2, intersections))
    {
      meos_array_destroy(intersections);
      return NULL;
    }

    /* A boundary intersection was reported, but there are no discrete
     * nodes. This indicates a coincident/overlapping-boundary case.
     * Defer it to the next topology layer. */
    if (meos_array_count(intersections) == 0)
    {
      meos_array_destroy(intersections);
      return NULL;
    }
  }

  /* Extract and split both complete boundaries */
  MeosArray *raw_a = meos_array_create(sizeof(BufferPiece));
  MeosArray *raw_b = meos_array_create(sizeof(BufferPiece));
  if (! buffer_pieces_from_geometry(geom1, raw_a) ||
      ! buffer_pieces_from_geometry(geom2, raw_b))
  {
    meos_array_destroy(raw_a); meos_array_destroy(raw_b);
    meos_array_destroy(intersections);
    return NULL;
  }

  MeosArray *split_a = meos_array_create(sizeof(BufferPiece));
  MeosArray *split_b = meos_array_create(sizeof(BufferPiece));
  buffer_split_pieces(raw_a, intersections, split_a);
  buffer_split_pieces(raw_b, intersections, split_b);

  /* Select the portions of both boundaries the operation keeps */
  MeosArray *selected = meos_array_create(sizeof(BufferPiece));
  MeosArray *boundary = meos_array_create(sizeof(BufferPiece));
  bool coincident;
  buffer_select_overlay_boundary(split_a, geom2, split_b, geom1, oper,
    selected, boundary, &coincident);

  /* Two boundaries that meet only at isolated points without their interiors
   * overlapping, as two discs touching at one point do, leave every piece of
   * both boundaries on the union boundary. There is nothing to dissolve, and
   * chaining rings that meet at a single node does not produce a surface, so
   * the pair is left to the caller as two surfaces. Boundaries that never meet
   * keep every piece for a union too, and that is an answer rather than a
   * refusal, so the question is only asked where they do */
  if (crossing && meos_array_count(selected) ==
      meos_array_count(split_a) + meos_array_count(split_b))
  {
    *touching = true;
    meos_array_destroy(selected); meos_array_destroy(boundary);
    meos_array_destroy(split_a); meos_array_destroy(split_b);
    meos_array_destroy(raw_a); meos_array_destroy(raw_b);
    meos_array_destroy(intersections);
    return NULL;
  }

  /* We reject unresolved coincident pieces here */
  if (meos_array_count(boundary) > 0)
  {
    meos_array_destroy(selected); meos_array_destroy(boundary);
    meos_array_destroy(split_a); meos_array_destroy(split_b);
    meos_array_destroy(raw_a); meos_array_destroy(raw_b);
    meos_array_destroy(intersections);
    return NULL;
  }
  /* An operation keeping no piece of either boundary covers no area, and what
   * that means differs between the two operations.
   *
   * A DIFFERENCE keeping nothing says the FIRST geometry is wholly inside the
   * second: no piece of its boundary lies outside, and no piece of the other's
   * lies within. A region inside another leaves nothing behind, and it leaves
   * nothing behind whether or not the two boundaries touch on the way -- what
   * they share is of no area, and a difference of regions does not answer one.
   * So the region of no area IS the answer here.
   *
   * An INTERSECTION keeping nothing does NOT say that. Where the boundaries
   * stay apart the two share nothing and the region of no area is right; but
   * where they MEET, the two share exactly what they meet along -- two discs
   * touching at a point share that point, two surfaces meeting along an edge
   * share the edge. That answer is of lower dimension than the surfaces this
   * assembles, so it is read off the meeting itself.
   *
   * The two meetings are answered differently, and the difference is what
   * separates a point from a curve. Where no piece of either boundary lies ON
   * the other, the two meet at the isolated NODES collected above and those
   * nodes ARE the answer -- exactly, since each is solved on the circles the
   * operands carry. Where a piece does lie on the other, they meet along a
   * whole STRETCH whose ends are the nodes, so the nodes state where the
   * meeting begins and ends and not what it draws; such a pair is left to the
   * caller, as is a union, which reaches this at all only where both
   * boundaries lie wholly within the other */
  int32_t srid = lwgeom_get_srid(geom1);
  if (meos_array_count(selected) == 0 && crossing && oper != CL_DIFFERENCE)
  {
    LWGEOM *meeting = (oper == CL_INTERSECTION && ! coincident) ?
      buffer_nodes_geometry(intersections, srid) : NULL;
    meos_array_destroy(selected); meos_array_destroy(boundary);
    meos_array_destroy(split_a); meos_array_destroy(split_b);
    meos_array_destroy(raw_a); meos_array_destroy(raw_b);
    meos_array_destroy(intersections);
    return meeting;
  }

  /* Reconstruct the complete overlay boundary:
   * selected pieces
   * -> closed rings
   * -> containment hierarchy
   * -> shell/hole classification
   * -> orientation normalization
   * -> CURVEPOLYGON / MULTISURFACE
   */
  LWGEOM *result = meos_array_count(selected) == 0 ?
    lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0)) :
    buffer_make_surfaces_from_pieces(selected, srid);
  /* Clean up and return */
  meos_array_destroy(selected); meos_array_destroy(boundary);
  meos_array_destroy(split_a); meos_array_destroy(split_b);
  meos_array_destroy(raw_a); meos_array_destroy(raw_b);
  meos_array_destroy(intersections);
  return result;
}

/**
 * @brief Union two crossing buffer surfaces while preserving circular arcs.
 * @details The caller asks whether the two surfaces MERGE into one, so only
 * the proper crossing case answers it: a pair whose boundaries stay apart, or
 * one of which contains the other, is left to the caller to assemble.
 */
static LWGEOM *
buffer_union_crossing(const LWGEOM *geom1, const LWGEOM *geom2,
  bool *touching)
{
  assert(geom1); assert(geom2); assert(touching);
  *touching = false;
  if (buffer_components_relation(geom1, geom2) != 1)
    return NULL;
  return buffer_areal_overlay(geom1, geom2, CL_UNION, touching);
}

/*****************************************************************************
 * Polygon buffer
 *****************************************************************************/

/**
 * @brief Return the signed area of a ring.
 * @details A positive value means counter-clockwise orientation and a
 * negative value means clockwise orientation.
 */
static double
buffer_ring_area(const POINTARRAY *pa)
{
  assert(pa);
  if (pa->npoints < 3)
    return 0.0;
  double area = 0.0;
  for (uint32_t i = 0; i < pa->npoints - 1; i++)
  {
    POINT4D p1, p2;
    getPoint4d_p(pa, i, &p1);
    getPoint4d_p(pa, i + 1, &p2);
    area += p1.x * p2.y - p2.x * p1.y;
  }
  return area * 0.5;
}

/**
 * @brief Return the effective outward side of a polygon ring.
 * @details For a counter-clockwise ring the interior is on the left and
 * therefore the exterior is on the right. For a clockwise ring the interior
 * is on the right and therefore the exterior is on the left.
 */
static bool
buffer_ring_outward_left(const POINTARRAY *pa)
{
  assert(pa);
  return buffer_ring_area(pa) < 0.0;
}


/**
 * @brief Construct the offset of a polygon ring
 * @details The offset of every segment is joined to the next one at the
 * vertex between them: on the side the ring turns away from, the two offset
 * segments leave a gap that the requested join style fills, and on the other
 * side they cross and their intersection is the single point the boundary
 * passes through.
 *
 * The ring therefore alternates straight portions with joins, which is a
 * compound curve, exactly as the boundary of a line buffer is. A circular
 * string cannot carry it, because every three consecutive points of one read
 * as an arc, so a straight portion would have to be written as a collinear
 * triple and every join would have to align with that parity.
 * @param[in] source Source polygon ring, explicitly closed
 * @param[in] radius Buffer distance
 * @param[in] outward_left True if the buffered side is left of the ring
 * traversal direction
 * @param[in] join_style Join style
 * @param[in] mitre_limit Maximum mitre ratio
 * @param[in] srid Spatial reference identifier
 * @return The offset ring, or @p NULL if it cannot be constructed
 */
static LWCOMPOUND *
buffer_ring(const POINTARRAY *source, double radius, bool outward_left,
  JoinStyle join_style, double mitre_limit, int32_t srid)
{
  assert(source); assert(radius > 0.0);
  /* The last point of a closed ring repeats the first one and is not a vertex
   * of its own, so four points are what carries the three distinct vertices a
   * ring must have */
  if (source->npoints < 4)
    return NULL;
  uint32_t n = source->npoints - 1;
  POINT2D *points = palloc(sizeof(POINT2D) * n);
  for (uint32_t i = 0; i < n; i++)
  {
    POINT4D p;
    getPoint4d_p(source, i, &p);
    points[i].x = p.x;
    points[i].y = p.y;
  }

  /* The unit normal of every segment, pointing to the buffered side */
  double *nx = palloc(sizeof(double) * n);
  double *ny = palloc(sizeof(double) * n);
  for (uint32_t i = 0; i < n; i++)
  {
    uint32_t next = (i + 1) % n;
    double dx = points[next].x - points[i].x;
    double dy = points[next].y - points[i].y;
    double len = hypot(dx, dy);
    if (len <= MEOS_GEOM_TOLERANCE)
    {
      pfree(points); pfree(nx); pfree(ny);
      return NULL;
    }
    dx /= len;
    dy /= len;
    nx[i] = outward_left ? -dy : dy;
    ny[i] = outward_left ? dx : -dx;
  }

  LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);
  POINT2D first = { 0, 0 }, cursor = { 0, 0 };
  bool started = false;
  for (uint32_t i = 0; i < n; i++)
  {
    uint32_t prev = (i + n - 1) % n;
    uint32_t next = (i + 1) % n;
    /* The boundary reaches the vertex along the offset of the segment ending
     * there and leaves it along the offset of the one starting there */
    POINT2D incoming, outgoing;
    incoming.x = points[i].x + radius * nx[prev];
    incoming.y = points[i].y + radius * ny[prev];
    outgoing.x = points[i].x + radius * nx[i];
    outgoing.y = points[i].y + radius * ny[i];

    double in_dx = points[i].x - points[prev].x;
    double in_dy = points[i].y - points[prev].y;
    double out_dx = points[next].x - points[i].x;
    double out_dy = points[next].y - points[i].y;
    double turn = buffer_cross(in_dx, in_dy, out_dx, out_dy);
    /* The buffered side is convex at the vertex when the ring turns away
     * from it, which is a right turn when that side is the left one */
    bool convex = outward_left ? turn < -MEOS_GEOM_TOLERANCE : turn > MEOS_GEOM_TOLERANCE;

    POINT2D enter = incoming, leave = outgoing;
    bool bridge = false;
    if (! convex)
    {
      /* The two offsets of a vertex the buffered side is concave at cross, and
       * the crossing is the point the boundary passes through — while it lies
       * on both of them. The two offset LINES always cross; a crossing further
       * from the vertex than either segment is long lies beyond an offset's
       * own end, so neither ever reaches it, and the boundary runs through the
       * points at the buffer distance from the vertex instead */
      POINT2D crossing;
      if (! buffer_line_intersection(incoming, in_dx, in_dy, outgoing, out_dx,
          out_dy, &crossing))
        bridge = true;
      else
      {
        double in_len = hypot(in_dx, in_dy), out_len = hypot(out_dx, out_dy);
        double ux = crossing.x - points[i].x, uy = crossing.y - points[i].y;
        double back = -(ux * in_dx + uy * in_dy) / in_len;
        double ahead = (ux * out_dx + uy * out_dy) / out_len;
        if (back > in_len + MEOS_GEOM_TOLERANCE ||
            ahead > out_len + MEOS_GEOM_TOLERANCE)
          bridge = true;
        else
          enter = leave = crossing;
      }
    }

    if (! started)
    {
      first = enter;
      started = true;
    }
    else
    {
      /* The offset of the segment ending at this vertex. An offset running
       * opposite to the segment it comes from has been consumed: the ring is
       * contracted past its own width there and bounds no surface, which the
       * boundary overlay would have to resolve. */
      buffer_add_segment(ring, srid, cursor, enter);
    }
    if (convex || bridge)
      buffer_add_join(ring, srid, points[i], incoming, outgoing, radius,
        join_style, mitre_limit, true);
    cursor = leave;
  }
  /* The offset of the segment closing the ring */
  buffer_add_segment(ring, srid, cursor, first);

  pfree(points); pfree(nx); pfree(ny);
  if (ring->ngeoms == 0)
  {
    lwgeom_free(lwcompound_as_lwgeom(ring));
    return NULL;
  }
  return ring;
}

/*****************************************************************************
 * Buffer - offsetting a chain of edges
 *****************************************************************************/

/**
 * @brief Offset one edge of a boundary
 * @details A straight edge moves along its normal, and a circular arc keeps
 * its centre and its angles and changes only its radius, which is what lets a
 * curved geometry be buffered without stroking it. The left of an arc
 * traversed counterclockwise points to its centre, so the radius decreases
 * there and increases on the other side.
 * @return False if the edge is degenerate, or if the arc is contracted past
 * its own radius and leaves no curve
 */
static bool
buffer_offset_edge(const Edge *edge, double radius, bool left,
  BufferPiece *piece)
{
  assert(edge); assert(piece);
  memset(piece, 0, sizeof(BufferPiece));
  if (edge->etype == EDGE_POLYSEG || edge->etype == EDGE_LINESEG)
  {
    double length = hypot(edge->x2 - edge->x1, edge->y2 - edge->y1);
    if (length <= MEOS_GEOM_TOLERANCE)
      return false;
    double nx = -(edge->y2 - edge->y1) / length;
    double ny =  (edge->x2 - edge->x1) / length;
    if (! left)
    {
      nx = -nx;
      ny = -ny;
    }
    piece->type = BUFFER_SEGMENT;
    piece->x1 = edge->x1 + radius * nx;
    piece->y1 = edge->y1 + radius * ny;
    piece->x2 = edge->x2 + radius * nx;
    piece->y2 = edge->y2 + radius * ny;
    return true;
  }
  if (edge->etype == EDGE_POLYARC || edge->etype == EDGE_LINEARC)
  {
    bool inward = edge->ccw ? left : ! left;
    double r = inward ? edge->radius - radius : edge->radius + radius;
    /* Offsetting into an arc by more than it turns on carries the offset past
     * the centre, where it comes out on the far side at the distance it
     * overshot by and points the other way. That curve is nearer the arc than
     * the buffer distance, so it bounds nothing and #buffer_ring_resolve
     * drops it; it is built rather than refused so that the ring it belongs
     * to is closed and the rest of it can be read */
    double half = 0.0;
    if (r <= MEOS_GEOM_TOLERANCE)
    {
      r = -r;
      half = M_PI;
      if (r <= MEOS_GEOM_TOLERANCE)
      {
        /* Offsetting into an arc by exactly what it turns on carries every
         * point of the offset onto the centre, so the offset IS that point.
         * A point bounds nothing, which is the case above one step further,
         * and it is emitted as a piece of no length for the same reason: the
         * ring stays closed and #buffer_ring_resolve drops what bounds
         * nothing. Refusing it loses a buffer that exists on both sides of
         * this radius */
        piece->type = BUFFER_SEGMENT;
        piece->x1 = piece->x2 = edge->cx;
        piece->y1 = piece->y2 = edge->cy;
        return true;
      }
    }
    piece->type = BUFFER_ARC;
    piece->cx = edge->cx;
    piece->cy = edge->cy;
    piece->radius = r;
    piece->theta1 = edge->theta0 + half;
    piece->theta2 = edge->theta1 + half;
    piece->ccw = edge->ccw;
    piece->x1 = edge->cx + r * cos(piece->theta1);
    piece->y1 = edge->cy + r * sin(piece->theta1);
    piece->x2 = edge->cx + r * cos(piece->theta2);
    piece->y2 = edge->cy + r * sin(piece->theta2);
    return true;
  }
  return false;
}

/**
 * @brief Return the direction an edge runs in at one of its ends
 * @details A direction, so of unit length at both ends of both kinds of edge.
 * The tangent of an arc is a sine and a cosine and is already of unit length;
 * the chord of a straight edge is as long as the edge, and the turn between
 * two directions is read as a cross product, so leaving it unnormalized makes
 * that turn an area where two straight edges meet, a length where a straight
 * edge meets an arc, and a sine where two arcs do. Only the last is what the
 * one tolerance the turn is judged against can bound, so the other two shrink
 * with the geometry and a genuine turn between small edges reads as tangent.
 * @param[in] edge Edge
 * @param[in] at_start True for the direction it leaves its start point in,
 * false for the direction it arrives at its end point in
 * @param[out] dx,dy Direction, of unit length, or zero for an edge of no
 * length, which runs in no direction
 */
static void
buffer_edge_tangent(const Edge *edge, bool at_start, double *dx, double *dy)
{
  assert(edge); assert(dx); assert(dy);
  if (edge->etype == EDGE_POLYARC || edge->etype == EDGE_LINEARC)
  {
    double theta = at_start ? edge->theta0 : edge->theta1;
    double s = sin(theta), c = cos(theta);
    *dx = edge->ccw ? -s : s;
    *dy = edge->ccw ? c : -c;
    return;
  }
  double ex = edge->x2 - edge->x1, ey = edge->y2 - edge->y1;
  double len = hypot(ex, ey);
  if (len <= 0.0)
  {
    *dx = *dy = 0.0;
    return;
  }
  *dx = ex / len;
  *dy = ey / len;
}

/**
 * @brief Return the direction an edge leaves its start point in
 */
static void
buffer_edge_start_tangent(const Edge *edge, double *dx, double *dy)
{
  buffer_edge_tangent(edge, true, dx, dy);
}

/**
 * @brief Return the direction an edge arrives at its end point in
 */
static void
buffer_edge_end_tangent(const Edge *edge, double *dx, double *dy)
{
  buffer_edge_tangent(edge, false, dx, dy);
}

/**
 * @brief Move the end point of an offset piece onto a point of its support
 */
static void
buffer_piece_set_end(BufferPiece *piece, double x, double y)
{
  assert(piece);
  piece->x2 = x;
  piece->y2 = y;
  if (piece->type == BUFFER_ARC)
    piece->theta2 = atan2(y - piece->cy, x - piece->cx);
}

/**
 * @brief Move the start point of an offset piece onto a point of its support
 */
static void
buffer_piece_set_start(BufferPiece *piece, double x, double y)
{
  assert(piece);
  piece->x1 = x;
  piece->y1 = y;
  if (piece->type == BUFFER_ARC)
    piece->theta1 = atan2(y - piece->cy, x - piece->cx);
}

/**
 * @brief Keep the candidate closest to a point
 */
static void
buffer_keep_closest(double x, double y, double px, double py, double *bestx,
  double *besty, double *best, bool *found)
{
  double distance = hypot(x - px, y - py);
  if (! *found || distance < *best)
  {
    *found = true;
    *best = distance;
    *bestx = x;
    *besty = y;
  }
}

/**
 * @brief Return the point where the supports of two offset pieces meet
 * @details On the inner side of a turn the two offset pieces cross, and the
 * boundary passes through the crossing closest to the vertex between them.
 * The supports are a whole line and a whole circle, since the crossing may lie
 * beyond the ends of both pieces, which is what shortens them.
 */
static bool
buffer_pieces_meet(const BufferPiece *a, const BufferPiece *b, double vx,
  double vy, double *x, double *y)
{
  assert(a); assert(b); assert(x); assert(y);
  bool found = false;
  double best = 0.0;

  /* Two straight supports meet in one point */
  if (a->type == BUFFER_SEGMENT && b->type == BUFFER_SEGMENT)
  {
    POINT2D p = { a->x1, a->y1 }, q = { b->x1, b->y1 };
    POINT2D result;
    if (! buffer_line_intersection(p, a->x2 - a->x1, a->y2 - a->y1, q,
        b->x2 - b->x1, b->y2 - b->y1, &result))
      return false;
    *x = result.x;
    *y = result.y;
    return true;
  }

  /* A straight support and a circular one meet in at most two points */
  if (a->type != b->type)
  {
    const BufferPiece *line = a->type == BUFFER_SEGMENT ? a : b;
    const BufferPiece *arc = a->type == BUFFER_SEGMENT ? b : a;
    double dx = line->x2 - line->x1, dy = line->y2 - line->y1;
    double length = hypot(dx, dy);
    if (length <= MEOS_GEOM_TOLERANCE)
      return false;
    dx /= length;
    dy /= length;
    /* The projection of the centre on the line, and the half chord there */
    double t = (arc->cx - line->x1) * dx + (arc->cy - line->y1) * dy;
    double px = line->x1 + t * dx, py = line->y1 + t * dy;
    double gap = hypot(arc->cx - px, arc->cy - py);
    if (gap > arc->radius + MEOS_GEOM_TOLERANCE)
      return false;
    double half = arc->radius * arc->radius - gap * gap;
    half = half > 0.0 ? sqrt(half) : 0.0;
    buffer_keep_closest(px + half * dx, py + half * dy, vx, vy, x, y, &best,
      &found);
    buffer_keep_closest(px - half * dx, py - half * dy, vx, vy, x, y, &best,
      &found);
    return found;
  }

  /* Two circular supports meet on their radical line */
  double dx = b->cx - a->cx, dy = b->cy - a->cy;
  double distance = hypot(dx, dy);
  if (distance <= MEOS_GEOM_TOLERANCE ||
      distance > a->radius + b->radius + MEOS_GEOM_TOLERANCE ||
      distance < fabs(a->radius - b->radius) - MEOS_GEOM_TOLERANCE)
    return false;
  double along = (distance * distance + a->radius * a->radius -
    b->radius * b->radius) / (2 * distance);
  double half = a->radius * a->radius - along * along;
  half = half > 0.0 ? sqrt(half) : 0.0;
  double ux = dx / distance, uy = dy / distance;
  double mx = a->cx + along * ux, my = a->cy + along * uy;
  buffer_keep_closest(mx - half * uy, my + half * ux, vx, vy, x, y, &best,
    &found);
  buffer_keep_closest(mx + half * uy, my - half * ux, vx, vy, x, y, &best,
    &found);
  return found;
}

/**
 * @brief Return whether a point of a piece's support lies on the piece itself
 * @details #buffer_pieces_meet answers where two offsets' SUPPORTS cross — two
 * whole lines, or two whole circles. A crossing bounds the buffer only where
 * it falls on the pieces those supports carry; one landing beyond an end
 * belongs to a part of the support the offset never reaches.
 */
static bool
buffer_piece_holds(const BufferPiece *piece, double x, double y)
{
  assert(piece);
  if (piece->type == BUFFER_ARC)
    return arc_span_contains(piece->theta1, piece->theta2, piece->ccw,
      atan2(y - piece->cy, x - piece->cx));
  double dx = piece->x2 - piece->x1, dy = piece->y2 - piece->y1;
  double length2 = dx * dx + dy * dy;
  if (length2 <= MEOS_GEOM_TOLERANCE)
    return true;
  double t = ((x - piece->x1) * dx + (y - piece->y1) * dy) / length2;
  double tol = MEOS_GEOM_TOLERANCE / sqrt(length2);
  return t >= -tol && t <= 1.0 + tol;
}

/**
 * @brief Offset a chain of edges onto one boundary
 * @details Every edge is offset on the given side, and consecutive offsets are
 * joined at the vertex between them: where the chain turns away from the
 * buffered side the two offsets leave a gap that the join style fills, and
 * where it turns towards it they cross and both are shortened to the crossing.
 * @param[in] edges Edges of the chain
 * @param[in] radius Buffer distance
 * @param[in] left True to offset to the left of the traversal direction
 * @param[in] join_style Join style
 * @param[in] mitre_limit Maximum mitre ratio
 * @param[in] closed True if the last edge of the chain meets the first one
 * @param[in] srid Spatial reference identifier
 * @param[out] curve Curve the offset is appended to
 * @param[out] first,last Points the offset starts and ends at, which a cap
 * joining two offsets must be given rather than recompute, since a point
 * recomputed from its angle lands a rounding step away and leaves the boundary
 * they belong to unclosed
 * @return False if an edge or a junction leaves no curve
 */
static bool
buffer_offset_edges(const MeosArray *edges, double radius, bool left,
  JoinStyle join_style, double mitre_limit, bool closed, int32_t srid,
  LWCOMPOUND *curve, POINT2D *first, POINT2D *last)
{
  assert(edges); assert(curve); assert(radius > 0.0);
  uint32_t count = edges->count;
  if (count == 0)
    return false;
  BufferPiece *pieces = palloc(sizeof(BufferPiece) * count);
  bool *join = palloc0(sizeof(bool) * count);
  for (uint32_t i = 0; i < count; i++)
  {
    const Edge *edge = (const Edge *) meos_array_get(edges, i);
    if (! edge || ! buffer_offset_edge(edge, radius, left, &pieces[i]))
    {
      pfree(pieces); pfree(join);
      return false;
    }
  }

  /* Settle every junction between two consecutive offsets */
  uint32_t njunctions = closed ? count : count - 1;
  for (uint32_t i = 0; i < njunctions; i++)
  {
    uint32_t next = (i + 1) % count;
    const Edge *e1 = (const Edge *) meos_array_get(edges, i);
    const Edge *e2 = (const Edge *) meos_array_get(edges, next);
    double in_dx, in_dy, out_dx, out_dy;
    buffer_edge_end_tangent(e1, &in_dx, &in_dy);
    buffer_edge_start_tangent(e2, &out_dx, &out_dy);
    double turn = buffer_cross(in_dx, in_dy, out_dx, out_dy);
    /* The buffered side is convex at the vertex when the chain turns away
     * from it, which is a right turn when that side is the left one */
    if (left ? turn < -MEOS_GEOM_TOLERANCE : turn > MEOS_GEOM_TOLERANCE)
    {
      join[i] = true;
      continue;
    }
    /* Tangent edges continue into each other */
    if (fabs(turn) <= MEOS_GEOM_TOLERANCE)
      continue;
    double x, y;
    if (! buffer_pieces_meet(&pieces[i], &pieces[next], e1->x2, e1->y2, &x, &y)
        || ! buffer_piece_holds(&pieces[i], x, y)
        || ! buffer_piece_holds(&pieces[next], x, y))
    {
      /* The two offsets settle the junction where they cross, and a crossing
       * of their supports that lies beyond an offset's own end is one neither
       * ever reaches — as is the case of an offset into a turn tighter than
       * the buffer distance, which parts from its neighbour instead. What
       * spans the two is then the points at the buffer distance from the
       * vertex they turn about, which is the join the other side of a turn is
       * given */
      join[i] = true;
      continue;
    }
    buffer_piece_set_end(&pieces[i], x, y);
    buffer_piece_set_start(&pieces[next], x, y);
  }

  /* Emit the offsets and the joins between them */
  for (uint32_t i = 0; i < count; i++)
  {
    buffer_append_piece_to_curve(curve, srid, &pieces[i]);
    if (! join[i])
      continue;
    uint32_t next = (i + 1) % count;
    const Edge *edge = (const Edge *) meos_array_get(edges, i);
    POINT2D vertex = { edge->x2, edge->y2 };
    POINT2D p1 = { pieces[i].x2, pieces[i].y2 };
    POINT2D p2 = { pieces[next].x1, pieces[next].y1 };
    buffer_add_join(curve, srid, vertex, p1, p2, radius, join_style,
      mitre_limit, true);
  }
  if (first)
  {
    first->x = pieces[0].x1;
    first->y = pieces[0].y1;
  }
  if (last)
  {
    last->x = pieces[count - 1].x2;
    last->y = pieces[count - 1].y2;
  }
  pfree(pieces); pfree(join);
  return true;
}

/**
 * @brief Reverse the direction an edge is traversed in
 */
static void
buffer_edge_reverse(Edge *edge)
{
  assert(edge);
  double tmp = edge->x1;
  edge->x1 = edge->x2;
  edge->x2 = tmp;
  tmp = edge->y1;
  edge->y1 = edge->y2;
  edge->y2 = tmp;
  if (edge->etype == EDGE_POLYARC || edge->etype == EDGE_LINEARC)
  {
    tmp = edge->theta0;
    edge->theta0 = edge->theta1;
    edge->theta1 = tmp;
    edge->ccw = ! edge->ccw;
  }
  else
  {
    edge->dx = -edge->dx;
    edge->dy = -edge->dy;
  }
}

/**
 * @brief Return the edges of a chain traversed backwards
 */
static MeosArray *
buffer_edges_reversed(const MeosArray *edges)
{
  assert(edges);
  MeosArray *result = meos_array_create(sizeof(Edge));
  if (! result)
    return NULL;
  for (uint32_t i = edges->count; i > 0; i--)
  {
    const Edge *edge = (const Edge *) meos_array_get(edges, i - 1);
    if (! edge)
      continue;
    Edge reversed = *edge;
    buffer_edge_reverse(&reversed);
    meos_array_add(result, &reversed);
  }
  return result;
}

/**
 * @brief Return true if the exterior of a ring given as edges lies left of the
 * direction it is traversed in
 */
static bool
buffer_ring_edges_outward_left(const MeosArray *edges)
{
  assert(edges);
  MeosArray *pieces = meos_array_create(sizeof(BufferPiece));
  if (! pieces)
    return false;
  for (uint32_t i = 0; i < edges->count; i++)
  {
    const Edge *edge = (const Edge *) meos_array_get(edges, i);
    if (! edge)
      continue;
    BufferPiece piece;
    buffer_piece_from_edge(edge, &piece);
    meos_array_add(pieces, &piece);
  }
  /* A ring traversed clockwise, whose area is negative, has its exterior on
   * its left */
  double area = buffer_ring_signed_area(pieces);
  meos_array_destroy(pieces);
  return area < 0.0;
}

/*****************************************************************************
 * Buffer overlay - resolving an offset that runs into itself
 * The boundary of a buffer is the set of points at exactly the buffer
 * distance from the geometry. Offsetting each edge gives a curve every point
 * of which is that far from the edge it came from, which is not the same
 * thing: where the geometry turns tighter than the distance, the offset runs
 * past itself and encloses points nearer the geometry than that. Those parts
 * belong to no boundary and are dropped, which is what lets a curve be
 * buffered by more than the radius it turns on
 *****************************************************************************/

/**
 * @brief Collect the points at which two edges meet
 */
static void
buffer_collect_edge_intersections(const Edge *e1, const Edge *e2,
  MeosArray *points)
{
  assert(e1); assert(e2); assert(points);
  bool arc1 = e1->etype == EDGE_POLYARC || e1->etype == EDGE_LINEARC;
  bool arc2 = e2->etype == EDGE_POLYARC || e2->etype == EDGE_LINEARC;
  if (! arc1 && ! arc2)
    buffer_collect_line_line_intersections(e1, e2, points);
  else if (! arc1)
    buffer_collect_line_arc_intersections(e1, e2, points);
  else if (! arc2)
    buffer_collect_line_arc_intersections(e2, e1, points);
  else
    buffer_collect_arc_arc_intersections(e1, e2, points);
  return;
}

/**
 * @brief Return the distance from a point to an edge
 */
static double
buffer_point_edge_distance(double x, double y, const Edge *e)
{
  assert(e);
  double result = Min(hypot(x - e->x1, y - e->y1), hypot(x - e->x2, y - e->y2));
  if (e->etype == EDGE_POLYARC || e->etype == EDGE_LINEARC)
  {
    /* The nearest point of an arc is the one its span holds in the direction
     * of the point, and an end of it otherwise */
    if (arc_contains_angle(e, atan2(y - e->cy, x - e->cx)))
      result = Min(result, fabs(hypot(x - e->cx, y - e->cy) - e->radius));
    return result;
  }
  double dx = e->x2 - e->x1, dy = e->y2 - e->y1;
  double len2 = dx * dx + dy * dy;
  if (len2 > 0)
  {
    double t = ((x - e->x1) * dx + (y - e->y1) * dy) / len2;
    if (t > 0 && t < 1)
      result = Min(result, fabs((x - e->x1) * dy - (y - e->y1) * dx) /
        sqrt(len2));
  }
  return result;
}

/**
 * @brief Return the distance from a point to the nearest edge of a geometry
 */
static double
buffer_point_edges_distance(double x, double y, const MeosArray *edges)
{
  assert(edges);
  double result = DBL_MAX;
  for (uint32_t i = 0; i < edges->count; i++)
  {
    const Edge *e = (const Edge *) meos_array_get(edges, i);
    if (! e || e->etype == EDGE_POINT)
      continue;
    double d = buffer_point_edge_distance(x, y, e);
    if (d < result)
      result = d;
  }
  return result;
}

/**
 * @brief Return the boundary of the buffer of a geometry, given a ring of
 * offsets that may run into itself
 * @details Every piece of the ring is split where it meets any other piece of
 * it, which leaves pieces that are wholly on one side of the question, and a
 * piece is kept when the geometry is the buffer distance away from it rather
 * than nearer. The pieces kept chain into the rings of the answer
 * @param[in] raw Ring of offsets, as a geometry bounding a surface
 * @param[in] edges Edges of the geometry the buffer is taken of
 * @param[in] radius Buffer distance
 * @param[in] srid SRID of the answer
 * @return @p NULL when the pieces kept do not chain into a closed ring
 */
static LWGEOM *
buffer_ring_resolve(const LWGEOM *raw, const MeosArray *edges, double radius,
  int32_t srid)
{
  assert(raw); assert(edges); assert(radius > 0.0);
  MeosArray *arr = geom_extract_edges(raw);
  if (! arr)
    return NULL;
  uint32_t n = meos_array_count(arr);

  /* Where the ring meets itself */
  MeosArray *nodes = meos_array_create(sizeof(POINT2D));
  MeosArray *pieces = meos_array_create(sizeof(BufferPiece));
  for (uint32_t i = 0; i < n; i++)
  {
    const Edge *e1 = (const Edge *) meos_array_get(arr, i);
    if (! e1 || ! buffer_is_boundary_edge(e1))
      continue;
    BufferPiece piece;
    buffer_piece_from_edge(e1, &piece);
    meos_array_add(pieces, &piece);
    for (uint32_t j = i + 1; j < n; j++)
    {
      const Edge *e2 = (const Edge *) meos_array_get(arr, j);
      if (! e2 || ! buffer_is_boundary_edge(e2))
        continue;
      MeosArray *points = meos_array_create(sizeof(POINT2D));
      buffer_collect_edge_intersections(e1, e2, points);
      for (int k = 0; k < meos_array_count(points); k++)
      {
        const POINT2D *p = (const POINT2D *) meos_array_get(points,
          (uint32_t) k);
        if (p)
          buffer_intersections_add(nodes, p->x, p->y);
      }
      meos_array_destroy(points);
    }
  }
  meos_array_destroy(arr);

  MeosArray *split = meos_array_create(sizeof(BufferPiece));
  buffer_split_pieces(pieces, nodes, split);

  /* A piece the geometry comes nearer to than the buffer distance lies inside
   * the buffer rather than on its boundary. The distance is read at the
   * middle of the piece, which the splitting above leaves wholly on one side
   * of the question */
  double tol = Max(MEOS_GEOM_TOLERANCE, radius * 1.0e-9);
  MeosArray *keep = meos_array_create(sizeof(BufferPiece));
  for (int i = 0; i < meos_array_count(split); i++)
  {
    BufferPiece *piece = (BufferPiece *) meos_array_get(split, (uint32_t) i);
    POINT2D mid;
    if (! piece || ! buffer_piece_midpoint(piece, &mid))
      continue;
    /* The midpoint carries the magnitude the rounding is proportional to. A
     * piece kept here lies AT the buffer distance, and the distance deciding
     * it is read from the coordinates, so what it is rounded to is the size of
     * THEIR last bits and not of the radius. At a projected 6.4e6 a piece
     * lying exactly on the offset reads about 1e-09 below the radius, past a
     * bound of `radius * 1e-9` calibrated to a radius of 1, and the piece is
     * dropped: the ring it belongs to then reaches a point nothing continues */
    double mid_tol = Max(tol, coordinate_tolerance(mid.x, mid.y));
    if (buffer_point_edges_distance(mid.x, mid.y, edges) >= radius - mid_tol)
      meos_array_add(keep, piece);
  }

  LWGEOM *result = (meos_array_count(keep) > 0) ?
    buffer_make_surfaces_from_pieces(keep, srid) : NULL;
  meos_array_destroy(keep); meos_array_destroy(split);
  meos_array_destroy(pieces); meos_array_destroy(nodes);
  return result;
}

/**
 * @brief Return the boundary of a buffer whose ring of offsets runs into
 * itself, resolved, and the ring itself when it does not
 * @details Where a geometry turns tighter than the buffer distance its
 * offsets cross, and the loop the crossing leaves lies inside the buffer
 * rather than on its boundary. Naming what the crossing leaves is what
 * #buffer_ring_resolve does, so a ring meeting itself is resolved rather than
 * reported as not supported
 * @param[in] raw Ring of offsets, released here
 * @param[in] input Geometry the buffer is taken of
 * @param[in] radius Buffer distance
 * @param[in] srid SRID of the answer
 */
static LWGEOM *
buffer_ring_resolved(LWGEOM *raw, const LWGEOM *input, double radius,
  int32_t srid)
{
  assert(raw); assert(input);
  if (! buffer_boundary_self_intersects(raw))
    return raw;
  MeosArray *edges = geom_extract_edges(input);
  LWGEOM *result = edges ? buffer_ring_resolve(raw, edges, radius, srid) : NULL;
  if (edges)
    meos_array_destroy(edges);
  lwgeom_free(raw);
  return result;
}

/**
 * @brief Return whether a contracted ring has passed through itself
 * @details Contracting a ring by more than it encloses carries the contraction
 * through itself, and it comes back out inverted at the distance it overshot
 * by. That curve is nearer the input than the buffer distance, so it bounds
 * nothing and the hole it would stand for is gone rather than uncovered. The
 * point the ring holds is what says which of the two it is: an interior point
 * of a true hole lies further from the input than the distance, and an
 * interior point of the inverted curve lies nearer.
 * @param[in] ring Contracted ring
 * @param[in] edges Edges of the geometry being buffered
 * @param[in] radius Buffer distance
 * @param[in] srid Spatial reference identifier
 */
static bool
buffer_ring_inverted(LWCOMPOUND *ring, const MeosArray *edges, double radius,
  int32_t srid)
{
  double hx, hy;
  if (! ring || ! edges)
    return false;
  if (! buffer_ring_representative_point(ring, srid, &hx, &hy))
    return false;
  return buffer_point_edges_distance(hx, hy, edges) <
    radius - MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Buffer a curve, which is a circular string or a compound curve
 * @details The boundary walks the offset of the curve on its left, caps the
 * far end, walks the offset of the reversed curve, which is its right, and
 * caps the near end, exactly as the boundary of a line buffer does. A curve
 * closing on itself has no end to cap and bounds a band instead, whose outer
 * boundary is the surface and whose inner one is its hole.
 */
static LWGEOM *
meos_buffer_curve(const LWGEOM *geom, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(geom); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid(geom);
  MeosArray *edges = geom_extract_edges(geom);
  if (! edges || edges->count == 0)
  {
    if (edges)
      meos_array_destroy(edges);
    return NULL;
  }
  const Edge *first = (const Edge *) meos_array_get(edges, 0);
  const Edge *last = (const Edge *) meos_array_get(edges, edges->count - 1);
  bool closed = buffer_nodes_equal(first->x1, first->y1, last->x2, last->y2);

  if (closed)
  {
    /* The ring bounds the surface on the side its exterior lies, which the
     * direction it is traversed in decides */
    bool outward = buffer_ring_edges_outward_left(edges);
    LWCOMPOUND *outer = lwcompound_construct_empty(srid, 0, 0);
    LWCOMPOUND *inner = lwcompound_construct_empty(srid, 0, 0);
    bool ok = buffer_offset_edges(edges, radius, outward, join_style,
      mitre_limit, true, srid, outer, NULL, NULL);
    bool has_hole = ok && buffer_offset_edges(edges, radius, ! outward,
      join_style, mitre_limit, true, srid, inner, NULL, NULL);
    /* A closed curve contracts into itself the same way a closed line does */
    if (has_hole && buffer_ring_inverted(inner, edges, radius, srid))
      has_hole = false;
    meos_array_destroy(edges);
    if (! ok)
    {
      lwgeom_free(lwcompound_as_lwgeom(outer));
      lwgeom_free(lwcompound_as_lwgeom(inner));
      return NULL;
    }
    LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
    buffer_curvepoly_add_ring(result, outer);
    if (has_hole)
      buffer_curvepoly_add_ring(result, inner);
    else
      lwgeom_free(lwcompound_as_lwgeom(inner));
    return lwcurvepoly_as_lwgeom(result);
  }

  /* An open curve: the two offsets and the caps between them */
  MeosArray *backwards = buffer_edges_reversed(edges);
  LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);
  LWCOMPOUND *back_curve = lwcompound_construct_empty(srid, 0, 0);
  POINT2D left_first, left_last, right_first, right_last;
  double start_dx, start_dy, end_dx, end_dy;
  buffer_edge_start_tangent(first, &start_dx, &start_dy);
  buffer_edge_end_tangent(last, &end_dx, &end_dy);
  POINT2D far_end = { last->x2, last->y2 };
  POINT2D near_end = { first->x1, first->y1 };
  bool ok = backwards &&
    buffer_offset_edges(edges, radius, true, join_style, mitre_limit, false,
      srid, ring, &left_first, &left_last) &&
    buffer_offset_edges(backwards, radius, true, join_style, mitre_limit,
      false, srid, back_curve, &right_first, &right_last);
  meos_array_destroy(edges);
  if (backwards)
    meos_array_destroy(backwards);
  if (! ok)
  {
    lwgeom_free(lwcompound_as_lwgeom(ring));
    lwgeom_free(lwcompound_as_lwgeom(back_curve));
    return NULL;
  }

  /* The cap at the far end, then the offset of the reversed curve, then the
   * cap at the near end */
  if (cap_style == ENDCAP_ROUND)
    buffer_add_round_cap(ring, srid, far_end, left_last, right_first, radius,
      false);
  else
  {
    POINT2D l = left_last, r = right_first;
    if (cap_style == ENDCAP_SQUARE)
    {
      double length = hypot(end_dx, end_dy);
      l.x += end_dx / length * radius; l.y += end_dy / length * radius;
      r.x += end_dx / length * radius; r.y += end_dy / length * radius;
    }
    buffer_add_segment(ring, srid, l, r);
  }
  for (uint32_t i = 0; i < back_curve->ngeoms; i++)
    lwcompound_add_lwgeom(ring, back_curve->geoms[i]);
  /* The pieces belong to the ring now, so only the shell is released */
  lwfree(back_curve->geoms);
  lwfree(back_curve);
  if (cap_style == ENDCAP_ROUND)
    buffer_add_round_cap(ring, srid, near_end, right_last, left_first, radius,
      false);
  else
  {
    POINT2D l = left_first, r = right_last;
    if (cap_style == ENDCAP_SQUARE)
    {
      double length = hypot(start_dx, start_dy);
      l.x -= start_dx / length * radius; l.y -= start_dy / length * radius;
      r.x -= start_dx / length * radius; r.y -= start_dy / length * radius;
    }
    buffer_add_segment(ring, srid, r, l);
  }

  LWCURVEPOLY *curvepoly = lwcurvepoly_construct_empty(srid, 0, 0);
  buffer_curvepoly_add_ring(curvepoly, ring);
  LWGEOM *result = lwcurvepoly_as_lwgeom(curvepoly);
  return buffer_ring_resolved(result, geom, radius, srid);
}

/**
 * @brief Buffer a CURVEPOLYGON
 * @details Its rings are offset the way those of a polygon are, the exterior
 * one away from the surface and the holes into it, except that a ring holding
 * circular arcs is offset arc by arc rather than vertex by vertex.
 */
static LWGEOM *
meos_buffer_curvepoly(const LWCURVEPOLY *curvepoly, double radius,
  JoinStyle join_style, double mitre_limit, bool inward)
{
  assert(curvepoly); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) curvepoly);
  if (curvepoly->nrings == 0)
    return lwcollection_as_lwgeom(lwcollection_construct_empty(
      MULTISURFACETYPE, srid, 0, 0));
  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  for (uint32_t i = 0; i < curvepoly->nrings; i++)
  {
    MeosArray *edges = geom_extract_edges(curvepoly->rings[i]);
    if (! edges || edges->count == 0)
    {
      if (edges)
        meos_array_destroy(edges);
      lwgeom_free(lwcurvepoly_as_lwgeom(result));
      return NULL;
    }
    /* The exterior ring is buffered away from the surface and a hole into it,
     * and an erosion reverses both */
    bool outward = buffer_ring_edges_outward_left(edges);
    bool left = (i == 0) ? outward : ! outward;
    if (inward)
      left = ! left;
    LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);
    bool ok = buffer_offset_edges(edges, radius, left, join_style, mitre_limit,
      true, srid, ring, NULL, NULL);
    meos_array_destroy(edges);
    if (! ok)
    {
      lwgeom_free(lwcompound_as_lwgeom(ring));
      lwgeom_free(lwcurvepoly_as_lwgeom(result));
      return NULL;
    }
    buffer_curvepoly_add_ring(result, ring);
  }
  LWGEOM *geom = lwcurvepoly_as_lwgeom(result);
  return buffer_ring_resolved(geom, lwcurvepoly_as_lwgeom(
    (LWCURVEPOLY *) curvepoly), radius, srid);
}

/*****************************************************************************
 * Buffer - combining the component buffers
 *****************************************************************************/

/**
 * @brief Merge the buffers of the components of a geometry into their union
 * @details The buffers of two components overlap whenever the components lie
 * closer to each other than twice the buffer distance, while the surfaces of a
 * MULTISURFACE have disjoint interiors, so an overlapping pair is dissolved
 * into one surface. Every buffer is merged into the accumulated surface it
 * meets, and the enlarged surface may in turn meet one put aside earlier, so
 * the scan restarts on it until nothing more merges.
 * @param[in] buffers Component buffers, owned by this function
 * @param[in] count Number of component buffers, at least one
 * @param[in] srid Spatial reference identifier
 * @return The union, or @p NULL when the boundary overlay does not cover the
 * topology of one of the pairs. The array itself belongs to the caller.
 */
static LWGEOM *
buffer_union_components(LWGEOM **buffers, uint32_t count, int32_t srid)
{
  assert(buffers); assert(count > 0);
  LWGEOM **merged = palloc(sizeof(LWGEOM *) * count);
  uint32_t nmerged = 0;
  for (uint32_t i = 0; i < count; i++)
  {
    LWGEOM *current = buffers[i];
    bool again = true;
    while (again)
    {
      again = false;
      for (uint32_t j = 0; j < nmerged; j++)
      {
        /* Disjoint surfaces stay apart */
        if (buffer_components_relation(current, merged[j]) == 0)
          continue;
        bool touching = false;
        LWGEOM *both = buffer_areal_union_simple(current, merged[j],
          &touching);
        if (! both)
        {
          /* Their boundaries meet but their interiors do not, as those of two
           * discs touching at one point, and a MULTISURFACE only requires
           * disjoint interiors, so they stay apart */
          if (touching)
            continue;
          /* The overlay does not cover the topology of this pair */
          lwgeom_free(current);
          for (uint32_t k = 0; k < nmerged; k++)
            lwgeom_free(merged[k]);
          for (uint32_t k = i + 1; k < count; k++)
            lwgeom_free(buffers[k]);
          pfree(merged);
          return NULL;
        }
        lwgeom_free(current); lwgeom_free(merged[j]);
        current = both;
        /* The last surface takes the place of the one just consumed */
        merged[j] = merged[--nmerged];
        again = true;
        break;
      }
    }
    merged[nmerged++] = current;
  }

  /* One surface needs no collection wrapper */
  if (nmerged == 1)
  {
    LWGEOM *result = merged[0];
    pfree(merged);
    return result;
  }
  /* lwcollection_construct() takes ownership of the geometry array, so merged
   * must NOT be freed after this call */
  LWCOLLECTION *result = lwcollection_construct(MULTISURFACETYPE, srid, NULL,
    nmerged, merged);
  return lwcollection_as_lwgeom(result);
}

/**
 * @brief Return whether the overlay reads a geometry as the surfaces it draws
 * @details The engine walks a boundary that bounds area, so what it reads is a
 * surface or a collection of surfaces and nothing besides: a member of any
 * other kind draws something the boundary walk has no place for
 */
static bool
buffer_is_areal_geometry(const LWGEOM *geom)
{
  assert(geom);
  uint8_t type = geom->type;
  if (type == POLYGONTYPE || type == CURVEPOLYTYPE || type == TRIANGLETYPE)
    return true;
  if (type != MULTIPOLYGONTYPE && type != MULTISURFACETYPE &&
      type != COLLECTIONTYPE)
    return false;
  const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
  if (coll->ngeoms == 0)
    return false;
  for (uint32_t i = 0; i < coll->ngeoms; i++)
  {
    uint8_t comp = coll->geoms[i]->type;
    if (comp != POLYGONTYPE && comp != CURVEPOLYTYPE && comp != TRIANGLETYPE)
      return false;
  }
  return true;
}

/**
 * @brief Return a triangle as the polygon it draws
 * @details A triangle is a polygon of a single ring, so the two draw the same
 * region -- but the readers of a collection of surfaces do not take them
 * alike. @c lwmsurface_linearize (@c postgis/liblwgeom/lwstroke.c) answers a
 * CURVEPOLYGON and a POLYGON and has no arm for anything else, so a member of
 * another type leaves its slot of the output array UNSET, and the collection
 * built from that array reads whatever the allocation happened to hold.
 * Writing the ring into a polygon loses nothing and keeps every member a type
 * the readers answer
 */
static LWGEOM *
buffer_triangle_as_poly(const LWTRIANGLE *tri)
{
  assert(tri);
  POINTARRAY **rings = lwalloc(sizeof(POINTARRAY *));
  rings[0] = ptarray_clone_deep(tri->points);
  return lwpoly_as_lwgeom(lwpoly_construct(tri->srid, NULL, 1, rings));
}

/**
 * @brief Return the union of the areal components of a geometry
 * @details The components are dissolved into the surfaces they cover: a pair
 * whose interiors meet becomes one surface, and a pair that only touches stays
 * apart, which is what a multisurface asks of its components. The answer
 * covers the same region the geometry does, read as the fewest surfaces it
 * takes. It is the dissolve the buffer of a geometry of several components
 * already performs, answered for a geometry a caller brings
 * @param[in] geom Geometry
 * @return The union, or @p NULL when the geometry carries something that is
 * not a surface, or when the boundary overlay does not cover the topology of
 * one of the pairs -- a caller that has another way to answer may take it
 */
LWGEOM *
meos_areal_union(const LWGEOM *geom)
{
  assert(geom);
  if (lwgeom_is_empty(geom))
    return NULL;

  /* A point or a line has no area to dissolve, and a geometry carrying one is
   * not what this answers */
  if (! buffer_is_areal_geometry(geom))
    return NULL;

  /* One surface is its own union */
  uint8_t type = geom->type;
  if (type == POLYGONTYPE || type == CURVEPOLYTYPE || type == TRIANGLETYPE)
    return lwgeom_clone_deep(geom);

  const LWCOLLECTION *coll = (const LWCOLLECTION *) geom;
  if (coll->ngeoms == 1)
    return lwgeom_clone_deep(coll->geoms[0]);

  /* #buffer_union_components() owns the geometries it is given */
  LWGEOM **surfaces = palloc(sizeof(LWGEOM *) * coll->ngeoms);
  for (uint32_t i = 0; i < coll->ngeoms; i++)
    surfaces[i] = lwgeom_clone_deep(coll->geoms[i]);
  LWGEOM *result = buffer_union_components(surfaces, coll->ngeoms,
    lwgeom_get_srid(geom));
  pfree(surfaces);
  if (! result)
    return NULL;

  /* The collection carries the type its members do: a multisurface is what a
   * curved surface asks for, while a set of polygons is a multipolygon.
   * ⛔ The test is the TYPE, not #lwgeom_is_collection(), which answers true
   * for a CURVEPOLYGON and a COMPOUNDCURVE as well -- their rings and pieces
   * are sub-geometries -- so reading THEIR members as surfaces turns a curve
   * polygon into a multisurface holding its own boundary ring */
  if (result->type == MULTIPOLYGONTYPE || result->type == MULTISURFACETYPE ||
      result->type == COLLECTIONTYPE)
  {
    LWCOLLECTION *res_coll = (LWCOLLECTION *) result;
    uint8_t collected = MULTIPOLYGONTYPE;
    for (uint32_t i = 0; i < res_coll->ngeoms; i++)
    {
      uint8_t comp = res_coll->geoms[i]->type;
      if (comp == POLYGONTYPE)
        continue;
      if (comp != CURVEPOLYTYPE && comp != TRIANGLETYPE)
      {
        /* A member that is not a surface is not an answer this gives */
        lwgeom_free(result);
        return NULL;
      }
      /* A TRIANGLE draws a region a POLYGON draws, and only the polygon is a
       * type every reader of the collection answers, so the member is written
       * as one and the collection stays a multipolygon */
      if (comp == TRIANGLETYPE)
      {
        LWGEOM *poly = buffer_triangle_as_poly(
          (const LWTRIANGLE *) res_coll->geoms[i]);
        lwgeom_free(res_coll->geoms[i]);
        res_coll->geoms[i] = poly;
        continue;
      }
      collected = MULTISURFACETYPE;
    }
    res_coll->type = collected;
  }
  else if (result->type != POLYGONTYPE && result->type != CURVEPOLYTYPE &&
    result->type != TRIANGLETYPE)
  {
    lwgeom_free(result);
    return NULL;
  }

  /* The boundary is welded out of pieces, so a surface comes back as a curve
   * polygon even where every piece of it is a segment. A caller that gave
   * polygons is answered with polygons: the conversion reads the pieces of a
   * ring that carries no arc into one point array and loses nothing */
  if (! lwgeom_has_arc(result))
  {
    LWGEOM *straight = lwgeom_stroke(result, 0);
    if (straight)
    {
      lwgeom_free(result);
      result = straight;
    }
  }
  return result;
}

/**
 * @brief Answer a Boolean operation on two areal geometries
 * @details The operands are read as the surfaces they draw and the answer is
 * built from their two boundaries, so an operand bounded by circular arcs is
 * answered on those circles. An operand carrying anything that is not a
 * surface is not one this reads, and neither is a pair whose boundaries run
 * along one another rather than crossing
 * @param[in] geom1,geom2 Geometries
 * @param[in] oper Operation
 * @return The overlay, an EMPTY geometry where the operation covers nothing,
 * the point set an intersection meets at where the two share no area, or
 * @p NULL for a pair this does not answer -- a caller that has another way to
 * answer may take it
 */
static LWGEOM *
buffer_areal_operation(const LWGEOM *geom1, const LWGEOM *geom2,
  ClipOper oper)
{
  assert(geom1); assert(geom2);
  int32_t srid = lwgeom_get_srid(geom1);
  /* What an empty operand answers is the set identity of the operation:
   * nothing meets an empty region, and an empty region takes nothing from a
   * subject while leaving nothing of one */
  bool empty1 = lwgeom_is_empty(geom1);
  bool empty2 = lwgeom_is_empty(geom2);
  if (empty1 || empty2)
  {
    if (oper == CL_DIFFERENCE && ! empty1)
      return lwgeom_clone_deep(geom1);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0));
  }
  if (! buffer_is_areal_geometry(geom1) || ! buffer_is_areal_geometry(geom2))
    return NULL;

  bool touching;
  LWGEOM *result = buffer_areal_overlay(geom1, geom2, oper, &touching);
  if (! result)
    return NULL;

  /* The boundary is welded out of pieces, so a surface comes back as a curve
   * polygon even where every piece of it is a segment. A pair of operands
   * carrying no arc is answered with polygons: the conversion reads the pieces
   * of a ring that carries no arc into one point array and loses nothing */
  if (! lwgeom_has_arc(result))
  {
    LWGEOM *straight = lwgeom_stroke(result, 0);
    if (straight)
    {
      lwgeom_free(result);
      result = straight;
    }
  }
  return result;
}

/**
 * @brief Return the intersection of two areal geometries
 * @details See #buffer_areal_operation()
 * @param[in] geom1,geom2 Geometries
 * @return The region both cover, an EMPTY geometry where they cover none in
 * common, or @p NULL for a pair this does not answer
 */
LWGEOM *
meos_areal_intersection(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  return buffer_areal_operation(geom1, geom2, CL_INTERSECTION);
}

/**
 * @brief Return the difference of two areal geometries
 * @details See #buffer_areal_operation()
 * @param[in] geom1,geom2 Geometries
 * @return The region the first covers and the second does not, an EMPTY
 * geometry where the second covers all of the first, or @p NULL for a pair
 * this does not answer
 */
LWGEOM *
meos_areal_difference(const LWGEOM *geom1, const LWGEOM *geom2)
{
  assert(geom1); assert(geom2);
  return buffer_areal_operation(geom1, geom2, CL_DIFFERENCE);
}

/*****************************************************************************
 * Buffer - POINT
 *****************************************************************************/

/**
 * @brief Construct a circle buffer around a POINT
 */
static LWGEOM *
meos_buffer_point(const LWPOINT *point, double radius)
{
  assert(point); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) point);
  POINT4D pt;
  lwpoint_getPoint4d_p(point, &pt);
  return lwcircle_make(pt.x, pt.y, radius, srid);
}

/*****************************************************************************
 * Buffer - MULTIPOINT
 *****************************************************************************/

/**
 * @brief Buffer a MULTIPOINT
 * @details Every point gives the disc of the buffer distance around it, and
 * two discs merge whenever their points lie closer than twice that distance.
 */
static LWGEOM *
meos_buffer_mpoint(const LWMPOINT *mpoint, double radius)
{
  assert(mpoint); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) mpoint);
  uint32_t ngeoms = mpoint->ngeoms;
  if (ngeoms == 0)
    return lwcollection_as_lwgeom(lwcollection_construct_empty(
      MULTISURFACETYPE, srid, 0, 0));

  LWGEOM **buffers = palloc(sizeof(LWGEOM *) * ngeoms);
  uint32_t count = 0;
  for (uint32_t i = 0; i < ngeoms; i++)
  {
    const LWGEOM *component = (const LWGEOM *) mpoint->geoms[i];
    if (! component || lwgeom_is_empty(component) ||
        component->type != POINTTYPE)
      continue;
    LWGEOM *buffer = meos_buffer_point((const LWPOINT *) component, radius);
    if (buffer)
      buffers[count++] = buffer;
  }
  if (count == 0)
  {
    pfree(buffers);
    return lwcollection_as_lwgeom(lwcollection_construct_empty(
      MULTISURFACETYPE, srid, 0, 0));
  }

  /* Dissolve the discs that overlap */
  LWGEOM *result = buffer_union_components(buffers, count, srid);
  pfree(buffers);
  return result;
}

/*****************************************************************************
 * Buffer - LINESTRING
 *****************************************************************************/

/**
 * @brief Construct a curved buffer around a LINESTRING by offsetting it
 * @details The boundary consists of LINESTRING and CIRCULARSTRING components.
 * Supported:
 *   - round joins
 *   - mitre joins
 *   - bevel joins
 *   - round caps
 *   - flat caps
 *   - square caps
 * @return The buffer, or @p NULL when the offset of the line crosses itself,
 * which leaves a ring bounding no surface. #meos_buffer_line answers that case
 * by cutting the line where it crosses itself.
 */
static LWGEOM *
meos_buffer_line_offset(const LWLINE *line, double radius,
  JoinStyle join_style, EndCapStyle cap_style, double mitre_limit)
{
  assert(line); assert(radius > 0.0);
  const int32_t srid = lwgeom_get_srid((const LWGEOM *) line);
  if (! line->points || line->points->npoints < 2)
  {
    LWPOLY *empty = lwpoly_construct_empty(srid, 0, 0);
    return lwpoly_as_lwgeom(empty);
  }
  uint32_t npoints = line->points->npoints;
  POINT2D *points = palloc(sizeof(POINT2D) * npoints);
  for (uint32_t i = 0; i < npoints; i++)
  {
    POINT4D point;
    getPoint4d_p(line->points, i, &point);
    points[i].x = point.x;
    points[i].y = point.y;
  }

  /* Remove duplicate consecutive points */
  uint32_t nvalid = 0;
  for (uint32_t i = 0; i < npoints; i++)
  {
    if (nvalid == 0 ||
        hypot(points[i].x - points[nvalid - 1].x,
              points[i].y - points[nvalid - 1].y) >  MEOS_GEOM_TOLERANCE)
    {
      points[nvalid++] = points[i];
    }
  }
  if (nvalid < 2)
  {
    LWGEOM *result = lwcircle_make(points[0].x, points[0].y, radius, srid);
    pfree(points);
    return result;
  }
  npoints = nvalid;

  /* A closed line has no end to cap: its buffer is the band of twice the
   * distance centred on it, whose two boundaries are the line offset to either
   * side. The outer one bounds the surface and the inner one is its hole,
   * which the line encloses only while it stays wider than the distance. */
  if (npoints >= 4 && buffer_nodes_equal(points[0].x, points[0].y,
        points[npoints - 1].x, points[npoints - 1].y))
  {
    POINTARRAY *ring = ptarray_construct_empty(LW_FALSE, LW_FALSE, npoints);
    for (uint32_t i = 0; i < npoints; i++)
      buffer_append_point(ring, points[i].x, points[i].y);
    pfree(points);
    bool outward = buffer_ring_outward_left(ring);
    LWCOMPOUND *outer = buffer_ring(ring, radius, outward, join_style,
      mitre_limit, srid);
    if (! outer)
    {
      ptarray_free(ring);
      return NULL;
    }
    LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
    buffer_curvepoly_add_ring(result, outer);
    LWCOMPOUND *inner = buffer_ring(ring, radius, ! outward, join_style,
      mitre_limit, srid);
    /* Contracting a ring by more than it encloses carries the contraction
     * through itself, and it comes back out inverted at the distance it
     * overshot by. That curve is nearer the line than the buffer distance, so
     * it bounds nothing and the hole it would stand for is gone rather than
     * uncovered. The point it holds is what says which of the two it is: an
     * interior point of a true hole lies further from the line than the
     * distance, and an interior point of the inverted curve lies nearer */
    bool inverted = false;
    if (inner)
    {
      MeosArray *ledges = geom_extract_edges((const LWGEOM *) line);
      if (buffer_ring_inverted(inner, ledges, radius, srid))
      {
        lwgeom_free(lwcompound_as_lwgeom(inner));
        inner = NULL;
        inverted = true;
      }
      if (ledges)
        meos_array_destroy(ledges);
    }
    if (inner)
      buffer_curvepoly_add_ring(result, inner);
    else if (! inverted && ! geom_ring_is_convex(ring))
    {
      /* The hole is uncovered rather than absent: contracting a ring that is
       * not convex may leave several holes, which the boundary overlay has to
       * name */
      lwgeom_free(lwcurvepoly_as_lwgeom(result));
      ptarray_free(ring);
      return NULL;
    }
    ptarray_free(ring);
    return lwcurvepoly_as_lwgeom(result);
  }

  /* Compute the direction and normal of every segment */
  double *dx = palloc(sizeof(double) * (npoints - 1));
  double *dy = palloc(sizeof(double) * (npoints - 1));
  double *nx = palloc(sizeof(double) * (npoints - 1));
  double *ny = palloc(sizeof(double) * (npoints - 1));
  double *len = palloc(sizeof(double) * (npoints - 1));
  for (uint32_t i = 0; i < npoints - 1; i++)
  {
    dx[i] = points[i + 1].x - points[i].x;
    dy[i] = points[i + 1].y - points[i].y;
    len[i] = hypot(dx[i], dy[i]);
    dx[i] /= len[i];
    dy[i] /= len[i];
    nx[i] = -dy[i];
    ny[i] = dx[i];
  }

  /* Offset points on the left and right side.
   * For vertices the intersection of the two offset lines is used.
   * This is important: simply averaging normals produces incorrect
   * buffer distances at sharp angles. */
  POINT2D *left = palloc(sizeof(POINT2D) * npoints);
  POINT2D *right = palloc(sizeof(POINT2D) * npoints);
  left[0] = buffer_point_offset(points[0].x, points[0].y, nx[0], ny[0],
    radius);
  right[0] = buffer_point_offset(points[0].x, points[0].y, -nx[0], -ny[0],
    radius);
  left[npoints - 1] = buffer_point_offset(points[npoints - 1].x,
    points[npoints - 1].y, nx[npoints - 2], ny[npoints - 2], radius);
  right[npoints - 1] = buffer_point_offset(points[npoints - 1].x,
    points[npoints - 1].y, -nx[npoints - 2], -ny[npoints - 2], radius);

  for (uint32_t i = 1; i < npoints - 1; i++)
  {
    POINT2D p1 = buffer_point_offset(points[i].x, points[i].y,
      nx[i - 1], ny[i - 1], radius);
    POINT2D p2 = buffer_point_offset(points[i].x, points[i].y,
      nx[i], ny[i], radius);
    if (! buffer_line_intersection(p1, dx[i - 1], dy[i - 1], p2, dx[i], dy[i],
      &left[i]))
    {
      /* Parallel segments. Use the second offset point */
      left[i] = p2;
    }

    POINT2D r1 = buffer_point_offset(points[i].x, points[i].y,
      -nx[i - 1], -ny[i - 1], radius);
    POINT2D r2 = buffer_point_offset(points[i].x, points[i].y,
      -nx[i], -ny[i], radius);
    if (! buffer_line_intersection(r1, dx[i - 1], dy[i - 1], r2, dx[i], dy[i],
      &right[i]))
    {
      right[i] = r2;
    }
  }

  /* The offsets of the two segments meeting at a vertex cross at the point
   * their two offset LINES share, and that point bounds the buffer only while
   * it lies on both offset SEGMENTS. It sits the same distance from the vertex
   * along each of them, so one comparison against each length answers it */
#define BUFFER_MITRE_ON_SEGMENTS(mitre, i)                                    \
  (Min(-(((mitre).x - points[i].x) * dx[(i) - 1] +                            \
         ((mitre).y - points[i].y) * dy[(i) - 1]),                            \
        (((mitre).x - points[i].x) * dx[i] +                                  \
         ((mitre).y - points[i].y) * dy[i]))                                  \
     <= Min(len[(i) - 1], len[i]) + MEOS_GEOM_TOLERANCE)

  /* Construct the outer boundary as a compound curve */
  LWCOMPOUND *ring = lwcompound_construct_empty(srid, 0, 0);

  /* Left side. On the outer side of a turn the two offset segments do not
   * meet and the join fills the gap between their endpoints; on the inner
   * side they cross and their intersection is the single shared endpoint. */
  POINT2D cursor = left[0];
  for (uint32_t i = 1; i < npoints - 1; i++)
  {
    /* A left turn leaves the left side concave, so its offset segments cross
     * and the join belongs to the right side, and conversely */
    double turn = buffer_cross(dx[i - 1], dy[i - 1], dx[i], dy[i]);
    if (turn < -MEOS_GEOM_TOLERANCE || ! BUFFER_MITRE_ON_SEGMENTS(left[i], i))
    {
      POINT2D p1 = buffer_point_offset(points[i].x, points[i].y, nx[i - 1],
        ny[i - 1], radius);
      POINT2D p2 = buffer_point_offset(points[i].x, points[i].y, nx[i], ny[i],
        radius);
      buffer_add_segment(ring, srid, cursor, p1);
      buffer_add_join(ring, srid, points[i], p1, p2, radius, join_style,
        mitre_limit, true);
      cursor = p2;
    }
    else
    {
      buffer_add_segment(ring, srid, cursor, left[i]);
      cursor = left[i];
    }
  }
  buffer_add_segment(ring, srid, cursor, left[npoints - 1]);

  /* End cap */
  if (cap_style == ENDCAP_ROUND)
  {
    buffer_add_round_cap(ring, srid, points[npoints - 1], left[npoints - 1],
      right[npoints - 1], radius, false);
  }
  else
  {
    POINT2D l = left[npoints - 1];
    POINT2D r = right[npoints - 1];
    if (cap_style == ENDCAP_SQUARE)
    {
      l.x += dx[npoints - 2] * radius;
      l.y += dy[npoints - 2] * radius;
      r.x += dx[npoints - 2] * radius;
      r.y += dy[npoints - 2] * radius;
    }
    buffer_add_segment(ring, srid, l, r);
  }

  /* Right side, walked backwards, where the outer side of a turn is the
   * other one */
  cursor = right[npoints - 1];
  for (int i = (int) npoints - 2; i > 0; i--)
  {
    double turn = buffer_cross(dx[i - 1], dy[i - 1], dx[i], dy[i]);
    if (turn > MEOS_GEOM_TOLERANCE || ! BUFFER_MITRE_ON_SEGMENTS(right[i], i))
    {
      POINT2D p1 = buffer_point_offset(points[i].x, points[i].y, -nx[i],
        -ny[i], radius);
      POINT2D p2 = buffer_point_offset(points[i].x, points[i].y, -nx[i - 1],
        -ny[i - 1], radius);
      buffer_add_segment(ring, srid, cursor, p1);
      buffer_add_join(ring, srid, points[i], p1, p2, radius, join_style,
        mitre_limit, true);
      cursor = p2;
    }
    else
    {
      buffer_add_segment(ring, srid, cursor, right[i]);
      cursor = right[i];
    }
  }
  buffer_add_segment(ring, srid, cursor, right[0]);

  /* Start cap */
  if (cap_style == ENDCAP_ROUND)
  {
    buffer_add_round_cap(ring, srid, points[0], right[0], left[0], radius,
      false);
  }
  else
  {
    POINT2D r = right[0];
    POINT2D l = left[0];
    if (cap_style == ENDCAP_SQUARE)
    {
      r.x -= dx[0] * radius;
      r.y -= dy[0] * radius;
      l.x -= dx[0] * radius;
      l.y -= dy[0] * radius;
    }
    buffer_add_segment(ring, srid, r, l);
  }

  /* A CURVEPOLYGON can directly contain the compound curve */
  LWCURVEPOLY *curvepoly = lwcurvepoly_construct_empty(srid, 0, 0);
  buffer_curvepoly_add_ring(curvepoly, ring);

  pfree(points); pfree(dx); pfree(dy); pfree(nx); pfree(ny); pfree(len);
  pfree(left); pfree(right);
#undef BUFFER_MITRE_ON_SEGMENTS

  /* The offsets of two segments meeting at a sharp turn cross on the inner
   * side, and the loop they leave is not part of the buffer */
  LWGEOM *result = lwcurvepoly_as_lwgeom(curvepoly);
  return buffer_ring_resolved(result, lwline_as_lwgeom((LWLINE *) line),
    radius, srid);
}

/**
 * @brief Construct a curved buffer around a LINESTRING that crosses itself
 * @details The offset of a line that crosses itself crosses itself too, and
 * the loop it leaves bounds no part of the buffer, so #meos_buffer_line_offset
 * declines the line as a whole. The buffer of a line is the set of points
 * within the distance of it, so the buffer of a union of lines is the union of
 * their buffers. The line is therefore cut where it crosses itself into
 * fragments that do not, each fragment is buffered on its own, and the
 * fragments' buffers are merged.
 *
 * A cut keeps its point in both fragments, so the fragments cover the whole
 * line, but the two round caps that meet at a cut cover the disk around it
 * only where the line runs straight through: where it turns, the caps leave
 * the wedge of the turn uncovered. The disk around a cut point is part of the
 * buffer because the point is on the line, so it is merged in as well, and
 * every cut is covered whatever the turn.
 *
 * The identity holds for round joins and round caps only. A flat or square
 * cap and a mitre or bevel join answer something other than the points within
 * the distance, and what they answer does not distribute over a union: the cut
 * would show up in the result as a notch or a spike. The caller applies this
 * only for the round styles.
 * @param[in] line LINESTRING to buffer, crossing itself
 * @param[in] radius Buffer radius
 * @param[in] join_style Join style
 * @param[in] cap_style End-cap style
 * @param[in] mitre_limit Mitre limit
 * @return The buffer, or @p NULL if the line does not cross itself, if a
 * fragment cannot be buffered, or if the union does not cover the topology
 */
static LWGEOM *
meos_buffer_line_split(const LWLINE *line, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(line); assert(radius > 0.0);
  const int32_t srid = lwgeom_get_srid((const LWGEOM *) line);
  if (! line->points || line->points->npoints < 3)
    return NULL;

  /* Collect the vertices, dropping a point that repeats the one before it so
   * that only a genuine crossing is reported as a cut */
  uint32_t npoints = line->points->npoints;
  POINT2D *points = palloc(sizeof(POINT2D) * npoints);
  uint32_t nvalid = 0;
  for (uint32_t i = 0; i < npoints; i++)
  {
    POINT4D point;
    getPoint4d_p(line->points, i, &point);
    if (nvalid == 0 ||
        hypot(point.x - points[nvalid - 1].x,
              point.y - points[nvalid - 1].y) > MEOS_GEOM_TOLERANCE)
    {
      points[nvalid].x = point.x;
      points[nvalid].y = point.y;
      nvalid++;
    }
  }
  npoints = nvalid;
  if (npoints < 3)
  {
    pfree(points);
    return NULL;
  }

  /* Find the vertices at which the line crosses itself */
  const POINT2D **ptrs = palloc(sizeof(POINT2D *) * npoints);
  for (uint32_t i = 0; i < npoints; i++)
    ptrs[i] = &points[i];
  int nsplits = 0;
  bool *splits = pointarr_find_splits(ptrs, (int) npoints, &nsplits);
  pfree(ptrs);
  if (nsplits == 0)
  {
    pfree(splits); pfree(points);
    return NULL;
  }

  /* One buffer per fragment, plus one disk per cut point */
  LWGEOM **buffers = palloc(sizeof(LWGEOM *) * (2 * (uint32_t) nsplits + 1));
  uint32_t count = 0;
  bool ok = true;
  uint32_t start = 0;
  while (ok && start < npoints - 1)
  {
    uint32_t end = start + 1;
    while (end < npoints - 1 && ! splits[end])
      end++;
    /* The fragment runs from start to end inclusive */
    POINTARRAY *pa = ptarray_construct_empty(0, 0, end - start + 1);
    POINT4D point;
    point.z = 0.0; point.m = 0.0;
    for (uint32_t i = start; i <= end; i++)
    {
      point.x = points[i].x; point.y = points[i].y;
      ptarray_append_point(pa, &point, LW_TRUE);
    }
    LWLINE *fragment = lwline_construct(srid, NULL, pa);
    LWGEOM *buffer = meos_buffer_line_offset(fragment, radius, join_style,
      cap_style, mitre_limit);
    lwline_free(fragment);
    if (! buffer)
      ok = false;
    else
    {
      buffers[count++] = buffer;
      /* The disk around the cut fills the wedge the two caps leave */
      if (end < npoints - 1)
        buffers[count++] = lwcircle_make(points[end].x, points[end].y, radius,
          srid);
    }
    start = end;
  }
  pfree(splits); pfree(points);

  if (! ok)
  {
    for (uint32_t i = 0; i < count; i++)
      lwgeom_free(buffers[i]);
    pfree(buffers);
    return NULL;
  }
  LWGEOM *result = buffer_union_components(buffers, count, srid);
  pfree(buffers);
  return result;
}

/**
 * @brief Construct a curved buffer around a LINESTRING
 * @details The line is offset to either side, and a line that crosses itself,
 * whose offset bounds no surface, is cut into fragments that do not.
 * @param[in] line LINESTRING to buffer
 * @param[in] radius Buffer radius
 * @param[in] join_style Join style
 * @param[in] cap_style End-cap style
 * @param[in] mitre_limit Mitre limit
 * @return The buffer, or @p NULL if it is not covered
 */
static LWGEOM *
meos_buffer_line(const LWLINE *line, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(line); assert(radius > 0.0);
  LWGEOM *result = meos_buffer_line_offset(line, radius, join_style, cap_style,
    mitre_limit);
  /* Cutting the line answers the points within the distance of it, which is
   * what the round styles ask for and the other styles do not */
  if (result || join_style != JOIN_ROUND || cap_style != ENDCAP_ROUND)
    return result;
  return meos_buffer_line_split(line, radius, join_style, cap_style,
    mitre_limit);
}

/*****************************************************************************
 * Buffer - MULTILINE
 *****************************************************************************/

/**
 * @brief Buffer a MULTILINESTRING.
 * @details Each LINESTRING component is buffered independently.
 * Disjoint component buffers are returned as a MULTISURFACE.
 * Overlapping component buffers are also preserved as a MULTISURFACE
 * at this stage. The subsequent polygon-union layer is responsible for
 * dissolving overlapping components.
 * @param[in] mline MULTILINESTRING to buffer
 * @param[in] radius Buffer radius
 * @param[in] join_style Join style
 * @param[in] cap_style End-cap style
 * @param[in] mitre_limit Mitre limit
 * @return Buffered geometry, or an empty MULTISURFACE
 */
static LWGEOM *
meos_buffer_mline(const LWMLINE *mline, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(mline); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) mline);
  uint32_t ngeoms = mline->ngeoms;
  /* Empty MULTILINESTRING. */
  if (ngeoms == 0)
  {
    LWCOLLECTION *result = lwcollection_construct_empty(MULTISURFACETYPE,
      srid, 0, 0);
    return lwcollection_as_lwgeom(result);
  }

  /* Allocate space for the buffered components */
  LWGEOM **buffers = palloc(sizeof(LWGEOM *) * ngeoms);
  uint32_t count = 0;

  /* Buffer every LINESTRING component independently */
  for (uint32_t i = 0; i < ngeoms; i++)
  {
    const LWGEOM *component = (const LWGEOM *) mline->geoms[i];
    if (! component || lwgeom_is_empty(component))
      continue;
    /* LWMLINE components should be LINESTRINGs */
    if (component->type != LINETYPE)
      continue;

    LWGEOM *buffer = meos_buffer_line((const LWLINE *) component, radius,
      join_style, cap_style, mitre_limit);
    if (! buffer)
    {
      /* A component the buffer does not cover cannot be left out of the
       * answer: what is left is a geometry smaller than the buffer asked
       * for, valid and of the right shape and reported by nothing */
      for (uint32_t j = 0; j < count; j++)
        lwgeom_free(buffers[j]);
      pfree(buffers);
      return NULL;
    }
    buffers[count++] = buffer;
  }

  /* All components were empty */
  if (count == 0)
  {
    pfree(buffers);
    LWCOLLECTION *result = lwcollection_construct_empty(MULTISURFACETYPE,
      srid, 0, 0);
    return lwcollection_as_lwgeom(result);
  }

  /* A single component does not need a MULTISURFACE wrapper. */
  if (count == 1)
  {
    LWGEOM *result = buffers[0];
    pfree(buffers);
    return result;
  }

  /* Dissolve the component buffers that overlap */
  LWGEOM *result = buffer_union_components(buffers, count, srid);
  pfree(buffers);
  return result;
}

/*****************************************************************************
 * Buffer - POLYGON
 *****************************************************************************/

/**
 * @brief Return true if a boundary ring encloses no area
 * @details A ring contracted to exactly the width of the geometry it comes
 * from keeps its shape and its orientation and crosses itself nowhere, so only
 * the area it encloses tells it apart from one that still bounds a surface.
 */
static bool
buffer_ring_encloses_no_area(const LWCOMPOUND *ring, int32_t srid)
{
  assert(ring);
  LWCURVEPOLY *probe = lwcurvepoly_construct_empty(srid, 0, 0);
  lwcurvepoly_add_ring(probe, lwgeom_clone_deep(lwcompound_as_lwgeom(
    (LWCOMPOUND *) ring)));
  MeosArray *pieces = meos_array_create(sizeof(BufferPiece));
  LWGEOM *geom = lwcurvepoly_as_lwgeom(probe);
  bool result = ! buffer_pieces_from_geometry(geom, pieces) ||
    fabs(buffer_ring_signed_area(pieces)) <= MEOS_GEOM_TOLERANCE;
  meos_array_destroy(pieces);
  lwgeom_free(geom);
  return result;
}

/**
 * @brief Buffer a POLYGON.
 * @details The exterior ring is expanded and interior rings are contracted.
 * Round joins are represented using exact circular arcs.
 *
 * This implementation deliberately does not perform polygon overlay.
 * Consequently, cases where an offset ring collapses or self-intersects
 * are rejected and return NULL.
 */
static LWGEOM *
meos_buffer_poly(const LWPOLY *poly, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit, bool inward)
{
  assert(poly); assert(radius > 0.0);
  (void) cap_style;
  int32_t srid = lwgeom_get_srid((const LWGEOM *) poly);
  if (poly->nrings == 0)
  {
    LWCOLLECTION *result = lwcollection_construct_empty(MULTISURFACETYPE,
      srid, 0, 0);
    return lwcollection_as_lwgeom(result);
  }

  /* The first ring is the exterior ring, which a buffer expands and an
   * erosion contracts */
  bool exterior_left = buffer_ring_outward_left(poly->rings[0]);
  if (inward)
    exterior_left = ! exterior_left;
  LWCOMPOUND *exterior = buffer_ring(poly->rings[0], radius, exterior_left,
    join_style, mitre_limit, srid);
  if (! exterior)
    /* A convex ring contracted past its own width leaves nothing */
    return (inward && geom_ring_is_convex(poly->rings[0])) ?
      lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0)) : NULL;

  /* Construct the curved polygon */
  LWCURVEPOLY *result = lwcurvepoly_construct_empty(srid, 0, 0);
  buffer_curvepoly_add_ring(result, exterior);

  /* Holes.
   * A positive polygon buffer contracts the holes. Therefore the
   * buffering side is the opposite of the exterior side.
   */
  for (uint32_t i = 1; i < poly->nrings; i++)
  {
    /* A positive buffer erodes a hole, and what is left of one is non-empty
     * only where the hole holds a disc of the buffer distance, so a hole
     * enclosing less area than that disc holds none of it and closes
     * completely. What this reaches on real data is a ring enclosing NO area
     * at all, a slit that runs out to a point and back: offsetting one sweeps
     * a band of the buffer distance about the slit and punches that band out
     * of the answer, which is a hole the geometry never had. An erosion
     * widens a hole rather than closing it, so this reads the outward case */
    if (! inward &&
        fabs(buffer_ring_area(poly->rings[i])) < M_PI * radius * radius)
      continue;
    bool hole_left = ! buffer_ring_outward_left(poly->rings[i]);
    if (inward)
      hole_left = ! hole_left;
    LWCOMPOUND *hole = buffer_ring(poly->rings[i], radius, hole_left,
      join_style, mitre_limit, srid);
    if (! hole)
    {
      lwgeom_free(lwcurvepoly_as_lwgeom(result));
      return NULL;
    }
    buffer_curvepoly_add_ring(result, hole);
  }

  /* A ring contracted past the width of the polygon crosses itself, and it
   * takes the boundary overlay to say which surfaces the crossing leaves */
  LWGEOM *geom = lwcurvepoly_as_lwgeom(result);
  if (inward && buffer_boundary_self_intersects(geom))
  {
    lwgeom_free(geom);
    return NULL;
  }
  /* A ring contracted to exactly the width of the polygon encloses no area,
   * and a surface of no area is nothing */
  if (inward && buffer_ring_encloses_no_area(exterior, srid))
  {
    lwgeom_free(geom);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0));
  }
  /* A ring turning tighter than the buffer distance carries offsets that run
   * into one another, and what a crossing encloses lies inside the buffer
   * rather than on its boundary */
  return inward ? geom :
    buffer_ring_resolved(geom, (const LWGEOM *) poly, radius, srid);
}

/*****************************************************************************
 * Buffer - MULTIPOLYGON
 *****************************************************************************/

/**
 * @brief Buffer a MULTIPOLYGON.
 * @details Each polygon component is buffered independently.
 * If the resulting components are disjoint, the result is returned as a
 * MULTISURFACE. If two component buffers overlap, they must be unioned
 * before returning the result. The polygon overlay/union layer is
 * responsible for that operation and is not performed here.
 * @param[in] mpoly MULTIPOLYGON
 * @param[in] radius Buffer radius
 * @param[in] join_style Join style
 * @param[in] cap_style Cap style
 * @param[in] mitre_limit Mitre limit
 * @return Buffered geometry, or @p NULL if a polygon union is required
 */
static LWGEOM *
meos_buffer_mpoly(const LWMPOLY *mpoly, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(mpoly); assert(radius > 0.0);
  uint32_t ngeoms = mpoly->ngeoms;
  int32_t srid = lwgeom_get_srid((const LWGEOM *) mpoly);
  if (ngeoms == 0)
  {
    LWCOLLECTION *result = lwcollection_construct_empty(MULTISURFACETYPE,
      srid, 0, 0);
    return lwcollection_as_lwgeom(result);
  }

  LWGEOM **buffers = palloc(sizeof(LWGEOM *) * ngeoms);
  uint32_t count = 0;
  /* Buffer every polygon independently */
  for (uint32_t i = 0; i < ngeoms; i++)
  {
    const LWGEOM *component = (const LWGEOM *) mpoly->geoms[i];
    if (! component || lwgeom_is_empty(component))
      continue;
    if (component->type != POLYGONTYPE)
      continue;
    LWGEOM *buffer = meos_buffer_poly((const LWPOLY *) component, radius,
      join_style, cap_style, mitre_limit, false);
    if (! buffer)
    {
      /* A component the buffer does not cover cannot be left out of the
       * answer: what is left is a geometry smaller than the buffer asked
       * for, valid and of the right shape and reported by nothing */
      for (uint32_t j = 0; j < count; j++)
        lwgeom_free(buffers[j]);
      pfree(buffers);
      return NULL;
    }
    buffers[count++] = buffer;
  }

  /* All components were empty */
  if (count == 0)
  {
    pfree(buffers);
    LWCOLLECTION *result = lwcollection_construct_empty(MULTISURFACETYPE,
      srid, 0, 0);
    return lwcollection_as_lwgeom(result);
  }

  /* Dissolve the component buffers that overlap */
  LWGEOM *result = buffer_union_components(buffers, count, srid);
  pfree(buffers);
  return result;
}

/*****************************************************************************
 * Buffer - GEOMETRYCOLLECTION
 *****************************************************************************/

/**
 * @brief Buffer a GEOMETRYCOLLECTION.
 * @details Each component is buffered recursively. Nested geometry
 * collections are therefore handled transparently.
 *
 * The component buffers are not unioned at this stage. If two component
 * buffers overlap, the function returns NULL because returning the
 * overlapping surfaces would not be equivalent to ST_Buffer.
 *
 * If all component buffers are disjoint, they are returned as a
 * MULTISURFACE.
 */
static LWGEOM *
meos_buffer_collection(const LWCOLLECTION *collection, double radius,
  JoinStyle join_style, EndCapStyle cap_style, double mitre_limit)
{
  assert(collection); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) collection);

  /* First count the maximum number of output components.
   * A nested GEOMETRYCOLLECTION may produce more than one component,
   * therefore we allocate dynamically below rather than relying on
   * collection->ngeoms as the final count. */
  uint32_t capacity = collection->ngeoms > 0 ? collection->ngeoms : 1;
  uint32_t count = 0;
  LWGEOM **buffers = palloc(sizeof(LWGEOM *) * capacity);

  /* Buffer every component */
  for (uint32_t i = 0; i < collection->ngeoms; i++)
  {
    const LWGEOM *component = collection->geoms[i];
    if (! component || lwgeom_is_empty(component))
      continue;
    LWGEOM *buffer = NULL;
    /* A nested GEOMETRYCOLLECTION may itself produce a MULTISURFACE.
     * We keep this as one component for now. The polygon-union layer
     * will later be responsible for dissolving all overlapping pieces. */
    if (component->type == COLLECTIONTYPE)
      buffer = meos_buffer_collection((const LWCOLLECTION *) component, radius,
        join_style, cap_style, mitre_limit);
    else
      buffer = meos_buffer(component, radius, join_style, cap_style,
        mitre_limit);
    if (! buffer)
    {
      /* A component the buffer does not cover cannot be left out of the
       * answer: what is left is a geometry smaller than the buffer asked
       * for, valid and of the right shape and reported by nothing */
      for (uint32_t j = 0; j < count; j++)
        lwgeom_free(buffers[j]);
      pfree(buffers);
      return NULL;
    }
    /* Grow the output array if necessary */
    if (count == capacity)
    {
      capacity *= 2;
      buffers = repalloc(buffers, sizeof(LWGEOM *) * capacity);
    }
    buffers[count++] = buffer;
  }

  /* No non-empty component produced a buffer */
  if (count == 0)
  {
    pfree(buffers);
    LWCOLLECTION *result = lwcollection_construct_empty(MULTISURFACETYPE,
      srid, 0, 0);
    return lwcollection_as_lwgeom(result);
  }

  /* A single component needs no collection wrapper */
  if (count == 1)
  {
    LWGEOM *result = buffers[0];
    pfree(buffers);
    return result;
  }

  /* Dissolve the component buffers that overlap */
  LWGEOM *result = buffer_union_components(buffers, count, srid);
  pfree(buffers);
  return result;
}

/*****************************************************************************
 * Buffer - Dispatcher
 *****************************************************************************/

/**
 * @brief Buffer a TRIANGLE
 * @details A triangle is a polygon of a single ring.
 */
static LWGEOM *
meos_buffer_triangle(const LWTRIANGLE *triangle, double radius,
  JoinStyle join_style, EndCapStyle cap_style, double mitre_limit)
{
  assert(triangle); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) triangle);
  POINTARRAY **rings = lwalloc(sizeof(POINTARRAY *));
  rings[0] = ptarray_clone_deep(triangle->points);
  LWPOLY *poly = lwpoly_construct(srid, NULL, 1, rings);
  LWGEOM *result = meos_buffer_poly(poly, radius, join_style, cap_style,
    mitre_limit, false);
  lwpoly_free(poly);
  return result;
}

/**
 * @brief Return true if an areal geometry encloses no area
 * @details A ring whose vertices are collinear, or that repeats one point,
 * bounds nothing, and the geometry it belongs to has no interior to keep.
 */
static bool
buffer_areal_is_degenerate(const LWGEOM *geom)
{
  assert(geom);
  if (geom->type == POLYGONTYPE)
  {
    const LWPOLY *poly = (const LWPOLY *) geom;
    return poly->nrings == 0 ||
      fabs(buffer_ring_area(poly->rings[0])) <= MEOS_GEOM_TOLERANCE;
  }
  if (geom->type == MULTIPOLYGONTYPE)
  {
    const LWMPOLY *mpoly = (const LWMPOLY *) geom;
    for (uint32_t i = 0; i < mpoly->ngeoms; i++)
      if (! buffer_areal_is_degenerate((const LWGEOM *) mpoly->geoms[i]))
        return false;
    return true;
  }
  return true;
}

/**
 * @brief Erode a MULTIPOLYGON
 * @details Erosion only ever shrinks a component, so the eroded components
 * stay as disjoint as the ones they come from and need no union. A component
 * eroded away contributes nothing.
 */
static LWGEOM *
meos_erode_mpoly(const LWMPOLY *mpoly, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(mpoly); assert(radius > 0.0);
  int32_t srid = lwgeom_get_srid((const LWGEOM *) mpoly);
  LWGEOM **eroded = palloc(sizeof(LWGEOM *) * (mpoly->ngeoms + 1));
  uint32_t count = 0;
  for (uint32_t i = 0; i < mpoly->ngeoms; i++)
  {
    const LWGEOM *component = (const LWGEOM *) mpoly->geoms[i];
    if (! component || lwgeom_is_empty(component) ||
        component->type != POLYGONTYPE)
      continue;
    LWGEOM *part = meos_buffer_poly((const LWPOLY *) component, radius,
      join_style, cap_style, mitre_limit, true);
    /* A component the erosion does not cover leaves the whole result
     * uncovered, since dropping it would answer a smaller geometry */
    if (! part)
    {
      for (uint32_t j = 0; j < count; j++)
        lwgeom_free(eroded[j]);
      pfree(eroded);
      return NULL;
    }
    eroded[count++] = part;
  }
  if (count == 0)
  {
    pfree(eroded);
    return lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0));
  }
  if (count == 1)
  {
    LWGEOM *result = eroded[0];
    pfree(eroded);
    return result;
  }
  /* lwcollection_construct() takes ownership of the geometry array */
  return lwcollection_as_lwgeom(lwcollection_construct(MULTISURFACETYPE, srid,
    NULL, count, eroded));
}

/**
 * @brief Native MEOS implementation of ST_Buffer.
 * @details Currently supported:
 * - POINT
 * - MULTIPOINT
 * - LINESTRING
 * - MULTILINESTRING
 * - POLYGONTYPE
 * - MULTIPOLYGONTYPE
 * - TRIANGLE
 * - CIRCULARSTRING, COMPOUNDCURVE and MULTICURVE
 * - CURVEPOLYGON and MULTISURFACE
 * - GEOMETRYCOLLECTION
 * Polygon buffering is handled by the polygon buffering layer.
 * Component buffers are not unioned yet. If buffering a collection
 * produces overlapping components, NULL is returned.
 */
LWGEOM *
meos_buffer(const LWGEOM *geom, double radius, JoinStyle join_style,
  EndCapStyle cap_style, double mitre_limit)
{
  assert(geom);
  int32_t srid = lwgeom_get_srid(geom);
  if (mitre_limit <= 0.0)
    mitre_limit = 5.0;

  /* A buffer of a non-positive distance keeps the points of the geometry that
   * are at least that far from its exterior, which is nothing unless the
   * geometry has an interior of its own */
  if (radius <= 0.0)
  {
    bool areal = geom->type == POLYGONTYPE || geom->type == MULTIPOLYGONTYPE;
    if (! areal)
      return lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0));
    /* A zero distance keeps the geometry itself, unless it encloses no area */
    if (radius == 0.0)
      return buffer_areal_is_degenerate(geom) ?
        lwpoly_as_lwgeom(lwpoly_construct_empty(srid, 0, 0)) :
        lwgeom_clone_deep(geom);
    if (geom->type == POLYGONTYPE)
      return meos_buffer_poly((const LWPOLY *) geom, -radius, join_style,
        cap_style, mitre_limit, true);
    return meos_erode_mpoly((const LWMPOLY *) geom, -radius, join_style,
      cap_style, mitre_limit);
  }
  switch (geom->type)
  {
    case POINTTYPE:
      return meos_buffer_point((const LWPOINT *) geom, radius);
    case MULTIPOINTTYPE:
      return meos_buffer_mpoint((const LWMPOINT *) geom, radius);
    case LINETYPE:
      return meos_buffer_line((const LWLINE *) geom, radius, join_style,
        cap_style, mitre_limit);
    case MULTILINETYPE:
      return meos_buffer_mline((const LWMLINE *) geom, radius, join_style,
        cap_style, mitre_limit);
    case POLYGONTYPE:
      return meos_buffer_poly((const LWPOLY *) geom, radius, join_style,
        cap_style, mitre_limit, false);
    case MULTIPOLYGONTYPE:
      return meos_buffer_mpoly((const LWMPOLY *) geom, radius, join_style,
        cap_style, mitre_limit);
    case TRIANGLETYPE:
      return meos_buffer_triangle((const LWTRIANGLE *) geom, radius,
        join_style, cap_style, mitre_limit);
    case CIRCSTRINGTYPE:
    case COMPOUNDTYPE:
      return meos_buffer_curve(geom, radius, join_style, cap_style,
        mitre_limit);
    case CURVEPOLYTYPE:
      return meos_buffer_curvepoly((const LWCURVEPOLY *) geom, radius,
        join_style, mitre_limit, false);
    /* A TIN (collection of triangles) and a polyhedral surface (collection of
     * polygonal faces) share the collection memory layout, and the collection
     * branch buffers each component through this same dispatch, so a triangle
     * and a face are each buffered by the entry that owns them */
    case MULTICURVETYPE:
    case MULTISURFACETYPE:
    case TINTYPE:
    case POLYHEDRALSURFACETYPE:
    case COLLECTIONTYPE:
      return meos_buffer_collection((const LWCOLLECTION *) geom, radius,
        join_style, cap_style, mitre_limit);
    default:
      return NULL;
  }
}

/**
 * @ingroup meos_geo_base_spatial
 * @brief Return a @p POLYGON or a @p MULTIPOLYGON that represents all points
 * whose distance from a geometry/geography is less than or equal to a given
 * distance
 * @param[in] gs Geometry
 * @param[in] size Distance
 * @param[in] params Buffer style parameters
 * @note PostGIS function: @p ST_Buffer(PG_FUNCTION_ARGS)
 * @csqlfn #Geom_buffer()
 */
GSERIALIZED *
geom_buffer(const GSERIALIZED *gs, double size, const char *params)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(gs, NULL); VALIDATE_NOT_NULL(params, NULL);
  if (! ensure_not_geodetic_geo(gs))
    return NULL;

  const double DEFAULT_MITRE_LIMIT = 5.0;
  const int DEFAULT_ENDCAP_STYLE = ENDCAP_ROUND;
  const int DEFAULT_JOIN_STYLE = JOIN_ROUND;
  double mitre_limit = DEFAULT_MITRE_LIMIT;
  int cap_style = DEFAULT_ENDCAP_STYLE;
  int join_style  = DEFAULT_JOIN_STYLE;

  /* In the for loop below the params parameter is modified.
   * Therefore we need to take a copy of it */
  size_t params_size = strlen(params) + 1;
  char *params1 = palloc(params_size);
  memcpy(params1, params, params_size);
  char *param;
  for (param = params1; ; param = NULL)
  {
    char *key, *val;
    param = strtok(param, " ");
    if (! param)
      break;

    key = param;
    val = strchr(key, '=');
    if (! val || *(val + 1) == '\0')
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Missing value for buffer parameter %s", key);
      pfree(params1);
      return NULL;
    }
    *val = '\0';
    ++val;

    if (! strcmp(key, "endcap"))
    {
      /* Supported end cap styles: "round", "flat", "square" */
      if (! strcmp(val, "round"))
        cap_style = ENDCAP_ROUND;
      else if (! strcmp(val, "flat") || ! strcmp(val, "butt"))
        cap_style = ENDCAP_FLAT;
      else if (! strcmp(val, "square"))
        cap_style = ENDCAP_SQUARE;
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid buffer end cap style: %s (accept: 'round', 'flat', "
          "'butt' or 'square')", val);
        return NULL;
      }
    }
    else if (! strcmp(key, "join"))
    {
      if (! strcmp(val, "round"))
        join_style = JOIN_ROUND;
      else if (! strcmp(val, "mitre") || ! strcmp(val, "miter"))
        join_style = JOIN_MITRE;
      else if (! strcmp(val, "bevel"))
        join_style = JOIN_BEVEL;
      else
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid buffer end cap style: %s (accept: 'round', 'mitre', "
          "'miter'  or 'bevel')", val);
        pfree(params1);
        return NULL;
      }
    }
    else if (! strcmp(key, "mitre_limit") || ! strcmp(key, "miter_limit"))
      /* mitre_limit is a float */
      mitre_limit = atof(val);
    else if (! strcmp(key, "quad_segs"))
    {
      /* The number of segments approximating a quarter circle. This
       * implementation represents a round join and a round end cap with the
       * exact circular arc, so there is no approximation to control and the
       * value is accepted without effect. */
      if (atoi(val) < 1)
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid number of quadrant segments: %s (accept: a positive "
          "integer)", val);
        pfree(params1);
        return NULL;
      }
    }
    else if (! strcmp(key, "side"))
    {
      if (! strcmp(val, "left") || ! strcmp(val, "right"))
      {
        meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
          "Single-sided buffer is not supported: side=%s", val);
        pfree(params1);
        return NULL;
      }
      else if (strcmp(val, "both"))
      {
        meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
          "Invalid side parameter: %s (accept: 'right', 'left', 'both')",
          val);
        pfree(params1);
        return NULL;
      }
    }
    else
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Invalid buffer parameter: %s (accept: 'endcap', 'join', "
        "'mitre_limit', 'miter_limit', 'quad_segs' and 'side')", key);
      pfree(params1);
      return NULL;
    }
  }
  pfree(params1);

  LWGEOM *lwg;

  /* Empty.Buffer() == Empty[polygon] */
  if (gserialized_is_empty(gs))
  {
    lwg = lwpoly_as_lwgeom(lwpoly_construct_empty(gserialized_get_srid(gs),
      0, 0)); // buffer wouldn't give back z or m anyway
    GSERIALIZED *result = geo_serialize(lwg);
    lwgeom_free(lwg);
    return result;
  }

  lwg = lwgeom_from_gserialized(gs);
  if (! lwgeom_isfinite(lwg))
  {
    meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
      "Geometry contains invalid coordinates");
    lwgeom_free(lwg);
    return NULL;
  }

  LWGEOM *res = meos_buffer(lwg, size, join_style, cap_style, mitre_limit);
  /* A geometry the arc-exact buffer does not cover, such as one whose offset
   * ring collapses or self-intersects, which the boundary overlay resolves
   * only for the configurations it reaches */
  if (! res)
  {
    meos_error(ERROR, MEOS_ERR_FEATURE_NOT_SUPPORTED,
      "The buffer of the geometry is not supported: %s", geo_out(gs));
    lwgeom_free(lwg);
    return NULL;
  }
  GSERIALIZED *result = geo_serialize(res);
  lwgeom_free(lwg); lwgeom_free(res);
  return result;
}


/*****************************************************************************/