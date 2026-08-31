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
 * @brief Fast 2D/3D temporal point clipping against 2D geometries
 * @details Support (multi)point, (multi)line, triangle, (multi)polygons with
 * holes and islands inside holes (recursively), and collection of the above
 * @note Processing is done natively to improve performance
 */

/* C */
#include <math.h>
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
#include "temporal/span.h"
#include "temporal/temporal.h"
#include "temporal/temporal_restrict.h"
#include "geo/geo_funcs.h"
#include "geo/tgeo.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/postgis_funcs.h"

/* Minimum number of edges to use an R-tree index in order to compensate the
 * overhead of the tree construction and destruction */
#define RTREE_MIN_NUMBER_ELEMS 100

/*****************************************************************************
 * Data structures
 *****************************************************************************/

/* Per-thread arrays for accumulating the results of the clipping process.
 * MEOS_TLS is required: concurrent callers from different threads would
 * otherwise race on these file-scope pointers, causing heap corruption. */
static MEOS_TLS MeosArray *events = NULL;
static MEOS_TLS MeosArray *intervals = NULL;
static MEOS_TLS MeosArray *periods = NULL;
static MEOS_TLS MeosArray *rtree_results = NULL;

/**
 * @brief Return true if a point is located in a polygon 
 */
static inline int
point_in_polygon_impl(double x, double y, Edge **edges, int nedges,
  const RTree *rtree, int32_t srid, double xmax)
{
  int inside = 0;
  /* The height the ray is cast at. A ray at the height of a vertex meets the
   * two edges sharing it in the one point they share, and which of the two
   * owns that crossing is decided by one rule for a segment and another for an
   * arc: the two can both claim it, counting it twice, or both disclaim it,
   * counting it not at all, and either way the parity, and with it the answer,
   * turns over. A boundary point is answered above without casting anything,
   * so the point here lies off the boundary and the ray may be cast at any
   * height near its own: it is moved until no vertex sits on it, which leaves
   * no crossing for two edges to share and no rule to reconcile */
  double ry = y;
  double bump = MEOS_GEOM_TOLERANCE * 4.0;
  for (int attempt = 0; attempt < 8; attempt++)
  {
    bool shared = false;
    inside = 0;
    int n = nedges;
    if (rtree)
    {
      /* Only edges whose bounding box meets the +x ray from (x,ry) can cross
       * it or contain the point; querying the R-tree for those instead of
       * scanning every edge turns the O(nedges) test into
       * O(log nedges + candidates). The even-odd parity is order-independent
       * and every excluded edge lies left of x or off the ray's height, so it
       * can neither cross the +x ray nor contain the point -- the result is
       * identical to the full scan. */
      STBox query;
      double xhi = (x > xmax) ? x : xmax;
      stbox_set(true, false, false, srid, x, xhi, ry, ry, 0, 0, NULL, &query);
      n = rtree_search(rtree, INDEX_OVERLAPS, &query, rtree_results);
    }
    for (int i = 0; i < n && ! shared; i++)
    {
      const Edge *restrict e = rtree ?
        edges[*(int64 *) meos_array_get(rtree_results, i)] : edges[i];

      /* Only polygon boundary edges bound a region. Point, line, and
       * standalone (1D) arc edges are ignored by the even-odd containment
       * test */
      if (e->etype != EDGE_POLYARC && e->etype != EDGE_POLYSEG)
        continue;

      /* An end of this edge sits on the ray, so the crossing there is shared
       * with the edge that follows it round the ring.
       * How near is near enough is read at the scale of the edge: an arc is
       * held within its span to an angular tolerance, which at a radius of
       * r stands for a distance of r times that tolerance, so an end of a
       * large arc reaches the ray from far further off than an end of a
       * segment does */
      double tol = MEOS_GEOM_TOLERANCE;
      if (e->etype == EDGE_POLYARC && e->radius > 1.0)
        tol *= e->radius;
      if (fabs(e->y1 - ry) <= tol || fabs(e->y2 - ry) <= tol)
      {
        shared = true;
        /* The ray has to clear the end by more than that same distance */
        if (bump < 4.0 * tol)
          bump = 4.0 * tol;
        break;
      }

      if (e->etype == EDGE_POLYARC)
      {
        /* Boundary check, which reads the point itself rather than the ray */
        if (point_on_arc(x, y, e))
          return 1;
        /* Cast a ray towards +x. The horizontal line at height ry meets the
         * supporting circle at cx +/- sqrt(r^2 - (ry - cy)^2); flip the parity
         * for each crossing that lies strictly to the right of the point and
         * within the arc's angular span. A ray that only grazes the circle
         * tangentially (h2 ~ 0) does not cross the boundary */
        const double dyc = ry - e->cy;
        const double h2 = e->radius * e->radius - dyc * dyc;
        if (h2 <= MEOS_GEOM_TOLERANCE)
          continue;
        const double h = sqrt(h2);
        const double xhit[2] = {e->cx - h, e->cx + h};
        for (int k = 0; k < 2; k++)
        {
          const double xi = xhit[k];
          if (xi <= x)
            continue;
          const double phi = atan2(dyc, xi - e->cx);
          if (! arc_contains_angle(e, phi))
            continue;
          inside ^= 1;
        }
        continue;
      }

      const double dx  = e->dx;
      const double dy  = e->dy;
      const double x1  = e->x1;
      const double y1  = e->y1;

      const double dxp = x - x1;

      /* Boundary check, which reads the point itself rather than the ray. The
       * test is the one #point_on_segment_within makes, taking the tolerance
       * the edge carries: a cross product is an AREA, so bounding it by a
       * distance reads a point as lying on a segment far from it once the
       * coordinates are large, and reads a point as lying off a short segment
       * it does lie on. A repeated vertex draws an edge of no length, whose
       * cross and dot products are BOTH zero wherever the point is, and the
       * bounding-box rejection the shared test opens with is what keeps such
       * an edge from claiming every point of the plane as its own */
      if (point_on_segment_within(x, y, x1, y1, e->x2, e->y2, e->tol))
        return 1;

      /* Ray casting */
      if ((y1 > ry) != ((y1 + dy) > ry))
      {
        const double rhs = dx * (ry - y1);
        const double lhs = dxp * dy;
        inside ^= ((dy > 0) ? (rhs > lhs) : (rhs < lhs));
      }
    }
    if (! shared)
      return inside;
    /* Move the ray off the vertex it meets and cast it again */
    ry = y + bump;
    bump *= 3.0;
  }
  return inside;
}

/**
 * @brief Return true if a point is located in a polygon, scanning every edge
 */
int
point_in_polygon(double x, double y, Edge **edges, int nedges)
{
  return point_in_polygon_impl(x, y, edges, nedges, NULL, 0, 0.0);
}

/**
 * @brief Return true if a point is located in a polygon, reading the edges the
 * ray from it can meet out of an index
 * @param[in] x,y Point
 * @param[in] edges,nedges Edges the index is built over, in its own order
 * @param[in] rtree Index over the boxes of those edges, or @p NULL to scan
 * @param[in] xmax Greatest x the edges reach, which bounds the ray
 */
int
point_in_polygon_index(double x, double y, Edge **edges, int nedges,
  const RTree *rtree, double xmax)
{
  /* The indexed walk reads its candidates into the per-thread array the clip
   * context otherwise owns. A caller outside that context finds it empty, and
   * an empty one is what the search would read through */
  if (rtree && ! rtree_results)
    rtree_results = meos_array_create(sizeof(int64));
  return point_in_polygon_impl(x, y, edges, nedges, rtree, 0, xmax);
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of point edges
 * @details A segment that does not move carries no direction to solve the
 * parameter along, and meets a point edge exactly where it stands, for the
 * whole of its extent
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
    ((fabs(dx) > MEOS_GEOM_TOLERANCE) ? 1.0 / dx : 0.0) :
    ((fabs(dy) > MEOS_GEOM_TOLERANCE) ? 1.0 / dy : 0.0);

  /* The segment stands still */
  if (inv == 0.0)
  {
    for (int i = 0; i < nedges; i++)
    {
      const Edge *e = edges[i];
      if (e->etype != EDGE_POINT)
        continue;
      if (fabs(a->x - e->x1) < MEOS_GEOM_TOLERANCE &&
          fabs(a->y - e->y1) < MEOS_GEOM_TOLERANCE)
      {
        Span in;
        span_set(Float8GetDatum(0.0), Float8GetDatum(1.0), true, true,
          T_FLOAT8, T_FLOATSPAN, &in);
        meos_array_add(intervals, &in);
      }
    }
    return;
  }

  /* Iterate through the points */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i]; 
    /* Iterate only for the points */
    if (e->etype != EDGE_POINT)
      continue;
    // assert(e->x1 == e->x2 && e->y1 == e->y2);

    /* Solve parameter t */
    double t = use_x ? (e->x1 - a->x) * inv : (e->y1 - a->y) * inv;
    /* Check bounds */
    if (t < -MEOS_GEOM_TOLERANCE || t > 1.0 + MEOS_GEOM_TOLERANCE)
      continue;

    /* Reconstruct point and add interval */
    double x = a->x + t * dx;
    double y = a->y + t * dy;
    if (fabs(x - e->x1) < MEOS_GEOM_TOLERANCE && fabs(y - e->y1) < MEOS_GEOM_TOLERANCE)
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
  const double seg_xmin = Min(ax, bx);
  const double seg_xmax = Max(ax, bx);
  const double seg_ymin = Min(ay, by);
  const double seg_ymax = Max(ay, by);
  /* Segment vector */
  const double rx = bx - ax;  
  const double ry = by - ay;  

  bool has_intersection = false;
  Span in;

  /* Iterate through the lines */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    /* Iterate only for the line edges */
    if (e->etype != EDGE_LINESEG)
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
      const Edge *e = edges[i];
      /* Iterate only for the lines edges */
      if (e->etype != EDGE_LINESEG)
        continue;

      /* Fast bbox check first */
      if (mx < e->xmin - MEOS_GEOM_TOLERANCE || mx > e->xmax + MEOS_GEOM_TOLERANCE ||
          my < e->ymin - MEOS_GEOM_TOLERANCE || my > e->ymax + MEOS_GEOM_TOLERANCE)
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
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of arc edges
 */
