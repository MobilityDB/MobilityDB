/*****************************************************************************
 *
 * PeriodSet.c
 *	Basic functions for set of periods.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "PeriodSet.h"

#include <assert.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/rangetypes.h>
#include <utils/timestamp.h>

#include "TimestampSet.h"
#include "Period.h"
#include "TimeOps.h"
#include "TemporalUtil.h"
#include "Parser.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/
 
/* 
 * The memory structure of a PeriodSet with, e.g., 3 periods is as follows
 *
 *  --------------------------------------------------------------------
 *	( PeriodSet | offset_0 | offset_1 | offset_2 | offset_3 | )_X | ...
 *	--------------------------------------------------------------------
 *	--------------------------------------------------------------------
 *	(( Period_0 )_Y | ( Period_1 )_Y | ( Period_2 )_Y | ( bbox )_Y )_X |
 *	--------------------------------------------------------------------
 *
 * where the X are unused bytes added for double padding, the Y are unused bytes 
 * added for int4 padding, offset_0 to offset_2 are offsets for the corresponding 
 * periods, and offset_3 is the offset for the bounding box which is a Period.
 */
 
/* Pointer to array of offsets of the PeriodSet */

static size_t *
periodset_offsets_ptr(PeriodSet *ps)
{
	return (size_t *) (((char *)ps) + sizeof(PeriodSet));
}

/* Pointer to the first period */

static char * 
periodset_data_ptr(PeriodSet *ps)
{
	return (char *)ps + double_pad(sizeof(PeriodSet) + 
		sizeof(size_t) * (ps->count+1));
}

/* N-th Period of a PeriodSet */

Period *
periodset_per_n(PeriodSet *ps, int index)
{
	size_t *offsets = periodset_offsets_ptr(ps);
	return (Period *) (periodset_data_ptr(ps) + offsets[index]);
}

/* Bounding box of a PeriodSet */

Period *
periodset_bbox(PeriodSet *ps) 
{
	size_t *offsets = periodset_offsets_ptr(ps);
	assert(offsets[ps->count] != 0);
	return (Period *)(periodset_data_ptr(ps) + offsets[ps->count]);
}

/* Construct a PeriodSet from an array of Period */

PeriodSet *
periodset_from_periodarr_internal(Period **periods, int count, bool normalize)
{
	Period bbox;
	/* Test the validity of the periods */
	for (int i = 0; i < count-1; i++)
	{
		if (timestamp_cmp_internal(periods[i]->upper, periods[i+1]->lower) > 0 ||
			(timestamp_cmp_internal(periods[i]->upper, periods[i+1]->lower) == 0 &&
			periods[i]->upper_inc && periods[i+1]->lower_inc))
			ereport(ERROR, (errcode(ERRCODE_RESTRICT_VIOLATION),
				errmsg("Invalid value for period set")));
	}

	Period **newperiods = periods;
	int newcount = count;
	if (normalize && count > 1)
		newperiods = periodarr_normalize(periods, count, &newcount);
	size_t memsize = double_pad(sizeof(Period)) * (newcount+1);
	/* Array of pointers containing the pointers to the component Period,
	   and a pointer to the bbox */
	size_t pdata = double_pad(sizeof(PeriodSet) + (newcount+1) * sizeof(size_t));
	PeriodSet *result = palloc0(pdata + memsize);
	SET_VARSIZE(result, pdata + memsize);
	result->count = newcount;

	size_t *offsets = periodset_offsets_ptr(result);
	size_t pos = 0;	
	for (int i = 0; i < newcount; i++)
	{
		memcpy(((char *) result) + pdata + pos, newperiods[i], sizeof(Period));
		offsets[i] = pos;
		pos += double_pad(sizeof(Period));
	}
	/* Precompute the bounding box */
	period_set(&bbox, newperiods[0]->lower, newperiods[newcount-1]->upper,
		newperiods[0]->lower_inc, newperiods[newcount-1]->upper_inc);
	offsets[newcount] = pos;
	memcpy(((char *) result) + pdata + pos, &bbox, sizeof(Period));
	pos += double_pad(sizeof(Period));
	/* Normalize */
	if (normalize && count > 1)
	{
		for (int i = 0; i < newcount; i++)
			pfree(newperiods[i]);
		pfree(newperiods);
	}
	return result;
}

