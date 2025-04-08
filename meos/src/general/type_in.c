/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @file
 * @brief Input of temporal types in WKT, MF-JSON, WKB, EWKB, and HexWKB
 * representation
 */

/* C */
#include <assert.h>
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include "utils/timestamp.h"
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/set.h"
#include "general/span.h"
#include "general/tbox.h"
#include "geo/postgis_funcs.h"
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#if CBUFFER
  #include "cbuffer/cbuffer.h"
#endif
#if NPOINT
  #include "npoint/tnpoint.h"
#endif
#if POSE
  #include <meos_pose.h>
  #include "pose/pose.h"
#endif
#if RGEO
  #include "rgeo/trgeo.h"
#endif

/*****************************************************************************/

/**
 * @brief Structure used for passing the parse state between the parsing
 * functions
 */
typedef struct
{
  const uint8_t *wkb;     /**< Points to start of WKB */
  size_t wkb_size;        /**< Expected size of WKB */
  bool swap_bytes;        /**< Do an endian flip? */
  uint8_t type;           /**< Current type we are handling */
  uint8_t spantype;       /**< Current span type we are handling */
  uint8_t basetype;       /**< Current base type we are handling */
  uint8_t temptype;       /**< Current temporal type we are handling */
  uint8_t subtype;        /**< Current temporal subtype we are handling */
  int32_t srid;           /**< Current SRID we are handling */
  bool ordered;           /**< Is the set ordered? */
  bool hasx;              /**< X? */
  bool hasz;              /**< Z? */
  bool hast;              /**< T? */
  bool geodetic;          /**< Geodetic? */
  bool has_srid;          /**< SRID? */
  interpType interp;      /**< Interpolation */
  const uint8_t *pos;     /**< Current parse position */
} meos_wkb_parse_state;

extern LWGEOM *parse_geojson(json_object *geojson, int *hasz);

/*****************************************************************************
 * Input of base types
 *****************************************************************************/

/**
 * @brief Return a base value from its string representation
 */
bool
#if CBUFFER || NPOINT || POSE || RGEO
basetype_in(const char *str, meosType type, bool end, Datum *result)
#else
basetype_in(const char *str, meosType type,
  bool end __attribute__((unused)), Datum *result)
#endif
{
  assert(meos_basetype(type));
  switch (type)
  {
    case T_TIMESTAMPTZ:
    {
      TimestampTz t = pg_timestamptz_in(str, -1);
      if (t == DT_NOEND)
        return false;
      *result = TimestampTzGetDatum(t);
      return true;
    }
    case T_DATE:
    {
      DateADT d = pg_date_in(str);
      if (d == DATEVAL_NOEND)
        return false;
      *result = DateADTGetDatum(d);
      return true;
    }
    case T_BOOL:
    {
      bool b = bool_in(str);
      if (meos_errno())
        return false;
      *result = BoolGetDatum(b);
      return true;
    }
    case T_INT4:
    {
      int i = int4_in(str);
      if (i == PG_INT32_MAX)
        return false;
      *result = Int32GetDatum(i);
      return true;
    }
    case T_INT8:
    {
      int64 i = int8_in(str);
      if (i == PG_INT64_MAX)
        return false;
      *result = Int64GetDatum(i);
      return true;
    }
    case T_FLOAT8:
    {
      double d = float8_in(str, "double precision", str);
      if (d == DBL_MAX)
        return false;
      *result = Float8GetDatum(d);
      return true;
    }
    case T_TEXT:
    {
      text *txt = cstring2text(str);
      if (! txt)
        return false;
      *result = PointerGetDatum(txt);
      return true;
    }
    case T_GEOMETRY:
    {
      GSERIALIZED *gs = geom_in(str, -1);
      if (! gs)
        return false;
      *result = PointerGetDatum(gs);
      return true;
    }
    case T_GEOGRAPHY:
    {
      GSERIALIZED *gs = geog_in(str, -1);
      if (! gs)
        return false;
      *result = PointerGetDatum(gs);
      return true;
    }
#if CBUFFER
    case T_CBUFFER:
    {
      Cbuffer *cbuf = cbuffer_parse(&str, end);
      if (! cbuf)
        return false;
      *result = PointerGetDatum(cbuf);
      return true;
    }
#endif
#if NPOINT
    case T_NPOINT:
    {
      Npoint *np = npoint_parse(&str, end);
      if (! np)
        return false;
      *result = PointerGetDatum(np);
      return true;
    }
#endif
#if POSE || RGEO
    case T_POSE:
    {
      Pose *pose = pose_parse(&str, end);
      if (! pose)
        return false;
      *result = PointerGetDatum(pose);
      return true;
    }
#endif
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_INTERNAL_TYPE_ERROR,
        "Unknown input function for type: %s", meostype_name(type));
      return false;
  }
}

/*****************************************************************************
 * Input in MF-JSON representation
 *****************************************************************************/

/**
 * @brief Return the JSON member corresponding to the name
 * @note Function taken from PostGIS file lwin_geojson.c
 */
static json_object *
findMemberByName(json_object *poObj, const char *pszName)
{
  json_object *poTmp = poObj;
  json_object_iter it;

  if (pszName == NULL || poObj == NULL)
    return NULL;

  it.key = NULL;
  it.val = NULL;
  it.entry = NULL;

  if (json_object_get_object(poTmp))
  {
    if (! json_object_get_object(poTmp)->head)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT, "Invalid MFJSON string");
      return NULL;
    }
    for (it.entry = json_object_get_object(poTmp)->head;
        ( it.entry ?
          ( it.key = (char *) it.entry->k,
          it.val = (json_object *) it.entry->v, it.entry) : 0);
        it.entry = it.entry->next)
    {
      if (pg_strcasecmp(it.key, pszName) == 0)
        return it.val;
    }
  }
  return NULL;
}

/**
 * @brief Return a single point from its MF-JSON coordinates
 * @details In this case the coordinate array is a single array of cordinations
 * such as `"coordinates":[1,1]`.
 */
static Datum
parse_mfjson_coord(json_object *poObj, int srid, bool geodetic)
{
  if (json_type_array != json_object_get_type(poObj))
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of the 'coordinates' array in MFJSON string");
    return 0;
  }
  int ncoord = (int) json_object_array_length(poObj);
  if (ncoord < 2)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Too few elements in 'coordinates' values in MFJSON string");
    return 0;
  }
  if (ncoord > 3)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Too many elements in 'coordinates' values in MFJSON string");
    return 0;
  }

  double x, y;
  json_object *poObjCoord = NULL;

  /* Read X coordinate */
  poObjCoord = json_object_array_get_idx(poObj, 0);
  x = json_object_get_double(poObjCoord);

  /* Read Y coordinate */
  poObjCoord = json_object_array_get_idx(poObj, 1);
  y = json_object_get_double(poObjCoord);

  LWPOINT *point;
  if (ncoord == 3)
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
 * @brief Return an array of values from its MF-JSON representation
 */
static Datum *
parse_mfjson_values(json_object *mfjson, meosType temptype, int *count)
{
  json_object *mfjsonTmp = mfjson;
  json_object *jvalues = NULL;
  jvalues = findMemberByName(mfjsonTmp, "values");
  if (jvalues == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'values' in MFJSON string");
    return NULL;
  }
  if (json_object_get_type(jvalues) != json_type_array)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'values' array in MFJSON string");
    return NULL;
  }
  int nvalues = (int) json_object_array_length(jvalues);
  if (nvalues < 1)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of 'values' array in MFJSON string");
    return NULL;
  }
  Datum *values = palloc(sizeof(Datum) * nvalues);
  for (int i = 0; i < nvalues; ++i)
  {
    json_object *jvalue = NULL;
    jvalue = json_object_array_get_idx(jvalues, i);
    switch (temptype)
    {
      case T_TBOOL:
        if (json_object_get_type(jvalue) != json_type_boolean)
        {
          meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
            "Invalid boolean value in 'values' array in MFJSON string");
          return NULL;
        }
        values[i] = BoolGetDatum(json_object_get_boolean(jvalue));
        break;
      case T_TINT:
        if (json_object_get_type(jvalue) != json_type_int)
        {
          meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
            "Invalid integer value in 'values' array in MFJSON string");
          return NULL;
        }
        values[i] = Int32GetDatum(json_object_get_int(jvalue));
        break;
      case T_TFLOAT:
        values[i] = Float8GetDatum(json_object_get_double(jvalue));
        break;
      case T_TTEXT:
        if (json_object_get_type(jvalue) != json_type_string)
        {
          meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
            "Invalid string value in 'values' array in MFJSON string");
          return NULL;
        }
        values[i] = PointerGetDatum(cstring2text(json_object_get_string(jvalue)));
        break;
      default: /* Error! */
        meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
          "Unknown temporal type in MFJSON string: %s",
          meostype_name(temptype));
        return NULL;
    }
  }
  *count = nvalues;
  return values;
}

