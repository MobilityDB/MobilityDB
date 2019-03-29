/*****************************************************************************
 *
 * GenericOps.c
 *	  Generic functions for temporal operators on temporal types.
 *
 * These functions are used for defining arithmetic operators (+, -, *, /), 
 * Boolean operators (and, or, not), comparisons (<, <=, >, >=), 
 * distance, etc.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/* 1) There are 3 families of functions accounting for 
 *    - binary operators, such as spatial relationships functions (e.g. 
 *      intersects). 
 *    - ternary operators, such as spatial relationships functions (e.g. 
 *      tdwithin) that need an additional parameter. 
 *    - quaternary operators which apply binary operators (e.g. + or <) to
 *      temporal numeric types that can be of different base type (that is,
 *      integer and float), and thus the third and fourth arguments are the
 *      Oids of the first two arguments.
 *  2) For each of the previous families, there are two set of functions
 *     depending on whether the resulting temporal type is discrete (e.g., 
 *     = for temporal floats that results in a temporal Boolean) or 
 *     continuous (e.g., distance for temporal points that results in a 
 *     temporal float).
 *  3) For each of the previous cases there are two set of functions
 *     depending on whether the arguments are 
 *     - a temporal type and a base type. In this case the operand is applied 
 *       to each instant of the temporal type.
 *     - two temporal types. In this case the operands must be synchronized 
 *       and the operator is applied to each pair of synchronized instants. 
 *       Furthermore, some operators require in addition to add intermediate
 *       points between synchronized instants to take into account the crossings
 *       or the turning points (or local minimum/maximum) of the function 
 *       defined by the operator. For example, tfloat + tfloat only needs to 
 *       synchronize the arguments while tfloat * tfloat requires in addition 
 *       to add the turning point, which is defined at the middle between the
 *       two instants in which the linear functions defined by the arguments
 *       take the value 0.
 * 
 * Examples
 *   - tfloatseq * base => oper4_temporalseq_base
 *     applies the * operator to each instant.
 *   - tfloatseq < base => oper4_temporalseq_base_crossdisc
 *     synchronizes the sequences, applies the < operator to each instant, 
 *     and if the tfloatseq is equal to base in the middle of two subsequent
 *     instants add an instant sequence at the crossing. The result is a 
 *     tfloats.
 *   - tfloatseq + tfloatseq => oper4_temporalseq_temporalseq
 *     synchronizes the sequences and applies the + operator to each instant.
 *   - tfloatseq * tfloatseq => oper4_temporalseq_temporalseq_crosscont
 *     synchronizes the sequences adding the turning points and applies the *
 *     operator to each instant. The result is a tfloatseq.
 *   - tfloatseq < tfloatseq => oper4_temporalseq_temporalseq_crossdisc
 *     synchronizes the sequences, applies the < operator to each instant, 
 *     and if there is a crossing in the middle of two subsequent pairs of 
 *     instants add an instant sequence and the crossing. The result is a 
 *     tfloats.
 */


#include "TemporalTypes.h"

/*****************************************************************************
 * Version of the functions where the arguments are a temporal type and a base
 * type which apply the operator to the composing instants without looking 
 * for crossings or local minimum/maximum. The last argument states whether 
 * we are computing (1) base <oper> temporal or (2) temporal <oper> base
 *****************************************************************************/

/* Temporal op Base */

TemporalInst *
oper2_temporalinst_base(TemporalInst *inst, Datum value, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	Datum value1 = temporalinst_value(inst);
	TemporalInst *inst1 = invert ?
		temporalinst_make(operator(value, value1), inst->t, valuetypid) :
		temporalinst_make(operator(value1, value), inst->t, valuetypid);
	return inst1;
}

TemporalI *
oper2_temporali_base(TemporalI *ti, Datum value, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = oper2_temporalinst_base(inst, value, operator, 
			valuetypid, invert);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
    for (int i = 0; i < ti->count; i++)
        pfree(instants[i]);
    pfree(instants);
    return result;
}

TemporalSeq *
oper2_temporalseq_base(TemporalSeq *seq, Datum value, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = oper2_temporalinst_base(inst, value, operator, 
			valuetypid, invert);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
    for (int i = 0; i < seq->count; i++)
        pfree(instants[i]);
    pfree(instants);
    return result;
}

TemporalS *
oper2_temporals_base(TemporalS *ts, Datum value, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = oper2_temporalseq_base(seq, value, operator, 
			valuetypid, invert);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count, true);
	
    for (int i = 0; i < ts->count; i++)
        pfree(sequences[i]);
    pfree(sequences);
	
    return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
oper2_temporal_base(Temporal *temp, Datum d, 
	Datum (*operator)(Datum, Datum), Oid valuetypid, bool invert)
{
	Temporal *result;
	if (temp->type == TEMPORALINST)
		result = (Temporal *)oper2_temporalinst_base((TemporalInst *)temp, d, 
			operator, valuetypid, invert);
	else if (temp->type == TEMPORALI)
		result = (Temporal *)oper2_temporali_base((TemporalI *)temp, d, 
			operator, valuetypid, invert);
	else if (temp->type == TEMPORALSEQ)
		result = (Temporal *)oper2_temporalseq_base((TemporalSeq *)temp, d, 
			operator, valuetypid, invert);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)oper2_temporals_base((TemporalS *)temp, d,
			operator, valuetypid, invert);
    else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Version of the functions where the operator takes 3 arguments
 * These functions are currently not used. 
 *****************************************************************************/

/* Temporal op Base */

TemporalInst *
oper3_temporalinst_base(TemporalInst *inst, Datum value, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	Datum value1 = temporalinst_value(inst);
	TemporalInst *inst1 = invert ?
		temporalinst_make(operator(value, value1, param), inst->t, valuetypid) :
		temporalinst_make(operator(value1, value, param), inst->t, valuetypid);
	return inst1;
}

TemporalI *
oper3_temporali_base(TemporalI *ti, Datum value, Datum param, 
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = oper3_temporalinst_base(inst, value, param, operator, 
			valuetypid, invert);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
    for (int i = 0; i < ti->count; i++)
        pfree(instants[i]);
    pfree(instants);
    return result;
}

TemporalSeq *
oper3_temporalseq_base(TemporalSeq *seq, Datum value, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = oper3_temporalinst_base(inst, value, param, operator, 
			valuetypid, invert);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
    for (int i = 0; i < seq->count; i++)
        pfree(instants[i]);
    pfree(instants);
    return result;
}
 
TemporalS *
oper3_temporals_base(TemporalS *ts, Datum value, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = oper3_temporalseq_base(seq, value, param, operator, 
			valuetypid, invert);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count, true);
    for (int i = 0; i < ts->count; i++)
        pfree(sequences[i]);
    pfree(sequences);
    return result;
}

/*****************************************************************************
 * Version of the functions where the operator takes 4 arguments 
 *****************************************************************************/

/* Temporal op Base */

TemporalInst *
oper4_temporalinst_base(TemporalInst *inst, Datum value,  
	Datum (*operator)(Datum, Datum, Oid, Oid), 
	Oid datumtypid, Oid valuetypid, bool invert)
{
	TemporalInst *inst1 = invert ?
		temporalinst_make(
			operator(value, temporalinst_value(inst), datumtypid, inst->valuetypid), 
			inst->t, valuetypid) :
		temporalinst_make(
			operator(temporalinst_value(inst), value, inst->valuetypid, datumtypid), 
			inst->t, valuetypid);
	return inst1;
}

