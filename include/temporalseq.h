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
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "temporal.h"

/*****************************************************************************/

extern TemporalInst *temporalseq_inst_n(const TemporalSeq *seq, int index);
extern TemporalSeq *temporalseq_make(TemporalInst **instants, 
	int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TemporalSeq *temporalseq_make_free(TemporalInst **instants, 
	int count, bool lower_inc, bool upper_inc, bool linear, bool normalize);
extern TemporalSeq *temporalseq_copy(const TemporalSeq *seq);
extern int temporalseq_find_timestamp(const TemporalSeq *seq, TimestampTz t);
extern Datum temporalseq_value_at_timestamp1(const TemporalInst *inst1,
	const TemporalInst *inst2, bool linear, TimestampTz t);
extern TemporalSeq **temporalseqarr_normalize(TemporalSeq **sequences, int count, 
	int *newcount);
extern TemporalSeq **temporalseqarr2_to_temporalseqarr(TemporalSeq ***sequences, 
	int *countseqs, int count, int totalseqs);

/* Intersection functions */

extern bool intersection_temporalseq_temporalinst(const TemporalSeq *seq, const TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporalseq(const TemporalInst *inst, const TemporalSeq *seq,
	TemporalInst **inter1, TemporalInst **inter2);	
extern bool intersection_temporalseq_temporali(const TemporalSeq *seq, const TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporali_temporalseq(const TemporalI *ti, const TemporalSeq *seq,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	TemporalSeq **inter1, TemporalSeq **inter2);

/* Synchronize functions */

extern bool synchronize_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	TemporalSeq **sync1, TemporalSeq **sync2, bool interpoint);

extern bool tlinearseq_intersection_value(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum value, Oid valuetypid, Datum *inter, TimestampTz *t);

extern bool tgeompointseq_intersection(const TemporalInst *start1, const TemporalInst *end1,
	const TemporalInst *start2, const TemporalInst *end2, TimestampTz *t);

extern bool temporalseq_intersection(const TemporalInst *start1, const TemporalInst *end1, bool linear1,
	const TemporalInst *start2, const TemporalInst *end2, bool linear2,
	Datum *inter1, Datum *inter2, TimestampTz *t);

/* Input/output functions */

extern char *temporalseq_to_string(const TemporalSeq *seq, bool component, char *(*value_out)(Oid, Datum));
extern void temporalseq_write(const TemporalSeq *seq, StringInfo buf);
extern TemporalSeq *temporalseq_read(StringInfo buf, Oid valuetypid);

/* Constructor functions */

extern TemporalSeq *temporalseq_from_base_internal(Datum value, Oid valuetypid, const Period *p, bool linear);

extern Datum temporalseq_from_base(PG_FUNCTION_ARGS);

/* Append and merge functions */

extern TemporalSeq *temporalseq_join(const TemporalSeq *seq1, const TemporalSeq *seq2, bool last, bool first);
extern TemporalSeq *temporalseq_append_instant(const TemporalSeq *seq, const TemporalInst *inst);
extern Temporal *temporalseq_merge(const TemporalSeq *seq1, const TemporalSeq *seq2);
extern Temporal *temporalseq_merge_array(TemporalSeq **sequences, int count);

/* Cast functions */

extern TemporalSeq *tintseq_to_tfloatseq(const TemporalSeq *seq);
extern TemporalSeq *tfloatseq_to_tintseq(const TemporalSeq *seq);

/* Transformation functions */

extern TemporalSeq *temporalinst_to_temporalseq(const TemporalInst *inst, bool linear);
extern TemporalSeq *temporali_to_temporalseq(const TemporalI *ti, bool linear);
extern TemporalSeq *temporals_to_temporalseq(const TemporalS *ts);
extern int tstepseq_to_linear1(TemporalSeq **result, const TemporalSeq *seq);
extern TemporalS *tstepseq_to_linear(const TemporalSeq *seq);

/* Accessor functions */

extern Datum *temporalseq_values1(const TemporalSeq *seq, int *count);
extern ArrayType *temporalseq_values(const TemporalSeq *seq);
extern int tfloatseq_ranges1(RangeType **result, const TemporalSeq *seq);
extern PeriodSet *temporalseq_get_time(const TemporalSeq *seq);
extern void *temporalseq_bbox_ptr(const TemporalSeq *seq);
extern void temporalseq_bbox(void *box, const TemporalSeq *seq);
extern RangeType *tfloatseq_range(const TemporalSeq *seq);
extern ArrayType *tfloatseq_ranges(const TemporalSeq *seq);
extern TemporalInst *temporalseq_min_instant(const TemporalSeq *seq);
extern Datum temporalseq_min_value(const TemporalSeq *seq);
extern Datum temporalseq_max_value(const TemporalSeq *seq);
extern void temporalseq_period(Period *p, const TemporalSeq *seq);
extern Datum temporalseq_timespan(const TemporalSeq *seq);
extern TemporalInst **temporalseq_instants(const TemporalSeq *seq);
extern ArrayType *temporalseq_instants_array(const TemporalSeq *seq);
extern TimestampTz temporalseq_start_timestamp(const TemporalSeq *seq);
extern TimestampTz temporalseq_end_timestamp(const TemporalSeq *seq);
extern TimestampTz *temporalseq_timestamps1(const TemporalSeq *seq);
extern ArrayType *temporalseq_timestamps(const TemporalSeq *seq);
extern TemporalSeq *temporalseq_shift(const TemporalSeq *seq,
	const Interval *interval);

extern bool temporalseq_ever_eq(const TemporalSeq *seq, Datum value);
extern bool temporalseq_ever_lt(const TemporalSeq *seq, Datum value);
extern bool temporalseq_ever_le(const TemporalSeq *seq, Datum value);

extern bool temporalseq_always_eq(const TemporalSeq *seq, Datum value);
extern bool temporalseq_always_lt(const TemporalSeq *seq, Datum value);
extern bool temporalseq_always_le(const TemporalSeq *seq, Datum value);

/* Restriction Functions */

extern int temporalseq_at_value2(TemporalSeq **result, const TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_at_value(const TemporalSeq *seq, Datum value);
extern int temporalseq_minus_value2(TemporalSeq **result, const TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_minus_value(const TemporalSeq *seq, Datum value);
extern int temporalseq_at_values1(TemporalSeq **result, const TemporalSeq *seq, const Datum *values,
	int count);	
extern TemporalS *temporalseq_at_values(const TemporalSeq *seq, const Datum *values, int count);
extern int temporalseq_minus_values1(TemporalSeq **result, const TemporalSeq *seq, const Datum *values,
	int count);
extern TemporalS *temporalseq_minus_values(const TemporalSeq *seq, const Datum *values, int count);
extern int tnumberseq_at_range2(TemporalSeq **result, const TemporalSeq *seq, RangeType *range);
extern TemporalS *tnumberseq_at_range(const TemporalSeq *seq, RangeType *range);
extern int tnumberseq_minus_range1(TemporalSeq **result, const TemporalSeq *seq, RangeType *range);
extern TemporalS *tnumberseq_minus_range(const TemporalSeq *seq, RangeType *range);
extern int tnumberseq_at_ranges1(TemporalSeq **result, const TemporalSeq *seq,
	RangeType **normranges, int count);
extern TemporalS *tnumberseq_at_ranges(const TemporalSeq *seq,
	RangeType **normranges, int count);
extern int tnumberseq_minus_ranges1(TemporalSeq **result, const TemporalSeq *seq,
	RangeType **normranges, int count);
extern TemporalS *tnumberseq_minus_ranges(const TemporalSeq *seq,
	RangeType **normranges, int count);
extern int temporalseq_at_minmax(TemporalSeq **result, const TemporalSeq *seq, Datum value);
extern TemporalS *temporalseq_at_min(const TemporalSeq *seq);
extern TemporalS *temporalseq_minus_min(const TemporalSeq *seq);
extern TemporalS *temporalseq_at_max(const TemporalSeq *seq);
extern TemporalS *temporalseq_minus_max(const TemporalSeq *seq);
extern TemporalInst *temporalseq_at_timestamp1(const TemporalInst *inst1,
	const TemporalInst *inst2, bool linear, TimestampTz t);
extern TemporalInst *temporalseq_at_timestamp(const TemporalSeq *seq, TimestampTz t);
extern bool temporalseq_value_at_timestamp(const TemporalSeq *seq, TimestampTz t, Datum *result);
extern int temporalseq_minus_timestamp1(TemporalSeq **result, const TemporalSeq *seq,
	TimestampTz t);
extern TemporalS *temporalseq_minus_timestamp(const TemporalSeq *seq, TimestampTz t);
extern TemporalI *temporalseq_at_timestampset(const TemporalSeq *seq, const TimestampSet *ts);
extern int temporalseq_minus_timestampset1(TemporalSeq **result, const TemporalSeq *seq,
	const TimestampSet *ts);
extern TemporalS *temporalseq_minus_timestampset(const TemporalSeq *seq, const TimestampSet *ts);
extern TemporalSeq *temporalseq_at_period(const TemporalSeq *seq, const Period *p);
extern TemporalS *temporalseq_minus_period(const TemporalSeq *seq, const Period *p);
extern int temporalseq_at_periodset1(TemporalSeq **result, const TemporalSeq *seq, const PeriodSet *ps);
extern TemporalSeq **temporalseq_at_periodset2(const TemporalSeq *seq, const PeriodSet *ps, int *count);
extern TemporalS *temporalseq_at_periodset(const TemporalSeq *seq, const PeriodSet *ps);
extern int temporalseq_minus_periodset1(TemporalSeq **result, const TemporalSeq *seq,
	const PeriodSet *ps, int from);
extern TemporalS *temporalseq_minus_periodset(const TemporalSeq *seq, const PeriodSet *ps);
extern bool temporalseq_intersects_timestamp(const TemporalSeq *seq, TimestampTz t);
extern bool temporalseq_intersects_timestampset(const TemporalSeq *seq, const TimestampSet *t);
extern bool temporalseq_intersects_period(const TemporalSeq *seq, const Period *p);
extern bool temporalseq_intersects_periodset(const TemporalSeq *seq, const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumberseq_integral(const TemporalSeq *seq);
extern double tnumberseq_twavg(const TemporalSeq *seq);

/* Comparison functions */

extern int temporalseq_cmp(const TemporalSeq *seq1, const TemporalSeq *seq2);
extern bool temporalseq_eq(const TemporalSeq *seq1, const TemporalSeq *seq2);

/* Function for defining hash index */

extern uint32 temporalseq_hash(const TemporalSeq *seq);

/*****************************************************************************/

#endif
