/*****************************************************************************
 *
 * TemporalTypes.c
 *	  Generic functions for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalTypes.h"
#ifdef WITH_POSTGIS
#include "TemporalPoint.h"
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
int4_pad(size_t size)
{
	if (size % 4)
		return size + (4 - size % 4);
	else
		return size;
}

size_t
double_pad(size_t size)
{
	if (size % 8)
		return size + (8 - size % 8);
	else
		return size;
}

/* Is the base type continuous? */

bool
type_is_continuous(Oid type)
{
	if (type == FLOAT8OID || type == type_oid(T_DOUBLE2) || 
		type == type_oid(T_DOUBLE3)	 || type == type_oid(T_DOUBLE4))
		return true;
#ifdef WITH_POSTGIS
	if (type == type_oid(T_GEOGRAPHY) || type == type_oid(T_GEOMETRY)) 
		return true;
#endif
	return false;
}

/* 
 * Is the type passed by value?
 * This function is called only for the base types of the temporal types. 
 * To avoid a call of the slow function get_typbyval (which makes a 
 * lookup call), the known base types are explicitly enumerated.
 */

bool 
type_byval_fast(Oid type) 
{
	if (type == BOOLOID || type == INT4OID || type == FLOAT8OID || 
		type == TIMESTAMPOID || type == TIMESTAMPTZOID)
		return true;
	if (type == type_oid(T_DOUBLE2) || type == type_oid(T_DOUBLE3) || 
		type == type_oid(T_DOUBLE4) || type == TEXTOID)
		return false;
#ifdef WITH_POSTGIS
	if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY))
		return false;
#endif
	ereport(WARNING, (errcode(ERRCODE_WARNING), 
		errmsg("Using slow get_typbyval function for unknown data type")));
	return (get_typbyval(type));
}

/* 
 * Get length of type
 * This function is called only for the base types of the temporal types. 
 * To avoid a call of the slow function get_typlen (which makes a 
 * lookup call), the known base types are explicitly enumerated.
 */
 
int 
get_typlen_fast(Oid type) 
{
	if (type == BOOLOID)
		return 1;
	if (type == INT4OID)
		return 4;
	if (type == FLOAT8OID || type == TIMESTAMPOID || type == TIMESTAMPTZOID)
		return 8;
	if (type == type_oid(T_DOUBLE2))
		return 16;
	if (type == type_oid(T_DOUBLE3))
		return 24;
	if (type == type_oid(T_DOUBLE4))
		return 32;
	if (type == TEXTOID)
		return -1;
#ifdef WITH_POSTGIS
	if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY))
		return -1;
#endif
	ereport(WARNING, (errcode(ERRCODE_WARNING), 
		errmsg("Using slow get_typlen function for unknown data type")));
	return get_typlen(type);
}

/* Copy a Datum if it is passed by reference */

Datum
datum_copy(Datum value, Oid type)
{
	/* For types passed by value */
	if (type_byval_fast(type) )
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
	if (valuetypid == INT4OID)
		return (double)(DatumGetInt32(d));
	if (valuetypid == FLOAT8OID)
		return DatumGetFloat8(d);
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), 
		errmsg("Operation not supported")));
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
	InitFunctionCallInfoData(fcinfo, NULL, 1, InvalidOid, NULL, NULL);
	fcinfo.flinfo = &flinfo;
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
	InitFunctionCallInfoData(fcinfo, NULL, 2, DEFAULT_COLLATION_OID, NULL, NULL);
	fcinfo.flinfo = &flinfo;
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
   InitFunctionCallInfoData(fcinfo, NULL, 3, InvalidOid, NULL, NULL);
   fcinfo.flinfo = &flinfo;
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
   InitFunctionCallInfoData(fcinfo, NULL, 4, InvalidOid, NULL, NULL);
   fcinfo.flinfo = &flinfo;
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

#define NEXTVAL(x) ( (Temporal *)( (char*)(x) + DOUBLEALIGN( VARSIZE(x) ) ) )