/* TODO MAKE POSSIBLE TO CALL THIS FUNCTION */
/**
 * @brief Return an array of points from its MF-JSON coordinates
 * @details In this case the coordinate array is an array of arrays of
 * cordinates such as `"values":[1.5,2.5]`.
 */
static Datum *
parse_mfjson_points(json_object *mfjson, int srid, bool geodetic, int *count)
{
  json_object *mfjsonTmp = mfjson;
  json_object *coordinates = NULL;
  coordinates = findMemberByName(mfjsonTmp, "coordinates");
  if (coordinates == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'coordinates' in MFJSON string");
    return NULL;
  }
  if (json_object_get_type(coordinates) != json_type_array)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'coordinates' array in MFJSON string");
    return NULL;
  }

  int npoints = (int) json_object_array_length(coordinates);
  if (npoints < 1)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of 'coordinates' array in MFJSON string");
    return NULL;
  }

  Datum *values = palloc(sizeof(Datum) * npoints);
  for (int i = 0; i < npoints; ++i)
  {
    json_object *coords = json_object_array_get_idx(coordinates, i);
    values[i] = parse_mfjson_coord(coords, srid, geodetic);
  }
  *count = npoints;
  return values;
}

/**
 * @brief Return an array of geometries/geographies from their MF-JSON
 * representation
 */
static Datum *
parse_mfjson_geos(json_object *mfjson, int srid, bool geodetic, int *count)
{
  json_object *mfjsonTmp = mfjson;
  json_object *values_json = NULL;
  values_json = findMemberByName(mfjsonTmp, "values");
  if (values_json == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'values' in MFJSON string");
    return NULL;
  }
  if (json_object_get_type(values_json) != json_type_array)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'values' array in MFJSON string");
    return NULL;
  }

  int ngeos = (int) json_object_array_length(values_json);
  if (ngeos < 1)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of 'values' array in MFJSON string");
    return NULL;
  }

  Datum *values = palloc(sizeof(Datum) * ngeos);
  for (int i = 0; i < ngeos; ++i)
  {
    json_object *geo_json = json_object_array_get_idx(values_json, i);
    int hasz = LW_FALSE;
    LWGEOM *geo = parse_geojson(geo_json, &hasz);
    if (! geo)
    {
      pfree(values);
      return NULL;
    }
    if (! hasz)
    {
      LWGEOM *tmp = lwgeom_force_2d(geo);
      lwgeom_free(geo);
      geo = tmp;
    }
    lwgeom_add_bbox(geo);
    lwgeom_set_srid(geo, srid);
    lwgeom_set_geodetic(geo, geodetic);
    values[i] = PointerGetDatum(geo_serialize(geo));
    lwgeom_free(geo);
  }
  *count = ngeos;
  return values;
}

#if RGEO
/**
 * @brief Return a reference geometry of a temporal rigid geometry from its
 * MF-JSON representation
 */
static GSERIALIZED *
parse_mfjson_ref_geo(json_object *mfjson, int srid, bool geodetic)
{
  json_object *mfjsonTmp = mfjson;
  json_object *geo_json = NULL;
  geo_json = findMemberByName(mfjsonTmp, "geometry");
  if (geo_json == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'geometry' in MFJSON string");
    return NULL;
  }
  int hasz = LW_FALSE;
  LWGEOM *geo = parse_geojson(geo_json, &hasz);
  if (! geo)
    return NULL;
  if (! hasz)
  {
    LWGEOM *tmp = lwgeom_force_2d(geo);
    lwgeom_free(geo);
    geo = tmp;
  }
  lwgeom_add_bbox(geo);
  lwgeom_set_srid(geo, srid);
  lwgeom_set_geodetic(geo, geodetic);
  GSERIALIZED *result = geo_serialize(geo);
  lwgeom_free(geo);
  return result;
}
#endif /* RGEO */

/*****************************************************************************/

#if POSE || RGEO
/**
 * @brief Return a pose from its GeoJSON representation
 */Pose *
parse_mfjson_pose(json_object *mfjson, int srid)
{
  assert(mfjson);
  /* Determine if the pose is 2D or 3D depending on whether there is an 
   * member "quaternion" which implies 3D */
  json_object *quaternion = NULL;
  quaternion = findMemberByName(mfjson, "quaternion");
  bool hasZ = (quaternion == NULL) ? false : true;

  /* Get position */
  json_object *position = NULL;
  position = findMemberByName(mfjson, "position");
  if (position == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'position' in MFJSON string");
    return NULL;
  }
  double x, y, z, w_q, x_q, y_q, z_q;
  json_object *lat = NULL;
  lat = findMemberByName(position, "lat");
  if (lat == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'lat' in MFJSON string");
    return NULL;
  }
  y = json_object_get_double(lat);
  json_object *lon = NULL;
  lon = findMemberByName(position, "lon");
  if (lon == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'lon' in MFJSON string");
    return NULL;
  }
  x = json_object_get_double(lon);
  json_object *h = NULL;
  if (hasZ)
  {
    h = findMemberByName(position, "h");
    if (h == NULL)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unable to find 'h' in MFJSON string");
      return NULL;
    }
    z = json_object_get_double(h);
  }

  /* Get rotation/quaternion */
  if (hasZ)
  {
    /* The "quaternion" member has been already found when determining 2D/3D */
    json_object *x_mfjson = NULL;
    x_mfjson = findMemberByName(quaternion, "x");
    if (x_mfjson == NULL)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unable to find 'x' in MFJSON string");
      return NULL;
    }
    x_q = json_object_get_double(x_mfjson);
    json_object *y_mfjson = NULL;
    y_mfjson = findMemberByName(quaternion, "y");
    if (y_mfjson == NULL)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unable to find 'y' in MFJSON string");
      return NULL;
    }
    y_q = json_object_get_double(y_mfjson);
    json_object *z_mfjson = NULL;
    z_mfjson = findMemberByName(quaternion, "z");
    if (z_mfjson == NULL)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unable to find 'z' in MFJSON string");
      return NULL;
    }
    z_q = json_object_get_double(z_mfjson);
    json_object *w_mfjson = NULL;
    w_mfjson = findMemberByName(quaternion, "w");
    if (w_mfjson == NULL)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unable to find 'w' in MFJSON string");
      return NULL;
    }
    w_q = json_object_get_double(w_mfjson);
  }
  else
  {
    json_object *rotation = NULL;
    rotation = findMemberByName(mfjson, "rotation");
    if (rotation == NULL)
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unable to find 'rotation' in MFJSON string");
      return NULL;
    }
    z = json_object_get_double(rotation);
  }
  Pose *result = hasZ ? pose_make_3d(x, y, z, w_q, x_q, y_q, z_q, srid) :
    pose_make_2d(x, y, z, srid);
  return result;
}

/**
 * @brief Return an array of poses from its GeoJSON pose values
 */
