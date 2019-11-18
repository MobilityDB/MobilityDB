/*****************************************************************************
 *
 * temporalseq.h
 *	  Basic functions for temporal sequences.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALSEQ_H__
#define __TEMPORALSEQ_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
#include "temporal.h"

/*****************************************************************************/

extern TemporalInst *temporalseq_inst_n(TemporalSeq *seq, int index);
extern TemporalSeq *temporalseq_from_temporalinstarr(TemporalInst **instants, 
	int count, bool lower_inc, bool upper_inc, bool normalize);
extern TemporalSeq *temporalseq_copy(TemporalSeq *seq);
extern int temporalseq_find_timestamp(TemporalSeq *seq, TimestampTz t);
extern Datum temporalseq_value_at_timestamp1(TemporalInst *inst1, 
	TemporalInst *inst2, TimestampTz t);
extern TemporalSeq **temporalseqarr_normalize(TemporalSeq **sequences, int count, 
	int *newcount);

/* Intersection functions */

extern bool intersection_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	TemporalInst **inter1, TemporalInst **inter2);	
extern bool intersection_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	TemporalSeq **inter1, TemporalSeq **inter2);

/* Synchronize functions */

extern bool temporalseq_add_crossing(TemporalInst *inst1, TemporalInst *next1, 
	TemporalInst *inst2, TemporalInst *next2, 
	TemporalInst **cross1, TemporalInst **cross2);
extern bool synchronize_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	TemporalSeq **sync1, TemporalSeq **sync2, bool interpoint);

extern bool tnumberseq_mult_maxmin_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t);
// To put it in TempDistance.c ?
extern bool tpointseq_min_dist_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t);
extern bool tpointseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t);
extern bool temporalseq_intersect_at_timestamp(TemporalInst *start1, 
	TemporalInst *end1, TemporalInst *start2, TemporalInst *end2, TimestampTz *inter);

/* Input/output functions */

extern char *temporalseq_to_string(TemporalSeq *seq, char *(*value_out)(Oid, Datum));
extern void temporalseq_write(TemporalSeq *seq, StringInfo buf);
extern TemporalSeq *temporalseq_read(StringInfo buf, Oid valuetypid);

/* Append function */

extern TemporalSeq *temporalseq_append_instant(TemporalSeq *seq, TemporalInst *inst);

/* Cast functions */

extern int tintseq_as_tfloatseq1(TemporalSeq **result, TemporalSeq *seq);
extern TemporalS *tintseq_as_tfloatseq(TemporalSeq *seq);
extern TemporalSeq *tfloatseq_as_tintseq(TemporalSeq *seq);

/* Transformation functions */

extern TemporalSeq *temporalinst_as_temporalseq(TemporalInst *inst);
extern TemporalSeq *temporali_as_temporalseq(TemporalI *ti);
extern TemporalSeq *temporals_as_temporalseq(TemporalS *ts);

/* Accessor functions */

extern Datum *tempdiscseq_values1(TemporalSeq *seq);
extern ArrayType *tempdiscseq_values(TemporalSeq *seq);
extern PeriodSet *temporalseq_get_time(TemporalSeq *seq);
extern void *temporalseq_bbox_ptr(TemporalSeq *seq);
extern void temporalseq_bbox(void *box, TemporalSeq *seq);
extern RangeType *tnumberseq_value_range(TemporalSeq *seq);
extern RangeType *tfloatseq_range(TemporalSeq *seq);
extern ArrayType *tfloatseq_ranges(TemporalSeq *seq);
extern Datum temporalseq_min_value(TemporalSeq *seq);
extern Datum temporalseq_max_value(TemporalSeq *seq);
extern void temporalseq_timespan(Period *p, TemporalSeq *seq);
extern Datum temporalseq_duration(TemporalSeq *seq);
extern TemporalInst **temporalseq_instants(TemporalSeq *seq);
extern ArrayType *temporalseq_instants_array(TemporalSeq *seq);
extern TimestampTz temporalseq_start_timestamp(TemporalSeq *seq);
extern TimestampTz temporalseq_end_timestamp(TemporalSeq *seq);
extern TimestampTz *temporalseq_timestamps1(TemporalSeq *seq);
extern ArrayType *temporalseq_timestamps(TemporalSeq *seq);
extern bool temporalseq_ever_eq(TemporalSeq *seq, Datum value);
extern bool temporalseq_always_eq(TemporalSeq *seq, Datum value);
extern TemporalSeq *temporalseq_shift(TemporalSeq *seq, 
	Interval *interval);

/* Restriction Functions */

extern bool tempcontseq_timestamp_at_value(TemporalInst *inst1, TemporalInst *inst2, 
	Datum value, Oid valuetypid, TimestampTz *t);
