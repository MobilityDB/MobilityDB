/*****************************************************************************
 *
 * tnumber_distance.h
 *    Distance functions for temporal numbers.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi,
 *     Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TNUMBER_DISTANCE_H__
#define __TNUMBER_DISTANCE_H__

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <postgres.h>
#include <catalog/pg_type.h>
#include <float.h>

#include "temporal.h"

/*****************************************************************************/

/* Distance functions */

extern Datum distance_base_tnumber(PG_FUNCTION_ARGS);
extern Datum distance_tnumber_base(PG_FUNCTION_ARGS);
extern Datum distance_tnumber_tnumber(PG_FUNCTION_ARGS);

/* Nearest approach distance */

extern Datum NAD_base_tnumber(PG_FUNCTION_ARGS);
extern Datum NAD_tnumber_base(PG_FUNCTION_ARGS);
extern Datum NAD_tbox_tbox(PG_FUNCTION_ARGS);
extern Datum NAD_tbox_tnumber(PG_FUNCTION_ARGS);
extern Datum NAD_tnumber_tbox(PG_FUNCTION_ARGS);
extern Datum NAD_tnumber_tnumber(PG_FUNCTION_ARGS);

extern double NAD_tbox_tbox_internal(const TBOX *box1, const TBOX *box2);

// NAI and shortestline functions are not yet implemented
// Are they useful ?

/*****************************************************************************/

#endif
