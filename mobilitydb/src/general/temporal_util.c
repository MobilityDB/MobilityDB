/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief General utility functions for temporal types.
 */

#include "general/temporal_util.h"

/* C */
#include <assert.h>
/* PostgreSQL */
#include <utils/lsyscache.h>
#include <catalog/pg_collation_d.h>
#include <catalog/pg_type_d.h>
#include <utils/array.h>
#include <utils/rangetypes.h>
#include <utils/varlena.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_general/doublen.h"
#include "pg_general/temporal_catalog.h"

/*****************************************************************************
 * Call PostgreSQL functions
 *****************************************************************************/

/**
 * Call receive function of the base type
 */
Datum
call_recv(mobdbType type, StringInfo buf)
{
  if (type == T_DOUBLE2)
    return PointerGetDatum(double2_recv(buf));
  if (type == T_DOUBLE3)
    return PointerGetDatum(double3_recv(buf));
  if (type == T_DOUBLE4)
    return PointerGetDatum(double4_recv(buf));

  Oid typid = type_oid(type);
  if (typid == 0)
    elog(ERROR, "Unknown type when calling receive function: %d", type);
  Oid recvfunc;
  Oid basetypid;
  FmgrInfo recvfuncinfo;
  getTypeBinaryInputInfo(typid, &recvfunc, &basetypid);
  fmgr_info(recvfunc, &recvfuncinfo);
  return ReceiveFunctionCall(&recvfuncinfo, buf, basetypid, -1);
}

/**
 * Call send function of the base type
 */
bytea *
call_send(mobdbType type, Datum value)
{
  if (type == T_DOUBLE2)
    return double2_send(DatumGetDouble2P(value));
  if (type == T_DOUBLE3)
    return double3_send(DatumGetDouble3P(value));
  if (type == T_DOUBLE4)
    return double4_send(DatumGetDouble4P(value));

  Oid typid = type_oid(type);
  if (typid == 0)
    elog(ERROR, "Unknown type when calling send function: %d", type);
  Oid sendfunc;
  bool isvarlena;
  FmgrInfo sendfuncinfo;
  getTypeBinaryOutputInfo(typid, &sendfunc, &isvarlena);
  fmgr_info(sendfunc, &sendfuncinfo);
  return SendFunctionCall(&sendfuncinfo, value);
}

/**
 * Call PostgreSQL function with 1 argument
 */
Datum
call_function1(PGFunction func, Datum arg1)
{
  LOCAL_FCINFO(fcinfo, 1);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 1, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "Function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 2 arguments
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
  InitFunctionCallInfoData(*fcinfo, &flinfo, 2, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo->args[0].value = arg1;
  fcinfo->args[0].isnull = false;
  fcinfo->args[1].value = arg2;
  fcinfo->args[1].isnull = false;
  result = (*func) (fcinfo);
  if (fcinfo->isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}

#if 0 /* not used */
/**
 * Call PostgreSQL function with 3 arguments
 */
Datum
call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3)
{
  LOCAL_FCINFO(fcinfo, 3);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(*fcinfo, &flinfo, 3, DEFAULT_COLLATION_OID, NULL, NULL);
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
#endif /* not used */

/*****************************************************************************
 * Array functions
 *****************************************************************************/

/**
 * Extract a C array from a PostgreSQL array containing datums
 * If array elements are pass-by-ref data type, the returned Datums will
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
  deconstruct_array(array, array->elemtype, typlen, byval, align,
    &result, NULL, count);
  return result;
}

/**
 * Extract a C array from a PostgreSQL array containing timestamps
 */
TimestampTz *
timestamparr_extract(ArrayType *array, int *count)
{
  return (TimestampTz *) datumarr_extract(array, count);
}

/**
 * Extract a C array from a PostgreSQL array containing periods
 */
Period **
periodarr_extract(ArrayType *array, int *count)
{
  return (Period **) datumarr_extract(array, count);
}

/**
 * Extract a C array from a PostgreSQL array containing spans
 */
Span **
spanarr_extract(ArrayType *array, int *count)
{
  return (Span **) datumarr_extract(array, count);
}

/**
 * Extract a C array from a PostgreSQL array containing temporal values
 */
Temporal **
temporalarr_extract(ArrayType *array, int *count)
{
  Temporal **result;
  deconstruct_array(array, array->elemtype, -1, false, 'd',
    (Datum **) &result, NULL, count);
  return result;
}

/*****************************************************************************/

/**
 * Convert a C array of datums into a PostgreSQL array.
 * Note that the values will be copied into the object even if pass-by-ref type
 */
ArrayType *
datumarr_to_array(Datum *values, int count, mobdbType type)
{
  int16 elmlen;
  bool elmbyval;
  char elmalign;
  assert(count > 0);
  Oid typid = type_oid(type);
  get_typlenbyvalalign(typid, &elmlen, &elmbyval, &elmalign);
  ArrayType *result = construct_array(values, count, typid, elmlen, elmbyval,
    elmalign);
  return result;
}

/**
 * Convert a C array of timestamps into a PostgreSQL array
 */
ArrayType *
int64arr_to_array(const int64 *longints, int count)
{
  assert(count > 0);
  Datum *values = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    values[i] = Int64GetDatum(longints[i]);
  ArrayType *result = construct_array(values, count, INT8OID, 8, true, 'd');
  pfree(values);
  return result;
}

/**
 * Convert a C array of timestamps into a PostgreSQL array
 */
ArrayType *
timestamparr_to_array(const TimestampTz *times, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) times, count, TIMESTAMPTZOID,
    8, true, 'd');
  return result;
}

