/*****************************************************************************
 *
 * tsequenceset.c
 *	  Basic functions for temporal sequence sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tsequenceset.h"

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

/**
 * Returns the n-th sequence of the temporal value
 */
TSequence *
tsequenceset_seq_n(const TSequenceSet *ts, int index)
{
	return (TSequence *)(
		(char *)(&ts->offsets[ts->count + 1]) + 	/* start of data */
			ts->offsets[index]);					/* offset */
}

/**
 * Returns a pointer to the precomputed bounding box of the temporal value
 */
void *
tsequenceset_bbox_ptr(const TSequenceSet *ts)
{
	return (char *)(&ts->offsets[ts->count + 1]) +	/* start of data */
		ts->offsets[ts->count];						/* offset */
}

/**
 * Copy in the first argument the bounding box of the temporal value
 */
void
tsequenceset_bbox(void *box, const TSequenceSet *ts)
{
	void *box1 = tsequenceset_bbox_ptr(ts);
	size_t bboxsize = temporal_bbox_size(ts->valuetypid);
	memcpy(box, box1, bboxsize);
}

/**
 * Construct a temporal sequence set value from the array of temporal
 * sequence values
 *
 * For example, the memory structure of a temporal sequence set value
 * with 2 sequences is as follows
 * @code
 * --------------------------------------------------------
 * ( TSequenceSet )_X | offset_0 | offset_1 | offset_2 | ...
 * --------------------------------------------------------
 * --------------------------------------------------------
 * ( TSequence_0 )_X | ( TSequence_1 )_X | ( bbox )_X | 
 * --------------------------------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding, `offset_0` and
 * `offset_1` are offsets for the corresponding sequences and `offset_2`
 * is the offset for the bounding box. Temporal sequence set values do not
 * have precomputed trajectory.
 *
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized.
 * In particular, normalize is false when synchronizing two
 * temporal sequence set values before applying an operation to them.
 */