TemporalI *
oper4_temporali_base(TemporalI *ti, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = oper4_temporalinst_base(inst, value, operator, 
			datumtypid, valuetypid, invert);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
    for (int i = 0; i < ti->count; i++)
        pfree(instants[i]);
    pfree(instants);
    return result;
}

TemporalSeq *
oper4_temporalseq_base(TemporalSeq *seq, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = oper4_temporalinst_base(inst, value, operator, 
			datumtypid, valuetypid, invert);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, true);
    for (int i = 0; i < seq->count; i++)
        pfree(instants[i]);
    pfree(instants);
    return result;
}

TemporalS *
oper4_temporals_base(TemporalS *ts, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = oper4_temporalseq_base(seq, value, operator, 
			datumtypid, valuetypid, invert);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count, true);
	
    for (int i = 0; i < ts->count; i++)
        pfree(sequences[i]);
    pfree(sequences);
	
    return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
oper4_temporal_base(Datum value, Temporal *temp, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool inverted)
{
	Temporal *result;
	if (temp->type == TEMPORALINST)
		result = (Temporal *)oper4_temporalinst_base((TemporalInst *)temp, 
			value, operator, datumtypid, valuetypid, inverted);
	else if (temp->type == TEMPORALI)
		result = (Temporal *)oper4_temporali_base((TemporalI *)temp, 
			value, operator, datumtypid, valuetypid, inverted);
	else if (temp->type == TEMPORALSEQ)
		result = (Temporal *)oper4_temporalseq_base((TemporalSeq *)temp, 
			value, operator, datumtypid, valuetypid, inverted);
	else if (temp->type == TEMPORALS)
		result = (Temporal *)oper4_temporals_base((TemporalS *)temp, 
			value, operator, datumtypid, valuetypid, inverted);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Functions that apply the operator to the composing instants and to the 
 * crossings when the resulting value is discrete as required for comparisons 
 * (e.g., #<) and spatial relationships (e.g., tintersects).
 *****************************************************************************/

static int
oper4_temporalseq_base_crossdisc1(TemporalSeq **result,
	TemporalInst *start, TemporalInst *end, 
	bool lower_inc, bool upper_inc, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	Datum startvalue = temporalinst_value(start);
	Datum endvalue = temporalinst_value(end);
	Datum startresult = invert ?
		operator(value, startvalue, datumtypid, start->valuetypid) :
		operator(startvalue, value, start->valuetypid, datumtypid);
	TemporalInst *instants[2];
	int k = 0;
	
	/* Start value is equal to end value */
	if (datum_eq(startvalue, endvalue, start->valuetypid))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		FREE_DATUM(startresult, valuetypid);
		pfree(instants[0]); pfree(instants[1]);
		return 1;
	}
	
	/* Segment is constant but start value is different from end value */
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start->flags))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = invert ?
				operator(value, endvalue, datumtypid, start->valuetypid) :
				operator(endvalue, value, start->valuetypid, datumtypid);
			instants[0] = temporalinst_make(endresult, end->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid);
		}
		FREE_DATUM(startresult, valuetypid);
		return k;
	}

	/* If either the start or the end value is equal to base */	
	if (datum_eq2(startvalue, value, start->valuetypid, datumtypid) ||
		datum_eq2(endvalue, value, start->valuetypid, datumtypid))
	{
		/* Compute the operator at the start instant */
		if (lower_inc)
		{
			instants[0] = temporalinst_make(startresult, start->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
		}
		/* Find the middle time between start and the end instant 
		 * and compute the operator at that point */
		double time1 = start->t;
		double time2 = end->t;
		TimestampTz inttime = time1 + ((time2 - time1)/2);
		Datum intvalue = temporalseq_value_at_timestamp1(start, end, inttime);
		Datum intresult = invert ?
			operator(value, intvalue, datumtypid, start->valuetypid) :
			operator(intvalue, value, start->valuetypid, datumtypid);
		instants[0] = temporalinst_make(intresult, start->t, valuetypid);
		instants[1] = temporalinst_make(intresult, end->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, false, false);			
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(intvalue, start->valuetypid); FREE_DATUM(intresult, valuetypid);
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = invert ?
				operator(value, endvalue, datumtypid, start->valuetypid) :
				operator(endvalue, value, start->valuetypid, datumtypid);
			instants[0] = temporalinst_make(endresult, end->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}
	
	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	bool cross = tempcontseq_timestamp_at_value(start, end, value, 
		datumtypid, &crosstime);

	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startresult, start->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		FREE_DATUM(startresult, valuetypid); 
		pfree(instants[0]); pfree(instants[1]); 
		return 1;
	}

	/* There is a crossing at the middle
	 * Compute the operator from the start instant to the crossing */
	instants[0] = temporalinst_make(startresult, start->t, valuetypid);
	instants[1] = temporalinst_make(startresult, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2, 
		lower_inc, false, false);
	FREE_DATUM(startresult, valuetypid);
	pfree(instants[0]); pfree(instants[1]); 
	/* Compute operator at the cross 
	   Due to floating point precision we cannot compute the operator at the
	   crosstime as follows
			startresult = temporalseq_value_at_timestamp1(start, end, crosstime);
	   Since this operator is (currently) called only for tfloat then we 
	   assume startresult = value */
	Datum value2 = operator(value, value, datumtypid, datumtypid);
	instants[0] = temporalinst_make(value2, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1, 
		true, true, false);
	FREE_DATUM(value2, valuetypid);
	pfree(instants[0]); 
	/* Find the middle time between crossing and the end instant 
	 * and compute the operator at that point */
	double time1 = crosstime;
	double time2 = end->t;
	TimestampTz inttime = time1 + ((time2 - time1)/2);
	startresult = temporalseq_value_at_timestamp1(start, end, inttime);
	value2 = invert ?
		operator(value, startresult, datumtypid, start->valuetypid) :
		operator(startresult, value, start->valuetypid, datumtypid);
	instants[0] = temporalinst_make(value2, crosstime, valuetypid);
	instants[1] = temporalinst_make(value2, end->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2, 
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startresult, valuetypid); FREE_DATUM(value2, valuetypid); 
	return 3;
}

static TemporalSeq **
oper4_temporalseq_base_crossdisc2(TemporalSeq *seq, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, int *count, bool invert)
{
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value1 = invert ?
			operator(value, temporalinst_value(inst), datumtypid, inst->valuetypid) :
			operator(temporalinst_value(inst), value, inst->valuetypid, datumtypid);
		TemporalInst *inst1 = temporalinst_make(value1, inst->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1, 
			true, true, false);
		FREE_DATUM(value1, valuetypid);
		*count = 1;
		return result;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * seq->count * 3);
	int k = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		int countseq = oper4_temporalseq_base_crossdisc1(&result[k], inst1, inst2, lower_inc, 
			upper_inc, value, operator, datumtypid, valuetypid, invert);
		/* The previous step has added between one and three sequences */
		k += countseq;
		inst1 = inst2;
		lower_inc = true;
	}	
	*count = k;
    return result;
}

TemporalS *
oper4_temporalseq_base_crossdisc(TemporalSeq *seq, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	int count;
	TemporalSeq **sequences = oper4_temporalseq_base_crossdisc2(seq, value, 
		operator, datumtypid, valuetypid, &count, invert);
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
    return result;
}

/*****************************************************************************/

