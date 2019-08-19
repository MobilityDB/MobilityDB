/*****************************************************************************
 *
 * time_gist.c
 *	R-tree GiST index for time types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_GIST_H__
#define __TIME_GIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

extern Datum gist_time_consistent_exact(PG_FUNCTION_ARGS);
extern Datum gist_time_consistent_recheck(PG_FUNCTION_ARGS);
extern Datum gist_time_union(PG_FUNCTION_ARGS);
extern Datum gist_timestampset_compress(PG_FUNCTION_ARGS);
extern Datum gist_period_compress(PG_FUNCTION_ARGS);
extern Datum gist_periodset_compress(PG_FUNCTION_ARGS);
extern Datum gist_time_decompress(PG_FUNCTION_ARGS);
extern Datum gist_time_penalty(PG_FUNCTION_ARGS);
extern Datum gist_time_picksplit(PG_FUNCTION_ARGS);
extern Datum gist_time_same(PG_FUNCTION_ARGS);
extern Datum gist_time_fetch(PG_FUNCTION_ARGS);

extern bool index_leaf_consistent_time(Period *key, Period *query, StrategyNumber strategy);
extern bool index_internal_consistent_time(Period *key, Period *query, StrategyNumber strategy);
extern bool index_time_bbox_recheck(StrategyNumber strategy);

#endif

/*****************************************************************************/