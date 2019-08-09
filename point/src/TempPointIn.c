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

#include <postgres.h>
#include <float.h>
#include <catalog/pg_type.h>
#include <executor/spi.h>
#include <json-c/json.h>
#include <json-c/json_object_private.h>
#include <utils/rangetypes.h>

#include "TemporalPoint.h"

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

static json_object *
findMemberByName(json_object *poObj, const char *pszName )
{
	json_object *poTmp;
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
				errmsg("Invalid MFJSON string")));
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

static Datum
parse_mfjson_coord(json_object *poObj)
{
	if (json_type_array != json_object_get_type(poObj))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid coordinate array in MFJSON string")));

	const int numcoord = json_object_array_length(poObj);
	if (numcoord < 2)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Too few coordinates in MFJSON string")));
	else if (numcoord > 3)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Too many coordinates in MFJSON string")));

	double x, y, z;
	Datum result;
	json_object *poObjCoord = NULL;

	/* Read X coordinate */
	poObjCoord = json_object_array_get_idx(poObj, 0);
	x = json_object_get_double(poObjCoord);

	/* Read Y coordinate */
	poObjCoord = json_object_array_get_idx(poObj, 1);
	y = json_object_get_double(poObjCoord);

	if (numcoord == 3)
	{
		/* Read Z coordinate */
		poObjCoord = json_object_array_get_idx(poObj, 2);
		z = json_object_get_double(poObjCoord);
		result = call_function3(LWGEOM_makepoint, Float8GetDatum(x),
			Float8GetDatum(y), Float8GetDatum(z));
	}
	else 
		result = call_function2(LWGEOM_makepoint, Float8GetDatum(x),
			Float8GetDatum(y));
	return result;
}

/* TODO MAKE POSSIBLE TO CALL THIS FUNCTION
static Datum *
parse_mfjson_points(json_object *mfjson, int *count)
{
	json_object *mfjsonTmp = mfjson;
	json_object *coordinates = NULL;
	coordinates = findMemberByName(mfjsonTmp, "coordinates");
	if (coordinates == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'coordinates' in MFJSON string")));
	if (json_object_get_type(coordinates) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'coordinates' array in MFJSON string")));

	int numpoints = json_object_array_length(coordinates);
	if (numpoints < 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of 'coordinates' array in MFJSON string")));
	
	Datum *values = palloc(sizeof(Datum) * numpoints);
	for (int i = 0; i < numpoints; ++i)
	{
		json_object *coords = NULL;
		coords = json_object_array_get_idx(coordinates, i);
		values[i] = parse_mfjson_coord(coords);
	}
	*count = numpoints;
	return values;
}
*/

static TimestampTz *
parse_mfjson_datetimes(json_object *mfjson, int *count)
{
	json_object *datetimes = NULL;
	datetimes = findMemberByName(mfjson, "datetimes");
	if (datetimes == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'datetimes' in MFJSON string")));
	if (json_object_get_type(datetimes) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'datetimes' array in MFJSON string")));

	int numdates = json_object_array_length(datetimes);
	if (numdates < 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of 'datetimes' array in MFJSON string")));
	
	TimestampTz *times = palloc(sizeof(TimestampTz) * numdates);
	for (int i = 0; i < numdates; i++)
	{
		char datetime[33];
		json_object* datevalue = NULL;
		datevalue = json_object_array_get_idx(datetimes, i);
		const char *strdatevalue = json_object_get_string(datevalue);
		if (strdatevalue)
		{
			strcpy(datetime, strdatevalue);
			/* Replace 'T' by ' ' before converting to timestamptz */
			datetime[10] = ' ';
			times[i] = call_input(TIMESTAMPTZOID, datetime);
		}
	}
	*count = numdates;
	return times;
}

/*****************************************************************************/

static TemporalInst *
tpointinst_from_mfjson(json_object *mfjson)
{
	/* Get coordinates */
	json_object *coordinates = findMemberByName(mfjson, "coordinates");
	if (coordinates == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'coordinates' in MFJSON string")));
	Datum value = parse_mfjson_coord(coordinates);

	/* Get datetimes 
	 * The maximum length of a datetime is 32 characters, e.g.,
	 *  "2019-08-06T18:35:48.021455+02:30" 
	 */
	char str[33];
	json_object *datetimes = findMemberByName(mfjson, "datetimes");
	if (datetimes == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'datetimes' in MFJSON string")));
	const char *strdatetimes = json_object_get_string(datetimes);
	if (!strdatetimes)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'datetimes' value in MFJSON string")));
	strcpy(str, strdatetimes);
	/* Replace 'T' by ' ' before converting to timestamptz */
	str[10] = ' ';
	TimestampTz t = call_input(TIMESTAMPTZOID, str);
	TemporalInst *result = temporalinst_make(value, t, type_oid(T_GEOMETRY));
	pfree(DatumGetPointer(value));
	return result;
}

static TemporalI *
tpointi_from_mfjson(json_object *mfjson)
{
	Datum *values;

	/* Get coordinates */
	// THIS FUNCTION CALL INVALIDATES THE NEXT FUNCTION CALL
	// int numpoints = parse_mfjson_points(mfjson, &values);
	/* */
	json_object *coordinates = NULL;
	coordinates = findMemberByName(mfjson, "coordinates");
	if (coordinates == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'coordinates' in MFJSON string")));
	if (json_object_get_type(coordinates) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'coordinates' array in MFJSON string")));

	int numpoints = json_object_array_length(coordinates);
	if (numpoints < 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of 'coordinates' array in MFJSON string")));
	
	values = palloc(sizeof(Datum) * numpoints);
	for (int i = 0; i < numpoints; ++i)
	{
		json_object *coords = NULL;
		coords = json_object_array_get_idx(coordinates, i);
		values[i] = parse_mfjson_coord(coords);
	}
	/* */
	/* Get datetimes */
	// FUNCTION CALL DOES NOT WORK
	int numdates;
	TimestampTz *times = parse_mfjson_datetimes(mfjson, &numdates);

	if (numpoints != numdates)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Distinct number of elements in 'coordinates' and 'datetimes' arrays")));

	/* Construct the temporal point */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * numpoints);
	for (int i = 0; i < numpoints; i++)
		instants[i] = temporalinst_make(values[i], times[i], type_oid(T_GEOMETRY));
	TemporalI *result = temporali_from_temporalinstarr(instants, numpoints);

	for (int i = 0; i < numpoints; i++)
	{
		pfree(instants[i]);
		pfree(DatumGetPointer(values[i]));
	}
	pfree(instants);
	return result;
}

static TemporalSeq *
tpointseq_from_mfjson(json_object *mfjson)
{
	Datum *values;

	/* Get coordinates */
	// THIS FUNCTION CALL INVALIDATES THE NEXT FUNCTION CALL
	// int numpoints = parse_mfjson_points(mfjson, &values);
	
	json_object *coordinates = NULL;
	coordinates = findMemberByName(mfjson, "coordinates");
	if (coordinates == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'coordinates' in MFJSON string")));
	if (json_object_get_type(coordinates) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'coordinates' array in MFJSON string")));

	int numpoints = json_object_array_length(coordinates);
	if (numpoints < 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of 'coordinates' array in MFJSON string")));
	
	values = palloc(sizeof(Datum) * numpoints);
	for (int i = 0; i < numpoints; ++i)
	{
		json_object *coords = NULL;
		coords = json_object_array_get_idx(coordinates, i);
		values[i] = parse_mfjson_coord(coords);
	}

	/* Get datetimes */
	// FUNCTION CALL DOES NOT WORK
	int numdates;
	TimestampTz *times = parse_mfjson_datetimes(mfjson, &numdates);

	if (numpoints != numdates)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Distinct number of elements in 'coordinates' and 'datetimes' arrays")));

	json_object *lowerinc = NULL;
	lowerinc = findMemberByName(mfjson, "lower_inc");
	if (lowerinc == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'lower_inc' in MFJSON string")));
	bool lower_inc = json_object_get_boolean(lowerinc);

	json_object *upperinc = NULL;
	upperinc = findMemberByName(mfjson, "upper_inc");
	if (upperinc == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'upper_inc' in MFJSON string")));
	bool upper_inc = json_object_get_boolean(upperinc);

	/* Construct the temporal point */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * numpoints);
	for (int i = 0; i < numpoints; i++)
		instants[i] = temporalinst_make(values[i], times[i], type_oid(T_GEOMETRY));
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, numpoints, 
		lower_inc, upper_inc, true);

	for (int i = 0; i < numpoints; i++)
	{
		pfree(instants[i]);
		pfree(DatumGetPointer(values[i]));
	}
	pfree(instants);
	pfree(values);
	pfree(times);
	return result;
}

static TemporalS *
tpoints_from_mfjson(json_object *mfjson)
{
	json_object *seqs = NULL;
	seqs = findMemberByName(mfjson, "sequences");
	if (seqs == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'sequences' in MFJSON string")));
	if (json_object_get_type(seqs) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'sequences' array in MFJSON string")));
	int numseqs = json_object_array_length(seqs);
	if (numseqs < 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of 'sequences' array in MFJSON string")));

	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * numseqs);
	for (int i = 0; i < numseqs; ++i)
	{
		Datum *values;
		TimestampTz *times;

		json_object* seqvalue = NULL;
		seqvalue = json_object_array_get_idx(seqs, i);

		/* Get coordinates */
		// THIS FUNCTION CALL INVALIDATES THE NEXT FUNCTION CALL
		// int numpoints = parse_mfjson_points(mfjson, &values);
		
		json_object *coordinates = NULL;
		coordinates = findMemberByName(seqvalue, "coordinates");
		if (coordinates == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'coordinates' in MFJSON string")));
		if (json_object_get_type(coordinates) != json_type_array)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid 'coordinates' array in MFJSON string")));

		int numpoints = json_object_array_length(coordinates);
		if (numpoints < 1)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid value of 'coordinates' array in MFJSON string")));
		
		values = palloc(sizeof(Datum) * numpoints);
		for (int j = 0; j < numpoints; ++j)
		{
			json_object* coords = NULL;
			coords = json_object_array_get_idx(coordinates, j);
			values[j] = parse_mfjson_coord(coords);
		}

		/* Get datetimes */
		// FUNCTION CALL DOES NOT WORK
		// int numdates = parse_mfjson_datetimes(sequence, &times);

		json_object *datetimes = NULL;
		datetimes = findMemberByName(seqvalue, "datetimes");
		if (datetimes == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'datetimes' in MFJSON string")));
		if (json_object_get_type(datetimes) != json_type_array)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid datetimes array in MFJSON string")));

		int numdates = json_object_array_length(datetimes);
		if (numdates < 1)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid datetimes array in MFJSON string")));
		
		times = palloc(sizeof(TimestampTz) * numdates);
		for (int j = 0; j < numdates; ++j)
		{
			char datetime[33];
			json_object* datevalue = NULL;
			datevalue = json_object_array_get_idx(datetimes, j);
			const char *strdatevalue = json_object_get_string(datevalue);
			if (strdatevalue)
			{
				strcpy(datetime, strdatevalue);
				/* Replace 'T' by ' ' before converting to timestamptz */
				datetime[10] = ' ';
				times[j] = call_input(TIMESTAMPTZOID, datetime);
			}
		}

		if (numpoints != numdates)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Distinct number of elements in 'coordinates' and 'datetimes'")));

		json_object *lowerinc = NULL;
		lowerinc = findMemberByName(seqvalue, "lower_inc");
		if (lowerinc == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'lower_inc' in MFJSON string")));
		bool lower_inc = json_object_get_boolean(lowerinc);

		json_object *upperinc = NULL;
		upperinc = findMemberByName(seqvalue, "upper_inc");
		if (upperinc == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'upper_inc' in MFJSON string")));
		bool upper_inc = json_object_get_boolean(upperinc);

		/* Construct the temporal point */
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * numpoints);
		for (int i = 0; i < numpoints; i++)
			instants[i] = temporalinst_make(values[i], times[i], type_oid(T_GEOMETRY));
		sequences[i] = temporalseq_from_temporalinstarr(instants, numpoints, 
			lower_inc, upper_inc, true);
		for (int i = 0; i < numpoints; i++)
		{
			pfree(instants[i]);
			pfree(DatumGetPointer(values[i]));
		}
		pfree(instants);
		pfree(values);
		pfree(times);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, numseqs, true);
	for (int i = 0; i < numseqs; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
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
			errmsg("Error while processing MFJSON string")));
	}
	json_tokener_free(jstok);
	
	/*
	 * Ensure that it is a moving point
	 */
	poObjType = findMemberByName(poObj, "type");
	if (poObjType == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'type' in MFJSON string")));

	const char *pszType = json_object_get_string(poObjType);
	if (strcmp(pszType, "MovingPoint") != 0)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'type' value in MFJSON string")));

	/*
	 * Determine duration of temporal point and dispatch to the 
	 *  corresponding parse function 
	 */
	poObjInterp = findMemberByName(poObj, "interpolations");
	if (poObjInterp == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'interpolations' in MFJSON string")));

	if (json_object_get_type(poObjInterp) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'interpolations' value in MFJSON string")));

	const int nSize = json_object_array_length(poObjInterp);
	if (nSize != 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'interpolations' value in MFJSON string")));

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
				errmsg("Invalid MFJSON string")));
	}

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
