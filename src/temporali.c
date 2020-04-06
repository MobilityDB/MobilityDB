/*****************************************************************************
 *
 * temporali.c
 *	  Basic functions for temporal instant sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporali.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>

#include "timetypes.h"
#include "timestampset.h"
#include "period.h"
#include "periodset.h"
#include "timeops.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "temporal_boxops.h"
#include "rangetypes_ext.h"

#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/* 
 * The memory structure of a TemporalI with, e.g., 2 instants is as follows
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

/* N-th TemporalInst of a TemporalI */

TemporalInst *
temporali_inst_n(const TemporalI *ti, int index)
{
	return (TemporalInst *) (
		(char *)(&ti->offsets[ti->count + 1]) + 	/* start of data */
			ti->offsets[index]);					/* offset */
}

/* Pointer to the bounding box of a TemporalI */

void * 
temporali_bbox_ptr(const TemporalI *ti)
{
	return (char *)(&ti->offsets[ti->count + 1]) +  /* start of data */
		ti->offsets[ti->count];						/* offset */
}

/* Copy the bounding box of a TemporalI in the first argument */

void 
temporali_bbox(void *box, const TemporalI *ti)
{
	void *box1 = temporali_bbox_ptr(ti);
	size_t bboxsize = temporal_bbox_size(ti->valuetypid);
	memcpy(box, box1, bboxsize);
}

/* Construct a TemporalI from an array of TemporalInst */

