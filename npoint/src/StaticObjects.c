/*****************************************************************************
 *
 * StaticObjects.c
 *	  Network-based static point/region
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalNPoint.h"

/*****************************************************************************
 * Input/Output functions for npoint
 *****************************************************************************/

/*
 * Input function.
 * Example of input:
 * 		(1, 0.5)
 */
PG_FUNCTION_INFO_V1(npoint_in);

PGDLLEXPORT Datum
npoint_in(PG_FUNCTION_ARGS)
{
	char *str = PG_GETARG_CSTRING(0);
	int64 rid;
	double pos;
	npoint *result;

	if (sscanf(str, "(%ld,%lf)", &rid, &pos) != 2)
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION),
			errmsg("invalid input syntax for npoint: \"%s\"",	str)));
	if (pos < 0 || pos > 1)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("the relative position must be a real number between 0 and 1")));

	result = (npoint *) palloc(sizeof(npoint));
	result->rid = rid;
	result->pos = pos;
	PG_RETURN_POINTER(result);
}

/* Output function */

PG_FUNCTION_INFO_V1(npoint_out);

PGDLLEXPORT Datum
npoint_out(PG_FUNCTION_ARGS)
{
	npoint	 *np = PG_GETARG_NPOINT(0);
	char	   *result;

	result = psprintf("(%ld,%g)", np->rid, np->pos);
	PG_RETURN_CSTRING(result);
}

/* Receive function */

PG_FUNCTION_INFO_V1(npoint_recv);

PGDLLEXPORT Datum
npoint_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	npoint	 *result;

	result = (npoint *) palloc(sizeof(npoint));
	result->rid = pq_getmsgint64(buf);
	result->pos = pq_getmsgfloat8(buf);
	PG_RETURN_POINTER(result);
}

/* Send function */

PG_FUNCTION_INFO_V1(npoint_send);

