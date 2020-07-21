/*****************************************************************************
 *
 * temporals.c
 *	  Basic functions for temporal sequence sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporals.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/lsyscache.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "temporal_util.h"
#include "oidcache.h"
#include "temporal_boxops.h"
#include "rangetypes_ext.h"

#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/* 
 * The memory structure of a TemporalS with, e.g., 2 sequences is as follows
 *
 *	--------------------------------------------------------
 *	( TemporalS )_ X | offset_0 | offset_1 | offset_2 | ...
 *	--------------------------------------------------------
 *	--------------------------------------------------------
 *	( TemporalSeq_0 )_X | ( TemporalSeq_1 )_X | ( bbox )_X | 
 *	--------------------------------------------------------
 *
 * where the X are unused bytes added for double padding, offset_0 and offset_1
 * are offsets for the corresponding sequences and offset_2 is the offset for the 
 * bounding box. There is no precomputed trajectory for TemporalS.
 */

/* N-th TemporalSeq of a TemporalS */

TemporalSeq *
temporals_seq_n(const TemporalS *ts, int index)
{
	return (TemporalSeq *)(
		(char *)(&ts->offsets[ts->count + 1]) + 	/* start of data */
			ts->offsets[index]);					/* offset */
}

/* Pointer to the bounding box of a TemporalS */

void *
temporals_bbox_ptr(const TemporalS *ts)
{
	return (char *)(&ts->offsets[ts->count + 1]) +  /* start of data */
		ts->offsets[ts->count];						/* offset */
}

/* Copy the bounding box of a TemporalS in the first argument */

void
temporals_bbox(void *box, const TemporalS *ts)
{
	void *box1 = temporals_bbox_ptr(ts);
	size_t bboxsize = temporal_bbox_size(ts->valuetypid);
	memcpy(box, box1, bboxsize);
}

/* Construct a TemporalS from an array of TemporalSeq 
 * The normalize argument determines whether the resulting value will be
 * normalized. In particular, normalize is false when synchronizing two 
 * TemporalS before applying an operation to them */

TemporalS *
temporals_make(TemporalSeq **sequences, int count, bool normalize)
{
	/* Test the validity of the sequences */
	assert(count > 0);
	bool isgeo = (sequences[0]->valuetypid == type_oid(T_GEOMETRY) ||
		sequences[0]->valuetypid == type_oid(T_GEOGRAPHY));
	for (int i = 1; i < count; i++)
	{
		if (sequences[i - 1]->period.upper > sequences[i]->period.lower ||
		   (sequences[i - 1]->period.upper == sequences[i]->period.lower &&
		   sequences[i - 1]->period.upper_inc && sequences[i]->period.lower_inc))
		{
			char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(sequences[i - 1]->period.upper));
			char *t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(sequences[i]->period.lower));
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
				errmsg("Timestamps for temporal value must be increasing: %s, %s", t1, t2)));
		}
		if (isgeo)
		{
			ensure_same_srid_tpoint((Temporal *)sequences[i - 1], (Temporal *)sequences[i]);
			ensure_same_dimensionality_tpoint((Temporal *)sequences[i - 1], (Temporal *)sequences[i]);
		}
	}

	TemporalSeq **newsequences = sequences;
	int newcount = count;
	if (normalize && count > 1)
		newsequences = temporalseqarr_normalize(sequences, count, &newcount);
	/* Add the size of the struct and the offset array 
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalS)) + newcount * sizeof(size_t);
	size_t memsize = 0;
	int totalcount = 0;
	for (int i = 0; i < newcount; i++)
	{
		totalcount += newsequences[i]->count;
		memsize += double_pad(VARSIZE(newsequences[i]));
	}
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(sequences[0]->valuetypid);
	memsize += double_pad(bboxsize);
	TemporalS *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;
	result->totalcount = totalcount;
	result->valuetypid = sequences[0]->valuetypid;
	result->duration = TEMPORALS;
	MOBDB_FLAGS_SET_LINEAR(result->flags,
		MOBDB_FLAGS_GET_LINEAR(sequences[0]->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(sequences[0]->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(sequences[0]->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;	
	for (int i = 0; i < newcount; i++)
	{
		memcpy(((char *) result) + pdata + pos, newsequences[i], VARSIZE(newsequences[i]));
		result->offsets[i] = pos;
		pos += double_pad(VARSIZE(newsequences[i]));
	}
	/*
	 * Precompute the bounding box 
	 * Only external types have precomputed bounding box, internal types such
	 * as double2, double3, or double4 do not have precomputed bounding box
	 */
	if (bboxsize != 0) 
	{
		void *bbox = ((char *) result) + pdata + pos;
		temporals_make_bbox(bbox, newsequences, newcount);
		result->offsets[newcount] = pos;
	}
	if (normalize && count > 1)
	{
		for (int i = 0; i < newcount; i++)
			pfree(newsequences[i]);
		pfree(newsequences);
	}
	return result;
}

/* Construct a TemporalS from a TemporalSeq */

TemporalS *
temporalseq_to_temporals(const TemporalSeq *seq)
{
	return temporals_make((TemporalSeq **)&seq, 1, false);
}

/* Consruct a TemporalS from a base value and a timestamp set */