static void
intervals_from_arcs(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;
  /* Segment bounding box */
  const double seg_xmin = Min(ax, bx);
  const double seg_xmax = Max(ax, bx);
  const double seg_ymin = Min(ay, by);
  const double seg_ymax = Max(ay, by);
  /* Segment vector */
  const double rx = bx - ax;
  const double ry = by - ay;

  Span in;
  /* Iterate through the arc edges */
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype != EDGE_LINEARC)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    /* Compute the intersection: at most two point crossings */
    double t[2];
    int n = arcsegm_intersect(ax, ay, rx, ry, e, t);
    for (int k = 0; k < n; k++)
    {
      span_set(Float8GetDatum(t[k]), Float8GetDatum(t[k]), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
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
 * @brief Return true if a point lies on the boundary of a polygonal component
 * @details Only the polygon boundary edges are considered, straight
 * (#EDGE_POLYSEG) and circular (#EDGE_POLYARC). The candidate array filtered by
 * the box of a segment suffices for a point lying on that segment: a boundary
 * edge through such a point has a box meeting the segment box, so it is in the
 * candidates
 */
static bool
point_on_poly_boundary(double px, double py, Edge **edges, int nedges)
{
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype == EDGE_POLYSEG)
    {
      if (point_on_segment(px, py, e->x1, e->y1, e->x2, e->y2))
        return true;
    }
    else if (e->etype == EDGE_POLYARC)
    {
      if (point_on_arc(px, py, e))
        return true;
    }
  }
  return false;
}

/**
 * @brief Compute the intersection intervals of a trajectory segment with an
 * array of polygon edges
 * @details The ray-casting cannot use the edges filtered by the segment box,
 * since an edge outside it still crosses the ray and the even-odd containment
 * test would break. It is given the full array and its own R-tree query
 * instead, the +x ray of #point_in_polygon_impl, which excludes only the edges
 * that lie left of the point or off the ray's height and so is identical to
 * the full scan
 * @param[in] rtree R-tree over @p all_edges, or NULL to scan them all
 * @param[in] srid,xmax SRID and geometry bounding-box maximum abscissa, used
 * to query @p rtree
 */
static void
intervals_from_polygons(const POINT2D *a, const POINT2D *b, Edge **edges,
  int nedges, Edge **all_edges, int all_nedges, const RTree *rtree,
  int32_t srid, double xmax)
{
  assert(a); assert(b); assert(edges); assert(nedges >= 0);

  /* Reset event array */
  events->count = 0;

  const double ax = a->x, ay = a->y;
  const double bx = b->x, by = b->y;

  /* Segment bounding box */
  const double seg_xmin = Min(ax, bx);
  const double seg_xmax = Max(ax, bx);
  const double seg_ymin = Min(ay, by);
  const double seg_ymax = Max(ay, by);
  /* Segment vector */
  const double rx = bx - ax;
  const double ry = by - ay;

  /* Check whether any polygon boundary edges exist using the full edge array.
   * A curve polygon contributes straight (EDGE_POLYSEG) and arc (EDGE_POLYARC)
   * boundary edges */
  bool has_polys = false;
  for (int i = 0; i < all_nedges; i++)
  {
    EdgeType et = all_edges[i]->etype;
    if (et == EDGE_POLYSEG || et == EDGE_POLYARC)
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
    const Edge *e = edges[i];
    /* Iterate only for the polygon boundary edges (straight or arc) */
    if (e->etype != EDGE_POLYSEG && e->etype != EDGE_POLYARC)
      continue;

    /* Bounding box filter */
    if (e->xmax < seg_xmin || e->xmin > seg_xmax ||
        e->ymax < seg_ymin || e->ymin > seg_ymax)
      continue;

    if (e->etype == EDGE_POLYSEG)
    {
      /* Compute the crossing with the straight boundary segment */
      IntersectResult r = linesegm_intersect(ax, ay, rx, ry,
        e->x1, e->y1, e->x2, e->y2);
      if (r.type == INTERSECT_POINT)
      {
        double t = r.t0;
        if (t >= -MEOS_GEOM_TOLERANCE && t <= 1.0 + MEOS_GEOM_TOLERANCE)
          meos_array_add(events, &t);
      }
    }
    else
    {
      /* Compute the crossings with the arc boundary edge (at most two) */
      double t[2];
      int n = arcsegm_intersect(ax, ay, rx, ry, e, t);
      for (int k = 0; k < n; k++)
        if (t[k] >= -MEOS_GEOM_TOLERANCE && t[k] <= 1.0 + MEOS_GEOM_TOLERANCE)
          meos_array_add(events, &t[k]);
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
        fabs(evtarr[i] - evtarr[newcount - 1]) > MEOS_GEOM_TOLERANCE)
    {
      evtarr[newcount++] = evtarr[i];
    }
  }
  events->count = newcount;

  /* Build intervals using midpoint test, recording for each event whether it
   * bounds an interval the point spends in the polygon interior */
  int nevents = (int) events->count;
  bool *bounded = palloc0(sizeof(bool) * (nevents > 0 ? nevents : 1));
  for (int i = 0; i < nevents - 1; i++)
  {
    double ta = evtarr[i];
    double tb = evtarr[i + 1];
    if (tb - ta <= MEOS_GEOM_TOLERANCE)
      continue;

    /* Midpoint test */
    double tm = (ta + tb) * 0.5;
    double x = ax + tm * rx;
    double y = ay + tm * ry;
    if (point_in_polygon_impl(x, y, all_edges, all_nedges, rtree, srid, xmax))
    {
      Span in;
      span_set(Float8GetDatum(ta), Float8GetDatum(tb), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
      bounded[i] = bounded[i + 1] = true;
    }
  }

  /* A segment that meets the boundary without entering the interior is in the
   * closure of the polygon at that instant alone. Such a contact bounds no
   * interval, so it is emitted as the instantaneous interval that it is */
  for (int i = 0; i < nevents; i++)
  {
    if (bounded[i])
      continue;
    double t = evtarr[i];
    if (t < 0.0)
      t = 0.0;
    else if (t > 1.0)
      t = 1.0;
    double x = ax + t * rx;
    double y = ay + t * ry;
    if (! point_on_poly_boundary(x, y, edges, nedges))
      continue;
    Span in;
    span_set(Float8GetDatum(t), Float8GetDatum(t), true, true,
      T_FLOAT8, T_FLOATSPAN, &in);
    meos_array_add(intervals, &in);
  }
  pfree(bounded);
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
    const Edge *e = edges[i];
    if (e->etype == EDGE_POINT)
    {
      if (fabs(e->x1 - ax) < MEOS_GEOM_TOLERANCE && fabs(e->y1 - ay) < MEOS_GEOM_TOLERANCE)
        return true;
    }
    else if (e->etype == EDGE_LINESEG)
    {
      if (point_on_segment(ax, ay, e->x1, e->y1, e->x2, e->y2))
        return true;
    }
    else if (e->etype == EDGE_LINEARC)
    {
      if (point_on_arc(ax, ay, e))
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
  const RTree *rtree, Edge **cand_edges, double xmax)
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
    int cand_nedges = rtree_search(rtree, INDEX_OVERLAPS, &query, rtree_results);

    /* Convert the result of an R-tree look up into an edge pointer array */
    for (int j = 0; j < cand_nedges; j++)
      cand_edges[j] = edges[*(int64 *) meos_array_get(rtree_results, j)];
    sel_edges = cand_edges;
    sel_nedges = cand_nedges;
  }

  /* Reset the interval array */
  intervals->count = 0;
  /* Compute the intervals for the points, lines, and polygon edges */
  bool found = point_inter_points_lines(a, sel_edges, sel_nedges);
  if (! found)
  {
    intervals_from_polygons(a, a, sel_edges, sel_nedges, edges, nedges, rtree,
      tspatial_srid((Temporal *) inst), xmax);
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
  const RTree *rtree, Edge **cand_edges, double xmax)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
    return tpointinst_clip_edges(TSEQUENCE_INST_N(seq, 0), edges, nedges,
      rtree, cand_edges, xmax);

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
      stbox_set(true, false, false, srid, Min(a->x, b->x),
        Max(a->x, b->x), Min(a->y, b->y), Max(a->y, b->y),
        0, 0, NULL, &query);
      /* Query the R-tree */
      int cand_nedges = rtree_search(rtree, INDEX_OVERLAPS, &query, rtree_results);

      /* Convert the result of an R-tree look up into an edge pointer array */
      for (int j = 0; j < cand_nedges; j++)
        cand_edges[j] = edges[*(int64 *) meos_array_get(rtree_results, j)];
      sel_edges = cand_edges;
      sel_nedges = cand_nedges;
    }

    /* Reset the interval array */
    intervals->count = 0;
    /* Compute the intervals for the points, lines, and polygon edges */
    intervals_from_points(a, b, sel_edges, sel_nedges);
    intervals_from_lines(a, b, sel_edges, sel_nedges);
    intervals_from_arcs(a, b, sel_edges, sel_nedges);
    intervals_from_polygons(a, b, sel_edges, sel_nedges, edges, nedges, rtree,
      srid, xmax);
    /* The array is declared before the jump below, which would otherwise skip
     * its initializer while `next_segment` reads it */
    Span *intervarr = NULL;
    if (intervals->count == 0)
      goto next_segment;

    /* Normalize the intervals */
    int count;
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
      if (fabs(upper - lower) < MEOS_GEOM_TOLERANCE)
      {
        /* Remove intersection points on exclusive lower and upper bounds */
        if (! lower_inc && fabs(lower) < MEOS_GEOM_TOLERANCE &&
            fabs(upper) < MEOS_GEOM_TOLERANCE)
          continue;
        if (! upper_inc && fabs(lower - 1.0) < MEOS_GEOM_TOLERANCE &&
            fabs(upper - 1.0) < MEOS_GEOM_TOLERANCE)
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
 * Geometry edge context
 *
 * Everything the engine derives from a geometry -- its bounding box, its edge
 * decomposition, and the R-tree indexing those edges -- depends on that
 * geometry alone. A context keeps that work so that the many operations
 * resolved against one geometry share it, instead of each rebuilding the
 * decomposition and the index. The `_ctx` functions below take a context and
 * the operations named without the suffix build one, use it, and free it, so
 * a single operation costs exactly what it did before.
 *****************************************************************************/

/**
 * @brief Structure keeping the reusable decomposition of a geometry
 */
typedef struct
{
  STBox box;           /**< Bounding box of the geometry */
  int32_t srid;        /**< SRID of the geometry */
  MeosArray *edges;    /**< Edges of the geometry */
  Edge **edge_ptrs;    /**< Pointers to the edges, as the kernels expect them */
  int nedges;          /**< Number of edges */
  RTree *rtree;        /**< Index over the edges, NULL when there are too few
                            of them to amortize its construction */
  Edge **cand_edges;   /**< Buffer receiving the edges selected by the index,
                            NULL when there is no index */
} GeoEdgeCtx;

/**
 * @brief Return the edge context of a geometry, or NULL if the geometry is
 * empty
 * @details The context owns the edges of the geometry and, when they are
 * numerous enough to amortize its construction, an R-tree indexing them
 * @note At most one context may be alive per thread, since the buffer
 * collecting the results of an index search is the thread-local
 * `rtree_results` shared with the clip kernels, created and destroyed with the
 * index (the same lifetime the operations gave it when each built its own)
 */
void *
geo_edge_ctx_make(const GSERIALIZED *gs)
{
  assert(gs);
  if (gserialized_is_empty(gs))
    return NULL;

  GeoEdgeCtx *ctx = palloc0(sizeof(GeoEdgeCtx));
  geo_set_stbox(gs, &ctx->box);
  ctx->srid = gserialized_get_srid(gs);
  /* Extract the edges */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  ctx->edges = geom_extract_edges(geom);
  lwgeom_free(geom);
  ctx->nedges = (int) ctx->edges->count;
  /* Transform the edge array into an edge pointer array */
  ctx->edge_ptrs = palloc(sizeof(Edge *) * ctx->nedges);
  for (int i = 0; i < ctx->nedges; i++)
    ctx->edge_ptrs[i] = (Edge *) meos_array_get(ctx->edges, i);

  /* Index the edges only when there are enough of them to compensate the
   * overhead of the tree construction and destruction */
  if (ctx->nedges > RTREE_MIN_NUMBER_ELEMS)
  {
    ctx->rtree = build_edge_rtree(ctx->edges->elems, ctx->nedges, ctx->srid);
    if (! ctx->rtree)
    {
      /* Release what the context holds before reporting the error, which may
       * not return control here */
      meos_array_destroy(ctx->edges);
      pfree(ctx->edge_ptrs); pfree(ctx);
      meos_error(ERROR, MEOS_ERR_INTERNAL_ERROR,
        "Error when creating R-tree");
      return NULL;
    }
    ctx->cand_edges = palloc(sizeof(Edge *) * ctx->nedges);
    /* Array for collecting the ids resulting from an R-tree search */
    rtree_results = meos_array_create(sizeof(int64));
  }
  return ctx;
}

/**
 * @brief Free an edge context built by #geo_edge_ctx_make
 */
void
geo_edge_ctx_free(void *ctxv)
{
  if (! ctxv)
    return;
  GeoEdgeCtx *ctx = (GeoEdgeCtx *) ctxv;
  if (ctx->rtree)
  {
    rtree_free(ctx->rtree);
    pfree(ctx->cand_edges);
    meos_array_destroy(rtree_results);
    rtree_results = NULL;
  }
  meos_array_destroy(ctx->edges);
  pfree(ctx->edge_ptrs);
  pfree(ctx);
  return;
}

/*****************************************************************************/

/**
 * @brief Return true if two geometry edges intersect
 * @details Dispatches on the edge-type pair, reusing the straight-segment
 * (#linesegm_intersect), segment/arc (#arcsegm_intersect), and arc/arc
 * (#arcarc_intersect) primitives. A point edge (#EDGE_POINT) has no extent,
 * so it meets another edge iff it lies on it.
 */
static bool
edge_intersect(const Edge *e1, const Edge *e2)
{
  /* Bounding-box reject */
  if (e1->xmax < e2->xmin - MEOS_GEOM_TOLERANCE || e2->xmax < e1->xmin - MEOS_GEOM_TOLERANCE ||
      e1->ymax < e2->ymin - MEOS_GEOM_TOLERANCE || e2->ymax < e1->ymin - MEOS_GEOM_TOLERANCE)
    return false;

  bool arc1 = (e1->etype == EDGE_LINEARC || e1->etype == EDGE_POLYARC);
  bool arc2 = (e2->etype == EDGE_LINEARC || e2->etype == EDGE_POLYARC);

  /* A point edge meets another edge only by lying on it */
  if (e1->etype == EDGE_POINT && e2->etype == EDGE_POINT)
    return fabs(e1->x1 - e2->x1) < MEOS_GEOM_TOLERANCE &&
      fabs(e1->y1 - e2->y1) < MEOS_GEOM_TOLERANCE;
  if (e1->etype == EDGE_POINT)
    return arc2 ? point_on_arc(e1->x1, e1->y1, e2) :
      point_on_segment(e1->x1, e1->y1, e2->x1, e2->y1, e2->x2, e2->y2);
  if (e2->etype == EDGE_POINT)
    return arc1 ? point_on_arc(e2->x1, e2->y1, e1) :
      point_on_segment(e2->x1, e2->y1, e1->x1, e1->y1, e1->x2, e1->y2);

  /* Arc/arc, segment/arc, or segment/segment */
  if (arc1 && arc2)
    return arcarc_intersect(e1, e2);
  if (arc1)
  {
    double out[2];
    return arcsegm_intersect(e2->x1, e2->y1, e2->dx, e2->dy, e1, out) > 0;
  }
  if (arc2)
  {
    double out[2];
    return arcsegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy, e2, out) > 0;
  }
  IntersectResult r = linesegm_intersect(e1->x1, e1->y1, e1->dx, e1->dy,
    e2->x1, e2->y1, e2->x2, e2->y2);
  return r.type != INTERSECT_NONE;
}

/**
 * @brief Return true if the edge array contains a polygon (area) edge
 */
static bool
edges_have_area(Edge **edges, int nedges)
{
  for (int i = 0; i < nedges; i++)
    if (edges[i]->etype == EDGE_POLYSEG || edges[i]->etype == EDGE_POLYARC)
      return true;
  return false;
}

/**
 * @brief Return true if a 2D geometry intersects the geometry of a clip
 * context, computed natively
 * @details Native counterpart of PostGIS `ST_Intersects` for the geometry
 * types the clip engine extracts into edges: two geometries meet when a
 * boundary edge of one crosses a boundary edge of the other, or when a
 * vertex of one lies inside the polygonal interior of the other. Points,
 * (multi)lines, (multi)polygons with holes, triangles, circular strings,
 * curve polygons, and collections of these are supported. The candidate edge
 * pairs are pruned with the R-tree the context keeps over its edges, mirroring
 * #tpoint_linear_inter_geom_ctx. Testing many geometries against one geometry
 * builds that index once, since the context outlives the call
 * @pre The arguments have the same SRID
 */
bool
geo_intersects2d_ctx(const GSERIALIZED *gs, const void *ctxv)
{
  assert(gs); assert(ctxv);
  const GeoEdgeCtx *ctx = (const GeoEdgeCtx *) ctxv;
  /* An empty geometry intersects nothing, matching PostGIS ST_Intersects.
   * Callers such as the touches predicates pass the (possibly empty) boundary
   * of a geometry or trajectory, so the leaf must tolerate empty input. */
  if (gserialized_is_empty(gs))
    return false;
  /* Bounding box test */
  STBox box;
  geo_set_stbox(gs, &box);
  if (! overlaps_stbox_stbox(&box, &ctx->box))
    return false;

  /* Extract the edges of the geometry given, those of the context geometry
   * being already extracted and indexed */
  LWGEOM *lw = lwgeom_from_gserialized(gs);
  MeosArray *edges = geom_extract_edges(lw);
  lwgeom_free(lw);
  int n = (int) edges->count;
  Edge **ptr = palloc(sizeof(Edge *) * n);
  for (int i = 0; i < n; i++)
    ptr[i] = (Edge *) meos_array_get(edges, i);

  bool result = false;

  /* Phase 1: a boundary edge of the geometry crosses a boundary edge of the
   * context geometry */
  for (int i = 0; i < n && ! result; i++)
  {
    const Edge *e = ptr[i];
    if (ctx->rtree)
    {
      STBox query;
      stbox_set(true, false, false, ctx->srid, e->xmin, e->xmax, e->ymin,
        e->ymax, 0, 0, NULL, &query);
      int nc = rtree_search(ctx->rtree, INDEX_OVERLAPS, &query, rtree_results);
      for (int j = 0; j < nc; j++)
        ctx->cand_edges[j] =
          ctx->edge_ptrs[*(int64 *) meos_array_get(rtree_results, j)];
      for (int j = 0; j < nc && ! result; j++)
        if (edge_intersect(e, ctx->cand_edges[j]))
          result = true;
    }
    else
    {
      for (int j = 0; j < ctx->nedges && ! result; j++)
        if (edge_intersect(e, ctx->edge_ptrs[j]))
          result = true;
    }
  }

  /* Phase 2: containment -- a vertex of one geometry inside the other's
   * polygonal interior (only meaningful when the other has area) */
  if (! result && edges_have_area(ctx->edge_ptrs, ctx->nedges))
    for (int i = 0; i < n && ! result; i++)
      if (point_in_polygon_impl(ptr[i]->x1, ptr[i]->y1, ctx->edge_ptrs,
            ctx->nedges, ctx->rtree, ctx->srid, ctx->box.xmax))
        result = true;
  if (! result && edges_have_area(ptr, n))
    for (int i = 0; i < ctx->nedges && ! result; i++)
      if (point_in_polygon(ctx->edge_ptrs[i]->x1, ctx->edge_ptrs[i]->y1, ptr,
            n))
        result = true;

  /* Clean up */
  meos_array_destroy(edges);
  pfree(ptr);
  return result;
}

/**
 * @brief Return true if two 2D geometries intersect, computed natively
 * @details Builds the edge context of the second geometry, which is the one
 * whose edges are indexed, and resolves the relationship with
 * #geo_intersects2d_ctx
 * @pre The arguments have the same SRID
 */
bool
geo_intersects2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  /* An empty geometry intersects nothing, matching PostGIS ST_Intersects.
   * Callers such as the touches predicates pass the (possibly empty) boundary
   * of a geometry or trajectory, so the leaf must tolerate empty input. */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;
  /* Bounding box test, made before extracting the edges of the second
   * geometry so that a rejected pair does not pay for its decomposition */
  STBox box1, box2;
  geo_set_stbox(gs1, &box1);
  geo_set_stbox(gs2, &box2);
  if (! overlaps_stbox_stbox(&box1, &box2))
    return false;

  void *ctx = geo_edge_ctx_make(gs2);
  if (! ctx)
    return false;
  bool result = geo_intersects2d_ctx(gs1, ctx);
  geo_edge_ctx_free(ctx);
  return result;
}

