/*****************************************************************************
 *
 * TemporalI.c
 *	  Basic functions for temporal instants.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
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
 * The memory structure of a TemporalI with, e.g., 3 instants is as follows
 *
 * 	------------------------------------------------------
 * 	( TemporalI | offset_0 | offset_1 | offset_2 )_X | ...
 *	------------------------------------------------------
 *	----------------------------------------------------------
 *	( TemporalInst_0 )_X | ( TemporalInst_1 )_X | ( bbox )_X | 
 *	----------------------------------------------------------
 *
 * where the X are unused bytes added for double padding, offset_0 to offset_1
 * are offsets for the corresponding instants, and offset_2 is the offset for 
 * the bounding box.
 */

/* Pointer to the offset array of a TemporalI */

static size_t *
temporali_offsets_ptr(TemporalI *ti)
{
	return (size_t *) (((char *)ti) + sizeof(TemporalI));
}

/* Pointer to the first TemporalInst */

static char * 
temporali_data_ptr(TemporalI *ti) 
{
	return (char *)ti + double_pad(sizeof(TemporalI) + 
		sizeof(size_t) * (ti->count+1));
}

/* N-th TemporalInst of a TemporalI */

TemporalInst *
temporali_inst_n(TemporalI *ti, int index)
{
	size_t *offsets = temporali_offsets_ptr(ti);
	return (TemporalInst *) (temporali_data_ptr(ti) + offsets[index]);
}

/* Pointer to the bounding box of a TemporalI */

void * 
temporali_bbox_ptr(TemporalI *ti) 
{
    size_t *offsets = temporali_offsets_ptr(ti);
    return temporali_data_ptr(ti) + offsets[ti->count];
}

/* Copy the bounding box of a TemporalI in the first argument */

void 
temporali_bbox(void *box, TemporalI *ti) 
{
    void *box1 = temporali_bbox_ptr(ti);
	size_t bboxsize = temporal_bbox_size(ti->valuetypid);
	memcpy(box, box1, bboxsize);
    return;
}

/* Construct a TemporalI from an array of TemporalInst */

TemporalI *
temporali_from_temporalinstarr(TemporalInst **instants, int count)
{
	Oid valuetypid = instants[0]->valuetypid;
	/* Test the validity of the instants */
	if (count < 1)
		ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
			errmsg("A temporal instant set must have at least one temporal instant")));
#ifdef WITH_POSTGIS
	bool isgeo = false, hasz;
	int srid;
	if (valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_GEOGRAPHY))
	{
		isgeo = true;
		hasz = MOBDB_FLAGS_GET_Z(instants[0]->flags);
		srid = tpoint_srid_internal((Temporal *)instants[0]);
	}
#endif
	for (int i = 1; i < count; i++)
	{
		if (timestamp_cmp_internal(instants[i-1]->t, instants[i]->t) >= 0)
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
				errmsg("Invalid timestamps for temporal value")));
#ifdef WITH_POSTGIS
		if (isgeo)
		{
			if (tpoint_srid_internal((Temporal *)instants[i]) != srid)
				ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
					errmsg("All geometries composing a temporal point must be of the same srid")));
			if (MOBDB_FLAGS_GET_Z(instants[i]->flags) != hasz)
				ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION), 
					errmsg("All geometries composing a temporal point must be of the same dimensionality")));
		}
#endif
	}

	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < count; i++)
		memsize += double_pad(VARSIZE(instants[i]));
	/* Add the size of the struct and the offset array */
	size_t pdata = double_pad(sizeof(TemporalI) + (count + 1) * sizeof(size_t));
	/* Create the TemporalI */
	TemporalI *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = count;
	result->valuetypid = valuetypid;
	result->type = TEMPORALI;
#ifdef WITH_POSTGIS
	if (isgeo)
		MOBDB_FLAGS_SET_Z(result->flags, hasz);
#endif
	/* Initialization of the variable-length part */
	size_t *offsets = temporali_offsets_ptr(result);
	size_t pos = 0;
	for (int i = 0; i < count; i++)
	{
		memcpy(((char *)result) + pdata + pos, instants[i], VARSIZE(instants[i]));
		offsets[i] = pos;
		pos += double_pad(VARSIZE(instants[i]));
	}
	/*
	 * Precompute the bounding box 
	 * Only external types have precomputed bounding box, internal types such
	 * as double2, double3, or double4 do not have one
	 */
	if (bboxsize != 0) 
	{
		void *bbox = ((char *) result) + pdata + pos;
		temporali_make_bbox(bbox, instants, count);
		offsets[count] = pos;
	}
	return result;
}

