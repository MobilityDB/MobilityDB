/*****************************************************************************
 *
 * TimeTypes.h
 * 		Functions for time types based on timestamptz, that is,
 *		timestampset, period, periodset
 *
 * The Period type is a specialized version of the RangeType in PostgreSQL. 
 * It is considerably more efficient, in particular because it is a  
 * fix-length type, it has finite bounds, and do not allow empty periods. 
 * The TimestampSet type represents a set of disjoint timestamptz.
 * The PeriodSet type represents a set of disjoint periods. 
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIMETYPES_H__
#define __TIMETYPES_H__

#include <postgres.h>
#include <datatype/timestamp.h>

typedef struct 
{
	TimestampTz	lower;			/* the lower bound value */
	TimestampTz	upper;			/* the upper bound value */
	char 		lower_inc;		/* the lower bound is inclusive (vs exclusive) */
	char 		upper_inc;		/* the upper bound is inclusive (vs exclusive) */
} Period;

/* Internal representation of either bound of a period (not what's on disk) */
typedef struct
{
	TimestampTz val;			/* the bound value */
	bool		inclusive;		/* bound is inclusive (vs exclusive) */
	bool		lower;			/* this is the lower (vs upper) bound */
} PeriodBound;

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int32		count;			/* number of Period elements */
    /* variable-length data follows */
} PeriodSet;

typedef struct 
{
	int32		vl_len_;		/* varlena header (do not touch directly!) */
	int32		count;			/* number of Period elements */
	double		padding1;		/* Test for solving index problems */
	double		padding2;		/* Test for solving index problems */
    /* variable-length data follows */
} TimestampSet;

/*
 * fmgr macros for period objects
 */

#define DatumGetTimestampSet(X)		((TimestampSet *) DatumGetPointer(X))
#define TimestampSetGetDatum(X)		PointerGetDatum(X)
#define PG_GETARG_TIMESTAMPSET(n)	DatumGetTimestampSet(PG_GETARG_POINTER(n))
#define PG_RETURN_TIMESTAMPSET(x)	PG_RETURN_POINTER(x)

#define DatumGetPeriod(X)			((Period *) DatumGetPointer(X))
#define PeriodGetDatum(X)			PointerGetDatum(X)
#define PG_GETARG_PERIOD(n)			DatumGetPeriod(PG_GETARG_POINTER(n))
#define PG_RETURN_PERIOD(x)			PG_RETURN_POINTER(x)

#define DatumGetPeriodSet(X)		((PeriodSet *) DatumGetPointer(X))
#define PeriodSetGetDatum(X)		PointerGetDatum(X)
#define PG_GETARG_PERIODSET(n)		DatumGetPeriodSet(PG_GETARG_POINTER(n))
#define PG_RETURN_PERIODSET(x)		PG_RETURN_POINTER(x)

/* Operator strategy numbers used in the GiST and SP-GiST period opclasses */
/* Numbers are chosen to match up operator names with existing usages */
#define TEMPORALSTRAT_CONTAINS			RTContainsStrategyNumber
#define TEMPORALSTRAT_CONTAINS_ELEM		RTContainsElemStrategyNumber
#define TEMPORALSTRAT_CONTAINED			RTContainedByStrategyNumber
#define TEMPORALSTRAT_OVERLAPS			RTOverlapStrategyNumber
#define TEMPORALSTRAT_SAME				RTSameStrategyNumber
#define TEMPORALSTRAT_EQ				RTEqualStrategyNumber
#define TEMPORALSTRAT_BEFORE			RTLeftStrategyNumber
#define TEMPORALSTRAT_OVERBEFORE		RTOverLeftStrategyNumber
#define TEMPORALSTRAT_AFTER				RTRightStrategyNumber
#define TEMPORALSTRAT_OVERAFTER			RTOverRightStrategyNumber

/*****************************************************************************
 * Prototypes for functions defined in Period.c
 *****************************************************************************/

/* Input/output functions */

extern Datum period_in(PG_FUNCTION_ARGS);
extern Datum period_out(PG_FUNCTION_ARGS);
extern Datum period_recv(PG_FUNCTION_ARGS);
extern Datum period_send(PG_FUNCTION_ARGS);

void period_send_internal(Period *period, StringInfo buf);
Period *period_recv_internal(StringInfo buf);

char *period_to_string(Period *period);

/* Constructors */
extern Datum period_constructor2(PG_FUNCTION_ARGS);
extern Datum period_constructor4(PG_FUNCTION_ARGS);

