/*****************************************************************************
 *
 * temporalseq.h
 *	  Basic functions for temporal sequences.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
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

extern TemporalInst *temporalseq_inst_n(const TemporalSeq *seq, int index);
extern TemporalSeq *temporalseq_make(TemporalInst **instants, 
	int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TemporalSeq *temporalseq_copy(TemporalSeq *seq);
extern int temporalseq_find_timestamp(TemporalSeq *seq, TimestampTz t);
extern Datum temporalseq_value_at_timestamp1(TemporalInst *inst1, 
	TemporalInst *inst2, bool linear, TimestampTz t);
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

extern bool synchronize_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	TemporalSeq **sync1, TemporalSeq **sync2, bool interpoint);

extern bool tpointseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	bool linear1, TemporalInst *start2, TemporalInst *end2, bool linear2, TimestampTz *t);
extern bool temporalseq_intersect_at_timestamp(TemporalInst *start1, TemporalInst *end1, 
	bool linear1, TemporalInst *start2, TemporalInst *end2, bool linear2, TimestampTz *inter);

/* Input/output functions */

extern char *temporalseq_to_string(TemporalSeq *seq, bool component, char *(*value_out)(Oid, Datum));
extern void temporalseq_write(TemporalSeq *seq, StringInfo buf);
extern TemporalSeq *temporalseq_read(StringInfo buf, Oid valuetypid);

/* Append function */

extern TemporalSeq *temporalseq_append_instant(TemporalSeq *seq, TemporalInst *inst);

/* Cast functions */

extern TemporalSeq *tintseq_to_tfloatseq(TemporalSeq *seq);
extern TemporalSeq *tfloatseq_to_tintseq(TemporalSeq *seq);

/* Transformation functions */

extern TemporalSeq *temporalinst_to_temporalseq(TemporalInst *inst, bool linear);
extern TemporalSeq *temporali_to_temporalseq(TemporalI *ti, bool linear);
extern TemporalSeq *temporals_to_temporalseq(TemporalS *ts);
extern int tstepwseq_to_linear1(TemporalSeq **result, TemporalSeq *seq);
extern TemporalS *tstepwseq_to_linear(TemporalSeq *seq);

/* Accessor functions */

extern Datum *temporalseq_values1(TemporalSeq *seq, int *count);
extern ArrayType *temporalseq_values(TemporalSeq *seq);
extern int tfloatseq_ranges1(RangeType **result, TemporalSeq *seq);
extern PeriodSet *temporalseq_get_time(TemporalSeq *seq);
extern void *temporalseq_bbox_ptr(TemporalSeq *seq);
extern void temporalseq_bbox(void *box, TemporalSeq *seq);
extern RangeType *tfloatseq_range(TemporalSeq *seq);
extern ArrayType *tfloatseq_ranges(TemporalSeq *seq);
extern Datum temporalseq_min_value(TemporalSeq *seq);
extern Datum temporalseq_max_value(TemporalSeq *seq);
extern void temporalseq_period(Period *p, TemporalSeq *seq);
extern Datum temporalseq_timespan(TemporalSeq *seq);
extern TemporalInst **temporalseq_instants(TemporalSeq *seq);
extern ArrayType *temporalseq_instants_array(TemporalSeq *seq);
extern TimestampTz temporalseq_start_timestamp(TemporalSeq *seq);
extern TimestampTz temporalseq_end_timestamp(TemporalSeq *seq);
extern TimestampTz *temporalseq_timestamps1(TemporalSeq *seq);
extern ArrayType *temporalseq_timestamps(TemporalSeq *seq);
extern TemporalSeq *temporalseq_shift(TemporalSeq *seq, 
	Interval *interval);

extern bool temporalseq_ever_eq(TemporalSeq *seq, Datum value);
extern bool temporalseq_ever_lt(TemporalSeq *seq, Datum value);
extern bool temporalseq_ever_le(TemporalSeq *seq, Datum value);

extern bool temporalseq_always_eq(TemporalSeq *seq, Datum value);
extern bool temporalseq_always_lt(TemporalSeq *seq, Datum value);
extern bool temporalseq_always_le(TemporalSeq *seq, Datum value);

/* Restriction Functions */

extern bool tlinearseq_timestamp_at_value(TemporalInst *inst1, TemporalInst *inst2, 
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
	TemporalInst *inst2, TimestampTz t, bool linear);
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

extern double tnumberseq_integral(TemporalSeq *seq);
extern double tnumberseq_twavg(TemporalSeq *seq);

/* Comparison functions */

extern int temporalseq_cmp(TemporalSeq *seq1, TemporalSeq *seq2);
extern bool temporalseq_eq(TemporalSeq *seq1, TemporalSeq *seq2);

/* Function for defining hash index */

extern uint32 temporalseq_hash(TemporalSeq *seq);

/*****************************************************************************/

#endif