static Datum *
parse_mfjson_poses(json_object *mfjson, int srid, int *count)
{
  json_object *mfjsonTmp = mfjson;
  json_object *values_json = NULL;
  values_json = findMemberByName(mfjsonTmp, "values");
  if (values_json == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'values' in MFJSON string");
    return NULL;
  }
  if (json_object_get_type(values_json) != json_type_array)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'values' array in MFJSON string");
    return NULL;
  }

  int nposes = (int) json_object_array_length(values_json);
  if (nposes < 1)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of 'values' array in MFJSON string");
    return NULL;
  }

  Datum *values = palloc(sizeof(Datum) * nposes);
  for (int i = 0; i < nposes; ++i)
  {
    json_object *pose = json_object_array_get_idx(values_json, i);
    values[i] = PointerGetDatum(parse_mfjson_pose(pose, srid));
  }
  *count = nposes;
  return values;
}
#endif /* POSE */

/*****************************************************************************/

/**
 * @brief Return an array of timestamps from their MF-JSON datetimes values
 */
static TimestampTz *
parse_mfjson_datetimes(json_object *mfjson, int *count)
{
  json_object *datetimes = NULL;
  datetimes = findMemberByName(mfjson, "datetimes");
  if (datetimes == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'datetimes' in MFJSON string");
    return NULL;
  }
  if (json_object_get_type(datetimes) != json_type_array)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'datetimes' array in MFJSON string");
    return NULL;
  }

  int ndates = (int) json_object_array_length(datetimes);
  if (ndates < 1)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of 'datetimes' array in MFJSON string");
    return NULL;
  }

  TimestampTz *times = palloc(sizeof(TimestampTz) * ndates);
  for (int i = 0; i < ndates; i++)
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
  *count = ndates;
  return times;
}

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal instant from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] spatial True when the input value is a geometry/geography
 * @param[in] srid SRID
 * @param[in] temptype Temporal type
 */
TInstant *
tinstant_from_mfjson(json_object *mfjson, bool spatial, int srid,
  meosType temptype)
{
  assert(mfjson); assert(temporal_type(temptype));
  bool geodetic = tgeodetic_type(temptype);
  /* Get coordinates and datetimes */
  int nvalues = 0, ndates = 0;
  Datum *values;
  if (! spatial)
    values = parse_mfjson_values(mfjson, temptype, &nvalues);
  else
  {
    if (tpoint_type(temptype))
      values = parse_mfjson_points(mfjson, srid, geodetic, &nvalues);
    else if (tgeo_type(temptype))
      values = parse_mfjson_geos(mfjson, srid, geodetic, &nvalues);
#if POSE || RGEO
    else if (temptype == T_TPOSE || temptype == T_TRGEOMETRY)
      values = parse_mfjson_poses(mfjson, srid, &nvalues);
#endif /* POSE */
    else
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unknown spatial type for MF-JSON input function: %s", 
        meostype_name(temptype));
      return NULL;
    }
  }
  TimestampTz *times = parse_mfjson_datetimes(mfjson, &ndates);
  if (nvalues != 1 || ndates != 1)
  {
    pfree(values); pfree(times);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid number of elements in '%s' and/or 'datetimes' arrays",
      ! tpoint_type(temptype) ? "values" : "coordinates");
    return NULL;
  }
  TInstant *result = tinstant_make_free(values[0], temptype, times[0]);
  pfree(values); pfree(times);
  return result;
}

/**
 * @brief Return an array of temporal instants from their MF-JSON
 * representation
 */
static TInstant **
tinstarr_from_mfjson(json_object *mfjson, bool isgeo, int srid,
  meosType temptype, int *count)
{
  assert(mfjson); assert(count);
  bool geodetic = tgeodetic_type(temptype);
  /* Get coordinates and datetimes */
  int nvalues = 0, ndates = 0;
  Datum *values;
  if (! isgeo)
    values = parse_mfjson_values(mfjson, temptype, &nvalues);
  else
  {
    if (tpoint_type(temptype))
      values = parse_mfjson_points(mfjson, srid, geodetic, &nvalues);
    else if (tgeo_type(temptype))
      values = parse_mfjson_geos(mfjson, srid, geodetic, &nvalues);
#if POSE || RGEO
    else if (temptype == T_TPOSE || temptype == T_TRGEOMETRY)
      values = parse_mfjson_poses(mfjson, srid, &nvalues);
#endif /* RGEO */
   else
    {
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Unknown spatial type for MF-JSON input function: %s", 
        meostype_name(temptype));
      return NULL;
    }
  }
  TimestampTz *times = parse_mfjson_datetimes(mfjson, &ndates);
  if (nvalues != ndates)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Distinct number of elements in '%s' and 'datetimes' arrays",
      ! isgeo ? "values" : "coordinates");
    return NULL;
  }

  /* Construct the array of temporal point instants */
  TInstant **result = palloc(sizeof(TInstant *) * nvalues);
  for (int i = 0; i < nvalues; i++)
    result[i] = tinstant_make_free(values[i], temptype, times[i]);

  pfree(values); pfree(times);
  *count = nvalues;
  return result;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] spatial True when the input value is a spatial value
 * @param[in] srid SRID
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 */
TSequence *
tsequence_from_mfjson(json_object *mfjson, bool spatial, int srid,
  meosType temptype, interpType interp)
{
  assert(mfjson);
  /* Get the array of temporal point instants */
  int count = 0;
  TInstant **instants = tinstarr_from_mfjson(mfjson, spatial, srid, temptype,
    &count);

  /* Get lower bound flag, default to true if not specified */
  bool lower_inc = true;
  json_object *lowerinc = findMemberByName(mfjson, "lower_inc");
  if (lowerinc)
  {
    if (json_object_get_type(lowerinc) != json_type_boolean)
      meos_error(WARNING, MEOS_ERR_MFJSON_INPUT,
        "Type of 'lower_inc' value in MFJSON string is not boolean, defaulting to true");
    else
      lower_inc = (bool) json_object_get_boolean(lowerinc);
  }

  /* Get upper bound flag, default to true if not specified */
  bool upper_inc = true;
  json_object *upperinc = findMemberByName(mfjson, "upper_inc");
  if (upperinc)
  {
    if (json_object_get_type(upperinc) != json_type_boolean)
      meos_error(WARNING, MEOS_ERR_MFJSON_INPUT,
        "Type of 'upper_inc' value in MFJSON string is not boolean, defaulting to true");
    else
      upper_inc = (bool) json_object_get_boolean(upperinc);
  }

  /* Construct the temporal sequence */
  return tsequence_make_free(instants, count, lower_inc, upper_inc, interp,
    NORMALIZE);
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal sequence set from its MF-JSON representation
 * @param[in] mfjson MFJSON object
 * @param[in] spatial True when the input value is a geometry/geography
 * @param[in] srid SRID
 * @param[in] temptype Temporal type
 * @param[in] interp Interpolation
 */
TSequenceSet *
tsequenceset_from_mfjson(json_object *mfjson, bool spatial, int srid,
  meosType temptype, interpType interp)
{
  assert(mfjson);
  json_object *seqs = NULL;
  seqs = findMemberByName(mfjson, "sequences");
  /* We don't need to test that seqs is NULL since to differentiate between
   * a sequence and a sequence set we look for the "sequences" member and
   * then call this function */
  if (json_object_get_type(seqs) != json_type_array)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'sequences' array in MFJSON string");
    return NULL;
  }
  int nseqs = (int) json_object_array_length(seqs);
  if (nseqs < 1)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid value of 'sequences' array in MFJSON string");
    return NULL;
  }

  /* Construct the temporal point */
  TSequence **sequences = palloc(sizeof(TSequence *) * nseqs);
  for (int i = 0; i < nseqs; i++)
  {
    json_object* seqvalue = NULL;
    seqvalue = json_object_array_get_idx(seqs, i);
    sequences[i] = tsequence_from_mfjson(seqvalue, spatial, srid, temptype,
      interp);
  }
  return tsequenceset_make_free(sequences, nseqs, NORMALIZE);
}

/*****************************************************************************/

/**
 * @brief Return true if a string containts an MF-JSON type
 * @param[in] typestr MFJSON type string
 */

