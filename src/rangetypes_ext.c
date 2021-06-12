/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/**
 * @file rangetypes_ext.c
 * Extended operators for range types.
 *
 * These operators have been submitted as a PR to PostgreSQL.
 */

#include "rangetypes_ext.h"

#include <assert.h>
#include <float.h>
#include <utils/builtins.h>

#include "temporal.h"
#include "tempcache.h"
#include "temporal_util.h"
#include "tnumber_mathfuncs.h"

/*****************************************************************************
 * Generic range functions
 *****************************************************************************/

/**
 * Returns the string representation of the range value, used for debugging
 */
const char *
range_to_string(const RangeType *range)
{
  return call_output(range->rangetypid, PointerGetDatum(range));
}

/**
 * Returns the lower bound of the range value
 */
Datum
lower_datum(const RangeType *range)
{
  return call_function1(range_lower, PointerGetDatum(range));
}

/**
 * Returns the upper bound of the range value
 */
Datum
upper_datum(const RangeType *range)
{
  Datum upper = call_function1(range_upper, PointerGetDatum(range));
  /* intranges are in canonical form so their upper bound is exclusive */
  if (range->rangetypid == type_oid(T_INTRANGE))
    upper = Int32GetDatum(DatumGetInt32(upper) - 1);
  return upper;
}

/**
 * Returns true if the lower bound of the range value is inclusive
 */
bool
#if MOBDB_PGSQL_VERSION < 130000
lower_inc(RangeType *range)
#else
lower_inc(const RangeType *range)
#endif
{
  return (range_get_flags(range) & RANGE_LB_INC) != 0;
}

/**
 * Returns true if the upper bound of the range value is inclusive
 */
bool
#if MOBDB_PGSQL_VERSION < 130000
upper_inc(RangeType *range)
#else
upper_inc(const RangeType *range)
#endif
{
  return (range_get_flags(range) & RANGE_UB_INC) != 0;
}

/**
 * Get the bounds of the range as double values.
 *
 * @param[in] range Input ranges
 * @param[out] xmin, xmax Lower and upper bounds
 */
void
range_bounds(const RangeType *range, double *xmin, double *xmax)
{
  ensure_tnumber_range_type(range->rangetypid);
  if (range->rangetypid == type_oid(T_INTRANGE))
  {
    *xmin = (double)(DatumGetInt32(lower_datum(range)));
    *xmax = (double)(DatumGetInt32(upper_datum(range)));
  }
  else /* range->rangetypid == type_oid(T_FLOATRANGE) */
  {
    *xmin = DatumGetFloat8(lower_datum(range));
    *xmax = DatumGetFloat8(upper_datum(range));
  }
}

/**
 * Construct a range value from given arguments
 */
