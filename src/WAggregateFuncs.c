/*****************************************************************************
 *
 * WAggregateFuncs.c
 *	  Window temporal aggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/* Extend the temporal value by a time interval */

static TemporalSeq *
temporalinst_extend1(TemporalInst *inst, Interval *interval)
{
	TemporalSeq *result;
	TemporalInst *instants[2];
	TimestampTz upper = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(inst->t),
		PointerGetDatum(interval)));
	instants[0] = inst;
	instants[1] = temporalinst_make(temporalinst_value(inst), 
		upper, inst->valuetypid);
	result = temporalseq_from_temporalinstarr(instants, 2,
		true, true, false);
	pfree(instants[1]);
	return result;
}

static TemporalSeq **
temporalinst_extend(TemporalInst *inst, Interval *interval)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *));
	result[0] = temporalinst_extend1(inst, interval);
	return result;
}

static TemporalSeq **
temporali_extend(TemporalI *ti, Interval *interval)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result[i] = temporalinst_extend1(inst, interval);
	}
	return result;
}

static TemporalSeq **
tempdiscseq_extend(TemporalSeq *seq, Interval *interval)
{
	if (seq->count == 1)
		return temporalinst_extend(temporalseq_inst_n(seq, 0), interval);
	
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * (seq->count-1));
	TemporalInst *instants[2];
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i+1);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false ;
		TimestampTz upper = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst2->t),
			PointerGetDatum(interval)));
		instants[0] = inst1;
		instants[1] = temporalinst_make(temporalinst_value(inst1), 
			upper, inst1->valuetypid);
		result[i] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}
	return result;
}

static TemporalSeq **
tempcontseq_extend(TemporalSeq *seq, Interval *interval, bool min)
{
	if (seq->count == 1)
		return temporalinst_extend(temporalseq_inst_n(seq, 0), interval);

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * (seq->count-1));
	TemporalInst *instants[3];
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i+1);
		Datum value2 = temporalinst_value(inst2);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false ;

		/* Constant segment */
		if (datum_eq(value1, value2, inst1->valuetypid))
		{
			TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
				timestamptz_pl_interval, TimestampTzGetDatum(inst2->t),
				PointerGetDatum(interval)));
			instants[0] = inst1;
			instants[1] = temporalinst_make(value1, upper, inst1->valuetypid);
			result[i] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, upper_inc, false);
			pfree(instants[1]);
		}
		else
		{
			/* Increasing period and minimum function or
			 * decreasing period and maximum function */
			if ((datum_lt(value1, value2, inst1->valuetypid) && min) ||
				(datum_gt(value1, value2, inst1->valuetypid) && !min))
			{
				/* Extend the start value for the duration of the window */
				TimestampTz lower = DatumGetTimestampTz(DirectFunctionCall2(
					timestamptz_pl_interval, TimestampTzGetDatum(inst1->t),
					PointerGetDatum(interval)));
				TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
					timestamptz_pl_interval, TimestampTzGetDatum(inst2->t),
					PointerGetDatum(interval)));
				instants[0] = inst1;
				instants[1] = temporalinst_make(value1, lower, inst1->valuetypid);
				instants[2] = temporalinst_make(value2, upper, inst1->valuetypid);
				result[i] = temporalseq_from_temporalinstarr(instants, 3,
					lower_inc, upper_inc, false);
				pfree(instants[1]); pfree(instants[2]);
			}
			else
			{
				/* Extend the end value for the duration of the window */
				TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
					timestamptz_pl_interval, TimestampTzGetDatum(seq->period.upper),
					PointerGetDatum(interval)));
				instants[0] = inst1;
				instants[1] = inst2;
				instants[2] = temporalinst_make(value2, upper, inst1->valuetypid);
				result[i] = temporalseq_from_temporalinstarr(instants, 3,
					lower_inc, upper_inc, false);
				pfree(instants[2]);
			}
		}
		inst1 = inst2;
		lower_inc = true;
	}	
	return result;
}

static TemporalSeq **
tempdiscs_extend(TemporalS *ts, Interval *interval, int *count)
{
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		*count = seq->count == 1 ? 1 : seq->count - 1;
		return tempdiscseq_extend(seq, interval);
	}

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tempdiscseq_extend(seq, interval);
		countseqs[i] = seq->count == 1 ? 1 : seq->count - 1;
		totalseqs += countseqs[i];
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		pfree(sequences[i]);
	}

	pfree(sequences); pfree(countseqs);
	
	*count = totalseqs;
	return result;
}

