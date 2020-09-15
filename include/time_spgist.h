/*****************************************************************************
 *
 * time_spgist.h
 *  Quad-tree SP-GiST index for time types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TIME_SPGIST_H__
#define __TIME_SPGIST_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "timetypes.h"

/*****************************************************************************/

extern Datum spperiod_gist_config(PG_FUNCTION_ARGS);
extern Datum spperiod_gist_choose(PG_FUNCTION_ARGS);
extern Datum spperiod_gist_picksplit(PG_FUNCTION_ARGS);
extern Datum spperiod_gist_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spperiod_gist_leaf_consistent(PG_FUNCTION_ARGS);

extern int16 getQuadrant(Period *centroid, Period *tst);

#endif

/*****************************************************************************/