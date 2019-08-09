/*****************************************************************************
 *
 * TPointSelFuncs.c
 *      Functions for selectivity estimation of operators on temporal point 
 *      types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *	These functions are only stubs, they need to be written TODO
 *
 *****************************************************************************/

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/rangetypes.h>

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

/*****************************************************************************/

/*
 * Selectivity for operators for bounding box operators, i.e., overlaps (&&), 
 * contains (@>), contained (<@), and, same (~=). These operators depend on 
 * volume. Contains and contained are tighter contraints than overlaps, so 
 * the former should produce lower estimates than the latter. Similarly, 
 * equals is a tighter constrain tha contains and contained.
 */

PG_FUNCTION_INFO_V1(tpoint_overlaps_sel);

PGDLLEXPORT Datum
tpoint_overlaps_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(tpoint_overlaps_joinsel);

PGDLLEXPORT Datum
tpoint_overlaps_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.005);
}

PG_FUNCTION_INFO_V1(tpoint_contains_sel);

PGDLLEXPORT Datum
tpoint_contains_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(tpoint_contains_joinsel);

PGDLLEXPORT Datum
tpoint_contains_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.002);
}

PG_FUNCTION_INFO_V1(tpoint_same_sel);

PGDLLEXPORT Datum
tpoint_same_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(tpoint_same_joinsel);

PGDLLEXPORT Datum
tpoint_same_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/

/*
 * Selectivity for operators for relative position box operators, i.e., 
 * left (<<), overleft (&<), right (>>), overright (&>), 
 * below (<<|), overbelow (&<|), above (|>>), overabove (|&>), 
 * front (<</), overfront (&</), back (/>>), overfront (/&>), 
 * before (<<#), overbefore (&<#), after (#>>), overafter (#&>). 
 */

PG_FUNCTION_INFO_V1(tpoint_position_sel);

PGDLLEXPORT Datum
tpoint_position_sel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

PG_FUNCTION_INFO_V1(tpoint_position_joinsel);

PGDLLEXPORT Datum
tpoint_position_joinsel(PG_FUNCTION_ARGS)
{
	PG_RETURN_FLOAT8(0.001);
}

/*****************************************************************************/
