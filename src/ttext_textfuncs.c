/*****************************************************************************
 *
 * ttext_textfuncs.c
 *	Temporal text functions (textcat, lower, upper).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "ttext_textfuncs.h"

#include <utils/builtins.h>
#include "temporal.h"
#include "temporal_util.h"
#include "lifting.h"

/*****************************************************************************
 * Textual functions on datums
 *****************************************************************************/

/**
 * Returns the concatenation of the two text values
 */
static Datum
datum_textcat(Datum l, Datum r)
{
	return call_function2(textcat, l, r);
}

/**
 * Convert the text value to lowercase 
 */
static Datum
datum_lower(Datum value)
{
	return call_function1(lower, value);
}

/**
 * Convert the text value to uppercase 
 */
static Datum
datum_upper(Datum value)
{
	return call_function1(upper, value);
}

/*****************************************************************************
 * Text concatenation
 *****************************************************************************/

PG_FUNCTION_INFO_V1(textcat_base_ttext);
/**
 * Returns the concatenation of the text value and the temporal text values
 */
PGDLLEXPORT Datum
textcat_base_ttext(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tfunc_temporal_base(temp, value, TEXTOID, (Datum) NULL,
		(varfunc) &datum_textcat, 2, TEXTOID, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(textcat_ttext_base);
/**
 * Returns the concatenation of the temporal text value and the text value
 */
PGDLLEXPORT Datum
textcat_ttext_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_DATUM(1);
	Temporal *result = tfunc_temporal_base(temp, value, TEXTOID, (Datum) NULL,
		(varfunc) &datum_textcat, 2, TEXTOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(textcat_ttext_ttext);
/**
 * Returns the concatenation of the two temporal text values
 */
PGDLLEXPORT Datum
textcat_ttext_ttext(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
	 	(varfunc) &datum_textcat, 2, TEXTOID, false, false, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(ttext_upper);
/**
 * Transform the temporal text value into uppercase
 */
PGDLLEXPORT Datum
ttext_upper(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc_temporal(temp, (Datum) NULL,
		(varfunc) &datum_upper, 1, TEXTOID);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_lower);
/**
 * Transform the temporal text value into lowercase
 */
PGDLLEXPORT Datum
ttext_lower(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc_temporal(temp, (Datum) NULL,
		(varfunc) &datum_lower, 1, TEXTOID);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
