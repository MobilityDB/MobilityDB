/*****************************************************************************
 *
 * tpoint_in.c
 *		Input of temporal points in WKT, EWKT and MF-JSON format
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

#include "tpoint_in.h"

#include <assert.h>
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

/**
 * Convert a text value into a C string
 *
 * @note We don't include <utils/builtins.h> to avoid collisions with json-c/json.h 
 * @note Function taken from PostGIS file lwgeom_in_geojson.c
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

/**
 * Returns the JSON member corresponding to the name
 *
 * @note Function taken from PostGIS file lwin_geojson.c
 */
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

/**
 * Returns a single point from its MF-JSON coordinates. In this case the
 * coordinate array is a single array of cordinations such as 
 * "coordinates":[1,1]
 */
static Datum
parse_mfjson_coord(json_object *poObj)
{
	if (json_type_array != json_object_get_type(poObj))
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of the 'coordinates' array in MFJSON string")));

	const int numcoord = json_object_array_length(poObj);
	if (numcoord < 2)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Too few elements in 'coordinates' values in MFJSON string")));
	else if (numcoord > 3)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Too many elements in 'coordinates' values in MFJSON string")));

	double x, y;
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
		double z = json_object_get_double(poObjCoord);
		result = call_function3(LWGEOM_makepoint, Float8GetDatum(x),
			Float8GetDatum(y), Float8GetDatum(z));
	}
	else 
		result = call_function2(LWGEOM_makepoint, Float8GetDatum(x),
			Float8GetDatum(y));
	return result;
}

/* TODO MAKE POSSIBLE TO CALL THIS FUNCTION */
/**
 * Returns an array of points from its MF-JSON coordinates. In this case the
 * coordinate array is an array of arrays of cordinates such as 
 * "coordinates":[[1,1],[2,2]]
 */
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

/**
 * Returns an array of timestamps from its MF-JSON datetimes values
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
		json_object* datevalue = NULL;
		datevalue = json_object_array_get_idx(datetimes, i);
		const char *strdatevalue = json_object_get_string(datevalue);
		if (strdatevalue)
		{
			char datetime[33];
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

/**
 * Returns a temporal instant point from its MF-JSON representation
 */
static TInstant *
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
	if (strdatetimes == NULL)
	{
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'datetimes' value in MFJSON string")));
		return NULL; /* make Codacy quiet */
	}
	strcpy(str, strdatetimes);
	/* Replace 'T' by ' ' before converting to timestamptz */
	str[10] = ' ';
	TimestampTz t = call_input(TIMESTAMPTZOID, str);
	TInstant *result = tinstant_make(value, t, type_oid(T_GEOMETRY));
	pfree(DatumGetPointer(value));
	return result;
}

/**
 * Returns array of temporal instant points from its MF-JSON representation
 */
static TInstant **
tpointinstarr_from_mfjson(json_object *mfjson, int *count)
{
	/* Get coordinates and datetimes */
	int numpoints, numdates;
	Datum *values = parse_mfjson_points(mfjson, &numpoints);
	TimestampTz *times = parse_mfjson_datetimes(mfjson, &numdates);
	if (numpoints != numdates)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Distinct number of elements in 'coordinates' and 'datetimes' arrays")));

	/* Construct the array of temporal instant points */
	TInstant **result = palloc(sizeof(TInstant *) * numpoints);
	for (int i = 0; i < numpoints; i++)
		result[i] = tinstant_make(values[i], times[i], type_oid(T_GEOMETRY));

	for (int i = 0; i < numpoints; i++)
		pfree(DatumGetPointer(values[i]));
	pfree(values); pfree(times);
	*count = numpoints;
	return result;
}

/**
 * Returns a temporal instant set point from its MF-JSON representation
 */
static TInstantSet *
tpointinstset_from_mfjson(json_object *mfjson)
{
	int count;
	TInstant **instants = tpointinstarr_from_mfjson(mfjson, &count);
	return tinstantset_make_free(instants, count);
}

/**
 * Returns a temporal sequence point from its MF-JSON representation
 */