/* Copy a TemporalI */
TemporalI *
temporali_copy(TemporalI *ti)
{
	TemporalI *result = palloc0(VARSIZE(ti));
	memcpy(result, ti, VARSIZE(ti));
	return result;
}

/* Binary search of a timestamptz in a TemporalI or in an array of TemporalInst*/

int
temporali_find_timestamp(TemporalI *ti, TimestampTz t) 
{
	int first = 0;
	int last = ti->count - 1;
	int middle = (first + last)/2;
	while (first <= last) 
	{
		TemporalInst *inst = temporali_inst_n(ti, middle);
		if (timestamp_cmp_internal(t, inst->t) == 0)
			return middle;
		if (timestamp_cmp_internal(t, inst->t) < 0)
			last = middle - 1;
		else
			first = middle + 1;    
		middle = (first + last)/2;
	}
	return -1;
}

int 
temporalinstarr_find_timestamp(TemporalInst **array, int from, int count, 
	TimestampTz t) 
{
	int first = from;
	int last = count - 1;
	int middle = (first + last)/2;
	while (first <= last) 
	{
		if (timestamp_cmp_internal(t, array[middle]->t) == 0)
			return middle;
		if (timestamp_cmp_internal(t, array[middle]->t) < 0)
			last = middle - 1;
		else
			first = middle + 1;	
		middle = (first + last)/2;
	}
	return middle;
}

/* Range of a Temporali expressed as a floatrange */

