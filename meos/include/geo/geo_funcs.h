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
 * @brief The geometry a native implementation works on, decomposed into its
 * edges
 * @details An implementation that answers a question about a geometry without
 * calling GEOS works on the edges of that geometry: the segments and the
 * circular arcs its boundary is made of. The decomposition that produces them
 * lived inside the clipping engine, reachable only from there, so anything
 * else needing it had to build its own. It lives here instead, where every
 * implementation reaches it.
 */

#ifndef __GEO_FUNCS_H__
#define __GEO_FUNCS_H__

/* C */
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <meos_error.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"

/*****************************************************************************/

/**
 * @brief Enumeration defining the edge types 
 */
typedef enum
{
  EDGE_POINT = 0,
  EDGE_LINESEG,
  EDGE_LINEARC,
  EDGE_POLYSEG,
  EDGE_POLYARC
} EdgeType;

/**
 * @brief Tolerance under which planar geometry reads two quantities as the same
 * @details Coordinates, the parameter along a segment, a determinant and a
 * duration are all compared against this value. MEOS owns it rather than
 * reading the tolerance of a PostGIS internal header, which two headers of
 * that library define differently, so what a kernel compares against does not
 * depend on the order a caller includes them in. It carries the value
 * `liblwgeom_internal.h` states for planar work.
 * @note Distinct from `MEOS_EPSILON`, which reads two modelled quantities as
 * equal at 1e-06 and is what the `MEOS_FP_*` comparisons are built on. That one
 * is a fraction of a duration, bounded below by the microsecond a `TimestampTz`
 * resolves, while this one is the rounding of coordinate arithmetic.
 * @note `FP_TOLERANCE` survives where MEOS hands a tolerance to PostGIS or runs
 * a copy of its code, so that those calls keep tracking the value PostGIS means
 * -- including the smaller one `lwgeodetic.h` states for spherical work.
 */
#define MEOS_GEOM_TOLERANCE 1e-12


/**
 * @brief Structure keeping a geometry edge
 */
typedef struct
{
  double x1, y1, x2, y2;         /**< Coordinates of the start/end 2D points */
  double xmin, ymin, xmax, ymax; /**< Precomputed bounding box of the edge */
  double dx, dy, length;         /**< Precomputed dx, dy, and length */
  double tol;                    /**< Precomputed #coordinate_tolerance of the
                                      edge, which its own coordinates fix */
  double cx, cy, radius;         /**< Arc center and radius (an arc only) */
  double theta0, theta1;         /**< Arc start/end angles (an arc only) */
  bool ccw;                      /**< Arc traversed counterclockwise */
  EdgeType etype;                /**< Edge type */
} Edge;

/**
 * @brief Enumeration defining the intersection types 
 */
typedef enum
{
  INTERSECT_NONE = 0,
  INTERSECT_POINT,
  INTERSECT_OVERLAP
} IntersectType;

/**
 * @brief Structure keeping an intersection result
 */
typedef struct
{
  IntersectType type;  /**< Intersection type */
  double t0;           /**< Always valid if type != NONE */
  double t1;           /**< Only valid for OVERLAP */
} IntersectResult;

/*****************************************************************************/

/**
 * @brief Return an angle brought into the interval [0, 2*pi)
 */
static inline double
angle_normalize(double a)
{
  double r = fmod(a, 2 * M_PI);
  if (r < 0)
    r += 2 * M_PI;
  return r;
}

/*****************************************************************************/

extern LWGEOM *meos_oriented_envelope(const LWGEOM *geom);
extern LWGEOM *meos_areal_union(const LWGEOM *geom);
extern LWGEOM *meos_areal_intersection(const LWGEOM *geom1,
  const LWGEOM *geom2);
extern LWGEOM *meos_areal_difference(const LWGEOM *geom1,
  const LWGEOM *geom2);