TemporalS *
oper4_temporals_base_crossdisc(TemporalS *ts, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0, count;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = oper4_temporalseq_base_crossdisc2(seq, value, operator,
			datumtypid, valuetypid, &count, invert);
		countseqs[i] = count;
		totalseqs += count;
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, k, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
    return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply an operator in
 * a single pass.
 *****************************************************************************/

/*****************************************************************************
 * TemporalInst and <Type>
 *****************************************************************************/

TemporalInst *
sync_oper2_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) != 0)
		return NULL;

	Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper2_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper2_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	return sync_oper2_temporali_temporalinst(ti, inst, operator, valuetypid);
}

TemporalInst *
sync_oper2_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper2_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	return sync_oper2_temporalseq_temporalinst(seq, inst, operator, valuetypid);
}

TemporalInst *
sync_oper2_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper2_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	return sync_oper2_temporals_temporalinst(ts, inst, operator, valuetypid);
}

/*****************************************************************************
 * TemporalI and <Type>
 *****************************************************************************/

TemporalI *
sync_oper2_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporali_timespan(&p1, ti1);
	temporali_timespan(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	int count = Min(ti1->count, ti2->count);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value = operator(temporalinst_value(inst1), 
				temporalinst_value(inst2));
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants); 

	return result;
}

TemporalI *
sync_oper2_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporali_timespan(&p, ti);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			temporalseq_value_at_timestamp(seq, inst->t, &value1);
			Datum value = operator(value1, temporalinst_value(inst));
			instants[k++] = temporalinst_make(value, inst->t, valuetypid);
			FREE_DATUM(value1, seq->valuetypid); FREE_DATUM(value, valuetypid);
		}
		if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
			break;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	
	for (int i = 0; i < k; i++) 
		pfree(instants[i]);
	pfree(instants); 

	return result;
}

TemporalI *
sync_oper2_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	return sync_oper2_temporalseq_temporali(seq, ti, operator, valuetypid);
}

TemporalI *
sync_oper2_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts);
	temporali_timespan(&p2, ti);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ti->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst *inst = temporali_inst_n(ti, j);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			temporals_value_at_timestamp(ts, inst->t, &value1);
			Datum value = operator(value1, temporalinst_value(inst));
			instants[k++] = temporalinst_make(value, inst->t, valuetypid);
			FREE_DATUM(value1, ts->valuetypid); FREE_DATUM(value, valuetypid);
		}
		int cmp = timestamp_cmp_internal(seq->period.upper, inst->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	for (int i = 0; i < k; i++) 
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalI *
sync_oper2_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	return sync_oper2_temporals_temporali(ts, ti, operator, valuetypid);
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

TemporalSeq *
sync_oper2_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return NULL;
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(value1, value2);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		return result;
	}
	
	/* 
	 * General case 
	 * seq1 =  ... *      *   *   *       *>
	 * seq2 =        <*           *   *      * ...
	 * result =      <X I X I X I * I X I X>
	 * where *, X, and I are values computed, respectively at common points, 
	 * synchronization points, and intermediate points
	 */
	TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
	TemporalInst *tofreeinst = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	if (timestamp_cmp_internal(inst1->t, inter->lower) < 0)
	{
		inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofreeinst = inst1;
		i = temporalseq_find_timestamp(seq1, inter->lower);
	}
	else if (timestamp_cmp_internal(inst2->t, inter->lower) < 0)
	{
		inst2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofreeinst = inst2;
		j = temporalseq_find_timestamp(seq2, inter->lower);
	}
	int count = (seq1->count - i + seq2->count - j) * 2;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count);
	if (tofreeinst != NULL)
		tofree[l++] = tofreeinst;
	TemporalInst *prev1, *prev2;
	Datum inter1, inter2, value;
	TimestampTz intertime;
	while (i < seq1->count && j < seq2->count &&
		(timestamp_cmp_internal(inst1->t, inter->upper) <= 0 ||
		timestamp_cmp_internal(inst2->t, inter->upper) <= 0))
	{
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			inst2 = temporalseq_at_timestamp(seq2, inst1->t);
			tofree[l++] = inst2;
		}
		else 
		{
			j++;
			inst1 = temporalseq_at_timestamp(seq1, inst2->t);
			tofree[l++] = inst1;
		}
		/* If not the first instant compute the operator on the potential
		   intermediate point before adding the new instants */
		if (interpoint != NULL && k > 0 && 
			interpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			inter1 = temporalseq_value_at_timestamp1(prev1, inst1, intertime);
			inter2 = temporalseq_value_at_timestamp1(prev2, inst2, intertime);
			value = operator(inter1, inter2);
			instants[k++] = temporalinst_make(value, intertime, valuetypid);
			FREE_DATUM(inter1, seq1->valuetypid); FREE_DATUM(inter2, seq2->valuetypid);
			FREE_DATUM(value, valuetypid);
		}
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
		instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
		FREE_DATUM(value, valuetypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	if (!type_is_continuous(valuetypid) && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k-1];
		value = temporalinst_value(instants[k-2]);
		instants[k-1] = temporalinst_make(value, instants[k-1]->t, valuetypid); 		
	}

   TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		inter->lower_inc, inter->upper_inc, true);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants);
	for (int i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper2_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid, 
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_oper2_temporalseq_temporalseq(seq1, seq, 
			operator, valuetypid, interpoint);
		if (seq2 != NULL)
			sequences[k++] = seq2;
		if (timestamp_cmp_internal(seq->period.upper, seq1->period.upper) < 0 ||
			(timestamp_cmp_internal(seq->period.upper, seq1->period.upper) == 0 &&
			(seq->period.upper_inc == seq->period.lower_inc || 
			(!seq->period.upper_inc && seq->period.lower_inc))))
			break;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
sync_oper2_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))	
{
	return sync_oper2_temporals_temporalseq(ts, seq, operator, valuetypid, interpoint);
}

TemporalS *
sync_oper2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* Previously it was Max(ts1->count, ts2->count) and was not correct */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		TemporalSeq *seq = sync_oper2_temporalseq_temporalseq(seq1, seq2, 
			operator, valuetypid, interpoint);
		if (seq != NULL)
			sequences[k++] = seq;
		int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
		if (cmp == 0)
		{
			if (!seq1->period.upper_inc && seq2->period.upper_inc)
				cmp = -1;
			else if (seq1->period.upper_inc && !seq2->period.upper_inc)
				cmp = 1;
		}
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences); 
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_oper2_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	Temporal *result;
	if (temp1->type == TEMPORALINST && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper2_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			operator, valuetypid);
	
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper2_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			operator, valuetypid);
	
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper2_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid, interpoint);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			operator, valuetypid, interpoint);
	
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper2_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid, interpoint);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			operator, valuetypid, interpoint);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************
 * TemporalInst and <Type>
 *****************************************************************************/

TemporalInst *
sync_oper3_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) != 0)
		return NULL;
	Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
		param);
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper3_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper3_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	return sync_oper3_temporali_temporalinst(ti, inst, param, operator, valuetypid);
}

TemporalInst *
sync_oper3_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper3_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	return sync_oper3_temporalseq_temporalinst(seq, inst, param, operator, valuetypid);
}

TemporalInst *
sync_oper3_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper3_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	return sync_oper3_temporals_temporalinst(ts, inst, param, operator, valuetypid);
}

/*****************************************************************************
 * TemporalI and <Type>
 *****************************************************************************/

TemporalI *
sync_oper3_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporali_timespan(&p1, ti1);
	temporali_timespan(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	int count = Min(ti1->count, ti2->count);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				param);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants); 

	return result;
}

