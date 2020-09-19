/*****************************************************************************
 *
 * tpoint_spatialfuncs.h
 *    Spatial functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_SPATIALFUNCS_H__
#define __TPOINT_SPATIALFUNCS_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <catalog/pg_type.h>
#include <float.h>

#include "temporal.h"
#include <liblwgeom.h>
#include "tpoint.h"

/*****************************************************************************/

/* Fetch from and store in the cache the fcinfo of the external function */

extern FunctionCallInfo fetch_fcinfo();
extern void store_fcinfo(FunctionCallInfo fcinfo);

/* Functions derived from PostGIS to increase floating-point precision */

extern double closest_point2d_on_segment_ratio(const POINT2D *p, const POINT2D *A,
  const POINT2D *B, POINT2D *closest);
extern double closest_point3dz_on_segment_ratio(const POINT3DZ *p, const POINT3DZ *A,
  const POINT3DZ *B, POINT3DZ *closest);
extern double closest_point_on_segment_sphere(const POINT4D *p, const POINT4D *A,
  const POINT4D *B, POINT4D *closest, double *dist);
extern void interpolate_point4d_sphere(const POINT3D *p1, const POINT3D *p2,
  const POINT4D *v1, const POINT4D *v2, double f, POINT4D *p);

/* Parameter tests */

extern void ensure_spatial_validity(const Temporal *temp1, const Temporal *temp2);
extern void ensure_same_geodetic_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_same_geodetic_tpoint_stbox(const Temporal *temp, const STBOX *box);
extern void ensure_same_srid_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_same_srid_tpoint(const Temporal *temp1, const Temporal *temp2);
extern void ensure_same_srid_tpoint_stbox(const Temporal *temp, const STBOX *box);
extern void ensure_same_srid_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs);
extern void ensure_same_dimensionality_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_same_dimensionality_tpoint(const Temporal *temp1, const Temporal *temp2);
extern void ensure_same_spatial_dimensionality_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_same_spatial_dimensionality_tpoint_stbox(const Temporal *temp, const STBOX *box);
extern void ensure_same_dimensionality_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_same_dimensionality_tpoint_stbox(const Temporal *temp, const STBOX *box);
extern void ensure_same_dimensionality_tpoint_gs(const Temporal *temp, const GSERIALIZED *gs);
extern void ensure_common_dimension_stbox(const STBOX *box1, const STBOX *box2);
extern void ensure_has_X_stbox(const STBOX *box);
extern void ensure_has_Z_stbox(const STBOX *box);
extern void ensure_has_T_stbox(const STBOX *box);
extern void ensure_has_not_Z_tpoint(const Temporal *temp);
extern void ensure_has_not_Z_gs(const GSERIALIZED *gs);
extern void ensure_has_M_gs(const GSERIALIZED *gs);
extern void ensure_has_not_M_gs(const GSERIALIZED *gs);
extern void ensure_point_type(const GSERIALIZED *gs);
extern void ensure_non_empty(const GSERIALIZED *gs);

/* Utility functions */

extern const POINT2D *gs_get_point2d_p(GSERIALIZED *gs);
extern const POINT3DZ *gs_get_point3dz_p(GSERIALIZED *gs);
extern POINT2D datum_get_point2d(Datum value);
extern const POINT2D *datum_get_point2d_p(Datum value);
extern POINT3DZ datum_get_point3dz(Datum value);
extern const POINT3DZ *datum_get_point3dz_p(Datum value);
extern POINT4D datum_get_point4d(Datum value);
extern bool datum_point_eq(Datum geopoint1, Datum geopoint2);
extern Datum datum2_point_eq(Datum geopoint1, Datum geopoint2);
extern Datum datum2_point_ne(Datum geopoint1, Datum geopoint2);
extern GSERIALIZED *geo_serialize(LWGEOM *geom);
extern Datum datum_transform(Datum value, Datum srid);

extern Datum geom_distance2d(Datum geom1, Datum geom2);
extern Datum geom_distance3d(Datum geom1, Datum geom2);
extern Datum geog_distance(Datum geog1, Datum geog2);
extern Datum pt_distance2d(Datum geom1, Datum geom2);
extern Datum pt_distance3d(Datum geom1, Datum geom2);

extern double lw_dist_sphere_point_dist(const LWGEOM *lw1, const LWGEOM *lw2,
  int mode, double *fraction);

