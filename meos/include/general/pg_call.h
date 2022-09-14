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
 * @brief MobilityDB functions `pg_func(...)` corresponding to external
 * PostgreSQL functions `func(PG_FUNCTION_ARGS)`. This avoids bypassing the
 * function manager `fmgr.c`.
 */

#ifndef PG_CALL_H
#define PG_CALL_H

/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>

/* Functions adadpted from bool.c */

extern bool bool_in(const char *in_str);
extern char *bool_out(bool b);

/* Functions adapted from int.c */

extern int32 int4_in(const char *str);
extern char *int4_out(int32 val);

/* Functions adapted from int8.c */

extern int64 int8_in(const char *str);
extern char *int8_out(int64 val);

/* Functions adapted from float.c */

extern float8 float8_in(const char *num, const char *type_name,
  const char *orig_string);
extern char *float8_out(double num, int maxdd);
extern float8 pg_dsin(float8 arg1);
extern float8 pg_dcos(float8 arg1);
extern float8 pg_datan(float8 arg1);
extern float8 pg_datan2(float8 arg1, float8 arg2);

/* Functions adadpted from timestamp.c */

extern TimestampTz pg_timestamptz_in(const char *str, int32 typmod);
extern char *pg_timestamptz_out(TimestampTz dt);

extern Interval *pg_interval_pl(const Interval *span1, const Interval *span2);
extern TimestampTz pg_timestamp_pl_interval(TimestampTz timestamp,
  const Interval *span);
extern TimestampTz pg_timestamp_mi_interval(TimestampTz timestamp,
  const Interval *span);
extern Interval *pg_interval_justify_hours(const Interval *span);
extern Interval *pg_timestamp_mi(TimestampTz dt1, TimestampTz dt2);
extern int pg_interval_cmp(const Interval *interval1,
  const Interval *interval2);

/* Functions adapted from hashfn.h and hashfn.c */

extern uint32 pg_hashint8(int64 val);
extern uint32 pg_hashfloat8(float8 key);
extern uint64 pg_hashint8extended(int64 val, uint64 seed);
extern uint64 pg_hashfloat8extended(float8 key, uint64 seed);
extern uint32 pg_hashtext(text *key);

/*****************************************************************************/

#endif /* PG_CALL_H */
