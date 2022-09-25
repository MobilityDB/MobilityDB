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
 * @brief Input of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB format.
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_call.h"
#include "general/tbox.h"
#include "general/temporal_util.h"
#include "general/temporal_parser.h"
#include "point/stbox.h"
#include "point/tpoint_spatialfuncs.h"
#if NPOINT
  #include "npoint/tnpoint_static.h"
#endif /* NPOINT */

/*****************************************************************************/

/**
 * Structure used for passing the parse state between the parsing functions.
 */
typedef struct
{
  const uint8_t *wkb;  /**< Points to start of WKB */
  size_t wkb_size;     /**< Expected size of WKB */
  bool swap_bytes;     /**< Do an endian flip? */
  uint8_t temptype;    /**< Current temporal type we are handling */
  uint8_t basetype;    /**< Current base type we are handling */
  uint8_t subtype;     /**< Current subtype we are handling */
  int32_t srid;        /**< Current SRID we are handling */
  bool hasx;           /**< X? */
  bool hasz;           /**< Z? */
  bool hast;           /**< T? */
  bool geodetic;       /**< Geodetic? */
  bool has_srid;       /**< SRID? */
  interpType interp;   /**< Interpolation */
  const uint8_t *pos;  /**< Current parse position */
} wkb_parse_state;

/*****************************************************************************
 * Input in MFJSON format
 *****************************************************************************/

/**
 * Return the JSON member corresponding to the name
 *
 * @note Function taken from PostGIS file lwin_geojson.c
 */
json_object *
findMemberByName(json_object *poObj, const char *pszName)
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
parse_mfjson_values(json_object *mfjson, mobdbType temptype, int *count)
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
      /* The last argument is for an unused typmod */
      times[i] = pg_timestamptz_in(datetime, -1);
    }
  }
  *count = numdates;
  return times;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant from its MF-JSON representation.
 */
TInstant *
tinstant_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  mobdbType temptype)
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
  /* We don't need to test that datetimes is NULL since we look for the
   * "datetimes" member and then call this function */
  const char *strdatetimes = json_object_get_string(datetimes);
  if (strdatetimes == NULL)
  {
    elog(ERROR, "Invalid 'datetimes' value in MFJSON string");
    return NULL; /* make Codacy quiet */
  }
  strcpy(str, strdatetimes);
  /* Replace 'T' by ' ' before converting to timestamptz */
  str[10] = ' ';
  TimestampTz t = pg_timestamptz_in(str, -1);
  TInstant *result = tinstant_make(value, temptype, t);
  if (! byvalue)
    pfree(DatumGetPointer(value));
  return result;
}

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant boolean from its MF-JSON representation.
 * @sqlfunc tboolFromMFJSON()
 */
TInstant *
tboolinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TBOOL);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant integer from its MF-JSON representation.
 * @sqlfunc tintFromMFJSON()
 */
TInstant *
tintinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TINT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant float from its MF-JSON representation.
 * @sqlfunc tfloatFromMFJSON()
 */
TInstant *
tfloatinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TFLOAT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant text from its MF-JSON representation.
 * @sqlfunc ttextFromMFJSON()
 */
TInstant *
ttextinst_from_mfjson(json_object *mfjson)
{
  return tinstant_from_mfjson(mfjson, false, 0, T_TTEXT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant geometric point from its MF-JSON
 * representation.
 * @sqlfunc tgeompointFromMFJSON()
 */
TInstant *
tgeompointinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOMPOINT);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal instant geographic point from its MF-JSON
 * representation.
 * @sqlfunc tgeogpointFromMFJSON()
 */
TInstant *
tgeogpointinst_from_mfjson(json_object *mfjson, int srid)
{
  return tinstant_from_mfjson(mfjson, true, srid, T_TGEOGPOINT);
}
#endif /* MEOS */

/**
 * Return array of temporal instant points from its MF-JSON representation
 */
static TInstant **
tinstarr_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  mobdbType temptype, int *count)
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
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence point from its MF-JSON representation.
 */
TSequence *
tsequence_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  mobdbType temptype, interpType interp)
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
  return tsequence_make_free(instants, count, count, lower_inc, upper_inc,
    interp, NORMALIZE);
}

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence boolean from its MF-JSON representation.
 * @sqlfunc tboolFromMFJSON()
 */
TSequence *
tboolseq_from_mfjson(json_object *mfjson)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TBOOL, STEPWISE);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence integer from its MF-JSON representation.
 * @sqlfunc  tintFromMFJSON()
 */
TSequence *
tintseq_from_mfjson(json_object *mfjson)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TINT, STEPWISE);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence float from its MF-JSON representation.
 * @sqlfunc tfloattFromMFJSON()
 */
