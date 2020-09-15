/*****************************************************************************
 *
 * period.h
 *    Basic routines for timestamptz periods
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __PERIOD_H__
#define __PERIOD_H__

#include <postgres.h>
#include <lib/stringinfo.h>
#include <catalog/pg_type.h>

#include "timetypes.h"

/*****************************************************************************/

/* Input/output functions */

extern Datum period_in(PG_FUNCTION_ARGS);
extern Datum period_out(PG_FUNCTION_ARGS);
extern Datum period_recv(PG_FUNCTION_ARGS);
extern Datum period_send(PG_FUNCTION_ARGS);

void period_send_internal(const Period *p, StringInfo buf);
Period *period_recv_internal(StringInfo buf);

char *period_to_string(const Period *p);

/* Constructors */
extern Datum period_constructor2(PG_FUNCTION_ARGS);
extern Datum period_constructor4(PG_FUNCTION_ARGS);

/* Casting */
extern Datum timestamp_to_period(PG_FUNCTION_ARGS);
extern Datum period_to_tstzrange(PG_FUNCTION_ARGS);
extern Datum tstzrange_to_period(PG_FUNCTION_ARGS);

/* period -> timestamptz */
extern Datum period_lower(PG_FUNCTION_ARGS);
extern Datum period_upper(PG_FUNCTION_ARGS);

/* period -> bool */
extern Datum period_lower_inc(PG_FUNCTION_ARGS);
extern Datum period_upper_inc(PG_FUNCTION_ARGS);

/* period -> period */
extern Datum period_shift(PG_FUNCTION_ARGS);

extern Period *period_shift_internal(const Period *p,
  const Interval *start);
extern void period_shift_tscale(Period *result, const Interval *start,
  const Interval *duration);

/* period -> interval */

extern Datum period_timespan(PG_FUNCTION_ARGS);

/* Functions for defining B-tree index */

extern Datum period_eq(PG_FUNCTION_ARGS);
extern Datum period_ne(PG_FUNCTION_ARGS);
extern Datum period_cmp(PG_FUNCTION_ARGS);
extern Datum period_lt(PG_FUNCTION_ARGS);
extern Datum period_le(PG_FUNCTION_ARGS);
extern Datum period_ge(PG_FUNCTION_ARGS);
extern Datum period_gt(PG_FUNCTION_ARGS);

extern bool period_eq_internal(const Period *p1, const Period *p2);
extern bool period_ne_internal(const Period *p1, const Period *p2);
extern int period_cmp_internal(const Period *p1, const Period *p2);
extern bool period_lt_internal(const Period *p1, const Period *p2);
extern bool period_le_internal(const Period *p1, const Period *p2);
extern bool period_ge_internal(const Period *p1, const Period *p2);
extern bool period_gt_internal(const Period *p1, const Period *p2);

/* Assorted support functions */

extern void period_deserialize(const Period *p, PeriodBound *lower, PeriodBound *upper);
extern int period_cmp_bounds(const PeriodBound *lower, const PeriodBound *upper);
extern Period *period_make(TimestampTz lower, TimestampTz upper,
  bool lower_inc, bool upper_inc);
extern void period_set(Period *p, TimestampTz lower, TimestampTz upper, 
  bool lower_inc, bool upper_inc);
extern Period *period_copy(const Period *p);
extern float8 period_to_secs(TimestampTz t1, TimestampTz t2);
extern Interval *period_timespan_internal(const Period *p);
extern Period **periodarr_normalize(Period **periods, int count, int *newcount);
extern Period *period_super_union(const Period *p1, const Period *p2);
extern void period_expand(Period *p1, const Period *p2);

extern int period_bound_qsort_cmp(const void *a1, const void *a2);

#endif

/*****************************************************************************/
