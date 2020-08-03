/*****************************************************************************
 *
 * tinstantset.c
 *	  Basic functions for temporal instant sets.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tinstantset.h"

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

/**
 * Returns the n-th instant of the temporal value
 */
TInstant *
tinstantset_inst_n(const TInstantSet *ti, int index)
{
	return (TInstant *) (
		(char *)(&ti->offsets[ti->count + 1]) + 	/* start of data */
			ti->offsets[index]);					/* offset */
}

/**
 * Returns a pointer to the precomputed bounding box of the temporal value
 */
void *
tinstantset_bbox_ptr(const TInstantSet *ti)
{
	return (char *)(&ti->offsets[ti->count + 1]) +  /* start of data */
		ti->offsets[ti->count];						/* offset */
}

/**
 * Copy in the first argument the bounding box of the temporal value
 */
void
tinstantset_bbox(void *box, const TInstantSet *ti)
{
	void *box1 = tinstantset_bbox_ptr(ti);
	size_t bboxsize = temporal_bbox_size(ti->valuetypid);
	memcpy(box, box1, bboxsize);
}

/**
 * Construct a temporal instant set value from the array of temporal
 * instant values
 *
 * For example, the memory structure of a temporal instant set value
 * with 2 instants is as follows
 * @code
 *  ------------------------------------------------------
 *  ( TInstantSet | offset_0 | offset_1 | offset_2 )_X | ...
 *  ------------------------------------------------------
 *  ----------------------------------------------------------
 *  ( TInstant_0 )_X | ( TInstant_1 )_X | ( bbox )_X |
 *  ----------------------------------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding, `offset_0` and 
 * `offset_1` are offsets for the corresponding instants, and `offset_2`
 * is the offset for the bounding box.
 * 
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 */
TInstantSet *
tinstantset_make(TInstant **instants, int count)
{
	/* Test the validity of the instants */
	assert(count > 0);
	bool isgeo = (instants[0]->valuetypid == type_oid(T_GEOMETRY) ||
		instants[0]->valuetypid == type_oid(T_GEOGRAPHY));
	ensure_valid_tinstantarr(instants, count, isgeo);

	/* Get the bounding box size */
	size_t bboxsize = temporal_bbox_size(instants[0]->valuetypid);
	size_t memsize = double_pad(bboxsize);
	/* Add the size of composing instants */
	for (int i = 0; i < count; i++)
		memsize += double_pad(VARSIZE(instants[i]));
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TInstantSet) + count * sizeof(size_t));
	/* Create the TInstantSet */
	TInstantSet *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = count;
	result->valuetypid = instants[0]->valuetypid;
	result->duration = INSTANTSET;
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
		tinstantset_make_bbox(bbox, instants, count);
		result->offsets[count] = pos;
	}
	return result;
}

/**
 * Construct a temporal instant set value from the array of temporal
 * instant values and free the array and the instants after the creation
 *
 * @param[in] instants Array of instants
 * @param[in] count Number of elements in the array
 */
TInstantSet *
tinstantset_make_free(TInstant **instants, int count)
{
	if (count == 0)
	{
		pfree(instants);
		return NULL;
	}
	TInstantSet *result = tinstantset_make(instants, count);
	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

/**
 * Construct a temporal instant set value from a base value and a timestamp set
 */
TInstantSet *
tinstantset_from_base_internal(Datum value, Oid valuetypid, const TimestampSet *ts)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
	for (int i = 0; i < ts->count; i++)
		instants[i] = tinstant_make(value, timestampset_time_n(ts, i), valuetypid);
	return tinstantset_make_free(instants, ts->count);
}

PG_FUNCTION_INFO_V1(tinstantset_from_base);

PGDLLEXPORT Datum
tinstantset_from_base(PG_FUNCTION_ARGS)
{
	Datum value = PG_GETARG_ANYDATUM(0);
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(1);
	Oid valuetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
	TInstantSet *result = tinstantset_from_base_internal(value, valuetypid, ts);
	DATUM_FREE_IF_COPY(value, valuetypid, 0);
	PG_FREE_IF_COPY(ts, 1);
	PG_RETURN_POINTER(result);
}

/**
 * Append an instant to the temporal value
 */
TInstantSet *
tinstantset_append_tinstant(const TInstantSet *ti, const TInstant *inst)
{
	/* Test the validity of the instant */
	assert(ti->valuetypid == inst->valuetypid);
	assert(MOBDB_FLAGS_GET_GEODETIC(ti->flags) == MOBDB_FLAGS_GET_GEODETIC(inst->flags));
	TInstant *inst1 = tinstantset_inst_n(ti, ti->count - 1);
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
		memsize += double_pad(VARSIZE(tinstantset_inst_n(ti, i)));
	memsize += double_pad(VARSIZE(inst));
	/* Add the size of the struct and the offset array
	 * Notice that the first offset is already declared in the struct */
	size_t pdata = double_pad(sizeof(TInstantSet) + (ti->count + 1) * sizeof(size_t));
	/* Create the TInstantSet */
	TInstantSet *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = ti->count + 1;
	result->valuetypid = ti->valuetypid;
	result->duration = INSTANTSET;
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
		inst1 = tinstantset_inst_n(ti, i);
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
		memcpy(bbox, tinstantset_bbox_ptr(ti), bboxsize);
		tinstant_make_bbox(&box, inst);
		temporal_bbox_expand(bbox, &box, ti->valuetypid);
		result->offsets[ti->count + 1] = pos;
	}
	return result;
}

