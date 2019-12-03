/*****************************************************************************
 *
 * temporali.h
 *	  Basic functions for temporal instant sets.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
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

extern TemporalInst *temporali_inst_n(TemporalI *ti, int index);
extern bool temporali_find_timestamp(TemporalI *ti, TimestampTz t, int *pos);
extern bool temporalinstarr_find_timestamp(TemporalInst **instants, int from, 
	int count, TimestampTz t, int *pos);
extern TemporalI *temporali_from_temporalinstarr(TemporalInst **instants, 
	int count);
extern TemporalI *temporali_copy(TemporalI *ti);

/* Intersection functions */

extern bool intersection_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	TemporalInst **inter1, TemporalInst **inter2);
extern bool intersection_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	TemporalI **inter1, TemporalI **inter2);

/* Input/output functions */

extern char *temporali_to_string(TemporalI *ti, char *(*value_out)(Oid, Datum));
extern void temporali_write(TemporalI *ti, StringInfo buf);
extern TemporalI *temporali_read(StringInfo buf, Oid valuetypid);

/* Append function */

extern TemporalI *temporali_append_instant(TemporalI *ti, TemporalInst *inst);

/* Cast functions */
 
TemporalI *tinti_to_tfloati(TemporalI *ti);
TemporalI *tfloati_to_tinti(TemporalI *ti);

/* Transformation functions */

extern TemporalI *temporalinst_to_temporali(TemporalInst *inst);
extern TemporalI *temporalseq_to_temporali(TemporalSeq *seq);
extern TemporalI *temporals_to_temporali(TemporalS *ts);

/* Accessor functions */

extern Datum *temporali_values1(TemporalI *ti, int *count);
extern ArrayType *temporali_values(TemporalI *ti);
extern ArrayType *tfloati_ranges(TemporalI *ti);
extern PeriodSet *temporali_get_time(TemporalI *ti);
extern void *temporali_bbox_ptr(TemporalI *ti);
extern void temporali_bbox(void *box, TemporalI *ti);
extern RangeType *tnumberi_value_range(TemporalI *ti);
extern Datum temporali_min_value(TemporalI *ti);
extern Datum temporali_max_value(TemporalI *ti);
extern void temporali_period(Period *p, TemporalI *ti);
extern TemporalInst **temporali_instants(TemporalI *ti);
extern ArrayType *temporali_instants_array(TemporalI *ti);
extern TimestampTz temporali_start_timestamp(TemporalI *ti);
extern TimestampTz temporali_end_timestamp(TemporalI *ti);
extern ArrayType *temporali_timestamps(TemporalI *ti);
extern bool temporali_ever_eq(TemporalI *ti, Datum value);
extern bool temporali_always_eq(TemporalI *ti, Datum value);
extern TemporalI *temporali_shift(TemporalI *ti, Interval *interval);

/* Restriction Functions */

extern TemporalI *temporali_at_value(TemporalI *ti, Datum value);
extern TemporalI *temporali_minus_value(TemporalI *ti, Datum value);
extern TemporalI *temporali_at_values(TemporalI *ti, Datum *values, int count);
extern TemporalI *temporali_minus_values(TemporalI *ti, Datum *values, int count);
extern TemporalI *tnumberi_at_range(TemporalI *ti, RangeType *range);
extern TemporalI *tnumberi_minus_range(TemporalI *ti, RangeType *range);
extern TemporalI *tnumberi_at_ranges(TemporalI *ti, RangeType **normranges, int count);
extern TemporalI *tnumberi_minus_ranges(TemporalI *ti, RangeType **normranges, int count);
extern TemporalI *temporali_at_min(TemporalI *ti);
extern TemporalI *temporali_minus_min(TemporalI *ti);
extern TemporalI *temporali_at_max(TemporalI *ti);
extern TemporalI *temporali_minus_max(TemporalI *ti);
extern TemporalInst *temporali_at_timestamp(TemporalI *ti, TimestampTz t);
extern bool temporali_value_at_timestamp(TemporalI *ti, TimestampTz t, Datum *result);
extern TemporalI * temporali_minus_timestamp(TemporalI *ti, TimestampTz t);
extern TemporalI *temporali_at_timestampset(TemporalI *ti, TimestampSet *ts);
extern TemporalI *temporali_minus_timestampset(TemporalI *ti, TimestampSet *ts);
extern TemporalI *temporali_at_period(TemporalI *ti, Period *p);
extern TemporalI *temporali_minus_period(TemporalI *ti, Period *p);
extern TemporalI *temporali_at_periodset(TemporalI *ti, PeriodSet *ps);
extern TemporalI *temporali_minus_periodset(TemporalI *ti, PeriodSet *ps);
extern bool temporali_intersects_timestamp(TemporalI *ti, TimestampTz t);
extern bool temporali_intersects_timestampset(TemporalI *ti, TimestampSet *ts);
extern bool temporali_intersects_period(TemporalI *ti, Period *p);
extern bool temporali_intersects_periodset(TemporalI *ti, PeriodSet *ps);

/* Local aggregate functions */

extern double temporali_twavg(TemporalI *ti);

/* Comparison functions */

extern int temporali_cmp(TemporalI *ti1, TemporalI *ti2);
extern bool temporali_eq(TemporalI *ti1, TemporalI *ti2);

/* Function for defining hash index */

extern uint32 temporali_hash(TemporalI *ti);

/*****************************************************************************/

#endif
