/*****************************************************************************
 *
 * timestampset.c
 *	  Basic functions for set of timestamps.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "timestampset.h"

#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "temporal.h"
#include "temporal_parser.h"
#include "period.h"
#include "temporal_util.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/
 
/**
 * Returns a pointer to the array of offsets of the timestamp set value
 */
static size_t *
timestampset_offsets_ptr(const TimestampSet *ts)
{
	return (size_t *) (((char *)ts) + sizeof(TimestampSet));
}

/**
 * Returns a pointer to the first timestamp of the timestamp set value
 */
static char * 
timestampset_data_ptr(const TimestampSet *ts)
{
	return (char *)ts + double_pad(sizeof(TimestampSet) + 
		sizeof(size_t) * (ts->count + 1));
}

/**
 * Returns the n-th timestamp of the timestamp set value
 */
TimestampTz
timestampset_time_n(const TimestampSet *ts, int index)
{
	size_t *offsets = timestampset_offsets_ptr(ts);
	TimestampTz *result = (TimestampTz *) (timestampset_data_ptr(ts) + offsets[index]);
	return *result;
}

/**
 * Returns a pointer to the precomputed bounding box of the timestamp set value
 */
Period *
timestampset_bbox(const TimestampSet *ts)
{
	size_t *offsets = timestampset_offsets_ptr(ts);
	return (Period *)(timestampset_data_ptr(ts) + offsets[ts->count]);
}

/**
 * Construct a timestamp set from an array of timestamps 
 * 
 * For example, the memory structure of a timestamp set with 3 
 * timestamps is as follows
 * @code
 * --------------------------------------------------------------------
 * ( TimestampSet | offset_0 | offset_1 | offset_2 | offset_3 | )_X | ...
 * --------------------------------------------------------------------
 * ------------------------------------------------------------
 * ( Timestamp_0 | Timestamp_1 | Timestamp_2 | ( bbox )_Y )_X |
 * ------------------------------------------------------------
 * @endcode
 * where the `X` are unused bytes added for double padding, the `Y` are 
 * unused bytes added for int4 padding, `offset_0` to `offset_2` are 
 * offsets for the corresponding timestamps, and `offset_3` is the offset 
 * for the bounding box which is a period.
 *
 * @param[in] times Array of timestamps
 * @param[in] count Number of elements in the array
 */
TimestampSet *
timestampset_make_internal(const TimestampTz *times, int count)
{
	Period bbox;
	/* Test the validity of the timestamps */
	for (int i = 0; i < count - 1; i++)
	{
		if (times[i] >= times[i + 1])
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
				errmsg("Invalid value for timestamp set")));
	}

	size_t memsize = double_pad(sizeof(TimestampTz) * count + double_pad(sizeof(Period)));
	/* Array of pointers containing the pointers to the component timestamps,
	   and a pointer to the bbox */
	size_t pdata = double_pad(sizeof(TimestampSet) + (count + 1) * sizeof(size_t));
	/* Create the TimestampSet */
	TimestampSet *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = count;

	size_t *offsets = timestampset_offsets_ptr(result);
	size_t pos = 0;	
	for (int i = 0; i < count; i++)
	{
		memcpy(((char *) result) + pdata + pos, &times[i], sizeof(TimestampTz));
		offsets[i] = pos;
		pos += sizeof(TimestampTz);
	}
	/* Precompute the bounding box */
	period_set(&bbox, times[0], times[count - 1], true, true);
	offsets[count] = pos;
	memcpy(((char *) result) + pdata + pos, &bbox, sizeof(Period));
	return result;
}

TimestampSet *
timestampset_copy(const TimestampSet *ts)
{
	TimestampSet *result = palloc(VARSIZE(ts));
	memcpy(result, ts, VARSIZE(ts));
	return result;
}

/**
 * Returns the location of the timestamp in the timestamp set value
 * using binary search
 *
 * If the timestamp is found, the index of the timestamp is returned
 * in the output parameter. Otherwise, return a number encoding whether it 
 * is before, between two timestamps, or after the timestamp set value. 
 * For example, given a value composed of 3 timestamps and a timestamp, 
 * the result of the function is as follows: 
 * @code
 *            0       1        2
 *            |       |        |
 * 1)    t^                            => loc = 0
 * 2)        t^                        => loc = 0
 * 3)            t^                    => loc = 1
 * 4)                    t^            => loc = 2
 * 5)                            t^    => loc = 3
 * @endcode
 *
 * @param[in] ts Timestamp set value
 * @param[in] t Timestamp
 * @param[out] loc Location
 * @result Returns true if the timestamp is contained in the timestamp set value
 */
bool 
timestampset_find_timestamp(const TimestampSet *ts, TimestampTz t, int *loc)
{
	int first = 0;
	int last = ts->count - 1;
	int middle = 0; /* make compiler quiet */
	while (first <= last) 
	{
		middle = (first + last)/2;
		TimestampTz t1 = timestampset_time_n(ts, middle);
		int cmp = timestamp_cmp_internal(t, t1);
		if (cmp == 0)
		{
			*loc = middle;
			return true;
		}
		if (cmp < 0)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (middle == ts->count)
		middle++;
	*loc = middle;
	return false;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestampset_in);
/**
 * Input function for timestamp set values
 */
PGDLLEXPORT Datum
timestampset_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	TimestampSet *result = timestampset_parse(&input);
	PG_RETURN_POINTER(result);
}

/**
 * Returns the string representation of the timestamp set value
 */
char *
timestampset_to_string(const TimestampSet *ts)
{
	char **strings = palloc(sizeof(char *) * ts->count);
	size_t outlen = 0;

	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		strings[i] = call_output(TIMESTAMPTZOID, TimestampTzGetDatum(t));
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

PG_FUNCTION_INFO_V1(timestampset_out);
/**
 * Output function for timestamp set values
 */ 
PGDLLEXPORT Datum
timestampset_out(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	char *result = timestampset_to_string(ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(timestampset_send);
/**
 * Send function for timestamp set values
 */
PGDLLEXPORT Datum
timestampset_send(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
#if MOBDB_PGSQL_VERSION < 110000
	pq_sendint(&buf, (uint32) ts->count, 4);
#else
	pq_sendint32(&buf, ts->count);
#endif
	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		bytea *t1 = call_send(TIMESTAMPTZOID, TimestampTzGetDatum(t));
		pq_sendbytes(&buf, VARDATA(t1), VARSIZE(t1) - VARHDRSZ);
		pfree(t1);
	}
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

PG_FUNCTION_INFO_V1(timestampset_recv);
/**
 * Receive function for timestamp set values
 */
PGDLLEXPORT Datum
timestampset_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	int count = (int) pq_getmsgint(buf, 4);
	TimestampTz *times = palloc(sizeof(TimestampTz) * count);
	for (int i = 0; i < count; i++)
		times[i] = call_recv(TIMESTAMPTZOID, buf);
	TimestampSet *result = timestampset_make_internal(times, count);
	pfree(times);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestampset_make);
/**
 * Construct a timestamp set value from an array of timestamp values
 */
PGDLLEXPORT Datum
timestampset_make(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR), 
			errmsg("A timestamp set must have at least one timestamp")));
	}
	
	TimestampTz *times = timestamparr_extract(array, &count);
	TimestampSet *result = timestampset_make_internal(times, count);
	
	pfree(times);
	PG_FREE_IF_COPY(array, 0);
	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestamp_to_timestampset);
/**
 * Cast a timestamp value as a timestamp set value
 */
PGDLLEXPORT Datum
timestamp_to_timestampset(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	TimestampSet *result = timestampset_make_internal(&t, 1);
	PG_RETURN_POINTER(result);
}

/**
 * Returns the bounding period on which the timestamp set value is defined
 * (internal function)
 */
void
timestampset_to_period_internal(Period *p, const TimestampSet *ts)
{
	TimestampTz start = timestampset_time_n(ts, 0);
	TimestampTz end = timestampset_time_n(ts, ts->count - 1);
	period_set(p, start, end, true, true);
}

PG_FUNCTION_INFO_V1(timestampset_to_period);
/**
 * Returns the bounding period on which the timestamp set value is defined
 */
PGDLLEXPORT Datum
timestampset_to_period(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Period *result = timestampset_bbox(ts);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestampset_mem_size);
/**
 * Returns the size in bytes of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_mem_size(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Datum result = Int32GetDatum((int)VARSIZE(DatumGetPointer(ts)));
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(timestampset_timespan);
/**
 * Returns the timespan of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_timespan(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TimestampTz start = timestampset_time_n(ts, 0);
	TimestampTz end = timestampset_time_n(ts, ts->count - 1);
	Datum result = call_function2(timestamp_mi, TimestampTzGetDatum(end), 
		TimestampTzGetDatum(start));
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(timestampset_num_timestamps);
/**
 * Returns the number of timestamps of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_num_timestamps(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_INT32(ts->count);
}

PG_FUNCTION_INFO_V1(timestampset_start_timestamp);
/**
 * Returns the start timestamp of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_start_timestamp(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TimestampTz result = timestampset_time_n(ts, 0);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(timestampset_end_timestamp);
/**
 * Returns the end timestamp of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_end_timestamp(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TimestampTz result = timestampset_time_n(ts, ts->count - 1);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

PG_FUNCTION_INFO_V1(timestampset_timestamp_n);
/**
 * Returns the n-th timestamp of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_timestamp_n(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	int n = PG_GETARG_INT32(1); /* Assume 1-based */
	if (n < 1 || n > ts->count)
	{
		PG_FREE_IF_COPY(ts, 0);
		PG_RETURN_NULL();
	}
	TimestampTz result = timestampset_time_n(ts, n - 1);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/**
 * Returns the timestamps of the timestamp set value (internal function)
 */
TimestampTz *
timestampset_timestamps_internal(const TimestampSet *ts)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
	for (int i = 0; i < ts->count; i++) 
		times[i] = timestampset_time_n(ts, i);
	return times;
}

