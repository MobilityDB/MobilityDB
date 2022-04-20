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
 * @file time_ops.h
 * Operators for time types.
 */

#ifndef __TIME_OPS_H__
#define __TIME_OPS_H__

/* PostgreSQL */
#include <postgres.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/tempcache.h"
#include "general/timetypes.h"

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

/*****************************************************************************/

/* Miscellaneous */

extern uint32_t time_max_header_size(void);
extern bool time_type(CachedType timetype);
extern void ensure_time_type(CachedType timetype);

/* Functions for aggregations */

extern RelativeTimePos pos_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern RelativeTimePos pos_period_timestamp(const Period *p, TimestampTz t);

/* contains? */

extern bool contains_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool contains_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool contains_period_timestamp(const Period *p,
  TimestampTz t);
extern bool contains_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool contains_period_period(const Period *p1,
  const Period *p2);
extern bool contains_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool contains_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool contains_periodset_period(const PeriodSet *ps,
  const Period *p);
extern bool contains_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool contains_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* contained? */

extern bool contained_period_period(const Period *p1, const Period *p2);

/* overlaps? */

extern bool overlaps_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overlaps_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool overlaps_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overlaps_period_period(const Period *p1, const Period *p2);
extern bool overlaps_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overlaps_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* before */

extern bool before_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool before_timestamp_period(TimestampTz t, const Period *p);
extern bool before_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool before_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool before_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool before_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool before_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool before_period_timestamp(const Period *p, TimestampTz t);
extern bool before_period_period(const Period *p1, const Period *p2);
extern bool before_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool before_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool before_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool before_periodset_period(const PeriodSet *ps, const Period *p);
extern bool before_period_periodset(const Period *p, const PeriodSet *ps);
extern bool before_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* after */

extern bool after_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool after_timestamp_period(TimestampTz t, const Period *p);
extern bool after_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);
extern bool after_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool after_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool after_timestampset_period(const TimestampSet *ts, const Period *p);
extern bool after_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool after_period_timestamp(const Period *p, TimestampTz t);
extern bool after_period_period(const Period *p1, const Period *p2);
extern bool after_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool after_period_timestampset(const Period *p, const TimestampSet *ts);
extern bool after_periodset_timestamp(const PeriodSet *ps, TimestampTz t);
extern bool after_periodset_period(const PeriodSet *ps, const Period *p);
extern bool after_period_periodset(const Period *p, const PeriodSet *ps);
extern bool after_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* overbefore */

extern bool overbefore_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool overbefore_timestamp_period(TimestampTz t, const Period *p);
extern bool overbefore_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);
extern bool overbefore_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool overbefore_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overbefore_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool overbefore_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overbefore_period_timestamp(const Period *p, TimestampTz t);
extern bool overbefore_period_period(const Period *p1,
  const Period *p2);
extern bool overbefore_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool overbefore_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool overbefore_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool overbefore_periodset_period(const PeriodSet *ps,
  const Period *p);
extern bool overbefore_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool overbefore_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* overafter */

extern bool overafter_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool overafter_timestamp_period(TimestampTz t,
  const Period *p);
extern bool overafter_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);
extern bool overafter_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool overafter_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overafter_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool overafter_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overafter_period_timestamp(const Period *p, TimestampTz t);
extern bool overafter_period_period(const Period *p1,
  const Period *p2);
extern bool overafter_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool overafter_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool overafter_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool overafter_periodset_period(const PeriodSet *ps,
  const Period *p);
extern bool overafter_period_periodset(const Period *p, const PeriodSet *ps);
extern bool overafter_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* adjacent */

extern bool adjacent_timestamp_period(TimestampTz t, const Period *p);
extern bool adjacent_timestamp_periodset(TimestampTz t, const PeriodSet *ps);
extern bool adjacent_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool adjacent_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool adjacent_period_timestamp(const Period *p, TimestampTz t);
extern bool adjacent_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool adjacent_period_period(const Period *p1, const Period *p2);
extern bool adjacent_period_periodset(const Period *p, const PeriodSet *ps);
extern bool adjacent_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool adjacent_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool adjacent_periodset_period(const PeriodSet *ps, const Period *p);
extern bool adjacent_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* union */

extern TimestampSet *union_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern TimestampSet *union_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern PeriodSet *union_timestamp_period(TimestampTz t, const Period *p);
extern PeriodSet *union_timestamp_periodset(TimestampTz t, const PeriodSet *ps);

extern TimestampSet *union_timestampset_timestamp(const TimestampSet *ts,
  const TimestampTz t);
