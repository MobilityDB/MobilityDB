/*****************************************************************************
 *
 * temporal_boolops.c
 *	  Temporal Boolean operators (and, or, not).
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_boolops.h"

#include "temporaltypes.h"
#include "lifting.h"

/*****************************************************************************
 * Boolean operations functions on datums
 *****************************************************************************/

/**
 * Returns the Boolean and of the two values
 */
Datum
datum_and(Datum l, Datum r)
{
	return BoolGetDatum(DatumGetBool(l) && DatumGetBool(r));
}


/**
 * Returns the Boolean or of the two values
 */
Datum
datum_or(Datum l, Datum r)
{
	return BoolGetDatum(DatumGetBool(l) || DatumGetBool(r));
}

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tand_bool_tbool);
/**
 * Returns the temporal boolean and of the value and the temporal value
 */
PGDLLEXPORT Datum
tand_bool_tbool(PG_FUNCTION_ARGS)
{
	Datum b = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tfunc_temporal_base(temp, b, BOOLOID, (Datum) NULL,
		(varfunc) &datum_and, 2, BOOLOID, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tand_tbool_bool);
/**
 * Returns the temporal boolean and of the temporal value and the value
 */
PGDLLEXPORT Datum
tand_tbool_bool(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum b = PG_GETARG_DATUM(1);
	Temporal *result = tfunc_temporal_base(temp, b, BOOLOID, (Datum) NULL,
		(varfunc) &datum_and, 2, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tand_tbool_tbool);
/**
 * Returns the temporal boolean and of the temporal values
 */
PGDLLEXPORT Datum
tand_tbool_tbool(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
		(varfunc) &datum_and, 2, BOOLOID, false, false, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tor_bool_tbool);
/**
 * Returns the temporal boolean or of the value and the temporal value
 */
PGDLLEXPORT Datum
tor_bool_tbool(PG_FUNCTION_ARGS)
{
	Datum b = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = tfunc_temporal_base(temp, b, BOOLOID, (Datum) NULL,
		(varfunc) &datum_or, 2, BOOLOID, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tor_tbool_bool);
/**
 * Returns the temporal boolean or of the temporal value and the value
 */
PGDLLEXPORT Datum
tor_tbool_bool(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum b = PG_GETARG_DATUM(1);
	Temporal *result = tfunc_temporal_base(temp, b, BOOLOID, (Datum) NULL,
		(varfunc) &datum_or, 2, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tor_tbool_tbool);
/**
 * Returns the temporal boolean or of the temporal values
 */
PGDLLEXPORT Datum
tor_tbool_tbool(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *result = sync_tfunc_temporal_temporal(temp1, temp2, (Datum) NULL,
		(varfunc) &datum_or, 2, BOOLOID, false, false, NULL);
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

/**
 * Returns the temporal boolean not of the temporal value
 */
static TemporalInst *
tnot_tboolinst(const TemporalInst *inst)
{
	TemporalInst *result = temporalinst_copy(inst);
	Datum *value_ptr = temporalinst_value_ptr(result);
	*value_ptr = BoolGetDatum(!DatumGetBool(temporalinst_value(inst)));
	return result;
}

/**
 * Returns the temporal boolean not of the temporal value
 */
static TemporalI *
tnot_tbooli(const TemporalI *ti)
{
	TemporalI *result = temporali_copy(ti);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(result, i);
		Datum *value_ptr = temporalinst_value_ptr(inst);
		*value_ptr = BoolGetDatum(!DatumGetBool(temporalinst_value(inst)));
	}
	return result;

}

/**
 * Returns the temporal boolean not of the temporal value
 */
static TemporalSeq *
tnot_tboolseq(const TemporalSeq *seq)
{
	TemporalSeq *result = temporalseq_copy(seq);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(result, i);
		Datum *value_ptr = temporalinst_value_ptr(inst);
		*value_ptr = BoolGetDatum(!DatumGetBool(temporalinst_value(inst)));
	}
	return result;
}

/**
 * Returns the temporal boolean not of the temporal value
 */
static TemporalS *
tnot_tbools(const TemporalS *ts)
{
	TemporalS *result = temporals_copy(ts);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(result, i);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			Datum *value_ptr = temporalinst_value_ptr(inst);
			*value_ptr = BoolGetDatum(!DatumGetBool(temporalinst_value(inst)));
		}
	}
	return result;
}

/**
 * Returns the temporal boolean not of the temporal value
 * (dispatch function)
 */
Temporal *
tnot_tbool_internal(const Temporal *temp)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tnot_tboolinst((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tnot_tbooli((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tnot_tboolseq((TemporalSeq *)temp);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tnot_tbools((TemporalS *)temp);
	return result;
}

PG_FUNCTION_INFO_V1(tnot_tbool);
/**
 * Returns the temporal boolean not of the temporal value
 */
PGDLLEXPORT Datum
tnot_tbool(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result = tnot_tbool_internal(temp);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

