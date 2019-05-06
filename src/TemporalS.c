/*****************************************************************************
 *
 * TemporalS.c
 *	  Basic functions for temporal set sequences.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <TemporalTypes.h>

#ifdef WITH_POSTGIS
#include "TemporalPoint.h"
#endif

/*****************************************************************************
 * General functions
 *****************************************************************************/

/* 
 * The memory structure of a TemporalS with, e.g., 2 sequences is as follows
 *
 *	--------------------------------------------------------
 *	( TemporalS | offset_0 | offset_1 | offset_2 )_ X | ...
 *	--------------------------------------------------------
 *	--------------------------------------------------------
 *	( TemporalSeq_0 )_X | ( TemporalSeq_1 )_X | ( bbox )_X | 
 *	--------------------------------------------------------
 *
 * where the X are unused bytes added for double padding, offset_0 and offset_1
 * are offsets for the corresponding sequences and offset_2 is the offset for the 
 * bounding box. There is no precomputed trajectory for TemporalS.
 */

/* Pointer to the offset array of the TemporalS */

static size_t *
temporals_offsets_ptr(TemporalS *ts)
{
	return (size_t *) (((char *)ts) + sizeof(TemporalS));
}

/* Pointer to the first TemporalSeq */

static char * 
temporals_data_ptr(TemporalS *ts)
{
	return (char *)ts + double_pad(sizeof(TemporalS) + 
		sizeof(size_t) * (ts->count+1));
}

/* N-th TemporalSeq of a TemporalS */

TemporalSeq *
temporals_seq_n(TemporalS *ts, int index)
{
	size_t *offsets = temporals_offsets_ptr(ts);
	return (TemporalSeq *) (temporals_data_ptr(ts) + offsets[index]);
}

/* Pointer to the bounding box of a TemporalS */

void *
temporals_bbox_ptr(TemporalS *ts) 
{
	size_t *offsets = temporals_offsets_ptr(ts);
	return temporals_data_ptr(ts) + offsets[ts->count];
}

/* Copy the bounding box of a TemporalS in the first argument */

void
temporals_bbox(void *box, TemporalS *ts) 
{
	void *box1 = temporals_bbox_ptr(ts);
	size_t bboxsize = temporal_bbox_size(ts->valuetypid);
	memcpy(box, box1, bboxsize);
	return;
}

/* Construct a TemporalS from an array of TemporalSeq 
 * The normalize argument determines whether the resulting value will be
 * normalized. In particular, normalize is false when synchronizing two 
 * TemporalS before applying an operation to them */

TemporalS *
temporals_from_temporalseqarr(TemporalSeq **sequences, int count, 
	bool normalize)
{
	Oid valuetypid = sequences[0]->valuetypid;
	/* Test the validity of the sequences */
	if (count < 1)
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
			errmsg("A temporal sequence set must have at least one temporal sequence")));
	bool tempcontinuous = true;
#ifdef WITH_POSTGIS
	bool isgeo = false, hasz = false;
	int srid;
	if (valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_GEOGRAPHY))
	{
		isgeo = true;
		hasz = MOBDB_FLAGS_GET_Z(sequences[0]->flags);
		srid = tpoint_srid_internal((Temporal *)sequences[0]);
	}
#endif
	for (int i = 1; i < count; i++)
	{
		if (timestamp_cmp_internal(sequences[i-1]->period.upper, sequences[i]->period.lower) > 0 ||
		   (timestamp_cmp_internal(sequences[i-1]->period.upper, sequences[i]->period.lower) == 0 &&
		   sequences[i-1]->period.upper_inc && sequences[i]->period.lower_inc))
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
				errmsg("Invalid sequence for temporal sequence set")));
		tempcontinuous &= (timestamp_cmp_internal(sequences[i-1]->period.upper, 
			sequences[i]->period.lower) == 0 && 
			(sequences[i-1]->period.upper_inc || sequences[i]->period.lower_inc));
#ifdef WITH_POSTGIS
		if (isgeo)
		{
			if (tpoint_srid_internal((Temporal *)sequences[i]) != srid)
				ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
					errmsg("All geometries composing a temporal point must be of the same srid")));
			if (MOBDB_FLAGS_GET_Z(sequences[i]->flags) != hasz)
				ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
					errmsg("All geometries composing a temporal point must be of the same dimensionality")));
		}
#endif
	}

	TemporalSeq **newsequences = sequences;
	int newcount = count;
	if (normalize && count > 1)
		newsequences = temporalseqarr_normalize(sequences, count, &newcount);
	/* Compute the size of the TemporalS */
	size_t pdata = double_pad(sizeof(TemporalS) + (newcount+1) * sizeof(size_t));
	size_t memsize = 0;
	int totalcount = 0;
	for (int i = 0; i < newcount; i++)
	{
		totalcount += newsequences[i]->count;
		memsize += double_pad(VARSIZE(newsequences[i]));
	}
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(valuetypid);
	memsize += double_pad(bboxsize);
	TemporalS *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;
	result->totalcount = totalcount;
	result->valuetypid = valuetypid;
	result->type = TEMPORALS;
	bool continuous = MOBDB_FLAGS_GET_CONTINUOUS(newsequences[0]->flags);
	MOBDB_FLAGS_SET_CONTINUOUS(result->flags, continuous);
	MOBDB_FLAGS_SET_TEMPCONTINUOUS(result->flags, tempcontinuous);
