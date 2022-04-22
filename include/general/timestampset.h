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
#include <libpq/pqformat.h>
/* MobilityDB */
#include "general/timetypes.h"

/*****************************************************************************/

/* assorted support functions */

extern TimestampTz timestampset_time_n(const TimestampSet *ts, int index);
extern TimestampSet *timestampset_make(const TimestampTz *times, int count);
extern TimestampSet *timestampset_make_free(TimestampTz *times, int count);
extern TimestampSet *timestampset_copy(const TimestampSet *ts);

extern const Period *timestampset_bbox_ptr(const TimestampSet *ts);
extern void timestampset_bbox(const TimestampSet *ts, Period *p);
extern void timestampset_bbox_slice(Datum tsdatum, Period *p);
extern bool timestampset_find_timestamp(const TimestampSet *ts, TimestampTz t,
  int *loc);

/* Input/output functions */

extern char *timestampset_to_string(const TimestampSet *ts);
extern void timestampset_write(const TimestampSet *ts, StringInfo buf);
extern TimestampSet *timestampset_read(StringInfo buf);

/* Constructor function */


/* Cast function */

extern TimestampSet *timestamp_timestampset(TimestampTz t);
extern void timestampset_period(const TimestampSet *ts, Period *p);

/* Accessor functions */

extern int timestampset_mem_size(const TimestampSet *ts);
extern Interval *timestampset_timespan(const TimestampSet *ts);
extern int timestampset_num_timestamps(const TimestampSet *ts);
extern TimestampTz timestampset_start_timestamp(const TimestampSet *ts);
extern TimestampTz timestampset_end_timestamp(const TimestampSet *ts);
extern bool timestampset_timestamp_n(const TimestampSet *ts, int n,
  TimestampTz *result);
extern TimestampTz *timestampset_timestamps(const TimestampSet *ts);

/* Modification functions */

extern TimestampSet *timestampset_shift_tscale(const TimestampSet *ts,
  const Interval *start, const Interval *duration);

/* Comparison functions */

extern int timestampset_cmp(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_eq(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_ne(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_lt(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_le(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_gt(const TimestampSet *ts1, const TimestampSet *ts2);
extern bool timestampset_ge(const TimestampSet *ts1, const TimestampSet *ts2);

/* Hash functions */

extern uint32 timestampset_hash(const TimestampSet *ts);
extern uint64 timestampset_hash_extended(const TimestampSet *ts, Datum seed);

/*****************************************************************************/

#endif
