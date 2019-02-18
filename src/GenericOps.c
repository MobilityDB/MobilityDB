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

/* 1) There are 4 families of functions accounting for 
 *    - binary operators, such as spatial relationships functions (e.g. 
 *      intersects). 
 *    - ternary operators, such as spatial relationships functions (e.g. 
 *      tdwithin) that need an additional parameter. 
 *    - quaternary operators which apply binary operators (e.g. + or <) to
 *      temporal numeric types that can be of different base type (that is,
 *      integer and float), and thus the third and fourth arguments are the
 *      Oids of the first two arguments.
 *  2) For each of the previous families, there are two set of functions
 *     accounting for the following three cases
 *    - the operator is applied to the synchronized arguments without looking
 *      for the crossings (or local minimum/maximum). These are typically 
 *      applied to temporal discrete types.
 *    - the operator is applied to the synchronized arguments to which the
 *      crossings have been added. These are typically applied to temporal  
 *      continuous types. There are two cases depending on whether the resulting  
 *      temporal type is discrete (e.g., < for temporal floats that results in a  
 *      temporal Boolean) or continuous (e.g., distance for temporal points that  
 *      results in a temporal float).
 * The functions that take two temporal types as argument suppose that they 
 * have been previously synchronized and added the crossings if necessary
 * depending on the function. For example tfloat + tfloat only needs to 
 * synchronize the arguments while tfloat * tfloat requires in addition to add
 * the crossings.
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
 *     synchronizes the sequences adding the crossings and applies the * 
 *     operator to each instant. The result is a tfloatseq.
 *   - tfloatseq < tfloatseq => oper4_temporalseq_temporalseq_crossdisc
 *     synchronizes the sequences, applies the < operator to each instant, 
 *     and if there is a crossing in the middle of two subsequent pairs of 
 *     instants add an instant sequence and the crossing. The result is a 
 *     tfloats.
 */

#include "TemporalTypes.h"

/*****************************************************************************
 * Functions that apply the operator to the composing instants without
 * looking for crossings (or local minimum/maximum).
 * The functions suppose that the two temporal values are synchronized.
 * This should be ensured by the calling function. 
 *****************************************************************************/

/*****************************************************************************
 * Version of the functions where the operator takes 2 arguments and apply
 * the operator to the composing instants without looking for crossings
 * The last argument states whether we are computing base <oper> temporal
 * or temporal <oper> base
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

/* Temporal op Temporal */

TemporalInst *
oper2_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
	return temporalinst_make(value, inst1->t, valuetypid);
}

TemporalI *
oper2_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti1->count);
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
		instants[i] = temporalinst_make(value, inst1->t, valuetypid); 
		FREE_DATUM(value, valuetypid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti1->count);
	for (int i = 0; i < ti1->count; i++)
		pfree(instants[i]);
	pfree(instants);
    return result;
}

TemporalSeq *
oper2_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq1->count);
	TemporalInst *inst1, *inst2;
	Datum value;
	for (int i = 0; i < seq1->count - 1; i++)
	{
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, i);
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
		instants[i] = temporalinst_make(value, inst1->t, valuetypid);
		FREE_DATUM(value, valuetypid);
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
	if (type_is_continuous(valuetypid) || seq1->period.upper_inc)
	{
		inst2 = temporalseq_inst_n(seq2, seq1->count - 1);
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
	}
	else
		value = temporalinst_value(instants[seq1->count - 2]);
	instants[seq1->count - 1] = temporalinst_make(value, inst1->t, valuetypid); 		

	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq1->count, seq1->period.lower_inc, seq1->period.upper_inc, true);
	
	for (int i = 0; i < seq1->count; i++)
		pfree(instants[i]);
	pfree(instants);
    return result;
}

TemporalS *
oper2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = oper2_temporalseq_temporalseq(seq1, seq2,
			operator, valuetypid); 
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts1->count,	true);
	
	for (int i = 0; i < ts1->count; i++)
			pfree(sequences[i]);
	pfree(sequences);
    return result;
}

/*****************************************************************************/
/* Dispatch functions */

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

Temporal *
oper2_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	Temporal *result;
	if (temp1->type == TEMPORALINST)
		result = (Temporal *)oper2_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALI)
		result = (Temporal *)oper2_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, 
			operator, valuetypid);
	else if (temp1->type == TEMPORALSEQ)
		result = (Temporal *)oper2_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			operator, valuetypid);
	else if (temp1->type == TEMPORALS)
		result = (Temporal *)oper2_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			operator, valuetypid);
    else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Version of the functions where the operator takes 3 arguments and apply
 * the operator to the composing instants without looking for crossings
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

