/*****************************************************************************
 *
 * temporalinst.c
 *	  Basic functions for temporal instants.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporalinst.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
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
temporalinst_data_ptr(const TemporalInst *inst)
{
	return (char *)inst + double_pad(sizeof(TemporalInst));
}

/* Get pointer to value */

Datum *
temporalinst_value_ptr(const TemporalInst *inst)
{
	return (Datum *)temporalinst_data_ptr(inst);
}

/* Get value depending on whether it is passed by value or by reference */
Datum
temporalinst_value(const TemporalInst *inst)
{
	Datum *value = temporalinst_value_ptr(inst);
	/* For base types passed by value */
	if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
		return *value;
	/* For base types passed by reference */
	return PointerGetDatum(value);
}

Datum
temporalinst_value_copy(const TemporalInst *inst)
{
	Datum *value = temporalinst_value_ptr(inst);
	/* For base types passed by value */
	if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
		return *value;
	/* For base types passed by reference */
	int typlen = get_typlen_fast(inst->valuetypid);
	size_t value_size = typlen != -1 ? (unsigned int) typlen : VARSIZE(value);
	void *result = palloc0(value_size);
	memcpy(result, value, value_size);
	return PointerGetDatum(result);
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
	bool byval = get_typbyval_fast(valuetypid);
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
		value_size = typlen != -1 ? double_pad((unsigned int) typlen) : 
			double_pad(VARSIZE(value_from));
		size += value_size;
		result = palloc0(size);
		void *value_to = ((char *) result) + value_offset;
		memcpy(value_to, value_from, value_size);
	}
	/* Initialize fixed-size values */
	result->duration = TEMPORALINST;
	result->valuetypid = valuetypid;
	result->t = t;
	SET_VARSIZE(result, size);
	MOBDB_FLAGS_SET_BYVAL(result->flags, byval);
	MOBDB_FLAGS_SET_LINEAR(result->flags, linear_interpolation(valuetypid));
	if (valuetypid == type_oid(T_GEOMETRY) ||
		valuetypid == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(value);
		MOBDB_FLAGS_SET_Z(result->flags, FLAGS_GET_Z(gs->flags));
		MOBDB_FLAGS_SET_GEODETIC(result->flags, FLAGS_GET_GEODETIC(gs->flags));
		POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(value));
	}
	return result;
}

 /* Append an instant to another instant resulting in a TemporalI */

TemporalI *
temporalinst_append_instant(TemporalInst *inst1, TemporalInst *inst2)
{
	ensure_increasing_timestamps(inst1, inst2);
	TemporalInst *instants[2];
	instants[0] = inst1;
	instants[1] = inst2;
	return temporali_make(instants, 2);
}

/* Copy a temporal value */
TemporalInst *
temporalinst_copy(TemporalInst *inst)
{
	TemporalInst *result = palloc0(VARSIZE(inst));
	memcpy(result, inst, VARSIZE(inst));
	return result;
}

/* Set the value and the timestamp of an existing temporal instant.
 * This function only works for for base types passed by value.
 * This should be ensured by the calling function! */
void
temporalinst_set(TemporalInst *inst, Datum value, TimestampTz t)
{
	inst->t = t;
	Datum *value_ptr = temporalinst_value_ptr(inst);
	*value_ptr = value;

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
	char *t = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
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
	bytea *bt = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
	bytea *bv = call_send(inst->valuetypid, temporalinst_value(inst));
	pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
	pq_sendint32(buf, VARSIZE(bv) - VARHDRSZ) ;
	pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
}

/* 
 * Receive function. 
 */
TemporalInst *
temporalinst_read(StringInfo buf, Oid valuetypid)
{
	TimestampTz t = call_recv(TIMESTAMPTZOID, buf);
	int size = pq_getmsgint(buf, 4) ;
	StringInfoData buf2 =
	{
		.cursor = 0,
		.len = size,
		.maxlen = size,
		.data = buf->data + buf->cursor
	};	
	Datum value = call_recv(valuetypid, &buf2);
	buf->cursor += size ;
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
	if (inst1->t != inst2->t)
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
tintinst_to_tfloatinst(TemporalInst *inst)
{
	TemporalInst *result = temporalinst_copy(inst);
	result->valuetypid = FLOAT8OID;
	MOBDB_FLAGS_SET_LINEAR(result->flags, true);
	Datum *value_ptr = temporalinst_value_ptr(result);
	*value_ptr = Float8GetDatum((double)DatumGetInt32(temporalinst_value(inst)));
	return result;
}

/* Cast temporal float as temporal integer */

TemporalInst *
tfloatinst_to_tintinst(TemporalInst *inst)
{
	TemporalInst *result = temporalinst_copy(inst);
	result->valuetypid = INT4OID;
	MOBDB_FLAGS_SET_LINEAR(result->flags, true);
	Datum *value_ptr = temporalinst_value_ptr(result);
	*value_ptr = Int32GetDatum((double)DatumGetFloat8(temporalinst_value(inst)));
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

TemporalInst *
temporali_to_temporalinst(TemporalI *ti)
{
	if (ti->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return temporalinst_copy(temporali_inst_n(ti, 0));
}

TemporalInst *
temporalseq_to_temporalinst(TemporalSeq *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return temporalinst_copy(temporalseq_inst_n(seq, 0));
}

TemporalInst *
temporals_to_temporalinst(TemporalS *ts)
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
	PeriodSet *result = periodset_make_internal(&p, 1, false);
	pfree(p);
	return result;
}

/* Bounding period on which the temporal value is defined */

void
temporalinst_period(Period *p, TemporalInst *inst)
{
	return period_set(p, inst->t, inst->t, true, true);
}

/* Timestamps */

ArrayType *
temporalinst_timestamps(TemporalInst *inst)
{
	TimestampTz t = inst->t;
	return timestamparr_to_array(&t, 1);
}

/* Instants */

ArrayType *
temporalinst_instants_array(TemporalInst *inst)
{
	return temporalarr_to_array((Temporal **)(&inst), 1);
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
 * Ever/always comparison operators
 *****************************************************************************/

/* Is the temporal value ever equal to the value? */

bool
temporalinst_ever_eq(TemporalInst *inst, Datum value)
{
	return datum_eq(temporalinst_value(inst), value, inst->valuetypid);
}

/* Is the temporal value always equal to the value? */

bool
temporalinst_always_eq(TemporalInst *inst, Datum value)
{
	return datum_eq(temporalinst_value(inst), value, inst->valuetypid);
}

/*****************************************************************************/

/* Is the temporal value ever less than the value? */

bool
temporalinst_ever_lt(TemporalInst *inst, Datum value)
{
	return datum_lt(temporalinst_value(inst), value, inst->valuetypid);
}

/* Is the temporal value ever less than or equal to the value? */

bool
temporalinst_ever_le(TemporalInst *inst, Datum value)
{
	return datum_le(temporalinst_value(inst), value, inst->valuetypid);
}

/* Is the temporal value always less than the value? */

bool
temporalinst_always_lt(TemporalInst *inst, Datum value)
{
	return datum_lt(temporalinst_value(inst), value, inst->valuetypid);
}

/* Is the temporal value always less than or equal to the value? */

bool
temporalinst_always_le(TemporalInst *inst, Datum value)
{
	return datum_le(temporalinst_value(inst), value, inst->valuetypid);
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/* Restriction to a value */

TemporalInst *
temporalinst_at_value(TemporalInst *inst, Datum value)
{
	if (datum_ne(value, temporalinst_value(inst), inst->valuetypid))
		return NULL;
	return temporalinst_copy(inst);
}
  
/* Restriction to the complement of a value */

TemporalInst *
temporalinst_minus_value(TemporalInst *inst, Datum value)
{
	if (datum_eq(value, temporalinst_value(inst), inst->valuetypid))
		return NULL;
	return temporalinst_copy(inst);
}

/* 
 * Restriction to an array of values.
 * The function assumes that there are no duplicates values.
 */
 
TemporalInst *
temporalinst_at_values(TemporalInst *inst, Datum *values, int count)
{
	Datum value = temporalinst_value(inst);
	for (int i = 0; i < count; i++)
		if (datum_eq(value, values[i], inst->valuetypid))
			return temporalinst_copy(inst);
	return NULL;
}

/* Restriction to the complement of an array of values.
 * The function assumes that there are no duplicates values. */

TemporalInst *
temporalinst_minus_values(TemporalInst *inst, Datum *values, int count)
{
	Datum value = temporalinst_value(inst);
	for (int i = 0; i < count; i++)
		if (datum_eq(value, values[i], inst->valuetypid))
			return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the range */

TemporalInst *
tnumberinst_at_range(TemporalInst *inst, RangeType *range)
{
	Datum d = temporalinst_value(inst);
	TypeCacheEntry* typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range, d);
	if (!contains) 
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the complement of a range */

TemporalInst *
tnumberinst_minus_range(TemporalInst *inst, RangeType *range)
{
	Datum d = temporalinst_value(inst);
	TypeCacheEntry* typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range, d);
	if (contains)
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to an array of ranges.
 * The function assumes that the ranges are normalized. */

TemporalInst *
tnumberinst_at_ranges(TemporalInst *inst, RangeType **normranges, int count)
{
	Datum d = temporalinst_value(inst);
	TypeCacheEntry *typcache = lookup_type_cache(normranges[0]->rangetypid,
		 TYPECACHE_RANGE_INFO);
	for (int i = 0; i < count; i++)
	{
		if (range_contains_elem_internal(typcache, normranges[i], d))
			return temporalinst_copy(inst);
	}
	return NULL;
}

/* Restriction to the complement of ranges.
 * The function assumes that the ranges are normalized. */

TemporalInst *
tnumberinst_minus_ranges(TemporalInst *inst, RangeType **normranges, int count)
{
	Datum d = temporalinst_value(inst);
	TypeCacheEntry *typcache = lookup_type_cache(normranges[0]->rangetypid,
		 TYPECACHE_RANGE_INFO);
	for (int i = 0; i < count; i++)
	{
		if (range_contains_elem_internal(typcache, normranges[i], d))
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
	if (t == inst->t)
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
	if (t != inst->t)
		return false;
	*result = temporalinst_value_copy(inst);
	return true;
}

/* Restriction to the complement of a timestamptz */

TemporalInst *
temporalinst_minus_timestamp(TemporalInst *inst, TimestampTz t)
{
	if (t == inst->t)
		return NULL;
	return temporalinst_copy(inst);
}

/* Restriction to the timestamp set */

TemporalInst *
temporalinst_at_timestampset(TemporalInst *inst, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
			return temporalinst_copy(inst);
	return NULL;
}

/* Restriction to the complement of a timestamp set */

TemporalInst *
temporalinst_minus_timestampset(TemporalInst *inst, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
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
	return (inst->t == t);
}

/* Does the temporal value intersects the timestamp set? */

bool
temporalinst_intersects_timestampset(TemporalInst *inst, TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
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

/*****************************************************************************
 * Functions for defining B-tree index
 * The functions assume that the arguments are of the same temptypid 
 *****************************************************************************/

/* 
 * Equality operator
 * The internal B-tree comparator is not used to increase efficiency
 */
bool
temporalinst_eq(TemporalInst *inst1, TemporalInst *inst2)
{
	/* If flags are not equal */
	if (inst1->flags != inst2->flags) 
		return false;

	/* Compare values and timestamps */
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	return datum_eq(value1, value2, inst1->valuetypid) &&
		(inst1->t == inst2->t);
}

/* 
 * B-tree comparator
 */
int
temporalinst_cmp(TemporalInst *inst1, TemporalInst *inst2)
{
	/* Compare timestamps */
	int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
	if (cmp < 0)
		return -1;
	if (cmp > 0)
		return 1;
	/* Compare values */
	if (datum_lt(temporalinst_value(inst1), temporalinst_value(inst2),
		inst1->valuetypid))
		return -1;
	if (datum_gt(temporalinst_value(inst1), temporalinst_value(inst2),
		inst1->valuetypid))
		return 1;
	/* Compare flags */
	if (inst1->flags < inst2->flags)
		return -1;
	if (inst1->flags > inst2->flags)
		return 1;
	/* The two values are equal */
	return 0;
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
	uint32		time_hash;

	Datum value = temporalinst_value(inst);
	/* Apply the hash function according to the subtype */
	uint32 value_hash = 0; 
	ensure_temporal_base_type(inst->valuetypid);
	if (inst->valuetypid == BOOLOID)
		value_hash = DatumGetUInt32(call_function1(hashchar, value));
	else if (inst->valuetypid == INT4OID)
		value_hash = DatumGetUInt32(call_function1(hashint4, value));
	else if (inst->valuetypid == FLOAT8OID)
		value_hash = DatumGetUInt32(call_function1(hashfloat8, value));
	else if (inst->valuetypid == TEXTOID)
		value_hash = DatumGetUInt32(call_function1(hashtext, value));
	else if (inst->valuetypid == type_oid(T_GEOMETRY) ||
		inst->valuetypid == type_oid(T_GEOGRAPHY))
		value_hash = DatumGetUInt32(call_function1(lwgeom_hash, value));
	/* Apply the hash function according to the timestamp */
	time_hash = DatumGetUInt32(call_function1(hashint8, TimestampTzGetDatum(inst->t)));

	/* Merge hashes of value and timestamp */
	result = value_hash;
	result = (result << 1) | (result >> 31);
	result ^= time_hash;

	return result;
}

/*****************************************************************************/