#ifdef WITH_POSTGIS
	if (isgeo)
		MOBDB_FLAGS_SET_Z(result->flags, hasz);
#endif
	/* Initialization of the variable-length part */
	size_t *offsets = temporals_offsets_ptr(result);
	size_t pos = 0;	
	for (int i = 0; i < newcount; i++)
	{
		memcpy(((char *) result) + pdata + pos, newsequences[i], VARSIZE(newsequences[i]));
		offsets[i] = pos;
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
		offsets[newcount] = pos;
	}
	if (normalize && count > 1)
	{
		for (int i = 0; i < newcount; i++)
			pfree(newsequences[i]);
		pfree(newsequences);
	}
	return result;
}

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
temporals_find_timestamp(TemporalS *ts, TimestampTz t, int *pos) 
{
	int first = 0;
	int last = ts->count - 1;
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
		if (timestamp_cmp_internal(t, seq->period.lower) <= 0)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (timestamp_cmp_internal(t, seq->period.upper) >= 0)
		middle++;
	*pos = middle;
	return false;
}

bool 
temporalseqarr_find_timestamp(TemporalSeq **sequences, int from, int count, 
	TimestampTz t, int *pos) 
{
	int first = from;
	int last = count - 1;
	int middle = 0; /* make compiler quiet */
	TemporalSeq *seq = NULL; /* make compiler quiet */
	while (first <= last) 
	{
		middle = (first + last)/2;
		seq = sequences[middle];
		if (contains_period_timestamp_internal(&seq->period, t))
		{
			*pos = middle;
			return true;
		}
		if (timestamp_cmp_internal(t, seq->period.lower) <= 0)
			last = middle - 1;
		else
			first = middle + 1;	
	}
	if (timestamp_cmp_internal(t, seq->period.upper) >= 0)
		middle++;
	*pos = middle;
	return false;
}

/* Copy a TemporalS */
TemporalS *
temporals_copy(TemporalS *ts)
{
	TemporalS *result = palloc0(VARSIZE(ts));
	memcpy(result, ts, VARSIZE(ts));
	return result;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/* 
 * Intersection of a TemporalSeq and a TemporalInst values. 
 */
bool
intersection_temporals_temporalinst(TemporalS *ts, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	TemporalInst *inst1 = temporals_at_timestamp(ts, inst->t);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = temporalinst_copy(inst1);
	return true;
}

bool
intersection_temporalinst_temporals(TemporalInst *inst, TemporalS *ts, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	return intersection_temporals_temporalinst(ts, inst, inter2, inter1);
}

/* 
 * Intersection of a TemporalS and a TemporalI values. Each value keeps the instants 
 * in the intersection of their time spans.
 */
bool
intersection_temporals_temporali(TemporalS *ts, TemporalI *ti, 
	TemporalI **inter1, TemporalI **inter2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts);
	temporali_timespan(&p2, ti);
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
		pfree(instants1); pfree(instants2); 
		return false;
	}
	
	*inter1 = temporali_from_temporalinstarr(instants1, k);
	*inter2 = temporali_from_temporalinstarr(instants2, k);	
	for (int i = 0; i < k; i++) 
		pfree(instants1[i]);
	pfree(instants1); pfree(instants2); 
	return true;
}


bool
intersection_temporali_temporals(TemporalI *ti, TemporalS *ts,
	TemporalI **inter1, TemporalI **inter2)
{
	return intersection_temporals_temporali(ts, ti, inter2, inter1);
}

/* 
 * Intersection of a TemporalS and a TemporalSeq values. 
 */

bool
intersection_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	TemporalS **inter1, TemporalS **inter2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
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
		if (timestamp_cmp_internal(seq->period.upper, seq1->period.upper) < 0 ||
			(timestamp_cmp_internal(seq->period.upper, seq1->period.upper) == 0 &&
			(seq->period.upper_inc == seq->period.lower_inc || 
			(!seq->period.upper_inc && seq->period.lower_inc))))
			break;
	}
	if (k == 0)
	{
		pfree(sequences); 
		return false;
	}
	
	*inter2 = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++) 
		pfree(sequences[i]);
	pfree(sequences); 
	return true;
}

bool
intersection_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
	TemporalS **inter1, TemporalS **inter2)
{
	return intersection_temporals_temporalseq(ts, seq, inter2, inter1);
}

/* 
 * Intersection two TemporalS values. 
 */

bool
intersection_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	TemporalS **inter1, TemporalS **inter2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
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
	
	*inter1 = temporals_from_temporalseqarr(sequences1, k, false);
	*inter2 = temporals_from_temporalseqarr(sequences2, k, false);
	for (int i = 0; i < k; i++) 
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
synchronize_temporals_temporalseq(TemporalS *ts, TemporalSeq *seq, 
	TemporalS **sync1, TemporalS **sync2, bool crossings)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p;
	temporals_timespan(&p, ts);
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
		if (timestamp_cmp_internal(seq->period.upper, seq1->period.upper) < 0 ||
			(timestamp_cmp_internal(seq->period.upper, seq1->period.upper) == 0 &&
			(seq->period.upper_inc == seq->period.lower_inc || 
			(!seq->period.upper_inc && seq->period.lower_inc))))
			break;
	}
	if (k == 0)
	{
		pfree(sequences1); pfree(sequences2); 
		return false;
	}
	
	*sync1 = temporals_from_temporalseqarr(sequences1, k, false);
	*sync2 = temporals_from_temporalseqarr(sequences2, k, false);
	for (int i = 0; i < k; i++) 
	{
		pfree(sequences1[i]); pfree(sequences2[i]);
	}
	pfree(sequences1); pfree(sequences2); 
	return true;
}

bool
synchronize_temporalseq_temporals(TemporalSeq *seq, TemporalS *ts,
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
synchronize_temporals_temporals(TemporalS *ts1, TemporalS *ts2, 
	TemporalS **sync1, TemporalS **sync2, bool crossings)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
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
	
	*sync1 = temporals_from_temporalseqarr(sequences1, k, false);
	*sync2 = temporals_from_temporalseqarr(sequences2, k, false);
	for (int i = 0; i < k; i++) 
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
temporals_to_string(TemporalS *ts, char *(*value_out)(Oid, Datum))
{
	char **strings = palloc((int) (sizeof(char *) * ts->count));
	size_t outlen = 0;

	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		strings[i] = temporalseq_to_string(seq, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	char *result = palloc(outlen + 3);
	result[outlen] = '\0';
	result[0] = '{';
	size_t pos = 1;
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
temporals_write(TemporalS *ts, StringInfo buf)
{
	pq_sendint(buf, ts->count, 4);
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
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, false);

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
tints_as_tfloats(TemporalS *ts)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tintseq_as_tfloatseq(temporals_seq_n(ts, 0));

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tintseq_as_tfloatseq1(&sequences[k], seq);
		k += countstep;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	 pfree(sequences); 
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalS *
temporalinst_as_temporals(TemporalInst *inst)
{
	TemporalSeq *seq = temporalseq_from_temporalinstarr(&inst, 1, 
		true, true, false);
	TemporalS *result = temporals_from_temporalseqarr(&seq, 1, false);
	pfree(seq);
	return result;
}

TemporalS *
temporali_as_temporals(TemporalI *ti)
{
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		sequences[i] = temporalseq_from_temporalinstarr(&inst, 1, 
			true, true, false);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, ti->count, false);
	pfree(sequences);
	return result;
}

TemporalS *
temporalseq_as_temporals(TemporalSeq *seq)
{
	return temporals_from_temporalseqarr(&seq, 1, false);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* Values of a discrete TemporalS */

ArrayType *
tempdiscs_values(TemporalS *ts)
{
	Datum **values = palloc(sizeof(Datum *) * ts->count);
	int *countvalues = palloc0(sizeof(int) * ts->count);
	int count = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		values[i] = tempdiscseq_values1(seq);
		countvalues[i] = seq->count;
		count += seq->count;
	}
	Datum *allvalues = palloc(sizeof(Datum *) * count);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		for (int j = 0; j < countvalues[i]; j ++)
			allvalues[k++] = values[i][j];
		pfree(values[i]);
	}
	datum_sort(allvalues, count, ts->valuetypid);
	int newcount = datum_remove_duplicates(allvalues, count, ts->valuetypid);
	ArrayType *result = datumarr_to_array(allvalues, newcount, ts->valuetypid);
	pfree(values); pfree(countvalues); pfree(allvalues);
	return result;
}

/* Ranges of a TemporalS float */

ArrayType *
tfloats_ranges(TemporalS *ts)
{
	RangeType **ranges = palloc(sizeof(RangeType *) * ts->count);
	for (int i = 0; i < ts->count; i++) 
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		ranges[i] = tfloatseq_range(seq);
	}
	int count = ts->count;
	RangeType **normranges = rangearr_normalize(ranges, &count);
	rangearr_sort(normranges, count);
	ArrayType *result = rangearr_to_array(normranges, count, 
		type_oid(T_FLOATRANGE));

	for (int i = 0; i < ts->count; i++)
		pfree(ranges[i]);
	pfree(ranges);
	for (int i = 0; i < count; i++)
		pfree(normranges[i]);
	pfree(normranges);
	
	return result;
}