TemporalI *
sync_oper3_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporali_timespan(&p, ti);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			temporalseq_value_at_timestamp(seq, inst->t, &value1);
			Datum value = operator(value1, temporalinst_value(inst), param);
			instants[k++] = temporalinst_make(value, inst->t, valuetypid);
			FREE_DATUM(value1, seq->valuetypid); FREE_DATUM(value, valuetypid);
		}
		if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
			break;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	
	for (int i = 0; i < k; i++) 
		pfree(instants[i]);
	pfree(instants); 

	return result;
}

TemporalI *
sync_oper3_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	return sync_oper3_temporalseq_temporali(seq, ti, param, operator, valuetypid);
}

TemporalI *
sync_oper3_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts);
	temporali_timespan(&p2, ti);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ti->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst *inst = temporali_inst_n(ti, j);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			temporals_value_at_timestamp(ts, inst->t, &value1);
			Datum value = operator(value1, temporalinst_value(inst), param);
			instants[k++] = temporalinst_make(value, inst->t, valuetypid);
			FREE_DATUM(value1, ts->valuetypid); FREE_DATUM(value, valuetypid);
		}
		int cmp = timestamp_cmp_internal(seq->period.upper, inst->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	for (int i = 0; i < k; i++) 
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalI *
sync_oper3_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	return sync_oper3_temporals_temporali(ts, ti, param, operator, valuetypid);
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

TemporalSeq *
sync_oper3_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return NULL;
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(value1, value2, param);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		return result;
	}

	/* 
	 * General case 
	 * seq1 =  ... *      *   *   *       *>
	 * seq2 =        <*           *   *      * ...
	 * result =      <X I X I X I * I X I X>
	 * where *, X, and I are values computed, respectively at common points, 
	 * synchronization points, and intermediate points
	 */
	TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
	TemporalInst *tofreeinst = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	if (timestamp_cmp_internal(inst1->t, inter->lower) < 0)
	{
		inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofreeinst = inst1;
		i = temporalseq_find_timestamp(seq1, inter->lower);
	}
	else if (timestamp_cmp_internal(inst2->t, inter->lower) < 0)
	{
		inst2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofreeinst = inst2;
		j = temporalseq_find_timestamp(seq2, inter->lower);
	}
	int count = (seq1->count - i + seq2->count - j) * 2;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count);
	if (tofreeinst != NULL)
		tofree[l++] = tofreeinst;
	TemporalInst *prev1, *prev2;
	Datum inter1, inter2, value;
	TimestampTz intertime;
	while (i < seq1->count && j < seq2->count &&
		(timestamp_cmp_internal(inst1->t, inter->upper) <= 0 ||
		timestamp_cmp_internal(inst2->t, inter->upper) <= 0))
	{
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			inst2 = temporalseq_at_timestamp(seq2, inst1->t);
			tofree[l++] = inst2;
		}
		else
		{
			j++;
			inst1 = temporalseq_at_timestamp(seq1, inst2->t);
			tofree[l++] = inst1;
		}
		/* If not the first instant compute the operator on the potential
		   intermediate point before adding the new instants */
		if (interpoint != NULL && k > 0 && 
			interpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			inter1 = temporalseq_value_at_timestamp1(prev1, inst1, intertime);
			inter2 = temporalseq_value_at_timestamp1(prev2, inst2, intertime);
			value = operator(inter1, inter2, param);
			instants[k++] = temporalinst_make(value, intertime, valuetypid);
			FREE_DATUM(inter1, seq1->valuetypid); FREE_DATUM(inter2, seq2->valuetypid);
			FREE_DATUM(value, valuetypid);
		}
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2), param);
		instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
		FREE_DATUM(value, valuetypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	if (!type_is_continuous(valuetypid) && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k-1];
		value = temporalinst_value(instants[k-2]);
		instants[k-1] = temporalinst_make(value, instants[k-1]->t, valuetypid); 		
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		inter->lower_inc, inter->upper_inc, true);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants);
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper3_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_oper3_temporalseq_temporalseq(seq1, seq, 
			param, operator, valuetypid, interpoint);
		if (seq2 != NULL)
			sequences[k++] = seq2;
		if (timestamp_cmp_internal(seq->period.upper, seq1->period.upper) < 0 ||
			(timestamp_cmp_internal(seq->period.upper, seq1->period.upper) == 0 &&
			(seq->period.upper_inc == seq->period.lower_inc || 
			(!seq->period.upper_inc && seq->period.lower_inc))))
			break;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
sync_oper3_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	return sync_oper3_temporals_temporalseq(ts, seq, param, operator, valuetypid, interpoint);
}

TemporalS *
sync_oper3_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* Previously it was Max(ts1->count, ts2->count) and was not correct */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		TemporalSeq *seq = sync_oper3_temporalseq_temporalseq(seq1, seq2, 
			param, operator, valuetypid, interpoint);
		if (seq != NULL)
			sequences[k++] = seq;
		int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
		if (cmp == 0)
		{
			if (!seq1->period.upper_inc && seq2->period.upper_inc)
				cmp = -1;
			else if (seq1->period.upper_inc && !seq2->period.upper_inc)
				cmp = 1;
		}
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences); 
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_oper3_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	Temporal *result;
	if (temp1->type == TEMPORALINST && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper3_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			param, operator, valuetypid);
	
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper3_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			param, operator, valuetypid);
	
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper3_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			param, operator, valuetypid, interpoint);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			param, operator, valuetypid, interpoint);
	
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			param, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper3_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			param, operator, valuetypid, interpoint);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			param, operator, valuetypid, interpoint);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************
 * TemporalInst and <Type>
 *****************************************************************************/

TemporalInst *
sync_oper4_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) != 0)
		return NULL;
	Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
		inst1->valuetypid, inst2->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper4_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst),
		ti->valuetypid, inst->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper4_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	return sync_oper4_temporali_temporalinst(ti, inst, operator, valuetypid);
}

TemporalInst *
sync_oper4_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst),
		seq->valuetypid, inst->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper4_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	return sync_oper4_temporalseq_temporalinst(seq, inst, operator, valuetypid);
}

TemporalInst *
sync_oper4_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;
	
	Datum value = operator(value1, temporalinst_value(inst),
		ts->valuetypid, inst->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_oper4_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	return sync_oper4_temporals_temporalinst(ts, inst, operator, valuetypid);
}

/*****************************************************************************
 * TemporalI and <Type>
 *****************************************************************************/

TemporalI *
sync_oper4_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporali_timespan(&p1, ti1);
	temporali_timespan(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	int count = Min(ti1->count, ti2->count);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				inst1->valuetypid, inst2->valuetypid);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants); 

	return result;
}

TemporalI *
sync_oper4_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporali_timespan(&p, ti);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			temporalseq_value_at_timestamp(seq, inst->t, &value1);
			Datum value = operator(value1, temporalinst_value(inst),
				seq->valuetypid, inst->valuetypid);
			instants[k++] = temporalinst_make(value, inst->t, valuetypid);
			FREE_DATUM(value1, seq->valuetypid); FREE_DATUM(value, valuetypid);
		}
		if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
			break;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	
	for (int i = 0; i < k; i++) 
		pfree(instants[i]);
	pfree(instants); 

	return result;
}

TemporalI *
sync_oper4_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	return sync_oper4_temporalseq_temporali(seq, ti, operator, valuetypid);
}

