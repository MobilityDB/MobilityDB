/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @file
 * @brief Functions for base and time types corresponding to the external (SQL)
 * PostgreSQL functions
 */

#ifndef __PG_TIME_H__
#define __PG_TIME_H__

typedef int32_t int32;
typedef int64_t int64;
typedef double float8;
typedef int32 DateADT;
typedef int64 TimeADT;

// typedef struct
// {
  // TimeADT time;    /* all time units other than months and years */
  // int32 zone;      /* numeric time zone, in seconds */
// } TimeTzADT;

struct NumericData;
typedef struct NumericData *Numeric;

/*****************************************************************************/

/* Functions for time */

// extern TimestampTz date_timetz_to_timestamptz(DateADT date, const TimeTzADT *timetz);
extern TimeADT interval_to_time(const Interval *interv);
extern TimeADT minus_time_interval(TimeADT time, const Interval *interv);
extern Interval *minus_time_time(TimeADT time1, TimeADT time2);
// extern TimeTzADT *minus_timetz_interval(const TimeTzADT *timetz, const Interval *interv);
extern TimeADT plus_time_interval(TimeADT time, Interval *interv);
// extern TimeTzADT *plus_timetz_interval(const TimeTzADT *timetz, const Interval *interv);
extern int time_cmp(TimeADT time1, TimeADT time2);
extern bool time_eq(TimeADT time1, TimeADT time2);
extern Numeric time_extract(TimeADT time, const text *units);
extern bool time_ge(TimeADT time1, TimeADT time2);
extern bool time_gt(TimeADT time1, TimeADT time2);
extern uint32 time_hash(TimeADT time);
extern uint64 time_hash_extended(TimeADT time, int32 seed);
extern TimeADT time_in(const char *str, int32 typmod);
extern TimeADT time_larger(TimeADT time1, TimeADT time2);
extern bool time_le(TimeADT time1, TimeADT time2);
extern bool time_lt(TimeADT time1, TimeADT time2);
extern TimeADT time_make(int tm_hour, int tm_min, double sec);
extern bool time_ne(TimeADT time1, TimeADT time2);
extern char *time_out(TimeADT time);
extern bool time_overlaps(TimeADT ts1, TimeADT te1, TimeADT ts2, TimeADT te2);
extern float8 time_part(TimeADT time, const text *units);
extern TimeADT time_scale(TimeADT date, int32 typmod);
extern TimeADT time_smaller(TimeADT time1, TimeADT time2);
extern Interval *time_to_interval(TimeADT time);
// extern TimeTzADT *time_to_timetz(TimeADT time);
extern TimeADT timestamp_to_time(Timestamp ts);
extern TimeADT timestamptz_to_time(TimestampTz tztz);
// extern TimeTzADT *timestamptz_to_timetz(TimestampTz tztz);
// extern TimeTzADT *timetz_at_local(const TimeTzADT *timetz);
// extern int32 timetz_cmp(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern bool timetz_eq(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern Numeric timetz_extract(const TimeTzADT *timetz, const text *units);
// extern bool timetz_ge(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern bool timetz_gt(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern uint32 timetz_hash(const TimeTzADT *timetz);
// extern uint64 timetz_hash_extended(const TimeTzADT *timetz, int64 seed);
// extern TimeTzADT *timetz_in(const char *str, int32 typmod);
// extern TimeTzADT *timetz_izone(const TimeTzADT *timetz, const Interval *zone);
// extern TimeTzADT *timetz_larger(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern bool timetz_le(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern bool timetz_lt(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern bool timetz_ne(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern TimeTzADT *timetz_copy(const TimeTzADT *timetz);
// extern char *timetz_out(const TimeTzADT *timetz);
// extern bool timetz_overlaps(const TimeTzADT *ts1, const TimeTzADT *te1, const TimeTzADT *ts2, const TimeTzADT *te2);
// extern float8 timetz_part(const TimeTzADT *timetz, const text *units);
// extern TimeTzADT *timetz_scale(const TimeTzADT *timetz, int32 typmod);
// extern TimeTzADT *timetz_smaller(const TimeTzADT *timetz1, const TimeTzADT *timetz2);
// extern TimeADT timetz_to_time(const TimeTzADT *timetz);
// extern TimeTzADT *timetz_zone(const TimeTzADT *timetz, const text *zone);

/*****************************************************************************/

#endif /* __PG_TIME_H__ */