TemporalS *
temporals_from_base_internal(Datum value, Oid valuetypid, const PeriodSet *ps, bool linear)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ps->count);
	for (int i = 0; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		sequences[i] = temporalseq_from_base_internal(value, valuetypid, p, linear);
	}
	TemporalS *result = temporals_make(sequences, ps->count, false);
	for (int i = 0; i < ps->count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

PG_FUNCTION_INFO_V1(temporals_from_base);

PGDLLEXPORT Datum
temporals_from_base(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	bool linear = PG_GETARG_BOOL(2);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TemporalS *result = temporals_from_base_internal(value, valuetypid, ps, linear);
	DATUM_FREE_IF_COPY(value, valuetypid, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_POINTER(result);
}

/* Append an TemporalInst to to the last sequence of a TemporalS */

TemporalS *
temporals_append_instant(const TemporalS *ts, const TemporalInst *inst)
{
	/* The validity tests are done in the temporalseq_append_instant function */
	TemporalSeq *seq = temporals_seq_n(ts, ts->count - 1);
	TemporalSeq *newseq = temporalseq_append_instant(seq, inst);
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalS)) + ts->count * sizeof(size_t);
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(ts->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < ts->count - 1; i++)
		memsize += double_pad(VARSIZE(temporals_seq_n(ts, i)));
	memsize += double_pad(VARSIZE(newseq));
	/* Create the TemporalS */
	TemporalS *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = ts->count;
	result->totalcount = ts->totalcount - seq->count + newseq->count;
	result->valuetypid = ts->valuetypid;
	result->duration = TEMPORALS;
	MOBDB_FLAGS_SET_LINEAR(result->flags, MOBDB_FLAGS_GET_LINEAR(ts->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (ts->valuetypid == type_oid(T_GEOMETRY) ||
		ts->valuetypid == type_oid(T_GEOGRAPHY))
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(ts->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(ts->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;	
	for (int i = 0; i < ts->count - 1; i++)
	{
		seq = temporals_seq_n(ts,i);
		memcpy(((char *) result) + pdata + pos, seq, VARSIZE(seq));
		result->offsets[i] = pos;
		pos += double_pad(VARSIZE(seq));
	}
	memcpy(((char *) result) + pdata + pos, newseq, VARSIZE(newseq));
	result->offsets[ts->count - 1] = pos;
	pos += double_pad(VARSIZE(newseq));
	/*
	 * Precompute the bounding box 
	 * Only external types have precomputed bounding box, internal types such
	 * as double2, double3, or double4 do not have precomputed bounding box
	 */
	if (bboxsize != 0) 
	{
		union bboxunion box;
		void *bbox = ((char *) result) + pdata + pos;
		memcpy(bbox, temporals_bbox_ptr(ts), bboxsize);
		temporalinst_make_bbox(&box, inst);
		temporal_bbox_expand(bbox, &box, ts->valuetypid);
		result->offsets[ts->count] = pos;
	}
	pfree(newseq);
	return result;
}

/* Merge two temporal values */

TemporalS *
temporals_merge(const TemporalS *ts1, const TemporalS *ts2)
{
	const TemporalS *seqsets[] = {ts1, ts2};
	return temporals_merge_array((TemporalS **) seqsets, 2);
}

/* Merge an array of temporal values */

TemporalS *
temporals_merge_array(TemporalS **seqsets, int count)
{
	/* Test the validity of the temporal values */
	int totalcount = seqsets[0]->count;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seqsets[0]->flags);
	Oid valuetypid = seqsets[0]->valuetypid;
	bool isgeo = (seqsets[0]->valuetypid == type_oid(T_GEOMETRY) ||
		seqsets[0]->valuetypid == type_oid(T_GEOGRAPHY));
	for (int i = 1; i < count; i++)
	{
		assert(valuetypid == seqsets[i]->valuetypid);
		assert(linear == MOBDB_FLAGS_GET_LINEAR(seqsets[i]->flags));
		if (isgeo)
		{
			assert(MOBDB_FLAGS_GET_GEODETIC(seqsets[0]->flags) ==
				MOBDB_FLAGS_GET_GEODETIC(seqsets[i]->flags));
			ensure_same_srid_tpoint((Temporal *)seqsets[0], (Temporal *)seqsets[i]);
			ensure_same_dimensionality_tpoint((Temporal *)seqsets[0], (Temporal *)seqsets[i]);
		}
		totalcount += seqsets[i]->count;
	}
	/* Collect the composing sequences */
	TemporalSeq **sequences = palloc0(sizeof(TemporalSeq *) * totalcount);
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < seqsets[i]->count; j++)
			sequences[k++] = temporals_seq_n(seqsets[i], j);
	}
	temporalseqarr_sort(sequences, totalcount);
	/* Test the validity of the composing sequences */
	TemporalSeq *seq1 = sequences[0];
	for (int i = 1; i < totalcount; i++)
	{
		TemporalInst *inst1 = temporalseq_inst_n(seq1, seq1->count - 1);
		TemporalSeq *seq2 = sequences[i];
		TemporalInst *inst2 = temporalseq_inst_n(seq2, 0);
		char *t1;
		if (inst1->t > inst2->t)
		{
			char *t2;
			t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
			t2 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst2->t));
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("The temporal values cannot overlap on time: %s, %s", t1, t2)));
		}
		if (inst1->t == inst2->t && seq1->period.upper_inc && seq2->period.lower_inc)
		{
			if (! datum_eq(temporalinst_value(inst1), temporalinst_value(inst2), inst1->valuetypid))
			{
				t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
				ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
					errmsg("The temporal values have different value at their overlapping instant %s", t1)));
			}
		}
		seq1 = seq2;
	}
	/* Create the result */
	int totalcount1;
	TemporalSeq **normseqs = temporalseqarr_normalize(sequences, totalcount, &totalcount1);
	TemporalS *result = temporals_make(normseqs, totalcount1, linear);
	for (int i = 0; i < totalcount1; i++)
		pfree(normseqs[i]);
	pfree(normseqs);
	pfree(sequences);
	return result;
}

/* Copy a TemporalS */
TemporalS *
temporals_copy(const TemporalS *ts)
{
	TemporalS *result = palloc0(VARSIZE(ts));
	memcpy(result, ts, VARSIZE(ts));
	return result;
}

/*****************************************************************************/

/*
 * Binary search of a timestamptz in a TemporalS or in an array of TemporalSeq.
 * If the timestamp is found, the position of the sequence is returned in pos.
 * Otherwise, return a number encoding whether it is before, between two 
 * sequences or after. For example, given 3 sequences, the result of the 
 * function if the value is not found will be as follows: 
 *				0			1			2
 *			|------|	|------|	|------|   
 * 1)	t^ 											=> result = 0
 * 2)				 t^ 							=> result = 1
 * 3)							 t^ 				=> result = 2
 * 4)										  t^	=> result = 3
 */

bool 
temporals_find_timestamp(const TemporalS *ts, TimestampTz t, int *pos)
{
	int first = 0, last = ts->count - 1;
	int middle = 0; /* make compiler quiet */
	TemporalSeq *seq = NULL; /* make compiler quiet */
	while (first <= last) 
	{
		middle = (first + last)/2;
		seq = temporals_seq_n(ts, middle);
		if (contains_period_timestamp_internal(&seq->period, t))
		{
			*pos = middle;
			return true;
		}
		if (t <= seq->period.lower)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (t >= seq->period.upper)
		middle++;
	*pos = middle;
	return false;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/* 
 * Intersection of a TemporalSeq and a TemporalInst values. 
 */
bool
intersection_temporals_temporalinst(const TemporalS *ts, const TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2)
{
	TemporalInst *inst1 = temporals_at_timestamp(ts, inst->t);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = temporalinst_copy(inst);
	return true;
}

bool
intersection_temporalinst_temporals(const TemporalInst *inst, const TemporalS *ts,
	TemporalInst **inter1, TemporalInst **inter2)
{
	return intersection_temporals_temporalinst(ts, inst, inter2, inter1);
}

/* 
 * Intersection of a TemporalS and a TemporalI values. Each value keeps the instants 
 * in the intersection of their time spans.
 */
bool
intersection_temporals_temporali(const TemporalS *ts, const TemporalI *ti,
	TemporalI **inter1, TemporalI **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts);
	temporali_period(&p2, ti);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;
	
	TemporalInst **instants1 = palloc(sizeof(TemporalInst *) * ti->count);
	TemporalInst **instants2 = palloc(sizeof(TemporalInst *) * ti->count);
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ti->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		TemporalInst *inst = temporali_inst_n(ti, j);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			instants1[k] = temporalseq_at_timestamp(seq, inst->t);
			instants2[k++] = inst;
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
		pfree(instants1); pfree(instants2); 
		return false;
	}
	
	*inter1 = temporali_make(instants1, k);
	*inter2 = temporali_make(instants2, k);
	for (i = 0; i < k; i++)
		pfree(instants1[i]);
	pfree(instants1); pfree(instants2); 
	return true;
}