/* Bounding box range of a temporal number */

RangeType *
tnumbers_value_range(TemporalS *ts)
{
	BOX *box = temporals_bbox_ptr(ts);
	Datum min, max;
	if (ts->valuetypid == INT4OID)
	{
		min = Int32GetDatum(box->low.x);
		max = Int32GetDatum(box->high.x);
	}
	else if (ts->valuetypid == FLOAT8OID)
	{
		min = Float8GetDatum(box->low.x);
		max = Float8GetDatum(box->high.x);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return range_make(min, max, true, true, ts->valuetypid);
}

/* Range of a TemporalS expressed as floatrange */

RangeType *
tnumbers_floatrange(TemporalS *ts)
{
	if (ts->valuetypid == INT4OID)
	{
		RangeType *range = tnumbers_value_range(ts);
		RangeType *result = numrange_to_floatrange_internal(range);
		pfree(range);
		return result;
	}
	else if (ts->valuetypid == FLOAT8OID)
		return tnumbers_value_range(ts);
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
}

/* Minimum value */

Datum
temporals_min_value(TemporalS *ts)
{
	Oid valuetypid = ts->valuetypid;
	if (valuetypid == INT4OID)
	{
		BOX *box = temporals_bbox_ptr(ts);
		return Int32GetDatum((int)(box->low.x));
	}
	if (valuetypid == FLOAT8OID)
	{
		BOX *box = temporals_bbox_ptr(ts);
		return Float8GetDatum(box->low.x);
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
temporals_max_value(TemporalS *ts)
{
	Oid valuetypid = ts->valuetypid;
	if (valuetypid == INT4OID)
	{
		BOX *box = temporals_bbox_ptr(ts);
		return Int32GetDatum((int)(box->high.x));
	}
	if (valuetypid == FLOAT8OID)
	{
		BOX *box = temporals_bbox_ptr(ts);
		return Float8GetDatum(box->high.x);
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
temporals_get_time(TemporalS *ts)
{
	Period **periods = palloc(sizeof(Period *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		periods[i] = &seq->period;
	}
	PeriodSet *result = periodset_from_periodarr_internal(periods, 
		ts->count, false);
	pfree(periods);
	return result;
}

/* Duration */

Datum
temporals_duration(TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	Datum result = call_function2(timestamp_mi, 
		seq->period.upper, seq->period.lower);
	for (int i = 1; i < ts->count; i++)
	{
		seq = temporals_seq_n(ts, i);
		Datum interval1 = call_function2(timestamp_mi, 
			seq->period.upper, seq->period.lower);
		Datum interval2 = call_function2(interval_pl, result, interval1);
		pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
		result = interval2;
	}
	return result;
}

/* Duration of the TemporalS as a double */

double
temporals_duration_time(TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		double lower = (double)(seq->period.lower);
		double upper = (double)(seq->period.upper);
		result += (upper - lower);
	}
	return result;
}

/* Bounding period on which the temporal value is defined */

void
temporals_timespan(Period *p, TemporalS *ts)
{
	TemporalSeq *start = temporals_seq_n(ts, 0);
	TemporalSeq *end = temporals_seq_n(ts, ts->count - 1);
	period_set(p, start->period.lower, end->period.upper, 
		start->period.lower_inc, end->period.upper_inc);
}

/* Sequences */

TemporalSeq **
temporals_sequencearr(TemporalS *ts)
{
	TemporalSeq **result = palloc(sizeof(TemporalSeq *) * ts->count);
	for (int i = 0; i < ts->count; i++) 
		result[i] = temporals_seq_n(ts, i);
	return result;
}

ArrayType *
temporals_sequences_internal(TemporalS *ts)
{
	TemporalSeq **sequences = temporals_sequencearr(ts);
	ArrayType *result = temporalarr_to_array((Temporal **)sequences, ts->count);
	pfree(sequences);
	return result;
}

/* Number of distinct instants */

int
temporals_num_instants(TemporalS *ts)
{
	int result = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		result += seq->count;
	}
	return result;
}

/* N-th instant */

TemporalInst *
temporals_instant_n(TemporalS *ts, int n)
{
	if (n < 1) 
		return NULL;
	
	TemporalInst *result = NULL;
	int i = 0, count = 0, prevcount = 0;
	while (i < ts->count && prevcount < n)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		count += seq->count;
		if (prevcount <= n && n <= count)
		{
			result = temporalseq_inst_n(seq, n - prevcount - 1);
			break;
		}
		prevcount = count;
		i++;
	}
	return result;
}

/* Distinct instants */

TemporalInst **
temporals_instants1(TemporalS *ts, int *count)
{
	TemporalInst ***instants = palloc(sizeof(TemporalInst *) * ts->count);
	int *countinstants = palloc0(sizeof(int) * ts->count);
	int totalinstants = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		instants[i] = temporalseq_instants(seq);
		countinstants[i] = seq->count;
		totalinstants += seq->count;
	}
	TemporalInst **result = palloc(sizeof(TemporalInst *) * totalinstants);
	int k = 0;
	for (int i = 0; i < ts->count; i++)
		for (int j = 0; j < countinstants[i]; j ++)
			result[k++] = instants[i][j];
	
	pfree(instants); pfree(countinstants);
	*count = k;
	return result;
}

ArrayType *
temporals_instants(TemporalS *ts)
{
	int count;
	TemporalInst **instants = temporals_instants1(ts, &count);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, count);
	pfree(instants);
	return result;
}

/* Start timestamptz */

TimestampTz
temporals_start_timestamp(TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	return seq->period.lower;
}

/* End timestamptz */

TimestampTz
temporals_end_timestamp(TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, ts->count - 1);
	return seq->period.upper;
}

