/*****************************************************************************
 *
 * period.c
 *	  Basic routines for timestamptz periods
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "period.h"

#include <assert.h>
#include <access/hash.h>
#include <libpq/pqformat.h>
#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "periodset.h"
#include "timeops.h"
#include "temporal.h"
#include "temporal_util.h"
#include "temporal_parser.h"
#include "rangetypes_ext.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/*
 * Convert a deserialized period value to text form
 *
 * Inputs are the lower_inc and upper_inc bytes, and the two bound values
 * already converted to text (but not yet quoted).  
 *
 * Result is a palloc'd string
 */
static char *
period_deparse(bool lower_inc, bool upper_inc, const char *lbound_str, 
	const char *ubound_str)
{
	StringInfoData buf;

	initStringInfo(&buf);
	appendStringInfoChar(&buf, lower_inc ? '[' : '(');
	appendStringInfoString(&buf, lbound_str);
	appendStringInfoString(&buf, ", ");
	appendStringInfoString(&buf, ubound_str);
	appendStringInfoChar(&buf, upper_inc ? ']' : ')');
	return buf.data;
}

/*
 * period_deserialize: deconstruct a period value
 */
void
period_deserialize(Period *p, PeriodBound *lower, PeriodBound *upper)
{
	if (lower) 
	{
		lower->val = p->lower;
		lower->inclusive = p->lower_inc;
		lower->lower = true;
	}

	if (upper) 
	{
		upper->val = p->upper;
		upper->inclusive = p->upper_inc;
		upper->lower = false;
	}
}

/*****************************************************************************/

/*
 * Compare two period boundary points, returning <0, 0, or >0 according to
 * whether b1 is less than, equal to, or greater than b2.
 *
 * The boundaries can be any combination of upper and lower; so it's useful
 * for a variety of operators.
 *
 * The simple case is when b1 and b2 are both inclusive, in which
 * case the result is just a comparison of the values held in b1 and b2.
 *
 * If a bound is exclusive, then we need to know whether it's a lower bound,
 * in which case we treat the boundary point as "just greater than" the held
 * value; or an upper bound, in which case we treat the boundary point as
 * "just less than" the held value.
 *
 * There is only one case where two boundaries compare equal but are not
 * identical: when both bounds are inclusive and hold the same value,
 * but one is an upper bound and the other a lower bound.
 */

int
period_cmp_bounds(TimestampTz t1, TimestampTz t2, bool lower1, bool lower2, 
	bool inclusive1, bool inclusive2)
{
	int32		result;

	/* Compare the values */
	result = timestamp_cmp_internal(t1, t2);

	/*
	 * If the comparison is not equal and the bounds are both inclusive or 
	 * both exclusive, we're done. If they compare equal, we still have to 
	 * consider whether the boundaries are inclusive or exclusive. 
	*/
	if (result == 0)
	{
		if (!inclusive1 && !inclusive2)
		{
			/* both are exclusive */
			if (lower1 == lower2)
				return 0;
			else
				return lower1 ? 1 : -1;
		}
		else if (!inclusive1)
			return lower1 ? 1 : -1;
		else if (!inclusive2)
			return lower2 ? -1 : 1;
	}	
	
	return result;
}

/*
 * Check if two bounds A and B are "adjacent", where A is an upper bound and B
 * is a lower bound. For the bounds to be adjacent, each timestamp must
 * satisfy strictly one of the bounds: there are no values which satisfy both
 * bounds (i.e. less than A and greater than B); and there are no values which
 * satisfy neither bound (i.e. greater than A and less than B).
 *
 * If A == B, the periods are adjacent only if the bounds have different
 * inclusive flags (i.e., exactly one of the periods includes the common
 * boundary point).
 */

bool
period_bounds_adjacent(TimestampTz t1, TimestampTz t2, bool inclusive1, bool inclusive2)
{
	int			cmp;

	cmp = timestamp_cmp_internal(t1, t2);
	if (cmp == 0)
		return inclusive1 != inclusive2;
	else 
		return false;
}

/*
 * Compare periods by lower bound.
 */