extern Datum geoseg_interpolate_point(Datum value1, Datum value2, double ratio);
extern double geoseg_locate_point(Datum start, Datum end, Datum point, double *dist);

extern void spheroid_init(SPHEROID *s, double a, double b);
extern void geography_interpolate_point4d(const POINT3D *p1, const POINT3D *p2,
  const POINT4D *v1, const POINT4D *v2, double f, POINT4D *p);

/* Functions for spatial reference systems */

extern Datum tpoint_srid(PG_FUNCTION_ARGS);
extern Datum tpoint_set_srid(PG_FUNCTION_ARGS);

extern Temporal *tpoint_set_srid_internal(Temporal *temp, int32 srid) ;
extern int tpointinst_srid(const TInstant *inst);
extern int tpointinstset_srid(const TInstantSet *ti);
extern int tpointseq_srid(const TSequence *seq);
extern int tpointseqset_srid(const TSequenceSet *ts);
extern int tpoint_srid_internal(const Temporal *t);
extern TInstant *tpointinst_transform(const TInstant *inst, Datum srid);

/* Cast functions */

extern Datum tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS);
extern Datum tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS);

extern TInstant *tgeogpointinst_to_tgeompointinst(const TInstant *inst);
extern TSequence *tgeogpointseq_to_tgeompointseq(const TSequence *seq);
extern TSequenceSet *tgeogpoints_to_tgeompoints(const TSequenceSet *ts);

/* Trajectory functions */

extern Datum tpoint_trajectory(PG_FUNCTION_ARGS);

extern Datum tpointinstset_trajectory(const TInstantSet *ti);
extern Datum tpoint_trajectory_internal(const Temporal *temp);
extern Datum tpointseq_make_trajectory(TInstant **instants, int count, bool linear);

extern Datum geopoint_line(Datum value1, Datum value2);
extern LWLINE *geopoint_lwline(Datum value1, Datum value2);

extern Datum tpointseq_trajectory(const TSequence *seq);
extern Datum tpointseq_trajectory_copy(const TSequence *seq);
extern Datum tpointseqset_trajectory(const TSequenceSet *ts);

/* Length, speed, time-weighted centroid, and temporal azimuth functions */

extern Datum tpoint_length(PG_FUNCTION_ARGS);
extern Datum tpoint_cumulative_length(PG_FUNCTION_ARGS);
extern Datum tpoint_speed(PG_FUNCTION_ARGS);
extern Datum tgeompoint_twcentroid(PG_FUNCTION_ARGS);
extern Datum tpoint_azimuth(PG_FUNCTION_ARGS);

extern Datum tgeompointi_twcentroid(const TInstantSet *ti);
extern Datum tgeompointseq_twcentroid(const TSequence *seq);
extern Datum tgeompoints_twcentroid(const TSequenceSet *ts);

/* Restriction functions */

extern Datum tpoint_at_geometry(PG_FUNCTION_ARGS);
extern Datum tpoint_at_stbox(PG_FUNCTION_ARGS);

extern Datum tpoint_minus_geometry(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_stbox(PG_FUNCTION_ARGS);

extern TSequence **tpointseq_at_geometry2(const TSequence *seq, Datum geo, int *count);

extern Temporal *tpoint_at_geometry_internal(const Temporal *temp, Datum geo);
extern Temporal *tpoint_minus_geometry_internal(const Temporal *temp, Datum geo);

/* Distance functions */

extern Datum distance_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_tpoint(PG_FUNCTION_ARGS);

extern bool tpointseq_min_dist_at_timestamp(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  TimestampTz *t);
extern bool tgeogpointseq_min_dist_at_timestamp(const TInstant *start1,
  const TInstant *end1, const TInstant *start2, const TInstant *end2,
  double *mindist, TimestampTz *t);

extern Temporal *distance_tpoint_geo_internal(const Temporal *temp, Datum geo);
extern Temporal *distance_tpoint_tpoint_internal(const Temporal *temp1, const Temporal *temp2);

/* Nearest approach distance/instance and shortest line functions */

extern Datum NAI_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum NAD_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum shortestline_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_tpoint(PG_FUNCTION_ARGS);

/* Convert a temporal point into a PostGIS trajectory geometry/geography */

extern Datum tpoint_to_geo(PG_FUNCTION_ARGS);
extern Datum geo_to_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