bool
intersection_temporali_temporals(const TemporalI *ti, const TemporalS *ts,
	TemporalI **inter1, TemporalI **inter2)
{
	return intersection_temporals_temporali(ts, ti, inter2, inter1);
}

/* 
 * Intersection of a TemporalS and a TemporalSeq values. 
 */

bool
intersection_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	TemporalS **inter1, TemporalS **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporals_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;

	*inter1 = temporals_at_period(ts, &seq->period);

	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *interseq =	temporalseq_at_period(seq1, &seq->period);
		if (interseq != NULL)
			sequences[k++] = interseq;
		int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
		if (cmp < 0 ||
			(cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
			break;
	}
	if (k == 0)
	{
		pfree(sequences); 
		return false;
	}
	
	*inter2 = temporals_make(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return true;
}

bool
intersection_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	TemporalS **inter1, TemporalS **inter2)
{
	return intersection_temporals_temporalseq(ts, seq, inter2, inter1);
}

/* 
 * Intersection two TemporalS values. 
 */

bool
intersection_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	TemporalS **inter1, TemporalS **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;
	
	TemporalSeq **sequences1 = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	TemporalSeq **sequences2 = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		TemporalSeq *interseq1, *interseq2;
		if (intersection_temporalseq_temporalseq(seq1, seq2, 
			&interseq1, &interseq2))
		{
			sequences1[k] = interseq1;
			sequences2[k++] = interseq2;
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
		pfree(sequences1); pfree(sequences2); 
		return false;
	}
	
	*inter1 = temporals_make(sequences1, k, false);
	*inter2 = temporals_make(sequences2, k, false);
	for (i = 0; i < k; i++)
	{
		pfree(sequences1[i]); pfree(sequences2[i]);
	}
	pfree(sequences1); pfree(sequences2); 
	return true;
}

/*****************************************************************************
 * Synchronize functions
 *****************************************************************************/

/* 
 * Synchronize a TemporalS and a TemporalSeq values. The values are split into
 * (redundant) segments defined over the same set of sequences covering the 
 * intersection of their time spans.
 */

bool
synchronize_temporals_temporalseq(const TemporalS *ts, const TemporalSeq *seq,
	TemporalS **sync1, TemporalS **sync2, bool crossings)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	temporals_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	int n;
	temporals_find_timestamp(ts, seq->period.lower, &n);
	/* We are sure that n < ts->count due to the bounding period test above */
	TemporalSeq **sequences1 = palloc(sizeof(TemporalSeq *) * ts->count - n);
	TemporalSeq **sequences2 = palloc(sizeof(TemporalSeq *) * ts->count - n);
	int k = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts, i);
		TemporalSeq *syncseq1, *syncseq2;
		if (synchronize_temporalseq_temporalseq(seq, seq1, &syncseq1, &syncseq2, crossings))
		{
			sequences1[k] = syncseq1;
			sequences2[k++] = syncseq2;
		}
		int cmp = timestamp_cmp_internal(seq->period.upper, seq1->period.upper);
		if (cmp < 0 ||
			(cmp == 0 && (!seq->period.upper_inc || seq1->period.upper_inc)))
			break;
	}
	if (k == 0)
	{
		pfree(sequences1); pfree(sequences2); 
		return false;
	}
	
	*sync1 = temporals_make(sequences1, k, false);
	*sync2 = temporals_make(sequences2, k, false);
	for (int i = 0; i < k; i++) 
	{
		pfree(sequences1[i]); pfree(sequences2[i]);
	}
	pfree(sequences1); pfree(sequences2); 
	return true;
}

bool
synchronize_temporalseq_temporals(const TemporalSeq *seq, const TemporalS *ts,
	TemporalS **sync1, TemporalS **sync2, bool crossings)
{
	return synchronize_temporals_temporalseq(ts, seq, sync2, sync1, crossings);
}

/* 
 * Synchronize two TemporalS values. The values are split into (redundant)
 * segments defined over the same set of sequences covering the intersection of
 * their time spans. If crossings is true, then the crossings are also added.
 */

bool
synchronize_temporals_temporals(const TemporalS *ts1, const TemporalS *ts2,
	TemporalS **sync1, TemporalS **sync2, bool crossings)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporals_period(&p1, ts1);
	temporals_period(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;
	
	/* Previously it was Max(ts1->count, ts2->count) and was not correct */
	TemporalSeq **sequences1 = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	TemporalSeq **sequences2 = palloc(sizeof(TemporalSeq *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		TemporalSeq *syncseq1, *syncseq2;
		if (synchronize_temporalseq_temporalseq(seq1, seq2, 
			&syncseq1, &syncseq2, crossings))
		{
			sequences1[k] = syncseq1;
			sequences2[k++] = syncseq2;
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
		pfree(sequences1); pfree(sequences2); 
		return false;
	}
	
	*sync1 = temporals_make(sequences1, k, false);
	*sync2 = temporals_make(sequences2, k, false);
	for (i = 0; i < k; i++)
	{
		pfree(sequences1[i]); pfree(sequences2[i]);
	}
	pfree(sequences1); pfree(sequences2); 
	return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Convert to string */
 
char *
temporals_to_string(const TemporalS *ts, char *(*value_out)(Oid, Datum))
{
	char **strings = palloc(sizeof(char *) * ts->count);
	size_t outlen = 0;
	char str[20];
	if (linear_interpolation(ts->valuetypid) && 
		! MOBDB_FLAGS_GET_LINEAR(ts->flags))
		sprintf(str, "Interp=Stepwise;");
	else
		str[0] = '\0';
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		strings[i] = temporalseq_to_string(seq, true, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	char *result = palloc(strlen(str) + outlen + 3);
	result[outlen] = '\0';
	size_t pos = 0;
	strcpy(result, str);
	pos += strlen(str);
	result[pos++] = '{';
	for (int i = 0; i < ts->count; i++)
	{
		strcpy(result + pos, strings[i]);
		pos += strlen(strings[i]);
		result[pos++] = ',';
		result[pos++] = ' ';
		pfree(strings[i]);
	}
	result[pos - 2] = '}';
	result[pos - 1] = '\0';
	pfree(strings);
	return result;
}

/* Send function */

void
temporals_write(const TemporalS *ts, StringInfo buf)
{
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(buf, (uint32) ts->count, 4);
#else
	pq_sendint32(buf, ts->count);
#endif
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		temporalseq_write(seq, buf);
	}
}
 
/* Receive function */

TemporalS *
temporals_read(StringInfo buf, Oid valuetypid)
{
	int count = (int) pq_getmsgint(buf, 4);
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++)
		sequences[i] = temporalseq_read(buf, valuetypid);
	TemporalS *result = temporals_make(sequences, count, false);

	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);	
	
	return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/* Cast a temporal integer value as a temporal float value */

TemporalS *
tints_to_tfloats(const TemporalS *ts)
{
	/* It is not necessary to set the linear flag to false since it is already
	 * set by the fact that the input argument is a temporal integer */
	TemporalS *result = temporals_copy(ts);
	result->valuetypid = FLOAT8OID;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(result, i);
		seq->valuetypid = FLOAT8OID;
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			inst->valuetypid = FLOAT8OID;
			Datum *value_ptr = temporalinst_value_ptr(inst);
			*value_ptr = Float8GetDatum((double)DatumGetInt32(temporalinst_value(inst)));
		}
	}
	return result;
}