static bool
ensure_temptype_mfjson(const char *typestr)
{
  if (strcmp(typestr, "MovingBoolean") != 0 &&
      strcmp(typestr, "MovingInteger") != 0 &&
      strcmp(typestr, "MovingFloat") != 0 &&
      strcmp(typestr, "MovingText") != 0 &&
      strcmp(typestr, "MovingPoint") != 0 &&
      strcmp(typestr, "MovingGeometry") != 0  &&
      strcmp(typestr, "MovingPose") != 0 &&
      strcmp(typestr, "MovingRigidGeometry") != 0 )
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'type' value in MFJSON string: %s", typestr);
    return false;
  }
  return true;
}

/**
 * @ingroup meos_internal_temporal_inout
 * @brief Return a temporal object from its MF-JSON representation
 * @param[in] mfjson MFJSON string
 * @param[in] temptype expected temporal type
 * @return On error return @p NULL
 * @see #tinstant_from_mfjson()
 * @see #tsequence_from_mfjson()
 * @see #tsequenceset_from_mfjson()
 */
Temporal *
temporal_from_mfjson(const char *mfjson, meosType temptype)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(mfjson, NULL);

  /* Begin to parse json */
  json_tokener *jstok = json_tokener_new();
  json_object *poObj = json_tokener_parse_ex(jstok, mfjson, -1);
  if (jstok->err != json_tokener_success)
  {
    char err[256];
    snprintf(err, sizeof(err), "%s (at offset %d)",
      json_tokener_error_desc(jstok->err), jstok->char_offset);
    json_tokener_free(jstok);
    json_object_put(poObj);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Error while processing MFJSON string %s", err);
    return NULL;
  }
  json_tokener_free(jstok);

  /*
   * Ensure that it is a moving type
   */
  json_object *poObjType = findMemberByName(poObj, "type");
  if (poObjType == NULL)
  {
    json_object_put(poObj);
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'type' in MFJSON string");
    return NULL;
  }

  /* Determine the type of temporal type */
  const char *typestr = json_object_get_string(poObjType);
  meosType jtemptype = T_UNKNOWN;
  if (! ensure_temptype_mfjson(typestr))
    return NULL;
  if (strcmp(typestr, "MovingBoolean") == 0)
    jtemptype = T_TBOOL;
  else if (strcmp(typestr, "MovingInteger") == 0)
    jtemptype = T_TINT;
  else if (strcmp(typestr, "MovingFloat") == 0)
    jtemptype = T_TFLOAT;
  else if (strcmp(typestr, "MovingText") == 0)
    jtemptype = T_TTEXT;
  else if (strcmp(typestr, "MovingPoint") == 0)
  {
    if (temptype == T_TGEOGPOINT)
      jtemptype = T_TGEOGPOINT;
    else /* Default to T_TGEOMPOINT */
      jtemptype = T_TGEOMPOINT;
  }
  else if (strcmp(typestr, "MovingGeometry") == 0)
  {
    if (temptype == T_TGEOGRAPHY)
      jtemptype = T_TGEOGRAPHY;
    else /* Default to T_TGEOMETRY */
      jtemptype = T_TGEOMETRY;
  }
  else if (strcmp(typestr, "MovingPose") == 0)
    jtemptype = T_TPOSE;
  else if (strcmp(typestr, "MovingRigidGeometry") == 0)
    jtemptype = T_TRGEOMETRY;

  if (temptype != T_UNKNOWN && jtemptype != temptype)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Invalid 'type' value in MFJSON string, expected: %s, received: %s",
      meostype_name(temptype), meostype_name(jtemptype));
    return NULL;
  }

  if (temptype == T_UNKNOWN)
    temptype = jtemptype;

  /*
   * Determine interpolation type
   */
  json_object *poObjInterp = findMemberByName(poObj, "interpolation");
  if (poObjInterp == NULL)
  {
    meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
      "Unable to find 'interpolation' in MFJSON string");
    return NULL;
  }

  const char *pszName = NULL;
  int srid = 0;
  bool spatial = tspatial_type(temptype);
  if (spatial)
  {
    /* Parse crs and set SRID of temporal spatial value */
    json_object *poObjSrs = findMemberByName(poObj, "crs");
    if (poObjSrs)
    {
      json_object *poObjSrsType = findMemberByName(poObjSrs, "type");
      if (poObjSrsType)
      {
        json_object *poObjSrsProps = findMemberByName(poObjSrs, "properties");
        if (poObjSrsProps)
        {
          json_object *poNameURL = findMemberByName(poObjSrsProps, "name");
          if (poNameURL)
          {
            pszName = json_object_get_string(poNameURL);
          }
        }
      }
    }
    if (pszName)
    {
      // The following call requires a valid spatial_ref_sys table entry
      // srid = getSRIDbySRS(fcinfo, srs);
      sscanf(pszName, "EPSG:%d", &srid);
    }
    else if (tgeodetic_type(temptype))
      srid = WGS84_SRID;
  }

  /* Read interpolation value and parse the temporal value */
  const char *pszInterp = json_object_get_string(poObjInterp);
  Temporal *result = NULL;
#if RGEO
  GSERIALIZED *gs = NULL;
#endif /* RGEO */
  if (pszInterp)
  {
#if RGEO
    /* Read the reference geometry of a temporal rigid geometry */
    if (temptype == T_TRGEOMETRY)
    {
      gs = parse_mfjson_ref_geo(poObj, srid, false);
      if (! gs)
      {
        meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
          "Invalid 'geometry' value in MFJSON string");
        return NULL;
      }
    }
#endif /* RGEO */
    if (strcmp(pszInterp, "None") == 0)
      result = (Temporal *) tinstant_from_mfjson(poObj, spatial, srid,
        temptype);
    else if (strcmp(pszInterp, "Discrete") == 0)
      result = (Temporal *) tsequence_from_mfjson(poObj, spatial, srid,
        temptype, DISCRETE);
    else if (strcmp(pszInterp, "Step") == 0 ||
      strcmp(pszInterp, "Linear") == 0)
    {
      interpType interp = (strcmp(pszInterp, "Linear") == 0) ? LINEAR : STEP;
      json_object *poObjSeqs = findMemberByName(poObj, "sequences");
      if (poObjSeqs)
        result = (Temporal *) tsequenceset_from_mfjson(poObj, spatial, srid,
          temptype, interp);
      else
        result = (Temporal *) tsequence_from_mfjson(poObj, spatial, srid,
          temptype, interp);
    }
    else
    {
      json_object_put(poObj);
      meos_error(ERROR, MEOS_ERR_MFJSON_INPUT,
        "Invalid 'interpolation' value in MFJSON string");
      return NULL;
    }
  }
  json_object_put(poObj);
#if RGEO
  if (temptype == T_TRGEOMETRY)
    return geo_tpose_to_trgeo(gs, result);
#endif /* RGEO */
  return result;
}

/*****************************************************************************
 * Input in Well-Known Binary (WKB) representation
 * The file type_in.c explains the binary representation
 *****************************************************************************/

/**
 * @brief Check that we are not about to read off the end of the WKB array
 */
static inline void
wkb_parse_state_check(meos_wkb_parse_state *s, size_t next)
{
  if ((s->pos + next) > (s->wkb + s->wkb_size))
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT,
      "WKB structure does not match expected size!");
    return;
  }
}

/**
 * @brief Read a byte and advance the parse state forward
 */
uint8_t
byte_from_wkb_state(meos_wkb_parse_state *s)
{
  uint8_t byte_value = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_BYTE_SIZE);
  /* Get the data */
  byte_value = s->pos[0];
  s->pos += MEOS_WKB_BYTE_SIZE;
  return byte_value;
}

/**
 * @brief Read a 2-byte integer and advance the parse state forward
 */
int16_t
int16_from_wkb_state(meos_wkb_parse_state *s)
{
  int16_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_INT2_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MEOS_WKB_INT2_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MEOS_WKB_INT2_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MEOS_WKB_INT2_SIZE - j - 1];
      ((uint8_t*)(&i))[MEOS_WKB_INT2_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MEOS_WKB_INT2_SIZE;
  return i;
}

/**
 * @brief Read a 4-byte integer and advance the parse state forward
 */
