/*****************************************************************************
 *
 * TimeOps.h
 *	  Operators for time types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIMEOPS_H__
#define __TIMEOPS_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "TimeTypes.h"

/*****************************************************************************/

/* Miscellaneous */

extern Datum timestampset_to_period(PG_FUNCTION_ARGS);
extern Datum periodset_to_period(PG_FUNCTION_ARGS);

extern void time_type_oid(Oid timetypid);

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

extern bool contains_timestampset_timestamp_internal(TimestampSet *ts, TimestampTz t);
extern bool contains_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool contains_period_timestamp_internal(Period *p, TimestampTz t);
extern bool contains_period_timestampset_internal(Period *p, TimestampSet *ts);
extern bool contains_period_period_internal(Period *p1, Period *p2);
extern bool contains_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern bool contains_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern bool contains_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool contains_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool contains_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

/* overlaps? */

extern Datum overlaps_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum overlaps_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_period_timestamp(PG_FUNCTION_ARGS);
extern Datum overlaps_period_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_period_period(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_timestamp(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_timestampset(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_period(PG_FUNCTION_ARGS);
extern Datum overlaps_period_periodset(PG_FUNCTION_ARGS);
extern Datum overlaps_periodset_periodset(PG_FUNCTION_ARGS);

extern bool overlaps_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool overlaps_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool overlaps_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool overlaps_period_period_internal(Period *p1, Period *p2);
extern bool overlaps_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool overlaps_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern bool before_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern bool before_timestamp_period_internal(TimestampTz t, Period *p);
extern bool before_timestamp_periodset_internal(TimestampTz t, PeriodSet *ps);
extern bool before_timestampset_timestamp_internal(TimestampSet *ts, TimestampTz t);
extern bool before_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool before_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool before_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool before_period_timestamp_internal(Period *p, TimestampTz t);
extern bool before_period_period_internal(Period *p1, Period *p2);
extern bool before_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern bool before_period_timestampset_internal(Period *p, TimestampSet *ts);
extern bool before_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern bool before_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool before_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool before_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern bool after_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern bool after_timestamp_period_internal(TimestampTz t, Period *p);
extern bool after_timestamp_periodset_internal(TimestampTz t, PeriodSet *ps);
extern bool after_timestampset_timestamp_internal(TimestampSet *ts, TimestampTz t);
extern bool after_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool after_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool after_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool after_period_timestamp_internal(Period *p, TimestampTz t);
extern bool after_period_period_internal(Period *p1, Period *p2);
extern bool after_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern bool after_period_timestampset_internal(Period *p, TimestampSet *ts);
extern bool after_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern bool after_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool after_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool after_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern bool overbefore_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern bool overbefore_timestamp_period_internal(TimestampTz t, Period *p);
extern bool overbefore_timestamp_periodset_internal(TimestampTz t, PeriodSet *ps);
extern bool overbefore_timestampset_timestamp_internal(TimestampSet *ts, TimestampTz t);
extern bool overbefore_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool overbefore_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool overbefore_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool overbefore_period_timestamp_internal(Period *p, TimestampTz t);
extern bool overbefore_period_period_internal(Period *p1, Period *p2);
extern bool overbefore_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern bool overbefore_period_timestampset_internal(Period *p, TimestampSet *ts);
extern bool overbefore_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern bool overbefore_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool overbefore_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool overbefore_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern bool overafter_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern bool overafter_timestamp_period_internal(TimestampTz t, Period *p);
extern bool overafter_timestamp_periodset_internal(TimestampTz t, PeriodSet *ps);
extern bool overafter_timestampset_timestamp_internal(TimestampSet *ts, TimestampTz t);
extern bool overafter_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool overafter_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool overafter_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool overafter_period_timestamp_internal(Period *p, TimestampTz t);
extern bool overafter_period_period_internal(Period *p1, Period *p2);
extern bool overafter_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern bool overafter_period_timestampset_internal(Period *p, TimestampSet *ts);
extern bool overafter_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern bool overafter_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool overafter_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool overafter_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern bool adjacent_timestamp_period_internal(TimestampTz t, Period *p);
extern bool adjacent_timestamp_periodset_internal(TimestampTz t, PeriodSet *ps);
extern bool adjacent_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool adjacent_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool adjacent_period_timestamp_internal(Period *p, TimestampTz t);
extern bool adjacent_period_timestampset_internal(Period *p, TimestampSet *ts);
extern bool adjacent_period_period_internal(Period *p1, Period *p2);
extern bool adjacent_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool adjacent_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern bool adjacent_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern bool adjacent_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool adjacent_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern TimestampSet *union_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern TimestampSet *union_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern PeriodSet *union_period_period_internal(Period *p1, Period *p2);
extern PeriodSet *union_period_periodset_internal(Period *p, PeriodSet *ps);
extern PeriodSet *union_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern TimestampSet *intersection_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern TimestampSet *intersection_timestampset_period_internal(TimestampSet *ts, Period *p);
extern Period *intersection_period_period_internal(Period *p1, Period *p2);
extern PeriodSet *intersection_period_periodset_internal(Period *p, PeriodSet *ps);
extern PeriodSet *intersection_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern TimestampSet *minus_timestampset_timestamp_internal(TimestampSet *ts, TimestampTz t);
extern TimestampSet *minus_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern TimestampSet *minus_timestampset_period_internal(TimestampSet *ts, Period *p);
extern TimestampSet *minus_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern PeriodSet *minus_period_timestamp_internal(Period *p, TimestampTz t);
extern PeriodSet *minus_period_timestampset_internal(Period *p, TimestampSet *ts);
extern PeriodSet *minus_period_period_internal(Period *p1, Period *p2);
extern PeriodSet *minus_period_periodset_internal(Period *p, PeriodSet *ps);
extern PeriodSet *minus_periodset_timestamp_internal(PeriodSet *ps, TimestampTz t);
extern PeriodSet *minus_periodset_timestampset_internal(PeriodSet *ps, TimestampSet *ts);
extern PeriodSet *minus_periodset_period_internal(PeriodSet *ps, Period *p);
extern PeriodSet *minus_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

#endif

/*****************************************************************************/