PG_FUNCTION_INFO_V1(timestampset_timestamps);
/**
 * Returns the timestamps of the timestamp set value
 */
PGDLLEXPORT Datum
timestampset_timestamps(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TimestampTz *times = timestampset_timestamps_internal(ts);
	ArrayType *result = timestamparr_to_array(times, ts->count);
	pfree(times);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

/**
 * Shift the period set value by the interval (internal function)
 */
TimestampSet *
timestampset_shift_internal(const TimestampSet *ts, const Interval *interval)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		times[i] = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(t), PointerGetDatum(interval)));
	}
	TimestampSet *result = timestampset_make_internal(times, ts->count);
	pfree(times);
	return result;
}

PG_FUNCTION_INFO_V1(timestampset_shift);
/**
 * Shift the period set value by the interval
 */
PGDLLEXPORT Datum
timestampset_shift(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Interval *interval = PG_GETARG_INTERVAL_P(1);
	TimestampSet *result = timestampset_shift_internal(ts, interval);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/**
 * Returns -1, 0, or 1 depending on whether the first timestamp set value 
 * is less than, equal, or greater than the second temporal value (internal
 * function)
 *
 * @note Function used for B-tree comparison
 */
int
timestampset_cmp_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
	int count = Min(ts1->count, ts2->count);
	int result = 0;
	for (int i = 0; i < count; i++)
	{
		TimestampTz t1 = timestampset_time_n(ts1, i);
		TimestampTz t2 = timestampset_time_n(ts2, i);
		result = timestamp_cmp_internal(t1, t2);
		if (result) 
			break;
	}
	/* The first count times of the two TimestampSet are equal */
	if (!result) 
	{
		if (count < ts1->count) /* ts1 has more timestamps than ts2 */
			result = 1;
		else if (count < ts2->count) /* ts2 has more timestamps than ts1 */
			result = -1;
		else
			result = 0;
	}
	return result;
}

PG_FUNCTION_INFO_V1(timestampset_cmp);
/**
 * Returns -1, 0, or 1 depending on whether the first timestamp set value 
 * is less than, equal, or greater than the second temporal value
 */
PGDLLEXPORT Datum
timestampset_cmp(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	int cmp = timestampset_cmp_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_INT32(cmp);
}

/**
 * Returns true if the first timestamp set value is equal to the second one
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency 
 */
bool
timestampset_eq_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
	if (ts1->count != ts2->count)
		return false;
	/* ts1 and ts2 have the same number of TimestampSet */
	for (int i = 0; i < ts1->count; i++)
	{
		TimestampTz t1 = timestampset_time_n(ts1, i);
		TimestampTz t2 = timestampset_time_n(ts2, i);
		if (t1 != t2)
			return false;
	}
	/* All timestamps of the two TimestampSet are equal */
	return true;
}

PG_FUNCTION_INFO_V1(timestampset_eq);
/**
 * Returns true if the first timestamp set value is equal to the second one
 */
PGDLLEXPORT Datum
timestampset_eq(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	bool result = timestampset_eq_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_BOOL(result);
}

/**
 * Returns true if the first timestamp set value is different from the second one
 * (internal function)
 *
 * @note The internal B-tree comparator is not used to increase efficiency 
 */
bool
timestampset_ne_internal(const TimestampSet *ts1, const TimestampSet *ts2)
{
	return !timestampset_eq_internal(ts1, ts2);
}

PG_FUNCTION_INFO_V1(timestampset_ne);
/**
 * Returns true if the first timestamp set value is different from the second one
 */
PGDLLEXPORT Datum
timestampset_ne(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	bool result = timestampset_ne_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_BOOL(result);
}


PG_FUNCTION_INFO_V1(timestampset_lt);
/**
 * Returns true if the first timestamp set value is less than the second one
 */
PGDLLEXPORT Datum
timestampset_lt(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	int cmp = timestampset_cmp_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(timestampset_le);
/**
 * Returns true if the first timestamp set value is less than 
 * or equal to the second one
 */
PGDLLEXPORT Datum
timestampset_le(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	int cmp = timestampset_cmp_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(timestampset_ge);
/**
 * Returns true if the first timestamp set value is greater than 
 * or equal to the second one
 */
PGDLLEXPORT Datum
timestampset_ge(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	int cmp = timestampset_cmp_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(timestampset_gt);
/**
 * Returns true if the first timestamp set value is greater than the second one
 */
PGDLLEXPORT Datum
timestampset_gt(PG_FUNCTION_ARGS)
{
	TimestampSet *ts1 = PG_GETARG_TIMESTAMPSET(0);
	TimestampSet *ts2 = PG_GETARG_TIMESTAMPSET(1);
	int cmp = timestampset_cmp_internal(ts1, ts2);
	PG_FREE_IF_COPY(ts1, 0);
	PG_FREE_IF_COPY(ts2, 1);
	PG_RETURN_BOOL(cmp > 0);
}

/*****************************************************************************/
