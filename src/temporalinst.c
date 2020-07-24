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
#include "tpoint_spatialfuncs.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

/**
 * @brief Returns a pointer to the base value of the temporal instant value
 */
Datum *
temporalinst_value_ptr(const TemporalInst *inst)
{
	return (Datum *)((char *)inst + double_pad(sizeof(TemporalInst)));
}

/**
 * @brief Returns the base value of the temporal value
 */
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

/**
 * @brief Returns a copy of the base value of the temporal instant value
 */
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

/**
 * @brief Construct a temporal instant value from the arguments
 * @details The memory structure of a temporal instant value is as follows
 * @code
 * ----------------------------------
 * ( TemporalInst )_X | ( Value )_X | 
 * ----------------------------------
 * @endcode
 * where the @c X are unused bytes added for double padding.
 * @param value Base value
 * @param t Timestamp
 * @param valuetypid Oid of the base type
 */
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
 * @brief Append the second temporal instant value to the first one
 */
TemporalI *
temporalinst_append_instant(const TemporalInst *inst1, const TemporalInst *inst2)
{
	ensure_increasing_timestamps(inst1, inst2);
	const TemporalInst *instants[] = {inst1, inst2};
	return temporali_make((TemporalInst **)instants, 2);
}

/**
 * @brief Merge two temporal instant values
 */
Temporal *
temporalinst_merge(const TemporalInst *inst1, const TemporalInst *inst2)
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
	if (inst1->t == inst2->t && ! datum_eq(temporalinst_value(inst1),
		temporalinst_value(inst2), inst1->valuetypid))
	{
		char *t1 = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(inst1->t));
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The temporal values have different value at their overlapping instant %s", t1)));
	}

	/* Result is a TemporalInst */
	if (temporalinst_eq(inst1, inst2))
		return (Temporal *) temporalinst_copy(inst1);

	/* Result is a TemporalI */
	TemporalInst *instants[2];
	if (inst1->t < inst2->t)
	{
		instants[0] = (TemporalInst *) inst1;
		instants[1] = (TemporalInst *) inst2;
	}
	else
	{
		instants[0] = (TemporalInst *) inst2;
		instants[1] = (TemporalInst *) inst1;
	}
	return (Temporal *) temporali_make(instants, 2);
}

/**
 * @brief Merge the array of temporal instant values
 */
TemporalI *
temporalinst_merge_array(TemporalInst **instants, int count)
{
	temporalinstarr_sort(instants, count);
	int newcount = temporalinstarr_remove_duplicates(instants, count);
	return temporali_make(instants, newcount);
}

/**
 * @brief Returns a copy of the temporal instant value
 */
TemporalInst *
temporalinst_copy(const TemporalInst *inst)
{
	TemporalInst *result = palloc0(VARSIZE(inst));
	memcpy(result, inst, VARSIZE(inst));
	return result;
}

/**
 * @brief Sets the value and the timestamp of the temporal instant value
 * @note This function only works for for base types passed by value.
 * 		This should be ensured by the calling function!
 * @param[in,out] inst Temporal value to be modified
 * @param[in] value Value
 * @param[in] t Timestamp
 */
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

/**
 * @brief Returns the string representation of the temporal value
 * @param[in] inst Temporal value
 * @param[in] value_out Function called to output the base value
 *		depending on its Oid
 */
char *
temporalinst_to_string(const TemporalInst *inst, char *(*value_out)(Oid, Datum))
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

/**
 * @brief Write the binary representation of the temporal value into the buffer
 * @param[in] inst Temporal value
 * @param[in] buf Buffer
 */
void
temporalinst_write(const TemporalInst *inst, StringInfo buf)
{
	bytea *bt = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(inst->t));
	bytea *bv = call_send(inst->valuetypid, temporalinst_value(inst));
	pq_sendbytes(buf, VARDATA(bt), VARSIZE(bt) - VARHDRSZ);
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(buf, VARSIZE(bv) - VARHDRSZ, 4) ;
#else
	pq_sendint32(buf, VARSIZE(bv) - VARHDRSZ) ;
#endif
	pq_sendbytes(buf, VARDATA(bv), VARSIZE(bv) - VARHDRSZ);
}

