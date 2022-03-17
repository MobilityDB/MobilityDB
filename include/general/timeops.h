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
 * @file timeops.h
 * Operators for time types.
 */

#ifndef __TIMEOPS_H__
#define __TIMEOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

/**
 * Enumeration for the relative position of a given element into a skiplist
 */
typedef enum
{
  BEFORE,
  DURING,
  AFTER
} RelativeTimePos;

/* Miscellaneous */

extern bool time_type(Oid timetypid);
extern void ensure_time_type(Oid timetypid);

/* Functions for aggregations */

extern RelativeTimePos pos_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern RelativeTimePos pos_period_timestamp(const Period *p, TimestampTz t);

/* contains? */

extern Datum contains_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum contains_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum contains_period_timestamp(PG_FUNCTION_ARGS);
extern Datum contains_period_timestampset(PG_FUNCTION_ARGS);
extern Datum contains_period_period(PG_FUNCTION_ARGS);
extern Datum contains_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum contains_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum contains_periodset_period(PG_FUNCTION_ARGS);
extern Datum contains_period_periodset(PG_FUNCTION_ARGS);
extern Datum contains_periodset_periodset(PG_FUNCTION_ARGS);

extern bool contains_timestampset_timestamp_internal(const TimestampSet *ts,
  TimestampTz t);
extern bool contains_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool contains_period_timestamp_internal(const Period *p,
  TimestampTz t);
extern bool contains_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern bool contains_period_period_internal(const Period *p1,
  const Period *p2);
extern bool contains_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern bool contains_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool contains_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern bool contains_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool contains_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* contained? */

extern Datum contained_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum contained_timestamp_period(PG_FUNCTION_ARGS);
extern Datum contained_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum contained_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum contained_timestampset_period(PG_FUNCTION_ARGS);
extern Datum contained_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum contained_period_period(PG_FUNCTION_ARGS);
extern Datum contained_period_periodset(PG_FUNCTION_ARGS);
extern Datum contained_periodset_period(PG_FUNCTION_ARGS);
extern Datum contained_periodset_periodset(PG_FUNCTION_ARGS);

extern bool contained_period_period_internal(const Period *p1,
  const Period *p2);

/* overlaps? */

