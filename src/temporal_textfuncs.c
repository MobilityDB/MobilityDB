/*****************************************************************************
 *
 * temporal_textfuncs.c
 *	Text functions (textcat, lower, upper).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_textfuncs.h"

#include <utils/builtins.h>
#include "temporal.h"
#include "temporal_util.h"
#include "lifting.h"

/*****************************************************************************
 * Mathematical functions on datums
 *****************************************************************************/

/* String textcatenation */

static Datum
datum_textcat(Datum l, Datum r)
{
	return call_function2(textcat, l, r);
}

/* Convert to upper/lower case */

static Datum
datum_lower(Datum value)
{
	return call_function1(lower, value);
}

static Datum
datum_upper(Datum value)
{
	return call_function1(upper, value);
}

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(textcat_base_temporal);

PGDLLEXPORT Datum
textcat_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tfunc2_temporal_base(temp, value, 
	 	&datum_textcat, TEXTOID, false, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(textcat_temporal_base);

PGDLLEXPORT Datum
textcat_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_DATUM(1);
	Temporal *result = tfunc2_temporal_base(temp, value, 
	 	&datum_textcat, TEXTOID, false, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(textcat_temporal_temporal);

PGDLLEXPORT Datum
textcat_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *result = sync_tfunc2_temporal_temporal(temp1, temp2, 
	 	&datum_textcat, TEXTOID, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_upper);

PGDLLEXPORT Datum
temporal_upper(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &datum_upper, TEXTOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_lower);

PGDLLEXPORT Datum
temporal_lower(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &datum_lower, TEXTOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
