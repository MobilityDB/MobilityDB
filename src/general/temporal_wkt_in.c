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
 * @file temporal_wkt_in.c
 * @brief Output of temporal values in WKT, EWKT, and MF-JSON format.
 */

// #include "general/temporal_wkt_in.h"

/* C */
#include <assert.h>
#include <float.h>
/* PostGIS */
#include <liblwgeom_internal.h>
/* MobilityDB */
#include <libmeos.h>
#include "general/tinstant.h"
#include "general/tinstantset.h"
#include "general/tsequence.h"
#include "general/tsequenceset.h"
#include "general/temporal_util.h"
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
  if (numcoord > 3)
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

/**
 * Return an array of points from its MF-JSON coordinates. In this case the
 * coordinate array is an array of arrays of cordinates such as
 * "values":[1.5,2.5]
 */
static Datum *
parse_mfjson_values(json_object *mfjson, CachedType temptype, int *count)
{
  json_object *mfjsonTmp = mfjson;
  json_object *jvalues = NULL;
  jvalues = findMemberByName(mfjsonTmp, "values");
  if (jvalues == NULL)
    elog(ERROR, "Unable to find 'values' in MFJSON string");
  if (json_object_get_type(jvalues) != json_type_array)
    elog(ERROR, "Invalid 'values' array in MFJSON string");

  int numvalues = json_object_array_length(jvalues);
  if (numvalues < 1)
    elog(ERROR, "Invalid value of 'values' array in MFJSON string");

  Datum *values = palloc(sizeof(Datum) * numvalues);
  for (int i = 0; i < numvalues; ++i)
  {
    json_object *jvalue = NULL;
    jvalue = json_object_array_get_idx(jvalues, i);
    switch (temptype)
    {
      case T_TBOOL:
        if (json_object_get_type(jvalue) != json_type_boolean)
          elog(ERROR, "Invalid boolean value in 'values' array in MFJSON string");
        values[i] = BoolGetDatum(json_object_get_boolean(jvalue));
        break;
      case T_TINT:
        if (json_object_get_type(jvalue) != json_type_int)
          elog(ERROR, "Invalid integer value in 'values' array in MFJSON string");
        values[i] = Int32GetDatum(json_object_get_int(jvalue));
        break;
      case T_TFLOAT:
        values[i] = Float8GetDatum(json_object_get_double(jvalue));
        break;
      case T_TTEXT:
        if (json_object_get_type(jvalue) != json_type_string)
          elog(ERROR, "Invalid string value in 'values' array in MFJSON string");
        values[i] = PointerGetDatum(cstring2text(json_object_get_string(jvalue)));
        break;
      default: /* Error! */
        elog(ERROR, "Unknown temporal type: %d", temptype);
    }
  }
  *count = numvalues;
  return values;
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
      times[i] = basetype_input(T_TIMESTAMPTZ, datetime, false);
    }
  }
  *count = numdates;
  return times;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal instant point from its MF-JSON representation.
 * @sqlfunc tboolFromMFJSON(), tintFromMFJSON() tfloattFromMFJSON(),
 * ttextFromMFJSON(), tgeompointFromMFJSON(), tgeogpointFromMFJSON()
 */
TInstant *
tinstant_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  CachedType temptype)
{
  bool geodetic = (temptype == T_TGEOGPOINT);
  bool byvalue = basetype_byvalue(temptype_basetype(temptype));
  Datum value;
  if (! isgeo)
  {
    /* Get values */
    json_object *values = findMemberByName(mfjson, "values");
    if (values == NULL)
      elog(ERROR, "Unable to find 'values' in MFJSON string");
    switch (temptype)
    {
      case T_TBOOL:
        if (json_object_get_type(values) != json_type_boolean)
          elog(ERROR, "Invalid boolean value in 'values' array in MFJSON string");
        value = BoolGetDatum(json_object_get_boolean(values));
        break;
      case T_TINT:
        if (json_object_get_type(values) != json_type_int)
          elog(ERROR, "Invalid integer value in 'values' array in MFJSON string");
        value = Int32GetDatum(json_object_get_int(values));
        break;
      case T_TFLOAT:
        value = Float8GetDatum(json_object_get_double(values));
        break;
      case T_TTEXT:
        if (json_object_get_type(values) != json_type_string)
          elog(ERROR, "Invalid string value in 'values' array in MFJSON string");
        value = PointerGetDatum(cstring2text(json_object_get_string(values)));
        break;
      default: /* Error! */
        elog(ERROR, "Unknown temporal type: %d", temptype);
    }
  }
  else
  {
    /* Get coordinates */
    json_object *coordinates = findMemberByName(mfjson, "coordinates");
    if (coordinates == NULL)
      elog(ERROR, "Unable to find 'coordinates' in MFJSON string");
    value = parse_mfjson_coord(coordinates, srid, geodetic);
  }

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
  TimestampTz t = basetype_input(T_TIMESTAMPTZ, str, false);
  TInstant *result = tinstant_make(value, temptype, t);
  if (! byvalue)
    pfree(DatumGetPointer(value));
  return result;
}

/**
 * Return array of temporal instant points from its MF-JSON representation
 */