/*****************************************************************************
 * Native planar covers predicate
 *****************************************************************************/

/**
 * @brief Return true if a point lies in the closure (interior or boundary) of
 * the geometry whose edges are given
 * @details A point is in the closure when it lies on any extracted edge -- the
 * 1D extent of a point/line geometry or the boundary of a polygon -- or, when
 * the geometry has an areal component, inside its polygonal interior. The
 * #point_in_polygon test already reports points on a polygon boundary as
 * inside, so the explicit on-edge scan only adds the lower-dimensional
 * (point/line/standalone-arc) closure that the even-odd test ignores.
 */
static bool
edges_contain_point(double px, double py, Edge **edges, int nedges,
  bool has_area)
{
  for (int i = 0; i < nedges; i++)
  {
    const Edge *e = edges[i];
    if (e->etype == EDGE_POINT)
    {
      if (fabs(px - e->x1) < MEOS_GEOM_TOLERANCE && fabs(py - e->y1) < MEOS_GEOM_TOLERANCE)
        return true;
    }
    else if (e->etype == EDGE_LINEARC || e->etype == EDGE_POLYARC)
    {
      if (point_on_arc(px, py, e))
        return true;
    }
    else /* EDGE_LINESEG, EDGE_POLYSEG */
    {
      if (point_on_segment(px, py, e->x1, e->y1, e->x2, e->y2))
        return true;
    }
  }
  if (has_area && point_in_polygon(px, py, edges, nedges))
    return true;
  return false;
}