PGDLLEXPORT Datum
npoint_send(PG_FUNCTION_ARGS)
{
	npoint	*np = PG_GETARG_NPOINT(0);
	StringInfoData buf;

	pq_begintypsend(&buf);
	pq_sendint64(&buf, np->rid);
	pq_sendfloat8(&buf, np->pos);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/*****************************************************************************
 * Input/Output functions for nregion
 *****************************************************************************/

static int
nsegment_sort_cmp(nsegment *nseg1, nsegment *nseg2)
{
	if (nseg1->rid < nseg2->rid) return -1;
	else if (nseg1->rid > nseg2->rid) return 1;
	else if (nseg1->pos1 < nseg2->pos1) return -1;
	else if (nseg1->pos1 > nseg2->pos1) return 1;
	else return 0;
}

nregion *
nregion_from_nsegmentarr_internal(nsegment *nsegs, int count)
{
	qsort(nsegs, count, sizeof(nsegment), (qsort_comparator) &nsegment_sort_cmp);

	int k = 0;
	for (int i = 1; i < count; i++)
	{
		if (nsegs[i].rid == nsegs[k].rid && nsegs[i].pos1 <= nsegs[k].pos2)
			nsegs[k].pos2 = Max(nsegs[k].pos2, nsegs[i].pos2);
		else
		{
			k++;
			nsegs[k].rid = nsegs[i].rid;
			nsegs[k].pos1 = nsegs[i].pos1;
			nsegs[k].pos2 = nsegs[i].pos2;
		}
	}

	size_t size = VARHDRSZ + sizeof(int32) + sizeof(nsegment) * (k + 1);
	nregion *result = (nregion *)palloc(size);
	SET_VARSIZE(result, size);
	result->count = k + 1;
	memcpy(result->nsegs, nsegs, sizeof(nsegment) * (k + 1));
	return result;
}

/* Input function
 * Example of input:
 * 		{(1, 0.2, 0.3), (2, 0.4, 0.6)}
 */

PG_FUNCTION_INFO_V1(nregion_in);

PGDLLEXPORT Datum
nregion_in(PG_FUNCTION_ARGS)
{
	char *input = PG_GETARG_CSTRING(0);
	nregion *result = nregion_parse(&input);
	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/* Output function */

static char *
nregion_to_string(nregion *nreg)
{
	char** strings = palloc((sizeof(char *)) * nreg->count);
	size_t outlen = 0;
	for (int i = 0; i < nreg->count; i++)
	{
		nsegment nseg = nreg->nsegs[i];
		strings[i] = psprintf("(%ld,%g,%g)", nseg.rid, nseg.pos1, nseg.pos2);
		outlen += strlen(strings[i]) + 2;
	}

	char *result = palloc(outlen + 3);
	result[outlen] = '\0';
	result[0] = '{';

	size_t pos = 1;
	for (int i = 0; i < nreg->count; i++)
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

PG_FUNCTION_INFO_V1(nregion_out);

PGDLLEXPORT Datum
nregion_out(PG_FUNCTION_ARGS)
{
	nregion *nreg = PG_GETARG_NREGION(0);
	char *result = nregion_to_string(nreg);
	PG_FREE_IF_COPY(nreg, 0);
	PG_RETURN_CSTRING(result);
}

/* Send function */

PG_FUNCTION_INFO_V1(nregion_send);

PGDLLEXPORT Datum
nregion_send(PG_FUNCTION_ARGS)
{
	nregion *nreg = PG_GETARG_NREGION(0);
	StringInfoData buf;
	pq_begintypsend(&buf);
	pq_sendint(&buf, nreg->count, 4);
	for (int i = 0; i < nreg->count; i++)
	{
		nsegment nseg = nreg->nsegs[i];
		pq_sendint64(&buf, nseg.rid);
		pq_sendfloat8(&buf, nseg.pos1);
		pq_sendfloat8(&buf, nseg.pos2);
	}
	PG_FREE_IF_COPY(nreg, 0);
	PG_RETURN_BYTEA_P(pq_endtypsend(&buf));
}

/* Receive function */

PG_FUNCTION_INFO_V1(nregion_recv);

PGDLLEXPORT Datum
nregion_recv(PG_FUNCTION_ARGS)
{
	StringInfo	buf = (StringInfo) PG_GETARG_POINTER(0);
	int count = (int) pq_getmsgint(buf, 4);
	size_t size = VARHDRSZ + sizeof(int32) + sizeof(nsegment) * count;
	nregion *result = (nregion *)palloc0(size);
	SET_VARSIZE(result, size);
	result->count = count;
	for (int i = 0; i < count; i++)
	{
		result->nsegs[i].rid = pq_getmsgint64(buf);
		result->nsegs[i].pos1 = pq_getmsgfloat8(buf);
		result->nsegs[i].pos2 = pq_getmsgfloat8(buf);
	}

	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Constructors
 *****************************************************************************/

npoint *
npoint_constructor_internal(int64 rid, double pos)
{
	if (pos < 0 || pos > 1)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("the relative position must be a real number between 0 and 1")));

	npoint *result = (npoint *)palloc(sizeof(npoint));
	result->rid = rid;
	result->pos = pos;
	return result;
}

PG_FUNCTION_INFO_V1(npoint_constructor);

PGDLLEXPORT Datum
npoint_constructor(PG_FUNCTION_ARGS)
{
	int64 rid = PG_GETARG_INT64(0);
	double pos = PG_GETARG_FLOAT8(1);
	npoint *result = npoint_constructor_internal(rid, pos);
	PG_RETURN_POINTER(result);
}

nregion *
nregion_from_segment_internal(int64 rid, double pos1, double pos2)
{
	if (pos1 < 0 || pos1 > 1 || pos2 < 0 || pos2 > 1)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("The relative position must be a real number between 0 and 1")));

	size_t size = VARHDRSZ + sizeof(int32) + sizeof(nsegment);
	nregion *result = (nregion *)palloc0(size);
	SET_VARSIZE(result, size);
	result->count = 1;
	result->nsegs[0].rid = rid;
	result->nsegs[0].pos1 = pos1;
	result->nsegs[0].pos2 = pos2;
	return result;
}

nregion *
nregion_from_nregionarr_internal(nregion **nregs, int count)
{
	int countsegs = 0;
	for (int i = 0; i < count; i++)
		countsegs += nregs[i]->count;

	nsegment *nsegs = palloc(sizeof(nsegment) * countsegs);
	int k = 0;
	for (int i = 0; i < count; i++)
	{
		memcpy(nsegs + k, nregs[i]->nsegs, sizeof(nsegment) * nregs[i]->count);
		k += nregs[i]->count;
	}

	nregion *result = nregion_from_nsegmentarr_internal(nsegs, countsegs);
	pfree(nsegs);
	return result;
}

PG_FUNCTION_INFO_V1(nregion_from_segment);

PGDLLEXPORT Datum
nregion_from_segment(PG_FUNCTION_ARGS)
{
	int64 rid = PG_GETARG_INT64(0);
	double pos1 = PG_GETARG_FLOAT8(1);
	double pos2 = PG_GETARG_FLOAT8(2);
	nregion *result = nregion_from_segment_internal(rid, pos1, pos2);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nregion_from_route);