TSequenceSet *
tsequenceset_make(TSequence **sequences, int count, bool normalize)
{
	/* Test the validity of the sequences */
	assert(count > 0);
	bool isgeo = tgeo_base_type(sequences[0]->valuetypid);
	ensure_valid_tsequencearr(sequences, count, isgeo);

	TSequence **newsequences = sequences;
	int newcount = count;
	if (normalize && count > 1)
		newsequences = tsequencearr_normalize(sequences, count, &newcount);
	/* Add the size of the struct and the offset array 
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TSequenceSet)) + newcount * sizeof(size_t);
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
	TSequenceSet *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;
	result->totalcount = totalcount;
	result->valuetypid = sequences[0]->valuetypid;
	result->duration = SEQUENCESET;
	MOBDB_FLAGS_SET_LINEAR(result->flags,
		MOBDB_FLAGS_GET_LINEAR(sequences[0]->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags,
			MOBDB_FLAGS_GET_Z(sequences[0]->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, 
			MOBDB_FLAGS_GET_GEODETIC(sequences[0]->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	for (int i = 0; i < newcount; i++)
	{
		memcpy(((char *) result) + pdata + pos, newsequences[i], 
			VARSIZE(newsequences[i]));
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
		tsequenceset_make_bbox(bbox, newsequences, newcount);
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

/**
 * Construct a temporal sequence set value from the array of temporal
 * sequence values and free the array and the sequences after the creation
 *
 * @param[in] sequences Array of sequences
 * @param[in] count Number of elements in the array
 * @param[in] normalize True when the resulting value should be normalized.
 */
TSequenceSet *
tsequenceset_make_free(TSequence **sequences, int count, bool normalize)
{
	if (count == 0)
	{
		pfree(sequences);
		return NULL;
	}	
	TSequenceSet *result = tsequenceset_make(sequences, count, normalize);
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/**
 * Construct a temporal sequence set value from the temporal sequence
 */
TSequenceSet *
tsequence_to_tsequenceset(const TSequence *seq)
{
	return tsequenceset_make((TSequence **)&seq, 1, NORMALIZE_NO);
}

/**
 * Construct a temporal sequence set value from from a base value and
 * a timestamp set (internal function)
 $
 * @param[in] value Base value
 * @param[in] valuetypid Oid of the base type
 * @param[in] ps Period set
 * @param[in] linear True when the resulting value has linear interpolation
*/
TSequenceSet *
tsequenceset_from_base_internal(Datum value, Oid valuetypid,
	const PeriodSet *ps, bool linear)
{
	TSequence **sequences = palloc(sizeof(TSequence *) * ps->count);
	for (int i = 0; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		sequences[i] = tsequence_from_base_internal(value, valuetypid, p, linear);
	}
	return tsequenceset_make_free(sequences, ps->count, NORMALIZE_NO);
}

PG_FUNCTION_INFO_V1(tsequenceset_from_base);
/**
 * Construct a temporal sequence set value from from a base value and
 a timestamp set
 */
PGDLLEXPORT Datum
tsequenceset_from_base(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	PeriodSet *ps = PG_GETARG_PERIODSET(1);
	bool linear = PG_GETARG_BOOL(2);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TSequenceSet *result = tsequenceset_from_base_internal(value, valuetypid, ps, linear);
	DATUM_FREE_IF_COPY(value, valuetypid, 0);
	PG_FREE_IF_COPY(ps, 1);
	PG_RETURN_POINTER(result);
}

/**
 * Append an instant to the temporal value
 */
TSequenceSet *
tsequenceset_append_tinstant(const TSequenceSet *ts, const TInstant *inst)
{
	/* The validity tests are done in the tsequence_append_tinstant function */
	TSequence *seq = tsequenceset_seq_n(ts, ts->count - 1);
	TSequence *newseq = tsequence_append_tinstant(seq, inst);
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TSequenceSet)) + ts->count * sizeof(size_t);
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(ts->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < ts->count - 1; i++)
		memsize += double_pad(VARSIZE(tsequenceset_seq_n(ts, i)));
	memsize += double_pad(VARSIZE(newseq));
	/* Create the TSequenceSet */
	TSequenceSet *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = ts->count;
	result->totalcount = ts->totalcount - seq->count + newseq->count;
	result->valuetypid = ts->valuetypid;
	result->duration = SEQUENCESET;
	MOBDB_FLAGS_SET_LINEAR(result->flags, MOBDB_FLAGS_GET_LINEAR(ts->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (tgeo_base_type(ts->valuetypid))
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(ts->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(ts->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	for (int i = 0; i < ts->count - 1; i++)
	{
		seq = tsequenceset_seq_n(ts,i);
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
		bboxunion box;
		void *bbox = ((char *) result) + pdata + pos;
		memcpy(bbox, tsequenceset_bbox_ptr(ts), bboxsize);
		tinstant_make_bbox(&box, inst);
		temporal_bbox_expand(bbox, &box, ts->valuetypid);
		result->offsets[ts->count] = pos;
	}
	pfree(newseq);
	return result;
}

/**
 * Merge the two temporal values
 */
TSequenceSet *
tsequenceset_merge(const TSequenceSet *ts1, const TSequenceSet *ts2)
{
	const TSequenceSet *seqsets[] = {ts1, ts2};
	return tsequenceset_merge_array((TSequenceSet **) seqsets, 2);
}

/**
 * Merge the array of temporal sequence set values. The function does not assume
 * that the values in the array can be strictly ordered on time, i.e., the
 * intersection of the bounding boxes of two values may be a period. 
 * For this reason two passes are necessary.
 *
 * @param[in] seqsets Array of values
 * @param[in] count Number of elements in the array
 * @result Merged value 
 */
TSequenceSet *
tsequenceset_merge_array(TSequenceSet **seqsets, int count)
{
	/* Test the validity of the temporal values */
	int totalcount = seqsets[0]->count;
	bool linear = MOBDB_FLAGS_GET_LINEAR(seqsets[0]->flags);
	Oid valuetypid = seqsets[0]->valuetypid;
	bool isgeo = tgeo_base_type(seqsets[0]->valuetypid);
	for (int i = 1; i < count; i++)
	{
		assert(valuetypid == seqsets[i]->valuetypid);
		assert(linear == MOBDB_FLAGS_GET_LINEAR(seqsets[i]->flags));
		if (isgeo)
		{
			ensure_same_srid_tpoint((Temporal *)seqsets[0], (Temporal *)seqsets[i]);
			ensure_same_dimensionality_tpoint((Temporal *)seqsets[0], (Temporal *)seqsets[i]);
		}
		totalcount += seqsets[i]->count;
	}
	/* Collect the composing sequences */
	TSequence **sequences = palloc0(sizeof(TSequence *) * totalcount);
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < seqsets[i]->count; j++)
			sequences[k++] = tsequenceset_seq_n(seqsets[i], j);
	}
	if (totalcount > 1)
		tsequencearr_sort(sequences, totalcount);
	/* Test the validity of the composing sequences */
	TSequence *seq1 = sequences[0];
	for (int i = 1; i < totalcount; i++)
	{
		TInstant *inst1 = tsequence_inst_n(seq1, seq1->count - 1);
		TSequence *seq2 = sequences[i];
		TInstant *inst2 = tsequence_inst_n(seq2, 0);
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
			if (! datum_eq(tinstant_value(inst1), tinstant_value(inst2), inst1->valuetypid))
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
	TSequence **normseqs = tsequencearr_normalize(sequences, totalcount, &totalcount1);
	/* We cannot free the composing sequences */
	pfree(sequences);
	TSequenceSet *result = tsequenceset_make_free(normseqs, totalcount1, NORMALIZE_NO);
	return result;
}

/**
 * Returns a copy of the temporal value
 */
TSequenceSet *
tsequenceset_copy(const TSequenceSet *ts)
{
	TSequenceSet *result = palloc0(VARSIZE(ts));
	memcpy(result, ts, VARSIZE(ts));
	return result;
}

/*****************************************************************************/

/**
 * Returns the location of the timestamp in the temporal sequence set value
 * using binary search
 *
 * If the timestamp is contained in the temporal value, the index of the
 * sequence is returned in the output parameter. Otherwise, returns a number
 * encoding whether the timestamp is before, between two sequences, or after
 * the temporal value. For example, given a value composed of 3 sequences 
 * and a timestamp, the value returned in the output parameter is as follows:
 * @code
 *               0          1          2
 *            |-----|    |-----|    |-----|   
 * 1)    t^                                         => loc = 0
 * 2)                 t^                            => loc = 1
 * 3)                       t^                      => loc = 1
 * 4)                             t^                => loc = 2
 * 5)                                         t^    => loc = 3
 * @endcode
 *
 * @param[in] ts Temporal sequence set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Returns true if the timestamp is contained in the temporal value
 */
bool 
tsequenceset_find_timestamp(const TSequenceSet *ts, TimestampTz t, int *loc)
{
	int first = 0, last = ts->count - 1;
	int middle = 0; /* make compiler quiet */
	TSequence *seq = NULL; /* make compiler quiet */
	while (first <= last) 
	{
		middle = (first + last)/2;
		seq = tsequenceset_seq_n(ts, middle);
		if (contains_period_timestamp_internal(&seq->period, t))
		{
			*loc = middle;
			return true;
		}
		if (t <= seq->period.lower)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (t >= seq->period.upper)
		middle++;
	*loc = middle;
	return false;
}

/*****************************************************************************
 * Intersection/synchronize functions
 *****************************************************************************/

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ts,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tinstant(const TSequenceSet *ts, const TInstant *inst,
	TInstant **inter1, TInstant **inter2)
{
	TInstant *inst1 = (TInstant *) tsequenceset_restrict_timestamp(ts, inst->t, REST_AT);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = tinstant_copy(inst);
	return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst,ts Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tinstant_tsequenceset(const TInstant *inst, const TSequenceSet *ts,
	TInstant **inter1, TInstant **inter2)
{
	return intersection_tsequenceset_tinstant(ts, inst, inter2, inter1);
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ts,ti Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tinstantset(const TSequenceSet *ts, const TInstantSet *ti,
	TInstantSet **inter1, TInstantSet **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	tsequenceset_period(&p1, ts);
	tinstantset_period(&p2, ti);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;
	
	TInstant **instants1 = palloc(sizeof(TInstant *) * ti->count);
	TInstant **instants2 = palloc(sizeof(TInstant *) * ti->count);
	int i = 0, j = 0, k = 0;
	while (i < ts->count && j < ti->count)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		TInstant *inst = tinstantset_inst_n(ti, j);
		if (contains_period_timestamp_internal(&seq->period, inst->t))
		{
			instants1[k] = tsequence_at_timestamp(seq, inst->t);
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
	
	*inter1 = tinstantset_make_free(instants1, k);
	*inter2 = tinstantset_make(instants2, k);
	pfree(instants2); 
	return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti,ts Input values
 * @param[out] inter1,inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tsequenceset(const TInstantSet *ti, const TSequenceSet *ts,
	TInstantSet **inter1, TInstantSet **inter2)
{
	return intersection_tsequenceset_tinstantset(ts, ti, inter2, inter1);
}

/**
 * Temporally intersect or synchronize the two temporal values
 *
 * The resulting values are composed of denormalized sequences 
 * covering the intersection of their time spans
 *
 * @param[in] ts,seq Input values
 * @param[out] inter1, inter2 Output values
 * @param[in] crossings State whether turning points are added in the segments
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tsequence(const TSequenceSet *ts, const TSequence *seq,
	TIntersection mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p;
	tsequenceset_period(&p, ts);
	if (!overlaps_period_period_internal(&seq->period, &p))
		return false;
	
	int loc;
	tsequenceset_find_timestamp(ts, seq->period.lower, &loc);
	/* We are sure that loc < ts->count due to the bounding period test above */
	TSequence **sequences1 = palloc(sizeof(TSequence *) * ts->count - loc);
	TSequence **sequences2 = palloc(sizeof(TSequence *) * ts->count - loc);
	int k = 0;
	for (int i = loc; i < ts->count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts, i);
		TSequence *interseq1, *interseq2;
		bool hasinter;
		if (mode == INTERSECT)
			hasinter = intersection_tsequence_tsequence(seq, seq1,
				&interseq1, &interseq2);
		else /* mode == SYNCHRONIZE */
			hasinter = synchronize_tsequence_tsequence(seq, seq1,
				&interseq1, &interseq2, CROSSINGS_NO);
		if (hasinter)
		{
			sequences1[k] = interseq1;
			sequences2[k++] = interseq2;
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
	
	*inter1 = tsequenceset_make_free(sequences1, k, NORMALIZE_NO);
	*inter2 = tsequenceset_make_free(sequences2, k, NORMALIZE_NO);
	return true;
}

/**
 * Temporally intersect or synchronize the two temporal values
 *
 * @param[in] seq,ts Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on times
 */
bool
intersection_tsequence_tsequenceset(const TSequence *seq, const TSequenceSet *ts,
	TIntersection mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
	return intersection_tsequenceset_tsequence(ts, seq, mode, inter2, inter1);
}

/**
 * Temporally intersect or synchronize the two temporal values
 *
 * @param[in] ts1,ts2 Input values
 * @param[in] mode Intersection or synchronization (with or without adding crossings)
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tsequenceset_tsequenceset(const TSequenceSet *ts1, const TSequenceSet *ts2,
	TIntersection mode, TSequenceSet **inter1, TSequenceSet **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	tsequenceset_period(&p1, ts1);
	tsequenceset_period(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;
	
	TSequence **sequences1 = palloc(sizeof(TSequence *) * 
		(ts1->count + ts2->count));
	TSequence **sequences2 = palloc(sizeof(TSequence *) * 
		(ts1->count + ts2->count));
	int i = 0, j = 0, k = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, j);
		TSequence *interseq1, *interseq2;
		bool hasinter;
		if (mode == INTERSECT)
			hasinter = intersection_tsequence_tsequence(seq1, seq2, 
				&interseq1, &interseq2);
		else /* mode == SYNCHRONIZE */
			hasinter = synchronize_tsequence_tsequence(seq1, seq2, 
				&interseq1, &interseq2, CROSSINGS_NO);
		if (hasinter)
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
	
	*inter1 = tsequenceset_make_free(sequences1, k, NORMALIZE_NO);
	*inter2 = tsequenceset_make_free(sequences2, k, NORMALIZE_NO);
	return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Returns the string representation of the temporal value
 *
 * @param[in] ts Temporal value
 * @param[in] value_out Function called to output the base value
 * depending on its Oid
 */
char *
tsequenceset_to_string(const TSequenceSet *ts, char *(*value_out)(Oid, Datum))
{
	char **strings = palloc(sizeof(char *) * ts->count);
	size_t outlen = 0;
	char prefix[20];
	if (linear_interpolation(ts->valuetypid) && 
		! MOBDB_FLAGS_GET_LINEAR(ts->flags))
		sprintf(prefix, "Interp=Stepwise;");
	else
		prefix[0] = '\0';
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		strings[i] = tsequence_to_string(seq, true, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	return stringarr_to_string(strings, ts->count, outlen, prefix, '{', '}');
}

/**
 * Write the binary representation of the temporal value
 * into the buffer
 *
 * @param[in] ts Temporal value
 * @param[in] buf Buffer
 */
void
tsequenceset_write(const TSequenceSet *ts, StringInfo buf)
{
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(buf, (uint32) ts->count, 4);
#else
	pq_sendint32(buf, ts->count);
#endif
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		tsequence_write(seq, buf);
	}
}
 
/**
 * Returns a new temporal value from its binary representation 
 * read from the buffer
 *
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
 */
TSequenceSet *
tsequenceset_read(StringInfo buf, Oid valuetypid)
{
	int count = (int) pq_getmsgint(buf, 4);
	assert(count > 0);
	TSequence **sequences = palloc(sizeof(TSequence *) * count);
	for (int i = 0; i < count; i++)
		sequences[i] = tsequence_read(buf, valuetypid);
	return tsequenceset_make_free(sequences, count, NORMALIZE_NO);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * Cast the temporal integer value as a temporal float value
 */
TSequenceSet *
tintseqset_to_tfloatseqset(const TSequenceSet *ts)
{
	/* It is not necessary to set the linear flag to false since it is already
	 * set by the fact that the input argument is a temporal integer */
	TSequenceSet *result = tsequenceset_copy(ts);
	result->valuetypid = FLOAT8OID;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(result, i);
		seq->valuetypid = FLOAT8OID;
		for (int j = 0; j < seq->count; j++)
		{
			TInstant *inst = tsequence_inst_n(seq, j);
			inst->valuetypid = FLOAT8OID;
			Datum *value_ptr = tinstant_value_ptr(inst);
			*value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
		}
	}
	return result;
}

/**
 * Cast the temporal float value as a temporal integer value
 */
TSequenceSet *
tfloatseqset_to_tintseqset(const TSequenceSet *ts)
{
	if (MOBDB_FLAGS_GET_LINEAR(ts->flags))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Cannot cast temporal float with linear interpolation to temporal integer")));
	/* It is not necessary to set the linear flag to false since it is already
	 * set by the fact that the input argument has step interpolation */
	TSequenceSet *result = tsequenceset_copy(ts);
	result->valuetypid = INT4OID;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(result, i);
		seq->valuetypid = INT4OID;
		for (int j = 0; j < seq->count; j++)
		{
			TInstant *inst = tsequence_inst_n(seq, j);
			inst->valuetypid = INT4OID;
			Datum *value_ptr = tinstant_value_ptr(inst);
			*value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
		}
	}
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * Transform the temporal instant value into a temporal sequence set value
 */
TSequenceSet *
tinstant_to_tsequenceset(const TInstant *inst, bool linear)
{
	TSequence *seq = tsequence_make((TInstant **)&inst, 1, true, true,
		linear, NORMALIZE_NO);
	TSequenceSet *result = tsequence_to_tsequenceset(seq);
	pfree(seq);
	return result;
}

/**
 * Transform the temporal instant set value into a temporal sequence set value
 */
TSequenceSet *
tinstantset_to_tsequenceset(const TInstantSet *ti, bool linear)
{
	TSequence **sequences = palloc(sizeof(TSequence *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		sequences[i] = tinstant_to_tsequence(inst, linear);
	}
	TSequenceSet *result = tsequenceset_make(sequences, ti->count, NORMALIZE_NO);
	pfree(sequences);
	return result;
}

/**
 * Transform the temporal value with continuous base type from stepwise
 * to linear interpolation
 */
TSequenceSet *
tstepseqset_to_linear(const TSequenceSet *ts)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tstepseq_to_linear(tsequenceset_seq_n(ts, 0));

	/* General case */
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tstepseq_to_linear1(&sequences[k], seq);
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Returns the distinct base values of the temporal value with stepwise
 * interpolation
 *
 * @param[in] ts Temporal value
 * @param[out] result Array of Datums
 * @result Number of elements in the output array
 */
static int 
tsequenceset_values1(Datum *result, const TSequenceSet *ts)
{
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
			result[k++] = tinstant_value(tsequence_inst_n(seq, j));
	}
	if (k > 1)
	{
		datumarr_sort(result, k, ts->valuetypid);
		k = datumarr_remove_duplicates(result, k, ts->valuetypid);
	}
	return k;
}

/**
 * Returns the distinct base values of the temporal value with stepwise
 * interpolation as a PostgreSQL array
 */
ArrayType *
tsequenceset_values(const TSequenceSet *ts)
{
	Datum *values = palloc(sizeof(Datum *) * ts->totalcount);
	int count = tsequenceset_values1(values, ts);
	ArrayType *result = datumarr_to_array(values, count, ts->valuetypid);
	pfree(values);
	return result;
}

/**
 * Returns the ranges of base values of the temporal float value
 * as a PostgreSQL array
 */
ArrayType *
tfloatseqset_ranges(const TSequenceSet *ts)
{
	int count = MOBDB_FLAGS_GET_LINEAR(ts->flags) ? ts->count : ts->totalcount;
	RangeType **ranges = palloc(sizeof(RangeType *) * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++) 
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tfloatseq_ranges1(&ranges[k], seq);
	}
	int newcount;
	RangeType **normranges = rangearr_normalize(ranges, k, &newcount);
	if (newcount > 1)
		rangearr_sort(normranges, newcount);
	ArrayType *result = rangearr_to_array(normranges, newcount, 
		type_oid(T_FLOATRANGE), true);

	for (int i = 0; i < k; i++)
		pfree(ranges[i]);
	pfree(ranges);

	return result;
}

/**
 * Returns a pointer to the instant with minimum base value of the
 * temporal value
 *
 * The function does not take into account whether the
 * instant is at an exclusive bound or not
 *
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance.
 */
TInstant *
tsequenceset_min_instant(const TSequenceSet *ts)
{
	TSequence *seq = tsequenceset_seq_n(ts, 0);
	TInstant *result = tsequence_inst_n(seq, 0);
	Datum min = tinstant_value(result);
	for (int i = 0; i < ts->count; i++)
	{
		seq = tsequenceset_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
		{
			TInstant *inst = tsequence_inst_n(seq, j);
			Datum value = tinstant_value(inst);
			if (datum_lt(value, min, seq->valuetypid))
			{
				min = value;
				result = inst;
			}
		}
	}
	return result;
}

/**
 * Returns the minimum base value of the temporal value
 */
Datum
tsequenceset_min_value(const TSequenceSet *ts)
{
	Oid valuetypid = ts->valuetypid;
	if (valuetypid == INT4OID)
	{
		TBOX *box = tsequenceset_bbox_ptr(ts);
		return Int32GetDatum((int)(box->xmin));
	}
	if (valuetypid == FLOAT8OID)
	{
		TBOX *box = tsequenceset_bbox_ptr(ts);
		return Float8GetDatum(box->xmin);
	}
	Datum result = tsequence_min_value(tsequenceset_seq_n(ts, 0));
	for (int i = 1; i < ts->count; i++)
	{
		Datum value = tsequence_min_value(tsequenceset_seq_n(ts, i));
		if (datum_lt(value, result, valuetypid))
			result = value;
	}
	return result;
}

/**
 * Returns the maximum base value of the temporal value
 */
Datum
tsequenceset_max_value(const TSequenceSet *ts)
{
	Oid valuetypid = ts->valuetypid;
	if (valuetypid == INT4OID)
	{
		TBOX *box = tsequenceset_bbox_ptr(ts);
		return Int32GetDatum((int)(box->xmax));
	}
	if (valuetypid == FLOAT8OID)
	{
		TBOX *box = tsequenceset_bbox_ptr(ts);
		return Float8GetDatum(box->xmax);
	}
	Datum result = tsequence_max_value(tsequenceset_seq_n(ts, 0));
	for (int i = 1; i < ts->count; i++)
	{
		Datum value = tsequence_max_value(tsequenceset_seq_n(ts, i));
		if (datum_gt(value, result, valuetypid))
			result = value;
	}
	return result;
}

/**
 * Returns the time on which the temporal value is defined as a period set
 */
PeriodSet *
tsequenceset_get_time(const TSequenceSet *ts)
{
	Period **periods = palloc(sizeof(Period *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		periods[i] = &seq->period;
	}
	PeriodSet *result = periodset_make(periods, ts->count, NORMALIZE_NO);
	pfree(periods);
	return result;
}

/**
 * Returns the timespan of the temporal value
 */
Datum
tsequenceset_timespan(const TSequenceSet *ts)
{
	TSequence *seq = tsequenceset_seq_n(ts, 0);
	Datum result = call_function2(timestamp_mi, 
		TimestampTzGetDatum(seq->period.upper), TimestampTzGetDatum(seq->period.lower));
	for (int i = 1; i < ts->count; i++)
	{
		seq = tsequenceset_seq_n(ts, i);
		Datum interval1 = call_function2(timestamp_mi, 
			TimestampTzGetDatum(seq->period.upper), TimestampTzGetDatum(seq->period.lower));
		Datum interval2 = call_function2(interval_pl, result, interval1);
		pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
		result = interval2;
	}
	return result;
}

/**
 * Returns the duration of the temporal value as a double
 */
double
tsequenceset_interval_double(const TSequenceSet *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		result += (double) (seq->period.upper - seq->period.lower);
	}
	return result;
}

/**
 * Returns the bounding period on which the temporal value is defined
 */
void
tsequenceset_period(Period *p, const TSequenceSet *ts)
{
	TSequence *start = tsequenceset_seq_n(ts, 0);
	TSequence *end = tsequenceset_seq_n(ts, ts->count - 1);
	period_set(p, start->period.lower, end->period.upper, 
		start->period.lower_inc, end->period.upper_inc);
}

/**
 * Returns the sequences of the temporal value as a C array
 */
TSequence **
tsequenceset_sequences(const TSequenceSet *ts)
{
	TSequence **result = palloc(sizeof(TSequence *) * ts->count);
	for (int i = 0; i < ts->count; i++) 
		result[i] = tsequenceset_seq_n(ts, i);
	return result;
}

/**
 * Returns the sequences of the temporal value as a PostgreSQL array
 */
ArrayType *
tsequenceset_sequences_array(const TSequenceSet *ts)
{
	TSequence **sequences = tsequenceset_sequences(ts);
	ArrayType *result = temporalarr_to_array((Temporal **)sequences, ts->count);
	pfree(sequences);
	return result;
}

/**
 * Returns the number of distinct instants of the temporal value
 */
int
tsequenceset_num_instants(const TSequenceSet *ts)
{
	TInstant *lastinst;
	bool first = true;
	int result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		result += seq->count;
		if (!first)
		{
			if (tinstant_eq(lastinst, tsequence_inst_n(seq, 0)))
				result --;
		}
		lastinst = tsequence_inst_n(seq, seq->count - 1);
		first = false;
	}
	return result;
}

/**
 * Returns the n-th distinct instant of the temporal value
 */
TInstant *
tsequenceset_instant_n(const TSequenceSet *ts, int n)
{
	assert (n >= 1 && n <= ts->totalcount);
	if (n == 1)
	{
		TSequence *seq = tsequenceset_seq_n(ts, 0);
		return tsequence_inst_n(seq, 0);
	}
	
	/* Continue the search 0-based */
	n--;
	TInstant *prev, *next;
	bool first = true, found = false;
	int i = 0, count = 0, prevcount = 0;
	while (i < ts->count)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		count += seq->count;
		if (!first && tinstant_eq(prev, tsequence_inst_n(seq, 0)))
		{
				prevcount --;
				count --;
		}
		if (prevcount <= n && n < count)
		{
			next = tsequence_inst_n(seq, n - prevcount);
			found = true;
			break;
		}
		prevcount = count;
		prev = tsequence_inst_n(seq, seq->count - 1);
		first = false;
		i++;
	}
	if (!found) 
		return NULL;
	return next;
}

/**
 * Returns the distinct instants of the temporal value
 */
ArrayType *
tsequenceset_instants_array(const TSequenceSet *ts)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ts->totalcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		for (int j = 0; j < seq->count; j++)
			instants[k++] = tsequence_inst_n(seq, j);
	}
	int count = tinstantarr_remove_duplicates(instants, k);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, count);
	pfree(instants);
	return result;
}

/**
 * Returns the start timestamp of the temporal value.
 */
TimestampTz
tsequenceset_start_timestamp(const TSequenceSet *ts)
{
	TSequence *seq = tsequenceset_seq_n(ts, 0);
	return seq->period.lower;
}

/**
 * Returns the end timestamp of the temporal value 
 */
TimestampTz
tsequenceset_end_timestamp(const TSequenceSet *ts)
{
	TSequence *seq = tsequenceset_seq_n(ts, ts->count - 1);
	return seq->period.upper;
}

/**
 * Returns the number of distinct timestamps of the temporal value
 */
int
tsequenceset_num_timestamps(const TSequenceSet *ts)
{
	TimestampTz lasttime;
	bool first = true;
	int result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		result += seq->count;
		if (!first)
		{
			if (lasttime == tsequence_inst_n(seq, 0)->t)
				result --;
		}
		lasttime = tsequence_inst_n(seq, seq->count - 1)->t;
		first = false;
	}
	return result;
}

/**
 * Returns the n-th distinct timestamp of the temporal value
 */
bool
tsequenceset_timestamp_n(const TSequenceSet *ts, int n, TimestampTz *result)
{
	bool found = false;
	if (n < 1)
		return false;
	if (n == 1)
	{
		TSequence *seq = tsequenceset_seq_n(ts, 0);
		*result = tsequence_inst_n(seq, 0)->t;
		return true ;
	}
	
	/* Continue the search 0-based */
	n--;
	TimestampTz prev, next;
	bool first = true;
	int i = 0, count = 0, prevcount = 0;
	while (i < ts->count)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		count += seq->count;
		if (!first && prev == tsequence_inst_n(seq, 0)->t)
		{
				prevcount --;
				count --;
		}
		if (prevcount <= n && n < count)
		{
			next = tsequence_inst_n(seq, n - prevcount)->t;
			found = true;
			break;
		}
		prevcount = count;
		prev = tsequence_inst_n(seq, seq->count - 1)->t;
		first = false;
		i++;
	}
	if (!found) 
		return false;
	*result = next;
	return true;
}

/**
 * Returns the distinct timestamps of the temporal value
 *
 * @param[out] result Array on which the timestamps are stored
 * @param[in] ts Temporal value
 * @result Number of elements in the output array
 */
int
tsequenceset_timestamps1(TimestampTz *result, const TSequenceSet *ts)
{
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tsequence_timestamps1(&result[k], seq);
	}
	if (k > 1)
	{
		timestamparr_sort(result, k);
		k = timestamparr_remove_duplicates(result, k);
	}
	return k;
}

/**
 * Returns the distinct timestamps of the temporal value as a
 * PostgreSQL array
 */
ArrayType *
tsequenceset_timestamps(const TSequenceSet *ts)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ts->totalcount);
	int count = tsequenceset_timestamps1(times, ts);
	ArrayType *result = timestamparr_to_array(times, count);
	pfree(times);
	return result;
}

/**
 * Shift the time span of the temporal value by the interval
 */
TSequenceSet *
tsequenceset_shift(const TSequenceSet *ts, const Interval *interval)
{
	TSequenceSet *result = tsequenceset_copy(ts);
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->count);
	TInstant **instants = palloc(sizeof(TInstant *) * ts->totalcount);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = sequences[i] = tsequenceset_seq_n(result, i);
		for (int j = 0; j < seq->count; j++)
		{
			TInstant *inst = instants[j] = tsequence_inst_n(seq, j);
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
		void *bbox = tsequence_bbox_ptr(seq); 
		temporal_bbox_shift(bbox, interval, seq->valuetypid);
	
	}
	/* Shift bounding box */
	void *bbox = tsequenceset_bbox_ptr(result); 
	temporal_bbox_shift(bbox, interval, ts->valuetypid);
	pfree(sequences);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