/* Cast a temporal float with step interpolation as a temporal integer */

TemporalS *
tfloats_to_tints(const TemporalS *ts)
{
	if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Cannot cast temporal float with linear interpolation to temporal integer")));
	/* It is not necessary to set the linear flag to false since it is already
	 * set by the fact that the input argument has step interpolation */
	TemporalS *result = temporals_copy(ts);
	result->valuetypid = INT4OID;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(result, i);
		seq->valuetypid = INT4OID;
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			inst->valuetypid = INT4OID;
			Datum *value_ptr = temporalinst_value_ptr(inst);
			*value_ptr = Int32GetDatum((double)DatumGetFloat8(temporalinst_value(inst)));
		}
	}
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalS *
temporalinst_to_temporals(const TemporalInst *inst, bool linear)
{
	TemporalSeq *seq = temporalseq_make((TemporalInst **)&inst, 1, true, true,
		linear, false);
	TemporalS *result = temporalseq_to_temporals(seq);
	pfree(seq);
	return result;
}

TemporalS *
temporali_to_temporals(const TemporalI *ti, bool linear)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		sequences[i] = temporalseq_make(&inst, 1, 
			true, true, linear, false);
	}
	TemporalS *result = temporals_make(sequences, ti->count, false);
	pfree(sequences);
	return result;
}

/* Transform a temporal value with continuous base type from step to linear interpolation */

TemporalS *
tsteps_to_linear(const TemporalS *ts)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tstepseq_to_linear(temporals_seq_n(ts, 0));

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tstepseq_to_linear1(&sequences[k], seq);
	}
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* Values of a TemporalS with step interpolation */

Datum *
temporals_values1(const TemporalS *ts, int *count)
{
	Datum *result = palloc(sizeof(Datum *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
			result[k++] = temporalinst_value(temporalseq_inst_n(seq, j));
	}
	datumarr_sort(result, k, ts->valuetypid);
	*count = datumarr_remove_duplicates(result, k, ts->valuetypid);
	return result;
}

ArrayType *
temporals_values(const TemporalS *ts)
{
	int count;
	Datum *values = temporals_values1(ts, &count);
	ArrayType *result = datumarr_to_array(values, count, ts->valuetypid);
	pfree(values);
	return result;
}

/* Ranges of a TemporalS float */

ArrayType *
tfloats_ranges(const TemporalS *ts)
{
	int count = MOBDB_FLAGS_GET_LINEAR(ts->flags) ? ts->count : ts->totalcount;
	RangeType **ranges = palloc(sizeof(RangeType *) * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++) 
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tfloatseq_ranges1(&ranges[k], seq);
	}
	int count1 = k;
	RangeType **normranges = rangearr_normalize(ranges, &count1);
	rangearr_sort(normranges, count1);
	ArrayType *result = rangearr_to_array(normranges, count1, 
		type_oid(T_FLOATRANGE));

	for (int i = 0; i < k; i++)
		pfree(ranges[i]);
	pfree(ranges);
	for (int i = 0; i < count1; i++)
		pfree(normranges[i]);
	pfree(normranges);
	
	return result;
}

/* Minimum instant without taking into account whether the instant is at an
 * exclusive bound or not. Needed for computing e.g. shortest line.
 * It returns an pointer to the instant NOT a new instant */

TemporalInst *
temporals_min_instant(const TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	TemporalInst *result = temporalseq_inst_n(seq, 0);
	Datum min = temporalinst_value(result);
	for (int i = 0; i < ts->count; i++)
	{
		seq = temporals_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			Datum value = temporalinst_value(inst);
			if (datum_lt(value, min, seq->valuetypid))
			{
				min = value;
				result = inst;
			}
		}
	}
	return result;
}

/* Minimum value */

Datum
temporals_min_value(const TemporalS *ts)
{
	Oid valuetypid = ts->valuetypid;
	if (valuetypid == INT4OID)
	{
		TBOX *box = temporals_bbox_ptr(ts);
		return Int32GetDatum((int)(box->xmin));
	}
	if (valuetypid == FLOAT8OID)
	{
		TBOX *box = temporals_bbox_ptr(ts);
		return Float8GetDatum(box->xmin);
	}
	Datum result = temporalseq_min_value(temporals_seq_n(ts, 0));
	for (int i = 1; i < ts->count; i++)
	{
		Datum value = temporalseq_min_value(temporals_seq_n(ts, i));
		if (datum_lt(value, result, valuetypid))
			result = value;
	}
	return result;
}

/* Maximum value */

Datum
temporals_max_value(const TemporalS *ts)
{
	Oid valuetypid = ts->valuetypid;
	if (valuetypid == INT4OID)
	{
		TBOX *box = temporals_bbox_ptr(ts);
		return Int32GetDatum((int)(box->xmax));
	}
	if (valuetypid == FLOAT8OID)
	{
		TBOX *box = temporals_bbox_ptr(ts);
		return Float8GetDatum(box->xmax);
	}
	Datum result = temporalseq_max_value(temporals_seq_n(ts, 0));
	for (int i = 1; i < ts->count; i++)
	{
		Datum value = temporalseq_max_value(temporals_seq_n(ts, i));
		if (datum_gt(value, result, valuetypid))
			result = value;
	}
	return result;
}

