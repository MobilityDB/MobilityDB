/*****************************************************************************
 *
 * ProjectionGK.h
 *	  Implementation of the Gauss Krueger projection that is used in Secondo
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORALPOINT_H__
#define __TEMPORALPOINT_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum tgeompoint_transform_gk(PG_FUNCTION_ARGS);
extern Datum geometry_transform_gk(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