PGDLLEXPORT Datum
nregion_from_route(PG_FUNCTION_ARGS)
{
	int64 rid = PG_GETARG_INT64(0);
	nregion *result = nregion_from_segment_internal(rid, 0, 1);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nregion_from_route_pos);

PGDLLEXPORT Datum
nregion_from_route_pos(PG_FUNCTION_ARGS)
{
	int64 rid = PG_GETARG_INT64(0);
	double pos = PG_GETARG_FLOAT8(1);
	nregion *result = nregion_from_segment_internal(rid, pos, pos);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nregion_from_npoint);

PGDLLEXPORT Datum
nregion_from_npoint(PG_FUNCTION_ARGS)
{
	npoint *np = PG_GETARG_NPOINT(0);
	nregion *result = nregion_from_segment_internal(np->rid, np->pos, np->pos);
	PG_FREE_IF_COPY(np, 0);
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(nregion_from_nregionarr);

PGDLLEXPORT Datum
nregion_from_nregionarr(PG_FUNCTION_ARGS)
{
	ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
	int count = ArrayGetNItems(ARR_NDIM(array), ARR_DIMS(array));
	if (count < 1)
	{
		PG_FREE_IF_COPY(array, 0);
		PG_RETURN_NULL();
	}
	nregion **nregs;
	deconstruct_array(array, array->elemtype, -1, false, 'd', 
		(Datum **) &nregs, NULL, &count);

	nregion *result = nregion_from_nregionarr_internal(nregs, count);
	PG_FREE_IF_COPY(array, 0);
	pfree(nregs);
	PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessing values
 *****************************************************************************/

PG_FUNCTION_INFO_V1(npoint_route);

PGDLLEXPORT Datum
npoint_route(PG_FUNCTION_ARGS)
{
	npoint *np = PG_GETARG_NPOINT(0);
	int64 rid = np->rid;
	PG_FREE_IF_COPY(np, 0);
	PG_RETURN_INT64(rid);
}

PG_FUNCTION_INFO_V1(npoint_pos);

PGDLLEXPORT Datum
npoint_pos(PG_FUNCTION_ARGS)
{
	npoint *np = PG_GETARG_NPOINT(0);
	double pos = np->pos;
	PG_FREE_IF_COPY(np, 0);
	PG_RETURN_FLOAT8(pos);
}

PG_FUNCTION_INFO_V1(nregion_segments);

PGDLLEXPORT Datum
nregion_segments(PG_FUNCTION_ARGS)
{
	FuncCallContext	 *funcctx;
	TupleDesc			tuple_desc;
	nregion			 *nreg = PG_GETARG_NREGION(0);

	if (SRF_IS_FIRSTCALL())
	{
		MemoryContext   oldcontext;
		funcctx = SRF_FIRSTCALL_INIT();
		oldcontext = MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
		funcctx->max_calls = (uint32_t)nreg->count;
		if (get_call_result_type(fcinfo, NULL, &tuple_desc) != TYPEFUNC_COMPOSITE)
			ereport(ERROR, (errcode(ERRCODE_FEATURE_NOT_SUPPORTED),
				errmsg("Function returning record called in context that cannot accept type record")));
		funcctx->tuple_desc = tuple_desc;
		MemoryContextSwitchTo(oldcontext);
	}

	funcctx = SRF_PERCALL_SETUP();
	tuple_desc = funcctx->tuple_desc;
	size_t call_cntr = funcctx->call_cntr;

	if (call_cntr < funcctx->max_calls)
	{
		/* OUT rid bigint,
		 * OUT pos1 double precision,
		 * OUT pos2 double precision
		 */
		bool *nulls = palloc(3 * sizeof(bool));
		nulls[0] = nulls[1] = nulls[2] = false;

		Datum *values = palloc(3 * sizeof(Datum));
		values[0] = Int64GetDatum(nreg->nsegs[call_cntr].rid);
		values[1] = Float8GetDatum(nreg->nsegs[call_cntr].pos1);
		values[2] = Float8GetDatum(nreg->nsegs[call_cntr].pos2);

		HeapTuple tuple = heap_form_tuple(tuple_desc, values, nulls);
		Datum result = HeapTupleGetDatum(tuple);
		SRF_RETURN_NEXT(funcctx, result);
	}
	else
		SRF_RETURN_DONE(funcctx);
}

/*****************************************************************************
 * Comparisons on route identifier
 *****************************************************************************/

bool
eq_npoint_npoint_internal(npoint *np1, npoint *np2)
{
	return np1->rid == np2->rid && np1->pos == np2->pos;
}

bool
lt_npoint_npoint(npoint *np1, npoint *np2)
{
	return np1->rid < np2->rid || (np1->rid == np2->rid && np1->pos < np2->pos);
}

/*****************************************************************************
 * Conversions between network and Euclidean space
 *****************************************************************************/

/* Access edge table to get the route length with corresponding rid */

double
route_length_with_rid(int64 rid)
{
	char		*sql = (char *)palloc(64 * sizeof(char));
	bool		 isNull = true;
	double	   result = 0;

	sprintf(sql, "SELECT length FROM ways WHERE gid = %ld", rid);
	SPI_connect();
	int ret = SPI_execute(sql, true, 1);
	uint64 proc = SPI_processed;
	if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
	{
		SPITupleTable *tuptable = SPI_tuptable;
		result = DatumGetFloat8(SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull));
	}
	SPI_finish();
	pfree(sql);

	if (isNull)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("cannot get the length for route %ld", rid)));

	return result;
}

