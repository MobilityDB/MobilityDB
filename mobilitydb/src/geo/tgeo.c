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
 * @brief General functions for temporal geos
 */

#include "geo/tgeo.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/temporal.h"
#include "geo/postgis_funcs.h"
#include "geo/stbox.h"
#include "geo/tspatial_parser.h"
#include "geo/tgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * General functions
 *****************************************************************************/

#define PGC_ERRMSG_MAXLEN 2048

/**
 * @brief Output an error message
 */
static void
pg_error(const char *fmt, va_list ap)
{
  char errmsg[PGC_ERRMSG_MAXLEN];
  vsnprintf(errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);
  ereport(ERROR, (errmsg_internal("%s", errmsg)));
  return;
}

/**
 * @brief Output a notice message
 */
static void
pg_notice(const char *fmt, va_list ap)
{
  char errmsg[PGC_ERRMSG_MAXLEN];
  vsnprintf(errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);
  ereport(NOTICE, (errmsg_internal("%s", errmsg)));
  return;
}

/**
 * @brief Set the handlers for initializing the liblwgeom library
 */
void
mobilitydb_init()
{
  lwgeom_set_handlers(palloc, repalloc, pfree, pg_error, pg_notice);
  return;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

#define TSPATIAL_MAX_TYPMOD 3

PGDLLEXPORT Datum Tpoint_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpoint_in);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return a temporal point from its Well-Known Text (WKT) representation
 * @details Input examples for the various temporal subtypes are as follows:
 * @code
 * -- Instant
 * Point(0 0) @ 2012-01-01 08:00:00
 * -- Sequence with discrete interpolation
 * { Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 }
 * -- Sequence with linear interpolation
 * [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 )
 * -- Sequence with step interploation
 * Interp=Step;[ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 )
 * -- Sequence set with linear interpolation
 * { [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 ) ,
 *   [ Point(1 1) @ 2012-01-01 08:20:00 , Point(0 0) @ 2012-01-01 08:30:00 ] }
 * -- Sequence set with step interpolation
 * Interp=Step;{ [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 ) ,
 *   [ Point(1 1) @ 2012-01-01 08:20:00 , Point(0 0) @ 2012-01-01 08:30:00 ] }
 * @endcode
 * @sqlfn tpoint_in()
 */
Datum
Tpoint_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  PG_RETURN_TEMPORAL_P(tpoint_parse(&input, oid_type(temptypid)));
}

PGDLLEXPORT Datum Tgeo_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeo_in);
/**
 * @ingroup mobilitydb_geo_inout
 * @brief Return a temporal geo from its Well-Known Text (WKT) representation
 * @sqlfn tgeo_in()
 */
Datum
Tgeo_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Temporal *result = tspatial_parse(&input, oid_type(temptypid));
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Input typmod information for temporal geos
 */
