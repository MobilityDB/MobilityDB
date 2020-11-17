/*****************************************************************************
 *
 * temporal_util.c
 *    Miscellaneous utility functions for temporal types.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB contributors
 *
 * Permission to use, copy, modify, and distribute this software and its documentation for any purpose, without fee, and without a written agreement is hereby
 * granted, provided that the above copyright notice and this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
 * PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO PROVIDE
 * MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

#include "temporal_util.h"

#include <assert.h>
#include <catalog/pg_collation.h>
#include <fmgr.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
#include <utils/varlena.h>

#include "period.h"
#include "temporaltypes.h"
#include "oidcache.h"
#include "doublen.h"

#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

/*
 * This is required for builds against pgsql
 */
PG_MODULE_MAGIC;

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/**
 * Initialize the extension
 */
void
_PG_init(void)
{
  /* elog(WARNING, "This is MobilityDB."); */
  temporalgeom_init();
}

/**
 * Align to double
 */
size_t
double_pad(size_t size)
{
  if (size % 8)
    return size + (8 - size % 8);
  return size;
}

/**
 * Returns true if the values of the type are passed by value.
 *
 * This function is called only for the base types of the temporal types
 * and for TimestampTz. To avoid a call of the slow function get_typbyval
 * (which makes a lookup call), the known base types are explicitly enumerated.
 */
bool
get_typbyval_fast(Oid type)
{
  ensure_temporal_base_type_all(type);
  bool result = false;
  if (type == BOOLOID || type == INT4OID || type == FLOAT8OID ||
    type == TIMESTAMPTZOID)
    result = true;
  else if (type == type_oid(T_DOUBLE2) || type == TEXTOID)
    result = false;
  else if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY) ||
       type == type_oid(T_DOUBLE3) || type == type_oid(T_DOUBLE4))
    result = false;
  return result;
}

/**
 * returns the length of type
 *
 * This function is called only for the base types of the temporal types
 * and for TimestampTz. To avoid a call of the slow function get_typlen
 * (which makes a lookup call), the known base types are explicitly enumerated.
 */
int
get_typlen_fast(Oid type)
{
  ensure_temporal_base_type_all(type);
  int result = 0;
  if (type == BOOLOID)
    result = 1;
  else if (type == INT4OID)
    result = 4;
  else if (type == FLOAT8OID || type == TIMESTAMPTZOID)
    result = 8;
  else if (type == type_oid(T_DOUBLE2))
    result = 16;
  else if (type == TEXTOID)
    result = -1;
  else if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY))
    result = -1;
  else if (type == type_oid(T_DOUBLE3))
    result = 24;
  else if (type == type_oid(T_DOUBLE4))
    result = 32;
  return result;
}

/**
 * Copy a Datum if it is passed by reference
 */
Datum
datum_copy(Datum value, Oid type)
{
  /* For types passed by value */
  if (get_typbyval_fast(type))
    return value;
  /* For types passed by reference */
  int typlen = get_typlen_fast(type);
  size_t value_size = typlen != -1 ? (unsigned int) typlen : VARSIZE(value);
  void *result = palloc0(value_size);
  memcpy(result, DatumGetPointer(value), value_size);
  return PointerGetDatum(result);
}

/**
 * Convert a number to a double
 */
double
datum_double(Datum d, Oid valuetypid)
{
  double result = 0.0;
  ensure_tnumber_base_type(valuetypid);
  if (valuetypid == INT4OID)
    result = (double)(DatumGetInt32(d));
  else /* valuetypid == FLOAT8OID */
    result = DatumGetFloat8(d);
  return result;
}

/*****************************************************************************
 * Call PostgreSQL functions
 *****************************************************************************/

/**
 * Call input function of the base type
 */
Datum
call_input(Oid type, char *str)
{
  Oid infunc;
  Oid basetype;
  FmgrInfo infuncinfo;
  getTypeInputInfo(type, &infunc, &basetype);
  fmgr_info(infunc, &infuncinfo);
  return InputFunctionCall(&infuncinfo, str, basetype, -1);
}

/**
 * Call output function of the base type
 */
