/*****************************************************************************
 *
 * time_gist.c
 *  R-tree GiST index for time types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
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

extern Datum period_gist_union(PG_FUNCTION_ARGS);
extern Datum timestampset_gist_compress(PG_FUNCTION_ARGS);
extern Datum period_gist_compress(PG_FUNCTION_ARGS);
extern Datum periodset_gist_compress(PG_FUNCTION_ARGS);
extern Datum period_gist_penalty(PG_FUNCTION_ARGS);
extern Datum period_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum period_gist_same(PG_FUNCTION_ARGS);
extern Datum period_gist_fetch(PG_FUNCTION_ARGS);

extern int common_entry_cmp(const void *i1, const void *i2);

extern bool period_index_consistent_leaf(const Period *key, const Period *query, 
  StrategyNumber strategy);
extern bool period_gist_consistent_internal(const Period *key, const Period *query, 
  StrategyNumber strategy);
extern bool period_index_recheck(StrategyNumber strategy);

#endif

/*****************************************************************************/