/* Number of distinct timestamps */

int
temporals_num_timestamps(TemporalS *ts)
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
		lasttime = temporalseq_inst_n(seq, seq->count-1)->t;
		first = false;
	}
	return result;
}

/* N-th distinct timestamp */

bool
temporals_timestamp_n(TemporalS *ts, int n, TimestampTz *result)
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
		prev = temporalseq_inst_n(seq, seq->count-1)->t;
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
temporals_timestamps1(TemporalS *ts, int *count)
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
	timestamp_sort(result, totaltimes);
	totaltimes = timestamp_remove_duplicates(result, totaltimes);
	
	pfree(times); pfree(counttimes);
	
	*count = totaltimes;
	return result;
}

ArrayType *
temporals_timestamps(TemporalS *ts)
{
	int count;
	TimestampTz *times = temporals_timestamps1(ts, &count);
	ArrayType *result = timestamparr_to_array(times, count);
	pfree(times);
	return result;
}

/* Is the temporal value ever equal to the value? */

bool
temporals_ever_equals(TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporals_bbox(&box1, ts);
		base_to_box(&box2, value, ts->valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
		if (temporalseq_ever_equals(temporals_seq_n(ts, i), value))
			return true;
	return false;
}

/* Is the temporal value always equal to the value? */

bool
temporals_always_equals(TemporalS *ts, Datum value)
{
	/* Bounding box test */
	if (ts->valuetypid == INT4OID || ts->valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporals_bbox(&box1, ts);
		base_to_box(&box2, value, ts->valuetypid);
		if (same_box_box_internal(&box1, &box2))
			return true;
		else
			return false;
	}

	for (int i = 0; i < ts->count; i++) 
		if (!temporalseq_always_equals(temporals_seq_n(ts, i), value))
			return false;
	return true;
}

/* Shift the time span of a temporal value by an interval */

TemporalS *
temporals_shift(TemporalS *ts, Interval *interval)
{
	TemporalS *result = temporals_copy(ts);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(result, i);
		for (int j = 0; j < seq->count; j++)
		{
			TemporalInst *inst = temporalseq_inst_n(seq, j);
			inst->t = DatumGetTimestampTz(
				DirectFunctionCall2(timestamptz_pl_interval,
				TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
		}
	}
	return result;
}

/* Is the TemporalS continuous in value? */

bool
temporals_continuous_value_internal(TemporalS *ts)
{
	TemporalSeq *seq1 = temporals_seq_n(ts, 0);
	Oid valuetypid = seq1->valuetypid;
	for (int i = 1; i < ts->count; i++)
	{
		TemporalSeq *seq2 = temporals_seq_n(ts, i);
		Datum value1 = temporalinst_value(temporalseq_inst_n(seq1, seq1->count - 1));
		Datum value2 = temporalinst_value(temporalseq_inst_n(seq1, 0));
		if (datum_ne(value1, value2, valuetypid))
			return false;
		seq1 = seq2;
	}
	return true;
}

/* Is the TemporalS continuous in time? */

bool
temporals_continuous_time_internal(TemporalS *ts)
{
	TemporalSeq *seq1 = temporals_seq_n(ts, 0);
	for (int i = 1; i < ts->count; i++)
	{
		TemporalSeq *seq2 = temporals_seq_n(ts, i);
		if (timestamp_cmp_internal(seq1->period.upper, seq2->period.lower) != 0)
			return false;
		seq1 = seq2;
	}
	return true;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

TemporalS *
temporals_at_value(TemporalS *ts, Datum value)
{
	Oid valuetypid = ts->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporals_bbox(&box1, ts);
		base_to_box(&box2, value, valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return NULL;
	}

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_at_value(temporals_seq_n(ts, 0), value);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = temporalseq_at_value2(&sequences[k], seq, value);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);	
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	 pfree(sequences); 
	return result;
}

/* Restriction to the complement of a value */

TemporalS *
temporals_minus_value(TemporalS *ts, Datum value)
{
	Oid valuetypid = ts->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporals_bbox(&box1, ts);
		base_to_box(&box2, value, valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return temporals_copy(ts);
	}

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_value(temporals_seq_n(ts, 0), value);

	/* General case */
	int count;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(ts->flags))
		count = ts->totalcount;
	else 
		count = ts->totalcount * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * count);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = temporalseq_minus_value2(&sequences[k], seq, value);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
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
temporals_at_values(TemporalS *ts, Datum *values, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_at_values(temporals_seq_n(ts, 0), values, count);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount * count);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = temporalseq_at_values1(&sequences[k], seq, values, count);
		k += countstep;
	}
	if (k == 0) 
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
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
temporals_minus_values(TemporalS *ts, Datum *values, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_values(temporals_seq_n(ts, 0), values, count);

	/* General case */
	int maxcount;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(ts->flags))
		maxcount = ts->totalcount * count;
	else 
		maxcount = ts->totalcount * count *2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);	
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = temporalseq_minus_values1(&sequences[k], seq, values, count);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}

	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	 pfree(sequences); 
	return result;
}