int32_t
int32_from_wkb_state(meos_wkb_parse_state *s)
{
  int32_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_INT4_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MEOS_WKB_INT4_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MEOS_WKB_INT4_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MEOS_WKB_INT4_SIZE - j - 1];
      ((uint8_t*)(&i))[MEOS_WKB_INT4_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MEOS_WKB_INT4_SIZE;
  return i;
}

/**
 * @brief Read an 8-byte integer and advance the parse state forward
 */
int64_t
int64_from_wkb_state(meos_wkb_parse_state *s)
{
  int64_t i = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_INT8_SIZE);
  /* Get the data */
  memcpy(&i, s->pos, MEOS_WKB_INT8_SIZE);
  /* Swap? Copy into a stack-allocated integer. */
  if (s->swap_bytes)
  {
    for (int j = 0; j < MEOS_WKB_INT8_SIZE/2; j++)
    {
      uint8_t tmp = ((uint8_t*)(&i))[j];
      ((uint8_t*)(&i))[j] = ((uint8_t*)(&i))[MEOS_WKB_INT8_SIZE - j - 1];
      ((uint8_t*)(&i))[MEOS_WKB_INT8_SIZE - j - 1] = tmp;
    }
  }
  s->pos += MEOS_WKB_INT8_SIZE;
  return i;
}

/**
 * @brief Read an 8-byte double and advance the parse state forward
 */
double
double_from_wkb_state(meos_wkb_parse_state *s)
{
  double d = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_DOUBLE_SIZE);
  /* Get the data */
  memcpy(&d, s->pos, MEOS_WKB_DOUBLE_SIZE);
  /* Swap? Copy into a stack-allocated double */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MEOS_WKB_DOUBLE_SIZE/2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&d))[i];
      ((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[MEOS_WKB_DOUBLE_SIZE - i - 1];
      ((uint8_t*)(&d))[MEOS_WKB_DOUBLE_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MEOS_WKB_DOUBLE_SIZE;
  return d;
}

/**
 * @brief Read an 8-byte timestamp and advance the parse state forward
 */
DateADT
date_from_wkb_state(meos_wkb_parse_state *s)
{
  int32_t d = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_DATE_SIZE);
  /* Get the data */
  memcpy(&d, s->pos, MEOS_WKB_DATE_SIZE);
  /* Swap? Copy into a stack-allocated timestamp */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MEOS_WKB_DATE_SIZE / 2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&d))[i];
      ((uint8_t*)(&d))[i] = ((uint8_t*)(&d))[MEOS_WKB_DATE_SIZE - i - 1];
      ((uint8_t*)(&d))[MEOS_WKB_DATE_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MEOS_WKB_DATE_SIZE;
  return (DateADT) d;
}

/**
 * @brief Read an 8-byte timestamp and advance the parse state forward
 */
TimestampTz
timestamp_from_wkb_state(meos_wkb_parse_state *s)
{
  int64_t t = 0;
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_TIMESTAMP_SIZE);
  /* Get the data */
  memcpy(&t, s->pos, MEOS_WKB_TIMESTAMP_SIZE);
  /* Swap? Copy into a stack-allocated timestamp */
  if (s->swap_bytes)
  {
    for (int i = 0; i < MEOS_WKB_TIMESTAMP_SIZE / 2; i++)
    {
      uint8_t tmp = ((uint8_t*)(&t))[i];
      ((uint8_t*)(&t))[i] = ((uint8_t*)(&t))[MEOS_WKB_TIMESTAMP_SIZE - i - 1];
      ((uint8_t*)(&t))[MEOS_WKB_TIMESTAMP_SIZE - i - 1] = tmp;
    }
  }
  s->pos += MEOS_WKB_TIMESTAMP_SIZE;
  return (TimestampTz) t;
}

/**
 * @brief Read a text and advance the parse state forward
 */
text *
text_from_wkb_state(meos_wkb_parse_state *s)
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

/*****************************************************************************/

// /**
 // * @brief Return a point from its WKB representation
 // * @note A WKB point has just a set of doubles, with the quantity depending on
 // * the dimension of the point
 // */
// Datum
// point_from_wkb_state(meos_wkb_parse_state *s)
// {
  // double x, y, z = 0; /* make compiler quiet */
  // x = double_from_wkb_state(s);
  // y = double_from_wkb_state(s);
  // if (s->hasz)
    // z = double_from_wkb_state(s);
  // LWPOINT *point = s->hasz ? lwpoint_make3dz(s->srid, x, y, z) :
    // lwpoint_make2d(s->srid, x, y);
  // FLAGS_SET_GEODETIC(point->flags, s->geodetic);
  // Datum result = PointerGetDatum(geo_serialize((LWGEOM *) point));
  // lwpoint_free(point);
  // return result;
// }

/**
 * @brief Structure used in PostGIS for passing the parse state between the
 * parsing functions.
 */
typedef struct
{
  const uint8_t *wkb; /* Points to start of WKB */
  int32_t srid;    /* Current SRID we are handling */
  size_t wkb_size; /* Expected size of WKB */
  int8_t swap_bytes;  /* Do an endian flip? */
  int8_t check;       /* Simple validity checks on geometries */
  int8_t lwtype;      /* Current type we are handling */
  int8_t has_z;       /* Z? */
  int8_t has_m;       /* M? */
  int8_t has_srid;    /* SRID? */
  int8_t error;       /* An error was found (not enough bytes to read) */
  uint8_t depth;      /* Current recursion level (to prevent stack overflows). 
                         Maxes at LW_PARSER_MAX_DEPTH */
  const uint8_t *pos; /* Current parse position */
} wkb_parse_state;

extern LWGEOM* lwgeom_from_wkb_state(wkb_parse_state *s);

/**
 * @brief Read a geo value from its WKB representation
 * @note We cannot call directly lwgeom_from_wkb since we need to know
 * the number of bytes read from the buffer after decoding a geometry
 */
GSERIALIZED *
geo_from_wkb_state(meos_wkb_parse_state *s)
{
  /* PostGIS parse structure, which is different from the MEOS one */
  wkb_parse_state s1;
  /* Initialize the state appropriately */
  s1.wkb = s1.pos = s->pos;
  s1.wkb_size = s->wkb_size - (s->pos - s->wkb);
  s1.swap_bytes = s->swap_bytes;
  s1.check = LW_PARSER_CHECK_ALL;
  s1.lwtype = 0;
  s1.srid = s->srid;
  s1.has_z = s->hasz;
  s1.has_m = LW_FALSE;
  s1.has_srid = s->has_srid;
  s1.error = LW_FALSE;
  s1.depth = 1;

  /* Read for the geometry */
  LWGEOM *geo = lwgeom_from_wkb_state(&s1);
  if (! geo)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT,
      "Unable to parse geometry from WKB");
    return NULL;
  }
  /* Advance the state by the number of bytes read for the geometry */
  s->pos += (s1.pos - s1.wkb);
  /* Create the geometry. We cannot call gserialized_from_lwgeom since it does 
   * not set the geodetic flag. Therefore we need to call the corresponding
   * MEOS function for doing this */
  GSERIALIZED *result = s->geodetic ? 
    geog_serialize(geo) : geom_serialize(geo);
  lwgeom_free(geo);
  return result;
}

#if CBUFFER
/**
 * @brief Read a circular buffer and advance the parse state forward
 */
Cbuffer *
cbuffer_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Does the data we want to read exist? */
  wkb_parse_state_check(s, MEOS_WKB_INT4_SIZE + MEOS_WKB_DOUBLE_SIZE * 3);
  /* Get the data */
  int32_t srid = s->has_srid ? int32_from_wkb_state(s) : SRID_UNKNOWN;
  double x = double_from_wkb_state(s);
  double y = double_from_wkb_state(s);
  double radius = double_from_wkb_state(s);
  GSERIALIZED *gs = geopoint_make(x, y, 0.0, false, false, srid);
  Cbuffer *result = cbuffer_make(gs, radius);
  pfree(gs);
  return result;
}
#endif /* CBUFFER */

#if NPOINT
/**
 * @brief Return the state flags initialized with a byte flag read from the
 * buffer
 */