extern int temporalseq_at_value2(TemporalSeq **result, TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_at_value(TemporalSeq *seq, Datum value);
extern int temporalseq_minus_value2(TemporalSeq **result, TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_minus_value(TemporalSeq *seq, Datum value);
extern int temporalseq_at_values1(TemporalSeq **result, TemporalSeq *seq, Datum *values, 
	int count);	
extern TemporalS *temporalseq_at_values(TemporalSeq *seq, Datum *values, int count);
extern int temporalseq_minus_values1(TemporalSeq **result, TemporalSeq *seq, Datum *values, 
	int count);
extern TemporalS *temporalseq_minus_values(TemporalSeq *seq, Datum *values, int count);
extern TemporalS *temporalseq_minus_values(TemporalSeq *seq, Datum *values, int count);
extern int tnumberseq_at_range2(TemporalSeq **result, TemporalSeq *seq, RangeType *range);
extern TemporalS *tnumberseq_at_range(TemporalSeq *seq, RangeType *range);
extern int tnumberseq_minus_range1(TemporalSeq **result, TemporalSeq *seq, RangeType *range);
extern TemporalS *tnumberseq_minus_range(TemporalSeq *seq, RangeType *range);
extern int tnumberseq_at_ranges1(TemporalSeq **result, TemporalSeq *seq, 
	RangeType **normranges, int count);
extern TemporalS *tnumberseq_at_ranges(TemporalSeq *seq, 
	RangeType **normranges, int count);
extern int tnumberseq_minus_ranges1(TemporalSeq **result, TemporalSeq *seq, 
	RangeType **normranges, int count);
extern TemporalS *tnumberseq_minus_ranges(TemporalSeq *seq,
	RangeType **normranges, int count);
extern int temporalseq_at_minmax(TemporalSeq **result, TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_at_min(TemporalSeq *seq);
extern TemporalS *temporalseq_minus_min(TemporalSeq *seq);
extern TemporalS *temporalseq_at_max(TemporalSeq *seq);
extern TemporalS *temporalseq_minus_max(TemporalSeq *seq);
extern TemporalInst *temporalseq_at_timestamp1(TemporalInst *inst1, 
	TemporalInst *inst2, TimestampTz t);
extern TemporalInst *temporalseq_at_timestamp(TemporalSeq *seq, TimestampTz t);
extern bool temporalseq_value_at_timestamp(TemporalSeq *seq, TimestampTz t, Datum *result);
extern int temporalseq_minus_timestamp1(TemporalSeq **result, TemporalSeq *seq, 
	TimestampTz t);
extern TemporalS *temporalseq_minus_timestamp(TemporalSeq *seq, TimestampTz t);
extern TemporalI *temporalseq_at_timestampset(TemporalSeq *seq, TimestampSet *ts);
extern int temporalseq_minus_timestampset1(TemporalSeq **result, TemporalSeq *seq, 
	TimestampSet *ts);
extern TemporalS *temporalseq_minus_timestampset(TemporalSeq *seq, TimestampSet *ts);
extern TemporalSeq *temporalseq_at_period(TemporalSeq *seq, Period *p);
extern TemporalS *temporalseq_minus_period(TemporalSeq *seq, Period *p);
extern int temporalseq_at_periodset1(TemporalSeq **result, TemporalSeq *seq, PeriodSet *ps);
extern TemporalSeq **temporalseq_at_periodset2(TemporalSeq *seq, PeriodSet *ps, int *count);
extern TemporalS *temporalseq_at_periodset(TemporalSeq *seq, PeriodSet *ps);
extern int temporalseq_minus_periodset1(TemporalSeq **result, TemporalSeq *seq, PeriodSet *ps, 
	int from, int count);
extern TemporalS *temporalseq_minus_periodset(TemporalSeq *seq, PeriodSet *ps);
extern bool temporalseq_intersects_timestamp(TemporalSeq *seq, TimestampTz t);
extern bool temporalseq_intersects_timestampset(TemporalSeq *seq, TimestampSet *t);
extern bool temporalseq_intersects_period(TemporalSeq *seq, Period *p);
extern bool temporalseq_intersects_periodset(TemporalSeq *seq, PeriodSet *ps);

/* Local aggregate functions */

extern double tintseq_integral(TemporalSeq *seq);
extern double tfloatseq_integral(TemporalSeq *seq);
extern double tintseq_twavg(TemporalSeq *seq);
extern double tfloatseq_twavg(TemporalSeq *seq);

/* Comparison functions */

extern int temporalseq_cmp(TemporalSeq *seq1, TemporalSeq *seq2);
extern bool temporalseq_eq(TemporalSeq *seq1, TemporalSeq *seq2);

/* Function for defining hash index */

extern uint32 temporalseq_hash(TemporalSeq *seq);

/*****************************************************************************/

#endif