/*
 * Restriction to a range.
 */
TemporalS *
tnumbers_at_range(TemporalS *ts, RangeType *range)
{
	/* Bounding box test */
	BOX box1, box2;
	temporals_bbox(&box1, ts);
	range_to_box(&box2, range);
	if (!overlaps_box_box_internal(&box1, &box2))
		return NULL;

	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_at_range(temporals_seq_n(ts, 0), range);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tnumberseq_at_range2(&sequences[k], seq, range);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	 pfree(sequences	); 
	return result;
}

/*
 * Restriction to the complement of range.
 */
TemporalS *
tnumbers_minus_range(TemporalS *ts, RangeType *range)
{
	/* Bounding box test */
	BOX box1, box2;
	temporals_bbox(&box1, ts);
	range_to_box(&box2, range);
	if (!overlaps_box_box_internal(&box1, &box2))
		return temporals_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_minus_range(temporals_seq_n(ts, 0), range);

	/* General case */
	int maxcount;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(ts->flags))
		maxcount = ts->totalcount;
	else 
		maxcount = ts->totalcount * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tnumberseq_minus_range1(&sequences[k], seq, range);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
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
tnumbers_at_ranges(TemporalS *ts, RangeType **ranges, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_at_ranges(temporals_seq_n(ts, 0), ranges, count);

	/* General case */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->totalcount * count);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tnumberseq_at_ranges1(&sequences[k], seq, ranges, count);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
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
tnumbers_minus_ranges(TemporalS *ts, RangeType **ranges, int count)
{
	/* Singleton sequence set */
	if (ts->count == 1)
		return tnumberseq_minus_ranges(temporals_seq_n(ts, 0), ranges, count);

	/* General case */
	int maxcount;
	if (! MOBDB_FLAGS_GET_CONTINUOUS(ts->flags))
		maxcount = ts->totalcount;
	else 
		maxcount = ts->totalcount * 2;
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * maxcount);
	int k = 0, countstep;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		countstep = tnumberseq_minus_ranges1(&sequences[k], seq, ranges, count);
		k += countstep;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/* Restriction to the minimum value */

static int
temporalseqarr_remove_duplicates(TemporalSeq **sequences, int count)
{
	if (count == 0)
		return 0;
	int newcount = 0;
	for (int i = 1; i < count; i++) 
		if (temporalseq_ne(sequences[newcount], sequences[i]))
			sequences[++ newcount] = sequences[i];
		else 
			pfree(sequences[i]);
	return newcount+1;
}
static TemporalS *
temporals_at_minmax(TemporalS *ts, Datum value)
{
	TemporalS *result = temporals_at_value(ts, value);
	/* If minimum/maximum is at an exclusive bound */
	if (result == NULL)
	{
		TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * ts->count * 2);
		int k = 0, countstep;
		for (int i = 0; i < ts->count; i++)
		{
			TemporalSeq *seq = temporals_seq_n(ts, i);
			countstep = temporalseq_at_minmax(&sequences[k], seq, value);
			k += countstep;
		}
		/* The minimum/maximum could be at the upper exclusive bound of one
		* sequence and at the lower exclusive bound of the next one
		* e.g., .... min@t) (min@t .... */
		temporalseqarr_sort(sequences, k);
		int count = temporalseqarr_remove_duplicates(sequences, k);
		result = temporals_from_temporalseqarr(sequences, count, true);
		for (int i = 0; i < count; i++)
			pfree(sequences[i]);
		pfree(sequences);	
	}
	return result;
}

TemporalS *
temporals_at_min(TemporalS *ts)
{
	/* General case */
	Datum minvalue = temporals_min_value(ts);
	return temporals_at_minmax(ts, minvalue);
}

/* Restriction to the complement of the minimum value */

TemporalS *
temporals_minus_min(TemporalS *ts)
{
	Datum minvalue = temporals_min_value(ts);
	return temporals_minus_value(ts, minvalue);
}

/* Restriction to the maximum value */
 
