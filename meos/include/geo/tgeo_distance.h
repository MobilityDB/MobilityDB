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
 * @brief Distance functions for temporal points.
 */

#ifndef __TGEO_DISTANCE_H__
#define __TGEO_DISTANCE_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include "temporal/temporal.h"
#include <meos_tls.h>

/*****************************************************************************/

extern bool point3d_min_dist(const POINT3DZ *p1, const POINT3DZ *p2,
  const POINT3DZ *p3, const POINT3DZ *p4, double *fraction);

extern int tgeompointsegm_distance_turnpt(Datum start1, Datum end1,
  Datum start2, Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
extern int tgeogpointsegm_distance_turnpt(Datum start1, Datum end1,
  Datum start2, Datum end2, Datum param UNUSED, TimestampTz lower, TimestampTz upper,
  TimestampTz *t1, TimestampTz *t2);
  
extern double tinstant_distance(const TInstant *inst1, const TInstant *inst2,
  datum_func2 func);

/*****************************************************************************/

/* GEOS-free analytic distance engine shared with the temporal circular
 * buffer family: a moving disc (radius may be 0, i.e. a moving point)
 * against a decomposed geometry, indexed by a Morton bucket hierarchy for
 * the nearest-approach (knn) kernels and the generic R-tree elsewhere. */

/**
 * @brief A geometry boundary segment with its precomputed 2D bounding box;
 * @p is_poly marks segments that belong to a polygon ring (used by the
 * ray-casting interior test)
 */
typedef struct
{
  double x1, y1, x2, y2;
  double xmin, ymin, xmax, ymax;
  bool is_poly;
  /* Circular-arc edge (from a CIRCULARSTRING): when @p is_arc is true the edge
   * is the arc of the circle centred at (@p acx, @p acy) with radius @p arad,
   * from angle @p at0 to @p at1 traversed counterclockwise when @p accw. The
   * endpoints (x1,y1)/(x2,y2) are the arc start/end; the bounding box already
   * accounts for the arc bulge (cardinal extremes within the angular span). */
  bool is_arc;
  double acx, acy, arad, at0, at1;
  bool accw;
} GeoDistEdge;

/**
 * @brief Witness of the nearest approach: the point @p (px,py) on the
 * swept-capsule boundary and @p (qx,qy) on the geometry
 */
typedef struct
{
  double d;
  double px, py, qx, qy;
  bool set;
} GeoDistShortLine;

/**
 * @brief A spatially-local group of consecutive segments with its bounding box
 */
typedef struct
{
  int start, n;                  /**< Range [start, start+n) in the segments */
  double xmin, ymin, xmax, ymax; /**< Bounding box of the bucket */
} GeoDistBucket;

/**
 * @brief Geometry context over the boundary segments and its overall bounding
 * box, with whether any polygon ring is present. Exactly one spatial index is
 * built: the unbounded nearest-approach kernels (nad/shortestLine/nai) use the
 * Morton bucket hierarchy @p bks; the fixed-reach relationship kernels
 * (within/touches/contains/covers) use the generic R-tree @p rtree.
 */
typedef struct
{
  const GeoDistEdge *segs;
  int n;
  bool has_poly;
  double xmin, ymin, xmax, ymax; /**< Overall geometry bounding box */
  const GeoDistBucket *bks; /**< Morton bucket BVH (nad path), or NULL */
  int nbk;
  RTree *rtree;                  /**< Generic R-tree (relationship path), or NULL */
} GeoDistGeom;

/**
 * @brief Running witness for the nearest approach instant: the minimum
 * swept-capsule distance and the timestamp attaining it
 */
typedef struct
{
  double d;        /**< signed minimum disc-to-geometry distance (< 0: overlap) */
  TimestampTz t;   /**< timestamp attaining the minimum */
  bool set;        /**< whether a candidate has been recorded */
} GeoDistNai;

extern bool geodist_geom_build(const GSERIALIZED *gs, GeoDistGeom *g);
extern void geodist_geom_free(GeoDistGeom *g);
extern bool geodist_geom_edges(const LWGEOM *lw, bool allow_arc, GeoDistEdge **arr, int *cap,
  int *cnt, bool *has_poly);
extern double geodist_segm_edge_mindist(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const GeoDistEdge *e);
extern double geodist_segm_arc_mindist(double cx1, double cy1, double cx2, double cy2,
  double r1, double r2, const GeoDistEdge *e);
extern MEOS_TLS MeosArray *geodist_pip_results;
extern bool geodist_geom_point_inside(double x, double y, const GeoDistGeom *g);
extern void geodist_segm_nad(double cx1, double cy1, double r1, double cx2, double cy2,
  double r2, const GeoDistGeom *g, double *best);
extern void geodist_segm_shortestline(double cx1, double cy1, double r1, double cx2,
  double cy2, double r2, const GeoDistGeom *g, GeoDistShortLine *w);
extern void geodist_segm_nai(double cx1, double cy1, double r1, TimestampTz t1, double cx2,
  double cy2, double r2, TimestampTz t2, const GeoDistGeom *g, GeoDistNai *w);
extern RTree * geodist_geom_build_rtree(const GeoDistEdge *segs, int n);

/*****************************************************************************/

#endif /* __TGEO_DISTANCE_H__ */