static TSequence *
tpointseq_from_mfjson(json_object *mfjson, bool linear)
{
	/* Get the array of temporal instant points */
	int count;
	TInstant **instants = tpointinstarr_from_mfjson(mfjson, &count);

	/* Get lower bound flag */
	json_object *lowerinc = NULL;
	lowerinc = findMemberByName(mfjson, "lower_inc");
	if (lowerinc == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'lower_inc' in MFJSON string")));
	bool lower_inc = (bool) json_object_get_boolean(lowerinc);

	/* Get upper bound flag */
	json_object *upperinc = NULL;
	upperinc = findMemberByName(mfjson, "upper_inc");
	if (upperinc == NULL)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Unable to find 'upper_inc' in MFJSON string")));
	bool upper_inc = (bool) json_object_get_boolean(upperinc);

	/* Construct the temporal point */
	return tsequence_make_free(instants, count, lower_inc, upper_inc, 
		linear, NORMALIZE);
}

/**
 * Returns a temporal sequence set point from its MF-JSON representation
 */
static TSequenceSet *
tpointseqset_from_mfjson(json_object *mfjson, bool linear)
{
	json_object *seqs = NULL;
	seqs = findMemberByName(mfjson, "sequences");
	/* We don't need to test that seqs is NULL since to differentiate between
	 * a sequence and a sequence set we look for the "sequences" member and
	 * then call this function */
	if (json_object_get_type(seqs) != json_type_array)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid 'sequences' array in MFJSON string")));
	int numseqs = json_object_array_length(seqs);
	if (numseqs < 1)
		ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
			errmsg("Invalid value of 'sequences' array in MFJSON string")));

	/* Construct the temporal point */
	TSequence **sequences = palloc(sizeof(TSequence *) * numseqs);
	for (int i = 0; i < numseqs; i++)
	{
		json_object* seqvalue = NULL;
		seqvalue = json_object_array_get_idx(seqs, i);
		sequences[i] = tpointseq_from_mfjson(seqvalue, linear);
	}
	return tsequenceset_make_free(sequences, numseqs, NORMALIZE);
}

PG_FUNCTION_INFO_V1(tpoint_from_mfjson);
/**
 * Returns a temporal point from its MF-JSON representation
 */
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
			errmsg("Multiple 'interpolations' values in MFJSON string")));

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
				temp = (Temporal *)tpointinstset_from_mfjson(poObj);
			else
				temp = (Temporal *)tpointinst_from_mfjson(poObj);
		}
		else if (strcmp(pszInterp, "Stepwise") == 0)
		{
			json_object *poObjSeqs = findMemberByName(poObj, "sequences");
			if (poObjSeqs != NULL)
				temp = (Temporal *)tpointseqset_from_mfjson(poObj, false);
			else
				temp = (Temporal *)tpointseq_from_mfjson(poObj, false);
		}
		else if (strcmp(pszInterp, "Linear") == 0)
		{
			json_object *poObjSeqs = findMemberByName(poObj, "sequences");
			if (poObjSeqs != NULL)
				temp = (Temporal *)tpointseqset_from_mfjson(poObj, true);
			else
				temp = (Temporal *)tpointseq_from_mfjson(poObj, true);
		}
		else
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), 
				errmsg("Invalid 'interpolations' value in MFJSON string")));
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
 * Structure used for passing the parse state between the parsing functions.
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
 * Check that we are not about to read off the end of the WKB array
 */
static inline void 
wkb_parse_state_check(wkb_parse_state *s, size_t next)
{
	if ((s->pos + next) > (s->wkb + s->wkb_size))
		elog(ERROR, "WKB structure does not match expected size!");
}

/**
 * Read a byte and advance the parse state forward
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
 * Read 4-byte integer and advance the parse state forward
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
		for (int j = 0; j < WKB_INT_SIZE/2; j++)
		{
			uint8_t tmp = ((uint8_t*)(&i))[j];
			((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[WKB_INT_SIZE - j - 1];
			((uint8_t*)(&i))[WKB_INT_SIZE - j - 1] = tmp;
		}
	}
	s->pos += WKB_INT_SIZE;
	return i;
}

/**
 * Read an 8-byte double and advance the parse state forward
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
		for (int i = 0; i < WKB_DOUBLE_SIZE/2; i++)
		{
			uint8_t tmp = ((uint8_t*)(&d))[i];
			((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[WKB_DOUBLE_SIZE - i - 1];
			((uint8_t*)(&d))[WKB_DOUBLE_SIZE - i - 1] = tmp;
		}
	}
	s->pos += WKB_DOUBLE_SIZE;
	return d;
}

/**
 * Read an 8-byte timestamp and advance the parse state forward
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
		for (int i = 0; i < WKB_TIMESTAMP_SIZE/2; i++)
		{
			uint8_t tmp = ((uint8_t*)(&t))[i];
			((uint8_t*)(&t))[i] = ((uint8_t*)(&t))[WKB_TIMESTAMP_SIZE - i - 1];
			((uint8_t*)(&t))[WKB_TIMESTAMP_SIZE - i - 1] = tmp;
		}
	}
	s->pos += WKB_TIMESTAMP_SIZE;
	return (TimestampTz) t;
}

/**
 * Take in an unknown kind of WKB type number and ensure it comes out as an
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
		case WKB_INSTANT:
			s->duration = INSTANT;
			break;
		case WKB_INSTANTSET:
			s->duration = INSTANTSET;
			break;
		case WKB_SEQUENCE:
			s->duration = SEQUENCE;
			break;
		case WKB_SEQUENCESET:
			s->duration = SEQUENCESET;
			break;
		default: /* Error! */
			elog(ERROR, "Unknown WKB duration (%d)!", wkb_type);
			break;
	}
}

