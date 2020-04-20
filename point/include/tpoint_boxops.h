/*****************************************************************************
 *
 * tpoint_boxops.h
 *	  Bounding box operators for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_BOXOPS_H__
#define __TPOINT_BOXOPS_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"
#include "temporal_util.h"
#include <liblwgeom.h>

/*****************************************************************************/

/* Transform a <Type> to a STBOX */

extern Datum geo_to_stbox(PG_FUNCTION_ARGS);
extern Datum timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum timestampset_to_stbox(PG_FUNCTION_ARGS);
extern Datum period_to_stbox(PG_FUNCTION_ARGS);
extern Datum periodset_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_timestamp_to_stbox(PG_FUNCTION_ARGS);
extern Datum geo_period_to_stbox(PG_FUNCTION_ARGS);

extern bool geo_to_stbox_internal(STBOX *box, const GSERIALIZED *gs);
extern void timestamp_to_stbox_internal(STBOX *box, TimestampTz t);
extern void timestampset_to_stbox_internal(STBOX *box, const TimestampSet *ps);
extern void period_to_stbox_internal(STBOX *box, const Period *p);
extern void periodset_to_stbox_internal(STBOX *box, const PeriodSet *ps);
extern bool geo_timestamp_to_stbox_internal(STBOX *box, const GSERIALIZED* geom, TimestampTz t);
extern bool geo_period_to_stbox_internal(STBOX *box, const GSERIALIZED* geom, const Period *p);

/* Functions computing the bounding box at the creation of the temporal point */

extern void tpointinst_make_stbox(STBOX *box, const TemporalInst *inst);
extern void tpointinstarr_to_stbox(STBOX *box, TemporalInst **inst, int count);
extern void tpointseqarr_to_stbox(STBOX *box, TemporalSeq **seq, int count);

/* Boxes functions */

extern Datum tpoint_stboxes(PG_FUNCTION_ARGS);

extern ArrayType *tpointseq_stboxes(const TemporalSeq *seq);
extern ArrayType *tpoints_stboxes(const TemporalS *ts);

/* Functions for expanding the bounding box */

extern Datum stbox_expand_spatial(PG_FUNCTION_ARGS);
extern Datum tpoint_expand_spatial(PG_FUNCTION_ARGS);
extern Datum stbox_expand_temporal(PG_FUNCTION_ARGS);
extern Datum tpoint_expand_temporal(PG_FUNCTION_ARGS);


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

extern Datum adjacent_bbox_geo_tpoint(PG_FUNCTION_ARGS);
extern Datum adjacent_bbox_stbox_tpoint(PG_FUNCTION_ARGS);
extern Datum adjacent_bbox_tpoint_geo(PG_FUNCTION_ARGS);
extern Datum adjacent_bbox_tpoint_stbox(PG_FUNCTION_ARGS);
extern Datum adjacent_bbox_tpoint_tpoint(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