Temporal **
temporalarr_extract(ArrayType *array, int *count)
{
	*count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (*count == 0)
		return NULL;
	Temporal **result = palloc(sizeof(Temporal *) * *count);
	result[0] = (Temporal *)ARR_DATA_PTR(array);
	for (int i = 1; i < *count; i++)
		result[i] = NEXTVAL(result[i-1]);
	return result;
}

Temporal **
temporalarr_extract_old(ArrayType *array, int *count)
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
	Oid type = temporal_oid_from_base(temporalarr[0]->valuetypid);
	ArrayType *result = construct_array((Datum *)temporalarr, count, type, -1, false, 'd');
	return result;
}

/*****************************************************************************
 * Sort functions 
 *****************************************************************************/

/* Comparator functions */
 
static int
datum_sort_cmp(Datum *l, Datum *r, Oid *type) 
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
timestamp_sort_cmp(TimestampTz *l, TimestampTz *r)
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
double2_sort_cmp(double2 **l, double2 **r) 
{
	return double2_cmp(*l, *r);
}

static int 
double3_sort_cmp(double3 **l, double3 **r) 
{
	return double3_cmp(*l, *r);
}

static int 
double4_sort_cmp(double4 **l, double4 **r) 
{
	return double4_cmp(*l, *r);
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
	qsort_arg(values, count, sizeof(Datum), 
		(qsort_arg_comparator) &datum_sort_cmp, &type);
}

void
timestamp_sort(TimestampTz *times, int count)
{
	qsort(times, count, sizeof(Timestamp), 
		(qsort_comparator) &timestamp_sort_cmp);
}

void
double2_sort(double2 **doubles, int count)
{
	qsort(doubles, count, sizeof(double2 *), 
		(qsort_comparator) &double2_sort_cmp);
}

void
double3_sort(double3 **triples, int count)
{
	qsort(triples, count, sizeof(double3 *), 
		(qsort_comparator) &double3_sort_cmp);
}

void
double4_sort(double4 **quadruples, int count)
{
	qsort(quadruples, count, sizeof(double4 *), 
		(qsort_comparator) &double4_sort_cmp);
}

void
periodarr_sort(Period **periods, int count)
{
	qsort(periods, count, sizeof(Period *), 
		(qsort_comparator) &period_sort_cmp);
}

void
rangearr_sort(RangeType **ranges, int count)
{
	qsort(ranges, count, sizeof(RangeType *), 
		(qsort_comparator) &range_sort_cmp);
}

void
temporalinstarr_sort(TemporalInst **instants, int count)
{
	qsort(instants, count, sizeof(TemporalInst *), 
		(qsort_comparator) &temporalinstarr_sort_cmp);
}

