/*****************************************************************************
 *
 * TemporalNPoint.c
 *	  Basic functions for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpointseq_in);

PGDLLEXPORT Datum
tnpointseq_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	Oid temptypid = PG_GETARG_OID(1);
	Oid valuetypid;
	temporal_typinfo(temptypid, &valuetypid);
	TemporalSeq *result = tnpointseq_parse(&input, valuetypid);
	if (result == 0)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnpoints_in);

PGDLLEXPORT Datum
tnpoints_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	Oid temptypid = PG_GETARG_OID(1);
	Oid valuetypid;
	temporal_typinfo(temptypid, &valuetypid);
	TemporalS *result = tnpoints_parse(&input, valuetypid);
	if (result == 0)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tnpoint_make_tnpointseq);

PGDLLEXPORT Datum
tnpoint_make_tnpointseq(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	bool lower_inc = PG_GETARG_BOOL(1);
	bool upper_inc = PG_GETARG_BOOL(2);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count < 1)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
			errmsg("A temporal value must have at least one instant")));
	}

	TemporalInst **instants = (TemporalInst **)temporalarr_extract(array, &count);
	npoint *np = DatumGetNpoint(temporalinst_value(instants[0]));
	int64 rid = np->rid;
	for (int i = 1; i < count; i++)
	{
		np = DatumGetNpoint(temporalinst_value(instants[i]));
		if (np->rid != rid)
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
				errmsg("Temporal sequence must have same rid")));
	}

	TemporalSeq *result = temporalseq_from_temporalinstarr(instants,
		count, lower_inc, upper_inc, true);

	pfree(instants);
	PG_FREE_IF_COPY(array, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/*
 * Positions functions
 * Return the network region covered by the moving object
 */

nregion *
tnpointseq_positions(TemporalSeq *seq)
{
	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	npoint *np = DatumGetNpoint(temporalinst_value(inst));
	int64 rid = np->rid;
	double minPos = np->pos, maxPos = np->pos;
	for (int i = 1; i < seq->count; i++)
	{
		inst = temporalseq_inst_n(seq, i);
		np = DatumGetNpoint(temporalinst_value(inst));
		minPos = Min(minPos, np->pos);
		maxPos = Max(maxPos, np->pos);
	}
	return nregion_from_segment_internal(rid, minPos, maxPos);
}

nregion *
tnpoints_positions(TemporalS *ts)
{
	nregion **nregs = palloc(sizeof(nregion *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		nregs[i] = tnpointseq_positions(seq);
	}
	nregion *result = nregion_from_nregionarr_internal(nregs, ts->count);

	for (int i = 0; i < ts->count; i++)
		pfree(nregs[i]);
	pfree(nregs);
	return result;
}

PG_FUNCTION_INFO_V1(tnpoint_positions);

PGDLLEXPORT Datum
tnpoint_positions(PG_FUNCTION_ARGS)
{
	Temporal *temp = PG_GETARG_TEMPORAL(0);
	if (temp->type != TEMPORALSEQ && temp->type != TEMPORALS)
	{
		PG_FREE_IF_COPY(temp, 0);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Input values must be of type temporal sequence (set)")));
	}
	
	nregion *result = NULL; /* initialized to make the compiler quiet */
	if (temp->type == TEMPORALSEQ) 
		result = tnpointseq_positions((TemporalSeq *)temp);
	else /* temp->type == TEMPORALS */
		result = tnpoints_positions((TemporalS *)temp);
	
	PG_FREE_IF_COPY(temp, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
