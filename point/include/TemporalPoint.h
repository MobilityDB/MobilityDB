/*****************************************************************************
 *
 * TemporalPoint.h
 *	  Functions for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALPOINT_H__
#define __TEMPORALPOINT_H__

#include "TemporalTypes.h"
#include "PostGIS.h"
#include "GeoEstimate.h"

/*****************************************************************************
 * Macros for manipulating the 'typemod' int. An int32_t used as follows:
 * Plus/minus = Top bit.
 * Spare bits = Next 2 bits.
 * SRID = Next 21 bits.
 * TYPE = Next 6 bits.
 * ZM Flags = Bottom 2 bits.
 *****************************************************************************/

/* The following (commented out) definitions are taken from POSTGIS
#define TYPMOD_GET_SRID(typmod) ((((typmod) & 0x0FFFFF00) - ((typmod) & 0x10000000)) >> 8)
#define TYPMOD_SET_SRID(typmod, srid) ((typmod) = (((typmod) & 0xE00000FF) | ((srid & 0x001FFFFF)<<8)))
#define TYPMOD_GET_TYPE(typmod) ((typmod & 0x000000FC)>>2)
#define TYPMOD_SET_TYPE(typmod, type) ((typmod) = (typmod & 0xFFFFFF03) | ((type & 0x0000003F)<<2))
#define TYPMOD_GET_Z(typmod) ((typmod & 0x00000002)>>1)
#define TYPMOD_SET_Z(typmod) ((typmod) = typmod | 0x00000002)
#define TYPMOD_GET_M(typmod) (typmod & 0x00000001)
#define TYPMOD_SET_M(typmod) ((typmod) = typmod | 0x00000001)
#define TYPMOD_GET_NDIMS(typmod) (2+TYPMOD_GET_Z(typmod)+TYPMOD_GET_M(typmod))
*/

/* In order to reuse the above (commented out) macros for manipulating the
   typmod from POSTGIS we need to shift them to take into account that the 
   first 4 bits are taken for the duration type */

#define TYPMOD_DEL_DURATION(typmod) (typmod = typmod >> 4 )
#define TYPMOD_SET_DURATION(typmod, durtype) ((typmod) = typmod << 4 | durtype)

/*****************************************************************************
 * Indexing constants
 *****************************************************************************/

/* Strategy number */

#define GeoStrategyNumberGroup			23
#define GBoxDStrategyNumberGroup		24
#define TPointInstStrategyNumberGroup	25
#define TPointIStrategyNumberGroup		26
#define TPointSeqStrategyNumberGroup	27
#define TPointSStrategyNumberGroup		28

/*****************************************************************************
 * GBOX macros
 *****************************************************************************/

#define DatumGetGboxP(X)    ((GBOX *) DatumGetPointer(X))
#define GboxPGetDatum(X)    PointerGetDatum(X)
#define PG_GETARG_GBOX_P(n) DatumGetGboxP(PG_GETARG_DATUM(n))
#define PG_RETURN_GBOX_P(x) return GboxPGetDatum(x)

/*****************************************************************************
 * Parsing routines: File Parser.c
 *****************************************************************************/

extern GBOX *gbox_parse(char **str);
extern Temporal *tpoint_parse(char **str, Oid basetype);

/*****************************************************************************
 * Miscellaneous functions defined in TemporalPoint.c
 *****************************************************************************/

extern void temporalgeom_init();

/* Input/output functions */

