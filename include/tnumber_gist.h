/*****************************************************************************
 *
 * tnumber_gist.c
 *	  R-tree GiST index for temporal integers and temporal floats
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TNUMBER_GIST_H__
#define __TNUMBER_GIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum gist_tbox_union(PG_FUNCTION_ARGS);
extern Datum gist_tbox_penalty(PG_FUNCTION_ARGS);
extern Datum gist_tbox_picksplit(PG_FUNCTION_ARGS);
extern Datum gist_tnumber_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tnumber_compress(PG_FUNCTION_ARGS);
extern Datum gist_tbox_same(PG_FUNCTION_ARGS);

/* The following functions are also called by tpoint_gist.c */
extern int interval_cmp_lower(const void *i1, const void *i2);
extern int interval_cmp_upper(const void *i1, const void *i2);
extern float non_negative(float val);

/* The following functions are also called by tnumber_spgist.c */
extern bool index_leaf_consistent_tbox(const TBOX *key, const TBOX *query, 
	StrategyNumber strategy);

/*****************************************************************************/

#endif