int
period_cmp_lower(const void** a, const void** b)
{
	Period *i1 = (Period *) *a;
	Period *i2 = (Period *) *b;
	return period_cmp_bounds(i1->lower, i2->lower, true, true, 
		i1->lower_inc, i2->lower_inc);
}

/*
 * Compare periods by upper bound.
 */
int
period_cmp_upper(const void** a, const void** b)
{
	Period *i1 = (Period *) *a;
	Period *i2 = (Period *) *b;
	return period_cmp_bounds(i1->upper, i2->upper, false, false, 
		i1->upper_inc, i2->upper_inc);
}

/*
 * period_make: construct a period value from bounds
 */
 
void
period_set(Period *p, TimestampTz lower, TimestampTz upper, 
	bool lower_inc, bool upper_inc)
{
	int	cmp = timestamp_cmp_internal(lower, upper);	

	/* error check: if lower bound value is above upper, it's wrong */
	if (cmp > 0)
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
			errmsg("Period lower bound must be less than or equal to period upper bound")));

	/* error check: if bounds are equal, and not both inclusive, period is empty */
	if (cmp == 0 && !(lower_inc && upper_inc))
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
			errmsg("Period cannot be empty")));

	/* Now fill in the period */
	p->lower = lower;
	p->upper = upper;
	p->lower_inc = lower_inc;
	p->upper_inc = upper_inc;
}

Period *
period_make(TimestampTz lower, TimestampTz upper, bool lower_inc, bool upper_inc)
{
	/* Note: zero-fill is required here, just as in heap tuples */
	Period 	   *period = (Period *) palloc0(sizeof(Period));
	period_set(period, lower, upper, lower_inc, upper_inc);
	return period;
}

/* Copy a period */

Period *
period_copy(Period *p)
{
	Period *result = (Period *) palloc(sizeof(Period));
	memcpy((char *) result, (char *) p, sizeof(Period));
	return result;
}

/* Number of seconds in a period */

float8
period_duration_secs(TimestampTz v1, TimestampTz v2)
{
	float8		result;

	result = ((float8) v1 - (float8) v2) / USECS_PER_SEC;
	return result;
}

/* Duration of the period as an interval */

Interval *
period_duration_internal(Period *p)
{
	return DatumGetIntervalP(call_function2(timestamp_mi, p->upper, 
		p->lower));
}

/*
 * Normalize an array of periods
 * The input periods may overlap and may be non contiguous.
 * The normalized periods are new periods that must be freed.
 */
