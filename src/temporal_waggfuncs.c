/*****************************************************************************
 *
 * temporal_waggfuncs.c
 *	  Window temporal aggregate functions
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_waggfuncs.h"

#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_aggfuncs.h"
#include "doublen.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Extend the temporal instant value by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] inst Temporal value
 * @param[in] interval Interval
 */
static int
tinstant_extend(TSequence **result, const TInstant *inst,
	const Interval *interval)
{
	/* Should be additional attribute */
	bool linear = linear_interpolation(inst->valuetypid);
	TInstant *instants[2];
	TimestampTz upper = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(inst->t),
		PointerGetDatum(interval)));
	instants[0] = (TInstant *) inst;
	instants[1] = tinstant_make(tinstant_value(inst), upper,
		inst->valuetypid);
	result[0] = tsequence_make(instants, 2, true, true, linear, false);
	pfree(instants[1]);
	return 1;
}

/**
 * Extend the temporal instant set value by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ti Temporal value
 * @param[in] interval Interval
 */
static int
tinstantset_extend(TSequence **result, const TInstantSet *ti,
	const Interval *interval)
{
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		tinstant_extend(&result[i], inst, interval);
	}
	return ti->count;
}

/**
 * Extend the temporal sequence value with stepwise interpolation by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 */
static int
tstepseq_extend(TSequence **result, const TSequence *seq, 
	const Interval *interval)
{
	if (seq->count == 1)
		return tinstant_extend(result, tsequence_inst_n(seq, 0), interval);
	
	TInstant *instants[2];
	TInstant *inst1 = tsequence_inst_n(seq, 0);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TInstant *inst2 = tsequence_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false ;
		TimestampTz upper = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst2->t),
			PointerGetDatum(interval)));
		instants[0] = inst1;
		instants[1] = tinstant_make(tinstant_value(inst1), 
			upper, inst1->valuetypid);
		result[i] = tsequence_make(instants, 2, lower_inc, upper_inc, 
			linear, false);
		pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}
	return seq->count - 1;
}

/**
 * Extend the temporal sequence value with linear interpolation by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @param[in] min True if the calling function is min (max otherwise)
 */
static int
tlinearseq_extend(TSequence **result, const TSequence *seq,
	const Interval *interval, bool min)
{
	if (seq->count == 1)
		return tinstant_extend(result, tsequence_inst_n(seq, 0), interval);

	TInstant *instants[3];
	TInstant *inst1 = tsequence_inst_n(seq, 0);
	Datum value1 = tinstant_value(inst1);
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TInstant *inst2 = tsequence_inst_n(seq, i + 1);
		Datum value2 = tinstant_value(inst2);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false ;

		/* Constant segment */
		if (datum_eq(value1, value2, inst1->valuetypid))
		{
			TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
				timestamptz_pl_interval, TimestampTzGetDatum(inst2->t),
				PointerGetDatum(interval)));
			instants[0] = inst1;
			instants[1] = tinstant_make(value1, upper, inst1->valuetypid);
			result[i] = tsequence_make(instants, 2, lower_inc, upper_inc,
				linear, false);
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
				instants[1] = tinstant_make(value1, lower, inst1->valuetypid);
				instants[2] = tinstant_make(value2, upper, inst1->valuetypid);
				result[i] = tsequence_make(instants, 3, lower_inc, upper_inc,
					linear, false);
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
				instants[2] = tinstant_make(value2, upper, inst1->valuetypid);
				result[i] = tsequence_make(instants, 3, lower_inc, upper_inc,
					linear, false);
				pfree(instants[2]);
			}
		}
		inst1 = inst2;
		lower_inc = true;
	}	
	return seq->count - 1;
}

/**
 * Extend the temporal sequence set value with stepwise interpolation by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ts Temporal value
 * @param[in] interval Interval
 */
static int
tstepseqset_extend(TSequence **result, const TSequenceSet *ts,
	const Interval *interval)
{
	if (ts->count == 1)
		return tstepseq_extend(result, tsequenceset_seq_n(ts, 0), interval);

	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tstepseq_extend(&result[k], seq, interval);
	}
	return k;
}

/**
 * Extend the temporal sequence set value with linear interpolation by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ts Temporal value
 * @param[in] interval Interval
 * @param[in] min True if the calling function is min (max otherwise)
 */
static int
tlinearseqset_extend(TSequence **result, const TSequenceSet *ts,
	const Interval *interval, bool min)
{
	if (ts->count == 1)
		return tstepseq_extend(result, tsequenceset_seq_n(ts, 0), interval);

	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tlinearseq_extend(&result[k], seq, interval, min);
	}
	return k;
}

/**
 * Extend the temporal value by the time interval (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[in] min True if the calling function is min (max otherwise)
 * @param[out] count Number of elements in the output array
 */
static TSequence **
temporal_extend(Temporal *temp, Interval *interval, bool min, int *count)
{
	TSequence **result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TINSTANT)
	{
		TInstant *inst = (TInstant *)temp;
		result = palloc(sizeof(TSequence *));
		*count = tinstant_extend(result, inst, interval);
	}
	else if (temp->duration == TINSTANTSET)
	{
		TInstantSet *ti = (TInstantSet *)temp;
		result = palloc(sizeof(TSequence *) * ti->count);
		*count = tinstantset_extend(result, ti, interval);
	}
	else if (temp->duration == TSEQUENCE)
	{
		TSequence *seq = (TSequence *)temp;
		result = palloc(sizeof(TSequence *) * seq->count);
		if (! MOBDB_FLAGS_GET_LINEAR(temp->flags))
			*count = tstepseq_extend(result, seq, interval);
		else
			*count = tlinearseq_extend(result, seq, interval, min);
	}
	else /* temp->duration == TSEQUENCESET */
	{
		TSequenceSet *ts = (TSequenceSet *)temp;
		result = palloc(sizeof(TSequence *) * ts->totalcount);
		if (! MOBDB_FLAGS_GET_LINEAR(temp->flags))
			*count = tstepseqset_extend(result, ts, interval);
		else
			*count = tlinearseqset_extend(result, ts, interval, min);
	}
	return result;
}

/*****************************************************************************
 * Transform a temporal numeric type into a temporal integer type with value 1 
 * extended by a time interval. 
 *****************************************************************************/

/**
 * Transform the temporal numeric instant value by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] inst Temporal value
 * @param[in] interval Interval
 */
static int
tinstant_transform_wcount(TSequence **result, TInstant *inst, 
	Interval *interval)
{
	TInstant *instants[2];
	TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
		timestamptz_pl_interval, TimestampTzGetDatum(inst->t), 
		PointerGetDatum(interval)));
	instants[0] = tinstant_make(Int32GetDatum(1), inst->t, INT4OID);
	instants[1] = tinstant_make(Int32GetDatum(1), upper, INT4OID);
	result[0] = tsequence_make(instants, 2, true, true, false, false);
	pfree(instants[0]);	pfree(instants[1]);
	return 1;
}

/**
 * Transform the temporal numeric instant set value by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ti Temporal value
 * @param[in] interval Interval
 */
static int
tinstantset_transform_wcount(TSequence **result, TInstantSet *ti, Interval *interval)
{
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		tinstant_transform_wcount(&result[i], inst, interval);
	}
	return ti->count;
}

/**
 * Transform the temporal numeric sequence value by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 */
static int
tsequence_transform_wcount(TSequence **result, TSequence *seq, Interval *interval)
{
	if (seq->count == 1)
		return tinstant_transform_wcount(result, tsequence_inst_n(seq, 0), interval);

	TInstant *instants[2];
	TInstant *inst1 = tsequence_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TInstant *inst2 = tsequence_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false ;
		TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
			timestamptz_pl_interval, TimestampTzGetDatum(inst2->t), 
			PointerGetDatum(interval)));
		instants[0] = tinstant_make(Int32GetDatum(1), inst1->t, INT4OID);
		instants[1] = tinstant_make(Int32GetDatum(1), upper, INT4OID);
		result[i] = tsequence_make(instants, 2, lower_inc, upper_inc,
			false, false);
		pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}	
	return seq->count - 1;
}

/**
 * Transform the temporal numeric sequence set value by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ts Temporal value
 * @param[in] interval Interval
 */
static int
tsequenceset_transform_wcount(TSequence **result, TSequenceSet *ts, Interval *interval)
{
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tsequence_transform_wcount(&result[k], seq, interval);
	}
	return k;
}