PeriodSet *
periodset_copy(PeriodSet *ps)
{
	PeriodSet *result = palloc(VARSIZE(ps));
	memcpy(result, ps, VARSIZE(ps));
	return result;
}

/*
 * Binary search of a timestamptz in a periodset.
 * If the timestamp is found, the position of the period is returned in pos.
 * Otherwise, return a number encoding whether it is before, between two 
 * periods or after. For example, given 3 periods, the result of the 
 * function if the value is not found will be as follows: 
 *				0			1			2
 *			|------|	|------|	|------|   
 * 1)	t^ 											=> result = 0
 * 2)				 t^ 							=> result = 1
 * 3)							 t^ 				=> result = 2
 * 4)										  t^	=> result = 3
 */

bool 
periodset_find_timestamp(PeriodSet *ps, TimestampTz t, int *pos) 
{
	int first = 0;
	int last = ps->count - 1;
	int middle = 0; /* make compiler quiet */
	Period *p = NULL; /* make compiler quiet */
	while (first <= last) 
	{
		middle = (first + last)/2;
		p = periodset_per_n(ps, middle);
		if (contains_period_timestamp_internal(p, t))
		{
			*pos = middle;
			return true;
		}
		if (timestamp_cmp_internal(t, p->lower) <= 0)
			last = middle - 1;
		else
			first = middle + 1;
	}
	if (timestamp_cmp_internal(t, p->upper) >= 0)
		middle++;
	*pos = middle;
	return false;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Input function */
 
PG_FUNCTION_INFO_V1(periodset_in);

PGDLLEXPORT Datum
periodset_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	PeriodSet *result = periodset_parse(&input);
	PG_RETURN_POINTER(result);
}

/* Convert to string */
 
