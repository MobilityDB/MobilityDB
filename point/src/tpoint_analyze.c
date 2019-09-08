/*****************************************************************************
 *
 * tpoint_analyze.c
 *	  Functions for gathering statistics from temporal point columns
 *
 * Various kind of statistics are collected for both the value and the time
 * dimensions of temporal types. The kind of statistics depends on the duration
 * of the temporal type, which is defined in the table schema by the typmod
 * attribute. Please refer to the PostgreSQL file pg_statistic_d.h and the
 * PostGIS file gserialized_estimate.c for more information about the 
 * statistics collected.
 * 
 * For the spatial dimension, the statistics collected are the same for all 
 * durations. These statistics are obtained by calling the PostGIS function
 * gserialized_analyze_nd.
 * - Slot 1
 * 		- stakind contains the type of statistics which is STATISTIC_SLOT_2D.
 * 		- stanumbers stores the 2D histrogram of occurrence of features.
 * - Slot 2
 * 		- stakind contains the type of statistics which is STATISTIC_SLOT_ND.
 * 		- stanumbers stores the ND histrogram of occurrence of features.
 * For the time dimension, the statistics collected in Slots 3 and 4 depend on 
 * the duration. Please refer to file temporal_analyze.c for more information.
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

/*
 * While statistic functions are running, we keep a pointer to the extra data
 * here for use by assorted subroutines.  The functions doesn't
 * currently need to be re-entrant, so avoiding this is not worth the extra
 * notational cruft that would be needed.
 */
TemporalAnalyzeExtraData *temporal_extra_data;

/*****************************************************************************/

/* 
 * This function is used to remove the time part from the sample rows after
 * getting the statistics from the time dimension, to be able to collect
 * spatial statistics in the same stats variable.
 */
static HeapTuple
tpoint_remove_timedim(HeapTuple tuple, TupleDesc tupDesc, int attrNum,
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
	else
		temporals_compute_stats(stats, fetchfunc, samplerows, totalrows);

	stawidth = stats->stawidth;

	/* Remove time component for the tuples */
	for (int i = 0; i < samplerows; i++)
	{
		bool isnull;
		Datum value = fetchfunc(stats, i, &isnull);
		if (isnull)
			continue;
		stats->rows[i] = tpoint_remove_timedim(stats->rows[i],
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

	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

	/* 
	 * Collect extra information about the temporal type and its value
	 * and time types
	 */
	temporal_extra_info(stats);
	temporal_extra_data = (TemporalAnalyzeExtraData *)stats->extra_data;

	/* Compute statistics */
	stats->compute_stats = tpoint_compute_stats;
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