extern Datum tpoint_in(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum tpoint_value(PG_FUNCTION_ARGS);
extern Datum tpoint_values(PG_FUNCTION_ARGS);
extern Datum tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum tpoint_ever_equals(PG_FUNCTION_ARGS);
extern Datum tpoint_always_equals(PG_FUNCTION_ARGS);

extern Datum tpoint_values_internal(Temporal *temp);

extern bool tpointinst_ever_equals(TemporalInst *inst, GSERIALIZED *value);

extern Datum tgeompointi_values(TemporalI *ti);
extern Datum tgeogpointi_values(TemporalI *ti);
extern Datum tpointi_values(TemporalI *ti);

/* Restriction functions */

extern Datum tpoint_at_value(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_value(PG_FUNCTION_ARGS);
extern Datum tpoint_at_values(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_values(PG_FUNCTION_ARGS);
extern Datum tpoints_at_values(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Spatial functions defined in TemporalGeo.c
 *****************************************************************************/

extern POINT2D gs_get_point2d(GSERIALIZED *gs);
extern POINT3DZ gs_get_point3dz(GSERIALIZED *gs);
extern POINT2D datum_get_point2d(Datum value);
extern POINT3DZ datum_get_point3dz(Datum value);
extern GSERIALIZED* geometry_serialize(LWGEOM* geom);

/* Functions for output in WKT format */

extern Datum tpoint_astext(PG_FUNCTION_ARGS);
extern Datum tpoint_asewkt(PG_FUNCTION_ARGS);
extern Datum geoarr_astext(PG_FUNCTION_ARGS);
extern Datum geoarr_asewkt(PG_FUNCTION_ARGS);
extern Datum tpointarr_astext(PG_FUNCTION_ARGS);
extern Datum tpointarr_asewkt(PG_FUNCTION_ARGS);

/* Functions for spatial reference systems */

extern Datum tpoint_srid(PG_FUNCTION_ARGS);
extern Datum tpoint_set_srid(PG_FUNCTION_ARGS);
extern Datum tgeompoint_transform(PG_FUNCTION_ARGS);

extern int tpoint_srid_internal(Temporal *t);
extern TemporalInst *tgeompointinst_transform(TemporalInst *inst, Datum srid);

/* Cast functions */

extern Datum tgeompoint_as_tgeogpoint(PG_FUNCTION_ARGS);
extern Datum tgeogpoint_as_tgeompoint(PG_FUNCTION_ARGS);

extern TemporalInst *tgeogpointinst_as_tgeompointinst(TemporalInst *inst);
extern TemporalI *tgeogpointi_as_tgeompointi(TemporalI *ti);
extern TemporalSeq *tgeogpointseq_as_tgeompointseq(TemporalSeq *seq);
extern TemporalS *tgeogpoints_as_tgeompoints(TemporalS *ts);

/* Trajectory functions */

extern Datum tgeompoint_synctrajectory(PG_FUNCTION_ARGS);
extern Datum tgeompoint_synctrajectorypers(PG_FUNCTION_ARGS);

extern Datum tpoint_trajectory(PG_FUNCTION_ARGS);

extern Datum tpointseq_make_trajectory(TemporalInst **instants, int count);

extern Datum geompoint_trajectory(Datum value1, Datum value2);
extern Datum tgeompointseq_trajectory1(TemporalInst *inst1, TemporalInst *inst2);
extern Datum tgeogpointseq_trajectory1(TemporalInst *inst1, TemporalInst *inst2);

extern Datum tpointseq_trajectory(TemporalSeq *seq);
extern Datum tpointseq_trajectory_copy(TemporalSeq *seq);
extern Datum tpoints_trajectory(TemporalS *ts);

/* Length, speed, time-weighted centroid, and temporal azimuth functions */

extern Datum tpoint_length(PG_FUNCTION_ARGS);
extern Datum tpoint_cumulative_length(PG_FUNCTION_ARGS);
extern Datum tpoint_speed(PG_FUNCTION_ARGS);
extern Datum tgeompoint_twcentroid(PG_FUNCTION_ARGS);
extern Datum tpoint_azimuth(PG_FUNCTION_ARGS);

/* Restriction functions */

extern Datum tpoint_at_geometry(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_geometry(PG_FUNCTION_ARGS);

extern TemporalSeq **tpointseq_at_geometry2(TemporalSeq *seq, Datum geo, int *count);

/* Nearest approach functions */

extern Datum NAI_tpoint_geometry(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum NAD_geometry_tpoint(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_geometry(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum shortestline_geometry_tpoint(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_geometry(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_tpoint(PG_FUNCTION_ARGS);

/* Functions converting a temporal point to/from a PostGIS trajectory */

extern Datum tpoint_to_geo(PG_FUNCTION_ARGS);
extern Datum geo_to_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Restriction functions defined in ProjectionGK.c
 *****************************************************************************/

extern Datum tgeompoint_transform_gk(PG_FUNCTION_ARGS);
extern Datum geometry_transform_gk(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Geometric aggregate functions defined in GeoAggFuncs.c
 *****************************************************************************/

extern Datum tpoint_tcentroid_transfn(PG_FUNCTION_ARGS);
extern Datum tpoint_tcentroid_combinefn(PG_FUNCTION_ARGS);
extern Datum tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Spatial relationship functions defined in SpatialRels.c
 *****************************************************************************/

extern Datum geom_contains(Datum geom1, Datum geom2);
extern Datum geom_containsproperly(Datum geom1, Datum geom2);
extern Datum geom_containedproperlyby(Datum geom1, Datum geom2);
extern Datum geom_covers(Datum geom1, Datum geom2);
extern Datum geom_coveredby(Datum geom1, Datum geom2);
extern Datum geom_crosses(Datum geom1, Datum geom2);
extern Datum geom_crossedby(Datum geom1, Datum geom2);
extern Datum geom_disjoint(Datum geom1, Datum geom2);
extern Datum geom_equals(Datum geom1, Datum geom2);
extern Datum geom_intersects2d(Datum geom1, Datum geom2);
extern Datum geom_intersects3d(Datum geom1, Datum geom2);
extern Datum geom_overlaps(Datum geom1, Datum geom2);
extern Datum geom_touches(Datum geom1, Datum geom2);
extern Datum geom_within(Datum geom1, Datum geom2);
extern Datum geom_dwithin2d(Datum geom1, Datum geom2, Datum dist);
extern Datum geom_dwithin3d(Datum geom1, Datum geom2, Datum dist);
extern Datum geom_relate(Datum geom1, Datum geom2);
extern Datum geom_relate_pattern(Datum geom1, Datum geom2, Datum pattern);

extern Datum geog_covers(Datum geog1, Datum geog2);
extern Datum geog_coveredby(Datum geog1, Datum geog2);
extern Datum geog_disjoint(Datum geog1, Datum geog2);
extern Datum geog_intersects(Datum geog1, Datum geog2);
extern Datum geog_dwithin(Datum geog1, Datum geog2, Datum dist);

extern Datum contains_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum contains_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum contains_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum containsproperly_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum containsproperly_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum containsproperly_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum covers_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum covers_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum covers_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum coveredby_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum coveredby_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum coveredby_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum crosses_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum crosses_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum crosses_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum disjoint_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum disjoint_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum disjoint_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum equals_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum equals_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum equals_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum intersects_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum intersects_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum intersects_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum overlaps_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum overlaps_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum touches_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum touches_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum touches_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum within_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum within_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum within_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum dwithin_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum dwithin_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum dwithin_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum relate_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum relate_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum relate_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum relate_pattern_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum relate_pattern_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum relate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Temporal spatial relationship functions defined in TempSpatialRels.c
 *****************************************************************************/

extern Datum tcontains_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcontains_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcontains_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tcontainsproperly_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcontainsproperly_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcontainsproperly_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tcovers_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcovers_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcovers_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tcoveredby_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tcoveredby_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tcoveredby_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tdisjoint_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tdisjoint_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tdisjoint_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tequals_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tequals_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tequals_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tintersects_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tintersects_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tintersects_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum ttouches_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum ttouches_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum ttouches_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum twithin_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum twithin_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum twithin_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum tdwithin_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum tdwithin_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum trelate_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum trelate_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum trelate_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum trelate_pattern_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum trelate_pattern_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum trelate_pattern_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Bounding box operators defined in BoundBoxOps.c
 *****************************************************************************/

/* GBOX functions */

extern Datum contains_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum contained_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overlaps_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum same_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum distance_gbox_gbox(PG_FUNCTION_ARGS);

extern bool contains_gbox_gbox_internal(const GBOX *box1, const GBOX *box2);
extern bool contained_gbox_gbox_internal(const GBOX *box1, const GBOX *box2);
extern bool overlaps_gbox_gbox_internal(const GBOX *box1, const GBOX *box2);
extern bool same_gbox_gbox_internal(const GBOX *box1, const GBOX *box2);
extern double distance_gbox_gbox_internal(GBOX *box1, GBOX *box2);

/* Functions computing the bounding box at the creation of the temporal point */

extern void tpointinst_make_gbox(GBOX *box, Datum value, TimestampTz t);
extern void tpointinstarr_to_gbox(GBOX *box, TemporalInst **inst, int count) ;
extern void tpointseqarr_to_gbox(GBOX *box, TemporalSeq **seq, int count) ;

/* Functions for expanding the bounding box */

extern Datum gbox_expand_spatial(PG_FUNCTION_ARGS);
extern Datum tpoint_expand_spatial(PG_FUNCTION_ARGS);
extern Datum gbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum tpoint_expand_temporal(PG_FUNCTION_ARGS);

/* Transform a <Type> to a GBOX */

extern Datum geo_to_gbox(PG_FUNCTION_ARGS);
extern Datum geo_timestamp_to_gbox(PG_FUNCTION_ARGS);
extern Datum geo_period_to_gbox(PG_FUNCTION_ARGS);
extern Datum tpoint_to_gbox(PG_FUNCTION_ARGS);

extern bool geo_to_gbox_internal(GBOX *box, GSERIALIZED *gs);
extern void timestamp_to_gbox_internal(GBOX *box, TimestampTz t);
extern void timestampset_to_gbox_internal(GBOX *box, TimestampSet *ps);
extern void period_to_gbox_internal(GBOX *box, Period *p);
extern void periodset_to_gbox_internal(GBOX *box, PeriodSet *ps);
extern bool geo_timestamp_to_gbox_internal(GBOX *box, GSERIALIZED* geom, TimestampTz t);
extern bool geo_period_to_gbox_internal(GBOX *box, GSERIALIZED* geom, Period *period);

/*****************************************************************************/

extern Datum overlaps_bbox_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum contains_bbox_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum contains_bbox_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum contained_bbox_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum contained_bbox_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum same_bbox_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum same_bbox_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum same_bbox_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum same_bbox_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum same_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Relative position functions defined in RelativePosOpsM.c
 *****************************************************************************/

extern Datum left_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overleft_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum right_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overright_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum below_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overbelow_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum above_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overabove_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum front_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overfront_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum back_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overback_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum before_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overbefore_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum after_gbox_gbox(PG_FUNCTION_ARGS);
extern Datum overafter_gbox_gbox(PG_FUNCTION_ARGS);

extern bool left_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overleft_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool right_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overright_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool below_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overbelow_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool above_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overabove_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool front_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overfront_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool back_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overback_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool before_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overbefore_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool after_gbox_gbox_internal(GBOX *box1, GBOX *box2);
extern bool overafter_gbox_gbox_internal(GBOX *box1, GBOX *box2);

extern Datum left_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum right_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum below_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum above_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum front_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum back_geom_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_geom_tpoint(PG_FUNCTION_ARGS);

extern Datum left_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum right_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum above_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum below_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum front_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum back_tpoint_geom(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_geom(PG_FUNCTION_ARGS);

extern Datum left_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum right_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum below_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum above_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum front_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum back_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum before_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overbefore_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum after_gbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overafter_gbox_tpoint(PG_FUNCTION_ARGS);

extern Datum left_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum right_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum above_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum below_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum front_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum back_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum before_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overbefore_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum after_tpoint_gbox(PG_FUNCTION_ARGS);
extern Datum overafter_tpoint_gbox(PG_FUNCTION_ARGS);

extern Datum left_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overleft_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum right_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overright_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum above_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overabove_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum below_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overbelow_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum front_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overfront_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum back_tpoint_tpoint(PG_FUNCTION_ARGS);
extern Datum overback_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Temporal distance functions defined in TempDistance.c
 *****************************************************************************/

extern Datum geom_distance2d(Datum geom1, Datum geom2);
extern Datum geom_distance3d(Datum geom1, Datum geom2);
extern Datum geog_distance(Datum geog1, Datum geog2);

extern Datum distance_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum distance_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Index functions defined in IndexGistTPoint.c
 *****************************************************************************/

extern Datum gist_tpoint_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_union(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_penalty(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_picksplit(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_same(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_compress(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_decompress(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_distance(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_fetch(PG_FUNCTION_ARGS);

/* The following functions are also called by IndexSpgistTPoint.c */
extern bool index_tpoint_bbox_recheck(StrategyNumber strategy);
extern bool index_leaf_consistent_gbox_box2D(GBOX *key, GBOX *query, 
	StrategyNumber strategy);
extern bool index_leaf_consistent_gbox_period(GBOX *key, Period *query, 
	StrategyNumber strategy);
extern bool index_leaf_consistent_gbox_gbox(GBOX *key, GBOX *query, 
	StrategyNumber strategy);

extern bool index_tpoint_recheck(StrategyNumber strategy);
extern bool index_leaf_consistent_gbox(GBOX *key, GBOX *query, 
	StrategyNumber strategy);

/*****************************************************************************
 * Index functions defined in IndexSpgistTPoint.c
 *****************************************************************************/

extern Datum spgist_tgeogpoint_config(PG_FUNCTION_ARGS);
extern Datum spgist_tpoint_choose(PG_FUNCTION_ARGS);
extern Datum spgist_tpoint_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_tpoint_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tpoint_leaf_consistent(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
