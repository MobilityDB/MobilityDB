/*****************************************************************************
 *
 * temporal_spgist.c
 *	Quad-tree SP-GiST index for temporal boolean and temporal text types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_SPGIST_H__
#define __TEMPORAL_SPGIST_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum spgist_temporal_config(PG_FUNCTION_ARGS);
extern Datum spgist_temporalinst_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporali_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporalseq_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporals_choose(PG_FUNCTION_ARGS);
extern Datum spgist_temporalinst_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporali_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporalseq_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporals_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_temporal_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporalinst_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporali_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporalseq_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporals_leaf_consistent(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
