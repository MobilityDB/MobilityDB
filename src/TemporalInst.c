/*****************************************************************************
 *
 * TemporalInst.c
 *	  Basic functions for temporal instants.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"

#ifdef WITH_POSTGIS
#include "TemporalPoint.h"
#endif

/*****************************************************************************
 * General functions
 *****************************************************************************/

/* 
 * The memory structure of a TemporalInst is as follows
 *
 *	----------------------------------
 *	( TemporalInst )_X | ( Value )_X | 
 *	----------------------------------
 *
 * where the X are unused bytes added for double padding.
 */
 
/* Pointer to the value */

static char * 
temporalinst_data_ptr(TemporalInst *inst) 
{
	return (char *)inst + double_pad(sizeof(TemporalInst));
}

/* Get pointer to value */

Datum *
temporalinst_value_ptr(TemporalInst *inst)
{
	return (Datum *)temporalinst_data_ptr(inst);
}

/* Get value depending on whether it is passed by value or by reference */
Datum
temporalinst_value(TemporalInst *inst)
{
	char *value = temporalinst_data_ptr(inst);
	/* For base types passed by value */
	if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
		return *(Datum *)value;
	/* For base types passed by reference */
	return PointerGetDatum(value);
}

Datum
temporalinst_value_copy(TemporalInst *inst)
{
	char *value = temporalinst_data_ptr(inst);
	/* For base types passed by value */
	if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
		return *(Datum *)value;
	/* For base types passed by reference */
	int typlen = get_typlen_fast(inst->valuetypid);
	size_t value_size = typlen != -1 ? (uint) typlen : VARSIZE(value);
	void *result = palloc0(value_size);
	memcpy(result, value, value_size);
	return PointerGetDatum(result);
}

/* Get the bounding box of a TemporalInst */

void
temporalinst_bbox(void *box, TemporalInst *inst) 
{
	Datum value = temporalinst_value(inst);
	temporalinst_make_bbox(box, value, inst->t, inst->valuetypid);
	return;
}

/* Construct a temporal instant value */
 
TemporalInst *
temporalinst_make(Datum value, TimestampTz t, Oid valuetypid)
{
	size_t value_offset = double_pad(sizeof(TemporalInst));
	size_t size = value_offset;
	/* Create the temporal value */
	TemporalInst *result;
	size_t value_size;
	/* Copy value */
	bool byval = type_byval_fast(valuetypid);
	if (byval)
	{
		/* For base types passed by value */
		value_size = double_pad(sizeof(Datum));
		size += value_size;
		result = palloc0(size);
		void *value_to = ((char *) result) + value_offset;
		memcpy(value_to, &value, sizeof(Datum));
	}
	else 
	{
		/* For base types passed by reference */
		void *value_from = DatumGetPointer(value);
		int typlen = get_typlen_fast(valuetypid);
		value_size = typlen != -1 ? double_pad((uint) typlen) : 
			double_pad(VARSIZE(value_from));
		size += value_size;
		result = palloc0(size);
		void *value_to = ((char *) result) + value_offset;
		memcpy(value_to, value_from, value_size);
	}
	/* Initialize fixed-size values */
	result->type = TEMPORALINST;
	result->valuetypid = valuetypid;
	result->t = t;
	SET_VARSIZE(result, size);
	MOBDB_FLAGS_SET_BYVAL(result->flags, byval);
	MOBDB_FLAGS_SET_CONTINUOUS(result->flags, type_is_continuous(valuetypid));
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY) || 
		valuetypid == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(value);
		MOBDB_FLAGS_SET_Z(result->flags, FLAGS_GET_Z(gs->flags));
		POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(value));
	}
#endif
	return result;
}

/* Copy a temporal value */
TemporalInst *
temporalinst_copy(TemporalInst *inst)
{
	TemporalInst *result = palloc0(VARSIZE(inst));
	memcpy(result, inst, VARSIZE(inst));
	return result;
}