/**
 * Returns true if the temporal value is ever equal to the base value
 */
bool
tsequenceset_ever_eq(const TSequenceSet *ts, Datum value)
{
	/* Bounding box test */
	if (! temporal_bbox_ever_eq((Temporal *)ts, value))
		return false;

	for (int i = 0; i < ts->count; i++) 
		if (tsequence_ever_eq(tsequenceset_seq_n(ts, i), value))
			return true;
	return false;
}

/**
 * Returns true if the temporal value is always equal to the base value
 */
bool
tsequenceset_always_eq(const TSequenceSet *ts, Datum value)
{
	/* Bounding box test */
	if (! temporal_bbox_always_eq((Temporal *)ts, value))
		return false;

	/* The bounding box test above is enough to compute
	 * the answer for temporal numbers and points */
	if (tnumber_base_type(ts->valuetypid) || tgeo_base_type(ts->valuetypid))
		return true;

	for (int i = 0; i < ts->count; i++) 
		if (!tsequence_always_eq(tsequenceset_seq_n(ts, i), value))
			return false;
	return true;
}

/*****************************************************************************/

/**
 * Returns true if the temporal value is ever less than the base value
 */
bool
tsequenceset_ever_lt(const TSequenceSet *ts, Datum value)
{
	/* Bounding box test */
	if (! temporal_bbox_ever_lt_le((Temporal *)ts, value))
		return false;

	for (int i = 0; i < ts->count; i++) 
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (tsequence_ever_lt(seq, value))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value is ever less than or equal 
 * to the base value
 */
bool
tsequenceset_ever_le(const TSequenceSet *ts, Datum value)
{
	/* Bounding box test */
	if (! temporal_bbox_ever_lt_le((Temporal *)ts, value))
		return false;

	for (int i = 0; i < ts->count; i++) 
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (tsequence_ever_le(seq, value))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value is always less than the base value
 */
bool
tsequenceset_always_lt(const TSequenceSet *ts, Datum value)
{
	/* Bounding box test */
	if (! temporal_bbox_always_lt_le((Temporal *)ts, value))
		return false;

	for (int i = 0; i < ts->count; i++) 
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (! tsequence_always_lt(seq, value))
			return false;
	}
	return true;
}

/**
 * Returns true if the temporal value is always less than or equal 
 * to the base value
 */
bool
tsequenceset_always_le(const TSequenceSet *ts, Datum value)
{
	/* Bounding box test */
	if (! temporal_bbox_always_lt_le((Temporal *)ts, value))
		return false;

	/* The bounding box test above is enough to compute
	 * the answer for temporal numbers */
	if (tnumber_base_type(ts->valuetypid))
		return true;

	for (int i = 0; i < ts->count; i++) 
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (! tsequence_always_le(seq, value))
			return false;
	}
	return true;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/**
 * Restricts the temporal value to the base value.
 *
 * @note There is no bounding box test in this function, it is done in the
 * dispatch function for all durations.
 */
TSequenceSet *
tsequenceset_restrict_value(const TSequenceSet *ts, Datum value, bool atfunc)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tsequence_restrict_value(tsequenceset_seq_n(ts, 0), 
			value, atfunc);

	/* General case */
	int count = ts->totalcount;
	/* For minus and linear interpolation we need the double of the count */
	if (!atfunc && MOBDB_FLAGS_GET_LINEAR(ts->flags))
		count *= 2;
			
	TSequence **sequences = palloc(sizeof(TSequence *) * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tsequence_restrict_value1(&sequences[k], seq, value, atfunc) ;
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Restricts the temporal value to the (complement of the) array of base values
 * 
 * @param[in] ts Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus 
 * @pre There are no duplicates values in the array
 */
TSequenceSet *
tsequenceset_restrict_values(const TSequenceSet *ts, const Datum *values,
	int count, bool atfunc)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tsequence_restrict_values(tsequenceset_seq_n(ts, 0), values, 
			count, atfunc);

	/* General case 
	 * Compute the AT function */
	TSequence **sequences = palloc(sizeof(TSequence *) * ts->totalcount * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tsequence_at_values1(&sequences[k], seq, values, count);
	}
	TSequenceSet *atresult = tsequenceset_make_free(sequences, k, NORMALIZE);
	if (atfunc)
		return atresult;
	
	/* 
	 * MINUS function
	 * Compute the complement of the previous value.
	 */
	if (k == 0)
		return tsequenceset_copy(ts);

	PeriodSet *ps1 = tsequenceset_get_time(ts);
	PeriodSet *ps2 = tsequenceset_get_time(atresult);
	PeriodSet *ps = minus_periodset_periodset_internal(ps1, ps2);
	TSequenceSet *result = NULL;
	if (ps != NULL)
	{
		result = tsequenceset_restrict_periodset(ts, ps, REST_AT);
		pfree(ps);
	}
	pfree(atresult); pfree(ps1);  pfree(ps2);
	return result;
}

/**
 * Restricts the temporal number to the range of base values
 * @note It is supposed that a bounding box test has been done in the dispatch 
 * function.
 */
TSequenceSet *
tnumberseqset_restrict_range(const TSequenceSet *ts, RangeType *range, bool atfunc)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_restrict_range(tsequenceset_seq_n(ts, 0), range, atfunc);

	/* General case */
	int count = ts->totalcount;
	/* For minus and linear interpolation we need the double of the count */
	if (!atfunc && MOBDB_FLAGS_GET_LINEAR(ts->flags))
		count *= 2;
	TSequence **sequences = palloc(sizeof(TSequence *) * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tnumberseq_restrict_range1(&sequences[k], seq, range, atfunc);
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Restricts the temporal number to the (complement of the) array of ranges of
 * base values
 *
 * @param[in] ts Temporal number
 * @param[in] normranges Array of ranges of base values
 * @param[in] count Number of elements in the input array
 * @param[in] atfunc True when the restriction is at, false for minus 
 * @return Resulting temporal number value
 * @pre The array of ranges is normalized
 * @note A bounding box test has been done in the dispatch function.
 */
TSequenceSet *
tnumberseqset_restrict_ranges(const TSequenceSet *ts, RangeType **normranges,
	int count, bool atfunc)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_restrict_ranges(tsequenceset_seq_n(ts, 0),
			normranges, count, atfunc, BBOX_TEST_NO);

	/* General case */
	int maxcount = ts->totalcount * count;
	/* For minus and linear interpolation we need the double of the count */
	if (! atfunc && MOBDB_FLAGS_GET_LINEAR(ts->flags))
		maxcount *= 2;
	TSequence **sequences = palloc(sizeof(TSequence *) * maxcount);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		k += tnumberseq_restrict_ranges1(&sequences[k], seq, normranges,
			count, atfunc, BBOX_TEST);
	}
	return tsequenceset_make_free(sequences, k, NORMALIZE);
}

