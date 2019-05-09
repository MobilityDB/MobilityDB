/*****************************************************************************
 *
 * Range.c
 *	  Extension of operators for range types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
 
#include "TemporalTypes.h"

/*****************************************************************************
 * Generic range functions
 *****************************************************************************/

/* Convert the range into a string, used in debugging */ 

const char *
range_to_string(RangeType *range) 
{
	return call_output(range->rangetypid, RangeTypePGetDatum(range));
}

/* Get the lower bound of the range */ 

Datum
lower_datum(RangeType *range)
{
	return call_function1(range_lower, RangeTypePGetDatum(range));
}

/* Get the upper bound of the range */ 

Datum
upper_datum(RangeType *range)
{
	Datum upper = call_function1(range_upper, RangeTypePGetDatum(range));
	/* intranges are in canonical form so their upper bound is exclusive */
	if (range->rangetypid == type_oid(T_INTRANGE))
		upper = Int32GetDatum(DatumGetInt32(upper) - 1);
	return upper;
}

/* Is the lower bound of the range inclusive? */ 

bool
lower_inc(RangeType *range)
{
	return (range_get_flags(range) & RANGE_LB_INC) != 0;
}

/* Is the upper bound of the range inclusive? */ 

bool
upper_inc(RangeType *range)
{
	return (range_get_flags(range) & RANGE_UB_INC) != 0;
}

/* Construct a range from given arguments */

RangeType *
range_make(Datum from, Datum to, bool lower_inc, bool upper_inc, Oid subtypid)
{
	Oid range_oid;
	if (subtypid == INT4OID)
		range_oid = type_oid(T_INTRANGE);
	else if (subtypid == FLOAT8OID)
		range_oid = type_oid(T_FLOATRANGE);
	else if (subtypid == TIMESTAMPTZOID)
		range_oid = TSTZRANGEOID;
	else
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
			errmsg("Operation not supported")));
	
	TypeCacheEntry* typcache = lookup_type_cache(range_oid, TYPECACHE_RANGE_INFO);

	RangeBound lower;
	RangeBound upper;
	lower.val = from;
	lower.infinite = false;
	lower.inclusive = lower_inc;
	lower.lower = true;
	upper.val = to;
	upper.infinite = false;
	upper.inclusive = upper_inc;
	upper.lower = false;

	return make_range(typcache, &lower, &upper, false);
}

/*
 * Set union.  If strict is true, it is an error that the two input ranges
 * are not adjacent or overlapping.
 * Function copied verbatim from rangetypes.c since it is marked static.
 */
 
static RangeType *
range_union_internal(TypeCacheEntry *typcache, RangeType *r1, RangeType *r2,
					 bool strict)
{
	RangeBound	lower1,
				lower2;
	RangeBound	upper1,
				upper2;
	bool		empty1,
				empty2;
	RangeBound *result_lower;
	RangeBound *result_upper;

	/* Different types should be prevented by ANYRANGE matching rules */
	if (RangeTypeGetOid(r1) != RangeTypeGetOid(r2))
		elog(ERROR, "range types do not match");

	range_deserialize(typcache, r1, &lower1, &upper1, &empty1);
	range_deserialize(typcache, r2, &lower2, &upper2, &empty2);

	/* if either is empty, the other is the correct answer */
	if (empty1)
		return r2;
	if (empty2)
		return r1;
	if (strict &&
		!DatumGetBool(range_overlaps_internal(typcache, r1, r2)) &&
		!DatumGetBool(range_adjacent_internal(typcache, r1, r2)))
		ereport(ERROR, (errcode(ERRCODE_DATA_EXCEPTION),
			errmsg("result of range union would not be contiguous")));

	if (range_cmp_bounds(typcache, &lower1, &lower2) < 0)
		result_lower = &lower1;
	else
		result_lower = &lower2;
	if (range_cmp_bounds(typcache, &upper1, &upper2) > 0)
		result_upper = &upper1;
	else
		result_upper = &upper2;
	return make_range(typcache, result_lower, result_upper, false);
}

/*
 * Normalize an array of ranges
 * This function allows the ranges to be non contiguous 
 */
RangeType **
rangearr_normalize(RangeType **ranges, int *count)
{
	rangearr_sort(ranges, *count);
	int newcount = 0;
	RangeType **result = palloc(sizeof(RangeType *) * *count);
	RangeType *current = ranges[0];
	TypeCacheEntry* typcache = lookup_type_cache(ranges[0]->rangetypid, TYPECACHE_RANGE_INFO);
	bool copy = true;
	for (int i = 1; i < *count; i++)
	{
		RangeType *range = ranges[i];
		if (range_overlaps_internal(typcache, current, range) ||
			range_adjacent_internal(typcache, current, range)) 
		{
			RangeType *r = range_union_internal(typcache, current, range, true);
			if (!copy)
				pfree(current);
			current = r;
			copy = false;
		} 
		else 
		{
			if (copy) 
			{
				result[newcount++] = palloc(VARSIZE(current));
				memcpy(result[newcount-1], current, VARSIZE(current));
			} 
			else
				result[newcount++] = current;
			current = range;
			copy = true;
		}
	}
	if (copy)
	{
		result[newcount++] = palloc(VARSIZE(current));
		memcpy(result[newcount-1], current, VARSIZE(current));
	} else
		result[newcount++] = current;

	*count = newcount;
	return result;
}