/* Access edge table to get the route geometry with corresponding rid */

Datum
route_geom_with_rid(int64 rid)
{
	char		*sql = (char *)palloc(64 * sizeof(char));
	bool		 isNull = true;
	GSERIALIZED *result = NULL;

	sprintf(sql, "SELECT the_geom FROM ways WHERE gid = %ld", rid);
	SPI_connect();
	int ret = SPI_execute(sql, true, 1);
	uint64 proc = SPI_processed;
	if (ret > 0 && proc > 0 && SPI_tuptable != NULL)
	{
		SPITupleTable *tuptable = SPI_tuptable;
		Datum line = SPI_getbinval(tuptable->vals[0], tuptable->tupdesc, 1, &isNull);
		if (!isNull)
		{
			/* Must allocate this in upper executor context to keep it alive after SPI_finish() */
			GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(line);
			result = (GSERIALIZED *)SPI_palloc(gs->size);
			memcpy(result, gs, gs->size);
		}
	}
	SPI_finish();
	pfree(sql);

	if (isNull)
		ereport(ERROR, (errcode(ERRCODE_INTERNAL_ERROR),
			errmsg("cannot get the geometry for route %ld", rid)));

	PG_RETURN_POINTER(result);
}

/* npoint to geometry */

Datum
npoint_geom_internal(npoint *np)
{
	Datum line = route_geom_with_rid(np->rid);
	Datum result = call_function2(LWGEOM_line_interpolate_point, line, Float8GetDatum(np->pos));
	pfree(DatumGetPointer(line));
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(npoint_geom);

PGDLLEXPORT Datum
npoint_geom(PG_FUNCTION_ARGS)
{
	npoint *np = PG_GETARG_NPOINT(0);
	Datum result = npoint_geom_internal(np);
	PG_FREE_IF_COPY(np, 0);
	PG_RETURN_DATUM(result);
}

/* nregion to geometry */

Datum
nregion_geom_internal(nregion *nreg)
{
	Datum *sublines = palloc(sizeof(Datum) * nreg->count);

	for (int i = 0; i < nreg->count; i++)
	{
		nsegment nseg = nreg->nsegs[i];
		Datum line = route_geom_with_rid(nseg.rid);

		if (nseg.pos1 == 0 && nseg.pos2 == 1)
			sublines[i] = PointerGetDatum(gserialized_copy((GSERIALIZED *)PG_DETOAST_DATUM(line)));
		else if (nseg.pos1 == nseg.pos2)
			sublines[i] = call_function2(LWGEOM_line_interpolate_point, line, Float8GetDatum(nseg.pos1));
		else
			sublines[i] = call_function3(LWGEOM_line_substring, line,
											Float8GetDatum(nseg.pos1), Float8GetDatum(nseg.pos2));

		pfree(DatumGetPointer(line));
	}

	Datum result;
	ArrayType *array = datumarr_to_array(sublines, nreg->count, type_oid(T_GEOMETRY));
	result = call_function1(pgis_union_geometry_array, PointerGetDatum(array));
	pfree(sublines);
	PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(nregion_geom);

PGDLLEXPORT Datum
nregion_geom(PG_FUNCTION_ARGS)
{
	nregion *nreg = PG_GETARG_NREGION(0);
	Datum result = nregion_geom_internal(nreg);
	PG_FREE_IF_COPY(nreg, 0);
	PG_RETURN_DATUM(result);
}

/*****************************************************************************/
