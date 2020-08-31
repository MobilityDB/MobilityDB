/*****************************************************************************
 *
 * tnumber_spgist.c
 *	SP-GiST implementation of 4-dimensional quad tree over temporal
 *	integers and floats.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TNUMBER_SPGIST_H__
#define __TNUMBER_SPGIST_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum spgist_tbox_config(PG_FUNCTION_ARGS);
extern Datum spgist_tbox_choose(PG_FUNCTION_ARGS);
extern Datum spgist_tbox_picksplit(PG_FUNCTION_ARGS);
extern Datum spgist_tbox_inner_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tbox_leaf_consistent(PG_FUNCTION_ARGS);
extern Datum spgist_tnumber_compress(PG_FUNCTION_ARGS);

/* The following functions are also called by tpoint_spgist.c */
extern int compareDoubles(const void *a, const void *b);

/*****************************************************************************/

#endif