TSequence *
tfloatseq_from_mfjson(json_object *mfjson, interpType interp)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TFLOAT, interp);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence text from its MF-JSON representation.
 * @sqlfunc ttextFromMFJSON()
 */
TSequence *
ttextseq_from_mfjson(json_object *mfjson)
{
  return tsequence_from_mfjson(mfjson, false, 0, T_TTEXT, STEPWISE);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence geometric point from its MF-JSON
 * representation.
 * @sqlfunc tgeompointFromMFJSON()
 */
TSequence *
tgeompointseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOMPOINT, interp);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence geographic point from its MF-JSON
 * representation.
 * @sqlfunc tgeogpointFromMFJSON()
 */
TSequence *
tgeogpointseq_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequence_from_mfjson(mfjson, true, srid, T_TGEOGPOINT, interp);
}
#endif /* MEOS */

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set point from its MF-JSON representation.
 */
TSequenceSet *
tsequenceset_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  mobdbType temptype, interpType interp)
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
    sequences[i] = tsequence_from_mfjson(seqvalue, isgeo, srid, temptype, interp);
  }
  return tsequenceset_make_free(sequences, numseqs, NORMALIZE);
}

#if MEOS
/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set boolean from its MF-JSON
 * representation.
 * @sqlfunc tboolFromMFJSON()
 */
TSequenceSet *
tboolseqset_from_mfjson(json_object *mfjson)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TBOOL, STEPWISE);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set integer from its MF-JSON representation.
 * @sqlfunc tintFromMFJSON()
 */
TSequenceSet *
tintseqset_from_mfjson(json_object *mfjson)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TINT, STEPWISE);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set float from its MF-JSON representation.
 * @sqlfunc tfloatFromMFJSON()
 */
TSequenceSet *
tfloatseqset_from_mfjson(json_object *mfjson, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TFLOAT, interp);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set text from its MF-JSON representation.
 * @sqlfunc ttextFromMFJSON()
 */
TSequenceSet *
ttextseqset_from_mfjson(json_object *mfjson)
{
  return tsequenceset_from_mfjson(mfjson, false, 0, T_TTEXT, STEPWISE);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set geometric point from its MF-JSON
 * representation.
 * @sqlfunc tgeompointFromMFJSON()
 */
TSequenceSet *
tgeompointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOMPOINT, interp);
}

/**
 * @ingroup libmeos_int_temporal_in_out
 * @brief Return a temporal sequence set geographic point from its MF-JSON
 * representation.
 * @sqlfunc tgeogpointFromMFJSON()
 */
TSequenceSet *
tgeogpointseqset_from_mfjson(json_object *mfjson, int srid, interpType interp)
{
  return tsequenceset_from_mfjson(mfjson, true, srid, T_TGEOGPOINT, interp);
}
#endif /* MEOS */

/*****************************************************************************/

static void
ensure_temptype_mfjson(const char *typestr)
{
  if (strcmp(typestr, "MovingBoolean") != 0 && strcmp(typestr, "MovingInteger") != 0 &&
      strcmp(typestr, "MovingFloat") != 0 && strcmp(typestr, "MovingText") != 0 &&
      strcmp(typestr, "MovingGeomPoint") != 0 && strcmp(typestr, "MovingGeogPoint") != 0 )
    elog(ERROR, "Invalid 'type' value in MFJSON string");
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal point from its MF-JSON representation
 */
Temporal *
temporal_from_mfjson(const char *mfjson)
{
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
   * Ensure that it is a moving type
   */
  poObjType = findMemberByName(poObj, "type");
  if (poObjType == NULL)
    elog(ERROR, "Unable to find 'type' in MFJSON string");

  /*
   * Determine type of temporal type
   */
  const char *typestr = json_object_get_string(poObjType);
  mobdbType temptype;
  ensure_temptype_mfjson(typestr);
  if (strcmp(typestr, "MovingBoolean") == 0)
    temptype = T_TBOOL;
  else if (strcmp(typestr, "MovingInteger") == 0)
    temptype = T_TINT;
  else if (strcmp(typestr, "MovingFloat") == 0)
    temptype = T_TFLOAT;
  else if (strcmp(typestr, "MovingText") == 0)
    temptype = T_TTEXT;
  else if (strcmp(typestr, "MovingGeomPoint") == 0)
    temptype = T_TGEOMPOINT;
  else /* typestr == "MovingGeogPoint" */
    temptype = T_TGEOGPOINT;

  /*
   * Determine interpolation type
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
      // The following call requires a valid spatial_ref_sys table entry
      // srid = getSRIDbySRS(fcinfo, srs);
      sscanf(srs, "EPSG:%d", &srid);
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
        result = (Temporal *) tsequence_from_mfjson(poObj, isgeo, srid,
          temptype, DISCRETE);
      else
        result = (Temporal *) tinstant_from_mfjson(poObj, isgeo, srid,
          temptype);
    }
    else if (strcmp(pszInterp, "Stepwise") == 0 ||
      strcmp(pszInterp, "Linear") == 0)
    {
      interpType interp = (strcmp(pszInterp, "Linear") == 0) ? LINEAR : STEPWISE;
      json_object *poObjSeqs = findMemberByName(poObj, "sequences");
      if (poObjSeqs != NULL)
        result = (Temporal *) tsequenceset_from_mfjson(poObj, isgeo, srid,
          temptype, interp);
      else
        result = (Temporal *) tsequence_from_mfjson(poObj, isgeo, srid,
          temptype, interp);
    }
    else
      elog(ERROR, "Invalid 'interpolations' value in MFJSON string");
  }

  return result;
}

/*****************************************************************************
 * Input in WKB format
 * Please refer to the file temporal_wkb_out.c where the binary format is
 * explained
 *****************************************************************************/

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
char
byte_from_wkb_state(wkb_parse_state *s)
{
  char char_value = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_BYTE_SIZE);
  /* Get the data */
  char_value = s->pos[0];
  s->pos += MOBDB_WKB_BYTE_SIZE;
  return char_value;
}

/**
 * Read a 2-byte integer and advance the parse state forward
 */
uint16_t
int16_from_wkb_state(wkb_parse_state *s)
{
  uint16_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT2_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MOBDB_WKB_INT2_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MOBDB_WKB_INT2_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MOBDB_WKB_INT2_SIZE - j - 1];
      ((uint8_t*)(&i))[MOBDB_WKB_INT2_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_INT2_SIZE;
  return i;
}

/**
 * Read a 4-byte integer and advance the parse state forward
 */
uint32_t
int32_from_wkb_state(wkb_parse_state *s)
{
  uint32_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT4_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MOBDB_WKB_INT4_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MOBDB_WKB_INT4_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MOBDB_WKB_INT4_SIZE - j - 1];
      ((uint8_t*)(&i))[MOBDB_WKB_INT4_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_INT4_SIZE;
  return i;
}

/**
 * Read an 8-byte integer and advance the parse state forward
 */
uint64_t
int64_from_wkb_state(wkb_parse_state *s)
{
  uint64_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT8_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MOBDB_WKB_INT8_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MOBDB_WKB_INT8_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MOBDB_WKB_INT8_SIZE - j - 1];
      ((uint8_t*)(&i))[MOBDB_WKB_INT8_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_INT8_SIZE;
  return i;
}

/**
 * Read an 8-byte double and advance the parse state forward
 */
double
double_from_wkb_state(wkb_parse_state *s)
{
  double d = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_DOUBLE_SIZE);
  /* Get the data */
  memcpy(&d, s->pos, MOBDB_WKB_DOUBLE_SIZE);
  /* Swap? Copy into a stack-allocated double */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MOBDB_WKB_DOUBLE_SIZE/2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&d))[i];
      ((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[MOBDB_WKB_DOUBLE_SIZE - i - 1];
      ((uint8_t*)(&d))[MOBDB_WKB_DOUBLE_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_DOUBLE_SIZE;
  return d;
}

/**
 * Read an 8-byte timestamp and advance the parse state forward
 */
TimestampTz
timestamp_from_wkb_state(wkb_parse_state *s)
{
  int64_t t = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_TIMESTAMP_SIZE);
  /* Get the data */
  memcpy(&t, s->pos, MOBDB_WKB_TIMESTAMP_SIZE);
  /* Swap? Copy into a stack-allocated timestamp */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MOBDB_WKB_TIMESTAMP_SIZE/2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&t))[i];
      ((uint8_t*)(&t))[i] = ((uint8_t*)(&t))[MOBDB_WKB_TIMESTAMP_SIZE - i - 1];
      ((uint8_t*)(&t))[MOBDB_WKB_TIMESTAMP_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MOBDB_WKB_TIMESTAMP_SIZE;
  return (TimestampTz) t;
}

/**
 * Read a text and advance the parse state forward
 */
text *
text_from_wkb_state(wkb_parse_state *s)
{
  /* Get the size of the text value */
  size_t size = int64_from_wkb_state(s);
  assert(size > 0);
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, size);
  /* Get the data */
  char *str = palloc(size + 1);
  memcpy(str, s->pos, size);
  s->pos += size;
  text *result = cstring2text(str);
  pfree(str);
  /* Advance the state and return */
  return result;
}