/* Get time */

PeriodSet *
temporals_get_time(const TemporalS *ts)
{
	Period **periods = palloc(sizeof(Period *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		periods[i] = &seq->period;
	}
	PeriodSet *result = periodset_make_internal(periods, ts->count, false);
	pfree(periods);
	return result;
}

/* Timespan */

Datum
temporals_timespan(const TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	Datum result = call_function2(timestamp_mi, 
		TimestampTzGetDatum(seq->period.upper), TimestampTzGetDatum(seq->period.lower));
	for (int i = 1; i < ts->count; i++)
	{
		seq = temporals_seq_n(ts, i);
		Datum interval1 = call_function2(timestamp_mi, 
			TimestampTzGetDatum(seq->period.upper), TimestampTzGetDatum(seq->period.lower));
		Datum interval2 = call_function2(interval_pl, result, interval1);
		pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
		result = interval2;
	}
	return result;
}

/* Interval of the TemporalS as a double */

double
temporals_interval_double(const TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		result += (double) (seq->period.upper - seq->period.lower);
	}
	return result;
}

/* Bounding period on which the temporal value is defined */

void
temporals_period(Period *p, const TemporalS *ts)
{
	TemporalSeq *start = temporals_seq_n(ts, 0);
	TemporalSeq *end = temporals_seq_n(ts, ts->count - 1);
	period_set(p, start->period.lower, end->period.upper, 
		start->period.lower_inc, end->period.upper_inc);
}

/* Sequences */

TemporalSeq **
temporals_sequences(const TemporalS *ts)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++) 
		result[i] = temporals_seq_n(ts, i);
	return result;
}

ArrayType *
temporals_sequences_array(const TemporalS *ts)
{
	TemporalSeq **sequences = temporals_sequences(ts);
	ArrayType *result = temporalarr_to_array((Temporal **)sequences, ts->count);
	pfree(sequences);
	return result;
}

/* Number of distinct instants */

int
temporals_num_instants(const TemporalS *ts)
{
	TemporalInst *lastinst;
	bool first = true;
	int result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		result += seq->count;
		if (!first)
		{
			if (temporalinst_eq(lastinst, temporalseq_inst_n(seq, 0)))
				result --;
		}
		lastinst = temporalseq_inst_n(seq, seq->count - 1);
		first = false;
	}
	return result;
}

/* N-th distinct instant */

TemporalInst *
temporals_instant_n(const TemporalS *ts, int n)
{
	assert (n >= 1 && n <= ts->totalcount);
	if (n == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		return temporalseq_inst_n(seq, 0);
	}
	
	/* Continue the search 0-based */
	n--;
	TemporalInst *prev, *next;
	bool first = true, found = false;
	int i = 0, count = 0, prevcount = 0;
	while (i < ts->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		count += seq->count;
		if (!first && temporalinst_eq(prev, temporalseq_inst_n(seq, 0)))
		{
				prevcount --;
				count --;
		}
		if (prevcount <= n && n < count)
		{
			next = temporalseq_inst_n(seq, n - prevcount);
			found = true;
			break;
		}
		prevcount = count;
		prev = temporalseq_inst_n(seq, seq->count - 1);
		first = false;
		i++;
	}
	if (!found) 
		return NULL;
	return next;
}

ArrayType *
temporals_instants_array(const TemporalS *ts)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
			instants[k++] = temporalseq_inst_n(seq, j);
	}
	int count = temporalinstarr_remove_duplicates(instants, k);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, count);
	pfree(instants);
	return result;
}

/* Start timestamptz */

TimestampTz
temporals_start_timestamp(const TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	return seq->period.lower;
}

/* End timestamptz */

TimestampTz
temporals_end_timestamp(const TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, ts->count - 1);
	return seq->period.upper;
}

/* Number of distinct timestamps */

int
temporals_num_timestamps(const TemporalS *ts)
{
	TimestampTz lasttime;
	bool first = true;
	int result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		result += seq->count;
		if (!first)
		{
			if (lasttime == temporalseq_inst_n(seq, 0)->t)
				result --;
		}
		lasttime = temporalseq_inst_n(seq, seq->count - 1)->t;
		first = false;
	}
	return result;
}

/* N-th distinct timestamp */

bool
temporals_timestamp_n(const TemporalS *ts, int n, TimestampTz *result)
{
	bool found = false;
	if (n < 1)
		return false;
	if (n == 1)
	{
		TemporalSeq *seq = temporals_seq_n(ts, 0);
		*result = temporalseq_inst_n(seq, 0)->t;
		return true ;
	}
	
	/* Continue the search 0-based */
	n--;
	TimestampTz prev, next;
	bool first = true;
	int i = 0, count = 0, prevcount = 0;
	while (i < ts->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		count += seq->count;
		if (!first && prev == temporalseq_inst_n(seq, 0)->t)
		{
				prevcount --;
				count --;
		}
		if (prevcount <= n && n < count)
		{
			next = temporalseq_inst_n(seq, n - prevcount)->t;
			found = true;
			break;
		}
		prevcount = count;
		prev = temporalseq_inst_n(seq, seq->count - 1)->t;
		first = false;
		i++;
	}
	if (!found) 
		return false;
	*result = next;
	return true;
}

/* Distinct timestamps */

TimestampTz *
temporals_timestamps1(const TemporalS *ts, int *count)
{
	TimestampTz **times = palloc(sizeof(TimestampTz *) * ts->count);
	int *counttimes = palloc0(sizeof(int) * ts->count);
	int totaltimes = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		times[i] = temporalseq_timestamps1(seq);
		counttimes[i] = seq->count;
		totaltimes += seq->count;
	}
	TimestampTz *result = palloc(sizeof(TimestampTz) * totaltimes);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < counttimes[i]; j ++)
			result[k++] = times[i][j];
		pfree(times[i]);
	}
	timestamparr_sort(result, totaltimes);
	totaltimes = timestamparr_remove_duplicates(result, totaltimes);
	
	pfree(times); pfree(counttimes);
	
	*count = totaltimes;
	return result;
}

ArrayType *
temporals_timestamps(const TemporalS *ts)
{
	int count;
	TimestampTz *times = temporals_timestamps1(ts, &count);
	ArrayType *result = timestamparr_to_array(times, count);
	pfree(times);
	return result;
}

/* Shift the time span of a temporal value by an interval */

