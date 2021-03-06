/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written 
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO 
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/**
 * @file timestampset.h
 * Basic functions for set of (distinct) timestamps.
 */

#ifndef __TIMESTAMPSET_H__
#define __TIMESTAMPSET_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

/* assorted support functions */

extern TimestampTz timestampset_time_n(const TimestampSet *ts, int index);
extern Period *timestampset_bbox(const TimestampSet *ts);
extern TimestampSet *timestampset_make(const TimestampTz *times, int count);
extern TimestampSet *timestampset_make_free(TimestampTz *times, int count);
extern TimestampSet *timestampset_copy(const TimestampSet *ts);
extern bool timestampset_find_timestamp(const TimestampSet *ts, TimestampTz t, int *loc);

/* Input/output functions */

extern Datum timestampset_in(PG_FUNCTION_ARGS);
extern Datum timestampset_out(PG_FUNCTION_ARGS);
extern Datum timestampset_send(PG_FUNCTION_ARGS);
extern Datum timestampset_recv(PG_FUNCTION_ARGS);

extern char *timestampset_to_string(const TimestampSet *ts);

/* Constructor function */

extern Datum timestampset_constructor(PG_FUNCTION_ARGS);

/* Cast function */

extern Datum timestamp_to_timestampset(PG_FUNCTION_ARGS);
extern Datum timestampset_to_period(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum timestampset_mem_size(PG_FUNCTION_ARGS);
extern Datum timestampset_timespan(PG_FUNCTION_ARGS);
extern Datum timestampset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum timestampset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum timestampset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum timestampset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum timestampset_timestamps(PG_FUNCTION_ARGS);
extern Datum timestampset_shift(PG_FUNCTION_ARGS);

extern void timestampset_to_period_internal(Period *p, const TimestampSet *ts);
extern TimestampTz *timestampset_timestamps_internal(const TimestampSet *ts);
extern TimestampSet *timestampset_shift_internal(const TimestampSet *ts, const Interval *interval);

/* Functions for defining B-tree index */

extern Datum timestampset_cmp(PG_FUNCTION_ARGS);
extern Datum timestampset_eq(PG_FUNCTION_ARGS);
extern Datum timestampset_ne(PG_FUNCTION_ARGS);
extern Datum timestampset_lt(PG_FUNCTION_ARGS);
extern Datum timestampset_le(PG_FUNCTION_ARGS);
extern Datum timestampset_ge(PG_FUNCTION_ARGS);
extern Datum timestampset_gt(PG_FUNCTION_ARGS);

extern int timestampset_cmp_internal(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_eq_internal(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_ne_internal(const TimestampSet *ts1, const TimestampSet *ts2);

#endif

/*****************************************************************************/
