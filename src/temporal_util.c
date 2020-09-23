/*****************************************************************************
 *
 * temporal_util.c
 *	  Miscellaneous utility functions for temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "temporal_util.h"

#include <assert.h>
#include <catalog/pg_collation.h>
#include <utils/builtins.h>
#include <utils/lsyscache.h>
#include <utils/timestamp.h>
#include <utils/varlena.h>

#include "period.h"
#include "temporal.h"
#include "oidcache.h"
#include "doublen.h"

#ifdef WITH_POSTGIS
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"
#endif

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

/* Initialization of the extension */

void
_PG_init(void)
{
	/* elog(WARNING, "This is MobilityDB."); */
#ifdef WITH_POSTGIS
	temporalgeom_init();
#endif
}

/* Print messages while debugging */

void
debugstr(char *msg)
{
	ereport(WARNING, (errcode(ERRCODE_WARNING), errmsg("DEBUG: %s", msg)));
}

/* Align to double */

size_t
double_pad(size_t size)
{
	if (size % 8)
		return size + (8 - size % 8);
	else
		return size;
}

/* 
 * Is the type passed by value?
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
#ifdef WITH_POSTGIS
	else if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY) ||
			 type == type_oid(T_DOUBLE3) || type == type_oid(T_DOUBLE4))
		result = false;
#endif
	return result;
}

/* 
 * Get length of type
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
#ifdef WITH_POSTGIS
	else if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY))
		result = -1;
	else if (type == type_oid(T_DOUBLE3))
		result = 24;
	else if (type == type_oid(T_DOUBLE4))
		result = 32;
#endif
	return result;
}

/* Copy a Datum if it is passed by reference */

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

double
datum_double(Datum d, Oid valuetypid)
{
	double result = 0.0;
	ensure_numeric_base_type(valuetypid);
	if (valuetypid == INT4OID)
		result = (double)(DatumGetInt32(d));
	if (valuetypid == FLOAT8OID)
		result = DatumGetFloat8(d);
	return result;
}

/*****************************************************************************
 * Call PostgreSQL functions
 *****************************************************************************/

/* Call input function of the base type of a temporal type */

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

/* Call output function of the base type of a temporal type */

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

/* Call send function of the base type of a temporal type */

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

/* Call receive function of the base type of a temporal type */

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

/* Call PostgreSQL function with 1 to 4 arguments */