TemporalI *
temporali_make(TemporalInst **instants, int count)
{
	/* Test the validity of the instants */
	assert(count > 0);
	bool isgeo = (instants[0]->valuetypid == type_oid(T_GEOMETRY) ||
		instants[0]->valuetypid == type_oid(T_GEOGRAPHY));
	for (int i = 1; i < count; i++)
	{
		ensure_increasing_timestamps(instants[i - 1], instants[i]);
		if (isgeo)
		{
			ensure_same_srid_tpoint((Temporal *)instants[i - 1], (Temporal *)instants[i]);
			ensure_same_dimensionality_tpoint((Temporal *)instants[i - 1], (Temporal *)instants[i]);
		}
	}

	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(instants[0]->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < count; i++)
		memsize += double_pad(VARSIZE(instants[i]));
	/* Add the size of the struct and the offset array 
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalI) + count * sizeof(size_t));
	/* Create the TemporalI */
	TemporalI *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = count;
	result->valuetypid = instants[0]->valuetypid;
	result->duration = TEMPORALI;
	MOBDB_FLAGS_SET_LINEAR(result->flags, 
		MOBDB_FLAGS_GET_LINEAR(instants[0]->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(instants[0]->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(instants[0]->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	for (int i = 0; i < count; i++)
	{
		memcpy(((char *)result) + pdata + pos, instants[i], VARSIZE(instants[i]));
		result->offsets[i] = pos;
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
		result->offsets[count] = pos;
	}
	return result;
}

/* Consruct a TemporalI from a base value and a timestamp set */

TemporalI *
temporali_from_base_internal(Datum value, Oid valuetypid, const TimestampSet *ts)
{
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ts->count);
	for (int i = 0; i < ts->count; i++)
		instants[i] = temporalinst_make(value, timestampset_time_n(ts, i), valuetypid);
	TemporalI *result = temporali_make(instants, ts->count);
	for (int i = 0; i < ts->count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

PG_FUNCTION_INFO_V1(temporali_from_base);

PGDLLEXPORT Datum
temporali_from_base(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TemporalI *result = temporali_from_base_internal(value, valuetypid, ts);
	DATUM_FREE_IF_COPY(value, valuetypid, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_POINTER(result);
}

/* Append a TemporalInst to a TemporalI */

TemporalI *
temporali_append_instant(const TemporalI *ti, const TemporalInst *inst)
{
	/* Test the validity of the instant */
	assert(ti->valuetypid == inst->valuetypid);
	assert(MOBDB_FLAGS_GET_GEODETIC(ti->flags) == MOBDB_FLAGS_GET_GEODETIC(inst->flags));
	TemporalInst *inst1 = temporali_inst_n(ti, ti->count - 1);
	ensure_increasing_timestamps(inst1, inst);
	bool isgeo = (ti->valuetypid == type_oid(T_GEOMETRY) ||
		ti->valuetypid == type_oid(T_GEOGRAPHY));
	if (isgeo)
	{
		ensure_same_srid_tpoint((Temporal *)ti, (Temporal *)inst);
		ensure_same_dimensionality_tpoint((Temporal *)ti, (Temporal *)inst);
	}
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(ti->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < ti->count; i++)
		memsize += double_pad(VARSIZE(temporali_inst_n(ti, i)));
	memsize += double_pad(VARSIZE(inst));
	/* Add the size of the struct and the offset array 
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalI) + (ti->count + 1) * sizeof(size_t));
	/* Create the TemporalI */
	TemporalI *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = ti->count + 1;
	result->valuetypid = ti->valuetypid;
	result->duration = TEMPORALI;
	MOBDB_FLAGS_SET_LINEAR(result->flags,
		MOBDB_FLAGS_GET_LINEAR(inst->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(ti->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(ti->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	for (int i = 0; i < ti->count; i++)
	{
		inst1 = temporali_inst_n(ti, i);
		memcpy(((char *)result) + pdata + pos, inst1, VARSIZE(inst1));
		result->offsets[i] = pos;
		pos += double_pad(VARSIZE(inst1));
	}
	memcpy(((char *)result) + pdata + pos, inst, VARSIZE(inst));
	result->offsets[ti->count] = pos;
	pos += double_pad(VARSIZE(inst));
	/* Expand the bounding box */
	if (bboxsize != 0) 
	{
		union bboxunion box;
		void *bbox = ((char *) result) + pdata + pos;
		memcpy(bbox, temporali_bbox_ptr(ti), bboxsize);
		temporalinst_make_bbox(&box, inst);
		temporal_bbox_expand(bbox, &box, ti->valuetypid);
		result->offsets[ti->count + 1] = pos;
	}
	return result;
}

/* Append two temporal values */

TemporalI *
temporali_append(const TemporalI *ti1, const TemporalI *ti2)
{
	/* Test the validity of both temporal values */
	assert(ti1->valuetypid == ti2->valuetypid);
	assert(MOBDB_FLAGS_GET_LINEAR(ti1->flags) == MOBDB_FLAGS_GET_LINEAR(ti2->flags));
	assert(MOBDB_FLAGS_GET_GEODETIC(ti1->flags) == MOBDB_FLAGS_GET_GEODETIC(ti2->flags));
	bool isgeo = (ti1->valuetypid == type_oid(T_GEOMETRY) ||
		ti1->valuetypid == type_oid(T_GEOGRAPHY));
	if (isgeo)
	{
		ensure_same_srid_tpoint((Temporal *)ti1, (Temporal *)ti2);
		ensure_same_dimensionality_tpoint((Temporal *)ti1, (Temporal *)ti2);
	}
	TemporalInst *inst1 = temporali_inst_n(ti1, ti1->count - 1);
	TemporalInst *inst2 = temporali_inst_n(ti2, 0);
	if (inst1->t > inst2->t)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values cannot overlap on time")));
	if (inst1->t == inst2->t &&
		! datum_eq(temporalinst_value(inst1), temporalinst_value(inst2), inst1->valuetypid))
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values have different value at their overlapping instant")));

	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(ti1->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < ti1->count; i++)
		memsize += double_pad(VARSIZE(temporali_inst_n(ti1, i)));
	int start = inst1->t == inst2->t ? 1 : 0;
	for (int i = start; i < ti2->count; i++)
		memsize += double_pad(VARSIZE(temporali_inst_n(ti2, i)));
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalI) +
		(ti1->count + ti2->count - start) * sizeof(size_t));
	/* Create the TemporalI */
	TemporalI *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = ti1->count + ti2->count - start;
	result->valuetypid = ti1->valuetypid;
	result->duration = TEMPORALI;
	MOBDB_FLAGS_SET_LINEAR(result->flags,
		MOBDB_FLAGS_GET_LINEAR(ti1->flags));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags, MOBDB_FLAGS_GET_Z(ti1->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, MOBDB_FLAGS_GET_GEODETIC(ti1->flags));
	}
	/* Initialization of the variable-length part */
	size_t pos = 0;
	int k = 0;
	for (int i = 0; i < ti1->count; i++)
	{
		inst1 = temporali_inst_n(ti1, i);
		memcpy(((char *)result) + pdata + pos, inst1, VARSIZE(inst1));
		result->offsets[k++] = pos;
		pos += double_pad(VARSIZE(inst1));
	}
	for (int i = start; i < ti2->count; i++)
	{
		inst2 = temporali_inst_n(ti2, i);
		memcpy(((char *)result) + pdata + pos, inst2, VARSIZE(inst2));
		result->offsets[k++] = pos;
		pos += double_pad(VARSIZE(inst2));
	}
	result->offsets[ti1->count + ti2->count - start] = pos;
	pos += double_pad(VARSIZE(ti2));
	/* Expand the bounding box */
	if (bboxsize != 0)
	{
		void *bbox = ((char *) result) + pdata + pos;
		memcpy((char *)bbox, temporali_bbox_ptr(ti1), bboxsize);
		temporal_bbox_expand(bbox, temporali_bbox_ptr(ti2), ti1->valuetypid);
		result->offsets[ti1->count + ti2->count - start] = pos;
	}
	return result;
}

/* Append two temporal values */

TemporalI *
temporali_append_array(TemporalI **tis, int count)
{
	Oid valuetypid = tis[0]->valuetypid;
	bool linear = MOBDB_FLAGS_GET_LINEAR(tis[0]->flags);
	bool isgeo = (tis[0]->valuetypid == type_oid(T_GEOMETRY) ||
		tis[0]->valuetypid == type_oid(T_GEOGRAPHY));
	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(tis[0]->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < tis[0]->count; i++)
		memsize += double_pad(VARSIZE(temporali_inst_n(tis[0], i)));
	TemporalInst *inst1, *inst2;
	int start;
	int k = tis[0]->count;
	for (int i = 1; i < count; i++)
	{
		/* Test the validity of consecutive temporal values */
		assert(tis[i]->valuetypid == valuetypid);
		assert(MOBDB_FLAGS_GET_LINEAR(tis[i]->flags) == linear);
		if (isgeo)
		{
			ensure_same_srid_tpoint((Temporal *)tis[i - 1], (Temporal *)tis[i]);
			ensure_same_dimensionality_tpoint((Temporal *)tis[i - 1], (Temporal *)tis[i]);
		}
		inst1 = temporali_inst_n(tis[i - 1], tis[i - 1]->count - 1);
		inst2 = temporali_inst_n(tis[i], 0);
		if (inst1->t > inst2->t)
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("The temporal values cannot overlap on time")));
		if (inst1->t == inst2->t &&
			! datum_eq(temporalinst_value(inst1), temporalinst_value(inst2), inst1->valuetypid))
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("The temporal values have different value at their overlapping instant")));
		start = inst1->t == inst2->t ? 1 : 0;
		for (int j = start; j < tis[i]->count; j++)
		{
			memsize += double_pad(VARSIZE(temporali_inst_n(tis[i], j)));
			k++;
		}
	}
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TemporalI) + k * sizeof(size_t));
	/* Create the TemporalI */
	TemporalI *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = k;
	result->valuetypid = valuetypid;
	result->duration = TEMPORALI;
	MOBDB_FLAGS_SET_LINEAR(result->flags, linear);
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
	if (isgeo)
	{
		MOBDB_FLAGS_SET_Z(result->flags,  MOBDB_FLAGS_GET_Z(tis[0]->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags,  MOBDB_FLAGS_GET_GEODETIC(tis[0]->flags));
	}
	/* Initialization of the variable-length part */
	union bboxunion box;
	if (bboxsize != 0)
		memcpy((char *)&box, temporali_bbox_ptr(tis[0]), bboxsize);
	size_t pos = 0;
	k = 0;
	for (int i = 0; i < count; i++)
	{
		start = 0;
		inst2 = temporali_inst_n(tis[i], 0);
		if (i > 0 && inst1->t == inst2->t)
			start = 1;
		for (int j = start; j < tis[i]->count; j++)
		{
			inst2 = temporali_inst_n(tis[i], j);
			memcpy(((char *)result) + pdata + pos, inst2, VARSIZE(inst2));
			result->offsets[k++] = pos;
			pos += double_pad(VARSIZE(inst2));
		}
		/* Expand the bounding box */
		if (bboxsize != 0)
			temporal_bbox_expand(&box, temporali_bbox_ptr(tis[i]), valuetypid);
		inst1 = temporali_inst_n(tis[i], tis[i]->count - 1);
	}
	if (bboxsize != 0)
	{
		void *bbox = ((char *) result) + pdata + pos;
		memcpy((char *)bbox, &box, bboxsize);
	}
	result->offsets[k] = pos;
	return result;
}

/* Copy a TemporalI */
TemporalI *
temporali_copy(const TemporalI *ti)
{
	TemporalI *result = palloc0(VARSIZE(ti));
	memcpy(result, ti, VARSIZE(ti));
	return result;
}

/*
 * Binary search of a timestamptz in a TemporalI or in an array of TemporalInst.
 * If the timestamp is found, the position of the instant is returned in pos.
 * Otherwise, return a number encoding whether it is before, between two 
 * instants or after. For example, given 3 instants, the result of the 
 * function if the value is not found will be as follows: 
 *			0		1		2
 *			|		|		|
 * 1)	t^ 								=> result = 0
 * 2)			t^ 						=> result = 1
 * 3)					t^ 				=> result = 2
 * 4)							t^		=> result = 3
 */

bool
temporali_find_timestamp(const TemporalI *ti, TimestampTz t, int *pos)
{
	int first = 0, last = ti->count - 1;
	int middle = 0; /* make compiler quiet */
	TemporalInst *inst = NULL; /* make compiler quiet */
	while (first <= last) 
	{
		middle = (first + last)/2;
		inst = temporali_inst_n(ti, middle);
		int cmp = timestamp_cmp_internal(inst->t, t);
		if (cmp == 0)
		{
			*pos = middle;
			return true;
		}
		if (cmp > 0)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (t > inst->t)
		middle++;
	*pos = middle;
	return false;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/* 
 * Intersection of a TemporalI and a TemporalInst values. 
 */

bool
intersection_temporali_temporalinst(const TemporalI *ti, const TemporalInst *inst,
	TemporalInst **inter1, TemporalInst **inter2)
{
	TemporalInst *inst1 = temporali_at_timestamp(ti, inst->t);
	if (inst1 == NULL)
		return false;
	
	*inter1 = inst1;
	*inter2 = temporalinst_copy(inst);
	return true;
}

bool
intersection_temporalinst_temporali(const TemporalInst *inst, const TemporalI *ti,
	TemporalInst **inter1, TemporalInst **inter2)
{
	return intersection_temporali_temporalinst(ti, inst, inter2, inter1);
}

/* 
 * Intersection two TemporalI values. Each value keeps the instants 
 * in the intersection of their time spans.
 */

bool
intersection_temporali_temporali(const TemporalI *ti1, const TemporalI *ti2,
	TemporalI **inter1, TemporalI **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	temporali_period(&p1, ti1);
	temporali_period(&p2, ti2);
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
		int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
		if (cmp == 0)
		{
			instants1[k] = inst1;
			instants2[k++] = inst2;
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
	
	pfree(instants1); pfree(instants2); 

	return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Convert to string */
 
char*
temporali_to_string(const TemporalI *ti, char *(*value_out)(Oid, Datum))
{
	char** strings = palloc(sizeof(char *) * ti->count);
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
temporali_write(const TemporalI *ti, StringInfo buf)
{
	pq_sendint(buf, (uint32) ti->count, 4);
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
	TemporalI *result = temporali_make(instants, count);

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
tinti_to_tfloati(const TemporalI *ti)
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

/* Cast a temporal float as a temporal integer */

TemporalI *
tfloati_to_tinti(const TemporalI *ti)
{
	TemporalI *result = temporali_copy(ti);
	result->valuetypid = INT4OID;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(result, i);
		inst->valuetypid = INT4OID;
		Datum *value_ptr = temporalinst_value_ptr(inst);
		*value_ptr = Int32GetDatum((double)DatumGetFloat8(temporalinst_value(inst)));
	}
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalI *
temporalseq_to_temporali(const TemporalSeq *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant set")));

	TemporalInst *inst = temporalseq_inst_n(seq, 0);
	return temporalinst_to_temporali(inst);
}

TemporalI *
temporals_to_temporali(const TemporalS *ts)
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
	TemporalI *result = temporali_make(instants, ts->count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Accessor functions 
 *****************************************************************************/

/* Set of values taken by the temporal value */

static Datum *
temporali_values1(const TemporalI *ti, int *count)
{
	Datum *result = palloc(sizeof(Datum *) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		result[i] = temporalinst_value(temporali_inst_n(ti, i));
	datum_sort(result, ti->count, ti->valuetypid);
	*count = datum_remove_duplicates(result, ti->count, ti->valuetypid);
	return result;
}

ArrayType *
temporali_values(const TemporalI *ti)
{
	int count;
	Datum *values = temporali_values1(ti, &count);
	ArrayType *result = datumarr_to_array(values, count, ti->valuetypid);
	pfree(values);
	return result;
}

/* Set of ranges taken by the temporal value */

ArrayType *
tfloati_ranges(const TemporalI *ti)
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
temporali_get_time(const TemporalI *ti)
{
	Period **periods = palloc(sizeof(Period *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		periods[i] = period_make(inst->t, inst->t, true, true);
	}
	PeriodSet *result = periodset_make_internal(periods, ti->count, false);
	for (int i = 0; i < ti->count; i++)
		pfree(periods[i]);
	pfree(periods);
	return result;
}

/* Minimum value */

Datum
temporali_min_value(const TemporalI *ti)
{
	if (ti->valuetypid == INT4OID)
	{
		TBOX *box = temporali_bbox_ptr(ti);
		return Int32GetDatum((int)(box->xmin));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		TBOX *box = temporali_bbox_ptr(ti);
		return Float8GetDatum(box->xmin);
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
temporali_max_value(const TemporalI *ti)
{
	if (ti->valuetypid == INT4OID)
	{
		TBOX *box = temporali_bbox_ptr(ti);
		return Int32GetDatum((int)(box->xmax));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		TBOX *box = temporali_bbox_ptr(ti);
		return Float8GetDatum(box->xmax);
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

/* Bounding period on which the temporal value is defined */

void
temporali_period(Period *p, const TemporalI *ti)
{
	TimestampTz lower = temporali_start_timestamp(ti);
	TimestampTz upper = temporali_end_timestamp(ti);
	return period_set(p, lower, upper, true, true);
}

/* Instants */

TemporalInst **
temporali_instants(const TemporalI *ti)
{
	TemporalInst **result = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		result[i] = temporali_inst_n(ti, i);
	return result;	
}

ArrayType *
temporali_instants_array(const TemporalI *ti)
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
temporali_start_timestamp(const TemporalI *ti)
{
	return (temporali_inst_n(ti, 0))->t;
}

/* End timestamptz */

TimestampTz
temporali_end_timestamp(const TemporalI *ti)
{
	return (temporali_inst_n(ti, ti->count - 1))->t;
}

/* Set of instants on which the temporal value is defined */

ArrayType *
temporali_timestamps(const TemporalI *ti)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ti->count);
	for (int i = 0; i < ti->count; i++) 
		times[i] = (temporali_inst_n(ti, i))->t;
	ArrayType *result = timestamparr_to_array(times, ti->count);
	pfree(times);
	return result;
}

/* Shift the time span of a temporal value by an interval */

TemporalI *
temporali_shift(const TemporalI *ti, const Interval *interval)
{
   	TemporalI *result = temporali_copy(ti);
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = instants[i] = temporali_inst_n(result, i);
		inst->t = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	}
	/* Recompute the bounding box */
	void *bbox = temporali_bbox_ptr(result); 
	temporali_make_bbox(bbox, instants, ti->count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

/* Is the temporal value ever equal to the value? */

bool
temporali_ever_eq(const TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporali_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d < box.xmin || box.xmax < d)
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
temporali_always_eq(const TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporali_bbox(&box, ti);
		if (ti->valuetypid == INT4OID)
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetInt32(value);
		else
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetFloat8(value);
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (datum_ne(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/*****************************************************************************/

/* Is the temporal value ever less than to the value? */

bool
temporali_ever_lt(const TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporali_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d <= box.xmin)
			return false;
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (datum_lt(valueinst, value, ti->valuetypid))
			return true;
	}
	return false;
}

/* Is the temporal value ever less than or equal to the value? */

bool
temporali_ever_le(const TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporali_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d < box.xmin)
			return false;
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (datum_le(valueinst, value, ti->valuetypid))
			return true;
	}
	return false;
}

/* Is the temporal value always less than the value? */

bool
temporali_always_lt(const TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporali_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d <= box.xmax)
			return false;
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (! datum_lt(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/* Is the temporal value always less than or equal to the value? */

bool
temporali_always_le(const TemporalI *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		temporali_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d < box.xmax)
			return false;
	}

	for (int i = 0; i < ti->count; i++) 
	{
		Datum valueinst = temporalinst_value(temporali_inst_n(ti, i));
		if (! datum_le(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

TemporalI *
temporali_at_value(const TemporalI *ti, Datum value)
{
	Oid valuetypid = ti->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		temporali_bbox(&box1, ti);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
			return NULL;
	}

	/* Singleton instant set */
	if (ti->count == 1)
	{
		if (datum_ne(value, temporalinst_value(temporali_inst_n(ti, 0)), 
			valuetypid))
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
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a value. */

TemporalI *
temporali_minus_value(const TemporalI *ti, Datum value)
{
	Oid valuetypid = ti->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		temporali_bbox(&box1, ti);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
			return temporali_copy(ti);
	}

	/* Singleton instant set */
	if (ti->count == 1)
	{
		if (datum_eq(value, temporalinst_value(temporali_inst_n(ti, 0)), 
			valuetypid))
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
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* 
 * Restriction to an array of values.
 * The function assumes that there are no duplicates values.
 */
 
TemporalI *
temporali_at_values(const TemporalI *ti, const Datum *values, int count)
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
		temporali_make(instants, newcount);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of an array of values
 * The function assumes that there are no duplicates values.
 */

TemporalI *
temporali_minus_values(const TemporalI *ti, const Datum *values, int count)
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
		temporali_make(instants, newcount);
	pfree(instants);
	return result;
}

/* Restriction to a range. */

TemporalI *
tnumberi_at_range(const TemporalI *ti, RangeType *range)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporali_bbox(&box1, ti);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
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
	
	TemporalI *result = temporali_make(instants, count);
	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a range */

TemporalI *
tnumberi_minus_range(const TemporalI *ti, RangeType *range)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	temporali_bbox(&box1, ti);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
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

	TemporalI *result = temporali_make(instants, newcount);
	for (int i = 0; i < newcount; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the ranges */

TemporalI *
tnumberi_at_ranges(const TemporalI *ti, RangeType **normranges, int count)
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
	
	TemporalI *result = temporali_make(instants, newcount);
	for (int i = 0; i < newcount; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the complement of ranges */

TemporalI *
tnumberi_minus_ranges(const TemporalI *ti, RangeType **normranges, int count)
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

	TemporalI *result = temporali_make(instants, newcount);
	for (int i = 0; i < newcount; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/* Restriction to the minimum value */

TemporalI *
temporali_at_min(const TemporalI *ti)
{
	Datum xmin = temporali_min_value(ti);
	return temporali_at_value(ti, xmin);	
}

/* Restriction to the complement of the minimum value */

TemporalI *
temporali_minus_min(const TemporalI *ti)
{
	Datum xmin = temporali_min_value(ti);
	return temporali_minus_value(ti, xmin);	
}

/* Restriction to the maximum value */

TemporalI *
temporali_at_max(const TemporalI *ti)
{
	Datum xmax = temporali_max_value(ti);
	return temporali_at_value(ti, xmax);	
}

/* Restriction to the complement of the maximum value */

TemporalI *
temporali_minus_max(const TemporalI *ti)
{
	Datum xmax = temporali_max_value(ti);
	return temporali_minus_value(ti, xmax);	
}

/* 
 * Restriction to the timestamp
 * To be compatible with the corresponding functions for temporal sequences
 * that need to interpolate the value, it is necessary to return a copy of
 * the value 
 */

TemporalInst *
temporali_at_timestamp(const TemporalI *ti, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporali_period(&p, ti);
	if (!contains_period_timestamp_internal(&p, t))
		return NULL;

	/* Singleton instant set */
	if (ti->count == 1)
		return temporalinst_copy(temporali_inst_n(ti, 0));

	/* General case */
	int n;
	if (! temporali_find_timestamp(ti, t, &n))
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
temporali_value_at_timestamp(const TemporalI *ti, TimestampTz t, Datum *result)
{
	int n;
	if (! temporali_find_timestamp(ti, t, &n))
		return false;

	TemporalInst *inst = temporali_inst_n(ti, n);
	*result = temporalinst_value_copy(inst);
	return true;
}

/* Restriction to the complement of a timestamptz */

TemporalI *
temporali_minus_timestamp(const TemporalI *ti, TimestampTz t)
{
	/* Bounding box test */
	Period p;
	temporali_period(&p, ti);
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
		if (inst->t != t)
			instants[count++] = inst;
	}
	TemporalI *result = (count == 0) ? NULL :
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* 
 * Restriction to a timestamp set
 */

TemporalI *
temporali_at_timestampset(const TemporalI *ti, const TimestampSet *ts)
{
	/* Bounding box test */
	Period p1;
	temporali_period(&p1, ti);
	Period *p2 = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&p1, p2))
		return NULL;

	/* Singleton timestamp set */
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
		int cmp = timestamp_cmp_internal(t, inst->t);
		if (cmp == 0)
		{
			instants[count++] = inst;
			i++;
		}
		else if (cmp < 0)
			i++;
		else
			j++;
	}	
	TemporalI *result = (count == 0) ? NULL :
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/*
 * Restriction to the complement of a timestamp set
 */

TemporalI *
temporali_minus_timestampset(const TemporalI *ti, const TimestampSet *ts)
{
	/* Bounding box test */
	Period p1;
	temporali_period(&p1, ti);
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
		if (t <= inst->t)
			i++;
		else /* t > inst->t */
		{
			instants[count++] = inst;
			j++;
		}
	}
	TemporalI *result = (count == 0) ? NULL : temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the period */

TemporalI *
temporali_at_period(const TemporalI *ti, const Period *period)
{
	/* Bounding box test */
	Period p;
	temporali_period(&p, ti);
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
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a period */

TemporalI *
temporali_minus_period(const TemporalI *ti, const Period *period)
{
	/* Bounding box test */
	Period p;
	temporali_period(&p, ti);
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
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to a period set */

TemporalI *
temporali_at_periodset(const TemporalI *ti, const PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporali_period(&p1, ti);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return NULL;

	/* Singleton period set */
	if (ps->count == 1)
		return temporali_at_period(ti, periodset_per_n(ps, 0));

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
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/* Restriction to the complement of a period set */

TemporalI *
temporali_minus_periodset(const TemporalI *ti, const PeriodSet *ps)
{
	/* Bounding box test */
	Period p1;
	temporali_period(&p1, ti);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return temporali_copy(ti);

	/* Singleton period set */
	if (ps->count == 1)
		return temporali_minus_period(ti, periodset_per_n(ps, 0));

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
		temporali_make(instants, count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

 /* Does the temporal value intersects the timestamp? */

bool
temporali_intersects_timestamp(const TemporalI *ti, TimestampTz t)
{
	int n;
	return temporali_find_timestamp(ti, t, &n);
}

/* Does the temporal value intersects the timestamp set? */

bool
temporali_intersects_timestampset(const TemporalI *ti, const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (temporali_intersects_timestamp(ti, timestampset_time_n(ts, i)))
			return true;
	return false;
}

/* Does the temporal value intersects the period? */

bool
temporali_intersects_period(const TemporalI *ti, const Period *period)
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
temporali_intersects_periodset(const TemporalI *ti, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (temporali_intersects_period(ti, periodset_per_n(ps, i))) 
			return true;
	return false;
}

/*****************************************************************************
 * Local aggregate functions 
 *****************************************************************************/

double
tnumberi_twavg(const TemporalI *ti)
{
	double result = 0.0;
	for (int i = 0; i < ti->count; i++)
	{
		TemporalInst *inst = temporali_inst_n(ti, i);
		result += datum_double(temporalinst_value(inst), inst->valuetypid);
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
temporali_eq(const TemporalI *ti1, const TemporalI *ti2)
{
	/* If number of sequences or flags are not equal */
	if (ti1->count != ti2->count || ti1->flags != ti2->flags)
		return false;

	/* If bounding boxes are not equal */
	void *box1 = temporali_bbox_ptr(ti1);
	void *box2 = temporali_bbox_ptr(ti2);
	if (! temporal_bbox_eq(box1, box2, ti1->valuetypid))
		return false;
	
	/* Compare the composing instants */
	for (int i = 0; i < ti1->count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		if (! temporalinst_eq(inst1, inst2))
			return false;
	}
	return true;
}

/* 
 * B-tree comparator
 * This function supposes for optimization purposes that
 * - a bounding box comparison has been done before in the calling function
 *   and thus that the bounding boxes are equal
 * - the flags of two TemporalI values of the same base type are equal.
 * These hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
int
temporali_cmp(const TemporalI *ti1, const TemporalI *ti2)
{
	int result;
	/* Compare composing instants */
	int count = Min(ti1->count, ti2->count);
	for (int i = 0; i < count; i++)
	{
		TemporalInst *inst1 = temporali_inst_n(ti1, i);
		TemporalInst *inst2 = temporali_inst_n(ti2, i);
		result = temporalinst_cmp(inst1, inst2);
		if (result) 
			return result;
	}
	/* The two values are equal */
	return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for array types for combining the hash of  
 * the elements.
 *****************************************************************************/

uint32
temporali_hash(const TemporalI *ti)
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