char *
call_output(Oid type, Datum value)
{
  Oid outfunc;
  bool isvarlena;
  FmgrInfo outfuncinfo;
  getTypeOutputInfo(type, &outfunc, &isvarlena);
  fmgr_info(outfunc, &outfuncinfo);
  return OutputFunctionCall(&outfuncinfo, value);
}

/**
 * Call send function of the base type
 */
bytea *
call_send(Oid type, Datum value)
{
  Oid sendfunc;
  bool isvarlena;
  FmgrInfo sendfuncinfo;
  getTypeBinaryOutputInfo(type, &sendfunc, &isvarlena);
  fmgr_info(sendfunc, &sendfuncinfo);
  return SendFunctionCall(&sendfuncinfo, value);
}

/**
 * Call receive function of the base type
 */
Datum
call_recv(Oid type, StringInfo buf)
{
  Oid recvfunc;
  Oid basetype;
  FmgrInfo recvfuncinfo;
  getTypeBinaryInputInfo(type, &recvfunc, &basetype);
  fmgr_info(recvfunc, &recvfuncinfo);
  return ReceiveFunctionCall(&recvfuncinfo, buf, basetype, -1);
}

/**
 * Call PostgreSQL function with 1 argument
 */
#if MOBDB_PGSQL_VERSION >= 120000
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
  memset(&flinfo, 0, sizeof(flinfo)) ;
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

/**
 * Call PostgreSQL function with 3 arguments
 */