Period **
periodarr_normalize(Period **periods, int count, int *newcount)
{
	periodarr_sort(periods, count);
	int count1 = 0;
	Period **result = palloc(sizeof(Period *) * count);
	Period *current = periods[0];
	bool isnew = false;
	for (int i = 1; i < count; i++)
	{
		Period *next = periods[i];
		if (overlaps_period_period_internal(current, next) ||
			adjacent_period_period_internal(current, next)) 
		{
			PeriodSet *ps = union_period_period_internal(current, next);
			Period *newper = period_copy(periodset_per_n(ps, 0));
			pfree(ps);
			if (isnew)
				pfree(current);
			current = newper;
			isnew = true;
		} 
		else 
		{
			if (!isnew) 
			{
				result[count1++] = palloc(sizeof(Period));
				memcpy(result[count1-1], current, sizeof(Period));
			} 
			else
				result[count1++] = current;
			current = next;
			isnew = false;
		}
	}
	if (!isnew)
	{
		result[count1++] = palloc(sizeof(Period));
		memcpy(result[count1-1], current, sizeof(Period));
	}
	else
		result[count1++] = current;

	*newcount = count1;
	return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/* Input function */
 
PG_FUNCTION_INFO_V1(period_in);

PGDLLEXPORT Datum
period_in(PG_FUNCTION_ARGS) 
{
	char *input = PG_GETARG_CSTRING(0);
	Period *result = period_parse(&input);
	PG_RETURN_POINTER(result);
}

/* Convert to string */

static void unquote(char *str) 
{
	char *last = str;
	while (*str != '\0') 
	{
		if (*str != '"') 
		{
			*last++ = *str;
		}
		str++;
	}
	*last = '\0';
}

char*
period_to_string(Period *p) 
{
	char *lower = call_output(TIMESTAMPTZOID, p->lower);
	char *upper = call_output(TIMESTAMPTZOID, p->upper);
	char *result = period_deparse(p->lower_inc, p->upper_inc, lower, upper);
	unquote(result);
	pfree(lower); pfree(upper);
	return result;
}

/* Output function */
 
PG_FUNCTION_INFO_V1(period_out);

PGDLLEXPORT Datum
period_out(PG_FUNCTION_ARGS) 
{
	Period *p = PG_GETARG_PERIOD(0);
	PG_RETURN_CSTRING(period_to_string(p));
}

/* Send function */

void
period_send_internal(Period *p, StringInfo buf)
{
	bytea *lower = call_send(TIMESTAMPTZOID, p->lower);
	bytea *upper = call_send(TIMESTAMPTZOID, p->upper);
	pq_sendbytes(buf, VARDATA(lower), VARSIZE(lower) - VARHDRSZ);
	pq_sendbytes(buf, VARDATA(upper), VARSIZE(upper) - VARHDRSZ);
	pq_sendbyte(buf, p->lower_inc);
	pq_sendbyte(buf, p->upper_inc);
	pfree(lower);
	pfree(upper);
}
 
PG_FUNCTION_INFO_V1(period_send);

PGDLLEXPORT Datum
period_send(PG_FUNCTION_ARGS) 
{
	Period *p = PG_GETARG_PERIOD(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	period_send_internal(p, &buf);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Receive function */

Period *
period_recv_internal(StringInfo buf) 
{
	Period *result = (Period *) palloc0(sizeof(Period));
	result->lower = call_recv(TIMESTAMPTZOID, buf);
	result->upper = call_recv(TIMESTAMPTZOID, buf);
	result->lower_inc = (char) pq_getmsgbyte(buf);
	result->upper_inc = (char) pq_getmsgbyte(buf);
	return result;
}

PG_FUNCTION_INFO_V1(period_recv);

PGDLLEXPORT Datum
period_recv(PG_FUNCTION_ARGS) 
{
	StringInfo buf = (StringInfo) PG_GETARG_POINTER(0);
	PG_RETURN_POINTER(period_recv_internal(buf));
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

/* Construct standard-form period value from two arguments */

PG_FUNCTION_INFO_V1(period_constructor2);

PGDLLEXPORT Datum
period_constructor2(PG_FUNCTION_ARGS)
{
	TimestampTz lower = PG_GETARG_TIMESTAMPTZ(0);
	TimestampTz upper = PG_GETARG_TIMESTAMPTZ(1);
	Period	   *period;

	period = period_make(lower, upper, true, false);

	PG_RETURN_PERIOD(period);
}

/* Construct general period value from four arguments */

PG_FUNCTION_INFO_V1(period_constructor4);

PGDLLEXPORT Datum
period_constructor4(PG_FUNCTION_ARGS)
{
	TimestampTz lower = PG_GETARG_TIMESTAMPTZ(0);
	TimestampTz upper = PG_GETARG_TIMESTAMPTZ(1);
	bool lower_inc = PG_GETARG_BOOL(2);
	bool upper_inc = PG_GETARG_BOOL(3);
	Period	   *period;

	period = period_make(lower, upper, lower_inc, upper_inc);

	PG_RETURN_PERIOD(period);
}

/*****************************************************************************
 * Casting
 *****************************************************************************/

/* Cast a TimestampTz value as a TimestampSet value */

PG_FUNCTION_INFO_V1(timestamp_as_period);

PGDLLEXPORT Datum
timestamp_as_period(PG_FUNCTION_ARGS)
{
	TimestampTz t = PG_GETARG_TIMESTAMPTZ(0);
	Period *result = period_make(t, t, true, true);
	PG_RETURN_POINTER(result);
}

/* Conversion functions period <-> range */

PG_FUNCTION_INFO_V1(period_as_tstzrange);

PGDLLEXPORT Datum
period_as_tstzrange(PG_FUNCTION_ARGS)
{
	Period	   *period = PG_GETARG_PERIOD(0);
	RangeType  *range;
	range = range_make(TimestampTzGetDatum(period->lower), 
		TimestampTzGetDatum(period->upper), period->lower_inc, 
		period->upper_inc, TIMESTAMPTZOID);
	PG_RETURN_POINTER(range);
}

PG_FUNCTION_INFO_V1(tstzrange_as_period);

PGDLLEXPORT Datum
tstzrange_as_period(PG_FUNCTION_ARGS)
{
	RangeType  *range = PG_GETARG_RANGE_P(0);
	TypeCacheEntry *typcache;
	char		flags = range_get_flags(range);
	RangeBound	lower;
	RangeBound	upper;
	bool		empty;
	Period	   *period;
	
	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
	assert(typcache->rngelemtype->type_id == TIMESTAMPTZOID);
	if (flags & RANGE_EMPTY)
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
			errmsg("Range cannot be empty")));
	if ((flags & RANGE_LB_INF) || (flags & RANGE_UB_INF))
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
			errmsg("Range bounds cannot be infinite")));

	range_deserialize(typcache, range, &lower, &upper, &empty);
	period = period_make(DatumGetTimestampTz(lower.val),
		DatumGetTimestampTz(upper.val), lower.inclusive, upper.inclusive);
	PG_RETURN_POINTER(period);
}

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/* period -> timestamptz functions */

