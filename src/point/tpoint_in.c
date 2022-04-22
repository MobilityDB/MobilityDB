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
 * @file tpoint_in.c
 * @brief Input of temporal points in WKT, EWKT, WKB, EWKB, and MF-JSON format.
 */

#include "point/tpoint_in.h"

/* PostgreSQL */
#include <assert.h>
#include <float.h>
/* JSON-C */
#include <json-c/json.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "point/postgis.h"
#include "point/tpoint.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

/**
 * Return the JSON member corresponding to the name
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
          ( it.key = (char *) it.entry->k,
          it.val = (json_object *) it.entry->v, it.entry) : 0);
        it.entry = it.entry->next)
    {
      if (strcasecmp(it.key, pszName) == 0)
        return it.val;
    }
  }
  return NULL;
}

/**
 * Return a single point from its MF-JSON coordinates. In this case the
 * coordinate array is a single array of cordinations such as
 * "coordinates":[1,1]
 */
static Datum
parse_mfjson_coord(json_object *poObj, int srid, bool geodetic)
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
  json_object *poObjCoord = NULL;

  /* Read X coordinate */
  poObjCoord = json_object_array_get_idx(poObj, 0);
  x = json_object_get_double(poObjCoord);

  /* Read Y coordinate */
  poObjCoord = json_object_array_get_idx(poObj, 1);
  y = json_object_get_double(poObjCoord);

  LWPOINT *point;
  if (numcoord == 3)
  {
    /* Read Z coordinate */
    poObjCoord = json_object_array_get_idx(poObj, 2);
    double z = json_object_get_double(poObjCoord);
    point = lwpoint_make3dz(srid, x, y, z);
  }
  else
    point = lwpoint_make2d(srid, x, y);
  FLAGS_SET_GEODETIC(point->flags, geodetic);

  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
  lwpoint_free(point);
  return result;
}

/* TODO MAKE POSSIBLE TO CALL THIS FUNCTION */
/**
 * Return an array of points from its MF-JSON coordinates. In this case the
 * coordinate array is an array of arrays of cordinates such as
 * "coordinates":[[1,1],[2,2]]
 */
static Datum *
parse_mfjson_points(json_object *mfjson, int srid, bool geodetic,
  int *count)
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
    values[i] = parse_mfjson_coord(coords, srid, geodetic);
  }
  *count = numpoints;
  return values;
}

/**
 * Return an array of timestamps from its MF-JSON datetimes values
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
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its MF-JSON representation.
 */
TInstant *
tpointinst_from_mfjson(json_object *mfjson, int srid, CachedType temptype)
{
  bool geodetic = (temptype == T_TGEOGPOINT);
  /* Get coordinates */
  json_object *coordinates = findMemberByName(mfjson, "coordinates");
  if (coordinates == NULL)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Unable to find 'coordinates' in MFJSON string")));
  Datum value = parse_mfjson_coord(coordinates, srid, geodetic);

  /* Get datetimes
   * The maximum length of a datetime is 32 characters, e.g.,
   *  "2019-08-06T18:35:48.021455+02:30"
   */
  char str[33];
  json_object *datetimes = findMemberByName(mfjson, "datetimes");
  /* We don't need to test that datetimes is NULL since to differentiate
   * between an instant and a instant set we look for the "datetimes"
   * member and then call this function */
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
  TInstant *result = tinstant_make(value, t, temptype);
  pfree(DatumGetPointer(value));
  return result;
}

/**
 * Return array of temporal instant points from its MF-JSON representation
 */
