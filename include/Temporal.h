/*****************************************************************************
 *
 * Temporal.h
 *	Basic functions for temporal types of any duration.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_H__
#define __TEMPORAL_H__

#include <postgres.h>

/*****************************************************************************/

/* Internal functions */

extern Temporal *temporal_copy(Temporal *temp);
extern Temporal *pg_getarg_temporal(Temporal *temp);
extern bool intersection_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Temporal **inter1, Temporal **inter2);
extern bool synchronize_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Temporal **sync1, Temporal **sync2, bool interpoint);
extern RangeType *tnumber_floatrange(Temporal *temp);
extern const char *temporal_type_name(uint8_t type);
extern bool temporal_type_from_string(const char *str, uint8_t *type);


/* Input/output functions */

extern Datum temporal_in(PG_FUNCTION_ARGS); 
extern Datum temporal_out(PG_FUNCTION_ARGS); 
extern Datum temporal_send(PG_FUNCTION_ARGS); 
extern Datum temporal_recv(PG_FUNCTION_ARGS);
extern Temporal* temporal_read(StringInfo buf, Oid valuetypid);
extern void temporal_write(Temporal* temp, StringInfo buf);

/* Constructor functions */

extern Datum temporal_make_temporalinst(PG_FUNCTION_ARGS);
extern Datum temporal_make_temporali(PG_FUNCTION_ARGS);
extern Datum temporal_make_temporalseq(PG_FUNCTION_ARGS);
extern Datum temporal_make_temporals(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum tint_as_tfloat(PG_FUNCTION_ARGS);

extern Temporal *tint_as_tfloat_internal(Temporal *temp);

/* Accessor functions */

extern Datum temporal_type(PG_FUNCTION_ARGS);
extern Datum temporal_mem_size(PG_FUNCTION_ARGS);
extern Datum tempdisc_get_values(PG_FUNCTION_ARGS);
extern Datum tfloat_ranges(PG_FUNCTION_ARGS);
extern Datum temporal_get_time(PG_FUNCTION_ARGS);
extern Datum temporalinst_get_value(PG_FUNCTION_ARGS);
extern Datum tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum tnumber_value_range(PG_FUNCTION_ARGS);
extern Datum temporal_start_value(PG_FUNCTION_ARGS);
extern Datum temporal_end_value(PG_FUNCTION_ARGS);
extern Datum temporal_min_value(PG_FUNCTION_ARGS);
extern Datum temporal_max_value(PG_FUNCTION_ARGS);
extern Datum temporal_time(PG_FUNCTION_ARGS);
extern Datum temporal_timespan(PG_FUNCTION_ARGS);
extern Datum temporal_num_instants(PG_FUNCTION_ARGS);
extern Datum temporal_start_instant(PG_FUNCTION_ARGS);
extern Datum temporal_end_instant(PG_FUNCTION_ARGS);
extern Datum temporal_instant_n(PG_FUNCTION_ARGS);
extern Datum temporal_instants(PG_FUNCTION_ARGS);
extern Datum temporal_num_timestamps(PG_FUNCTION_ARGS);
extern Datum temporal_start_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_end_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_timestamp_n(PG_FUNCTION_ARGS);
extern Datum temporal_ever_equals(PG_FUNCTION_ARGS);
extern Datum temporal_always_equals(PG_FUNCTION_ARGS);
extern Datum temporal_shift(PG_FUNCTION_ARGS);

extern Datum tempdisc_get_values_internal(Temporal *temp);
extern Datum temporal_min_value_internal(Temporal *temp);
extern TimestampTz temporal_start_timestamp_internal(Temporal *temp);

/* Restriction functions */

extern Datum temporal_at_value(PG_FUNCTION_ARGS);
extern Datum temporal_minus_value(PG_FUNCTION_ARGS);
extern Datum temporal_at_values(PG_FUNCTION_ARGS);
extern Datum tnumber_at_range(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_range(PG_FUNCTION_ARGS);
extern Datum tnumber_at_ranges(PG_FUNCTION_ARGS);
extern Datum tnumber_minus_ranges(PG_FUNCTION_ARGS);
extern Datum temporal_at_min(PG_FUNCTION_ARGS);
extern Datum temporal_minus_min(PG_FUNCTION_ARGS);
extern Datum temporal_at_max(PG_FUNCTION_ARGS);
extern Datum temporal_minus_max(PG_FUNCTION_ARGS);
extern Datum temporal_at_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_minus_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_value_at_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_at_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_minus_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_at_period(PG_FUNCTION_ARGS);
extern Datum temporal_minus_period(PG_FUNCTION_ARGS);
extern Datum temporal_at_periodset(PG_FUNCTION_ARGS);
extern Datum temporal_minus_periodset(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_timestamp(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_timestampset(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_period(PG_FUNCTION_ARGS);
extern Datum temporal_intersects_periodset(PG_FUNCTION_ARGS);
 
extern Temporal *temporal_at_min_internal(Temporal *temp);
extern TemporalInst *temporal_at_timestamp_internal(Temporal *temp, TimestampTz t);
extern void temporal_timespan_internal(Period *p, Temporal *temp);
extern char *temporal_to_string(Temporal *temp, char *(*value_out)(Oid, Datum));
extern void temporal_bbox(void *box, const Temporal *temp);

/* Comparison functions */

extern Datum temporal_lt(PG_FUNCTION_ARGS);
extern Datum temporal_le(PG_FUNCTION_ARGS);
extern Datum temporal_eq(PG_FUNCTION_ARGS);
extern Datum temporal_ge(PG_FUNCTION_ARGS);
extern Datum temporal_gt(PG_FUNCTION_ARGS);
extern Datum temporal_cmp(PG_FUNCTION_ARGS);
extern Datum temporal_hash(PG_FUNCTION_ARGS);

extern uint32 temporal_hash_internal(const Temporal *temp);

/*****************************************************************************/

#endif