static void
npoint_flags_from_wkb_state(meos_wkb_parse_state *s, uint8_t wkb_flags)
{
  assert(wkb_flags & MEOS_WKB_XFLAG);
  s->hasx = true;
  s->hasz = false;
  s->hast = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_flags & MEOS_WKB_SRIDFLAG)
    s->has_srid = true;
  return;
}

/**
 * @brief Read a network point and advance the parse state forward
 */
Npoint *
npoint_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Does the data we want to read exist? 
   * Flags + */
  wkb_parse_state_check(s, MEOS_WKB_BYTE_SIZE + MEOS_WKB_INT8_SIZE + 
    MEOS_WKB_DOUBLE_SIZE);
  /* Read the flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  npoint_flags_from_wkb_state(s, wkb_flags);
  /* Read the SRID, if necessary */
  int32_t srid = s->has_srid ? int32_from_wkb_state(s) : SRID_UNKNOWN;
  /* Disable the warning unused variable ‘srid’ */
  if (srid)
  {
    ;
  }
  int64 rid = int64_from_wkb_state(s);
  double pos = double_from_wkb_state(s);
  Npoint *result = palloc(sizeof(Npoint));
  npoint_set(rid, pos, result);
  return result;
}
#endif /* NPOINT */

#if POSE || RGEO
/**
 * @brief Return the state flags initialized with a byte flag read from the
 * buffer
 */
static void
pose_flags_from_wkb_state(meos_wkb_parse_state *s, uint8_t wkb_flags)
{
  assert(wkb_flags & MEOS_WKB_XFLAG);
  s->hasx = true;
  s->hasz = false;
  s->hast = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_flags & MEOS_WKB_ZFLAG)
    s->hasz = true;
  if (wkb_flags & MEOS_WKB_SRIDFLAG)
    s->has_srid = true;
  return;
}

/**
 * @brief Read a pose and advance the parse state forward
 */
Pose *
pose_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Does the data we want to read exist? */
  // wkb_parse_state_check(s, MEOS_WKB_INT8_SIZE + MEOS_WKB_DOUBLE_SIZE);
  /* Read the flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  pose_flags_from_wkb_state(s, wkb_flags);
  /* Read the SRID, if necessary */
  int32_t srid = s->has_srid ? int32_from_wkb_state(s) : SRID_UNKNOWN;
  Pose *result;
  if (s->hasz)
  {
    double x = double_from_wkb_state(s);
    double y = double_from_wkb_state(s);
    double z = double_from_wkb_state(s);
    double W = double_from_wkb_state(s);
    double X = double_from_wkb_state(s);
    double Y = double_from_wkb_state(s);
    double Z = double_from_wkb_state(s);
    result = pose_make_3d(x, y, z, W, X, Y, Z, srid);
  }
  else
  {
    double x = double_from_wkb_state(s);
    double y = double_from_wkb_state(s);
    double theta = double_from_wkb_state(s);
    result = pose_make_2d(x, y, theta, srid);
  }
  return result;
}
#endif /* POSE */

/*****************************************************************************/

/**
 * @brief Return a base value from its WKB representation
 */
static Datum
base_from_wkb_state(meos_wkb_parse_state *s)
{
  switch (s->basetype)
  {
    case T_BOOL:
      return BoolGetDatum(byte_from_wkb_state(s));
    case T_INT4:
      return Int32GetDatum(int32_from_wkb_state(s));
    case T_INT8:
      return Int64GetDatum(int64_from_wkb_state(s));
    case T_FLOAT8:
      return Float8GetDatum(double_from_wkb_state(s));
    case T_DATE:
      return DateADTGetDatum(date_from_wkb_state(s));
    case T_TIMESTAMPTZ:
      return TimestampTzGetDatum(timestamp_from_wkb_state(s));
    case T_TEXT:
      return PointerGetDatum(text_from_wkb_state(s));
    case T_GEOMETRY:
    case T_GEOGRAPHY:
      return PointerGetDatum(geo_from_wkb_state(s));
#if CBUFFER
    case T_CBUFFER:
      return PointerGetDatum(cbuffer_from_wkb_state(s));
#endif /* NPOINT */
#if NPOINT
    case T_NPOINT:
      return PointerGetDatum(npoint_from_wkb_state(s));
#endif /* NPOINT */
#if POSE || RGEO
    case T_POSE:
      return PointerGetDatum(pose_from_wkb_state(s));
#endif /* POSE */
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_WKB_INPUT,
        "Unknown base type in WKB string: %s", meostype_name(s->basetype));
      return 0;
  }
}

/**
 * @brief Return the size of a span base value from its WKB representation
 */
static size_t
span_basevalue_from_wkb_size(meos_wkb_parse_state *s)
{
  assert(span_basetype(s->basetype));
  switch (s->basetype)
  {
    case T_INT4:
      return sizeof(int);
    case T_INT8:
      return sizeof(int64);
    case T_FLOAT8:
      return sizeof(double);
    case T_TIMESTAMPTZ:
      return sizeof(TimestampTz);
  }
  return 0; /* Error */
}

/**
 * @brief Return the bound flags initialized from their WKB representation
 */
static void
bounds_from_wkb_state(uint8_t wkb_bounds, bool *lower_inc, bool *upper_inc)
{
  if (wkb_bounds & MEOS_WKB_LOWER_INC)
    *lower_inc = true;
  else
    *lower_inc = false;
  if (wkb_bounds & MEOS_WKB_UPPER_INC)
    *upper_inc = true;
  else
    *upper_inc = false;
  return;
}

/**
 * @brief Return a span from its WKB representation when reading components
 * spans in a span set (which does not repeat the spantype for every component
 * (iterator function)
 */
static void
span_from_wkb_state_iter(meos_wkb_parse_state *s, Span *result)
{
  /* Read the span bounds */
  uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
  bool lower_inc, upper_inc;
  bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);

  /* Does the data we want to read exist? */
  size_t size = 2 * span_basevalue_from_wkb_size(s);
  wkb_parse_state_check(s, size);

  /* Read the values and create the span */
  Datum lower = base_from_wkb_state(s);
  Datum upper = base_from_wkb_state(s);
  span_set(lower, upper, lower_inc, upper_inc, s->basetype, s->spantype,
    result);
  return;
}

/**
 * @brief Return a span from its WKB representation
 */
Span
span_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the span type */
  uint16_t wkb_spantype = int16_from_wkb_state(s);
  s->spantype = (uint8_t) wkb_spantype;
  s->basetype = spantype_basetype(wkb_spantype);
  Span result;
  span_from_wkb_state_iter(s, &result);
  return result;
}

/*****************************************************************************/

/**
 * @brief Return a span set from its WKB representation
 */
static SpanSet *
spanset_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the span type */
  uint16_t wkb_spansettype = int16_from_wkb_state(s);
  /* For template classes it is necessary to store the specific type */
  s->type = (uint8_t) wkb_spansettype;
  s->spantype = spansettype_spantype(s->type);
  s->basetype = spantype_basetype(s->spantype);

  /* Read the number of spans and allocate space for them */
  int count = int32_from_wkb_state(s);
  Span *spans = palloc(sizeof(Span) * count);

  /* Read and create the span set */
  for (int i = 0; i < count; i++)
    span_from_wkb_state_iter(s, &spans[i]);
  return spanset_make_free(spans, count, NORMALIZE, ORDER_NO);
}

/*****************************************************************************/

/**
 * @brief Parse the WKB flags
 */
void
set_flags_from_wkb_state(meos_wkb_parse_state *s, uint8_t wkb_flags)
{
  s->hasz = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_flags & MEOS_WKB_ORDERED)
    s->ordered = true;
  /* Get the flags */
  if (spatial_basetype(s->basetype))
  {
    if (wkb_flags & MEOS_WKB_ZFLAG)
      s->hasz = true;
    if (wkb_flags & MEOS_WKB_GEODETICFLAG)
      s->geodetic = true;
    if (wkb_flags & MEOS_WKB_SRIDFLAG)
      s->has_srid = true;
  }
  return;
}

