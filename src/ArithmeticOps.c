/*****************************************************************************
 *
 * ArithmeticOps.c
 *	  Temporal arithmetic operators (+, -, *, /).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

/*****************************************************************************
 * Arithmetic operations functions on datums
 *****************************************************************************/

/* Addition */

static Datum
datum_add(Datum l, Datum r, Oid typel, Oid typer)
{
	if (typel == INT4OID && typer == INT4OID)
		return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
	if (typel == INT4OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetInt32(l) + DatumGetFloat8(r));
	if (typel == FLOAT8OID && typer == INT4OID)
		return Float8GetDatum(DatumGetFloat8(l) + DatumGetInt32(r));
	if (typel == FLOAT8OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
		errmsg("Operation not supported")));
}


/* Subtraction */

static Datum
datum_sub(Datum l, Datum r, Oid typel, Oid typer)
{
	if (typel == INT4OID && typer == INT4OID)
		return Int32GetDatum(DatumGetInt32(l) - DatumGetInt32(r));
	if (typel == INT4OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetInt32(l) - DatumGetFloat8(r));
	if (typel == FLOAT8OID && typer == INT4OID)
		return Float8GetDatum(DatumGetFloat8(l) - DatumGetInt32(r));
	if (typel == FLOAT8OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetFloat8(l) - DatumGetFloat8(r));
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
		errmsg("Operation not supported")));
}

/* Multiplication */

static Datum
datum_mult(Datum l, Datum r, Oid typel, Oid typer)
{
	if (typel == INT4OID && typer == INT4OID)
		return Int32GetDatum(DatumGetInt32(l) * DatumGetInt32(r));
	if (typel == INT4OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetInt32(l) * DatumGetFloat8(r));
	if (typel == FLOAT8OID && typer == INT4OID)
		return Float8GetDatum(DatumGetFloat8(l) * DatumGetInt32(r));
	if (typel == FLOAT8OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetFloat8(l) * DatumGetFloat8(r));
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
		errmsg("Operation not supported")));
}

/* Division */

static Datum
datum_div(Datum l, Datum r, Oid typel, Oid typer)
{
	if (typel == INT4OID && typer == INT4OID)
		return Int32GetDatum(DatumGetInt32(l) / DatumGetInt32(r));
	if (typel == INT4OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetInt32(l) / DatumGetFloat8(r));
	if (typel == FLOAT8OID && typer == INT4OID)
		return Float8GetDatum(DatumGetFloat8(l) / DatumGetInt32(r));
	if (typel == FLOAT8OID && typer == FLOAT8OID)
		return Float8GetDatum(DatumGetFloat8(l) / DatumGetFloat8(r));
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
		errmsg("Operation not supported")));
}

/* Round to n decimal places */

static Datum
datum_round(Datum l, Datum r, Oid typel, Oid typer) {
	Assert(typel == FLOAT8OID && typer == INT4OID) ;
	double x = DatumGetFloat8(l) ;
	int n = DatumGetInt32(r) ;
	// using strings for truncating. not efficient but I don't know how to do it correctly numerically...
	char pattern[16] ;
	char str[128] ;
	sprintf(pattern, "%%.%df", n) ;
	sprintf(str, pattern, x) ;
	double x2 = strtod(str, NULL) ;
	return Float8GetDatum(x2) ;
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value, 
		 	&datum_add, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value, 
		 	&datum_add, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_add, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_add, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp1->valuetypid == temp2->valuetypid || temp1->type == TEMPORALINST || 
		temp1->type == TEMPORALI || temp2->type == TEMPORALINST || 
		temp2->type == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = sync_oper4_temporal_temporal(temp1, temp2, 
		 	&datum_add, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result = sync_oper4_temporal_temporal(ftemp1, temp2, 
		 	&datum_add, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result = sync_oper4_temporal_temporal(temp1, ftemp2, 
		 	&datum_add, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_sub, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_sub, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_sub, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_add, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp1->valuetypid == temp2->valuetypid || temp1->type == TEMPORALINST || 
		temp1->type == TEMPORALI || temp2->type == TEMPORALINST || 
		temp2->type == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = sync_oper4_temporal_temporal(temp1, temp2, 
		 	&datum_sub, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result = sync_oper4_temporal_temporal(ftemp1, temp2, 
		 	&datum_sub, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result = sync_oper4_temporal_temporal(temp1, ftemp2, 
		 	&datum_sub, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_mult, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_mult, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_mult, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_mult, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp1->valuetypid == temp2->valuetypid || temp1->type == TEMPORALINST || 
		temp1->type == TEMPORALI || temp2->type == TEMPORALINST || 
		temp2->type == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = crossings ?
			sync_oper4_temporal_temporal(temp1, temp2, 
		 		&datum_mult, valuetypid, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_oper4_temporal_temporal(temp1, temp2, 
		 		&datum_mult, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result =  crossings ?
			sync_oper4_temporal_temporal(ftemp1, temp2, 
		 		&datum_mult, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_oper4_temporal_temporal(ftemp1, temp2, 
		 		&datum_mult, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result =  crossings ?
			sync_oper4_temporal_temporal(temp1, ftemp2, 
		 		&datum_mult, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_oper4_temporal_temporal(temp1, ftemp2, 
		 		&datum_mult, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_div, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_div, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp->valuetypid == datumtypid || temp->type == TEMPORALINST || 
		temp->type == TEMPORALI)
 		result = oper4_temporal_base(temp, value,
		 	&datum_div, datumtypid, valuetypid, true);
	else if (datumtypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_as_tfloat_internal(temp);
		result = oper4_temporal_base(ftemp, value,
		 	&datum_div, FLOAT8OID, FLOAT8OID, true);
		pfree(ftemp);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
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
	Temporal *result;
	if (temp1->valuetypid == temp2->valuetypid || temp1->type == TEMPORALINST || 
		temp1->type == TEMPORALI || temp2->type == TEMPORALINST || 
		temp2->type == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid valuetypid = base_oid_from_temporal(temptypid);
 		result = crossings ?
			sync_oper4_temporal_temporal(temp1, temp2, 
		 		&datum_div, valuetypid, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_oper4_temporal_temporal(temp1, temp2, 
		 		&datum_div, valuetypid, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_as_tfloat_internal(temp1);
		result =  crossings ?
			sync_oper4_temporal_temporal(ftemp1, temp2, 
		 		&datum_div, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_oper4_temporal_temporal(ftemp1, temp2, 
		 		&datum_div, FLOAT8OID, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_as_tfloat_internal(temp2);
		result =  crossings ?
			sync_oper4_temporal_temporal(temp1, ftemp2, 
		 		&datum_div, FLOAT8OID, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_oper4_temporal_temporal(temp1, ftemp2, 
		 		&datum_div, FLOAT8OID, NULL);
		pfree(ftemp2);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/


PG_FUNCTION_INFO_V1(round_temporal);

PGDLLEXPORT Datum
round_temporal(PG_FUNCTION_ARGS)
{
	//Datum value = PG_GETARG_DATUM(1);
	int digits = PG_GETARG_INT32(1) ;
	//Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	//double d = datum_double(value, datumtypid);

	Temporal *temp = PG_GETARG_TEMPORAL(0);

	Temporal *result;
	result = oper4_temporal_base(temp, Int32GetDatum(digits),
		&datum_round, INT4OID, FLOAT8OID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}
