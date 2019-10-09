/*****************************************************************************
 *
 * temporal_textfuncs.h
 *	Text functions (textcat, lower, upper).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_TEXTFUNCS_H__
#define __TEMPORAL_TEXTFUNCS_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum textcat_base_temporal(PG_FUNCTION_ARGS);
extern Datum textcat_temporal_base(PG_FUNCTION_ARGS);
extern Datum textcat_temporal_temporal(PG_FUNCTION_ARGS);
extern Datum temporal_upper(PG_FUNCTION_ARGS);
extern Datum temporal_lower(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
