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
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
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
 * @brief Structure keeping a geometry edge
 */
typedef struct
{
  double x1, y1, x2, y2;         /**< Coordinates of the start/end 2D points */
  double xmin, ymin, xmax, ymax; /**< Precomputed bounding box of the edge */
  double dx, dy, length;         /**< Precomputed dx, dy, and length */
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

extern void arc_set_bbox(Edge *e);
extern bool arc_contains_angle(const Edge *e, double phi);
extern LWGEOM *meos_oriented_envelope(const LWGEOM *geom);
extern bool meos_is_simple(const LWGEOM *geom, bool *result);
extern bool point_on_arc(double px, double py, const Edge *e);
extern bool point_on_segment(double px, double py, double x1, double y1,
  double x2, double y2);
extern int point_in_polygon(double x, double y, Edge **edges, int nedges);
extern IntersectResult linesegm_intersect(double ax, double ay, double rx,
  double ry, double cx, double cy, double dx, double dy);
extern int arcsegm_intersect(double ax, double ay, double rx, double ry,
  const Edge *e, double out[2]);
extern bool arcarc_intersect(const Edge *e1, const Edge *e2);
extern bool relate_point_on_boundary(double x, double y, Edge **edges,
  int nedges);
extern int relate_point_in_area(double x, double y, Edge **edges, int nedges);

extern bool ensure_srid_is_latlong(int32_t srid);
extern bool ensure_geodetic_geo(const GSERIALIZED *gs);
extern bool ensure_not_geodetic_geo(const GSERIALIZED *gs);
extern bool ensure_geodetic(int16 flags);
extern bool ensure_not_geodetic(int16 flags);
extern bool ensure_same_geodetic(int16 flags1, int16 flags2);
extern bool ensure_same_geodetic_geo(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool ensure_srid_known(int32_t srid);
extern bool ensure_same_srid(int32_t srid1, int32_t srid2);
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

#endif /* __GEO_FUNCS_H__ */