/**
 * Merge the two temporal values
 */
Temporal *
tinstantset_merge(const TInstantSet *ti1, const TInstantSet *ti2)
{
	const TInstantSet *instsets[] = {ti1, ti2};
	return tinstantset_merge_array((TInstantSet **) instsets, 2);
}

/**
 * Merge the array of temporal values
 *
 * @param[in] instsets Array of values
 * @param[in] count Number of elements in the array
 * @result Merged value that can be either a temporal instant or a 
 * temporal instant set
 */
Temporal *
tinstantset_merge_array(TInstantSet **instsets, int count)
{
	/* Test the validity of the temporal values */
	int totalcount = instsets[0]->count;
	bool linear = MOBDB_FLAGS_GET_LINEAR(instsets[0]->flags);
	Oid valuetypid = instsets[0]->valuetypid;
	bool isgeo = (instsets[0]->valuetypid == type_oid(T_GEOMETRY) ||
		instsets[0]->valuetypid == type_oid(T_GEOGRAPHY));
	for (int i = 1; i < count; i++)
	{
		assert(valuetypid == instsets[i]->valuetypid);
		assert(linear == MOBDB_FLAGS_GET_LINEAR(instsets[i]->flags));
		if (isgeo)
		{
			assert(MOBDB_FLAGS_GET_GEODETIC(instsets[0]->flags) ==
				MOBDB_FLAGS_GET_GEODETIC(instsets[i]->flags));
			ensure_same_srid_tpoint((Temporal *)instsets[0], (Temporal *)instsets[i]);
			ensure_same_dimensionality_tpoint((Temporal *)instsets[0], (Temporal *)instsets[i]);
		}
		totalcount += instsets[i]->count;
	}
	/* Collect the composing instants */
	TInstant **instants = palloc0(sizeof(TInstant *) * totalcount);
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < instsets[i]->count; j++)
			instants[k++] = tinstantset_inst_n(instsets[i], j);
	}
	tinstantarr_sort(instants, totalcount);
	int totalcount1;
	totalcount1 = tinstantarr_remove_duplicates(instants, totalcount);
	/* Test the validity of the composing instants */
	TInstant *inst1 = instants[0];
	for (int i = 1; i < totalcount1; i++)
	{
		TInstant *inst2 = instants[i];
		if (inst1->t == inst2->t && ! datum_eq(tinstant_value(inst1),
			tinstant_value(inst2), inst1->valuetypid))
		{
			char *t = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
			ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
				errmsg("The temporal values have different value at their common instant %s", t)));
		}
		inst1 = inst2;
	}
	/* Create the result */
	Temporal *result = (k == 1) ? (Temporal *) instants[0] :
		(Temporal *) tinstantset_make(instants, totalcount1);
	pfree(instants);
	return result;
}

/**
 * Returns a copy of the temporal value
 */
TInstantSet *
tinstantset_copy(const TInstantSet *ti)
{
	TInstantSet *result = palloc0(VARSIZE(ti));
	memcpy(result, ti, VARSIZE(ti));
	return result;
}

/**
 * Returns the location of the timestamp in the temporal instant set 
 * value using binary search
 *
 * If the timestamp is contained in the temporal value, the index
 * of the sequence is returned in the output parameter. Otherwise,
 * returns a number encoding whether the timestamp is before, between
 * two sequences, or after the temporal value.
 * For example, given a value composed of 3 instants and a timestamp, 
 * the value returned in the output parameter is as follows:
 * @code
 *            0        1        2
 *            |        |        |
 * 1)    t^                            => result = 0
 * 2)        t^                        => result = 0
 * 3)            t^                    => result = 1
 * 4)                    t^            => result = 2
 * 5)                            t^    => result = 3
 * @endcode
 *
 * @param[in] ti Temporal instant set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Returns true if the timestamp is contained in the temporal value
 */