Datum
call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3)
{
  LOCAL_FCINFO(fcinfo, 3);
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
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
#else /* MOBDB_PGSQL_VERSION < 120000 */
/**
 * Call PostgreSQL function with 1 argument
 */
Datum
call_function1(PGFunction func, Datum arg1)
{
  FunctionCallInfoData fcinfo;
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo));
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(fcinfo, &flinfo, 1, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo.arg[0] = arg1;
  fcinfo.argnull[0] = false;
  result = (*func) (&fcinfo);
  if (fcinfo.isnull)
    elog(ERROR, "Function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 2 arguments
 */
Datum
call_function2(PGFunction func, Datum arg1, Datum arg2)
{
  FunctionCallInfoData fcinfo;
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(fcinfo, &flinfo, 2, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo.arg[0] = arg1;
  fcinfo.argnull[0] = false;
  fcinfo.arg[1] = arg2;
  fcinfo.argnull[1] = false;
  result = (*func) (&fcinfo);
  if (fcinfo.isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}

/**
 * Call PostgreSQL function with 3 arguments
 */
Datum
call_function3(PGFunction func, Datum arg1, Datum arg2, Datum arg3)
{
  FunctionCallInfoData fcinfo;
  FmgrInfo flinfo;
  memset(&flinfo, 0, sizeof(flinfo)) ;
  flinfo.fn_mcxt = CurrentMemoryContext;
  Datum result;
  InitFunctionCallInfoData(fcinfo, &flinfo, 3, DEFAULT_COLLATION_OID, NULL, NULL);
  fcinfo.arg[0] = arg1;
  fcinfo.argnull[0] = false;
  fcinfo.arg[1] = arg2;
  fcinfo.argnull[1] = false;
  fcinfo.arg[2] = arg3;
  fcinfo.argnull[2] = false;
  result = (*func) (&fcinfo);
  if (fcinfo.isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);
  return result;
}
#endif

/*****************************************************************************/

/* CallerFInfoFunctionCall 1 to 3 are provided by PostGIS */

#if MOBDB_PGSQL_VERSION < 120000
Datum
CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo, Oid collation,
  Datum arg1, Datum arg2, Datum arg3, Datum arg4)
{
  FunctionCallInfoData fcinfo;
  Datum    result;

  InitFunctionCallInfoData(fcinfo, flinfo, 3, collation, NULL, NULL);

  fcinfo.arg[0] = arg1;
  fcinfo.arg[1] = arg2;
  fcinfo.arg[2] = arg3;
  fcinfo.arg[3] = arg4;
  fcinfo.argnull[0] = false;
  fcinfo.argnull[1] = false;
  fcinfo.argnull[2] = false;
  fcinfo.argnull[3] = false;

  result = (*func) (&fcinfo);

  /* Check for null result, since caller is clearly not expecting one */
  if (fcinfo.isnull)
    elog(ERROR, "function %p returned NULL", (void *) func);

  return result;
}
#else
/* PgSQL 12+ still lacks 3-argument version of these functions */
Datum
CallerFInfoFunctionCall4(PGFunction func, FmgrInfo *flinfo, Oid collation,
  Datum arg1, Datum arg2, Datum arg3, Datum arg4)
{
    LOCAL_FCINFO(fcinfo, 4);
    Datum       result;

    InitFunctionCallInfoData(*fcinfo, flinfo, 3, collation, NULL, NULL);

    fcinfo->args[0].value = arg1;
    fcinfo->args[0].isnull = false;
    fcinfo->args[1].value = arg2;
    fcinfo->args[1].isnull = false;
    fcinfo->args[2].value = arg3;
    fcinfo->args[2].isnull = false;
    fcinfo->args[3].value = arg4;
    fcinfo->args[3].isnull = false;

    result = (*func) (fcinfo);

    /* Check for null result, since caller is clearly not expecting one */
    if (fcinfo->isnull)
        elog(ERROR, "function %p returned NULL", (void *) func);

    return result;
}
#endif

/*****************************************************************************
 * Array functions
 *****************************************************************************/

/**
 * Returns the string resulting from assembling the array of strings.
 * The function frees the memory of the input strings after finishing.
 */
char *
stringarr_to_string(char **strings, int count, int outlen,
  char *prefix, char open, char close)
{
  char *result = palloc(strlen(prefix) + outlen + 3);
  result[outlen] = '\0';
  size_t pos = 0;
  strcpy(result, prefix);
  pos += strlen(prefix);
  result[pos++] = open;
  for (int i = 0; i < count; i++)
  {
    strcpy(result + pos, strings[i]);
    pos += strlen(strings[i]);
    result[pos++] = ',';
    result[pos++] = ' ';
    pfree(strings[i]);
  }
  result[pos - 2] = close;
  result[pos - 1] = '\0';
  pfree(strings);
  return result;
}

/**
 * Extract a C array from a PostgreSQL array containing datums
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
 * Extract a C array from a PostgreSQL array containing ranges
 */
RangeType **
rangearr_extract(ArrayType *array, int *count)
{
  return (RangeType **) datumarr_extract(array, count);
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
 * Convert a C array of datums into a PostgreSQL array
 */
ArrayType *
datumarr_to_array(Datum *values, int count, Oid type)
{
  int16 elmlen;
  bool elmbyval;
  char elmalign;
  assert(count > 0);
  get_typlenbyvalalign(type, &elmlen, &elmbyval, &elmalign);
  ArrayType *result = construct_array(values, count, type, elmlen, elmbyval, elmalign);
  return result;
}

/**
 * Convert a C array of timestamps into a PostgreSQL array
 */
ArrayType *
timestamparr_to_array(TimestampTz *times, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *)times, count, TIMESTAMPTZOID, 8, true, 'd');
  return result;
}

/**
 * Convert a C array of periods into a PostgreSQL array
 */
ArrayType *
periodarr_to_array(Period **periods, int count)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *)periods, count, type_oid(T_PERIOD),
    sizeof(Period), false, 'd');
  return result;
}

/**
 * Convert a C array of ranges into a PostgreSQL array
 */
ArrayType *
rangearr_to_array(RangeType **ranges, int count, Oid type, bool free)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *)ranges, count, type, -1, false, 'd');
  if (free)
  {
    for (int i = 0; i < count; i++)
      pfree(ranges[i]);
    pfree(ranges);
  }
  return result;
}

/**
 * Convert a C array of text values into a PostgreSQL array
 */
ArrayType *
textarr_to_array(text **textarr, int count, bool free)
{
  assert(count > 0);
  ArrayType *result = construct_array((Datum *)textarr, count, TEXTOID, -1, false, 'i');
  if (free)
  {
    for (int i = 0; i < count; i++)
      pfree(textarr[i]);
    pfree(textarr);
  }
  return result;
}

/**
 * Convert a C array of temporal values into a PostgreSQL array
 */
