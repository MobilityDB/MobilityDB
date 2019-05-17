/*****************************************************************************
 *
 * AggregateFuncs.c
 *	  Temporal aggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"
#include "Aggregates.h"

/*****************************************************************************
 * Numeric aggregate functions on datums
 *****************************************************************************/

/* Get the minimum/maximum value of the two arguments */

Datum
datum_min_int32(Datum l, Datum r)
{
	return DatumGetInt32(l) < DatumGetInt32(r) ? l : r;
}

Datum
datum_max_int32(Datum l, Datum r)
{
	return DatumGetInt32(l) > DatumGetInt32(r) ? l : r;
}

Datum
datum_min_float8(Datum l, Datum r)
{
	return DatumGetFloat8(l) < DatumGetFloat8(r) ? l : r;
}

Datum
datum_max_float8(Datum l, Datum r)
{
	return DatumGetFloat8(l) > DatumGetFloat8(r) ? l : r;
}

Datum
datum_min_text(Datum l, Datum r)
{
	return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0 ? l : r;
}

Datum
datum_max_text(Datum l, Datum r)
{
	return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) > 0 ? l : r;
}

/* Get the sum of the two arguments */

Datum
datum_sum_int32(Datum l, Datum r)
{
	return Int32GetDatum(DatumGetInt32(l) + DatumGetInt32(r));
}

Datum
datum_sum_float8(Datum l, Datum r)
{
	return Float8GetDatum(DatumGetFloat8(l) + DatumGetFloat8(r));
}

Datum
datum_sum_double2(Datum l, Datum r)
{
	return PointerGetDatum(double2_add((double2 *)DatumGetPointer(l), 
		(double2 *)DatumGetPointer(r)));
}

Datum
datum_sum_double3(Datum l, Datum r)
{
	return PointerGetDatum(double3_add((double3 *)DatumGetPointer(l), 
		(double3 *)DatumGetPointer(r)));
}

Datum
datum_sum_double4(Datum l, Datum r)
{
	return PointerGetDatum(double4_add((double4 *)DatumGetPointer(l), 
		(double4 *)DatumGetPointer(r)));
}

/*****************************************************************************
 * Generic aggregate functions 
 *****************************************************************************/

void 
aggstate_write(AggregateState *state, StringInfo buf)
{

	pq_sendint32(buf, (uint32) state->size);
	Oid valuetypid = InvalidOid;
	if (state->size > 0)
		valuetypid = state->values[0]->valuetypid;
	pq_sendint32(buf, valuetypid);
	for (int i = 0; i < state->size; i ++)
		temporal_write(state->values[i], buf);
}

AggregateState *
aggstate_read(FunctionCallInfo fcinfo, StringInfo buf)
{
	MemoryContext ctx;
	if (!AggCheckCallContext(fcinfo, &ctx))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	MemoryContext oldctx = MemoryContextSwitchTo(ctx);

	int size = pq_getmsgint(buf, 4);
	Oid valuetypid = pq_getmsgint(buf, 4);
	AggregateState *result = palloc0(sizeof(AggregateState) + 
		size * sizeof(TemporalSeq *));

	for (int i = 0; i < size; i ++)
		result->values[i] = temporal_read(buf, valuetypid);
	result->size = size;

	MemoryContextSwitchTo(oldctx);
	return result;
}

void
aggstate_clear(AggregateState *state)
{
	for (int i = 0; i < state->size; i ++) {
		pfree(state->values[i]) ;
		state->values[i] = NULL ;
		state->size = 0 ;
	}
}

PG_FUNCTION_INFO_V1(temporal_tagg_serialize);

PGDLLEXPORT Datum
temporal_tagg_serialize(PG_FUNCTION_ARGS)
{
	AggregateState *state = (AggregateState *) PG_GETARG_POINTER(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	aggstate_write(state, &buf);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(temporal_tagg_deserialize);

PGDLLEXPORT Datum
temporal_tagg_deserialize(PG_FUNCTION_ARGS)
{
	bytea* data = PG_GETARG_BYTEA_P(0);
	StringInfoData buf = {
		.cursor = 0,
		.data = VARDATA(data),
		.len = VARSIZE(data),
		.maxlen = VARSIZE(data)
	};
	AggregateState *result = aggstate_read(fcinfo, &buf);
	PG_RETURN_POINTER(result);
}

AggregateState *
aggstate_make(FunctionCallInfo fcinfo, int size, Temporal **values)
{
	MemoryContext ctx;
	if (!AggCheckCallContext(fcinfo, &ctx))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	MemoryContext oldctx = MemoryContextSwitchTo(ctx);
	AggregateState *result = palloc0(sizeof(AggregateState) + 
		size * sizeof(Temporal *));
	for (int i = 0; i < size; i ++)
		result->values[i] = temporal_copy(values[i]);
	result->size = size;
	result->extra = NULL;
	MemoryContextSwitchTo(oldctx);
	return result;
}

void
aggstate_set_extra(FunctionCallInfo fcinfo, AggregateState* state, void* data, size_t size)
{
	MemoryContext ctx;
	if (!AggCheckCallContext(fcinfo, &ctx))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Operation not supported")));
	MemoryContext oldctx = MemoryContextSwitchTo(ctx);
	state->extra = palloc(size) ;
	memcpy(state->extra, data, size) ;
	MemoryContextSwitchTo(oldctx);
}

AggregateState *
aggstate_splice(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2, int from, int to)
{
	MemoryContext ctx;
	if (!AggCheckCallContext(fcinfo, &ctx))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	MemoryContext oldctx = MemoryContextSwitchTo(ctx);
	AggregateState *result = palloc0(sizeof(AggregateState) + 
		(state1->size + state2->size - (to - from)) * sizeof(Temporal *));
	int count = 0;
	for (int i = from; i < to; i ++)
		/* Values that are spliced out must be freed */
		pfree(state1->values[i]);
	for (int i = 0; i < from; i ++)
		result->values[count++] = state1->values[i];
	for (int i = 0; i < state2->size; i ++)
		result->values[count++] = state2->values[i];
	for (int i = to; i < state1->size; i ++)
		result->values[count++] = state1->values[i];
	result->size = count;
	MemoryContextSwitchTo(oldctx);
	return result;
}

/*****************************************************************************
 * Transformation functions for count and avg
 *****************************************************************************/

/*
 * Transform a temporal number into a temporal integer for performing count
 * aggregation
 */
 
static TemporalInst *
temporalinst_transform_tcount(TemporalInst *inst)
{
	return temporalinst_make(Int32GetDatum(1), inst->t, INT4OID);
}

static TemporalI *
temporali_transform_tcount(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = temporalinst_make(Int32GetDatum(1), inst->t, INT4OID);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

static TemporalSeq *
temporalseq_transform_tcount(TemporalSeq *seq)
{
	TemporalSeq *result;
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalinst_make(Int32GetDatum(1), seq->period.lower,
			INT4OID); 
		result = temporalseq_from_temporalinstarr(&inst, 1,
			true, true, false);
		pfree(inst);
		return result;
	}

	TemporalInst *instants[2];
	instants[0] = temporalinst_make(Int32GetDatum(1), seq->period.lower,
		INT4OID); 
	instants[1] = temporalinst_make(Int32GetDatum(1), seq->period.upper,
		INT4OID); 
	result = temporalseq_from_temporalinstarr(instants, 2,
		seq->period.lower_inc, seq->period.upper_inc, false);
	pfree(instants[0]); pfree(instants[1]); 
	return result;
}

static TemporalS *
temporals_transform_tcount(TemporalS *ts)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = temporalseq_transform_tcount(seq);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count, true);

	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* Dispatch function */

static Temporal *
temporal_transform_tcount(Temporal *temp)
{
	assert(temporal_duration_is_valid(temp->duration));
    Temporal *result = NULL;
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_transform_tcount((TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)temporali_transform_tcount((TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_transform_tcount((TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)temporals_transform_tcount((TemporalS *)temp);
	return result;
}

/*****************************************************************************/

/*
 * Transform a temporal number type into a temporal double2 type for 
 * performing average aggregation 
 */

static TemporalInst *
tnumberinst_transform_tavg(TemporalInst *inst)
{
	double value = datum_double(temporalinst_value(inst), inst->valuetypid);
	double2 *dvalue = double2_construct(value, 1);
	return temporalinst_make(PointerGetDatum(dvalue), inst->t,
		type_oid(T_DOUBLE2));
}

static TemporalInst **
tnumberi_transform_tavg(TemporalI *ti)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result[i] = tnumberinst_transform_tavg(inst);
	}
	return result;
}

static int
tintseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalInst *inst1 = tnumberinst_transform_tavg(inst);
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1,
			true, true, false);
		pfree(inst1);
		return 1;
	}
	
	int count;
	TemporalInst *instants[2];
	TemporalInst *inst1, *inst2;
	inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	Datum value2;
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		inst2 = temporalseq_inst_n(seq, i+1);
		value2 = temporalinst_value(inst2);
		instants[0] = tnumberinst_transform_tavg(inst1);
		TemporalInst *inst = temporalinst_make(value1, inst2->t,
			inst1->valuetypid);
		instants[1] = tnumberinst_transform_tavg(inst);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false ;
		bool upper_inc1 = upper_inc && 
			datum_eq(value1, value2, inst1->valuetypid);
		result[i] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc1, false);
		pfree(inst); pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}
	if (seq->period.upper_inc && seq->count > 1 &&
		datum_ne(value1, value2, inst1->valuetypid))
	{
		instants[0] = tnumberinst_transform_tavg(inst2);
		result[seq->count-1] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		count = seq->count;
		pfree(instants[0]);
	}
	else
		count = seq->count - 1;
	return count;
}

int 
tfloatseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tnumberinst_transform_tavg(inst);
	}
	result[0] = temporalseq_from_temporalinstarr(instants, 
		seq->count,	seq->period.lower_inc, seq->period.upper_inc, false);
		
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return 1;
}

static int
tnumberseq_transform_tavg(TemporalSeq **result, TemporalSeq *seq)
{
    int returnvalue = 0;
	assert(temporal_number_is_valid(seq->valuetypid));
	if (seq->valuetypid == INT4OID)
		returnvalue = tintseq_transform_tavg(result, seq);
	if (seq->valuetypid == FLOAT8OID)
		returnvalue = tfloatseq_transform_tavg(result, seq);
	return returnvalue;
}

static int
tnumbers_transform_tavg(TemporalSeq **result, TemporalS *ts)
{
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tnumberseq_transform_tavg(&result[k], seq);
		k += countstep;
	}
	return k;
}

/*****************************************************************************
 * TemporalInst generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate transition function for TemporalInst
 */
AggregateState *
temporalinst_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalInst *inst, Datum (*func)(Datum, Datum))
{
	AggregateState *state2 = aggstate_make(fcinfo, 1, (Temporal **)&inst);
	if (state->size == 0)
		return state2;

	AggregateState *result = temporalinst_tagg_combinefn(fcinfo, state, 
		state2, func);

	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

/*
 * Generic aggregate combine function for TemporalInst
 */
AggregateState *
temporalinst_tagg_combinefn(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2,	Datum (*func)(Datum, Datum))
{
	int count1 = state1->size;
	int count2 = state2->size;
	if (count1 == 0)
		return state2;
	if (count2 == 0)
		return state1;

	if (state1->values[0]->duration != TEMPORALINST || 
		state2->values[0]->duration != TEMPORALINST) 
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Cannot aggregate temporal values of different duration")));

	TemporalInst **state11 = (TemporalInst **)state1->values;
	TemporalInst **state22 = (TemporalInst **)state2->values;
	TimestampTz lower = state22[0]->t;
	TimestampTz upper = state22[count2-1]->t;
	int loweridx = temporalinstarr_find_timestamp(state11, 0, count1, lower);
	int upperidx = temporalinstarr_find_timestamp(state11, loweridx, count1, upper);
	TemporalInst **newinsts = palloc(sizeof(TemporalInst *) * 
		(1 + upperidx - loweridx + count2));
	TemporalInst **mustfree = palloc(sizeof(TemporalInst *) * 
		(1 + upperidx - loweridx + count2));
	int i = 0;
	int j = loweridx;
	int newcount1 = 0;
	int freecount = 0;
	while (i < count2 && j <= upperidx)
	{
		TemporalInst *inst = state22[i];
		if (timestamp_cmp_internal(inst->t, state11[j]->t) == 0)
		{
			TemporalInst *inst1 = temporalinst_make(
				func(temporalinst_value(state11[j]), temporalinst_value(inst)),
				inst->t, inst->valuetypid);
			newinsts[newcount1++] = mustfree[freecount++] = inst1;
			i++;
			j++;
		}
		else if (timestamp_cmp_internal(inst->t, state11[j]->t) < 0)
		{
			newinsts[newcount1++] = inst;
			i++;
		}
		else
		{
			newinsts[newcount1++] = state11[j];
			j++;
		}
	}
	while (i < count2)
	{
		newinsts[newcount1++] = state22[i];
		i++;
	}	
	while (j <= upperidx)
	{
		newinsts[newcount1++] = state11[j];
		j++;
	}
	
	int newcount = (count1 - (1 + upperidx - loweridx)) + newcount1;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * newcount);
	memcpy(instants, state11, loweridx * sizeof(TemporalInst *));
	memcpy(instants + loweridx, newinsts, newcount1 * sizeof(TemporalInst *));
	memcpy(instants + loweridx + newcount1, 1 + state11 + upperidx, 
		(count1 - upperidx - 1) * sizeof(TemporalInst *));
	AggregateState *result = aggstate_make(fcinfo, newcount, (Temporal **)instants);

	pfree(newinsts);  
	for (int i = 0; i < freecount; i++) 
		pfree(mustfree[i]);
	pfree(mustfree);	
	pfree(instants);
	
	return result;
}

/*****************************************************************************
 * TemporalI generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate transition function for TemporalI
 */
static AggregateState *
temporali_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	TemporalI *ti, Datum (*func)(Datum, Datum))
{
	TemporalInst **instants = temporali_instantarr(ti);
	AggregateState *state2 = aggstate_make(fcinfo, ti->count, (Temporal **)instants);
	if (state->size == 0)
		return state2;

	AggregateState *result = temporalinst_tagg_combinefn(fcinfo, state, 
		state2, func);

	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

/*****************************************************************************
 * TemporalSeq generic aggregation functions
 *****************************************************************************/

/* 
 * Generic aggregate function for temporal sequences.
 * Returns new sequences that must be freed by the calling function.
 */

void
temporalseq_tagg1(TemporalSeq **result,
	TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*func)(Datum, Datum), bool crossings, int *newcount)
{
	Period *intersect = intersection_period_period_internal(&seq1->period, &seq2->period);
	if (intersect == NULL)
	{
		/* The two sequences do not intersect: copy the sequences in the right order */
		if (period_cmp_internal(&seq1->period, &seq2->period) < 0)
		{
			result[0] = temporalseq_copy(seq1);
			result[1] = temporalseq_copy(seq2);
		}
		else
		{
			result[0] = temporalseq_copy(seq2);
			result[1] = temporalseq_copy(seq1);
		}
		*newcount = 2;	
		return;
	}

	/* 
	 * If the two sequences intersect there will be at most 3 sequences in the
	 * result: one before the intersection, one for the intersection, and one 
	 * after the intersection. This will be also the case for discrete sequences
	 * (e.g., tint) that has the last value different from the previous one as
	 * tint '[1@2000-01-03, 2@2000-01-04]' and tint '[3@2000-01-01, 4@2000-01-05]'
	 * whose result for sum would be the following three sequences
	 * [3@2000-01-01, 3@2000-01-03), [4@2000-01-03, 5@2000-01-04], and
	 * (3@2000-01-04, 3@2000-01-05]
	 */
	Period period;
	TimestampTz lower1 = seq1->period.lower;
	TimestampTz upper1 = seq1->period.upper;
	bool lower1_inc = seq1->period.lower_inc;
	bool upper1_inc = seq1->period.upper_inc;

	TimestampTz lower2 = seq2->period.lower;
	TimestampTz upper2 = seq2->period.upper;
	bool lower2_inc = seq2->period.lower_inc;
	bool upper2_inc = seq2->period.upper_inc;

	TimestampTz lower = intersect->lower;
	TimestampTz upper = intersect->upper;
	bool lower_inc = intersect->lower_inc;
	bool upper_inc = intersect->upper_inc;
	TemporalSeq *sequences[3];
	int k = 0;

	/* Compute the aggregation on the period before the 
	 * intersection of the intervals */
	if (timestamp_cmp_internal(lower1, lower) < 0 ||
		(lower1_inc && !lower_inc && timestamp_cmp_internal(lower1, lower) == 0))
	{
		period_set(&period, lower1, lower, lower1_inc, !lower_inc);
		sequences[k++] = temporalseq_at_period(seq1, &period);
	}
	else if (timestamp_cmp_internal(lower2, lower) < 0 ||
		(lower2_inc && !lower_inc && timestamp_cmp_internal(lower2, lower) == 0))
	{
		period_set(&period, lower2, lower, lower2_inc, !lower_inc);
		sequences[k++] = temporalseq_at_period(seq2, &period);
	}
	
	/*
	 * Compute the aggregation on the intersection of intervals
	 */
	TemporalSeq *syncseq1, *syncseq2;
	synchronize_temporalseq_temporalseq(seq1, seq2, &syncseq1, &syncseq2, crossings);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * syncseq1->count);
	for (int i = 0; i < syncseq1->count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(syncseq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(syncseq2, i);
		instants[i] = temporalinst_make(
			func(temporalinst_value(inst1), temporalinst_value(inst2)),
			inst1->t, inst1->valuetypid);
	}
	sequences[k++] = temporalseq_from_temporalinstarr(instants, syncseq1->count, 
		lower_inc, upper_inc, true);
	for (int i = 0; i < syncseq1->count; i++)
		pfree(instants[i]);
	pfree(instants); pfree(syncseq1); pfree(syncseq2);
	
	/* Compute the aggregation on the period after the intersection 
	 * of the intervals */
	if (timestamp_cmp_internal(upper, upper1) < 0 ||
		(!upper_inc && upper1_inc && timestamp_cmp_internal(upper, upper1) == 0))
	{
		period_set(&period, upper, upper1, !upper_inc, upper1_inc);
		sequences[k++] = temporalseq_at_period(seq1, &period);
	}
	else if (timestamp_cmp_internal(upper, upper2) < 0 ||
		(!upper_inc && upper2_inc && timestamp_cmp_internal(upper, upper2) == 0))
	{
		period_set(&period, upper, upper2, !upper_inc, upper2_inc);
		sequences[k++] = temporalseq_at_period(seq2, &period);
	}
	pfree(intersect); 

	/* Normalization */
	int l;
	TemporalSeq **normsequences = temporalseqarr_normalize(sequences, k, &l);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	for (int i = 0; i < l; i++)
		result[i] = normsequences[i];
	pfree(normsequences);
	*newcount = l;	
	return;
}

/* 
 * Generic aggregate function for temporal sequences.
 * Arguments:
 * - sequences1 is the accumulated state 
 * - sequences2 are the sequences of a TemporalS value
 * where both may be non contiguous
 * The function returns new sequences in the result that must
 * be freed by the calling function. 
 */
TemporalSeq **
temporalseq_tagg2(TemporalSeq **sequences1, int count1, TemporalSeq **sequences2, 
	int count2, Datum (*func)(Datum, Datum), bool crossings, int *newcount)
{
	/*
	 * Each sequence can be split 3 times, there may be count - 1 holes between
	 * sequences for both sequences 1 and sequences2, and there may be 
	 * 2 sequences before and after.
	 * TODO Verify this formula
	 */
	int seqcount = (count1 * 3) + count1 + count2 + 1;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seqcount);
	int i = 0, j = 0, k = 0, countstep;
	TemporalSeq *seq1 = sequences1[i];
	TemporalSeq *seq2 = sequences2[j];
	while (i < count1 && j < count2)
	{
		temporalseq_tagg1(&sequences[k], seq1, seq2, func, crossings,
			&countstep);
		k += countstep - 1;
		/* If both upper bounds are equal */
		if (timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) == 0 &&
			seq1->period.upper_inc == seq2->period.upper_inc)
		{
			k++; i++; j++;
			if (i == count1 || j == count2)
				break;
			seq1 = sequences1[i];
			seq2 = sequences2[j];
		}
		/* If upper bound of seq1 is less than or equal to the upper bound of seq2 */
		else if (timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) < 0 ||
			(!seq1->period.upper_inc && seq2->period.upper_inc &&
			timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) == 0))
		{
			i++;
			if (i == count1)
			{
				k++; j++;
				break;				
			}
			seq1 = sequences1[i];
			seq2 = sequences[k];
		}
		else
		{
			j++;
			if (j == count2)
			{
				k++; i++;
				break;				
			}
			seq1 = sequences[k];
			seq2 = sequences2[j];
		}
	}
	while (i < count1)
		sequences[k++] = temporalseq_copy(sequences1[i++]);
	while (j < count2)
		sequences[k++] = temporalseq_copy(sequences2[j++]);

	/* Normalization */
	int l;
	TemporalSeq **result = temporalseqarr_normalize(sequences, k, &l);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	*newcount = l;	
	return result;
}

/*
 * Generic aggregate transition function for TemporalSeq
 */
 
AggregateState *
temporalseq_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	TemporalSeq *seq, Datum (*func)(Datum, Datum), bool crossings)
{
	AggregateState *state2 = aggstate_make(fcinfo, 1, (Temporal **)&seq);
	AggregateState *result = temporalseq_tagg_combinefn(fcinfo, state, state2,
		func, crossings);
	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);
	return result;
}
 
/*
 * Generic aggregate combine function for TemporalSeq
 */
AggregateState *
temporalseq_tagg_combinefn(FunctionCallInfo fcinfo, AggregateState *state1, 
	AggregateState *state2,	Datum (*func)(Datum, Datum), bool crossings)
{
	int count1 = state1->size;
	int count2 = state2->size;
	if (count1 == 0)
		return state2;
	if (count2 == 0)
		return state1;

	if (state1->values[0]->duration != TEMPORALSEQ || 
		state2->values[0]->duration != TEMPORALSEQ)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Cannot aggregate temporal values of different duration")));

	TemporalSeq **state11 = (TemporalSeq **)state1->values;	
	TemporalSeq **state22 = (TemporalSeq **)state2->values;	
	Period period_state2;
	period_set(&period_state2,
		state22[0]->period.lower, state22[count2-1]->period.upper,
		state22[0]->period.lower_inc, state22[count2-1]->period.upper_inc);

	AggregateState *result;
	/* The state2 is before all state1 sequences */
	if (before_period_period_internal(&state22[count2-1]->period, 
		&state11[0]->period))
		result = aggstate_splice(fcinfo, state1, state2, 0, 0) ;
	/* The state2 is after all state1 sequences */
	else if (after_period_period_internal(&state22[0]->period, 
		&state11[count1-1]->period))
		result = aggstate_splice(fcinfo, state2, state1, 0, 0) ;
	else
	{
		int loweridx, upperidx;
		bool foundlower = temporalseqarr_find_timestamp(state11, 0, count1,
			period_state2.lower, &loweridx);
		bool foundupper = temporalseqarr_find_timestamp(state11, loweridx, count1,
			period_state2.upper, &upperidx);
		/* If found upper the sequence to be copied is the next one */
		if (foundupper)
			upperidx++;
		if (!foundlower && !foundupper && loweridx == upperidx)
			/* The state2 is in a gap between two sequences of state1 */
			result = aggstate_splice(fcinfo, state1, state2, loweridx, loweridx);
		else
		{
			/* Compute the aggregation of state1[loweridx] -> state1[upperidx-1]
			 * and state2 */
			int newcount1 = upperidx - loweridx;
			int newcount2;
			TemporalSeq **newseqs = temporalseq_tagg2(&state11[loweridx], newcount1,
				state22, count2, func, crossings, &newcount2);
			/* Copy them into the aggregation state memory context */
			AggregateState *tempstate = aggstate_make(fcinfo, newcount2, (Temporal **)newseqs); 
			result = aggstate_splice(fcinfo, state1, tempstate, loweridx, upperidx);
			pfree(tempstate);
			/* free the values in state2 that have been aggregated: */
			aggstate_clear(state2) ;
			for (int i = 0; i < newcount2; i++)
				pfree(newseqs[i]);
			pfree(newseqs);
		}
	}

	return result;
}

/*****************************************************************************
 * TemporalS generic aggregation functions
 *****************************************************************************/

/*
 * Generic aggregate transition function for TemporalS
 */
static AggregateState *
temporals_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	TemporalS *ts, Datum (*func)(Datum, Datum), bool crossings)
{
	TemporalSeq **sequences = temporals_sequencearr(ts);
	AggregateState *state2 = aggstate_make(fcinfo, ts->count, 
		(Temporal **)sequences);
	pfree(sequences);
	if (state->size == 0)
		return state2;

	AggregateState *result = temporalseq_tagg_combinefn(fcinfo, state, state2, 
		func, crossings);

	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

/*****************************************************************************
 * Temporal generic aggregation functions
 *****************************************************************************/

static AggregateState *
temporal_tagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	Temporal *temp, Datum (*func)(Datum, Datum), bool crossings)
{
	assert(temporal_duration_is_valid(temp->duration));
	AggregateState *result = NULL;
	if (temp->duration == TEMPORALINST) 
		result =  temporalinst_tagg_transfn(fcinfo, state, (TemporalInst *)temp, 
			func);
	else if (temp->duration == TEMPORALI) 
		result =  temporali_tagg_transfn(fcinfo, state, (TemporalI *)temp, 
			func);
	else if (temp->duration == TEMPORALSEQ) 
		result =  temporalseq_tagg_transfn(fcinfo, state, (TemporalSeq *)temp, 
			func, crossings);
	else if (temp->duration == TEMPORALS) 
		result = temporals_tagg_transfn(fcinfo, state, (TemporalS *)temp, 
			func, crossings);
	return result;
}

static AggregateState *
temporal_tagg_combinefn(FunctionCallInfo fcinfo, 
	AggregateState *state1, AggregateState *state2,
	Datum (*func)(Datum, Datum), bool crossings)
{
	if (state1->size == 0)
		return state2;
	if (state2->size == 0)
		return state1;

	/* Get a pointer to the first element of the first array */
	Temporal *temp = (Temporal*) state1->values[0];
	if (temp->duration == TEMPORALINST)
		return temporalinst_tagg_combinefn(fcinfo, state1, state2, func);
	if (temp->duration == TEMPORALSEQ)
		return temporalseq_tagg_combinefn(fcinfo, state1, state2, func, crossings);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Operation not supported")));
}

/*****************************************************************************
 * Temporal aggregate functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tbool_tand_transfn);

PGDLLEXPORT Datum
tbool_tand_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_and, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbool_tand_combinefn);

PGDLLEXPORT Datum
tbool_tand_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_and, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbool_tor_transfn);

PGDLLEXPORT Datum
tbool_tor_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_or, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tbool_tor_combinefn);

PGDLLEXPORT Datum
tbool_tor_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_or, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tint_tmin_transfn);

PGDLLEXPORT Datum
tint_tmin_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_min_int32, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tmin_combinefn);

PGDLLEXPORT Datum
tint_tmin_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_min_int32, true);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_transfn);

PGDLLEXPORT Datum
tfloat_tmin_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_min_float8, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmin_combinefn);

PGDLLEXPORT Datum
tfloat_tmin_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_min_float8, true);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tmax_transfn);

PGDLLEXPORT Datum
tint_tmax_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_max_int32, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tmax_combinefn);

PGDLLEXPORT Datum
tint_tmax_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_max_int32, true);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_transfn);

PGDLLEXPORT Datum
tfloat_tmax_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_max_float8, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tmax_combinefn);

PGDLLEXPORT Datum
tfloat_tmax_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_max_float8, true);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tsum_transfn);

PGDLLEXPORT Datum
tint_tsum_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_sum_int32, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tint_tsum_combinefn);

PGDLLEXPORT Datum
tint_tsum_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_sum_int32, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_transfn);

PGDLLEXPORT Datum
tfloat_tsum_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_sum_float8, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_tsum_combinefn);

PGDLLEXPORT Datum
tfloat_tsum_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_sum_float8, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tcount_transfn);

PGDLLEXPORT Datum 
temporal_tcount_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Temporal *tempcount = temporal_transform_tcount(temp);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, tempcount, 
		&datum_sum_int32, false);
	pfree(tempcount);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_tcount_combinefn);

PGDLLEXPORT Datum 
temporal_tcount_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_sum_int32, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/* Transition function for tavg */

AggregateState *
temporalinst_tavg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalInst *inst)
{
	TemporalInst *newinst = tnumberinst_transform_tavg(inst);
	AggregateState *state2 = aggstate_make(fcinfo, 1, (Temporal **)&newinst);
	AggregateState *result = temporalinst_tagg_combinefn(fcinfo, state, state2, 
		&datum_sum_double2);

	pfree(newinst);
	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

AggregateState *
temporali_tavg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalI *ti)
{
	TemporalInst **instants = tnumberi_transform_tavg(ti);
	AggregateState *state2 = aggstate_make(fcinfo, ti->count, (Temporal **)instants);
	AggregateState *result = temporalinst_tagg_combinefn(fcinfo, state, state2, 
		&datum_sum_double2);

	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

AggregateState *
temporalseq_tavg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalSeq *seq)
{
	int maxcount = 0;
	assert(temporal_number_is_valid(seq->valuetypid));
	if (seq->valuetypid == INT4OID)
		maxcount = seq->count;
	else if (seq->valuetypid == FLOAT8OID)
		maxcount = 1;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = tnumberseq_transform_tavg(sequences, seq);
	AggregateState *state2 = aggstate_make(fcinfo, count, (Temporal **)sequences);
	AggregateState *result = temporalseq_tagg_combinefn(fcinfo, state, state2,
		&datum_sum_double2, false);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

AggregateState *
temporals_tavg_transfn(FunctionCallInfo fcinfo, AggregateState *state,
	TemporalS *ts)
{
	int maxcount = 0;
	assert(temporal_number_is_valid(ts->valuetypid));
	if (ts->valuetypid == INT4OID)
		maxcount = ts->totalcount;
	else if (ts->valuetypid == FLOAT8OID)
		maxcount = ts->count;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int count = tnumbers_transform_tavg(sequences, ts);
	AggregateState *state2 = aggstate_make(fcinfo, count, (Temporal **)sequences);
	AggregateState *result = temporalseq_tagg_combinefn(fcinfo, state, state2, 
		&datum_sum_double2, false);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	if (result != state)
		pfree(state);
	if (result != state2)
		pfree(state2);

	return result;
}

PG_FUNCTION_INFO_V1(temporal_tavg_transfn);

PGDLLEXPORT Datum
temporal_tavg_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state =  (PG_ARGISNULL(0)) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = NULL;
	assert(temporal_duration_is_valid(temp->duration));
	if (temp->duration == TEMPORALINST)
		result = temporalinst_tavg_transfn(fcinfo, state, (TemporalInst *)temp);
	else if (temp->duration == TEMPORALI)
		result = temporali_tavg_transfn(fcinfo, state, (TemporalI *)temp);
	else if (temp->duration == TEMPORALSEQ)
		result = temporalseq_tavg_transfn(fcinfo, state, (TemporalSeq *)temp);
	else if (temp->duration == TEMPORALS)
		result = temporals_tavg_transfn(fcinfo, state, (TemporalS *)temp);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

/* Combine function for tavg */

PG_FUNCTION_INFO_V1(temporal_tavg_combinefn);

PGDLLEXPORT Datum
temporal_tavg_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = (PG_ARGISNULL(1)) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2,
		&datum_sum_double2, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/* Generic final function for min, max, sum, count */

PG_FUNCTION_INFO_V1(temporal_tagg_finalfn);

PGDLLEXPORT Datum
temporal_tagg_finalfn(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	AggregateState *state = (AggregateState *) PG_GETARG_POINTER(0);
	if (state->size == 0)
		PG_RETURN_NULL();
	Temporal *result = NULL;
	if (state->values[0]->duration == TEMPORALINST)
		result = (Temporal *)temporali_from_temporalinstarr(
			(TemporalInst **)state->values, state->size);
	else if (state->values[0]->duration == TEMPORALSEQ)
		result = (Temporal *)temporals_from_temporalseqarr(
			(TemporalSeq **)state->values, state->size, true);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported")));

	PG_RETURN_POINTER(result);
}

/* Final function for tavg */

TemporalI *
temporalinst_tavg_finalfn(TemporalInst **instants, int count)
{
	TemporalInst **newinstants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst = instants[i];
		double2 *value = (double2 *)DatumGetPointer(temporalinst_value(inst));
		double tavg = value->a / value->b;
		newinstants[i] = temporalinst_make(Float8GetDatum(tavg), inst->t,
			FLOAT8OID);
	}
	TemporalI *result = temporali_from_temporalinstarr(newinstants, count);

	for (int i = 0; i < count; i++)
		pfree(newinstants[i]);
	pfree(newinstants);
	
	return result;
}

TemporalS *
temporalseq_tavg_finalfn(TemporalSeq **sequences, int count)
{
	TemporalSeq **newsequences = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++)
	{
		TemporalSeq *seq = sequences[i];
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			double2 *value2 = (double2 *)DatumGetPointer(temporalinst_value(inst));
			double value = value2->a / value2->b;
			instants[j] = temporalinst_make(Float8GetDatum(value), inst->t,
				FLOAT8OID);
		}
		newsequences[i] = temporalseq_from_temporalinstarr(instants, 
			seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
		for (int j = 0; j < seq->count; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_from_temporalseqarr(newsequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(newsequences[i]);
	pfree(newsequences);
	
	return result;
}


PG_FUNCTION_INFO_V1(temporal_tavg_finalfn);

PGDLLEXPORT Datum
temporal_tavg_finalfn(PG_FUNCTION_ARGS)
{
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	AggregateState *state = (AggregateState *) PG_GETARG_POINTER(0);
	if (state->size == 0)
		PG_RETURN_NULL();
	Temporal *result = NULL;
	if (state->values[0]->duration == TEMPORALINST)
		result = (Temporal *)temporalinst_tavg_finalfn(
			(TemporalInst **)state->values, state->size);
	else if (state->values[0]->duration == TEMPORALSEQ)
		result = (Temporal *)temporalseq_tavg_finalfn(
			(TemporalSeq **)state->values, state->size);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("Operation not supported")));

	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(ttext_tmin_transfn);

PGDLLEXPORT Datum
ttext_tmin_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_min_text, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_tmin_combinefn);

PGDLLEXPORT Datum
ttext_tmin_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_min_text, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_tmax_transfn);

PGDLLEXPORT Datum
ttext_tmax_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1))
		PG_RETURN_POINTER(state);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	AggregateState *result = temporal_tagg_transfn(fcinfo, state, temp, 
		&datum_max_text, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttext_tmax_combinefn);

PGDLLEXPORT Datum
ttext_tmax_combinefn(PG_FUNCTION_ARGS)
{
	AggregateState *state1 = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	AggregateState *state2 = PG_ARGISNULL(1) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(1);
	AggregateState *result = temporal_tagg_combinefn(fcinfo, state1, state2, 
		&datum_max_text, false);
	if (result != state1)
		pfree(state1);
	if (result != state2)
		pfree(state2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