bool
tinstantset_find_timestamp(const TInstantSet *ti, TimestampTz t, int *loc)
{
	int first = 0;
	int last = ti->count - 1;
	int middle = 0; /* make compiler quiet */
	TInstant *inst = NULL; /* make compiler quiet */
	while (first <= last)
	{
		middle = (first + last)/2;
		inst = tinstantset_inst_n(ti, middle);
		int cmp = timestamp_cmp_internal(inst->t, t);
		if (cmp == 0)
		{
			*loc = middle;
			return true;
		}
		if (cmp > 0)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (t > inst->t)
		middle++;
	*loc = middle;
	return false;
}

/*****************************************************************************
 * Intersection functions
 *****************************************************************************/

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti,inst Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstant(const TInstantSet *ti, const TInstant *inst,
	TInstant **inter1, TInstant **inter2)
{
	TInstant *inst1 = tinstantset_at_timestamp(ti, inst->t);
	if (inst1 == NULL)
		return false;

	*inter1 = inst1;
	*inter2 = tinstant_copy(inst);
	return true;
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst,ti Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tinstant_tinstantset(const TInstant *inst, const TInstantSet *ti,
	TInstant **inter1, TInstant **inter2)
{
	return intersection_tinstantset_tinstant(ti, inst, inter2, inter1);
}

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] ti1,ti2 Input values
 * @param[out] inter1, inter2 Output values
 * @result Returns false if the input values do not overlap on time
 */
bool
intersection_tinstantset_tinstantset(const TInstantSet *ti1, const TInstantSet *ti2,
	TInstantSet **inter1, TInstantSet **inter2)
{
	/* Test whether the bounding period of the two temporal values overlap */
	Period p1, p2;
	tinstantset_period(&p1, ti1);
	tinstantset_period(&p2, ti2);
	if (!overlaps_period_period_internal(&p1, &p2))
		return false;

	int count = Min(ti1->count, ti2->count);
	TInstant **instants1 = palloc(sizeof(TInstant *) * count);
	TInstant **instants2 = palloc(sizeof(TInstant *) * count);
	int i = 0, j = 0, k = 0;
	while (i < ti1->count && j < ti2->count)
	{
		TInstant *inst1 = tinstantset_inst_n(ti1, i);
		TInstant *inst2 = tinstantset_inst_n(ti2, j);
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

	*inter1 = tinstantset_make(instants1, k);
	*inter2 = tinstantset_make(instants2, k);

	pfree(instants1); pfree(instants2);

	return true;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Returns the string representation of the temporal value
 *
 * @param[in] ti Temporal value
 * @param[in] value_out Function called to output the base value depending on
 * its Oid
 */
char*
tinstantset_to_string(const TInstantSet *ti, char *(*value_out)(Oid, Datum))
{
	char** strings = palloc(sizeof(char *) * ti->count);
	size_t outlen = 0;

	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		strings[i] = tinstant_to_string(inst, value_out);
		outlen += strlen(strings[i]) + 2;
	}
	return stringarr_to_string(strings, ti->count, outlen, "", '{', '}');	
}

/**
 * Write the binary representation of the temporal value
 * into the buffer
 *
 * @param[in] ti Temporal value
 * @param[in] buf Buffer
 */
void
tinstantset_write(const TInstantSet *ti, StringInfo buf)
{
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(buf, (uint32) ti->count, 4);
#else
	pq_sendint32(buf, ti->count);
#endif
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		tinstant_write(inst, buf);
	}
}

/**
 * Returns a new temporal value from its binary representation 
 * read from the buffer
 *
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
 */
TInstantSet *
tinstantset_read(StringInfo buf, Oid valuetypid)
{
	int count = (int) pq_getmsgint(buf, 4);
	TInstant **instants = palloc(sizeof(TInstant *) * count);
	for (int i = 0; i < count; i++)
		instants[i] = tinstant_read(buf, valuetypid);
	return tinstantset_make_free(instants, count);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * Cast the temporal integer value as a temporal float value
 */
TInstantSet *
tintinstset_to_tfloatinstset(const TInstantSet *ti)
{
	TInstantSet *result = tinstantset_copy(ti);
	result->valuetypid = FLOAT8OID;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(result, i);
		inst->valuetypid = FLOAT8OID;
		Datum *value_ptr = tinstant_value_ptr(inst);
		*value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
	}
	return result;
}

/**
 * Cast the temporal float value as a temporal integer value
 */
TInstantSet *
tfloatinstset_to_tintinstset(const TInstantSet *ti)
{
	TInstantSet *result = tinstantset_copy(ti);
	result->valuetypid = INT4OID;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(result, i);
		inst->valuetypid = INT4OID;
		Datum *value_ptr = tinstant_value_ptr(inst);
		*value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
	}
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/
 
/**
 * Transforms the temporal sequence value into a temporal instant value
 *
 * @return Returns an error if the temporal sequence has more than one instant
 */
TInstantSet *
tsequence_to_tinstantset(const TSequence *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant set")));

	TInstant *inst = tsequence_inst_n(seq, 0);
	return tinstant_to_tinstantset(inst);
}

/**
 * Transforms the temporal sequence set value into a temporal instant
 * set value
 *
 * @return Returns an error if any of the composing temporal sequences has 
 * more than one instant
*/
TInstantSet *
tsequenceset_to_tinstantset(const TSequenceSet *ts)
{
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		if (seq->count != 1)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				errmsg("Cannot transform input to a temporal instant set")));
	}

	TInstant **instants = palloc(sizeof(TInstant *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TSequence *seq = tsequenceset_seq_n(ts, i);
		instants[i] = tsequence_inst_n(seq, 0);
	}
	TInstantSet *result = tinstantset_make(instants, ts->count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Returns the base values of the temporal value as a C array
 *
 * @param[in] ti Temporal value
 * @param[out] count Number of elements in the output array
 */
static Datum *
tinstantset_values1(const TInstantSet *ti, int *count)
{
	Datum *result = palloc(sizeof(Datum *) * ti->count);
	for (int i = 0; i < ti->count; i++)
		result[i] = tinstant_value(tinstantset_inst_n(ti, i));
	datumarr_sort(result, ti->count, ti->valuetypid);
	*count = datumarr_remove_duplicates(result, ti->count, ti->valuetypid);
	return result;
}
/**
 * Returns the base values of the temporal value as a PostgreSQL array
 */
ArrayType *
tinstantset_values(const TInstantSet *ti)
{
	int count;
	Datum *values = tinstantset_values1(ti, &count);
	ArrayType *result = datumarr_to_array(values, count, ti->valuetypid);
	pfree(values);
	return result;
}

/**
 * Returns the base values of the temporal float value as an array of ranges
 */
ArrayType *
tfloatinstset_ranges(const TInstantSet *ti)
{
	int count;
	Datum *values = tinstantset_values1(ti, &count);
	RangeType **ranges = palloc(sizeof(RangeType *) * count);
	for (int i = 0; i < count; i++)
		ranges[i] = range_make(values[i], values[i], true, true, FLOAT8OID);
	ArrayType *result = rangearr_to_array(ranges, count, 
		type_oid(T_FLOATRANGE), true);
	pfree(values);
	return result;
}

/**
 * Returns the time on which the temporal value is defined as a period set
 */
PeriodSet *
tinstantset_get_time(const TInstantSet *ti)
{
	Period **periods = palloc(sizeof(Period *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		periods[i] = period_make(inst->t, inst->t, true, true);
	}
	return periodset_make_free(periods, ti->count, false);
}

/**
 * Returns the minimum base value of the temporal value
 */
Datum
tinstantset_min_value(const TInstantSet *ti)
{
	if (ti->valuetypid == INT4OID)
	{
		TBOX *box = tinstantset_bbox_ptr(ti);
		return Int32GetDatum((int)(box->xmin));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		TBOX *box = tinstantset_bbox_ptr(ti);
		return Float8GetDatum(box->xmin);
	}
	else
	{
		Oid valuetypid = ti->valuetypid;
		Datum min = tinstant_value(tinstantset_inst_n(ti, 0));
		int idx = 0;
		for (int i = 1; i < ti->count; i++)
		{
			Datum value = tinstant_value(tinstantset_inst_n(ti, i));
			if (datum_lt(value, min, valuetypid))
			{
				min = value;
				idx = i;
			}
		}
		return tinstant_value(tinstantset_inst_n(ti, idx));
	}
}

/**
 * Returns the maximum base value of the temporal value
 */
Datum
tinstantset_max_value(const TInstantSet *ti)
{
	if (ti->valuetypid == INT4OID)
	{
		TBOX *box = tinstantset_bbox_ptr(ti);
		return Int32GetDatum((int)(box->xmax));
	}
	else if (ti->valuetypid == FLOAT8OID)
	{
		TBOX *box = tinstantset_bbox_ptr(ti);
		return Float8GetDatum(box->xmax);
	}
	else
	{
		Oid valuetypid = ti->valuetypid;
		Datum max = tinstant_value(tinstantset_inst_n(ti, 0));
		int idx = 0;
		for (int i = 1; i < ti->count; i++)
		{
			Datum value = tinstant_value(tinstantset_inst_n(ti, i));
			if (datum_gt(value, max, valuetypid))
			{
				max = value;
				idx = i;
			}
		}
		return tinstant_value(tinstantset_inst_n(ti, idx));
	}
}

/**
 * Returns the bounding period on which the temporal value is defined 
 */
void
tinstantset_period(Period *p, const TInstantSet *ti)
{
	TimestampTz lower = tinstantset_start_timestamp(ti);
	TimestampTz upper = tinstantset_end_timestamp(ti);
	return period_set(p, lower, upper, true, true);
}

/**
 * Returns the instants of the temporal value as a C array
 */
TInstant **
tinstantset_instants(const TInstantSet *ti)
{
	TInstant **result = palloc(sizeof(TInstant *) * ti->count);
	for (int i = 0; i < ti->count; i++)
		result[i] = tinstantset_inst_n(ti, i);
	return result;
}

/**
 * Returns the instants of the temporal value as an PostgreSQL array
 */
ArrayType *
tinstantset_instants_array(const TInstantSet *ti)
{
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	for (int i = 0; i < ti->count; i++)
		instants[i] = tinstantset_inst_n(ti, i);
	ArrayType *result = temporalarr_to_array((Temporal **)instants, ti->count);
	pfree(instants);
	return result;
}

/**
 * Returns the start timestamp of the temporal value
 */
TimestampTz
tinstantset_start_timestamp(const TInstantSet *ti)
{
	return (tinstantset_inst_n(ti, 0))->t;
}

/**
 * Returns the end timestamp of the temporal value
 */
TimestampTz
tinstantset_end_timestamp(const TInstantSet *ti)
{
	return (tinstantset_inst_n(ti, ti->count - 1))->t;
}

/**
 * Returns the distinct timestamps of the temporal value as an array
 */
ArrayType *
tinstantset_timestamps(const TInstantSet *ti)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ti->count);
	for (int i = 0; i < ti->count; i++)
		times[i] = (tinstantset_inst_n(ti, i))->t;
	ArrayType *result = timestamparr_to_array(times, ti->count);
	pfree(times);
	return result;
}

/**
 * Shift the time span of the temporal value by the interval
 */
TInstantSet *
tinstantset_shift(const TInstantSet *ti, const Interval *interval)
{
   	TInstantSet *result = tinstantset_copy(ti);
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = instants[i] = tinstantset_inst_n(result, i);
		inst->t = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	}
	/* Recompute the bounding box */
	void *bbox = tinstantset_bbox_ptr(result);
	tinstantset_make_bbox(bbox, instants, ti->count);
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
tinstantset_ever_eq(const TInstantSet *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		tinstantset_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d < box.xmin || box.xmax < d)
			return false;
	}

	for (int i = 0; i < ti->count; i++)
	{
		Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
		if (datum_eq(valueinst, value, ti->valuetypid))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value is always equal to the base value
 */
bool
tinstantset_always_eq(const TInstantSet *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		tinstantset_bbox(&box, ti);
		if (ti->valuetypid == INT4OID)
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetInt32(value);
		else
			return box.xmin == box.xmax &&
				(int)(box.xmax) == DatumGetFloat8(value);
	}

	for (int i = 0; i < ti->count; i++)
	{
		Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
		if (datum_ne(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/*****************************************************************************/

/**
 * Returns true if the temporal value is ever less than the base value
 */
bool
tinstantset_ever_lt(const TInstantSet *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		tinstantset_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d <= box.xmin)
			return false;
	}

	for (int i = 0; i < ti->count; i++)
	{
		Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
		if (datum_lt(valueinst, value, ti->valuetypid))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value is ever less than or equal 
 * to the base value
 */
bool
tinstantset_ever_le(const TInstantSet *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		tinstantset_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d < box.xmin)
			return false;
	}

	for (int i = 0; i < ti->count; i++)
	{
		Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
		if (datum_le(valueinst, value, ti->valuetypid))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value is always less than the base value
 */
bool
tinstantset_always_lt(const TInstantSet *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		tinstantset_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d <= box.xmax)
			return false;
	}

	for (int i = 0; i < ti->count; i++)
	{
		Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
		if (! datum_lt(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/**
 * Returns true if the temporal value is always less than or equal 
 * to the base value
 */
bool
tinstantset_always_le(const TInstantSet *ti, Datum value)
{
	/* Bounding box test */
	if (ti->valuetypid == INT4OID || ti->valuetypid == FLOAT8OID)
	{
		TBOX box;
		memset(&box, 0, sizeof(TBOX));
		tinstantset_bbox(&box, ti);
		double d = datum_double(value, ti->valuetypid);
		if (d < box.xmax)
			return false;
	}

	for (int i = 0; i < ti->count; i++)
	{
		Datum valueinst = tinstant_value(tinstantset_inst_n(ti, i));
		if (! datum_le(valueinst, value, ti->valuetypid))
			return false;
	}
	return true;
}

/*****************************************************************************
 * Restriction Functions
 *****************************************************************************/

/**
 * Restricts the temporal value to the (complement of the) base value
 */
TInstantSet *
tinstantset_restrict_value(const TInstantSet *ti, Datum value, bool at)
{
	Oid valuetypid = ti->valuetypid;
	/* Bounding box test */
	if (valuetypid == INT4OID || valuetypid == FLOAT8OID)
	{
		TBOX box1, box2;
		memset(&box1, 0, sizeof(TBOX));
		memset(&box2, 0, sizeof(TBOX));
		tinstantset_bbox(&box1, ti);
		number_to_box(&box2, value, valuetypid);
		if (!contains_tbox_tbox_internal(&box1, &box2))
			return at ? NULL : tinstantset_copy(ti);
	}

	/* Singleton instant set */
	if (ti->count == 1)
	{
		Datum value1 = tinstant_value(tinstantset_inst_n(ti, 0));
		if ((at && datum_ne(value, value1, valuetypid)) ||
			(!at && datum_eq(value, value1, valuetypid)))
			return NULL;
		return tinstantset_copy(ti);
	}

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		if ((at && datum_eq(value, tinstant_value(inst), valuetypid)) ||
			(!at && datum_ne(value, tinstant_value(inst), valuetypid)))
			instants[count++] = inst;
	}
	TInstantSet *result = (count == 0) ? NULL :
		tinstantset_make(instants, count);
	pfree(instants);
	return result;
}

TInstantSet *
tinstantset_at_value(const TInstantSet *ti, Datum value)
{
	return tinstantset_restrict_value(ti, value, true);
}

TInstantSet *
tinstantset_minus_value(const TInstantSet *ti, Datum value)
{
	return tinstantset_restrict_value(ti, value, false);
}


/**
 * Restricts the temporal value to the (complement of the) array of base values
 *
 * @param[in] ti Temporal value
 * @param[in] values Array of base values
 * @param[in] count Number of elements in the input array 
 * @param[in] at True when the restriction is at, false for minus 
 * @pre There are no duplicates values in the array
 */
TInstantSet *
tinstantset_restrict_values(const TInstantSet *ti, const Datum *values, 
	int count, bool at)
{
	/* Singleton instant set */
	if (ti->count == 1)
	{
		TInstant *inst = tinstantset_inst_n(ti, 0);
		TInstant *inst1 = at ?
			tinstant_at_values(inst, values, count) :
			tinstant_minus_values(inst, values, count);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1);
		return tinstantset_copy(ti);
	}

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int newcount = 0;
	for (int i = 0; i < ti->count; i++)
	{
		bool found = false;
		TInstant *inst = tinstantset_inst_n(ti, i);
		for (int j = 0; j < count; j++)
		{
			if (datum_eq(tinstant_value(inst), values[j], ti->valuetypid))
			{
				if (at)
					instants[newcount++] = inst;
				else
					found = true;
				break;
			}
		}
		if (!at && !found)
			instants[newcount++] = inst;
	}
	TInstantSet *result = (newcount == 0) ? NULL :
		tinstantset_make(instants, newcount);
	pfree(instants);
	return result;
}

/**
 * Restricts the temporal value to the (complement of the) range of base values
 */
TInstantSet *
tnumberinstset_restrict_range(const TInstantSet *ti, RangeType *range, bool at)
{
	/* Bounding box test */
	TBOX box1, box2;
	memset(&box1, 0, sizeof(TBOX));
	memset(&box2, 0, sizeof(TBOX));
	tinstantset_bbox(&box1, ti);
	range_to_tbox_internal(&box2, range);
	if (!overlaps_tbox_tbox_internal(&box1, &box2))
		return at ? NULL : tinstantset_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
		return at ? tinstantset_copy(ti) : NULL;

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		TInstant *inst1 = at ?
			tnumberinst_at_range(inst, range) :
			tnumberinst_minus_range(inst, range);
		if (inst1 != NULL)
			instants[count++] = inst1;
	}
	return tinstantset_make_free(instants, count);
}

/**
 * Restricts the temporal value to the (complement of the) array of ranges
 * of base values
 */
TInstantSet *
tnumberinstset_restrict_ranges(const TInstantSet *ti, RangeType **normranges, 
	int count, bool at)
{
	/* Singleton instant set */
	if (ti->count == 1)
	{
		TInstant *inst = tinstantset_inst_n(ti, 0);
		TInstant *inst1 = at ?
			tnumberinst_at_ranges(inst, normranges, count) :
			tnumberinst_minus_ranges(inst, normranges, count);
		if (inst1 == NULL)
			return NULL;
		pfree(inst1);
		return tinstantset_copy(ti);
	}

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int newcount = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		for (int j = 0; j < count; j++)
		{
			TInstant *inst1 = at ? 
				tnumberinst_at_range(inst, normranges[j]) :
				tnumberinst_minus_range(inst, normranges[j]);
			if (inst1 != NULL)
			{
				instants[newcount++] = inst1;
				break;
			}
		}
	}
	return tinstantset_make_free(instants, newcount);
}

/**
 * Returns a pointer to the instant with minimum base value of the  
 * temporal value
 *
 * @note Function used, e.g., for computing the shortest line between two
 * temporal points from their temporal distance
 */
TInstant *
tinstantset_min_instant(const TInstantSet *ti)
{
	Datum min = tinstant_value(tinstantset_inst_n(ti, 0));
	int k = 0;
	for (int i = 1; i < ti->count; i++)
	{
		Datum value = tinstant_value(tinstantset_inst_n(ti, i));
		if (datum_lt(value, min, ti->valuetypid))
		{
			min = value;
			k = i;
		}
	}
	return tinstantset_inst_n(ti, k);
}

/**
 * Restricts the temporal value to the minimum base value
 */
TInstantSet *
tinstantset_at_min(const TInstantSet *ti)
{
	Datum xmin = tinstantset_min_value(ti);
	return tinstantset_at_value(ti, xmin);
}

/**
 * Restricts the temporal value to the complement of the minimum base value
 */
TInstantSet *
tinstantset_minus_min(const TInstantSet *ti)
{
	Datum xmin = tinstantset_min_value(ti);
	return tinstantset_minus_value(ti, xmin);
}

/* Restriction to the maximum value */
/**
 * Restricts the temporal value to the maximum base value
 */
TInstantSet *
tinstantset_at_max(const TInstantSet *ti)
{
	Datum xmax = tinstantset_max_value(ti);
	return tinstantset_at_value(ti, xmax);
}

/**
 * Restricts the temporal value to the complement of the maximum base value
 */
TInstantSet *
tinstantset_minus_max(const TInstantSet *ti)
{
	Datum xmax = tinstantset_max_value(ti);
	return tinstantset_minus_value(ti, xmax);
}

/**
 * Restricts the temporal value to the timestamp
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 */
Temporal *
tinstantset_restrict_timestamp(const TInstantSet *ti, TimestampTz t, bool at)
{
	/* Bounding box test */
	Period p;
	tinstantset_period(&p, ti);
	if (!contains_period_timestamp_internal(&p, t))
		return at ? NULL : (Temporal *) tinstantset_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
		return at ? (Temporal *) tinstant_copy(tinstantset_inst_n(ti, 0)) : NULL;

	/* General case */
	if (at)
	{
		int loc;
		if (! tinstantset_find_timestamp(ti, t, &loc))
			return NULL;
		TInstant *inst = tinstantset_inst_n(ti, loc);
		return (Temporal *)tinstant_copy(inst);
	}
	else
	{
		TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
		int count = 0;
		for (int i = 0; i < ti->count; i++)
		{
			TInstant *inst= tinstantset_inst_n(ti, i);
			if (inst->t != t)
				instants[count++] = inst;
		}
		TInstantSet *result = (count == 0) ? NULL :
			tinstantset_make(instants, count);
		pfree(instants);
		return (Temporal *)result;
	}
}

TInstant *
tinstantset_at_timestamp(const TInstantSet *ti, TimestampTz t)
{
	return (TInstant *) tinstantset_restrict_timestamp(ti, t, true);
}

TInstantSet *
tinstantset_minus_timestamp(const TInstantSet *ti, TimestampTz t)
{
	return (TInstantSet *) tinstantset_restrict_timestamp(ti, t, false);
}

/**
 * Returns the base value of the temporal value at the timestamp
 *
 * @note In order to be compatible with the corresponding functions for temporal
 * sequences that need to interpolate the value, it is necessary to return
 * a copy of the value
 */
bool
tinstantset_value_at_timestamp(const TInstantSet *ti, TimestampTz t, Datum *result)
{
	int loc;
	if (! tinstantset_find_timestamp(ti, t, &loc))
		return false;

	TInstant *inst = tinstantset_inst_n(ti, loc);
	*result = tinstant_value_copy(inst);
	return true;
}

/**
 * Restricts the temporal value to the (complement of the) timestamp set
 */
TInstantSet *
tinstantset_restrict_timestampset(const TInstantSet *ti, 
	const TimestampSet *ts, bool at)
{
	/* Bounding box test */
	Period p1;
	tinstantset_period(&p1, ti);
	Period *p2 = timestampset_bbox(ts);
	if (!overlaps_period_period_internal(&p1, p2))
		return at ? NULL : tinstantset_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TInstant *inst = tinstantset_inst_n(ti, 0);
		TInstant *inst1 = at ?
			tinstant_at_timestampset(inst, ts) :
			tinstant_minus_timestampset(inst, ts);
		if (inst1 == NULL)
			return NULL;

		pfree(inst1);
		return tinstantset_copy(ti);
	}

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int count = 0;
	int i = 0, j = 0;
	while (i < ts->count && j < ti->count)
	{
		TInstant *inst = tinstantset_inst_n(ti, j);
		TimestampTz t = timestampset_time_n(ts, i);
		if (at)
		{
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
		else
		{
			if (t <= inst->t)
				i++;
			else /* t > inst->t */
			{
				instants[count++] = inst;
				j++;
			}
		}
	}
	TInstantSet *result = (count == 0) ? NULL :
		tinstantset_make(instants, count);
	pfree(instants);
	return result;
}

/**
 * Restricts the temporal value to the (complement of the) period
 */
TInstantSet *
tinstantset_restrict_period(const TInstantSet *ti, const Period *period, bool at)
{
	/* Bounding box test */
	Period p;
	tinstantset_period(&p, ti);
	if (!overlaps_period_period_internal(&p, period))
		return at ? NULL : tinstantset_copy(ti);

	/* Singleton instant set */
	if (ti->count == 1)
		return at ? tinstantset_copy(ti) : NULL;

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		if ((at && contains_period_timestamp_internal(period, inst->t)) ||
			(!at && !contains_period_timestamp_internal(period, inst->t)))
			instants[count++] = inst;
	}
	TInstantSet *result = (count == 0) ? NULL :
		tinstantset_make(instants, count);
	pfree(instants);
	return result;
}

/**
 * Restricts the temporal value to the period set
 */
TInstantSet *
tinstantset_restrict_periodset(const TInstantSet *ti, const PeriodSet *ps,
	bool at)
{
	/* Bounding box test */
	Period p1;
	tinstantset_period(&p1, ti);
	Period *p2 = periodset_bbox(ps);
	if (!overlaps_period_period_internal(&p1, p2))
		return at ? NULL : tinstantset_copy(ti);

	/* Singleton period set */
	if (ps->count == 1)
		return tinstantset_restrict_period(ti, periodset_per_n(ps, 0), at);

	/* Singleton instant set */
	if (ti->count == 1)
	{
		TInstant *inst = tinstantset_inst_n(ti, 0);
		TInstant *inst1 = at ?
			tinstant_at_periodset(inst, ps) :
			tinstant_minus_periodset(inst, ps);
		if (inst1 == NULL)
			return NULL;

		pfree(inst1);
		return tinstantset_copy(ti);
	}

	/* General case */
	TInstant **instants = palloc(sizeof(TInstant *) * ti->count);
	int count = 0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		if ((at && contains_periodset_timestamp_internal(ps, inst->t)) ||
			(!at && !contains_periodset_timestamp_internal(ps, inst->t)))
			instants[count++] = inst;
	}
	TInstantSet *result = (count == 0) ? NULL :
		tinstantset_make(instants, count);
	pfree(instants);
	return result;
}

