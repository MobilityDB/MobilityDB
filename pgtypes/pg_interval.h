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

#ifndef PG_INTERVAL_H
#define PG_INTERVAL_H

typedef float float4;
typedef double float8;

struct NumericData;
typedef struct NumericData *Numeric;

/*****************************************************************************/

/* Functions for intervals */

extern Interval *add_interval_interval(const Interval *interv1, const Interval *interv2);
extern Interval *div_interval_float8(const Interval *interv, float8 factor);
extern int32 interval_cmp(const Interval *interv1, const Interval *interv2);
extern Interval *interval_copy(const Interval *interv);
extern bool interval_eq(const Interval *interv1, const Interval *interv2);
extern Numeric interval_extract(const Interval *interv, const text *units);
extern bool interval_ge(const Interval *interv1, const Interval *interv2);
extern bool interval_gt(const Interval *interv1, const Interval *interv2);
extern uint32 interval_hash(const Interval *interv);
extern uint64 interval_hash_extended(const Interval *interv, uint64 seed);
extern Interval *interval_in(const char *str, int32 typmod);
extern bool interval_is_finite(const Interval *interv);
extern Interval *interval_justify_days(const Interval *interv);
extern Interval *interval_justify_hours(const Interval *interv);
extern Interval *interval_justify_interval(const Interval *interv);
extern Interval *interval_larger(const Interval *interv1, const Interval *interv2);
extern bool interval_le(const Interval *interv1, const Interval *interv2);
extern bool interval_lt(const Interval *interv1, const Interval *interv2);
extern Interval *interval_make(int32 years, int32 months, int32 weeks, int32 days, int32 hours, int32 mins, float8 secs);
extern bool interval_ne(const Interval *interv1, const Interval *interv2);
extern Interval *interval_negate(const Interval *interv);
extern char *interval_out(const Interval *interv);
extern float8 interval_part(const Interval *interv, const text *units);
extern Interval *interval_scale(const Interval *interv, int32 typmod);
extern Interval *interval_smaller(const Interval *interv1, const Interval *interv2);
extern Interval *interval_trunc(const Interval *interv, const text *units);
extern Interval *minus_interval_interval(const Interval *interv1, const Interval *interv2);
extern Interval *mul_float8_interval(float8 factor, const Interval *interv);
extern Interval *mul_interval_float8(const Interval *interv, float8 factor);

/*****************************************************************************/

#endif /* PG_INTERVAL_H */
