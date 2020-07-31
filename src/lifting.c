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

/**
 * @file lifting.c
 * Generic functions for lifting functions and operators on temporal types.
 *
 * 1. There are 3 families of functions accounting for 
 *	- binary functions, such as spatial relationships functions (e.g. 
 *	  intersects). 
 *	- ternary functions, such as spatial relationships functions that need 
 *	  an additional parameter (e.g. tdwithin). 
 *	- quaternary functions which apply binary operators (e.g. + or <) to
 *	  temporal numeric types that can be of different base type (that is,
 *	  integer and float), and thus the third and fourth arguments are the
 *	  Oids of the first two arguments.
 * 2. For each of the previous families, there are two set of functions
 *	 depending on whether the interpolation of the resulting temporal type is
 *   step (e.g., for temporal floats that results in a temporal Boolean)
 *	 or linear (e.g., distance for temporal points that results in a 
 *	 temporal float).
 * 3. For each of the previous cases there are two set of functions
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
 *   - tfloatseq * base => tfunc_temporalseq_base
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
 * Generic version with variadic function pointers
 *****************************************************************************/

/**
 * Applies the function with the possible additional parameter to the 
 * temporal value
 *
 * @param[in] inst Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
tfunc_temporalinst(const TemporalInst *inst, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum resvalue;
	if (numparam == 1)
		resvalue = (*func)(temporalinst_value(inst));
	else if (numparam == 2)
		resvalue = (*func)(temporalinst_value(inst), param);
	else
		elog(ERROR, "Number of function parameters not supported: %u", numparam);
	TemporalInst *result = temporalinst_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Applies the function with the additional parameter to the temporal value
 *
 * @param[in] ti Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalI *
tfunc_temporali(const TemporalI *ti, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc_temporalinst(inst, param, func, numparam, restypid);
	}
	return temporali_make_free(instants, ti->count);
}

/**
 * Applies the function with the additional parameter to the temporal value
 *
 * @param[in] seq Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalSeq *
tfunc_temporalseq(const TemporalSeq *seq, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc_temporalinst(inst, param, func, numparam, restypid);
	}
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) &&
		linear_interpolation(restypid);
	return temporalseq_make_free(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc, linear, true);
}

/**
 * Applies the function with the additional parameter to the temporal value
 *
 * @param[in] ts Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalS *
tfunc_temporals(const TemporalS *ts, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc_temporalseq(seq, param, func, numparam, restypid);
	}
	return temporals_make_free(sequences, ts->count, true);
}

/**
 * Applies the function with the additional parameter to the temporal value
 * (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
Temporal *
tfunc_temporal(const Temporal *temp, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc_temporalinst((TemporalInst *)temp,
			param, func, numparam, restypid);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc_temporali((TemporalI *)temp,
			param, func, numparam, restypid);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tfunc_temporalseq((TemporalSeq *)temp,
			param, func, numparam, restypid);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tfunc_temporals((TemporalS *)temp,
			param, func, numparam, restypid);
	return result;
}

/*****************************************************************************
 * Functions where the arguments are a temporal type and a base type.
 * The function is applied to the composing instants without looking
 * for crossings or local minimum/maximum. The last argument states whether
 * we are computing (1) base <oper> temporal or (2) temporal <oper> base
 * Generic version with a variadic function pointer
 *****************************************************************************/

/**
 * Applies the function to the temporal value and the base value 
 *
 * @param[in] inst Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first argument
 * of the function
 */
TemporalInst *
tfunc_temporalinst_base(const TemporalInst *inst, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	Datum value1 = temporalinst_value(inst);
	Datum resvalue;
	if (numparam == 2)
		resvalue = invert ? (*func)(value, value1) : (*func)(value1, value);
	else if (numparam == 3)
		resvalue = invert ? 
			(*func)(value, value1, param) : (*func)(value1, value, param);
	else if (numparam == 4)
		resvalue = invert ? 
			(*func)(value, value1, valuetypid, inst->valuetypid) :
			(*func)(value1, value, inst->valuetypid, valuetypid);
	else
		elog(ERROR, "Number of function parameters not supported: %u", numparam);
	TemporalInst *result = temporalinst_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Applies the binary function to the temporal value and the base value
 * when their base type is different
 *
 * @param[in] ti Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first argument
 * of the function
 */
TemporalI *
tfunc_temporali_base(const TemporalI *ti, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		instants[i] = tfunc_temporalinst_base(inst, value, valuetypid, param,
			func, numparam, restypid, invert);
	}
	return temporali_make_free(instants, ti->count);
}