/**
 * Returns a point from its WKB representation. A WKB point has just a set of doubles, 
 * with the quantity depending on the dimension of the point.
 */
Datum
point_from_wkb_state(wkb_parse_state *s)
{
	double x, y, z;
	x = double_from_wkb_state(s);
	y = double_from_wkb_state(s);
	if (s->has_z)
		z = double_from_wkb_state(s);
	LWPOINT *point = s->has_z ? lwpoint_make3dz(s->srid, x, y, z) :
		lwpoint_make2d(s->srid, x, y);
	Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
	lwpoint_free(point);
	return result;
}

/**
 * Returns a temporal instant point from its WKB representation.
 *
 * It starts reading it just after the endian byte,
 * the type byte and the optional srid number.
 * Advance the parse state forward appropriately.
 */
static TInstant * 
tpointinst_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Does the data we want to read exist? */
	size_t size = (ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE;
	wkb_parse_state_check(s, size);
	/* Create the instant point */
	Datum value = point_from_wkb_state(s);
	TimestampTz t = timestamp_from_wkb_state(s);
	TInstant *result = tinstant_make(value, t, type_oid(T_GEOMETRY)); // ????
	pfree(DatumGetPointer(value));
	return result;
}

/**
 * Returns a temporal instant array from its WKB representation
 */
static TInstant **
tpointinstarr_from_wkb_state(wkb_parse_state *s, int count)
{
	TInstant **result = palloc(sizeof(TInstant *) * count);
	for (int i = 0; i < count; i++)
	{
		/* Parse the point and the timestamp to create the instant point */
		Datum value = point_from_wkb_state(s);
		TimestampTz t = timestamp_from_wkb_state(s);
		result[i] = tinstant_make(value, t, type_oid(T_GEOMETRY));
		pfree(DatumGetPointer(value));
	}
	return result;
}

/**
 * Returns a temporal instant set point from its WKB representation
 */
static TInstantSet * 
tpointinstset_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Get the number of instants */
	int count = integer_from_wkb_state(s);
	assert(count > 0);
	/* Does the data we want to read exist? */
	size_t size = count * ((ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE);
	wkb_parse_state_check(s, size);
	/* Parse the instants */
	TInstant **instants = tpointinstarr_from_wkb_state(s, count);
	return tinstantset_make_free(instants, count);
}

/**
 * Set the bound flags from their WKB representation
 */
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

/**
 * Returns a temporal sequence point from its WKB representation
 */
static TSequence * 
tpointseq_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Get the number of instants */
	int count = integer_from_wkb_state(s);
	assert(count > 0);
	/* Get the period bounds */
	uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
	bool lower_inc, upper_inc;
	tpoint_bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
	/* Does the data we want to read exist? */
	size_t size = count * ((ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE);
	wkb_parse_state_check(s, size);
	/* Parse the instants */
	TInstant **instants = tpointinstarr_from_wkb_state(s, count);
	return tsequence_make_free(instants, count, lower_inc, upper_inc,
		s->linear, NORMALIZE); 
}

/**
 * Returns a temporal sequence set point from its WKB representation
 */
static TSequenceSet * 
tpointseqset_from_wkb_state(wkb_parse_state *s)
{
	/* Count the dimensions. */
	uint32_t ndims = (s->has_z) ? 3 : 2;
	/* Get the number of sequences */
	int count = integer_from_wkb_state(s);
	assert(count > 0);
	/* Parse the sequences */
	TSequence **sequences = palloc(sizeof(TSequence *) * count);
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
		TInstant **instants = palloc(sizeof(TInstant *) * countinst);
		for (int j = 0; j < countinst; j++)
		{
			/* Parse the point and the timestamp to create the instant point */
			Datum value = point_from_wkb_state(s);
			TimestampTz t = timestamp_from_wkb_state(s);
			instants[j] = tinstant_make(value, t, type_oid(T_GEOMETRY));
			pfree(DatumGetPointer(value));
		}
		sequences[i] = tsequence_make_free(instants, countinst, lower_inc,
			upper_inc, s->linear, NORMALIZE); 
	}
	return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Returns a temporal point from its WKB representation
 */
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
	if (s->duration == INSTANT)
		return (Temporal *)tpointinst_from_wkb_state(s);
	else if (s->duration == INSTANTSET)
		return (Temporal *)tpointinstset_from_wkb_state(s);
	else if (s->duration == SEQUENCE)
		return (Temporal *)tpointseq_from_wkb_state(s);
	else /* s->duration == SEQUENCESET */
		return (Temporal *)tpointseqset_from_wkb_state(s);
	return NULL; /* make compiler quiet */
}

PG_FUNCTION_INFO_V1(tpoint_from_ewkb);
/**
 * Returns a temporal point from its EWKB representation
 */
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
