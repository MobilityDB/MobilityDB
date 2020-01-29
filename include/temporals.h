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
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "temporal.h"

/*****************************************************************************/

/* General functions */

extern TemporalSeq *temporals_seq_n(TemporalS *ts, int index);
extern TemporalS *temporals_from_temporalseqarr(TemporalSeq **sequences, 
	int count, bool linear, bool normalize);
extern TemporalS *temporals_copy(TemporalS *ts);
extern bool temporals_find_timestamp(TemporalS *ts, TimestampTz t, int *pos);
extern double temporals_interval_double(TemporalS *ts);

/* Intersection functions */

extern bool intersection_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporals(TemporalInst *inst, TemporalS *ts,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporali_temporals(TemporalI *ti, TemporalS *ts,
	TemporalI **inter1, TemporalI **inter2);
extern bool intersection_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	TemporalS **inter1, TemporalS **inter2);
extern bool intersection_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts, 
	TemporalS **inter1, TemporalS **inter2);
extern bool intersection_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	TemporalS **inter1, TemporalS **inter2);

/* Synchronize functions */

extern bool synchronize_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq,
	TemporalS **sync1, TemporalS **sync2, bool interpoint);
extern bool synchronize_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts, 
	TemporalS **sync1, TemporalS **sync2, bool interpoint);
extern bool synchronize_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	TemporalS **sync1, TemporalS **sync2, bool interpoint);

/* Input/output functions */

extern char *temporals_to_string(TemporalS *ts, char *(*value_out)(Oid, Datum));
extern void temporals_write(TemporalS *ts, StringInfo buf);
extern TemporalS *temporals_read(StringInfo buf, Oid valuetypid);

/* Append function */

extern TemporalS *temporals_append_instant(TemporalS *ts, TemporalInst *inst);

/* Cast functions */

extern TemporalS *tints_to_tfloats(TemporalS *ts);
extern TemporalS *tfloats_to_tints(TemporalS *ts);

/* Transformation functions */

extern TemporalS *temporalinst_to_temporals(TemporalInst *inst, bool linear);
extern TemporalS *temporali_to_temporals(TemporalI *ti, bool linear);
extern TemporalS *temporalseq_to_temporals(TemporalSeq *seq);
extern TemporalS *tstepws_to_linear(TemporalS *ts);

/* Accessor functions */

extern Datum *temporals_values1(TemporalS *ts, int *count);
extern ArrayType *temporals_values(TemporalS *ts);
extern ArrayType *tfloats_ranges(TemporalS *ts);
extern void *temporals_bbox_ptr(TemporalS *ts);
extern void temporals_bbox(void *box, TemporalS *ts);
extern Datum temporals_min_value(TemporalS *ts);
extern Datum temporals_max_value(TemporalS *ts);
extern PeriodSet *temporals_get_time(TemporalS *ts);
extern Datum temporals_timespan(TemporalS *ts);
extern void temporals_period(Period *p, TemporalS *ts);
extern TemporalSeq **temporals_sequences(TemporalS *ts);
extern ArrayType *temporals_sequences_array(TemporalS *ts);
extern int temporals_num_instants(TemporalS *ts);
extern TemporalInst *temporals_instant_n(TemporalS *ts, int n);
extern ArrayType *temporals_instants_array(TemporalS *ts);
extern TimestampTz temporals_start_timestamp(TemporalS *ts);
extern TimestampTz temporals_end_timestamp(TemporalS *ts);
extern int temporals_num_timestamps(TemporalS *ts);
extern bool temporals_timestamp_n(TemporalS *ts, int n, TimestampTz *result);
extern TimestampTz *temporals_timestamps1(TemporalS *ts, int *count);
extern ArrayType *temporals_timestamps(TemporalS *ts);
extern TemporalS *temporals_shift(TemporalS *ts, Interval *interval);

extern bool temporals_ever_eq(TemporalS *ts, Datum value);
extern bool temporals_ever_lt(TemporalS *ts, Datum value);
extern bool temporals_ever_le(TemporalS *ts, Datum value);

extern bool temporals_always_eq(TemporalS *ts, Datum value);
extern bool temporals_always_lt(TemporalS *ts, Datum value);
extern bool temporals_always_le(TemporalS *ts, Datum value);

/* Restriction Functions */

extern TemporalS *temporals_at_value(TemporalS *ts, Datum value);
extern TemporalS *temporals_minus_value(TemporalS *ts, Datum value);
extern TemporalS *temporals_at_values(TemporalS *ts, Datum *values, int count);
extern TemporalS *temporals_minus_values(TemporalS *ts, Datum *values, int count);
extern TemporalS *tnumbers_at_range(TemporalS *ts, RangeType *range);
extern TemporalS *tnumbers_minus_range(TemporalS *ts, RangeType *range);
extern TemporalS *tnumbers_at_ranges(TemporalS *ts, RangeType **normranges, int count);
extern TemporalS *tnumbers_minus_ranges(TemporalS *ts, RangeType **normranges, int count);
extern TemporalS *temporals_at_min(TemporalS *ts);
extern TemporalS *temporals_minus_min(TemporalS *ts);
extern TemporalS *temporals_at_max(TemporalS *ts);
extern TemporalS *temporals_minus_max(TemporalS *ts);
extern TemporalInst *temporals_at_timestamp(TemporalS *ts, TimestampTz t);
extern bool temporals_value_at_timestamp(TemporalS *ts, TimestampTz t, Datum *result);
extern TemporalS *temporals_minus_timestamp(TemporalS *ts, TimestampTz t);
extern TemporalI *temporals_at_timestampset(TemporalS *ts, TimestampSet *ts1);
extern TemporalS *temporals_minus_timestampset(TemporalS *ts, TimestampSet *ts1);
extern TemporalS *temporals_at_period(TemporalS *ts, Period *p);
extern TemporalS *temporals_minus_period(TemporalS *ts, Period *p);
extern TemporalS *temporals_at_periodset(TemporalS *ts, PeriodSet *ps);
extern TemporalS *temporals_minus_periodset(TemporalS *ts, PeriodSet *ps);
extern bool temporals_intersects_timestamp(TemporalS *ts, TimestampTz t);
extern bool temporals_intersects_timestampset(TemporalS *ts, TimestampSet *ts1);
extern bool temporals_intersects_period(TemporalS *ts, Period *p);
extern bool temporals_intersects_periodset(TemporalS *ts, PeriodSet *ps);

/* Local aggregate functions */

extern double tnumbers_integral(TemporalS *ts);
extern double tnumbers_twavg(TemporalS *ts);

/* Comparison functions */

extern int temporals_cmp(TemporalS *ts1, TemporalS *ts2);
extern bool temporals_eq(TemporalS *ts1, TemporalS *ts2);

/* Function for defining hash index */

extern uint32 temporals_hash(TemporalS *ts);

/*****************************************************************************/

#endif
