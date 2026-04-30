/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Universite libre de Bruxelles and MobilityDB
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
 * @brief Fast 2D/3D temporal point clipping against 2D geometries
 * @details Support (multi)point, (multi)line, triangle, (multi)polygons with
 * holes and islands inside holes (recursively), and collection of the above
 * @note Avoid processing in GEOS to improve performance
 */

/* C */
#include <math.h>
/* PostgreSQL */
#include "postgres.h"
#include <utils/float.h>
/* PostGIS */
#include "liblwgeom.h"
#include "liblwgeom_internal.h"
/* MEOS */
#include "meos.h"
#include "meos_internal_geo.h"
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/temporal_restrict.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"

/* Minimum number of edges to use an R-tree index in order to compensate the
 * overhead of the tree construction and destruction */
#define RTREE_MIN_NUMBER_ELEMS 100

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/* Global static arrays for accumulating the results of the clipping process */
static MeosArray *events = NULL;
static MeosArray *intervals = NULL;
static MeosArray *periods = NULL;
static int *rtree_results = NULL;

/**
 * @brief Enumeration defining the edge types 
 */
typedef enum
{
  EDGE_POINT = 0,
  EDGE_LINE,
  EDGE_POLY
} EdgeType;

/**
 * @brief Structure keeping a geometry edge
 */
