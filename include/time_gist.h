/*****************************************************************************
 *
 * time_gist.c
 *	R-tree GiST index for time types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_GIST_H__
#define __TIME_GIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

extern Datum gist_period_union(PG_FUNCTION_ARGS);
extern Datum gist_timestampset_compress(PG_FUNCTION_ARGS);
extern Datum gist_period_compress(PG_FUNCTION_ARGS);
extern Datum gist_periodset_compress(PG_FUNCTION_ARGS);
extern Datum gist_period_penalty(PG_FUNCTION_ARGS);
extern Datum gist_period_picksplit(PG_FUNCTION_ARGS);
extern Datum gist_period_same(PG_FUNCTION_ARGS);
extern Datum gist_period_fetch(PG_FUNCTION_ARGS);

extern int common_entry_cmp(const void *i1, const void *i2);

extern bool index_leaf_consistent_time(const Period *key, const Period *query, 
	StrategyNumber strategy);
extern bool gist_internal_consistent_period(const Period *key, const Period *query, 
	StrategyNumber strategy);
extern bool index_period_bbox_recheck(StrategyNumber strategy);

#endif

/*****************************************************************************/