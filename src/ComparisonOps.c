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
	Datum (*func)(Datum, Datum, Oid, Oid), bool invert)
{
	Temporal *result = NULL;
	if (temp->type == TEMPORALINST) 
		result = (Temporal *)tfunc4_temporalinst_base((TemporalInst *)temp,
			value, func, datumtypid, BOOLOID, invert);
	else if (temp->type == TEMPORALI) 
		result = (Temporal *)tfunc4_temporali_base((TemporalI *)temp,
			value, func, datumtypid, BOOLOID, invert);
	else if (temp->type == TEMPORALSEQ) 
		result = MOBDB_FLAGS_GET_CONTINUOUS(temp->flags) ?
			/* Result is a TemporalS */
			(Temporal *)tfunc4_temporalseq_base_crossdisc((TemporalSeq *)temp,
				value, func, datumtypid, BOOLOID, invert) :
			/* Result is a TemporalSeq */
			(Temporal *)tfunc4_temporalseq_base((TemporalSeq *)temp,
				value, func, datumtypid, BOOLOID, invert);
	else if (temp->type == TEMPORALS) 
		result = MOBDB_FLAGS_GET_CONTINUOUS(temp->flags) ?
			(Temporal *)tfunc4_temporals_base_crossdisc((TemporalS *)temp,
				value, func, datumtypid, BOOLOID, invert) :
			(Temporal *)tfunc4_temporals_base((TemporalS *)temp,
				value, func, datumtypid, BOOLOID, invert);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));
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

PG_FUNCTION_INFO_V1(teq_temporal_temporal);

PGDLLEXPORT Datum
teq_temporal_temporal(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *result = sync_tfunc4_temporal_temporal_crossdisc(temp1, temp2, 
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
	Temporal *result = sync_tfunc4_temporal_temporal_crossdisc(temp1, temp2, 
		&datum2_ne2, BOOLOID);
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
	Temporal *result = sync_tfunc4_temporal_temporal_crossdisc(temp1, temp2, 
		&datum2_lt2, BOOLOID);
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
	Temporal *result = sync_tfunc4_temporal_temporal_crossdisc(temp1, temp2, 
		&datum2_le2, BOOLOID);
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
	Temporal *result = sync_tfunc4_temporal_temporal_crossdisc(temp1, temp2, 
		&datum2_gt2, BOOLOID);
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
	Temporal *result = sync_tfunc4_temporal_temporal_crossdisc(temp1, temp2, 
		&datum2_ge2, BOOLOID);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