TemporalI *
sync_oper4_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts);
	temporali_timespan(&p2, ti);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ti->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst *inst = temporali_inst_n(ti, j);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			temporals_value_at_timestamp(ts, inst->t, &value1);
			Datum value = operator(value1, temporalinst_value(inst),
				ts->valuetypid, inst->valuetypid);
			instants[k++] = temporalinst_make(value, inst->t, valuetypid);
			FREE_DATUM(value1, ts->valuetypid); FREE_DATUM(value, valuetypid);
		}
		int cmp = timestamp_cmp_internal(seq->period.upper, inst->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, k);
	for (int i = 0; i < k; i++) 
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalI *
sync_oper4_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	return sync_oper4_temporals_temporali(ts, ti, operator, valuetypid);
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

TemporalSeq *
sync_oper4_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return NULL;
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(value1, value2, 
			seq1->valuetypid, seq2->valuetypid);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		return result;
	}
	
	/* 
	 * General case 
	 * seq1 =  ... *      *   *   *       *>
	 * seq2 =        <*           *   *      * ...
	 * result =      <X I X I X I * I X I X>
	 * where *, X, and I are values computed, respectively at common points, 
	 * synchronization points, and intermediate points
	 */
	TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
	TemporalInst *tofreeinst = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	if (timestamp_cmp_internal(inst1->t, inter->lower) < 0)
	{
		inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofreeinst = inst1;
		i = temporalseq_find_timestamp(seq1, inter->lower);
	}
	else if (timestamp_cmp_internal(inst2->t, inter->lower) < 0)
	{
		inst2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofreeinst = inst2;
		j = temporalseq_find_timestamp(seq2, inter->lower);
	}
	int count = (seq1->count - i + seq2->count - j) * 2;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count);
	if (tofreeinst != NULL)
		tofree[l++] = tofreeinst;
	TemporalInst *prev1, *prev2;
	Datum inter1, inter2, value;
	TimestampTz intertime;
	while (i < seq1->count && j < seq2->count &&
		(timestamp_cmp_internal(inst1->t, inter->upper) <= 0 ||
		timestamp_cmp_internal(inst2->t, inter->upper) <= 0))
	{
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			inst2 = temporalseq_at_timestamp(seq2, inst1->t);
			tofree[l++] = inst2;
		}
		else 
		{
			j++;
			inst1 = temporalseq_at_timestamp(seq1, inst2->t);
			tofree[l++] = inst1;
		}
		/* If not the first instant compute the operator on the potential
		   intermediate point before adding the new instants */
		if (interpoint != NULL && k > 0 && 
			interpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			inter1 = temporalseq_value_at_timestamp1(prev1, inst1, intertime);
			inter2 = temporalseq_value_at_timestamp1(prev2, inst2, intertime);
			value = operator(inter1, inter2, seq1->valuetypid, seq2->valuetypid);
			instants[k++] = temporalinst_make(value, intertime, valuetypid);
			FREE_DATUM(inter1, seq1->valuetypid); FREE_DATUM(inter2, seq2->valuetypid);
			FREE_DATUM(value, valuetypid);
		}
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2), 
			seq1->valuetypid, seq2->valuetypid);
		instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
		FREE_DATUM(value, valuetypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	if (k == 0)
	{
		pfree(instants); 
		return NULL;
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	if (!type_is_continuous(valuetypid) && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k-1];
		value = temporalinst_value(instants[k-2]);
		instants[k-1] = temporalinst_make(value, instants[k-1]->t, valuetypid); 		
	}

   TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		inter->lower_inc, inter->upper_inc, true);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants);
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper4_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_oper4_temporalseq_temporalseq(seq1, seq, 
			operator, valuetypid, interpoint);
		if (seq2 != NULL)
			sequences[k++] = seq2;
		if (timestamp_cmp_internal(seq->period.upper, seq1->period.upper) < 0 ||
			(timestamp_cmp_internal(seq->period.upper, seq1->period.upper) == 0 &&
			(!seq->period.upper_inc || seq1->period.upper_inc)))
			break;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
sync_oper4_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	return sync_oper4_temporals_temporalseq(ts, seq, operator, valuetypid, interpoint);
}

TemporalS *
sync_oper4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* Previously it was Max(ts1->count, ts2->count) and was not correct */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		TemporalSeq *seq = sync_oper4_temporalseq_temporalseq(seq1, seq2, 
			operator, valuetypid, interpoint);
		if (seq != NULL)
			sequences[k++] = seq;
		int cmp = timestamp_cmp_internal(seq1->period.upper, seq2->period.upper);
		if (cmp == 0)
		{
			if (!seq1->period.upper_inc && seq2->period.upper_inc)
				cmp = -1;
			else if (seq1->period.upper_inc && !seq2->period.upper_inc)
				cmp = 1;
		}
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences); 
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_oper4_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	Temporal *result;
	if (temp1->type == TEMPORALINST && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper4_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			operator, valuetypid);
	
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper4_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			operator, valuetypid);
	
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper4_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid, interpoint);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			operator, valuetypid, interpoint);
	
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper4_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid, interpoint);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			operator, valuetypid, interpoint);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

static int
sync_oper2_temporalseq_temporalseq_crossdisc1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, bool lower_inc, bool upper_inc,
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	Datum startvalue1 = temporalinst_value(start1);
	Datum endvalue1 = temporalinst_value(end1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum endvalue2 = temporalinst_value(end2);
	Datum startresult = operator(startvalue1, startvalue2);
	TemporalInst *instants[2];
	int k = 0;

	/* Both segments are constant */
	if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
		datum_eq(startvalue2, endvalue2, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startresult, valuetypid); 
		return 1;
	}

	/* Both segments are constant but at least one start value is different 
	 * from the corresponding end value */
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start1->flags) && 
		! MOBDB_FLAGS_GET_CONTINUOUS(start2->flags))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid);
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* If the start or end values are equal */	
	if (datum_eq2(startvalue1, startvalue2, start1->valuetypid, start2->valuetypid) ||
		datum_eq2(endvalue1, endvalue2, start1->valuetypid, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		if (lower_inc)
		{
			instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(startresult, valuetypid);
		}
		/* Find the middle time between start and the end instant 
		 * and compute the operator at that point */
		double time1 = start1->t;
		double time2 = end1->t;
		TimestampTz inttime = time1 + ((time2 - time1)/2);
		Datum value1 = temporalseq_value_at_timestamp1(start1, end1, inttime);
		Datum value2 = temporalseq_value_at_timestamp1(start2, end2, inttime);
		Datum intresult = operator(value1, value2);
		instants[0] = temporalinst_make(intresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(intresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, false, false);			
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value1, start1->valuetypid); FREE_DATUM(value2, start1->valuetypid);
		FREE_DATUM(intresult, valuetypid); 
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* Determine whether there is a crossing 
	   It may be the case that one of the segments is discrete and the
	   start and end values of that segment are different */
	TimestampTz crosstime;
	bool cross;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start1->flags))
		cross = tempcontseq_timestamp_at_value(start2, end2, 
			startvalue1, start1->valuetypid, &crosstime);
	else if (! MOBDB_FLAGS_GET_CONTINUOUS(start2->flags))
		cross = tempcontseq_timestamp_at_value(start1, end1, 
			startvalue2, start2->valuetypid, &crosstime);
	else 
		cross = temporalseq_intersect_at_timestamp(start1, end1, 
			start2, end2, &crosstime);
	
	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]); 
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* There is a crossing at the middle */
	instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
	instants[1] = temporalinst_make(startresult, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2,
		lower_inc, false, false);		
	pfree(instants[0]); pfree(instants[1]);
	/* Find the values at the local minimum/maximum */
	Datum cross1 = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum cross2 = temporalseq_value_at_timestamp1(start2, end2, crosstime);
	Datum crossvalue = operator(cross1, cross2);
	instants[0] = temporalinst_make(crossvalue, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1,
		true, true, false);
	pfree(instants[0]); 
	Datum endresult = operator(endvalue1, endvalue2);
	instants[0] = temporalinst_make(endresult, crosstime, valuetypid);
	instants[1] = temporalinst_make(endresult, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2,
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startresult, valuetypid); FREE_DATUM(endresult, valuetypid); 
	FREE_DATUM(cross1, start1->valuetypid); FREE_DATUM(cross2, start1->valuetypid); 
	FREE_DATUM(crossvalue, valuetypid); 
	return 3;
}