/**
 * @brief Return a set from its WKB representation
 */
static Set *
set_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the set type */
  uint16_t wkb_settype = int16_from_wkb_state(s);
  /* For template classes it is necessary to store the specific type */
  s->type = (uint8_t) wkb_settype;
  s->basetype = settype_basetype(s->type);
  /* Read the flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  set_flags_from_wkb_state(s, wkb_flags);
  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);

  /* Read the number of values and allocate space for them */
  int count = int32_from_wkb_state(s);
  Datum *values = palloc(sizeof(Datum) * count);

  /* Read and create the set */
  for (int i = 0; i < count; i++)
    values[i] = base_from_wkb_state(s);
  return set_make_free(values, count, s->basetype, ORDER_NO);
}

/*****************************************************************************/

/**
 * @brief Return the state flags initialized according to a box byte flag read
 * from the buffer
 */
static void
tbox_flags_from_wkb_state(meos_wkb_parse_state *s, uint8_t wkb_flags)
{
  assert(wkb_flags & MEOS_WKB_XFLAG || wkb_flags & MEOS_WKB_TFLAG);
  s->hasx = false;
  s->hast = false;
  if (wkb_flags & MEOS_WKB_XFLAG)
    s->hasx = true;
  if (wkb_flags & MEOS_WKB_TFLAG)
    s->hast = true;
  return;
}

/**
 * @brief Return a temporal box from its WKB representation
 */
static TBox *
tbox_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the temporal flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  tbox_flags_from_wkb_state(s, wkb_flags);

  /* Read and create the box */
  Span span, period;
  /* Read the temporal dimension if any */
  if (s->hast)
    period = span_from_wkb_state(s);
  /* Read the value dimension if any */
  if (s->hasx)
    span = span_from_wkb_state(s);
  /* Create the temporal box */
  return tbox_make(s->hasx ? &span : NULL, s->hast ? &period : NULL);
}

/*****************************************************************************/

/**
 * @brief Return the state flags initialized with a byte flag read from the
 * buffer
 */
static void
stbox_flags_from_wkb_state(meos_wkb_parse_state *s, uint8_t wkb_flags)
{
  assert(wkb_flags & MEOS_WKB_XFLAG || wkb_flags & MEOS_WKB_TFLAG);
  s->hasx = false;
  s->hasz = false;
  s->hast = false;
  s->geodetic = false;
  s->has_srid = false;
  if (wkb_flags & MEOS_WKB_XFLAG)
    s->hasx = true;
  if (wkb_flags & MEOS_WKB_ZFLAG)
    s->hasz = true;
  if (wkb_flags & MEOS_WKB_TFLAG)
    s->hast = true;
  if (wkb_flags & MEOS_WKB_GEODETICFLAG)
    s->geodetic = true;
  if (wkb_flags & MEOS_WKB_SRIDFLAG)
    s->has_srid = true;
  return;
}

/**
 * @brief Return a spatiotemporal box from its WKB representation
 */
static STBox *
stbox_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the temporal flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  stbox_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);

  /* Read and create the box */
  double xmin = 0, xmax = 0, ymin = 0, ymax = 0, zmin = 0, zmax = 0;
  Span period;
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
  return stbox_make(s->hasx, s->hasz, s->geodetic, s->srid,
    xmin, xmax, ymin, ymax, zmin, zmax, s->hast ? &period : NULL);
}

/*****************************************************************************/

/**
 * @brief Take in an unknown kind of WKB type number and ensure it comes out as
 * an extended WKB type number with the `Z/GEODETIC/SRID/LINEAR_INTERP`
 * flags masked onto the high bits
 */
void
temporal_flags_from_wkb_state(meos_wkb_parse_state *s, uint8_t wkb_flags)
{
  s->hasx = true;
  s->hast = true;
  s->hasz = false;
  s->geodetic = false;
  s->has_srid = false;
  /* Get the interpolation */
  s->interp = MEOS_WKB_GET_INTERP(wkb_flags);
  /* Get the flags */
  if (tspatial_type(s->temptype))
  {
    if (wkb_flags & MEOS_WKB_ZFLAG)
      s->hasz = true;
    if (wkb_flags & MEOS_WKB_GEODETICFLAG)
      s->geodetic = true;
    if (wkb_flags & MEOS_WKB_SRIDFLAG)
      s->has_srid = true;
  }
  /* Mask off the upper flags to get the subtype */
  uint8 subtype = wkb_flags & (uint8_t) 0x03;
  switch (subtype)
  {
    case TINSTANT:
      s->subtype = TINSTANT;
      break;
    case TSEQUENCE:
      s->subtype = TSEQUENCE;
      break;
    case TSEQUENCESET:
      s->subtype = TSEQUENCESET;
      break;
    default: /* Error! */
      meos_error(ERROR, MEOS_ERR_WKB_INPUT,
        "Unknown WKB flags: %d", wkb_flags);
      break;
  }
  return;
}

/**
 * @brief Return a temporal instant from its WKB representation
 * @details The function reads the base type value and the timestamp and
 * advances the parse state forward appropriately. It starts reading it just
 * after the endian byte, the temporal type (an @p int16), and the temporal
 * flags byte.
 */
static TInstant *
tinstant_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the values from the buffer and create the instant */
  Datum value = base_from_wkb_state(s);
  TimestampTz t = timestamp_from_wkb_state(s);
  return tinstant_make_free(value, s->temptype, t);
}

/**
 * @brief Return a temporal instant array from its WKB representation
 */
static TInstant **
tinstarr_from_wkb_state(meos_wkb_parse_state *s, int count)
{
  TInstant **result = palloc(sizeof(TInstant *) * count);
  for (int i = 0; i < count; i++)
  {
    /* Parse the point and the timestamp to create the instant point */
    Datum value = base_from_wkb_state(s);
    TimestampTz t = timestamp_from_wkb_state(s);
    result[i] = tinstant_make_free(value, s->temptype, t);
  }
  return result;
}

/**
 * @brief Return a temporal sequence value from its WKB representation
 */
static TSequence *
tsequence_from_wkb_state(meos_wkb_parse_state *s)
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
  return tsequence_make_free(instants, count, lower_inc, upper_inc, s->interp,
    NORMALIZE);
}

/**
 * @brief Return a temporal sequence set from its WKB representation
 */
static TSequenceSet *
tsequenceset_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Get the number of sequences */
  int count = int32_from_wkb_state(s);
  assert(count > 0);
  /* Parse the sequences */
  TSequence **sequences = palloc(sizeof(TSequence *) * count);
  for (int i = 0; i < count; i++)
  {
    /* Get the number of instants */
    int ninst = int32_from_wkb_state(s);
    /* Get the period bounds */
    uint8_t wkb_bounds = (uint8_t) byte_from_wkb_state(s);
    bool lower_inc, upper_inc;
    bounds_from_wkb_state(wkb_bounds, &lower_inc, &upper_inc);
    /* Parse the instants */
    TInstant **instants = palloc(sizeof(TInstant *) * ninst);
    for (int j = 0; j < ninst; j++)
    {
      /* Parse the value and the timestamp to create the temporal instant */
      Datum value = base_from_wkb_state(s);
      TimestampTz t = timestamp_from_wkb_state(s);
      instants[j] = tinstant_make_free(value, s->temptype, t);
    }
    sequences[i] = tsequence_make_free(instants, ninst, lower_inc, upper_inc,
      s->interp, NORMALIZE);
  }
  return tsequenceset_make_free(sequences, count, NORMALIZE);
}

/**
 * @brief Return a temporal value from its WKB representation
 */