void
temporalseqarr_sort(TemporalSeq **sequences, int count)
{
	qsort(sequences, count, sizeof(TemporalSeq *), 
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
	if (count == 0)
		return 0;
	int newcount = 0;
	for (int i = 1; i < count; i++) 
		if (datum_ne(values[newcount], values[i], type))
			values[++ newcount] = values[i];
	return newcount+1;
}

/* Remove duplicates from an array of timestamps */

int
timestamp_remove_duplicates(TimestampTz *values, int count)
{
	if (count == 0)
		return 0;
	int newcount = 0;
	for (int i = 1; i < count; i++) 
		if (values[newcount] != values[i])
			values[++ newcount] = values[i];
	return newcount+1;
}

/*****************************************************************************
 * Text functions
 * Function copied from PostgreSQL since they are not exported
 *****************************************************************************/

int
text_cmp(text *arg1, text *arg2, Oid collid)
{
	char	   *a1p,
			   *a2p;
	int			len1,
				len2;

	a1p = VARDATA_ANY(arg1);
	a2p = VARDATA_ANY(arg2);

	len1 = VARSIZE_ANY_EXHDR(arg1);
	len2 = VARSIZE_ANY_EXHDR(arg2);

	return varstr_cmp(a1p, len1, a2p, len2, collid);
}

text *
text_copy(text *t)
{
	/*
	 * VARSIZE is the total size of the struct in bytes.
	 */
	text	   *new_t = (text *) palloc(VARSIZE(t));

	SET_VARSIZE(new_t, VARSIZE(t));

	/*
	 * VARDATA is a pointer to the data region of the struct.
	 */
	memcpy((void *) VARDATA(new_t), /* destination */
		   (void *) VARDATA(t), /* source */
		   VARSIZE(t) - VARHDRSZ);	/* how many bytes */
	return new_t;
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
	if (type == 0)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Invalid Oid")));
	if (type == BOOLOID || type == INT4OID || type == FLOAT8OID || 
		type == TIMESTAMPOID || type == TIMESTAMPTZOID)
		return l == r;
	else if (type == TEXTOID) 
		return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) == 0;
	else if (type == type_oid(T_DOUBLE2)) 
		return double2_eq((double2 *)DatumGetPointer(l), (double2 *)DatumGetPointer(r));
	else if (type == type_oid(T_DOUBLE3)) 
		return double3_eq((double3 *)DatumGetPointer(l), (double3 *)DatumGetPointer(r));
	else if (type == type_oid(T_DOUBLE4)) 
		return double4_eq((double4 *)DatumGetPointer(l), (double4 *)DatumGetPointer(r));
#ifdef WITH_POSTGIS
	else if (type == type_oid(T_GEOMETRY) || type == type_oid(T_GEOGRAPHY))
	{
		GSERIALIZED *g1 = (GSERIALIZED *)DatumGetPointer(l);
		GSERIALIZED *g2 = (GSERIALIZED *)DatumGetPointer(r);
		if (VARSIZE(g1) == VARSIZE(g2) && !memcmp(g1, g2, VARSIZE(g1))) 
			return true;
		return false;
	}
#endif

	List *lst = list_make1(makeString("="));
	ereport(WARNING, (errcode(ERRCODE_WARNING), 
		errmsg("Using slow comparison for unknown data type")));
	RegProcedure oper = get_opcode(OpernameGetOprid(lst, type, type));
	pfree(lst);
	return DatumGetBool(OidFunctionCall2Coll(oper, DEFAULT_COLLATION_OID, l, r));
}

bool
datum_ne(Datum l, Datum r, Oid type)
{
	return !datum_eq(l, r, type);
}

bool
datum_lt(Datum l, Datum r, Oid type)
{
	if (type == BOOLOID)
		return DatumGetBool(l) < DatumGetBool(r);
	else if (type == INT4OID)
		return DatumGetInt32(l) < DatumGetInt32(r);
	else if (type == FLOAT8OID)
		return DatumGetFloat8(l) < DatumGetFloat8(r);
	else if (type == TEXTOID) 
		return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0;
#ifdef WITH_POSTGIS
	else if (type == type_oid(T_GEOMETRY))
		return DatumGetBool(call_function2(lwgeom_lt, l, r));
	else if (type == type_oid(T_GEOGRAPHY))
		return DatumGetBool(call_function2(geography_lt, l, r));
#endif
	
	/* All supported temporal types should have been considered before */
	List *lst = list_make1(makeString("<"));
	ereport(WARNING, (errcode(ERRCODE_WARNING), 
		errmsg("Using slow comparison for unknown data type")));
	RegProcedure oper = get_opcode(OpernameGetOprid(lst, type, type));
	pfree(lst);
	return DatumGetBool(OidFunctionCall2Coll(oper, DEFAULT_COLLATION_OID, l, r));
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
	if ((typel == BOOLOID && typer == BOOLOID) ||
		(typel == INT4OID && typer == INT4OID) ||
		(typel == FLOAT8OID && typer == FLOAT8OID))
		return l == r;
	else if (typel == INT4OID && typer == FLOAT8OID)
		return DatumGetInt32(l) == DatumGetFloat8(r);
	else if (typel == FLOAT8OID && typer == INT4OID)
		return DatumGetFloat8(l) == DatumGetInt32(r);
	else if (typel == TEXTOID && typer == TEXTOID) 
		return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) == 0;
	else if (typel == type_oid(T_DOUBLE2) && typer == type_oid(T_DOUBLE2)) 
		return double2_eq((double2 *)DatumGetPointer(l), (double2 *)DatumGetPointer(r));
	else if (typel == type_oid(T_DOUBLE3) && typer == type_oid(T_DOUBLE3)) 
		return double3_eq((double3 *)DatumGetPointer(l), (double3 *)DatumGetPointer(r));
	else if (typel == type_oid(T_DOUBLE4) && typer == type_oid(T_DOUBLE4)) 
		return double4_eq((double4 *)DatumGetPointer(l), (double4 *)DatumGetPointer(r));