/**
 * Return a point from its WKB representation. A WKB point has just a set of
 * doubles, with the quantity depending on the dimension of the point.
 */
Datum
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

#if NPOINT
/**
 * Read an npoint and advance the parse state forward
 */
Npoint *
npoint_from_wkb_state(wkb_parse_state *s)
{
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MOBDB_WKB_INT8_SIZE + MOBDB_WKB_DOUBLE_SIZE);
  /* Get the data */
  int64 rid = int64_from_wkb_state(s);
  double pos = double_from_wkb_state(s);
  Npoint *result = palloc(sizeof(Npoint));
  npoint_set(rid, pos, result);
  return result;
}
#endif /* ! MEOS */

/*****************************************************************************/

/**
 * Take in an unknown span type of WKB type number and ensure it comes out
 * as an extended WKB span type number.
 */
void
span_spantype_from_wkb_state(wkb_parse_state *s, uint16_t wkb_spantype)
{
  switch (wkb_spantype)
  {
    case MOBDB_WKB_T_INTSPAN:
      s->temptype = T_INTSPAN;
      break;
    case MOBDB_WKB_T_FLOATSPAN:
      s->temptype = T_FLOATSPAN;
      break;
    case MOBDB_WKB_T_PERIOD:
      s->temptype = T_PERIOD;
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB span type: %d", wkb_spantype);
      break;
  }
  s->basetype = spantype_basetype(s->temptype);
  return;
}

/**
 * Return the size of a span base value from its WKB representation.
 */
static size_t
span_basevalue_from_wkb_size(wkb_parse_state *s)
{
  size_t result = 0;
  ensure_span_basetype(s->basetype);
  switch (s->basetype)
  {
    case T_INT4:
      result = sizeof(int);
      break;
    case T_FLOAT8:
      result = sizeof(double);
      break;
    case T_TIMESTAMPTZ:
      result = sizeof(TimestampTz);
      break;
  }
  return result;
}


/**
 * Return a value from its WKB representation.
 */
static Datum
span_basevalue_from_wkb_state(wkb_parse_state *s)
{
  Datum result;
  ensure_span_type(s->temptype);
  switch (s->temptype)
  {
    case T_INTSPAN:
      result = Int32GetDatum(int32_from_wkb_state(s));
      break;
    case T_FLOATSPAN:
      result = Float8GetDatum(double_from_wkb_state(s));
      break;
    case T_PERIOD:
      result = TimestampTzGetDatum(timestamp_from_wkb_state(s));
      break;
    default: /* Error! */
      elog(ERROR, "Unknown span type: %d",
        s->temptype);
      break;
  }
  return result;
}

/**
 * Set the bound flags from their WKB representation
 */
static void
bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc, bool *upper_inc)
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
 * Return a span from its WKB representation
 */
Span *
span_from_wkb_state(wkb_parse_state *s)
{
  /* Read the span type */
  uint16_t wkb_spantype = (uint16_t) int16_from_wkb_state(s);
  span_spantype_from_wkb_state(s, wkb_spantype);

  /* Read the span bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);

  /* Does the data we want to read exist? */
  size_t size = 2 * span_basevalue_from_wkb_size(s);
  wkb_parse_state_check(s, size);

  /* Read the values and create the span */
  Datum lower = span_basevalue_from_wkb_state(s);
  Datum upper = span_basevalue_from_wkb_state(s);
  Span *result = span_make(lower, upper, lower_inc, upper_inc, s->basetype);
  return result;
}

/*****************************************************************************/

/**
 * Return a timestamp set from its WKB representation
 */
static TimestampSet *
timestampset_from_wkb_state(wkb_parse_state *s)
{
  /* Read the number of timestamps and allocate space for them */
  int count = int32_from_wkb_state(s);
  TimestampTz *times = palloc(sizeof(TimestampTz) * count);

  /* Read and create the timestamp set */
  for (int i = 0; i < count; i++)
    times[i] = timestamp_from_wkb_state(s);
  TimestampSet *result = timestampset_make_free(times, count);
  return result;
}

/*****************************************************************************/

/**
 * Optimized version of span_from_wkb_state for reading the periods in a period
 * set. The endian byte and the basetype int16 are not read from the buffer.
 */
static Period *
period_from_wkb_state(wkb_parse_state *s)
{
  /* Read the span bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);

  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, 2 * MOBDB_WKB_TIMESTAMP_SIZE);

  /* Read the values and create the span */
  Datum lower = TimestampTzGetDatum(timestamp_from_wkb_state(s));
  Datum upper = TimestampTzGetDatum(timestamp_from_wkb_state(s));
  Span *result = span_make(lower, upper, lower_inc, upper_inc, s->basetype);
  return result;
}