char *
periodset_to_string(PeriodSet *ps)
{
	char **strings = palloc((int) (sizeof(char *) * ps->count));
	size_t outlen = 0;

	for (int i = 0; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		strings[i] = period_to_string(p);
		outlen += strlen(strings[i]) + 2;
	}
	char *result = palloc(outlen + 3);
	result[outlen] = '\0';
	result[0] = '{';
	size_t pos = 1;
	for (int i = 0; i < ps->count; i++)
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

PG_FUNCTION_INFO_V1(periodset_out);

PGDLLEXPORT Datum
periodset_out(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	char *result = periodset_to_string(ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_CSTRING(result);
}

/* Send function */
 
PG_FUNCTION_INFO_V1(periodset_send);

PGDLLEXPORT Datum
periodset_send(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendint(&buf, ps->count, 4);
	for (int i = 0; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		period_send_internal(p, &buf);
	}
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Receive function */

PG_FUNCTION_INFO_V1(periodset_recv);

PGDLLEXPORT Datum
periodset_recv(PG_FUNCTION_ARGS)
{
	StringInfo buf = (StringInfo)PG_GETARG_POINTER(0);
	int count = (int) pq_getmsgint(buf, 4);
	Period **periods = palloc(sizeof(Period *) * count);
	for (int i = 0; i < count; i++)
		periods[i] = period_recv_internal(buf);
	PeriodSet *result = periodset_from_periodarr_internal(periods, count, false);

	for (int i = 0; i < count; i++)
		pfree(periods[i]);
	pfree(periods);
	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructor function
 ******************************************	**********************************/

/* Construct a PeriodSet from an array of Period */

PG_FUNCTION_INFO_V1(periodset_from_periodarr);

PGDLLEXPORT Datum
periodset_from_periodarr(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count == 0)
	{
		PG_FREE_IF_COPY(array, 0);
		ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR), 
			errmsg("A period set must have at least one period")));
	}
	
	Period **periods = periodarr_extract(array, &count);
	PeriodSet *result = periodset_from_periodarr_internal(periods, count, true);
	
	pfree(periods);
	PG_FREE_IF_COPY(array, 0);
	
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast function
 *****************************************************************************/

/* Cast a TimestampTz value as a PeriodSet value */

PG_FUNCTION_INFO_V1(timestamp_as_periodset);

PGDLLEXPORT Datum
timestamp_as_periodset(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Period *p = period_make(t, t, true, true);
	PeriodSet *result = periodset_from_periodarr_internal(&p, 1, false);
	pfree(p);
	PG_RETURN_POINTER(result);
}

/* Cast a TimestampSet value as a PeriodSet value */

PeriodSet *
timestampset_as_periodset_internal(TimestampSet *ts)
{
	Period **periods = palloc(sizeof(Period *) * ts->count);
	for (int i = 0; i < ts->count; i++)
	{
		TimestampTz t = timestampset_time_n(ts, i);
		periods[i] = period_make(t, t, true, true);
	}
	PeriodSet *result = periodset_from_periodarr_internal(periods, ts->count, false);
	for (int i = 0; i < ts->count; i++)
		pfree(periods[i]);
	pfree(periods);
	return result;
}

PG_FUNCTION_INFO_V1(timestampset_as_periodset);

PGDLLEXPORT Datum
timestampset_as_periodset(PG_FUNCTION_ARGS)
{
	TimestampSet *ts = PG_GETARG_TIMESTAMPSET(0);
	PeriodSet *result = timestampset_as_periodset_internal(ts);
	PG_RETURN_POINTER(result);
}

/* Cast a Period value as a PeriodSet value */

PG_FUNCTION_INFO_V1(period_as_periodset);

PGDLLEXPORT Datum
period_as_periodset(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	PeriodSet *result = periodset_from_periodarr_internal(&p, 1, false);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions 
 *****************************************************************************/

PG_FUNCTION_INFO_V1(periodset_mem_size);

PGDLLEXPORT Datum
periodset_mem_size(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Datum result = Int32GetDatum((int)VARSIZE(DatumGetPointer(ps)));
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_DATUM(result);
}

/* Time of a PeriodSet */

/* Bounding period on which the temporal value is defined */

void
periodset_timespan_internal(Period *p, PeriodSet *ps)
{
	Period *start = periodset_per_n(ps, 0);
	Period *end = periodset_per_n(ps, ps->count - 1);
	period_set(p, start->lower, end->upper, 
		start->lower_inc, end->upper_inc);
}

PG_FUNCTION_INFO_V1(periodset_timespan);

PGDLLEXPORT Datum
periodset_timespan(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *result = (Period *)palloc(sizeof(Period));
	periodset_timespan_internal(result, ps);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/* Duration */

PG_FUNCTION_INFO_V1(periodset_duration);

PGDLLEXPORT Datum
periodset_duration(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *p = periodset_per_n(ps, 0);
	Datum result = call_function2(timestamp_mi, p->upper, p->lower);
	for (int i = 1; i < ps->count; i++)
	{
		p = periodset_per_n(ps, i);
		Datum interval1 = call_function2(timestamp_mi, p->upper, p->lower);
		Datum interval2 = call_function2(interval_pl, result, interval1);
		pfree(DatumGetPointer(result)); pfree(DatumGetPointer(interval1));
		result = interval2;
	}
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_DATUM(result);
}

/* Number of periods */

PG_FUNCTION_INFO_V1(periodset_num_periods);

PGDLLEXPORT Datum
periodset_num_periods(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	int result = ps->count;
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_INT32(result);
}

/* Start period */

PG_FUNCTION_INFO_V1(periodset_start_period);

PGDLLEXPORT Datum
periodset_start_period(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *result = periodset_per_n(ps, 0);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/* End period */

PG_FUNCTION_INFO_V1(periodset_end_period);

PGDLLEXPORT Datum
periodset_end_period(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *result = periodset_per_n(ps, ps->count - 1);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/* N-th period */

PG_FUNCTION_INFO_V1(periodset_period_n);

PGDLLEXPORT Datum
periodset_period_n(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	int i = PG_GETARG_INT32(1); /* Assume 1-based */
	Period *result = NULL;
	if (i >= 1 && i <= ps->count)
		result = periodset_per_n(ps, i - 1);
	PG_FREE_IF_COPY(ps, 0);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Array of periods of a PeriodSet */

Period **
periodset_periods_internal(PeriodSet *ps)
{
	Period **periods = palloc(sizeof(Period *) * ps->count);
	for (int i = 0; i < ps->count; i++) 
		periods[i] = periodset_per_n(ps, i);
	return periods;
}

PG_FUNCTION_INFO_V1(periodset_periods);

PGDLLEXPORT Datum
periodset_periods(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period **periods = periodset_periods_internal(ps);
	ArrayType *result = periodarr_to_array(periods, ps->count);
	pfree(periods);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_ARRAYTYPE_P(result);
}

/* Number of timestamps */

PG_FUNCTION_INFO_V1(periodset_num_timestamps);

PGDLLEXPORT Datum
periodset_num_timestamps(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *p = periodset_per_n(ps, 0);
	TimestampTz prev = p->lower;
	bool start = false;
	int result = 1;
	TimestampTz d;
	int i = 1;
	while (i < ps->count || !start)
	{
		if (start)
		{
			p = periodset_per_n(ps, i++);
			d = p->lower;
			start = !start;
		}
		else
		{
			d = p->upper;
			start = !start;
		}
		if (timestamp_cmp_internal(prev, d) != 0)
		{
			result++;
			prev = d;
		}
	}
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_INT32(result);
}

/* Start timestamptz */

TimestampTz
periodset_start_timestamp_internal(PeriodSet *ps)
{
	Period *p = periodset_per_n(ps, 0);
	return p->lower;
}

PG_FUNCTION_INFO_V1(periodset_start_timestamp);

PGDLLEXPORT Datum
periodset_start_timestamp(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *p = periodset_per_n(ps, 0);
	TimestampTz result = p->lower;
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/* End timestamptz */

TimestampTz
periodset_end_timestamp_internal(PeriodSet *ps)
{
	Period *p = periodset_per_n(ps, ps->count - 1);
	return p->upper;
}

PG_FUNCTION_INFO_V1(periodset_end_timestamp);

PGDLLEXPORT Datum
periodset_end_timestamp(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Period *p = periodset_per_n(ps, ps->count - 1);
	TimestampTz result = p->upper;
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_TIMESTAMPTZ(result);
}

/* N-th timestamptz */

PG_FUNCTION_INFO_V1(periodset_timestamp_n);

PGDLLEXPORT Datum
periodset_timestamp_n(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	int n = PG_GETARG_INT32(1); /* Assume 1-based */
	int pernum = 0;
	Period *p = periodset_per_n(ps, pernum);
	TimestampTz d = p->lower;
	if (n == 1)
	{
		PG_FREE_IF_COPY(ps, 0);
		PG_RETURN_TIMESTAMPTZ(d);
	}
	
	bool start = false;
	int i = 1;
	TimestampTz prev = d;
	while (i < n)
	{
		if (start)
		{
			pernum++;
			if (pernum == ps->count)
				break;
				
			p = periodset_per_n(ps, pernum);
			d = p->lower;
			start = !start;
		}
		else
		{
			d = p->upper;
			start = !start;
		}
		if (timestamp_cmp_internal(prev, d) != 0)
		{
			i++;
			prev = d;
		}
	}
	PG_FREE_IF_COPY(ps, 0);
	if (i != n) 
		PG_RETURN_NULL();
	PG_RETURN_TIMESTAMPTZ(d);
}

/* Timestamps */

PG_FUNCTION_INFO_V1(periodset_timestamps);

PGDLLEXPORT Datum
periodset_timestamps(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	TimestampTz *times = palloc(sizeof(TimestampTz) * 2 * ps->count);
	Period *p = periodset_per_n(ps, 0);
	times[0] = p->lower;
	int k = 1;
	if (timestamp_cmp_internal(p->lower, p->upper) != 0)
		times[k++] = p->upper;
	for (int i = 1; i < ps->count; i++)
	{
		p = periodset_per_n(ps, i);
		if (timestamp_cmp_internal(times[k-1], p->lower) != 0)
			times[k++] = p->lower;
		if (timestamp_cmp_internal(times[k-1], p->upper) != 0)
			times[k++] = p->upper;
	}
	ArrayType *result = timestamparr_to_array(times, k);
	pfree(times);
	PG_FREE_IF_COPY(ps, 0);

	PG_RETURN_ARRAYTYPE_P(result);
}

/* Shift the period set by an interval */

PeriodSet *
periodset_shift_internal(PeriodSet *ps, Interval *interval)
{
	Period **periods = palloc(sizeof(Period *) * ps->count);
	for (int i = 0; i < ps->count; i++)
	{
		Period *p = periodset_per_n(ps, i);
		periods[i] = period_shift_internal(p, interval);
	}
	PeriodSet *result = periodset_from_periodarr_internal(periods, ps->count, false);
	for (int i = 0; i < ps->count; i++)
		pfree(periods[i]);
	pfree(periods);
	return result;
}

PG_FUNCTION_INFO_V1(periodset_shift);

PGDLLEXPORT Datum
periodset_shift(PG_FUNCTION_ARGS)
{
	PeriodSet *ps = PG_GETARG_PERIODSET(0);
	Interval *interval = PG_GETARG_INTERVAL_P(1);
	PeriodSet *result = periodset_shift_internal(ps, interval);
	PG_FREE_IF_COPY(ps, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for defining B-tree index
 *****************************************************************************/

/* B-tree comparator */

int
periodset_cmp_internal(PeriodSet *ps1, PeriodSet *ps2)
{
	int count1 = ps1->count;
	int count2 = ps2->count;
	int count = count1 < count2 ? count1 : count2;
	int result = 0;
	for (int i = 0; i < count; i++)
	{
		Period *p1 = periodset_per_n(ps1, i);
		Period *p2 = periodset_per_n(ps2, i);
		result = period_cmp_internal(p1, p2);
		if (result) 
			break;
	}
	/* The first count periods of the two PeriodSet are equal */
	if (!result) 
	{
		if (count < count1) /* ps1 has more PeriodSet than ps2 */
			result = 1;
		else if (count < count2) /* ps2 has more PeriodSet than ps1 */
			result = -1;
		else
			result = 0;
	}
	return result;
}

PG_FUNCTION_INFO_V1(periodset_cmp);

PGDLLEXPORT Datum
periodset_cmp(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	int cmp = periodset_cmp_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_INT32(cmp);
}

/* 
 * Equality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
periodset_eq_internal(PeriodSet *ps1, PeriodSet *ps2)
{
	if (ps1->count != ps2->count)
		return false;
	/* ps1 and ps2 have the same number of PeriodSet */
	for (int i = 0; i < ps1->count; i++)
	{
		Period *p1 = periodset_per_n(ps1, i);
		Period *p2 = periodset_per_n(ps2, i);
		if (period_ne_internal(p1, p2))
			return false;
	}
	/* All periods of the two PeriodSet are equal */
	return true;
}

PG_FUNCTION_INFO_V1(periodset_eq);

PGDLLEXPORT Datum
periodset_eq(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	bool result = periodset_eq_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_BOOL(result);
}

/* 
 * Inequality operator
 * The internal B-tree comparator is not used to increase efficiency 
 */
bool
periodset_ne_internal(PeriodSet *ps1, PeriodSet *ps2)
{
	return !periodset_eq_internal(ps1, ps2);
}

PG_FUNCTION_INFO_V1(periodset_ne);

PGDLLEXPORT Datum
periodset_ne(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	bool result = periodset_ne_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_BOOL(result);
}

/* Comparison operators using the internal B-tree comparator */

PG_FUNCTION_INFO_V1(periodset_lt);

PGDLLEXPORT Datum
periodset_lt(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	int cmp = periodset_cmp_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_BOOL(cmp < 0);
}

PG_FUNCTION_INFO_V1(periodset_le);

PGDLLEXPORT Datum
periodset_le(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	int cmp = periodset_cmp_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_BOOL(cmp <= 0);
}

PG_FUNCTION_INFO_V1(periodset_ge);

PGDLLEXPORT Datum
periodset_ge(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	int cmp = periodset_cmp_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_BOOL(cmp >= 0);
}

PG_FUNCTION_INFO_V1(periodset_gt);

PGDLLEXPORT Datum
periodset_gt(PG_FUNCTION_ARGS)
{
	PeriodSet *ps1 = PG_GETARG_PERIODSET(0);
	PeriodSet *ps2 = PG_GETARG_PERIODSET(1);
	int cmp = periodset_cmp_internal(ps1, ps2);
	PG_FREE_IF_COPY(ps1, 0);
	PG_FREE_IF_COPY(ps2, 1);
	PG_RETURN_BOOL(cmp > 0);
}

/*****************************************************************************/
