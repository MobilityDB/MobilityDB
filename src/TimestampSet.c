/*****************************************************************************
 *
 * TimestampSet.c
 *	  Basic functions for set of timestamps.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include <TemporalTypes.h>

/*****************************************************************************
 * General functions
 *****************************************************************************/
 
/* 
 * The memory structure of a TimestampSet with, e.g., 3 timestamps is as follows
 *
 *  --------------------------------------------------------------------
 *	( TimestampSet | offset_0 | offset_1 | offset_2 | offset_3 | )_X | ...
 *	--------------------------------------------------------------------
 *	------------------------------------------------------------
 *	( Timestamp_0 | Timestamp_1 | Timestamp_2 | ( bbox )_Y )_X |
 *	------------------------------------------------------------
 *
 * where the X are unused bytes added for double padding, the Y are unused bytes 
 * added for int4 padding, offset_0 to offset_2 are offsets for the corresponding 
 * timestamps, and offset_3 is the offset for the bounding box.
 */
 
/* Pointer to array of offsets of the TimestampSet */

static size_t *
timestampset_offsets_ptr(TimestampSet *ts)
{
	return (size_t *) (((char *)ts) + sizeof(TimestampSet));
}

/* Pointer to the first timestamp */

static char * 
timestampset_data_ptr(TimestampSet *ts)
{
	return (char *)ts + double_pad(sizeof(TimestampSet) + 
		sizeof(size_t) * (ts->count+1));
}

/* N-th TimestampTz of a TimestampSet */

TimestampTz
timestampset_time_n(TimestampSet *ts, int index)
{
	size_t *offsets = timestampset_offsets_ptr(ts);
	TimestampTz *result = (TimestampTz *) (timestampset_data_ptr(ts) + offsets[index]);
	return *result;
}

/* Bounding box of a TimestampSet */

Period *
timestampset_bbox(TimestampSet *ts) 
{
	size_t *offsets = timestampset_offsets_ptr(ts);
	return (Period *)(timestampset_data_ptr(ts) + offsets[ts->count]);
}

/* Construct a TimestampSet from an array of TimestampTz */

TimestampSet *
timestampset_from_timestamparr_internal(TimestampTz *times, int count)
{
	Period bbox;
	/* Test the validity of the timestamps */
	for (int i = 0; i < count-1; i++)
	{
		if (timestamp_cmp_internal(times[i], times[i+1]) >= 0)
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
	period_set(&bbox, times[0], times[count-1], true, true);
	offsets[count] = pos;
	memcpy(((char *) result) + pdata + pos, &bbox, sizeof(Period));
	pos += double_pad(sizeof(Period));
	return result;
}

TimestampSet *
timestampset_copy(TimestampSet *ts)
{
	TimestampSet *result = palloc(VARSIZE(ts));
	memcpy(result, ts, VARSIZE(ts));
	return result;
}

/*
 * Binary search of a timestamptz in a timestampset.
 * If the timestamp is found, the position of the period is returned in pos.
 * Otherwise, return a number encoding whether it is before, between two 
 * timestamps or after. For example, given 3 timestamps, the result of the 
 * function if the timestamp is not found will be as follows: 
 *			0   	1		2
 *			|		|		|
 * 1)	t^ 							=> result = 0
 * 2)			t^ 					=> result = 1
 * 3)					t^ 			=> result = 2
 * 4)							t^	=> result = 3
 */

bool 
timestampset_find_timestamp(TimestampSet *ts, TimestampTz t, int *pos) 
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
			*pos = middle;
			return true;
		}
		if (cmp < 0)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (middle == ts->count)
		middle++;
	*pos = middle;
	return false;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Input function */
 
PG_FUNCTION_INFO_V1(timestampset_in);

PGDLLEXPORT Datum
timestampset_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	TimestampSet *result = timestampset_parse(&input);
	PG_RETURN_POINTER(result);
}

/* Convert to string */
 
