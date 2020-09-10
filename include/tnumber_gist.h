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

extern Datum tbox_gist_union(PG_FUNCTION_ARGS);
extern Datum tbox_gist_penalty(PG_FUNCTION_ARGS);
extern Datum tbox_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum tnumber_gist_consistent(PG_FUNCTION_ARGS);
extern Datum tnumber_gist_compress(PG_FUNCTION_ARGS);
extern Datum tbox_gist_same(PG_FUNCTION_ARGS);

/* The following functions are also called by tpoint_gist.c */
extern int interval_cmp_lower(const void *i1, const void *i2);
extern int interval_cmp_upper(const void *i1, const void *i2);
extern float non_negative(float val);

/* The following functions are also called by tnumber_spgist.c */
extern bool tbox_index_consistent_leaf(const TBOX *key, const TBOX *query, 
	StrategyNumber strategy);

/*****************************************************************************/

#endif