/**
 * Return a period set from its WKB representation
 */
static PeriodSet *
periodset_from_wkb_state(wkb_parse_state *s)
{
  /* Read the number of periods and allocate space for them */
  int count = int32_from_wkb_state(s);
  Period **periods = palloc(sizeof(Period *) * count);

  /* Set the state basetype to TimestampTz */
  s->basetype = T_TIMESTAMPTZ;

  /* Read and create the period set */
  for (int i = 0; i < count; i++)
    periods[i] = (Period *) period_from_wkb_state(s);
  PeriodSet *result = periodset_make_free(periods, count, NORMALIZE);
  return result;
}

/*****************************************************************************/

/**
 * Set the state flags according to a box byte flag read from the buffer.
 */
static void
tbox_flags_from_wkb_state(wkb_parse_state *s, uint8_t wkb_flags)
{
  assert(wkb_flags & MOBDB_WKB_XFLAG || wkb_flags & MOBDB_WKB_TFLAG);
  s->hasx = false;
  s->hast = false;
  if (wkb_flags & MOBDB_WKB_XFLAG)
    s->hasx = true;
  if (wkb_flags & MOBDB_WKB_TFLAG)
    s->hast = true;
  return;
}

/**
 * Return a temporal box from its WKB representation
 */
static TBOX *
tbox_from_wkb_state(wkb_parse_state *s)
{
  /* Read the temporal flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  tbox_flags_from_wkb_state(s, wkb_flags);

  /* Read and create the box */
  Span *span = NULL;
  Span *period = NULL;
  /* Read the temporal dimension if any */
  if (s->hast)
    period = span_from_wkb_state(s);
  /* Read the value dimension if any */
  if (s->hasx)
    span = span_from_wkb_state(s);
  /* Create the temporal box */
  TBOX *result = tbox_make(period, span);
  return result;
}

/*****************************************************************************/

/**
 * Set the state flags according to a box byte flag read from the buffer.
 */
static void
stbox_flags_from_wkb_state(wkb_parse_state *s, uint8_t wkb_flags)
{
  assert(wkb_flags & MOBDB_WKB_XFLAG || wkb_flags & MOBDB_WKB_TFLAG);
  s->hasx = false;
  s->hasz = false;
  s->hast = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_flags & MOBDB_WKB_XFLAG)
    s->hasx = true;
  if (wkb_flags & MOBDB_WKB_ZFLAG)
    s->hasz = true;
  if (wkb_flags & MOBDB_WKB_TFLAG)
    s->hast = true;
  if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    s->geodetic = true;
  if (wkb_flags & MOBDB_WKB_SRIDFLAG)
    s->has_srid = true;
  return;
}

/**
 * Return a spatiotemporal box from its WKB representation
 */
static STBOX *
stbox_from_wkb_state(wkb_parse_state *s)
{
  /* Read the temporal flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  stbox_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);
  else if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    s->srid = SRID_DEFAULT;

  /* Read and create the box */
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  Span *period = NULL;
  if (s->hast)
    period = span_from_wkb_state(s);
  if (s->hasx)
  {
    xmin = double_from_wkb_state(s);
    xmax = double_from_wkb_state(s);
    ymin = double_from_wkb_state(s);
    ymax = double_from_wkb_state(s);
    if (s->hasz)
    {
      zmin = double_from_wkb_state(s);
      zmax = double_from_wkb_state(s);
    }
  }
  STBOX *result = stbox_make(period, s->hasx, s->hasz, s->geodetic, s->srid,
    xmin, xmax, ymin, ymax, zmin, zmax);
  if (s->hast)
    pfree(period);
  return result;
}

/*****************************************************************************/

/**
 * Take in an unknown temporal type of WKB type number and ensure it comes out
 * as an extended WKB temporal type number.
 */
void
temporal_temptype_from_wkb_state(wkb_parse_state *s, uint16_t wkb_temptype)
{
  switch (wkb_temptype)
  {
    case MOBDB_WKB_T_TBOOL:
      s->temptype = T_TBOOL;
      break;
    case MOBDB_WKB_T_TINT:
      s->temptype = T_TINT;
      break;
    case MOBDB_WKB_T_TFLOAT:
      s->temptype = T_TFLOAT;
      break;
    case MOBDB_WKB_T_TTEXT:
      s->temptype = T_TTEXT;
      break;
    case MOBDB_WKB_T_TGEOMPOINT:
      s->temptype = T_TGEOMPOINT;
      break;
    case MOBDB_WKB_T_TGEOGPOINT:
      s->temptype = T_TGEOGPOINT;
      break;
#if NPOINT
    case MOBDB_WKB_T_TNPOINT:
      s->temptype = T_TNPOINT;
      break;
#endif /* NPOINT */
    default: /* Error! */
      elog(ERROR, "Unknown WKB temporal type: %d", wkb_temptype);
      break;
  }
  s->basetype = temptype_basetype(s->temptype);
  return;
}