/**
 * @brief Return true if a straight edge lies entirely within the closure of
 * the geometry whose edges are given
 * @details The edge is split at every crossing with an edge of the covering
 * geometry, and the midpoint of each resulting sub-segment is tested for
 * closure membership. An edge whose endpoints are both in the closure can
 * still leave it between two crossings (through a concavity or a hole), which
 * this per-sub-segment test detects.
 */
static bool
segment_within_closure(const Edge *e, Edge **aedges, int na, bool has_area)
{
  double *ts = palloc(sizeof(double) * (size_t) (2 * na + 2));
  int nt = 0;
  ts[nt++] = 0.0;
  ts[nt++] = 1.0;
  for (int i = 0; i < na; i++)
  {
    const Edge *ea = aedges[i];
    if (ea->etype == EDGE_LINEARC || ea->etype == EDGE_POLYARC)
    {
      double out[2];
      int m = arcsegm_intersect(e->x1, e->y1, e->dx, e->dy, ea, out);
      for (int k = 0; k < m; k++)
        ts[nt++] = out[k];
    }
    else
    {
      IntersectResult r = linesegm_intersect(e->x1, e->y1, e->dx, e->dy,
        ea->x1, ea->y1, ea->x2, ea->y2);
      if (r.type == INTERSECT_POINT)
        ts[nt++] = r.t0;
      else if (r.type == INTERSECT_OVERLAP)
      {
        ts[nt++] = r.t0;
        ts[nt++] = r.t1;
      }
    }
  }
  qsort(ts, (size_t) nt, sizeof(double), float8_qsort_cmp);
  bool result = true;
  for (int i = 0; i < nt - 1 && result; i++)
  {
    if (ts[i + 1] - ts[i] < MEOS_GEOM_TOLERANCE)
      continue;
    double tm = 0.5 * (ts[i] + ts[i + 1]);
    double mx = e->x1 + tm * e->dx, my = e->y1 + tm * e->dy;
    if (! edges_contain_point(mx, my, aedges, na, has_area))
      result = false;
  }
  pfree(ts);
  return result;
}

/**
 * @brief Return true if an arc edge lies entirely within the closure of the
 * geometry whose edges are given
 * @details The arc is sampled at interior angles across its span; each sample
 * must lie in the closure. Endpoints are tested by the caller.
 */
static bool
arc_within_closure(const Edge *e, Edge **aedges, int na, bool has_area)
{
  const int nsamp = 16;
  double sweep = e->ccw ? angle_normalize(e->theta1 - e->theta0) :
    - angle_normalize(e->theta0 - e->theta1);
  for (int k = 1; k < nsamp; k++)
  {
    double phi = e->theta0 + sweep * ((double) k / nsamp);
    double px = e->cx + e->radius * cos(phi);
    double py = e->cy + e->radius * sin(phi);
    if (! edges_contain_point(px, py, aedges, na, has_area))
      return false;
  }
  return true;
}

/**
 * @brief Return true if a geometry is a (multi)point
 */
static bool
geo_is_point(const GSERIALIZED *gs)
{
  int type = gserialized_get_type(gs);
  return type == POINTTYPE || type == MULTIPOINTTYPE;
}

/**
 * @brief Return true if a geometry is a (multi)polygon
 */
static bool
geo_is_poly(const GSERIALIZED *gs)
{
  int type = gserialized_get_type(gs);
  return type == POLYGONTYPE || type == MULTIPOLYGONTYPE;
}

/**
 * @brief Return true if the first 2D geometry covers the second, computed
 * natively
 * @details Geometry A covers geometry B when every point of B lies in the
 * closure of A, that is, B has no point in A's exterior (the DE-9IM
 * `T*****FF*` family). Every vertex of B, and the midpoint of every
 * sub-segment obtained by splitting each edge of B at its crossings with A,
 * must lie in A's closure. Supports the geometry types the clip engine
 * extracts into edges: points, (multi)lines, (multi)polygons with holes,
 * triangles, circular strings, curve polygons, and collections of these.
 * The dispatch mirrors #geom_spatialrel: an empty operand and a (multi)polygon
 * covering a (multi)point are handled by the same native
 * #meos_point_in_polygon short-circuit, so only the general case replaces the
 * GEOS covers call.
 * @pre The arguments have the same SRID
 */
