/*****************************************************************************
 *
 * TempPointAnalyze.c
 *	  Functions for gathering statistics from temporal point columns
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Mahmoud Sakr, Mohamed Bakli,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * IDENTIFICATION
 *	point/src/TempPointAnalyze.c
 *
 *****************************************************************************/
#include <TemporalTypes.h>
#include <TemporalAnalyze.h>
/*****************************************************************************/


PG_FUNCTION_INFO_V1(tpoint_analyze);
Datum
tpoint_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	int durationType = TYPMOD_GET_DURATION(stats->attrtypmod);
	assert(durationType == TEMPORAL || durationType == TEMPORALINST || 
		durationType == TEMPORALI || durationType == TEMPORALSEQ || 
		durationType == TEMPORALS);
	return tpoint_analyze_internal(stats, durationType);
}

Datum
tpoint_analyze_internal(VacAttrStats *stats, int durationType)
{
	/*
	 * Call the standard typanalyze function.  It may fail to find needed
	 * operators, in which case we also can't do anything, so just fail.
	 */
	if (!std_typanalyze(stats))
		PG_RETURN_BOOL(false);

	if (durationType == TEMPORALINST)
		temporal_info(stats);
	else
		temporal_extra_info(stats, durationType);

	stats->compute_stats = tpoint_compute_stats;

	PG_RETURN_BOOL(true);
}

void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					 int samplerows, double totalrows)
{
	int type = TYPMOD_GET_DURATION(stats->attrtypmod);
	int stawidth;
	/* Temporal Statistics */
	if (type == TEMPORALINST)
		temporalinst_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else if (type == TEMPORALI)
		temporali_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else if (type == TEMPORALSEQ || type == TEMPORALS ||
			 type == TEMPORAL)
		temporals_compute_stats(stats, fetchfunc, samplerows, totalrows);

	stawidth = stats->stawidth;
	/* Geometry Statistics */
	#ifdef WITH_POSTGIS
		call_function1(gserialized_analyze_nd, PointerGetDatum(stats));
		stats->compute_stats(stats, fetchfunc, samplerows, totalrows);
	#endif
	/* Put the total width of the column, variable size */
	stats->stawidth = stawidth;
}
/*****************************************************************************/