/**
 * Applies the binary function to the temporal value and the base value
 * when their base type is different
 *
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] param Parameter
 * @param[in] numparam Number of parameters of the function
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first argument
 * of the function
 */
TemporalSeq *
tfunc_temporalseq_base(const TemporalSeq *seq, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, i);
		instants[i] = tfunc_temporalinst_base(inst, value, valuetypid, param,
			func, numparam, restypid, invert);
	}
	return temporalseq_make(instants, seq->count, seq->period.lower_inc, 
		seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), true);
}

/**
 * Applies the binary function to the temporal value and the base value
 * when their base type is different
 *
 * @param[in] ts Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first argument
 * of the function
 */

TemporalS *
tfunc_temporals_base(const TemporalS *ts, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		sequences[i] = tfunc_temporalseq_base(seq, value, valuetypid, param,
			func, numparam, restypid, invert);
	}
	return temporals_make_free(sequences, ts->count, true);
}

/**
 * Applies the function to the temporal value and the base value
 * (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] param Parameter
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first 
 * argument of the function
 */
Temporal *
tfunc_temporal_base(const Temporal *temp, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == TEMPORALINST)
		result = (Temporal *)tfunc_temporalinst_base((TemporalInst *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	else if (temp->duration == TEMPORALI)
		result = (Temporal *)tfunc_temporali_base((TemporalI *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	else if (temp->duration == TEMPORALSEQ)
		result = (Temporal *)tfunc_temporalseq_base((TemporalSeq *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	else /* temp->duration == TEMPORALS */
		result = (Temporal *)tfunc_temporals_base((TemporalS *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	return result;
}

/*****************************************************************************
 * Functions that apply the function to the composing instants and to the
 * potential crossings when the resulting value has step interpolation
 * as required for temporal comparisons (e.g., #<).
 * The functions suppose that the resulting sequence has linear interpolation.
 * Parameters: valuetypid is the Oid of the value, restypid is the Oid
 * of the result of the function func.
 * N.B. The current version of the function supposes that the valuetypid
 * is passed by value and thus it is not necessary to create and pfree
 * each pair of instants used for constructing a segment of the result.
 * Similarly it is not necessary to pfree the values resulting from
 * the function func.
 *****************************************************************************/

/**
 * Applies the binary function to the temporal value and the base value
 * when their base type is different
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first 
 * argument of the function
 */
static int
tfunc4_temporalseq_base_cross1(TemporalSeq **result, const TemporalSeq *seq,
	Datum value, Oid valuetypid, Datum (*func)(Datum, Datum, Oid, Oid),
	Oid restypid, bool invert)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TemporalInst *inst = temporalseq_inst_n(seq, 0);
		Datum value1 = invert ?
			func(value, temporalinst_value(inst), valuetypid, inst->valuetypid) :
			func(temporalinst_value(inst), value, inst->valuetypid, valuetypid);
		TemporalInst *inst1 = temporalinst_make(value1, inst->t, restypid);
		/* Result has step interpolation */
		result[0] = temporalseq_make(&inst1, 1, true, true,
			false, false);
		return 1;
	}

	int k = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq, 0);
	Datum value1 = temporalinst_value(inst1);
	bool lower_inc = seq->period.lower_inc;
	Datum startresult = invert ?
		func(value, value1, valuetypid, inst1->valuetypid) :
		func(value1, value, inst1->valuetypid, valuetypid);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TemporalInst *instants[2];
	instants[0] = temporalinst_make(startresult, inst1->t, restypid);
	instants[1] = temporalinst_make(startresult, inst1->t, restypid);
	for (int i = 1; i < seq->count; i++)
	{
		/* Each iteration of the loop adds between one and three sequences */
		startresult = invert ?
			func(value, value1, valuetypid, inst1->valuetypid) :
			func(value1, value, inst1->valuetypid, valuetypid);
		TemporalInst *inst2 = temporalseq_inst_n(seq, i);
		Datum value2 = temporalinst_value(inst2);
		Datum intvalue, intresult, endresult;
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		/* If both segments are constant compute the function at the inst1 and
		 * inst2 instants */
		if (datum_eq(value1, value2, inst1->valuetypid))
		{
			/*  The first instant value created above is the one needed here */
			temporalinst_set(instants[0], startresult, inst1->t);
			temporalinst_set(instants[1], startresult, inst2->t);
			/* Result has step interpolation */
			result[k++] = temporalseq_make(instants, 2, lower_inc, upper_inc,
				false, false);
		}
			/* If either the inst1 or the inst2 value is equal to the value compute
			 * the function at the inst1, at the middle, and at the inst2 instants */
		else if (datum_eq2(value1, value, inst1->valuetypid, valuetypid) ||
				 datum_eq2(value2, value, inst1->valuetypid, valuetypid))
		{
			/* Compute the function at the inst1 instant */
			if (lower_inc)
			{
				temporalinst_set(instants[0], startresult, inst1->t);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 1, true, true,
					false, false);
			}
			/* Find the middle time between inst1 and the inst2 instant and compute
			 * the function at that point */
			TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
			/* Linear interpolation */
			intvalue = temporalseq_value_at_timestamp1(inst1, inst2, true, inttime);
			intresult = invert ?
				func(value, intvalue, valuetypid, inst1->valuetypid) :
				func(intvalue, value, inst1->valuetypid, valuetypid);
			temporalinst_set(instants[0], intresult, inst1->t);
			temporalinst_set(instants[1], intresult, inst2->t);
			/* Result has step interpolation */
			result[k++] = temporalseq_make(instants, 2, false, false,
				false, false);
			/* Compute the function at the inst2 instant */
			if (upper_inc)
			{
				endresult = invert ?
					func(value, value2, valuetypid, inst1->valuetypid) :
					func(value2, value, inst1->valuetypid, valuetypid);
				temporalinst_set(instants[0], endresult, inst2->t);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 1, true, true,
					false, false);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			/* Value projected on the segment to avoid floating point imprecision */
			Datum crossvalue;
			TimestampTz crosstime;
			bool hascross = tlinearseq_intersection_value(inst1, inst2, value,
				valuetypid, &crossvalue, &crosstime);

			/* If there is no crossing compute the function at the inst1 and
			 * inst2 instants */
			if (!hascross)
			{
				/* Compute the function at the inst1 and inst2 instants */
				temporalinst_set(instants[0], startresult, inst1->t);
				temporalinst_set(instants[1], startresult, inst2->t);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 2, lower_inc, upper_inc,
					false, false);
			}
			else
			{
				/* Since there is a crossing in the middle compute the function at the
				 * inst1 instant, at the crossing, and at the inst2 instant */
				temporalinst_set(instants[0], startresult, inst1->t);
				temporalinst_set(instants[1], startresult, crosstime);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 2, lower_inc, false,
					false, false);
				/* Compute the function at the crossing */
				intresult = func(crossvalue, value, valuetypid, valuetypid);
				temporalinst_set(instants[0], intresult, crosstime);
				/* Find the middle time between inst1 and the inst2 instant and compute
				 * the function at that point */
				TimestampTz inttime = crosstime + ((inst2->t - crosstime)/2);
				/* Linear interpolation */
				intvalue = temporalseq_value_at_timestamp1(inst1, inst2, true, inttime);
				endresult = invert ?
					func(value, intvalue, valuetypid, inst1->valuetypid) :
					func(intvalue, value, inst1->valuetypid, valuetypid);
				if (datum_eq(intresult, endresult, restypid))
				{
					temporalinst_set(instants[1], endresult, inst2->t);
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 2, true, upper_inc,
						false, false);
				}
				else
				{
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 1, true, true,
						false, false);
					temporalinst_set(instants[0], endresult, crosstime);
					temporalinst_set(instants[1], endresult, inst2->t);
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 2, false, upper_inc,
						false, false);
				}
			}
		}
		inst1 = inst2;
		value1 = value2;
		lower_inc = true;
	}
	pfree(instants[0]); pfree(instants[1]);
	return k;
}