bool
geo_covers2d(const GSERIALIZED *gs1, const GSERIALIZED *gs2)
{
  assert(gs1); assert(gs2);
  /* An empty geometry covers nothing and is covered by nothing, matching
   * PostGIS ST_Covers */
  if (gserialized_is_empty(gs1) || gserialized_is_empty(gs2))
    return false;
  /* Covers is reflexive: every non-empty geometry covers itself (it is a subset
   * of its own closure). Short-circuit byte-identical operands, mirroring
   * #geo_equals. This is exact and FP-free, hence environment-independent —
   * unlike the general edge test, whose point-on-boundary classification sits on
   * a floating-point knife edge for a degenerate self-covering geometry (e.g. an
   * antimeridian-wrapping H3 cell boundary), where -O2 coverage instrumentation
   * can flip the result */
  if (VARSIZE(gs1) == VARSIZE(gs2) && ! memcmp(gs1, gs2, VARSIZE(gs1)))
    return true;
  /* Bounding-box reject: covering requires the 2D boxes to overlap. Covers is a
   * planar (2D) predicate, so the reject must ignore Z; use the same canonical
   * 2D box overlap #geom_spatialrel applies before delegating to GEOS, rather
   * than #overlaps_stbox_stbox which compares Z for a 3D/3D pair and would
   * wrongly reject geometries whose X/Y overlap but whose Z ranges are disjoint */
  GBOX box1, box2;
  memset(&box1, 0, sizeof(GBOX));
  memset(&box2, 0, sizeof(GBOX));
  if (gserialized_get_gbox_p(gs1, &box1) && gserialized_get_gbox_p(gs2, &box2) &&
      gbox_overlaps_2d(&box1, &box2) == LW_FALSE)
    return false;
  /* A (multi)polygon covering a (multi)point is resolved by the native
   * point-in-polygon test, exactly as #geom_spatialrel does before delegating
   * to GEOS. That test answers in the direction of the polygon, so the reverse
   * pair, a (multi)point asked to cover a (multi)polygon, keeps the general
   * path below */
  if (geo_is_poly(gs1) && geo_is_point(gs2))
    return meos_point_in_polygon(gs1, gs2, COVERS);

  /* Extract the edges of both geometries */
  LWGEOM *lw1 = lwgeom_from_gserialized(gs1);
  LWGEOM *lw2 = lwgeom_from_gserialized(gs2);
  MeosArray *edges1 = geom_extract_edges(lw1);
  MeosArray *edges2 = geom_extract_edges(lw2);
  lwgeom_free(lw1); lwgeom_free(lw2);
  int na = (int) edges1->count, nb = (int) edges2->count;
  Edge **aedges = palloc(sizeof(Edge *) * na);
  Edge **bedges = palloc(sizeof(Edge *) * nb);
  for (int i = 0; i < na; i++)
    aedges[i] = (Edge *) meos_array_get(edges1, i);
  for (int i = 0; i < nb; i++)
    bedges[i] = (Edge *) meos_array_get(edges2, i);
  bool has_area = edges_have_area(aedges, na);

  bool result = true;
  /* Every vertex of B must lie in A's closure */
  for (int i = 0; i < nb && result; i++)
  {
    const Edge *e = bedges[i];
    if (! edges_contain_point(e->x1, e->y1, aedges, na, has_area))
      result = false;
    else if (e->etype != EDGE_POINT &&
      ! edges_contain_point(e->x2, e->y2, aedges, na, has_area))
      result = false;
  }
  /* Every edge of B must stay within A's closure */
  for (int i = 0; i < nb && result; i++)
  {
    const Edge *e = bedges[i];
    if (e->etype == EDGE_POINT)
      continue;
    if (e->etype == EDGE_LINEARC || e->etype == EDGE_POLYARC)
    {
      if (! arc_within_closure(e, aedges, na, has_area))
        result = false;
    }
    else if (! segment_within_closure(e, aedges, na, has_area))
      result = false;
  }

  meos_array_destroy(edges1);
  meos_array_destroy(edges2);
  pfree(aedges);
  pfree(bedges);
  return result;
}

