/*****************************************************************************
 *
 * tnumber_selfuncs.h
 *    Functions for selectivity estimation of operators on temporal number types
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, Anas Al Bassit
 *    Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TNUMBER_SELFUNCS_H__
#define __TNUMBER_SELFUNCS_H__

#include <postgres.h>
#include <catalog/pg_operator.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum tnumber_sel(PG_FUNCTION_ARGS);
extern Datum tnumber_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif