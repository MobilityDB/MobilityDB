/*****************************************************************************
 *
 * tinstant.c
 *	  Basic functions for temporal instants.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tinstant.h"

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
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * Returns a pointer to the base value of the temporal instant value
 */
Datum *
tinstant_value_ptr(const TInstant *inst)
{
	return (Datum *)((char *)inst + double_pad(sizeof(TInstant)));
}

/**
 * Returns the base value of the temporal value
 */
Datum
tinstant_value(const TInstant *inst)
{
	Datum *value = tinstant_value_ptr(inst);
	/* For base types passed by value */
	if (MOBDB_FLAGS_GET_BYVAL(inst->flags))
		return *value;
	/* For base types passed by reference */
	return PointerGetDatum(value);
}

/**
 * Returns a copy of the base value of the temporal instant value
 */
Datum
tinstant_value_copy(const TInstant *inst)
{
	Datum *value = tinstant_value_ptr(inst);
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

/**
 * Construct a temporal instant value from the arguments
 *
 * The memory structure of a temporal instant value is as follows
 * @code
 * ----------------------------------
 * ( TInstant )_X | ( Value )_X | 
 * ----------------------------------
 * @endcode
 * where the `_X` are unused bytes added for double padding.
 *
 * @param value Base value
 * @param t Timestamp
 * @param valuetypid Oid of the base type
 */
TInstant *
tinstant_make(Datum value, TimestampTz t, Oid valuetypid)
{
	size_t value_offset = double_pad(sizeof(TInstant));
	size_t size = value_offset;
	/* Create the temporal value */
	TInstant *result;
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
	result->duration = INSTANT;
	result->valuetypid = valuetypid;
	result->t = t;
	SET_VARSIZE(result, size);
	MOBDB_FLAGS_SET_BYVAL(result->flags, byval);
	MOBDB_FLAGS_SET_LINEAR(result->flags, linear_interpolation(valuetypid));
	MOBDB_FLAGS_SET_X(result->flags, true);
	MOBDB_FLAGS_SET_T(result->flags, true);
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

/**
 * Append the second temporal instant value to the first one
 */
TInstantSet *
tinstant_append_tinstant(const TInstant *inst1, const TInstant *inst2)
{
	ensure_increasing_timestamps(inst1, inst2);
	const TInstant *instants[] = {inst1, inst2};
	return tinstantset_make((TInstant **)instants, 2);
}

/**
 * Merge two temporal instant values
 */
Temporal *
tinstant_merge(const TInstant *inst1, const TInstant *inst2)
{
	/* Test the validity of the temporal values */
	assert(inst1->valuetypid == inst2->valuetypid);
	assert(MOBDB_FLAGS_GET_LINEAR(inst1->flags) == MOBDB_FLAGS_GET_LINEAR(inst2->flags));
	bool isgeo = (inst1->valuetypid == type_oid(T_GEOMETRY) ||
		inst1->valuetypid == type_oid(T_GEOGRAPHY));
	if (isgeo)
	{
		assert(MOBDB_FLAGS_GET_GEODETIC(inst1->flags) == MOBDB_FLAGS_GET_GEODETIC(inst2->flags));
		ensure_same_srid_tpoint((Temporal *) inst1, (Temporal *) inst2);
		ensure_same_dimensionality_tpoint((Temporal *) inst1, (Temporal *) inst2);
	}
	if (inst1->t == inst2->t && ! datum_eq(tinstant_value(inst1),
		tinstant_value(inst2), inst1->valuetypid))
	{
		char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values have different value at their overlapping instant %s", t1)));
	}

	/* Result is a TInstant */
	if (tinstant_eq(inst1, inst2))
		return (Temporal *) tinstant_copy(inst1);

	/* Result is a TInstantSet */
	TInstant *instants[2];
	if (inst1->t < inst2->t)
	{
		instants[0] = (TInstant *) inst1;
		instants[1] = (TInstant *) inst2;
	}
	else
	{
		instants[0] = (TInstant *) inst2;
		instants[1] = (TInstant *) inst1;
	}
	return (Temporal *) tinstantset_make(instants, 2);
}

/**
 * Merge the array of temporal instant values
 */
TInstantSet *
tinstant_merge_array(TInstant **instants, int count)
{
	tinstantarr_sort(instants, count);
	int newcount = tinstantarr_remove_duplicates(instants, count);
	return tinstantset_make(instants, newcount);
}

/**
 * Returns a copy of the temporal instant value
 */
TInstant *
tinstant_copy(const TInstant *inst)
{
	TInstant *result = palloc0(VARSIZE(inst));
	memcpy(result, inst, VARSIZE(inst));
	return result;
}

/**
 * Sets the value and the timestamp of the temporal instant value
 *
 * @param[in,out] inst Temporal value to be modified
 * @param[in] value Value
 * @param[in] t Timestamp
 * @pre This function only works for for base types passed by value.
 * This should be ensured by the calling function!
 */
void
tinstant_set(TInstant *inst, Datum value, TimestampTz t)
{
	inst->t = t;
	Datum *value_ptr = tinstant_value_ptr(inst);
	*value_ptr = value;
}

/*****************************************************************************
 * Intput/output functions
 *****************************************************************************/

/**
 * Returns the string representation of the temporal value
 *
 * @param[in] inst Temporal value
 * @param[in] value_out Function called to output the base value
 *		depending on its Oid
 */
char *
tinstant_to_string(const TInstant *inst, char *(*value_out)(Oid, Datum))
{
	char *t = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
	char *value = value_out(inst->valuetypid, tinstant_value(inst));
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

/**
 * Write the binary representation of the temporal value into the buffer
 *
 * @param[in] inst Temporal value
 * @param[in] buf Buffer
 */
void
tinstant_write(const TInstant *inst, StringInfo buf)
{
	bytea *bt = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
	bytea *bv = call_send(inst->valuetypid, tinstant_value(inst));
	pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(buf, VARSIZE(bv) - VARHDRSZ, 4) ;
#else
	pq_sendint32(buf, VARSIZE(bv) - VARHDRSZ) ;
#endif
	pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
}

/**
 * Returns a new temporal value from its binary representation read from 
 * the buffer
 *
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
 */
TInstant *
tinstant_read(StringInfo buf, Oid valuetypid)
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
	return tinstant_make(value, t, valuetypid);
}

/*****************************************************************************
 * Intersection function
 *****************************************************************************/

/**
 * Temporally intersect the two temporal values
 *
 * @param[in] inst1,inst2 Input values
 * @param[out] inter1, inter2 Output values 
 * @return Returns false if the values do not overlap on time
 */
bool
intersection_tinstant_tinstant(const TInstant *inst1, const TInstant *inst2,
	TInstant **inter1, TInstant **inter2)
{
	/* Test whether the two temporal values overlap on time */
	if (inst1->t != inst2->t)
		return false;
	*inter1 = tinstant_copy(inst1);
	*inter2 = tinstant_copy(inst2);
	return true;
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

/**
 * Cast the temporal integer value as a temporal float value
 */
TInstant *
tintinst_to_tfloatinst(const TInstant *inst)
{
	TInstant *result = tinstant_copy(inst);
	result->valuetypid = FLOAT8OID;
	MOBDB_FLAGS_SET_LINEAR(result->flags, true);
	Datum *value_ptr = tinstant_value_ptr(result);
	*value_ptr = Float8GetDatum((double)DatumGetInt32(tinstant_value(inst)));
	return result;
}

/**
 * Cast the temporal float value as a temporal integer value
 */
TInstant *
tfloatinst_to_tintinst(const TInstant *inst)
{
	TInstant *result = tinstant_copy(inst);
	result->valuetypid = INT4OID;
	MOBDB_FLAGS_SET_LINEAR(result->flags, true);
	Datum *value_ptr = tinstant_value_ptr(result);
	*value_ptr = Int32GetDatum((double)DatumGetFloat8(tinstant_value(inst)));
	return result;
}

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

/**
 * Transform the temporal instant value into a temporal instant set value
 */
TInstantSet *
tinstant_to_tinstantset(const TInstant *inst)
{
	return tinstantset_make((TInstant **)&inst, 1);
}

/**
 * Transform the temporal instant set value into a temporal instant value
 */
TInstant *
tinstantset_to_tinstant(const TInstantSet *ti)
{
	if (ti->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return tinstant_copy(tinstantset_inst_n(ti, 0));
}

/**
 * Transform the temporal sequence value into a temporal instant value
 */
TInstant *
tsequence_to_tinstant(const TSequence *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return tinstant_copy(tsequence_inst_n(seq, 0));
}

/**
 * Transform the temporal sequence set value into a temporal instant value
 */
TInstant *
tsequenceset_to_tinstant(const TSequenceSet *ts)
{
	TSequence *seq = tsequenceset_seq_n(ts, 0);
	if (ts->count != 1 || seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	 return tinstant_copy(tsequence_inst_n(seq, 0));
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

/**
 * Returns the base value of the temporal value as an array
 */
ArrayType *
tinstant_values(const TInstant *inst)
{
	Datum value = tinstant_value(inst);
	return datumarr_to_array(&value, 1, inst->valuetypid);
}

/* Get values */
/**
 * Returns the base value of the temporal float value as a range
 */
ArrayType *
tfloatinst_ranges(const TInstant *inst)
{
	Datum value = tinstant_value(inst);
	RangeType *range = range_make(value, value, true, true, inst->valuetypid);
	ArrayType *result = rangearr_to_array(&range, 1, type_oid(T_FLOATRANGE), false);
	pfree(range);
	return result;
}

/**
 * Returns the time on which the temporal value is defined as a period set
 */
PeriodSet *
tinstant_get_time(const TInstant *inst)
{
	PeriodSet *result = timestamp_to_periodset_internal(inst->t);
	return result;
}

/**
 * Returns the bounding period on which the temporal instant value is defined
 */
void
tinstant_period(Period *p, const TInstant *inst)
{
	return period_set(p, inst->t, inst->t, true, true);
}

/**
 * Returns the timestamp of the temporal value as an array
 */
ArrayType *
tinstant_timestamps(const TInstant *inst)
{
	TimestampTz t = inst->t;
	return timestamparr_to_array(&t, 1);
}

/**
 * Returns the temporal value as an array
 */
ArrayType *
tinstant_instants_array(const TInstant *inst)
{
	return temporalarr_to_array((Temporal **)(&inst), 1);
}

/**
 * Shift the time span of the temporal value by the interval
 */
TInstant *
tinstant_shift(const TInstant *inst, const Interval *interval)
{
	TInstant *result = tinstant_copy(inst);
	result->t = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(inst->t), PointerGetDatum(interval)));
	return result;
}

/*****************************************************************************
 * Ever/always comparison operators
 *****************************************************************************/

/**
 * Returns true if temporal value is ever equal to the base value
 */
bool
tinstant_ever_eq(const TInstant *inst, Datum value)
{
	return datum_eq(tinstant_value(inst), value, inst->valuetypid);
}

/**
 * Returns true if temporal value is always equal to the base value
 */
bool
tinstant_always_eq(const TInstant *inst, Datum value)
{
	return datum_eq(tinstant_value(inst), value, inst->valuetypid);
}

/*****************************************************************************/

/**
 * Returns true if the temporal value is ever less than the base value
 */
bool
tinstant_ever_lt(const TInstant *inst, Datum value)
{
	return datum_lt(tinstant_value(inst), value, inst->valuetypid);
}

/**
 * Returns true if the temporal value is ever less than or equal to
 * the base value
 */
bool
tinstant_ever_le(const TInstant *inst, Datum value)
{
	return datum_le(tinstant_value(inst), value, inst->valuetypid);
}

/**
 * Returns true if the temporal value is always less than the base value
 */
bool
tinstant_always_lt(const TInstant *inst, Datum value)
{
	return datum_lt(tinstant_value(inst), value, inst->valuetypid);
}

/**
 * Returns true if the temporal value is always less than or equal to
 *		the base value
 */
bool
tinstant_always_le(const TInstant *inst, Datum value)
{
	return datum_le(tinstant_value(inst), value, inst->valuetypid);
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/**
 * Restricts the temporal value to the base value
 */
TInstant *
tinstant_at_value(const TInstant *inst, Datum value)
{
	if (datum_ne(value, tinstant_value(inst), inst->valuetypid))
		return NULL;
	return tinstant_copy(inst);
}
  
/**
 * Restricts the temporal value to the complement of the base value
 */
TInstant *
tinstant_minus_value(const TInstant *inst, Datum value)
{
	if (datum_eq(value, tinstant_value(inst), inst->valuetypid))
		return NULL;
	return tinstant_copy(inst);
}

/**
 * Restricts the temporal value to the array of base values
 *
 * @pre There are no duplicates values in the array
 */
TInstant *
tinstant_restrict_values(const TInstant *inst, const Datum *values,
	int count, bool at)
{
	Datum value = tinstant_value(inst);
	for (int i = 0; i < count; i++)
	{
		if (at)
		{
			if (datum_eq(value, values[i], inst->valuetypid))
				return tinstant_copy(inst);
		}
		else
		{
			if (datum_eq(value, values[i], inst->valuetypid))
				return NULL;
		}
	}
	return at ? NULL : tinstant_copy(inst);
}


/**
 * Restricts the temporal value to the (complement of the) range of base values
 */
TInstant *
tnumberinst_restrict_range(const TInstant *inst, RangeType *range, bool at)
{
	Datum d = tinstant_value(inst);
	TypeCacheEntry* typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range, d);
	if (at)
		return contains ? tinstant_copy(inst) : NULL;
	else
		return contains ? NULL : tinstant_copy(inst);
}

/**
 * Restricts the temporal value to the (complement of the) array of ranges of 
 * base values
 * @pre The ranges are normalized
 */
TInstant *
tnumberinst_restrict_ranges(const TInstant *inst, RangeType **normranges, 
	int count, bool at)
{
	Datum d = tinstant_value(inst);
	TypeCacheEntry *typcache = lookup_type_cache(normranges[0]->rangetypid,
		 TYPECACHE_RANGE_INFO);
	for (int i = 0; i < count; i++)
	{
		if (range_contains_elem_internal(typcache, normranges[i], d))
			return at ? tinstant_copy(inst) : NULL;
	}
	return at ? NULL : tinstant_copy(inst);
}

/**
 * Restricts the temporal value to the (complement of the) timestamp
 *
 * @note Since the corresponding function for temporal sequences need to 
 * interpolate the value, it is necessary to return a copy of the value
 */
TInstant *
tinstant_restrict_timestamp(const TInstant *inst, TimestampTz t, bool at)
{
	if (t == inst->t)
		return at ? tinstant_copy(inst) : NULL;
	return at ? NULL : tinstant_copy(inst);
}

/**
 * Returns the base value of the temporal value at the timestamp
 *
 * @note Since the corresponding function for temporal sequences need to 
 * interpolate the value, it is necessary to return a copy of the value
 */
bool
tinstant_value_at_timestamp(const TInstant *inst, TimestampTz t, Datum *result)
{
	if (t != inst->t)
		return false;
	*result = tinstant_value_copy(inst);
	return true;
}

/**
 * Restricts the temporal value to the timestamp set
 */
TInstant *
tinstant_restrict_timestampset(const TInstant *inst, const TimestampSet *ts, bool at)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
			return at ? tinstant_copy(inst) : NULL;
	return at ? NULL : tinstant_copy(inst);
}

/**
 * Restricts the temporal value to the period
 */
TInstant *
tinstant_restrict_period(const TInstant *inst, const Period *period, bool at)
{
	if (at)
	{
		if (!contains_period_timestamp_internal(period, inst->t))
			return NULL;
	}
	else
	{
		if (contains_period_timestamp_internal(period, inst->t))
			return NULL;
	}
	return tinstant_copy(inst);
}

/**
 * Restricts the temporal value to the period set
 */
TInstant *
tinstant_restrict_periodset(const TInstant *inst,const  PeriodSet *ps, bool at)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return at ? tinstant_copy(inst) : NULL;
	return at ?  NULL : tinstant_copy(inst);
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/**
 * Returns true if the temporal value intersects the timestamp
 */
bool
tinstant_intersects_timestamp(const TInstant *inst, TimestampTz t)
{
	return (inst->t == t);
}

/**
 * Returns true if the temporal value intersects the timestamp set
 */
bool
tinstant_intersects_timestampset(const TInstant *inst, 
	const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
			return true;
	return false;
}

/**
 * Returns true if the temporal value intersects the period
 */
bool
tinstant_intersects_period(const TInstant *inst, const Period *p)
{
	return contains_period_timestamp_internal(p, inst->t);
}

/**
 * Returns true if the temporal value intersects the period set
 */
bool
tinstant_intersects_periodset(const TInstant *inst, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return true;
	return false;
}

/*****************************************************************************
 * Functions for defining B-tree indexes
 *****************************************************************************/

/**
 * Returns true if the two temporal instant values are equal
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of two
 * temporal instant values of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
bool
tinstant_eq(const TInstant *inst1, const TInstant *inst2)
{
	assert(inst1->valuetypid == inst2->valuetypid);
	/* Compare values and timestamps */
	Datum value1 = tinstant_value(inst1);
	Datum value2 = tinstant_value(inst2);
	return inst1->t == inst2->t && datum_eq(value1, value2, inst1->valuetypid);
}

/**
 * Returns -1, 0, or 1 depending on whether the first temporal value 
 * is less than, equal, or greater than the second one
 *
 * @pre The arguments are of the same base type
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of
 * two temporal instant values of the same base type are equal.
 * This hypothesis may change in the future and the function must be
 * adapted accordingly.
 */
int
tinstant_cmp(const TInstant *inst1, const TInstant *inst2)
{
	assert(inst1->valuetypid == inst2->valuetypid);
	/* Compare timestamps */
	int cmp = timestamp_cmp_internal(inst1->t, inst2->t);
	if (cmp < 0)
		return -1;
	if (cmp > 0)
		return 1;
	/* Compare values */
	if (datum_lt(tinstant_value(inst1), tinstant_value(inst2),
		inst1->valuetypid))
		return -1;
	if (datum_gt(tinstant_value(inst1), tinstant_value(inst2),
		inst1->valuetypid))
		return 1;
	/* The two values are equal */
	return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for range types for combining the hash of  
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * Returns the hash value of the temporal value
 */
uint32
tinstant_hash(const TInstant *inst)
{
	uint32		result;
	uint32		time_hash;

	Datum value = tinstant_value(inst);
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