uint32
tspatial_typmod_in(ArrayType *arr, int is_point, int is_geodetic)
{
  uint32 typmod = 0;
  Datum *elem_values;
  int n = 0;

  if (ARR_ELEMTYPE(arr) != CSTRINGOID)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_ELEMENT_ERROR),
      errmsg("typmod array must be type cstring[]")));
  if (ARR_NDIM(arr) != 1)
    ereport(ERROR, (errcode(ERRCODE_ARRAY_SUBSCRIPT_ERROR),
      errmsg("typmod array must be one-dimensional")));
  if (ARR_HASNULL(arr))
    ereport(ERROR, (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
      errmsg("typmod array must not contain nulls")));

  /*
   * There are several ways to define a column wrt type modifiers:
   *   column_type(TempSubType, Geometry, SRID) => All modifiers are determined.
   *   column_type(TempSubType, Geometry) => The SRID is generic.
   *   column_type(Geometry, SRID) => The temporal subtype is generic.
   *   column_type(Geometry) => The temporal subtype and SRID are generic.
   *   column_type(TempSubType) => The geometry type and SRID are generic.
   *   column_type => The temporal subtype, geometry type, and SRID are generic.
   *
   * For example, if the user did not set the temporal subtype, we can use any
   * temporal subtype in the same column. Similarly for all generic modifiers.
   */
  deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
  if (n > TSPATIAL_MAX_TYPMOD)
    elog(ERROR, "Incorrect number of type modifiers for spatiotemporal values");

  /* Set default values for typmod if they are not given */
  int16 tempsubtype = ANYTEMPSUBTYPE;
  uint8_t geometry_type = 0;
  int hasZ = 0, hasM = 0, srid = SRID_UNKNOWN;
  bool has_geo = false, has_srid = false;

  /* Get the string values from the input array */
  char *s[3] = {0,0,0};
  for (int i = 0; i < n; i++)
  {
    s[i] = DatumGetCString(elem_values[i]);
    if (strlen(s[i]) == 0)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Empty temporal subtype modifier")));
  }

  /* Extract the typmod values */
  if (n == 3)
  {
    /* Type_modifier is (TempSubType, Geometry, SRID) */
    if (tempsubtype_from_string(s[0], &tempsubtype) == false)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid temporal subtype modifier: %s", s[0])));
    if (geometry_type_from_string(s[1], &geometry_type, &hasZ, &hasM) == LW_FAILURE)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid geometry type modifier: %s", s[1])));
#if POSTGRESQL_VERSION_NUMBER >= 150000
    srid = pg_strtoint32(s[2]);
#else
    srid = pg_atoi(s[2], sizeof(int32), '\0');
#endif /* POSTGRESQL_VERSION_NUMBER >= 150000 */
    srid = clamp_srid(srid);
    has_geo = has_srid = true;
  }
  else if (n == 2)
  {
    /* Type modifier is either (TempSubType, Geometry), (TempSubType, SRID) or
      (Geometry, SRID) */
    if (tempsubtype_from_string(s[0], &tempsubtype))
    {
      if (geometry_type_from_string(s[1], &geometry_type, &hasZ, &hasM) == LW_FAILURE)
      {
#if POSTGRESQL_VERSION_NUMBER >= 150000
        srid = pg_strtoint32(s[1]);
#else
        srid = pg_atoi(s[1], sizeof(int32), '\0');
#endif /* POSTGRESQL_VERSION_NUMBER >= 150000 */
        srid = clamp_srid(srid);
        has_srid = true;
      }
      else
        has_geo = true;
    }
    else
    {
      if (geometry_type_from_string(s[0], &geometry_type, &hasZ, &hasM) == LW_FAILURE)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
            errmsg("Invalid geometry type modifier: %s", s[0])));
#if POSTGRESQL_VERSION_NUMBER >= 150000
      srid = pg_strtoint32(s[1]);
#else
      srid = pg_atoi(s[1], sizeof(int32), '\0');
#endif /* POSTGRESQL_VERSION_NUMBER >= 150000 */
      srid = clamp_srid(srid);
      has_geo = has_srid = true;
    }
  }
  else if (n == 1)
  {
    /* Type modifier: either (TempSubType), (Geometry), or (SRID) */
    has_srid = false;
    if (tempsubtype_from_string(s[0], &tempsubtype))
      ;
    else if (geometry_type_from_string(s[0], &geometry_type, &hasZ, &hasM))
      has_geo = true;
    else
    {
#if POSTGRESQL_VERSION_NUMBER >= 150000
      srid = pg_strtoint32(s[0]);
#else
      srid = pg_atoi(s[0], sizeof(int32), '\0');
#endif /* POSTGRESQL_VERSION_NUMBER >= 150000 */
      srid = clamp_srid(srid);
      has_srid = true;
    }
  }

  /* Set the temporal type */
  if (tempsubtype != ANYTEMPSUBTYPE)
    TYPMOD_SET_TEMPSUBTYPE(typmod, tempsubtype);

  /* Set the geometry type */
  if (has_geo)
  {
    if (is_point && (geometry_type != POINTTYPE || hasM))
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Only point geometries without M dimension accepted")));
    TYPMOD_SET_TYPE(typmod, geometry_type);
    if (hasZ)
      TYPMOD_SET_Z(typmod);
  }

  /* Set default SRID */
  if (is_geodetic)
    TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
  else
    TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);

  /* Set the SRID */
  if (has_srid)
  {
    if (srid != SRID_UNKNOWN)
      TYPMOD_SET_SRID(typmod, srid);
  }

  pfree(elem_values);
  return typmod;
}