/* Convert a temporal number into a float range */
RangeType *
tnumberinst_floatrange(TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	double d = datum_double(value, inst->valuetypid);
	return range_make(d, d, true, true, FLOAT8OID);
}

/*****************************************************************************
 * Intput/output functions
 *****************************************************************************/

/* 
 * Output a temporal value as a string. 
 */
char *
temporalinst_to_string(TemporalInst *inst, char *(*value_out)(Oid, Datum))
{
	char *t = call_output(TIMESTAMPTZOID, inst->t);
	char *value = value_out(inst->valuetypid, temporalinst_value(inst));
	char *result;
	if (inst->valuetypid == TEXTOID)
	{
		result = palloc(strlen(value) + strlen(t) + 4);
		sprintf(result, "\"%s\"@%s", value, t);
	}
	else
	{
		result = palloc(strlen(value) + strlen(t) + 2);
		sprintf(result, "%s@%s", value, t);
	}
	pfree(t);
	pfree(value);
	return result;
}

/* 
 * Send function. 
 */
void
temporalinst_write(TemporalInst *inst, StringInfo buf)
{
	bytea *bt = call_send(TIMESTAMPTZOID, inst->t);
	bytea *bv = call_send(inst->valuetypid, temporalinst_value(inst));
	pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
	pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
}

/* 
 * Receive function. 
 */
TemporalInst *
temporalinst_read(StringInfo buf, Oid valuetypid)
{
	TimestampTz t = call_recv(TIMESTAMPTZOID, buf);
	Datum value = call_recv(valuetypid, buf);
	return temporalinst_make(value, t, valuetypid);
}

/*****************************************************************************
 * Intersection function
 *****************************************************************************/