RangeType *
range_make(Datum from, Datum to, bool lower_inc, bool upper_inc, Oid basetypid)
{
  Oid rangetypid = 0;
  assert (basetypid == INT4OID || basetypid == FLOAT8OID ||
    basetypid == TIMESTAMPTZOID);
  if (basetypid == INT4OID)
    rangetypid = type_oid(T_INTRANGE);
  else if (basetypid == FLOAT8OID)
    rangetypid = type_oid(T_FLOATRANGE);
  else
    rangetypid = type_oid(T_TSTZRANGE);

  TypeCacheEntry* typcache = lookup_type_cache(rangetypid, TYPECACHE_RANGE_INFO);

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

/**
 * Construct a range value from given arguments
 */
static RangeType *
range_copy(const RangeType *range)
{
  RangeType *result = palloc(VARSIZE(range));
  memcpy(result, range, VARSIZE(range));
  return result;
}

#if MOBDB_PGSQL_VERSION < 140000
/**
 * Returns the union of the range values. If strict is true, it is an error
 * that the two input ranges are not adjacent or overlapping.
 *
 * @note Function copied verbatim from rangetypes.c since it is static.
 */
static RangeType *
range_union_internal(TypeCacheEntry *typcache, RangeType *r1, RangeType *r2,
  bool strict)
{
  RangeBound lower1, lower2;
  RangeBound upper1, upper2;
  bool empty1, empty2;
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
#endif

/**
 * Normalize an array of ranges, which may be non contiguous.
 *
 * @param[in] ranges Array of ranges
 * @param[in] count Number of elements in the input array
 * @param[out] newcount Number of elements in the output array
 * @pre The number of elements is greater than 0
 */
RangeType **
rangearr_normalize(RangeType **ranges, int count, int *newcount)
{
  assert(count > 0);
  if (count > 1)
    rangearr_sort(ranges, count);
  int k = 0;
  RangeType **result = palloc(sizeof(RangeType *) * count);
  RangeType *current = ranges[0];
  TypeCacheEntry *typcache = lookup_type_cache(ranges[0]->rangetypid, TYPECACHE_RANGE_INFO);
  bool isnew = false;
  for (int i = 1; i < count; i++)
  {
    RangeType *range = ranges[i];
    if (range_overlaps_internal(typcache, current, range) ||
      range_adjacent_internal(typcache, current, range))
    {
      RangeType *range1 = range_union_internal(typcache, current, range, true);
      if (isnew)
        pfree(current);
      current = range1;
      isnew = true;
    }
    else
    {
      result[k++] = (isnew) ? current : range_copy(current);
      current = range;
      isnew = false;
    }
  }
  result[k++] = (isnew) ? current : range_copy(current);

  *newcount = k;
  return result;
}

/*****************************************************************************
 * Generic functions for testing a Boolean function between the range and
 * the element
 *****************************************************************************/

/**
 * Returns true if the range value and the element satisfy the function
 */
Datum
range_func_elem1(FunctionCallInfo fcinfo, RangeType *range, Datum val,
  bool (*func)(TypeCacheEntry *, RangeBound , RangeBound , Datum))
{
  TypeCacheEntry *typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  RangeBound lower_bound, upper_bound;
  bool empty;
  range_deserialize(typcache, range, &lower_bound, &upper_bound, &empty);
  /* An empty range is neither left nor right any other range */
  if (empty)
    return false;
  return func(typcache, lower_bound, upper_bound, val);
}

/**
 * Returns true if the range value and the element satisfy the function
 */
Datum
range_func_elem(FunctionCallInfo fcinfo,
  bool (*func)(TypeCacheEntry *, RangeBound , RangeBound , Datum))
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(0);
#else
  RangeType *range = PG_GETARG_RANGE_P(0);
#endif
  Datum val = PG_GETARG_DATUM(1);
  PG_RETURN_BOOL(range_func_elem1(fcinfo, range, val, func));
}

/**
 * Returns true if the element and the range value satisfy the function
 */
PGDLLEXPORT Datum
elem_func_range(FunctionCallInfo fcinfo,
  bool (*func)(TypeCacheEntry *, RangeBound , RangeBound , Datum))
{
  Datum val = PG_GETARG_DATUM(0);
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(1);
#else
  RangeType *range = PG_GETARG_RANGE_P(1);
#endif
  PG_RETURN_BOOL(range_func_elem1(fcinfo, range, val, func));
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(intrange_canonical);
/**
 * Canonical function for defining the intrange type
 */
PGDLLEXPORT Datum
intrange_canonical(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(0);
#else
  RangeType *range = PG_GETARG_RANGE_P(0);
#endif
  TypeCacheEntry *typcache;
  RangeBound lower_bound;
  RangeBound upper_bound;
  bool empty;
  typcache = range_get_typcache(fcinfo, RangeTypeGetOid(range));
  range_deserialize(typcache, range, &lower_bound, &upper_bound, &empty);
  if (empty)
#if MOBDB_PGSQL_VERSION < 110000
    PG_RETURN_RANGE(range);
#else
    PG_RETURN_RANGE_P(range);
#endif
  if (!lower_bound.infinite && !lower_bound.inclusive)
  {
    lower_bound.val = DirectFunctionCall2(int4pl, lower_bound.val, Int32GetDatum(1));
    lower_bound.inclusive = true;
  }
  if (!upper_bound.infinite && upper_bound.inclusive)
  {
    upper_bound.val = DirectFunctionCall2(int4pl, upper_bound.val, Int32GetDatum(1));
    upper_bound.inclusive = false;
  }
#if MOBDB_PGSQL_VERSION < 110000
  PG_RETURN_RANGE(range_serialize(typcache, &lower_bound, &upper_bound, false));
#else
  PG_RETURN_RANGE_P(range_serialize(typcache, &lower_bound, &upper_bound, false));
#endif
}

/*****************************************************************************/

/**
 * Returns true if the range value is strictly to the left of the value
 * (internal function)
 */
bool
range_left_elem_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  if (!upper_bound.infinite)
  {
    int32 cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
      typcache->rng_collation, upper_bound.val, val));
    if (cmp < 0 ||
      (cmp == 0 && !upper_bound.inclusive))
      return true;
  }
  return false;
}

/**
 * Returns true if the range value does not extend to the right of the value
 * (internal function)
 */
