/*****************************************************************************
 *
 * tpoint_in.c
 *	  Input of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_in.h"

#include <float.h>
#include <json-c/json.h>

#include "temporaltypes.h"
#include "oidcache.h"
#include "temporal_util.h"
#include "postgis.h"
#include "tpoint.h"
#include "tpoint_spatialfuncs.h"

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
	char *str = palloc(size + 1);
	memcpy(str, VARDATA(textptr), size);
	str[size]='\0';
	return str;
}

/* Function taken from PostGIS file lwin_geojson.c */

static json_object *
findMemberByName(json_object *poObj, const char *pszName )
{
	json_object *poTmp = poObj;
	json_object_iter it;

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
			if (strcasecmp(it.key, pszName) == 0)
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

/* TODO MAKE POSSIBLE TO CALL THIS FUNCTION */
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
/* */

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
	int numpoints;
	values = parse_mfjson_points(mfjson, &numpoints);

	/* Get datetimes */
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
tpointseq_from_mfjson(json_object *mfjson, bool linear)
{
	Datum *values;

	/* Get coordinates */
	int numpoints;
	values = parse_mfjson_points(mfjson, &numpoints);

	/* Get datetimes */
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
	bool lower_inc = (bool) json_object_get_boolean(lowerinc);

	json_object *upperinc = NULL;
	upperinc = findMemberByName(mfjson, "upper_inc");
	if (upperinc == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'upper_inc' in MFJSON string")));
	bool upper_inc = (bool) json_object_get_boolean(upperinc);

	/* Construct the temporal point */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * numpoints);
	for (int i = 0; i < numpoints; i++)
		instants[i] = temporalinst_make(values[i], times[i], type_oid(T_GEOMETRY));
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, numpoints, 
		lower_inc, upper_inc, linear, true);

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
tpoints_from_mfjson(json_object *mfjson, bool linear)
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
	for (int i = 0; i < numseqs; i++)
	{
		Datum *values;
		TimestampTz *times;

		json_object* seqvalue = NULL;
		seqvalue = json_object_array_get_idx(seqs, i);

		/* Get coordinates */
		int numpoints;
		values = parse_mfjson_points(seqvalue, &numpoints);
		
		/* Get datetimes */
		int numdates;
		times = parse_mfjson_datetimes(seqvalue, &numdates);

		if (numpoints != numdates)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Distinct number of elements in 'coordinates' and 'datetimes'")));

		json_object *lowerinc = NULL;
		lowerinc = findMemberByName(seqvalue, "lower_inc");
		if (lowerinc == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'lower_inc' in MFJSON string")));
		bool lower_inc = (bool) json_object_get_boolean(lowerinc);

		json_object *upperinc = NULL;
		upperinc = findMemberByName(seqvalue, "upper_inc");
		if (upperinc == NULL)
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Unable to find 'upper_inc' in MFJSON string")));
		bool upper_inc = (bool) json_object_get_boolean(upperinc);

		/* Construct the temporal point */
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * numpoints);
		for (int j = 0; j < numpoints; j++)
			instants[j] = temporalinst_make(values[j], times[j], type_oid(T_GEOMETRY));
		sequences[i] = temporalseq_from_temporalinstarr(instants, numpoints, 
			lower_inc, upper_inc, linear, true);
		for (int j = 0; j < numpoints; j++)
		{
			pfree(instants[j]);
			pfree(DatumGetPointer(values[j]));
		}
		pfree(instants);
		pfree(values);
		pfree(times);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, numseqs, 
		linear, true);
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
		else if (strcmp(pszInterp, "Stepwise") == 0)
		{
			json_object *poObjSeqs = findMemberByName(poObj, "sequences");
			if (poObjSeqs != NULL)
				temp = (Temporal *)tpoints_from_mfjson(poObj, false);
			else
				temp = (Temporal *)tpointseq_from_mfjson(poObj, false);
		}
		else if (strcmp(pszInterp, "Linear") == 0)
		{
			json_object *poObjSeqs = findMemberByName(poObj, "sequences");
			if (poObjSeqs != NULL)
				temp = (Temporal *)tpoints_from_mfjson(poObj, true);
			else
				temp = (Temporal *)tpointseq_from_mfjson(poObj, true);
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

/*****************************************************************************
 * Input in EWKB format 
 *****************************************************************************/

/**
* Used for passing the parse state between the parsing functions.
*/
typedef struct
{
	const uint8_t *wkb;	/* Points to start of WKB */
	size_t wkb_size; 	/* Expected size of WKB */
	bool swap_bytes; 	/* Do an endian flip? */
	uint8_t duration;	/* Current duration we are handling */
	int32_t srid;		/* Current SRID we are handling */
	bool has_z; 		/* Z? */
	bool has_srid; 		/* SRID? */
	bool linear; 		/* Linear Interpolation? */
	const uint8_t *pos; /* Current parse position */
} wkb_parse_state;

/**********************************************************************/

/**
* Check that we are not about to read off the end of the WKB
* array.
*/
static inline void 
wkb_parse_state_check(wkb_parse_state *s, size_t next)
{
	if ((s->pos + next) > (s->wkb + s->wkb_size))
		elog(ERROR, "WKB structure does not match expected size!");
}

/**
* Byte
* Read a byte and advance the parse state forward.
*/
static char
byte_from_wkb_state(wkb_parse_state *s)
{
	char char_value = 0;
	wkb_parse_state_check(s, WKB_BYTE_SIZE);
	char_value = s->pos[0];
	s->pos += WKB_BYTE_SIZE;
	return char_value;
}

/**
* Int32
* Read 4-byte integer and advance the parse state forward.
*/
static uint32_t
integer_from_wkb_state(wkb_parse_state *s)
{
	uint32_t i = 0;
	wkb_parse_state_check(s, WKB_INT_SIZE);
	memcpy(&i, s->pos, WKB_INT_SIZE);
	/* Swap? Copy into a stack-allocated integer. */
	if (s->swap_bytes)
	{
		uint8_t tmp;
		for (int j = 0; j < WKB_INT_SIZE/2; j++)
		{
			tmp = ((uint8_t*)(&i))[j];
			((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[WKB_INT_SIZE - j - 1];
			((uint8_t*)(&i))[WKB_INT_SIZE - j - 1] = tmp;
		}
	}
	s->pos += WKB_INT_SIZE;
	return i;
}

/**
* Double
* Read an 8-byte double and advance the parse state forward.
*/
static double
double_from_wkb_state(wkb_parse_state *s)
{
	double d = 0;
	wkb_parse_state_check(s, WKB_DOUBLE_SIZE);
	memcpy(&d, s->pos, WKB_DOUBLE_SIZE);
	/* Swap? Copy into a stack-allocated integer. */
	if (s->swap_bytes)
	{
		uint8_t tmp;
		for (int i = 0; i < WKB_DOUBLE_SIZE/2; i++)
		{
			tmp = ((uint8_t*)(&d))[i];
			((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[WKB_DOUBLE_SIZE - i - 1];
			((uint8_t*)(&d))[WKB_DOUBLE_SIZE - i - 1] = tmp;
		}
	}
	s->pos += WKB_DOUBLE_SIZE;
	return d;
}

/**
* Double
* Read an 8-byte timestamp and advance the parse state forward.
*/
static TimestampTz
timestamp_from_wkb_state(wkb_parse_state *s)
{
	int64_t t = 0;
	wkb_parse_state_check(s, WKB_TIMESTAMP_SIZE);
	memcpy(&t, s->pos, WKB_TIMESTAMP_SIZE);
	/* Swap? Copy into a stack-allocated integer. */
	if (s->swap_bytes)
	{
		uint8_t tmp;
		for (int i = 0; i < WKB_TIMESTAMP_SIZE/2; i++)
		{
			tmp = ((uint8_t*)(&t))[i];
			((uint8_t*)(&t))[i] = ((uint8_t*)(&t))[WKB_TIMESTAMP_SIZE - i - 1];
			((uint8_t*)(&t))[WKB_TIMESTAMP_SIZE - i - 1] = tmp;
		}
	}
	s->pos += WKB_TIMESTAMP_SIZE;
	return (TimestampTz) t;
}

/**
* Take in an unknown kind of wkb type number and ensure it comes out as an
* extended WKB type number (with Z/SRID/LINEAR_INTERP flags masked onto the
* high bits).
*/
static void
tpoint_type_from_wkb_state(wkb_parse_state *s, uint8_t wkb_type)
{
	s->has_z = false;
	s->has_srid = false;
	/* If any of the higher bits are set, this is probably an extended type. */
	if (wkb_type & 0xF0)
	{
		if (wkb_type & WKB_ZFLAG) s->has_z = true;
		if (wkb_type & WKB_SRIDFLAG) s->has_srid = true;
		if (wkb_type & WKB_LINEAR_INTERP) s->linear = true;
	}
	/* Mask off the flags */
	wkb_type = wkb_type & (uint8_t) 0x0F;

	switch (wkb_type)
	{
		case WKB_TEMPORALINST:
			s->duration = TEMPORALINST;
			break;
		case WKB_TEMPORALI:
			s->duration = TEMPORALI;
			break;
		case WKB_TEMPORALSEQ:
			s->duration = TEMPORALSEQ;
			break;
		case WKB_TEMPORALS:
			s->duration = TEMPORALS;
			break;
		default: /* Error! */
			elog(ERROR, "Unknown WKB duration (%d)!", wkb_type);
			break;
	}
}

/**
* TemporalInst
* Read a WKB Temporal, starting just after the endian byte,
* type byte and optional srid number.
* Advance the parse state forward appropriately.
* WKB point has just a set of doubles, with the quantity depending on the
* dimension of the point.
*/
static TemporalInst * 
tpointinst_from_wkb_state(wkb_parse_state *s)
{
	double x, y, z;
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Does the data we want to read exist? */
	size_t size = (ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE;
	wkb_parse_state_check(s, size);
	/* Parse the coordinates and create the point */
	Datum value = 0;
	if (s->has_z)
	{
		x = double_from_wkb_state(s);
		y = double_from_wkb_state(s);
		z = double_from_wkb_state(s);
		value = call_function3(LWGEOM_makepoint, Float8GetDatum(x),
			Float8GetDatum(y), Float8GetDatum(z));

	}
	else 
	{
		x = double_from_wkb_state(s);
		y = double_from_wkb_state(s);
		value = call_function2(LWGEOM_makepoint, Float8GetDatum(x),
			Float8GetDatum(y));
	}
	TimestampTz t = timestamp_from_wkb_state(s);
	if (s->has_srid)
	{
		GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
		gserialized_set_srid(gs, s->srid);
	}
	TemporalInst *result = temporalinst_make(value, t, type_oid(T_GEOMETRY));
	pfree(DatumGetPointer(value));
	return result;
}

static TemporalI * 
tpointi_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Get the number of instants. */
	int count = integer_from_wkb_state(s);
	/* Does the data we want to read exist? */
	size_t size = count * ((ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE);
	wkb_parse_state_check(s, size);
	/* Parse the instants */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
	{
		double x, y, z;
		/* Parse the coordinates and create the point */
		Datum value = 0;
		if (s->has_z)
		{
			x = double_from_wkb_state(s);
			y = double_from_wkb_state(s);
			z = double_from_wkb_state(s);
			value = call_function3(LWGEOM_makepoint, Float8GetDatum(x),
				Float8GetDatum(y), Float8GetDatum(z));

		}
		else 
		{
			x = double_from_wkb_state(s);
			y = double_from_wkb_state(s);
			value = call_function2(LWGEOM_makepoint, Float8GetDatum(x),
				Float8GetDatum(y));
		}
		TimestampTz t = timestamp_from_wkb_state(s);
		if (s->has_srid)
		{
			GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
			gserialized_set_srid(gs, s->srid);
		}
		instants[i] = temporalinst_make(value, t, type_oid(T_GEOMETRY));
		pfree(DatumGetPointer(value));
	}
	TemporalI *result = temporali_from_temporalinstarr(instants, count); 
	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static void
tpoint_bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc, bool *upper_inc)
{
	if (wkb_bounds & WKB_LOWER_INC) 
		*lower_inc = true;
	else
		*lower_inc = false;
	if (wkb_bounds & WKB_UPPER_INC) 
		*upper_inc = true;
	else
		*upper_inc = false;
}

static TemporalSeq * 
tpointseq_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Get the number of instants. */
	int count = integer_from_wkb_state(s);
	/* Get the period bounds */
	uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
	bool lower_inc, upper_inc;
	tpoint_bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
	/* Does the data we want to read exist? */
	size_t size = count * ((ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE);
	wkb_parse_state_check(s, size);
	/* Parse the instants */
	TemporalInst **instants = palloc(sizeof(TemporalInst *) * count);
	for (int i = 0; i < count; i++)
	{
		double x, y, z;
		/* Parse the coordinates and create the point */
		Datum value = 0;
		if (s->has_z)
		{
			x = double_from_wkb_state(s);
			y = double_from_wkb_state(s);
			z = double_from_wkb_state(s);
			value = call_function3(LWGEOM_makepoint, Float8GetDatum(x),
				Float8GetDatum(y), Float8GetDatum(z));

		}
		else 
		{
			x = double_from_wkb_state(s);
			y = double_from_wkb_state(s);
			value = call_function2(LWGEOM_makepoint, Float8GetDatum(x),
				Float8GetDatum(y));
		}
		TimestampTz t = timestamp_from_wkb_state(s);
		if (s->has_srid)
		{
			GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
			gserialized_set_srid(gs, s->srid);
		}
		instants[i] = temporalinst_make(value, t, type_oid(T_GEOMETRY));
		pfree(DatumGetPointer(value));
	}
	TemporalSeq *result = temporalseq_from_temporalinstarr(instants, count, 
		lower_inc, upper_inc, s->linear, true); 
	for (int i = 0; i < count; i++)
		pfree(instants[i]);
	pfree(instants);
	return result;
}

static TemporalS * 
tpoints_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Get the number of sequences. */
	int count = integer_from_wkb_state(s);
	/* Parse the sequences */
	TemporalSeq **sequences = palloc(sizeof(TemporalSeq *) * count);
	for (int i = 0; i < count; i++)
	{
		/* Get the number of instants */
		int countinst = integer_from_wkb_state(s);
		/* Get the period bounds */
		uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
		bool lower_inc, upper_inc;
		tpoint_bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
		/* Does the data we want to read exist? */
		size_t size = countinst * ((ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE);
		wkb_parse_state_check(s, size);
		/* Parse the instants */
		TemporalInst **instants = palloc(sizeof(TemporalInst *) * countinst);
		for (int j = 0; j < countinst; j++)
		{
			double x, y, z;
			/* Parse the coordinates and create the point */
			Datum value = 0;
			if (s->has_z)
			{
				x = double_from_wkb_state(s);
				y = double_from_wkb_state(s);
				z = double_from_wkb_state(s);
				value = call_function3(LWGEOM_makepoint, Float8GetDatum(x),
					Float8GetDatum(y), Float8GetDatum(z));

			}
			else 
			{
				x = double_from_wkb_state(s);
				y = double_from_wkb_state(s);
				value = call_function2(LWGEOM_makepoint, Float8GetDatum(x),
					Float8GetDatum(y));
			}
			TimestampTz t = timestamp_from_wkb_state(s);
			if (s->has_srid)
			{
				GSERIALIZED *gs = (GSERIALIZED *)DatumGetPointer(value);
				gserialized_set_srid(gs, s->srid);
			}
			instants[j] = temporalinst_make(value, t, type_oid(T_GEOMETRY));
			pfree(DatumGetPointer(value));
		}
		sequences[i] = temporalseq_from_temporalinstarr(instants, countinst,
			lower_inc, upper_inc, s->linear, true); 
		for (int j = 0; j < countinst; j++)
			pfree(instants[j]);
		pfree(instants);
	}
	TemporalS *result = temporals_from_temporalseqarr(sequences, count, 
		s->linear, true); 
	for (int i = 0; i < count; i++)
		pfree(sequences[i]);
	pfree(sequences);
	return result;
}

Temporal *
tpoint_from_wkb_state(wkb_parse_state *s)
{
	/* Fail when handed incorrect starting byte */
	char wkb_little_endian = byte_from_wkb_state(s);
	if (wkb_little_endian != 1 && wkb_little_endian != 0)
		elog(ERROR, "Invalid endian flag value encountered.");

	/* Check the endianness of our input  */
	s->swap_bytes = false;
	if (getMachineEndian() == NDR)	/* Machine arch is little */
	{
		if (! wkb_little_endian)	/* Data is big! */
			s->swap_bytes = true;
	}
	else							/* Machine arch is big */
	{
		if (wkb_little_endian)		/* Data is little! */
			s->swap_bytes = true;
	}

	/* Read the temporal and interpolation flags */
	uint8_t wkb_type = (uint8_t) byte_from_wkb_state(s);
	tpoint_type_from_wkb_state(s, wkb_type);

	/* Read the SRID, if necessary */
	if (s->has_srid)
		s->srid = integer_from_wkb_state(s);

	ensure_valid_duration(s->duration);
	if (s->duration == TEMPORALINST)
		return (Temporal *)tpointinst_from_wkb_state(s);
	else if (s->duration == TEMPORALI)
		return (Temporal *)tpointi_from_wkb_state(s);
	else if (s->duration == TEMPORALSEQ)
		return (Temporal *)tpointseq_from_wkb_state(s);
	else if (s->duration == TEMPORALS)
		return (Temporal *)tpoints_from_wkb_state(s);
	return NULL; /* make compiler quiet */
}

PG_FUNCTION_INFO_V1(tpoint_from_ewkb);

PGDLLEXPORT Datum
tpoint_from_ewkb(PG_FUNCTION_ARGS)
{
	bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
	uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);

	/* Initialize the state appropriately */
	wkb_parse_state s;
	s.wkb = wkb;
	s.wkb_size = VARSIZE(bytea_wkb)-VARHDRSZ;
	s.swap_bytes = false;
	s.duration = 0;
	s.srid = SRID_UNKNOWN;
	s.has_z = false;
	s.has_srid = false;
	s.linear = false;
	s.pos = wkb;

	Temporal *temp = tpoint_from_wkb_state(&s);
	PG_FREE_IF_COPY(bytea_wkb, 0);
	PG_RETURN_POINTER(temp);
}

/*****************************************************************************/
