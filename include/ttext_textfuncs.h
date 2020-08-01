/*****************************************************************************
 *
 * ttext_textfuncs.h
 *		Text functions (textcat, lower, upper).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TTEXT_TEXTFUNCS_H__
#define __TTEXT_TEXTFUNCS_H__

#include <postgres.h>
#include <fmgr.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum textcat_base_ttext(PG_FUNCTION_ARGS);
extern Datum textcat_ttext_base(PG_FUNCTION_ARGS);
extern Datum textcat_ttext_ttext(PG_FUNCTION_ARGS);
extern Datum ttext_upper(PG_FUNCTION_ARGS);
extern Datum ttext_lower(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
