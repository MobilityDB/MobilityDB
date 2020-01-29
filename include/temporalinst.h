/*****************************************************************************
 *
 * temporalinst.h
 *	  Basic functions for temporal instants.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALINST_H__
#define __TEMPORALINST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>
#include "temporal.h"
#include "postgis.h"

/*****************************************************************************/
 
extern TemporalInst *temporalinst_make(Datum value, TimestampTz t, Oid valuetypid);
extern TemporalInst *temporalinst_copy(TemporalInst *inst);
extern Datum* temporalinst_value_ptr(TemporalInst *inst);
extern Datum temporalinst_value(TemporalInst *inst);
extern Datum temporalinst_value_copy(TemporalInst *inst);

/* Input/output functions */

char *temporalinst_to_string(TemporalInst *inst, char *(*value_out)(Oid, Datum));
extern void temporalinst_write(TemporalInst *inst, StringInfo buf);
extern TemporalInst *temporalinst_read(StringInfo buf, Oid valuetypid);

/* Intersection function */

extern bool intersection_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	TemporalInst **inter1, TemporalInst **inter2);

/* Append function */

extern TemporalI *temporalinst_append_instant(TemporalInst *inst1, TemporalInst *inst2);

/* Cast functions */

extern TemporalInst *tintinst_to_tfloatinst(TemporalInst *inst);
extern TemporalInst *tfloatinst_to_tintinst(TemporalInst *inst);

/* Transformation functions */

extern TemporalInst *temporali_to_temporalinst(TemporalI *ti);
extern TemporalInst *temporalseq_to_temporalinst(TemporalSeq *seq);
extern TemporalInst *temporals_to_temporalinst(TemporalS *ts);

/* Accessor functions */

extern ArrayType *temporalinst_values(TemporalInst *inst);
extern ArrayType *tfloatinst_ranges(TemporalInst *inst);
extern PeriodSet *temporalinst_get_time(TemporalInst *inst);
extern void temporalinst_bbox(void *box, TemporalInst *inst);
extern void temporalinst_period(Period *p, TemporalInst *inst);
extern ArrayType *temporalinst_timestamps(TemporalInst *inst);
extern ArrayType *temporalinst_instants_array(TemporalInst *inst);
extern TemporalInst *temporalinst_shift(TemporalInst *inst, Interval *interval);

extern bool temporalinst_ever_eq(TemporalInst *inst, Datum value);
extern bool temporalinst_ever_lt(TemporalInst *inst, Datum value);
extern bool temporalinst_ever_le(TemporalInst *inst, Datum value);

extern bool temporalinst_always_eq(TemporalInst *inst, Datum value);
extern bool temporalinst_always_lt(TemporalInst *inst, Datum value);
extern bool temporalinst_always_le(TemporalInst *inst, Datum value);

/* Restriction Functions */

extern TemporalInst *temporalinst_at_value(TemporalInst *inst, Datum val);
extern TemporalInst *temporalinst_minus_value(TemporalInst *inst, Datum val);
extern TemporalInst *temporalinst_at_values(TemporalInst *inst, Datum *values, int count);
extern TemporalInst *temporalinst_minus_values(TemporalInst *inst, Datum *values, int count);
extern TemporalInst *tnumberinst_at_range(TemporalInst *inst, RangeType *range);
extern TemporalInst *tnumberinst_minus_range(TemporalInst *inst, RangeType *range);

extern TemporalInst *temporalinst_at_timestamp(TemporalInst *inst, TimestampTz t);
extern bool temporalinst_value_at_timestamp(TemporalInst *inst, TimestampTz t, Datum *result);
extern TemporalInst *temporalinst_minus_timestamp(TemporalInst *inst, TimestampTz t);
extern TemporalInst *temporalinst_at_timestampset(TemporalInst *inst, TimestampSet *ts);
extern TemporalInst *temporalinst_minus_timestampset(TemporalInst *inst, TimestampSet *ts);
extern TemporalInst *temporalinst_at_period(TemporalInst *inst, Period *p);
extern TemporalInst *temporalinst_minus_period(TemporalInst *inst, Period *p);
extern TemporalInst *temporalinst_at_periodset(TemporalInst *inst, PeriodSet *ps);
extern TemporalInst *temporalinst_minus_periodset(TemporalInst *inst, PeriodSet *ps);

extern TemporalInst *tnumberinst_at_ranges(TemporalInst *inst, RangeType **normranges, int count);
extern TemporalInst *tnumberinst_minus_ranges(TemporalInst *inst, RangeType **normranges, int count);

extern bool temporalinst_intersects_timestamp(TemporalInst *inst, TimestampTz t);
extern bool temporalinst_intersects_timestampset(TemporalInst *inst, TimestampSet *ts);
extern bool temporalinst_intersects_period(TemporalInst *inst, Period *p);
extern bool temporalinst_intersects_periodset(TemporalInst *inst, PeriodSet *ps);

/* Comparison functions */

extern int temporalinst_cmp(TemporalInst *inst1, TemporalInst *inst2);
extern bool temporalinst_eq(TemporalInst *inst1, TemporalInst *inst2);

/* Function for defining hash index */

extern uint32 temporalinst_hash(TemporalInst *inst);

/*****************************************************************************/

#endif
