/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @file
 * @brief General utility functions for temporal types
 */

#include "temporal/type_util.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/lsyscache.h>
#include <catalog/pg_type_d.h>
#include <utils/array.h>
#include <utils/rangetypes.h>
#if POSTGRESQL_VERSION_NUMBER >= 140000
  #include <utils/multirangetypes.h>
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
#endif
#if POSE
  #include "pose/pose.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/doublen.h"

extern Datum range_out(PG_FUNCTION_ARGS);
extern Datum multirange_out(PG_FUNCTION_ARGS);

/*****************************************************************************
 * Call PostgreSQL functions
 *****************************************************************************/

/**
 * @brief Call receive function of the base type
 */
Datum
call_recv(meosType type, StringInfo buf)
{
  if (type == T_DOUBLE2)
    return PointerGetDatum(double2_recv(buf));
  if (type == T_DOUBLE3)
    return PointerGetDatum(double3_recv(buf));
  if (type == T_DOUBLE4)
    return PointerGetDatum(double4_recv(buf));

  Oid typid = type_oid(type);
  if (typid == 0)
    elog(ERROR, "Unknown type when calling receive function: %s",
      meostype_name(type));
  Oid recvfunc;
  Oid basetypid;
  FmgrInfo recvfuncinfo;
  getTypeBinaryInputInfo(typid, &recvfunc, &basetypid);
  fmgr_info(recvfunc, &recvfuncinfo);
  return ReceiveFunctionCall(&recvfuncinfo, buf, basetypid, -1);
}

/**
 * @brief Call send function of the base type
 */
bytea *
call_send(meosType type, Datum value)
{
  if (type == T_DOUBLE2)
    return double2_send(DatumGetDouble2P(value));
  if (type == T_DOUBLE3)
    return double3_send(DatumGetDouble3P(value));
  if (type == T_DOUBLE4)
    return double4_send(DatumGetDouble4P(value));

  Oid typid = type_oid(type);
  if (typid == 0)
    elog(ERROR, "Unknown type when calling send function: %s",
      meostype_name(type));
  Oid sendfunc;
  bool isvarlena;
  FmgrInfo sendfuncinfo;
  getTypeBinaryOutputInfo(typid, &sendfunc, &isvarlena);
  fmgr_info(sendfunc, &sendfuncinfo);
  return SendFunctionCall(&sendfuncinfo, value);
}

/**
 * @brief Call PostgreSQL function with 1 argument
 */
Datum
call_function1(PGFunction func, Datum arg1)
{
  LOCAL_FCINFO(fcinfo, 1);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 1, DEFAULT_COLLATION_OID, NULL,
    NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "Function %p returned NULL", (void *) func);
  return result;
}

#if 0 /* unused */
/**
 * @brief Call PostgreSQL function with 2 arguments
 */
Datum
call_function2(PGFunction func, Datum arg1, Datum arg2)
{
  LOCAL_FCINFO(fcinfo, 2);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_nargs = 2;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 2, DEFAULT_COLLATION_OID, NULL,
    NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  fcinfo->args[1].value = arg2;
  fcinfo->args[1].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}
#endif /* unused */

/**
 * @brief Call PostgreSQL function with 3 arguments
 */
Datum
call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3)
{
  LOCAL_FCINFO(fcinfo, 3);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 3, DEFAULT_COLLATION_OID, NULL,
    NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  fcinfo->args[1].value = arg2;
  fcinfo->args[1].isnull = false;
  fcinfo->args[2].value = arg3;
  fcinfo->args[2].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}

/*****************************************************************************
 * Array functions
 *****************************************************************************/

/**
 * @brief Extract a C array from a PostgreSQL array containing datums.
 * @note If array elements are pass-by-ref data type, the returned Datums will
 * be pointers into the array object.
 */
Datum *
datumarr_extract(ArrayType *array, int *count)
{
  bool byval;
  int16 typlen;
  char align;
  get_typlenbyvalalign(array->elemtype, &typlen, &byval, &align);
  Datum *result;
  deconstruct_array(array, array->elemtype, typlen, byval, align, &result,
    NULL, count);
  return result;
}

#if CBUFFER
/**
 * @brief Extract a C array from a PostgreSQL array containing circular buffers
 */
Cbuffer **
cbufferarr_extract(ArrayType *array, int *count)
{
  Cbuffer **result;
  deconstruct_array(array, array->elemtype, -1, false, 'd', (Datum **) &result,
    NULL, count);
  return result;
}
#endif /* CBUFFER */

#if POSE
/**
 * @brief Extract a C array from a PostgreSQL array containing poses
 */
Pose **
posearr_extract(ArrayType *array, int *count)
{
  Pose **result;
  deconstruct_array(array, array->elemtype, -1, false, 'd', (Datum **) &result,
    NULL, count);
  return result;
}
#endif /* POSE */

#if RGEO
/**
 * @brief Extract a C array from a PostgreSQL array containing rigid geometries
 */
