/*****************************************************************************
 *
 * GeoParser.c
 *	  Functions for parsing temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __GEOPARSER_H__
#define __GEOPARSER_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "Temporal.h"

/*****************************************************************************/

extern STBOX *stbox_parse(char **str);
extern Temporal *tpoint_parse(char **str, Oid basetype);

/*****************************************************************************/

#endif
