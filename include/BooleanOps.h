/*****************************************************************************
 *
 * BooleanOps.h
 *	  Temporal Boolean operators (and, or, not).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __BOOLEANOPS_H__
#define __BOOLEANOPS_H__

#include <postgres.h>

/*****************************************************************************/

extern Datum datum_and(Datum l, Datum r);
extern Datum datum_or(Datum l, Datum r);

extern Datum tand_bool_tbool(PG_FUNCTION_ARGS);
extern Datum tand_tbool_bool(PG_FUNCTION_ARGS);
extern Datum tand_tbool_tbool(PG_FUNCTION_ARGS);

extern Datum tor_bool_tbool(PG_FUNCTION_ARGS);
extern Datum tor_tbool_bool(PG_FUNCTION_ARGS);
extern Datum tor_tbool_tbool(PG_FUNCTION_ARGS);

extern Datum tnot_tbool(PG_FUNCTION_ARGS);


/*****************************************************************************/

#endif
