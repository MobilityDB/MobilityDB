/*****************************************************************************
 *
 * TempSelFuncs.c
 *	  Functions for selectivity estimation of operators on temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *	XXX These are totally bogus.  Perhaps someone will make them do
 *	something reasonable, someday.
 *
 *****************************************************************************/
 
#include "TemporalTypes.h"

/*
 *	Selectivity functions for temporal types operators.  These are bogus -- 
 *	unless we know the actual key distribution in the index, we can't make
 *	a good prediction of the selectivity of these operators.
 *
 *	Note: the values used here may look unreasonably small.  Perhaps they
 *	are.  For now, we want to make sure that the optimizer will make use
 *	of a geometric index if one is available, so the selectivity had better
 *	be fairly small.
 *
 *	In general, GiST needs to search multiple subtrees in order to guarantee
 *	that all occurrences of the same key have been found.  Because of this,
 *	the estimated cost for scanning the index ought to be higher than the
 *	output selectivity would indicate.  gistcostestimate(), over in selfuncs.c,
 *	ought to be adjusted accordingly --- but until we can generate somewhat
 *	realistic numbers here, it hardly matters...
 */

/*
 * Selectivity for operators that depend on volume, such as "overlap".
 */

PG_FUNCTION_INFO_V1(volumesel);

PGDLLEXPORT Datum
volumesel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(volumejoinsel);

PGDLLEXPORT Datum
volumejoinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.005);
}

/*
 *	positionseltemp
 *
 * How likely is a box to be strictly left of (right of, above, below,
 * before, after) a given box?
 */

PG_FUNCTION_INFO_V1(positionseltemp);

PGDLLEXPORT Datum
positionseltemp(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.1);
}

PG_FUNCTION_INFO_V1(positionjoinseltemp);

PGDLLEXPORT Datum
positionjoinseltemp(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.1);
}

/*
 *	contseltemp: How likely is a box to contain (be contained by) a given box?
 *
 * This is a tighter constraint than "overlap", so produce a smaller
 * estimate than volumesel does.
 */

PG_FUNCTION_INFO_V1(contseltemp);

PGDLLEXPORT Datum
contseltemp(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(contjoinseltemp);

PGDLLEXPORT Datum
contjoinseltemp(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/