/**
 * Restricts the temporal value to (the complement of) the 
 * minimum/maximum base value
 */
TSequenceSet *
tsequenceset_restrict_minmax(const TSequenceSet *ts, bool min, bool atfunc)
{
	Datum minmax = min ? tsequenceset_min_value(ts) : tsequenceset_max_value(ts);
	return tsequenceset_restrict_value(ts, minmax, atfunc);
}

/**
 * Restricts the temporal value to the (the complement of) timestamp
 */
Temporal *
tsequenceset_restrict_timestamp(const TSequenceSet *ts, TimestampTz t,
	bool atfunc)
{
	/* Bounding box test */
	Period p;
	tsequenceset_period(&p, ts);
	if (!contains_period_timestamp_internal(&p, t))
		return atfunc ? NULL : (Temporal *) tsequenceset_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return atfunc ?
			(Temporal *) tsequence_at_timestamp(tsequenceset_seq_n(ts, 0), t) :
			(Temporal *) tsequence_minus_timestamp(tsequenceset_seq_n(ts, 0), t);

	/* General case */
	if (atfunc)
	{
		int loc;
		if (!tsequenceset_find_timestamp(ts, t, &loc))
			return NULL;
		TSequence *seq = tsequenceset_seq_n(ts, loc);
		return (Temporal *) tsequence_at_timestamp(seq, t);
	}
	else
	{
		/* At most one composing sequence can be split into two */
		TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count + 1));
		int k = 0;
		int i;
		for (i = 0; i < ts->count; i++)
		{
			TSequence *seq = tsequenceset_seq_n(ts, i);
			k += tsequence_minus_timestamp1(&sequences[k], seq, t);
			if (t < seq->period.upper)
			{
				i++;
				break;
			}
		}
		/* Copy the remaining sequences if went out of the for loop with the break */
		for (int j = i; j < ts->count; j++)
			sequences[k++] = tsequence_copy(tsequenceset_seq_n(ts, j));
		/* k is never equal to 0 since in that case it is a singleton sequence set 
		   and it has been dealt by tsequence_minus_timestamp above */
		return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE_NO);
	}
}