/*****************************************************************************/

/* Temporal op Temporal */

TemporalInst *
oper3_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2), param);
	return temporalinst_make(value, inst1->t, valuetypid);
}

TemporalI *
oper3_temporali_temporali(TemporalI *ti1, TemporalI *ti2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti1->count);
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2), param);
		instants[i] = temporalinst_make(value, inst1->t, valuetypid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti1->count);
	for (int i = 0; i < ti1->count; i++)
		pfree(instants[i]);
	pfree(instants);
    return result;
}

TemporalSeq *
oper3_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq1->count);
	for (int i = 0; i < seq1->count; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, i);
		Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2), param);
		instants[i] = temporalinst_make(value, inst1->t, valuetypid); 
		FREE_DATUM(value, valuetypid);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq1->count, seq1->period.lower_inc, seq1->period.upper_inc, true);
	for (int i = 0; i < seq1->count; i++)
		pfree(instants[i]);
	pfree(instants);
    return result;
}

TemporalS *
oper3_temporals_temporals(TemporalS *ts1, TemporalS *ts2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = oper3_temporalseq_temporalseq(seq1, seq2, param,
			operator, valuetypid); 
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts1->count,	true);
	
	for (int i = 0; i < ts1->count; i++)
			pfree(sequences[i]);
	pfree(sequences);
    return result;
}

/*****************************************************************************
 * Version of the functions where the operator takes 4 arguments and apply
 * the operator to the composing instants without looking for crossings
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

/* Temporal op Temporal */

TemporalInst *
oper4_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return temporalinst_make(
		operator(temporalinst_value(inst1), temporalinst_value(inst2), 
			inst1->valuetypid, inst2->valuetypid), 
		inst1->t, valuetypid);
}

TemporalI *
oper4_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti1->count);
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		instants[i] = temporalinst_make(
			operator(temporalinst_value(inst1), temporalinst_value(inst2), 
				inst1->valuetypid, inst2->valuetypid), 
			inst1->t, valuetypid); 
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti1->count);
	for (int i = 0; i < ti1->count; i++)
		pfree(instants[i]);
	pfree(instants);
    return result;
}

TemporalSeq *
oper4_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq1->count);
	TemporalInst *inst1, *inst2;
	Datum value;
	for (int i = 0; i < seq1->count-1; i++)
	{
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, i);
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2), 
			inst1->valuetypid, inst2->valuetypid);
		instants[i] = temporalinst_make(value, inst1->t, valuetypid); 
		FREE_DATUM(value, valuetypid);
	}
	/* The last two values of discrete sequences with exclusive upper bound 
	   must be equal */
	inst1 = temporalseq_inst_n(seq1, seq1->count-1);
	if (type_is_continuous(valuetypid) || seq1->period.upper_inc)
	{
		inst2 = temporalseq_inst_n(seq2, seq1->count-1);
		value = operator(temporalinst_value(inst1), temporalinst_value(inst2), 
			inst1->valuetypid, inst2->valuetypid);
	}
	else
		value = temporalinst_value(instants[seq1->count-2]);
	instants[seq1->count-1] = temporalinst_make(value, inst1->t, valuetypid); 		

	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq1->count, seq1->period.lower_inc, seq1->period.upper_inc, true);
	for (int i = 0; i < seq1->count; i++)
		pfree(instants[i]);
	pfree(instants);
    return result;
}

TemporalS *
oper4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = oper4_temporalseq_temporalseq(seq1, seq2, 
			operator, valuetypid); 
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, 
		ts1->count,	true);
	
	for (int i = 0; i < ts1->count; i++)
			pfree(sequences[i]);
	pfree(sequences);
    return result;
}

/*****************************************************************************/
/* Dispatch functions */

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

Temporal *
oper4_temporal_temporal(Temporal *temp1, Temporal *temp2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	Temporal *result;
	if (temp1->type == TEMPORALINST)
		result = (Temporal *)oper4_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, operator,
			valuetypid);
	else if (temp1->type == TEMPORALI)
		result = (Temporal *)oper4_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, operator,
			valuetypid);
	else if (temp1->type == TEMPORALSEQ)
		result = (Temporal *)oper4_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, operator,
			valuetypid);
	else if (temp1->type == TEMPORALS)
		result = (Temporal *)oper4_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2, operator,
			valuetypid);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return result;
}

