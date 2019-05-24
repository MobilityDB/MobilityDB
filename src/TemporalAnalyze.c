/*****************************************************************************
 *
 * TemporalAnalyze.c
 *	  Functions for gathering statistics from temporal columns
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
#include <TemporalTypes.h>
#include <TemporalAnalyze.h>
/*****************************************************************************/


PG_FUNCTION_INFO_V1(temporal_analyze);
Datum
temporal_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	Datum result = 0;   /* keep compiler quiet */
	int type = TYPMOD_GET_DURATION(stats->attrtypmod);
	temporal_duration_is_valid(type);
	if (type == TEMPORALINST)
		result = temporalinst_analyze(stats);
	else if (type == TEMPORALI)
		result = temporali_analyze(stats);
	else if(type == TEMPORALSEQ || type == TEMPORALS ||
			type == TEMPORAL)
		result = temporal_traj_analyze(stats);
	return result;
}

PG_FUNCTION_INFO_V1(tnumber_analyze);
Datum
tnumber_analyze(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
