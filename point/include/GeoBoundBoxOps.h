/*****************************************************************************
 *
 * GeoBoundBoxOps.h
 *	  Bounding box operators for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __GEOBONDBOXOPS_H__
#define __GEOBONDBOXOPS_H__

/*****************************************************************************/

/* STBOX functions */

extern Datum contains_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum contained_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum overlaps_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum same_stbox_stbox(PG_FUNCTION_ARGS);
extern Datum distance_stbox_stbox(PG_FUNCTION_ARGS);

extern bool contains_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool contained_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool overlaps_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern bool same_stbox_stbox_internal(const STBOX *box1, const STBOX *box2);
extern double distance_stbox_stbox_internal(STBOX *box1, STBOX *box2);

/* Functions computing the bounding box at the creation of the temporal point */

extern void tpointinst_make_stbox(STBOX *box, Datum value, TimestampTz t);
extern void tpointinstarr_to_stbox(STBOX *box, TemporalInst **inst, int count);
extern void tpointseqarr_to_stbox(STBOX *box, TemporalSeq **seq, int count);

extern void tpoint_expand_stbox(STBOX *box, Temporal *temp, TemporalInst *inst);

/* Functions for expanding the bounding box */

extern Datum stbox_expand_spatial(PG_FUNCTION_ARGS);
extern Datum tpoint_expand_spatial(PG_FUNCTION_ARGS);
extern Datum stbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum tpoint_expand_temporal(PG_FUNCTION_ARGS);

/* Transform a <Type> to a STBOX */

extern Datum geo_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_period_to_stbox(PG_FUNCTION_ARGS);

extern bool geo_to_stbox_internal(STBOX *box, GSERIALIZED *gs);
extern void timestamp_to_stbox_internal(STBOX *box, TimestampTz t);
extern void timestampset_to_stbox_internal(STBOX *box, TimestampSet *ps);
extern void period_to_stbox_internal(STBOX *box, Period *p);
extern void periodset_to_stbox_internal(STBOX *box, PeriodSet *ps);
extern bool geo_timestamp_to_stbox_internal(STBOX *box, GSERIALIZED* geom, TimestampTz t);
extern bool geo_period_to_stbox_internal(STBOX *box, GSERIALIZED* geom, Period *p);

/*****************************************************************************/

extern Datum overlaps_bbox_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum contains_bbox_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum contains_bbox_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum contained_bbox_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum contained_bbox_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

extern Datum same_bbox_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum same_bbox_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum same_bbox_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum same_bbox_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum same_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