extern LWGEOM *meos_linear_union(const LWGEOM *geom);
extern LWGEOM *meos_centroid(const LWGEOM *geom);
extern LWGEOM *meos_lift_ordinates(const LWGEOM *geom, const LWGEOM **geoms,
  int count);
extern bool meos_is_simple(const LWGEOM *geom, bool *result);
extern bool meos_relate(const LWGEOM *g1, const LWGEOM *g2, char result[10]);
extern bool meos_relate_pattern(const LWGEOM *g1, const LWGEOM *g2,
  const char *pattern, bool *result);
extern bool meos_spatialrel(const LWGEOM *g1, const LWGEOM *g2, spatialRel rel,
  bool *result);
extern bool relate_is_areal(const LWGEOM *geom);

/* The edges of one geometry, kept so that several relationships asked about it
 * read them once. A relationship extracts the edges of both its operands, and
 * for a multi-surface — whose edges are those of the union of its members —
 * that is what the call mostly costs. A caller asking about the same geometry
 * many times, as an all-pairs walk does, holds its context and pays the
 * extraction once instead of once per pair. NULL where the geometry is one the
 * engine does not cover, which is what #meos_spatialrel answers false for */
extern void *relate_ctx_make(const LWGEOM *geom);
extern void relate_ctx_free(void *ctx);
extern bool meos_spatialrel_ctx(const void *ctx1, const void *ctx2,
  spatialRel rel, bool *result);
extern bool de9im_match(const char matrix[10], const char pattern[10]);
extern int point_in_polygon(double x, double y, Edge **edges, int nedges);
extern int point_in_polygon_index(double x, double y, Edge **edges,
  int nedges, const RTree *rtree, double xmax);
/**
 * @brief Return true if a polygon ring turns the same way at every vertex,
 * which is what makes it convex
 * @details The cross product of two consecutive edges is signed by the turn
 * they make, so a ring whose turns all carry one sign is convex, and one
 * change of sign is a vertex the ring is concave at. A ring of fewer than
 * three distinct vertices bounds no area and is not convex.
 * @note Defined here so that every caller inlines it, the buffer engine
 * included, rather than reaching it through a call across files.
 */
static inline bool
geom_ring_is_convex(const POINTARRAY *pa)
{
  if (! pa || pa->npoints < 4)
    return false;
  uint32_t n = pa->npoints - 1;
  int sign = 0;
  for (uint32_t i = 0; i < n; i++)
  {
    POINT4D a, b, c;
    getPoint4d_p(pa, i, &a);
    getPoint4d_p(pa, (i + 1) % n, &b);
    getPoint4d_p(pa, (i + 2) % n, &c);
    double turn = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
    /* The tolerance liblwgeom uses for a coordinate comparison */
    if (fabs(turn) <= MEOS_GEOM_TOLERANCE)
      continue;
    int current = turn > 0.0 ? 1 : -1;
    if (sign == 0)
      sign = current;
    else if (sign != current)
      return false;
  }
  return sign != 0;
}

extern bool relate_point_on_boundary(double x, double y, Edge **edges,
  int nedges);
extern int relate_point_in_area(double x, double y, Edge **edges, int nedges);
/* Reading a point question out of an index instead of the whole array, which
 * the buffer overlay asks as well: it locates a point per boundary piece
 * against each operand, so the edges are read once and the questions asked
 * through them. The threshold lives here so the two engines gate on ONE
 * number rather than on copies that can drift apart */
#define RELATE_INDEX_MIN_PAIRS 100000

/**
 * @brief An edge array together with what a question about one point needs in
 * order to read only the edges that can answer it
 * @details The index is absent below the threshold, and every function taking
 * this reads the whole array in that case, so the answer never depends on
 * whether the index was built
 */
typedef struct
{
  Edge **edges;   /**< Edges the array holds */
  int nedges;     /**< Number of edges */
  RTree *index;   /**< Index over the edge boxes, NULL below the threshold */
  double xmax;    /**< Greatest x the edges reach */
  double tol;     /**< Widest tolerance any of the edges asks for */
} RelateEdges;

