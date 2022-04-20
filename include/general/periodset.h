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
#include <libpq/pqformat.h>
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

extern char *periodset_to_string(const PeriodSet *ps);
extern void periodset_write(const PeriodSet *ps, StringInfo buf);
extern PeriodSet *periodset_read(StringInfo buf);

/* Constructor function */

/* Cast functions */

extern PeriodSet *timestamp_periodset(TimestampTz t);
extern PeriodSet *timestampset_periodset(const TimestampSet *ts);
extern PeriodSet *period_periodset(const Period *p);
extern void periodset_period(const PeriodSet *ps, Period *p);

/* Accessor functions */

extern int periodset_mem_size(const PeriodSet *ps);
extern Interval *periodset_timespan(const PeriodSet *ps);
extern Interval *periodset_duration(const PeriodSet *ps);
extern int periodset_num_periods(const PeriodSet *ps);
extern Period *periodset_start_period(const PeriodSet *ps);
extern Period *periodset_end_period(const PeriodSet *ps);
extern Period *periodset_period_n(const PeriodSet *ps, int i);
extern const Period **periodset_periods(const PeriodSet *ps);
extern int periodset_num_timestamps(const PeriodSet *ps);
extern TimestampTz periodset_start_timestamp(const PeriodSet *ps);
extern TimestampTz periodset_end_timestamp(const PeriodSet *ps);
extern bool periodset_timestamp_n(const PeriodSet *ps, int n,
  TimestampTz *result);
TimestampTz *periodset_timestamps(const PeriodSet *ps, int *count);

/* Modification functions */

extern PeriodSet *periodset_shift_tscale(const PeriodSet *ps,
  const Interval *start, const Interval *duration);

/* Comparison functions */

extern int periodset_cmp(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_eq(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_ne(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_lt(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_le(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_gt(const PeriodSet *ps1, const PeriodSet *ps2);
extern bool periodset_ge(const PeriodSet *ps1, const PeriodSet *ps2);

/* Hash functions */

extern uint32 periodset_hash(const PeriodSet *ps);
extern uint64 periodset_hash_extended(const PeriodSet *ps, Datum seed);

#endif

/*****************************************************************************/