Datum
call_function1(PGFunction func, Datum arg1)
{
	FunctionCallInfoData fcinfo;
	FmgrInfo flinfo;
	memset(&flinfo, 0, sizeof(flinfo)) ;
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

Datum
call_function4(PGFunction func, Datum arg1, Datum arg2, Datum arg3, Datum arg4)
{
	FunctionCallInfoData fcinfo;
	FmgrInfo flinfo;
	memset(&flinfo, 0, sizeof(flinfo)) ;
	flinfo.fn_mcxt = CurrentMemoryContext;
	Datum result;
	InitFunctionCallInfoData(fcinfo, &flinfo, 4, DEFAULT_COLLATION_OID, NULL, NULL);
	fcinfo.arg[0] = arg1;
	fcinfo.argnull[0] = false;
	fcinfo.arg[1] = arg2;
	fcinfo.argnull[1] = false;
	fcinfo.arg[2] = arg3;
	fcinfo.argnull[2] = false;
	fcinfo.arg[3] = arg4;
	fcinfo.argnull[3] = false;
	result = (*func) (&fcinfo);
	if (fcinfo.isnull)
		elog(ERROR, "function %p returned NULL", (void *) func);
	return result;
}

/*****************************************************************************/

/* CallerFInfoFunctionCall 1 to 3 are provided by PostGIS */

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


/*****************************************************************************
 * Array functions
 *****************************************************************************/

/* Extract a C array from a PostgreSQL array */

Datum *
datumarr_extract(ArrayType *array, int *count)
{
	bool byval;
	int16 typlen;
	char align;
	get_typlenbyvalalign(array->elemtype, &typlen, &byval, &align);
	Datum *result;
	deconstruct_array(array, array->elemtype, typlen, byval, align, &result, NULL, count);
	return result;
}

TimestampTz *
timestamparr_extract(ArrayType *array, int *count)
{
	return (TimestampTz *) datumarr_extract(array, count);
}

Period **
periodarr_extract(ArrayType *array, int *count)
{
	return (Period **) datumarr_extract(array, count);
}

RangeType **
rangearr_extract(ArrayType *array, int *count)
{
	return (RangeType **) datumarr_extract(array, count);
}

Temporal **
temporalarr_extract(ArrayType *array, int *count)
{
	Temporal **result;
	deconstruct_array(array, array->elemtype, -1, false, 'd', (Datum **) &result, NULL, count);
	return result;
}

/*****************************************************************************/

/* Convert a C array into a PostgreSQL array */

ArrayType *
datumarr_to_array(Datum *values, int count, Oid type)
{
	int16 elmlen;
	bool elmbyval;
	char elmalign;
	get_typlenbyvalalign(type, &elmlen, &elmbyval, &elmalign);
	ArrayType *result = construct_array(values, count, type, elmlen, elmbyval, elmalign);
	return result;
}

ArrayType *
timestamparr_to_array(TimestampTz *times, int count)
{
	ArrayType *result = construct_array((Datum *)times, count, TIMESTAMPTZOID, 8, true, 'd');
	return result;
}

ArrayType *
periodarr_to_array(Period **periods, int count)
{
	ArrayType *result = construct_array((Datum *)periods, count, type_oid(T_PERIOD),
		sizeof(Period), false, 'd');
	return result;
}

ArrayType *
rangearr_to_array(RangeType **ranges, int count, Oid type)
{
	ArrayType *result = construct_array((Datum *)ranges, count, type, -1, false, 'd');
	return result;
}

ArrayType *
textarr_to_array(text **textarr, int count)
{
	ArrayType *result = construct_array((Datum *)textarr, count, TEXTOID, -1, false, 'i');
	return result;
}

ArrayType *
temporalarr_to_array(Temporal **temporalarr, int count)
{
	assert(count > 0);
	Oid type = temporal_oid_from_base(temporalarr[0]->valuetypid);
	ArrayType *result = construct_array((Datum *)temporalarr, count, type, -1, false, 'd');
	return result;
}

/*****************************************************************************
 * Sort functions 
 *****************************************************************************/

/* Comparator functions */

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

static int
timestamp_sort_cmp(const TimestampTz *l, const TimestampTz *r)
{
	TimestampTz x = *l;
	TimestampTz y = *r;
	return timestamp_cmp_internal(x, y);
}

static int
period_sort_cmp(Period **l, Period **r)
{
	return period_cmp_internal(*l, *r);
}

static int
range_sort_cmp(RangeType **l, RangeType **r)
{
	return DatumGetInt32(call_function2(range_cmp, RangeTypePGetDatum(*l),
										RangeTypePGetDatum(*r)));
}

static int
temporalinstarr_sort_cmp(TemporalInst **l, TemporalInst **r)
{
	return timestamp_cmp_internal((*l)->t, (*r)->t);
}

static int
temporalseqarr_sort_cmp(TemporalSeq **l, TemporalSeq **r)
{
	TimestampTz lt = (*l)->period.lower;
	TimestampTz rt = (*r)->period.lower;
	return timestamp_cmp_internal(lt, rt);
}

/*****************************************************************************/

/* Sort functions */

void
datum_sort(Datum *values, int count, Oid type)
{
	qsort_arg(values, (size_t) count, sizeof(Datum),
			  (qsort_arg_comparator) &datum_sort_cmp, &type);
}

void
timestamp_sort(TimestampTz *times, int count)
{
	qsort(times, (size_t) count, sizeof(TimestampTz),
		  (qsort_comparator) &timestamp_sort_cmp);
	qsort(times, (size_t) count, sizeof(TimestampTz),
		(qsort_comparator) &timestamp_sort_cmp);
}

void
periodarr_sort(Period **periods, int count)
{
	qsort(periods, (size_t) count, sizeof(Period *),
		  (qsort_comparator) &period_sort_cmp);
}

void
rangearr_sort(RangeType **ranges, int count)
{
	qsort(ranges, (size_t) count, sizeof(RangeType *),
		  (qsort_comparator) &range_sort_cmp);
}

void
temporalinstarr_sort(TemporalInst **instants, int count)
{
	qsort(instants, (size_t) count, sizeof(TemporalInst *),
		  (qsort_comparator) &temporalinstarr_sort_cmp);
}

void
temporalseqarr_sort(TemporalSeq **sequences, int count)
{
	qsort(sequences, (size_t) count, sizeof(TemporalSeq *),
		  (qsort_comparator) &temporalseqarr_sort_cmp);
}

/*****************************************************************************
 * Remove duplicate functions
 * These functions assume that the array has been sorted before 
 *****************************************************************************/

/* Remove duplicates from an array of datums */

int
datum_remove_duplicates(Datum *values, int count, Oid type)
{
	assert (count > 0);
	int newcount = 0;
	for (int i = 1; i < count; i++)
		if (datum_ne(values[newcount], values[i], type))
			values[++ newcount] = values[i];
	return newcount + 1;
}

/* Remove duplicates from an array of timestamps */

int
timestamp_remove_duplicates(TimestampTz *values, int count)
{
	assert (count > 0);
	int newcount = 0;
	for (int i = 1; i < count; i++)
		if (values[newcount] != values[i])
			values[++ newcount] = values[i];
	return newcount + 1;
}

/*****************************************************************************
 * Text functions
 * Function copied from PostgreSQL since they are not exported
 *****************************************************************************/

int
text_cmp(text *arg1, text *arg2, Oid collid)
{
	char	*a1p,
			*a2p;
	int		len1,
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

/*
* Version of the functions where the types of both arguments is equal
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
#ifdef WITH_POSTGIS
	else if (type == type_oid(T_GEOMETRY))
		//	result = DatumGetBool(call_function2(lwgeom_eq, l, r));
		result = datum_point_eq(l, r);
	else if (type == type_oid(T_GEOGRAPHY))
		//	result = DatumGetBool(call_function2(geography_eq, l, r));
		result = datum_point_eq(l, r);
#endif
	return result;
}

bool
datum_ne(Datum l, Datum r, Oid type)
{
	return !datum_eq(l, r, type);
}

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
#ifdef WITH_POSTGIS
	else if (type == type_oid(T_GEOMETRY))
		result = DatumGetBool(call_function2(lwgeom_lt, l, r));
	else if (type == type_oid(T_GEOGRAPHY))
		result = DatumGetBool(call_function2(geography_lt, l, r));
#endif
	return result;
}

bool
datum_le(Datum l, Datum r, Oid type)
{
	return datum_eq(l, r, type) || datum_lt(l, r, type);
}

bool
datum_gt(Datum l, Datum r, Oid type)
{
	return datum_lt(r, l, type);
}

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
#ifdef WITH_POSTGIS
	else if (typel == type_oid(T_GEOMETRY) && typer == type_oid(T_GEOMETRY))
		//	result = DatumGetBool(call_function2(lwgeom_eq, l, r));
		result = datum_point_eq(l, r);
	else if (typel == type_oid(T_GEOGRAPHY) && typer == type_oid(T_GEOGRAPHY))
		//	result = DatumGetBool(call_function2(geography_eq, l, r));
		result = datum_point_eq(l, r);
#endif
	return result;
}

bool
datum_ne2(Datum l, Datum r, Oid typel, Oid typer)
{
	return !datum_eq2(l, r, typel, typer);
}

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

bool
datum_le2(Datum l, Datum r, Oid typel, Oid typer)
{
	return datum_eq2(l, r, typel, typer) || datum_lt2(l, r, typel, typer);
}

bool
datum_gt2(Datum l, Datum r, Oid typel, Oid typer)
{
	return datum_lt2(r, l, typer, typel);
}

bool
datum_ge2(Datum l, Datum r, Oid typel, Oid typer)
{
	return datum_eq2(l, r, typel, typer) || datum_gt2(l, r, typel, typer);
}

/*****************************************************************************/

Datum
datum2_eq2(Datum l, Datum r, Oid typel, Oid typer)
{
	return BoolGetDatum(datum_eq2(l, r, typel, typer));
}

Datum
datum2_ne2(Datum l, Datum r, Oid typel, Oid typer)
{
	return BoolGetDatum(datum_ne2(l, r, typel, typer));
}

Datum
datum2_lt2(Datum l, Datum r, Oid typel, Oid typer)
{
	return BoolGetDatum(datum_lt2(l, r, typel, typer));
}

Datum
datum2_le2(Datum l, Datum r, Oid typel, Oid typer)
{
	return BoolGetDatum(datum_le2(l, r, typel, typer));
}

Datum
datum2_gt2(Datum l, Datum r, Oid typel, Oid typer)
{
	return BoolGetDatum(datum_gt2(l, r, typel, typer));
}

Datum
datum2_ge2(Datum l, Datum r, Oid typel, Oid typer)
{
	return BoolGetDatum(datum_ge2(l, r, typel, typer));
}

/*****************************************************************************/

