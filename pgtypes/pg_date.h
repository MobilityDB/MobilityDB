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

#ifndef __PG_DATE_H__
#define __PG_DATE_H__

typedef int32_t int32;
typedef int64_t int64;
typedef int32 DateADT;
typedef int64 TimeADT;

struct NumericData;
typedef struct NumericData *Numeric;

/*****************************************************************************/

/* Functions for dates */

extern DateADT add_date_int(DateADT date, int32 days);
extern DateADT add_date_interval(DateADT date, Interval *interv);
extern int32 cmp_date_timestamp(DateADT date, Timestamp ts);
extern int cmp_date_date(DateADT date1, DateADT date2);
extern int32 cmp_date_timestamptz(DateADT date, TimestampTz tstz);
extern int32 cmp_timestamp_date(Timestamp ts, DateADT date);
extern int32 cmp_timestamptz_date(TimestampTz tstz, DateADT date);
extern Numeric date_extract(DateADT date, const text *units);
extern uint32 date_hash(DateADT date);
extern uint64 date_hash_extended(DateADT date, int64 seed);
extern DateADT date_in(const char *str);
extern bool date_is_finite(DateADT date);
extern DateADT date_larger(DateADT date1, DateADT date2);
extern DateADT date_make(int year, int mon, int mday);
extern char *date_out(DateADT date);
extern DateADT date_smaller(DateADT date1, DateADT date2);
extern Timestamp date_time_to_timestamp(DateADT date, TimeADT time);
extern Timestamp date_to_timestamp(DateADT date);
extern TimestampTz date_to_timestamptz(DateADT date);
extern bool eq_date_date(DateADT date1, DateADT date2);
extern bool eq_date_timestamp(DateADT date, Timestamp ts);
extern bool eq_date_timestamptz(DateADT date, TimestampTz tstz);
extern bool eq_timestamp_date(Timestamp ts, DateADT date);
extern bool eq_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool ge_date_date(DateADT date1, DateADT date2);
extern bool ge_date_timestamp(DateADT date, Timestamp ts);
extern bool ge_date_timestamptz(DateADT date, TimestampTz tstz);
extern bool ge_timestamp_date(Timestamp ts, DateADT date);
extern bool ge_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool gt_date_date(DateADT date1, DateADT date2);
extern bool gt_date_timestamp(DateADT date, Timestamp ts);
extern bool gt_date_timestamptz(DateADT date, TimestampTz tstz);
extern bool gt_timestamp_date(Timestamp ts, DateADT date);
extern bool gt_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool le_date_date(DateADT date1, DateADT date2);
extern bool le_date_timestamp(DateADT date, Timestamp ts);
extern bool le_date_timestamptz(DateADT date, TimestampTz tstz);
extern bool le_timestamp_date(Timestamp ts, DateADT date);
extern bool le_timestamptz_date(TimestampTz tstz, DateADT date);
extern bool lt_date_date(DateADT date1, DateADT date2);
extern bool lt_date_timestamp(DateADT date, Timestamp ts);
extern bool lt_date_timestamptz(DateADT date, TimestampTz tstz);
extern bool lt_timestamp_date(Timestamp ts, DateADT date);
extern bool lt_timestamptz_date(TimestampTz tstz, DateADT date);
extern int32 minus_date_date(DateADT date1, DateADT date2);
extern DateADT minus_date_int(DateADT date, int32 days);
extern DateADT minus_date_interval(DateADT date, Interval *span);
extern bool ne_date_date(DateADT date1, DateADT date2);
extern bool ne_date_timestamp(DateADT date, Timestamp ts);
extern bool ne_date_timestamptz(DateADT date, TimestampTz tstz);
extern bool ne_timestamp_date(Timestamp ts, DateADT date);
extern bool ne_timestamptz_date(TimestampTz tstz, DateADT date);
extern DateADT timestamp_to_date(Timestamp ts);
extern DateADT timestamptz_to_date(TimestampTz tstz);

/*****************************************************************************/

#endif /* __PG_DATE_H__ */