/* Casting */
extern Datum timestamp_as_period(PG_FUNCTION_ARGS);
extern Datum period_to_range(PG_FUNCTION_ARGS);
extern Datum range_to_period(PG_FUNCTION_ARGS);

/* period -> timestamptz */
extern Datum period_lower(PG_FUNCTION_ARGS);
extern Datum period_upper(PG_FUNCTION_ARGS);

/* period -> bool */
extern Datum period_lower_inc(PG_FUNCTION_ARGS);
extern Datum period_upper_inc(PG_FUNCTION_ARGS);

/* period -> period */
extern Datum period_shift(PG_FUNCTION_ARGS);

Period *period_shift_internal(Period *p, Interval *interval);

/* period -> interval */

extern Datum period_duration(PG_FUNCTION_ARGS);

/* Functions for defining B-tree index */

extern Datum period_eq(PG_FUNCTION_ARGS);
extern Datum period_ne(PG_FUNCTION_ARGS);
extern Datum period_cmp(PG_FUNCTION_ARGS);
extern Datum period_lt(PG_FUNCTION_ARGS);
extern Datum period_le(PG_FUNCTION_ARGS);
extern Datum period_ge(PG_FUNCTION_ARGS);
extern Datum period_gt(PG_FUNCTION_ARGS);

extern bool period_eq_internal(Period *p1, Period *p2);
extern bool period_ne_internal(Period *p1, Period *p2);
extern int period_cmp_internal(Period *p1, Period *p2);
extern bool period_lt_internal(Period *p1, Period *p2);
extern bool period_le_internal(Period *p1, Period *p2);
extern bool period_eq_internal(Period *p1, Period *p2);
extern bool period_ge_internal(Period *p1, Period *p2);
extern bool period_gt_internal(Period *p1, Period *p2);

/* Assorted support functions */

extern void period_deserialize(Period *period, PeriodBound *lower, PeriodBound *upper);
extern bool periodarr_find_timestamp(Period **array, int from, int count,
	TimestampTz t, int *pos, bool ignorebounds);
extern int period_cmp_bounds(TimestampTz t1, TimestampTz t2, bool lower1, 
	bool lower2, bool inclusive1, bool inclusive2);
extern bool period_bounds_adjacent(TimestampTz t1, TimestampTz t2, 
	bool inclusive1, bool inclusive2);
extern Period *period_make(TimestampTz lower, TimestampTz upper, 
	bool lower_inc, bool upper_inc);
extern void period_set(Period *period, TimestampTz lower, TimestampTz upper, 
	bool lower_inc, bool upper_inc);
extern Period *period_copy(Period *period);
extern float8 period_duration_secs(TimestampTz t1, TimestampTz t2);
extern double period_duration_time(Period *period);
extern Interval *period_duration_internal(Period *period);
extern Period **periodarr_normalize(Period **periods, int count, int *newcount);
	
/* Used in for GiST and SP-GiST */

int	period_cmp_lower(const void **a, const void **b);
int	period_cmp_upper(const void **a, const void **b);

/*****************************************************************************
 * Prototypes for functions defined in TimestampSet.c
 *****************************************************************************/

/* assorted support functions */

extern TimestampTz timestampset_time_n(TimestampSet *ts, int index);
extern Period *timestampset_bbox(TimestampSet *ts);
extern TimestampSet *timestampset_from_timestamparr_internal(TimestampTz *times, int count);
extern TimestampSet *timestampset_copy(TimestampSet *ts);
extern int timestampset_find_timestamp(TimestampSet *ts, TimestampTz t);

/* Input/output functions */

extern Datum timestampset_in(PG_FUNCTION_ARGS);
extern Datum timestampset_send(PG_FUNCTION_ARGS);
extern Datum timestampset_recv(PG_FUNCTION_ARGS);
extern Datum timestampset_send(PG_FUNCTION_ARGS);

extern char *timestampset_to_string(TimestampSet *ts);

/* Constructor function */

extern Datum timestampset_from_timestamparr(PG_FUNCTION_ARGS);

/* Cast function */