/*****************************************************************************
 * Intersects functions
 *****************************************************************************/

/**
 * Returns true if the temporal value intersects the timestamp
 */
bool
tinstantset_intersects_timestamp(const TInstantSet *ti, TimestampTz t)
{
	int loc;
	return tinstantset_find_timestamp(ti, t, &loc);
}

/**
 * Returns true if the temporal value intersects the timestamp set
 */
bool
tinstantset_intersects_timestampset(const TInstantSet *ti, const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (tinstantset_intersects_timestamp(ti, timestampset_time_n(ts, i)))
			return true;
	return false;
}

/**
 * Returns true if the temporal value intersects the period
 */
bool
tinstantset_intersects_period(const TInstantSet *ti, const Period *period)
{
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		if (contains_period_timestamp_internal(period, inst->t))
			return true;
	}
	return false;
}

/**
 * Returns true if the temporal value intersects the period set
 */
bool
tinstantset_intersects_periodset(const TInstantSet *ti, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (tinstantset_intersects_period(ti, periodset_per_n(ps, i)))
			return true;
	return false;
}

/*****************************************************************************
 * Local aggregate functions
 *****************************************************************************/

/**
 * Returns the time-weighted average of the temporal number
 */
double
tnumberinstset_twavg(const TInstantSet *ti)
{
	double result = 0.0;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		result += datum_double(tinstant_value(inst), inst->valuetypid);
	}
	return result / ti->count;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * Returns true if the two temporal instant set values are equal
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency
 */
