/*****************************************************************************
 *
 * tpoint_spatialfuncs.h
 *	  Spatial functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_SPATIALFUNCS_H__
#define __TPOINT_SPATIALFUNCS_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <liblwgeom.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

/* Parameter tests */

extern void ensure_same_srid_tpoint(Temporal *temp1, Temporal *temp2);
extern void ensure_same_srid_tpoint_gs(Temporal *temp, GSERIALIZED *gs);
extern void ensure_same_dimensionality_tpoint(Temporal *temp1, Temporal *temp2);
extern void ensure_same_dimensionality_tpoint_gs(Temporal *temp, GSERIALIZED *gs);
extern void ensure_has_Z_tpoint(Temporal *temp);
extern void ensure_point_type(GSERIALIZED *gs);
extern void ensure_non_empty(GSERIALIZED *gs);
extern void ensure_has_Z(GSERIALIZED *gs);
extern void ensure_has_M(GSERIALIZED *gs);
extern void ensure_has_not_M(GSERIALIZED *gs);

/* Utility functions */

extern POINT2D gs_get_point2d(GSERIALIZED *gs);
extern POINT3DZ gs_get_point3dz(GSERIALIZED *gs);
extern POINT2D datum_get_point2d(Datum value);
extern POINT3DZ datum_get_point3dz(Datum value);
extern bool datum_point_eq(Datum geopoint1, Datum geopoint2);
extern GSERIALIZED* geometry_serialize(LWGEOM* geom);

/* Functions for spatial reference systems */

extern Datum tpoint_srid(PG_FUNCTION_ARGS);
extern Datum tpoint_set_srid(PG_FUNCTION_ARGS);

extern Temporal *tpoint_set_srid_internal(Temporal *temp, int32 srid) ;
extern int tpoint_srid_internal(Temporal *t);
extern TemporalInst *tgeompointinst_transform(TemporalInst *inst, Datum srid);

/* Cast functions */

extern Datum tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS);
extern Datum tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS);

extern TemporalInst *tgeogpointinst_to_tgeompointinst(TemporalInst *inst);
extern TemporalSeq *tgeogpointseq_to_tgeompointseq(TemporalSeq *seq);
extern TemporalS *tgeogpoints_to_tgeompoints(TemporalS *ts);

/* Trajectory functions */

extern Datum tpoint_trajectory(PG_FUNCTION_ARGS);

extern Datum tpoint_trajectory_internal(Temporal *temp);
extern Datum tpointseq_make_trajectory(TemporalInst **instants, int count, bool linear);
extern Datum tpointseq_trajectory_append(TemporalSeq *seq, TemporalInst *inst, bool replace);

extern Datum geompoint_trajectory(Datum value1, Datum value2);
extern Datum geogpoint_trajectory(Datum value1, Datum value2);

extern Datum tpointseq_trajectory(TemporalSeq *seq);
extern Datum tpointseq_trajectory_copy(TemporalSeq *seq);
extern Datum tpoints_trajectory(TemporalS *ts);

/* Length, speed, time-weighted centroid, and temporal azimuth functions */

extern Datum tpoint_length(PG_FUNCTION_ARGS);
extern Datum tpoint_cumulative_length(PG_FUNCTION_ARGS);
extern Datum tpoint_speed(PG_FUNCTION_ARGS);
extern Datum tgeompoint_twcentroid(PG_FUNCTION_ARGS);
extern Datum tpoint_azimuth(PG_FUNCTION_ARGS);

extern Datum tgeompointi_twcentroid(TemporalI *ti);
extern Datum tgeompointseq_twcentroid(TemporalSeq *seq);
extern Datum tgeompoints_twcentroid(TemporalS *ts);

/* Restriction functions */

extern Datum tpoint_at_geometry(PG_FUNCTION_ARGS);
extern Datum tpoint_minus_geometry(PG_FUNCTION_ARGS);

extern TemporalSeq **tpointseq_at_geometry2(TemporalSeq *seq, Datum geo, int *count);

/* Nearest approach functions */

extern Datum NAI_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAI_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum NAD_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum NAD_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum shortestline_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum shortestline_tpoint_tpoint(PG_FUNCTION_ARGS);

/* Functions converting a temporal point to/from a PostGIS trajectory */

extern Datum tpoint_to_geo(PG_FUNCTION_ARGS);
extern Datum geo_to_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
