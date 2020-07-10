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
tnumberseq_mult_maxmin_at_timestamp(const TemporalInst *start1, const TemporalInst *end1,
	const TemporalInst *start2, const TemporalInst *end2, TimestampTz *t)
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
	if ((x2 - x1) == 0.0 || (x4 - x3) == 0.0)
		return false;

	long double d1 = (-1 * x1) / (x2 - x1);
	long double d2 = (-1 * x3) / (x4 - x3);
	long double min = Min(d1, d2);
	long double max = Max(d1, d2);
	long double fraction = min + (max - min)/2;
	long double duration = (long double) (end1->t - start1->t);
	if (fraction <= EPSILON || fraction >= (1.0 - EPSILON))
		/* Minimum/maximum occurs out of the period */
		return false;

	*t = start1->t + (long) (duration * fraction);
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
	/* Result will never be null due to the denominator test above */
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

/***********************************************************************
 * Simple Douglas-Peucker-like value simplification for temporal floats.
 ***********************************************************************/

static void
tfloatseq_dp_findsplit(const TemporalSeq *seq, int i1, int i2,
	int *split, double *dist)
{
	*split = i1;
	*dist = -1;
	if (i1 + 1 < i2)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq, i1);
		TemporalInst *inst2 = temporalseq_inst_n(seq, i2);
		double start = DatumGetFloat8(temporalinst_value(inst1));
		double end = DatumGetFloat8(temporalinst_value(inst2));
		double duration2 = (double) (inst2->t - inst1->t);
		for (int k = i1 + 1; k < i2; k++)
		{
			inst2 = temporalseq_inst_n(seq, k);
			double value = DatumGetFloat8(temporalinst_value(inst2));
			double duration1 = (double) (inst2->t - inst1->t);
			double ratio = duration1 / duration2;
			double value_interp = start + (end - start) * ratio;
			double d = fabs(value - value_interp);
			if (d > *dist)
			{
				/* record the maximum */
				*split = k;
				*dist = d;
			}
		}
	}
}

static int
int_cmp(const void *a, const void *b)
{
	/* casting pointer types */
	const int *ia = (const int *)a;
	const int *ib = (const int *)b;
	/* returns negative if b > a and positive if a > b */
	return *ia - *ib;
}

TemporalSeq *
tfloatseq_simplify(const TemporalSeq *seq, double eps_dist, uint32_t minpts)
{
	static size_t stack_size = 256;
	int *stack, *outlist; /* recursion stack */
	int stack_static[stack_size];
	int outlist_static[stack_size];
	int sp = -1; /* recursion stack pointer */
	int i1, split;
	uint32_t outn = 0;
	uint32_t i;
	double dist;

	/* Do not try to simplify really short things */
	if (seq->count < 3)
		return temporalseq_copy(seq);

	/* Only heap allocate book-keeping arrays if necessary */
	if ((unsigned int) seq->count > stack_size)
	{
		stack = palloc(sizeof(int) * seq->count);
		outlist = palloc(sizeof(int) * seq->count);
	}
	else
	{
		stack = stack_static;
		outlist = outlist_static;
	}

	i1 = 0;
	stack[++sp] = seq->count - 1;
	/* Add first point to output list */
	outlist[outn++] = 0;
	do
	{
		tfloatseq_dp_findsplit(seq, i1, stack[sp], &split, &dist);
		bool dosplit;
		dosplit = (dist >= 0 &&
			(dist > eps_dist || outn + sp + 1 < minpts));
		if (dosplit)
			stack[++sp] = split;
		else
		{
			outlist[outn++] = stack[sp];
			i1 = stack[sp--];
		}
	}
	while (sp >= 0);

	/* Put list of retained points into order */
	qsort(outlist, outn, sizeof(int), int_cmp);
	/* Create new TemporalSeq */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * outn);
	for (i = 0; i < outn; i++)
		instants[i] = temporalseq_inst_n(seq, outlist[i]);
	TemporalSeq *result = temporalseq_make(instants, outn,
		seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	pfree(instants);

	/* Only free if arrays are on heap */
	if (stack != stack_static)
		pfree(stack);
	if (outlist != outlist_static)
		pfree(outlist);

	return result;
}

TemporalS *
tfloats_simplify(const TemporalS *ts, double eps_dist, uint32_t minpts)
{
	TemporalS *result;
	/* Singleton sequence set */
	if (ts->count == 1)
	{
        TemporalSeq *seq = tfloatseq_simplify(temporals_seq_n(ts, 0), eps_dist, minpts);
		result = temporalseq_to_temporals(seq);
		pfree(seq);
		return result;
	}

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
		sequences[i] = tfloatseq_simplify(temporals_seq_n(ts, i), eps_dist, minpts);
	result = temporals_make(sequences, ts->count, true);
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

PG_FUNCTION_INFO_V1(tfloat_simplify);

Datum
tfloat_simplify(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	double eps_dist = PG_GETARG_FLOAT8(1);

	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST || temp->duration == TEMPORALI ||
		! MOBDB_FLAGS_GET_LINEAR(temp->flags))
		result = temporal_copy(temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *) tfloatseq_simplify((TemporalSeq *)temp,
			eps_dist, 2);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *) tfloats_simplify((TemporalS *)temp,
			eps_dist, 2);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
