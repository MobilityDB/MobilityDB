/*****************************************************************************
 *
 * temporal_gist.c
 *		R-tree GiST index for temporal types where only the time dimension is
 *		taken into account, e.g., tbool and ttext.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_gist.h"

#include <access/gist.h>

#include "timetypes.h"
#include "temporal.h"
#include "oidcache.h"
#include "time_gist.h"

/*****************************************************************************
 * Consistent method for temporal types
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_temporal_consistent);

PGDLLEXPORT Datum
gist_temporal_consistent(PG_FUNCTION_ARGS)
{
	GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	StrategyNumber strategy = (StrategyNumber) PG_GETARG_UINT16(2);
	Oid 		subtype = PG_GETARG_OID(3);
	bool	   *recheck = (bool *) PG_GETARG_POINTER(4),
				periodfree = false,
				result;
	Period	   *key = DatumGetPeriod(entry->key),
			   *period;
	
	/* Determine whether the operator is exact */
	*recheck = index_period_bbox_recheck(strategy);
	
	if (subtype == type_oid(T_PERIOD))
	{
		period = PG_GETARG_PERIOD(1);
		if (period == NULL)
			PG_RETURN_BOOL(false);
	}
	else if (temporal_type_oid(subtype))
	{
		Temporal *query = PG_GETARG_TEMPORAL(1);
		if (query == NULL)
			PG_RETURN_BOOL(false);
		period = palloc(sizeof(Period));
		temporal_bbox(period, query);
		periodfree = true;
		PG_FREE_IF_COPY(query, 1);
	}
	else
		elog(ERROR, "unrecognized strategy number: %d", strategy);

	if (GIST_LEAF(entry))
		result = index_leaf_consistent_time(key, period, strategy);
	else
		result = index_internal_consistent_period(key, period, strategy);
	
	if (periodfree)
		pfree(period);

	PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Compress methods for temporal Boolean and temporal text
 *****************************************************************************/

/*
 * GiST compress method for temporals
 */
PG_FUNCTION_INFO_V1(gist_temporal_compress);

PGDLLEXPORT Datum
gist_temporal_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	
	if (entry->leafkey)
	{
		GISTENTRY *retval = palloc(sizeof(GISTENTRY));
		Temporal *temp = DatumGetTemporal(entry->key);
		Period *period = palloc(sizeof(Period));
		temporal_bbox(period, temp);
		gistentryinit(*retval, PointerGetDatum(period),
			entry->rel, entry->page, entry->offset, false);
		PG_RETURN_POINTER(retval);
	}
	
	PG_RETURN_POINTER(entry);
}

/*****************************************************************************/