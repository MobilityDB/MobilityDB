/*****************************************************************************
 *
 * lifting.c
 *	Generic functions for lifting functions and operators on temporal types.
 *
 * These functions are used for lifting arithmetic operators (+, -, *, /), 
 * Boolean operators (and, or, not), comparisons (<, <=, >, >=), 
 * distance, spatial relationships, etc.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/* 1) There are 3 families of functions accounting for 
 *	- binary functions, such as spatial relationships functions (e.g. 
 *	  intersects). 
 *	- ternary functions, such as spatial relationships functions that need 
 *	  an additional parameter (e.g. tdwithin). 
 *	- quaternary functions which apply binary operators (e.g. + or <) to
 *	  temporal numeric types that can be of different base type (that is,
 *	  integer and float), and thus the third and fourth arguments are the
 *	  Oids of the first two arguments.
 *  2) For each of the previous families, there are two set of functions
 *	 depending on whether the interpolation of the resulting temporal type is
 *   stepwise (e.g., for temporal floats that results in a temporal Boolean)
 *	 or linear (e.g., distance for temporal points that results in a 
 *	 temporal float).
 *  3) For each of the previous cases there are two set of functions
 *	 depending on whether the arguments are 
 *	 - a temporal type and a base type. In this case the operand is applied 
 *	   to each instant of the temporal type.
 *	 - two temporal types. In this case the operands must be synchronized 
 *	   and the function is applied to each pair of synchronized instants. 
 *	   Furthermore, some functions require in addition to add intermediate
 *	   points between synchronized instants to take into account the crossings
 *	   or the turning points (or local minimum/maximum) of the function.
 *	   For example, tfloat + tfloat only needs to synchronize the arguments 
 *	   while tfloat * tfloat requires in addition to add the turning point, 
 *	   which is defined at the middle between the two instants in which the
 *	   linear functions defined by the arguments take the value 0.
 * 
 * Examples
 *   - tfloatseq * base => tfunc4_temporalseq_base
 *	 applies the * operator to each instant.
 *   - tfloatseq < base => tfunc4_temporalseq_base_cross
 *	 synchronizes the sequences, applies the < operator to each instant, 
 *	 and if the tfloatseq is equal to base in the middle of two consecutive
 *	 instants add an instant sequence at the crossing. The result is a 
 *	 tfloats.
 *   - tfloatseq + tfloatseq => tfunc4_temporalseq_temporalseq
 *	 synchronizes the sequences and applies the + operator to each instant.
 *   - tfloatseq * tfloatseq => tfunc4_temporalseq_temporalseq_crosscont
 *	 synchronizes the sequences adding the turning points and applies the *
 *	 operator to each instant. The result is a tfloatseq.
 *   - tfloatseq < tfloatseq => tfunc4_temporalseq_temporalseq_cross
 *	 synchronizes the sequences, applies the < operator to each instant, 
 *	 and if there is a crossing in the middle of two consecutive pairs of 
 *	 instants add an instant sequence and the crossing. The result is a 
 *	 tfloats.
 */

#include "lifting.h"

#include <utils/timestamp.h>

#include "period.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "temporal_util.h"

/*****************************************************************************
 * Functions where the argument is a temporal type. 
 * The funcion is applied to the composing instants.
 *****************************************************************************/

TemporalInst *
tfunc1_temporalinst(TemporalInst *inst, Datum (*func)(Datum), Oid valuetypid)
{
	Datum value = func(temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

static TemporalI *
tfunc1_temporali(TemporalI *ti, Datum (*func)(Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc1_temporalinst(inst, func, valuetypid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalSeq *
tfunc1_temporalseq(TemporalSeq *seq, Datum (*func)(Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc1_temporalinst(inst, func, valuetypid);
	}
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) && 
		linear_interpolation(valuetypid);
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, 
		linear, true);
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalS *
tfunc1_temporals(TemporalS *ts, Datum (*func)(Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc1_temporalseq(seq, func, valuetypid);
	}
	bool linear = MOBDB_FLAGS_GET_LINEAR(ts->flags) && 
		linear_interpolation(valuetypid);
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count,
		linear, true);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* Dispatch function */

Temporal *
tfunc1_temporal(Temporal *temp, Datum (*func)(Datum), Oid valuetypid)
{
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc1_temporalinst((TemporalInst *)temp,
			func, valuetypid);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc1_temporali((TemporalI *)temp,
			func, valuetypid);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tfunc1_temporalseq((TemporalSeq *)temp,
			func, valuetypid);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tfunc1_temporals((TemporalS *)temp,
			func, valuetypid);
	return result;
}

/*****************************************************************************/

TemporalInst *
tfunc2_temporalinst(TemporalInst *inst, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid)
{
	Datum value = func(temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalI *
tfunc2_temporali(TemporalI *ti, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc2_temporalinst(inst, param, func, valuetypid);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalSeq *
tfunc2_temporalseq(TemporalSeq *seq, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc2_temporalinst(inst, param, func, valuetypid);
	}
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) && 
		linear_interpolation(valuetypid);
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, 
		linear, true);
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalS *
tfunc2_temporals(TemporalS *ts, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc2_temporalseq(seq, param, func, valuetypid);
	}
	bool linear = MOBDB_FLAGS_GET_LINEAR(ts->flags) && 
		linear_interpolation(valuetypid);
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count,
		linear, true);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* Dispatch function */

Temporal *
tfunc2_temporal(Temporal *temp, Datum param,
    Datum (*func)(Datum, Datum), Oid valuetypid)
{
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc2_temporalinst((TemporalInst *)temp,
			param, func, valuetypid);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc2_temporali((TemporalI *)temp,
			param, func, valuetypid);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tfunc2_temporalseq((TemporalSeq *)temp,
			param, func, valuetypid);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tfunc2_temporals((TemporalS *)temp,
			param, func, valuetypid);
	return result;
}

/*****************************************************************************
 * Functions where the arguments are a temporal type and a base type.
 * The function is applied to the composing instants without looking 
 * for crossings or local minimum/maximum. The last argument states whether 
 * we are computing (1) base <oper> temporal or (2) temporal <oper> base
 *****************************************************************************/

TemporalInst *
tfunc2_temporalinst_base(TemporalInst *inst, Datum value, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	Datum value1 = temporalinst_value(inst);
	Datum resultvalue = invert ? func(value, value1) : func(value1, value);
	TemporalInst *result = temporalinst_make(resultvalue, inst->t, valuetypid);
	FREE_DATUM(resultvalue, valuetypid);
	return result;
}

TemporalI *
tfunc2_temporali_base(TemporalI *ti, Datum value, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc2_temporalinst_base(inst, value, func, 
			valuetypid, invert);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalSeq *
tfunc2_temporalseq_base(TemporalSeq *seq, Datum value, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc2_temporalinst_base(inst, value, func, 
			valuetypid, invert);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, 
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalS *
tfunc2_temporals_base(TemporalS *ts, Datum value, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc2_temporalseq_base(seq, value, func, 
			valuetypid, invert);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count,
		MOBDB_FLAGS_GET_LINEAR(ts->flags), true);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* Dispatch function */

Temporal *
tfunc2_temporal_base(Temporal *temp, Datum d, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool invert)
{
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc2_temporalinst_base((TemporalInst *)temp, d, 
			func, valuetypid, invert);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc2_temporali_base((TemporalI *)temp, d, 
			func, valuetypid, invert);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tfunc2_temporalseq_base((TemporalSeq *)temp, d, 
			func, valuetypid, invert);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tfunc2_temporals_base((TemporalS *)temp, d,
			func, valuetypid, invert);
	return result;
}

/*****************************************************************************
 * Versions of the functions that take 3 arguments 
 *****************************************************************************/

TemporalInst *
tfunc3_temporalinst_base(TemporalInst *inst, Datum value, Datum param, 
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	Datum value1 = temporalinst_value(inst);
	Datum resultvalue = invert ? func(value, value1, param) : 
		func(value1, value, param);
	TemporalInst *result = temporalinst_make(resultvalue, inst->t, valuetypid);
	return result;
}

TemporalI *
tfunc3_temporali_base(TemporalI *ti, Datum value, Datum param, 
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc3_temporalinst_base(inst, value, param, func, 
			valuetypid, invert);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* 
 * These functions are currently not used. They are kept as comment if they 
 * may be needed in the future.
 * 
TemporalSeq *
tfunc3_temporalseq_base(TemporalSeq *seq, Datum value, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc3_temporalinst_base(inst, value, param, func, 
			valuetypid, invert);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc,
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}
 
TemporalS *
tfunc3_temporals_base(TemporalS *ts, Datum value, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc3_temporalseq_base(seq, value, param, func, 
			valuetypid, invert);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count,
		MOBDB_FLAGS_GET_LINEAR(ts->flags), true);
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}
*/

/*****************************************************************************
 * Versions of the functions that take 4 arguments 
 *****************************************************************************/

TemporalInst *
tfunc4_temporalinst_base(TemporalInst *inst, Datum value,  
	Datum (*func)(Datum, Datum, Oid, Oid), 
	Oid datumtypid, Oid valuetypid, bool invert)
{
	Datum value1 = temporalinst_value(inst);
	Datum resultvalue = invert ? func(value, value1, datumtypid, inst->valuetypid) : 
		func(value1, value, inst->valuetypid, datumtypid);
	TemporalInst *result = temporalinst_make(resultvalue, inst->t, valuetypid);
	return result;
}

TemporalI *
tfunc4_temporali_base(TemporalI *ti, Datum value, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc4_temporalinst_base(inst, value, func, 
			datumtypid, valuetypid, invert);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ti->count);
	for (int i = 0; i < ti->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalSeq *
tfunc4_temporalseq_base(TemporalSeq *seq, Datum value, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc4_temporalinst_base(inst, value, func, 
			datumtypid, valuetypid, invert);
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, 
		seq->count, seq->period.lower_inc, seq->period.upper_inc, 
		MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
	for (int i = 0; i < seq->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalS *
tfunc4_temporals_base(TemporalS *ts, Datum value, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc4_temporalseq_base(seq, value, func, 
			datumtypid, valuetypid, invert);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ts->count,
		MOBDB_FLAGS_GET_LINEAR(ts->flags), true);
	
	for (int i = 0; i < ts->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

/* Dispatch function */

Temporal *
tfunc4_temporal_base(Temporal *temp, Datum value, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool inverted)
{
	Temporal *result = NULL;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc4_temporalinst_base((TemporalInst *)temp, 
			value, func, datumtypid, valuetypid, inverted);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc4_temporali_base((TemporalI *)temp, 
			value, func, datumtypid, valuetypid, inverted);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tfunc4_temporalseq_base((TemporalSeq *)temp, 
			value, func, datumtypid, valuetypid, inverted);
	else if (temp->duration == TEMPORALS)
		result = (Temporal *)tfunc4_temporals_base((TemporalS *)temp, 
			value, func, datumtypid, valuetypid, inverted);
	return result;
}

/*****************************************************************************
 * Functions that apply the function to the composing instants and to the 
 * potential crossings when the resulting value has stepwise interpolation  
 * as required for temporal comparisons (e.g., #<).
 * The functions suppose that the resulting sequence has linear interpolation.
 * Parameters: datumtypid is the Oid of the value, valuetypid is the Oid
 * of the result of the function func.
 * N.B. The current version of the function supposes that the valuetypid
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly it is not necessary to pfree the values resulting from
 * the function func.
 *****************************************************************************/

static int
tfunc4_temporalseq_base_cross1(TemporalSeq **result, TemporalSeq *seq,
	Datum value, Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value1 = invert ?
			func(value, temporalinst_value(inst), datumtypid, inst->valuetypid) :
			func(temporalinst_value(inst), value, inst->valuetypid, datumtypid);
		TemporalInst *inst1 = temporalinst_make(value1, inst->t, valuetypid);
		/* Result has stepwise interpolation */
		result[0] = temporalseq_from_temporalinstarr(&inst1, 1, 
			true, true, false, false);
		return 1;
	}

	int k = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	bool lower_inc = seq->period.lower_inc;
	Datum startresult = invert ?
		func(value, value1, datumtypid, inst1->valuetypid) :
		func(value1, value, inst1->valuetypid, datumtypid);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(startresult, inst1->t, valuetypid);
	instants[1] = temporalinst_make(startresult, inst1->t, valuetypid);
	for (int i = 1; i < seq->count; i++)
	{
		/* Each iteration of the loop adds between one and three sequences */
		startresult = invert ?
			func(value, value1, datumtypid, inst1->valuetypid) :
			func(value1, value, inst1->valuetypid, datumtypid);
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		Datum value2 = temporalinst_value(inst2);
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		/* If both segments are constant compute the function at the inst1 and
		 * inst2 instants */
		if (datum_eq(value1, value2, inst1->valuetypid))
		{
			/*  The first instant value created above is the ones needed here */
			temporalinst_set(instants[1], startresult, inst2->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, upper_inc, false, false);
		}
			/* If either the inst1 or the inst2 value is equal to the value compute
			 * the function at the inst1, at the middle, and at the inst2 instants */
		else if (datum_eq2(value1, value, inst1->valuetypid, datumtypid) ||
				 datum_eq2(value2, value, inst1->valuetypid, datumtypid))
		{
			/* Compute the function at the inst1 instant */
			if (lower_inc)
			{
				temporalinst_set(instants[0], startresult, inst1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
			/* Find the middle time between inst1 and the inst2 instant and compute
			 * the function at that point */
			TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
			/* Linear interpolation */
			Datum intvalue = temporalseq_value_at_timestamp1(inst1, inst2, true, inttime);
			Datum intresult = invert ?
				func(value, intvalue, datumtypid, inst1->valuetypid) :
				func(intvalue, value, inst1->valuetypid, datumtypid);
			temporalinst_set(instants[0], intresult, inst1->t);
			temporalinst_set(instants[1], intresult, inst2->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false, false);
			/* Compute the function at the inst2 instant */
			if (upper_inc)
			{
				Datum endresult = invert ?
					func(value, value2, datumtypid, inst1->valuetypid) :
					func(value2, value, inst1->valuetypid, datumtypid);
				temporalinst_set(instants[0], endresult, inst2->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			TimestampTz crosstime;
			bool hascross = tlinearseq_timestamp_at_value(inst1, inst2, value,
				datumtypid, &crosstime);

			/* If there is no crossing compute the function at the inst1 and
			 * inst2 instants */
			if (!hascross)
			{
				/* Compute the function at the inst1 and inst2 instants */
				temporalinst_set(instants[0], startresult, inst1->t);
				temporalinst_set(instants[1], startresult, inst2->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, upper_inc, false, false);
			}
			else
			{
				/* Since there is a crossing in the middle compute the function at the
				 * inst1 instant, at the crossing, and at the inst2 instant */
				temporalinst_set(instants[0], startresult, inst1->t);
				temporalinst_set(instants[1], startresult, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				/* Compute the function at the crossing. Due to floating point precision
				 * we cannot compute the function at the crosstime as follows
						startresult = temporalseq_value_at_timestamp1(inst1, inst2, true, crosstime);
				   Since this function is (currently) called only for tfloat then we
				   assume startresult = value */
				Datum value2 = func(value, value, datumtypid, datumtypid);
				temporalinst_set(instants[0], value2, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
				/* Find the middle time between inst1 and the inst2 instant and compute
				 * the function at that point */
				TimestampTz inttime = crosstime + ((inst2->t - crosstime)/2);
				/* Linear interpolation */
				startresult = temporalseq_value_at_timestamp1(inst1, inst2, true, inttime);
				value2 = invert ?
					func(value, startresult, datumtypid, inst1->valuetypid) :
					func(startresult, value, inst1->valuetypid, datumtypid);
				temporalinst_set(instants[0], value2, crosstime);
				temporalinst_set(instants[1], value2, inst2->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					false, upper_inc, false, false);
			}
		}
		inst1 = inst2;
		value1 = value2;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]);
	return k;
}

TemporalS *
tfunc4_temporalseq_base_cross(TemporalSeq *seq, Datum value, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * 3);
	int count = tfunc4_temporalseq_base_cross1(sequences, seq, value, 
		func, datumtypid, valuetypid, invert);
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, count,
		false, true);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	
	return result;
}

TemporalS *
tfunc4_temporals_base_cross(TemporalS *ts, Datum value, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid datumtypid, 
	Oid valuetypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tfunc4_temporalseq_base_cross1(&sequences[k], seq, value,
			func, datumtypid, valuetypid, invert);
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass. Version for 2 arguments.
 *****************************************************************************/

TemporalInst *
sync_tfunc2_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) != 0)
		return NULL;

	Datum value = func(temporalinst_value(inst1), temporalinst_value(inst2));
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc2_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc2_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	return sync_tfunc2_temporali_temporalinst(ti, inst, func, valuetypid);
}

TemporalInst *
sync_tfunc2_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc2_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	return sync_tfunc2_temporalseq_temporalinst(seq, inst, func, valuetypid);
}

TemporalInst *
sync_tfunc2_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst));
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc2_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	return sync_tfunc2_temporals_temporalinst(ts, inst, func, valuetypid);
}

/*****************************************************************************/

TemporalI *
sync_tfunc2_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporali_period(&p1, ti1);
	temporali_period(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * 
		Min(ti1->count, ti2->count));
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value = func(temporalinst_value(inst1), 
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
	
	for (i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants); 

	return result;
}

TemporalI *
sync_tfunc2_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporali_period(&p, ti);
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
			Datum value = func(value1, temporalinst_value(inst));
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
sync_tfunc2_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	return sync_tfunc2_temporalseq_temporali(seq, ti, func, valuetypid);
}

TemporalI *
sync_tfunc2_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts);
	temporali_period(&p2, ti);
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
			Datum value = func(value1, temporalinst_value(inst));
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
	for (i = 0; i < k; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalI *
sync_tfunc2_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	return sync_tfunc2_temporals_temporali(ts, ti, func, valuetypid);
}

/*****************************************************************************/

TemporalSeq *
sync_tfunc2_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
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
		Datum value = func(value1, value2);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		/* Result has stepwise interpolation */
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, linear, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst); pfree(inter);
		return result;
	}
	
	/* 
	 * General case 
	 * seq1 =  ...    *       *       *>
	 * seq2 =    <*       *   *   * ...
	 * result =  <X I X I X I * I X I X>
	 * where X, I, and * are values computed, respectively at synchronization points, 
	 * intermediate points, and common points
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
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
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
		/* If not the first instant compute the function on the potential
		   intermediate point before adding the new instants */
		if (interpoint != NULL && k > 0 && 
			interpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			inter1 = temporalseq_value_at_timestamp1(prev1, inst1,
				linear1, intertime);
			inter2 = temporalseq_value_at_timestamp1(prev2, inst2,
				linear2, intertime);
			value = func(inter1, inter2);
			instants[k++] = temporalinst_make(value, intertime, valuetypid);
			FREE_DATUM(inter1, seq1->valuetypid); FREE_DATUM(inter2, seq2->valuetypid);
			FREE_DATUM(value, valuetypid);
		}
		value = func(temporalinst_value(inst1), temporalinst_value(inst2));
		instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
		FREE_DATUM(value, valuetypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	/* We are sure that k != 0 due to the period intersection test above */
	/* The last two values of sequences with stepwise interpolation and  
	   exclusive upper bound must be equal */
	if (!linear && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k - 1];
		value = temporalinst_value(instants[k - 2]);
		instants[k - 1] = temporalinst_make(value, instants[k - 1]->t, valuetypid); 		
	}

   TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		inter->lower_inc, inter->upper_inc, linear, true);
	
	for (i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants);
	for (i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************/

TemporalS *
sync_tfunc2_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporals_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count - n));
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_tfunc2_temporalseq_temporalseq(seq1, seq, 
			func, valuetypid, linear, interpoint);
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		linear, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
sync_tfunc2_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))	
{
	return sync_tfunc2_temporals_temporalseq(ts, seq, func, valuetypid, linear, interpoint);
}

TemporalS *
sync_tfunc2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
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
		TemporalSeq *seq = sync_tfunc2_temporalseq_temporalseq(seq1, seq2, 
			func, valuetypid, linear, interpoint);
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		linear, false);
	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/

/* Dispatch function */

Temporal *
sync_tfunc2_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			func, valuetypid);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			func, valuetypid);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			func, valuetypid, linear, interpoint);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			func, valuetypid, linear, interpoint);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			func, valuetypid, linear, interpoint);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			func, valuetypid, linear, interpoint);

	return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass. Version for 3 arguments.
 *****************************************************************************/

TemporalInst *
sync_tfunc3_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) != 0)
		return NULL;
	Datum value = func(temporalinst_value(inst1), temporalinst_value(inst2),
		param);
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc3_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc3_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	return sync_tfunc3_temporali_temporalinst(ti, inst, param, func, valuetypid);
}

TemporalInst *
sync_tfunc3_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc3_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	return sync_tfunc3_temporalseq_temporalinst(seq, inst, param, func, valuetypid);
}

TemporalInst *
sync_tfunc3_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst), param);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc3_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	return sync_tfunc3_temporals_temporalinst(ts, inst, param, func, valuetypid);
}

/*****************************************************************************/

TemporalI *
sync_tfunc3_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporali_period(&p1, ti1);
	temporali_period(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) *
		Min(ti1->count, ti2->count));
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value = func(temporalinst_value(inst1), temporalinst_value(inst2),
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
	
	for (i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants); 

	return result;
}

TemporalI *
sync_tfunc3_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporali_period(&p, ti);
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
			Datum value = func(value1, temporalinst_value(inst), param);
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
sync_tfunc3_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	return sync_tfunc3_temporalseq_temporali(seq, ti, param, func, valuetypid);
}

TemporalI *
sync_tfunc3_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts);
	temporali_period(&p2, ti);
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
			Datum value = func(value1, temporalinst_value(inst), param);
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
	for (i = 0; i < k; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalI *
sync_tfunc3_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	return sync_tfunc3_temporals_temporali(ts, ti, param, func, valuetypid);
}

/*****************************************************************************/

/* 
 * These functions are currently not used. They are kept as comment if they 
 * may be needed in the future.
 * TemporalSeq *
sync_tfunc3_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/ * Test whether the bounding period of the two temporal values overlap * /
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return NULL;
	
	/ * If the two sequences intersect at an instant * /
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = func(value1, value2, param);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, linear, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst); pfree(inter);
		return result;
	}

	/ * 
	 * General case 
	 * seq1 =  ...    *       *       *>
	 * seq2 =    <*       *   *   * ...
	 * result =  <X I X I X I * I X I X>
	 * where X, I, and * are values computed, respectively at synchronization points, 
	 * intermediate points, and common points
	 * /
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
		/ * If not the first instant compute the function on the potential
		   intermediate point before adding the new instants * /
		if (interpoint != NULL && k > 0 && 
			interpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			inter1 = temporalseq_value_at_timestamp1(prev1, inst1, 
				MOBDB_FLAGS_GET_LINEAR(seq1->flags), intertime);
			inter2 = temporalseq_value_at_timestamp1(prev2, inst2, 
				MOBDB_FLAGS_GET_LINEAR(seq2->flags), intertime);
			value = func(inter1, inter2, param);
			instants[k++] = temporalinst_make(value, intertime, valuetypid);
			FREE_DATUM(inter1, seq1->valuetypid); FREE_DATUM(inter2, seq2->valuetypid);
			FREE_DATUM(value, valuetypid);
		}
		value = func(temporalinst_value(inst1), temporalinst_value(inst2), param);
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
	/ * The last two values of sequences with stepwise interpolation and  
	   exclusive upper bound must be equal * /
	if (!linear && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k - 1];
		value = temporalinst_value(instants[k - 2]);
		instants[k - 1] = temporalinst_make(value, instants[k - 1]->t, valuetypid); 		
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		inter->lower_inc, inter->upper_inc, linear, true);
	
	for (int i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants);
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

	return result; 
}

/ ***************************************************************************** /

TemporalS *
sync_tfunc3_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/ * Test whether the bounding period of the two temporal values overlap * /
	Period p;
	temporals_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/ * We are sure that n < ts->count due to the bounding period test above * /
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_tfunc3_temporalseq_temporalseq(seq1, seq, 
			param, func, valuetypid, linear, interpoint);
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		linear, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
sync_tfunc3_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	return sync_tfunc3_temporals_temporalseq(ts, seq, param, func, valuetypid, linear, interpoint);
}

TemporalS *
sync_tfunc3_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/ * Test whether the bounding period of the two temporal values overlap * /
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/ * Previously it was Max(ts1->count, ts2->count) and was not correct * /
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		TemporalSeq *seq = sync_tfunc3_temporalseq_temporalseq(seq1, seq2, 
			param, func, valuetypid, linear, interpoint);
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		linear, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/ ***************************************************************************** /
/ * Dispatch function * /

Temporal *
sync_tfunc3_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			param, func, valuetypid);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			param, func, valuetypid);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			param, func, valuetypid, linear, interpoint);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			param, func, valuetypid, linear, interpoint);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			param, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			param, func, valuetypid, linear, interpoint);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			param, func, valuetypid, linear, interpoint);

	return result;
}
*/

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass. Version for 4 arguments.
 *****************************************************************************/

TemporalInst *
sync_tfunc4_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) != 0)
		return NULL;
	Datum value = func(temporalinst_value(inst1), temporalinst_value(inst2),
		inst1->valuetypid, inst2->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst1->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc4_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst),
		ti->valuetypid, inst->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc4_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return sync_tfunc4_temporali_temporalinst(ti, inst, func, valuetypid);
}

TemporalInst *
sync_tfunc4_temporalseq_temporalinst(TemporalSeq *seq, TemporalInst *inst, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst),
		seq->valuetypid, inst->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc4_temporalinst_temporalseq(TemporalInst *inst, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return sync_tfunc4_temporalseq_temporalinst(seq, inst, func, valuetypid);
}

TemporalInst *
sync_tfunc4_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;
	
	Datum value = func(value1, temporalinst_value(inst),
		ts->valuetypid, inst->valuetypid);
	TemporalInst *result = temporalinst_make(value, inst->t, valuetypid);
	FREE_DATUM(value, valuetypid);
	return result;
}

TemporalInst *
sync_tfunc4_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return sync_tfunc4_temporals_temporalinst(ts, inst, func, valuetypid);
}

/*****************************************************************************/

TemporalI *
sync_tfunc4_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporali_period(&p1, ti1);
	temporali_period(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) *
		Min(ti1->count, ti2->count));
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value = func(temporalinst_value(inst1), temporalinst_value(inst2),
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
	
	for (i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants); 

	return result;
}

TemporalI *
sync_tfunc4_temporalseq_temporali(TemporalSeq *seq, TemporalI *ti,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporali_period(&p, ti);
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
			Datum value = func(value1, temporalinst_value(inst),
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
sync_tfunc4_temporali_temporalseq(TemporalI *ti, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return sync_tfunc4_temporalseq_temporali(seq, ti, func, valuetypid);
}

TemporalI *
sync_tfunc4_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts);
	temporali_period(&p2, ti);
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
			Datum value = func(value1, temporalinst_value(inst),
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
	for (i = 0; i < k; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

TemporalI *
sync_tfunc4_temporali_temporals(TemporalI *ti, TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return sync_tfunc4_temporals_temporali(ts, ti, func, valuetypid);
}

/*****************************************************************************/

TemporalSeq *
sync_tfunc4_temporalseq_temporalseq(TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
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
		Datum value = func(value1, value2, 
			seq1->valuetypid, seq2->valuetypid);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		TemporalSeq *result = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, linear, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst); pfree(inter);
		return result;
	}
	
	/* 
	 * General case 
	 * seq1 =  ...    *       *       *>
	 * seq2 =    <*       *   *   * ...
	 * result =  <X I X I X I * I X I X>
	 * where X, I, and * are values computed, respectively at synchronization points, 
	 * intermediate points, and common points
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
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
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
		/* If not the first instant compute the function on the potential
		   intermediate point before adding the new instants */
		if (interpoint != NULL && k > 0 && 
			interpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			inter1 = temporalseq_value_at_timestamp1(prev1, inst1, 
				linear1, intertime);
			inter2 = temporalseq_value_at_timestamp1(prev2, inst2, 
				linear2, intertime);
			value = func(inter1, inter2, seq1->valuetypid, seq2->valuetypid);
			instants[k++] = temporalinst_make(value, intertime, valuetypid);
			FREE_DATUM(inter1, seq1->valuetypid); FREE_DATUM(inter2, seq2->valuetypid);
			FREE_DATUM(value, valuetypid);
		}
		value = func(temporalinst_value(inst1), temporalinst_value(inst2), 
			seq1->valuetypid, seq2->valuetypid);
		instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
		FREE_DATUM(value, valuetypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	/* We are sure that k != 0 due to the period intersection test above */
	/* The last two values of sequences with stepwise interpolation and  
	   exclusive upper bound must be equal */
	if (!linear && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k - 1];
		value = temporalinst_value(instants[k - 2]);
		instants[k - 1] = temporalinst_make(value, instants[k - 1]->t, valuetypid); 		
	}

   TemporalSeq *result = temporalseq_from_temporalinstarr(instants, k, 
		inter->lower_inc, inter->upper_inc, linear, true);
	
	for (i = 0; i < k; i++)
		pfree(instants[i]); 
	pfree(instants);
	for (i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************/

TemporalS *
sync_tfunc4_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporals_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count - n));
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_tfunc4_temporalseq_temporalseq(seq1, seq, 
			func, valuetypid, linear, interpoint);
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		linear, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

TemporalS *
sync_tfunc4_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	return sync_tfunc4_temporals_temporalseq(ts, seq, func, valuetypid, linear, interpoint);
}

TemporalS *
sync_tfunc4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
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
		TemporalSeq *seq = sync_tfunc4_temporalseq_temporalseq(seq1, seq2, 
			func, valuetypid, linear, interpoint);
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		linear, false);
	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_tfunc4_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid, bool linear,
	bool (*interpoint)(TemporalInst *, TemporalInst *, TemporalInst *, TemporalInst *, TimestampTz *))
{
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, 
			func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, 
			func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, 
			func, valuetypid);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc4_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc4_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, 
			func, valuetypid);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc4_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			func, valuetypid, linear, interpoint);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc4_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			func, valuetypid, linear, interpoint);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc4_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			func, valuetypid, linear, interpoint);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc4_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			func, valuetypid, linear, interpoint);

	return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass while adding intermediate point for crossings. 
 * Version for 2 arguments.
 * N.B. The current version of the function supposes that the valuetypid
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly it is not necessary to pfree the values resulting from
 * the function func.

 *****************************************************************************/

/* This function is called when at least one segment has linear interpolation */

static int 
sync_tfunc2_temporalseq_temporalseq_cross1(TemporalSeq **result, TemporalSeq *seq1,
	TemporalSeq *seq2, Datum (*func)(Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period, 
		&seq2->period);
	if (inter == NULL)
		return 0;
	
	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum startresult, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &startresult);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = func(startresult, value2);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		/* Result has stepwise interpolation */
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, 
			false, false);
		FREE_DATUM(startresult, seq1->valuetypid);
		FREE_DATUM(value2, seq2->valuetypid);
		pfree(inst); pfree(inter);
		return 1;
	}

	/* General case */
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * 
		(seq1->count + seq2->count) * 2);
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
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	Datum startvalue1 = temporalinst_value(start1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum startresult = func(startvalue1, startvalue2);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
	instants[1] = temporalinst_copy(instants[0]);
	while (i < seq1->count && j < seq2->count)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j);
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;			
		}
		else if (cmp < 0)
		{
			i++;
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t, linear2);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t, linear1);
			tofree[l++] = end1;
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		/* The remaining of the loop adds between one and three sequences */
		Datum endvalue1 = temporalinst_value(end1);
		Datum endvalue2 = temporalinst_value(end2);

		/* If both segments are constant compute the function at the start and
		 * end instants */
		if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
			datum_eq(startvalue2, endvalue2, start2->valuetypid))
		{
			/* The first instant value created above is the one needed here */
			temporalinst_set(instants[1], startresult, end1->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, upper_inc, false, false);
		}
		/* If either the start values are equal or the end values are equal and
		 * both have linear interpolation compute the function at the start
		 * instant, at an intermediate point, and at the end instant */
		else if (datum_eq(startvalue1, startvalue2, start1->valuetypid) ||
				 (linear1 && linear2 &&
				  datum_eq(endvalue1, endvalue2, start1->valuetypid)))
		{
			/* Compute the function at the start instant */
			if (lower_inc)
			{
				temporalinst_set(instants[0], startresult, start1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
			/* Find the middle time between start and the end instant and compute
			 * the function at that point */
			TimestampTz inttime = start1->t + ((end1->t - start1->t)/2);
			Datum value1 = temporalseq_value_at_timestamp1(start1, end1, linear1, inttime);
			Datum value2 = temporalseq_value_at_timestamp1(start2, end2, linear2, inttime);
			Datum intresult = func(value1, value2);
			temporalinst_set(instants[0], intresult, start1->t);
			temporalinst_set(instants[1], intresult, end1->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false, false);
			FREE_DATUM(value1, start1->valuetypid);
			FREE_DATUM(value2, start1->valuetypid);
			/* Compute the function at the end instant */
			if (upper_inc)
			{
				Datum endresult = func(endvalue1, endvalue2);
				temporalinst_set(instants[0], endresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			TimestampTz crosstime;
			bool hascross;
			if (! linear1)
				hascross = tlinearseq_timestamp_at_value(start2, end2,
					startvalue1, start1->valuetypid, &crosstime);
			else if (! linear2)
				hascross = tlinearseq_timestamp_at_value(start1, end1,
					startvalue2, start2->valuetypid, &crosstime);
			else
				hascross = temporalseq_intersect_at_timestamp(start1, end1, linear1,
					start2, end2, linear2, &crosstime);

			/* If there is no crossing compute the function at the start and end
			 * instants taking into account that the start and end values of the
			 * result may be different */
			if (!hascross)
			{
				temporalinst_set(instants[0], startresult, start1->t);
				temporalinst_set(instants[1], startresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				if (upper_inc)
				{
					Datum endresult = func(endvalue1, endvalue2);
					temporalinst_set(instants[0], endresult, end1->t);
					/* Result has stepwise interpolation */
					result[k++] = temporalseq_from_temporalinstarr(instants, 1,
						true, true, false, false);
				}
			}
			else
			{
				/* There is a crossing at the middle */
				temporalinst_set(instants[0], startresult, start1->t);
				temporalinst_set(instants[1], startresult, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				/* Find the values at the local minimum/maximum */
				Datum cross1 = temporalseq_value_at_timestamp1(start1, end1, linear1, crosstime);
				Datum cross2 = temporalseq_value_at_timestamp1(start2, end2, linear2, crosstime);
				Datum cross = func(cross1, cross2);
				temporalinst_set(instants[0], cross, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
				Datum endresult = func(endvalue1, endvalue2);
				temporalinst_set(instants[0], endresult, crosstime);
				temporalinst_set(instants[1], endresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					false, upper_inc, false, false);
				FREE_DATUM(cross1, start1->valuetypid);
				FREE_DATUM(cross2, start1->valuetypid);
			}
		}
		start1 = end1; start2 = end2;
		startvalue1 = endvalue1; startvalue2 = endvalue2;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]);
	pfree(inter);
	return k;
}

TemporalS *
sync_tfunc2_temporalseq_temporalseq_cross(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(seq1->count + seq2->count) * 3);
	int count = sync_tfunc2_temporalseq_temporalseq_cross1(sequences,
		seq1, seq2, func, valuetypid); 
	if (count == 0)
		return NULL;
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, count,
		false, true);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_tfunc2_temporals_temporalseq_cross(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts->totalcount + seq->count) * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		k += sync_tfunc2_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq, func, valuetypid);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

TemporalS *
sync_tfunc2_temporalseq_temporals_cross(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	return sync_tfunc2_temporals_temporalseq_cross(ts, seq, func, valuetypid);
}

TemporalS *
sync_tfunc2_temporals_temporals_cross(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts1->totalcount + ts2->totalcount) * 3);
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		k += sync_tfunc2_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq2, func, valuetypid);
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_tfunc2_temporal_temporal_cross(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum), Oid valuetypid)
{
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, func, valuetypid);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, func, valuetypid);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporalseq_cross(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporalseq_temporals_cross(
			(TemporalSeq *)temp1, (TemporalS *)temp2, func, valuetypid);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc2_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc2_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc2_temporals_temporalseq_cross(
			(TemporalS *)temp1, (TemporalSeq *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc2_temporals_temporals_cross(
			(TemporalS *)temp1, (TemporalS *)temp2, func, valuetypid);

	return result;
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

/* This function is called when at least one segment has linear interpolation */
static int
sync_tfunc3_temporalseq_temporalseq_cross1(TemporalSeq **result,
	TemporalSeq *seq1, TemporalSeq *seq2, Datum param,
	Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period,
		&seq2->period);
	if (inter == NULL)
		return 0;

	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum startresult, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &startresult);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = func(startresult, value2, param);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		/* Result has stepwise interpolation */
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true,
			false, false);
		FREE_DATUM(startresult, seq1->valuetypid);
		FREE_DATUM(value2, seq2->valuetypid);
		pfree(inst); pfree(inter);
		return 1;
	}

	/* General case */
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) *
								   (seq1->count + seq2->count) * 2);
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
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	Datum startvalue1 = temporalinst_value(start1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum startresult = func(startvalue1, startvalue2, param);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
	instants[1] = temporalinst_copy(instants[0]);
	while (i < seq1->count && j < seq2->count)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j);
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t, linear2);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t, linear1);
			tofree[l++] = end1;
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ?
			inter->upper_inc : false;
		/* The remaining of the loop adds between one and three sequences */
		Datum endvalue1 = temporalinst_value(end1);
		Datum endvalue2 = temporalinst_value(end2);

		/* If both segments are constant compute the function at the start and
		 * end instants */
		if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
			datum_eq(startvalue2, endvalue2, start2->valuetypid))
		{
			/* The first instant value created above is the one needed here */
			temporalinst_set(instants[1], startresult, end1->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, upper_inc, false, false);
		}
			/* If either the start values are equal or the end values are equal and
			 * both have linear interpolation compute the function at the start
			 * instant, at an intermediate point, and at the end instant */
		else if (datum_eq(startvalue1, startvalue2, start1->valuetypid) ||
				 (linear1 && linear2 &&
				  datum_eq(endvalue1, endvalue2, start1->valuetypid)))
		{
			/* Compute the function at the start instant */
			if (lower_inc)
			{
				temporalinst_set(instants[0], startresult, start1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
			/* Find the middle time between start and the end instant and compute
			 * the function at that point */
			TimestampTz inttime = start1->t + ((end1->t - start1->t)/2);
			Datum value1 = temporalseq_value_at_timestamp1(start1, end1, linear1, inttime);
			Datum value2 = temporalseq_value_at_timestamp1(start2, end2, linear2, inttime);
			Datum intresult = func(value1, value2, param);
			temporalinst_set(instants[0], intresult, start1->t);
			temporalinst_set(instants[1], intresult, end1->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false, false);
			FREE_DATUM(value1, start1->valuetypid);
			FREE_DATUM(value2, start1->valuetypid);
			/* Compute the function at the end instant */
			if (upper_inc)
			{
				Datum endresult = func(endvalue1, endvalue2, param);
				temporalinst_set(instants[0], endresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			TimestampTz crosstime;
			bool hascross;
			if (! linear1)
				hascross = tlinearseq_timestamp_at_value(start2, end2,
					startvalue1, start1->valuetypid, &crosstime);
			else if (! linear2)
				hascross = tlinearseq_timestamp_at_value(start1, end1,
					startvalue2, start2->valuetypid, &crosstime);
			else
				hascross = temporalseq_intersect_at_timestamp(start1, end1, linear1,
					start2, end2, linear2, &crosstime);

			/* If there is no crossing compute the function at the start and end
			 * instants taking into account that the start and end values of the
			 * result may be different */
			if (!hascross)
			{
				temporalinst_set(instants[0], startresult, start1->t);
				temporalinst_set(instants[1], startresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				if (upper_inc)
				{
					Datum endresult = func(endvalue1, endvalue2, param);
					temporalinst_set(instants[0], endresult, end1->t);
					/* Result has stepwise interpolation */
					result[k++] = temporalseq_from_temporalinstarr(instants, 1,
						true, true, false, false);
				}
			}
			else
			{
				/* There is a crossing at the middle */
				temporalinst_set(instants[0], startresult, start1->t);
				temporalinst_set(instants[1], startresult, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				/* Find the values at the local minimum/maximum */
				Datum cross1 = temporalseq_value_at_timestamp1(start1, end1, linear1, crosstime);
				Datum cross2 = temporalseq_value_at_timestamp1(start2, end2, linear2, crosstime);
				Datum cross = func(cross1, cross2, param);
				temporalinst_set(instants[0], cross, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
				Datum endresult = func(endvalue1, endvalue2, param);
				temporalinst_set(instants[0], endresult, crosstime);
				temporalinst_set(instants[1], endresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					false, upper_inc, false, false);
				FREE_DATUM(cross1, start1->valuetypid);
				FREE_DATUM(cross2, start1->valuetypid);
			}
		}
		start1 = end1; start2 = end2;
		startvalue1 = endvalue1; startvalue2 = endvalue2;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]);
	pfree(inter);
	return k;
}

TemporalS *
sync_tfunc3_temporalseq_temporalseq_cross(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(seq1->count + seq2->count) * 3);
	int count = sync_tfunc3_temporalseq_temporalseq_cross1(sequences,
		seq1, seq2, param, func, valuetypid); 
	if (count == 0)
		return NULL;
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, count,
		false, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_tfunc3_temporals_temporalseq_cross(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(ts->totalcount + seq->count) * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		k += sync_tfunc3_temporalseq_temporalseq_cross1(&sequences[k], 
			seq1, seq, param, func, valuetypid);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

TemporalS *
sync_tfunc3_temporalseq_temporals_cross(TemporalSeq *seq, TemporalS *ts,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	return sync_tfunc3_temporals_temporalseq_cross(ts, seq, param, 
		func, valuetypid);
}

TemporalS *
sync_tfunc3_temporals_temporals_cross(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) *
		(ts1->totalcount + ts2->totalcount) * 3);
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		k += sync_tfunc3_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq2, param, func, valuetypid);
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_tfunc3_temporal_temporal_cross(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*func)(Datum, Datum, Datum), Oid valuetypid)
{

	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, param, func, valuetypid);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, param, func, valuetypid);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc3_temporalseq_temporalseq_cross(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporalseq_temporals_cross(
				(TemporalSeq *)temp1, (TemporalS *)temp2, param, func, valuetypid);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc3_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc3_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc3_temporals_temporalseq_cross(
				(TemporalS *)temp1, (TemporalSeq *)temp2, param, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc3_temporals_temporals_cross(
				(TemporalS *)temp1, (TemporalS *)temp2, param, func, valuetypid);

	return result;
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

/* This function is called when at least one segment has linear interpolation */
static int
sync_tfunc4_temporalseq_temporalseq_cross1(TemporalSeq **result,
	TemporalSeq *seq1, TemporalSeq *seq2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period,
		&seq2->period);
	if (inter == NULL)
		return 0;

	/* If the two sequences intersect at an instant */
	if (timestamp_cmp_internal(inter->lower, inter->upper) == 0)
	{
		Datum startresult, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &startresult);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = func(startresult, value2, seq1->valuetypid, seq2->valuetypid);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		/* Result has stepwise interpolation */
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true,
			false, false);
		FREE_DATUM(startresult, seq1->valuetypid);
		FREE_DATUM(value2, seq2->valuetypid);
		pfree(inst); pfree(inter);
		return 1;
	}

	/* General case */
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) *
		(seq1->count + seq2->count) * 2);
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
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	Datum startvalue1 = temporalinst_value(start1);
	Datum startvalue2 = temporalinst_value(start2);
	Datum startresult = func(startvalue1, startvalue2, start1->valuetypid,
		start2->valuetypid);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(startresult, start1->t, valuetypid);
	instants[1] = temporalinst_copy(instants[0]);
	while (i < seq1->count && j < seq2->count)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j);
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t, linear2);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t, linear1);
			tofree[l++] = end1;
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ?
			inter->upper_inc : false;
		/* The remaining of the loop adds between one and three sequences */
		Datum endvalue1 = temporalinst_value(end1);
		Datum endvalue2 = temporalinst_value(end2);
		startresult = func(startvalue1, startvalue2, start1->valuetypid,
			start2->valuetypid);

		/* If both segments are constant compute the function at the start and
		 * end instants */
		if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
			datum_eq(startvalue2, endvalue2, start2->valuetypid))
		{
			/* The first instant value created above is the one needed here */
			temporalinst_set(instants[1], startresult, end1->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				lower_inc, upper_inc, false, false);
		}
			/* If either the start values are equal or the end values are equal and
			 * both have linear interpolation compute the function at the start
			 * instant, at an intermediate point, and at the end instant */
		else if (datum_eq2(startvalue1, startvalue2, start1->valuetypid, start2->valuetypid) ||
				 (linear1 && linear2 &&
				  datum_eq2(endvalue1, endvalue2, start1->valuetypid, start2->valuetypid)))
		{
			/* Compute the function at the start instant */
			if (lower_inc)
			{
				temporalinst_set(instants[0], startresult, start1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
			/* Find the middle time between start and the end instant and compute
			 * the function at that point */
			TimestampTz inttime = start1->t + ((end1->t - start1->t)/2);
			Datum value1 = temporalseq_value_at_timestamp1(start1, end1, linear1, inttime);
			Datum value2 = temporalseq_value_at_timestamp1(start2, end2, linear2, inttime);
			Datum intresult = func(value1, value2, start1->valuetypid,
								   start2->valuetypid);
			temporalinst_set(instants[0], intresult, start1->t);
			temporalinst_set(instants[1], intresult, end1->t);
			/* Result has stepwise interpolation */
			result[k++] = temporalseq_from_temporalinstarr(instants, 2,
				false, false, false, false);
			FREE_DATUM(value1, start1->valuetypid);
			FREE_DATUM(value2, start1->valuetypid);
			/* Compute the function at the end instant */
			if (upper_inc)
			{
				Datum endresult = func(endvalue1, endvalue2, end1->valuetypid,
									   end2->valuetypid);
				temporalinst_set(instants[0], endresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			TimestampTz crosstime;
			bool hascross;
			if (! linear1)
				hascross = tlinearseq_timestamp_at_value(start2, end2,
					startvalue1, start1->valuetypid, &crosstime);
			else if (! linear2)
				hascross = tlinearseq_timestamp_at_value(start1, end1,
					startvalue2, start2->valuetypid, &crosstime);
			else
				hascross = temporalseq_intersect_at_timestamp(start1, end1, linear1,
					start2, end2, linear2, &crosstime);

			/* If there is no crossing compute the function at the start and end
			 * instants taking into account that the start and end values of the
			 * result may be different */
			if (!hascross)
			{
				temporalinst_set(instants[0], startresult, start1->t);
				temporalinst_set(instants[1], startresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				if (upper_inc)
				{
					Datum endresult = func(endvalue1, endvalue2, end1->valuetypid,
						end2->valuetypid);
					temporalinst_set(instants[0], endresult, end1->t);
					/* Result has stepwise interpolation */
					result[k++] = temporalseq_from_temporalinstarr(instants, 1,
						true, true, false, false);
				}
			}
			else
			{
				/* There is a crossing at the middle */
				temporalinst_set(instants[0], startresult, start1->t);
				temporalinst_set(instants[1], startresult, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					lower_inc, false, false, false);
				/* Find the values at the local minimum/maximum */
				Datum cross1 = temporalseq_value_at_timestamp1(start1, end1, linear1, crosstime);
				Datum cross2 = temporalseq_value_at_timestamp1(start2, end2, linear2, crosstime);
				Datum cross = func(cross1, cross2, start1->valuetypid, start2->valuetypid);
				temporalinst_set(instants[0], cross, crosstime);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 1,
					true, true, false, false);
				Datum endresult = func(endvalue1, endvalue2, end1->valuetypid,
									   end2->valuetypid);
				temporalinst_set(instants[0], endresult, crosstime);
				temporalinst_set(instants[1], endresult, end1->t);
				/* Result has stepwise interpolation */
				result[k++] = temporalseq_from_temporalinstarr(instants, 2,
					false, upper_inc, false, false);
				FREE_DATUM(cross1, start1->valuetypid);
				FREE_DATUM(cross2, start1->valuetypid);
			}
		}
		start1 = end1; start2 = end2;
		startvalue1 = endvalue1; startvalue2 = endvalue2;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]);
	pfree(inter);
	return k;
}

TemporalS *
sync_tfunc4_temporalseq_temporalseq_cross(TemporalSeq *seq1, TemporalSeq *seq2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * 
		(seq1->count + seq2->count) * 3);
	int count = sync_tfunc4_temporalseq_temporalseq_cross1(sequences,
		seq1, seq2, func, valuetypid);
	if (count == 0)
		return NULL;
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, count,
		false, true);
		
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_tfunc4_temporals_temporalseq_cross(TemporalS *ts, TemporalSeq *seq, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) *
		(ts->totalcount + seq->count) * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		k += sync_tfunc4_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq, func, valuetypid);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

TemporalS *
sync_tfunc4_temporalseq_temporals_cross(TemporalSeq *seq, TemporalS *ts,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	return sync_tfunc4_temporals_temporalseq_cross(ts, seq, func, valuetypid);
}

TemporalS *
sync_tfunc4_temporals_temporals_cross(TemporalS *ts1, TemporalS *ts2, 
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;
	
	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) *
		(ts1->totalcount + ts2->totalcount) * 3);
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		k += sync_tfunc4_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq2, func, valuetypid);
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++; 
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Result has stepwise interpolation */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k,
		false, true);

	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	
	return result;
}

/*****************************************************************************/
/* Dispatch function */

Temporal *
sync_tfunc4_temporal_temporal_cross(Temporal *temp1, Temporal *temp2,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid valuetypid)
{
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) || 
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc4_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, func, valuetypid);
	
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ) 
		result = (Temporal *)sync_tfunc4_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS) 
		result = (Temporal *)sync_tfunc4_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, func, valuetypid);
	
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ)
		result = linear ?
			(Temporal *)sync_tfunc4_temporalseq_temporalseq_cross(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, func, valuetypid) :
			(Temporal *)sync_tfunc4_temporalseq_temporalseq(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, func, valuetypid, linear, false);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS) 
		result = linear ?
			(Temporal *)sync_tfunc4_temporalseq_temporals_cross(
				(TemporalSeq *)temp1, (TemporalS *)temp2, func, valuetypid) :
			(Temporal *)sync_tfunc4_temporalseq_temporals(
				(TemporalSeq *)temp1, (TemporalS *)temp2, func, valuetypid, linear, false);
	
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST) 
		result = (Temporal *)sync_tfunc4_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI) 
		result = (Temporal *)sync_tfunc4_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, func, valuetypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ) 
		result = linear ?
			(Temporal *)sync_tfunc4_temporals_temporalseq_cross(
				(TemporalS *)temp1, (TemporalSeq *)temp2, func, valuetypid) :
			(Temporal *)sync_tfunc4_temporals_temporalseq(
				(TemporalS *)temp1, (TemporalSeq *)temp2, func, valuetypid, linear, false);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS) 
		result = linear ?
			(Temporal *)sync_tfunc4_temporals_temporals_cross(
				(TemporalS *)temp1, (TemporalS *)temp2, func, valuetypid) :
			(Temporal *)sync_tfunc4_temporals_temporals(
				(TemporalS *)temp1, (TemporalS *)temp2, func, valuetypid, linear, false);

	return result;
}

/*****************************************************************************/