TemporalSeq **
sync_oper2_temporalseq_temporalseq_crossdisc2(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum), Datum valuetypid, int *count)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
	{
		*count = 0;
		return NULL;
	}
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum startresult, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &startresult);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(startresult, value2);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, false);
		FREE_DATUM(startresult, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		*count = 1;
		return result;
	}

	/* General case */
	int count1 = (seq1->count + seq2->count);
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count1 * 3);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count1 * 2);
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	int i = 1, j = 1, k = 0, l = 0;
	if (timestamp_cmp_internal(start1->t, inter->lower) < 0)
	{
		start1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofree[l++] = start1;
		i = temporalseq_find_timestamp(seq1, inter->lower) + 1;
	}
	else if (timestamp_cmp_internal(start2->t, inter->lower) < 0)
	{
		start2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofree[l++] = start2;
		j = temporalseq_find_timestamp(seq2, inter->lower) + 1;
	}
	bool lower_inc = inter->lower_inc;
	while (i < seq1->count && j < seq2->count)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j);
		if (timestamp_cmp_internal(end1->t, inter->upper) > 0 &&
			timestamp_cmp_internal(end2->t, inter->upper) > 0)
			break;
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;			
		}
		else if (cmp < 0)
		{
			i++;
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t);
			tofree[l++] = end1;
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		int countseq = sync_oper2_temporalseq_temporalseq_crossdisc1(&result[k], 
			start1, end1, start2, end2, lower_inc, upper_inc, operator, valuetypid);
		/* The previous step has added between one and three sequences */
		k += countseq;
		start1 = end1;
		start2 = end2;
		lower_inc = true;
	}
	*count = k;
	return result;
}

TemporalS *
sync_oper2_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	int count;
	TemporalSeq **sequences = sync_oper2_temporalseq_temporalseq_crossdisc2(
		seq1, seq2, operator, valuetypid, &count); 
	if (count == 0)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences,
		count, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper2_temporals_temporalseq_crossdisc(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		sequences[i] = sync_oper2_temporalseq_temporalseq_crossdisc2(seq1, seq, operator, 
			valuetypid, &countseqs[i]);
		totalseqs += countseqs[i];
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
		
	TemporalS *result = temporals_from_temporalseqarr(allsequences, k, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
	return result;
}

TemporalS *
sync_oper2_temporalseq_temporals_crossdisc(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	return sync_oper2_temporals_temporalseq_crossdisc(ts, seq, operator, valuetypid);
}

TemporalS *
sync_oper2_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	int count = Max(ts1->count, ts2->count);
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * count);
	int *countseqs = palloc0(sizeof(int) * count);
	int totalseqs = 0;
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		sequences[k] = sync_oper2_temporalseq_temporalseq_crossdisc2(seq1, seq2, operator, 
			valuetypid, &countseqs[k]);
		totalseqs += countseqs[k++];
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++; 
		else 
			j++;
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}
	
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int l = 0;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[l++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, l, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_oper2_temporal_temporal_crossdisc(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
		MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	Temporal *result;
	if (temp1->type == TEMPORALINST && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper2_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, operator, valuetypid);
	
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper2_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, operator, valuetypid);
	
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALSEQ) 
		result = continuous ?
			(Temporal *)sync_oper2_temporalseq_temporalseq_crossdisc(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper2_temporalseq_temporalseq(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, valuetypid, false);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = continuous ?
			(Temporal *)sync_oper2_temporalseq_temporals_crossdisc(
				(TemporalSeq *)temp1, (TemporalS *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper2_temporalseq_temporals(
				(TemporalSeq *)temp1, (TemporalS *)temp2, operator, valuetypid, false);
	
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper2_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper2_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALSEQ) 
		result = continuous ?
			(Temporal *)sync_oper2_temporals_temporalseq_crossdisc(
				(TemporalS *)temp1, (TemporalSeq *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper2_temporals_temporalseq(
				(TemporalS *)temp1, (TemporalSeq *)temp2, operator, valuetypid, false);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = continuous ?
			(Temporal *)sync_oper2_temporals_temporals_crossdisc(
				(TemporalS *)temp1, (TemporalS *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper2_temporals_temporals(
				(TemporalS *)temp1, (TemporalS *)temp2, operator, valuetypid, false);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

static int
sync_oper3_temporalseq_temporalseq_crossdisc1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, 
    bool lower_inc, bool upper_inc, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	Datum startvalue1 = temporalinst_value(start1);
	Datum endvalue1 = temporalinst_value(end1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum endvalue2 = temporalinst_value(end2);
	Datum startresult = operator(startvalue1, startvalue2, param);
	TemporalInst *instants[2];
	int k = 0;

	/* Both segments are constant */
	if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
		datum_eq(startvalue2, endvalue2, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startresult, valuetypid); 
		return 1;
	}

	/* Both segments are constant but at least one start value is different 
	 * from the corresponding end value */
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start1->flags) && 
		! MOBDB_FLAGS_GET_CONTINUOUS(start2->flags))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2, param);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid);
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* If the start or end values are equal */	
	if (datum_eq2(startvalue1, startvalue2, start1->valuetypid, start2->valuetypid) ||
		datum_eq2(endvalue1, endvalue2, start1->valuetypid, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		if (lower_inc)
		{
			instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(startresult, valuetypid);
		}
		/* Find the middle time between start and the end instant 
		 * and compute the operator at that point */
		double time1 = start1->t;
		double time2 = end1->t;
		TimestampTz inttime = time1 + ((time2 - time1)/2);
		Datum value1 = temporalseq_value_at_timestamp1(start1, end1, inttime);
		Datum value2 = temporalseq_value_at_timestamp1(start2, end2, inttime);
		Datum intresult = operator(value1, value2, param);
		instants[0] = temporalinst_make(intresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(intresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, false, false);			
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value1, start1->valuetypid); FREE_DATUM(value2, start1->valuetypid);
		FREE_DATUM(intresult, valuetypid); 
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2, param);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* Determine whether there is a crossing 
	   It may be the case that one of the segments is discrete and the
	   start and end values of that segment are different */
	TimestampTz crosstime;
	bool cross;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start1->flags))
		cross = tempcontseq_timestamp_at_value(start2, end2, 
			startvalue1, start1->valuetypid, &crosstime);
	else if (! MOBDB_FLAGS_GET_CONTINUOUS(start2->flags))
		cross = tempcontseq_timestamp_at_value(start1, end1, 
			startvalue2, start2->valuetypid, &crosstime);
	else 
		cross = temporalseq_intersect_at_timestamp(start1, end1, 
			start2, end2, &crosstime);
	
	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]); 
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2, param);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* There is a crossing at the middle */
	instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
	instants[1] = temporalinst_make(startresult, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2,
		lower_inc, false, false);		
	pfree(instants[0]); pfree(instants[1]);
	/* Find the values at the local minimum/maximum */
	Datum cross1 = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum cross2 = temporalseq_value_at_timestamp1(start2, end2, crosstime);
	Datum crossvalue = operator(cross1, cross2, param);
	instants[0] = temporalinst_make(crossvalue, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1,
		true, true, false);
	pfree(instants[0]); 
	Datum endresult = operator(endvalue1, endvalue2, param);
	instants[0] = temporalinst_make(endresult, crosstime, valuetypid);
	instants[1] = temporalinst_make(endresult, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2,
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startresult, valuetypid); FREE_DATUM(endresult, valuetypid); 
	FREE_DATUM(cross1, start1->valuetypid); FREE_DATUM(cross2, start1->valuetypid); 
	FREE_DATUM(crossvalue, valuetypid); 
	return 3;
}