static TemporalSeq **
tempconts_extend(TemporalS *ts, Interval *interval, bool min, int *count)
{
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		*count = seq->count == 1 ? 1 : seq->count - 1;
		return tempcontseq_extend(seq, interval, min);
	}

	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tempcontseq_extend(seq, interval, min);
		countseqs[i] = seq->count == 1 ? 1 : seq->count - 1;
		totalseqs += countseqs[i];
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		pfree(sequences[i]);
	}
	
	pfree(sequences); pfree(countseqs);
	
	*count = totalseqs;
	return result;
}

/* Dispatch function */

static TemporalSeq **
temporal_extend(Temporal *temp, Interval *interval, 
	bool min, int *count)
{
	if (temp->type == TEMPORALINST)
	{
		TemporalInst *inst = (TemporalInst *)temp;
		*count = 1;
		return temporalinst_extend(inst, interval);
	}
	if (temp->type == TEMPORALI)
	{
		TemporalI *ti = (TemporalI *)temp;
		*count = ti->count;
		return temporali_extend(ti, interval);
	}
	else if (temp->type == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		*count = seq->count == 1 ? 1 : seq->count - 1;
		if (! MOBDB_FLAGS_GET_CONTINUOUS(temp->flags))
			return tempdiscseq_extend(seq, interval);
		else
			return tempcontseq_extend(seq, interval, min);
	}
	else if (temp->type == TEMPORALS)
	{
		if (! MOBDB_FLAGS_GET_CONTINUOUS(temp->flags))
			return tempdiscs_extend(
				(TemporalS *)temp, interval, count);
		else
			return tempconts_extend(
				(TemporalS *)temp, interval, min, count);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported")));
}

/*****************************************************************************
 * Transform a temporal numeric type into a temporal integer type with value 1 
 * extended by a time interval. 
 *****************************************************************************/

static TemporalSeq *
temporalinst_transform_wcount1(TemporalInst *inst, Interval *interval)
{
	TemporalSeq *result;
	TemporalInst *instants[2];
	TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
		timestamptz_pl_interval, TimestampTzGetDatum(inst->t), 
		PointerGetDatum(interval)));
	instants[0] = temporalinst_make(Int32GetDatum(1), inst->t, INT4OID);
	instants[1] = temporalinst_make(Int32GetDatum(1), upper, INT4OID);
	result = temporalseq_from_temporalinstarr(instants, 2, true, true, false);
	pfree(instants[0]);	pfree(instants[1]);
	return result;
}

static TemporalSeq **
temporalinst_transform_wcount(TemporalInst *inst, Interval *interval)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *));
	result[0] = temporalinst_transform_wcount1(inst, interval);
	return result;
}


static TemporalSeq **
temporali_transform_wcount(TemporalI *ti, Interval *interval)
{
	TemporalSeq **result= palloc(sizeof(TemporalSeq *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result[i] = temporalinst_transform_wcount1(inst, interval);
	}
	return result;
}

static TemporalSeq **
temporalseq_transform_wcount(TemporalSeq *seq, Interval *interval)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		result[0] = temporalinst_transform_wcount1(inst, interval);
		return result;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * (seq->count-1));
	TemporalInst *instants[2];
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i+1);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false ;
		TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
			timestamptz_pl_interval, TimestampTzGetDatum(inst2->t), 
			PointerGetDatum(interval)));
		instants[0] = temporalinst_make(Int32GetDatum(1), inst1->t, INT4OID);
		instants[1] = temporalinst_make(Int32GetDatum(1), upper, INT4OID);
		result[i] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}	
	return result;
}

static TemporalSeq **
temporals_transform_wcount(TemporalS *ts, Interval *interval, int *count)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = temporalseq_transform_wcount(seq, interval);
		countseqs[i] = seq->count == 1 ? 1 : seq->count - 1;
		totalseqs += countseqs[i];
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		pfree(sequences[i]);
	}
		
	pfree(sequences); pfree(countseqs);
	
	*count = totalseqs;
	return result;
}

/* Dispatch function */