/**
 * Take in an unknown kind of WKB type number and ensure it comes out as an
 * extended WKB type number (with Z/GEODETIC/SRID/LINEAR_INTERP flags masked
 * onto the high bits).
 */
void
temporal_flags_from_wkb_state(wkb_parse_state *s, uint8_t wkb_flags)
{
  s->hasx = true;
  s->hast = true;
  s->hasz = false;
  s->geodetic = false;
  s->has_srid = false;
  /* Get the interpolation */
  s->interp = MOBDB_WKB_GET_INTERP(wkb_flags);
  /* Get the flags */
  if (tgeo_type(s->temptype))
  {
    if (wkb_flags & MOBDB_WKB_ZFLAG)
      s->hasz = true;
    if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
      s->geodetic = true;
    if (wkb_flags & MOBDB_WKB_SRIDFLAG)
      s->has_srid = true;
  }
  /* Mask off the upper flags to get the subtype */
  wkb_flags &= (uint8_t) 0x03;
  switch (wkb_flags)
  {
    case MOBDB_WKB_TINSTANT:
      s->subtype = TINSTANT;
      break;
    case MOBDB_WKB_TSEQUENCE:
      s->subtype = TSEQUENCE;
      break;
    case MOBDB_WKB_TSEQUENCESET:
      s->subtype = TSEQUENCESET;
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB flags: %d", wkb_flags);
      break;
  }
  return;
}

/**
 * Return a value from its WKB representation.
 */
static Datum
temporal_basevalue_from_wkb_state(wkb_parse_state *s)
{
  Datum result;
  ensure_temporal_basetype(s->basetype);
  switch (s->temptype)
  {
    case T_TBOOL:
      result = BoolGetDatum(byte_from_wkb_state(s));
      break;
    case T_TINT:
      result = Int32GetDatum(int32_from_wkb_state(s));
      break;
#if 0 /* not used */
    case T_TINT8:
      result = Int64GetDatum(int64_from_wkb_state(s));
      break;
#endif /* not used */
    case T_TFLOAT:
      result = Float8GetDatum(double_from_wkb_state(s));
      break;
    case T_TTEXT:
      result = PointerGetDatum(text_from_wkb_state(s));
      break;
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
      result = point_from_wkb_state(s);
      break;
#if NPOINT
    case T_TNPOINT:
      result = PointerGetDatum(npoint_from_wkb_state(s));
      break;
#endif /* NPOINT */
    default: /* Error! */
      elog(ERROR, "Unknown temporal type: %d",
        s->temptype);
      break;
  }
  return result;
}

/**
 * @brief Return a temporal instant point from its WKB representation.
 *
 * It reads the base type value and the timestamp and advances the parse state
 * forward appropriately.
 * @note It starts reading it just after the endian byte, the temporal type
 * int16, and the temporal flags byte.
 */
static TInstant *
tinstant_from_wkb_state(wkb_parse_state *s)
{
  /* Read the values from the buffer and create the instant */
  Datum value = temporal_basevalue_from_wkb_state(s);
  TimestampTz t = timestamp_from_wkb_state(s);
  TInstant *result = tinstant_make(value, s->temptype, t);
  if (! basetype_byvalue(s->basetype))
    pfree(DatumGetPointer(value));
  return result;
}

/**
 * Return a temporal instant array from its WKB representation
 */
static TInstant **
tinstarr_from_wkb_state(wkb_parse_state *s, int count)
{
  TInstant **result = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    /* Parse the point and the timestamp to create the instant point */
    Datum value = temporal_basevalue_from_wkb_state(s);
    TimestampTz t = timestamp_from_wkb_state(s);
    result[i] = tinstant_make(value, s->temptype, t);
    if (! basetype_byvalue(s->basetype))
      pfree(DatumGetPointer(value));
  }
  return result;
}

/**
 * Return a temporal sequence value from its WKB representation
 */
static TSequence *
tsequence_from_wkb_state(wkb_parse_state *s)
{
  /* Get the number of instants */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Get the period bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
  /* Parse the instants */
  TInstant **instants = tinstarr_from_wkb_state(s, count);
  return tsequence_make_free(instants, count, count, lower_inc, upper_inc,
    s->interp, NORMALIZE);
}