ArrayType *
temporalarr_to_array(Temporal **temporalarr, int count)
{
  assert(count > 0);
  Oid type = temporal_oid_from_base(temporalarr[0]->valuetypid);
  ArrayType *result = construct_array((Datum *)temporalarr, count, type, -1, false, 'd');
  return result;
}

/**
 * Convert a C array of spatiotemporal boxes into a PostgreSQL array
 */
ArrayType *
stboxarr_to_array(STBOX *boxarr, int count)
{
  assert(count > 0);
  STBOX **boxptrs = palloc(sizeof(STBOX *) * count);
  for (int i = 0; i < count; i++)
    boxptrs[i] = &boxarr[i];
  ArrayType *result = construct_array((Datum *)boxptrs, count, type_oid(T_STBOX), sizeof(STBOX), false, 'd');
  pfree(boxptrs);
  return result;
}

/*****************************************************************************
 * Sort functions
 *****************************************************************************/

/**
 * Comparator function for datums
 */
static int
datum_sort_cmp(const Datum *l, const Datum *r, const Oid *type)
{
  Datum x = *l;
  Datum y = *r;
  Oid t = *type;
  if (datum_eq(x, y, t))
    return 0;
  else if (datum_lt(x, y, t))
    return -1;
  else
    return 1;
}

/**
 * Comparator function for timestamps
 */
static int
timestamp_sort_cmp(const TimestampTz *l, const TimestampTz *r)
{
  TimestampTz x = *l;
  TimestampTz y = *r;
  return timestamp_cmp_internal(x, y);
}

/**
 * Comparator function for periods
 */
static int
period_sort_cmp(const Period **l, const Period **r)
{
  return period_cmp_internal(*l, *r);
}

/**
 * Comparator function for ranges
 */
static int
range_sort_cmp(const RangeType **l, const RangeType **r)
{
#if MOBDB_PGSQL_VERSION < 110000
  return DatumGetInt32(call_function2(range_cmp, RangeTypeGetDatum(*l),
    RangeTypeGetDatum(*r)));
#else
  return DatumGetInt32(call_function2(range_cmp, RangeTypePGetDatum(*l),
    RangeTypePGetDatum(*r)));
#endif
}

/**
 * Comparator function for temporal instants
 */
static int
tinstantarr_sort_cmp(const TInstant **l, const TInstant **r)
{
  return timestamp_cmp_internal((*l)->t, (*r)->t);
}

/**
 * Comparator function for temporal sequences
 */
static int
tsequencearr_sort_cmp(TSequence **l, TSequence **r)
{
  Period lp = (*l)->period;
  Period rp = (*r)->period;
  return period_cmp_internal(&lp, &rp);
}

/*****************************************************************************/

/**
 * Sort function for datums
 */
void
datumarr_sort(Datum *values, int count, Oid type)
{
  qsort_arg(values, (size_t) count, sizeof(Datum),
    (qsort_arg_comparator) &datum_sort_cmp, &type);
}

/**
 * Sort function for timestamps
 */
void
timestamparr_sort(TimestampTz *times, int count)
{
  qsort(times, (size_t) count, sizeof(TimestampTz),
    (qsort_comparator) &timestamp_sort_cmp);
}

/**
 * Sort function for periods
 */
void
periodarr_sort(Period **periods, int count)
{
  qsort(periods, (size_t) count, sizeof(Period *),
    (qsort_comparator) &period_sort_cmp);
}

/**
 * Sort function for ranges
 */
void
rangearr_sort(RangeType **ranges, int count)
{
  qsort(ranges, (size_t) count, sizeof(RangeType *),
    (qsort_comparator) &range_sort_cmp);
}

/**
 * Sort function for temporal instants
 */
void
tinstantarr_sort(TInstant **instants, int count)
{
  qsort(instants, (size_t) count, sizeof(TInstant *),
    (qsort_comparator) &tinstantarr_sort_cmp);
}

/**
 * Sort function for temporal sequences
 */
void
tsequencearr_sort(TSequence **sequences, int count)
{
  qsort(sequences, (size_t) count, sizeof(TSequence *),
    (qsort_comparator) &tsequencearr_sort_cmp);
}

/*****************************************************************************
 * Remove duplicate functions
 * These functions assume that the array has been sorted before
 *****************************************************************************/