TemporalS *
temporals_shift(const TemporalS *ts, const Interval *interval)
{
	TemporalS *result = temporals_copy(ts);
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ts->totalcount);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = sequences[i] = temporals_seq_n(result, i);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = instants[j] = temporalseq_inst_n(seq, j);
			inst->t = DatumGetTimestampTz(
				DirectFunctionCall2(timestamptz_pl_interval,
				TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
		}
		/* Shift period */
		seq->period.lower = DatumGetTimestampTz(
				DirectFunctionCall2(timestamptz_pl_interval,
				TimestampTzGetDatum(seq->period.lower), PointerGetDatum(interval)));
		seq->period.upper = DatumGetTimestampTz(
				DirectFunctionCall2(timestamptz_pl_interval,
				TimestampTzGetDatum(seq->period.upper), PointerGetDatum(interval)));
		/* Shift bounding box */
		void *bbox = temporalseq_bbox_ptr(seq); 
		temporal_bbox_shift(bbox, interval, seq->valuetypid);
	
	}
	/* Shift bounding box */
	void *bbox = temporals_bbox_ptr(result); 
	temporal_bbox_shift(bbox, interval, ts->valuetypid);
	pfree(sequences);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

/* Is the temporal value ever equal to the value? */

bool
temporals_ever_eq(const TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporals_bbox(&box, ts);
		double d = datum_double(value, ts->valuetypid);
		if (d < box.xmin || box.xmax < d)
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
		if (temporalseq_ever_eq(temporals_seq_n(ts, i), value))
			return true;
	return false;
}

/* Is the temporal value always equal to the value? */

bool
temporals_always_eq(const TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporals_bbox(&box, ts);
		if (ts->valuetypid == INT4OID)
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetInt32(value);
		else
			return box.xmin == box.xmax &&
				box.xmax == DatumGetFloat8(value);
	}

	for (int i = 0; i < ts->count; i++) 
		if (!temporalseq_always_eq(temporals_seq_n(ts, i), value))
			return false;
	return true;
}

/*****************************************************************************/

/* Is the temporal value ever less than to the value? */

bool
temporals_ever_lt(const TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporals_bbox(&box, ts);
		double d = datum_double(value, ts->valuetypid);
		/* Maximum value may be non inclusive */ 
		if (d < box.xmin)
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (temporalseq_ever_lt(seq, value))
			return true;
	}
	return false;
}

/* Is the temporal value ever less than or equal to the value? */

bool
temporals_ever_le(const TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporals_bbox(&box, ts);
		double d = datum_double(value, ts->valuetypid);
		if (d < box.xmin)
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (temporalseq_ever_le(seq, value))
			return true;
	}
	return false;
}

/* Is the temporal value always less than the value? */

bool
temporals_always_lt(const TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporals_bbox(&box, ts);
		double d = datum_double(value, ts->valuetypid);
		if (d < box.xmax)
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (! temporalseq_always_lt(seq, value))
			return false;
	}
	return true;
}

/* Is the temporal value always less than or equal to the value? */

bool
temporals_always_le(const TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporals_bbox(&box, ts);
		double d = datum_double(value, ts->valuetypid);
		if (d < box.xmax)
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (! temporalseq_always_le(seq, value))
			return false;
	}
	return true;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

TemporalS *
temporals_at_value(const TemporalS *ts, Datum value)
{
	Oid valuetypid = ts->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		temporals_bbox(&box1, ts);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
			return NULL;
	}

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_at_value(temporals_seq_n(ts, 0), value);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += temporalseq_at_value2(&sequences[k], seq, value);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* Restriction to the complement of a value */

TemporalS *
temporals_minus_value(const TemporalS *ts, Datum value)
{
	Oid valuetypid = ts->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		temporals_bbox(&box1, ts);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
			return temporals_copy(ts);
	}

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_value(temporals_seq_n(ts, 0), value);

	/* General case */
	int count;
	if (! MOBDB_FLAGS_GET_LINEAR(ts->flags))
		count = ts->totalcount;
	else 
		count = ts->totalcount * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += temporalseq_minus_value2(&sequences[k], seq, value);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/* 
 * Restriction to an array of values.
 * The function assumes that there are no duplicates values.
 */
TemporalS *
temporals_at_values(const TemporalS *ts, const Datum *values, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_at_values(temporals_seq_n(ts, 0), values, count);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += temporalseq_at_values1(&sequences[k], seq, values, count);
	}
	if (k == 0) 
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of an array of values.
 * The function assumes that there are no duplicates values.
 */
TemporalS *
temporals_minus_values(const TemporalS *ts, const Datum *values, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_values(temporals_seq_n(ts, 0), values, count);

	/* General case */
	int maxcount;
	if (! MOBDB_FLAGS_GET_LINEAR(ts->flags))
		maxcount = ts->totalcount * count;
	else 
		maxcount = ts->totalcount * count *2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);	
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += temporalseq_minus_values1(&sequences[k], seq, values, count);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to a range.
 */
TemporalS *
tnumbers_at_range(const TemporalS *ts, RangeType *range)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporals_bbox(&box1, ts);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
		return NULL;

	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_at_range(temporals_seq_n(ts, 0), range);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tnumberseq_at_range2(&sequences[k], seq, range);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of range.
 */
TemporalS *
tnumbers_minus_range(const TemporalS *ts, RangeType *range)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporals_bbox(&box1, ts);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
		return temporals_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_minus_range(temporals_seq_n(ts, 0), range);

	/* General case */
	int maxcount;
	if (! MOBDB_FLAGS_GET_LINEAR(ts->flags))
		maxcount = ts->totalcount;
	else 
		maxcount = ts->totalcount * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tnumberseq_minus_range1(&sequences[k], seq, range);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/* 
 * Restriction to an array of ranges
 * The function assumes that the ranges are normalized.
 */
