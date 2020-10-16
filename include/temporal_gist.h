/*****************************************************************************
 *
 * temporal_gist.c
 *  Quad-tree SP-GiST index for temporal boolean and temporal text types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_GIST_H__
#define __TEMPORAL_GIST_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum gist_temporal_consistent(PG_FUNCTION_ARGS);
extern Datum gist_temporal_compress(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