TemporalSeq **
sync_oper3_temporalseq_temporalseq_crossdisc2(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid, int *count)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
	{
		*count = 0;
		return NULL;
	}
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(value1, value2, param);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		*count = 1;
		return result;
	}

	/* General case */
	int count1 = (seq1->count + seq2->count);
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count1 * 3);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count1 * 2);
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	int i = 1, j = 1, k = 0, l = 0;
	if (timestamp_cmp_internal(start1->t, inter->lower) < 0)
	{
		start1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofree[l++] = start1;
		i = temporalseq_find_timestamp(seq1, inter->lower) + 1;
	}
	else if (timestamp_cmp_internal(start2->t, inter->lower) < 0)
	{
		start2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofree[l++] = start2;
		j = temporalseq_find_timestamp(seq2, inter->lower) + 1;
	}
	bool lower_inc = inter->lower_inc;
	while (i < seq1->count && j < seq2->count)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j);
		if (timestamp_cmp_internal(end1->t, inter->upper) > 0 &&
			timestamp_cmp_internal(end2->t, inter->upper) > 0)
			break;
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;			
		}
		else if (cmp < 0)
		{
			i++;
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t);
			tofree[l++] = end1;
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		int countseq = sync_oper3_temporalseq_temporalseq_crossdisc1(&result[k],
			start1, end1, start2, end2, lower_inc, upper_inc, param, operator, valuetypid);
		/* The previous step has added between one and three sequences */
		k += countseq;
		start1 = end1;
		start2 = end2;
		lower_inc = true;
	}
	*count = k;
	return result;
}

TemporalS *
sync_oper3_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	int count;
	TemporalSeq **sequences = sync_oper3_temporalseq_temporalseq_crossdisc2(
		seq1, seq2, param, operator, valuetypid, &count); 
	if (count == 0)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences,
		count, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper3_temporals_temporalseq_crossdisc(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		sequences[i] = sync_oper3_temporalseq_temporalseq_crossdisc2(seq1, seq, 
			param, operator, valuetypid, &countseqs[i]);
		totalseqs += countseqs[i];
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, k, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
	return result;
}

TemporalS *
sync_oper3_temporalseq_temporals_crossdisc(TemporalSeq *seq, TemporalS *ts,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	return sync_oper3_temporals_temporalseq_crossdisc(ts, seq, param, 
		operator, valuetypid);
}

TemporalS *
sync_oper3_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* Previously it was Max(ts1->count, ts2->count) and was not correct ????*/
	int count = ts1->count + ts2->count;
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * count);
	int *countseqs = palloc0(sizeof(int) * count);
	int totalseqs = 0;
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		sequences[k] = sync_oper3_temporalseq_temporalseq_crossdisc2(seq1, seq2, 
			param, operator, valuetypid, &countseqs[k]);
		totalseqs += countseqs[k++];
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++; 
		else 
			j++;
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int l = 0;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[l++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, l, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_oper3_temporal_temporal_crossdisc(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid)
{
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
		MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	Temporal *result;
	if (temp1->type == TEMPORALINST && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper3_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, param, operator, valuetypid);
	
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper3_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, param, operator, valuetypid);
	
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALSEQ)
		result = continuous ?
			(Temporal *)sync_oper3_temporalseq_temporalseq_crossdisc(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, operator, valuetypid) :
			(Temporal *)sync_oper3_temporalseq_temporalseq(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, operator, valuetypid, false);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = continuous ?
			(Temporal *)sync_oper3_temporalseq_temporals_crossdisc(
				(TemporalSeq *)temp1, (TemporalS *)temp2, param, operator, valuetypid) :
			(Temporal *)sync_oper3_temporalseq_temporals(
				(TemporalSeq *)temp1, (TemporalS *)temp2, param, operator, valuetypid, false);
	
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper3_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper3_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, param, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALSEQ) 
		result = continuous ?
			(Temporal *)sync_oper3_temporals_temporalseq_crossdisc(
				(TemporalS *)temp1, (TemporalSeq *)temp2, param, operator, valuetypid) :
			(Temporal *)sync_oper3_temporals_temporalseq(
				(TemporalS *)temp1, (TemporalSeq *)temp2, param, operator, valuetypid, false);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = continuous ?
			(Temporal *)sync_oper3_temporals_temporals_crossdisc(
				(TemporalS *)temp1, (TemporalS *)temp2, param, operator, valuetypid) :
			(Temporal *)sync_oper3_temporals_temporals(
				(TemporalS *)temp1, (TemporalS *)temp2, param, operator, valuetypid, false);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