/**
 * Applies the binary function to the temporal value and the base value
 * when their base type is different
 *
 * @param[in] seq Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first 
 * argument of the function
 */
TemporalS *
tfunc4_temporalseq_base_cross(const TemporalSeq *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * seq->count * 3);
	int count = tfunc4_temporalseq_base_cross1(sequences, seq, value, valuetypid,
		func, restypid, invert);
	/* Result has step interpolation */
	return temporals_make_free(sequences, count, true);
}

/**
 * Applies the binary function to the temporal value and the base value
 * when their base type is different
 *
 * @param[in] ts Temporal value
 * @param[in] value Base value
 * @param[in] valuetypid Base type
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] invert True when the base value is the first 
 * argument of the function
 */
TemporalS *
tfunc4_temporals_base_cross(const TemporalS *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tfunc4_temporalseq_base_cross1(&sequences[k], seq, value, valuetypid,
			func, restypid, invert);
	}
	return temporals_make_free(sequences, k, true);
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass. Generic version with a variadic function
 *****************************************************************************/

static Datum
tfunc(Datum value1, Datum value2, Oid valuetypid1, Oid valuetypid2, 
	Datum param, Datum (*func)(Datum, ...), int numparam)
{
	if (numparam == 2)
		return (*func)(value1, value2);
	else if (numparam == 3)
		return (*func)(value1, value2, param);
	else if (numparam == 4)
		return (*func)(value1, value2, valuetypid1, valuetypid2);
	else
		elog(ERROR, "Number of function parameters not supported: %u", numparam);
}

/**
 * Synchronizes the temporal values and applies to them the function 
 *
 * @param[in] inst1,inst2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	/* Test whether the two temporal values overlap on time */
	if (inst1->t != inst2->t)
		return NULL;
	
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	Datum resvalue = tfunc(value1, value2, inst1->valuetypid, inst2->valuetypid,
		param, func, numparam);
	TemporalInst *result = temporalinst_make(resvalue, inst1->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,inst Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporali_temporalinst(const TemporalI *ti, const TemporalInst *inst,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum value1;
	if (!temporali_value_at_timestamp(ti, inst->t, &value1))
		return NULL;

	Datum value2 = temporalinst_value(inst);
	Datum resvalue = tfunc(value1, value2, ti->valuetypid, inst->valuetypid,
		param, func, numparam);
	TemporalInst *result = temporalinst_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;	
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,ti Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporalinst_temporali(const TemporalInst *inst, const TemporalI *ti,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_temporali_temporalinst(ti, inst, param, func,
		numparam, restypid);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,inst Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporalseq_temporalinst(const TemporalSeq *seq, const TemporalInst *inst,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum value1;
	if (!temporalseq_value_at_timestamp(seq, inst->t, &value1))
		return NULL;

	Datum value2 = temporalinst_value(inst);
	Datum resvalue = tfunc(value1, value2, seq->valuetypid, inst->valuetypid,
		param, func, numparam);
	TemporalInst *result = temporalinst_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,seq Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporalinst_temporalseq(const TemporalInst *inst, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_temporalseq_temporalinst(seq, inst, param, func,
		numparam, restypid);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,inst Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporals_temporalinst(const TemporalS *ts, const TemporalInst *inst,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum value1;
	if (!temporals_value_at_timestamp(ts, inst->t, &value1))
		return NULL;

	Datum value2 = temporalinst_value(inst);
	Datum resvalue = tfunc(value1, value2, ts->valuetypid, inst->valuetypid,
		param, func, numparam);
	TemporalInst *result = temporalinst_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalInst *
sync_tfunc_temporalinst_temporals(const TemporalInst *inst, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_temporals_temporalinst(ts, inst, param, func, 
		numparam, restypid);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function 
 *
 * @param[in] ti1,ti2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalI *
sync_tfunc_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
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
			Datum value1 = temporalinst_value(inst1);
			Datum value2 = temporalinst_value(inst2);
			Datum resvalue = tfunc(value1, value2, ti1->valuetypid, 
				ti2->valuetypid, param, func, numparam);
			instants[k++] = temporalinst_make(resvalue, inst1->t, restypid);
			DATUM_FREE(resvalue, restypid);
			i++; j++;
		}
		else if (cmp < 0)
			i++;
		else
			j++;
	}
	return temporali_make_free(instants, k);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,ti Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalI *
sync_tfunc_temporalseq_temporali(const TemporalSeq *seq, const TemporalI *ti,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
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
			Datum value2 = temporalinst_value(inst);
			Datum resvalue = tfunc(value1, value2, seq->valuetypid, 
				ti->valuetypid, param, func, numparam);
			instants[k++] = temporalinst_make(resvalue, inst->t, restypid);
			DATUM_FREE(value1, seq->valuetypid); DATUM_FREE(resvalue, restypid);
		}
		if (seq->period.upper < inst->t)
			break;
	}
	return temporali_make_free(instants, k);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,seq Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalI *
sync_tfunc_temporali_temporalseq(const TemporalI *ti, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_temporalseq_temporali(seq, ti, param, func, numparam,
		restypid);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,ti Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalI *
sync_tfunc_temporals_temporali(const TemporalS *ts, const TemporalI *ti,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
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
			Datum value2 = temporalinst_value(inst);
			Datum resvalue = tfunc(value1, value2, ts->valuetypid,
				ti->valuetypid, param, func, numparam);
			instants[k++] = temporalinst_make(resvalue, inst->t, restypid);
			DATUM_FREE(value1, ts->valuetypid); DATUM_FREE(resvalue, restypid);
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
	return temporali_make_free(instants, k);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalI *
sync_tfunc_temporali_temporals(const TemporalI *ti, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_temporals_temporali(ts, ti, param, func, numparam,
		restypid);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TemporalSeq *
sync_tfunc_temporalseq_temporalseq(const TemporalSeq *seq1, const TemporalSeq *seq2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period,
		&seq2->period);
	if (inter == NULL)
		return NULL;

	/* If the two sequences intersect at an instant */
	if (inter->lower == inter->upper)
	{
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum resvalue = tfunc(value1, value2, seq1->valuetypid,
			seq2->valuetypid, param, func, numparam);
		TemporalInst *inst = temporalinst_make(resvalue, inter->lower, restypid);
		/* Result has step interpolation */
		TemporalSeq *result = temporalseq_make(&inst, 1, true, true,
			reslinear, false);
		DATUM_FREE(value1, seq1->valuetypid); DATUM_FREE(value2, seq2->valuetypid);
		DATUM_FREE(resvalue, restypid); pfree(inst); pfree(inter);
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
	if (inst1->t < inter->lower)
	{
		inst1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofreeinst = inst1;
		i = temporalseq_find_timestamp(seq1, inter->lower);
	}
	else if (inst2->t < inter->lower)
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
	Datum value;
	TimestampTz intertime;
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	while (i < seq1->count && j < seq2->count &&
		(inst1->t <= inter->upper || inst2->t <= inter->upper))
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
		if (turnpoint != NULL && k > 0 &&
			turnpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			Datum inter1 = temporalseq_value_at_timestamp1(prev1, inst1,
				linear1, intertime);
			Datum inter2 = temporalseq_value_at_timestamp1(prev2, inst2,
				linear2, intertime);
			value = tfunc(inter1, inter2, seq1->valuetypid, seq2->valuetypid,
				param, func, numparam);
			instants[k++] = temporalinst_make(value, intertime, restypid);
			DATUM_FREE(inter1, seq1->valuetypid); DATUM_FREE(inter2, seq2->valuetypid);
			DATUM_FREE(value, restypid);
			}
		Datum value1 = temporalinst_value(inst1);
		Datum value2 = temporalinst_value(inst2);
		value = tfunc(value1, value2, seq1->valuetypid, seq2->valuetypid,
			param, func, numparam);
		instants[k++] = temporalinst_make(value, inst1->t, restypid);
		DATUM_FREE(value, restypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = temporalseq_inst_n(seq1, i);
		inst2 = temporalseq_inst_n(seq2, j);
	}
	/* We are sure that k != 0 due to the period intersection test above */
	/* The last two values of sequences with step interpolation and
	   exclusive upper bound must be equal */
	if (!reslinear && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k - 1];
		value = temporalinst_value(instants[k - 2]);
		instants[k - 1] = temporalinst_make(value, instants[k - 1]->t, restypid);
	}

	for (i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

   return temporalseq_make_free(instants, k, inter->lower_inc,
		inter->upper_inc, reslinear, true);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,seq Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TemporalS *
sync_tfunc_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *))
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporals_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return NULL;

	int loc;
	temporals_find_timestamp(ts, seq->period.lower, &loc);
	/* We are sure that loc < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count - loc));
	int k = 0;
	for (int i = loc; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_tfunc_temporalseq_temporalseq(seq1, seq,
			param, func, numparam, restypid, reslinear, turnpoint);
		if (seq2 != NULL)
			sequences[k++] = seq2;
		int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
		if (cmp < 0 ||
			(cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
			break;
	}
	return temporals_make_free(sequences, k, false);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TemporalS *
sync_tfunc_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *))
{
	return sync_tfunc_temporals_temporalseq(ts, seq, param, func, numparam,
		restypid, reslinear, turnpoint);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TemporalS *
sync_tfunc_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *))
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
		TemporalSeq *seq = sync_tfunc_temporalseq_temporalseq(seq1, seq2,
			param, func, numparam, restypid, reslinear, turnpoint);
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
	return temporals_make_free(sequences, k, false);
}


/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 * (dispatch function)
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
Temporal *
sync_tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TemporalInst *, const TemporalInst *, const TemporalInst *,
		const TemporalInst *, TimestampTz *))
{
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS)
		result = (Temporal *)sync_tfunc_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2,
			param, func, numparam, restypid);

	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS)
		result = (Temporal *)sync_tfunc_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2,
			param, func, numparam, restypid);

	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc_temporalseq_temporalseq(
			(TemporalSeq *)temp1, (TemporalSeq *)temp2,
			param, func, numparam, restypid, reslinear, turnpoint);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS)
		result = (Temporal *)sync_tfunc_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			param, func, numparam, restypid, reslinear, turnpoint);

	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2,
			param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc_temporals_temporalseq(
			(TemporalS *)temp1, (TemporalSeq *)temp2,
			param, func, numparam, restypid, reslinear, turnpoint);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS)
		result = (Temporal *)sync_tfunc_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			param, func, numparam, restypid, reslinear, turnpoint);

	return result;
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass while adding intermediate point for crossings.
 * Generic version for variadic function.
 *****************************************************************************/

/**
 * Applies the binary function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[out] result Array on which the pointers of the newly constructed 
 * sequences are stored
 * @param[in] seq1,seq2 Temporal values
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
static int
sync_tfunc_temporalseq_temporalseq_cross1(TemporalSeq **result, 
	const TemporalSeq *seq1, const TemporalSeq *seq2,  Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period,
		&seq2->period);
	if (inter == NULL)
		return 0;

	Oid valuetypid1 = seq1->valuetypid, valuetypid2 = seq2->valuetypid;

	/* If the two sequences intersect at an instant */
	if (inter->lower == inter->upper)
	{
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = tfunc(value1, value2, valuetypid1, valuetypid2, 
			param, func, numparam);
		TemporalInst *inst = temporalinst_make(value, inter->lower, restypid);
		/* Result has step interpolation */
		result[0] = temporalseq_make(&inst, 1, true, true, false, false);
		DATUM_FREE(value1, seq1->valuetypid);
		DATUM_FREE(value2, seq2->valuetypid);
		pfree(inst); pfree(inter);
		return 1;
	}

	/* General case */
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) *
		(seq1->count + seq2->count) * 2);
	TemporalInst *start1 = temporalseq_inst_n(seq1, 0);
	TemporalInst *start2 = temporalseq_inst_n(seq2, 0);
	int i = 1, j = 1, k = 0, l = 0;
	if (start1->t < inter->lower)
	{
		start1 = temporalseq_at_timestamp(seq1, inter->lower);
		tofree[l++] = start1;
		i = temporalseq_find_timestamp(seq1, inter->lower) + 1;
	}
	else if (start2->t < inter->lower)
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
	TemporalInst *instants[2];
	Datum startresult;
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
			end2 = temporalseq_at_timestamp1(start2, end2, linear2, end1->t);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = temporalseq_at_timestamp1(start1, end1, linear1, end2->t);
			tofree[l++] = end1;
		}
		bool upper_inc = (end1->t == inter->upper) ? inter->upper_inc : false;
		/* The remaining of the loop adds between one and three sequences */
		Datum endvalue1 = temporalinst_value(end1);
		Datum endvalue2 = temporalinst_value(end2);
		startresult = tfunc(startvalue1, startvalue2, valuetypid1, valuetypid2, 
			param, func, numparam);

		/* If both segments are constant compute the function at the start and
		 * end instants */
		if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
			datum_eq(startvalue2, endvalue2, start2->valuetypid))
		{
			instants[0] = temporalinst_make(startresult, start1->t, restypid);
			instants[1] = temporalinst_make(startresult, end1->t, restypid);
			/* Result has step interpolation */
			result[k++] = temporalseq_make(instants, 2, lower_inc, upper_inc,
				false, false);
			pfree(instants[0]); pfree(instants[1]);
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
				instants[0] = temporalinst_make(startresult, start1->t, restypid);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 1, true, true,
					false, false);
				pfree(instants[0]);
			}
			/* Find the middle time between start and the end instant and compute
			 * the function at that point */
			TimestampTz inttime = start1->t + ((end1->t - start1->t)/2);
			Datum value1 = temporalseq_value_at_timestamp1(start1, end1, linear1, inttime);
			Datum value2 = temporalseq_value_at_timestamp1(start2, end2, linear2, inttime);
			Datum intresult = tfunc(value1, value2, valuetypid1, valuetypid2, 
				param, func, numparam);
			instants[0] = temporalinst_make(intresult, start1->t, restypid);
			instants[1] = temporalinst_make(intresult, end1->t, restypid);
			/* Result has step interpolation */
			result[k++] = temporalseq_make(instants, 2, false, false,
				false, false);
			DATUM_FREE(value1, start1->valuetypid); DATUM_FREE(value2, start2->valuetypid);
			DATUM_FREE(intresult, restypid);
			pfree(instants[0]); pfree(instants[1]);
			/* Compute the function at the end instant */
			if (upper_inc)
			{
				Datum endresult = tfunc(endvalue1, endvalue2, valuetypid1, valuetypid2, 
					param, func, numparam);
				instants[0] = temporalinst_make(endresult, end1->t, restypid);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 1, true, true,
					false, false);
				DATUM_FREE(endresult, restypid);
				pfree(instants[0]);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			TimestampTz crosstime;
			Datum crossvalue1, crossvalue2;
			bool hascross = temporalseq_intersection(start1, end1, linear1,
				start2, end2, linear2, &crossvalue1, &crossvalue2, &crosstime);
			/* If there is no crossing compute the function at the start and end
			 * instants taking into account that the start and end values of the
			 * result may be different */
			if (!hascross)
			{
				instants[0] = temporalinst_make(startresult, start1->t, restypid);
				instants[1] = temporalinst_make(startresult, end1->t, restypid);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 2, lower_inc, false,
					false, false);
				pfree(instants[0]); pfree(instants[1]);
				if (upper_inc)
				{
					Datum endresult = tfunc(endvalue1, endvalue2, valuetypid1, valuetypid2, 
						param, func, numparam);
					instants[0] = temporalinst_make(endresult, end1->t, restypid);
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 1, true, true,
						false, false);
					DATUM_FREE(endresult, restypid);
					pfree(instants[0]);
				}
			}
			else
			{
				/* There is a crossing at the middle */
				instants[0] = temporalinst_make(startresult, start1->t, restypid);
				instants[1] = temporalinst_make(startresult, crosstime, restypid);
				/* Result has step interpolation */
				result[k++] = temporalseq_make(instants, 2, lower_inc, false,
					false, false);
				pfree(instants[0]); pfree(instants[1]);
				/* Find the value at the local minimum/maximum */
				Datum cross = tfunc(crossvalue1, crossvalue2, valuetypid1, valuetypid2, 
					param, func, numparam);
				instants[0] = temporalinst_make(cross, crosstime, restypid);
				Datum endresult = tfunc(endvalue1, endvalue2, valuetypid1, valuetypid2, 
					param, func, numparam);
				if (datum_eq(cross, endresult, restypid))
				{
					instants[1] = temporalinst_make(endresult, end1->t, restypid);
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 2, true, upper_inc,
						false, false);
					pfree(instants[0]); pfree(instants[1]);
				}
				else
				{
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 1, true, true,
						false, false);
					pfree(instants[0]);
					instants[0] = temporalinst_make(endresult, crosstime, restypid);
					instants[1] = temporalinst_make(endresult, end1->t, restypid);
					/* Result has step interpolation */
					result[k++] = temporalseq_make(instants, 2, false, upper_inc,
						false, false);
					pfree(instants[0]); pfree(instants[1]);
				}
				DATUM_FREE(crossvalue1, start1->valuetypid);
				DATUM_FREE(crossvalue2, start2->valuetypid);
				DATUM_FREE(cross, restypid); DATUM_FREE(endresult, restypid);
			}
		}
		DATUM_FREE(startresult, restypid);
		start1 = end1; start2 = end2;
		startvalue1 = endvalue1; startvalue2 = endvalue2;
		lower_inc = true;
	}
	pfree(inter);
	return k;
}

/**
 * Applies the binary function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalS *
sync_tfunc_temporalseq_temporalseq_cross(const TemporalSeq *seq1, 
	const TemporalSeq *seq2, Datum param, Datum (*func)(Datum, ...), 
	int numparam, Oid restypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) *
		(seq1->count + seq2->count) * 3);
	int count = sync_tfunc_temporalseq_temporalseq_cross1(sequences, seq1, seq2,
		param, func, numparam, restypid);
	/* Result has step interpolation */
	return temporals_make_free(sequences, count, true);
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

/**
 * Applies the function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] ts,seq Temporal values
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalS *
sync_tfunc_temporals_temporalseq_cross(const TemporalS *ts, const TemporalSeq *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) *
		(ts->totalcount + seq->count) * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		k += sync_tfunc_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq, param, func, numparam, restypid);
	}
	/* Result has step interpolation */
	return temporals_make_free(sequences, k, true);
}

/**
 * Applies the function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] seq,ts Temporal values
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalS *
sync_tfunc_temporalseq_temporals_cross(const TemporalSeq *seq, const TemporalS *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	return sync_tfunc_temporals_temporalseq_cross(ts, seq, param, func, numparam, restypid);
}

/**
 * Applies the function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
TemporalS *
sync_tfunc_temporals_temporals_cross(const TemporalS *ts1, const TemporalS *ts2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) *
		(ts1->totalcount + ts2->totalcount) * 3);
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		k += sync_tfunc_temporalseq_temporalseq_cross1(&sequences[k],
			seq1, seq2, param, func, numparam, restypid);
		if (period_eq_internal(&seq1->period, &seq2->period))
		{
			i++; j++;
		}
		else if (period_lt_internal(&seq1->period, &seq2->period))
			i++;
		else
			j++;
	}
	/* Result has step interpolation */
	return temporals_make_free(sequences, k, true);
}

/*****************************************************************************/

/**
 * Applies the function to the temporal sequence values.
 * (dispatch function)
 *
 * The function is applied when at least one sequence has linear interpolation
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 */
Temporal *
sync_tfunc_temporal_temporal_cross(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	bool linear = MOBDB_FLAGS_GET_LINEAR(temp1->flags) ||
		MOBDB_FLAGS_GET_LINEAR(temp2->flags);
	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporalinst_temporalinst(
			(TemporalInst *)temp1, (TemporalInst *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporalinst_temporali(
			(TemporalInst *)temp1, (TemporalI *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc_temporalinst_temporalseq(
			(TemporalInst *)temp1, (TemporalSeq *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALINST && temp2->duration == TEMPORALS)
		result = (Temporal *)sync_tfunc_temporalinst_temporals(
			(TemporalInst *)temp1, (TemporalS *)temp2, param, func, numparam, restypid);

	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporali_temporalinst(
			(TemporalI *)temp1, (TemporalInst *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporali_temporali(
			(TemporalI *)temp1, (TemporalI *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALSEQ)
		result = (Temporal *)sync_tfunc_temporali_temporalseq(
			(TemporalI *)temp1, (TemporalSeq *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALI && temp2->duration == TEMPORALS)
		result = (Temporal *)sync_tfunc_temporali_temporals(
			(TemporalI *)temp1, (TemporalS *)temp2, param, func, numparam, restypid);

	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporalseq_temporalinst(
			(TemporalSeq *)temp1, (TemporalInst *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporalseq_temporali(
			(TemporalSeq *)temp1, (TemporalI *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALSEQ)
		result = linear ?
			(Temporal *)sync_tfunc_temporalseq_temporalseq_cross(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, func, numparam, restypid) :
			(Temporal *)sync_tfunc_temporalseq_temporalseq(
				(TemporalSeq *)temp1, (TemporalSeq *)temp2, param, func, numparam, restypid, linear, false);
	else if (temp1->duration == TEMPORALSEQ && temp2->duration == TEMPORALS)
		result = linear ?
			(Temporal *)sync_tfunc_temporalseq_temporals_cross(
				(TemporalSeq *)temp1, (TemporalS *)temp2, param, func, numparam, restypid) :
			(Temporal *)sync_tfunc_temporalseq_temporals(
				(TemporalSeq *)temp1, (TemporalS *)temp2, param, func, numparam, restypid, linear, false);

	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALINST)
		result = (Temporal *)sync_tfunc_temporals_temporalinst(
			(TemporalS *)temp1, (TemporalInst *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALI)
		result = (Temporal *)sync_tfunc_temporals_temporali(
			(TemporalS *)temp1, (TemporalI *)temp2, param, func, numparam, restypid);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALSEQ)
		result = linear ?
			(Temporal *)sync_tfunc_temporals_temporalseq_cross(
				(TemporalS *)temp1, (TemporalSeq *)temp2, param, func, numparam, restypid) :
			(Temporal *)sync_tfunc_temporals_temporalseq(
				(TemporalS *)temp1, (TemporalSeq *)temp2, param, func, numparam, restypid, linear, false);
	else if (temp1->duration == TEMPORALS && temp2->duration == TEMPORALS)
		result = linear ?
			(Temporal *)sync_tfunc_temporals_temporals_cross(
				(TemporalS *)temp1, (TemporalS *)temp2, param, func, numparam, restypid) :
			(Temporal *)sync_tfunc_temporals_temporals(
				(TemporalS *)temp1, (TemporalS *)temp2, param, func, numparam, restypid, linear, false);

	return result;
}

/*****************************************************************************/