#ifdef WITH_POSTGIS
	else if (typel == type_oid(T_GEOMETRY) && typer == type_oid(T_GEOMETRY))
		return DatumGetBool(call_function2(lwgeom_eq, l, r));	
	else if (typel == type_oid(T_GEOGRAPHY) && typer == type_oid(T_GEOGRAPHY)) 
		return DatumGetBool(call_function2(geography_eq, l, r));
#endif

	List *lst = list_make1(makeString("="));
	ereport(WARNING, (errcode(ERRCODE_WARNING), 
		errmsg("Using slow comparison for unknown data type")));
	RegProcedure oper = get_opcode(OpernameGetOprid(lst, typel, typer));
	pfree(lst);
	return DatumGetBool(OidFunctionCall2Coll(oper, DEFAULT_COLLATION_OID, l, r));
}

bool
datum_ne2(Datum l, Datum r, Oid typel, Oid typer)
{
	return !datum_eq2(l, r, typel, typer);
}

bool
datum_lt2(Datum l, Datum r, Oid typel, Oid typer)
{
	if (typel == BOOLOID && typer == BOOLOID)
		return DatumGetBool(l) < DatumGetBool(r);
	else if (typel == INT4OID && typer == INT4OID)
		return DatumGetInt32(l) < DatumGetInt32(r);
	else if (typel == INT4OID && typer == FLOAT8OID)
		return DatumGetInt32(l) < DatumGetFloat8(r);
	else if (typel == FLOAT8OID && typer == INT4OID)
		return DatumGetFloat8(l) < DatumGetInt32(r);
	else if (typel == FLOAT8OID && typer == FLOAT8OID)
		return DatumGetFloat8(l) < DatumGetFloat8(r);
	else if (typel == TEXTOID && typer == TEXTOID) 
		return text_cmp(DatumGetTextP(l), DatumGetTextP(r), DEFAULT_COLLATION_OID) < 0;
#ifdef WITH_POSTGIS
	else if (typel == type_oid(T_GEOMETRY) && typer == type_oid(T_GEOMETRY))
		return DatumGetBool(call_function2(lwgeom_lt, l, r));	
	else if (typel == type_oid(T_GEOGRAPHY) && typer == type_oid(T_GEOGRAPHY)) 
		return DatumGetBool(call_function2(geography_lt, l, r));
#endif
	
	List *lst = list_make1(makeString("<")); 
	ereport(WARNING, (errcode(ERRCODE_WARNING), 
		errmsg("Using slow comparison for unknown data type")));
	RegProcedure oper = get_opcode(OpernameGetOprid(lst, typel, typer));
	pfree(lst);
	return DatumGetBool(OidFunctionCall2Coll(oper, DEFAULT_COLLATION_OID, l, r));
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

/*****************************************************************************
 * Oid functions
 *****************************************************************************/
/* 
 * Obtain the Oid of the range type or the temporal type from the Oid of 
 * the base type 
 */

Oid
range_oid_from_base(Oid valuetypid)
{
	if (valuetypid == INT4OID)
		return type_oid(T_INTRANGE);
	else if (valuetypid == FLOAT8OID)
		return type_oid(T_FLOATRANGE);
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Invalid Oid")));
}

Oid
temporal_oid_from_base(Oid valuetypid)
{
	if (valuetypid == BOOLOID) return type_oid(T_TBOOL);
	if (valuetypid == INT4OID) return type_oid(T_TINT);
	if (valuetypid == FLOAT8OID) return type_oid(T_TFLOAT);
	if (valuetypid == TEXTOID) return type_oid(T_TTEXT);
	if (valuetypid == type_oid(T_DOUBLE2)) return type_oid(T_TDOUBLE2);
	if (valuetypid == type_oid(T_DOUBLE3)) return type_oid(T_TDOUBLE3);
	if (valuetypid == type_oid(T_DOUBLE4)) return type_oid(T_TDOUBLE4);
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY)) return type_oid(T_TGEOMPOINT);
	if (valuetypid == type_oid(T_GEOGRAPHY)) return type_oid(T_TGEOGPOINT);