/*****************************************************************************/

/* Canonical function for defining the intrange type */

PG_FUNCTION_INFO_V1(intrange_canonical);

Datum
intrange_canonical(PG_FUNCTION_ARGS)
{
	RangeType  *r = PG_GETARG_RANGE_P(0);
	TypeCacheEntry *typcache;
	RangeBound	lower;
	RangeBound	upper;
	bool		empty;
	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));
	range_deserialize(typcache, r, &lower, &upper, &empty);
	if (empty)
		PG_RETURN_RANGE_P(r);
	if (!lower.infinite && !lower.inclusive)
	{
		lower.val = DirectFunctionCall2(int4pl, lower.val, Int32GetDatum(1));
		lower.inclusive = true;
	}
	if (!upper.infinite && upper.inclusive)
	{
		upper.val = DirectFunctionCall2(int4pl, upper.val, Int32GetDatum(1));
		upper.inclusive = false;
	}
	PG_RETURN_RANGE_P(range_serialize(typcache, &lower, &upper, false));
}

/* Convert an integer range into a float range */

RangeType *
numrange_to_floatrange_internal(RangeType *range)
{
	if (range->rangetypid == type_oid(T_FLOATRANGE))
		return range_make(lower_datum(range), upper_datum(range), 
			lower_inc(range), lower_inc(range), FLOAT8OID);
	/* Function upper_datum subtract 1 to the result of upper(range) */
	Datum lower = Float8GetDatum((double)DatumGetInt32(lower_datum(range)));
	Datum upper = Float8GetDatum((double)(DatumGetInt32(upper_datum(range))));
	return range_make(lower, upper, true, true, FLOAT8OID);
}

PG_FUNCTION_INFO_V1(numrange_to_floatrange);

Datum
numrange_to_floatrange(PG_FUNCTION_ARGS)
{
	RangeType *range = PG_GETARG_RANGE_P(0);
	RangeType *result = numrange_to_floatrange_internal(range);
	PG_FREE_IF_COPY(range, 0);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/* strictly left of? */
bool
range_left_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, r, &lower, &upper, &empty);

	/* An empty range is neither left nor right any other range */
	if (empty)
		return false;

	if (!upper.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  upper.val, val));
		if (cmp < 0)
			return true;
		if (cmp == 0 && !upper.inclusive)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(range_left_elem);

PGDLLEXPORT Datum
range_left_elem(PG_FUNCTION_ARGS)
{
	RangeType  *r = PG_GETARG_RANGE_P(0);
	Datum		val = PG_GETARG_DATUM(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(range_left_elem_internal(typcache, r, val));
}

/* does not extend to right of? */
bool
range_overleft_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, r, &lower, &upper, &empty);

	/* An empty range is neither left nor right any element */
	if (empty)
		return false;

	if (!upper.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  upper.val, val));
		if (cmp <= 0)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(range_overleft_elem);

PGDLLEXPORT Datum
range_overleft_elem(PG_FUNCTION_ARGS)
{
	RangeType  *r = PG_GETARG_RANGE_P(0);
	Datum		val = PG_GETARG_DATUM(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(range_overleft_elem_internal(typcache, r, val));
}

/* strictly right of?  */
bool
range_right_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, r, &lower, &upper, &empty);

	/* An empty range is neither left nor right any other range */
	if (empty)
		return false;

	if (!lower.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  lower.val, val));
		if (cmp > 0)
			return true;
		if (cmp == 0 && !lower.inclusive)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(range_right_elem);

PGDLLEXPORT Datum
range_right_elem(PG_FUNCTION_ARGS)
{
	RangeType  *r = PG_GETARG_RANGE_P(0);
	Datum		val = PG_GETARG_DATUM(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(range_right_elem_internal(typcache, r, val));
}

/* does not extend to left of? */
bool
range_overright_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, r, &lower, &upper, &empty);

	/* An empty range is neither left nor right any element */
	if (empty)
		return false;

	if (!lower.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  lower.val, val));
		if (cmp >= 0)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(range_overright_elem);

