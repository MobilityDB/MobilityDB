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
	Temporal *result = oper4_temporal_base(value, temp, 
		&datum_add, datumtypid, valuetypid, true);
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
	Temporal *result = oper4_temporal_base(value, temp, 
		&datum_add, datumtypid, valuetypid, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(add_temporal_temporal_old);

PGDLLEXPORT Datum
add_temporal_temporal_old(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time 
	   No crossings added */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	Temporal *result = oper4_temporal_temporal(sync1, sync2, 
		&datum_add, valuetypid);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(add_temporal_temporal);

PGDLLEXPORT Datum
add_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	Temporal *result = sync_oper4_temporal_temporal(temp1, temp2,
		&datum_add, valuetypid, false);
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
	Temporal *result = oper4_temporal_base(value, temp, 
		&datum_sub, datumtypid, valuetypid, true);
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
	Temporal *result = oper4_temporal_base(value, temp, 
		&datum_sub, datumtypid, valuetypid, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(sub_temporal_temporal);

PGDLLEXPORT Datum
sub_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time 
	   No crossings added */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	Temporal *result = oper4_temporal_temporal(sync1, sync2, 
		&datum_sub, valuetypid);
	pfree(sync1); pfree(sync2); 
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
	Temporal *result = oper4_temporal_base(value, temp, 
		&datum_mult, datumtypid, valuetypid, true);
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
	Temporal *result = oper4_temporal_base(value, temp, 
		&datum_mult, datumtypid, valuetypid, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(mult_temporal_temporal);

PGDLLEXPORT Datum
mult_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	bool crossings = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
			MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	/* Return NULL if the temporal points do not intersect in time 
	   Crossings are added if both arguments are continuous */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, crossings))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	Temporal *result = oper4_temporal_temporal(sync1, sync2, 
		&datum_mult, valuetypid);
	pfree(sync1); pfree(sync2); 
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
	Temporal *result;
	result = oper4_temporal_base(value, temp,
		&datum_div, datumtypid, valuetypid, true);
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
	Temporal *result;
	result = oper4_temporal_base(value, temp,
		&datum_div, datumtypid, valuetypid, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(div_temporal_temporal);

PGDLLEXPORT Datum
div_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	bool crossings = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
		MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	/* Return NULL if the temporal points do not intersect in time 
	   Crossings are added if at least one of the arguments is continuous */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, crossings))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid valuetypid = base_oid_from_temporal(temptypid);
	Temporal *result = oper4_temporal_temporal(sync1, sync2, 
		&datum_div, valuetypid);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