/**
 * Returns the base value of the temporal value at the timestamp
 *
 * @param[in] ts Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Returns true if the timestamp is contained in the temporal value
 * @pre A bounding box test has been done before by the calling function
 */
bool
tsequenceset_value_at_timestamp(const TSequenceSet *ts, TimestampTz t, Datum *result)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tsequence_value_at_timestamp(tsequenceset_seq_n(ts, 0), t, result);

	/* General case */
	int loc;
	if (!tsequenceset_find_timestamp(ts, t, &loc))
		return false;
	return tsequence_value_at_timestamp(tsequenceset_seq_n(ts, loc), t, result);
}

/**
 * Returns the base value of the temporal value at the timestamp when the
 * timestamp may be at an exclusive bound
 *
 * @param[in] ts Temporal value
 * @param[in] t Timestamp
 * @param[out] result Base value
 * @result Returns true if the timestamp is found in the temporal value
 */
bool
tsequenceset_value_at_timestamp_inc(const TSequenceSet *ts, TimestampTz t,
	Datum *result)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tsequence_value_at_timestamp_inc(tsequenceset_seq_n(ts, 0),
			t, result);

	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (contains_period_timestamp_internal(&seq->period, t))
			return tsequence_value_at_timestamp(seq, t, result);
		/* Test whether the timestamp is at one of the bounds */
		TInstant *inst = tsequence_inst_n(seq, 0);
		if (inst->t == t)
			return tinstant_value_at_timestamp(inst, t, result);
		inst = tsequence_inst_n(seq, seq->count - 1);
		if (inst->t == t)
			return tinstant_value_at_timestamp(inst, t, result);
	}
	/* Since this function is always called with a timestamp that appears
	 * in the sequence set value the next statement is never reached */
	return false;
}