extern Datum timestamp_as_timestampset(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum timestampset_size(PG_FUNCTION_ARGS);
extern Datum timestampset_timespan(PG_FUNCTION_ARGS);
extern Datum timestampset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum timestampset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum timestampset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum timestampset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum timestampset_timestamps(PG_FUNCTION_ARGS);
extern Datum timestampset_shift(PG_FUNCTION_ARGS);

extern void timestampset_timespan_internal(Period *p, TimestampSet *ts);
extern TimestampTz timestampset_start_timestamp_internal(TimestampSet *ts);
extern TimestampTz timestampset_end_timestamp_internal(TimestampSet *ts);
extern TimestampTz *timestampset_timestamps_internal(TimestampSet *ts);
extern TimestampSet *timestampset_shift_internal(TimestampSet *ts, Interval *interval);

/* Functions for defining B-tree index */

extern Datum timestampset_cmp(PG_FUNCTION_ARGS);
extern Datum timestampset_eq(PG_FUNCTION_ARGS);
extern Datum timestampset_ne(PG_FUNCTION_ARGS);
extern Datum timestampset_lt(PG_FUNCTION_ARGS);
extern Datum timestampset_le(PG_FUNCTION_ARGS);
extern Datum timestampset_ge(PG_FUNCTION_ARGS);
extern Datum timestampset_gt(PG_FUNCTION_ARGS);

extern int timestampset_cmp_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool timestampset_eq_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool timestampset_ne_internal(TimestampSet *ts1, TimestampSet *ts2);

/*****************************************************************************
 * Prototypes for functions defined in PeriodSet.c
 *****************************************************************************/

/* Assorted support functions */

extern Period *periodset_per_n(PeriodSet *ps, int index);
extern Period *periodset_bbox(PeriodSet *ps);
extern PeriodSet *periodset_from_periodarr_internal(Period **periods, 
	int count, bool normalize);
extern PeriodSet *periodset_copy(PeriodSet *ps);
extern int periodset_find_timestamp(PeriodSet *ps, TimestampTz t);
extern double periodset_duration_time(PeriodSet *ps);

/* Input/output functions */

extern Datum periodset_in(PG_FUNCTION_ARGS);
extern Datum periodset_send(PG_FUNCTION_ARGS);
extern Datum periodset_recv(PG_FUNCTION_ARGS);
extern Datum periodset_send(PG_FUNCTION_ARGS);

extern char *periodset_to_string(PeriodSet *ps);

/* Constructor function */

extern Datum periodset_from_periodarr(PG_FUNCTION_ARGS);

/* Cast functions */

extern Datum timestamp_as_periodset(PG_FUNCTION_ARGS);
extern Datum period_as_periodset(PG_FUNCTION_ARGS);

/* Accessor functions */

extern Datum periodset_size(PG_FUNCTION_ARGS);
extern Datum periodset_timespan(PG_FUNCTION_ARGS);
extern Datum periodset_duration(PG_FUNCTION_ARGS);
extern Datum periodset_num_periods(PG_FUNCTION_ARGS);
extern Datum periodset_start_period(PG_FUNCTION_ARGS);
extern Datum periodset_end_period(PG_FUNCTION_ARGS);
extern Datum periodset_period_n(PG_FUNCTION_ARGS);
extern Datum periodset_periods(PG_FUNCTION_ARGS);
extern Datum periodset_num_timestamps(PG_FUNCTION_ARGS);
extern Datum periodset_start_timestamp(PG_FUNCTION_ARGS);
extern Datum periodset_end_timestamp(PG_FUNCTION_ARGS);
extern Datum periodset_timestamp_n(PG_FUNCTION_ARGS);
extern Datum periodset_timestamps(PG_FUNCTION_ARGS);
extern Datum periodset_shift(PG_FUNCTION_ARGS);

extern void periodset_timespan_internal(Period *p, PeriodSet *ps);
extern Period **periodset_periods_internal(PeriodSet *ps);
extern TimestampTz periodset_start_timestamp_internal(PeriodSet *ps);
extern TimestampTz periodset_end_timestamp_internal(PeriodSet *ps);
extern PeriodSet *periodset_shift_internal(PeriodSet *ps, Interval *interval);

/* Functions for defining B-tree index */

extern Datum periodset_cmp(PG_FUNCTION_ARGS);
extern Datum periodset_eq(PG_FUNCTION_ARGS);
extern Datum periodset_ne(PG_FUNCTION_ARGS);
extern Datum periodset_lt(PG_FUNCTION_ARGS);
extern Datum periodset_le(PG_FUNCTION_ARGS);
extern Datum periodset_ge(PG_FUNCTION_ARGS);
extern Datum periodset_gt(PG_FUNCTION_ARGS);

extern int periodset_cmp_internal(PeriodSet *ps1, PeriodSet *ps2);
extern bool periodset_eq_internal(PeriodSet *ps1, PeriodSet *ps2);
extern bool periodset_ne_internal(PeriodSet *ps1, PeriodSet *ps2);

/*****************************************************************************
 * Prototypes for functions defined in TimeTypesOps.c
 *****************************************************************************/

/* Function needed for GIST index */

extern Datum timestamp_to_period(PG_FUNCTION_ARGS);

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

extern bool contained_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern bool contained_timestamp_period_internal(TimestampTz t, Period *p);
extern bool contained_timestamp_periodset_internal(TimestampTz t, PeriodSet *ps);
extern bool contained_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern bool contained_timestampset_periodset_internal(TimestampSet *ts, PeriodSet *ps);
extern bool contained_timestampset_period_internal(TimestampSet *ts, Period *p);
extern bool contained_period_period_internal(Period *p1, Period *p2);
extern bool contained_period_periodset_internal(Period *p, PeriodSet *ps);
extern bool contained_periodset_period_internal(PeriodSet *ps, Period *p);
extern bool contained_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

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

extern Datum union_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum union_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum union_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum union_period_period(PG_FUNCTION_ARGS);
extern Datum union_period_periodset(PG_FUNCTION_ARGS);
extern Datum union_periodset_period(PG_FUNCTION_ARGS);
extern Datum union_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *union_timestamp_timestampset_internal(TimestampTz t, TimestampSet *ts);
extern TimestampSet *union_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern PeriodSet *union_period_period_internal(Period *p1, Period *p2);
extern PeriodSet *union_period_periodset_internal(Period *p, PeriodSet *ps);
extern PeriodSet *union_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

/* intersection */

extern Datum intersection_timestamp_timestampset(PG_FUNCTION_ARGS);
extern Datum intersection_timestampset_timestamp(PG_FUNCTION_ARGS);
extern Datum intersection_timestampset_timestampset(PG_FUNCTION_ARGS);
extern Datum intersection_period_period(PG_FUNCTION_ARGS);
extern Datum intersection_period_periodset(PG_FUNCTION_ARGS);
extern Datum intersection_periodset_period(PG_FUNCTION_ARGS);
extern Datum intersection_periodset_periodset(PG_FUNCTION_ARGS);

extern TimestampSet *intersection_timestampset_timestampset_internal(TimestampSet *ts1, TimestampSet *ts2);
extern Period *intersection_period_period_internal(Period *p1, Period *p2);
extern PeriodSet *intersection_period_periodset_internal(Period *p, PeriodSet *ps);
extern PeriodSet *intersection_periodset_periodset_internal(PeriodSet *ps1, PeriodSet *ps2);

/* minus */

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

/*****************************************************************************
 * Prototypes for functions defined in IndexGistTime.c
 *****************************************************************************/

extern Datum gist_time_consistent_exact(PG_FUNCTION_ARGS);
extern Datum gist_time_consistent_recheck(PG_FUNCTION_ARGS);
extern Datum gist_time_union(PG_FUNCTION_ARGS);
extern Datum gist_timestampset_compress(PG_FUNCTION_ARGS);
extern Datum gist_period_compress(PG_FUNCTION_ARGS);
extern Datum gist_periodset_compress(PG_FUNCTION_ARGS);
extern Datum gist_time_decompress(PG_FUNCTION_ARGS);
extern Datum gist_time_penalty(PG_FUNCTION_ARGS);
extern Datum gist_time_picksplit(PG_FUNCTION_ARGS);
extern Datum gist_time_same(PG_FUNCTION_ARGS);
extern Datum gist_time_fetch(PG_FUNCTION_ARGS);

extern bool index_leaf_consistent_time(Period *key, Period *query, StrategyNumber strategy);
extern bool index_internal_consistent_time(Period *key, Period *query, StrategyNumber strategy);
extern bool index_time_bbox_recheck(StrategyNumber strategy);

/*****************************************************************************
 * Prototypes for functions defined in IndexSpgistTime.c
 *****************************************************************************/

extern Datum spgist_timestampset_config(PG_FUNCTION_ARGS);
extern Datum spgist_period_config(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_config(PG_FUNCTION_ARGS);
extern Datum spgist_timestampset_choose(PG_FUNCTION_ARGS);
extern Datum spgist_period_choose(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_choose(PG_FUNCTION_ARGS);
extern Datum spgist_timestampset_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_period_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_time_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_period_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_leaf_consistent(PG_FUNCTION_ARGS);

extern int16 getQuadrant(Period *centroid, Period *tst);
extern int period_bound_cmp(const void *a, const void *b);

#endif

/*****************************************************************************/