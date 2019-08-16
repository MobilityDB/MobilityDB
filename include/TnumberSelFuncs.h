/*****************************************************************************
 *
 * TnumberSelFuncs.c
 *	  Functions for selectivity estimation of operators on temporal numeric types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Anas Al Bassit
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TNUMBERSELFUNCS_H__
#define __TNUMBERSELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_operator.h>

/*****************************************************************************/

extern Datum tnumber_overlaps_sel(PG_FUNCTION_ARGS);
extern Datum tnumber_overlaps_joinsel(PG_FUNCTION_ARGS);
extern Datum tnumber_contains_sel(PG_FUNCTION_ARGS);
extern Datum tnumber_contains_joinsel(PG_FUNCTION_ARGS);
extern Datum tnumber_same_sel(PG_FUNCTION_ARGS);
extern Datum tnumber_same_joinsel(PG_FUNCTION_ARGS);
extern Datum tnumber_position_sel(PG_FUNCTION_ARGS);
extern Datum tnumber_position_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif