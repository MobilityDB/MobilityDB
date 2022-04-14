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
 * @file periodset.c
 * Basic functions for set of disjoint periods.
 */

#ifndef __PERIODSET_H__
#define __PERIODSET_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/timetypes.h"

/*****************************************************************************/

/* General functions */

extern const Period *periodset_per_n(const PeriodSet *ps, int index);
extern const Period *periodset_bbox_ptr(const PeriodSet *ps);
extern void periodset_bbox(const PeriodSet *ps, Period *p);
extern void periodset_bbox_slice(Datum psdatum, Period *p);
extern PeriodSet *periodset_make(const Period **periods, int count,
  bool normalize);
extern PeriodSet *periodset_make_free(Period **periods, int count,
  bool normalize);
extern PeriodSet *periodset_copy(const PeriodSet *ps);
extern bool periodset_find_timestamp(const PeriodSet *ps, TimestampTz t,
  int *loc);

/* Input/output functions */

extern Datum Periodset_in(PG_FUNCTION_ARGS);
extern Datum Periodset_out(PG_FUNCTION_ARGS);
extern Datum Periodset_send(PG_FUNCTION_ARGS);
extern Datum Periodset_recv(PG_FUNCTION_ARGS);

extern char *periodset_to_string(const PeriodSet *ps);

/* Constructor function */

extern Datum Periodset_constructor(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum Timestamp_to_periodset(PG_FUNCTION_ARGS);
extern Datum Timestampset_to_periodset(PG_FUNCTION_ARGS);
extern Datum Period_to_periodset(PG_FUNCTION_ARGS);
extern Datum Periodset_to_period(PG_FUNCTION_ARGS);

extern PeriodSet *timestamp_periodset(TimestampTz t);
extern PeriodSet *timestampset_periodset(const TimestampSet *ts);
extern PeriodSet *period_periodset(const Period *p);
extern void periodset_period(const PeriodSet *ps, Period *p);

/* Accessor functions */

extern Datum Periodset_mem_size(PG_FUNCTION_ARGS);
extern Datum Periodset_timespan(PG_FUNCTION_ARGS);
extern Datum Periodset_duration(PG_FUNCTION_ARGS);
extern Datum Periodset_num_periods(PG_FUNCTION_ARGS);
extern Datum Periodset_start_period(PG_FUNCTION_ARGS);
extern Datum Periodset_end_period(PG_FUNCTION_ARGS);
extern Datum Periodset_period_n(PG_FUNCTION_ARGS);
extern Datum Periodset_periods(PG_FUNCTION_ARGS);
extern Datum Periodset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum Periodset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum Periodset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum Periodset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum Periodset_timestamps(PG_FUNCTION_ARGS);

extern const Period **periodset_periods(const PeriodSet *ps);
extern TimestampTz periodset_start_timestamp(const PeriodSet *ps);
extern TimestampTz periodset_end_timestamp(const PeriodSet *ps);

/* Modification functions */

extern Datum Periodset_shift(PG_FUNCTION_ARGS);

extern PeriodSet *periodset_shift(const PeriodSet *ps, const Interval *interval);

/* Comparison functions */

extern Datum Periodset_cmp(PG_FUNCTION_ARGS);
extern Datum Periodset_eq(PG_FUNCTION_ARGS);
extern Datum Periodset_ne(PG_FUNCTION_ARGS);
extern Datum Periodset_lt(PG_FUNCTION_ARGS);
extern Datum Periodset_le(PG_FUNCTION_ARGS);
extern Datum Periodset_ge(PG_FUNCTION_ARGS);
extern Datum Periodset_gt(PG_FUNCTION_ARGS);

extern int periodset_cmp(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_eq(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_ne(const PeriodSet *ps1, const PeriodSet *ps2);

/* Hash functions */

extern Datum Periodset_hash(PG_FUNCTION_ARGS);
extern Datum Periodset_hash_extended(PG_FUNCTION_ARGS);

#endif

/*****************************************************************************/
