/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @brief Spatial functions for temporal points.
 */

#ifndef __TGEO_SPATIALFUNCS_H__
#define __TGEO_SPATIALFUNCS_H__

/* PostgreSQL */
#include <postgres.h>
/* PostGIS */
#include <liblwgeom.h>
#include <lwgeodetic.h>
/* MEOS */
#include <meos.h>
#include "general/temporal.h"

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define TGEOMP_TO_TGEOGP    true
#define TGEOGP_TO_TGEOMP    false

/** Symbolic constants for transforming tgeompoint <-> tgeogpoint */
#define TGEOM_TO_TGEOG      true
#define TGEOG_TO_TGEOM      false

/** Symbolic constants for transforming tgeo <-> tpoint */
#define TGEO_TO_TPOINT      true
#define TPOINT_TO_TGEO      false

/*****************************************************************************/

/* Utility functions */

extern POINT2D datum_point2d(Datum value);
extern POINT3DZ datum_point3dz(Datum value);
extern void gs_point4d(const GSERIALIZED *gs, POINT4D *p);
extern void datum_point4d(Datum value, POINT4D *p);

extern int geopoint_cmp(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geopoint_eq(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool geopoint_same(const GSERIALIZED *gs1, const GSERIALIZED *gs2);
extern bool datum_point_eq(Datum point1, Datum point2);
extern bool datum_point_same(Datum point1, Datum point2);
extern Datum datum2_point_eq(Datum point1, Datum point2);
extern Datum datum2_point_ne(Datum point1, Datum point2);
extern Datum datum2_point_same(Datum point1, Datum oint2);
extern Datum datum2_point_nsame(Datum point1, Datum point2);
extern Datum datum2_geom_centroid(Datum geo);
extern Datum datum2_geog_centroid(Datum geo);
extern GSERIALIZED *geo_serialize(const LWGEOM *geom);
extern LWPROJ *lwproj_get(int32 srid_from, int32 srid_to);

/* Generic functions */

extern datum_func2 distance_fn(int16 flags);
extern datum_func2 pt_distance_fn(int16 flags);
extern Datum datum_geom_distance2d(Datum geom1, Datum geom2);
extern Datum datum_geom_distance3d(Datum geom1, Datum geom2);
extern Datum datum_geog_distance(Datum geog1, Datum geog2);
extern Datum datum_pt_distance2d(Datum geom1, Datum geom2);
extern Datum datum_pt_distance3d(Datum geom1, Datum geom2);

extern int16 spatial_flags(Datum d, meosType basetype);

/* Validity functions */

extern bool ensure_spatial_validity(const Temporal *temp1,
  const Temporal *temp2);
extern bool ensure_not_geodetic(int16 flags);
extern bool ensure_same_geodetic(int16 flags1, int16 flags2);
extern bool ensure_same_geodetic_geo(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool ensure_same_geodetic_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_srid_known(int32_t srid);
extern bool ensure_same_srid(int32_t srid1, int32_t srid2);
extern bool ensure_same_dimensionality(int16 flags1, int16 flags2);
extern bool same_spatial_dimensionality(int16 flags1, int16 flags2);
extern bool ensure_same_spatial_dimensionality(int16 flags1, int16 flags2);
extern bool ensure_same_dimensionality_geo(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool same_dimensionality_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_same_dimensionality_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_same_spatial_dimensionality_stbox_geo(const STBox *box,
  const GSERIALIZED *gs);
extern bool ensure_same_geodetic_stbox_geo(const STBox *box,
  const GSERIALIZED *gs);
extern bool ensure_has_Z_geo(const GSERIALIZED *gs);
extern bool ensure_has_not_Z_geo(const GSERIALIZED *gs);
extern bool ensure_has_M_geo(const GSERIALIZED *gs);
extern bool ensure_has_not_M_geo(const GSERIALIZED *gs);
extern bool ensure_not_geodetic_geo(const GSERIALIZED *gs);
extern bool ensure_point_type(const GSERIALIZED *gs);
extern bool ensure_mline_type(const GSERIALIZED *gs);
extern bool circle_type(const GSERIALIZED *gs);
extern bool ensure_circle_type(const GSERIALIZED *gs);
extern bool ensure_not_empty(const GSERIALIZED *gs);
extern bool ensure_valid_stbox_geo(const STBox *box, const GSERIALIZED *gs);
extern bool ensure_valid_tspatial_geo(const Temporal *temp,
  const GSERIALIZED *gs);
extern bool ensure_valid_tspatial_tspatial(const Temporal *temp1,
  const Temporal *temp2);
extern bool ensure_valid_spatial_stbox_stbox(const STBox *box1,
  const STBox *box2);
extern bool ensure_valid_tgeo_stbox(const Temporal *temp, const STBox *box);
extern bool ensure_valid_geo_geo(const GSERIALIZED *gs1,
  const GSERIALIZED *gs2);
extern bool ensure_valid_tgeo_geo(const Temporal *temp, const GSERIALIZED *gs);
extern bool ensure_valid_tgeo_tgeo(const Temporal *temp1,
  const Temporal *temp2);
extern bool ensure_valid_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs);
extern bool ensure_valid_tpoint_tpoint(const Temporal *temp1,
  const Temporal *temp2);

extern bool mline_type(const GSERIALIZED *gs);

/* Functions for extracting coordinates */

extern Temporal *tpoint_get_coord(const Temporal *temp, int coord);

/* Ever/always comparisons */

extern int eacomp_tpoint_geo(const Temporal *temp, const GSERIALIZED *gs,
  Datum (*func)(Datum, Datum, meosType), bool ever);

/* Functions derived from PostGIS to increase floating-point precision */

extern long double closest_point2d_on_segment_ratio(const POINT2D *p,
  const POINT2D *A, const POINT2D *B, POINT2D *closest);
extern long double closest_point3dz_on_segment_ratio(const POINT3DZ *p,
  const POINT3DZ *A, const POINT3DZ *B, POINT3DZ *closest);
extern long double closest_point_on_segment_sphere(const POINT4D *p,
  const POINT4D *A, const POINT4D *B, POINT4D *closest, double *dist);
extern void interpolate_point4d_spheroid(const POINT4D *p1, const POINT4D *p2,
  POINT4D *p, const SPHEROID *s, double f);

/* Functions specializing the PostGIS functions ST_LineInterpolatePoint and
 * ST_LineLocatePoint */

extern GSERIALIZED *geopoint_make(double x, double y, double z, bool hasz,
  bool geodetic, int32 srid);
extern GSERIALIZED *geocircle_make(double x, double y, double radius,
  int32_t srid);
extern Datum pointsegm_interpolate(Datum start, Datum end,
  long double ratio);
extern long double pointsegm_locate(Datum start, Datum end, Datum point,
  double *dist);
// TODO merge this function with the previous one
extern long double pointsegm_locate_point(Datum start, Datum end, Datum point,
  double *dist);

/* Intersection functions */

extern bool tpointsegm_intersection_value(const TInstant *inst1,
  const TInstant *inst2, Datum value, TimestampTz *t);
extern bool tgeompointsegm_intersection(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  TimestampTz *t);
extern bool tgeogpointsegm_intersection(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  TimestampTz *t);

extern bool geopoint_collinear(Datum value1, Datum value2, Datum value3,
  double ratio, bool hasz, bool geodetic);

/* Trajectory functions */

extern LWGEOM **lwpointarr_remove_duplicates(LWGEOM **points, int count,
  int *newcount);
extern LWGEOM *lwpointarr_make_trajectory(LWGEOM **points, int count,
  interpType interp);
extern LWLINE *lwline_make(Datum value1, Datum value2);
extern LWGEOM *lwcoll_from_points_lines(LWGEOM **points, LWGEOM **lines,
  int npoints, int nlines);

/* Stop function */

int tpointseq_stops_iter(const TSequence *seq, double maxdist, int64 mintunits,
  TSequence **result);

/*****************************************************************************/

#endif