Pose **
rgeomarr_extract(ArrayType *array, int *count)
{
  Pose **result;
  deconstruct_array(array, array->elemtype, -1, false, 'd', (Datum **) &result,
    NULL, count);
  return result;
}
#endif /* RGEO */

/**
 * @brief Extract a C array from a PostgreSQL array containing spans
 */
Span *
spanarr_extract(ArrayType *array, int *count)
{
  Span **spans = (Span **) datumarr_extract(array, count);
  Span *result = palloc(sizeof(Span) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = *spans[i];
  pfree(spans);
  return result;
}

/**
 * @brief Extract a C array from a PostgreSQL array containing spatiotemporal
 * boxes
 */
STBox *
stboxarr_extract(ArrayType *array, int *count)
{
  STBox **boxes = (STBox **) datumarr_extract(array, count);
  STBox *result = palloc(sizeof(STBox) * *count);
  for (int i = 0; i < *count; i++)
    result[i] = *boxes[i];
  pfree(boxes);
  return result;
}

/**
 * @brief Extract a C array from a PostgreSQL array containing temporal values
 */
Temporal **
temparr_extract(ArrayType *array, int *count)
{
  Temporal **result;
  deconstruct_array(array, array->elemtype, -1, false, 'd', (Datum **) &result,
    NULL, count);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a C array of datums converted into a PostgreSQL array.
 * @note The values will be copied into the object even if pass-by-ref type
 */
ArrayType *
datumarr_to_array(Datum *values, int count, meosType type)
{
  int16 elmlen;
  bool elmbyval;
  char elmalign;
  assert(count > 0);
  Oid typid = type_oid(type);
  get_typlenbyvalalign(typid, &elmlen, &elmbyval, &elmalign);
  return construct_array(values, count, typid, elmlen, elmbyval, elmalign);
}

/**
 * @brief Return a C array of timestamps converted into a PostgreSQL array
 */
ArrayType *
int64arr_to_array(int64 *values, int count)
{
  assert(count > 0);
  Datum *dvalues = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    dvalues[i] = Int64GetDatum(values[i]);
  ArrayType *result = construct_array(dvalues, count, INT8OID, 8, true, 'd');
  pfree(dvalues); pfree(values);
  return result;
}

/**
 * @brief Return a C array of timestamps converted into a PostgreSQL array
 */
ArrayType *
tstzarr_to_array(TimestampTz *times, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) times, count, TIMESTAMPTZOID,
    8, true, 'd');
  pfree(times);
  return result;
}

/**
 * @brief Return a C array of text values converted into a PostgreSQL array
 */
ArrayType *
strarr_to_textarray(char **strarr, int count)
{
  assert(count > 0);
  text **textarr = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    textarr[i] = cstring2text(strarr[i]);
  ArrayType *result = construct_array((Datum *) textarr, count, TEXTOID, -1,
    false, 'i');
  for (int i = 0; i < count; i++)
    pfree(strarr[i]);
  return result;
}

#if CBUFFER
/**
 * @brief Return a C array of circular buffers converted into a PostgreSQL array
 */
ArrayType *
cbufferarr_to_array(Cbuffer **cbarr, int count, bool free_all)
{
  assert(count > 0);
  Oid cbuftypid = type_oid(T_CBUFFER);
  ArrayType *result = construct_array((Datum *) cbarr, count, cbuftypid, -1,
    false, 'd');
  if (free_all)
    for (int i = 0; i < count; i++)
      pfree(cbarr[i]);
  pfree(cbarr);
  return result;
}
#endif /* CBUFFER */

#if POSE
/**
 * @brief Return a C array of poses converted into a PostgreSQL array
 */
ArrayType *
posearr_to_array(Pose **posearr, int count, bool free_all)
{
  assert(count > 0);
  Oid posetypid = type_oid(T_POSE);
  ArrayType *result = construct_array((Datum *) posearr, count, posetypid, -1,
    false, 'd');
  if (free_all)
    for (int i = 0; i < count; i++)
      pfree(posearr[i]);
  pfree(posearr);
  return result;
}
#endif /* POSE */

/**
 * @brief Return a C array of spans converted into a PostgreSQL array
 */
ArrayType *
spanarr_to_array(Span *spanarr, int count)
{
  assert(count > 0);
  Span **spans = palloc(sizeof(Span *) * count);
  for (int i = 0; i < count; i++)
    spans[i] = &spanarr[i];
  ArrayType *result = construct_array((Datum *) spans, count,
    type_oid(spans[0]->spantype), sizeof(Span), false, 'd');
  pfree(spans);
  return result;
}

/**
 * @brief Return a C array of temporal boxes converted into a PostgreSQL array
 */