/**
 * @brief Return the temporal intersection/intersects of a temporal geometric
 * point with linear interpolation and the geometry of a edge context
 * @details The temporal geometric point may be in 2D or 3D and the Z dimension
 * is also computed. Clipping several temporal points against one geometry
 * extracts and indexes its edges once, since the context outlives the call
 * @note For performance reasons the intersection is computed natively
 * instead of through ST_Intersection
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
Temporal *
tpoint_linear_inter_geom_ctx(const Temporal *temp, const void *ctxv, bool clip)
{
  assert(temp); assert(ctxv); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  const GeoEdgeCtx *ctx = (const GeoEdgeCtx *) ctxv;

  /* Bounding box test */
  STBox box1;
  tspatial_set_stbox(temp, &box1);
  if (! overlaps_stbox_stbox(&box1, &ctx->box))
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

  /* Initialize the static global arrays accumulating the clipping results */
  events = meos_array_create(sizeof(double));
  intervals = meos_array_create(sizeof(Span));
  periods = meos_array_create(sizeof(Span));

  /* Collect the clipping periods */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TINSTANT:
      tpointinst_clip_edges((TInstant *) temp, ctx->edge_ptrs, ctx->nedges,
        ctx->rtree, ctx->cand_edges, ctx->box.xmax);
      break;
    case TSEQUENCE:
      tpointseq_clip_edges((TSequence *) temp, ctx->edge_ptrs, ctx->nedges,
        ctx->rtree, ctx->cand_edges, ctx->box.xmax);
      break;
    default: /* TSEQUENCESET */
    {
      /* Loop for each segment */
      TSequenceSet *ss = (TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        tpointseq_clip_edges(TSEQUENCESET_SEQ_N(ss, i), ctx->edge_ptrs,
          ctx->nedges, ctx->rtree, ctx->cand_edges, ctx->box.xmax);
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
  meos_array_destroy(events);
  meos_array_destroy(intervals);
  meos_array_destroy(periods);
  return result;
}

/**
 * @ingroup meos_internal_geo
 * @brief Return the temporal intersection/intersects of a temporal geometric
 * point with linear interpolation and a 2D geometry
 * @details Builds the edge context of the geometry and resolves the
 * relationship with #tpoint_linear_inter_geom_ctx
 * @pre The arguments have the same SRID, the geometry is 2D and is not empty.
 * This is verified in #tgeo_restrict_geom
 */
Temporal *
tpoint_linear_inter_geom(const Temporal *temp, const GSERIALIZED *gs,
  bool clip)
{
  assert(temp); assert(gs); assert(! gserialized_is_empty(gs));
  /* Bounding box test, made before building the context so that a rejected
   * pair does not pay for the decomposition of the geometry. An empty
   * geometry has no box to read, and the context declines to build on one,
   * so the test yields to it rather than reading a box that was never set */
  STBox box1, box2;
  tspatial_set_stbox(temp, &box1);
  if (! geo_set_stbox(gs, &box2))
    return NULL;
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

  void *ctx = geo_edge_ctx_make(gs);
  if (! ctx)
    return NULL;
  Temporal *result = tpoint_linear_inter_geom_ctx(temp, ctx, clip);
  geo_edge_ctx_free(ctx);
  return result;
}

/*****************************************************************************
 * Within-distance (tDwithin / ever-always dwithin) native engine
 *
 * Distance-threshold sibling of the exact intersection engine above. The
 * within region of a geometry at distance @p dist is its Minkowski sum with a
 * closed disc of radius @p dist: a capsule around each segment, a disc around
 * each point, an annular sector around each arc, and the filled polygon
 * dilated by @p dist. For each moving-point segment the candidate boundary
 * crossing times are solved in closed form per edge (the roots of
 * dist(seg(t), edge) = dist), then each sub-interval is classified by the
 * exact interior-aware unit distance at its midpoint. This mirrors the
 * within-roots + midpoint-classification spanset assembler of the merged
 * temporal circular-buffer engine (tcbuffer_distance.c) specialized to a
 * moving point, i.e. a moving disc with radius r(t) = 0.
 *
 * A zero distance is exactly the temporal intersects relationship and is
 * delegated to #tpoint_linear_inter_geom so that tDwithin(., ., 0) is
 * bit-identical to tIntersects (including isolated contact instants, which are
 * measure-zero and therefore dropped by the positive-distance midpoint
 * classification, exactly as in the temporal circular-buffer engine).
 *****************************************************************************/

/**
 * @brief Return the squared distance from a point to a segment
 */
static double
point_seg_dist2(double px, double py, double x1, double y1, double x2,
  double y2)
{
  const double ux = x2 - x1, uy = y2 - y1;
  const double l2 = ux * ux + uy * uy;
  if (l2 < MEOS_GEOM_TOLERANCE)
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
 * is the difference to the supporting circle, otherwise it is the distance to
 * the nearer arc endpoint
 */
static double
point_arc_dist2(double px, double py, const Edge *e)
{
  const double dxc = px - e->cx, dyc = py - e->cy;
  const double dc = hypot(dxc, dyc);
  if (arc_contains_angle(e, atan2(dyc, dxc)))
  {
    const double dd = dc - e->radius;
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
 */
static double
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
 * @brief Return true if a point is within @p dist of the geometry, taking the
 * polygon interior into account (a point inside a polygon is at distance 0),
 * pruning both tests with an R-tree over the edges when one is given
 * @details Only an edge whose bounding box meets the square of side 2 * @p dist
 * centred on the point can be within that distance of it; querying the R-tree
 * for those instead of scanning every edge turns the O(nedges) test into
 * O(log nedges + candidates). An excluded edge is farther than @p dist from the
 * point along one axis alone, so it cannot satisfy the distance test and the
 * result is identical to the full scan. The interior test inherits the same
 * pruning from #point_in_polygon_impl
 */
static bool
point_geom_within(double px, double py, Edge **edges, int nedges,
  double dist, const RTree *rtree, int32_t srid, double xmax)
{
  const double d2 = dist * dist;
  if (rtree)
  {
    STBox query;
    stbox_set(true, false, false, srid, px - dist, px + dist, py - dist,
      py + dist, 0, 0, NULL, &query);
    int nc = rtree_search(rtree, INDEX_OVERLAPS, &query, rtree_results);
    for (int i = 0; i < nc; i++)
      if (point_edge_dist2(px, py,
            edges[*(int64 *) meos_array_get(rtree_results, i)]) <=
          d2 + MEOS_GEOM_TOLERANCE)
        return true;
  }
  else
  {
    for (int i = 0; i < nedges; i++)
      if (point_edge_dist2(px, py, edges[i]) <= d2 + MEOS_GEOM_TOLERANCE)
        return true;
  }
  return point_in_polygon_impl(px, py, edges, nedges, rtree, srid, xmax) ?
    true : false;
}

/**
 * @brief Append a candidate crossing time to the event array if it lies in
 * [0,1] (clamping tiny out-of-range values to the endpoints)
 */
static void
add_within_root(double t, MeosArray *ev)
{
  if (t > -MEOS_GEOM_TOLERANCE && t < 1.0 + MEOS_GEOM_TOLERANCE)
  {
    if (t < 0.0) t = 0.0; else if (t > 1.0) t = 1.0;
    meos_array_add(ev, &t);
  }
}

/**
 * @brief Append the [0,1] roots of the quadratic @p A t^2 + @p B t + @p C to
 * the event array
 */
static void
add_within_quad_roots(double A, double B, double C, MeosArray *ev)
{
  if (fabs(A) < MEOS_GEOM_TOLERANCE)
  {
    if (fabs(B) > MEOS_GEOM_TOLERANCE)
      add_within_root(-C / B, ev);
    return;
  }
  const double disc = B * B - 4.0 * A * C;
  if (disc < 0.0)
    return;
  const double sq = sqrt(disc);
  add_within_root((-B - sq) / (2.0 * A), ev);
  add_within_root((-B + sq) / (2.0 * A), ev);
}

/**
 * @brief Append to @p ev the trajectory-segment times at which the moving
 * point crosses the distance-@p dist boundary of one edge
 * @details The boundary of the edge's within region is composed of: for a
 * point, the disc of radius @p dist; for a segment, the two endpoint caps and
 * the two parallel offset lines; for an arc, the inner/outer offset circles
 * and the two endpoint caps. The candidate set is a superset (offset lines are
 * infinite, offset circles ignore the angular span); spurious candidates are
 * filtered out by the exact midpoint distance classification
 */
static void
within_roots_from_edge(double ax, double ay, double rx, double ry,
  const Edge *e, double dist, MeosArray *ev)
{
  const double A = rx * rx + ry * ry;
  const double d2 = dist * dist;
  switch (e->etype)
  {
    case EDGE_POINT:
    {
      const double wx = ax - e->x1, wy = ay - e->y1;
      add_within_quad_roots(A, 2.0 * (wx * rx + wy * ry),
        wx * wx + wy * wy - d2, ev);
      return;
    }
    case EDGE_LINESEG:
    case EDGE_POLYSEG:
    {
      /* Endpoint caps: discs of radius dist around each segment endpoint */
      const double w0x = ax - e->x1, w0y = ay - e->y1;
      add_within_quad_roots(A, 2.0 * (w0x * rx + w0y * ry),
        w0x * w0x + w0y * w0y - d2, ev);
      const double w1x = ax - e->x2, w1y = ay - e->y2;
      add_within_quad_roots(A, 2.0 * (w1x * rx + w1y * ry),
        w1x * w1x + w1y * w1y - d2, ev);
      /* Parallel offset lines at distance dist on both sides. The signed
       * perpendicular distance is (k0 + t k1) / sqrt(l2) */
      const double ux = e->x2 - e->x1, uy = e->y2 - e->y1;
      const double l2 = ux * ux + uy * uy;
      if (l2 > MEOS_GEOM_TOLERANCE)
      {
        const double k0 = w0x * uy - w0y * ux;
        const double k1 = rx * uy - ry * ux;
        if (fabs(k1) > MEOS_GEOM_TOLERANCE)
        {
          const double off = dist * sqrt(l2);
          add_within_root((off - k0) / k1, ev);
          add_within_root((-off - k0) / k1, ev);
        }
      }
      return;
    }
    case EDGE_LINEARC:
    case EDGE_POLYARC:
    {
      const double wx = ax - e->cx, wy = ay - e->cy;
      const double B = 2.0 * (wx * rx + wy * ry);
      const double C0 = wx * wx + wy * wy;
      const double ro = e->radius + dist;
      add_within_quad_roots(A, B, C0 - ro * ro, ev);
      const double ri = e->radius - dist;
      if (ri > 0.0)
        add_within_quad_roots(A, B, C0 - ri * ri, ev);
      /* Endpoint caps: discs of radius dist around each arc endpoint */
      const double e0x = ax - e->x1, e0y = ay - e->y1;
      add_within_quad_roots(A, 2.0 * (e0x * rx + e0y * ry),
        e0x * e0x + e0y * e0y - d2, ev);
      const double e1x = ax - e->x2, e1y = ay - e->y2;
      add_within_quad_roots(A, 2.0 * (e1x * rx + e1y * ry),
        e1x * e1x + e1y * e1y - d2, ev);
      return;
    }
  }
}

/**
 * @brief Collect into the interval array the [0,1] sub-intervals of one
 * trajectory segment along which the moving point is within @p dist of the
 * geometry
 * @param[in] a,b Endpoints of the trajectory segment
 * @param[in] sel_edges,sel_nedges Edges to gather crossing candidates from
 * (possibly R-tree filtered)
 * @param[in] all_edges,all_nedges Full edge array, used for the interior-aware
 * midpoint classification (the polygon ray-cast needs every edge)
 * @param[in] dist Distance threshold
 * @param[in] rtree R-tree over @p all_edges, or NULL to scan them all
 * @param[in] srid,xmax SRID and geometry bounding-box maximum abscissa, used
 * to query @p rtree
 */
static void
intervals_within_edges(const POINT2D *a, const POINT2D *b, Edge **sel_edges,
  int sel_nedges, Edge **all_edges, int all_nedges, double dist,
  const RTree *rtree, int32_t srid, double xmax)
{
  events->count = 0;
  const double ax = a->x, ay = a->y;
  const double rx = b->x - ax, ry = b->y - ay;
  const double seg_xmin = Min(a->x, b->x), seg_xmax = Max(a->x, b->x);
  const double seg_ymin = Min(a->y, b->y), seg_ymax = Max(a->y, b->y);

  /* Gather boundary crossing candidates from the (filtered) edges */
  for (int i = 0; i < sel_nedges; i++)
  {
    const Edge *e = sel_edges[i];
    /* Bounding-box filter expanded by dist: the moving point may be within
     * dist of an edge whose own box does not overlap the segment box */
    if (e->xmax + dist < seg_xmin || e->xmin - dist > seg_xmax ||
        e->ymax + dist < seg_ymin || e->ymin - dist > seg_ymax)
      continue;
    within_roots_from_edge(ax, ay, rx, ry, e, dist, events);
  }
  /* Add the segment endpoints */
  double t0 = 0.0, t1 = 1.0;
  meos_array_add(events, &t0);
  meos_array_add(events, &t1);

  /* Sort and deduplicate the candidates */
  qsort(events->elems, events->count, sizeof(double), float8_qsort_cmp);
  int newcount = 0;
  double *ev = (double *) events->elems;
  for (int i = 0; i < (int) events->count; i++)
    if (i == 0 || fabs(ev[i] - ev[newcount - 1]) > MEOS_GEOM_TOLERANCE)
      ev[newcount++] = ev[i];
  events->count = newcount;

  /* Keep each sub-interval whose midpoint is within dist of the geometry */
  for (int i = 0; i < (int) events->count - 1; i++)
  {
    const double ta = ev[i], tb = ev[i + 1];
    if (tb - ta <= MEOS_GEOM_TOLERANCE)
      continue;
    const double tm = 0.5 * (ta + tb);
    const double x = ax + tm * rx, y = ay + tm * ry;
    if (point_geom_within(x, y, all_edges, all_nedges, dist, rtree,
          srid, xmax))
    {
      Span in;
      span_set(Float8GetDatum(ta), Float8GetDatum(tb), true, true,
        T_FLOAT8, T_FLOATSPAN, &in);
      meos_array_add(intervals, &in);
    }
  }

  /* Isolated within instants: a trajectory that only grazes the distance
   * boundary tangentially touches the within region at a single time (a double
   * root, where the distance equals dist exactly) which the midpoint test
   * above cannot see. Emit a degenerate interval for each candidate time that
   * is within dist (inclusive). The span normalization absorbs the ones that
   * coincide with an interval endpoint, leaving only the genuine isolated
   * touches. This is what keeps the distance-inclusive semantics exact and, at
   * a zero distance, matches the isolated contact points of the intersection
   * engine (which the zero-distance path delegates to anyway). */
  for (int i = 0; i < (int) events->count; i++)
  {
    const double t = ev[i];
    const double x = ax + t * rx, y = ay + t * ry;
    if (point_geom_within(x, y, all_edges, all_nedges, dist, rtree,
          srid, xmax))
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
 * @brief Add the within-distance instantaneous period of a temporal instant
 * point to the period array
 */
static void
tpointinst_dwithin_edges(const TInstant *inst, Edge **edges, int nedges,
  double dist, const RTree *rtree, int32_t srid, double xmax)
{
  assert(inst); assert(edges); assert(nedges > 0);
  assert(inst->temptype == T_TGEOMPOINT);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst));
  if (! point_geom_within(a->x, a->y, edges, nedges, dist, rtree, srid,
        xmax))
    return;
  Span s;
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &s);
  meos_array_add(periods, &s);
  return;
}

/**
 * @brief Add to the period array the sub-periods of a temporal sequence point
 * with linear interpolation during which it is within @p dist of a geometry
 */
static void
tpointseq_dwithin_edges(const TSequence *seq, Edge **edges, int nedges,
  const RTree *rtree, Edge **cand_edges, double dist, double xmax)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
    return tpointinst_dwithin_edges(TSEQUENCE_INST_N(seq, 0), edges, nedges,
      dist, rtree, tspatial_srid((Temporal *) seq), xmax);

  bool use_index = (rtree != NULL && cand_edges != NULL);
  int32_t srid = tspatial_srid((Temporal *) seq);
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst1));
  bool lower_inc = seq->period.lower_inc;
  Edge **sel_edges = edges;
  int sel_nedges = nedges;
  /* Loop for each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *b = DATUM_POINT2D_P(tinstant_value_p(inst2));
    bool upper_inc = (i < seq->count - 1) ? false : seq->period.upper_inc;

    /* Filter the edges by an R-tree, expanding the query box by dist */
    if (use_index)
    {
      STBox query;
      stbox_set(true, false, false, srid, Min(a->x, b->x) - dist,
        Max(a->x, b->x) + dist, Min(a->y, b->y) - dist,
        Max(a->y, b->y) + dist, 0, 0, NULL, &query);
      int cand_nedges = rtree_search(rtree, INDEX_OVERLAPS, &query,
        rtree_results);
      for (int j = 0; j < cand_nedges; j++)
        cand_edges[j] = edges[*(int64 *) meos_array_get(rtree_results, j)];
      sel_edges = cand_edges;
      sel_nedges = cand_nedges;
    }

    /* Reset and compute the within intervals for this segment */
    intervals->count = 0;
    intervals_within_edges(a, b, sel_edges, sel_nedges, edges, nedges, dist,
      rtree, srid, xmax);
    /* The array is declared before the jump below, which would otherwise skip
     * its initializer while `next_segment` reads it */
    Span *intervarr = NULL;
    if (intervals->count == 0)
      goto next_segment;

    /* Normalize the intervals (sort: the midpoint intervals and the isolated
     * within points are appended in two separate passes, not globally sorted) */
    int count;
    if (intervals->count > 1)
      intervarr = spanarr_normalize(intervals->elems, intervals->count,
        ORDER, &count);
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
      if (fabs(upper - lower) < MEOS_GEOM_TOLERANCE)
      {
        /* Remove within points on exclusive lower and upper bounds */
        if (! lower_inc && fabs(lower) < MEOS_GEOM_TOLERANCE &&
            fabs(upper) < MEOS_GEOM_TOLERANCE)
          continue;
        if (! upper_inc && fabs(lower - 1.0) < MEOS_GEOM_TOLERANCE &&
            fabs(upper - 1.0) < MEOS_GEOM_TOLERANCE)
          continue;
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
    if (intervarr && intervals->count > 1)
      pfree(intervarr);
    inst1 = inst2;
    a = b;
  }
  return;
}

