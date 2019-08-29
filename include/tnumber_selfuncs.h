/*****************************************************************************
 *
 * tnumber_selfuncs.c
 *	  Functions for selectivity estimation of operators on temporal numeric types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Anas Al Bassit
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TNUMBER_SELFUNCS_H__
#define __TNUMBER_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_operator.h>
#include "temporal.h"

/*****************************************************************************
 * Some other helper functions.
 *****************************************************************************/

extern bool tnumber_const_bounds(Node *other, TBOX *box);

/*****************************************************************************/

extern Datum tnumber_sel(PG_FUNCTION_ARGS);
extern Datum tnumber_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif