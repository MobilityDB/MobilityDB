/*****************************************************************************
 *
 * ComparisonOps.c
 *	  Temporal comparison operators (=, <>, <, >, <=, >=).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

/*****************************************************************************
 * Generic dispatch functions
 *****************************************************************************/

static Temporal *
tcomp_temporal_base(Temporal *temp, Datum value, Oid datumtypid,
	Datum (*operator)(Datum, Datum, Oid, Oid), bool invert)
{
	Temporal *result = NULL;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)oper4_temporalinst_base((TemporalInst *)temp,
			value, operator, datumtypid, BOOLOID, invert);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)oper4_temporali_base((TemporalI *)temp,
			value, operator, datumtypid, BOOLOID, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = MOBDB_FLAGS_GET_CONTINUOUS(temp->flags) ?
			/* Result is a TemporalS */
			(Temporal *)oper4_temporalseq_base_crossdisc((TemporalSeq *)temp,
				value, operator, datumtypid, BOOLOID, invert) :
			/* Result is a TemporalSeq */
			(Temporal *)oper4_temporalseq_base((TemporalSeq *)temp,
				value, operator, datumtypid, BOOLOID, invert);
	else if (temp->type == TEMPORALS) 
		result = MOBDB_FLAGS_GET_CONTINUOUS(temp->flags) ?
			(Temporal *)oper4_temporals_base_crossdisc((TemporalS *)temp,
				value, operator, datumtypid, BOOLOID, invert) :
			(Temporal *)oper4_temporals_base((TemporalS *)temp,
				value, operator, datumtypid, BOOLOID, invert);
    else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));
	return result;
}

static Temporal *
tcomp_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum, Oid, Oid))
{
	Temporal *result = NULL;
	if (temp1->type == TEMPORALINST) 
		result = (Temporal *)oper4_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, operator, BOOLOID);
	else if (temp1->type == TEMPORALI) 
		result = (Temporal *)oper4_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,	operator, BOOLOID);
	else if (temp1->type == TEMPORALSEQ) 
	{
		bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
			MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
		result = continuous ?
			/* Result is a TemporalS */
			(Temporal *)oper4_temporalseq_temporalseq_crossdisc(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, BOOLOID) :
			/* Result is a TemporalSeq */
			(Temporal *)oper4_temporalseq_temporalseq(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, BOOLOID);
	}
	else if (temp1->type == TEMPORALS)
	{
		bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
			MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
		result = continuous ?
			(Temporal *)oper4_temporals_temporals_crossdisc(
				(TemporalS *)temp1, (TemporalS *)temp2, operator, BOOLOID) :
			(Temporal *)oper4_temporals_temporals(
				(TemporalS *)temp1, (TemporalS *)temp2, operator, BOOLOID);
	}
    else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

PG_FUNCTION_INFO_V1(teq_base_temporal);

PGDLLEXPORT Datum
teq_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_eq2, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(teq_temporal_base);

PGDLLEXPORT Datum
teq_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_eq2, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(teq_temporal_temporal_old);

PGDLLEXPORT Datum
teq_temporal_temporal_old(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tcomp_temporal_temporal(sync1, sync2, &datum2_eq2);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(teq_temporal_temporal);

PGDLLEXPORT Datum
teq_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *result = sync_oper4_temporal_temporal_crossdisc(temp1, temp2, 
		&datum2_eq2, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tne_base_temporal);

PGDLLEXPORT Datum
tne_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_ne2, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tne_temporal_base);

PGDLLEXPORT Datum
tne_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_ne2, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tne_temporal_temporal);

PGDLLEXPORT Datum
tne_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tcomp_temporal_temporal(sync1, sync2, &datum2_ne2);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tlt_base_temporal);

PGDLLEXPORT Datum
tlt_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_lt2, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tlt_temporal_base);

PGDLLEXPORT Datum
tlt_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_lt2, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tlt_temporal_temporal);

PGDLLEXPORT Datum
tlt_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tcomp_temporal_temporal(sync1, sync2, &datum2_lt2);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tle_base_temporal);

PGDLLEXPORT Datum
tle_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_le2, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tle_temporal_base);

PGDLLEXPORT Datum
tle_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_le2, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tle_temporal_temporal);

PGDLLEXPORT Datum
tle_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tcomp_temporal_temporal(sync1, sync2, &datum2_le2);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tgt_base_temporal);

PGDLLEXPORT Datum
tgt_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_gt2, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tgt_temporal_base);

PGDLLEXPORT Datum
tgt_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_gt2, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tgt_temporal_temporal);

PGDLLEXPORT Datum
tgt_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tcomp_temporal_temporal(sync1, sync2, &datum2_gt2);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tge_base_temporal);

PGDLLEXPORT Datum
tge_base_temporal(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_ge2, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tge_temporal_base);

PGDLLEXPORT Datum
tge_temporal_base(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum value = PG_GETARG_ANYDATUM(1);
	Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
	Temporal *result = tcomp_temporal_base(temp, value, datumtypid,
		&datum2_ge2, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tge_temporal_temporal);

PGDLLEXPORT Datum
tge_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return false if the temporal points do not intersect in time
	   The last parameter crossing must be set to false */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = tcomp_temporal_temporal(sync1, sync2, &datum2_ge2);
	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
