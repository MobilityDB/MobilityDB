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
#include <executor/spi.h>

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
 * The function is derived from PostgreSQL function heap_modify_tuple
 */

/* OLD VERSION
static HeapTuple
tpoint_remove_timedim_old(HeapTuple tuple, TupleDesc tupDesc, int tupattnum, 
	int natts, Datum value)
{
	Datum *replValues = (Datum *) palloc(natts * sizeof(Datum));
	bool *replIsnull = (bool *) palloc(natts * sizeof(bool));
	bool *doReplace = (bool *) palloc(natts * sizeof(bool));

	for (int i = 0; i < natts; i++)
	{
		/ * tupattnum is 1-based * /
		if (i == tupattnum - 1)
		{
			replValues[i] = tpoint_values_internal(DatumGetTemporal(value));
			replIsnull[i] = false;
			doReplace[i] = true;
		}
		else
		{
			replValues[i] = 0;
			replIsnull[i] = false;
			doReplace[i] = false;
		}
	}
	HeapTuple result = heap_modify_tuple(tuple, tupDesc, replValues, replIsnull, doReplace);
	pfree(replValues); pfree(replIsnull); pfree(doReplace);
	return result;
}
*/

static HeapTuple
tpoint_remove_timedim(HeapTuple tuple, TupleDesc tupDesc, int tupattnum, 
	int natts, Datum value, Datum *values, bool *isnull)
{
	heap_deform_tuple(tuple, tupDesc, values, isnull);

	SPI_connect();
	Datum replValue = tpoint_values_internal(DatumGetTemporal(value));
	SPI_finish();
	/* tupattnum is 1-based */
	values[tupattnum - 1] = replValue;
	HeapTuple result = heap_form_tuple(tupDesc, values, isnull);
	pfree(DatumGetPointer(replValue));

	/*
	 * copy the identification info of the old tuple: t_ctid, t_self, and OID
	 * (if any)
	 */
	result->t_data->t_ctid = tuple->t_data->t_ctid;
	result->t_self = tuple->t_self;
	result->t_tableOid = tuple->t_tableOid;
	if (tupDesc->tdhasoid)
		HeapTupleSetOid(result, HeapTupleGetOid(tuple));

	heap_freetuple(tuple);
	return result;
}

static void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					 int samplerows, double totalrows)
{
	MemoryContext old_context;
	int duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	int stawidth;

	/* Compute statistics for the time component */
	if (duration == TEMPORALINST)
		temporalinst_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else
		temporals_compute_stats(stats, fetchfunc, samplerows, totalrows);

	stawidth = stats->stawidth;

	/* Must copy the target values into anl_context */
	old_context = MemoryContextSwitchTo(stats->anl_context);

	Datum *values = (Datum *) palloc(stats->tupDesc->natts * sizeof(Datum));
	bool *isnull = (bool *) palloc(stats->tupDesc->natts * sizeof(bool));

	/* Remove time component for the tuples */
	for (int i = 0; i < samplerows; i++)
	{
		bool valueisnull;
		Datum value = fetchfunc(stats, i, &valueisnull);
		if (valueisnull)
			continue;

		stats->rows[i] = tpoint_remove_timedim(stats->rows[i], 	
			stats->tupDesc, stats->tupattnum, stats->tupDesc->natts, value,
			values, isnull);
	}

	/* Compute statistics for the geometry component */
	call_function1(gserialized_analyze_nd, PointerGetDatum(stats));
	stats->compute_stats(stats, fetchfunc, samplerows, totalrows);

	/* Put the total width of the column, variable size */
	stats->stawidth = stawidth;

	pfree(values); pfree(isnull);

	/* Switch back to the previous context */
	MemoryContextSwitchTo(old_context);

	return;
}

PG_FUNCTION_INFO_V1(tpoint_analyze);

PGDLLEXPORT Datum
tpoint_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	int duration;

	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

	/* 
	 * Ensure duration is valid and collect extra information about the 
	 * temporal type and its base and time types.
	 */
	duration = TYPMOD_GET_DURATION(stats->attrtypmod);
	temporal_duration_all_is_valid(duration);
	if (duration != TEMPORALINST)
		temporal_extra_info(stats);

	/* Set the callback function to compute statistics. */
	stats->compute_stats = tpoint_compute_stats;
	PG_RETURN_BOOL(true);
}

/*****************************************************************************/