static Temporal *
temporal_from_wkb_state(meos_wkb_parse_state *s)
{
  /* Read the temporal type */
  uint16_t wkb_temptype = int16_from_wkb_state(s);
  s->temptype = (uint8_t) wkb_temptype;
  s->basetype = temptype_basetype(s->temptype);

  /* Read the temporal and interpolation flags */
  uint8_t wkb_flags = (uint8_t) byte_from_wkb_state(s);
  temporal_flags_from_wkb_state(s, wkb_flags);

  /* Read the SRID, if necessary */
  if (s->has_srid)
    s->srid = int32_from_wkb_state(s);
  else if (wkb_flags & MEOS_WKB_GEODETICFLAG)
    s->srid = SRID_DEFAULT;

#if RGEO
  GSERIALIZED *gs;
  if (s->temptype == T_TRGEOMETRY)
    gs = geo_from_wkb_state(s);
#endif /* RGEO */

  /* Read the temporal value */
  Temporal *res;
  assert(temptype_subtype(s->subtype));
  switch (s->subtype)
  {
    case TINSTANT:
      res = (Temporal *) tinstant_from_wkb_state(s);
      break;
    case TSEQUENCE:
      res = (Temporal *) tsequence_from_wkb_state(s);
      break;
    default: /* TSEQUENCESET */
      res = (Temporal *) tsequenceset_from_wkb_state(s);
  }

#if RGEO
  if (s->temptype == T_TRGEOMETRY)
  {
    Temporal *result = geo_tpose_to_trgeo(gs, res);
    pfree(gs); pfree(res);
    return result;
  }
#endif /* RGEO */
  return res;
}

/*****************************************************************************/

/**
 * @brief Return a value from its Well-Known Binary (WKB) representation
 */
Datum
type_from_wkb(const uint8_t *wkb, size_t size, meosType type)
{
  /* Initialize the state appropriately */
  meos_wkb_parse_state s;
  memset(&s, 0, sizeof(meos_wkb_parse_state));
  s.wkb = s.pos = wkb;
  s.wkb_size = size;
  /* Fail when handed incorrect starting byte */
  uint8_t wkb_little_endian = byte_from_wkb_state(&s);
  if (wkb_little_endian != 1 && wkb_little_endian != 0)
  {
    meos_error(ERROR, MEOS_ERR_WKB_INPUT,
        "Invalid endian flag value in WKB string.");
    return 0;
  }
  /* Check the endianness of our input */
  s.swap_bytes = false;
  /* Machine arch is big endian, request is for little */
  if (MEOS_IS_BIG_ENDIAN && wkb_little_endian)
    s.swap_bytes = true;
  /* Machine arch is little endian, request is for big */
  else if ((! MEOS_IS_BIG_ENDIAN) && (! wkb_little_endian))
    s.swap_bytes = true;

  /* Call the type-specific function */
  s.type = type;
  if (set_type(type))
    return PointerGetDatum(set_from_wkb_state(&s));
  if (span_type(type))
  {
    Span *span = palloc(sizeof(Span));
    *span = span_from_wkb_state(&s);
    return PointerGetDatum(span);
  }
  if (spanset_type(type))
    return PointerGetDatum(spanset_from_wkb_state(&s));
  if (type == T_TBOX)
    return PointerGetDatum(tbox_from_wkb_state(&s));
  if (type == T_STBOX)
    return PointerGetDatum(stbox_from_wkb_state(&s));
#if CBUFFER
  if (type == T_CBUFFER)
    return PointerGetDatum(cbuffer_from_wkb_state(&s));
#endif /* CBUFFER */
#if NPOINT
  if (type == T_NPOINT)
    return PointerGetDatum(npoint_from_wkb_state(&s));
#endif /* NPOINT */
#if POSE || RGEO
  if (type == T_POSE)
    return PointerGetDatum(pose_from_wkb_state(&s));
#endif /* POSE */
  if (temporal_type(type))
    return PointerGetDatum(temporal_from_wkb_state(&s));
  /* Error! */
  meos_error(ERROR, MEOS_ERR_WKB_INPUT,
    "Unknown type in WKB string: %s", meostype_name(type));
  return 0;
}

/**
 * @brief Return a value from its HexEWKB representation
 */
Datum
type_from_hexwkb(const char *hexwkb, size_t size, meosType type)
{
  uint8_t *wkb = bytes_from_hexbytes(hexwkb, size);
  Datum result = type_from_wkb(wkb, size / 2, type);
  pfree(wkb);
  return result;
}

/*****************************************************************************
 * WKB and HexWKB input functions for set and span types
 *****************************************************************************/

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Set_recv(), #Set_from_wkb()
 */
Set *
set_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  /* We pass ANY set type, the actual type is read from the byte string */
  return DatumGetSetP(type_from_wkb(wkb, size, T_INTSET));
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a set from its hex-encoded ASCII Well-Known Binary (WKB)
 * representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Set_from_hexwkb()
 */
Set *
set_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  /* We pass ANY set type, the actual type is read from the byte string */
  return DatumGetSetP(type_from_hexwkb(hexwkb, size, T_INTSET));
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a span from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Span_recv(), #Span_from_wkb()
 */
Span *
span_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  /* We pass ANY span type, the actual type is read from the byte string */
  return DatumGetSpanP(type_from_wkb(wkb, size, T_INTSPAN));
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a span from its hex-encoded ASCII Well-Known Binary (WKB)
 * representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Span_from_hexwkb()
 */
Span *
span_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  /* We pass ANY span type, the actual type is read from the byte string */
  return DatumGetSpanP(type_from_hexwkb(hexwkb, size, T_INTSPAN));
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a span set from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Spanset_recv(), #Spanset_from_wkb()
 */
SpanSet *
spanset_from_wkb(const uint8_t *wkb, size_t size)
{
  VALIDATE_NOT_NULL(wkb, NULL);
  /* We pass ANY span set type, the actual type is read from the byte string */
  return DatumGetSpanSetP(type_from_wkb(wkb, size, T_INTSPANSET));
}

/**
 * @ingroup meos_setspan_inout
 * @brief Return a span set from its hex-encoded ASCII Well-Known Binary (WKB)
 * representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Spanset_from_hexwkb()
 */
SpanSet *
spanset_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  /* We pass ANY span set type, the actual type is read from the byte string */
  return DatumGetSpanSetP(type_from_hexwkb(hexwkb, size, T_INTSPANSET));
}

/*****************************************************************************
 * WKB and HexWKB input functions for bounding box types
 *****************************************************************************/

/**
 * @ingroup meos_box_inout
 * @brief Return a temporal box from its Well-Known Binary (WKB) representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @csqlfn #Tbox_recv(), #Tbox_from_wkb()
 */
TBox *
tbox_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  return DatumGetTboxP(type_from_wkb(wkb, size, T_TBOX));
}

/**
 * @ingroup meos_box_inout
 * @brief Return a temporal box from its hex-encoded ASCII Well-Known Binary
 * (WKB) representation
 * @param[in] hexwkb HexWKB string
 * @csqlfn #Tbox_from_hexwkb()
 */
TBox *
tbox_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  return DatumGetTboxP(type_from_hexwkb(hexwkb, size, T_TBOX));
}

/*****************************************************************************
 * WKB and HexWKB input functions for temporal types
 *****************************************************************************/

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal value from its Well-Known Binary (WKB)
 * representation
 * @param[in] wkb WKB string
 * @param[in] size Size of the string
 * @return On error return @p NULL
 * @csqlfn #Temporal_recv(), #Temporal_from_wkb()
 */
Temporal *
temporal_from_wkb(const uint8_t *wkb, size_t size)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(wkb, NULL);
  /* We pass ANY temporal type, the actual type is read from the byte string */
  return DatumGetTemporalP(type_from_wkb(wkb, size, T_TINT));
}

/**
 * @ingroup meos_temporal_inout
 * @brief Return a temporal value from its hex-encoded ASCII Extended
 * Well-Known Binary (EWKB) representation
 * @param[in] hexwkb HexWKB string
 * @return On error return @p NULL
 * @csqlfn #Temporal_from_hexwkb()
 */
Temporal *
temporal_from_hexwkb(const char *hexwkb)
{
  /* Ensure the validity of the arguments */
  VALIDATE_NOT_NULL(hexwkb, NULL);
  size_t size = strlen(hexwkb);
  /* We pass ANY temporal type, the actual type is read from the byte string */
  return DatumGetTemporalP(type_from_hexwkb(hexwkb, size, T_TINT));
}

/*****************************************************************************/