PGDLLEXPORT Datum Tgeometry_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeometry_typmod_in);
/**
 * @brief Input typmod information for temporal geometries
 */
Datum
Tgeometry_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tspatial_typmod_in(array, false, false);
  PG_RETURN_INT32(typmod);
}

PGDLLEXPORT Datum Tgeography_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeography_typmod_in);
/**
 * @brief Input typmod information for temporal geographies
 */
Datum
Tgeography_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  int32 typmod = tspatial_typmod_in(array, false, true);
  int32_t srid = TYPMOD_GET_SRID(typmod);
  /* Check the SRID is legal (geographic coordinates) */
  if (! ensure_srid_is_latlong(srid))
      PG_RETURN_INT32(-1);
  PG_RETURN_INT32(typmod);
}

PGDLLEXPORT Datum Tgeompoint_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeompoint_typmod_in);
/**
 * @brief Input typmod information for temporal geometries
 */
Datum
Tgeompoint_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tspatial_typmod_in(array, true, false);
  PG_RETURN_INT32(typmod);
}

PGDLLEXPORT Datum Tgeogpoint_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeogpoint_typmod_in);
/**
 * @brief Input typmod information for temporal geographies
 */
Datum
Tgeogpoint_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  int32 typmod = tspatial_typmod_in(array, true, true);
  int32_t srid = TYPMOD_GET_SRID(typmod);
  /* Check the SRID is legal (geographic coordinates) */
  if (! ensure_srid_is_latlong(srid))
      PG_RETURN_INT32(-1);
  PG_RETURN_INT32(typmod);
}

/* Maximum length of the typmod string */
#define MAX_TYPMOD_LEN 64

PGDLLEXPORT Datum Tspatial_typmod_out(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_typmod_out);
/**
 * @brief Output typmod information for temporal geos
 */
Datum
Tspatial_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = palloc(MAX_TYPMOD_LEN);
  char *str = s;
  size_t len = 0;
  int32 typmod = PG_GETARG_INT32(0);
  int16 tempsubtype = TYPMOD_GET_TEMPSUBTYPE(typmod);
  int32 srid = TYPMOD_GET_SRID(typmod);
  uint8_t geometry_type = (uint8_t) TYPMOD_GET_TYPE(typmod);
  int32 hasz = TYPMOD_GET_Z(typmod);

  /* No temporal subtype or geometry type? Then no typmod at all.
    Return empty string. */
  if (typmod < 0 || (tempsubtype == ANYTEMPSUBTYPE && !geometry_type))
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  /* Opening bracket */
  len = snprintf(str, MAX_TYPMOD_LEN - 1, "(");
  /* Has temporal subtype?  */
  if (tempsubtype != ANYTEMPSUBTYPE)
    len += snprintf(str + len, MAX_TYPMOD_LEN - len - 1, "%s",
      tempsubtype_name(tempsubtype));
  if (geometry_type)
  {
    if (tempsubtype != ANYTEMPSUBTYPE)
      len += snprintf(str + len, MAX_TYPMOD_LEN - len - 1, ",");
    len += snprintf(str + len, MAX_TYPMOD_LEN - len - 1, "%s",
      lwtype_name(geometry_type));
    /* Has Z?  */
    if (hasz)
      len += snprintf(str + len, MAX_TYPMOD_LEN - len - 1, "Z");
    /* Has SRID?  */
    if (srid)
      len += snprintf(str + len, MAX_TYPMOD_LEN - len - 1, ",%d", srid);
  }
  /* Closing bracket.  */
  snprintf(str + len, MAX_TYPMOD_LEN - len - 1, ")");

  PG_RETURN_CSTRING(s);
}