typedef struct
{
  double x1, y1, x2, y2;         /**< Coordinates of the start/end 2D points */
  double xmin, ymin, xmax, ymax; /**< Precomputed bounding box of the edge */
  double dx, dy, length;         /**< Precomputed dx, dy, and length */
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
 * - Overlap must satisfy: t1 - t0 > FP_TOLERANCE
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
  if (fabs(rxs) < FP_TOLERANCE)
  {
    /* Is point C aligned with segment AB? */
    double qpxr = qpx * ry - qpy * rx;
    /* If qpxr != 0: parallel, if qpxr == 0: collinear */
    if (fabs(qpxr) > FP_TOLERANCE)
      return res;

    /* Collinear case */
    double r2 = rx * rx + ry * ry;
    if (r2 < FP_TOLERANCE)
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

    if (fabs(t1 - t0) < FP_TOLERANCE)
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

  if (t < -FP_TOLERANCE || t > 1 + FP_TOLERANCE ||
      u < -FP_TOLERANCE || u > 1 + FP_TOLERANCE)
    return res;

  /* Clamp values */
  if (fabs(t) < FP_TOLERANCE) t = 0;
  if (fabs(t - 1) < FP_TOLERANCE) t = 1;

  res.type = INTERSECT_POINT;
  res.t0 = t;
  return res;
}

/*****************************************************************************
 * Compute the intervals in [0,1] resulting from the intersection of a
 * trajectory segment and an array of edges obtained from a (collection of)
 * polygon/line/point geometries
 *****************************************************************************/

/**
 * @brief Return true if a point is located on a segment
 */
static bool
point_on_segment(double px, double py, double x1, double y1, double x2,
  double y2)
{
  /* Vectors AP and AB */
  double apx = px - x1;
  double apy = py - y1;
  double abx = x2 - x1;
  double aby = y2 - y1;

  /* Fast bounding-box rejection */
  if ((px < fmin(x1, x2) - FP_TOLERANCE) ||
      (px > fmax(x1, x2) + FP_TOLERANCE) ||
      (py < fmin(y1, y2) - FP_TOLERANCE) ||
      (py > fmax(y1, y2) + FP_TOLERANCE))
    return false;

  /* Collinearity check via cross product */
  double cross = apx * aby - apy * abx;
  if (fabs(cross) > FP_TOLERANCE)
    return false;

  /* Projection check via dot product */
  double dot = apx * abx + apy * aby;
  if (dot < -FP_TOLERANCE)
    return false;

  /* Check if P lies between A and B */
  double ab2 = abx * abx + aby * aby;
  if (dot > ab2 + FP_TOLERANCE)
    return false;
  return true;
}

/**
 * @brief Return true if a point is located in a polygon 
 */
static inline int
point_in_polygon(double x, double y, Edge **edges, int nedges)
{
  int inside = 0;
  for (int i = 0; i < nedges; i++)
  {
    const Edge *restrict e = edges[i];

    const double dx  = e->dx;
    const double dy  = e->dy;
    const double x1  = e->x1;
    const double y1  = e->y1;

    const double dxp = x - x1;
    const double dyp = y - y1;

    /* Boundary check */
    const double cross = dx * dyp - dy * dxp;
    if (fabs(cross) < FP_TOLERANCE)
    {
      const double dot = dxp * dx + dyp * dy;
      if (dot >= -FP_TOLERANCE && dot <= (e->length) + FP_TOLERANCE)
        return 1;
    }

    /* Ray casting */
    if ((y1 > y) != ((y1 + dy) > y))
    {
      const double rhs = dx * dyp;
      const double lhs = dxp * dy;
      inside ^= ((dy > 0) ? (rhs > lhs) : (rhs < lhs));
    }
  }
  return inside;
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of point edges
 */
static void
intervals_from_points(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  /* Segment vector */
  double dx = b->x - a->x;
  double dy = b->y - a->y;

  /* Improve performance by removing the division inside the loop */
  bool use_x = fabs(dx) >= fabs(dy);
  double inv = use_x ?
    ((fabs(dx) > FP_TOLERANCE) ? 1.0 / dx : 0.0) :
    ((fabs(dy) > FP_TOLERANCE) ? 1.0 / dy : 0.0);

  if (inv == 0.0)
    return;
  
  /* Iterate through the points */
  for (int i = 0; i < nedges; i++)
  {
    Edge *e = edges[i]; 
    /* Iterate only for the points */
    if (e->etype != EDGE_POINT)
      continue;
    // assert(e->x1 == e->x2 && e->y1 == e->y2);

    /* Solve parameter t */
    double t = use_x ? (e->x1 - a->x) * inv : (e->y1 - a->y) * inv;
    /* Check bounds */
    if (t < -FP_TOLERANCE || t > 1.0 + FP_TOLERANCE)
      continue;

    /* Reconstruct point and add interval */
    double x = a->x + t * dx;
    double y = a->y + t * dy;
    if (fabs(x - e->x1) < FP_TOLERANCE && fabs(y - e->y1) < FP_TOLERANCE)
    {
      Span in;
      span_set(Float8GetDatum(t), Float8GetDatum(t), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }
  return;
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of linear or point edges
 */
static void
intervals_from_lines(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;

  /* Segment bounding box */
  const double seg_xmin = FP_MIN(ax, bx);
  const double seg_xmax = FP_MAX(ax, bx);
  const double seg_ymin = FP_MIN(ay, by);
  const double seg_ymax = FP_MAX(ay, by);
  /* Segment vector */
  const double rx = bx - ax;  
  const double ry = by - ay;  

  bool has_intersection = false;
  Span in;

  /* Iterate through the lines */
  for (int i = 0; i < nedges; i++)
  {
    Edge *e = edges[i];
    /* Iterate only for the line edges */
    if (e->etype != EDGE_LINE)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    /* Compute the intersection */
    IntersectResult r = linesegm_intersect(ax, ay, rx, ry,
      e->x1, e->y1, e->x2, e->y2);
    /* If there is no intersection  */
    if (r.type == INTERSECT_NONE)
      continue;

    /* Intersection found: compute the interval */
    has_intersection = true;
    if (r.type == INTERSECT_POINT)
      span_set(Float8GetDatum(r.t0), Float8GetDatum(r.t0), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
    else
      span_set(Float8GetDatum(r.t0), Float8GetDatum(r.t1), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
    meos_array_add(intervals, &in);
  }

  /* Full collinear segment */
  if (! has_intersection)
  {
    /* Test midpoint */
    double mx = (ax + bx) * 0.5;
    double my = (ay + by) * 0.5;
    for (int i = 0; i < nedges; i++)
    {
      Edge *e = edges[i];
      /* Iterate only for the lines edges */
      if (e->etype != EDGE_LINE)
        continue;

      /* Fast bbox check first */
      if (mx < e->xmin - FP_TOLERANCE || mx > e->xmax + FP_TOLERANCE ||
          my < e->ymin - FP_TOLERANCE || my > e->ymax + FP_TOLERANCE)
        continue;

      if (point_on_segment(mx, my, e->x1, e->y1, e->x2, e->y2))
      {
        span_set(Float8GetDatum(0.0), Float8GetDatum(1.0), true, true,
          T_FLOAT8, T_FLOATSPAN, &in);
        meos_array_add(intervals, &in);
        return;
      }
    }
  }
  return;
}

/**
 * @brief Comparison function for sorting float8 values
 */
static int
float8_qsort_cmp(const void *a1, const void *a2)
{
  double diff = *(const double *)a1 - *(const double *)a2;
  return (diff > 0) - (diff < 0);
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of polygon edges
 * @details For the #point_in_polygon ray-casting we must use ALL edges, not
 * only those filtered by the R-tree, otherwise the even-odd containment test
 * breaks
 */
static void
intervals_from_polygons(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges, Edge **all_edges, int all_nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  /* Reset event array */
  events->count = 0;

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;

  /* Segment bounding box */
  const double seg_xmin = FP_MIN(ax, bx);
  const double seg_xmax = FP_MAX(ax, bx);
  const double seg_ymin = FP_MIN(ay, by);
  const double seg_ymax = FP_MAX(ay, by);
  /* Segment vector */
  const double rx = bx - ax;
  const double ry = by - ay;

  /* Check whether any polygon edges exist using the full edge array */
  bool has_polys = false;
  for (int i = 0; i < all_nedges; i++)
  {
    if (all_edges[i]->etype == EDGE_POLY)
    {
      has_polys = true;
      break;
    }
  }
  /* If no polygon edges have been found, we do not continue */
  if (! has_polys)
    return;

  /* Collect all intersection parameters from the (possibly filtered) edges */
  for (int i = 0; i < nedges; i++)
  {
    Edge *e = edges[i];
    /* Iterate only for the polygon edges */
    if (e->etype != EDGE_POLY)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    /* Compute the intersection */
    IntersectResult r = linesegm_intersect(ax, ay, rx, ry,
      e->x1, e->y1, e->x2, e->y2);
    if (r.type == INTERSECT_POINT)
    {
      double t = r.t0;
      if (t >= -FP_TOLERANCE && t <= 1.0 + FP_TOLERANCE)
        meos_array_add(events, &t);
    }
  }

  /* Add endpoints */
  double t0 = 0.0, t1 = 1.0;
  meos_array_add(events, &t0);
  meos_array_add(events, &t1);

  /* Sort */
  qsort(events->elems, events->count, sizeof(double), float8_qsort_cmp);

  /* Deduplicate */
  int newcount = 0;
  double *evtarr = (double *) events->elems;
  for (int i = 0; i < (int) events->count; i++)
  {
    if (i == 0 ||
        fabs(evtarr[i] - evtarr[newcount - 1]) > FP_TOLERANCE)
    {
      evtarr[newcount++] = evtarr[i];
    }
  }
  events->count = newcount;

  /* Build intervals using midpoint test */
  for (int i = 0; i < (int) events->count - 1; i++)
  {
    double ta = evtarr[i];
    double tb = evtarr[i + 1];
    if (tb - ta <= FP_TOLERANCE)
      continue;

    /* Midpoint test */
    double tm = (ta + tb) * 0.5;
    double x = ax + tm * rx;
    double y = ay + tm * ry;
    if (point_in_polygon(x, y, all_edges, all_nedges))
    {
      Span in;
      span_set(Float8GetDatum(ta), Float8GetDatum(tb), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }
  return;
}

/*****************************************************************************/

/**
 * @brief Return true if a trajectory point intersects with an array of point
 * and linear edges
 */
static bool
point_inter_points_lines(const POINT2D *a, Edge **edges, int nedges)
{
  assert(a); assert(edges); assert(nedges >= 0);

  const double ax = a->x, ay = a->y;

  /* Iterate only through the point and linear edges */
  for (int i = 0; i < nedges; i++)
  {
    Edge *e = edges[i];
    if (e->etype == EDGE_POINT)
    {
      if (fabs(e->x1 - ax) < FP_TOLERANCE && fabs(e->y1 - ay) < FP_TOLERANCE)
        return true;
    }
    else if (e->etype == EDGE_LINE)
    {
      if (point_on_segment(ax, ay, e->x1, e->y1, e->x2, e->y2))
        return true;
    }
  }
  return false;
}

/*****************************************************************************
 * Clip a temporal geometry point
 *****************************************************************************/

/**
 * @brief Clip a 2D/3D trajectory with linear interpolation with respect to a
 * geometry
 * @param[in] inst Temporal sequence
 * @param[in] edges Array of geometry edges
 * @param[in] nedges Number of edges in the array
 * @param[in] rtree R-tree for the edges, may be `NULL` if no index is used
 * @param[in] cand_edges Edge array buffer of size `nedges` for storing the
 * result of an R-tree look up, may be `NULL` if no index is used
 */
static void
tpointinst_clip_edges(const TInstant *inst, Edge **edges, int nedges,
  RTree *rtree, Edge **cand_edges)
{
  assert(inst); assert(edges); assert(nedges > 0);
  assert(inst->temptype == T_TGEOMPOINT);

  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst));

  /* Edges to process: all of them (default) or those filtered by an R-tree */
  Edge **sel_edges = edges;
  int sel_nedges = nedges;
  bool use_index = (rtree != NULL && cand_edges != NULL);
  if (use_index)
  {
    /* Build the segment bounding box */
    STBox query;
    int32_t srid = tspatial_srid((Temporal *) inst);
    stbox_set(true, false, false, srid, a->x, a->x, a->y, a->y, 0, 0, NULL,
      &query);
    /* Query the R-tree */
    int cand_nedges = rtree_search(rtree, RTREE_OVERLAPS, &query, &rtree_results);

    /* Convert the result of an R-tree look up into an edge pointer array */
    for (int j = 0; j < cand_nedges; j++)
      cand_edges[j] = edges[rtree_results[j]];
    sel_edges = cand_edges;
    sel_nedges = cand_nedges;
  }

  /* Reset the interval array */
  intervals->count = 0;
  /* Compute the intervals for the points, lines, and polygon edges */
  bool found = point_inter_points_lines(a, sel_edges, sel_nedges);
  if (! found)
  {
    intervals_from_polygons(a, a, sel_edges, sel_nedges, edges, nedges);
    if (intervals->count == 0)
      return;
  }
  
  /* Generate the instantantaneous span */
  Span s;
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
  meos_array_add(periods, &s);
  return;
}

/**
 * @brief Clip a 2D/3D trajectory with linear interpolation with respect to a
 * geometry
 * @param[in] seq Temporal sequence
 * @param[in] edges Array of geometry edges
 * @param[in] nedges Number of edges in the array
 * @param[in] rtree R-tree for the edges, may be `NULL` if no index is used
 * @param[in] cand_edges Edge array buffer of size `nedges` for storing the
 * result of an R-tree look up, may be `NULL` if no index is used
 */
static void
tpointseq_clip_edges(const TSequence *seq, Edge **edges, int nedges,
  RTree *rtree, Edge **cand_edges)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
    return tpointinst_clip_edges(TSEQUENCE_INST_N(seq, 0), edges, nedges,
      rtree, cand_edges);

  bool use_index = (rtree != NULL && cand_edges != NULL);
  int32_t srid = tspatial_srid((Temporal *) seq);

  /* Initialize variables for the loop */
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst1));
  bool lower_inc = seq->period.lower_inc;
  /* Edges to process: either all of them or those filtered by an R-tree */
  Edge **sel_edges = edges;
  int sel_nedges = nedges;
  /* Loop for each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *b = DATUM_POINT2D_P(tinstant_value_p(inst2));
    bool upper_inc = (i < seq->count - 1) ? false : seq->period.upper_inc;

    /* Filter the edges to process by a R-tree, if any */
    if (use_index)
    {
      /* Build the segment bounding box */
      STBox query;
      stbox_set(true, false, false, srid, FP_MIN(a->x, b->x),
        FP_MAX(a->x, b->x), FP_MIN(a->y, b->y), FP_MAX(a->y, b->y),
        0, 0, NULL, &query);
      /* Query the R-tree */
      int cand_nedges = rtree_search(rtree, RTREE_OVERLAPS, &query, &rtree_results);

      /* Convert the result of an R-tree look up into an edge pointer array */
      for (int j = 0; j < cand_nedges; j++)
        cand_edges[j] = edges[rtree_results[j]];
      sel_edges = cand_edges;
      sel_nedges = cand_nedges;
    }

    /* Reset the interval array */
    intervals->count = 0;
    /* Compute the intervals for the points, lines, and polygon edges */
    intervals_from_points(a, b, sel_edges, sel_nedges);
    intervals_from_lines(a, b, sel_edges, sel_nedges);
    intervals_from_polygons(a, b, sel_edges, sel_nedges, edges, nedges);
    if (intervals->count == 0)
      goto next_segment;

    /* Normalize the intervals */
    int count;
    Span *intervarr = NULL;
    if (intervals->count > 1)
      intervarr = spanarr_normalize(intervals->elems, intervals->count,
        ORDER_NO, &count);
    else
    {
      intervarr = intervals->elems;
      count = 1;
    }

    /* Generate the periods from the float spans taking into account exclusive
     * temporal bounds */
    double duration = (double) (inst2->t - inst1->t);
    for (int j = 0; j < count; j++)
    {
      Span s;
      double lower = DatumGetFloat8(intervarr[j].lower);
      double upper = DatumGetFloat8(intervarr[j].upper);
      if (fabs(upper - lower) < FP_TOLERANCE)
      {
        /* Remove intersection points on exclusive lower and upper bounds */
        if (! lower_inc && fabs(lower) < FP_TOLERANCE &&
            fabs(upper) < FP_TOLERANCE)
          continue;
        if (! upper_inc && fabs(lower - 1.0) < FP_TOLERANCE &&
            fabs(upper - 1.0) < FP_TOLERANCE)
          continue;

        /* Interpolate only if 0 < lower/upper < 1 */
        TimestampTz t = (lower == 0.0) ?
          inst1->t : inst1->t + (TimestampTz) (duration * lower);
        span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
          T_TIMESTAMPTZ, T_TSTZSPAN, &s);
        meos_array_add(periods, &s);
      }
      else
      {
        TimestampTz t1 = (lower == 0.0) ?
          inst1->t : inst1->t + (TimestampTz) (duration * lower);
        TimestampTz t2 = (upper == 1.0) ?
          inst2->t : inst1->t + (TimestampTz) (duration * upper);
        span_set(TimestampTzGetDatum(t1), TimestampTzGetDatum(t2), true, true,
          T_TIMESTAMPTZ, T_TSTZSPAN, &s);
        meos_array_add(periods, &s);
      }
    }
    
next_segment:
    /* Prepare the next iteration */
    if (intervarr && intervals->count > 1)
      pfree(intervarr);
    inst1 = inst2;
    a = b;
  }
  return;
}

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
  emit_ring_edges(line->points, edges, EDGE_LINE);
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
    emit_ring_edges(poly->rings[r], edges, EDGE_POLY);
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
  emit_ring_edges(tri->points, edges, EDGE_POLY);
  return;
}

/**
 * @brief Return the edges of a geometry in a dynamic array (iterator)
 */
static void
geom_extract_edges_iter(const LWGEOM *geom, MeosArray *edges)
{
  if (! geom)
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

    case COLLECTIONTYPE:
    {
      const LWCOLLECTION *col = (const LWCOLLECTION *) geom;
      for (int i = 0; i < (int) col->ngeoms; i++)
        geom_extract_edges_iter(col->geoms[i], edges);
      break;
    }

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
static MeosArray *
geom_extract_edges(const LWGEOM *geom)
{
  MeosArray *edges = meos_array_create(sizeof(Edge));
  geom_extract_edges_iter(geom, edges);
  return edges;
}

/**
 * @brief Build an R-tree from edges
 */
static RTree *
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

/*****************************************************************************/

/**
 * @brief Return the temporal intersection/intersects of a temporal  geometric
 * point with linear interpolation and a 2D geometry
 * @details The temporal geometric point may be in 2D or 3D and the Z dimension 
 * is also computed
 * @note For performance reasons we avoid the call to ST_Intersection
 * which delegates the computation to GEOS
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
Temporal *
tpoint_linear_inter_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool clip)
{
  assert(temp); assert(gs); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  assert(! gserialized_is_empty(gs)); 

  /* Bounding box test */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  /* Non-empty geometries have a bounding box */
  geo_set_stbox(gs, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
  {
    if (clip)
      return NULL;
    SpanSet *ss = temporal_time(temp);
    Temporal *result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
    return result;
  }

  /* Initialize result to NULL to quickly clean up and return */
  Temporal *result = NULL;

  /* Extract the edges */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  MeosArray *edges = geom_extract_edges(geom);
  lwgeom_free(geom);
  /* Create an array of edge pointers */
  Edge **edge_ptrs = palloc(sizeof(Edge *) * edges->count);
  /* Transform the edge array into an edge pointer array */
  for (int i = 0; i < (int) edges->count; i++)
    edge_ptrs[i] = (Edge *) meos_array_get(edges, i);

  /* R-tree pointer: A NULL pointer passed to function #tpointseq_clip_edges
   * means that no index is used */
  RTree *rtree = NULL;
  /* Array of edge pointers for storing the edges filtered by the R-tree */
  Edge **cand_edges = NULL;
  /* Minimum number of edges to use an R-tree index in order to compensate the
   * overhead of the tree construction and destruction */
  if (edges->count > RTREE_MIN_NUMBER_ELEMS)
  {
    /* Build R-tree */
    int32_t srid = tspatial_srid(temp);
    rtree = build_edge_rtree(edges->elems, edges->count, srid);
    if (! rtree)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      return NULL;
    }
    /* Array of edge pointers for storing the edges filtered by the R-tree */
    cand_edges = palloc(sizeof(Edge *) * edges->count);
    if (! cand_edges)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      rtree_free(rtree); 
      return NULL;
    }
    /* Array of integer pointers for storing the results of an R-tree search */
    rtree_results = palloc(sizeof(int) * edges->count);
    if (! rtree_results)
    {
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      rtree_free(rtree); 
      return NULL;
    }
  }

  /* Initialize the static global arrays accumulating the clipping results */
  events = meos_array_create(sizeof(double));
  intervals = meos_array_create(sizeof(Span));
  periods = meos_array_create(sizeof(Span));
  
  /* Collect the clipping periods */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tpointinst_clip_edges((TInstant *) temp, edge_ptrs, edges->count,
        rtree, cand_edges);
      break;
    case TSEQUENCE:
      tpointseq_clip_edges((TSequence *) temp, edge_ptrs, edges->count,
        rtree, cand_edges);
      break;
    default: /* TSEQUENCESET */
    {
      /* Loop for each segment */
      TSequenceSet *ss = (TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        tpointseq_clip_edges(TSEQUENCESET_SEQ_N(ss, i), edge_ptrs,
          edges->count, rtree, cand_edges);
    }
  }

  SpanSet *ss;
  if (periods->count == 0)
  {
    if (clip)
      goto cleanup_return;
    ss = temporal_time(temp);
    result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
  }
  else
  {
    ss = spanset_make_exp(periods->elems, periods->count,
      periods->count, NORMALIZE, ORDER);
    if (clip)
      result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    else
    {
      SpanSet *ss1 = temporal_time(temp);
      Temporal *temp1 = (Temporal *) tsequenceset_from_base_tstzspanset(
        BoolGetDatum(false), T_TBOOL, ss1, STEP);
      Temporal *temp2 = temporal_restrict_tstzspanset(temp1, ss, REST_MINUS);
      if (temp2)
      {
        Temporal *temp3 = (Temporal *) tsequenceset_from_base_tstzspanset(
          BoolGetDatum(true), T_TBOOL, ss, STEP);
        result = temporal_merge(temp2, temp3);
        pfree(temp2); pfree(temp3);
      }
      else
        result = (Temporal *) tsequenceset_from_base_tstzspanset(
          BoolGetDatum(true), T_TBOOL, ss1, STEP);
      pfree(ss1); pfree(temp1);
    }
    pfree(ss);
  }
  
  /* Clean up and return */
cleanup_return:
  if (edges->count > RTREE_MIN_NUMBER_ELEMS)
  {
    rtree_free(rtree); pfree(cand_edges); pfree(rtree_results);
  }
  meos_array_destroy(events, false);
  meos_array_destroy(intervals, false);
  meos_array_destroy(periods, false);
  meos_array_destroy(edges, false); pfree(edge_ptrs);
  return result;  
}

/**
 * @brief Return a temporal geometric point with linear interpolation 
 * restricted to a 2D geometry
 * @details The temporal point may be 2D or 3D and the Z dimension is also
 * computed
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
Temporal *
tpoint_linear_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool atfunc)
{
  assert(temp); assert(gs); assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));

  /* Compute atGeometry for the temporal point */
  Temporal *result_at = tpoint_linear_inter_geom(temp, gs, true);

  /* If "at" restriction, return */
  if (atfunc)
    return result_at;

  /* If "minus" restriction, compute the complement wrt time */
  if (! result_at)
    return temporal_copy(temp);

  SpanSet *ss = temporal_time(result_at);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, atfunc);
  pfree(ss); pfree(result_at);
  return result;
}

/*****************************************************************************/