/**
 * Restricts the temporal value to the (complement of the) timestamp set
 */
Temporal *
tsequenceset_restrict_timestampset(const TSequenceSet *ts1, 
	const TimestampSet *ts2, bool atfunc)
{
	/* Bounding box test */
	Period p1;
	tsequenceset_period(&p1, ts1);
	Period *p2 = timestampset_bbox(ts2);
	if (!overlaps_period_period_internal(&p1, p2))
		return atfunc ? NULL : (Temporal *) tsequenceset_copy(ts1);

	/* Singleton sequence set */
	if (ts1->count == 1)
		return atfunc ?
			(Temporal *) tsequence_at_timestampset(tsequenceset_seq_n(ts1, 0), ts2) :
			(Temporal *) tsequence_minus_timestampset(tsequenceset_seq_n(ts1, 0), ts2);

	/* General case */
	if (atfunc)
	{
		TInstant **instants = palloc(sizeof(TInstant *) * ts2->count);
		int count = 0;
		int i = 0, j = 0;
		while (i < ts2->count && j < ts1->count)
		{
			TSequence *seq = tsequenceset_seq_n(ts1, j);
			TimestampTz t = timestampset_time_n(ts2, i);
			if (contains_period_timestamp_internal(&seq->period, t))
			{
				instants[count++] = tsequence_at_timestamp(seq, t);
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
		return (Temporal *) tinstantset_make_free(instants, count);
	}
	else
	{
		/* For the minus case each timestamp will split at most one 
		 * composing sequence into two */
		TSequence **sequences = palloc(sizeof(TSequence *) * (ts1->count + ts2->count + 1));
		int k = 0;
		for (int i = 0; i < ts1->count; i++)
		{
			TSequence *seq = tsequenceset_seq_n(ts1, i);
			k += tsequence_minus_timestampset1(&sequences[k], seq, ts2);

		}
		return (Temporal *) tsequenceset_make_free(sequences, k, NORMALIZE);
	}
}

/**
 * Restricts the temporal value to the (complement of the) period
 */
TSequenceSet *
tsequenceset_restrict_period(const TSequenceSet *ts, const Period *p, bool atfunc)
{
	/* Bounding box test */
	Period p1;
	tsequenceset_period(&p1, ts);
	if (!overlaps_period_period_internal(&p1, p))
		return atfunc ? NULL : tsequenceset_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
	{
		if (atfunc)
		{
			TSequence *seq = tsequence_at_period(tsequenceset_seq_n(ts, 0), p);
			return tsequence_to_tsequenceset(seq);
		}
		else
			tsequence_minus_period(tsequenceset_seq_n(ts, 0), p);
	}

	/* General case */
	if (atfunc)
	{
		int loc;
		tsequenceset_find_timestamp(ts, p->lower, &loc);
		/* We are sure that loc < ts->count because of the bounding period test above */
		TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count - loc));
		TSequence *tofree[2];
		int k = 0, l = 0;
		for (int i = loc; i < ts->count; i++)
		{
			TSequence *seq = tsequenceset_seq_n(ts, i);
			if (contains_period_period_internal(p, &seq->period))
					sequences[k++] = seq;
			else if (overlaps_period_period_internal(p, &seq->period))
			{
				TSequence *newseq = tsequence_at_period(seq, p);
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
		/* Since both the tsequenceset and the period are normalized it is not 
		 * necessary to normalize the result of the projection */
		TSequenceSet *result = tsequenceset_make(sequences, k, NORMALIZE_NO);
		for (int i = 0; i < l; i++)
			pfree(tofree[i]);
		pfree(sequences);
		return result;
	}
	else
	{
		PeriodSet *ps = tsequenceset_get_time(ts);
		PeriodSet *resultps = minus_periodset_period_internal(ps, p);
		TSequenceSet *result = NULL;
		if (resultps != NULL)
			result = tsequenceset_restrict_periodset(ts, resultps, REST_AT);

		pfree(ps); pfree(resultps);

		return result;
	}
}

/**
 * Restricts the temporal value to the (complement of the) period set
 */
TSequenceSet *
tsequenceset_restrict_periodset(const TSequenceSet *ts, const PeriodSet *ps,
	bool atfunc)
{
	/* Bounding box test */
	Period p1;
	tsequenceset_period(&p1, ts);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return atfunc ? NULL : tsequenceset_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return tsequence_restrict_periodset(tsequenceset_seq_n(ts, 0), ps, atfunc);

	/* General case */
	if (atfunc)
	{
		TimestampTz t = Max(p1.lower, p2->lower);
		int loc1, loc2;
		tsequenceset_find_timestamp(ts, t, &loc1);
		periodset_find_timestamp(ps, t, &loc2);
		TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count + ps->count - loc1 - loc2));
		int i = loc1, j = loc2, k = 0;
		while (i < ts->count && j < ps->count)
		{
			TSequence *seq = tsequenceset_seq_n(ts, i);
			Period *p = periodset_per_n(ps, j);
			TSequence *seq1 = tsequence_at_period(seq, p);
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
		/* Since both the tsequenceset and the periodset are normalized it is not 
		   necessary to normalize the result of the projection */
		return tsequenceset_make_free(sequences, k, NORMALIZE_NO);
	}
	else
	{
		TSequence **sequences = palloc(sizeof(TSequence *) * (ts->count + ps->count));
		int i = 0, j = 0, k = 0;
		while (i < ts->count && j < ps->count)
		{
			TSequence *seq = tsequenceset_seq_n(ts, i);
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
					sequences[k++] = tsequence_copy(seq);
					i++;
				}
			}
			else
			{
				/* Compute the difference of the overlapping periods */
				k += tsequence_minus_periodset(&sequences[k], seq, ps, j);
				i++;
			}
		}
		/* Copy the sequences after the period set */
		while (i < ts->count)
			sequences[k++] = tsequence_copy(tsequenceset_seq_n(ts, i++));
		/* Since both the tsequenceset and the periodset are normalized it is not 
		   necessary to normalize the result of the difference */
		return tsequenceset_make_free(sequences, k, NORMALIZE_NO);
	}
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/**
 * Returns true if the temporal value intersect the timestamp
 */