/*****************************************************************************
 * Functions that apply the operator to the composing instants and to the 
 * crossings when the resulting value is discrete as required for comparisons 
 * (e.g., #<) and spatial relationships (e.g., tintersects).
 * These functions suppose that the two temporal values are synchronized. 
 * This should be ensured by the calling function. 
 *****************************************************************************/

void
oper2_temporalseq_temporalseq_crossdisc1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, bool lower_inc, bool upper_inc,
	Datum (*operator)(Datum, Datum), Oid valuetypid, int *count)
{
	Datum startvalue1 = temporalinst_value(start1);
	Datum endvalue1 = temporalinst_value(end1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum endvalue2 = temporalinst_value(end2);
	Datum startvalue = operator(startvalue1, startvalue2);
	TemporalInst *instants[2];
	
	/* Both segments are constant */
	if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
		datum_eq(startvalue2, endvalue2, start2->valuetypid))
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid); 
		*count = 1;
		return;
	}

	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	bool cross = temporalseq_intersect_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	
	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]); 
		FREE_DATUM(startvalue, valuetypid); 
		*count = 1;
		return;
	}

	Datum endvalue = operator(endvalue1, endvalue2);
	int k = 0;
	/* If the crossing is at the start instant */	
	if (crosstime == start1->t)
	{
		if (lower_inc)
		{
			/* Compute the operator at the start instant */
			instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
		}		
		/* Compute the operator between the start and end instants */
		instants[0] = temporalinst_make(endvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);			
		pfree(instants[0]); pfree(instants[1]); 
		FREE_DATUM(startvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid); 
		*count = k;
		return;
	}

	/* If the crossing is at the end instant */	
	if (crosstime == end1->t)
	{
		/* Compute the operator beteen the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[k++] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); pfree(instants[1]);
		if (upper_inc)
		{
			/* Compute the operator at the end instant */
			instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
			result[k++] = temporalseq_from_temporalinstarr(instants, 1,
				true, true, false);
			pfree(instants[0]);
		}
		FREE_DATUM(startvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid); 
		*count = k;
		return;
	}

	/* If there is a crossing at the middle */
	instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
	instants[1] = temporalinst_make(startvalue, crosstime, valuetypid);
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
	instants[0] = temporalinst_make(endvalue, crosstime, valuetypid);
	instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2,
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startvalue, valuetypid); 
	FREE_DATUM(endvalue, valuetypid); 
	FREE_DATUM(cross1, start1->valuetypid); 
	FREE_DATUM(cross2, start1->valuetypid); 
	FREE_DATUM(crossvalue, valuetypid); 
	*count = 3;
	return;
}

static TemporalSeq **
oper2_temporalseq_temporalseq_crossdisc2(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum), Oid valuetypid, int *count)
{
	if (seq1->count == 1)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		TemporalInst *operinst = oper2_temporalinst_temporalinst(inst1, inst2, 
			operator, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&operinst, 1, 
			true, true, false);
		pfree(operinst);
		*count = 1;
		return result;
	}
	
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * seq1->count * 3);
	int k = 0;
	int countseq;
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool lower_inc = seq1->period.lower_inc;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		bool upper_inc = (i == seq1->count-1) ? seq1->period.upper_inc : false;
		oper2_temporalseq_temporalseq_crossdisc1(&result[k], start1, end1, 
			start2, end2, lower_inc, upper_inc, operator, valuetypid, &countseq);
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
oper2_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	int count;
    TemporalSeq **sequences = oper2_temporalseq_temporalseq_crossdisc2(
		seq1, seq2, operator, valuetypid, &count); 
    TemporalS *result = temporals_from_temporalseqarr(sequences,
		count, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************/

TemporalS *
oper2_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	int *countseqs = palloc0(sizeof(int) * ts1->count);
	int totalseqs = 0, countseq;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = oper2_temporalseq_temporalseq_crossdisc2(seq1, seq2, operator, 
			valuetypid, &countseq);
		countseqs[i] = countseq;
		totalseqs += countseq;
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
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

void
oper3_temporalseq_temporalseq_crossdisc1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, 
	bool lower_inc, bool upper_inc, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, int *count)
{
	Datum startvalue1 = temporalinst_value(start1);
	Datum endvalue1 = temporalinst_value(end1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum endvalue2 = temporalinst_value(end2);
	Datum startvalue = operator(startvalue1, startvalue2, param);
	TemporalInst *instants[2];
	
	/* Both segments are constant */
	if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
		datum_eq(startvalue2, endvalue2, start2->valuetypid))
	{
		/* Compute the operator at the start instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid); 
		*count = 1;
		return;
	}

	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	bool cross = temporalseq_intersect_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	
	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid); 
		*count = 1;
		return;
	}

	Datum endvalue = operator(endvalue1, endvalue2, param);
	/* If the crossing is at the start instant */	
	if (crosstime == start1->t)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		pfree(instants[0]); 
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid); 
		*count = 2;
		return;
	}

	/* If the crossing is at the end instant */	
	if (crosstime == end1->t)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); 
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		pfree(instants[0]); pfree(instants[1]); 
		FREE_DATUM(startvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid); 
		*count = 2;
		return;
	}

	/* If there is a crossing at the middle */
	instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
	instants[1] = temporalinst_make(startvalue, crosstime, valuetypid);
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
	instants[0] = temporalinst_make(endvalue, crosstime, valuetypid);
	instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2,
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startvalue, valuetypid); 
	FREE_DATUM(endvalue, valuetypid); 
	FREE_DATUM(cross1, start1->valuetypid); 
	FREE_DATUM(cross2, start1->valuetypid); 
	FREE_DATUM(crossvalue, valuetypid); 
	*count = 3;
	return;
}