/**
 * Transform the temporal number by the time interval (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[out] count Number of elements in the output array
 */
static TSequence **
temporal_transform_wcount(Temporal *temp, Interval *interval, int *count)
{
	ensure_valid_duration(temp->duration);
	TSequence **result;
	if (temp->duration == TINSTANT)
	{
		TInstant *inst = (TInstant *)temp;
		result = palloc(sizeof(TSequence *));
		*count = tinstant_transform_wcount(result, inst, interval);
	}
	else if (temp->duration == TINSTANTSET)
	{
		TInstantSet *ti = (TInstantSet *)temp;
		result = palloc(sizeof(TSequence *) * ti->count);
		*count = tinstantset_transform_wcount(result, ti, interval);
	}
	else if (temp->duration == TSEQUENCE)
	{
		TSequence *seq = (TSequence *)temp;
		result = palloc(sizeof(TSequence *) * seq->count);
		*count = tsequence_transform_wcount(result, seq, interval);
	}
	else /* temp->duration == TSEQUENCESET */
	{
		TSequenceSet *ts = (TSequenceSet *)temp;
		result = palloc(sizeof(TSequence *) * ts->totalcount);
		*count = tsequenceset_transform_wcount(result, ts, interval);
	}
	return result;
}

/*****************************************************************************/

/**
 * Transform the temporal number into a temporal double and extend it
 * by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] inst Temporal value
 * @param[in] interval Interval
 */
static int
tnumberinst_transform_wavg(TSequence **result, TInstant *inst, Interval *interval)
{
	/* Should be additional attribute */
	bool linear = true;
	float8 value = 0.0;
	ensure_numeric_base_type(inst->valuetypid);
	if (inst->valuetypid == INT4OID)
		value = DatumGetInt32(tinstant_value(inst)); 
	else if (inst->valuetypid == FLOAT8OID)
		value = DatumGetFloat8(tinstant_value(inst)); 
	double2 dvalue;
	double2_set(&dvalue, value, 1);
	TimestampTz upper = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(inst->t),
		PointerGetDatum(interval)));
	TInstant *instants[2];
	instants[0] = tinstant_make(PointerGetDatum(&dvalue),
		inst->t, type_oid(T_DOUBLE2));
	instants[1] = tinstant_make(PointerGetDatum(&dvalue),
		upper, type_oid(T_DOUBLE2));
	result[0] = tsequence_make(instants, 2, true, true, linear, false);
	pfree(instants[0]);	pfree(instants[1]);
	return 1;
}

/**
 * Transform the temporal number into a temporal double and extend it
 * by the time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ti Temporal value
 * @param[in] interval Interval
 */
static int
tnumberinstset_transform_wavg(TSequence **result, TInstantSet *ti, Interval *interval)
{
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		tnumberinst_transform_wavg(&result[i], inst, interval);
	}
	return ti->count;
}

/**
* Transform the temporal integer sequence value into a temporal double and extend
 * it by a time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] seq Temporal value
 * @param[in] interval Interval
 * @note There is no equivalent function for temporal float types 
 */
static int
tintseq_transform_wavg(TSequence **result, TSequence *seq, Interval *interval)
{
	/* Should be additional attribute */
	bool linear = true;
	TInstant *instants[2];
	if (seq->count == 1)
	{
		TInstant *inst = tsequence_inst_n(seq, 0);
		double value = DatumGetInt32(tinstant_value(inst)); 
		double2 dvalue;
		double2_set(&dvalue, value, 1);
		TimestampTz upper = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t),
			PointerGetDatum(interval)));
		instants[0] = tinstant_make(PointerGetDatum(&dvalue),
			inst->t, type_oid(T_DOUBLE2));
		instants[1] = tinstant_make(PointerGetDatum(&dvalue),
			upper, type_oid(T_DOUBLE2));
		result[0] = tsequence_make(instants, 2, true, true, linear, false);
		pfree(instants[0]);	pfree(instants[1]);
		return 1;
	}

	TInstant *inst1 = tsequence_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 0; i < seq->count - 1; i++)
	{
		TInstant *inst2 = tsequence_inst_n(seq, i + 1);
		bool upper_inc = (i == seq->count - 2) ? seq->period.upper_inc : false ;
		double value = DatumGetInt32(tinstant_value(inst1)); 
		double2 dvalue;
		double2_set(&dvalue, value, 1);
		TimestampTz upper = DatumGetTimestampTz(DirectFunctionCall2(
			timestamptz_pl_interval, TimestampTzGetDatum(inst2->t),
			PointerGetDatum(interval)));
		instants[0] = tinstant_make(PointerGetDatum(&dvalue), inst1->t,
			type_oid(T_DOUBLE2));
		instants[1] = tinstant_make(PointerGetDatum(&dvalue), upper,
			type_oid(T_DOUBLE2));
		result[i] = tsequence_make(instants, 2, lower_inc, upper_inc,
			linear, false);
		pfree(instants[0]); pfree(instants[1]);
		inst1 = inst2;
		lower_inc = true;
	}
	return seq->count - 1;
}

/**
* Transform the temporal integer sequence set value into a temporal double and extend
 * it by a time interval
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * values are stored
 * @param[in] ts Temporal value
 * @param[in] interval Interval
 * @note There is no equivalent function for temporal float types 
 */
static int
tintseqset_transform_wavg(TSequence **result, TSequenceSet *ts, Interval *interval)
{
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tintseq_transform_wavg(&result[k], seq, interval);
	}
	return k;
}

/**
 * Transform the temporal integer sequence set value into a temporal double and extend
 * it by a time interval (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[out] count Number of elements in the output array
 * @note There is no equivalent function for temporal float types 
*/
static TSequence **
tnumber_transform_wavg(Temporal *temp, Interval *interval, int *count)
{
	ensure_valid_duration(temp->duration);
	TSequence **result;
	if (temp->duration == TINSTANT)
	{	
		TInstant *inst = (TInstant *)temp;
		result = palloc(sizeof(TSequence *));
		*count = tnumberinst_transform_wavg(result, inst, interval);
	}
	else if (temp->duration == TINSTANTSET)
	{	
		TInstantSet *ti = (TInstantSet *)temp;
		result = palloc(sizeof(TSequence *) * ti->count);
		*count = tnumberinstset_transform_wavg(result, ti, interval);
	}
	else if (temp->duration == TSEQUENCE)
	{
		TSequence *seq = (TSequence *)temp;
		result = palloc(sizeof(TSequence *) * seq->count);
		*count = tintseq_transform_wavg(result, seq, interval);
	}
	else /* temp->duration == TSEQUENCESET */
	{
		TSequenceSet *ts = (TSequenceSet *)temp;
		result = palloc(sizeof(TSequence *) * ts->totalcount);
		*count = tintseqset_transform_wavg(result, ts, interval);
	}
	return result;
}

/*****************************************************************************
 * Generic moving window transition functions 
 *****************************************************************************/

/**
 * Generic moving window transition function for min, max, and sum aggregation
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[inout] state Skiplist containing the state
 * @param[in] temp Temporal value
 * @param[in] interval Interval
 * @param[in] func Function
 * @param[in] min True if the calling function is min (max otherwise)
 * @param[in] crossings State whether turning points are added in the segments
 * @note This function is directly called by the window sum aggregation for 
 * temporal floats after verifying since the operation is not supported for 
 * sequence (set) duration
 */
static SkipList *
temporal_wagg_transfn1(FunctionCallInfo fcinfo, SkipList *state, 
	Temporal *temp, Interval *interval,
	Datum (*func)(Datum, Datum), bool min, bool crossings)
{
	int count;
	TSequence **sequences = temporal_extend(temp, interval, min, &count);
	SkipList *result = tsequence_tagg_transfn(fcinfo, state, sequences[0], 
		func, crossings);
	for (int i = 1; i < count; i++)
		result = tsequence_tagg_transfn(fcinfo, result, sequences[i],
			func, crossings);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/**
 * Generic moving window transition function for min, max, and sum aggregation
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Function
 * @param[in] min True if the calling function is min (max otherwise)
 * @param[in] crossings State whether turning points are added in the segments
 */
Datum
temporal_wagg_transfn(FunctionCallInfo fcinfo, 
	Datum (*func)(Datum, Datum), bool min, bool crossings)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1) || PG_ARGISNULL(2))
	{
		if (! state)
			PG_RETURN_NULL();
		PG_RETURN_POINTER(state);
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);

	SkipList *result = temporal_wagg_transfn1(fcinfo, state, temp, interval, func,
		min, crossings);
	
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(tint_wmin_transfn);
/**
 * Transition function for moving window minimun aggregation for temporal integer values
 */