#endif			
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Invalid Oid")));
}

/*****************************************************************************/
/* 
 * Obtain the Oid of the base type from the Oid of the range type or the 
 * temporal type  
 */

Oid
base_oid_from_range(Oid rangetypid)
{
	if (rangetypid == type_oid(T_INTRANGE)) return INT4OID;
	if (rangetypid == type_oid(T_FLOATRANGE)) return FLOAT8OID;		
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Invalid Oid")));
}

Oid
base_oid_from_temporal(Oid temptypid)
{
	if (temptypid == type_oid(T_TBOOL)) return BOOLOID;
	if (temptypid == type_oid(T_TINT)) return INT4OID;
	if (temptypid == type_oid(T_TFLOAT)) return FLOAT8OID;
	if (temptypid == type_oid(T_TTEXT)) return TEXTOID;
#ifdef WITH_POSTGIS
	if (temptypid == type_oid(T_TGEOMPOINT)) return type_oid(T_GEOMETRY);
	if (temptypid == type_oid(T_TGEOGPOINT)) return type_oid(T_GEOGRAPHY);
#endif			
	ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR), errmsg("Invalid Oid")));
}

/*****************************************************************************/
/* 
 * Is the Oid a temporal type 
 */

bool
temporal_oid(Oid temptypid)
{
	if (temptypid == type_oid(T_TBOOL) ||
		temptypid == type_oid(T_TINT) ||
		temptypid == type_oid(T_TFLOAT) ||
		temptypid == type_oid(T_TTEXT) ||
		temptypid == type_oid(T_TDOUBLE2) ||
		temptypid == type_oid(T_TDOUBLE3) ||
		temptypid == type_oid(T_TDOUBLE4)
#ifdef WITH_POSTGIS
		|| temptypid == type_oid(T_TGEOMPOINT)
		|| temptypid == type_oid(T_TGEOGPOINT)
#endif
		)
		return true;
	return false;
}

/*****************************************************************************
 * Catalog functions
 *****************************************************************************/

/* Obtain the typinfo for the temporal type from the catalog */

void
temporal_typinfo(Oid temptypid, Oid* valuetypid) 
{
	Oid catalog = RelnameGetRelid("pg_temporal");
	Relation rel = heap_open(catalog, AccessShareLock);
	TupleDesc tupDesc = rel->rd_att;
	ScanKeyData scandata;
	ScanKeyInit(&scandata, 1, BTEqualStrategyNumber, F_OIDEQ, 
		ObjectIdGetDatum(temptypid));
	HeapScanDesc scan = heap_beginscan_catalog(rel, 1, &scandata);
	HeapTuple tuple = heap_getnext(scan, ForwardScanDirection);
	bool isnull = false;
	if (HeapTupleIsValid(tuple)) 
		*valuetypid = DatumGetObjectId(heap_getattr(tuple, 2, tupDesc, &isnull));
	heap_endscan(scan);
	heap_close(rel, AccessShareLock);
	if (! HeapTupleIsValid(tuple) || isnull) 
		elog(ERROR, "type %u is not a temporal type", temptypid);
}

/*****************************************************************************
 * Trajectory functions
 *****************************************************************************/

bool
type_has_precomputed_trajectory(Oid valuetypid) 
{
#ifdef WITH_POSTGIS
	if (valuetypid == type_oid(T_GEOMETRY) || 
		valuetypid == type_oid(T_GEOGRAPHY))
		return true;
#endif
	return false;
} 
 
/*****************************************************************************/

