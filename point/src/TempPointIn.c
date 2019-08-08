/*****************************************************************************
 *
 * TempPointIn.c
 *	  Input of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "TemporalPoint.h"
#include "executor/spi.h"
#include <json-c/json.h>
#include <json-c/json_object_private.h>

/*****************************************************************************
 * Input in MFJSON format 
 *****************************************************************************/

/* 
 * We don't include <utils/builtins.h> to avoid collisions with json-c/json.h 
 *
 * This function is taken from PostGIS file lwgeom_in_geojson.c
 */
static char*
text2cstring(const text *textptr)
{
	size_t size = VARSIZE_ANY_EXHDR(textptr);
	char *str = lwalloc(size+1);
	memcpy(str, VARDATA(textptr), size);
	str[size]='\0';
	return str;
}

/*
 * Retrieve an SRID from a given SRS
 * Require valid spatial_ref_sys table entry
 *
 * This function is taken from PostGIS file lwgeom_export.c
 */
int
getSRIDbySRS(const char *srs)
{
	char query[256];
	int32_t srid, err;

	if (!srs) return 0;

	if (SPI_OK_CONNECT != SPI_connect ())
	{
		elog(NOTICE, "getSRIDbySRS: could not connect to SPI manager");
		SPI_finish();
		return 0;
	}
	sprintf(query,
		"SELECT srid "
		"FROM spatial_ref_sys, "
		"regexp_matches('%s', E'([a-z]+):([0-9]+)', 'gi') AS re "
		"WHERE re[1] ILIKE auth_name AND int4(re[2]) = auth_srid", srs);

	err = SPI_exec(query, 1);
	if (err < 0)
	{
		elog(NOTICE, "getSRIDbySRS: error executing query %d", err);
		SPI_finish();
		return 0;
	}

	/* no entry in spatial_ref_sys */
	if (SPI_processed <= 0)
	{
		sprintf(query,
			"SELECT srid "
			"FROM spatial_ref_sys, "
			"regexp_matches('%s', E'urn:ogc:def:crs:([a-z]+):.*:([0-9]+)', 'gi') AS re "
			"WHERE re[1] ILIKE auth_name AND int4(re[2]) = auth_srid", srs);

		err = SPI_exec(query, 1);
		if (err < 0)
		{
			elog(NOTICE, "getSRIDbySRS: error executing query %d", err);
			SPI_finish();
			return 0;
		}

		if (SPI_processed <= 0) 
		{
			SPI_finish();
			return 0;
		}
	}

	srid = atoi(SPI_getvalue(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, 1));

	SPI_finish();

	return srid;
}

/* Function taken from PostGIS file lwin_geojson.c */

static json_object*
findMemberByName(json_object* poObj, const char* pszName )
{
	json_object* poTmp;
	json_object_iter it;

	poTmp = poObj;

	if (pszName == NULL || poObj == NULL)
		return NULL;

	it.key = NULL;
	it.val = NULL;
	it.entry = NULL;

	if (json_object_get_object(poTmp) != NULL)
	{
		if (json_object_get_object(poTmp)->head == NULL)
		{
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid MFJSON representation")));
			return NULL;
		}

		for (it.entry = json_object_get_object(poTmp)->head;
				( it.entry ?
				  ( it.key = (char*)it.entry->k,
					it.val = (json_object*)it.entry->v, it.entry) : 0);
				it.entry = it.entry->next)
		{
			if (strcasecmp((char *)it.key, pszName) == 0)
				return it.val;
		}
	}

	return NULL;
}

static bool
parse_mfjson_coord(json_object *poObj, bool *hasz, POINT3DZ *pt)
{
	if (json_type_array == json_object_get_type(poObj))
	{
		json_object *poObjCoord = NULL;
		const int nSize = json_object_array_length(poObj);
		if (nSize < 2)
		{
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Too few coordinates in MFJSON representation")));
			return false;
		}
		else if (nSize > 3)
		{
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Too many coordinates in MFJSON representation")));
			return false;
		}

		/* Read X coordinate */
		poObjCoord = json_object_array_get_idx(poObj, 0);
		pt->x = json_object_get_double(poObjCoord);

		/* Read Y coordinate */
		poObjCoord = json_object_array_get_idx(poObj, 1);
		pt->y = json_object_get_double(poObjCoord);

		if (nSize == 3)
		{
			/* Read Z coordinate */
			poObjCoord = json_object_array_get_idx(poObj, 2);
			pt->z = json_object_get_double(poObjCoord);
			*hasz = true;
		}
	}
	else
	{
		/* If it's not an array, just don't handle it */
		return false;
	}

	return true;
}

