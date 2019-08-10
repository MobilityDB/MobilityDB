/*****************************************************************************
 *
 * TempPointSelFuncs.h
 *      Functions for selectivity estimation of operators on temporal points
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPPOINTSELFUNCS_H__
#define __TEMPPOINTSELFUNCS_H__

/*****************************************************************************/

extern Datum tpoint_overlaps_sel(PG_FUNCTION_ARGS);
extern Datum tpoint_overlaps_joinsel(PG_FUNCTION_ARGS);
extern Datum tpoint_contains_sel(PG_FUNCTION_ARGS);
extern Datum tpoint_contains_joinsel(PG_FUNCTION_ARGS);
extern Datum tpoint_same_sel(PG_FUNCTION_ARGS);
extern Datum tpoint_same_joinsel(PG_FUNCTION_ARGS);
extern Datum tpoint_position_sel(PG_FUNCTION_ARGS);
extern Datum tpoint_position_joinsel(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