bool
range_overleft_elem_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  if (!upper_bound.infinite)
  {
    if (DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
      typcache->rng_collation, upper_bound.val, val)) <= 0)
      return true;
  }
  return false;
}

/**
 * Returns true if the range value is strictly to the right of the value
 * (internal function)
 */
bool
range_right_elem_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  if (!lower_bound.infinite)
  {
    int32 cmp = DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
      typcache->rng_collation, lower_bound.val, val));
    if (cmp > 0 ||
      (cmp == 0 && !lower_bound.inclusive))
      return true;
  }
  return false;
}

/**
 * Returns true if the range value does not extend to the left of the value
 * (internal function)
 */
bool
range_overright_elem_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  if (!lower_bound.infinite)
  {
    if (DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
      typcache->rng_collation, lower_bound.val, val)) >= 0)
      return true;
  }
  return false;
}

/**
 * Returns true if the range value and the value are adjacent
 * (internal function)
 */
bool
range_adjacent_elem_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  RangeBound elembound;
  bool isadj;
  /*
   * A range A..B and a value V are adjacent if and only if
   * B is adjacent to V, or V is adjacent to A.
   */
  elembound.val = val;
  elembound.infinite = false;
  elembound.inclusive = true;
  elembound.lower = true;
  isadj = bounds_adjacent(typcache, upper_bound, elembound);
  elembound.lower = false;
  return (isadj || bounds_adjacent(typcache, elembound, lower_bound));
}

/******************************************************************************/

PG_FUNCTION_INFO_V1(range_left_elem);
/**
 * Returns true if the range value is strictly to the left of the value
 */
PGDLLEXPORT Datum
range_left_elem(PG_FUNCTION_ARGS)
{
  return range_func_elem(fcinfo, &range_left_elem_internal);
}

PG_FUNCTION_INFO_V1(range_overleft_elem);
/**
 * Returns true if the range value does not extend to the right of the value
 */
PGDLLEXPORT Datum
range_overleft_elem(PG_FUNCTION_ARGS)
{
  return range_func_elem(fcinfo, &range_overleft_elem_internal);
}

PG_FUNCTION_INFO_V1(range_right_elem);
/**
 * Returns true if the range value is strictly to the right of the value
 */
PGDLLEXPORT Datum
range_right_elem(PG_FUNCTION_ARGS)
{
  return range_func_elem(fcinfo, &range_right_elem_internal);
}

PG_FUNCTION_INFO_V1(range_overright_elem);
/**
 * Returns true if the range value does not extend to the left of the value
 */
PGDLLEXPORT Datum
range_overright_elem(PG_FUNCTION_ARGS)
{
  return range_func_elem(fcinfo, &range_overright_elem_internal);
}

PG_FUNCTION_INFO_V1(range_adjacent_elem);
/**
 * Returns true if the range value and the value are adjacent
 */
PGDLLEXPORT Datum
range_adjacent_elem(PG_FUNCTION_ARGS)
{
  return range_func_elem(fcinfo, &range_adjacent_elem_internal);
}

/******************************************************************************/

/**
 * Returns true if the value does not extend to the right of the range value
 * (internal function)
 */
bool
elem_overleft_range_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  if (!upper_bound.infinite)
  {
    if (DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
      typcache->rng_collation, val, upper_bound.val)) <= 0)
      return true;
  }
  return false;
}

/**
 * Returns true if the value does not extend to the left of the range value
 * (internal function)
 */
bool
elem_overright_range_internal(TypeCacheEntry *typcache, RangeBound lower_bound,
  RangeBound upper_bound, Datum val)
{
  if (!lower_bound.infinite)
  {
    if (DatumGetInt32(FunctionCall2Coll(&typcache->rng_cmp_proc_finfo,
      typcache->rng_collation, val, lower_bound.val)) >= 0)
      return true;
  }
  return false;
}

/******************************************************************************/

PG_FUNCTION_INFO_V1(elem_left_range);
/**
 * Returns true if the value is strictly to the left of the range value
 */
PGDLLEXPORT Datum
elem_left_range(PG_FUNCTION_ARGS)
{
  return elem_func_range(fcinfo, &range_right_elem_internal);
}

PG_FUNCTION_INFO_V1(elem_overleft_range);
/**
 * Returns true if the value does not extend to the right of the range value
 */
PGDLLEXPORT Datum
elem_overleft_range(PG_FUNCTION_ARGS)
{
  return elem_func_range(fcinfo, &elem_overleft_range_internal);
}