bool
intersection_temporalinst_temporalinst(TemporalInst *inst1, TemporalInst *inst2, 
	TemporalInst **inter1, TemporalInst **inter2)
{
	/* Test whether the two temporal values overlap on time */
	if (timestamp_cmp_internal(inst1->t, inst2->t) == 0)
		return false;
	*inter1 = temporalinst_copy(inst1);
	*inter2 = temporalinst_copy(inst2);
	return true;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/* Cast temporal integer as temporal float */

TemporalInst *
tintinst_as_tfloatinst(TemporalInst *inst)
{
	TemporalInst *result = temporalinst_copy(inst);
	result->valuetypid = FLOAT8OID;
	MOBDB_FLAGS_SET_CONTINUOUS(result->flags, true);
	Datum *value_ptr = temporalinst_value_ptr(result);
	*value_ptr = Float8GetDatum((double)DatumGetInt32(temporalinst_value(inst)));
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalInst *
temporali_as_temporalinst(TemporalI *ti)
{
	if (ti->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return temporalinst_copy(temporali_inst_n(ti, 0));
}

TemporalInst *
temporalseq_as_temporalinst(TemporalSeq *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return temporalinst_copy(temporalseq_inst_n(seq, 0));
}

TemporalInst *
temporals_as_temporalinst(TemporalS *ts)
{
	TemporalSeq *seq = temporals_seq_n(ts, 0);
	if (ts->count != 1 || seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	 return temporalinst_copy(temporalseq_inst_n(seq, 0));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/* Get values */

ArrayType *
temporalinst_values(TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	return datumarr_to_array(&value, 1, inst->valuetypid);
}

/* Get values */

ArrayType *
tfloatinst_ranges(TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	RangeType *range = range_make(value, value, true, true, inst->valuetypid);
	ArrayType *result = rangearr_to_array(&range, 1, type_oid(T_FLOATRANGE));
	pfree(range);
	return result;
}

/* Get time */

PeriodSet *
temporalinst_get_time(TemporalInst *inst)
{
	Period *p = period_make(inst->t, inst->t, true, true);
	PeriodSet *result = periodset_from_periodarr_internal(&p, 1, false);
	pfree(p);
	return result;
}

/* Value range of a temporal integer */

RangeType *
tnumberinst_value_range(TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	return range_make(value, value,	true, true, inst->valuetypid);
}

/* Bounding period on which the temporal value is defined */

void
temporalinst_timespan(Period *p, TemporalInst *inst)
{
	return period_set(p, inst->t, inst->t, true, true);
}

/* Timestamps */

ArrayType *
temporalinst_timestamps(TemporalInst *inst)
{
	TimestampTz t;
	return timestamparr_to_array(&t, 1);
}

/* Timestamps */

ArrayType *
temporalinst_instants(TemporalInst *inst)
{
	return temporalarr_to_array((Temporal **)(&inst), 1);
}

/* Is the temporal value ever equal to the value? */

bool
temporalinst_ever_equals(TemporalInst *inst, Datum value)
{
	return datum_eq(temporalinst_value(inst), value, inst->valuetypid);
}

/* Is the temporal value always equal to the value? */

bool
temporalinst_always_equals(TemporalInst *inst, Datum value)
{
	return datum_eq(temporalinst_value(inst), value, inst->valuetypid);
}

/* Shift the time span of a temporal value by an interval */

TemporalInst *
temporalinst_shift(TemporalInst *inst, Interval *interval)
{
	TemporalInst *result = temporalinst_copy(inst);
	result->t = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	return result;
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

TemporalInst *
temporalinst_at_value(TemporalInst *inst, Datum value, Oid valuetypid)
{
	if (datum_ne2(value, temporalinst_value(inst), valuetypid, inst->valuetypid))
		return NULL;
	return temporalinst_copy(inst);
}
  
/* Restriction to the complement of a value */

TemporalInst *
temporalinst_minus_value(TemporalInst *inst, Datum value, Oid valuetypid)
{
	if (datum_eq2(value, temporalinst_value(inst), valuetypid, inst->valuetypid))
		return NULL;
	return temporalinst_copy(inst);
}

/* 
 * Restriction to an array of values.
 * The function assumes that there are no duplicates values.
 */
 
TemporalInst *
temporalinst_at_values(TemporalInst *inst, Datum *values, int count, Oid valuetypid)
{
	Datum value = temporalinst_value(inst);
	for (int i = 0; i < count; i++)
		if (datum_eq2(value, values[i], inst->valuetypid, valuetypid))
			return temporalinst_copy(inst);
	return NULL;
}

/* Restriction to the complement of an array of values.
 * The function assumes that there are no duplicates values. */

TemporalInst *
temporalinst_minus_values(TemporalInst *inst, Datum *values, 
	int count, Oid valuetypid)
{
	Datum value = temporalinst_value(inst);
	for (int i = 0; i < count; i++)
		if (datum_eq2(value, values[i], inst->valuetypid, valuetypid))
			return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the range */

TemporalInst *
tnumberinst_at_range(TemporalInst *inst, RangeType *range)
{
	/* Operations on range types require that they must be of the same type */
	Datum d = Float8GetDatum(datum_double(temporalinst_value(inst), inst->valuetypid));
	RangeType *range1 = numrange_to_floatrange_internal(range);
	TypeCacheEntry* typcache = lookup_type_cache(range1->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range1, d);
	pfree(range1);
	if (!contains) 
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the complement of a range */

TemporalInst *
tnumberinst_minus_range(TemporalInst *inst, RangeType *range)
{
	/* Operations on range types require that they must be of the same type */
	Datum d = Float8GetDatum(datum_double(temporalinst_value(inst), inst->valuetypid));
	RangeType *range1 = numrange_to_floatrange_internal(range);
	TypeCacheEntry* typcache = lookup_type_cache(range1->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range1, d);
	pfree(range1);
	if (contains)
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to an array of ranges.
 * The function assumes that the ranges are normalized. */

TemporalInst *
tnumberinst_at_ranges(TemporalInst *inst, RangeType **normranges, int count)
{
	/* Operations on range types require that they must be of the same type */
	Datum d = Float8GetDatum(datum_double(temporalinst_value(inst), inst->valuetypid));
	bool contains = false;
	for (int i = 0; i < count; i++)
	{
		RangeType *range = numrange_to_floatrange_internal(normranges[i]);
		TypeCacheEntry *typcache = lookup_type_cache(range->rangetypid, 
			TYPECACHE_RANGE_INFO);
		contains = range_contains_elem_internal(typcache, range, d);
		pfree(range);
		if (contains) 
			return temporalinst_copy(inst);
	}
	return NULL;
}

/* Restriction to the complement of ranges.
 * The function assumes that the ranges are normalized. */

TemporalInst *
tnumberinst_minus_ranges(TemporalInst *inst, RangeType **normranges, int count)
{
	/* Operations on range types require that they must be of the same type */
	Datum d = Float8GetDatum(datum_double(temporalinst_value(inst), inst->valuetypid));
	bool contains = false;
	for (int i = 0; i < count; i++)
	{
		RangeType *range = numrange_to_floatrange_internal(normranges[i]);
		TypeCacheEntry *typcache = lookup_type_cache(range->rangetypid, 
			TYPECACHE_RANGE_INFO);
		contains = range_contains_elem_internal(typcache, range, d);
		pfree(range);
		if (contains)
			return NULL;
	}
	return temporalinst_copy(inst);
}

/* 
 * Restriction to the timestamp
 * Since the corresponding functions for temporal sequences need to 
 * interpolate the value, it is necessary to return a copy of the value.
 */

TemporalInst *
temporalinst_at_timestamp(TemporalInst *inst, TimestampTz t)
{
	if (timestamp_cmp_internal(t, inst->t) == 0)
		return temporalinst_copy(inst);
	return NULL;
}

/* 
 * Value at the timestamp
 * Since the corresponding functions for temporal sequences need to 
 * interpolate the value, it is necessary to return a copy of the value.
 */

bool
temporalinst_value_at_timestamp(TemporalInst *inst, TimestampTz t, Datum *result)
{
	if (timestamp_cmp_internal(t, inst->t) != 0)
		return false;
	*result = temporalinst_value_copy(inst);
	return true;
}

/* Restriction to the complement of a timestamptz */

TemporalInst *
temporalinst_minus_timestamp(TemporalInst *inst, TimestampTz t)
{
	if (timestamp_cmp_internal(t, inst->t) == 0)
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the timestamp set */

TemporalInst *
temporalinst_at_timestampset(TemporalInst *inst, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (timestamp_cmp_internal(inst->t, timestampset_time_n(ts, i)) == 0)
			return temporalinst_copy(inst);
	return NULL;
}

/* Restriction to the complement of a timestamp set */

TemporalInst *
temporalinst_minus_timestampset(TemporalInst *inst, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (timestamp_cmp_internal(inst->t, timestampset_time_n(ts, i)) == 0)
			return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the period */

TemporalInst *
temporalinst_at_period(TemporalInst *inst, Period *period)
{
	if (!contains_period_timestamp_internal(period, inst->t))
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the complement of a period */

TemporalInst *
temporalinst_minus_period(TemporalInst *inst, Period *period)
{
	if (contains_period_timestamp_internal(period, inst->t))
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to a period set */

TemporalInst *
temporalinst_at_periodset(TemporalInst *inst, PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return temporalinst_copy(inst);
	return NULL;
}

/* Restriction to the complement of a periodset */

TemporalInst *
temporalinst_minus_periodset(TemporalInst *inst, PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return NULL;
	return temporalinst_copy(inst);
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/* Does the temporal value intersects the timestamp? */

bool
temporalinst_intersects_timestamp(TemporalInst *inst, TimestampTz t)
{
	return timestamp_cmp_internal(inst->t, t) == 0;
}

/* Does the temporal value intersects the timestamp set? */

bool
temporalinst_intersects_timestampset(TemporalInst *inst, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (timestamp_cmp_internal(inst->t, timestampset_time_n(ts, i)) == 0)
			return true;
	return false;
}

/* Does the temporal value intersects the period? */

bool
temporalinst_intersects_period(TemporalInst *inst, Period *p)
{
	return contains_period_timestamp_internal(p, inst->t);
}

/* Does the temporal value intersects the period set? */

bool
temporalinst_intersects_periodset(TemporalInst *inst, PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return true;
	return false;
}

/* Does the temporal values intersect? */

bool
temporalinst_intersects_temporalinst(TemporalInst *inst1, TemporalInst *inst2)
{
	return timestamp_cmp_internal(inst1->t, inst2->t) == 0;
}

/*****************************************************************************
 * Functions for defining B-tree index
 * The functions assume that the arguments are of the same temptypid 
 *****************************************************************************/

/* 
 * B-tree comparator
 */

int
temporalinst_cmp(TemporalInst *inst1, TemporalInst *inst2)
{
	if (timestamp_cmp_internal(inst1->t, inst2->t) < 0)
		return -1;
	if (timestamp_cmp_internal(inst1->t, inst2->t) > 0)
		return 1;
	/* Both timestamps are equal */
	if (datum_lt(temporalinst_value(inst1), temporalinst_value(inst2), 
		inst1->valuetypid))
		return -1;
	if (datum_gt(temporalinst_value(inst1), temporalinst_value(inst2), 
		inst1->valuetypid))
		return 1;
	return 0;
}

/* Comparison operators using the internal B-tree comparator */

bool
temporalinst_lt(TemporalInst *inst1, TemporalInst *inst2)
{
	return (temporalinst_cmp(inst1, inst2) < 0);
}

bool
temporalinst_le(TemporalInst *inst1, TemporalInst *inst2)
{
	return (temporalinst_cmp(inst1, inst2) <= 0);
}

bool
temporalinst_eq(TemporalInst *inst1, TemporalInst *inst2)
{
	/* Since we ensure a unique normal representation of temporal types
	   we can use memory comparison which is faster than comparing the
	   individual components */
	/* Total size */
	size_t sz1 = VARSIZE(inst1); 
	if (!memcmp(inst1, inst2, sz1))
		return true;
	return false;
}

bool
temporalinst_ne(TemporalInst *inst1, TemporalInst *inst2)
{
	return (temporalinst_cmp(inst1, inst2) != 0);
}

bool
temporalinst_ge(TemporalInst *inst1, TemporalInst *inst2)
{
	return (temporalinst_cmp(inst1, inst2) > 0);
}

bool
temporalinst_gt(TemporalInst *inst1, TemporalInst *inst2)
{
	return (temporalinst_cmp(inst1, inst2) > 0);
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for range types for combining the hash of  
 * the lower and upper bounds.
 *****************************************************************************/

uint32
temporalinst_hash(TemporalInst *inst)
{
	uint32		result;
	uint32		value_hash;
	uint32		time_hash;

	Datum value = temporalinst_value(inst);
	/* Apply the hash function to each bound */
	if (inst->valuetypid == BOOLOID)
		value_hash = DatumGetUInt32(call_function1(hashchar, value));
	else if (inst->valuetypid == INT4OID)
		value_hash = DatumGetUInt32(call_function1(hashint4, value));
	else if (inst->valuetypid == FLOAT8OID)
		value_hash = DatumGetUInt32(call_function1(hashfloat8, value));
	else if (inst->valuetypid == TEXTOID)
		value_hash = DatumGetUInt32(call_function1(hashtext, value));
#ifdef WITH_POSTGIS
	else if (inst->valuetypid == type_oid(T_GEOMETRY) || 
		inst->valuetypid == type_oid(T_GEOGRAPHY))
		value_hash = DatumGetUInt32(call_function1(lwgeom_hash, value));
#endif
	else 
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Invalid Oid")));

	time_hash = DatumGetUInt32(call_function1(hashint8, inst->t));

	/* Merge hashes of value and time */
	result = value_hash;
	result = (result << 1) | (result >> 31);
	result ^= time_hash;

	return result;
}

/*****************************************************************************/
