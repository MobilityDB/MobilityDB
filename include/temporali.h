/*****************************************************************************
 *
 * temporali.h
 *	  Basic functions for temporal instant sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALI_H__
#define __TEMPORALI_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
#include "temporal.h"

/*****************************************************************************/

extern TemporalInst *temporali_inst_n(const TemporalI *ti, int index);
extern bool temporali_find_timestamp(const TemporalI *ti, TimestampTz t, int *pos);
extern TemporalI *temporali_make(TemporalInst **instants, int count);
extern TemporalI *temporali_copy(const TemporalI *ti);

/* Intersection functions */

extern bool intersection_temporali_temporalinst(const TemporalI *ti, const TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporali(const TemporalInst *inst, const TemporalI *ti,
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	TemporalI **inter1, TemporalI **inter2);

/* Input/output functions */

extern char *temporali_to_string(const TemporalI *ti, char *(*value_out)(Oid, Datum));
extern void temporali_write(const TemporalI *ti, StringInfo buf);
extern TemporalI *temporali_read(StringInfo buf, Oid valuetypid);

/* Constructor functions */

extern TemporalI *temporali_from_base_internal(Datum value, Oid valuetypid, const TimestampSet *ts);

extern Datum temporali_from_base(PG_FUNCTION_ARGS);

/* Append and merge functions */

extern TemporalI *temporali_append_instant(const TemporalI *ti, const TemporalInst *inst);
extern Temporal *temporali_merge_array(TemporalI **tis, int count);
extern Temporal *temporali_merge(const TemporalI *ti1, const TemporalI *ti2);

/* Cast functions */
 
TemporalI *tinti_to_tfloati(const TemporalI *ti);
TemporalI *tfloati_to_tinti(const TemporalI *ti);

/* Transformation functions */

extern TemporalI *temporalinst_to_temporali(const TemporalInst *inst);
extern TemporalI *temporalseq_to_temporali(const TemporalSeq *seq);
extern TemporalI *temporals_to_temporali(const TemporalS *ts);

/* Accessor functions */

extern ArrayType *temporali_values(const TemporalI *ti);
extern ArrayType *tfloati_ranges(const TemporalI *ti);
extern PeriodSet *temporali_get_time(const TemporalI *ti);
extern void *temporali_bbox_ptr(const TemporalI *ti);
extern void temporali_bbox(void *box, const TemporalI *ti);
extern Datum temporali_min_value(const TemporalI *ti);
extern Datum temporali_max_value(const TemporalI *ti);
extern void temporali_period(Period *p, const TemporalI *ti);
extern TemporalInst **temporali_instants(const TemporalI *ti);
extern ArrayType *temporali_instants_array(const TemporalI *ti);
extern TimestampTz temporali_start_timestamp(const TemporalI *ti);
extern TimestampTz temporali_end_timestamp(const TemporalI *ti);
extern ArrayType *temporali_timestamps(const TemporalI *ti);
extern TemporalI *temporali_shift(const TemporalI *ti, const Interval *interval);

extern bool temporali_ever_eq(const TemporalI *ti, Datum value);
extern bool temporali_ever_lt(const TemporalI *ti, Datum value);
extern bool temporali_ever_le(const TemporalI *ti, Datum value);

extern bool temporali_always_eq(const TemporalI *ti, Datum value);
extern bool temporali_always_lt(const TemporalI *ti, Datum value);
extern bool temporali_always_le(const TemporalI *ti, Datum value);

/* Restriction Functions */

extern TemporalI *temporali_at_value(const TemporalI *ti, Datum value);
extern TemporalI *temporali_minus_value(const TemporalI *ti, Datum value);
extern TemporalI *temporali_at_values(const TemporalI *ti, const Datum *values, int count);
extern TemporalI *temporali_minus_values(const TemporalI *ti, const Datum *values, int count);
extern TemporalI *tnumberi_at_range(const TemporalI *ti, RangeType *range);
extern TemporalI *tnumberi_minus_range(const TemporalI *ti, RangeType *range);
extern TemporalI *tnumberi_at_ranges(const TemporalI *ti, RangeType **normranges, int count);
extern TemporalI *tnumberi_minus_ranges(const TemporalI *ti, RangeType **normranges, int count);
extern TemporalInst *temporali_min_instant(const TemporalI *ti);
extern TemporalI *temporali_at_min(const TemporalI *ti);
extern TemporalI *temporali_minus_min(const TemporalI *ti);
extern TemporalI *temporali_at_max(const TemporalI *ti);
extern TemporalI *temporali_minus_max(const TemporalI *ti);
extern TemporalInst *temporali_at_timestamp(const TemporalI *ti, TimestampTz t);
extern bool temporali_value_at_timestamp(const TemporalI *ti, TimestampTz t, Datum *result);
extern TemporalI * temporali_minus_timestamp(const TemporalI *ti, TimestampTz t);
extern TemporalI *temporali_at_timestampset(const TemporalI *ti, const TimestampSet *ts);
extern TemporalI *temporali_minus_timestampset(const TemporalI *ti, const TimestampSet *ts);
extern TemporalI *temporali_at_period(const TemporalI *ti, const Period *p);
extern TemporalI *temporali_minus_period(const TemporalI *ti, const Period *p);
extern TemporalI *temporali_at_periodset(const TemporalI *ti, const PeriodSet *ps);
extern TemporalI *temporali_minus_periodset(const TemporalI *ti, const PeriodSet *ps);
extern bool temporali_intersects_timestamp(const TemporalI *ti, const TimestampTz t);
extern bool temporali_intersects_timestampset(const TemporalI *ti, const TimestampSet *ts);
extern bool temporali_intersects_period(const TemporalI *ti, const Period *p);
extern bool temporali_intersects_periodset(const TemporalI *ti, const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumberi_twavg(const TemporalI *ti);

/* Comparison functions */

extern int temporali_cmp(const TemporalI *ti1, const TemporalI *ti2);
extern bool temporali_eq(const TemporalI *ti1, const TemporalI *ti2);

/* Function for defining hash index */

extern uint32 temporali_hash(const TemporalI *ti);

/*****************************************************************************/

#endif
