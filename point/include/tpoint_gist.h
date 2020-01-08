/*****************************************************************************
 *
 * tpoint_gist.c
 *	  R-tree GiST index for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
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

extern Datum gist_tpoint_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_union(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_penalty(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_picksplit(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_same(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_compress(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_decompress(PG_FUNCTION_ARGS);
extern Datum gist_tpoint_distance(PG_FUNCTION_ARGS);

/* The following functions are also called by IndexSpgistTPoint.c */
extern bool index_tpoint_bbox_recheck(StrategyNumber strategy);
extern bool index_leaf_consistent_stbox_box2D(STBOX *key, STBOX *query, 
	StrategyNumber strategy);
extern bool index_leaf_consistent_stbox_period(STBOX *key, Period *query, 
	StrategyNumber strategy);
extern bool index_leaf_consistent_stbox_stbox(STBOX *key, STBOX *query, 
	StrategyNumber strategy);

extern bool index_tpoint_recheck(StrategyNumber strategy);
extern bool index_leaf_consistent_stbox(STBOX *key, STBOX *query, 
	StrategyNumber strategy);

/*****************************************************************************/

#endif