/* extract lower bound value */

PG_FUNCTION_INFO_V1(period_lower);

PGDLLEXPORT Datum
period_lower(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	PG_RETURN_DATUM(p->lower);
}

/* extract upper bound value */

PG_FUNCTION_INFO_V1(period_upper);

PGDLLEXPORT Datum
period_upper(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	PG_RETURN_DATUM(p->upper);
}

/* period -> bool functions */

/* is lower bound inclusive? */

PG_FUNCTION_INFO_V1(period_lower_inc);

PGDLLEXPORT Datum
period_lower_inc(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	PG_RETURN_BOOL(p->lower_inc != 0);
}

/* is upper bound inclusive? */

PG_FUNCTION_INFO_V1(period_upper_inc);

PGDLLEXPORT Datum
period_upper_inc(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	PG_RETURN_BOOL(p->upper_inc != 0);
}

/* Shift the period by an interval */

Period *
period_shift_internal(Period *p, Interval *interval)
{
	TimestampTz t1 = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(p->lower),
		PointerGetDatum(interval)));
	TimestampTz t2 = DatumGetTimestampTz(
		DirectFunctionCall2(timestamptz_pl_interval,
		TimestampTzGetDatum(p->upper),
		PointerGetDatum(interval)));
	Period *result = period_make(t1, t2,
		p->lower_inc, p->upper_inc);
	return result;
}

PG_FUNCTION_INFO_V1(period_shift);

PGDLLEXPORT Datum
period_shift(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Interval *interval = PG_GETARG_INTERVAL_P(1);
	Period *result = period_shift_internal(p, interval);
	PG_RETURN_POINTER(result);
}


/* Duration of the period as an interval */

PG_FUNCTION_INFO_V1(period_duration);

