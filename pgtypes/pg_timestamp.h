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

#ifndef PG_TIMESTAMP_H
#define PG_TIMESTAMP_H

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 DateADT;
typedef int64 TimeADT;
typedef int64 Timestamp;
typedef int64 TimestampTz;

typedef float float4;
typedef double float8;

struct NumericData;
typedef struct NumericData *Numeric;

/*****************************************************************************/

/* Functions for timestamps */

extern Timestamp add_timestamp_interval(Timestamp ts, const Interval *interv);
extern Timestamp add_timestamptz_interval(TimestampTz tstz, const Interval *interv);
extern Timestamp add_timestamptz_interval_at_zone(TimestampTz tstz, const Interval *interv, const text *zone);
extern int32 cmp_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern int32 cmp_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern int32 cmp_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool eq_timestamp_date(Timestamp ts, DateADT date);
extern bool eq_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool eq_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool eq_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool eq_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool eq_timestamptz_timestamptz(TimestampTz tstz1, TimestampTz tstz2);
extern TimestampTz float8_to_timestamptz(float8 seconds);
extern bool gt_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool gt_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool gt_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool ge_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool ge_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool ge_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool le_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool le_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool le_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern bool lt_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool lt_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool lt_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern Timestamp minus_timestamp_interval(Timestamp ts, const Interval *interv);
extern Interval *minus_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern TimestampTz minus_timestamptz_interval(TimestampTz tstz, const Interval *interv);
extern TimestampTz minus_timestamptz_interval_at_zone(TimestampTz tstz, const Interval *interv, const text *zone);
extern Interval *minus_timestamptz_timestamptz(TimestampTz tstz1, TimestampTz tstz2);
extern bool ne_timestamp_date(Timestamp ts, DateADT date);
extern bool ne_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool ne_timestamp_timestamp(Timestamp ts1, Timestamp ts2);
extern bool ne_timestamp_timestamptz(Timestamp ts, TimestampTz tstz);
extern bool ne_timestamptz_timestamp(TimestampTz tstz, Timestamp ts);
extern Interval *timestamp_age(Timestamp ts1, Timestamp ts2);
extern TimestampTz timestamp_at_local(Timestamp ts);
extern Timestamp timestamp_bin(Timestamp ts, const Interval *stride, Timestamp origin);
extern Numeric timestamp_extract(Timestamp ts, const text *units);
extern uint32 timestamp_hash(Timestamp ts);
extern uint64 timestamp_hash_extended(TimestampTz tstz, uint64 seed);
extern Timestamp timestamp_in(const char *str, int32 typmod);
extern bool timestamp_is_finite(Timestamp ts);
extern TimestampTz timestamp_izone(Timestamp ts, const Interval *zone);
extern text *time_of_day(void);
extern Timestamp timestamp_larger(Timestamp ts1, Timestamp ts2);
extern Timestamp timestamp_make(int32 year, int32 month, int32 mday, int32 hour, int32 min, float8 sec);
extern char *timestamp_out(Timestamp ts);
extern bool timestamp_overlaps(Timestamp ts1, Timestamp te1, Timestamp ts2, Timestamp te2);
extern float8 timestamp_part(Timestamp ts, const text *units);
extern Timestamp timestamp_scale(Timestamp ts, int32 typmod);
extern Timestamp timestamp_smaller(Timestamp ts1, Timestamp ts2);
extern TimestampTz timestamp_to_timestamptz(Timestamp ts);
extern Timestamp timestamp_trunc(Timestamp ts, const text *units);
extern TimestampTz timestamp_zone(Timestamp ts, const text *zone);
extern Interval *timestamptz_age(TimestampTz tstz1, TimestampTz tstz2);
extern TimestampTz timestamptz_at_local(TimestampTz tstz);
extern TimestampTz timestamptz_bin(TimestampTz tstz, const Interval *stride, TimestampTz origin);
extern Numeric timestamptz_extract(TimestampTz tstz, const text *units);
extern int32 timestamptz_hash(TimestampTz tstz);
extern uint64 timestamptz_hash_extended(TimestampTz tstz, uint64 seed);
extern TimestampTz timestamptz_in(const char *str, int32 typmod);
extern Timestamp timestamptz_izone(TimestampTz tstz, const Interval *zone);
extern TimestampTz timestamptz_make(int32 year, int32 month, int32 day, int32 hour, int32 min, float8 sec);
extern TimestampTz timestamptz_make_at_timezone(int32 year, int32 month, int32 day, int32 hour, int32 min, float8 sec, const text *zone);
extern char *timestamptz_out(TimestampTz tstz);
extern bool timestamptz_overlaps(TimestampTz ts1, TimestampTz te1, TimestampTz ts2, TimestampTz te2);
extern float8 timestamptz_part(TimestampTz tstz, const text *units);
extern TimestampTz timestamptz_scale(TimestampTz tstz, int32 typmod);
extern TimestampTz timestamptz_shift(TimestampTz tstz, const Interval *interv);
extern Timestamp timestamptz_to_timestamp(TimestampTz tstz);
extern TimestampTz timestamptz_trunc(TimestampTz tstz, const text *units);
extern TimestampTz timestamptz_trunc_zone(TimestampTz tstz, const text *units, const text *zone);
extern Timestamp timestamptz_zone(TimestampTz tstz, const text *zone);


/*****************************************************************************/

#endif /* PG_TIMESTAMP_H */
