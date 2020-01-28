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
#include "temporal.h"

/*****************************************************************************/

extern Datum spgist_temporal_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporal_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_temporal_compress(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
