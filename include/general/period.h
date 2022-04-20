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
 * @file period.h
 * Basic routines for time periods composed of two `TimestampTz` values and
 * two Boolean values stating whether the bounds are inclusive or not.
 */

#ifndef __PERIOD_H__
#define __PERIOD_H__

/* PostgreSQL */
#include <postgres.h>
#include <lib/stringinfo.h>
#include <catalog/pg_type.h>
/* MobilityDB */
#include "general/timetypes.h"

/*****************************************************************************/

/* General functions */

extern void period_deserialize(const Period *p, PeriodBound *lower,
  PeriodBound *upper);
extern int period_bound_cmp(const PeriodBound *b1, const PeriodBound *b2);
extern int period_bound_qsort_cmp(const void *a1, const void *a2);
extern int period_lower_cmp(const Period *a, const Period *b);
extern int period_upper_cmp(const Period *a, const Period *b);
extern Period *period_make(TimestampTz lower, TimestampTz upper,
  bool lower_inc, bool upper_inc);
  extern void period_set(TimestampTz lower, TimestampTz upper, bool lower_inc,
  bool upper_inc, Period *p);
extern Period *period_copy(const Period *p);
extern float8 period_to_secs(TimestampTz t1, TimestampTz t2);
extern Period **periodarr_normalize(Period **periods, int count, int *newcount);
extern Period *period_super_union(const Period *p1, const Period *p2);
extern void period_expand(const Period *p2, Period *p1);

/* Input/output functions */

extern char *period_to_string(const Period *p);
extern void period_write(const Period *p, StringInfo buf);
extern Period *period_read(StringInfo buf);

/* Constructors */


/* Casting */

extern Period *timestamp_period(TimestampTz t);

/* Accessor functions */

extern TimestampTz period_lower(Period *p);
extern TimestampTz period_upper(Period *p);
extern bool period_lower_inc(Period *p);
extern bool period_upper_inc(Period *p);
extern Interval *period_duration(const Period *p);

/* Modification functions */

extern void period_shift_tscale(const Interval *start,
  const Interval *duration, Period *result);

/* Comparison functions */

extern bool period_eq(const Period *p1, const Period *p2);
extern bool period_ne(const Period *p1, const Period *p2);
extern int period_cmp(const Period *p1, const Period *p2);
extern bool period_lt(const Period *p1, const Period *p2);
extern bool period_le(const Period *p1, const Period *p2);
extern bool period_ge(const Period *p1, const Period *p2);
extern bool period_gt(const Period *p1, const Period *p2);

/* Hash functions */

extern uint32 period_hash(const Period *p);
extern uint64 period_hash_extended(const Period *p, Datum seed);

/*****************************************************************************/

#endif
