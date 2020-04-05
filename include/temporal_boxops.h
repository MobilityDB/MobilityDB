/*****************************************************************************
 *
 * temporal_boxops.h
 *	  Bounding box operators for temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_BOXOPS_H__
#define __TEMPORAL_BOXOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
#include "temporal.h"

/*****************************************************************************/

/* Functions on generic bounding boxes of temporal types */

extern size_t temporal_bbox_size(Oid valuetypid);
extern bool temporal_bbox_eq(const void *box1, const void *box2, Oid valuetypid);
extern int temporal_bbox_cmp(const void *box1, const void *box2, Oid valuetypid);
extern void temporal_bbox_expand(void *box1, const void *box2, Oid valuetypid);
extern void temporal_bbox_shift(void *box, const Interval *interval, Oid valuetypid);

/* Compute the bounding box at the creation of temporal values */

extern void temporalinst_make_bbox(void *bbox, const TemporalInst *inst);
extern void temporali_make_bbox(void *bbox, TemporalInst **inst, int count);
extern void temporalseq_make_bbox(void *bbox, TemporalInst** inst, int count,
	bool lower_inc, bool upper_inc);
extern void temporals_make_bbox(void *bbox, TemporalSeq **seqs, int count);

/* Transform a type to a TBOX */

extern Datum int_to_tbox(PG_FUNCTION_ARGS);
extern Datum float_to_tbox(PG_FUNCTION_ARGS);
extern Datum numeric_to_tbox(PG_FUNCTION_ARGS);
extern Datum intrange_to_tbox(PG_FUNCTION_ARGS);
extern Datum floatrange_to_tbox(PG_FUNCTION_ARGS);
extern Datum timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum period_to_tbox(PG_FUNCTION_ARGS);
extern Datum timestampset_to_tbox(PG_FUNCTION_ARGS);
extern Datum periodset_to_tbox(PG_FUNCTION_ARGS);
extern Datum int_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum float_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum int_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum float_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum intrange_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum floatrange_timestamp_to_tbox(PG_FUNCTION_ARGS);
extern Datum intrange_period_to_tbox(PG_FUNCTION_ARGS);
extern Datum floatrange_period_to_tbox(PG_FUNCTION_ARGS);

extern void number_to_box(TBOX *box, Datum value, Oid valuetypid);
extern void range_to_tbox_internal(TBOX *box, RangeType *r);
extern void int_to_tbox_internal(TBOX *box, int i);
extern void float_to_tbox_internal(TBOX *box, double d);
extern void intrange_to_tbox_internal(TBOX *box, RangeType *range);
extern void floatrange_to_tbox_internal(TBOX *box, RangeType *range);
extern void timestamp_to_tbox_internal(TBOX *box, TimestampTz t);
extern void timestampset_to_tbox_internal(TBOX *box, const TimestampSet *ts);
extern void period_to_tbox_internal(TBOX *box, const Period *p);
extern void periodset_to_tbox_internal(TBOX *box, const PeriodSet *ps);

/* Bounding box operators for temporal types */

extern Datum contains_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum contains_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum contained_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum contained_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum overlaps_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum same_bbox_period_temporal(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_period(PG_FUNCTION_ARGS);
extern Datum same_bbox_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum overlaps_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum overlaps_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum contains_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum contains_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum contained_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum contained_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);

extern Datum same_bbox_range_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_range(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum same_bbox_tnumber_tnumber(PG_FUNCTION_ARGS);
 
/*****************************************************************************/

#endif