PGDLLEXPORT Datum
tint_wmin_transfn(PG_FUNCTION_ARGS)
{
	return temporal_wagg_transfn(fcinfo, &datum_min_int32, true, true);
}

PG_FUNCTION_INFO_V1(tfloat_wmin_transfn);
/**
 * Transition function for moving window minimun
 */
PGDLLEXPORT Datum
tfloat_wmin_transfn(PG_FUNCTION_ARGS)
{
	return temporal_wagg_transfn(fcinfo, &datum_min_float8, true, true);
}

PG_FUNCTION_INFO_V1(tint_wmax_transfn);
/**
 * Transition function for moving window maximun aggregation for temporal integer values
 */
PGDLLEXPORT Datum
tint_wmax_transfn(PG_FUNCTION_ARGS)
{
	return temporal_wagg_transfn(fcinfo, &datum_max_int32, false, true);
}

PG_FUNCTION_INFO_V1(tfloat_wmax_transfn);
/**
 * Transition function for moving window maximun aggregation for temporal float values
 */
PGDLLEXPORT Datum
tfloat_wmax_transfn(PG_FUNCTION_ARGS)
{
	return temporal_wagg_transfn(fcinfo, &datum_max_float8, false, true);
}

PG_FUNCTION_INFO_V1(tint_wsum_transfn);
/**
 * Transition function for moving window sum aggregation for temporal inter values
 */
PGDLLEXPORT Datum
tint_wsum_transfn(PG_FUNCTION_ARGS)
{
	return temporal_wagg_transfn(fcinfo, &datum_sum_int32, true, false);
}

PG_FUNCTION_INFO_V1(tfloat_wsum_transfn);
/**
 * Transition function for moving window sum aggregation for temporal float values
 */
PGDLLEXPORT Datum
tfloat_wsum_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1) || PG_ARGISNULL(2))
	{
		if (! state)
			PG_RETURN_NULL();
		PG_RETURN_POINTER(state);
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	if ((temp->duration == TSEQUENCE || temp->duration == TSEQUENCESET) &&
		temp->valuetypid == FLOAT8OID)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("Operation not supported for temporal float sequences")));
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	SkipList *result = temporal_wagg_transfn1(fcinfo, state, temp, interval, 
		&datum_sum_float8, true, false);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(temporal_wcount_transfn);
/**
 * Transition function for moving window count aggregation for temporal values
 */
PGDLLEXPORT Datum
temporal_wcount_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1) || PG_ARGISNULL(2))
	{
		if (! state)
			PG_RETURN_NULL();
		PG_RETURN_POINTER(state);
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	int count;
	TSequence **sequences = temporal_transform_wcount(temp, interval, &count);
	SkipList *result = tsequence_tagg_transfn(fcinfo, state, sequences[0], 
		&datum_sum_int32, false);
	for (int i = 1; i < count; i++)
		result = tsequence_tagg_transfn(fcinfo, result, sequences[i], 
			&datum_sum_int32, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tnumber_wavg_transfn);
/**
 * Transition function for moving window average aggregation for temporal values
 */
PGDLLEXPORT Datum
tnumber_wavg_transfn(PG_FUNCTION_ARGS)
{
	SkipList *state = PG_ARGISNULL(0) ? NULL :
		(SkipList *) PG_GETARG_POINTER(0);
	if (PG_ARGISNULL(1) || PG_ARGISNULL(2))
	{
		if (! state)
			PG_RETURN_NULL();
		PG_RETURN_POINTER(state);
	}
	Temporal *temp = PG_GETARG_TEMPORAL(1);
	Interval *interval = PG_GETARG_INTERVAL_P(2);
	int count;
	TSequence **sequences = tnumber_transform_wavg(temp, interval, &count);
	SkipList *result = tsequence_tagg_transfn(fcinfo, state, sequences[0], 
		&datum_sum_double2, false);
	for (int i = 1; i < count; i++)
		result = tsequence_tagg_transfn(fcinfo, result, sequences[i], 
			&datum_sum_double2, false);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	PG_FREE_IF_COPY(temp, 1);
	PG_FREE_IF_COPY(interval, 2);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
