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
	TemporalInst *inst1, TemporalInst *inst2, 
	bool lower_inc, bool upper_inc, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	/* Value is equal to the lower bound */
	if (datum_eq(temporalinst_value(inst1), value, inst1->valuetypid))
	{
		/* Compute the operator at the start instant */
		TemporalInst *instants[2];
		Datum value1 = invert ?
			operator(value, temporalinst_value(inst1), datumtypid, inst1->valuetypid) :
			operator(temporalinst_value(inst1), value, inst1->valuetypid, datumtypid);
		instants[0] = temporalinst_make(value1, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value1, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		FREE_DATUM(value1, valuetypid);
		pfree(instants[0]); pfree(instants[1]);
		return 1;
	}
	
	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	bool cross = tempcontseq_timestamp_at_value(inst1, inst2, value, 
		datumtypid, &crosstime);

	/* If there is no crossing of the crossing is at a bound */	
	if (!cross || crosstime == inst1->t || crosstime == inst2->t)
	{
		/* Compute the operator at the start instant */
		TemporalInst *instants[2];
		Datum value1 = invert ?
			operator(value, temporalinst_value(inst1), datumtypid, inst1->valuetypid) :
			operator(temporalinst_value(inst1), value, inst1->valuetypid, datumtypid);
		instants[0] = temporalinst_make(value1, inst1->t, valuetypid);
		instants[1] = temporalinst_make(value1, inst2->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		FREE_DATUM(value1, valuetypid); 
		pfree(instants[0]); pfree(instants[1]);
		return 1;
	}
	
	/* If there is a crossing at the middle
	 * Compute the operator from the start instant to the crossing */
	TemporalInst *instants[2];
	Datum value1 = invert ?
		operator(value, temporalinst_value(inst1), datumtypid, inst1->valuetypid) :
		operator(temporalinst_value(inst1), value, inst1->valuetypid, datumtypid);
	instants[0] = temporalinst_make(value1, inst1->t, valuetypid);
	instants[1] = temporalinst_make(value1, crosstime, valuetypid);
	result[0] = temporalseq_from_temporalinstarr(instants, 2, 
		lower_inc, false, false);
	FREE_DATUM(value1, valuetypid);
	pfree(instants[0]); pfree(instants[1]); 
	/* Compute operator at the cross 
	   Due to floating point precision we cannot compute the operator at the
	   crosstime as follows
			value1 = temporalseq_value_at_timestamp1(inst1, inst2, crosstime);
	   Since this operator is (currently) called only for tfloat then we 
	   assume value1 = value */
	Datum value2 = operator(value, value, datumtypid, datumtypid);
	instants[0] = temporalinst_make(value2, crosstime, valuetypid);
	result[1] = temporalseq_from_temporalinstarr(instants, 1, 
		true, true, false);
	FREE_DATUM(value2, valuetypid);
	pfree(instants[0]); 
	/* Find the middle time between crossing and the end instant 
	 * and compute the operator at that point */
	double time1 = crosstime;
	double time2 = inst2->t;
	TimestampTz inttime = time1 + ((time2 - time1)/2);
	value1 = temporalseq_value_at_timestamp1(inst1, inst2, inttime);
	value2 = invert ?
		operator(value, value1, datumtypid, inst1->valuetypid) :
		operator(value1, value, inst1->valuetypid, datumtypid);
	instants[0] = temporalinst_make(value2, crosstime, valuetypid);
	instants[1] = temporalinst_make(value2, inst2->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2, 
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(value1, valuetypid); FREE_DATUM(value2, valuetypid); 
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

/*****************************************************************************/