static TemporalSeq **
oper3_temporalseq_temporalseq_crossdisc2(TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid, int *count)
{
	if (seq1->count == 1)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		TemporalInst *operinst = oper3_temporalinst_temporalinst(inst1, inst2, param,
			operator, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&operinst, 1, 
			true, true, false);
		pfree(operinst);
		*count = 1;
		return result;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * seq1->count * 3);
	int k = 0;
	int countseq;
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool lower_inc = seq1->period.lower_inc;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		bool upper_inc = (i == seq1->count-1) ? seq1->period.upper_inc : false;
		oper3_temporalseq_temporalseq_crossdisc1(&result[k], start1, end1, 
			start2, end2, lower_inc, upper_inc, param, operator, valuetypid, &countseq);
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
oper3_temporalseq_temporalseq_crossdisc(
	TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	int count;
    TemporalSeq **sequences = oper3_temporalseq_temporalseq_crossdisc2(
		seq1, seq2, param, operator, valuetypid, &count); 
    TemporalS *result = temporals_from_temporalseqarr(sequences,
		count, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************/

TemporalS *
oper3_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, Datum param,
	Datum (*operator)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	int *countseqs = palloc0(sizeof(int) * ts1->count);
	int totalseqs = 0, countseq;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = oper3_temporalseq_temporalseq_crossdisc2(seq1, seq2, 
			param, operator, valuetypid, &countseq);
		countseqs[i] = countseq;
		totalseqs += countseq;
	}
	TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
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

static void
oper4_temporalseq_base_crossdisc1(TemporalSeq **result,
	TemporalInst *inst1, TemporalInst *inst2, 
	bool lower_inc, bool upper_inc, Datum value, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, int *count, bool invert)
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
		*count = 1;
		return;
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
		*count = 1;
		return;
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
	*count = 3;
	return;
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
	int k = 0, countseq;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	bool lower_inc = seq->period.lower_inc;
	for (int i = 1; i < seq->count; i++)
	{
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		oper4_temporalseq_base_crossdisc1(&result[k], inst1, inst2, lower_inc, 
			upper_inc, value, operator, datumtypid, valuetypid, &countseq, invert);
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

void
oper4_temporalseq_temporalseq_crossdisc1(TemporalSeq **result,
	TemporalInst *start1, TemporalInst *end1, 
	TemporalInst *start2, TemporalInst *end2, bool lower_inc, bool upper_inc,
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid, int *count)
{
	Datum startvalue1 = temporalinst_value(start1);
	Datum endvalue1 = temporalinst_value(end1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum endvalue2 = temporalinst_value(end2);
	Datum startvalue = operator(startvalue1, startvalue2, 
		start1->valuetypid, start2->valuetypid);
	TemporalInst *instants[2];

	/* Both segments are constant */
	if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
		datum_eq(startvalue2, endvalue2, start2->valuetypid))
	{
		/* Compute the operator at the start instant */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2, 
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]);
		FREE_DATUM(startvalue, valuetypid); 
		*count = 1;
		return;
	}

	/* Determine whether there is a crossing */
	TimestampTz crosstime;
	bool cross = temporalseq_intersect_at_timestamp(start1, end1, 
		start2, end2, &crosstime);
	
	/* If there is no crossing */	
	if (!cross)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, upper_inc, false);
		pfree(instants[0]); pfree(instants[1]); 
		FREE_DATUM(startvalue, valuetypid); 
		*count = 1;
		return;
	}

	Datum endvalue = operator(endvalue1, endvalue2, 
			start1->valuetypid, start2->valuetypid);
	/* If the crossing is at the start instant */	
	if (crosstime == start1->t)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		pfree(instants[0]); 
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 2,
			false, upper_inc, false);			
		pfree(instants[0]); pfree(instants[1]); 
		FREE_DATUM(startvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid); 
		*count = 2;
		return;
	}

	/* If the crossing is at the end instant */	
	if (crosstime == end1->t)
	{
		/* Compute the operator at the start and end instants */
		instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
		instants[1] = temporalinst_make(startvalue, end1->t, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(instants, 2,
			lower_inc, false, false);
		pfree(instants[0]); 
		instants[0] = temporalinst_make(endvalue, end1->t, valuetypid);
		result[1] = temporalseq_from_temporalinstarr(instants, 1,
			true, true, false);
		pfree(instants[0]); pfree(instants[1]); 
		FREE_DATUM(startvalue, valuetypid); 
		FREE_DATUM(endvalue, valuetypid); 
		*count = 2;
		return;
	}

	/* If there is a crossing at the middle */
	instants[0] = temporalinst_make(startvalue, start1->t, valuetypid);
	instants[1] = temporalinst_make(startvalue, crosstime, valuetypid);
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
	instants[0] = temporalinst_make(endvalue, crosstime, valuetypid);
	instants[1] = temporalinst_make(endvalue, end1->t, valuetypid);
	result[2] = temporalseq_from_temporalinstarr(instants, 2,
		false, upper_inc, false);
	pfree(instants[0]); pfree(instants[1]);
	FREE_DATUM(startvalue, valuetypid); 
	FREE_DATUM(endvalue, valuetypid); 
	FREE_DATUM(cross1, start1->valuetypid); 
	FREE_DATUM(cross2, start1->valuetypid); 
	FREE_DATUM(crossvalue, valuetypid); 
	*count = 3;
	return;
}
static TemporalSeq **
oper4_temporalseq_temporalseq_crossdisc2(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid, int *count)
{
	if (seq1->count == 1)
	{
		TemporalSeq **result = palloc(sizeof(TemporalSeq *));
		TemporalInst *inst1 = temporalseq_inst_n(seq1, 0);
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		TemporalInst *operinst = oper4_temporalinst_temporalinst(inst1, inst2,
			operator, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&operinst, 1, 
			true, true, false);
		pfree(operinst);
		*count = 1;
		return result;
	}

	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * seq1->count * 3);
	int k = 0;
	int countseq;
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	bool lower_inc = seq1->period.lower_inc;
	for (int i = 1; i < seq1->count; i++)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, i);
		bool upper_inc = (i == seq1->count - 1) ? seq1->period.upper_inc : false;
		oper4_temporalseq_temporalseq_crossdisc1(&result[k], start1, end1, 
			start2, end2, lower_inc, upper_inc, operator, valuetypid, &countseq);
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
oper4_temporalseq_temporalseq_crossdisc(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	int count;
    TemporalSeq **sequences = oper4_temporalseq_temporalseq_crossdisc2(
		seq1, seq2, operator, valuetypid, &count); 
    TemporalS *result = temporals_from_temporalseqarr(sequences,
		count, true);
		
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


TemporalS *
oper4_temporals_temporals_crossdisc(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalSeq ***sequences = palloc(sizeof(TemporalSeq *) * ts1->count);
	int *countseqs = palloc0(sizeof(int) * ts1->count);
	int totalseqs = 0, countseq;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		sequences[i] = oper4_temporalseq_temporalseq_crossdisc2(seq1, seq2, 
			operator, valuetypid, &countseq);
		countseqs[i] = countseq;
		totalseqs += countseq;
	}
	TemporalS *result = NULL;
	if (totalseqs != 0)
	{
		TemporalSeq **allsequences = palloc(sizeof(TemporalSeq *) * totalseqs);
		int k = 0;
		for (int i = 0; i < ts1->count; i++)
		{
			for (int j = 0; j < countseqs[i]; j++)
				allsequences[k++] = sequences[i][j];
			if (sequences[i] != NULL)
				pfree(sequences[i]);
		}
		result = temporals_from_temporalseqarr(allsequences, k, true);
		for (int i = 0; i < totalseqs; i++)
			pfree(allsequences[i]);
		pfree(allsequences); 		
	}

	pfree(sequences); pfree(countseqs);
	
    return result;
}

/*****************************************************************************/