bool
tinstantset_eq(const TInstantSet *ti1, const TInstantSet *ti2)
{
	assert(ti1->valuetypid == ti2->valuetypid);
	/* If number of sequences or flags are not equal */
	if (ti1->count != ti2->count || ti1->flags != ti2->flags)
		return false;

	/* If bounding boxes are not equal */
	void *box1 = tinstantset_bbox_ptr(ti1);
	void *box2 = tinstantset_bbox_ptr(ti2);
	if (! temporal_bbox_eq(box1, box2, ti1->valuetypid))
		return false;

	/* Compare the composing instants */
	for (int i = 0; i < ti1->count; i++)
	{
		TInstant *inst1 = tinstantset_inst_n(ti1, i);
		TInstant *inst2 = tinstantset_inst_n(ti2, i);
		if (! tinstant_eq(inst1, inst2))
			return false;
	}
	return true;
}

/**
 * Returns -1, 0, or 1 depending on whether the first temporal value 
 * is less than, equal, or greater than the second one
 *
 * @pre The arguments are of the same base type
 * @pre This function supposes for optimization purposes that
 * 1. a bounding box comparison has been done before in the calling function
 *    and thus that the bounding boxes are equal,
 * 2. the flags of two temporal values of the same base type are equal.
 * These hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
int
tinstantset_cmp(const TInstantSet *ti1, const TInstantSet *ti2)
{
	assert(ti1->valuetypid == ti2->valuetypid);
	/* Compare composing instants */
	int count = Min(ti1->count, ti2->count);
	for (int i = 0; i < count; i++)
	{
		TInstant *inst1 = tinstantset_inst_n(ti1, i);
		TInstant *inst2 = tinstantset_inst_n(ti2, i);
		int result = tinstant_cmp(inst1, inst2);
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

/**
 * Returns the hash value of the temporal value
 */
uint32
tinstantset_hash(const TInstantSet *ti)
{
	uint32 result = 1;
	for (int i = 0; i < ti->count; i++)
	{
		TInstant *inst = tinstantset_inst_n(ti, i);
		uint32 inst_hash = tinstant_hash(inst);
		result = (result << 5) - result + inst_hash;
	}
	return result;
}

/*****************************************************************************/
