/*****************************************************************************
 *
 * tpoint_gist.c
 *    R-tree GiST index for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_GIST_H__
#define __TPOINT_GIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/builtins.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum stbox_gist_consistent(PG_FUNCTION_ARGS);
extern Datum stbox_gist_union(PG_FUNCTION_ARGS);
extern Datum stbox_gist_penalty(PG_FUNCTION_ARGS);
extern Datum stbox_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum stbox_gist_same(PG_FUNCTION_ARGS);
extern Datum tpoint_gist_compress(PG_FUNCTION_ARGS);

/* The following functions are also called by IndexSpgistTPoint.c */
extern bool tpoint_index_recheck(StrategyNumber strategy);
extern bool stbox_index_consistent_leaf(const STBOX *key, const STBOX *query,
  StrategyNumber strategy);

/*****************************************************************************/

#endif