/**
 * @ingroup meos_internal_geo
 * @brief Return a temporal Boolean that states whether a temporal geometric
 * point with linear interpolation is within a distance of a 2D geometry
 * @details Native counterpart of the polygonal-buffer approximation:
 * for a zero distance it is exactly #tpoint_linear_inter_geom_ctx
 * (tIntersects), otherwise it solves the per-segment within-distance
 * sub-intervals in closed form. The result is a temporal Boolean defined over
 * the whole time of the temporal point. Testing several temporal points
 * against one geometry extracts and indexes its edges once, since the context
 * outlives the call
 * @pre The arguments have the same SRID, are 2D and planar, and the geometry
 * is not empty and is supported by the clip engine. This is verified by the
 * caller
 */
Temporal *
tpoint_linear_dwithin_geom_ctx(const Temporal *temp, const void *ctxv,
  double dist)
{
  assert(temp); assert(ctxv); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  const GeoEdgeCtx *ctx = (const GeoEdgeCtx *) ctxv;

  /* A zero distance is exactly the temporal intersects relationship */
  if (dist <= 0.0)
    return tpoint_linear_inter_geom_ctx(temp, ctxv, false);

  /* Bounding box test: the geometry box expanded by dist must overlap the
   * temporal point box, otherwise the relationship is false throughout */
  STBox box1, box2e;
  tspatial_set_stbox(temp, &box1);
  stbox_expand_space_set(&ctx->box, dist, &box2e);
  bool overlap = overlaps_stbox_stbox(&box1, &box2e);
  if (! overlap)
  {
    SpanSet *ss = temporal_time(temp);
    Temporal *result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
    return result;
  }

  /* Initialize the static global arrays accumulating the results */
  events = meos_array_create(sizeof(double));
  intervals = meos_array_create(sizeof(Span));
  periods = meos_array_create(sizeof(Span));

  /* Collect the within-distance periods */
  assert(temptype_subtype(temp->subtype));
  switch (temp->subtype)
  {
    case TSEQUENCE:
      tpointseq_dwithin_edges((TSequence *) temp, ctx->edge_ptrs, ctx->nedges,
        ctx->rtree, ctx->cand_edges, dist, ctx->box.xmax);
      break;
    default: /* TSEQUENCESET */
    {
      TSequenceSet *ss = (TSequenceSet *) temp;
      for (int i = 0; i < ss->count; i++)
        tpointseq_dwithin_edges(TSEQUENCESET_SEQ_N(ss, i), ctx->edge_ptrs,
          ctx->nedges, ctx->rtree, ctx->cand_edges, dist, ctx->box.xmax);
    }
  }

  /* Assemble the temporal Boolean over the whole time of the temporal point */
  Temporal *result;
  if (periods->count == 0)
  {
    SpanSet *ss = temporal_time(temp);
    result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
  }
  else
  {
    SpanSet *ss = spanset_make_exp(periods->elems, periods->count,
      periods->count, NORMALIZE, ORDER);
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
    pfree(ss1); pfree(temp1); pfree(ss);
  }

  /* Clean up and return */
  meos_array_destroy(events);
  meos_array_destroy(intervals);
  meos_array_destroy(periods);
  return result;
}

/**
 * @ingroup meos_internal_geo
 * @brief Return a temporal Boolean that states whether a temporal geometric
 * point with linear interpolation is within a distance of a 2D geometry
 * @details Builds the edge context of the geometry and resolves the
 * relationship with #tpoint_linear_dwithin_geom_ctx
 * @pre The arguments have the same SRID, are 2D and planar, and the geometry
 * is not empty and is supported by the clip engine. This is verified by the
 * caller
 */
Temporal *
tpoint_linear_dwithin_geom(const Temporal *temp, const GSERIALIZED *gs,
  double dist)
{
  assert(temp); assert(gs); assert(! gserialized_is_empty(gs));
  /* Bounding box test, made before building the context so that a rejected
   * pair does not pay for the decomposition of the geometry. The geometry box
   * is the one the within region reaches, so it is expanded by the distance.
   * An empty geometry has no box to read, and the context declines to build
   * on one, so the test yields to it rather than reading a box that was
   * never set */
  STBox box1, box2, box2e;
  tspatial_set_stbox(temp, &box1);
  if (! geo_set_stbox(gs, &box2))
    return NULL;
  stbox_expand_space_set(&box2, (dist > 0.0) ? dist : 0.0, &box2e);
  if (! overlaps_stbox_stbox(&box1, &box2e))
  {
    SpanSet *ss = temporal_time(temp);
    Temporal *result = (Temporal *) tsequenceset_from_base_tstzspanset(
      BoolGetDatum(false), T_TBOOL, ss, STEP);
    pfree(ss);
    return result;
  }

  void *ctx = geo_edge_ctx_make(gs);
  if (! ctx)
    return NULL;
  Temporal *result = tpoint_linear_dwithin_geom_ctx(temp, ctx, dist);
  geo_edge_ctx_free(ctx);
  return result;
}

/*****************************************************************************
 * Temporal distance (tDistance) native engine
 *
 * Distance-value sibling of the within-distance engine above. It produces the
 * temporal float distance from a moving point to a whole (possibly curved) 2D
 * geometry, lifting the point-operand-only restriction of the generic lifting
 * path (whose per-segment turning-point function can only represent the
 * distance to a single static point, i.e. at most one interior extremum).
 *
 * For each trajectory segment the distance to the geometry is the pointwise
 * minimum, over all edges, of the exact point-to-edge distance. Its turning
 * points are the union of the per-edge critical times: the perpendicular-foot
 * and endpoint-closest-approach times of a straight edge, the radial extremum
 * and angular-sector crossing times of an arc edge, and the region-boundary
 * times where the nearest feature of an edge changes. At every such time the
 * exact distance to the whole geometry is emitted as a temporal float instant,
 * with linear interpolation in between, exactly as the point-to-point temporal
 * distance samples its analytic turning points. The global minimum of the
 * distance over a segment is min over edges of the per-edge minimum over the
 * segment (the two minimisations commute), so emitting every per-edge extremum
 * makes minValue exact.
 *****************************************************************************/

/**
 * @brief Return the exact distance from a point to the whole geometry, taking
 * the polygon interior into account (a point inside a filled polygon is at
 * distance zero)
 */
static double
point_geom_dist(double px, double py, Edge **edges, int nedges)
{
  double best = point_edge_dist2(px, py, edges[0]);
  for (int i = 1; i < nedges; i++)
  {
    const double d2 = point_edge_dist2(px, py, edges[i]);
    if (d2 < best)
      best = d2;
  }
  /* On (or numerically on) the boundary: distance is zero */
  if (best <= MEOS_GEOM_TOLERANCE)
    return 0.0;
  /* Strictly inside a filled polygon: distance is zero. The horizontal-ray
   * even-odd test of #point_in_polygon miscounts when the query height aligns
   * exactly with a vertex or an arc junction, which the turning-point sampler
   * can hit deterministically. Take the majority vote of the test at the point
   * and at two tiny vertical nudges that move the ray off any aligned junction;
   * the nudge is far below any real feature size so a strictly interior or
   * strictly exterior point is unaffected */
  const double eps = 1e-9 * Max(1.0, fabs(py));
  int inside = point_in_polygon(px, py, edges, nedges) +
    point_in_polygon(px, py + eps, edges, nedges) +
    point_in_polygon(px, py - eps, edges, nedges);
  return (inside >= 2) ? 0.0 : sqrt(best);
}

/**
 * @brief Append to the event array the [0,1] trajectory-segment critical times
 * of the distance from the moving point to one edge
 * @details The candidates are the local extrema and nearest-feature switch
 * times of the exact point-to-edge distance: for a point the single closest
 * approach; for a straight edge the perpendicular-foot time, the two
 * endpoint-closest-approach times, and the two foot-parameter region
 * boundaries; for an arc edge the radial extremum, the supporting-circle
 * crossings (where the distance reaches its zero minimum), the two
 * endpoint-closest approaches, and the two angular-sector boundary crossings.
 * Spurious candidates are harmless because the distance value emitted at each
 * time is the exact distance to the whole geometry
 */