PG_FUNCTION_INFO_V1(elem_right_range);
/**
 * Returns true if the value is strictly to the right of the range value
 */
PGDLLEXPORT Datum
elem_right_range(PG_FUNCTION_ARGS)
{
  return elem_func_range(fcinfo, &range_left_elem_internal);
}

PG_FUNCTION_INFO_V1(elem_overright_range);
/**
 * Returns true if the value does not extend to the left of the range value
 */
PGDLLEXPORT Datum
elem_overright_range(PG_FUNCTION_ARGS)
{
  return elem_func_range(fcinfo, &elem_overright_range_internal);
}

PG_FUNCTION_INFO_V1(elem_adjacent_range);
/**
 * Returns true if the value and the range value are adjacent
 */
PGDLLEXPORT Datum
elem_adjacent_range(PG_FUNCTION_ARGS)
{
  return elem_func_range(fcinfo, &range_adjacent_elem_internal);
}

/******************************************************************************/

PG_FUNCTION_INFO_V1(floatrange_set_precision);
/**
 * Set the precision of the float range to the number of decimal places
 */
PGDLLEXPORT Datum
floatrange_set_precision(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *range = PG_GETARG_RANGE(0);
#else
  RangeType *range = PG_GETARG_RANGE_P(0);
#endif
  Datum size = PG_GETARG_DATUM(1);
  RangeBound lower, upper;
  lower.lower = true;
  lower.inclusive = lower_inc(range);
  upper.lower = false;
  upper.inclusive = upper_inc(range);
  /* Set precision of bounds if they exist and not infinite */
  if (! (range_get_flags(range) & RANGE_LB_INF))
  {
    lower.val = lower_datum(range);
    if (DatumGetFloat8(lower.val) != -1 * get_float8_infinity())
      lower.val = datum_round(lower.val, size);
    lower.infinite = false;
  }
  else
    lower.infinite = true;
  if (! (range_get_flags(range) & RANGE_UB_INF))
  {
    upper.val = upper_datum(range);
    if (DatumGetFloat8(upper.val) != get_float8_infinity())
      upper.val = datum_round(upper.val, size);
    upper.infinite = false;
  }
  else
    upper.infinite = true;
  /* Create resulting range */
  TypeCacheEntry* typcache = lookup_type_cache(type_oid(T_FLOATRANGE),
    TYPECACHE_RANGE_INFO);
  RangeType *result = make_range(typcache, &lower, &upper, false);
  PG_RETURN_POINTER(result);
}

/******************************************************************************/

PG_FUNCTION_INFO_V1(range_extent_transfn);
/**
 * Transition function for temporal extent aggregation of period values
 * with period bounding box
 */
PGDLLEXPORT Datum
range_extent_transfn(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *r1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_RANGE(0);
  RangeType *r2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_RANGE(1);
#else
  RangeType *r1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_RANGE_P(0);
  RangeType *r2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_RANGE_P(1);
#endif
  RangeType *result;

  /* Can't do anything with null inputs */
  if (!r1 && !r2)
    PG_RETURN_NULL();
  /* Null period and non-null period, return the period */
  else if (!r1)
    result = range_copy(r2);
  /* Non-null period and null period, return the period */
  else if (!r2)
    result = range_copy(r1);
  else
  {
    TypeCacheEntry* typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r1));
    /* Non-strict union */
    result = range_union_internal(typcache, r1, r2, false);
  }
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(range_extent_combinefn);
/**
 * Combine function for temporal extent aggregation
 */
PGDLLEXPORT Datum
range_extent_combinefn(PG_FUNCTION_ARGS)
{
#if MOBDB_PGSQL_VERSION < 110000
  RangeType *r1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_RANGE(0);
  RangeType *r2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_RANGE(1);
#else
  RangeType *r1 = PG_ARGISNULL(0) ? NULL : PG_GETARG_RANGE_P(0);
  RangeType *r2 = PG_ARGISNULL(1) ? NULL : PG_GETARG_RANGE_P(1);
#endif

  if (!r2 && !r1)
    PG_RETURN_NULL();
  if (r1 && !r2)
    PG_RETURN_POINTER(r1);
  if (r2 && !r1)
    PG_RETURN_POINTER(r2);

  TypeCacheEntry* typcache = range_get_typcache(fcinfo, RangeTypeGetOid(r1));
  /* Non-strict union */
  RangeType *result = range_union_internal(typcache, r1, r2, false); 
  PG_RETURN_POINTER(result);
}

/******************************************************************************/
