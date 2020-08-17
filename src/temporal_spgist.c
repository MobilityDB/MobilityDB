/*****************************************************************************
 *
 * temporal_spgist.c
 *	Quad-tree SP-GiST index for temporal boolean and temporal text types.
 *
 * These functions are based on those in the file rangetypes_spgist.c.
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#if MOBDB_PGSQL_VERSION >= 110000

#include "temporal_spgist.h"

#include <assert.h>
#include <access/spgist.h>
#include <utils/builtins.h>

#include "timetypes.h"
#include "oidcache.h"
#include "period.h"
#include "time_gist.h"
#include "time_spgist.h"
#include "temporaltypes.h"

/*****************************************************************************
 * SP-GiST compress functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(spgist_temporal_compress);
/**
 * SP-GiST compress function for temporal values
 */
PGDLLEXPORT Datum
spgist_temporal_compress(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Period *period = palloc(sizeof(Period));

	temporal_bbox(period, temp);

	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_PERIOD(period);
}
#endif

/*****************************************************************************/
