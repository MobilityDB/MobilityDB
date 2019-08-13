/*****************************************************************************
 *
 * IndexGistTnumber.c
 *	  R-tree GiST index for temporal integers and temporal floats
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __INDEXGISTTNUMBER_H__
#define __INDEXGISTTNUMBER_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "Temporal.h"

/*****************************************************************************/

extern Datum gist_tintinst_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tfloatinst_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tnumber_consistent(PG_FUNCTION_ARGS);
extern Datum gist_tnumberinst_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumberi_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumberseq_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumbers_compress(PG_FUNCTION_ARGS);
extern Datum gist_tnumber_compress(PG_FUNCTION_ARGS);

/* The following functions are also called by IndexSpgistTnumber.c */
extern bool index_leaf_consistent_tbox(TBOX *key, TBOX *query, StrategyNumber strategy);

/*****************************************************************************/

#endif
