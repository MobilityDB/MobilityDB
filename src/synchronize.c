/*****************************************************************************
 *
 * Synchronize.c
 *	  Functions that synchronize temporal types and lift the operators.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

/*****************************************************************************
 * TemporalInst and <Type>
 *****************************************************************************/

TemporalInst *
sync_oper2_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
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
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			i++; j++;
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
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
		if (timestamp_cmp_internal(seq->period.upper, inst->t) == 0)
		{
			i++; j++;
		}
		else if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
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
	Datum (*operator)(Datum, Datum), Datum valuetypid, bool crossings)
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
	
	int n1 = temporalseq_find_timestamp(seq1, inter->lower);
	int n2 = temporalseq_find_timestamp(seq2, inter->lower);
	/* The lower bound of the intersection may be exclusive */
	if (n1 == -1) n1 = 0;
	if (n2 == -1) n2 = 0;
	int count = (seq1->count - n1 + seq2->count - n2) * 2;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	int i = n1, j = n2, k = 0, l = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, j);
	TemporalInst *prev1, *prev2, *next1, *next2;
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count * 2);
	Datum cross1, cross2, value;
	TimestampTz crosstime;
	while (i < seq1->count && j < seq2->count)
	{
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			/* If not the first instant compute the operator on the potential crossing 
			   before adding the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (i == seq1->count-1 || j == seq2->count-1)
				break;
			next1 = temporalseq_inst_n(seq1, i+1);
			next2 = temporalseq_inst_n(seq2, j+1);
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
		{
			next1 = temporalseq_inst_n(seq1, i+1);
			inst1 = temporalseq_at_timestamp1(inst1, next1, inst2->t);
			tofree[l++] = inst1;
			/* If not the first instant add potential crossing before adding
			   the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (j == seq2->count-1)
				break;
			next2 = temporalseq_inst_n(seq2, j+1);
		}
		else 
		{
			next2 = temporalseq_inst_n(seq2, j+1);
			inst2 = temporalseq_at_timestamp1(inst2, next2, inst1->t);
			tofree[l++] = inst2;
			/* If not the first instant add potential crossing before adding
			   the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2));
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (i == seq1->count-1)
				break;
			next1 = temporalseq_inst_n(seq1, i+1);
		}
		if (timestamp_cmp_internal(next1->t, next2->t) < 0)
		{
			i++;
			inst1 = next1;
		}
		else if (timestamp_cmp_internal(next2->t, next1->t) < 0)
		{
			j++;
			inst2 = next2;
		}
		else
		{
			i++; j++;
			inst1 = next1;
			inst2 = next2;
		}
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
		inter->lower_inc, inter->upper_inc, false);
	
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(instants); pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper2_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum), Datum valuetypid, bool crossings)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_oper2_temporalseq_temporalseq(seq1, seq, 
			operator, valuetypid, crossings);
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
	Datum (*operator)(Datum, Datum), Datum valuetypid, bool crossings)
{
	return sync_oper2_temporals_temporalseq(ts, seq, operator, valuetypid, crossings);
}

TemporalS *
sync_oper2_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum), Datum valuetypid, bool crossings)
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
			operator, valuetypid, crossings);
		if (seq != NULL)
		{
			sequences[k++] = seq;
		}
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
	Datum (*operator)(Datum, Datum), Datum valuetypid, bool crossings)
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
			operator, valuetypid, crossings);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			operator, valuetypid, crossings);
	
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
			operator, valuetypid, crossings);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper2_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			operator, valuetypid, crossings);
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
	if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
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
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				param);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			i++; j++;
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
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
		if (timestamp_cmp_internal(seq->period.upper, inst->t) == 0)
		{
			i++; j++;
		}
		else if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
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
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid, bool crossings)
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
	
	int n1 = temporalseq_find_timestamp(seq1, inter->lower);
	int n2 = temporalseq_find_timestamp(seq2, inter->lower);
	/* The lower bound of the intersection may be exclusive */
	if (n1 == -1) n1 = 0;
	if (n2 == -1) n2 = 0;
	int count = (seq1->count - n1 + seq2->count - n2) * 2;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	int i = n1, j = n2, k = 0, l = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, j);
	TemporalInst *prev1, *prev2, *next1, *next2;
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count * 2);
	Datum cross1, cross2, value;
	TimestampTz crosstime;
	while (i < seq1->count && j < seq2->count)
	{
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			/* If not the first instant compute the operator on the potential crossing 
			   before adding the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2, param);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				param);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (i == seq1->count-1 || j == seq2->count-1)
				break;
			next1 = temporalseq_inst_n(seq1, i+1);
			next2 = temporalseq_inst_n(seq2, j+1);
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
		{
			next1 = temporalseq_inst_n(seq1, i+1);
			inst1 = temporalseq_at_timestamp1(inst1, next1, inst2->t);
			tofree[l++] = inst1;
			/* If not the first instant add potential crossing before adding
			   the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2, param);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				param);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (j == seq2->count-1)
				break;
			next2 = temporalseq_inst_n(seq2, j+1);
		}
		else 
		{
			next2 = temporalseq_inst_n(seq2, j+1);
			inst2 = temporalseq_at_timestamp1(inst2, next2, inst1->t);
			tofree[l++] = inst2;
			/* If not the first instant add potential crossing before adding
			   the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2, param);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				param);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (i == seq1->count-1)
				break;
			next1 = temporalseq_inst_n(seq1, i+1);
		}
		if (timestamp_cmp_internal(next1->t, next2->t) < 0)
		{
			i++;
			inst1 = next1;
		}
		else if (timestamp_cmp_internal(next2->t, next1->t) < 0)
		{
			j++;
			inst2 = next2;
		}
		else
		{
			i++; j++;
			inst1 = next1;
			inst2 = next2;
		}
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
		inter->lower_inc, inter->upper_inc, false);
	
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(instants); pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper3_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid, bool crossings)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_oper3_temporalseq_temporalseq(seq1, seq, 
			param, operator, valuetypid, crossings);
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
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid, bool crossings)
{
	return sync_oper3_temporals_temporalseq(ts, seq, param, operator, valuetypid, crossings);
}

TemporalS *
sync_oper3_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid, bool crossings)
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
			param, operator, valuetypid, crossings);
		if (seq != NULL)
		{
			sequences[k++] = seq;
		}
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/
/* " function */

Temporal *
sync_oper3_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum param, Datum (*operator)(Datum, Datum, Datum), Datum valuetypid, bool crossings)
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
			param, operator, valuetypid, crossings);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			param, operator, valuetypid, crossings);
	
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
			param, operator, valuetypid, crossings);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper3_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			param, operator, valuetypid, crossings);
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
	if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
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
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			Datum value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				inst1->valuetypid, inst2->valuetypid);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			i++; j++;
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
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
		if (timestamp_cmp_internal(seq->period.upper, inst->t) == 0)
		{
			i++; j++;
		}
		else if (timestamp_cmp_internal(seq->period.upper, inst->t) < 0)
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
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid, bool crossings)
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
	
	int n1 = temporalseq_find_timestamp(seq1, inter->lower);
	int n2 = temporalseq_find_timestamp(seq2, inter->lower);
	/* The lower bound of the intersection may be exclusive */
	if (n1 == -1) n1 = 0;
	if (n2 == -1) n2 = 0;
	int count = (seq1->count - n1 + seq2->count - n2) * 2;
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	int i = n1, j = n2, k = 0, l = 0;
	TemporalInst *inst1 = temporalseq_inst_n(seq1, i);
	TemporalInst *inst2 = temporalseq_inst_n(seq2, j);
	TemporalInst *prev1, *prev2, *next1, *next2;
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count * 2);
	Datum cross1, cross2, value;
	TimestampTz crosstime;
	while (i < seq1->count && j < seq2->count)
	{
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			/* If not the first instant compute the operator on the potential crossing 
			   before adding the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2, seq1->valuetypid, seq2->valuetypid);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				inst1->valuetypid, inst2->valuetypid);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (i == seq1->count-1 || j == seq2->count-1)
				break;
			next1 = temporalseq_inst_n(seq1, i+1);
			next2 = temporalseq_inst_n(seq2, j+1);
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
		{
			next1 = temporalseq_inst_n(seq1, i+1);
			inst1 = temporalseq_at_timestamp1(inst1, next1, inst2->t);
			tofree[l++] = inst1;
			/* If not the first instant add potential crossing before adding
			   the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2, seq1->valuetypid, seq2->valuetypid);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				inst1->valuetypid, inst2->valuetypid);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (j == seq2->count-1)
				break;
			next2 = temporalseq_inst_n(seq2, j+1);
		}
		else 
		{
			next2 = temporalseq_inst_n(seq2, j+1);
			inst2 = temporalseq_at_timestamp1(inst2, next2, inst1->t);
			tofree[l++] = inst2;
			/* If not the first instant add potential crossing before adding
			   the new instants */
			if (crossings && k > 0 && 
				temporalseq_add_crossing_new(prev1, prev2, inst1, inst2,
					&cross1, &cross2, &crosstime))
			{
				value = operator(cross1, cross2, seq1->valuetypid, seq2->valuetypid);
				instants[k++] = temporalinst_make(value, crosstime, valuetypid);
				FREE_DATUM(cross1, seq1->valuetypid); FREE_DATUM(cross2, seq2->valuetypid);
				FREE_DATUM(value, valuetypid);
			}
			value = operator(temporalinst_value(inst1), temporalinst_value(inst2),
				inst1->valuetypid, inst2->valuetypid);
			instants[k++] = temporalinst_make(value, inst1->t, valuetypid);
			FREE_DATUM(value, valuetypid);
			prev1 = inst1; prev2 = inst2;
			if (i == seq1->count-1)
				break;
			next1 = temporalseq_inst_n(seq1, i+1);
		}
		if (timestamp_cmp_internal(next1->t, next2->t) < 0)
		{
			i++;
			inst1 = next1;
		}
		else if (timestamp_cmp_internal(next2->t, next1->t) < 0)
		{
			j++;
			inst2 = next2;
		}
		else
		{
			i++; j++;
			inst1 = next1;
			inst2 = next2;
		}
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
		inter->lower_inc, inter->upper_inc, false);
	
	for (int i = 0; i < l; i++) 
		pfree(tofree[i]);
	pfree(instants); pfree(tofree); pfree(inter);

	return result; 
}

