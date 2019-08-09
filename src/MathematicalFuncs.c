/*****************************************************************************
 *
 * MathematicalFuncs.c
 *	Temporal mathematical operators (+, -, *, /) and functions (round, 
 *	degrees).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <postgres.h>
#include <catalog/pg_type.h>
#include <utils/builtins.h>
#include <utils/rangetypes.h>

#include "TemporalTypes.h"
#include "LiftingFuncs.h"

/*****************************************************************************
 * Mathematical functions on datums
 * Since these functions are static, there is no need to verify the validity
 * of the Oids passed as arguments as this has been done in the calling
 * function.
 *****************************************************************************/

/* Addition */

static Datum
datum_add(Datum l, Datum r, Oid typel, Oid typer)
{
	Datum result = 0;
	if (typel == INT4OID && typer == INT4OID)
		result = Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
	else if (typel == INT4OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetInt32(l) + DatumGetFloat8(r));
	else if (typel == FLOAT8OID && typer == INT4OID)
		result = Float8GetDatum(DatumGetFloat8(l) + DatumGetInt32(r));
	else if (typel == FLOAT8OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
	return result;
}

/* Subtraction */

static Datum
datum_sub(Datum l, Datum r, Oid typel, Oid typer)
{
	Datum result = 0;
	if (typel == INT4OID && typer == INT4OID)
		result = Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
	else if (typel == INT4OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetInt32(l) - DatumGetFloat8(r));
	else if (typel == FLOAT8OID && typer == INT4OID)
		result = Float8GetDatum(DatumGetFloat8(l) - DatumGetInt32(r));
	else if (typel == FLOAT8OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
	return result;
}

/* Multiplication */

static Datum
datum_mult(Datum l, Datum r, Oid typel, Oid typer)
{
	Datum result = 0;
	if (typel == INT4OID && typer == INT4OID)
		result = Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
	else if (typel == INT4OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetInt32(l) * DatumGetFloat8(r));
	else if (typel == FLOAT8OID && typer == INT4OID)
		result = Float8GetDatum(DatumGetFloat8(l) * DatumGetInt32(r));
	else if (typel == FLOAT8OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
	return result;
}

/* Division */

static Datum
datum_div(Datum l, Datum r, Oid typel, Oid typer)
{
	Datum result = 0;
	if (typel == INT4OID && typer == INT4OID)
		result = Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
	else if (typel == INT4OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetInt32(l) / DatumGetFloat8(r));
	else if (typel == FLOAT8OID && typer == INT4OID)
		result = Float8GetDatum(DatumGetFloat8(l) / DatumGetInt32(r));
	else if (typel == FLOAT8OID && typer == FLOAT8OID)
		result = Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
	return result;
}

/* Round to n decimal places */

Datum
datum_round(Datum value, Datum prec)
{
	Datum numeric = call_function1(float8_numeric, value);
	Datum round = call_function2(numeric_round, numeric, prec);
	return call_function1(numeric_float8, round);
}

/* Convert to degrees */

static Datum
datum_degrees(Datum value)
{
	return call_function1(degrees, value);
}

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

PG_FUNCTION_INFO_V1(add_base_temporal);

PGDLLEXPORT Datum
add_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, 
		 	&datum_add, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, 
		 	&datum_add, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}


PG_FUNCTION_INFO_V1(add_temporal_base);

PGDLLEXPORT Datum
add_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_DATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_add, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_add, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(add_temporal_temporal);

PGDLLEXPORT Datum
add_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	/* The base types must be equal when the result is a temporal sequence (set) */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp1->duration);
	temporal_duration_is_valid(temp2->duration);
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST || 
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = sync_tfunc4_temporal_temporal(temp1, temp2, 
		 	&datum_add, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result = sync_tfunc4_temporal_temporal(ftemp1, temp2, 
		 	&datum_add, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result = sync_tfunc4_temporal_temporal(temp1, ftemp2, 
		 	&datum_add, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

PG_FUNCTION_INFO_V1(sub_base_temporal);

PGDLLEXPORT Datum
sub_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_sub, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_sub, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(sub_temporal_base);

PGDLLEXPORT Datum
sub_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_DATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_sub, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_add, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(sub_temporal_temporal);

PGDLLEXPORT Datum
sub_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	/* The base types must be equal when the result is a temporal sequence (set) */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp1->duration);
	temporal_duration_is_valid(temp2->duration);
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST || 
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = sync_tfunc4_temporal_temporal(temp1, temp2, 
		 	&datum_sub, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result = sync_tfunc4_temporal_temporal(ftemp1, temp2, 
		 	&datum_sub, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result = sync_tfunc4_temporal_temporal(temp1, ftemp2, 
		 	&datum_sub, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

PG_FUNCTION_INFO_V1(mult_base_temporal);

PGDLLEXPORT Datum
mult_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_mult, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_mult, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(mult_temporal_base);

PGDLLEXPORT Datum
mult_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_DATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_mult, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_mult, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(mult_temporal_temporal);

PGDLLEXPORT Datum
mult_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	bool crossings = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
		MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	/* The base types must be equal when the result is a temporal sequence (set) */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp1->duration);
	temporal_duration_is_valid(temp2->duration);
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST || 
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = crossings ?
			sync_tfunc4_temporal_temporal(temp1, temp2, 
		 		&datum_mult, valuetypid, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, temp2, 
		 		&datum_mult, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result =  crossings ?
			sync_tfunc4_temporal_temporal(ftemp1, temp2, 
		 		&datum_mult, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(ftemp1, temp2, 
		 		&datum_mult, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result =  crossings ?
			sync_tfunc4_temporal_temporal(temp1, ftemp2, 
		 		&datum_mult, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, ftemp2, 
		 		&datum_mult, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

PG_FUNCTION_INFO_V1(div_base_temporal);

PGDLLEXPORT Datum
div_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(0);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	double d = datum_double(value, datumtypid);
	if (fabs(d) < EPSILON)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Division by zero")));
	
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_div, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_div, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(div_temporal_base);

PGDLLEXPORT Datum
div_temporal_base(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_DATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	double d = datum_double(value, datumtypid);
	if (fabs(d) < EPSILON)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Division by zero")));

	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp->duration);
	number_base_type_oid(datumtypid);
	if (temp->valuetypid == datumtypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value,
		 	&datum_div, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value,
		 	&datum_div, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(div_temporal_temporal);

PGDLLEXPORT Datum
div_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	bool crossings = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
		MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	/* The base types must be equal when the result is a temporal sequence (set) */
	Temporal *result = NULL;
	temporal_duration_is_valid(temp1->duration);
	temporal_duration_is_valid(temp2->duration);
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST || 
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = crossings ?
			sync_tfunc4_temporal_temporal(temp1, temp2, 
		 		&datum_div, valuetypid, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, temp2, 
		 		&datum_div, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result =  crossings ?
			sync_tfunc4_temporal_temporal(ftemp1, temp2, 
		 		&datum_div, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(ftemp1, temp2, 
		 		&datum_div, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result =  crossings ?
			sync_tfunc4_temporal_temporal(temp1, ftemp2, 
		 		&datum_div, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, ftemp2, 
		 		&datum_div, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(temporal_round);

PGDLLEXPORT Datum
temporal_round(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum digits = PG_GETARG_DATUM(1);
	Temporal *result = tfunc2_temporal(temp, digits, &datum_round, FLOAT8OID, 
		false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_degrees);

PGDLLEXPORT Datum
temporal_degrees(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &datum_degrees, FLOAT8OID,
		false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