/**
 * Remove duplicates from an array of datums
 */
int
datumarr_remove_duplicates(Datum *values, int count, Oid type)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (datum_ne(values[newcount], values[i], type))
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * Remove duplicates from an array of timestamps
 */
int
timestamparr_remove_duplicates(TimestampTz *values, int count)
{
  assert (count > 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (values[newcount] != values[i])
      values[++ newcount] = values[i];
  return newcount + 1;
}

/**
 * Remove duplicates from an array of temporal instants
 */
int
tinstantarr_remove_duplicates(TInstant **instants, int count)
{
  assert(count != 0);
  int newcount = 0;
  for (int i = 1; i < count; i++)
    if (! tinstant_eq(instants[newcount], instants[i]))
      instants[++ newcount] = instants[i];
  return newcount + 1;
}

/*****************************************************************************
 * Text functions
 *****************************************************************************/

/**
 * Comparison function for text values
 *
 * @note Function copied from PostgreSQL since it is not exported
 */
int
text_cmp(text *arg1, text *arg2, Oid collid)
{
  char  *a1p,
      *a2p;
  int    len1,
      len2;

  a1p = VARDATA_ANY(arg1);
  a2p = VARDATA_ANY(arg2);

  len1 = (int) VARSIZE_ANY_EXHDR(arg1);
  len2 = (int) VARSIZE_ANY_EXHDR(arg2);

  return varstr_cmp(a1p, len1, a2p, len2, collid);
}

/*****************************************************************************
 * Comparison functions on datums
 *****************************************************************************/

/* Version of the functions where the types of both arguments is equal */

/**
 * Returns true if the two values are equal
 */
bool
datum_eq(Datum l, Datum r, Oid type)
{
  ensure_temporal_base_type_all(type);
  bool result = false;
  if (type == BOOLOID || type == INT4OID || type == FLOAT8OID)
    result = l == r;
  else if (type == TEXTOID)
    result = text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) == 0;
  else if (type == type_oid(T_DOUBLE2))
    result = double2_eq((double2 *)DatumGetPointer(l), (double2 *)DatumGetPointer(r));
  else if (type == type_oid(T_DOUBLE3))
    result = double3_eq((double3 *)DatumGetPointer(l), (double3 *)DatumGetPointer(r));
  else if (type == type_oid(T_DOUBLE4))
    result = double4_eq((double4 *)DatumGetPointer(l), (double4 *)DatumGetPointer(r));
  else if (type == type_oid(T_GEOMETRY))
    //  result = DatumGetBool(call_function2(lwgeom_eq, l, r));
    result = datum_point_eq(l, r);
  else if (type == type_oid(T_GEOGRAPHY))
    //  result = DatumGetBool(call_function2(geography_eq, l, r));
    result = datum_point_eq(l, r);
  return result;
}

/**
 * Returns true if the two values are different
 */
bool
datum_ne(Datum l, Datum r, Oid type)
{
  return !datum_eq(l, r, type);
}

/**
 * Returns true if the first value is less than the second one
 */
bool
datum_lt(Datum l, Datum r, Oid type)
{
  ensure_temporal_base_type(type);
  bool result = false;
  if (type == BOOLOID)
    result = DatumGetBool(l) < DatumGetBool(r);
  else if (type == INT4OID)
    result = DatumGetInt32(l) < DatumGetInt32(r);
  else if (type == FLOAT8OID)
    result = DatumGetFloat8(l) < DatumGetFloat8(r);
  else if (type == TEXTOID)
    result = text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0;
  else if (type == type_oid(T_GEOMETRY))
    result = DatumGetBool(call_function2(lwgeom_lt, l, r));
  else if (type == type_oid(T_GEOGRAPHY))
    result = DatumGetBool(call_function2(geography_lt, l, r));
  return result;
}

/**
 * Returns true if the first value is less than or equal to the second one
 */
bool
datum_le(Datum l, Datum r, Oid type)
{
  return datum_eq(l, r, type) || datum_lt(l, r, type);
}

/**
 * Returns true if the first value is greater than the second one
 */
bool
datum_gt(Datum l, Datum r, Oid type)
{
  return datum_lt(r, l, type);
}