Datum
range_overright_elem(PG_FUNCTION_ARGS)
{
	RangeType  *r = PG_GETARG_RANGE_P(0);
	Datum		val = PG_GETARG_DATUM(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(range_overright_elem_internal(typcache, r, val));
}

/* adjacent to (but not overlapping)? */
bool
range_adjacent_elem_internal(TypeCacheEntry *typcache, RangeType *r, Datum val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	RangeBound 	elembound;
	bool		isadj;

	range_deserialize(typcache, r, &lower, &upper, &empty);

	/* An empty range is not adjacent to any element */
	if (empty)
		return false;

	/*
	 * Given two ranges A..B and a value V, the ranges are adjacent if and only if
	 * B is adjacent to V, or V is adjacent to A.
	 */
	elembound.val = val;
	elembound.infinite = false;
	elembound.inclusive = true;
	elembound.lower = false;
	isadj = bounds_adjacent(typcache, upper, elembound);
	elembound.lower = true;
	return (isadj || bounds_adjacent(typcache, elembound, lower));
}

PG_FUNCTION_INFO_V1(range_adjacent_elem);

PGDLLEXPORT Datum
range_adjacent_elem(PG_FUNCTION_ARGS)
{
	RangeType  *r = PG_GETARG_RANGE_P(0);
	Datum		val = PG_GETARG_DATUM(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(range_adjacent_elem_internal(typcache, r, val));
}

/******************************************************************************/

/* strictly left of? */
bool
elem_left_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, val, &lower, &upper, &empty);

	/* An empty range is neither left nor right any other range */
	if (empty)
		return false;

	if (!lower.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  r, lower.val));
		if (cmp < 0)
			return true;
		if (cmp == 0 && !lower.inclusive)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(elem_left_range);

PGDLLEXPORT Datum
elem_left_range(PG_FUNCTION_ARGS)
{
	Datum		val = PG_GETARG_DATUM(0);
	RangeType  *r = PG_GETARG_RANGE_P(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(elem_left_range_internal(typcache, val, r));
}

/* does not extend to right of? */
bool
elem_overleft_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, val, &lower, &upper, &empty);

	/* An empty range is neither left nor right any element */
	if (empty)
		return false;

	if (!lower.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  r, lower.val));
		if (cmp <= 0)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(elem_overleft_range);

PGDLLEXPORT Datum
elem_overleft_range(PG_FUNCTION_ARGS)
{
	Datum		val = PG_GETARG_DATUM(0);
	RangeType  *r = PG_GETARG_RANGE_P(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(elem_overleft_range_internal(typcache, val, r));
}

/* strictly right of?  */
bool
elem_right_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, val, &lower, &upper, &empty);

	/* An empty range is neither left nor right any other range */
	if (empty)
		return false;

	if (!upper.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  r, upper.val));
		if (cmp > 0)
			return true;
		if (cmp == 0 && !upper.inclusive)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(elem_right_range);

PGDLLEXPORT Datum
elem_right_range(PG_FUNCTION_ARGS)
{
	Datum		val = PG_GETARG_DATUM(0);
	RangeType  *r = PG_GETARG_RANGE_P(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(elem_right_range_internal(typcache, val, r));
}

/* does not extend to left of? */
bool
elem_overright_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	int32		cmp;

	range_deserialize(typcache, val, &lower, &upper, &empty);

	/* An empty range is neither left nor right any element */
	if (empty)
		return false;

	if (!upper.infinite)
	{
		cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
											  typcache->rng_collation,
											  r, upper.val));
		if (cmp >= 0)
			return true;
	}

	return false;
}

PG_FUNCTION_INFO_V1(elem_overright_range);

Datum
elem_overright_range(PG_FUNCTION_ARGS)
{
	Datum		val = PG_GETARG_DATUM(0);
	RangeType  *r = PG_GETARG_RANGE_P(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(elem_overright_range_internal(typcache, val, r));
}

/* adjacent to (but not overlapping)? */
bool
elem_adjacent_range_internal(TypeCacheEntry *typcache, Datum r, RangeType *val)
{
	RangeBound	lower,
				upper;
	bool		empty;
	RangeBound 	elembound;
	bool		isadj;

	range_deserialize(typcache, val, &lower, &upper, &empty);

	/* An empty range is not adjacent to any element */
	if (empty)
		return false;

	/*
	 * Given two ranges A..B and a vrue V, the ranges are adjacent if and only if
	 * B is adjacent to V, or V is adjacent to A.
	 */
	elembound.val = r;
	elembound.infinite = false;
	elembound.inclusive = true;
	elembound.lower = false;
	isadj = bounds_adjacent(typcache, upper, elembound);
	elembound.lower = true;
	return (isadj || bounds_adjacent(typcache, elembound, lower));
}

PG_FUNCTION_INFO_V1(elem_adjacent_range);

PGDLLEXPORT Datum
elem_adjacent_range(PG_FUNCTION_ARGS)
{
	Datum		val = PG_GETARG_DATUM(0);
	RangeType  *r = PG_GETARG_RANGE_P(1);
	TypeCacheEntry *typcache;

	typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r));

	PG_RETURN_BOOL(elem_adjacent_range_internal(typcache, val, r));
}

/******************************************************************************/