/**
 * Return a temporal sequence set value from its WKB representation
 */
static TSequenceSet *
tsequenceset_from_wkb_state(wkb_parse_state *s)
{
  /* Get the number of sequences */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Parse the sequences */
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    /* Get the number of instants */
    int countinst = int32_from_wkb_state(s);
    /* Get the period bounds */
    uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
    bool lower_inc, upper_inc;
    bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
    /* Parse the instants */
    TInstant **instants = palloc(sizeof(TInstant *) * countinst);
    for (int j = 0; j < countinst; j++)
    {
      /* Parse the value and the timestamp to create the temporal instant */
      Datum value = temporal_basevalue_from_wkb_state(s);
      TimestampTz t = timestamp_from_wkb_state(s);
      instants[j] = tinstant_make(value, s->temptype, t);
      if (! basetype_byvalue(s->basetype))
        pfree(DatumGetPointer(value));
    }
    sequences[i] = tsequence_make_free(instants, countinst, countinst,
      lower_inc, upper_inc, s->interp, NORMALIZE);
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * Return a temporal value from its WKB representation
 */
static Temporal *
temporal_from_wkb_state(wkb_parse_state *s)
{
  /* Read the temporal type */
  uint16_t wkb_temptype = (uint16_t) int16_from_wkb_state(s);
  temporal_temptype_from_wkb_state(s, wkb_temptype);

  /* Read the temporal and interpolation flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  temporal_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);
  else if (wkb_flags & MOBDB_WKB_GEODETICFLAG)
    s->srid = SRID_DEFAULT;

  /* Read the temporal value */
  ensure_valid_tempsubtype(s->subtype);
  if (s->subtype == TINSTANT)
    return (Temporal *) tinstant_from_wkb_state(s);
  else if (s->subtype == TSEQUENCE)
    return (Temporal *) tsequence_from_wkb_state(s);
  else /* s->subtype == TSEQUENCESET */
    return (Temporal *) tsequenceset_from_wkb_state(s);
}

/*****************************************************************************/

/**
 * @brief Return a value from its Well-Known Binary (WKB) representation.
 */
Datum
datum_from_wkb(const uint8_t *wkb, int size, mobdbType type)
{
  /* Initialize the state appropriately */
  wkb_parse_state s;
  memset(&s, 0, sizeof(wkb_parse_state));
  s.wkb = s.pos = wkb;
  s.wkb_size = size;
  /* Fail when handed incorrect starting byte */
  char wkb_little_endian = byte_from_wkb_state(&s);
  if (wkb_little_endian != 1 && wkb_little_endian != 0)
    elog(ERROR, "Invalid endian flag value encountered.");

  /* Check the endianness of our input */
  s.swap_bytes = false;
  /* Machine arch is big endian, request is for little */
  if (MOBDB_IS_BIG_ENDIAN && wkb_little_endian)
    s.swap_bytes = true;
  /* Machine arch is little endian, request is for big */
  else if ((! MOBDB_IS_BIG_ENDIAN) && (! wkb_little_endian))
    s.swap_bytes = true;

  /* Call the type-specific function */
  Datum result;
  switch (type)
  {
    case T_INTSPAN:
    case T_FLOATSPAN:
    case T_PERIOD:
      result = PointerGetDatum(span_from_wkb_state(&s));
      break;
    case T_TIMESTAMPSET:
      result = PointerGetDatum(timestampset_from_wkb_state(&s));
      break;
    case T_PERIODSET:
      result = PointerGetDatum(periodset_from_wkb_state(&s));
      break;
    case T_TBOX:
      result = PointerGetDatum(tbox_from_wkb_state(&s));
      break;
    case T_STBOX:
      result = PointerGetDatum(stbox_from_wkb_state(&s));
      break;
    case T_TBOOL:
    case T_TINT:
    case T_TFLOAT:
    case T_TTEXT:
    case T_TGEOMPOINT:
    case T_TGEOGPOINT:
#if NPOINT
    case T_TNPOINT:
#endif /* NPOINT */
      result = PointerGetDatum(temporal_from_wkb_state(&s));
      break;
    default: /* Error! */
      elog(ERROR, "Unknown WKB type: %d", type);
      break;
  }

  return result;
}

/**
 * @brief Return a temporal type from its HexEWKB representation
 */
Datum
datum_from_hexwkb(const char *hexwkb, int size, mobdbType type)
{
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, size);
  Datum result = datum_from_wkb(wkb, size / 2, type);
  pfree(wkb);
  return result;
}