/*****************************************************************************
 * TemporalS and <Type>
 *****************************************************************************/

TemporalS *
sync_oper4_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid, bool crossings)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *seq2 = sync_oper4_temporalseq_temporalseq(seq1, seq, 
			operator, valuetypid, crossings);
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
sync_oper4_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid, bool crossings)
{
	return sync_oper4_temporals_temporalseq(ts, seq, operator, valuetypid, crossings);
}

TemporalS *
sync_oper4_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid, bool crossings)
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
			operator, valuetypid, crossings);
		if (seq != NULL)
			sequences[k++] = seq;
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
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*****************************************************************************/
/* " function */

Temporal *
sync_oper4_temporal_temporal(Temporal *temp1, Temporal *temp2,
	Datum (*operator)(Datum, Datum, Oid, Oid), Datum valuetypid, bool crossings)
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
			operator, valuetypid, crossings);
	else if (temp1->type == TEMPORALSEQ && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporalseq_temporals(
			(TemporalSeq *)temp1, (TemporalS *)temp2,
			operator, valuetypid, crossings);
	
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
			operator, valuetypid, crossings);
	else if (temp1->type == TEMPORALS && temp2->type == TEMPORALS) 
		result = (Temporal *)sync_oper4_temporals_temporals(
			(TemporalS *)temp1, (TemporalS *)temp2,
			operator, valuetypid, crossings);
    else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Bad temporal type")));

	return result;
}

/*****************************************************************************
 * TemporalSeq and <Type>
 *****************************************************************************/

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
		Datum value1, value2;
		temporalseq_value_at_timestamp(seq1, inter->lower, &value1);
		temporalseq_value_at_timestamp(seq2, inter->lower, &value2);
		Datum value = operator(value1, value2);
		TemporalInst *inst = temporalinst_make(value, inter->lower, valuetypid);
		result[0] = temporalseq_from_temporalinstarr(&inst, 1, true, true, false);
		FREE_DATUM(value1, seq1->valuetypid); FREE_DATUM(value2, seq2->valuetypid);
		FREE_DATUM(value, valuetypid); pfree(inst);
		*count = 1;
		return result;
	}
	
	int n1 = temporalseq_find_timestamp(seq1, inter->lower);
	int n2 = temporalseq_find_timestamp(seq2, inter->lower);
	/* The lower bound of the intersection may be exclusive */
	if (n1 == -1) n1 = 0;
	if (n2 == -1) n2 = 0;
	int count1 = (seq1->count - n1 + seq2->count - n2);
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count1 * 3);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count1 * 2);
	int i = n1, j = n2, k = 0, l = 0;
	TemporalInst *start1 = temporalseq_inst_n(seq1, i);
	TemporalInst *start2 = temporalseq_inst_n(seq2, j);
	TemporalInst *next;
	if (timestamp_cmp_internal(start1->t, start2->t) < 0)
	{
		next = temporalseq_inst_n(seq1, i+1);
		start1 = temporalseq_at_timestamp1(start1, next, start2->t);
		tofree[l++] = start1;
	}
	else if (timestamp_cmp_internal(start1->t, start2->t) > 0)
	{
		next = temporalseq_inst_n(seq2, j+1);
		start2 = temporalseq_at_timestamp1(start2, next, start1->t);
		tofree[l++] = start2;
	}
	bool lower_inc = (timestamp_cmp_internal(start1->t, inter->lower) == 0) ? 
		inter->lower_inc : true;
	while (i < seq1->count-1 && j < seq2->count-1)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i+1);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j+1);
		if (timestamp_cmp_internal(end1->t, end2->t) > 0)
		{
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t);
			tofree[l++] = end1;
			j++;
		}
		else if (timestamp_cmp_internal(end1->t, end2->t) < 0)
		{
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t);
			tofree[l++] = end2;
			i++;
		}
		else
		{
			i++; j++;			
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		int countseq;
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
	
	int n1 = temporalseq_find_timestamp(seq1, inter->lower);
	int n2 = temporalseq_find_timestamp(seq2, inter->lower);
	/* The lower bound of the intersection may be exclusive */
	if (n1 == -1) n1 = 0;
	if (n2 == -1) n2 = 0;
	int count1 = (seq1->count - n1 + seq2->count - n2);
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count1 * 3);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count1 * 2);
	int i = n1, j = n2, k = 0, l = 0;
	TemporalInst *start1 = temporalseq_inst_n(seq1, i);
	TemporalInst *start2 = temporalseq_inst_n(seq2, j);
	TemporalInst *next;
	if (timestamp_cmp_internal(start1->t, start2->t) < 0)
	{
		next = temporalseq_inst_n(seq1, i+1);
		start1 = temporalseq_at_timestamp1(start1, next, start2->t);
		tofree[l++] = start1;
	}
	else if (timestamp_cmp_internal(start1->t, start2->t) > 0)
	{
		next = temporalseq_inst_n(seq2, j+1);
		start2 = temporalseq_at_timestamp1(start2, next, start1->t);
		tofree[l++] = start2;
	}
	bool lower_inc = (timestamp_cmp_internal(start1->t, inter->lower) == 0) ? 
		inter->lower_inc : true;
	while (i < seq1->count-1 && j < seq2->count-1)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i+1);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j+1);
		if (timestamp_cmp_internal(end1->t, end2->t) > 0)
		{
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t);
			tofree[l++] = end1;
			j++;
		}
		else if (timestamp_cmp_internal(end1->t, end2->t) < 0)
		{
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t);
			tofree[l++] = end2;
			i++;
		}
		else
		{
			i++; j++;			
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		int countseq;
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
	
	int n1 = temporalseq_find_timestamp(seq1, inter->lower);
	int n2 = temporalseq_find_timestamp(seq2, inter->lower);
	/* The lower bound of the intersection may be exclusive */
	if (n1 == -1) n1 = 0;
	if (n2 == -1) n2 = 0;
	int count1 = (seq1->count - n1 + seq2->count - n2);
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * count1 * 3);
	TemporalInst **tofree = palloc(sizeof(TemporalInst *) * count1 * 2);
	int i = n1, j = n2, k = 0, l = 0;
	TemporalInst *start1 = temporalseq_inst_n(seq1, i);
	TemporalInst *start2 = temporalseq_inst_n(seq2, j);
	TemporalInst *next;
	if (timestamp_cmp_internal(start1->t, start2->t) < 0)
	{
		next = temporalseq_inst_n(seq1, i+1);
		start1 = temporalseq_at_timestamp1(start1, next, start2->t);
		tofree[l++] = start1;
	}
	else if (timestamp_cmp_internal(start1->t, start2->t) > 0)
	{
		next = temporalseq_inst_n(seq2, j+1);
		start2 = temporalseq_at_timestamp1(start2, next, start1->t);
		tofree[l++] = start2;
	}
	bool lower_inc = (timestamp_cmp_internal(start1->t, inter->lower) == 0) ? 
		inter->lower_inc : true;
	while (i < seq1->count-1 && j < seq2->count-1)
	{
		TemporalInst *end1 = temporalseq_inst_n(seq1, i+1);
		TemporalInst *end2 = temporalseq_inst_n(seq2, j+1);
		if (timestamp_cmp_internal(end1->t, end2->t) > 0)
		{
			end1 = temporalseq_at_timestamp1(start1, end1, end2->t);
			tofree[l++] = end1;
			j++;
		}
		else if (timestamp_cmp_internal(end1->t, end2->t) < 0)
		{
			end2 = temporalseq_at_timestamp1(start2, end2, end1->t);
			tofree[l++] = end2;
			i++;
		}
		else
		{
			i++; j++;			
		}
		bool upper_inc = (timestamp_cmp_internal(end1->t, inter->upper) == 0) ? 
			inter->upper_inc : false;
		int countseq;
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