RangeType *
tnumberi_floatrange(TemporalI *ti)
{
	BOX *box = temporali_bbox_ptr(ti);
	Datum min = Float8GetDatum(box->low.x);
	Datum max = Float8GetDatum(box->high.x);
	return range_make(min, max, true, true, FLOAT8OID);
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/* 
 * Intersection of a TemporalI and a TemporalInst values. 
 */

bool
intersection_temporali_temporalinst(TemporalI *ti, TemporalInst *inst, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	TemporalInst *inst1 = temporali_at_timestamp(ti, inst->t);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = temporalinst_copy(inst1);
	return true;
}

bool
intersection_temporalinst_temporali(TemporalInst *inst, TemporalI *ti, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	return intersection_temporali_temporalinst(ti, inst, inter2, inter1);
}

/* 
 * Intersection two TemporalI values. Each value keeps the instants 
 * in the intersection of their time spans.
 */

bool
intersection_temporali_temporali(TemporalI *ti1, TemporalI *ti2, 
	TemporalI **inter1, TemporalI **inter2)
{
	/* Test whether the bounding timespan of the two temporal values overlap */
	Period p1, p2;
	temporali_timespan(&p1, ti1);
	temporali_timespan(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;
	
	int count = Min(ti1->count, ti2->count);
	TemporalInst **instants1 = palloc(sizeof(TemporalInst *) * count);
	TemporalInst **instants2 = palloc(sizeof(TemporalInst *) * count);
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, j);
		if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		{
			instants1[k] = inst1;
			instants2[k++] = inst2;
			i++; j++;
		}
		else if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
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
	
	pfree(instants1); pfree(instants2); 

	return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Convert to string */
 
char*
temporali_to_string(TemporalI *ti, char *(*value_out)(Oid, Datum))
{
	char** strings = palloc((int) (sizeof(char *)) * ti->count);
	size_t outlen = 0;

	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		strings[i] = temporalinst_to_string(inst, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	char *result = palloc(outlen + 3);
	result[outlen] = '\0';
	result[0] = '{';
	size_t pos = 1;
	for (int i = 0; i < ti->count; i++)
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
temporali_write(TemporalI *ti, StringInfo buf)
{
	pq_sendint(buf, ti->count, 4);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
        temporalinst_write(inst, buf);
	}
}
 
/* Receive function */

TemporalI *
temporali_read(StringInfo buf, Oid valuetypid)
{
	int count = (int) pq_getmsgint(buf, 4);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
		instants[i] = temporalinst_read(buf, valuetypid);
	TemporalI *result = temporali_from_temporalinstarr(instants, count);

	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	
	return result;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/* Cast a temporal integer as a temporal float */

TemporalI *
tinti_as_tfloati(TemporalI *ti)
{
	TemporalI *result = temporali_copy(ti);
	result->valuetypid = FLOAT8OID;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(result, i);
		inst->valuetypid = FLOAT8OID;
		Datum *value_ptr = temporalinst_value_ptr(inst);
		*value_ptr = Float8GetDatum((double)DatumGetInt32(temporalinst_value(inst)));
	}
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalI *
temporalinst_as_temporali(TemporalInst *inst)
{
	return temporali_from_temporalinstarr(&inst, 1);
}

TemporalI *
temporalseq_as_temporali(TemporalSeq *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant set")));

	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	return temporali_from_temporalinstarr(&inst, 1);
}

TemporalI *
temporals_as_temporali(TemporalS *ts)
{
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		if (seq->count != 1)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Cannot transform input to a temporal instant set")));
	}
	
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TemporalSeq *seq = temporals_seq_n(ts, i);
		instants[i] = temporalseq_inst_n(seq, 0);
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, ts->count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Accessor functions 
 *****************************************************************************/

/* Set of values taken by the temporal value */

Datum *
temporali_values1(TemporalI *ti, int *count)
{
	Datum *result = palloc(sizeof(Datum *) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		result[i] = temporalinst_value(temporali_inst_n(ti, i));
	datum_sort(result, ti->count, ti->valuetypid);
	*count = datum_remove_duplicates(result, ti->count, ti->valuetypid);
	return result;
}

ArrayType *
temporali_values(TemporalI *ti)
{
	int count;
	Datum *values = temporali_values1(ti, &count);
	ArrayType *result = datumarr_to_array(values, count, ti->valuetypid);
	pfree(values);
	return result;
}

/* Set of ranges taken by the temporal value */

ArrayType *
tfloati_ranges(TemporalI *ti)
{
	int count;
	Datum *values = temporali_values1(ti, &count);
	RangeType **ranges = palloc(sizeof(RangeType *) * count);
	for (int i = 0; i < count; i++)
		ranges[i] = range_make(values[i], values[i], true, true, FLOAT8OID);
	ArrayType *result = rangearr_to_array(ranges, count, type_oid(T_FLOATRANGE));
	for (int i = 0; i < count; i++)
		pfree(ranges[i]);
	pfree(ranges); pfree(values);
	return result;
}

/* Get time */

PeriodSet *
temporali_get_time(TemporalI *ti)
{
	Period **periods = palloc(sizeof(Period *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		periods[i] = period_make(inst->t, inst->t, true, true);
	}
	PeriodSet *result = periodset_from_periodarr_internal(periods, ti->count, false);
	for (int i = 0; i < ti->count; i++)
		pfree(periods[i]);
	pfree(periods);
	return result;
}

/* Bounding box range of a temporal integer */

RangeType *
tnumberi_value_range(TemporalI *ti)
{
	BOX *box = temporali_bbox_ptr(ti);
	Datum min, max;
	if (ti->valuetypid == INT4OID)
	{
		min = Int32GetDatum((int)(box->low.x));
		max = Int32GetDatum((int)(box->high.x));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		min = Float8GetDatum(box->low.x);
		max = Float8GetDatum(box->high.x);
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	return range_make(min, max, true, true, ti->valuetypid);
}

/* Minimum value */

Datum
temporali_min_value(TemporalI *ti)
{
	if (ti->valuetypid == INT4OID)
	{
		BOX *box = temporali_bbox_ptr(ti);
		return Int32GetDatum((int)(box->low.x));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		BOX *box = temporali_bbox_ptr(ti);
		return Float8GetDatum(box->low.x);
	}
	else
	{
		Oid valuetypid = ti->valuetypid;
		Datum min = temporalinst_value(temporali_inst_n(ti, 0));
		int idx = 0;
		for (int i = 1; i < ti->count; i++)
		{
			Datum value = temporalinst_value(temporali_inst_n(ti, i));
			if (datum_lt(value, min, valuetypid))
			{
				min = value;
				idx = i;
			}
		}
		return temporalinst_value(temporali_inst_n(ti, idx));
	}
}

/* Maximum value */
 
Datum
temporali_max_value(TemporalI *ti)
{
	if (ti->valuetypid == INT4OID)
	{
		BOX *box = temporali_bbox_ptr(ti);
		return Int32GetDatum((int)(box->high.x));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		BOX *box = temporali_bbox_ptr(ti);
		return Float8GetDatum(box->high.x);
	}
	else
	{
		Oid valuetypid = ti->valuetypid;
		Datum max = temporalinst_value(temporali_inst_n(ti, 0));
		int idx = 0;
		for (int i = 1; i < ti->count; i++)
		{
			Datum value = temporalinst_value(temporali_inst_n(ti, i));
			if (datum_gt(value, max, valuetypid))
			{
				max = value;
				idx = i;
			}
		}
		return temporalinst_value(temporali_inst_n(ti, idx));
	}
}

/* Set of instants on which the temporal value is defined */

TimestampSet *
temporali_time(TemporalI *ti)
{
	TimestampTz *times = palloc(sizeof(Timestamp) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		times[i] = (temporali_inst_n(ti, i))->t;
	TimestampSet *result = timestampset_from_timestamparr_internal(times, ti->count);
	pfree(times);
	return result;
}

/* Bounding period on which the temporal value is defined */

void
temporali_timespan(Period *p, TemporalI *ti)
{
	TimestampTz lower = temporali_start_timestamp(ti);
	TimestampTz upper = temporali_end_timestamp(ti);
	return period_set(p, lower, upper, true, true);
}

/* Instants */

TemporalInst **
temporali_instantarr(TemporalI *ti)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		result[i] = temporali_inst_n(ti, i);
	return result;	
}

ArrayType *
temporali_instants(TemporalI *ti)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		instants[i] = temporali_inst_n(ti, i);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, ti->count);
	pfree(instants);
	return result;	
}

/* Start timestamptz */

TimestampTz
temporali_start_timestamp(TemporalI *ti)
{
	return (temporali_inst_n(ti, 0))->t;
}

/* End timestamptz */

TimestampTz
temporali_end_timestamp(TemporalI *ti)
{
	return (temporali_inst_n(ti, ti->count - 1))->t;
}

/* Set of instants on which the temporal value is defined */

ArrayType *
temporali_timestamps(TemporalI *ti)
{
	TimestampTz *times = palloc(sizeof(Timestamp) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		times[i] = (temporali_inst_n(ti, i))->t;
	ArrayType *result = timestamparr_to_array(times, ti->count);
	pfree(times);
	return result;
}

/* Is the temporal value ever equal to the value? */

bool
temporali_ever_equals(TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporali_bbox(&box1, ti);
		base_to_box(&box2, value, ti->valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return false;
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (datum_eq(valueinst, value, ti->valuetypid))
			return true;
	}
	return false;
}

/* Is the temporal value always equal to the value? */

bool
temporali_always_equals(TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporali_bbox(&box1, ti);
		base_to_box(&box2, value, ti->valuetypid);
		if (same_box_box_internal(&box1, &box2))
			return true;
		else
			return false;
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (datum_ne(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/* Shift the time span of a temporal value by an interval */

TemporalI *
temporali_shift(TemporalI *ti, Interval *interval)
{
	TemporalI *result = temporali_copy(ti);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(result, i);
		inst->t = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	}
	return result;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

TemporalI *
temporali_at_value(TemporalI *ti, Datum value)
{
	Oid valuetypid = ti->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporali_bbox(&box1, ti);
		base_to_box(&box2, value, valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return NULL;
	}

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporalinst_at_value(temporali_inst_n(ti, 0), 
			value);
		if (inst == NULL)
			return NULL;
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (datum_eq(value, temporalinst_value(inst), valuetypid)) 
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a value. */

TemporalI *
temporali_minus_value(TemporalI *ti, Datum value)
{
	Oid valuetypid = ti->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		BOX box1, box2;
		temporali_bbox(&box1, ti);
		base_to_box(&box2, value, valuetypid);
		if (!contains_box_box_internal(&box1, &box2))
			return temporali_copy(ti);
	}

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporalinst_minus_value(temporali_inst_n(ti, 0), 
			value);
		if (inst == NULL)
			return NULL;
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (datum_ne(value, temporalinst_value(inst), valuetypid))
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* 
 * Restriction to an array of values.
 * The function assumes that there are no duplicates values.
 */
 
TemporalI *
temporali_at_values(TemporalI *ti, Datum *values, int count)
{
	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = temporalinst_at_values(inst, values, count);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int newcount = 0;	
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		for (int j = 0; j < count; j++)
		{
			if (datum_eq(temporalinst_value(inst), values[j], ti->valuetypid))
			{
				instants[newcount++] = inst;
				break;
			}
		}
	}
	TemporalI *result = (newcount == 0) ? NULL :
		temporali_from_temporalinstarr(instants, newcount);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of an array of values
 * The function assumes that there are no duplicates values.
 */

TemporalI *
temporali_minus_values(TemporalI *ti, Datum *values, int count)
{
	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = temporalinst_minus_values(inst, values, count);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int newcount = 0;
	for (int i = 0; i < ti->count; i++)
	{
		bool found = false;
		TemporalInst *inst = temporali_inst_n(ti, i);
		for (int j = 0; j < count; j++)
		{
			if (datum_eq(temporalinst_value(inst), values[j], ti->valuetypid))
			{
				found = true;
				break;
			}
		}
		if (!found)
			instants[newcount++] = inst;
	}
	TemporalI *result = (newcount == 0) ? NULL :
		temporali_from_temporalinstarr(instants, newcount);
	pfree(instants);
	return result;
}

/* Restriction to a range. */

TemporalI *
tnumberi_at_range(TemporalI *ti, RangeType *range)
{
	/* Bounding box test */
	BOX box1, box2;
	temporali_bbox(&box1, ti);
	range_to_box(&box2, range);
	if (!overlaps_box_box_internal(&box1, &box2))
		return NULL;

	/* Singleton instant set */
	if (ti->count == 1)
		return temporali_copy(ti);

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		TemporalInst *inst1 = tnumberinst_at_range(inst, range);
		if (inst1 != NULL)
			instants[count++] = inst1;
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

/* Restriction to the complement of a range */

TemporalI *
tnumberi_minus_range(TemporalI *ti, RangeType *range)
{
	/* Bounding box test */
	BOX box1, box2;
	temporali_bbox(&box1, ti);
	range_to_box(&box2, range);
	if (!overlaps_box_box_internal(&box1, &box2))
		return temporali_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
		return NULL;

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int newcount = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		TemporalInst *inst1 = tnumberinst_minus_range(inst, range);
		if (inst1 != NULL)
			instants[newcount++] = inst1;
	}
	if (newcount == 0)
	{
		pfree(instants);
		return NULL;
	}

	TemporalI *result = temporali_from_temporalinstarr(instants, newcount);
	for (int i = 0; i < newcount; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the ranges */

TemporalI *
tnumberi_at_ranges(TemporalI *ti, RangeType **normranges, int count)
{
	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = tnumberinst_at_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int newcount = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		for (int j = 0; j < count; j++)
		{
			TemporalInst *inst1 = tnumberinst_at_range(inst, normranges[j]);
			if (inst1 != NULL)
			{
				instants[newcount++] = inst1;
				break;
			}
		}
	}
	if (newcount == 0) 
	{
		pfree(instants);
		return NULL;
	}
	
	TemporalI *result = temporali_from_temporalinstarr(instants, newcount);
	for (int i = 0; i < newcount; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the complement of ranges */

TemporalI *
tnumberi_minus_ranges(TemporalI *ti, RangeType **normranges, int count)
{
	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = tnumberinst_minus_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int newcount = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		for (int j = 0; j < count; j++)
		{
			TemporalInst *inst1 = tnumberinst_minus_range(inst, normranges[j]);
			if (inst1 != NULL)
			{
				instants[newcount++] = inst1;
				break;
			}
		}
	}
	if (newcount == 0)
	{
		pfree(instants);
		return NULL;
	}

	TemporalI *result = temporali_from_temporalinstarr(instants, newcount);
	for (int i = 0; i < newcount; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the minimum value */

TemporalI *
temporali_at_min(TemporalI *ti)
{
	Datum minvalue = temporali_min_value(ti);
	return temporali_at_value(ti, minvalue);	
}

/* Restriction to the complement of the minimum value */

TemporalI *
temporali_minus_min(TemporalI *ti)
{
	Datum minvalue = temporali_min_value(ti);
	return temporali_minus_value(ti, minvalue);	
}

/* Restriction to the maximum value */

TemporalI *
temporali_at_max(TemporalI *ti)
{
	Datum maxvalue = temporali_max_value(ti);
	return temporali_at_value(ti, maxvalue);	
}

/* Restriction to the complement of the maximum value */

TemporalI *
temporali_minus_max(TemporalI *ti)
{
	Datum maxvalue = temporali_max_value(ti);
	return temporali_minus_value(ti, maxvalue);	
}

/* 
 * Restriction to the timestamp
 * To be compatible with the corresponding functions for temporal sequences
 * that need to interpolate the value, it is necessary to return a copy of
 * the value 
 */

TemporalInst *
temporali_at_timestamp(TemporalI *ti, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporali_timespan(&p, ti);
	if (!contains_period_timestamp_internal(&p, t))
		return NULL;

	/* Singleton instant set */
	if (ti->count == 1)
		return temporalinst_copy(temporali_inst_n(ti, 0));

	/* General case */
	int n = temporali_find_timestamp(ti, t);
	if (n == -1)
		return NULL;
	TemporalInst *inst = temporali_inst_n(ti, n);
	return temporalinst_copy(inst);
}

/* 
 * Value at the timestamp
 * In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value 
 */

bool 
temporali_value_at_timestamp(TemporalI *ti, TimestampTz t, Datum *result)
{
	int n = temporali_find_timestamp(ti, t);
	if (n == -1)
		return false;		

	TemporalInst *inst = temporali_inst_n(ti, n);
	*result = temporalinst_value_copy(inst);
	return true;
}

/* Restriction to the complement of a timestamptz */

TemporalI *
temporali_minus_timestamp(TemporalI *ti, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporali_timespan(&p, ti);
	if (!contains_period_timestamp_internal(&p, t))
		return temporali_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
		return NULL;

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst= temporali_inst_n(ti, i);
		if (timestamp_cmp_internal(inst->t, t) != 0)
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* 
 * Restriction to a timestamp set
 */

TemporalI *
temporali_at_timestampset(TemporalI *ti, TimestampSet *ts)
{
	/* Bounding box test */
	Period p1;
	temporali_timespan(&p1, ti);
	Period *p2 = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&p1, p2))
		return NULL;

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = temporalinst_at_timestampset(inst, ts);
		if (inst1 == NULL)
			return NULL;

		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ts->count);
	int count = 0;
	int i = 0, j = 0;
	while (i < ts->count && j < ti->count) 
	{
		TemporalInst *inst = temporali_inst_n(ti, j);
		TimestampTz t = timestampset_time_n(ts, i);
		if (timestamp_cmp_internal(t, inst->t) == 0)
		{
			instants[count++] = inst;
			i++;
		}
		if (timestamp_cmp_internal(t, inst->t) < 0)
			i++;
		if (timestamp_cmp_internal(t, inst->t) > 0)
			j++;
	}	
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of a timestamp set
 */

TemporalI *
temporali_minus_timestampset(TemporalI *ti, TimestampSet *ts)
{
	/* Bounding box test */
	Period p1;
	temporali_timespan(&p1, ti);
	Period *p2 = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&p1, p2))
		return temporali_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = temporalinst_minus_timestampset(inst, ts);
		if (inst1 == NULL)
			return NULL;

		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	int i = 0, j = 0;
	while (i < ts->count && j < ti->count)
	{
		TemporalInst *inst = temporali_inst_n(ti, j);
		TimestampTz t = timestampset_time_n(ts, i);
		if (timestamp_cmp_internal(t, inst->t) <= 0)
			i++;
		else /* (timestamp_cmp_internal(t, inst->t) > 0) */
		{
			instants[count++] = inst;
			j++;
		}
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the period */

TemporalI *
temporali_at_period(TemporalI *ti, Period *period)
{
	/* Bounding box test */
	Period p;
	temporali_timespan(&p, ti);
	if (!overlaps_period_period_internal(&p, period))
		return NULL;

	/* Singleton instant set */
	if (ti->count == 1)
		return temporali_copy(ti);

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(period, inst->t))
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a period */

TemporalI *
temporali_minus_period(TemporalI *ti, Period *period)
{
	/* Bounding box test */
	Period p;
	temporali_timespan(&p, ti);
	if (!overlaps_period_period_internal(&p, period))
		return temporali_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
		return NULL;

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (!contains_period_timestamp_internal(period, inst->t))
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL : 
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to a period set */

TemporalI *
temporali_at_periodset(TemporalI *ti, PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporali_timespan(&p1, ti);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return NULL;

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = temporalinst_at_periodset(inst, ps);
		if (inst1 == NULL)
			return NULL;

		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_periodset_timestamp_internal(ps, inst->t))
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a period set */

TemporalI *
temporali_minus_periodset(TemporalI *ti, PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporali_timespan(&p1, ti);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return temporali_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TemporalInst *inst = temporali_inst_n(ti, 0);
		TemporalInst *inst1 = temporalinst_minus_periodset(inst, ps);
		if (inst1 == NULL)
			return NULL;

		pfree(inst1); 
		return temporali_copy(ti);
	}

	/* General case */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (!contains_periodset_timestamp_internal(ps, inst->t))
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_from_temporalinstarr(instants, count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

 /* Does the temporal value intersects the timestamp? */

bool
temporali_intersects_timestamp(TemporalI *ti, TimestampTz t)
{
	int n = temporali_find_timestamp(ti, t);
	if (n == -1)
		return false;
	return true;
}

/* Does the temporal value intersects the timestamp set? */

bool
temporali_intersects_timestampset(TemporalI *ti, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (temporali_intersects_timestamp(ti, timestampset_time_n(ts, i)))
			return true;
	return false;
}

/* Does the temporal value intersects the period? */

bool
temporali_intersects_period(TemporalI *ti, Period *period)
{
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (contains_period_timestamp_internal(period, inst->t))
			return true;
	}
	return false;
}

/* Does the temporal value intersects the period set? */

bool
temporali_intersects_periodset(TemporalI *ti, PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (temporali_intersects_period(ti, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/* Does the two temporal values intersect on the time dimension? */

bool
temporali_intersects_temporalinst(TemporalI *ti, TemporalInst *inst)
{
	return temporali_intersects_timestamp(ti, inst->t);
}

bool
temporali_intersects_temporali(TemporalI *ti1, TemporalI *ti2)
{
	for (int i = 0; i < ti2->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti2, i);
		if (temporali_intersects_timestamp(ti1, inst->t)) 
			return true;
	}
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

double
temporali_lavg(TemporalI *ti)
{
	double result = 0.0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		if (ti->valuetypid == INT4OID)
			result += (double)DatumGetInt32(temporalinst_value(inst));
		else
			result += DatumGetFloat8(temporalinst_value(inst));
	}
	return result / ti->count;
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
temporali_eq(TemporalI *ti1, TemporalI *ti2)
{
	/* If number of sequences are not equal */
	if (ti1->count != ti2->count)
		return false;
	/* If bounding boxes are not equal */
	size_t bboxsize = double_pad(temporal_bbox_size(ti1->valuetypid));
	void *box1 = temporali_bbox_ptr(ti1);
	void *box2 = temporali_bbox_ptr(ti2);
	if (memcmp(box1, box2, bboxsize))
		return false;
	
	/* We need to compare the composing instants */
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		if (!temporalinst_eq(inst1, inst2))
			return false;
	}
	return true;
}

/* 
 * Inequality operator
 */
bool
temporali_ne(TemporalI *ti1, TemporalI *ti2)
{
	return !temporali_eq(ti1, ti2);
}

/* 
 * B-tree comparator
 */

int
temporali_cmp(TemporalI *ti1, TemporalI *ti2)
{
	int count = Min(ti1->count, ti2->count);
	int result;
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		result = temporalinst_cmp(inst1, inst2);
		if (result) 
			return result;
	}
	/* The first count instants of both TemporalI values are equal */
	if (ti1->count < ti2->count) /* ti1 has less instants than ti2 */
		return -1;
	else if (ti2->count < ti1->count) /* ti2 has less instants than ti1 */
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
temporali_hash(TemporalI *ti)
{
	uint32 result = 1;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		uint32 inst_hash = temporalinst_hash(inst);
		result = (result << 5) - result + inst_hash;
	}
	return result;
}

/*****************************************************************************/