extern void relate_edges_init(RelateEdges *re, Edge **edges, int nedges,
  bool index);
extern void relate_edges_clear(RelateEdges *re);
extern int relate_point_in_area_index(double x, double y,
  const RelateEdges *re);
/* Where two arcs meet, which the buffer overlay asks as well: solving the two
 * circles and keeping the solutions both angular spans hold is one
 * computation, and an intersection the two engines place differently is a
 * defect neither of them can see from its own file */
extern int relate_arc_arc_points(const Edge *a, const Edge *b, double x[2],
  double y[2], bool *overlap);

extern bool ensure_srid_is_latlong(int32_t srid);
extern bool ensure_geodetic_geo(const GSERIALIZED *gs);
extern bool ensure_not_geodetic_geo(const GSERIALIZED *gs);
extern bool ensure_geodetic(int16 flags);
extern bool ensure_not_geodetic(int16 flags);
/**
 * @brief Ensure that two temporal values have the same geodetic flag
 */
static inline bool
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
extern bool ensure_same_geodetic_geo(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool ensure_srid_known(int32_t srid);
/**
 * @brief Ensure that two values have the same SRID
 */
static inline bool
ensure_same_srid(int32_t srid1, int32_t srid2)
{
  if (srid1 == srid2)
    return true;
  meos_error(ERROR, MEOS_ERR_INVALID_ARG_VALUE,
    "Operation on mixed SRID");
  return false;
}
extern bool ensure_same_srid_geoarr(const GSERIALIZED **geoms, int count);
extern bool ensure_srid_reconcile(int32_t srid1, int32_t srid2, int32_t *result);
extern bool ensure_same_dimensionality(int16 flags1, int16 flags2);
extern bool ensure_same_spatial_dimensionality(int16 flags1, int16 flags2);
extern bool ensure_has_Z_geo(const GSERIALIZED *gs);
extern bool ensure_has_not_Z_geo(const GSERIALIZED *gs);
extern bool ensure_has_M_geo(const GSERIALIZED *gs);
extern bool ensure_has_not_M_geo(const GSERIALIZED *gs);
extern bool ensure_point_type(const GSERIALIZED *gs);
extern bool ensure_mline_type(const GSERIALIZED *gs);
extern bool ensure_not_empty(const GSERIALIZED *gs);
extern long double closest_point2d_on_segment_ratio(const POINT2D *p,
  const POINT2D *A, const POINT2D *B, POINT2D *closest);
extern long double closest_point3dz_on_segment_ratio(const POINT3DZ *p,
  const POINT3DZ *A, const POINT3DZ *B, POINT3DZ *closest);
extern LWGEOM **lwpointarr_remove_duplicates(LWGEOM **points, int count,
  int *newcount);
extern LWGEOM *lwpointarr_make_trajectory(LWGEOM **points, int count,
  interpType interp);
extern bool ensure_valid_geo_geo(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern int geopoint_cmp(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geopoint_eq(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geopoint_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool same_spatial_dimensionality(int16 flags1, int16 flags2);
extern bool mline_type(const GSERIALIZED *gs);
extern GSERIALIZED *geopoint_make(double x, double y, double z, bool hasz,
  bool geodetic, int32_t srid);
extern GSERIALIZED **geo_extract_elements(const GSERIALIZED *gs, int *count);
extern GSERIALIZED *geo_serialize(const LWGEOM *geom);
extern bool circle_type(const GSERIALIZED *gs);
extern bool ensure_circle_type(const GSERIALIZED *gs);
extern LWGEOM *lwcircle_make(double x, double y, double radius,
  int32_t srid);
extern GSERIALIZED *geocircle_make(double x, double y, double radius,
  int32_t srid);
extern MeosArray *geom_extract_edges(const LWGEOM *geom);
extern RTree *build_edge_rtree(const Edge *edges, int nedges, int32_t srid);
extern bool *pointarr_find_splits(const POINT2D **points, int npoints,
  int *count);

/*****************************************************************************/

/*****************************************************************************
 * Kernels answering about the edges of a geometry
 *****************************************************************************/

/*****************************************************************************
 * Line segment intersection
 *****************************************************************************/

/**
 * @brief Return the intersection value obtained by computing the intersection 
 * of a line segment defined by two 2D points intersects an edge
 * @details Possible result values
 * - No intersection: INTERSECT_NONE -> t0 and t1 undefined
 * - Single point: INTERSECT_POINT -> t1 in [0,1], t1 ignored
 * - Overlap segment: INTERSECT_OVERLAP -> t0 <= t1 in [0,1]
 * Invariants:
 * - 0 <= t0 <= 1
 * - 0 <= t1 <= 1
 * - t0 <= t1
 * - Overlap must satisfy: t1 - t0 > MEOS_GEOM_TOLERANCE
 * @param[in] ax,ay Coordinates of the first point defining the first segment
 * @param[in] rx,ry Vector AB
 * @param[in] cx,cy,dx,dy Coordinates of the points defining the second segment
 * @note To avoid recomputing vector AB in EVERY call to the functions,
 * we pass the vector instead of the second point b computed as follows
 * @code
 * double rx = bx - ax, ry = by - ay;
 * @endcode
 */
static inline IntersectResult
linesegm_intersect(double ax, double ay, double rx, double ry,
  double cx, double cy, double dx, double dy)
{
  IntersectResult res = {INTERSECT_NONE, 0, 0};
  double sx = dx - cx, sy = dy - cy; /* vector CD */
  /* Where is the start of the second segment relative to the first? */
  double qpx = cx - ax, qpy = cy - ay;

  /* Are the two segments parallel?  */
  double rxs = rx * sy - ry * sx;

  /* Collinear / parallel */
  if (fabs(rxs) < MEOS_GEOM_TOLERANCE)
  {
    /* Is point C aligned with segment AB? */
    double qpxr = qpx * ry - qpy * rx;
    /* If qpxr != 0: parallel, if qpxr == 0: collinear */
    if (fabs(qpxr) > MEOS_GEOM_TOLERANCE)
      return res;

    /* Collinear case */
    double r2 = rx * rx + ry * ry;
    if (r2 < MEOS_GEOM_TOLERANCE)
      return res;

    double t0 = (qpx * rx + qpy * ry) / r2;
    double t1 = t0 + (sx * rx + sy * ry) / r2;

    /* Order t0 < t1 */
    if (t0 > t1) { double tmp = t0; t0 = t1; t1 = tmp; }
    /* No intersection */
    if (t1 < 0 || t0 > 1)
      return res;

    /* Clamp values */
    if (t0 < 0) t0 = 0;
    if (t1 > 1) t1 = 1;

    if (fabs(t1 - t0) < MEOS_GEOM_TOLERANCE)
    {
      res.type = INTERSECT_POINT;
      res.t0 = t0;
      return res;
    }

    res.type = INTERSECT_OVERLAP;
    res.t0 = t0;
    res.t1 = t1;
    return res;
  }

  /* Proper intersection */
  double t = (qpx * sy - qpy * sx) / rxs;
  double u = (qpx * ry - qpy * rx) / rxs;

  if (t < -MEOS_GEOM_TOLERANCE || t > 1 + MEOS_GEOM_TOLERANCE ||
      u < -MEOS_GEOM_TOLERANCE || u > 1 + MEOS_GEOM_TOLERANCE)
    return res;

  /* Clamp values */
  if (fabs(t) < MEOS_GEOM_TOLERANCE) t = 0;
  if (fabs(t - 1) < MEOS_GEOM_TOLERANCE) t = 1;

  res.type = INTERSECT_POINT;
  res.t0 = t;
  return res;
}

/*****************************************************************************
 * Circular arc intersection
 *****************************************************************************/

/**
 * @brief Return how far an angular span turns, in the sense it is traversed
 * @details The span runs from @p theta0 to @p theta1, counterclockwise when
 * @p ccw is true and clockwise otherwise
 */
static inline double
arc_span_sweep(double theta0, double theta1, bool ccw)
{
  return ccw ?
    angle_normalize(theta1 - theta0) :
    angle_normalize(theta0 - theta1);
}

/**
 * @brief Return how far an angle stands from the start of an angular span, in
 * the sense the span is traversed
 */
static inline double
arc_span_offset(double theta0, bool ccw, double phi)
{
  return ccw ?
    angle_normalize(phi - theta0) :
    angle_normalize(theta0 - phi);
}

/**
 * @brief Return true if an angle lies within an angular span
 * @details The span is traversed from @p theta0 to @p theta1, counterclockwise
 * when @p ccw is true and clockwise otherwise. Every arc question either
 * overlay asks reduces to this one, and an arc the two answer differently is a
 * defect neither of them can see from its own file
 */
static inline bool
arc_span_contains(double theta0, double theta1, bool ccw, double phi)
{
  double sweep = arc_span_sweep(theta0, theta1, ccw);
  double off = arc_span_offset(theta0, ccw, phi);
  return off <= sweep + MEOS_GEOM_TOLERANCE;
}

/**
 * @brief Return true if an angle lies within the angular span of an arc edge
 */
static inline bool
arc_contains_angle(const Edge *e, double phi)
{
  return arc_span_contains(e->theta0, e->theta1, e->ccw, phi);
}

/**
 * @brief Set the bounding box of an arc edge
 * @details The box spans the two endpoints plus any of the four cardinal
 * extreme points of the circle that fall within the arc's angular span
 */
static inline void
arc_set_bbox(Edge *e)
{
  double xmin = Min(e->x1, e->x2), xmax = Max(e->x1, e->x2);
  double ymin = Min(e->y1, e->y2), ymax = Max(e->y1, e->y2);
  const double ang[4] = {0.0, M_PI_2, M_PI, -M_PI_2};
  const double ex[4] = {e->cx + e->radius, e->cx, e->cx - e->radius, e->cx};
  const double ey[4] = {e->cy, e->cy + e->radius, e->cy, e->cy - e->radius};
  for (int k = 0; k < 4; k++)
    if (arc_contains_angle(e, ang[k]))
    {
      if (ex[k] < xmin) xmin = ex[k];
      if (ex[k] > xmax) xmax = ex[k];
      if (ey[k] < ymin) ymin = ey[k];
      if (ey[k] > ymax) ymax = ey[k];
    }
  e->xmin = xmin; e->xmax = xmax; e->ymin = ymin; e->ymax = ymax;
  return;
}

/**
 * @brief Return true if a point is located on an arc edge
 */
static inline bool
point_on_arc(double px, double py, const Edge *e)
{
  /* The radial distance is read from the coordinates of the point and of the
   * centre, so what it misses by is a property of their arithmetic and not of
   * the arc: a few units in the last place of the largest of them. An absolute
   * band asks a point at a projected 6.4e6 to sit on its own circle to a
   * thousandth of what that coordinate can express, and the point the engine
   * itself places on the arc then reads as lying off it. The edge carries the
   * tolerance its own coordinates call for -- the same quantity
   * #point_on_segment reads for a straight edge -- and the floor leaves the
   * band at MEOS_GEOM_TOLERANCE where an edge carries none */
  double d = hypot(px - e->cx, py - e->cy);
  if (fabs(d - e->radius) > fmax(e->tol, MEOS_GEOM_TOLERANCE))
    return false;
  return arc_contains_angle(e, atan2(py - e->cy, px - e->cx));
}

/**
 * @brief Return the trajectory parameters at which a trajectory segment
 * intersects an arc edge
 * @details Solves |A + t*R - C|^2 = r^2 for the trajectory parameter t in
 * [0,1], keeping only the roots whose point lies within the arc's angular
 * span. A straight segment meets a circle in at most two points, so the
 * result is never an overlap
 * @param[in] ax,ay Coordinates of the start of the trajectory segment
 * @param[in] rx,ry Vector of the trajectory segment
 * @param[in] e Arc edge
 * @param[out] out Accepted trajectory parameters, ordered as found
 * @return Number of accepted parameters (0, 1, or 2)
 */
static inline int
arcsegm_intersect(double ax, double ay, double rx, double ry, const Edge *e,
  double out[2])
{
  double aa = rx * rx + ry * ry;
  /* Degenerate (zero-length) trajectory segment */
  if (aa < MEOS_GEOM_TOLERANCE)
    return 0;

  double wx = ax - e->cx, wy = ay - e->cy;
  double bb = 2 * (wx * rx + wy * ry);
  double cc = wx * wx + wy * wy - e->radius * e->radius;
  double disc = bb * bb - 4 * aa * cc;
  /* The discriminant is the difference of two quantities of like size, so a
   * segment touching the circle reaches zero only to their rounding. Reading
   * it against that rounding rather than against zero keeps a touch one root:
   * against zero it becomes two roots a square root of the rounding apart,
   * which is a node pair far enough apart to be taken for two, and the sliver
   * between them is not a piece of any boundary.
   * Both terms VANISH where the segment starts on the circle and runs
   * tangent to it there, which is what the offset of an edge leaving a vertex
   * does against the round cap about that vertex, so a rounding read from them
   * alone degenerates to zero and protects nothing. The quantities they are
   * differences OF do not vanish: cc subtracts two of size radius squared, and
   * bb is twice a product of size w by the segment direction */
  double eps = 1e-14 * (fabs(bb * bb) + fabs(4 * aa * cc) +
    aa * (wx * wx + wy * wy + e->radius * e->radius));
  if (disc < -eps)
    return 0;
  if (disc <= eps)
    disc = 0;

  double sq = sqrt(disc);
  double roots[2];
  int nroots = 0;
  roots[nroots++] = (-bb - sq) / (2 * aa);
  /* Distinct second root only when the line is not tangent */
  if (sq > MEOS_GEOM_TOLERANCE)
    roots[nroots++] = (-bb + sq) / (2 * aa);

  int n = 0;
  for (int k = 0; k < nroots; k++)
  {
    double t = roots[k];
    if (t < -MEOS_GEOM_TOLERANCE || t > 1 + MEOS_GEOM_TOLERANCE)
      continue;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    double px = ax + t * rx, py = ay + t * ry;
    if (arc_contains_angle(e, atan2(py - e->cy, px - e->cx)))
      out[n++] = t;
  }
  return n;
}

/**
 * @brief Return where along an arc a point of it stands, as a fraction of the
 * arc's span
 * @details An arc is walked by its ANGLE, so the parameter of a point is how
 * far its direction from the centre has turned from the arc's start, over how
 * far the arc turns in all. The turn is read in the arc's own sense, so a
 * clockwise arc grows its parameter clockwise
 */
static inline double
arc_point_parameter(const Edge *e, double x, double y)
{
  double phi = atan2(y - e->cy, x - e->cx);
  double sweep = arc_span_sweep(e->theta0, e->theta1, e->ccw);
  if (sweep < MEOS_GEOM_TOLERANCE)
    return 0.0;
  double off = arc_span_offset(e->theta0, e->ccw, phi);
  double t = off / sweep;
  /* A point the arithmetic puts just OUTSIDE the span belongs to whichever end
   * it stands nearer, and one a rounding step BEFORE the start has turned
   * nearly the whole circle rather than none of it */
  if (t > 1.0 && (2 * M_PI - off) < (off - sweep))
    return 0.0;
  return (t < 0.0) ? 0.0 : ((t > 1.0) ? 1.0 : t);
}

/**
 * @brief Return the point standing at a fraction along an arc's span
 */
static inline void
arc_parameter_point(const Edge *e, double t, double *x, double *y)
{
  double sweep = arc_span_sweep(e->theta0, e->theta1, e->ccw);
  double phi = e->ccw ? e->theta0 + t * sweep : e->theta0 - t * sweep;
  *x = e->cx + e->radius * cos(phi);
  *y = e->cy + e->radius * sin(phi);
  return;
}

/**
 * @brief Return the points at which two circular arc edges meet, and whether
 * they are arcs of ONE circle
 * @details The supporting circles of two arcs meet on their radical line at
 * `a = (d^2 + r1^2 - r2^2) / (2 d)` from the first centre, at a half-chord
 * `h = sqrt(r1^2 - a^2)` off the centre line, giving at most two candidate
 * points; a candidate is a genuine arc intersection only when it lies within
 * the angular span of both arcs, tested with #arc_contains_angle. Concentric
 * arcs of equal radius lie on the same circle: they meet iff their angular
 * spans share an endpoint.
 */
static inline int
arcarc_intersect_points(const Edge *e1, const Edge *e2, double ix[2],
  double iy[2], bool *same_circle)
{
  double dx = e2->cx - e1->cx, dy = e2->cy - e1->cy;
  double d = hypot(dx, dy);
  double r1 = e1->radius, r2 = e2->radius;
  if (same_circle)
    *same_circle = false;

  /* Concentric supporting circles */
  if (d < MEOS_GEOM_TOLERANCE)
  {
    if (fabs(r1 - r2) > MEOS_GEOM_TOLERANCE)
      return 0;
    /* Same circle: the arcs run along one another wherever their spans do,
     * which is a shared stretch rather than a pair of points */
    if (same_circle)
      *same_circle = arc_contains_angle(e2, e1->theta0) ||
        arc_contains_angle(e2, e1->theta1) ||
        arc_contains_angle(e1, e2->theta0) ||
        arc_contains_angle(e1, e2->theta1);
    return 0;
  }
  /* Circles too far apart or one strictly inside the other */
  if (d > r1 + r2 + MEOS_GEOM_TOLERANCE || d < fabs(r1 - r2) - MEOS_GEOM_TOLERANCE)
    return 0;

  double a = (d * d + r1 * r1 - r2 * r2) / (2 * d);
  double h2 = r1 * r1 - a * a;
  if (h2 < 0)
    h2 = 0;
  double h = sqrt(h2);
  double ux = dx / d, uy = dy / d;         /* Unit vector from c1 to c2 */
  double mx = e1->cx + a * ux, my = e1->cy + a * uy; /* Foot on the centre line */

  /* Candidate points m +/- h * perp(u), kept when both spans hold them */
  int n = 0;
  for (int k = 0; k < 2; k++)
  {
    double px = mx + (k ? h : -h) * (-uy);
    double py = my + (k ? h : -h) * ux;
    if (arc_contains_angle(e1, atan2(py - e1->cy, px - e1->cx)) &&
        arc_contains_angle(e2, atan2(py - e2->cy, px - e2->cx)))
    {
      ix[n] = px; iy[n] = py; n++;
    }
    /* A tangency has a single candidate point */
    if (h < MEOS_GEOM_TOLERANCE)
      break;
  }
  return n;
}

/**
 * @brief Return true if two circular arc edges intersect
 * @details The points they meet at answer it, together with the case of two
 * arcs of ONE circle, which share a stretch rather than a point
 */
static inline bool
arcarc_intersect(const Edge *e1, const Edge *e2)
{
  double ix[2], iy[2];
  bool same_circle;
  int n = arcarc_intersect_points(e1, e2, ix, iy, &same_circle);
  return (n > 0 || same_circle);
}

/*****************************************************************************
 * Compute the intervals in [0,1] resulting from the intersection of a
 * trajectory segment and an array of edges obtained from a (collection of)
 * polygon/line/point geometries
 *****************************************************************************/

/**
 * @brief Return the distance within which two coordinates of this size are the
 * same point
 * @details The points a native implementation classifies are CONSTRUCTED: the
 * midpoint of an edge, the point at a parameter along it, the intersection of
 * two edges. None of them lands exactly where it should, and the distance it
 * misses by is not a property of the geometry but of the arithmetic -- a few
 * units in the last place of the largest coordinate involved. A tolerance
 * meant to absorb that error is therefore that rounding unit, plus the
 * absolute distance below which the implementation calls two points equal.
 */
static inline double
coordinate_tolerance(double c1, double c2)
{
  return MEOS_GEOM_TOLERANCE + 4.0 * DBL_EPSILON * fmax(fabs(c1), fabs(c2));
}

/**
 * @brief Set the tolerance the coordinates of an edge call for
 * @note Read from the bounding box, so an arc is covered by the extent it
 * occupies rather than by its two endpoints. Call once the box is set.
 */
static inline void
edge_set_tolerance(Edge *e)
{
  e->tol = fmax(coordinate_tolerance(e->xmin, e->xmax),
    coordinate_tolerance(e->ymin, e->ymax));
}

/**
 * @brief Return true if a point lies within a given distance of a segment
 * @details The tolerance is a DISTANCE. The cross and dot products below are
 * areas, so each is compared against that distance multiplied by the size of
 * the segment, which is what makes the test read as "the point lies within the
 * tolerance of the segment" at every scale.
 * @note Comparing an area against a distance is what this replaces, and it
 * fails in both directions: dividing through, the effective perpendicular
 * band was the tolerance divided by the length of the segment, so it closed
 * below the representable distance where coordinates are large and opened to
 * swallow the whole figure where the segment is short. A polygon then failed
 * to cover itself -- 360 of the 931 h3 cells of the tbl_th3index fixture, and
 * every one of them at a resolution finer than about 0.03 degrees.
 * @param[in] px,py Point
 * @param[in] x1,y1,x2,y2 Endpoints of the segment
 * @param[in] tol Distance within which the point counts as lying on it, which
 * an #Edge carries precomputed in its @p tol member
 */
static inline bool
point_on_segment_within(double px, double py, double x1, double y1, double x2,
  double y2, double tol)
{
  /* Fast bounding-box rejection, which is where all but a few calls end */
  if ((px < fmin(x1, x2) - tol) || (px > fmax(x1, x2) + tol) ||
      (py < fmin(y1, y2) - tol) || (py > fmax(y1, y2) + tol))
    return false;

  /* Vectors AP and AB */
  double apx = px - x1;
  double apy = py - y1;
  double abx = x2 - x1;
  double aby = y2 - y1;

  /* The tolerance as the area it bounds over a segment of this size */
  double areatol = tol * (fabs(abx) + fabs(aby));

  /* Collinearity check via cross product */
  double cross = apx * aby - apy * abx;
  if (fabs(cross) > areatol)
    return false;

  /* Projection check via dot product */
  double dot = apx * abx + apy * aby;
  if (dot < -areatol)
    return false;

  /* Check if P lies between A and B */
  double ab2 = abx * abx + aby * aby;
  if (dot > ab2 + areatol)
    return false;
  return true;
}

/**
 * @brief Return true if a point is located on a segment
 * @details For a caller holding an #Edge, #point_on_segment_within takes the
 * tolerance the edge already carries instead of deriving it again.
 */
static inline bool
point_on_segment(double px, double py, double x1, double y1, double x2,
  double y2)
{
  return point_on_segment_within(px, py, x1, y1, x2, y2,
    fmax(coordinate_tolerance(x1, x2), coordinate_tolerance(y1, y2)));
}

#endif /* __GEO_FUNCS_H__ */