/*****************************************************************************/

static TemporalInst *
tpointinst_from_mfjson(json_object *poObj)
{
	/* The maximum length of a datetime is 32 characters, e.g.,
	   "2019-08-06T18:35:48.021455+02:30" */
	char datetime[33];
	POINT3DZ pt;
	bool hasz;
	Datum value;
	TimestampTz t = 0; 

	/* Get coordinates */
	json_object* coords = findMemberByName(poObj, "coordinates");
	if (coords != NULL)
	{
		parse_mfjson_coord(coords, &hasz, &pt);
		if (hasz)
			value = call_function3(LWGEOM_makepoint, Float8GetDatum(pt.x),
				Float8GetDatum(pt.y), Float8GetDatum(pt.z));
		else 
			value = call_function2(LWGEOM_makepoint, Float8GetDatum(pt.x),
				Float8GetDatum(pt.y));
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid MFJSON representation")));

	/* Get datetimes */
	json_object *poObjDate = findMemberByName(poObj, "datetimes");
	if (poObjDate != NULL)
	{
		const char *pszDate = json_object_get_string(poObjDate);
		if (pszDate)
		{
			strcpy(datetime, pszDate);
			/* Replace 'T' by ' ' before converting to timestamptz */
			datetime[10] = ' ';
			t = call_input(TIMESTAMPTZOID, datetime);
		}
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid MFJSON representation")));

	TemporalInst *result = temporalinst_make(value, t, type_oid(T_GEOMETRY));
	pfree(DatumGetPointer(value));
	return result;
}

static TemporalI *
tpointi_from_mfjson(json_object *poObj)
{
	/* The maximum length of a datetime is 32 characters, e.g.,
	   "2019-08-06T18:35:48.021455+02:30" */
	char datetime[33];
	POINT3DZ pt;
	bool hasz;
	Datum *datumarr;
	TimestampTz *times;
	int numpoints, numdates;

	/* Get coordinates */
	json_object *points = findMemberByName(poObj, "coordinates");
	if (json_object_get_type(points) == json_type_array)
	{
		numpoints = json_object_array_length(points);
		datumarr = palloc(sizeof(Datum) * numpoints);
		for (int i = 0; i < numpoints; ++i)
		{
			json_object* coords = NULL;
			coords = json_object_array_get_idx(points, i);
			parse_mfjson_coord(coords, &hasz, &pt);
			if (hasz)
				datumarr[i] = call_function3(LWGEOM_makepoint, Float8GetDatum(pt.x),
					Float8GetDatum(pt.y), Float8GetDatum(pt.z));
			else 
				datumarr[i] = call_function2(LWGEOM_makepoint, Float8GetDatum(pt.x),
					Float8GetDatum(pt.y));
		}
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid MFJSON representation")));

	/* Get datetimes */
	json_object *dates = findMemberByName(poObj, "datetimes");
	if (dates != NULL &&
		json_object_get_type(dates) == json_type_array)
	{
		numdates = json_object_array_length(dates);
		if (numpoints == numdates)
		{
			times = palloc(sizeof(TimestampTz) * numdates);
			for (int i = 0; i < numdates; ++i)
			{
				json_object* datevalue = NULL;
				datevalue = json_object_array_get_idx(dates, i);
				const char *pszDate = json_object_get_string(datevalue);
				if (pszDate)
				{
					strcpy(datetime, pszDate);
					/* Replace 'T' by ' ' before converting to timestamptz */
					datetime[10] = ' ';
					times[i] = call_input(TIMESTAMPTZOID, datetime);
				}
			}
		}
		else
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid MFJSON representation")));
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid MFJSON representation")));


	return NULL;
}

static TemporalSeq *
tpointseq_from_mfjson(json_object *poObj)
{
	return NULL;
}

static TemporalS *
tpoints_from_mfjson(json_object *poObj)
{
	return NULL;
}

PG_FUNCTION_INFO_V1(tpoint_from_mfjson);

PGDLLEXPORT Datum
tpoint_from_mfjson(PG_FUNCTION_ARGS)
{
	Temporal *temp;
	text *mfjson_input;
	char *mfjson;
	char *srs = NULL;

	/* Get the mfjson stream */
	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	mfjson_input = PG_GETARG_TEXT_P(0);
	mfjson = text2cstring(mfjson_input);

	json_tokener *jstok = NULL;
	json_object *poObj = NULL;
	json_object *poObjType = NULL;
	json_object *poObjInterp = NULL;
	json_object *poObjInterp1 = NULL;
	json_object *poObjDates = NULL;
	json_object *poObjSrs = NULL;

	/* Begin to parse json */
	jstok = json_tokener_new();
	poObj = json_tokener_parse_ex(jstok, mfjson, -1);
	if (jstok->err != json_tokener_success)
	{
		char err[256];
		snprintf(err, 256, "%s (at offset %d)", json_tokener_error_desc(jstok->err), jstok->char_offset);
		json_tokener_free(jstok);
		json_object_put(poObj);
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Error while processing MFJSON representation")));
	}
	json_tokener_free(jstok);
	
	/*
	 * Ensure that it is a moving point
	 */
	poObjType = findMemberByName(poObj, "type");
	if (poObjType != NULL)
	{
		const char *pszType = json_object_get_string(poObjType);
		if (strcmp(pszType, "MovingPoint") != 0)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid MFJSON representation")));
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid MFJSON representation")));

	/*
	 * Determine duration of temporal point and dispatch to the 
	 *  corresponding parse function 
	 */
	poObjInterp = findMemberByName(poObj, "interpolations");
	if (poObjInterp != NULL && 
		json_object_get_type(poObjInterp) == json_type_array)
	{
		const int nSize = json_object_array_length(poObjInterp);
		if (nSize != 1)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid MFJSON representation")));

		/* Read interpolation value */
		poObjInterp1 = json_object_array_get_idx(poObjInterp, 0);
		const char *pszInterp = json_object_get_string(poObjInterp1);
		if (pszInterp)
		{
			if (strcmp(pszInterp, "Discrete") == 0)
			{
				poObjDates = findMemberByName(poObj, "datetimes");
				if (poObjDates != NULL &&
					json_object_get_type(poObjDates) == json_type_array)
					temp = (Temporal *)tpointi_from_mfjson(poObj);
				else
					temp = (Temporal *)tpointinst_from_mfjson(poObj);
			}
			else if (strcmp(pszInterp, "Linear") == 0)
			{
				json_object *poObjSeqs = findMemberByName(poObj, "sequences");
				if (poObjSeqs != NULL)
					temp = (Temporal *)tpoints_from_mfjson(poObj);
				else
					temp = (Temporal *)tpointseq_from_mfjson(poObj);
			}
			else
				ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
					errmsg("Invalid MFJSON representation")));
		}
	}
	else
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid MFJSON representation")));

	/* Parse crs and set SRID of temporal point */
	poObjSrs = findMemberByName(poObj, "crs");
	if (poObjSrs != NULL)
	{
		json_object *poObjSrsType = findMemberByName(poObjSrs, "type");
		if (poObjSrsType != NULL)
		{
			json_object *poObjSrsProps = findMemberByName(poObjSrs, "properties");
			if (poObjSrsProps)
			{
				json_object *poNameURL = findMemberByName(poObjSrsProps, "name");
				if (poNameURL)
				{
					const char *pszName = json_object_get_string(poNameURL);
					if (pszName)
					{
						srs = palloc(strlen(pszName) + 1);
						strcpy(srs, pszName);
					}
				}
			}
		}
	}
	Temporal *result;
	if (srs)
	{
		result = tpoint_set_srid_internal(temp, getSRIDbySRS(srs));
		pfree(srs);
		pfree(temp);
	}
	else
		result = temp;

	if (result == NULL)
		PG_RETURN_NULL();
	PG_RETURN_POINTER(result);
}

/*****************************************************************************/