static TemporalSeq **
temporal_transform_wcount(Temporal *temp, Interval *interval, int *count)
{
	if (temp->type == TEMPORALINST)
	{
		TemporalInst *inst = (TemporalInst *)temp;
		*count = 1;
		return temporalinst_transform_wcount(inst, interval);
	}
	else if (temp->type == TEMPORALI)
	{
		TemporalI *ti = (TemporalI *)temp;
		*count = ti->count;
		return temporali_transform_wcount(ti, interval);
	}
	else if (temp->type == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		*count = seq->count == 1 ? 1 : seq->count - 1;
		return temporalseq_transform_wcount(seq, interval);
	}
	else if (temp->type == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		return temporals_transform_wcount(ts, interval, count);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported")));
}

/*****************************************************************************/

/* Transform a temporal numeric type into a temporal double and 
 * extend it by a time interval */

static TemporalSeq *
temporalinst_transform_wavg1(TemporalInst *inst, Interval *interval)
{
	float8 value;
	if (inst->valuetypid == INT4OID)
		value = DatumGetInt32(temporalinst_value(inst)); 
	else if (inst->valuetypid == FLOAT8OID)
		value = DatumGetFloat8(temporalinst_value(inst)); 
	else 
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));

	double2 *dvalue = double2_construct(value, 1);
	TimestampTz upper = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(inst->t),
		PointerGetDatum(interval)));
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(PointerGetDatum(dvalue), 
		inst->t, type_oid(T_DOUBLE2));
	instants[1] = temporalinst_make(PointerGetDatum(dvalue), 
		upper, type_oid(T_DOUBLE2));
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 2,
		true, true, false);
	pfree(instants[0]);	pfree(instants[1]);
	return result;
}

static TemporalSeq **
temporalinst_transform_wavg(TemporalInst *inst, Interval *interval)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *));
	result[0] = temporalinst_transform_wavg1(inst, interval);
	return result;
}

static TemporalSeq **
temporali_transform_wavg(TemporalI *ti, Interval *interval)
{
	TemporalSeq **result= palloc(sizeof(TemporalSeq *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result[i] = temporalinst_transform_wavg1(inst, interval);
	}
	return result;
}

/* Transform a discrete temporal numeric sequence into a temporal double and extend
 * it by a time interval. There is no equivalent function for continuous types */

static TemporalSeq **
tintseq_transform_wavg(TemporalSeq *seq, Interval *interval)
{
	TemporalInst *instants[2];
	if (seq->count == 1)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		double value = DatumGetInt32(temporalinst_value(inst)); 
		double2 *dvalue = double2_construct(value, 1);
		TimestampTz upper = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t),
			PointerGetDatum(interval)));
		instants[0] = temporalinst_make(PointerGetDatum(dvalue), 
			inst->t, type_oid(T_DOUBLE2));
		instants[1] = temporalinst_make(PointerGetDatum(dvalue), 
			upper, type_oid(T_DOUBLE2));
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			true, true, false);
		pfree(instants[0]);	pfree(instants[1]);
		return result;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * (seq->count-1));
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count-1; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i+1);
		bool upper_inc = (i == seq->count-2) ? seq->period.upper_inc : false ;
		double value = DatumGetInt32(temporalinst_value(inst1)); 
		double2 *dvalue = double2_construct(value, 1);
		TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
			timestamptz_pl_interval, TimestampTzGetDatum(inst2->t),
			PointerGetDatum(interval)));
		instants[0] = temporalinst_make(PointerGetDatum(dvalue), inst1->t,
			type_oid(T_DOUBLE2));
		instants[1] = temporalinst_make(PointerGetDatum(dvalue), upper,
			type_oid(T_DOUBLE2));
		result[i] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}
	return result;
}

static TemporalSeq **
tints_transform_wavg(TemporalS *ts, Interval *interval, int *count)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tintseq_transform_wavg(seq, interval);
		countseqs[i] = seq->count == 1 ? 1 : seq->count - 1;
		totalseqs += countseqs[i];
	}
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			result[k++] = sequences[i][j];
		pfree(sequences[i]);
	}
		
	pfree(sequences); pfree(countseqs);
	
	*count = totalseqs;
	return result;
}

/* Dispatch function */