static void
distance_cands_from_edge(double ax, double ay, double rx, double ry,
  const Edge *e, MeosArray *ev)
{
  const double A = rx * rx + ry * ry;
  /* Constant (zero-length) trajectory segment: no interior turning point */
  if (A < MEOS_GEOM_TOLERANCE)
    return;
  switch (e->etype)
  {
    case EDGE_POINT:
    {
      const double wx = ax - e->x1, wy = ay - e->y1;
      add_within_root(-(wx * rx + wy * ry) / A, ev);
      return;
    }
    case EDGE_LINESEG:
    case EDGE_POLYSEG:
    {
      const double w0x = ax - e->x1, w0y = ay - e->y1;
      const double w1x = ax - e->x2, w1y = ay - e->y2;
      /* Closest approach to each segment endpoint */
      add_within_root(-(w0x * rx + w0y * ry) / A, ev);
      add_within_root(-(w1x * rx + w1y * ry) / A, ev);
      const double ux = e->x2 - e->x1, uy = e->y2 - e->y1;
      const double l2 = ux * ux + uy * uy;
      if (l2 > MEOS_GEOM_TOLERANCE)
      {
        /* Perpendicular-foot time (moving point on the supporting line) */
        const double k1 = rx * uy - ry * ux;
        if (fabs(k1) > MEOS_GEOM_TOLERANCE)
          add_within_root(-(w0x * uy - w0y * ux) / k1, ev);
        /* Foot-parameter region boundaries (s = 0 and s = 1) */
        const double ru = rx * ux + ry * uy;
        if (fabs(ru) > MEOS_GEOM_TOLERANCE)
        {
          const double w0u = w0x * ux + w0y * uy;
          add_within_root(-w0u / ru, ev);
          add_within_root((l2 - w0u) / ru, ev);
        }
      }
      return;
    }
    case EDGE_LINEARC:
    case EDGE_POLYARC:
    {
      const double wx = ax - e->cx, wy = ay - e->cy;
      /* Radial extremum: the time at which || P(t) - center || is stationary
       * (the distance-to-arc minimum when the segment stays on one side of the
       * supporting circle) */
      add_within_root(-(wx * rx + wy * ry) / A, ev);
      /* Supporting-circle crossings, where the distance to the arc reaches its
       * zero minimum (a kink not seen by the radial extremum): the roots of
       * || P(t) - center ||^2 = radius^2 */
      add_within_quad_roots(A, 2.0 * (wx * rx + wy * ry),
        wx * wx + wy * wy - e->radius * e->radius, ev);
      /* Closest approach to each arc endpoint */
      const double w0x = ax - e->x1, w0y = ay - e->y1;
      const double w1x = ax - e->x2, w1y = ay - e->y2;
      add_within_root(-(w0x * rx + w0y * ry) / A, ev);
      add_within_root(-(w1x * rx + w1y * ry) / A, ev);
      /* Angular-sector boundary crossings (rays from the center through the
       * arc endpoints) */
      const double d0x = e->x1 - e->cx, d0y = e->y1 - e->cy;
      const double den0 = rx * d0y - ry * d0x;
      if (fabs(den0) > MEOS_GEOM_TOLERANCE)
        add_within_root(-(wx * d0y - wy * d0x) / den0, ev);
      const double d1x = e->x2 - e->cx, d1y = e->y2 - e->cy;
      const double den1 = rx * d1y - ry * d1x;
      if (fabs(den1) > MEOS_GEOM_TOLERANCE)
        add_within_root(-(wx * d1y - wy * d1x) / den1, ev);
      return;
    }
  }
}

/**
 * @brief Return the temporal float distance of one temporal sequence point
 * with linear interpolation to a geometry given as an edge array
 */
static TSequence *
tpointseq_distance_geom(const TSequence *seq, Edge **edges, int nedges)
{
  assert(seq); assert(edges); assert(nedges > 0);
  assert(seq->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(seq->flags));

  /* Singleton sequence */
  if (seq->count == 1)
  {
    const TInstant *inst = TSEQUENCE_INST_N(seq, 0);
    const POINT2D *p = DATUM_POINT2D_P(tinstant_value_p(inst));
    double d = point_geom_dist(p->x, p->y, edges, nedges);
    TInstant *resinst = tinstant_make(Float8GetDatum(d), T_TFLOAT, inst->t);
    TSequence *res = tsequence_make(&resinst, 1, true, true, LINEAR, NORMALIZE);
    pfree(resinst);
    return res;
  }

  /* Upper bound on the number of result instants: the two endpoints of every
   * segment plus up to six interior turning points per edge and per segment */
  int maxinsts = 1 + (seq->count - 1) * (nedges * 6 + 3);
  TInstant **instants = palloc(sizeof(TInstant *) * maxinsts);
  int ninsts = 0;
  const TInstant *inst1 = TSEQUENCE_INST_N(seq, 0);
  const POINT2D *a = DATUM_POINT2D_P(tinstant_value_p(inst1));
  instants[ninsts++] = tinstant_make(
    Float8GetDatum(point_geom_dist(a->x, a->y, edges, nedges)), T_TFLOAT,
    inst1->t);
  /* Loop for each segment */
  for (int i = 1; i < seq->count; i++)
  {
    const TInstant *inst2 = TSEQUENCE_INST_N(seq, i);
    const POINT2D *b = DATUM_POINT2D_P(tinstant_value_p(inst2));
    const double ax = a->x, ay = a->y, rx = b->x - ax, ry = b->y - ay;

    /* Gather the interior turning points of the distance to every edge */
    events->count = 0;
    for (int j = 0; j < nedges; j++)
      distance_cands_from_edge(ax, ay, rx, ry, edges[j], events);

    /* Sort the candidate parameters and emit an instant for each interior one
     * with the exact distance to the whole geometry */
    qsort(events->elems, events->count, sizeof(double), float8_qsort_cmp);
    const double *ev = (double *) events->elems;
    const double duration = (double) (inst2->t - inst1->t);
    TimestampTz prevt = inst1->t;
    for (int k = 0; k < (int) events->count; k++)
    {
      const double p = ev[k];
      if (p <= MEOS_GEOM_TOLERANCE || p >= 1.0 - MEOS_GEOM_TOLERANCE)
        continue;
      if (k > 0 && fabs(p - ev[k - 1]) < MEOS_GEOM_TOLERANCE)
        continue;
      TimestampTz t = inst1->t + (TimestampTz) (duration * p);
      /* Keep the instants strictly increasing and off the segment endpoints */
      if (t <= prevt || t >= inst2->t)
        continue;
      const double x = ax + p * rx, y = ay + p * ry;
      instants[ninsts++] = tinstant_make(
        Float8GetDatum(point_geom_dist(x, y, edges, nedges)), T_TFLOAT, t);
      prevt = t;
    }
    /* End instant of the segment */
    instants[ninsts++] = tinstant_make(
      Float8GetDatum(point_geom_dist(b->x, b->y, edges, nedges)), T_TFLOAT,
      inst2->t);
    inst1 = inst2;
    a = b;
  }

  return tsequence_make_free(instants, ninsts, seq->period.lower_inc,
    seq->period.upper_inc, LINEAR, NORMALIZE);
}

/**
 * @ingroup meos_internal_geo
 * @brief Return the temporal float distance between a temporal geometric point
 * with linear interpolation and a 2D geometry
 * @details Native counterpart of the generic distance lifting for a
 * non-point geometry operand: the distance to a multi-edge or curved target
 * has an arbitrary number of turning points per segment which the point-only
 * base turning-point function cannot represent. The result is a temporal float
 * with linear interpolation whose values at the analytic turning points and at
 * the trajectory instants are the exact distance to the geometry
 * @pre The arguments have the same SRID, are 2D and planar, and the geometry
 * is not empty and is supported by the clip engine. This is verified by the
 * caller
 */
Temporal *
tpoint_linear_distance_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  assert(temp); assert(gs); assert(temp->temptype == T_TGEOMPOINT);
  assert(MEOS_FLAGS_LINEAR_INTERP(temp->flags));
  assert(temp->subtype != TINSTANT);
  assert(! MEOS_FLAGS_GET_GEODETIC(temp->flags));
  assert(! gserialized_is_empty(gs));

  /* Extract the edges */
  LWGEOM *geom = lwgeom_from_gserialized(gs);
  MeosArray *edges = geom_extract_edges(geom);
  lwgeom_free(geom);
  Edge **edge_ptrs = palloc(sizeof(Edge *) * edges->count);
  for (int i = 0; i < (int) edges->count; i++)
    edge_ptrs[i] = (Edge *) meos_array_get(edges, i);

  /* Static array accumulating the per-segment candidate turning times */
  events = meos_array_create(sizeof(double));

  Temporal *result;
  assert(temptype_subtype(temp->subtype));
  if (temp->subtype == TSEQUENCE)
    result = (Temporal *) tpointseq_distance_geom((TSequence *) temp,
      edge_ptrs, edges->count);
  else /* TSEQUENCESET */
  {
    const TSequenceSet *ss = (TSequenceSet *) temp;
    TSequence **sequences = palloc(sizeof(TSequence *) * ss->count);
    for (int i = 0; i < ss->count; i++)
      sequences[i] = tpointseq_distance_geom(TSEQUENCESET_SEQ_N(ss, i),
        edge_ptrs, edges->count);
    result = (Temporal *) tsequenceset_make_free(sequences, ss->count,
      NORMALIZE);
  }

  /* Clean up and return */
  meos_array_destroy(events);
  meos_array_destroy(edges); pfree(edge_ptrs);
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
    /* Nothing intersects the geometry, so the result is the whole value. Return
     * it in the same container the partial-minus path below produces (a
     * continuous sequence yields a sequence set) so that minusGeometry is
     * container-consistent with tgeo_restrict_geom whether or not the geometry
     * is met. */
    return (temp->subtype == TSEQUENCE) ?
      (Temporal *) tsequence_as_tsequenceset((const TSequence *) temp) :
      temporal_copy(temp);

  SpanSet *ss = temporal_time(result_at);
  Temporal *result = temporal_restrict_tstzspanset(temp, ss, atfunc);
  pfree(ss); pfree(result_at);
  return result;
}

/*****************************************************************************/