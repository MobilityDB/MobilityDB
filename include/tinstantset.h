/*****************************************************************************
 *
 * tinstantset.h
 *	  Basic functions for temporal instant sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TINSTANTSET_H__
#define __TINSTANTSET_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/array.h>
#include <utils/rangetypes.h>

#include "temporal.h"

/*****************************************************************************/

extern TInstant *tinstantset_inst_n(const TInstantSet *ti, int index);
extern bool tinstantset_find_timestamp(const TInstantSet *ti, TimestampTz t, int *pos);
extern TInstantSet *tinstantset_make(TInstant **instants, int count);
extern TInstantSet *tinstantset_make_free(TInstant **instants, int count);
extern TInstantSet *tinstantset_copy(const TInstantSet *ti);

/* Intersection functions */

extern bool intersection_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
	TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
	TInstant **inter1, TInstant **inter2);
extern bool intersection_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
	TInstantSet **inter1, TInstantSet **inter2);

/* Input/output functions */

extern char *tinstantset_to_string(const TInstantSet *ti, char *(*value_out)(Oid, Datum));
extern void tinstantset_write(const TInstantSet *ti, StringInfo buf);
extern TInstantSet *tinstantset_read(StringInfo buf, Oid valuetypid);

/* Constructor functions */

extern TInstantSet *tinstantset_from_base_internal(Datum value, Oid valuetypid, const TimestampSet *ts);

extern Datum tinstantset_from_base(PG_FUNCTION_ARGS);

/* Append and merge functions */

extern TInstantSet *tinstantset_append_tinstant(const TInstantSet *ti, const TInstant *inst);
extern Temporal *tinstantset_merge(const TInstantSet *ti1, const TInstantSet *ti2);
extern Temporal *tinstantset_merge_array(TInstantSet **tis, int count);

/* Cast functions */
 
TInstantSet *tintinstset_to_tfloatinstset(const TInstantSet *ti);
TInstantSet *tfloatinstset_to_tintinstset(const TInstantSet *ti);

/* Transformation functions */

extern TInstantSet *tinstant_to_tinstantset(const TInstant *inst);
extern TInstantSet *tsequence_to_tinstantset(const TSequence *seq);
extern TInstantSet *tsequenceset_to_tinstantset(const TSequenceSet *ts);

/* Accessor functions */

extern ArrayType *tinstantset_values(const TInstantSet *ti);
extern ArrayType *tfloatinstset_ranges(const TInstantSet *ti);
extern PeriodSet *tinstantset_get_time(const TInstantSet *ti);
extern void *tinstantset_bbox_ptr(const TInstantSet *ti);
extern void tinstantset_bbox(void *box, const TInstantSet *ti);
extern Datum tinstantset_min_value(const TInstantSet *ti);
extern Datum tinstantset_max_value(const TInstantSet *ti);
extern void tinstantset_period(Period *p, const TInstantSet *ti);
extern TInstant **tinstantset_instants(const TInstantSet *ti);
extern ArrayType *tinstantset_instants_array(const TInstantSet *ti);
extern TimestampTz tinstantset_start_timestamp(const TInstantSet *ti);
extern TimestampTz tinstantset_end_timestamp(const TInstantSet *ti);
extern ArrayType *tinstantset_timestamps(const TInstantSet *ti);
extern TInstantSet *tinstantset_shift(const TInstantSet *ti, const Interval *interval);

extern bool tinstantset_ever_eq(const TInstantSet *ti, Datum value);
extern bool tinstantset_ever_lt(const TInstantSet *ti, Datum value);
extern bool tinstantset_ever_le(const TInstantSet *ti, Datum value);

extern bool tinstantset_always_eq(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_lt(const TInstantSet *ti, Datum value);
extern bool tinstantset_always_le(const TInstantSet *ti, Datum value);

/* Restriction Functions */

extern TInstantSet *tinstantset_at_value(const TInstantSet *ti, Datum value);
extern TInstantSet *tinstantset_minus_value(const TInstantSet *ti, Datum value);
extern TInstantSet *tinstantset_restrict_values(const TInstantSet *ti, 
	const Datum *values, int count, bool at);
extern TInstantSet *tnumberinstset_restrict_range(const TInstantSet *ti, 
	RangeType *range, bool at);
extern TInstantSet *tnumberinstset_restrict_ranges(const TInstantSet *ti, 
	RangeType **normranges, int count, bool at);
extern TInstant *tinstantset_min_instant(const TInstantSet *ti);
extern TInstantSet *tinstantset_at_min(const TInstantSet *ti);
extern TInstantSet *tinstantset_minus_min(const TInstantSet *ti);
extern TInstantSet *tinstantset_at_max(const TInstantSet *ti);
extern TInstantSet *tinstantset_minus_max(const TInstantSet *ti);
extern TInstant *tinstantset_at_timestamp(const TInstantSet *ti, TimestampTz t);
extern TInstantSet * tinstantset_minus_timestamp(const TInstantSet *ti, TimestampTz t);
extern bool tinstantset_value_at_timestamp(const TInstantSet *ti, TimestampTz t, Datum *result);
extern TInstantSet *tinstantset_restrict_timestampset(const TInstantSet *ti, 
	const TimestampSet *ts, bool at);
extern TInstantSet *tinstantset_restrict_period(const TInstantSet *ti, 
	const Period *p, bool at);
extern TInstantSet *tinstantset_restrict_periodset(const TInstantSet *ti, 
	const PeriodSet *ps, bool at);
extern bool tinstantset_intersects_timestamp(const TInstantSet *ti, const TimestampTz t);
extern bool tinstantset_intersects_timestampset(const TInstantSet *ti, const TimestampSet *ts);
extern bool tinstantset_intersects_period(const TInstantSet *ti, const Period *p);
extern bool tinstantset_intersects_periodset(const TInstantSet *ti, const PeriodSet *ps);

/* Local aggregate functions */

extern double tnumberinstset_twavg(const TInstantSet *ti);

/* Comparison functions */

extern int tinstantset_cmp(const TInstantSet *ti1, const TInstantSet *ti2);
extern bool tinstantset_eq(const TInstantSet *ti1, const TInstantSet *ti2);

/* Function for defining hash index */

extern uint32 tinstantset_hash(const TInstantSet *ti);

/*****************************************************************************/

#endif