extern TimestampSet *union_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern PeriodSet *union_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern PeriodSet *union_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);

extern PeriodSet *union_period_timestamp(const Period *p, TimestampTz t);
extern PeriodSet *union_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern PeriodSet *union_period_period(const Period *p1, const Period *p2);
extern PeriodSet *union_period_periodset(const Period *p, const PeriodSet *ps);

extern PeriodSet *union_periodset_timestamp(PeriodSet *ps, TimestampTz t);
extern PeriodSet *union_periodset_timestampset(PeriodSet *ps,
  TimestampSet *ts);
extern PeriodSet *union_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *union_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* intersection */

extern bool intersection_timestamp_timestamp(TimestampTz t1, TimestampTz t2,
  TimestampTz *result);
extern bool intersection_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts, TimestampTz *result);
extern bool intersection_timestamp_period(TimestampTz t, const Period *p,
  TimestampTz *result);
extern bool intersection_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps, TimestampTz *result);

extern bool intersection_timestampset_timestamp(const TimestampSet *ts,
  const TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_timestampset_timestampset(
  const TimestampSet *ts1, const TimestampSet *ts2);
extern TimestampSet *intersection_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern TimestampSet *intersection_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);

extern bool intersection_period_timestamp(const Period *p, TimestampTz t,
  TimestampTz *result);
extern TimestampSet *intersection_period_timestampset(const Period *ps,
  const TimestampSet *ts);
extern Period *intersection_period_period(const Period *p1,
  const Period *p2);
extern bool inter_period_period(const Period *p1, const Period *p2,
  Period *result);
extern PeriodSet *intersection_period_periodset(const Period *p,
  const PeriodSet *ps);

extern bool intersection_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t, TimestampTz *result);
extern TimestampSet *intersection_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern PeriodSet *intersection_periodset_period(const PeriodSet *ps,
  const Period *p);
extern PeriodSet *intersection_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* minus */

extern bool minus_timestamp_timestamp(TimestampTz t1, TimestampTz t2,
  TimestampTz *result);
extern bool minus_timestamp_timestampset(TimestampTz t, const TimestampSet *ts,
  TimestampTz *result);
extern bool minus_timestamp_period(TimestampTz t, const Period *p,
  TimestampTz *result);
extern bool minus_timestamp_periodset(TimestampTz t, const PeriodSet *ps,
  TimestampTz *result);

extern TimestampSet *minus_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern TimestampSet *minus_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern TimestampSet *minus_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern TimestampSet *minus_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);

extern PeriodSet *minus_period_timestamp(const Period *p, TimestampTz t);
extern PeriodSet *minus_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern PeriodSet *minus_period_period(const Period *p1, const Period *p2);
extern PeriodSet *minus_period_periodset(const Period *p,
  const PeriodSet *ps);

extern PeriodSet *minus_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern PeriodSet *minus_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern PeriodSet *minus_periodset_period(const PeriodSet *ps, const Period *p);
extern PeriodSet *minus_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* Distance returning an Interval */

extern Interval *distance_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern Interval *distance_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern Interval *distance_timestamp_period(TimestampTz t, const Period *p);
extern Interval *distance_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);

extern Interval *distance_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern Interval *distance_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern Interval *distance_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern Interval *distance_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);

extern Interval *distance_period_timestamp(const Period *p, TimestampTz t);
extern Interval *distance_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern Interval *distance_period_period(const Period *p1, const Period *p2);
extern Interval *distance_period_periodset(const Period *p,
  const PeriodSet *ps);

extern Interval *distance_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern Interval *distance_periodset_timestampset(const PeriodSet *ps,
  TimestampSet *ts);
extern Interval *distance_periodset_period(const PeriodSet *ps,
  const Period *p);
extern Interval *distance_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* Distance returning a float in seconds for use with indexes in
 * nearest neighbor searches */

extern double distance_secs_timestamp_timestamp(TimestampTz t1,
  TimestampTz t2);
extern double distance_secs_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern double distance_secs_timestamp_period(TimestampTz t, const Period *p);
extern double distance_secs_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);

extern double distance_secs_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern double distance_secs_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern double distance_secs_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern double distance_secs_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps)
;

extern double distance_secs_period_timestamp(const Period *p,
  TimestampTz t);
extern double distance_secs_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern double distance_secs_period_period(const Period *p1,
  const Period *p2);
extern double distance_secs_period_periodset(const Period *p,
  const PeriodSet *ps);

extern double distance_secs_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern double distance_secs_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern double distance_secs_periodset_period(const PeriodSet *ps,
  const Period *p);
extern double distance_secs_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/*****************************************************************************/

#endif
