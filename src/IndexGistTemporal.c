/*****************************************************************************
 *
 * IndexGistTemporal.c
 *		R-tree GiST index for temporal types where only the time dimension is
 *		taken into account, e.g., tbool and ttext.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

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
				result;
	Period	   *key = DatumGetPeriod(entry->key),
			   *query, period;
	
	/* Determine whether the operator is exact */
	*recheck = index_time_bbox_recheck(strategy);
	
	/* Since the function is defined as STRICT in SQL the query will never
	 * be null. */
	if (subtype == type_oid(T_PERIOD))
	{
		query = PG_GETARG_PERIOD(1);
		assert(query != NULL);
	}
	else if (temporal_oid(subtype))
	{
		Temporal *temp = PG_GETARG_TEMPORAL(1);
		assert(temp != NULL);
		query = &period;
		temporal_bbox(query, temp);
		PG_FREE_IF_COPY(temp, 1);
	}
	else
		elog(ERROR, "unrecognized strategy number: %d", strategy);

	if (GIST_LEAF(entry))
		result = index_leaf_consistent_time(key, query, strategy);
	else
		result = index_internal_consistent_time(key, query, strategy);
	
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