/**
 * Returns true if the first value is greater than or equal to the second one
 */
bool
datum_ge(Datum l, Datum r, Oid type)
{
  return datum_eq(l, r, type) || datum_lt(r, l, type);
}

/*****************************************************************************/

/*
 * Version of the functions where the types of both arguments may be different
 * but compatible, e.g., integer and float
 */

/**
 * Returns true if the two values are equal even if their type is not the same
 */
bool
datum_eq2(Datum l, Datum r, Oid typel, Oid typer)
{
  ensure_temporal_base_type_all(typel);
  ensure_temporal_base_type_all(typer);
  bool result = false;
  if ((typel == BOOLOID && typer == BOOLOID) ||
    (typel == INT4OID && typer == INT4OID) ||
    (typel == FLOAT8OID && typer == FLOAT8OID))
    result = l == r;
  else if (typel == INT4OID && typer == FLOAT8OID)
    result = DatumGetInt32(l) == DatumGetFloat8(r);
  else if (typel == FLOAT8OID && typer == INT4OID)
    result = DatumGetFloat8(l) == DatumGetInt32(r);
  else if (typel == TEXTOID && typer == TEXTOID)
    result = text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) == 0;
    /* This function is never called with doubleN */
  else if (typel == type_oid(T_GEOMETRY) && typer == type_oid(T_GEOMETRY))
    //  result = DatumGetBool(call_function2(lwgeom_eq, l, r));
    result = datum_point_eq(l, r);
  else if (typel == type_oid(T_GEOGRAPHY) && typer == type_oid(T_GEOGRAPHY))
    //  result = DatumGetBool(call_function2(geography_eq, l, r));
    result = datum_point_eq(l, r);
  return result;
}

/**
 * Returns true if the two values are different
 */
bool
datum_ne2(Datum l, Datum r, Oid typel, Oid typer)
{
  return !datum_eq2(l, r, typel, typer);
}

/**
 * Returns true if the first value is less than the second one
 */
bool
datum_lt2(Datum l, Datum r, Oid typel, Oid typer)
{
  assert(typel == INT4OID || typel == FLOAT8OID || typel == TEXTOID);
  assert(typer == INT4OID || typer == FLOAT8OID || typer == TEXTOID);
  bool result = false;
  if (typel == INT4OID && typer == INT4OID)
    result = DatumGetInt32(l) < DatumGetInt32(r);
  else if (typel == INT4OID && typer == FLOAT8OID)
    result = DatumGetInt32(l) < DatumGetFloat8(r);
  else if (typel == FLOAT8OID && typer == INT4OID)
    result = DatumGetFloat8(l) < DatumGetInt32(r);
  else if (typel == FLOAT8OID && typer == FLOAT8OID)
    result = DatumGetFloat8(l) < DatumGetFloat8(r);
  else if (typel == TEXTOID && typer == TEXTOID)
    result = text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0;
  return result;
}

/**
 * Returns true if the first value is less than or equal to the second one
 */
bool
datum_le2(Datum l, Datum r, Oid typel, Oid typer)
{
  return datum_eq2(l, r, typel, typer) || datum_lt2(l, r, typel, typer);
}

/**
 * Returns true if the first value is greater than the second one
 */
bool
datum_gt2(Datum l, Datum r, Oid typel, Oid typer)
{
  return datum_lt2(r, l, typer, typel);
}

/**
 * Returns true if the first value is greater than or equal to the second one
 */
bool
datum_ge2(Datum l, Datum r, Oid typel, Oid typer)
{
  return datum_eq2(l, r, typel, typer) || datum_gt2(l, r, typel, typer);
}

/*****************************************************************************/

/**
 * Returns a Datum true if the two values are equal
 */
Datum
datum2_eq2(Datum l, Datum r, Oid typel, Oid typer)
{
  return BoolGetDatum(datum_eq2(l, r, typel, typer));
}

/**
 * Returns a Datum true if the two values are different
 */
Datum
datum2_ne2(Datum l, Datum r, Oid typel, Oid typer)
{
  return BoolGetDatum(datum_ne2(l, r, typel, typer));
}

/**
 * Returns a Datum true if the first value is less than the second one
 */