PGDLLEXPORT Datum
period_duration(PG_FUNCTION_ARGS)
{
	Period *p = PG_GETARG_PERIOD(0);
	Datum result = call_function2(timestamp_mi, p->upper, p->lower);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Btree support
 *****************************************************************************/

/* equality  */
bool
period_eq_internal(Period *p1, Period *p2)
{
	if (p1->lower_inc != p2->lower_inc || p1->upper_inc != p2->upper_inc)
		return false;
	
	return timestamp_cmp_internal(p1->lower, p2->lower) == 0 && 
		timestamp_cmp_internal(p1->upper, p2->upper) == 0;
}

PG_FUNCTION_INFO_V1(period_eq);

PGDLLEXPORT Datum
period_eq(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_BOOL(period_eq_internal(p1, p2));
}

/* inequality */
bool
period_ne_internal(Period *p1, Period *p2)
{
	return (!period_eq_internal(p1, p2));
}

PG_FUNCTION_INFO_V1(period_ne);

PGDLLEXPORT Datum
period_ne(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_BOOL(period_ne_internal(p1, p2));
}

/* btree comparator */
int
period_cmp_internal(Period *p1, Period *p2)
{
	int cmp = period_cmp_bounds(p1->lower, p2->lower, true, true, 
		p1->lower_inc, p2->lower_inc);
	if (cmp == 0)
		cmp = period_cmp_bounds(p1->upper, p2->upper, false, false, 
			p1->upper_inc, p2->upper_inc);

	return cmp;
}

PG_FUNCTION_INFO_V1(period_cmp);

PGDLLEXPORT Datum
period_cmp(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_INT32(period_cmp_internal(p1, p2));	
}

/* inequality operators using the period_cmp function */
bool
period_lt_internal(Period *p1, Period *p2)
{
	int	cmp = period_cmp_internal(p1, p2);
	return (cmp < 0);
}

PG_FUNCTION_INFO_V1(period_lt);

PGDLLEXPORT Datum
period_lt(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_BOOL(period_lt_internal(p1, p2));
}

bool
period_le_internal(Period *p1, Period *p2)
{
	int cmp = period_cmp_internal(p1, p2);
	return (cmp <= 0);
}

PG_FUNCTION_INFO_V1(period_le);

PGDLLEXPORT Datum
period_le(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_BOOL(period_le_internal(p1, p2));
}

bool
period_ge_internal(Period *p1, Period *p2)
{
	int cmp = period_cmp_internal(p1, p2);
	return (cmp >= 0);
}

PG_FUNCTION_INFO_V1(period_ge);

PGDLLEXPORT Datum
period_ge(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_BOOL(period_ge_internal(p1, p2));
}

bool
period_gt_internal(Period *p1, Period *p2)
{
	int cmp = period_cmp_internal(p1, p2);
	return (cmp > 0);
}

PG_FUNCTION_INFO_V1(period_gt);

PGDLLEXPORT Datum
period_gt(PG_FUNCTION_ARGS)
{
	Period *p1 = PG_GETARG_PERIOD(0);
	Period *p2 = PG_GETARG_PERIOD(1);
	PG_RETURN_BOOL(period_gt_internal(p1, p2));
}

/*****************************************************************************
 * Hash support
 *****************************************************************************/

PG_FUNCTION_INFO_V1(period_hash);

PGDLLEXPORT Datum
period_hash(PG_FUNCTION_ARGS)
{
	Period	   *p = PG_GETARG_PERIOD(0);
	uint32		result;
	char		flags = '\0';
	uint32		lower_hash;
	uint32		upper_hash;

	/* Create flags from the lower_inc and upper_inc values */
	if (p->lower_inc)
		flags |= 0x01;
	if (p->upper_inc)
		flags |= 0x02;

	/* Apply the hash function to each bound */
	lower_hash = DatumGetUInt32(call_function1(hashint8, p->lower));
	upper_hash = DatumGetUInt32(call_function1(hashint8, p->upper));

	/* Merge hashes of flags and bounds */
	result = hash_uint32((uint32) flags);
	result ^= lower_hash;
	result = (result << 1) | (result >> 31);
	result ^= upper_hash;

	PG_RETURN_UINT32(result);
}

/*
 * Returns 64-bit value by hashing a value to a 64-bit value, with a seed.
 * Otherwise, similar to period_hash.
 */

PG_FUNCTION_INFO_V1(period_hash_extended);

PGDLLEXPORT Datum
period_hash_extended(PG_FUNCTION_ARGS)
{
	Period	   *p = PG_GETARG_PERIOD(0);
	Datum		seed = PG_GETARG_DATUM(1);
	uint64		result;
	char		flags = '\0';
	uint64		lower_hash;
	uint64		upper_hash;

	/* Create flags from the lower_inc and upper_inc values */
	if (p->lower_inc)
		flags |= 0x01;
	if (p->upper_inc)
		flags |= 0x02;

	/* Apply the hash function to each bound */
	lower_hash = DatumGetUInt64(call_function2(hashint8extended, 
		p->lower, seed));
	upper_hash = DatumGetUInt64(call_function2(hashint8extended, 
		p->upper, seed));

	/* Merge hashes of flags and bounds */
	result = DatumGetUInt64(hash_uint32_extended((uint32) flags,
		DatumGetInt64(seed)));
	result ^= lower_hash;
	result = ROTATE_HIGH_AND_LOW_32BITS(result);
	result ^= upper_hash;

	PG_RETURN_UINT64(result);
}
 
/*****************************************************************************/