/**
 * @brief Returns a new temporal value from its binary representation 
 *		read from the buffer
 * @param[in] buf Buffer
 * @param[in] valuetypid Oid of the base type
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

/**
 * @brief Temporally intersect the two temporal values
 * @return Returns false if the values do not overlap on time
 */
bool
intersection_temporalinst_temporalinst(const TemporalInst *inst1, const TemporalInst *inst2,
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

/**
 * @brief Cast the temporal integer value as a temporal float value
 */
TemporalInst *
tintinst_to_tfloatinst(const TemporalInst *inst)
{
	TemporalInst *result = temporalinst_copy(inst);
	result->valuetypid = FLOAT8OID;
	MOBDB_FLAGS_SET_LINEAR(result->flags, true);
	Datum *value_ptr = temporalinst_value_ptr(result);
	*value_ptr = Float8GetDatum((double)DatumGetInt32(temporalinst_value(inst)));
	return result;
}

/**
 * @brief Cast the temporal float value as a temporal integer value
 */
TemporalInst *
tfloatinst_to_tintinst(const TemporalInst *inst)
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

/**
 * @brief Transform the temporal instant value into a temporal instant set value
 */
TemporalI *
temporalinst_to_temporali(const TemporalInst *inst)
{
	return temporali_make((TemporalInst **)&inst, 1);
}

/**
 * @brief Transform the temporal instant set value into a temporal instant value
 */
TemporalInst *
temporali_to_temporalinst(const TemporalI *ti)
{
	if (ti->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return temporalinst_copy(temporali_inst_n(ti, 0));
}

/**
 * @brief Transform the temporal sequence value into a temporal instant value
 */
TemporalInst *
temporalseq_to_temporalinst(const TemporalSeq *seq)
{
	if (seq->count != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
			errmsg("Cannot transform input to a temporal instant")));

	return temporalinst_copy(temporalseq_inst_n(seq, 0));
}

/**
 * @brief Transform the temporal sequence set value into a temporal instant value
 */
TemporalInst *
temporals_to_temporalinst(const TemporalS *ts)
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

/**
 * @brief Returns the base value of the temporal value as an array
 */
ArrayType *
temporalinst_values(const TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	return datumarr_to_array(&value, 1, inst->valuetypid);
}

/* Get values */
/**
 * @brief Returns the base value of the temporal float value as a range
 */
ArrayType *
tfloatinst_ranges(const TemporalInst *inst)
{
	Datum value = temporalinst_value(inst);
	RangeType *range = range_make(value, value, true, true, inst->valuetypid);
	ArrayType *result = rangearr_to_array(&range, 1, type_oid(T_FLOATRANGE));
	pfree(range);
	return result;
}

/**
 * @brief Returns the time on which the temporal value is defined as a period set
 */
PeriodSet *
temporalinst_get_time(const TemporalInst *inst)
{
	PeriodSet *result = timestamp_to_periodset_internal(inst->t);
	return result;
}

/**
 * @brief Returns the bounding period on which the temporal instant value is defined
 */
void
temporalinst_period(Period *p, const TemporalInst *inst)
{
	return period_set(p, inst->t, inst->t, true, true);
}

/**
 * @brief Returns the timestamp of the temporal value as an array
 */
ArrayType *
temporalinst_timestamps(const TemporalInst *inst)
{
	TimestampTz t = inst->t;
	return timestamparr_to_array(&t, 1);
}

/**
 * @brief Returns the temporal value as an array
 */
ArrayType *
temporalinst_instants_array(const TemporalInst *inst)
{
	return temporalarr_to_array((Temporal **)(&inst), 1);
}

/**
 * @brief Shift the time span of the temporal value by the interval
 */
TemporalInst *
temporalinst_shift(const TemporalInst *inst, const Interval *interval)
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

/**
 * @brief Returns true if temporal value is ever equal to the base value
 */
bool
temporalinst_ever_eq(const TemporalInst *inst, Datum value)
{
	return datum_eq(temporalinst_value(inst), value, inst->valuetypid);
}

/**
 * @brief Returns true if temporal value is always equal to the base value
 */
bool
temporalinst_always_eq(const TemporalInst *inst, Datum value)
{
	return datum_eq(temporalinst_value(inst), value, inst->valuetypid);
}

/*****************************************************************************/

/**
 * @brief Returns true if the temporal value is ever less than the base value
 */
bool
temporalinst_ever_lt(const TemporalInst *inst, Datum value)
{
	return datum_lt(temporalinst_value(inst), value, inst->valuetypid);
}

/**
 * @brief Returns true if the temporal value is ever less than or equal to
 *		the base value
 */
bool
temporalinst_ever_le(const TemporalInst *inst, Datum value)
{
	return datum_le(temporalinst_value(inst), value, inst->valuetypid);
}

/**
 * @brief Returns true if the temporal value is always less than the base value
 */
bool
temporalinst_always_lt(const TemporalInst *inst, Datum value)
{
	return datum_lt(temporalinst_value(inst), value, inst->valuetypid);
}

/**
 * @brief Returns true if the temporal value is always less than or equal to
 *		the base value
 */
bool
temporalinst_always_le(const TemporalInst *inst, Datum value)
{
	return datum_le(temporalinst_value(inst), value, inst->valuetypid);
}

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

/**
 * @brief Restricts the temporal value to the base value
 */
TemporalInst *
temporalinst_at_value(const TemporalInst *inst, Datum value)
{
	if (datum_ne(value, temporalinst_value(inst), inst->valuetypid))
		return NULL;
	return temporalinst_copy(inst);
}
  
/**
 * @brief Restricts the temporal value to the complement of the base value
 */
TemporalInst *
temporalinst_minus_value(const TemporalInst *inst, Datum value)
{
	if (datum_eq(value, temporalinst_value(inst), inst->valuetypid))
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the array of base values
 * @pre There are no duplicates values in the array
 */
TemporalInst *
temporalinst_at_values(const TemporalInst *inst, const Datum *values, int count)
{
	Datum value = temporalinst_value(inst);
	for (int i = 0; i < count; i++)
		if (datum_eq(value, values[i], inst->valuetypid))
			return temporalinst_copy(inst);
	return NULL;
}

/**
 * @brief Restricts the temporal value to the complement of the array of base values
 * @pre There are no duplicates values in the array
 */
TemporalInst *
temporalinst_minus_values(const TemporalInst *inst, const Datum *values, int count)
{
	Datum value = temporalinst_value(inst);
	for (int i = 0; i < count; i++)
		if (datum_eq(value, values[i], inst->valuetypid))
			return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the range of base values
 */
TemporalInst *
tnumberinst_at_range(const TemporalInst *inst, RangeType *range)
{
	Datum d = temporalinst_value(inst);
	TypeCacheEntry* typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range, d);
	if (!contains) 
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the complement of the range of base values
 */
TemporalInst *
tnumberinst_minus_range(const TemporalInst *inst, RangeType *range)
{
	Datum d = temporalinst_value(inst);
	TypeCacheEntry* typcache = lookup_type_cache(range->rangetypid, TYPECACHE_RANGE_INFO);
	bool contains = range_contains_elem_internal(typcache, range, d);
	if (contains)
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the array of ranges of base values
 * @pre The ranges are normalized
 */
TemporalInst *
tnumberinst_at_ranges(const TemporalInst *inst, RangeType **normranges, int count)
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

/**
 * @brief Restricts the temporal value to the complement of the array of ranges of base values
 * @pre The ranges are normalized
 */
TemporalInst *
tnumberinst_minus_ranges(const TemporalInst *inst, RangeType **normranges, int count)
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

/**
 * @brief Restricts the temporal value to the timestamp
 * @note Since the corresponding function for temporal sequences need to 
 * 		interpolate the value, it is necessary to return a copy of the value
 */
TemporalInst *
temporalinst_at_timestamp(const TemporalInst *inst, TimestampTz t)
{
	if (t == inst->t)
		return temporalinst_copy(inst);
	return NULL;
}

/**
 * @brief Returns the base value of the temporal value at the timestamp
 * @note Since the corresponding function for temporal sequences need to 
 * 		interpolate the value, it is necessary to return a copy of the value
 */
bool
temporalinst_value_at_timestamp(const TemporalInst *inst, TimestampTz t, Datum *result)
{
	if (t != inst->t)
		return false;
	*result = temporalinst_value_copy(inst);
	return true;
}

/**
 * @brief Restricts the temporal value to the complement of the timestamp
 */
TemporalInst *
temporalinst_minus_timestamp(const TemporalInst *inst, TimestampTz t)
{
	if (t == inst->t)
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the timestamp set
 */
TemporalInst *
temporalinst_at_timestampset(const TemporalInst *inst, const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
			return temporalinst_copy(inst);
	return NULL;
}

/**
 * @brief Restricts the temporal value to the complement of the timestamp set
 */
TemporalInst *
temporalinst_minus_timestampset(const TemporalInst *inst, const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
			return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the period
 */
TemporalInst *
temporalinst_at_period(const TemporalInst *inst, const Period *period)
{
	if (!contains_period_timestamp_internal(period, inst->t))
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the complement of the period
 */
TemporalInst *
temporalinst_minus_period(const TemporalInst *inst, const Period *period)
{
	if (contains_period_timestamp_internal(period, inst->t))
		return NULL;
	return temporalinst_copy(inst);
}

/**
 * @brief Restricts the temporal value to the period set
 */
TemporalInst *
temporalinst_at_periodset(const TemporalInst *inst,const  PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return temporalinst_copy(inst);
	return NULL;
}

/**
 * @brief Restricts the temporal value to the complement of the period set
 */
TemporalInst *
temporalinst_minus_periodset(const TemporalInst *inst, const PeriodSet *ps)
{
	for (int i = 0; i < ps->count; i++)
		if (contains_period_timestamp_internal(periodset_per_n(ps, i), inst->t))
			return NULL;
	return temporalinst_copy(inst);
}

/*****************************************************************************
 * Intersects functions 
 *****************************************************************************/

/**
 * @brief Returns true if the temporal value intersects the timestamp
 */
bool
temporalinst_intersects_timestamp(const TemporalInst *inst, TimestampTz t)
{
	return (inst->t == t);
}

/**
 * @brief Returns true if the temporal value intersects the timestamp set
 */
bool
temporalinst_intersects_timestampset(const TemporalInst *inst, 
	const TimestampSet *ts)
{
	for (int i = 0; i < ts->count; i++)
		if (inst->t == timestampset_time_n(ts, i))
			return true;
	return false;
}

/**
 * @brief Returns true if the temporal value intersects the period
 */
bool
temporalinst_intersects_period(const TemporalInst *inst, const Period *p)
{
	return contains_period_timestamp_internal(p, inst->t);
}

/**
 * @brief Returns true if the temporal value intersects the period set
 */
bool
temporalinst_intersects_periodset(const TemporalInst *inst, const PeriodSet *ps)
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

/**
 * @brief Returns true if the temporal values are equal
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of two
 * 		temporal instant values of the same base type are equal.
 * 		This hypothesis may change in the future and the function must be
 * 		adapted accordingly.
 */
bool
temporalinst_eq(const TemporalInst *inst1, const TemporalInst *inst2)
{
	/* Compare values and timestamps */
	Datum value1 = temporalinst_value(inst1);
	Datum value2 = temporalinst_value(inst2);
	return inst1->t == inst2->t && datum_eq(value1, value2, inst1->valuetypid);
}

/**
 * @brief Returns -1, 0, or 1 depending on whether the first temporal value 
 *		is less than, equal, or greater than the second temporal value
 * @note The internal B-tree comparator is not used to increase efficiency.
 * @note This function supposes for optimization purposes that the flags of two
 * 		temporal instant values of the same base type are equal.
 * 		This hypothesis may change in the future and the function must be
 * 		adapted accordingly.
 */
int
temporalinst_cmp(const TemporalInst *inst1, const TemporalInst *inst2)
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
	/* The two values are equal */
	return 0;
}

/*****************************************************************************
 * Function for defining hash index
 * The function reuses the approach for range types for combining the hash of  
 * the lower and upper bounds.
 *****************************************************************************/

/**
 * @brief Returns the hash value of the temporal value
 */
uint32
temporalinst_hash(const TemporalInst *inst)
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
