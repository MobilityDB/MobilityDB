/*****************************************************************************
 *
 * temporal_mathfuncs.c
 *	Temporal mathematical operators (+, -, *, /) and functions (round, 
 *	degrees).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#ifndef __TEMPORAL_MATHFUNCS_H__
#define __TEMPORAL_MATHFUNCS_H__

#include <postgres.h>
#include <catalog/pg_type.h>

/*****************************************************************************/

extern Datum datum_round(Datum value, Datum prec);

extern Datum add_base_temporal(PG_FUNCTION_ARGS);
extern Datum add_temporal_base(PG_FUNCTION_ARGS);
extern Datum add_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum sub_base_temporal(PG_FUNCTION_ARGS);
extern Datum sub_temporal_base(PG_FUNCTION_ARGS);
extern Datum sub_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum mult_base_temporal(PG_FUNCTION_ARGS);
extern Datum mult_temporal_base(PG_FUNCTION_ARGS);
extern Datum mult_temporal_temporal(PG_FUNCTION_ARGS);

extern Datum div_base_temporal(PG_FUNCTION_ARGS);
extern Datum div_temporal_base(PG_FUNCTION_ARGS);
extern Datum div_temporal_temporal(PG_FUNCTION_ARGS);

/*****************************************************************************/

#endif
