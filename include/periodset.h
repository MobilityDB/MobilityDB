/*****************************************************************************
 *
 * periodset.c
 *	Basic functions for set of periods.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __PERIODSET_H__
#define __PERIODSET_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

/* Assorted support functions */

extern Period *periodset_per_n(PeriodSet *ps, int index);
extern Period *periodset_bbox(PeriodSet *ps);
extern PeriodSet *periodset_from_periodarr_internal(Period **periods, 
	int count, bool normalize);
extern PeriodSet *periodset_copy(PeriodSet *ps);
extern bool periodset_find_timestamp(PeriodSet *ps, TimestampTz t, int *pos);

/* Input/output functions */

extern Datum periodset_in(PG_FUNCTION_ARGS);
extern Datum periodset_send(PG_FUNCTION_ARGS);
extern Datum periodset_recv(PG_FUNCTION_ARGS);
extern Datum periodset_send(PG_FUNCTION_ARGS);

extern char *periodset_to_string(PeriodSet *ps);

/* Constructor function */

extern Datum periodset_from_periodarr(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum timestamp_as_periodset(PG_FUNCTION_ARGS);
extern Datum timestampset_as_periodset(PG_FUNCTION_ARGS);
extern Datum period_as_periodset(PG_FUNCTION_ARGS);

extern PeriodSet *timestampset_as_periodset_internal(TimestampSet *ts);

/* Accessor functions */

extern Datum periodset_mem_size(PG_FUNCTION_ARGS);
extern Datum periodset_timespan(PG_FUNCTION_ARGS);
extern Datum periodset_duration(PG_FUNCTION_ARGS);
extern Datum periodset_num_periods(PG_FUNCTION_ARGS);
extern Datum periodset_start_period(PG_FUNCTION_ARGS);
extern Datum periodset_end_period(PG_FUNCTION_ARGS);
extern Datum periodset_period_n(PG_FUNCTION_ARGS);
extern Datum periodset_periods(PG_FUNCTION_ARGS);
extern Datum periodset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum periodset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum periodset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum periodset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum periodset_timestamps(PG_FUNCTION_ARGS);
extern Datum periodset_shift(PG_FUNCTION_ARGS);

extern void periodset_timespan_internal(Period *p, PeriodSet *ps);
extern Period **periodset_periods_internal(PeriodSet *ps);
extern TimestampTz periodset_start_timestamp_internal(PeriodSet *ps);
extern TimestampTz periodset_end_timestamp_internal(PeriodSet *ps);
extern PeriodSet *periodset_shift_internal(PeriodSet *ps, Interval *interval);

/* Functions for defining B-tree index */

extern Datum periodset_cmp(PG_FUNCTION_ARGS);
extern Datum periodset_eq(PG_FUNCTION_ARGS);
extern Datum periodset_ne(PG_FUNCTION_ARGS);
extern Datum periodset_lt(PG_FUNCTION_ARGS);
extern Datum periodset_le(PG_FUNCTION_ARGS);
extern Datum periodset_ge(PG_FUNCTION_ARGS);
extern Datum periodset_gt(PG_FUNCTION_ARGS);

extern int periodset_cmp_internal(PeriodSet *ps1, PeriodSet *ps2);
extern bool periodset_eq_internal(PeriodSet *ps1, PeriodSet *ps2);
extern bool periodset_ne_internal(PeriodSet *ps1, PeriodSet *ps2);

#endif

/*****************************************************************************/