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
 * @brief Input of temporal points in WKT, EWKT, , EWKB, and MF-JSON format.
 */

#include "point/tpoint_in.h"

/* C */
#include <assert.h>
#include <float.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/temporal_in.h"
#include "general/temporal_util.h"
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
      elog(ERROR, "Invalid MFJSON string");
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
    elog(ERROR, "Invalid value of the 'coordinates' array in MFJSON string");

  const int numcoord = json_object_array_length(poObj);
  if (numcoord < 2)
    elog(ERROR, "Too few elements in 'coordinates' values in MFJSON string");
  else if (numcoord > 3)
    elog(ERROR, "Too many elements in 'coordinates' values in MFJSON string");

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
    elog(ERROR, "Unable to find 'coordinates' in MFJSON string");
  if (json_object_get_type(coordinates) != json_type_array)
    elog(ERROR, "Invalid 'coordinates' array in MFJSON string");

  int numpoints = json_object_array_length(coordinates);
  if (numpoints < 1)
    elog(ERROR, "Invalid value of 'coordinates' array in MFJSON string");

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
      elog(ERROR, "Unable to find 'datetimes' in MFJSON string");
  if (json_object_get_type(datetimes) != json_type_array)
    elog(ERROR, "Invalid 'datetimes' array in MFJSON string");

  int numdates = json_object_array_length(datetimes);
  if (numdates < 1)
    elog(ERROR, "Invalid value of 'datetimes' array in MFJSON string");

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
      times[i] = basetype_input(T_TIMESTAMPTZ, datetime);
    }
  }
  *count = numdates;
  return times;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal instant point from its MF-JSON representation.
 */
TInstant *
tpointinst_from_mfjson(json_object *mfjson, int srid, CachedType temptype)
{
  bool geodetic = (temptype == T_TGEOGPOINT);
  /* Get coordinates */
  json_object *coordinates = findMemberByName(mfjson, "coordinates");
  if (coordinates == NULL)
    elog(ERROR, "Unable to find 'coordinates' in MFJSON string");
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
    elog(ERROR, "Invalid 'datetimes' value in MFJSON string");
    return NULL; /* make Codacy quiet */
  }
  strcpy(str, strdatetimes);
  /* Replace 'T' by ' ' before converting to timestamptz */
  str[10] = ' ';
  TimestampTz t = basetype_input(T_TIMESTAMPTZ, str);
  TInstant *result = tinstant_make(value, temptype, t);
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
    elog(ERROR, "Distinct number of elements in 'coordinates' and 'datetimes' arrays");

  /* Construct the array of temporal instant points */
  TInstant **result = palloc(sizeof(TInstant *) * numpoints);
  for (int i = 0; i < numpoints; i++)
    result[i] = tinstant_make(values[i], temptype, times[i]);

  for (int i = 0; i < numpoints; i++)
    pfree(DatumGetPointer(values[i]));
  pfree(values); pfree(times);
  *count = numpoints;
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal instant set point from its MF-JSON representation.
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
 * @brief Return a temporal sequence point from its MF-JSON representation.
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
    elog(ERROR, "Unable to find 'lower_inc' in MFJSON string");
  bool lower_inc = (bool) json_object_get_boolean(lowerinc);

  /* Get upper bound flag */
  json_object *upperinc = NULL;
  upperinc = findMemberByName(mfjson, "upper_inc");
  if (upperinc == NULL)
    elog(ERROR, "Unable to find 'upper_inc' in MFJSON string");
  bool upper_inc = (bool) json_object_get_boolean(upperinc);

  /* Construct the temporal point */
  return tsequence_make_free(instants, count, lower_inc, upper_inc,
    linear, NORMALIZE);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal sequence set point from its MF-JSON representation.
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
    elog(ERROR, "Invalid 'sequences' array in MFJSON string");
  int numseqs = json_object_array_length(seqs);
  if (numseqs < 1)
    elog(ERROR, "Invalid value of 'sequences' array in MFJSON string");

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

/*****************************************************************************
 * Input in EWKT format
 *****************************************************************************/

#if MEOS
/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its WKT representation
 */
Temporal *
tpoint_from_text(const char *wkt, CachedType temptype)
{
  Temporal *result = tpoint_parse((char **) &wkt, temptype);
  return result;
}
#endif

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal point from its EWKT representation
 */
Temporal *
tpoint_from_ewkt(const char *ewkt, CachedType temptype)
{
  Temporal *result = tpoint_parse((char **) &ewkt, temptype);
  return result;
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

/**
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
    elog(ERROR, "Error while processing MFJSON string");
  }
  json_tokener_free(jstok);

  /*
   * Ensure that it is a moving point
   */
  poObjType = findMemberByName(poObj, "type");
  if (poObjType == NULL)
    elog(ERROR, "Unable to find 'type' in MFJSON string");

  const char *pszType = json_object_get_string(poObjType);
  if (strcmp(pszType, "MovingPoint") != 0)
    elog(ERROR, "Invalid 'type' value in MFJSON string");

  /*
   * Determine type of temporal point and call the corresponding parse function
   */
  poObjInterp = findMemberByName(poObj, "interpolations");
  if (poObjInterp == NULL)
    elog(ERROR, "Unable to find 'interpolations' in MFJSON string");

  if (json_object_get_type(poObjInterp) != json_type_array)
    elog(ERROR, "Invalid 'interpolations' value in MFJSON string");

  const int nSize = json_object_array_length(poObjInterp);
  if (nSize != 1)
    elog(ERROR, "Multiple 'interpolations' values in MFJSON string");

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
      elog(ERROR, "Invalid 'interpolations' value in MFJSON string");
  }

  return result;
}

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

#endif /* #if ! MEOS */

/*****************************************************************************/
