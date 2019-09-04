	/*****************************************************************************
 *
 * tpoint_analyze.c
 *	  Functions for gathering statistics from temporal point columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_analyze.h"

#include <assert.h>
#include <access/htup_details.h>

#include "temporal.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_analyze.h"
#include "postgis.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************/

/* 
 * This function is used to remove the time part from the sample rows after
 * getting the statistics from the time dimension, to be able to collect
 * spatial statistics in the same stats variable.
 */
static HeapTuple
remove_temporaldim(HeapTuple tuple, TupleDesc tupDesc, int attrNum,
				   Oid attrtypid, Datum value)
{
	Datum *values = (Datum *) palloc(attrNum * sizeof(Datum));
	bool *null_v = (bool *) palloc(attrNum * sizeof(bool));
	bool *rep_v = (bool *) palloc(attrNum * sizeof(bool));

	for (int j = 0; j < attrNum; j++)
	{
		if (attrtypid == tupDesc->attrs[j].atttypid)
		{
			values[j] = tpoint_values_internal(DatumGetTemporal(value));
			rep_v[j] = true;
			null_v[j] = false;
		}
		else
		{
			values[j] = 0;
			rep_v[j] = false;
			null_v[j] = false;
		}
	}
	return heap_modify_tuple(tuple, tupDesc, values, null_v, rep_v);
}

static void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					 int samplerows, double totalrows)
{
	int duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	int stawidth;

	/* Compute statistics for the time component */
	if (duration == TEMPORALINST)
		temporalinst_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else if (duration == TEMPORALI)
		temporali_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else if (duration == TEMPORALSEQ || duration == TEMPORALS ||
			 duration == TEMPORAL)
		temporals_compute_stats(stats, fetchfunc, samplerows, totalrows);

	stawidth = stats->stawidth;

	/* Remove time component for the tuples */
	for (int i = 0; i < samplerows; i++)
	{
		bool isnull;
		Datum value = fetchfunc(stats, i, &isnull);
		if (isnull)
			continue;
		stats->rows[i] = remove_temporaldim(stats->rows[i],
			stats->tupDesc, stats->tupDesc->natts,
			stats->attrtypid, value);
	}	
	/* Compute statistics for the geometry component */
	call_function1(gserialized_analyze_nd, PointerGetDatum(stats));
	stats->compute_stats(stats, fetchfunc, samplerows, totalrows);

	/* Put the total width of the column, variable size */
	stats->stawidth = stawidth;
}

PG_FUNCTION_INFO_V1(tpoint_analyze);

PGDLLEXPORT Datum
tpoint_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	int duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	assert(duration == TEMPORAL || duration == TEMPORALINST ||
		   duration == TEMPORALI || duration == TEMPORALSEQ ||
		   duration == TEMPORALS);
	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

	if (duration == TEMPORALINST)
		temporal_info(stats);
	else
		temporal_extra_info(stats);
	stats->compute_stats = tpoint_compute_stats;
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
