/*****************************************************************************
 *
 * timetypes.h
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
#include <access/stratnum.h>
#include <utils/timestamp.h>

typedef struct 
{
	TimestampTz	lower;			/* the lower bound value */
	TimestampTz	upper;			/* the upper bound value */
	bool lower_inc;				/* the lower bound is inclusive (vs exclusive) */
	bool upper_inc;				/* the upper bound is inclusive (vs exclusive) */
} Period;

/* Internal representation of either bound of a period (not what's on disk) */
typedef struct
{
	TimestampTz val;			/* the bound value */
	bool inclusive;				/* bound is inclusive (vs exclusive) */
	bool lower;					/* this is the lower (vs upper) bound */
} PeriodBound;

typedef struct 
{
	int32 vl_len_;				/* varlena header (do not touch directly!) */
	int32 count;				/* number of Period elements */
 	/* variable-length data follows */
} PeriodSet;

typedef struct 
{
	int32 vl_len_;				/* varlena header (do not touch directly!) */
	int32 count;				/* number of Period elements */
 	/* variable-length data follows */
} TimestampSet;

/*
 * fmgr macros for time types
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

/*****************************************************************************/

#endif