static TemporalSeq **
temporal_transform_wavg(Temporal *temp, Interval *interval, int *count)
{
	if (temp->type == TEMPORALINST)
	{	
		TemporalInst *inst = (TemporalInst *)temp;
		*count = 1;
		return temporalinst_transform_wavg(inst, interval);
	}
	else if (temp->type == TEMPORALI)
	{	
		TemporalI *ti = (TemporalI *)temp;
		*count = ti->count;
		return temporali_transform_wavg(ti, interval);
	}
	else if (temp->type == TEMPORALSEQ)
	{
		TemporalSeq *seq = (TemporalSeq *)temp;
		*count = seq->count == 1 ? 1 : seq->count - 1;
		return tintseq_transform_wavg(seq, interval);
	}
	else if (temp->type == TEMPORALS)
	{
		TemporalS *ts = (TemporalS *)temp;
		return tints_transform_wavg(ts, interval, count);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported")));
}

/*****************************************************************************
 * Temporal 
 *****************************************************************************/

/* Generic moving window transition function for min, max, sum */

static AggregateState *
temporal_wagg_transfn(FunctionCallInfo fcinfo, AggregateState *state, 
	Temporal *temp, Interval *interval,
	Datum (*operator)(Datum, Datum), bool min, bool crossings)
{
	int count;
	TemporalSeq **sequences = temporal_extend(temp, interval, min, &count);
	AggregateState *result = temporalseq_tagg_transfn(fcinfo, state, sequences[0], 
		operator, crossings);
	for (int i = 1; i < count; i++)
		result = temporalseq_tagg_transfn(fcinfo, result, sequences[i],
			operator, crossings);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}
 
/* Moving window minimum transition function */

PG_FUNCTION_INFO_V1(tint_wmin_transfn);

PGDLLEXPORT Datum
tint_wmin_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	AggregateState *result = temporal_wagg_transfn(fcinfo, state, temp, interval, 
		&datum_min_int32, true, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_wmin_transfn);

PGDLLEXPORT Datum
tfloat_wmin_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	AggregateState *result = temporal_wagg_transfn(fcinfo, state, temp, interval, 
		&datum_min_float8, true, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/* Moving window maximum transition function */

PG_FUNCTION_INFO_V1(tint_wmax_transfn);

PGDLLEXPORT Datum
tint_wmax_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	AggregateState *result = temporal_wagg_transfn(fcinfo, state, temp, interval, 
		&datum_max_int32, false, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_wmax_transfn);

PGDLLEXPORT Datum
tfloat_wmax_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	AggregateState *result = temporal_wagg_transfn(fcinfo, state, temp, interval, 
		&datum_max_float8, false, true);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/* Moving window sum transition function */

PG_FUNCTION_INFO_V1(tint_wsum_transfn);

PGDLLEXPORT Datum
tint_wsum_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	AggregateState *result = temporal_wagg_transfn(fcinfo, state, temp, interval, 
		&datum_sum_int32, true, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tfloat_wsum_transfn);

PGDLLEXPORT Datum
tfloat_wsum_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if ((temp->type == TEMPORALSEQ || temp->type == TEMPORALS) &&
		temp->valuetypid == FLOAT8OID)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported for temporal float sequences")));
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	AggregateState *result = temporal_wagg_transfn(fcinfo, state, temp, interval, 
		&datum_sum_float8, true, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/* Moving window count transition function */

PG_FUNCTION_INFO_V1(temporal_wcount_transfn);

PGDLLEXPORT Datum
temporal_wcount_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	int count;
	TemporalSeq **sequences = temporal_transform_wcount(temp, interval, &count);
	AggregateState *result = temporalseq_tagg_transfn(fcinfo, state, sequences[0], 
		&datum_sum_int32, false);
	for (int i = 1; i < count; i++)
		result = temporalseq_tagg_transfn(fcinfo, result, sequences[i], 
			&datum_sum_int32, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/* Moving window average transition function for TemporalInst */

PG_FUNCTION_INFO_V1(temporal_wavg_transfn);

PGDLLEXPORT Datum
temporal_wavg_transfn(PG_FUNCTION_ARGS)
{
	AggregateState *state = PG_ARGISNULL(0) ?
		aggstate_make(fcinfo, 0, NULL) : (AggregateState *) PG_GETARG_POINTER(0);
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	int count;
	TemporalSeq **sequences = temporal_transform_wavg(temp, interval, &count);
	AggregateState *result = temporalseq_tagg_transfn(fcinfo, state, sequences[0], 
		&datum_sum_double2, false);
	for (int i = 1; i < count; i++)
		result = temporalseq_tagg_transfn(fcinfo, result, sequences[i], 
			&datum_sum_double2, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
