/*****************************************************************************
 *
 * temporal_analyze.c
 *	  Functions for gathering statistics from temporal columns
 *
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 * * IDENTIFICATION
 *	include/TemporalAnalyze.c
 *
 *****************************************************************************/

#include "temporal_analyze.h"

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_analyze);

PGDLLEXPORT Datum
temporal_analyze(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

PG_FUNCTION_INFO_V1(tnumber_analyze);

PGDLLEXPORT Datum
tnumber_analyze(PG_FUNCTION_ARGS)
{
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