static int
sync_oper4_temporalseq_temporalseq_crossdisc1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, bool lower_inc, bool upper_inc,
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	Datum startvalue1 = temporalinst_value(start1);
	Datum endvalue1 = temporalinst_value(end1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum endvalue2 = temporalinst_value(end2);
	Datum startresult = operator(startvalue1, startvalue2, 
		start1->valuetypid, start2->valuetypid);
	TemporalInst *instants[2];
	int k = 0;

	/* Both segments are constant */
	if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
		datum_eq(startvalue2, endvalue2, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startresult, valuetypid); 
		return 1;
	}

	/* Both segments are constant but at least one start value is different 
	 * from the corresponding end value */
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start1->flags) && 
		! MOBDB_FLAGS_GET_CONTINUOUS(start2->flags))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2, 
				start1->valuetypid, start2->valuetypid);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid);
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* If the start or end values are equal */	
	if (datum_eq2(startvalue1, startvalue2, start1->valuetypid, start2->valuetypid) ||
		datum_eq2(endvalue1, endvalue2, start1->valuetypid, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		if (lower_inc)
		{
			instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(startresult, valuetypid);
		}
		/* Find the middle time between start and the end instant 
		 * and compute the operator at that point */
		double time1 = start1->t;
		double time2 = end1->t;
		TimestampTz inttime = time1 + ((time2 - time1)/2);
		Datum value1 = temporalseq_value_at_timestamp1(start1, end1, inttime);
		Datum value2 = temporalseq_value_at_timestamp1(start2, end2, inttime);
		Datum intresult = operator(value1, value2, 
			start1->valuetypid, start2->valuetypid);
		instants[0] = temporalinst_make(intresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(intresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, false, false);			
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(value1, start1->valuetypid); FREE_DATUM(value2, start1->valuetypid);
		FREE_DATUM(intresult, valuetypid); 
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2, 
				end1->valuetypid, end2->valuetypid);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* Determine whether there is a crossing 
	   It may be the case that one of the segments is discrete and the
	   start and end values of that segment are different */
	TimestampTz crosstime;
	bool cross;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(start1->flags))
		cross = tempcontseq_timestamp_at_value(start2, end2, 
			startvalue1, start1->valuetypid, &crosstime);
	else if (! MOBDB_FLAGS_GET_CONTINUOUS(start2->flags))
		cross = tempcontseq_timestamp_at_value(start1, end1, 
			startvalue2, start2->valuetypid, &crosstime);
	else 
		cross = temporalseq_intersect_at_timestamp(start1, end1, 
			start2, end2, &crosstime);
	
	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
		instants[1] = temporalinst_make(startresult, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]); 
		/* Compute the operator at the end instant */
		if (upper_inc)
		{
			Datum endresult = operator(endvalue1, endvalue2, 
				end1->valuetypid, end2->valuetypid);
			instants[0] = temporalinst_make(endresult, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
			FREE_DATUM(endresult, valuetypid); 
		}
		FREE_DATUM(startresult, valuetypid); 
		return k;
	}

	/* There is a crossing at the middle */
	instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
	instants[1] = temporalinst_make(startresult, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2,
		lower_inc, false, false);		
	pfree(instants[0]); pfree(instants[1]);
	/* Find the values at the local minimum/maximum */
	Datum cross1 = temporalseq_value_at_timestamp1(start1, end1, crosstime);
	Datum cross2 = temporalseq_value_at_timestamp1(start2, end2, crosstime);
	Datum crossvalue = operator(cross1, cross2, 
			start1->valuetypid, start2->valuetypid);
	instants[0] = temporalinst_make(crossvalue, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1,
		true, true, false);
	pfree(instants[0]); 
	Datum endresult = operator(endvalue1, endvalue2, 
		end1->valuetypid, end2->valuetypid);
	instants[0] = temporalinst_make(endresult, crosstime, valuetypid);
	instants[1] = temporalinst_make(endresult, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2,
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startresult, valuetypid); FREE_DATUM(endresult, valuetypid); 
	FREE_DATUM(cross1, start1->valuetypid); FREE_DATUM(cross2, start1->valuetypid); 
	FREE_DATUM(crossvalue, valuetypid); 
	return 3;
}

TemporalSeq **
sync_oper4_temporalseq_temporalseq_crossdisc2(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid, int *count)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
	{
		*count = 0;
		return NULL;
	}
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(value1, value2, seq1->valuetypid, seq2->valuetypid);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		*count = 1;
		return result;
	}

	/* General case */
	int count1 = (seq1->count + seq2->count);
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count1 * 3);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count1 * 2);
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	int i = 1, j = 1, k = 0, l = 0;
	if (timestamp_cmp_internal(start1->t, inter->lower) < 0)
	{
		start1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofree[l++] = start1;
		i = temporalseq_find_timestamp(seq1, inter->lower) + 1;
	}
	else if (timestamp_cmp_internal(start2->t, inter->lower) < 0)
	{
		start2 = temporalseq_at_timestamp(seq2, inter->lower);
		tofree[l++] = start2;
		j = temporalseq_find_timestamp(seq2, inter->lower) + 1;
	}
	bool lower_inc = inter->lower_inc;
	while (i < seq1->count && j < seq2->count)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j);
		if (timestamp_cmp_internal(end1->t, inter->upper) > 0 &&
			timestamp_cmp_internal(end2->t, inter->upper) > 0)
			break;
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;			
		}
		else if (cmp < 0)
		{
			i++;
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t);
			tofree[l++] = end1;
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		int countseq = sync_oper4_temporalseq_temporalseq_crossdisc1(&result[k], 
			start1, end1, start2, end2, lower_inc, upper_inc, operator, valuetypid);
		/* The previous step has added between one and three sequences */
		k += countseq;
		start1 = end1;
		start2 = end2;
		lower_inc = true;
	}
	*count = k;
	return result;
}

TemporalS *
sync_oper4_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	int count;
	TemporalSeq **sequences = sync_oper4_temporalseq_temporalseq_crossdisc2(
		seq1, seq2, operator, valuetypid, &count);
	if (count == 0)
		return NULL;

	TemporalS *result = temporals_from_temporalseqarr(sequences,
		count, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper4_temporals_temporalseq_crossdisc(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int *countseqs = palloc0(sizeof(int) * ts->count);
	int totalseqs = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		sequences[i] = sync_oper4_temporalseq_temporalseq_crossdisc2(seq1, seq, operator, 
			valuetypid, &countseqs[i]);
		totalseqs += countseqs[i];
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[k++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, k, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
	return result;
}

TemporalS *
sync_oper4_temporalseq_temporals_crossdisc(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	return sync_oper4_temporals_temporalseq_crossdisc(ts, seq, operator, valuetypid);
}

TemporalS *
sync_oper4_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* Previously it was Max(ts1->count, ts2->count) and was not correct ????*/
	int count = ts1->count + ts2->count;
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * count);
	int *countseqs = palloc0(sizeof(int) * count);
	int totalseqs = 0;
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		sequences[k] = sync_oper4_temporalseq_temporalseq_crossdisc2(seq1, seq2, operator, 
			valuetypid, &countseqs[k]);
		totalseqs += countseqs[k++];
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++; 
		else 
			j++;
	}
	if (totalseqs == 0)
	{
		pfree(sequences); pfree(countseqs);
		return NULL;
	}

	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int l = 0;
	for (int i = 0; i < k; i++)
	{
		for (int j = 0; j < countseqs[i]; j++)
			allsequences[l++] = sequences[i][j];
		if (sequences[i] != NULL)
			pfree(sequences[i]);
	}
	TemporalS *result = temporals_from_temporalseqarr(allsequences, l, true);

	pfree(sequences); pfree(countseqs);
	for (int i = 0; i < totalseqs; i++)
		pfree(allsequences[i]);
	pfree(allsequences); 
	
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_oper4_temporal_temporal_crossdisc(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid)
{
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(temp1->flags) || 
		MOBDB_FLAGS_GET_CONTINUOUS(temp2->flags);
	Temporal *result;
	if (temp1->type == TEMPORALINST && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper4_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALINST && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, operator, valuetypid);
	
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALSEQ) 
		result = (Temporal *)sync_oper4_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALI && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, operator, valuetypid);
	
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALSEQ)
		result = continuous ?
			(Temporal *)sync_oper4_temporalseq_temporalseq_crossdisc(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper4_temporalseq_temporalseq(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator, valuetypid, false);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = continuous ?
			(Temporal *)sync_oper4_temporalseq_temporals_crossdisc(
				(TemporalSeq *)temp1, (TemporalS *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper4_temporalseq_temporals(
				(TemporalSeq *)temp1, (TemporalS *)temp2, operator, valuetypid, false);
	
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALINST) 
		result = (Temporal *)sync_oper4_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALI) 
		result = (Temporal *)sync_oper4_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, operator, valuetypid);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALSEQ) 
		result = continuous ?
			(Temporal *)sync_oper4_temporals_temporalseq_crossdisc(
				(TemporalS *)temp1, (TemporalSeq *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper4_temporals_temporalseq(
				(TemporalS *)temp1, (TemporalSeq *)temp2, operator, valuetypid, false);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = continuous ?
			(Temporal *)sync_oper4_temporals_temporals_crossdisc(
				(TemporalS *)temp1, (TemporalS *)temp2, operator, valuetypid) :
			(Temporal *)sync_oper4_temporals_temporals(
				(TemporalS *)temp1, (TemporalS *)temp2, operator, valuetypid, false);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************/