ArrayType *
tboxarr_to_array(TBox *boxarr, int count)
{
  assert(count > 0);
  TBox **boxes = palloc(sizeof(TBox *) * count);
  for (int i = 0; i < count; i++)
    boxes[i] = &boxarr[i];
  ArrayType *result = construct_array((Datum *) boxes, count, type_oid(T_TBOX),
    sizeof(TBox), false, 'd');
  pfree(boxes);
  return result;
}

/**
 * @brief Return a C array of spatiotemporal boxes converted into a PostgreSQL array
 */
ArrayType *
stboxarr_to_array(STBox *boxarr, int count)
{
  assert(count > 0);
  STBox **boxes = palloc(sizeof(STBox *) * count);
  for (int i = 0; i < count; i++)
    boxes[i] = &boxarr[i];
  ArrayType *result = construct_array((Datum *) boxes, count,
    type_oid(T_STBOX), sizeof(STBox), false, 'd');
  pfree(boxes);
  return result;
}

/**
 * @brief Return a C array of temporal values converted into a PostgreSQL array
 */
ArrayType *
temparr_to_array(Temporal **temparr, int count, bool free_all)
{
  assert(count > 0);
  Oid temptypid = type_oid(temparr[0]->temptype);
  ArrayType *result = construct_array((Datum *) temparr, count, temptypid, -1,
    false, 'd');
  if (free_all)
  {
    for (int i = 0; i < count; i++)
      pfree(temparr[i]);
  }
  pfree(temparr);
  return result;
}

/*****************************************************************************
 * Range functions
 *****************************************************************************/

/**
 * @brief Return a range value from given arguments
 */
RangeType *
range_make(Datum from, Datum to, bool lower_inc, bool upper_inc,
  meosType basetype)
{
  Oid rangetypid = 0;
  assert(basetype == T_INT4 || basetype == T_INT8 || basetype == T_DATE ||
    basetype == T_TIMESTAMPTZ);
  if (basetype == T_INT4)
    rangetypid = type_oid(T_INT4RANGE);
  else if (basetype == T_INT8)
    rangetypid = type_oid(T_INT8RANGE);
  else if (basetype == T_DATE)
    rangetypid = type_oid(T_DATERANGE);
  else /* basetype == T_TIMESTAMPTZ */
    rangetypid = type_oid(T_TSTZRANGE);

  TypeCacheEntry* typcache = lookup_type_cache(rangetypid,
    TYPECACHE_RANGE_INFO);
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
#if POSTGRESQL_VERSION_NUMBER >= 160000
  return make_range(typcache, &lower, &upper, false, NULL);
#else
  return make_range(typcache, &lower, &upper, false);
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */
}

#if POSTGRESQL_VERSION_NUMBER >= 140000
/**
 * @brief Return a range value from given arguments
 */
MultirangeType *
multirange_make(const SpanSet *ss)
{
  RangeType **ranges = palloc(sizeof(RangeType *) * ss->count);
  Oid rangetypid = basetype_rangetype(ss->basetype);
  Oid mrangetypid = basetype_multirangetype(ss->basetype);
  TypeCacheEntry *typcache = lookup_type_cache(rangetypid,
    TYPECACHE_RANGE_INFO);
  for (int i = 0; i < ss->count; i++)
  {
    const Span *s = SPANSET_SP_N(ss, i);
    RangeBound lower;
    RangeBound upper;
    lower.val = s->lower;
    lower.infinite = false;
    lower.inclusive = s->lower_inc;
    lower.lower = true;
    upper.val = s->upper;
    upper.infinite = false;
    upper.inclusive = s->upper_inc;
    upper.lower = false;
#if POSTGRESQL_VERSION_NUMBER >= 160000
    ranges[i] = make_range(typcache, &lower, &upper, false, NULL);
#else
    ranges[i] = make_range(typcache, &lower, &upper, false);
#endif /* POSTGRESQL_VERSION_NUMBER >= 160000 */
  }
  MultirangeType *result = make_multirange(mrangetypid, typcache, ss->count,
    ranges);
  pfree_array((void **) ranges, ss->count);
  return result;
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */

#if DEBUG_BUILD
/**
 * @brief Return the string representation of a range
 * @param[in] r Timestamp
 * @note PostgreSQL function: @p range_out(PG_FUNCTION_ARGS)
 */
char *
pg_range_out(RangeType *r)
{
  Datum d = PointerGetDatum(r);
  return DatumGetCString(call_function1(range_out, d));
}

#if POSTGRESQL_VERSION_NUMBER >= 140000
/**
 * @brief Return the string representation of a multirange
 * @param[in] r Timestamp
 * @note PostgreSQL function: @p range_out(PG_FUNCTION_ARGS)
 */
char *
pg_multirange_out(MultirangeType *mr)
{
  Datum d = PointerGetDatum(mr);
  return DatumGetCString(call_function1(multirange_out, d));
}
#endif /* POSTGRESQL_VERSION_NUMBER >= 140000 */
#endif /* DEBUG_BUILD */

/*****************************************************************************/
