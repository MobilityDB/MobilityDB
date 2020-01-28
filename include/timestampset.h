/*****************************************************************************
 *
 * timestampset.h
 *	  Basic functions for set of timestamps.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIMESTAMPSET_H__
#define __TIMESTAMPSET_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

/* assorted support functions */

extern TimestampTz timestampset_time_n(TimestampSet *ts, int index);
extern Period *timestampset_bbox(TimestampSet *ts);
extern TimestampSet *timestampset_from_timestamparr_internal(TimestampTz *times, int count);
extern TimestampSet *timestampset_copy(TimestampSet *ts);
extern bool timestampset_find_timestamp(TimestampSet *ts, TimestampTz t, int *pos);

/* Input/output functions */

extern Datum timestampset_in(PG_FUNCTION_ARGS);
extern Datum timestampset_out(PG_FUNCTION_ARGS);
extern Datum timestampset_send(PG_FUNCTION_ARGS);
extern Datum timestampset_recv(PG_FUNCTION_ARGS);

extern char *timestampset_to_string(TimestampSet *ts);

/* Constructor function */

extern Datum timestampset_from_timestamparr(PG_FUNCTION_ARGS);

/* Cast function */

extern Datum timestamp_to_timestampset(PG_FUNCTION_ARGS);
extern Datum timestampset_to_period(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum timestampset_mem_size(PG_FUNCTION_ARGS);
extern Datum timestampset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum timestampset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum timestampset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum timestampset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum timestampset_timestamps(PG_FUNCTION_ARGS);
extern Datum timestampset_shift(PG_FUNCTION_ARGS);

extern void timestampset_to_period_internal(Period *p, TimestampSet *ts);
extern TimestampTz *timestampset_timestamps_internal(TimestampSet *ts);
extern TimestampSet *timestampset_shift_internal(TimestampSet *ts, Interval *interval);

/* Functions for defining B-tree index */

extern Datum timestampset_cmp(PG_FUNCTION_ARGS);
extern Datum timestampset_eq(PG_FUNCTION_ARGS);
extern Datum timestampset_ne(PG_FUNCTION_ARGS);
extern Datum timestampset_lt(PG_FUNCTION_ARGS);
extern Datum timestampset_le(PG_FUNCTION_ARGS);
extern Datum timestampset_ge(PG_FUNCTION_ARGS);
extern Datum timestampset_gt(PG_FUNCTION_ARGS);

extern int timestampset_cmp_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool timestampset_eq_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool timestampset_ne_internal(TimestampSet *ts1, TimestampSet *ts2);

#endif

/*****************************************************************************/