char *
timestampset_to_string(TimestampSet *ts)
{
	char **strings = palloc((int) (sizeof(char *) * ts->count));
	size_t outlen = 0;

	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		strings[i] = call_output(TIMESTAMPTZOID, t);
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

/* Output function */

PG_FUNCTION_INFO_V1(timestampset_out);

PGDLLEXPORT Datum
timestampset_out(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	char *result = timestampset_to_string(ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_CSTRING(result);
}

/* Send function */
 
PG_FUNCTION_INFO_V1(timestampset_send);

PGDLLEXPORT Datum
timestampset_send(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendint(&buf, ts->count, 4);
	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		bytea *t1 = call_send(TIMESTAMPTZOID, t);
		pq_sendbytes(&buf, VARDATA(t1), VARSIZE(t1) - VARHDRSZ);
		pfree(t1);
	}
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Receive function */

PG_FUNCTION_INFO_V1(timestampset_recv);

PGDLLEXPORT Datum
timestampset_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	int count = (int) pq_getmsgint(buf, 4);
	TimestampTz *times = palloc(sizeof(TimestampTz) * count);
	for (int i = 0; i < count; i++)
		times[i] = call_recv(TIMESTAMPTZOID, buf);
	TimestampSet *result = timestampset_from_timestamparr_internal(times, count);
	pfree(times);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor function
 *****************************************************************************/

/* Construct a TimestampSet from an array of TimestampTz */

PG_FUNCTION_INFO_V1(timestampset_from_timestamparr);

PGDLLEXPORT Datum
timestampset_from_timestamparr(PG_FUNCTION_ARGS)
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
	TimestampSet *result = timestampset_from_timestamparr_internal(times, count);
	
	pfree(times);
	PG_FREE_IF_COPY(array, 0);
	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/* Cast a TimestampTz value as a TimestampSet value */

PG_FUNCTION_INFO_V1(timestamp_as_timestampset);

PGDLLEXPORT Datum
timestamp_as_timestampset(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	TimestampSet *result = timestampset_from_timestamparr_internal(&t, 1);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(timestampset_mem_size);

PGDLLEXPORT Datum
timestampset_mem_size(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Datum result = Int32GetDatum((int)VARSIZE(DatumGetPointer(ts)));
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_DATUM(result);
}

/* Bounding period on which the temporal value is defined */

void
timestampset_timespan_internal(Period *p, TimestampSet *ts)
{
	TimestampTz start = timestampset_time_n(ts, 0);
	TimestampTz end = timestampset_time_n(ts, ts->count - 1);
	period_set(p, start, end, true, true);
}

PG_FUNCTION_INFO_V1(timestampset_timespan);

PGDLLEXPORT Datum
timestampset_timespan(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	Period *result = (Period *)palloc(sizeof(Period));
	timestampset_timespan_internal(result, ts);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_POINTER(result);
}

/* Number of timestamps */

PG_FUNCTION_INFO_V1(timestampset_num_timestamps);

PGDLLEXPORT Datum
timestampset_num_timestamps(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_INT32(ts->count);
}

/* Start timestamptz */

PG_FUNCTION_INFO_V1(timestampset_start_timestamp);

PGDLLEXPORT Datum
timestampset_start_timestamp(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TimestampTz result = timestampset_time_n(ts, 0);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/* End timestamptz */

PG_FUNCTION_INFO_V1(timestampset_end_timestamp);

PGDLLEXPORT Datum
timestampset_end_timestamp(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	TimestampTz result = timestampset_time_n(ts, ts->count - 1);
	PG_FREE_IF_COPY(ts, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/* N-th timestamptz */

PG_FUNCTION_INFO_V1(timestampset_timestamp_n);

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

/* Timestamps */

TimestampTz *
timestampset_timestamps_internal(TimestampSet *ts)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
	for (int i = 0; i < ts->count; i++) 
		times[i] = timestampset_time_n(ts, i);
	return times;
}

PG_FUNCTION_INFO_V1(timestampset_timestamps);

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

/* Shift the period set by an interval */

TimestampSet *
timestampset_shift_internal(TimestampSet *ts, Interval *interval)
{
	TimestampTz *times = palloc(sizeof(TimestampTz) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		times[i] = DatumGetTimestampTz(
			DirectFunctionCall2(timestamptz_pl_interval,
			TimestampTzGetDatum(t), PointerGetDatum(interval)));
	}
	TimestampSet *result = timestampset_from_timestamparr_internal(times, ts->count);
	pfree(times);
	return result;
}

PG_FUNCTION_INFO_V1(timestampset_shift);

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

/* B-tree comparator */

int
timestampset_cmp_internal(TimestampSet *ts1, TimestampSet *ts2)
{
	int count1 = ts1->count;
	int count2 = ts2->count;
	int count = count1 < count2 ? count1 : count2;
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
		if (count < count1) /* ts1 has more timestamps than ts2 */
			result = 1;
		else if (count < count2) /* ts2 has more timestamps than ts1 */
			result = -1;
		else
			result = 0;
	}
	return result;
}

PG_FUNCTION_INFO_V1(timestampset_cmp);

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

/* 
 * Equality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
timestampset_eq_internal(TimestampSet *ts1, TimestampSet *ts2)
{
	if (ts1->count != ts2->count)
		return false;
	/* ts1 and ts2 have the same number of TimestampSet */
	for (int i = 0; i < ts1->count; i++)
	{
		TimestampTz t1 = timestampset_time_n(ts1, i);
		TimestampTz t2 = timestampset_time_n(ts2, i);
		if (timestamp_cmp_internal(t1, t2) != 0)
			return false;
	}
	/* All timestamps of the two TimestampSet are equal */
	return true;
}

PG_FUNCTION_INFO_V1(timestampset_eq);

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

/* 
 * Inequality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
timestampset_ne_internal(TimestampSet *ts1, TimestampSet *ts2)
{
	return !timestampset_eq_internal(ts1, ts2);
}

PG_FUNCTION_INFO_V1(timestampset_ne);

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

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(timestampset_lt);

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