TemporalS *
temporals_at_max(TemporalS *ts)
{
	Datum maxvalue = temporals_max_value(ts);
	return temporals_at_minmax(ts, maxvalue);
}

/* Restriction to the complement of the maximum value */

TemporalS *
temporals_minus_max(TemporalS *ts)
{
	Datum maxvalue = temporals_max_value(ts);
	return temporals_minus_value(ts, maxvalue);
}

/*
 * Restriction to a timestamp.
 */
TemporalInst *
temporals_at_timestamp(TemporalS *ts, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporals_timespan(&p, ts);
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
temporals_minus_timestamp(TemporalS *ts, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporals_timespan(&p, ts);
	if (!contains_period_timestamp_internal(&p, t))
		return temporals_copy(ts);

	/* Singleton sequence set */
	if (ts->count == 1)
		return temporalseq_minus_timestamp(temporals_seq_n(ts, 0), t);

	/* General case */
	/* At most one composing sequence can be split into two */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * (ts->count + 1));
	int k = 0;
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		int count = temporalseq_minus_timestamp1(&sequences[k], seq, t);
		k += count;
		// if (timestamp_cmp_internal(t, seq->period.upper) < 0)
		// 	break;
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

/*
 * Value at a timestamp.
 * This function assumes a bounding box test has been done before.
 */
bool
temporals_value_at_timestamp(TemporalS *ts, TimestampTz t, Datum *result)
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
temporals_at_timestampset(TemporalS *ts1, TimestampSet *ts2)
{
	/* Bounding box test */
	Period p1;
	temporals_timespan(&p1, ts1);
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
		if (timestamp_cmp_internal(t, seq->period.lower) <= 0)
			i++;
		if (timestamp_cmp_internal(t, seq->period.upper) >= 0)
			j++;
	}
	if (count == 0)
	{
		pfree(instants);
		return NULL;
	}

	TemporalI *result = temporali_from_temporalinstarr(instants, count);
	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of a timestampset.
 */
TemporalS *
temporals_minus_timestampset(TemporalS *ts1, TimestampSet *ts2)
{
	/* Bounding box test */
	Period p1;
	temporals_timespan(&p1, ts1);
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

	TemporalS *result = temporals_from_temporalseqarr(sequences, k, true);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*
 * Restriction to a period.
 */
TemporalS *
temporals_at_period(TemporalS *ts, Period *p)
{
	/* Bounding box test */
	Period p1;
	temporals_timespan(&p1, ts);
	if (!overlaps_period_period_internal(&p1, p))
		return NULL;

	/* Singleton sequence set */
	if (ts->count == 1)
	{
		TemporalSeq *seq = temporalseq_at_period(temporals_seq_n(ts, 0), p);
		return temporals_from_temporalseqarr(&seq, 1, false);
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
		if (timestamp_cmp_internal(p->upper, seq->period.upper) < 0 ||
			(timestamp_cmp_internal(p->upper, seq->period.upper) == 0 &&
			 seq->period.upper_inc))
			break;
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;		
	}
	/* Since both the temporals and the period are normalized it is not 
	   necessary to normalize the result of the projection */	
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < l; i++)
		pfree(tofree[i]);
	pfree(sequences);
	return result;
}

/*
 * Restriction to the complement of a period.
 */
TemporalS *
temporals_minus_period(TemporalS *ts, Period *p)
{
	/* Bounding box test */
	Period p1;
	temporals_timespan(&p1, ts);
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
temporals_at_periodset(TemporalS *ts, PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporals_timespan(&p1, ts);
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
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences); 
	return result;
}

/*
 * Restriction to the complement of a period set.
 */

TemporalS *
temporals_minus_periodset(TemporalS *ts, PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporals_timespan(&p1, ts);
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
		/* The sequence and the period do not overlap, copy the sequence */
		if (!overlaps_period_period_internal(&seq->period, p2))
		{
			sequences[k++] = temporalseq_copy(seq);
			i++;
		}
		else
		{
			/* Find all periods in ps that overlap with seq
							  i
				|------------------------|  
					 |-----|  |-----|	  |---|
						j					l
			*/
			int l;
			for (l = j; l < ps->count; l++)
			{
				Period *p3 = periodset_per_n(ps, l);
				if (!overlaps_period_period_internal(&seq->period, p3))
					break;
			}
			int count = l - j;
			/* Compute the difference of the overlapping periods */
			int countstep = temporalseq_minus_periodset1(&sequences[k], seq,
				ps, j, count);
			k += countstep;
			i++;
			j = l;
		}
	}
	if (k == 0)
	{
		pfree(sequences);
		return NULL;
	}
	/* Since both the temporals and the periodset are normalized it is not 
	   necessary to normalize the result of the difference */
	TemporalS *result = temporals_from_temporalseqarr(sequences, k, false);
	for (int i = 0; i < k; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/* Does the temporal value intersect the timestamp? */

bool
temporals_intersects_timestamp(TemporalS *ts, TimestampTz t)
{
	int n;
	if (temporals_find_timestamp(ts, t, &n))
		return false;
	return true;
}

/* Does the temporal value intersect the timestamp set? */

bool
temporals_intersects_timestampset(TemporalS *ts, TimestampSet *ts1)
{
	for (int i = 0; i < ts1->count; i++)
		if (temporals_intersects_timestamp(ts, timestampset_time_n(ts1, i))) 
			return true;
	return false;
}

/* Does a TemporalS intersects a period? */

bool
temporals_intersects_period(TemporalS *ts, Period *p)
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
		if (timestamp_cmp_internal(p->upper, seq->period.upper) < 0)
			break;
	}
	return false;
}

