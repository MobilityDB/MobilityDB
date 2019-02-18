/*****************************************************************************
 *
 * BooleanOps.c
 *	  Temporal Boolean operators (and, or, not).
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

/*****************************************************************************
 * Boolean operations functions on datums
 *****************************************************************************/

/* Boolean and */

Datum
datum_and(Datum l, Datum r)
{
	return BoolGetDatum(DatumGetBool(l) && DatumGetBool(r));
}

/* Boolean or */

Datum
datum_or(Datum l, Datum r)
{
	return BoolGetDatum(DatumGetBool(l) || DatumGetBool(r));
}

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tand_bool_tbool);

PGDLLEXPORT Datum
tand_bool_tbool(PG_FUNCTION_ARGS)
{
	Datum b = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = oper2_temporal_base(temp, b, &datum_and, BOOLOID, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tand_tbool_bool);

PGDLLEXPORT Datum
tand_tbool_bool(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum b = PG_GETARG_DATUM(1);
	Temporal *result = oper2_temporal_base(temp, b, &datum_and, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tand_tbool_tbool);

PGDLLEXPORT Datum
tand_tbool_tbool(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = oper2_temporal_temporal(sync1, sync2, &datum_and, BOOLOID);

	pfree(sync1); pfree(sync2); 
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

PGDLLEXPORT Datum
tor_bool_tbool(PG_FUNCTION_ARGS)
{
	Datum b = PG_GETARG_DATUM(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *result = oper2_temporal_base(temp, b, &datum_or, BOOLOID, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tor_tbool_bool);

PGDLLEXPORT Datum
tor_tbool_bool(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Datum b = PG_GETARG_DATUM(1);
	Temporal *result = oper2_temporal_base(temp, b, &datum_or, BOOLOID, false);
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tor_tbool_tbool);

PGDLLEXPORT Datum
tor_tbool_tbool(PG_FUNCTION_ARGS)
{
	Temporal *temp1 = PG_GETARG_TEMPORAL(0);
	Temporal *temp2 = PG_GETARG_TEMPORAL(1);
	Temporal *sync1, *sync2;
	/* Return NULL if the temporal points do not intersect in time */
	if (!synchronize_temporal_temporal(temp1, temp2, &sync1, &sync2, false))
	{
		PG_FREE_IF_COPY(temp1, 0);
		PG_FREE_IF_COPY(temp2, 1);
		PG_RETURN_NULL();
	}

	Temporal *result = oper2_temporal_temporal(sync1, sync2, &datum_or, BOOLOID);

	pfree(sync1); pfree(sync2); 
	PG_FREE_IF_COPY(temp1, 0);
	PG_FREE_IF_COPY(temp2, 1);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

static TemporalInst *
tnot_tboolinst(TemporalInst *inst)
{
	TemporalInst *result = temporalinst_copy(inst);
	Datum *value_ptr = temporalinst_value_ptr(result);
	*value_ptr = BoolGetDatum(!DatumGetBool(temporalinst_value(inst)));
	return result;
}

static TemporalI *
tnot_tbooli(TemporalI *ti)
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

static TemporalSeq *
tnot_tboolseq(TemporalSeq *seq)
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

static TemporalS *
tnot_tbools(TemporalS *ts)
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

PG_FUNCTION_INFO_V1(tnot_tbool);

PGDLLEXPORT Datum
tnot_tbool(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	Temporal *result;
	if (temp->type == TEMPORALINST)
		result = (Temporal *)tnot_tboolinst((TemporalInst *)temp);
	else if (temp->type == TEMPORALI)
		result = (Temporal *)tnot_tbooli((TemporalI *)temp);
	else if (temp->type == TEMPORALSEQ)
		result = (Temporal *)tnot_tboolseq((TemporalSeq *)temp);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)tnot_tbools((TemporalS *)temp);	
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

