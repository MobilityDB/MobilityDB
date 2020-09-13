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
 * The lifting of functions and operators must take into account the following
 * characteristic of the non-lifted function
 * 1. The number of arguments
 *	- binary functions, such as spatial relationships functions (e.g. 
 *	  intersects). 
 *	- ternary functions, such as spatial relationships functions that need 
 *	  an additional parameter (e.g. tdwithin). 
 *	- quaternary functions which apply binary operators (e.g. + or <) to
 *	  temporal number types that can be of different base type (that is,
 *	  integer and float), and thus the third and fourth arguments are the
 *	  Oids of the first two arguments.
 * 2. The type of the arguments
 *	 - a temporal type and a base type. In this case the non-lifted function
 *	   is applied  to each instant of the temporal type.
 *	 - two temporal types. In this case the operands must be synchronized 
 *	   and the function is applied to each pair of synchronized instants. 
 * 3. Whether the function has instantaneous discontinuities at the crossings.
 *	  Examples of such functions are temporal comparisons for temporal floats
 *	  or temporal spatial relationships since the value of the result may
 *	  change immediately before, at, or immediately after a crossing.
 * 4. Whether intermediate points between synchronized instants must be added
 * 	  to take into account the crossings or the turning points (or local 
  *	  minimum/maximum) of the function. For example, tfloat + tfloat
 *	  only needs to synchronize the arguments while tfloat * tfloat requires
 *	  in addition to add the turning point, which is the timestamp between the
 *	  two consecutive synchronized instants in which the linear functions
 *	  defined by the segments are equal.
 * 
 * Examples
 *	 - tfloatseq * base => tfunc_tsequence_base
 *	   applies the * operator to each instant.
 *	 - tfloatseq < base => tfunc4_tsequence_base_discont
 *	   applies the < operator to each instant, if the tfloatseq is equal
 *	   to base in the middle of two consecutive instants add an instant
 *	   sequence at the crossing. The result is a tfloatseqset.
 *	 - tfloatseq + tfloatseq => sync_tfunc_tsequence_tsequence
 *	   synchronizes the sequences and applies the + operator to each instant.
 *	 - tfloatseq * tfloatseq => sync_tfunc_tsequence_tsequence
 *	   synchronizes the sequences adding the turning points and applies the *
 *	   operator to each instant. The result is a tfloatseq.
 *	 - tfloatseq < tfloatseq => sync_tfunc_tsequence_tsequence_discont
 *	   synchronizes the sequences, applies the < operator to each instant, 
 *	   and if there is a crossing in the middle of two consecutive pairs of 
 *	   instants add an instant sequence and the crossing. The result is a 
 *	   tfloatseqset.
 *
 * An important issue when lifting functions is to avoid code redundancy.
 * Indeed, the same code must be applied for functions with 2, 3 and 4 arguments.
 * Variadic function pointers are used to solve the problem. The idea is 
 * sketched next.
 * @code
 * typedef Datum (*varfunc)    (Datum, ...);
 * 
 * TInstant *
 * tfunc_tinstant(const TInstant *inst, Datum param,
 * Datum (*func)(Datum, ...), int numparam, Oid restypid)
 * {
 * 	Datum resvalue;
 * 	if (numparam == 1)
 * 		resvalue = (*func)(temporalinst_value(inst));
 * 	else if (numparam == 2)
 * 		resvalue = (*func)(temporalinst_value(inst), param);
 * 	else
 * 		elog(ERROR, "Number of function parameters not supported: %u", numparam);
 * 	TInstant *result = tinstant_make(resvalue, inst->t, restypid);
 * 	DATUM_FREE(resvalue, restypid);
 * 	return result;
 * }
 * 
 * // Definitions for TInstantSet, TSequence, and TSequenceSet
 * [...]
 * 
 * // Dispatch function
 * Temporal *
 * tfunc_temporal(const Temporal *temp, Datum param,
 * 	Datum (*func)(Datum, ...), int numparam, Oid restypid)
 * {
 * 	// Dispatch depending on the duration
 * 	[...]
 * }
 * @endcode
 * Examples of use of the above function are given next.
 * @code
 * // Transform the geometry to a geography
 * PGDLLEXPORT Datum
 * tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
 * {
 * 	Temporal *temp = PG_GETARG_TEMPORAL(0);
 * 	Temporal *result = tfunc_temporal(temp, (Datum) NULL,
 * 		(varfunc) &geom_to_geog, 1, type_oid(T_GEOGRAPHY));
 * 	PG_FREE_IF_COPY(temp, 0);
 * 	PG_RETURN_POINTER(result);
 * }
 * 
 * // Round the temporal number to the number of decimal places
 * PGDLLEXPORT Datum
 * tnumber_round(PG_FUNCTION_ARGS)
 * {
 * 	Temporal *temp = PG_GETARG_TEMPORAL(0);
 * 	Datum digits = PG_GETARG_DATUM(1);
 * 	Temporal *result = tfunc_temporal(temp, digits,
 * 		(varfunc) &datum_round, 2, FLOAT8OID);
 * 	PG_FREE_IF_COPY(temp, 0);
 * 	PG_RETURN_POINTER(result);
 * }
 * @endcode
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
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
tfunc_tinstant(const TInstant *inst, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum resvalue;
	if (numparam == 1)
		resvalue = (*func)(tinstant_value(inst));
	else if (numparam == 2)
		resvalue = (*func)(tinstant_value(inst), param);
	else
		elog(ERROR, "Number of function parameters not supported: %u", numparam);
	TInstant *result = tinstant_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Applies the function with the additional parameter to the temporal value
 *
 * @param[in] ti Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstantSet *
tfunc_tinstantset(const TInstantSet *ti, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		instants[i] = tfunc_tinstant(inst, param, func, numparam, restypid);
	}
	return tinstantset_make_free(instants, ti->count);
}

/**
 * Applies the function with the additional parameter to the temporal value
 *
 * @param[in] seq Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TSequence *
tfunc_tsequence(const TSequence *seq, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TInstant *inst = tsequence_inst_n(seq, i);
		instants[i] = tfunc_tinstant(inst, param, func, numparam, restypid);
	}
	bool linear = MOBDB_FLAGS_GET_LINEAR(seq->flags) &&
		linear_interpolation(restypid);
	return tsequence_make_free(instants, seq->count,
		seq->period.lower_inc, seq->period.upper_inc, linear, NORMALIZE);
}

/**
 * Applies the function with the additional parameter to the temporal value
 *
 * @param[in] ts Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TSequenceSet *
tfunc_tsequenceset(const TSequenceSet *ts, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		sequences[i] = tfunc_tsequence(seq, param, func, numparam, restypid);
	}
	return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
}

/**
 * Applies the function with the additional parameter to the temporal value
 * (dispatch function)
 *
 * @param[in] temp Temporal value
 * @param[in] param Parameter of the function
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
Temporal *
tfunc_temporal(const Temporal *temp, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Temporal *result;
	ensure_valid_duration(temp->duration);
	if (temp->duration == INSTANT)
		result = (Temporal *)tfunc_tinstant((TInstant *)temp,
			param, func, numparam, restypid);
	else if (temp->duration == INSTANTSET)
		result = (Temporal *)tfunc_tinstantset((TInstantSet *)temp,
			param, func, numparam, restypid);
	else if (temp->duration == SEQUENCE)
		result = (Temporal *)tfunc_tsequence((TSequence *)temp,
			param, func, numparam, restypid);
	else /* temp->duration == SEQUENCESET */
		result = (Temporal *)tfunc_tsequenceset((TSequenceSet *)temp,
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
TInstant *
tfunc_tinstant_base(const TInstant *inst, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	Datum value1 = tinstant_value(inst);
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
	TInstant *result = tinstant_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Applies the binary function to the temporal value and the base value
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
TInstantSet *
tfunc_tinstantset_base(const TInstantSet *ti, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		instants[i] = tfunc_tinstant_base(inst, value, valuetypid, param,
			func, numparam, restypid, invert);
	}
	return tinstantset_make_free(instants, ti->count);
}

/**
 * Applies the binary function to the temporal value and the base value
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
TSequence *
tfunc_tsequence_base(const TSequence *seq, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	TInstant **instants = palloc(sizeof(TInstant *) * seq->count);
	for (int i = 0; i < seq->count; i++)
	{
		TInstant *inst = tsequence_inst_n(seq, i);
		instants[i] = tfunc_tinstant_base(inst, value, valuetypid, param,
			func, numparam, restypid, invert);
	}
	return tsequence_make(instants, seq->count, seq->period.lower_inc, 
		seq->period.upper_inc, MOBDB_FLAGS_GET_LINEAR(seq->flags), NORMALIZE);
}

/**
 * Applies the binary function to the temporal value and the base value
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

TSequenceSet *
tfunc_tsequenceset_base(const TSequenceSet *ts, Datum value, Oid valuetypid, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid, bool invert)
{
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		sequences[i] = tfunc_tsequence_base(seq, value, valuetypid, param,
			func, numparam, restypid, invert);
	}
	return tsequenceset_make_free(sequences, ts->count, NORMALIZE);
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
	if (temp->duration == INSTANT)
		result = (Temporal *)tfunc_tinstant_base((TInstant *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	else if (temp->duration == INSTANTSET)
		result = (Temporal *)tfunc_tinstantset_base((TInstantSet *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	else if (temp->duration == SEQUENCE)
		result = (Temporal *)tfunc_tsequence_base((TSequence *)temp,
			value, valuetypid, param, func, numparam, restypid, invert);
	else /* temp->duration == SEQUENCESET */
		result = (Temporal *)tfunc_tsequenceset_base((TSequenceSet *)temp,
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
tfunc4_tsequence_base_discont1(TSequence **result, const TSequence *seq,
	Datum value, Oid valuetypid, Datum (*func)(Datum, Datum, Oid, Oid),
	Oid restypid, bool invert)
{
	/* Instantaneous sequence */
	if (seq->count == 1)
	{
		TInstant *inst = tsequence_inst_n(seq, 0);
		Datum value1 = invert ?
			func(value, tinstant_value(inst), valuetypid, inst->valuetypid) :
			func(tinstant_value(inst), value, inst->valuetypid, valuetypid);
		TInstant *inst1 = tinstant_make(value1, inst->t, restypid);
		/* Result has step interpolation */
		result[0] = tinstant_to_tsequence(inst1, STEP);
		return 1;
	}

	int k = 0;
	TInstant *inst1 = tsequence_inst_n(seq, 0);
	Datum value1 = tinstant_value(inst1);
	bool lower_inc = seq->period.lower_inc;
	Datum startresult = invert ?
		func(value, value1, valuetypid, inst1->valuetypid) :
		func(value1, value, inst1->valuetypid, valuetypid);
	/* We create two temporal instants with arbitrary values that are set in
	 * the for loop to avoid creating and freeing the instants each time a
	 * segment of the result is computed */
	TInstant *instants[2];
	instants[0] = tinstant_make(startresult, inst1->t, restypid);
	instants[1] = tinstant_make(startresult, inst1->t, restypid);
	for (int i = 1; i < seq->count; i++)
	{
		/* Each iteration of the loop adds between one and three sequences */
		startresult = invert ?
			func(value, value1, valuetypid, inst1->valuetypid) :
			func(value1, value, inst1->valuetypid, valuetypid);
		TInstant *inst2 = tsequence_inst_n(seq, i);
		Datum value2 = tinstant_value(inst2);
		Datum intvalue, intresult, endresult;
		bool upper_inc = (i == seq->count - 1) ? seq->period.upper_inc : false;
		/* If both segments are constant compute the function at the inst1 and
		 * inst2 instants */
		if (datum_eq(value1, value2, inst1->valuetypid))
		{
			/*  The first instant value created above is the one needed here */
			tinstant_set(instants[0], startresult, inst1->t);
			tinstant_set(instants[1], startresult, inst2->t);
			/* Result has step interpolation */
			result[k++] = tsequence_make(instants, 2, lower_inc, upper_inc,
				STEP, NORMALIZE_NO);
		}
			/* If either the inst1 or the inst2 value is equal to the value compute
			 * the function at the inst1, at the middle, and at the inst2 instants */
		else if (datum_eq2(value1, value, inst1->valuetypid, valuetypid) ||
				 datum_eq2(value2, value, inst1->valuetypid, valuetypid))
		{
			/* Compute the function at the inst1 instant */
			if (lower_inc)
			{
				tinstant_set(instants[0], startresult, inst1->t);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 1, true, true,
					STEP, NORMALIZE_NO);
			}
			/* Find the middle time between inst1 and the inst2 instant and compute
			 * the function at that point */
			TimestampTz inttime = inst1->t + ((inst2->t - inst1->t)/2);
			/* Linear interpolation */
			intvalue = tsequence_value_at_timestamp1(inst1, inst2, true, inttime);
			intresult = invert ?
				func(value, intvalue, valuetypid, inst1->valuetypid) :
				func(intvalue, value, inst1->valuetypid, valuetypid);
			tinstant_set(instants[0], intresult, inst1->t);
			tinstant_set(instants[1], intresult, inst2->t);
			/* Result has step interpolation */
			result[k++] = tsequence_make(instants, 2, false, false,
				STEP, NORMALIZE_NO);
			/* Compute the function at the inst2 instant */
			if (upper_inc)
			{
				endresult = invert ?
					func(value, value2, valuetypid, inst1->valuetypid) :
					func(value2, value, inst1->valuetypid, valuetypid);
				tinstant_set(instants[0], endresult, inst2->t);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 1, true, true,
					STEP, NORMALIZE_NO);
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
				tinstant_set(instants[0], startresult, inst1->t);
				tinstant_set(instants[1], startresult, inst2->t);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 2, lower_inc, upper_inc,
					STEP, NORMALIZE_NO);
			}
			else
			{
				/* Since there is a crossing in the middle compute the function at the
				 * inst1 instant, at the crossing, and at the inst2 instant */
				tinstant_set(instants[0], startresult, inst1->t);
				tinstant_set(instants[1], startresult, crosstime);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 2, lower_inc, false,
					STEP, NORMALIZE_NO);
				/* Compute the function at the crossing */
				intresult = func(crossvalue, value, valuetypid, valuetypid);
				tinstant_set(instants[0], intresult, crosstime);
				/* Find the middle time between inst1 and the inst2 instant and compute
				 * the function at that point */
				TimestampTz inttime = crosstime + ((inst2->t - crosstime)/2);
				/* Linear interpolation */
				intvalue = tsequence_value_at_timestamp1(inst1, inst2, true, inttime);
				endresult = invert ?
					func(value, intvalue, valuetypid, inst1->valuetypid) :
					func(intvalue, value, inst1->valuetypid, valuetypid);
				if (datum_eq(intresult, endresult, restypid))
				{
					tinstant_set(instants[1], endresult, inst2->t);
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 2, true, upper_inc,
						STEP, NORMALIZE_NO);
				}
				else
				{
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 1, true, true,
						STEP, NORMALIZE_NO);
					tinstant_set(instants[0], endresult, crosstime);
					tinstant_set(instants[1], endresult, inst2->t);
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 2, false, upper_inc,
						STEP, NORMALIZE_NO);
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
TSequenceSet *
tfunc4_tsequence_base_discont(const TSequence *seq, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert)
{
	TSequence **sequences = palloc(sizeof(TSequence *) * seq->count * 3);
	int count = tfunc4_tsequence_base_discont1(sequences, seq, value, valuetypid,
		func, restypid, invert);
	/* Result has step interpolation */
	return tsequenceset_make_free(sequences, count, NORMALIZE);
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
TSequenceSet *
tfunc4_tsequenceset_base_discont(const TSequenceSet *ts, Datum value, Oid valuetypid,
	Datum (*func)(Datum, Datum, Oid, Oid), Oid restypid, bool invert)
{
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tfunc4_tsequence_base_discont1(&sequences[k], seq, value, valuetypid,
			func, restypid, invert);
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************
 * Functions that synchronize two temporal values and apply a function in
 * a single pass. Generic version with a variadic function
 *****************************************************************************/

/*
 * Apply the variadic function to the values, when their type may be different
 */
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
TInstant *
sync_tfunc_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	/* Test whether the two temporal values overlap on time */
	if (inst1->t != inst2->t)
		return NULL;
	
	Datum value1 = tinstant_value(inst1);
	Datum value2 = tinstant_value(inst2);
	Datum resvalue = tfunc(value1, value2, inst1->valuetypid, inst2->valuetypid,
		param, func, numparam);
	TInstant *result = tinstant_make(resvalue, inst1->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,inst Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
sync_tfunc_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum value1;
	if (!tinstantset_value_at_timestamp(ti, inst->t, &value1))
		return NULL;

	Datum value2 = tinstant_value(inst);
	Datum resvalue = tfunc(value1, value2, ti->valuetypid, inst->valuetypid,
		param, func, numparam);
	TInstant *result = tinstant_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;	
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,ti Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
sync_tfunc_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_tinstantset_tinstant(ti, inst, param, func,
		numparam, restypid);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,inst Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
sync_tfunc_tsequence_tinstant(const TSequence *seq, const TInstant *inst,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum value1;
	if (!tsequence_value_at_timestamp(seq, inst->t, &value1))
		return NULL;

	Datum value2 = tinstant_value(inst);
	Datum resvalue = tfunc(value1, value2, seq->valuetypid, inst->valuetypid,
		param, func, numparam);
	TInstant *result = tinstant_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,seq Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
sync_tfunc_tinstant_tsequence(const TInstant *inst, const TSequence *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_tsequence_tinstant(seq, inst, param, func,
		numparam, restypid);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,inst Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
sync_tfunc_tsequenceset_tinstant(const TSequenceSet *ts, const TInstant *inst,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	Datum value1;
	if (!tsequenceset_value_at_timestamp(ts, inst->t, &value1))
		return NULL;

	Datum value2 = tinstant_value(inst);
	Datum resvalue = tfunc(value1, value2, ts->valuetypid, inst->valuetypid,
		param, func, numparam);
	TInstant *result = tinstant_make(resvalue, inst->t, restypid);
	DATUM_FREE(resvalue, restypid);
	return result;
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] inst,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstant *
sync_tfunc_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_tsequenceset_tinstant(ts, inst, param, func, 
		numparam, restypid);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function 
 *
 * @param[in] ti1,ti2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @note This function is called by other functions besides the dispatch 
 * function sync_tfunc_temporal_temporal and thus the bounding period test is
 * repeated
 */
TInstantSet *
sync_tfunc_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	tinstantset_period(&p1, ti1);
	tinstantset_period(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return NULL;

	TInstant **instants = palloc(sizeof(TInstant *) *
		Min(ti1->count, ti2->count));
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TInstant *inst1 = tinstantset_inst_n(ti1, i);
		TInstant *inst2 = tinstantset_inst_n(ti2, j);
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			Datum value1 = tinstant_value(inst1);
			Datum value2 = tinstant_value(inst2);
			Datum resvalue = tfunc(value1, value2, ti1->valuetypid, 
				ti2->valuetypid, param, func, numparam);
			instants[k++] = tinstant_make(resvalue, inst1->t, restypid);
			DATUM_FREE(resvalue, restypid);
			i++; j++;
		}
		else if (cmp < 0)
			i++;
		else
			j++;
	}
	return tinstantset_make_free(instants, k);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,ti Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstantSet *
sync_tfunc_tsequence_tinstantset(const TSequence *seq, const TInstantSet *ti,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int k = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			tsequence_value_at_timestamp(seq, inst->t, &value1);
			Datum value2 = tinstant_value(inst);
			Datum resvalue = tfunc(value1, value2, seq->valuetypid, 
				ti->valuetypid, param, func, numparam);
			instants[k++] = tinstant_make(resvalue, inst->t, restypid);
			DATUM_FREE(value1, seq->valuetypid); DATUM_FREE(resvalue, restypid);
		}
		if (seq->period.upper < inst->t)
			break;
	}
	return tinstantset_make_free(instants, k);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,seq Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstantSet *
sync_tfunc_tinstantset_tsequence(const TInstantSet *ti, const TSequence *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_tsequence_tinstantset(seq, ti, param, func, numparam,
		restypid);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,ti Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstantSet *
sync_tfunc_tsequenceset_tinstantset(const TSequenceSet *ts, const TInstantSet *ti,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ti->count)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		TInstant *inst = tinstantset_inst_n(ti, j);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			Datum value1;
			tsequenceset_value_at_timestamp(ts, inst->t, &value1);
			Datum value2 = tinstant_value(inst);
			Datum resvalue = tfunc(value1, value2, ts->valuetypid,
				ti->valuetypid, param, func, numparam);
			instants[k++] = tinstant_make(resvalue, inst->t, restypid);
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
	return tinstantset_make_free(instants, k);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ti,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TInstantSet *
sync_tfunc_tinstantset_tsequenceset(const TInstantSet *ti, const TSequenceSet *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	// Works for commutative functions
	return sync_tfunc_tsequenceset_tinstantset(ts, ti, param, func, numparam,
		restypid);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq1,seq2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 * @note This function is called for each composing sequence of a temporal 
 * sequence set and therefore the bounding period test is repeated
 */
TSequence *
sync_tfunc_tsequence_tsequence(const TSequence *seq1, const TSequence *seq2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TInstant *, const TInstant *, const TInstant *,
		const TInstant *, TimestampTz *))
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
		tsequence_value_at_timestamp(seq1, inter->lower, &value1);
		tsequence_value_at_timestamp(seq2, inter->lower, &value2);
		Datum resvalue = tfunc(value1, value2, seq1->valuetypid,
			seq2->valuetypid, param, func, numparam);
		TInstant *inst = tinstant_make(resvalue, inter->lower, restypid);
		TSequence *result = tinstant_to_tsequence(inst, reslinear);
		DATUM_FREE(value1, seq1->valuetypid);
		DATUM_FREE(value2, seq2->valuetypid);
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
	TInstant *inst1 = tsequence_inst_n(seq1, 0);
	TInstant *inst2 = tsequence_inst_n(seq2, 0);
	TInstant *tofreeinst = NULL;
	int i = 0, j = 0, k = 0, l = 0;
	if (inst1->t < inter->lower)
	{
		inst1 = tsequence_at_timestamp(seq1, inter->lower);
		tofreeinst = inst1;
		i = tsequence_find_timestamp(seq1, inter->lower);
	}
	else if (inst2->t < inter->lower)
	{
		inst2 = tsequence_at_timestamp(seq2, inter->lower);
		tofreeinst = inst2;
		j = tsequence_find_timestamp(seq2, inter->lower);
	}
	int count = (seq1->count - i + seq2->count - j) * 2;
	TInstant **instants = palloc(sizeof(TInstant *) * count);
	TInstant **tofree = palloc(sizeof(TInstant *) * count);
	if (tofreeinst != NULL)
		tofree[l++] = tofreeinst;
	TInstant *prev1, *prev2;
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
			inst2 = tsequence_at_timestamp(seq2, inst1->t);
			tofree[l++] = inst2;
		}
		else
		{
			j++;
			inst1 = tsequence_at_timestamp(seq1, inst2->t);
			tofree[l++] = inst1;
		}
		/* If not the first instant compute the function on the potential
		   intermediate point before adding the new instants */
		if (turnpoint != NULL && k > 0 &&
			turnpoint(prev1, inst1, prev2, inst2, &intertime))
		{
			Datum inter1 = tsequence_value_at_timestamp1(prev1, inst1,
				linear1, intertime);
			Datum inter2 = tsequence_value_at_timestamp1(prev2, inst2,
				linear2, intertime);
			value = tfunc(inter1, inter2, seq1->valuetypid, seq2->valuetypid,
				param, func, numparam);
			instants[k++] = tinstant_make(value, intertime, restypid);
			DATUM_FREE(inter1, seq1->valuetypid); DATUM_FREE(inter2, seq2->valuetypid);
			DATUM_FREE(value, restypid);
			}
		Datum value1 = tinstant_value(inst1);
		Datum value2 = tinstant_value(inst2);
		value = tfunc(value1, value2, seq1->valuetypid, seq2->valuetypid,
			param, func, numparam);
		instants[k++] = tinstant_make(value, inst1->t, restypid);
		DATUM_FREE(value, restypid);
		if (i == seq1->count || j == seq2->count)
			break;
		prev1 = inst1; prev2 = inst2;
		inst1 = tsequence_inst_n(seq1, i);
		inst2 = tsequence_inst_n(seq2, j);
	}
	/* We are sure that k != 0 due to the period intersection test above */
	/* The last two values of sequences with step interpolation and
	   exclusive upper bound must be equal */
	if (!reslinear && !inter->upper_inc && k > 1)
	{
		tofree[l++] = instants[k - 1];
		value = tinstant_value(instants[k - 2]);
		instants[k - 1] = tinstant_make(value, instants[k - 1]->t, restypid);
	}

	for (i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(tofree); pfree(inter);

   return tsequence_make_free(instants, k, inter->lower_inc,
		inter->upper_inc, reslinear, NORMALIZE);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts,seq Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TSequenceSet *
sync_tfunc_tsequenceset_tsequence(const TSequenceSet *ts, const TSequence *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TInstant *, const TInstant *, const TInstant *,
		const TInstant *, TimestampTz *))
{
	int loc;
	tsequenceset_find_timestamp(ts, seq->period.lower, &loc);
	/* We are sure that loc < ts->count due to the bounding period test made
	 * in the dispatch function */
	TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count - loc));
	int k = 0;
	for (int i = loc; i < ts->count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts, i);
		TSequence *seq2 = sync_tfunc_tsequence_tsequence(seq1, seq,
			param, func, numparam, restypid, reslinear, turnpoint);
		if (seq2 != NULL)
			sequences[k++] = seq2;
		int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
		if (cmp < 0 ||
			(cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
			break;
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE_NO);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] seq,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TSequenceSet *
sync_tfunc_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TInstant *, const TInstant *, const TInstant *,
		const TInstant *, TimestampTz *))
{
	return sync_tfunc_tsequenceset_tsequence(ts, seq, param, func, numparam,
		restypid, reslinear, turnpoint);
}

/**
 * Synchronizes the temporal values and applies to them the function
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] numparam Number of parameters of the function
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
TSequenceSet *
sync_tfunc_tsequenceset_tsequenceset(const TSequenceSet *ts1, const TSequenceSet *ts2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid, bool reslinear,
	bool (*turnpoint)(const TInstant *, const TInstant *, const TInstant *,
		const TInstant *, TimestampTz *))
{
	TSequence **sequences = palloc(sizeof(TSequence *) *
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, j);
		TSequence *seq = sync_tfunc_tsequence_tsequence(seq1, seq2,
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
	return tsequenceset_make_free(sequences, k, NORMALIZE_NO);
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
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @note This function is called for each composing sequence of a temporal 
 * sequence set and therefore the bounding period test is repeated
 */
static int
sync_tfunc_tsequence_tsequence_discont1(TSequence **result, 
	const TSequence *seq1, const TSequence *seq2, Datum param,
	Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period *inter = intersection_period_period_internal(&seq1->period,
		&seq2->period);
	if (inter == NULL)
		return 0;

	/* If the two sequences intersect at an instant */
	if (inter->lower == inter->upper)
	{
		Datum value1, value2;
		tsequence_value_at_timestamp(seq1, inter->lower, &value1);
		tsequence_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = tfunc(value1, value2, seq1->valuetypid, seq2->valuetypid, 
			param, func, numparam);
		TInstant *inst = tinstant_make(value, inter->lower, restypid);
		/* Result has step interpolation */
		result[0] = tinstant_to_tsequence(inst, STEP);
		DATUM_FREE(value1, seq1->valuetypid);
		DATUM_FREE(value2, seq2->valuetypid);
		pfree(inst); pfree(inter);
		return 1;
	}

	/* General case */
	TInstant **tofree = palloc(sizeof(TInstant *) *
		(seq1->count + seq2->count) * 2);
	TInstant *start1 = tsequence_inst_n(seq1, 0);
	TInstant *start2 = tsequence_inst_n(seq2, 0);
	int i = 1, j = 1, k = 0, l = 0;
	if (start1->t < inter->lower)
	{
		start1 = tsequence_at_timestamp(seq1, inter->lower);
		tofree[l++] = start1;
		i = tsequence_find_timestamp(seq1, inter->lower) + 1;
	}
	else if (start2->t < inter->lower)
	{
		start2 = tsequence_at_timestamp(seq2, inter->lower);
		tofree[l++] = start2;
		j = tsequence_find_timestamp(seq2, inter->lower) + 1;
	}
	bool lower_inc = inter->lower_inc;
	bool linear1 = MOBDB_FLAGS_GET_LINEAR(seq1->flags);
	bool linear2 = MOBDB_FLAGS_GET_LINEAR(seq2->flags);
	Datum startvalue1 = tinstant_value(start1);
	Datum startvalue2 = tinstant_value(start2);
	TInstant *instants[2];
	Datum startresult;
	while (i < seq1->count && j < seq2->count)
	{
		TInstant *end1 = tsequence_inst_n(seq1, i);
		TInstant *end2 = tsequence_inst_n(seq2, j);
		int cmp = timestamp_cmp_internal(end1->t, end2->t);
		if (cmp == 0)
		{
			i++; j++;
		}
		else if (cmp < 0)
		{
			i++;
			end2 = tsequence_at_timestamp1(start2, end2, linear2, end1->t);
			tofree[l++] = end2;
		}
		else
		{
			j++;
			end1 = tsequence_at_timestamp1(start1, end1, linear1, end2->t);
			tofree[l++] = end1;
		}
		bool upper_inc = (end1->t == inter->upper) ? inter->upper_inc : false;
		/* The remaining of the loop adds between one and three sequences */
		Datum endvalue1 = tinstant_value(end1);
		Datum endvalue2 = tinstant_value(end2);
		startresult = tfunc(startvalue1, startvalue2, seq1->valuetypid, seq2->valuetypid, 
			param, func, numparam);

		/* If both segments are constant compute the function at the start and
		 * end instants */
		if (datum_eq(startvalue1, endvalue1, start1->valuetypid) &&
			datum_eq(startvalue2, endvalue2, start2->valuetypid))
		{
			instants[0] = tinstant_make(startresult, start1->t, restypid);
			instants[1] = tinstant_make(startresult, end1->t, restypid);
			/* Result has step interpolation */
			result[k++] = tsequence_make(instants, 2, lower_inc, upper_inc,
				STEP, NORMALIZE_NO);
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
				instants[0] = tinstant_make(startresult, start1->t, restypid);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 1, true, true,
					STEP, NORMALIZE_NO);
				pfree(instants[0]);
			}
			/* Find the middle time between start and the end instant and compute
			 * the function at that point */
			TimestampTz inttime = start1->t + ((end1->t - start1->t)/2);
			Datum value1 = tsequence_value_at_timestamp1(start1, end1, linear1, inttime);
			Datum value2 = tsequence_value_at_timestamp1(start2, end2, linear2, inttime);
			Datum intresult = tfunc(value1, value2, seq1->valuetypid, seq2->valuetypid, 
				param, func, numparam);
			instants[0] = tinstant_make(intresult, start1->t, restypid);
			instants[1] = tinstant_make(intresult, end1->t, restypid);
			/* Result has step interpolation */
			result[k++] = tsequence_make(instants, 2, false, false,
				STEP, NORMALIZE_NO);
			DATUM_FREE(value1, start1->valuetypid); DATUM_FREE(value2, start2->valuetypid);
			DATUM_FREE(intresult, restypid);
			pfree(instants[0]); pfree(instants[1]);
			/* Compute the function at the end instant */
			if (upper_inc)
			{
				Datum endresult = tfunc(endvalue1, endvalue2, seq1->valuetypid, seq2->valuetypid, 
					param, func, numparam);
				instants[0] = tinstant_make(endresult, end1->t, restypid);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 1, true, true,
					STEP, NORMALIZE_NO);
				DATUM_FREE(endresult, restypid);
				pfree(instants[0]);
			}
		}
		else
		{
			/* Determine whether there is a crossing */
			TimestampTz crosstime;
			Datum crossvalue1, crossvalue2;
			bool hascross = tsequence_intersection(start1, end1, linear1,
				start2, end2, linear2, &crossvalue1, &crossvalue2, &crosstime);
			/* If there is no crossing compute the function at the start and end
			 * instants taking into account that the start and end values of the
			 * result may be different */
			if (!hascross)
			{
				instants[0] = tinstant_make(startresult, start1->t, restypid);
				instants[1] = tinstant_make(startresult, end1->t, restypid);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 2, lower_inc, false,
					STEP, NORMALIZE_NO);
				pfree(instants[0]); pfree(instants[1]);
				if (upper_inc)
				{
					Datum endresult = tfunc(endvalue1, endvalue2, seq1->valuetypid, seq2->valuetypid, 
						param, func, numparam);
					instants[0] = tinstant_make(endresult, end1->t, restypid);
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 1, true, true,
						STEP, NORMALIZE_NO);
					DATUM_FREE(endresult, restypid);
					pfree(instants[0]);
				}
			}
			else
			{
				/* There is a crossing at the middle */
				instants[0] = tinstant_make(startresult, start1->t, restypid);
				instants[1] = tinstant_make(startresult, crosstime, restypid);
				/* Result has step interpolation */
				result[k++] = tsequence_make(instants, 2, lower_inc, false,
					STEP, NORMALIZE_NO);
				pfree(instants[0]); pfree(instants[1]);
				/* Find the value at the local minimum/maximum */
				Datum cross = tfunc(crossvalue1, crossvalue2, seq1->valuetypid, seq2->valuetypid, 
					param, func, numparam);
				instants[0] = tinstant_make(cross, crosstime, restypid);
				Datum endresult = tfunc(endvalue1, endvalue2, seq1->valuetypid, seq2->valuetypid, 
					param, func, numparam);
				if (datum_eq(cross, endresult, restypid))
				{
					instants[1] = tinstant_make(endresult, end1->t, restypid);
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 2, true, upper_inc,
						STEP, NORMALIZE_NO);
					pfree(instants[0]); pfree(instants[1]);
				}
				else
				{
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 1, true, true,
						STEP, NORMALIZE_NO);
					pfree(instants[0]);
					instants[0] = tinstant_make(endresult, crosstime, restypid);
					instants[1] = tinstant_make(endresult, end1->t, restypid);
					/* Result has step interpolation */
					result[k++] = tsequence_make(instants, 2, false, upper_inc,
						STEP, NORMALIZE_NO);
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
 * @param[in] param Parameter for ternary functions
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TSequenceSet *
sync_tfunc_tsequence_tsequence_discont(const TSequence *seq1, 
	const TSequence *seq2, Datum param, Datum (*func)(Datum, ...), 
	int numparam, Oid restypid)
{
	TSequence **sequences = palloc(sizeof(TSequence *) *
		(seq1->count + seq2->count) * 3);
	int count = sync_tfunc_tsequence_tsequence_discont1(sequences, seq1, seq2,
		param, func, numparam, restypid);
	/* Result has step interpolation */
	return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/*****************************************************************************
 * TSequenceSet and <Type>
 *****************************************************************************/

/**
 * Applies the function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] ts,seq Temporal values
 * @param[in] func Function
 * @param[in] param Parameter for ternary functions
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TSequenceSet *
sync_tfunc_tsequenceset_tsequence_discont(const TSequenceSet *ts, const TSequence *seq,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TSequence **sequences = palloc(sizeof(TSequence *) *
		(ts->totalcount + seq->count) * 3);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts, i);
		k += sync_tfunc_tsequence_tsequence_discont1(&sequences[k],
			seq1, seq, param, func, numparam, restypid);
	}
	/* Result has step interpolation */
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Applies the function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] seq,ts Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TSequenceSet *
sync_tfunc_tsequence_tsequenceset_discont(const TSequence *seq, const TSequenceSet *ts,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	return sync_tfunc_tsequenceset_tsequence_discont(ts, seq, param, func, numparam, restypid);
}

/**
 * Applies the function to the temporal values.
 *
 * The function is applied when at least one temporal value has linear interpolation
 *
 * @param[in] ts1,ts2 Temporal values
 * @param[in] func Function
 * @param[in] param Parameter for ternary functions
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 */
TSequenceSet *
sync_tfunc_tsequenceset_tsequenceset_discont(const TSequenceSet *ts1, const TSequenceSet *ts2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid)
{
	TSequence **sequences = palloc(sizeof(TSequence *) *
		(ts1->totalcount + ts2->totalcount) * 3);
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, j);
		k += sync_tfunc_tsequence_tsequence_discont1(&sequences[k],
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
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************/

/**
 * Synchronizes the temporal values and applies to them the function
 * (dispatch function)
 *
 * @param[in] temp1,temp2 Temporal values
 * @param[in] param Parameter for ternary functions
 * @param[in] func Function
 * @param[in] numparam Number of parameters of the function
 * @param[in] restypid Oid of the resulting base type
 * @param[in] reslinear True when the resulting value has linear interpolation
 * @param[in] discont True when the resulting value has instantaneous discontinuities
 * @param[in] turnpoint Function to add additional intermediate points to 
 * the segments of the temporal values for the turning points
 */
Temporal *
sync_tfunc_temporal_temporal(const Temporal *temp1, const Temporal *temp2,
	Datum param, Datum (*func)(Datum, ...), int numparam, Oid restypid,
	bool reslinear, bool discont, bool (*turnpoint)(const TInstant *, 
		const TInstant *, const TInstant *, const TInstant *, TimestampTz *))
{
	/* Bounding box test */
	Period p1, p2;
	temporal_period(&p1, temp1);
	temporal_period(&p2, temp2);
	if (! overlaps_period_period_internal(&p1, &p2))
		return NULL;

	Temporal *result = NULL;
	ensure_valid_duration(temp1->duration);
	ensure_valid_duration(temp2->duration);
	if (temp1->duration == INSTANT)
	{
		if (temp2->duration == INSTANT)
			result = (Temporal *)sync_tfunc_tinstant_tinstant(
				(TInstant *)temp1, (TInstant *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == INSTANTSET)
			result = (Temporal *)sync_tfunc_tinstant_tinstantset(
				(TInstant *)temp1, (TInstantSet *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == SEQUENCE)
			result = (Temporal *)sync_tfunc_tinstant_tsequence(
				(TInstant *)temp1, (TSequence *)temp2,
				param, func, numparam, restypid);
		else /* temp2->duration == SEQUENCESET */
			result = (Temporal *)sync_tfunc_tinstant_tsequenceset(
				(TInstant *)temp1, (TSequenceSet *)temp2,
				param, func, numparam, restypid);
	}
	else if (temp1->duration == INSTANTSET)
	{
		if (temp2->duration == INSTANT)
			result = (Temporal *)sync_tfunc_tinstantset_tinstant(
				(TInstantSet *)temp1, (TInstant *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == INSTANTSET)
			result = (Temporal *)sync_tfunc_tinstantset_tinstantset(
				(TInstantSet *)temp1, (TInstantSet *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == SEQUENCE)
			result = (Temporal *)sync_tfunc_tinstantset_tsequence(
				(TInstantSet *)temp1, (TSequence *)temp2,
				param, func, numparam, restypid);
		else /* temp2->duration == SEQUENCESET */
			result = (Temporal *)sync_tfunc_tinstantset_tsequenceset(
				(TInstantSet *)temp1, (TSequenceSet *)temp2,
				param, func, numparam, restypid);
	}
	else if (temp1->duration == SEQUENCE)
	{
		if (temp2->duration == INSTANT)
			result = (Temporal *)sync_tfunc_tsequence_tinstant(
				(TSequence *)temp1, (TInstant *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == INSTANTSET)
			result = (Temporal *)sync_tfunc_tsequence_tinstantset(
				(TSequence *)temp1, (TInstantSet *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == SEQUENCE)
			result = discont ?
				(Temporal *)sync_tfunc_tsequence_tsequence_discont(
					(TSequence *)temp1, (TSequence *)temp2,
					param, func, numparam, restypid) : 
				(Temporal *)sync_tfunc_tsequence_tsequence(
					(TSequence *)temp1, (TSequence *)temp2,
					param, func, numparam, restypid, reslinear, turnpoint);
		else /* temp2->duration == SEQUENCESET */
			result = discont ?
				(Temporal *)sync_tfunc_tsequence_tsequenceset_discont(
					(TSequence *)temp1, (TSequenceSet *)temp2,
					param, func, numparam, restypid) : 
				(Temporal *)sync_tfunc_tsequence_tsequenceset(
					(TSequence *)temp1, (TSequenceSet *)temp2,
					param, func, numparam, restypid, reslinear, turnpoint);
	}
	else /* temp1->duration == SEQUENCESET */
	{
		if (temp2->duration == INSTANT)
			result = (Temporal *)sync_tfunc_tsequenceset_tinstant(
				(TSequenceSet *)temp1, (TInstant *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == INSTANTSET)
			result = (Temporal *)sync_tfunc_tsequenceset_tinstantset(
				(TSequenceSet *)temp1, (TInstantSet *)temp2,
				param, func, numparam, restypid);
		else if (temp2->duration == SEQUENCE)
			result = discont ?
				(Temporal *)sync_tfunc_tsequenceset_tsequence_discont(
					(TSequenceSet *)temp1, (TSequence *)temp2,
					param, func, numparam, restypid) : 
				(Temporal *)sync_tfunc_tsequenceset_tsequence(
					(TSequenceSet *)temp1, (TSequence *)temp2,
					param, func, numparam, restypid, reslinear, turnpoint);
		else /* temp2->duration == SEQUENCESET */
			result = discont ?
				(Temporal *)sync_tfunc_tsequenceset_tsequenceset_discont(
					(TSequenceSet *)temp1, (TSequenceSet *)temp2,
					param, func, numparam, restypid) : 
				(Temporal *)sync_tfunc_tsequenceset_tsequenceset(
					(TSequenceSet *)temp1, (TSequenceSet *)temp2,
					param, func, numparam, restypid, reslinear, turnpoint);
	}
	return result;
}

/*****************************************************************************/
