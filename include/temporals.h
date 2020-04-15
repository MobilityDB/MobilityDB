/*****************************************************************************
 *
 * temporals.h
 *	  Basic functions for temporal sequence sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALS_H__
#define __TEMPORALS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
#include "temporal.h"

/*****************************************************************************/

/* General functions */

extern TemporalSeq *temporals_seq_n(const TemporalS *ts, int index);
extern TemporalS *temporals_make(TemporalSeq **sequences, int count,
	bool normalize);
extern TemporalS *temporals_copy(const TemporalS *ts);
extern bool temporals_find_timestamp(const TemporalS *ts, TimestampTz t, int *pos);
extern double temporals_interval_double(const TemporalS *ts);

/* Intersection functions */

extern bool intersection_temporals_temporalinst(const TemporalS *ts, const TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporals(const TemporalInst *inst, const TemporalS *ts,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporals_temporali(const TemporalS *ts, const TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporali_temporals(const TemporalI *ti, const TemporalS *ts,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	TemporalS **inter1, TemporalS **inter2);
extern bool intersection_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	TemporalS **inter1, TemporalS **inter2);
extern bool intersection_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	TemporalS **inter1, TemporalS **inter2);

/* Synchronize functions */

extern bool synchronize_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	TemporalS **sync1, TemporalS **sync2, bool interpoint);
extern bool synchronize_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	TemporalS **sync1, TemporalS **sync2, bool interpoint);
extern bool synchronize_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	TemporalS **sync1, TemporalS **sync2, bool interpoint);

/* Input/output functions */

extern char *temporals_to_string(const TemporalS *ts, char *(*value_out)(Oid, Datum));
extern void temporals_write(const TemporalS *ts, StringInfo buf);
extern TemporalS *temporals_read(StringInfo buf, Oid valuetypid);

/* Constructor functions */

extern TemporalS *temporals_from_base_internal(Datum value, Oid valuetypid, const PeriodSet *ps, bool linear);

extern Datum temporals_from_base(PG_FUNCTION_ARGS);

/* Append and merge functions */

extern TemporalS *temporals_append_instant(const TemporalS *ts, const TemporalInst *inst);
extern TemporalS *temporals_merge(const TemporalS *ts1, const TemporalS *ts2);
extern TemporalS *temporals_merge_array(TemporalS **ts, int count);

/* Cast functions */

extern TemporalS *tints_to_tfloats(const TemporalS *ts);
extern TemporalS *tfloats_to_tints(const TemporalS *ts);

/* Transformation functions */

extern TemporalS *temporalinst_to_temporals(const TemporalInst *inst, bool linear);
extern TemporalS *temporali_to_temporals(const TemporalI *ti, bool linear);
extern TemporalS *temporalseq_to_temporals(const TemporalSeq *seq);
extern TemporalS *tsteps_to_linear(const TemporalS *ts);

/* Accessor functions */

extern Datum *temporals_values1(const TemporalS *ts, int *count);
extern ArrayType *temporals_values(const TemporalS *ts);
extern ArrayType *tfloats_ranges(const TemporalS *ts);
extern void *temporals_bbox_ptr(const TemporalS *ts);
extern void temporals_bbox(void *box, const TemporalS *ts);
extern TemporalInst *temporals_min_instant(const TemporalS *ts);
extern Datum temporals_min_value(const TemporalS *ts);
extern Datum temporals_max_value(const TemporalS *ts);
extern PeriodSet *temporals_get_time(const TemporalS *ts);
extern Datum temporals_timespan(const TemporalS *ts);
extern void temporals_period(Period *p, const TemporalS *ts);
extern TemporalSeq **temporals_sequences(const TemporalS *ts);
extern ArrayType *temporals_sequences_array(const TemporalS *ts);
extern int temporals_num_instants(const TemporalS *ts);
extern TemporalInst *temporals_instant_n(const TemporalS *ts, int n);
extern ArrayType *temporals_instants_array(const TemporalS *ts);
extern TimestampTz temporals_start_timestamp(const TemporalS *ts);
extern TimestampTz temporals_end_timestamp(const TemporalS *ts);
extern int temporals_num_timestamps(const TemporalS *ts);
extern bool temporals_timestamp_n(const TemporalS *ts, int n, TimestampTz *result);
extern TimestampTz *temporals_timestamps1(const TemporalS *ts, int *count);
extern ArrayType *temporals_timestamps(const TemporalS *ts);
extern TemporalS *temporals_shift(const TemporalS *ts, const Interval *interval);

extern bool temporals_ever_eq(const TemporalS *ts, Datum value);
extern bool temporals_ever_lt(const TemporalS *ts, Datum value);
extern bool temporals_ever_le(const TemporalS *ts, Datum value);

extern bool temporals_always_eq(const TemporalS *ts, Datum value);
extern bool temporals_always_lt(const TemporalS *ts, Datum value);
extern bool temporals_always_le(const TemporalS *ts, Datum value);

/* Restriction Functions */

extern TemporalS *temporals_at_value(const TemporalS *ts, Datum value);
extern TemporalS *temporals_minus_value(const TemporalS *ts, Datum value);
extern TemporalS *temporals_at_values(const TemporalS *ts,const  Datum *values, int count);
extern TemporalS *temporals_minus_values(const TemporalS *ts,const  Datum *values, int count);
extern TemporalS *tnumbers_at_range(const TemporalS *ts, RangeType *range);
extern TemporalS *tnumbers_minus_range(const TemporalS *ts, RangeType *range);
extern TemporalS *tnumbers_at_ranges(const TemporalS *ts, RangeType **normranges, int count);
extern TemporalS *tnumbers_minus_ranges(const TemporalS *ts, RangeType **normranges, int count);
extern TemporalS *temporals_at_min(const TemporalS *ts);
extern TemporalS *temporals_minus_min(const TemporalS *ts);
extern TemporalS *temporals_at_max(const TemporalS *ts);
extern TemporalS *temporals_minus_max(const TemporalS *ts);
extern TemporalInst *temporals_at_timestamp(const TemporalS *ts, TimestampTz t);
extern bool temporals_value_at_timestamp(const TemporalS *ts, TimestampTz t, Datum *result);
extern TemporalS *temporals_minus_timestamp(const TemporalS *ts, TimestampTz t);
extern TemporalI *temporals_at_timestampset(const TemporalS *ts, const TimestampSet *ts1);
extern TemporalS *temporals_minus_timestampset(const TemporalS *ts, const TimestampSet *ts1);
extern TemporalS *temporals_at_period(const TemporalS *ts, const Period *p);
extern TemporalS *temporals_minus_period(const TemporalS *ts, const Period *p);
extern TemporalS *temporals_at_periodset(const TemporalS *ts, const PeriodSet *ps);
extern TemporalS *temporals_minus_periodset(const TemporalS *ts, const PeriodSet *ps);
extern bool temporals_intersects_timestamp(const TemporalS *ts, TimestampTz t);
extern bool temporals_intersects_timestampset(const TemporalS *ts, const TimestampSet *ts1);
extern bool temporals_intersects_period(const TemporalS *ts, const Period *p);
extern bool temporals_intersects_periodset(const TemporalS *ts, const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumbers_integral(const TemporalS *ts);
extern double tnumbers_twavg(const TemporalS *ts);

/* Comparison functions */

extern int temporals_cmp(const TemporalS *ts1, const TemporalS *ts2);
extern bool temporals_eq(const TemporalS *ts1, const TemporalS *ts2);

/* Function for defining hash index */

extern uint32 temporals_hash(const TemporalS *ts);

/*****************************************************************************/

#endif