TemporalS *
tnumbers_at_ranges(const TemporalS *ts, RangeType **ranges, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_at_ranges(temporals_seq_n(ts, 0), ranges, count);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tnumberseq_at_ranges1(&sequences[k], seq, ranges, count);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*
 * Restriction to the complement of an array of ranges.
 * The function assumes that the ranges are normalized.
 */
TemporalS *
tnumbers_minus_ranges(const TemporalS *ts, RangeType **ranges, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_minus_ranges(temporals_seq_n(ts, 0), ranges, count);

	/* General case */
	int maxcount;
	if (! MOBDB_FLAGS_GET_LINEAR(ts->flags))
		maxcount = ts->totalcount;
	else 
		maxcount = ts->totalcount * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		k += tnumberseq_minus_ranges1(&sequences[k], seq, ranges, count);
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/* Restriction to the minimum value */

TemporalS *
temporals_at_min(const TemporalS *ts)
{
	/* General case */
	Datum min = temporals_min_value(ts);
	return temporals_at_value(ts, min);
}

/* Restriction to the complement of the minimum value */

TemporalS *
temporals_minus_min(const TemporalS *ts)
{
	Datum min = temporals_min_value(ts);
	return temporals_minus_value(ts, min);
}

/* Restriction to the maximum value */
 
TemporalS *
temporals_at_max(const TemporalS *ts)
{
	Datum max = temporals_max_value(ts);
	return temporals_at_value(ts, max);
}

/* Restriction to the complement of the maximum value */

TemporalS *
temporals_minus_max(const TemporalS *ts)
{
	Datum max = temporals_max_value(ts);
	return temporals_minus_value(ts, max);
}

/*
 * Restriction to a timestamp.
 */
TemporalInst *
temporals_at_timestamp(const TemporalS *ts, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporals_period(&p, ts);
	if (!contains_period_timestamp_internal(&p, t))
		return NULL;

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_at_timestamp(temporals_seq_n(ts, 0), t);

	/* General case */
	int n;
	if (!temporals_find_timestamp(ts, t, &n))
		return NULL;
	TemporalSeq *seq = temporals_seq_n(ts, n);
	return temporalseq_at_timestamp(seq, t);
}

/*
 * Restriction to the complement of a timestamp.
 */
TemporalS *
temporals_minus_timestamp(const TemporalS *ts, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporals_period(&p, ts);
	if (!contains_period_timestamp_internal(&p, t))
		return temporals_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_timestamp(temporals_seq_n(ts, 0), t);

	/* General case 
	 * At most one composing sequence can be split into two */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count + 1));
	int k = 0;
	int i;
	for (i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		int count = temporalseq_minus_timestamp1(&sequences[k], seq, t);
		k += count;
		if (t < seq->period.upper)
		{
			i++;
		 	break;
		}
	}
	/* Copy the remaining sequences if went out of the for loop with the break */
	for (int j = i; j < ts->count; j++)
		sequences[k++] = temporalseq_copy(temporals_seq_n(ts, j));
	/* k is never equal to 0 since in that case it is a singleton sequence set 
	   and it has been dealt by temporalseq_minus_timestamp above */
	TemporalS *result = temporals_make(sequences, k, false);
	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*
 * Value at a timestamp.
 * This function assumes a bounding box test has been done before.
 */
bool
temporals_value_at_timestamp(const TemporalS *ts, TimestampTz t, Datum *result)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_value_at_timestamp(temporals_seq_n(ts, 0), t, result);

	/* General case */
	int n;
	if (!temporals_find_timestamp(ts, t, &n))
		return false;	
	return temporalseq_value_at_timestamp(temporals_seq_n(ts, n), t, result);
}

/*
 * Restriction to a timestampset.
 */