static TInstant **
tpointinstarr_from_mfjson(json_object *mfjson, int srid, CachedType temptype,
  int *count)
{
  bool geodetic = (temptype == T_TGEOGPOINT);
  /* Get coordinates and datetimes */
  int numpoints, numdates;
  Datum *values = parse_mfjson_points(mfjson, srid, geodetic, &numpoints);
  TimestampTz *times = parse_mfjson_datetimes(mfjson, &numdates);
  if (numpoints != numdates)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Distinct number of elements in 'coordinates' and 'datetimes' arrays")));

  /* Construct the array of temporal instant points */
  TInstant **result = palloc(sizeof(TInstant *) * numpoints);
  for (int i = 0; i < numpoints; i++)
    result[i] = tinstant_make(values[i], times[i], temptype);

  for (int i = 0; i < numpoints; i++)
    pfree(DatumGetPointer(values[i]));
  pfree(values); pfree(times);
  *count = numpoints;
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its MF-JSON representation.
 */
TInstantSet *
tpointinstset_from_mfjson(json_object *mfjson, int srid, CachedType temptype)
{
  int count;
  TInstant **instants = tpointinstarr_from_mfjson(mfjson, srid, temptype,
    &count);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its MF-JSON representation.
 */
TSequence *
tpointseq_from_mfjson(json_object *mfjson, int srid, CachedType temptype,
  bool linear)
{
  /* Get the array of temporal instant points */
  int count;
  TInstant **instants = tpointinstarr_from_mfjson(mfjson, srid, temptype,
    &count);

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
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its MF-JSON representation.
 */
TSequenceSet *
tpointseqset_from_mfjson(json_object *mfjson, int srid, CachedType temptype,
  bool linear)
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
    sequences[i] = tpointseq_from_mfjson(seqvalue, srid, temptype, linear);
  }
  return tsequenceset_make_free(sequences, numseqs, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its MF-JSON representation
 */
Temporal *
tpoint_from_mfjson_ext(FunctionCallInfo fcinfo, text *mfjson_input,
  CachedType temptype)
{
  char *mfjson = text2cstring(mfjson_input);
  char *srs = NULL;
  int srid = 0;
  Temporal *result = NULL;

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
    snprintf(err, 256, "%s (at offset %d)",
      json_tokener_error_desc(jstok->err), jstok->char_offset);
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
   * Determine type of temporal point and call the corresponding parse function
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

  if (srs)
  {
    srid = getSRIDbySRS(fcinfo, srs);
    pfree(srs);
  }

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
        result = (Temporal *) tpointinstset_from_mfjson(poObj, srid, temptype);
      else
        result = (Temporal *) tpointinst_from_mfjson(poObj, srid, temptype);
    }
    else if (strcmp(pszInterp, "Stepwise") == 0 ||
      strcmp(pszInterp, "Linear") == 0)
    {
      bool linear = strcmp(pszInterp, "Linear") == 0;
      json_object *poObjSeqs = findMemberByName(poObj, "sequences");
      if (poObjSeqs != NULL)
        result = (Temporal *) tpointseqset_from_mfjson(poObj, srid, temptype, linear);
      else
        result = (Temporal *) tpointseq_from_mfjson(poObj, srid, temptype, linear);
    }
    else
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid 'interpolations' value in MFJSON string")));
  }

  return result;
}

/*****************************************************************************
 * Input in EWKB format
 * Please refer to the file tpoint_out.c where the binary format is explained
 *****************************************************************************/

/**
 * Structure used for passing the parse state between the parsing functions.
 */
typedef struct
{
  const uint8_t *wkb;  /**< Points to start of WKB */
  size_t wkb_size;     /**< Expected size of WKB */
  bool swap_bytes;     /**< Do an endian flip? */
  uint8_t subtype;     /**< Current subtype we are handling */
  int32_t srid;        /**< Current SRID we are handling */
  bool hasz;          /**< Z? */
  bool geodetic;    /**< Geodetic? */
  bool has_srid;       /**< SRID? */
  bool linear;         /**< Linear interpolation? */
  const uint8_t *pos;  /**< Current parse position */
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
 * extended WKB type number (with Z/GEOD/SRID/LINEAR_INTERP flags masked onto
 * the high bits).
 */
static void
tpoint_type_from_wkb_state(wkb_parse_state *s, uint8_t wkb_type)
{
  s->hasz = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_type & MOBDB_WKB_ZFLAG)
    s->hasz = true;
  if (wkb_type & MOBDB_WKB_GEODETICFLAG)
    s->geodetic = true;
  if (wkb_type & MOBDB_WKB_SRIDFLAG)
    s->has_srid = true;
  if (wkb_type & MOBDB_WKB_LINEAR_INTERP)
    s->linear = true;
  /* Mask off the upper flags to get the subtype */
  wkb_type = wkb_type & (uint8_t) 0x0F;

  switch (wkb_type)
  {
    case MOBDB_WKB_INSTANT:
      s->subtype = INSTANT;
      break;
    case MOBDB_WKB_INSTANTSET:
      s->subtype = INSTANTSET;
      break;
    case MOBDB_WKB_SEQUENCE:
      s->subtype = SEQUENCE;
      break;
    case MOBDB_WKB_SEQUENCESET:
      s->subtype = SEQUENCESET;
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB temporal type (%d)!", wkb_type);
      break;
  }
  return;
}

/**
 * Return a point from its WKB representation. A WKB point has just a set of doubles,
 * with the quantity depending on the dimension of the point.
 */
static Datum
point_from_wkb_state(wkb_parse_state *s)
{
  double x, y, z;
  x = double_from_wkb_state(s);
  y = double_from_wkb_state(s);
  if (s->hasz)
    z = double_from_wkb_state(s);
  LWPOINT *point = s->hasz ? lwpoint_make3dz(s->srid, x, y, z) :
    lwpoint_make2d(s->srid, x, y);
  FLAGS_SET_GEODETIC(point->flags, s->geodetic);
  Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
  lwpoint_free(point);
  return result;
}

/**
 * Return a temporal instant point from its WKB representation.
 *
 * It starts reading it just after the endian byte,
 * the type byte and the optional srid number.
 * Advance the parse state forward appropriately.
 */
static TInstant *
tpointinst_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions. */
  uint32_t ndims = (s->hasz) ? 3 : 2;
  /* Does the data we want to read exist? */
  size_t size = (ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE;
  wkb_parse_state_check(s, size);
  /* Create the instant point */
  Datum value = point_from_wkb_state(s);
  TimestampTz t = timestamp_from_wkb_state(s);
  CachedType temptype = (s->geodetic) ? T_TGEOGPOINT : T_TGEOMPOINT;
  TInstant *result = tinstant_make(value, t, temptype);
  pfree(DatumGetPointer(value));
  return result;
}

/**
 * Return a temporal instant array from its WKB representation
 */
static TInstant **
tpointinstarr_from_wkb_state(wkb_parse_state *s, int count)
{
  TInstant **result = palloc(sizeof(TInstant *) * count);
  CachedType temptype = (s->geodetic) ? T_TGEOGPOINT : T_TGEOMPOINT;
  for (int i = 0; i < count; i++)
  {
    /* Parse the point and the timestamp to create the instant point */
    Datum value = point_from_wkb_state(s);
    TimestampTz t = timestamp_from_wkb_state(s);
    result[i] = tinstant_make(value, t, temptype);
    pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * Return a temporal instant set point from its WKB representation
 */
static TInstantSet *
tpointinstset_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions */
  uint32_t ndims = (s->hasz) ? 3 : 2;
  /* Get the number of instants */
  int count = integer_from_wkb_state(s);
  assert(count > 0);
  /* Does the data we want to read exist? */
  size_t size = count * ((ndims * WKB_DOUBLE_SIZE) + WKB_TIMESTAMP_SIZE);
  wkb_parse_state_check(s, size);
  /* Parse the instants */
  TInstant **instants = tpointinstarr_from_wkb_state(s, count);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/**
 * Set the bound flags from their WKB representation
 */
static void
tpoint_bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc, bool *upper_inc)
{
  if (wkb_bounds & MOBDB_WKB_LOWER_INC)
    *lower_inc = true;
  else
    *lower_inc = false;
  if (wkb_bounds & MOBDB_WKB_UPPER_INC)
    *upper_inc = true;
  else
    *upper_inc = false;
  return;
}

/**
 * Return a temporal sequence point from its WKB representation
 */
static TSequence *
tpointseq_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions. */
  uint32_t ndims = (s->hasz) ? 3 : 2;
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
 * Return a temporal sequence set point from its WKB representation
 */
static TSequenceSet *
tpointseqset_from_wkb_state(wkb_parse_state *s)
{
  /* Count the dimensions. */
  uint32_t ndims = (s->hasz) ? 3 : 2;
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
    CachedType temptype = (s->geodetic) ? T_TGEOGPOINT : T_TGEOMPOINT;
    TInstant **instants = palloc(sizeof(TInstant *) * countinst);
    for (int j = 0; j < countinst; j++)
    {
      /* Parse the point and the timestamp to create the instant point */
      Datum value = point_from_wkb_state(s);
      TimestampTz t = timestamp_from_wkb_state(s);
      instants[j] = tinstant_make(value, t, temptype);
      pfree(DatumGetPointer(value));
    }
    sequences[i] = tsequence_make_free(instants, countinst, lower_inc,
      upper_inc, s->linear, NORMALIZE);
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Return a temporal point from its WKB representation
 */
static Temporal *
tpoint_from_wkb_state(wkb_parse_state *s)
{
  /* Fail when handed incorrect starting byte */
  char wkb_little_endian = byte_from_wkb_state(s);
  if (wkb_little_endian != 1 && wkb_little_endian != 0)
    elog(ERROR, "Invalid endian flag value encountered.");

  /* Check the endianness of our input */
  s->swap_bytes = false;
  /* Machine arch is big endian, request is for little */
#if POSTGIS_VERSION_NUMBER < 30000
  if (getMachineEndian() != NDR && wkb_little_endian)
#else
  if (IS_BIG_ENDIAN && wkb_little_endian)
#endif
    s->swap_bytes = true;
  /* Machine arch is little endian, request is for big */
#if POSTGIS_VERSION_NUMBER < 30000
  else if (getMachineEndian() == NDR && ! wkb_little_endian)
#else
  else if ((!IS_BIG_ENDIAN) && (!wkb_little_endian))
#endif
    s->swap_bytes = true;

  /* Read the temporal and interpolation flags */
  uint8_t wkb_type = (uint8_t) byte_from_wkb_state(s);
  tpoint_type_from_wkb_state(s, wkb_type);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = integer_from_wkb_state(s);
  else if (wkb_type & MOBDB_WKB_GEODETICFLAG)
    s->srid = SRID_DEFAULT;

  ensure_valid_tempsubtype(s->subtype);
  if (s->subtype == INSTANT)
    return (Temporal *) tpointinst_from_wkb_state(s);
  else if (s->subtype == INSTANTSET)
    return (Temporal *) tpointinstset_from_wkb_state(s);
  else if (s->subtype == SEQUENCE)
    return (Temporal *) tpointseq_from_wkb_state(s);
  else /* s->subtype == SEQUENCESET */
    return (Temporal *) tpointseqset_from_wkb_state(s);
  return NULL; /* make compiler quiet */
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its EWKB representation.
 */
Temporal *
tpoint_from_ewkb(uint8_t *wkb, int size)
{
  /* Initialize the state appropriately */
  wkb_parse_state s;
  s.wkb = wkb;
  s.wkb_size = size;
  s.swap_bytes = false;
  s.subtype = ANYTEMPSUBTYPE;
  s.srid = SRID_UNKNOWN;
  s.hasz = false;
  s.geodetic = false;
  s.has_srid = false;
  s.linear = false;
  s.pos = wkb;
  return tpoint_from_wkb_state(&s);
}

/*****************************************************************************
 * Input in HEXEWKB format
 *****************************************************************************/

/**
 * Return a temporal point from its HEXEWKB representation
 */
Temporal *
tpoint_from_hexewkb(const char *hexwkb)
{
  int hexwkb_len = strlen(hexwkb);
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, hexwkb_len);
  Temporal *result = tpoint_from_ewkb(wkb, hexwkb_len / 2);
  pfree(wkb);
  return result;
}

/*****************************************************************************
 * Input in EWKT format
 *****************************************************************************/

/**
 * This just does the same thing as the _in function, except it has to handle
 * a 'text' input. First, unwrap the text into a cstring, then do as tpoint_in
*/
Temporal *
tpoint_from_ewkt(const char *wkt, CachedType temptype)
{
  Temporal *result = tpoint_parse((char **) &wkt, temptype);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_mfjson);
/**
 * Return a temporal point from its MF-JSON representation
 */
PGDLLEXPORT Datum
Tpoint_from_mfjson(PG_FUNCTION_ARGS)
{
  text *mfjson_input = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Temporal *result = tpoint_from_mfjson_ext(fcinfo, mfjson_input,
    oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Input in EWKB format
 * Please refer to the file tpoint_out.c where the binary format is explained
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_ewkb);
/**
 * Return a temporal point from its EWKB representation
 */
PGDLLEXPORT Datum
Tpoint_from_ewkb(PG_FUNCTION_ARGS)
{
  bytea *bytea_wkb = PG_GETARG_BYTEA_P(0);
  uint8_t *wkb = (uint8_t *) VARDATA(bytea_wkb);
  Temporal *temp = tpoint_from_ewkb(wkb, VARSIZE(bytea_wkb) - VARHDRSZ);
  PG_FREE_IF_COPY(bytea_wkb, 0);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Input in HEXEWKB format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_hexewkb);
/**
 * Return a temporal point from its HEXEWKB representation
 */
PGDLLEXPORT Datum
Tpoint_from_hexewkb(PG_FUNCTION_ARGS)
{
  text *hexwkb_text = PG_GETARG_TEXT_P(0);
  char *hexwkb = text2cstring(hexwkb_text);
  Temporal *temp = tpoint_from_hexewkb(hexwkb);
  pfree(hexwkb);
  PG_FREE_IF_COPY(hexwkb_text, 0);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Input in EWKT format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_from_ewkt);
/**
 * This just does the same thing as the _in function, except it has to handle
 * a 'text' input. First, unwrap the text into a cstring, then do as tpoint_in
*/
PGDLLEXPORT Datum
Tpoint_from_ewkt(PG_FUNCTION_ARGS)
{
  text *wkt_text = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  char *wkt = text2cstring(wkt_text);
  Temporal *result = tpoint_from_ewkt(wkt, oid_type(temptypid));
  PG_FREE_IF_COPY(wkt_text, 0);
  PG_RETURN_POINTER(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