static TInstant **
tinstarr_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  CachedType temptype, int *count)
{
  bool geodetic = (temptype == T_TGEOGPOINT);
  bool byvalue = basetype_byvalue(temptype_basetype(temptype));
  /* Get coordinates and datetimes */
  int numvalues, numdates;
  Datum *values;
  if (! isgeo)
    values = parse_mfjson_values(mfjson, temptype, &numvalues);
  else
    values = parse_mfjson_points(mfjson, srid, geodetic, &numvalues);
  TimestampTz *times = parse_mfjson_datetimes(mfjson, &numdates);
  if (numvalues != numdates)
    elog(ERROR, "Distinct number of elements in '%s' and 'datetimes' arrays",
      ! isgeo ? "values" : "coordinates");

  /* Construct the array of temporal instant points */
  TInstant **result = palloc(sizeof(TInstant *) * numvalues);
  for (int i = 0; i < numvalues; i++)
    result[i] = tinstant_make(values[i], temptype, times[i]);

  if (! byvalue)
  {
    for (int i = 0; i < numvalues; i++)
      pfree(DatumGetPointer(values[i]));
  }
  pfree(values); pfree(times);
  *count = numvalues;
  return result;
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal instant set point from its MF-JSON representation.
 * @sqlfunc tboolFromMFJSON(), tintFromMFJSON() tfloattFromMFJSON(),
 * ttextFromMFJSON(), tgeompointFromMFJSON(), tgeogpointFromMFJSON()
 */
TInstantSet *
tinstantset_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  CachedType temptype)
{
  int count;
  TInstant **instants = tinstarr_from_mfjson(mfjson, isgeo, srid, temptype,
    &count);
  return tinstantset_make_free(instants, count, MERGE_NO);
}

/**
 * @ingroup libmeos_temporal_input_output
 * @brief Return a temporal sequence point from its MF-JSON representation.
 * @sqlfunc tboolFromMFJSON(), tintFromMFJSON() tfloattFromMFJSON(),
 * ttextFromMFJSON(), tgeompointFromMFJSON(), tgeogpointFromMFJSON()
 */
TSequence *
tsequence_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  CachedType temptype, bool linear)
{
  /* Get the array of temporal instant points */
  int count;
  TInstant **instants = tinstarr_from_mfjson(mfjson, isgeo, srid, temptype,
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
 * @sqlfunc tboolFromMFJSON(), tintFromMFJSON() tfloattFromMFJSON(),
 * ttextFromMFJSON(), tgeompointFromMFJSON(), tgeogpointFromMFJSON()
 */
TSequenceSet *
tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  CachedType temptype, bool linear)
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
    sequences[i] = tsequence_from_mfjson(seqvalue, isgeo, srid, temptype, linear);
  }
  return tsequenceset_make_free(sequences, numseqs, NORMALIZE);
}

/*****************************************************************************/
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#if ! MEOS

static void
ensure_temptype_mfjson(const char *typestr)
{
  if (strcmp(typestr, "MovingBoolean") != 0 && strcmp(typestr, "MovingInteger") != 0 &&
      strcmp(typestr, "MovingFloat") != 0 && strcmp(typestr, "MovingText") != 0 &&
      strcmp(typestr, "MovingPoint") != 0)
    elog(ERROR, "Invalid 'type' value in MFJSON string");
}

/**
 * @brief Return a temporal point from its MF-JSON representation
 */
Temporal *
temporal_from_mfjson_ext(FunctionCallInfo fcinfo, text *mfjson_input,
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
  ensure_temptype_mfjson(pszType);

  /*
   * Determine type of temporal type and call the corresponding parse function
   */
  poObjInterp = findMemberByName(poObj, "interpolations");
  if (poObjInterp == NULL)
    elog(ERROR, "Unable to find 'interpolations' in MFJSON string");

  if (json_object_get_type(poObjInterp) != json_type_array)
    elog(ERROR, "Invalid 'interpolations' value in MFJSON string");

  const int nSize = json_object_array_length(poObjInterp);
  if (nSize != 1)
    elog(ERROR, "Multiple 'interpolations' values in MFJSON string");

  bool isgeo = tgeo_type(temptype);
  if (isgeo)
  {
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
        result = (Temporal *) tinstantset_from_mfjson(poObj, isgeo, srid,
          temptype);
      else
        result = (Temporal *) tinstant_from_mfjson(poObj, isgeo, srid,
          temptype);
    }
    else if (strcmp(pszInterp, "Stepwise") == 0 ||
      strcmp(pszInterp, "Linear") == 0)
    {
      bool linear = strcmp(pszInterp, "Linear") == 0;
      json_object *poObjSeqs = findMemberByName(poObj, "sequences");
      if (poObjSeqs != NULL)
        result = (Temporal *) tsequenceset_from_mfjson(poObj, isgeo, srid,
          temptype, linear);
      else
        result = (Temporal *) tsequence_from_mfjson(poObj, isgeo, srid,
          temptype, linear);
    }
    else
      elog(ERROR, "Invalid 'interpolations' value in MFJSON string");
  }

  return result;
}

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Temporal_from_mfjson);
/**
 * Return a temporal point from its MF-JSON representation
 */
PGDLLEXPORT Datum
Temporal_from_mfjson(PG_FUNCTION_ARGS)
{
  text *mfjson_input = PG_GETARG_TEXT_P(0);
  Oid temptypid = get_fn_expr_rettype(fcinfo->flinfo);
  Temporal *result = temporal_from_mfjson_ext(fcinfo, mfjson_input,
    oid_type(temptypid));
  PG_RETURN_POINTER(result);
}

#endif /* #if ! MEOS */

/*****************************************************************************/
