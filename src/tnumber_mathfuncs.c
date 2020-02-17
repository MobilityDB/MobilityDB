/*****************************************************************************
 *
 * tnumber_mathfuncs.c
 *	Temporal mathematical operators (+, -, *, /) and functions (round, 
 *	degrees).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tnumber_mathfuncs.h"

#include <math.h>
#include <utils/builtins.h>

#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "temporal_util.h"
#include "lifting.h"

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
 * Find the single timestamptz at which the multiplication of two temporal 
 * number segments is at a local minimum/maximum. The function supposes that 
 * the instants are synchronized, that is  start1->t = start2->t and 
 * end1->t = end2->t. The function only return an intersection at the middle,
 * that is, it returns false if the timestamp found is not at a bound.
 *****************************************************************************/

static bool
tnumberseq_mult_maxmin_at_timestamp(TemporalInst *start1, TemporalInst *end1,
	TemporalInst *start2, TemporalInst *end2, TimestampTz *t)
{
	double x1 = datum_double(temporalinst_value(start1), start1->valuetypid);
	double x2 = datum_double(temporalinst_value(end1), start1->valuetypid);
	double x3 = datum_double(temporalinst_value(start2), start2->valuetypid);
	double x4 = datum_double(temporalinst_value(end2), start2->valuetypid);
	/* Compute the instants t1 and t2 at which the linear functions of the two
	   segments take the value 0: at1 + b = 0, ct2 + d = 0. There is a
	   minimum/maximum exactly at the middle between t1 and t2.
	   To reduce problems related to floating point arithmetic, t1 and t2
	   are shifted, respectively, to 0 and 1 before the computation */
	if ((x2 - x1) == 0 || (x4 - x3) == 0)
		return false;

	double d1 = (-1 * x1) / (x2 - x1);
	double d2 = (-1 * x3) / (x4 - x3);
	double min = Min(d1, d2);
	double max = Max(d1, d2);
	double fraction = min + (max - min)/2;
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		/* Minimum/maximum occurs out of the period */
		return false;

	*t = start1->t + (long) ((double) (end1->t - start1->t) * fraction);
	return true;	
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_add, restypid, true);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_add, FLOAT8OID, true);
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_add, restypid, false);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_add, FLOAT8OID, false);
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

	/* Bounding box test */
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	if (! overlaps_period_period_internal(&p1, &p2))
		PG_RETURN_NULL();

	/* The base types must be equal when the result is a temporal sequence (set) */
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = NULL;
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST ||
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid restypid = base_oid_from_temporal(temptypid);
 		result = sync_tfunc4_temporal_temporal(temp1, temp2, &datum_add,
			restypid, linear, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_to_tfloat_internal(temp1);
		result = sync_tfunc4_temporal_temporal(ftemp1, temp2, &datum_add,
		 	FLOAT8OID, linear, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_to_tfloat_internal(temp2);
		result = sync_tfunc4_temporal_temporal(temp1, ftemp2, &datum_add,
		 	FLOAT8OID, linear, NULL);
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_sub, restypid, true);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_sub, FLOAT8OID, true);
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_sub, restypid, false);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_add, FLOAT8OID, false);
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

	/* Bounding box test */
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	if (! overlaps_period_period_internal(&p1, &p2))
		PG_RETURN_NULL();

	/* The base types must be equal when the result is a temporal sequence (set) */
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = NULL;
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST ||
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid restypid = base_oid_from_temporal(temptypid);
 		result = sync_tfunc4_temporal_temporal(temp1, temp2, &datum_sub,
			restypid, linear, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_to_tfloat_internal(temp1);
		result = sync_tfunc4_temporal_temporal(ftemp1, temp2, &datum_sub,
		 	FLOAT8OID, linear, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_to_tfloat_internal(temp2);
		result = sync_tfunc4_temporal_temporal(temp1, ftemp2, &datum_sub,
		 	FLOAT8OID, linear, NULL);
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_mult, restypid, true);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_mult, FLOAT8OID, true);
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_mult, restypid, false);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_mult, FLOAT8OID, false);
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

	/* Bounding box test */
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	if (! overlaps_period_period_internal(&p1, &p2))
		PG_RETURN_NULL();

	/* The base types must be equal when the result is a temporal sequence (set) */
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = NULL;
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST ||
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid restypid = base_oid_from_temporal(temptypid);
 		result = linear ?
			sync_tfunc4_temporal_temporal(temp1, temp2, &datum_mult,
				restypid, linear, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, temp2, &datum_mult,
				restypid, linear, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_to_tfloat_internal(temp1);
		result =  linear ?
			sync_tfunc4_temporal_temporal(ftemp1, temp2, &datum_mult,
		 		FLOAT8OID, linear, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(ftemp1, temp2, &datum_mult,
		 		FLOAT8OID, linear, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_to_tfloat_internal(temp2);
		result =  linear ?
			sync_tfunc4_temporal_temporal(temp1, ftemp2, &datum_mult, 
		 		FLOAT8OID, linear, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, ftemp2, &datum_mult, 
		 		FLOAT8OID, linear, NULL);
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
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	/* Test whether the denominator will ever be zero */
	if (temporal_ever_eq_internal(temp, Float8GetDatum(0.0)))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Division by zero")));

	Datum value = PG_GETARG_DATUM(0);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
		 	&datum_div, restypid, true);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
		 	&datum_div, FLOAT8OID, true);
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
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	double d = datum_double(value, valuetypid);
	if (fabs(d) < EPSILON)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Division by zero")));

	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
	Oid restypid = base_oid_from_temporal(temptypid);
	/* The base type and the argument type must be equal for temporal sequences */
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	ensure_numeric_base_type(valuetypid);
	if (temp->valuetypid == valuetypid || temp->duration == TEMPORALINST || 
		temp->duration == TEMPORALI)
 		result = tfunc4_temporal_base(temp, value, valuetypid,
			&datum_div, restypid, false);
	else if (valuetypid == FLOAT8OID && temp->valuetypid == INT4OID)
	{
		Temporal *ftemp = tint_to_tfloat_internal(temp);
		result = tfunc4_temporal_base(ftemp, value, FLOAT8OID,
			&datum_div, FLOAT8OID, false);
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

	/* Bounding box test */
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	if (! overlaps_period_period_internal(&p1, &p2))
		PG_RETURN_NULL();

	/* Test whether the denominator will ever be zero during the common timespan */
	PeriodSet *ps = temporal_get_time_internal(temp1);
	Temporal *projtemp2 = temporal_at_periodset_internal(temp2, ps);
    if (projtemp2 == NULL)
        PG_RETURN_NULL();
	if (temporal_ever_eq_internal(projtemp2, Float8GetDatum(0.0)))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Division by zero")));

	/* The base types must be equal when the result is a temporal sequence (set) */
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = NULL;
	if (temp1->valuetypid == temp2->valuetypid || temp1->duration == TEMPORALINST ||
		temp1->duration == TEMPORALI || temp2->duration == TEMPORALINST || 
		temp2->duration == TEMPORALI)
	{
		Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
		Oid restypid = base_oid_from_temporal(temptypid);
 		result = linear ?
			sync_tfunc4_temporal_temporal(temp1, temp2, &datum_div,
				restypid, linear, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, temp2, &datum_div,
				restypid, linear, NULL);
	}
	else if (temp1->valuetypid == INT4OID && temp2->valuetypid == FLOAT8OID)
	{
		Temporal *ftemp1 = tint_to_tfloat_internal(temp1);
		result =  linear ?
			sync_tfunc4_temporal_temporal(ftemp1, temp2, &datum_div,
		 		FLOAT8OID, linear, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(ftemp1, temp2, &datum_div,
		 		FLOAT8OID, linear, NULL);
		pfree(ftemp1);
	}
	else if (temp1->valuetypid == FLOAT8OID && temp2->valuetypid == INT4OID)
	{
		Temporal *ftemp2 = tint_to_tfloat_internal(temp2);
		result =  linear ?
			sync_tfunc4_temporal_temporal(temp1, ftemp2, &datum_div,
		 		FLOAT8OID, linear, &tnumberseq_mult_maxmin_at_timestamp) :
			sync_tfunc4_temporal_temporal(temp1, ftemp2, &datum_div,
		 		FLOAT8OID, linear, NULL);
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
	Temporal *result = tfunc2_temporal(temp, digits, &datum_round, FLOAT8OID);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_degrees);

PGDLLEXPORT Datum
temporal_degrees(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tfunc1_temporal(temp, &datum_degrees, FLOAT8OID);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