Datum
datum2_lt2(Datum l, Datum r, Oid typel, Oid typer)
{
  return BoolGetDatum(datum_lt2(l, r, typel, typer));
}

/**
 * Returns a Datum true if the first value is less than or equal to the second one
 */
Datum
datum2_le2(Datum l, Datum r, Oid typel, Oid typer)
{
  return BoolGetDatum(datum_le2(l, r, typel, typer));
}

/**
 * Returns a Datum true if the first value is greater than the second one
 */
Datum
datum2_gt2(Datum l, Datum r, Oid typel, Oid typer)
{
  return BoolGetDatum(datum_gt2(l, r, typel, typer));
}

/**
 * Returns a Datum true if the first value is greater than or equal to the second one
 */
Datum
datum2_ge2(Datum l, Datum r, Oid typel, Oid typer)
{
  return BoolGetDatum(datum_ge2(l, r, typel, typer));
}

/*****************************************************************************/

/**
 * Determine the 3D hypotenuse.
 *
 * If required, x, y, and z are swapped to make x the larger number. The
 * traditional formula of x^2+y^2+z^2 is rearranged to factor x outside the
 * sqrt. This allows computation of the hypotenuse for significantly
 * larger values, and with a higher precision than when using the naive
 * formula. In particular, this cannot overflow unless the final result
 * would be out-of-range.
 * @code
 * sqrt( x^2 + y^2 + z^2 ) = sqrt( x^2( 1 + y^2/x^2 + z^2/x^2) )
 *                         = x * sqrt( 1 + y^2/x^2 + z^2/x^2)
 *                         = x * sqrt( 1 + y/x * y/x + z/x * z/x)
 * @endcode
 */
double
hypot3d(double x, double y, double z)
{
  double yx;
  double zx;
  double temp;

  /* Handle INF and NaN properly */
  if (isinf(x) || isinf(y) || isinf(z))
    return get_float8_infinity();

  if (isnan(x) || isnan(y) || isnan(z))
    return get_float8_nan();

  /* Else, drop any minus signs */
  x = fabs(x);
  y = fabs(y);
  z = fabs(z);

  /* Swap x, y and z if needed to make x the larger one */
  if (x < y)
  {
    temp = x;
    x = y;
    y = temp;
  }
  if (x < z)
  {
    temp = x;
    x = z;
    z = temp;
  }
  /*
   * If x is zero, the hypotenuse is computed with the 2D case.
   * This test saves a few cycles in such cases, but more importantly
   * it also protects against divide-by-zero errors, since now x >= y.
   */
  if (x == 0)
    return hypot(y, z);

  /* Determine the hypotenuse */
  yx = y / x;
  zx = z / x;
  return x * sqrt(1.0 + (yx * yx) + (zx * zx));
}

/**
 * Determine the 4D hypotenuse.
 *
 * @see The function is a generalization of the 3D case in function hypot3d
 */
double
hypot4d(double x, double y, double z, double m)
{
  double yx;
  double zx;
  double mx;
  double temp;

  /* Handle INF and NaN properly */
  if (isinf(x) || isinf(y) || isinf(z) || isinf(m))
    return get_float8_infinity();

  if (isnan(x) || isnan(y) || isnan(z) || isnan(m))
    return get_float8_nan();

  /* Else, drop any minus signs */
  x = fabs(x);
  y = fabs(y);
  z = fabs(z);
  m = fabs(m);

  /* Swap x, y, z, and m if needed to make x the larger one */
  if (x < y)
  {
    temp = x;
    x = y;
    y = temp;
  }
  if (x < z)
  {
    temp = x;
    x = z;
    z = temp;
  }
  if (x < m)
  {
    temp = x;
    x = m;
    m = temp;
  }
  /*
   * If x is zero, the hypotenuse is computed with the 3D case.
   * This test saves a few cycles in such cases, but more importantly
   * it also protects against divide-by-zero errors, since now x >= y.
   */
  if (x == 0)
    return hypot3d(y, z, m);

  /* Determine the hypotenuse */
  yx = y / x;
  zx = z / x;
  mx = m / x;
  return x * sqrt(1.0 + (yx * yx) + (zx * zx) + (mx * mx));
}

/*****************************************************************************/

