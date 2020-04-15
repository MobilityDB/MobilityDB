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
extern TemporalInst *temporalinst_copy(const TemporalInst *inst);
extern Datum* temporalinst_value_ptr(const TemporalInst *inst);
extern Datum temporalinst_value(const TemporalInst *inst);
extern Datum temporalinst_value_copy(const TemporalInst *inst);
extern void temporalinst_set(TemporalInst *inst, Datum value, TimestampTz t);

/* Input/output functions */

extern char *temporalinst_to_string(const TemporalInst *inst, char *(*value_out)(Oid, Datum));
extern void temporalinst_write(const TemporalInst *inst, StringInfo buf);
extern TemporalInst *temporalinst_read(StringInfo buf, Oid valuetypid);

/* Intersection function */

extern bool intersection_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
	TemporalInst **inter1, TemporalInst **inter2);

/* Append and merge functions */

extern TemporalI *temporalinst_append_instant(const TemporalInst *inst1, const TemporalInst *inst2);
extern Temporal *temporalinst_merge(const TemporalInst *inst1, const TemporalInst *inst2);
extern TemporalI *temporalinst_merge_array(TemporalInst **instants, int count);

/* Cast functions */

extern TemporalInst *tintinst_to_tfloatinst(const TemporalInst *inst);
extern TemporalInst *tfloatinst_to_tintinst(const TemporalInst *inst);

/* Transformation functions */

extern TemporalInst *temporali_to_temporalinst(const TemporalI *ti);
extern TemporalInst *temporalseq_to_temporalinst(const TemporalSeq *seq);
extern TemporalInst *temporals_to_temporalinst(const TemporalS *ts);

/* Accessor functions */

extern ArrayType *temporalinst_values(const TemporalInst *inst);
extern ArrayType *tfloatinst_ranges(const TemporalInst *inst);
extern PeriodSet *temporalinst_get_time(const TemporalInst *inst);
extern void temporalinst_period(Period *p, const TemporalInst *inst);
extern ArrayType *temporalinst_timestamps(const TemporalInst *inst);
extern ArrayType *temporalinst_instants_array(const TemporalInst *inst);
extern TemporalInst *temporalinst_shift(const TemporalInst *inst, const Interval *interval);

extern bool temporalinst_ever_eq(const TemporalInst *inst, Datum value);
extern bool temporalinst_ever_lt(const TemporalInst *inst, Datum value);
extern bool temporalinst_ever_le(const TemporalInst *inst, Datum value);

extern bool temporalinst_always_eq(const TemporalInst *inst, Datum value);
extern bool temporalinst_always_lt(const TemporalInst *inst, Datum value);
extern bool temporalinst_always_le(const TemporalInst *inst, Datum value);

/* Restriction Functions */

extern TemporalInst *temporalinst_at_value(const TemporalInst *inst, Datum value);
extern TemporalInst *temporalinst_minus_value(const TemporalInst *inst, Datum value);
extern TemporalInst *temporalinst_at_values(const TemporalInst *inst, const Datum *values, int count);
extern TemporalInst *temporalinst_minus_values(const TemporalInst *inst, const Datum *values, int count);
extern TemporalInst *tnumberinst_at_range(const TemporalInst *inst, RangeType *range);
extern TemporalInst *tnumberinst_minus_range(const TemporalInst *inst, RangeType *range);

extern TemporalInst *temporalinst_at_timestamp(const TemporalInst *inst, TimestampTz t);
extern bool temporalinst_value_at_timestamp(const TemporalInst *inst, TimestampTz t, Datum *result);
extern TemporalInst *temporalinst_minus_timestamp(const TemporalInst *inst, TimestampTz t);
extern TemporalInst *temporalinst_at_timestampset(const TemporalInst *inst, const TimestampSet *ts);
extern TemporalInst *temporalinst_minus_timestampset(const TemporalInst *inst, const TimestampSet *ts);
extern TemporalInst *temporalinst_at_period(const TemporalInst *inst, const Period *p);
extern TemporalInst *temporalinst_minus_period(const TemporalInst *inst, const Period *p);
extern TemporalInst *temporalinst_at_periodset(const TemporalInst *inst,const  PeriodSet *ps);
extern TemporalInst *temporalinst_minus_periodset(const TemporalInst *inst, const PeriodSet *ps);

extern TemporalInst *tnumberinst_at_ranges(const TemporalInst *inst, RangeType **normranges, int count);
extern TemporalInst *tnumberinst_minus_ranges(const TemporalInst *inst, RangeType **normranges, int count);

extern bool temporalinst_intersects_timestamp(const TemporalInst *inst, TimestampTz t);
extern bool temporalinst_intersects_timestampset(const TemporalInst *inst, const TimestampSet *ts);
extern bool temporalinst_intersects_period(const TemporalInst *inst, const Period *p);
extern bool temporalinst_intersects_periodset(const TemporalInst *inst, const PeriodSet *ps);

/* Comparison functions */

extern int temporalinst_cmp(const TemporalInst *inst1, const TemporalInst *inst2);
extern bool temporalinst_eq(const TemporalInst *inst1, const TemporalInst *inst2);

/* Function for defining hash index */

extern uint32 temporalinst_hash(const TemporalInst *inst);

/*****************************************************************************/

#endif
