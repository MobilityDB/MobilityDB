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

/* Miscellaneous */

extern bool time_type(CachedType timetype);
extern void ensure_time_type(CachedType timetype);

/* Functions for aggregations */

extern RelativeTimePos pos_timestamp_timestamp(TimestampTz t1, TimestampTz t2);
extern RelativeTimePos pos_period_timestamp(const Period *p, TimestampTz t);

/* contains? */

extern Datum Contains_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Contains_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Contains_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Contains_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Contains_period_period(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_period(PG_FUNCTION_ARGS);
extern Datum Contains_period_periodset(PG_FUNCTION_ARGS);
extern Datum Contains_periodset_periodset(PG_FUNCTION_ARGS);

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

extern Datum Contained_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Contained_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Contained_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Contained_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Contained_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Contained_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Contained_period_period(PG_FUNCTION_ARGS);
extern Datum Contained_period_periodset(PG_FUNCTION_ARGS);
extern Datum Contained_periodset_period(PG_FUNCTION_ARGS);
extern Datum Contained_periodset_periodset(PG_FUNCTION_ARGS);

extern bool contained_period_period(const Period *p1,
  const Period *p2);

/* overlaps? */

extern Datum Overlaps_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overlaps_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Overlaps_period_period(PG_FUNCTION_ARGS);
extern Datum Overlaps_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overlaps_periodset_period(PG_FUNCTION_ARGS);
extern Datum Overlaps_period_periodset(PG_FUNCTION_ARGS);
extern Datum Overlaps_periodset_periodset(PG_FUNCTION_ARGS);

extern bool overlaps_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool overlaps_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool overlaps_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool overlaps_period_period(const Period *p1,
  const Period *p2);
extern bool overlaps_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool overlaps_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* before */

extern Datum Before_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Before_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Before_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Before_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Before_period_period(PG_FUNCTION_ARGS);
extern Datum Before_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Before_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Before_periodset_period(PG_FUNCTION_ARGS);
extern Datum Before_period_periodset(PG_FUNCTION_ARGS);
extern Datum Before_periodset_periodset(PG_FUNCTION_ARGS);

extern bool before_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool before_timestamp_period(TimestampTz t, const Period *p);
extern bool before_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);
extern bool before_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool before_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool before_timestampset_period(const TimestampSet *ts,
  const Period *p);
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
extern bool before_periodset_period(const PeriodSet *ps,
  const Period *p);
extern bool before_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool before_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* after */

extern Datum After_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum After_timestamp_period(PG_FUNCTION_ARGS);
extern Datum After_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum After_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum After_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum After_timestampset_period(PG_FUNCTION_ARGS);
extern Datum After_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum After_period_timestamp(PG_FUNCTION_ARGS);
extern Datum After_period_period(PG_FUNCTION_ARGS);
extern Datum After_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum After_period_timestampset(PG_FUNCTION_ARGS);
extern Datum After_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum After_periodset_period(PG_FUNCTION_ARGS);
extern Datum After_period_periodset(PG_FUNCTION_ARGS);
extern Datum After_periodset_periodset(PG_FUNCTION_ARGS);

extern bool after_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool after_timestamp_period(TimestampTz t, const Period *p);
extern bool after_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);
extern bool after_timestampset_timestamp(const TimestampSet *ts,
  TimestampTz t);
extern bool after_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern bool after_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool after_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool after_period_timestamp(const Period *p, TimestampTz t);
extern bool after_period_period(const Period *p1, const Period *p2);
extern bool after_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool after_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool after_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool after_periodset_period(const PeriodSet *ps,
  const Period *p);
extern bool after_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool after_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* overbefore */

extern Datum Overbefore_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_period(PG_FUNCTION_ARGS);
extern Datum Overbefore_period_periodset(PG_FUNCTION_ARGS);
extern Datum Overbefore_periodset_periodset(PG_FUNCTION_ARGS);

extern bool overbefore_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern bool overbefore_timestamp_period(TimestampTz t,
  const Period *p);
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
extern bool overbefore_period_timestamp(const Period *p,
  TimestampTz t);
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