bool
tsequenceset_intersects_timestamp(const TSequenceSet *ts, TimestampTz t)
{
	int loc;
	if (tsequenceset_find_timestamp(ts, t, &loc))
		return false;
	return true;
}

/**
 * Returns true if the temporal value intersect the timestamp set
 */
bool
tsequenceset_intersects_timestampset(const TSequenceSet *ts, const TimestampSet *ts1)
{
	for (int i = 0; i < ts1->count; i++)
		if (tsequenceset_intersects_timestamp(ts, timestampset_time_n(ts1, i))) 
			return true;
	return false;
}

/**
 * Returns true if the temporal value intersect the period
 */
bool
tsequenceset_intersects_period(const TSequenceSet *ts, const Period *p)
{
	/* Binary search of lower and upper bounds of period */
	int loc1, loc2;
	if (tsequenceset_find_timestamp(ts, p->lower, &loc1) || 
		tsequenceset_find_timestamp(ts, p->upper, &loc2))
		return true;
	
	for (int i = loc1; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (overlaps_period_period_internal(&seq->period, p))
			return true;
		if (p->upper < seq->period.upper)
			break;
	}
	return false;
}

/**
 * Returns true if the temporal value intersect the period set
 */
bool
tsequenceset_intersects_periodset(const TSequenceSet *ts, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (tsequenceset_intersects_period(ts, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

/**
 * Returns the integral (area under the curve) of the temporal number
 */
double
tnumberseqset_integral(const TSequenceSet *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tnumberseq_integral(tsequenceset_seq_n(ts, i)); 
	return result;
}

/**
 * Returns the time-weighted average of the temporal number
 */
double
tnumberseqset_twavg(const TSequenceSet *ts)
{
	double duration = tsequenceset_interval_double(ts);
	double result;
	if (duration == 0.0)
	{
		result = 0;
		for (int i = 0; i < ts->count; i++)
			result += tnumberseq_twavg(tsequenceset_seq_n(ts, i)); 
		return result / ts->count;
	}
	else
		result = tnumberseqset_integral(ts) / duration;
	return result;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * Returns true if the two temporal sequence set values are equal
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tsequenceset_eq(const TSequenceSet *ts1, const TSequenceSet *ts2)
{
	assert(ts1->valuetypid == ts2->valuetypid);
	/* If number of sequences or flags are not equal */
	if (ts1->count != ts2->count || ts1->flags != ts2->flags)
		return false;

	/* If bounding boxes are not equal */
	void *box1 = tsequenceset_bbox_ptr(ts1);
	void *box2 = tsequenceset_bbox_ptr(ts2);
	if (! temporal_bbox_eq(box1, box2, ts1->valuetypid))
		return false;

	/* Compare the composing sequences */
	for (int i = 0; i < ts1->count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, i);
		if (! tsequence_eq(seq1, seq2))
			return false;
	}
	return true;
}

/**
 * Returns -1, 0, or 1 depending on whether the first temporal value 
 * is less than, equal, or greater than the second one
 *
 * @pre The arguments are of the same base type
 */
int
tsequenceset_cmp(const TSequenceSet *ts1, const TSequenceSet *ts2)
{
	assert(ts1->valuetypid == ts2->valuetypid);
	
	/* Compare bounding period
	 * We need to compare periods AND bounding boxes since the bounding boxes
	 * do not distinguish between inclusive and exclusive bounds */
	Period p1, p2;
	tsequenceset_period(&p1, ts1);
	tsequenceset_period(&p2, ts2);
	int result = period_cmp_internal(&p1, &p2);
	if (result)
		return result;

	/* Compare bounding box */
	bboxunion box1, box2;
	memset(&box1, 0, sizeof(bboxunion));
	memset(&box2, 0, sizeof(bboxunion));
	tsequenceset_bbox(&box1, ts1);
	tsequenceset_bbox(&box2, ts2);
	result = temporal_bbox_cmp(&box1, &box2, ts1->valuetypid);
	if (result)
		return result;
	
	/* Compare composing instants */
	int count = Min(ts1->count, ts2->count);
	for (int i = 0; i < count; i++)
	{
		TSequence *seq1 = tsequenceset_seq_n(ts1, i);
		TSequence *seq2 = tsequenceset_seq_n(ts2, i);
		result = tsequence_cmp(seq1, seq2);
		if (result) 
			return result;
	}

	/* ts1->count == ts2->count because of the bounding box and the
	 * composing sequence tests above */

	/* ts1->flags == ts2->flags since all the composing sequences are equal */

	/* The two values are equal */
	return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of  
 * the elements.
 *****************************************************************************/

/**
 * Returns the hash value of the temporal value
 */
uint32
tsequenceset_hash(const TSequenceSet *ts)
{
	uint32 result = 1;
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		uint32 seq_hash = tsequence_hash(seq);
		result = (result << 5) - result + seq_hash;
	}
	return result;
}

/*****************************************************************************/