extern Datum overlaps_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_period_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_period_period(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_period(PG_FUNCTION_ARGS);
extern Datum overlaps_period_periodset(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_periodset(PG_FUNCTION_ARGS);

extern bool overlaps_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overlaps_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern bool overlaps_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overlaps_period_period_internal(const Period *p1,
  const Period *p2);
extern bool overlaps_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool overlaps_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* before */

extern Datum before_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum before_timestamp_period(PG_FUNCTION_ARGS);
extern Datum before_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum before_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum before_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum before_timestampset_period(PG_FUNCTION_ARGS);
extern Datum before_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum before_period_timestamp(PG_FUNCTION_ARGS);
extern Datum before_period_period(PG_FUNCTION_ARGS);
extern Datum before_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum before_period_timestampset(PG_FUNCTION_ARGS);
extern Datum before_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum before_periodset_period(PG_FUNCTION_ARGS);
extern Datum before_period_periodset(PG_FUNCTION_ARGS);
extern Datum before_periodset_periodset(PG_FUNCTION_ARGS);

extern bool before_timestamp_timestampset_internal(TimestampTz t,
  const TimestampSet *ts);
extern bool before_timestamp_period_internal(TimestampTz t, const Period *p);
extern bool before_timestamp_periodset_internal(TimestampTz t,
  const PeriodSet *ps);
extern bool before_timestampset_timestamp_internal(const TimestampSet *ts,
  TimestampTz t);
extern bool before_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool before_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern bool before_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool before_period_timestamp_internal(const Period *p, TimestampTz t);
extern bool before_period_period_internal(const Period *p1, const Period *p2);
extern bool before_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool before_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern bool before_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern bool before_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern bool before_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool before_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* after */

extern Datum after_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum after_timestamp_period(PG_FUNCTION_ARGS);
extern Datum after_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum after_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum after_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum after_timestampset_period(PG_FUNCTION_ARGS);
extern Datum after_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum after_period_timestamp(PG_FUNCTION_ARGS);
extern Datum after_period_period(PG_FUNCTION_ARGS);
extern Datum after_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum after_period_timestampset(PG_FUNCTION_ARGS);
extern Datum after_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum after_periodset_period(PG_FUNCTION_ARGS);
extern Datum after_period_periodset(PG_FUNCTION_ARGS);
extern Datum after_periodset_periodset(PG_FUNCTION_ARGS);

extern bool after_timestamp_timestampset_internal(TimestampTz t,
  const TimestampSet *ts);
extern bool after_timestamp_period_internal(TimestampTz t, const Period *p);
extern bool after_timestamp_periodset_internal(TimestampTz t,
  const PeriodSet *ps);
extern bool after_timestampset_timestamp_internal(const TimestampSet *ts,
  TimestampTz t);
extern bool after_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool after_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern bool after_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool after_period_timestamp_internal(const Period *p, TimestampTz t);
extern bool after_period_period_internal(const Period *p1, const Period *p2);
extern bool after_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool after_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern bool after_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern bool after_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern bool after_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool after_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* overbefore */

extern Datum overbefore_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum overbefore_timestamp_period(PG_FUNCTION_ARGS);
extern Datum overbefore_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum overbefore_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum overbefore_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum overbefore_timestampset_period(PG_FUNCTION_ARGS);
extern Datum overbefore_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum overbefore_period_timestamp(PG_FUNCTION_ARGS);
extern Datum overbefore_period_period(PG_FUNCTION_ARGS);
extern Datum overbefore_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum overbefore_period_timestampset(PG_FUNCTION_ARGS);
extern Datum overbefore_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum overbefore_periodset_period(PG_FUNCTION_ARGS);
extern Datum overbefore_period_periodset(PG_FUNCTION_ARGS);
extern Datum overbefore_periodset_periodset(PG_FUNCTION_ARGS);

extern bool overbefore_timestamp_timestampset_internal(TimestampTz t,
  const TimestampSet *ts);
extern bool overbefore_timestamp_period_internal(TimestampTz t,
  const Period *p);
extern bool overbefore_timestamp_periodset_internal(TimestampTz t,
  const PeriodSet *ps);
extern bool overbefore_timestampset_timestamp_internal(const TimestampSet *ts,
  TimestampTz t);
extern bool overbefore_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overbefore_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern bool overbefore_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overbefore_period_timestamp_internal(const Period *p,
  TimestampTz t);
extern bool overbefore_period_period_internal(const Period *p1,
  const Period *p2);
extern bool overbefore_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool overbefore_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern bool overbefore_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern bool overbefore_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern bool overbefore_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool overbefore_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* overafter */

extern Datum overafter_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum overafter_timestamp_period(PG_FUNCTION_ARGS);
extern Datum overafter_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum overafter_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum overafter_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum overafter_timestampset_period(PG_FUNCTION_ARGS);
extern Datum overafter_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum overafter_period_timestamp(PG_FUNCTION_ARGS);
extern Datum overafter_period_period(PG_FUNCTION_ARGS);
extern Datum overafter_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum overafter_period_timestampset(PG_FUNCTION_ARGS);
extern Datum overafter_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum overafter_periodset_period(PG_FUNCTION_ARGS);
extern Datum overafter_period_periodset(PG_FUNCTION_ARGS);
extern Datum overafter_periodset_periodset(PG_FUNCTION_ARGS);

extern bool overafter_timestamp_timestampset_internal(TimestampTz t,
  const TimestampSet *ts);
extern bool overafter_timestamp_period_internal(TimestampTz t,
  const Period *p);
extern bool overafter_timestamp_periodset_internal(TimestampTz t,
  const PeriodSet *ps);
extern bool overafter_timestampset_timestamp_internal(const TimestampSet *ts,
  TimestampTz t);
extern bool overafter_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overafter_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern bool overafter_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overafter_period_timestamp_internal(const Period *p,
  TimestampTz t);
extern bool overafter_period_period_internal(const Period *p1,
  const Period *p2);
extern bool overafter_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool overafter_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern bool overafter_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern bool overafter_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern bool overafter_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool overafter_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* adjacent */

extern Datum adjacent_timestamp_period(PG_FUNCTION_ARGS);
extern Datum adjacent_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum adjacent_timestampset_period(PG_FUNCTION_ARGS);
extern Datum adjacent_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum adjacent_period_timestamp(PG_FUNCTION_ARGS);
extern Datum adjacent_period_timestampset(PG_FUNCTION_ARGS);
extern Datum adjacent_period_period(PG_FUNCTION_ARGS);
extern Datum adjacent_period_periodset(PG_FUNCTION_ARGS);
extern Datum adjacent_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum adjacent_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum adjacent_periodset_period(PG_FUNCTION_ARGS);
extern Datum adjacent_periodset_periodset(PG_FUNCTION_ARGS);

extern bool adjacent_timestamp_period_internal(TimestampTz t,
  const Period *p);
extern bool adjacent_timestamp_periodset_internal(TimestampTz t,
  const PeriodSet *ps);
extern bool adjacent_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern bool adjacent_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool adjacent_period_timestamp_internal(const Period *p,
  TimestampTz t);
extern bool adjacent_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern bool adjacent_period_period_internal(const Period *p1,
  const Period *p2);
extern bool adjacent_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern bool adjacent_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern bool adjacent_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool adjacent_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern bool adjacent_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* union */

extern Datum union_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum union_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum union_timestamp_period(PG_FUNCTION_ARGS);
extern Datum union_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum union_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum union_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum union_timestampset_period(PG_FUNCTION_ARGS);
extern Datum union_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum union_period_timestamp(PG_FUNCTION_ARGS);
extern Datum union_period_timestampset(PG_FUNCTION_ARGS);
extern Datum union_period_period(PG_FUNCTION_ARGS);
extern Datum union_period_periodset(PG_FUNCTION_ARGS);
extern Datum union_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum union_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum union_periodset_period(PG_FUNCTION_ARGS);
extern Datum union_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *union_timestamp_timestampset_internal(TimestampTz t,
  const TimestampSet *ts);
extern TimestampSet *union_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern PeriodSet *union_period_period_internal(const Period *p1,
  const Period *p2);
extern PeriodSet *union_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern PeriodSet *union_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* intersection */

extern Datum intersection_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum intersection_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum intersection_timestamp_period(PG_FUNCTION_ARGS);
extern Datum intersection_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum intersection_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum intersection_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum intersection_timestampset_period(PG_FUNCTION_ARGS);
extern Datum intersection_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum intersection_period_timestamp(PG_FUNCTION_ARGS);
extern Datum intersection_period_timestampset(PG_FUNCTION_ARGS);
extern Datum intersection_period_period(PG_FUNCTION_ARGS);
extern Datum intersection_period_periodset(PG_FUNCTION_ARGS);
extern Datum intersection_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum intersection_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum intersection_periodset_period(PG_FUNCTION_ARGS);
extern Datum intersection_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *intersection_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern TimestampSet *intersection_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern Period *intersection_period_period_internal(const Period *p1,
  const Period *p2);
extern bool inter_period_period(const Period *p1, const Period *p2, Period *result);
extern PeriodSet *intersection_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern PeriodSet *intersection_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* minus */

extern Datum minus_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum minus_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum minus_timestamp_period(PG_FUNCTION_ARGS);
extern Datum minus_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum minus_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum minus_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum minus_timestampset_period(PG_FUNCTION_ARGS);
extern Datum minus_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum minus_period_timestamp(PG_FUNCTION_ARGS);
extern Datum minus_period_timestampset(PG_FUNCTION_ARGS);
extern Datum minus_period_period(PG_FUNCTION_ARGS);
extern Datum minus_period_periodset(PG_FUNCTION_ARGS);
extern Datum minus_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum minus_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum minus_periodset_period(PG_FUNCTION_ARGS);
extern Datum minus_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *minus_timestampset_timestamp_internal(const TimestampSet *ts,
  TimestampTz t);
extern TimestampSet *minus_timestampset_timestampset_internal(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern TimestampSet *minus_timestampset_period_internal(const TimestampSet *ts,
  const Period *p);
extern TimestampSet *minus_timestampset_periodset_internal(const TimestampSet *ts,
  const PeriodSet *ps);
extern PeriodSet *minus_period_timestamp_internal(const Period *p, TimestampTz t);
extern PeriodSet *minus_period_timestampset_internal(const Period *p,
  const TimestampSet *ts);
extern PeriodSet *minus_period_period_internal(const Period *p1,
  const Period *p2);
extern PeriodSet *minus_period_periodset_internal(const Period *p,
  const PeriodSet *ps);
extern PeriodSet *minus_periodset_timestamp_internal(const PeriodSet *ps,
  TimestampTz t);
extern PeriodSet *minus_periodset_timestampset_internal(const PeriodSet *ps,
  const TimestampSet *ts);
extern PeriodSet *minus_periodset_period_internal(const PeriodSet *ps,
  const Period *p);
extern PeriodSet *minus_periodset_periodset_internal(const PeriodSet *ps1,
  const PeriodSet *ps2);

#endif

/*****************************************************************************/