extern Datum Overafter_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Overafter_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Overafter_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Overafter_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Overafter_period_period(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_period(PG_FUNCTION_ARGS);
extern Datum Overafter_period_periodset(PG_FUNCTION_ARGS);
extern Datum Overafter_periodset_periodset(PG_FUNCTION_ARGS);

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
extern bool overafter_period_timestamp(const Period *p,
  TimestampTz t);
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
extern bool overafter_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool overafter_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* adjacent */

extern Datum Adjacent_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Adjacent_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_period_periodset(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_period(PG_FUNCTION_ARGS);
extern Datum Adjacent_periodset_periodset(PG_FUNCTION_ARGS);

extern bool adjacent_timestamp_period(TimestampTz t,
  const Period *p);
extern bool adjacent_timestamp_periodset(TimestampTz t,
  const PeriodSet *ps);
extern bool adjacent_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern bool adjacent_timestampset_periodset(const TimestampSet *ts,
  const PeriodSet *ps);
extern bool adjacent_period_timestamp(const Period *p,
  TimestampTz t);
extern bool adjacent_period_timestampset(const Period *p,
  const TimestampSet *ts);
extern bool adjacent_period_period(const Period *p1,
  const Period *p2);
extern bool adjacent_period_periodset(const Period *p,
  const PeriodSet *ps);
extern bool adjacent_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern bool adjacent_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern bool adjacent_periodset_period(const PeriodSet *ps,
  const Period *p);
extern bool adjacent_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* union */

extern Datum Union_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Union_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Union_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Union_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_period_period(PG_FUNCTION_ARGS);
extern Datum Union_period_periodset(PG_FUNCTION_ARGS);
extern Datum Union_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Union_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Union_periodset_period(PG_FUNCTION_ARGS);
extern Datum Union_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *union_timestamp_timestampset(TimestampTz t,
  const TimestampSet *ts);
extern TimestampSet *union_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern PeriodSet *union_period_period(const Period *p1,
  const Period *p2);
extern PeriodSet *union_period_periodset(const Period *p,
  const PeriodSet *ps);
extern PeriodSet *union_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* intersection */

extern Datum Intersection_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Intersection_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Intersection_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Intersection_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_period_period(PG_FUNCTION_ARGS);
extern Datum Intersection_period_periodset(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_period(PG_FUNCTION_ARGS);
extern Datum Intersection_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *intersection_timestampset_timestampset(const TimestampSet *ts1,
  const TimestampSet *ts2);
extern TimestampSet *intersection_timestampset_period(const TimestampSet *ts,
  const Period *p);
extern Period *intersection_period_period(const Period *p1,
  const Period *p2);
extern bool inter_period_period(const Period *p1, const Period *p2, Period *result);
extern PeriodSet *intersection_period_periodset(const Period *p,
  const PeriodSet *ps);
extern PeriodSet *intersection_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* minus */

extern Datum Minus_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Minus_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Minus_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Minus_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_period_period(PG_FUNCTION_ARGS);
extern Datum Minus_period_periodset(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_period(PG_FUNCTION_ARGS);
extern Datum Minus_periodset_periodset(PG_FUNCTION_ARGS);

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
extern PeriodSet *minus_period_period(const Period *p1,
  const Period *p2);
extern PeriodSet *minus_period_periodset(const Period *p,
  const PeriodSet *ps);
extern PeriodSet *minus_periodset_timestamp(const PeriodSet *ps,
  TimestampTz t);
extern PeriodSet *minus_periodset_timestampset(const PeriodSet *ps,
  const TimestampSet *ts);
extern PeriodSet *minus_periodset_period(const PeriodSet *ps,
  const Period *p);
extern PeriodSet *minus_periodset_periodset(const PeriodSet *ps1,
  const PeriodSet *ps2);

/* Distance returning an Interval */

extern Datum Distance_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Distance_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Distance_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_period_period(PG_FUNCTION_ARGS);
extern Datum Distance_period_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_period(PG_FUNCTION_ARGS);
extern Datum Distance_periodset_periodset(PG_FUNCTION_ARGS);

extern Interval *distance_period_timestamp(const Period *p,
  TimestampTz t);
extern Interval *distance_period_period(const Period *p1,
  const Period *p2);

/* Distance returning a float in seconds for use with indexes in
 * nearest neighbor searches */

extern Datum Distance_secs_timestamp_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestamp_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestamp_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_timestampset_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_period_periodset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_period(PG_FUNCTION_ARGS);
extern Datum Distance_secs_periodset_periodset(PG_FUNCTION_ARGS);

extern double distance_secs_period_timestamp(const Period *p,
  TimestampTz t);
extern double distance_secs_period_period(const Period *p1,
  const Period *p2);

#endif

/*****************************************************************************/