/* Does the temporal value intersect the period set? */

bool
temporals_intersects_periodset(TemporalS *ts, PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (temporals_intersects_period(ts, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/* Does the two temporal values intersect on the time dimension? */

bool
temporals_intersects_temporalinst(TemporalS *ts, TemporalInst *inst)
{
	return temporals_intersects_timestamp(ts, inst->t);
}

bool
temporals_intersects_temporali(TemporalS *ts, TemporalI *ti)
{
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (temporals_intersects_timestamp(ts, inst->t))
			return true;
	}
	return false;
}

bool
temporals_intersects_temporalseq(TemporalS *ts, TemporalSeq *seq)
{
	return temporals_intersects_period(ts, &seq->period);
}

/* Does the temporal values intersect on the time dimension? */

bool
temporals_intersects_temporals(TemporalS *ts1, TemporalS *ts2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporals_timespan(&p1, ts1);
	temporals_timespan(&p2, ts2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;

	int i = 0, j = 0;
	while (i < ts1->count && j < ts2->count)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, j);
		if (overlaps_period_period_internal(&seq1->period, &seq2->period))
			return true;
		if (timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) == 0)
		{
			i++; j++;
		}
		else if (timestamp_cmp_internal(seq1->period.upper, seq2->period.upper) < 0)
			i++;
		else 
			j++;
	}
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

/* Integral of the temporal integer */

double
tints_integral(TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tintseq_integral(temporals_seq_n(ts, i)); 
	return result;
}

/* Integral of the temporal float */

double
tfloats_integral(TemporalS *ts)
{
	double result = 0;
	for (int i = 0; i < ts->count; i++)
		result += tfloatseq_integral(temporals_seq_n(ts, i)); 
	return result;
}

/* Time-weighted average of the temporal integer */

double
tints_twavg(TemporalS *ts)
{
	double duration = temporals_duration_time(ts);
	double result;
	if (duration == 0)
	{
		result = 0;
		for (int i = 0; i < ts->count; i++)
			result += tintseq_twavg(temporals_seq_n(ts, i)); 
		return result / ts->count;
	}
	else
		result = tints_integral(ts) / duration;
	return result;
}

/* Time-weighted average of the temporal float */

double
tfloats_twavg(TemporalS *ts)
{
	double duration = temporals_duration_time(ts);
	double result;
	if (duration == 0)
	{
		result = 0;
		for (int i = 0; i < ts->count; i++)
			result += tfloatseq_twavg(temporals_seq_n(ts, i)); 
		return result / ts->count;
	}
	else
		result = tfloats_integral(ts) / duration;
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
temporals_eq(TemporalS *ts1, TemporalS *ts2)
{
	/* If number of sequences are not equal */
	if (ts1->count != ts2->count)
		return false;

	/* If bounding boxes are not equal */
	void *box1 = temporals_bbox_ptr(ts1);
	void *box2 = temporals_bbox_ptr(ts2);
	if (!temporal_bbox_eq(ts1->valuetypid, box1, box2))
		return false;

	/* We need to compare the composing sequences */
	for (int i = 0; i < ts1->count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		if (!temporalseq_eq(seq1, seq2))
			return false;
	}
	return true;
}

/* 
 * Inequality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
temporals_ne(TemporalS *ts1, TemporalS *ts2)
{
	return !temporals_eq(ts1, ts2);
}

/* 
 * B-tree comparator
 */

int
temporals_cmp(TemporalS *ts1, TemporalS *ts2)
{
	int count = Min(ts1->count, ts2->count);
	int result;
	for (int i = 0; i < count; i++)
	{
		TemporalSeq *seq1 = temporals_seq_n(ts1, i);
		TemporalSeq *seq2 = temporals_seq_n(ts2, i);
		result = temporalseq_cmp(seq1, seq2);
		if (result) 
			return result;
	}
	/* The first count sequences of both temporals are equal */
	if (ts1->count < ts2->count) /* ts1 has less sequences than ts2 */
		return -1;
	else if (ts2->count < ts1->count) /* ts2 has less sequences than ts1 */
		return 1;
	else
		return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of  
 * the elements.
 *****************************************************************************/

uint32
temporals_hash(TemporalS *ts)
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