TemporalI *
temporals_at_timestampset(const TemporalS *ts1, const TimestampSet *ts2)
{
	/* Bounding box test */
	Period p1;
	temporals_period(&p1, ts1);
	Period *p2 = timestampset_bbox(ts2);
	if (!overlaps_period_period_internal(&p1, p2))
		return NULL;

	/* Singleton sequence set */
	if (ts1->count == 1)
		return temporalseq_at_timestampset(temporals_seq_n(ts1, 0), ts2);

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ts2->count);
	int count = 0;
	int i = 0, j = 0;
	while (i < ts2->count && j < ts1->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts1, j);
		TimestampTz t = timestampset_time_n(ts2, i);
		if (contains_period_timestamp_internal(&seq->period, t))
		{
			instants[count++] = temporalseq_at_timestamp(seq, t);
			i++;
		}
		else
		{
			if (t <= seq->period.lower)
				i++;
			if (t >= seq->period.upper)
				j++;
		}
	}
	if (count == 0)
	{
		pfree(instants);
		return NULL;
	}

	TemporalI *result = temporali_make(instants, count);
	for (i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of a timestampset.
 */
TemporalS *
temporals_minus_timestampset(const TemporalS *ts1, const TimestampSet *ts2)
{
	/* Bounding box test */
	Period p1;
	temporals_period(&p1, ts1);
	Period *p2 = timestampset_bbox(ts2);
	if (!overlaps_period_period_internal(&p1, p2))
		return temporals_copy(ts1);

	/* Singleton sequence set */
	if (ts1->count == 1)
		return temporalseq_minus_timestampset(temporals_seq_n(ts1, 0), ts2);

	/* General case */
	/* Each timestamp will split at most one composing sequence into two */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts1->count + ts2->count + 1));
	int k = 0;
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts1, i);
		int count = temporalseq_minus_timestampset1(&sequences[k], seq, ts2);
		k += count;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_make(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*
 * Restriction to a period.
 */
TemporalS *
temporals_at_period(const TemporalS *ts, const Period *p)
{
	/* Bounding box test */
	Period p1;
	temporals_period(&p1, ts);
	if (!overlaps_period_period_internal(&p1, p))
		return NULL;

	/* Singleton sequence set */
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporalseq_at_period(temporals_seq_n(ts, 0), p);
		return temporalseq_to_temporals(seq);
	}

	/* General case */
	int n;
	temporals_find_timestamp(ts, p->lower, &n);
	/* We are sure that n < ts->count because of the bounding period test above */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count - n));
	TemporalSeq *tofree[2];
	int k = 0, l = 0;
	for (int i = n; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (contains_period_period_internal(p, &seq->period))
				sequences[k++] = seq;
		else if (overlaps_period_period_internal(p, &seq->period))
		{
			TemporalSeq *newseq = temporalseq_at_period(seq, p);
			sequences[k++] = tofree[l++] = newseq;
		}
		int cmp = timestamp_cmp_internal(p->upper, seq->period.upper);
		if (cmp < 0 || (cmp == 0 && seq->period.upper_inc))
			break;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;		
	}
	/* Since both the temporals and the period are normalized it is not 
	   necessary to normalize the result of the projection */	
	TemporalS *result = temporals_make(sequences, k, false);
	for (int i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of a period.
 */
TemporalS *
temporals_minus_period(const TemporalS *ts, const Period *p)
{
	/* Bounding box test */
	Period p1;
	temporals_period(&p1, ts);
	if (!overlaps_period_period_internal(&p1, p))
		return temporals_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_period(temporals_seq_n(ts, 0), p);

	/* General case */
	PeriodSet *ps = temporals_get_time(ts);
	PeriodSet *resultps = minus_periodset_period_internal(ps, p);
	TemporalS *result = NULL;
	if (resultps != NULL)
		result = temporals_at_periodset(ts, resultps);

	pfree(ps); pfree(resultps);

	return result;
}

/*
 * Restriction to a periodset.
 */

TemporalS *
temporals_at_periodset(const TemporalS *ts, const PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporals_period(&p1, ts);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return NULL;

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_at_periodset(temporals_seq_n(ts, 0), ps);

	/* General case */
	TimestampTz t = Max(p1.lower, p2->lower);
	int n1, n2;
	temporals_find_timestamp(ts, t, &n1);
	periodset_find_timestamp(ps, t, &n2);
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count + ps->count - n1 - n2));
	int i = n1, j = n2, k = 0;
	while (i < ts->count && j < ps->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		Period *p = periodset_per_n(ps, j);
		TemporalSeq *seq1 = temporalseq_at_period(seq, p);
		if (seq1 != NULL)
			sequences[k++] = seq1;
		int cmp = timestamp_cmp_internal(seq->period.upper, p->upper);
		if (cmp == 0 && seq->period.upper_inc == p->upper_inc)
		{
			i++; j++;
		}
		else if (cmp < 0 || (cmp == 0 && ! seq->period.upper_inc && p->upper_inc))
			i++;
		else 
			j++;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Since both the temporals and the periodset are normalized it is not 
	   necessary to normalize the result of the projection */
	TemporalS *result = temporals_make(sequences, k, false);
	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*
 * Restriction to the complement of a period set.
 */

TemporalS *
temporals_minus_periodset(const TemporalS *ts, const PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporals_period(&p1, ts);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return temporals_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_periodset(temporals_seq_n(ts, 0), ps);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count + ps->count));
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ps->count)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		p2 = periodset_per_n(ps, j);
		/* The sequence and the period do not overlap */
		if (!overlaps_period_period_internal(&seq->period, p2))
		{
			if (before_period_period_internal(p2, &seq->period))
			{
				/* advance the component period  */
				j++;

			}
			else
			{
				/* copy the sequence */
				sequences[k++] = temporalseq_copy(seq);
				i++;
			}
		}
		else
		{
			/* Compute the difference of the overlapping periods */
			k += temporalseq_minus_periodset1(&sequences[k], seq, ps, j);
			i++;
		}
	}
	/* Copy the sequences after the period set */
	while (i < ts->count)
		sequences[k++] = temporalseq_copy(temporals_seq_n(ts, i++));
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Since both the temporals and the periodset are normalized it is not 
	   necessary to normalize the result of the difference */
	TemporalS *result = temporals_make(sequences, k, false);
	for (i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/* Does the temporal value intersect the timestamp? */

bool
temporals_intersects_timestamp(const TemporalS *ts, TimestampTz t)
{
	int n;
	if (temporals_find_timestamp(ts, t, &n))
		return false;
	return true;
}

/* Does the temporal value intersect the timestamp set? */

bool
temporals_intersects_timestampset(const TemporalS *ts, const TimestampSet *ts1)
{
	for (int i = 0; i < ts1->count; i++)
		if (temporals_intersects_timestamp(ts, timestampset_time_n(ts1, i))) 
			return true;
	return false;
}

/* Does a TemporalS intersects a period? */

bool
temporals_intersects_period(const TemporalS *ts, const Period *p)
{
	/* Binary search of lower and upper bounds of period */
	int n1, n2;
	if (temporals_find_timestamp(ts, p->lower, &n1) || 
		temporals_find_timestamp(ts, p->upper, &n2))
		return true;
	
	for (int i = n1; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (overlaps_period_period_internal(&seq->period, p))
			return true;
		if (p->upper < seq->period.upper)
			break;
	}
	return false;
}

/* Does the temporal value intersect the period set? */

bool
temporals_intersects_periodset(const TemporalS *ts, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (temporals_intersects_period(ts, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

/* Integral of the temporal numbers */

double
tnumbers_integral(const TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tnumberseq_integral(temporals_seq_n(ts, i)); 
	return result;
}

/* Time-weighted average of the temporal number */

double
tnumbers_twavg(const TemporalS *ts)
{
	double duration = temporals_interval_double(ts);
	double result;
	if (duration == 0)
	{
		result = 0;
		for (int i = 0; i < ts->count; i++)
			result += tnumberseq_twavg(temporals_seq_n(ts, i)); 
		return result / ts->count;
	}
	else
		result = tnumbers_integral(ts) / duration;
	return result;
}

/*****************************************************************************
 * Functions for defining B-tree index
 * The functions assume that the arguments are of the same temptypid
 *****************************************************************************/

/* 
 * Equality operator
 * The internal B-tree comparator is not used to increase efficiency
 */
bool
temporals_eq(const TemporalS *ts1, const TemporalS *ts2)
{
	/* If number of sequences or flags are not equal */
	if (ts1->count != ts2->count || ts1->flags != ts2->flags)
		return false;

	/* If bounding boxes are not equal */
	void *box1 = temporals_bbox_ptr(ts1);
	void *box2 = temporals_bbox_ptr(ts2);
	if (! temporal_bbox_eq(box1, box2, ts1->valuetypid))
		return false;

	/* Compare the composing sequences */
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		if (! temporalseq_eq(seq1, seq2))
			return false;
	}
	return true;
}

/* 
 * B-tree comparator
 * This function supposes for optimization purposes that
 * - a bounding box comparison has been done before in the calling function
 *   and thus that the bounding boxes are equal
 * - the flags of two TemporalSeq values of the same base type only differ
 *   on the linear interpolation flag.
 * These hypothesis may change in the future and the function must be
 * adapted accordingly.
 * */

int
temporals_cmp(const TemporalS *ts1, const TemporalS *ts2)
{
	/* Compare inclusive/exclusive bounds
	 * These tests are redundant for temporal types whose bounding box is a
	 * period, that is, tbool and ttext */
	TemporalSeq *first1 = temporals_seq_n(ts1, 0);
	TemporalSeq *first2 = temporals_seq_n(ts2, 0);
	TemporalSeq *last1 = temporals_seq_n(ts1, ts1->count - 1);
	TemporalSeq *last2 = temporals_seq_n(ts2, ts2->count - 1);
	if ((first1->period.lower_inc && ! first2->period.lower_inc) ||
		(! last1->period.upper_inc && last2->period.upper_inc))
		return -1;
	else if ((first2->period.lower_inc && ! first1->period.lower_inc) ||
		(! last2->period.upper_inc && last1->period.upper_inc))
		return 1;
	/* Compare composing instants */
	int count = Min(ts1->count, ts2->count);
	for (int i = 0; i < count; i++)
	{
		first1 = temporals_seq_n(ts1, i);
		first2 = temporals_seq_n(ts2, i);
		int result = temporalseq_cmp(first1, first2);
		if (result) 
			return result;
	}
	/* The two values are equal
	 * It is not necessary to compare flags since all the sequences are
	 * equal and thus their interpolation is also equal */
	return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of  
 * the elements.
 *****************************************************************************/

uint32
temporals_hash(const TemporalS *ts)
{
	uint32 result = 1;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		uint32 seq_hash = temporalseq_hash(seq);
		result = (result << 5) - result + seq_hash;
	}
	return result;
}

/*****************************************************************************/
