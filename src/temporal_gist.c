/*****************************************************************************
 *
 * temporal_gist.c
 *		R-tree GiST index for temporal types where only the time dimension 
 *		is taken into account for indexing, currently, tbool and ttext.
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
 * GiST compress method for temporal values
 *****************************************************************************/

PG_FUNCTION_INFO_V1(gist_temporal_compress);
/**
 * GiST compress method for temporal values
 */
PGDLLEXPORT Datum
gist_temporal_compress(PG_FUNCTION_ARGS)
{
	GISTENTRY *entry = (GISTENTRY *) PG_GETARG_POINTER(0);

	if (entry->leafkey)
	{
		GISTENTRY *retval = palloc(sizeof(GISTENTRY));
		Temporal *temp = DatumGetTemporal(entry->key);
		Period *period = palloc0(sizeof(Period));
		temporal_bbox(period, temp);
		gistentryinit(*retval, PointerGetDatum(period),
			entry->rel, entry->page, entry->offset, false);
		PG_RETURN_POINTER(retval);
	}

	PG_RETURN_POINTER(entry);
}

/*****************************************************************************
 * GiST decompress method for temporal values
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION < 110000
PG_FUNCTION_INFO_V1(gist_temporal_decompress);
/**
 * GiST decompress method for temporal values (result in a period)
 */
PGDLLEXPORT Datum
gist_temporal_decompress(PG_FUNCTION_ARGS)
{
	GISTENTRY  *entry = (GISTENTRY *) PG_GETARG_POINTER(0);
	PG_RETURN_POINTER(entry);
}
#endif

/*****************************************************************************/