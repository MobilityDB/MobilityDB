/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/timetypes.h"

/*****************************************************************************/

/* assorted support functions */

extern uint32_t time_max_header_size(void);
extern TimestampTz timestampset_time_n(const TimestampSet *ts, int index);
extern const Period *timestampset_bbox_ptr(const TimestampSet *ts);
extern void timestampset_bbox(const TimestampSet *ts, Period *p);
extern void timestampset_bbox_slice(Datum tsdatum, Period *p);
extern TimestampSet *timestampset_make(const TimestampTz *times, int count);
extern TimestampSet *timestampset_make_free(TimestampTz *times, int count);
extern TimestampSet *timestampset_copy(const TimestampSet *ts);
extern bool timestampset_find_timestamp(const TimestampSet *ts, TimestampTz t, int *loc);

/* Input/output functions */

extern Datum Timestampset_in(PG_FUNCTION_ARGS);
extern Datum Timestampset_out(PG_FUNCTION_ARGS);
extern Datum Timestampset_send(PG_FUNCTION_ARGS);
extern Datum Timestampset_recv(PG_FUNCTION_ARGS);

extern char *timestampset_to_string(const TimestampSet *ts);

/* Constructor function */

extern Datum Timestampset_constructor(PG_FUNCTION_ARGS);

/* Cast function */

extern Datum Timestamp_to_timestampset(PG_FUNCTION_ARGS);
extern Datum Timestampset_to_period(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum Timestampset_mem_size(PG_FUNCTION_ARGS);
extern Datum Timestampset_timespan(PG_FUNCTION_ARGS);
extern Datum Timestampset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum Timestampset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum Timestampset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum Timestampset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum Timestampset_timestamps(PG_FUNCTION_ARGS);

extern void timestampset_period(const TimestampSet *ts, Period *p);
extern TimestampTz *timestampset_timestamps(const TimestampSet *ts);

/* Modification functions */

extern Datum Timestampset_shift(PG_FUNCTION_ARGS);
extern Datum Timestampset_tscale(PG_FUNCTION_ARGS);
extern Datum Timestampset_shift_tscale(PG_FUNCTION_ARGS);

extern TimestampSet *timestampset_shift_tscale(const TimestampSet *ts,
  const Interval *start, const Interval *duration);

/* Comparison functions */

extern Datum Timestampset_cmp(PG_FUNCTION_ARGS);
extern Datum Timestampset_eq(PG_FUNCTION_ARGS);
extern Datum Timestampset_ne(PG_FUNCTION_ARGS);
extern Datum Timestampset_lt(PG_FUNCTION_ARGS);
extern Datum Timestampset_le(PG_FUNCTION_ARGS);
extern Datum Timestampset_ge(PG_FUNCTION_ARGS);
extern Datum Timestampset_gt(PG_FUNCTION_ARGS);

extern int timestampset_cmp(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_eq(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_ne(const TimestampSet *ts1, const TimestampSet *ts2);

/* Comparison functions */

extern Datum Timestampset_hash(PG_FUNCTION_ARGS);
extern Datum Timestampset_hash_extended(PG_FUNCTION_ARGS);

#endif

/*****************************************************************************/
