/*****************************************************************************
 *
 * time_spgist.h
 *	Quad-tree SP-GiST index for time types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_SPGIST_H__
#define __TIME_SPGIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

extern Datum spgist_timestampset_config(PG_FUNCTION_ARGS);
extern Datum spgist_period_config(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_config(PG_FUNCTION_ARGS);
extern Datum spgist_timestampset_choose(PG_FUNCTION_ARGS);
extern Datum spgist_period_choose(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_choose(PG_FUNCTION_ARGS);
extern Datum spgist_timestampset_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_period_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_period_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_period_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_periodset_leaf_consistent(PG_FUNCTION_ARGS);

extern int16 getQuadrant(Period *centroid, Period *tst);
extern int period_bound_cmp(const void *a, const void *b);

#endif

/*****************************************************************************/