/*****************************************************************************
 *
 * stbox.h
 *	  Basic functions for STBOX bounding box.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __STBOX_H__
#define __STBOX_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

extern Datum stbox_in(PG_FUNCTION_ARGS);
extern Datum stbox_out(PG_FUNCTION_ARGS);
extern Datum stbox_constructor(PG_FUNCTION_ARGS);
extern Datum stbox_constructor(PG_FUNCTION_ARGS);
extern Datum geodstbox_constructor(PG_FUNCTION_ARGS);

extern STBOX *stbox_new(bool hasx, bool hasz, bool hast, bool geodetic);
extern STBOX *stbox_copy(const STBOX *box);
extern void stbox_to_period(Period *period, const STBOX *box); 
extern int stbox_cmp_internal(const STBOX *box1, const STBOX *box2);

/*****************************************************************************/

#endif