/*****************************************************************************
 * WKB and HexWKB functions for the MEOS API
 *****************************************************************************/

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a span from its Well-Known Binary (WKB)
 * representation.
 * @sqlfunc intspanFromBinary(), floatspanFromBinary(), periodFromBinary(),
 */
Span *
span_from_wkb(const uint8_t *wkb, int size)
{
  /* We pass ANY span type to the dispatch function but the actual span type
   * will be read from the byte string */
  return DatumGetSpanP(datum_from_wkb(wkb, size, T_INTSPAN));
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a span from its WKB representation in hex-encoded ASCII.
 * @sqlfunc intspanFromHexWKB(), floatspanFromHexWKB(), periodFromHexWKB(),
 */
Span *
span_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  /* We pass ANY span type to the dispatch function but the actual span type
   * will be read from the byte string */
  return DatumGetSpanP(datum_from_hexwkb(hexwkb, size, T_INTSPAN));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a timestamp set from its Well-Known Binary (WKB)
 * representation.
 * @sqlfunc timestampsetFromBinary()
 */
TimestampSet *
timestampset_from_wkb(const uint8_t *wkb, int size)
{
  return DatumGetTimestampSetP(datum_from_wkb(wkb, size, T_TIMESTAMPSET));
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a timestamp set from its WKB representation in hex-encoded
 * ASCII.
 * @sqlfunc timestampsetFromHexWKB()
 */
TimestampSet *
timestampset_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetTimestampSetP(datum_from_hexwkb(hexwkb, size, T_TIMESTAMPSET));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a period set from its Well-Known Binary (WKB)
 * representation.
 * @sqlfunc periodsetFromBinary()
 */
PeriodSet *
periodset_from_wkb(const uint8_t *wkb, int size)
{
  return DatumGetPeriodSetP(datum_from_wkb(wkb, size, T_PERIODSET));
}

/**
 * @ingroup libmeos_spantime_in_out
 * @brief Return a period set from its WKB representation in hex-encoded ASCII
 * @sqlfunc periodsetFromHexWKB()
 */
PeriodSet *
periodset_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetPeriodSetP(datum_from_hexwkb(hexwkb, size, T_PERIODSET));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_in_out
 * @brief Return a temporal box from its Well-Known Binary (WKB)
 * representation.
 * @sqlfunc tboxFromBinary()
 */
TBOX *
tbox_from_wkb(const uint8_t *wkb, int size)
{
  return DatumGetTboxP(datum_from_wkb(wkb, size, T_TBOX));
}

/**
 * @ingroup libmeos_box_in_out
 * @brief Return a temporal box from its WKB representation in hex-encoded ASCII
 * @sqlfunc tboxFromHexWKB()
 */
TBOX *
tbox_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetTboxP(datum_from_hexwkb(hexwkb, size, T_TBOX));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_box_in_out
 * @brief Return a spatiotemporal box from its Well-Known Binary (WKB)
 * representation.
 * @sqlfunc stboxFromBinary()
 */
STBOX *
stbox_from_wkb(const uint8_t *wkb, int size)
{
  return DatumGetSTboxP(datum_from_wkb(wkb, size, T_STBOX));
}

/**
 * @ingroup libmeos_box_in_out
 * @brief Return a spatiotemporal box from its WKB representation in
 * hex-encoded ASCII
 * @sqlfunc stboxFromWKB()
 */
STBOX *
stbox_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  return DatumGetSTboxP(datum_from_hexwkb(hexwkb, size, T_STBOX));
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal value from its Well-Known Binary (WKB)
 * representation.
 * @sqlfunc tboolFromBinary(), tintFromBinary(), tfloatFromBinary(),
 * ttextFromBinary(), etc.
 */
Temporal *
temporal_from_wkb(const uint8_t *wkb, int size)
{
  /* We pass ANY temporal type to the dispatch function but the actual temporal
   * type will be read from the byte string */
  return DatumGetTemporalP(datum_from_wkb(wkb, size, T_TINT));
}

/**
 * @ingroup libmeos_temporal_in_out
 * @brief Return a temporal value from its HexEWKB representation
 * @sqlfunc tboolFromHexWKB(), tintFromHexWKB(), tfloatFromHexWKB(),
 * ttextFromHexWKB(), etc.
 */
Temporal *
temporal_from_hexwkb(const char *hexwkb)
{
  int size = strlen(hexwkb);
  /* We pass ANY temporal type to the dispatch function but the actual temporal
   * type will be read from the byte string */
  return DatumGetTemporalP(datum_from_hexwkb(hexwkb, size, T_TINT));
}

/*****************************************************************************/
