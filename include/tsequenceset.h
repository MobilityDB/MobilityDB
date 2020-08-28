/*****************************************************************************
 *
 * tsequenceset.h
 *	  Basic functions for temporal sequence sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TSEQUENCESET_H__
#define __TSEQUENCESET_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "temporal.h"

/*****************************************************************************/

/* General functions */

extern TSequence *tsequenceset_seq_n(const TSequenceSet *ts, int index);
extern TSequenceSet *tsequenceset_make(TSequence **sequences, int count,
	bool normalize);
extern TSequenceSet * tsequenceset_make_free(TSequence **sequences, int count, 
	bool normalize);
extern TSequenceSet *tsequenceset_copy(const TSequenceSet *ts);
extern bool tsequenceset_find_timestamp(const TSequenceSet *ts, TimestampTz t,
	int *loc);
extern double tsequenceset_interval_double(const TSequenceSet *ts);

/* Intersection/synchronize functions */

extern bool intersection_tsequenceset_tinstant(const TSequenceSet *ts,
	const TInstant *inst, TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tsequenceset(const TInstant *inst,
	const TSequenceSet *ts, TInstant **inter1, TInstant **inter2);
extern bool intersection_tsequenceset_tinstantset(const TSequenceSet *ts,
	const TInstantSet *ti, TInstantSet **inter1, TInstantSet **inter2);
extern bool intersection_tinstantset_tsequenceset(const TInstantSet *ti,
	const TSequenceSet *ts, TInstantSet **inter1, TInstantSet **inter2);
extern bool intersection_tsequenceset_tsequence(const TSequenceSet *ts,
	const TSequence *seq, TIntersection mode,
	TSequenceSet **inter1, TSequenceSet **inter2);
extern bool intersection_tsequence_tsequenceset(const TSequence *seq,
	const TSequenceSet *ts, TIntersection mode,
	TSequenceSet **inter1, TSequenceSet **inter2);
extern bool intersection_tsequenceset_tsequenceset(const TSequenceSet *ts1,
	const TSequenceSet *ts2, TIntersection mode,
	TSequenceSet **inter1, TSequenceSet **inter2);

/* Input/output functions */

extern char *tsequenceset_to_string(const TSequenceSet *ts,
	char *(*value_out)(Oid, Datum));
extern void tsequenceset_write(const TSequenceSet *ts, StringInfo buf);
extern TSequenceSet *tsequenceset_read(StringInfo buf, Oid valuetypid);

/* Constructor functions */

extern TSequenceSet *tsequenceset_from_base_internal(Datum value,
	Oid valuetypid, const PeriodSet *ps, bool linear);

extern Datum tsequenceset_from_base(PG_FUNCTION_ARGS);

/* Append and merge functions */

extern TSequenceSet *tsequenceset_append_tinstant(const TSequenceSet *ts,
	const TInstant *inst);
extern TSequenceSet *tsequenceset_merge(const TSequenceSet *ts1,
	const TSequenceSet *ts2);
extern TSequenceSet *tsequenceset_merge_array(TSequenceSet **ts,
	int count);

/* Cast functions */

extern TSequenceSet *tintseqset_to_tfloatseqset(const TSequenceSet *ts);
extern TSequenceSet *tfloatseqset_to_tintseqset(const TSequenceSet *ts);

/* Transformation functions */

extern TSequenceSet *tinstant_to_tsequenceset(const TInstant *inst,
	bool linear);
extern TSequenceSet *tinstantset_to_tsequenceset(const TInstantSet *ti,
	bool linear);
extern TSequenceSet *tsequence_to_tsequenceset(const TSequence *seq);
extern TSequenceSet *tstepseqset_to_linear(const TSequenceSet *ts);

/* Accessor functions */

extern ArrayType *tsequenceset_values(const TSequenceSet *ts);
extern ArrayType *tfloatseqset_ranges(const TSequenceSet *ts);
extern void *tsequenceset_bbox_ptr(const TSequenceSet *ts);
extern void tsequenceset_bbox(void *box, const TSequenceSet *ts);
extern TInstant *tsequenceset_min_instant(const TSequenceSet *ts);
extern Datum tsequenceset_min_value(const TSequenceSet *ts);
extern Datum tsequenceset_max_value(const TSequenceSet *ts);
extern PeriodSet *tsequenceset_get_time(const TSequenceSet *ts);
extern Datum tsequenceset_timespan(const TSequenceSet *ts);
extern void tsequenceset_period(Period *p, const TSequenceSet *ts);
extern TSequence **tsequenceset_sequences(const TSequenceSet *ts);
extern ArrayType *tsequenceset_sequences_array(const TSequenceSet *ts);
extern int tsequenceset_num_instants(const TSequenceSet *ts);
extern TInstant *tsequenceset_instant_n(const TSequenceSet *ts, int n);
extern ArrayType *tsequenceset_instants_array(const TSequenceSet *ts);
extern TimestampTz tsequenceset_start_timestamp(const TSequenceSet *ts);
extern TimestampTz tsequenceset_end_timestamp(const TSequenceSet *ts);
extern int tsequenceset_num_timestamps(const TSequenceSet *ts);
extern bool tsequenceset_timestamp_n(const TSequenceSet *ts, int n, 
	TimestampTz *result);
extern ArrayType *tsequenceset_timestamps(const TSequenceSet *ts);
extern TSequenceSet *tsequenceset_shift(const TSequenceSet *ts, 
	const Interval *interval);

extern bool tsequenceset_ever_eq(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_ever_lt(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_ever_le(const TSequenceSet *ts, Datum value);

extern bool tsequenceset_always_eq(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_always_lt(const TSequenceSet *ts, Datum value);
extern bool tsequenceset_always_le(const TSequenceSet *ts, Datum value);

/* Restriction Functions */

extern TSequenceSet *tsequenceset_restrict_value(const TSequenceSet *ts, 
	Datum value, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_values(const TSequenceSet *ts, 
	const Datum *values, int count, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_range(const TSequenceSet *ts, 
	RangeType *range, bool atfunc);
extern TSequenceSet *tnumberseqset_restrict_ranges(const TSequenceSet *ts, 
	RangeType **normranges, int count, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_minmax(const TSequenceSet *ts, 
	bool min, bool atfunc);
	
extern bool tsequenceset_value_at_timestamp(const TSequenceSet *ts, 
	TimestampTz t, Datum *result);
extern bool tsequenceset_value_at_timestamp_inc(const TSequenceSet *ts, 
	TimestampTz t, Datum *result);

extern Temporal *tsequenceset_restrict_timestamp(const TSequenceSet *ts, 
	TimestampTz t, bool atfunc);
extern Temporal *tsequenceset_restrict_timestampset(const TSequenceSet *ts1, 
	const TimestampSet *ts2, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_period(const TSequenceSet *ts, 
	const Period *p, bool atfunc);
extern TSequenceSet *tsequenceset_restrict_periodset(const TSequenceSet *ts, 
	const PeriodSet *ps, bool atfunc);

/* Intersection functions */

extern bool tsequenceset_intersects_timestamp(const TSequenceSet *ts,
	TimestampTz t);
extern bool tsequenceset_intersects_timestampset(const TSequenceSet *ts,
	const TimestampSet *ts1);
extern bool tsequenceset_intersects_period(const TSequenceSet *ts,
	const Period *p);
extern bool tsequenceset_intersects_periodset(const TSequenceSet *ts,
	const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumberseqset_integral(const TSequenceSet *ts);
extern double tnumberseqset_twavg(const TSequenceSet *ts);

/* Comparison functions */

extern int tsequenceset_cmp(const TSequenceSet *ts1, const TSequenceSet *ts2);
extern bool tsequenceset_eq(const TSequenceSet *ts1, const TSequenceSet *ts2);

/* Function for defining hash index */

extern uint32 tsequenceset_hash(const TSequenceSet *ts);

/*****************************************************************************/

#endif
