/*****************************************************************************
 *
 * tpoint_parser.h
 *	  Functions for parsing temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TPOINT_PARSER_H__
#define __TPOINT_PARSER_H__

#include <postgres.h>
#include <catalog/pg_type.h>
#include "temporal.h"

/*****************************************************************************/

extern STBOX *stbox_parse(char **str);
extern Temporal *tpoint_parse(char **str, Oid basetype);

/*****************************************************************************/

#endif
