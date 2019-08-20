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

static HeapTuple
remove_temporaldim(HeapTuple tuple, TupleDesc tupDesc, int attrNum,
				   Oid attrtypid, bool geom, Datum value)
{
	/* The variables below are used to remove the temporal part from
	 * the sample rows after getting the temporal value,
	 * to be able to collect other statistics in the same stats variable.
	 */
	Datum *values = (Datum *) palloc(attrNum * sizeof(Datum));
	bool *null_v = (bool *) palloc(attrNum * sizeof(bool));
	bool *rep_v = (bool *) palloc(attrNum * sizeof(bool));

	for (int j = 0; j < attrNum; j++)
	{
		int typemod = TYPMOD_GET_DURATION(tupDesc->attrs[j].atttypmod);
		switch (typemod)
		{
			case TEMPORAL:
			case TEMPORALINST:
			case TEMPORALI:
			case TEMPORALSEQ:
			case TEMPORALS:
			{
				if (attrtypid == tupDesc->attrs[j].atttypid)
				{
					if (geom)
					{
						if (attrtypid == type_oid(T_STBOX))
						{
							STBOX *box = DatumGetSTboxP(value);

							//Convert STBOX to geometry
							LWLINE *line;
							LWPOINT *lwpoint = lwpoint_make2d(4326, box->xmin, box->ymin);
							LWPOINT **points = palloc(sizeof(LWPOINT *) * 2);
							points[0] = lwpoint;
							lwpoint = lwpoint_make2d(4326, box->xmax, box->ymax);
							points[1] = lwpoint;
							line = lwline_from_ptarray(4326, 2, points);
							value = PointerGetDatum(geometry_serialize(lwline_as_lwgeom(line)));
							lwline_free(line);lwpoint_free(lwpoint);
							pfree(points);
							values[j] = value;
						}
						else
							values[j] = tpoint_values_internal(DatumGetTemporal(value));
					}

					else
						values[j] = tempdisc_get_values_internal(DatumGetTemporal(value));
					rep_v[j] = true;
					null_v[j] = false;
				}
				else
				{
					values[j] = 0;
					rep_v[j] = false;
					null_v[j] = false;
				}

				break;
			}
			default:
			{
				values[j] = 0;
				rep_v[j] = false;
				null_v[j] = false;
				break;
			}
		}
	}
	return heap_modify_tuple(tuple, tupDesc, values, null_v, rep_v);
}

static void
tpoint_compute_stats(VacAttrStats *stats, AnalyzeAttrFetchFunc fetchfunc,
					 int samplerows, double totalrows)
{
	int type = TYPMOD_GET_DURATION(stats->attrtypmod);
	int stawidth;

	/* Compute statistics for the time component */
	if (type == TEMPORALINST)
		temporalinst_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else if (type == TEMPORALI)
		temporali_compute_stats(stats, fetchfunc, samplerows, totalrows);
	else if (type == TEMPORALSEQ || type == TEMPORALS ||
			 type == TEMPORAL)
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
			stats->attrtypid, true, value);
	}	
	/* Compute statistics for the geometry component */
	call_function1(gserialized_analyze_nd, PointerGetDatum(stats));
	stats->compute_stats(stats, fetchfunc, samplerows, totalrows);

	/* Put the total width of the column, variable size */
	stats->stawidth = stawidth;
}

static Datum
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

PG_FUNCTION_INFO_V1(tpoint_analyze);

PGDLLEXPORT Datum
tpoint_analyze(PG_FUNCTION_ARGS)
{
	VacAttrStats *stats = (VacAttrStats *) PG_GETARG_POINTER(0);
	int durationType = TYPMOD_GET_DURATION(stats->attrtypmod);
	assert(durationType == TEMPORAL || durationType == TEMPORALINST ||
		   durationType == TEMPORALI || durationType == TEMPORALSEQ ||
		   durationType == TEMPORALS);
	return tpoint_analyze_internal(stats, durationType);
}

/*****************************************************************************/