/**
 * @brief Check the consistency of the metadata specified in the typmod: 
 * temporal subtype, geometry type, and SRID. If things are inconsistent, 
 * shut down the query.
 */
Temporal *
tspatial_valid_typmod(Temporal *temp, int32_t typmod)
{
  /* Get the characteristics of the temporal value */
  uint8 subtype = temp->subtype;
  int32 srid = tspatial_srid(temp);
  int32 hasz = MEOS_FLAGS_GET_Z(temp->flags);
  /* Get the characteristics of the typmod */
  uint8 typmod_subtype = TYPMOD_GET_TEMPSUBTYPE(typmod);
  int32 typmod_srid = TYPMOD_GET_SRID(typmod);
  int32 typmod_type = TYPMOD_GET_TYPE(typmod);
  int32 typmod_hasz = TYPMOD_GET_Z(typmod);
  const char *type_str = meostype_name(temp->temptype);

  /* No typmod (-1) */
  if (typmod < 0 && typmod_subtype == ANYTEMPSUBTYPE)
    return temp;

  /* Typmod has a preference for temporal subtype */
  if (typmod_subtype != ANYTEMPSUBTYPE && typmod_subtype != subtype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal subtype of the %s value (%s) does not match column subtype (%s)",
        type_str, tempsubtype_name(subtype), tempsubtype_name(typmod_subtype))));
  /* Typmod has a preference for SRID? Geometry SRID had better match */
  if (typmod_srid > 0 && typmod_srid != srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("SRID of the %s value (%d) does not match column SRID (%d)",
        type_str, srid, typmod_srid) ));
  /* Mismatched Z dimensionality in both ways  */
  if (typmod_hasz && ! hasz)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Column has Z dimension but the %s value does not", type_str)));
  if (typmod_type > 0 && hasz && ! typmod_hasz)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("The %s value has Z dimension but column does not", type_str)));

  /* For geometry types call the PostGIS function */
  if (typmod_type > 0 && tgeo_type_all(temp->temptype))
  {
    int count;
    Datum *datumarr = temporal_values_p(temp, &count);
    for (int i = 0; i < count; i++)
      if (! postgis_valid_typmod(DatumGetGserializedP(datumarr[i]), typmod))
        return NULL;
    return temp;
  }

  return temp;
}

PGDLLEXPORT Datum Tspatial_enforce_typmod(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tspatial_enforce_typmod);
/**
 * @brief Enforce typmod information for temporal geos with respect to
 * temporal type, dimensions, and SRID
 */
Datum
Tspatial_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if typmod of the temporal geo is consistent with the supplied one */
  temp = tspatial_valid_typmod(temp, typmod);
  PG_RETURN_TEMPORAL_P(temp);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpointinst_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpointinst_constructor);
/**
 * @ingroup mobilitydb_geo_constructor
 * @brief Return a temporal point instant from a point and a timestamptz
 * @sqlfn tgeompoint(), tgeogpoint()
 */
Datum
Tpointinst_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Temporal *result = (Temporal *) tpointinst_make(gs, t);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tgeoinst_constructor(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tgeoinst_constructor);
/**
 * @ingroup mobilitydb_geo_constructor
 * @brief Return a temporal geo instant from a geometry/geography and a 
 * timestamptz
 * @sqlfn tgeometry(), tgeography()
 */
Datum
Tgeoinst_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Temporal *result = (Temporal *) tgeoinst_make(gs, t);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