/**
 * Convert a C array of periods into a PostgreSQL array
 */
ArrayType *
periodarr_to_array(const Period **periods, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) periods, count,
    type_oid(T_PERIOD), sizeof(Period), false, 'd');
  return result;
}

/**
 * Convert a C array of spans into a PostgreSQL array
 */
ArrayType *
spanarr_to_array(Span **spans, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *) spans, count,
    type_oid(spans[0]->spantype), sizeof(Span), false, 'd');
  return result;
}

/**
 * Convert a C array of text values into a PostgreSQL array
 */
ArrayType *
strarr_to_textarray(char **strarr, int count)
{
  assert(count > 0);
  text **textarr = palloc(sizeof(text *) * count);
  for (int i = 0; i < count; i++)
    textarr[i] = cstring_to_text(strarr[i]);
  ArrayType *result = construct_array((Datum *) textarr, count, TEXTOID, -1,
    false, 'i');
  pfree_array((void **)textarr, count);
  return result;
}

/**
 * Convert a C array of temporal values into a PostgreSQL array
 */
ArrayType *
temporalarr_to_array(const Temporal **temporalarr, int count)
{
  assert(count > 0);
  Oid temptypid = type_oid(temporalarr[0]->temptype);
  ArrayType *result = construct_array((Datum *) temporalarr, count, temptypid,
    -1, false, 'd');
  return result;
}

/**
 * Convert a C array of spatiotemporal boxes into a PostgreSQL array
 */
ArrayType *
stboxarr_to_array(STBOX *boxarr, int count)
{
  assert(count > 0);
  STBOX **boxes = palloc(sizeof(STBOX *) * count);
  for (int i = 0; i < count; i++)
    boxes[i] = &boxarr[i];
  ArrayType *result = construct_array((Datum *) boxes, count,
    type_oid(T_STBOX), sizeof(STBOX), false, 'd');
  pfree(boxes);
  return result;
}

/*****************************************************************************
 * Range functions
 *****************************************************************************/

/**
 * Construct a range value from given arguments
 */
RangeType *
range_make(Datum from, Datum to, bool lower_inc, bool upper_inc,
  mobdbType basetype)
{
  Oid rangetypid = 0;
  assert (basetype == T_INT4 || basetype == T_TIMESTAMPTZ);
  if (basetype == T_INT4)
    rangetypid = type_oid(T_INT4RANGE);
  else /* basetype == T_TIMESTAMPTZ */
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

/*****************************************************************************/
