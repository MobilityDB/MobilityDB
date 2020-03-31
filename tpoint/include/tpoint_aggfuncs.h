/*****************************************************************************
 *
 * tpoint_aggfuncs.h
 *	Aggregate functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_AGGFUNCS_H__
#define __TPOINT_AGGFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum tpoint_extent_transfn(PG_FUNCTION_ARGS);
extern Datum tpoint_extent_combinefn(PG_FUNCTION_ARGS);

extern Datum tpoint_tcentroid_transfn(PG_FUNCTION_ARGS);
extern Datum tpoint_tcentroid_combinefn(PG_FUNCTION_ARGS);
extern Datum tpoint_tcentroid_finalfn(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
