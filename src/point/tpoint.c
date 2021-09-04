/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * @file tpoint.c
 * Basic functions for temporal points.
 */

#include "point/tpoint.h"

#include <utils/builtins.h>
#include <utils/timestamp.h>

#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/lifting.h"
#include "general/temporal_compops.h"
#include "point/stbox.h"
#include "point/tpoint_parser.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Miscellaneous functions
 *****************************************************************************/

#define PGC_ERRMSG_MAXLEN 2048

/**
 * Output an error message
 */
static void
pg_error(const char *fmt, va_list ap)
{
  char errmsg[PGC_ERRMSG_MAXLEN + 1];
  vsnprintf (errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);
  errmsg[PGC_ERRMSG_MAXLEN]='\0';
  ereport(ERROR, (errmsg_internal("%s", errmsg)));
  return;
}

/**
 * Output a notice message
 */
static void
pg_notice(const char *fmt, va_list ap)
{
  char errmsg[PGC_ERRMSG_MAXLEN + 1];
  vsnprintf (errmsg, PGC_ERRMSG_MAXLEN, fmt, ap);
  errmsg[PGC_ERRMSG_MAXLEN]='\0';
  ereport(NOTICE, (errmsg_internal("%s", errmsg)));
  return;
}

/**
 * Set the handlers for initializing the liblwgeom library
 */
void temporalgeom_init()
{
  lwgeom_set_handlers(palloc, repalloc, pfree, pg_error, pg_notice);
}

/**
 * Copy a GSERIALIZED. This function is not available anymore in PostGIS 3
 */
GSERIALIZED *
gserialized_copy(const GSERIALIZED *g)
{
  GSERIALIZED *result = palloc(VARSIZE(g));
  memcpy(result, g, VARSIZE(g));
  return result;
}

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

/**
 * Check the consistency of the metadata we want to enforce in the typmod:
 * SRID, type and dimensionality. If things are inconsistent, shut down the query.
 */
static Temporal *
tpoint_valid_typmod(Temporal *temp, int32_t typmod)
{
  int32 tpoint_srid = tpoint_srid_internal(temp);
  int16 tpoint_subtype = temp->subtype;
  int16 typmod_subtype = TYPMOD_GET_SUBTYPE(typmod);
  TYPMOD_DEL_SUBTYPE(typmod);
  /* If there is no geometry type */
  if (typmod == 0)
    typmod = -1;
  int32 tpoint_z = MOBDB_FLAGS_GET_Z(temp->flags);
  int32 typmod_srid = TYPMOD_GET_SRID(typmod);
  int32 typmod_type = TYPMOD_GET_TYPE(typmod);
  int32 typmod_z = TYPMOD_GET_Z(typmod);

  /* No typmod (-1) */
  if (typmod < 0 && typmod_subtype == ANYTEMPSUBTYPE)
    return temp;
  /* Typmod has a preference for SRID? Geometry SRID had better match */
  if (typmod_srid > 0 && typmod_srid != tpoint_srid)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal point SRID (%d) does not match column SRID (%d)",
        tpoint_srid, typmod_srid) ));
  /* Typmod has a preference for temporal type */
  if (typmod_type > 0 && typmod_subtype != ANYTEMPSUBTYPE && typmod_subtype != tpoint_subtype)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal type (%s) does not match column type (%s)",
        tempsubtype_name(tpoint_subtype), tempsubtype_name(typmod_subtype)) ));
  /* Mismatched Z dimensionality.  */
  if (typmod > 0 && typmod_z && ! tpoint_z)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Column has Z dimension but temporal point does not" )));
  /* Mismatched Z dimensionality (other way) */
  if (typmod > 0 && tpoint_z && ! typmod_z)
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
      errmsg("Temporal point has Z dimension but column does not" )));

  return temp;
}

PG_FUNCTION_INFO_V1(tpoint_in);
/**
 * Generic input function for temporal points
 *
 * @note Examples of input for the various temporal types:
 * - Instant
 * @code
 * Point(0 0) @ 2012-01-01 08:00:00
 * @endcode
 * - Instant set
 * @code
 * { Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 }
 * @endcode
 * - Sequence
 * @code
 * [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 )
 * @endcode
 * - Sequence set
 * @code
 * { [ Point(0 0) @ 2012-01-01 08:00:00 , Point(1 1) @ 2012-01-01 08:10:00 ) ,
 * [ Point(1 1) @ 2012-01-01 08:20:00 , Point(0 0) @ 2012-01-01 08:30:00 ] }
 * @endcode
 */
PGDLLEXPORT Datum
tpoint_in(PG_FUNCTION_ARGS)
{
  char *input = PG_GETARG_CSTRING(0);
  Oid temptypid = PG_GETARG_OID(1);
  Oid basetypid = temporal_basetypid(temptypid);
  Temporal *result = tpoint_parse(&input, basetypid);
  PG_RETURN_POINTER(result);
}

/**
 * Input typmod information for temporal points
 */
static uint32
tpoint_typmod_in(ArrayType *arr, int is_geography)
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
   *    column_type(TempSubType, Geometry) => The SRID is generic.
   *    column_type(Geometry, SRID) => The temporal type is generic.
   *    column_type(Geometry) => The temporal type and SRID are generic.
   *   column_type(TempSubType) => The geometry type and SRID are generic.
   *   column_type => The temporal type, geometry type, and SRID are generic.
   *
   * For example, if the user did not set the temporal type, we can use any
   * temporal type in the same column. Similarly for all generic modifiers.
   */
  deconstruct_array(arr, CSTRINGOID, -2, false, 'c', &elem_values, NULL, &n);
  int16 temp_subtype = ANYTEMPSUBTYPE;
  uint8_t geometry_type = 0;
  int hasZ = 0, hasM = 0, srid = SRID_UNKNOWN;
  char *s[3] = {0,0,0};
  for (int i = 0; i < n; i++)
  {
    s[i] = DatumGetCString(elem_values[i]);
    if (strlen(s[i]) == 0)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Empty temporal type modifier")));
  }

  bool has_geo = false, has_srid = false;
  if (n == 3)
  {
    /* Type_modifier is (TempSubType, Geometry, SRID) */
    if (tempsubtype_from_string(s[0], &temp_subtype) == false)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid temporal type modifier: %s", s[0])));
    if (geometry_type_from_string(s[1], &geometry_type, &hasZ, &hasM) == LW_FAILURE)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid geometry type modifier: %s", s[1])));
    srid = pg_atoi(s[2], sizeof(int32), '\0');
    srid = clamp_srid(srid);
    has_geo = has_srid = true;
  }
  else if (n == 2)
  {
    /* Type modifier is either (TempSubType, Geometry) or (Geometry, SRID) */
    if (tempsubtype_from_string(s[0], &temp_subtype))
    {
      if (geometry_type_from_string(s[1], &geometry_type, &hasZ, &hasM) == LW_FAILURE)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
            errmsg("Invalid geometry type modifier: %s", s[1])));
      has_geo = true;
    }
    else
    {
      if (geometry_type_from_string(s[0], &geometry_type, &hasZ, &hasM) == LW_FAILURE)
        ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
            errmsg("Invalid geometry type modifier: %s", s[0])));
      srid = pg_atoi(s[1], sizeof(int32), '\0');
      srid = clamp_srid(srid);
      has_geo = has_srid = true;
    }
  }
  else if (n == 1)
  {
    /* Type modifier: either (TempSubType) or (Geometry) */
    has_srid = false;
    if (tempsubtype_from_string(s[0], &temp_subtype))
      ;
    else if (geometry_type_from_string(s[0], &geometry_type, &hasZ, &hasM))
      has_geo = true;
    else
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid temporal point type modifier:")));
  }
  else
    ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Invalid temporal point type modifier:")));

  /* Shift to remove the 4 bits of the temporal type */
  TYPMOD_DEL_SUBTYPE(typmod);
  /* Set default values */
  if (is_geography)
    TYPMOD_SET_SRID(typmod, SRID_DEFAULT);
  else
    TYPMOD_SET_SRID(typmod, SRID_UNKNOWN);

  /* Geometry type */
  if (has_geo)
  {
    if (geometry_type != POINTTYPE || hasM)
      ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
        errmsg("Only point geometries without M dimension accepted")));
    TYPMOD_SET_TYPE(typmod, geometry_type);
    if (hasZ)
      TYPMOD_SET_Z(typmod);
  }

  /* SRID */
  if (has_srid)
  {
    if (srid != SRID_UNKNOWN)
      TYPMOD_SET_SRID(typmod, srid);
  }

  /* Shift to restore the 4 bits of the temporal type */
  TYPMOD_SET_SUBTYPE(typmod, temp_subtype);

  pfree(elem_values);
  return typmod;
}

PG_FUNCTION_INFO_V1(tgeompoint_typmod_in);
/**
 * Input typmod information for temporal geometric points
 */
PGDLLEXPORT Datum
tgeompoint_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tpoint_typmod_in(array, false); /* Not a geography  */;
  PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(tgeogpoint_typmod_in);
/**
 * Input typmod information for temporal geographic points
 */
PGDLLEXPORT Datum
tgeogpoint_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  int32 typmod = tpoint_typmod_in(array, true);
  // int srid = TYPMOD_GET_SRID(typmod);
  // /* Check the SRID is legal (geographic coordinates) */
  // srid_is_latlong(fcinfo, srid);
  PG_RETURN_INT32(typmod);
}

PG_FUNCTION_INFO_V1(tpoint_typmod_out);
/**
 * Output typmod information for temporal points
 */
PGDLLEXPORT Datum
tpoint_typmod_out(PG_FUNCTION_ARGS)
{
  char *s = (char *) palloc(64);
  char *str = s;
  int32 typmod = PG_GETARG_INT32(0);
  int16 temp_subtype = TYPMOD_GET_SUBTYPE(typmod);
  TYPMOD_DEL_SUBTYPE(typmod);
  int32 srid = TYPMOD_GET_SRID(typmod);
  uint8_t geometry_type = (uint8_t) TYPMOD_GET_TYPE(typmod);
  int32 hasz = TYPMOD_GET_Z(typmod);

  /* No temporal type or geometry type? Then no typmod at all.
    Return empty string. */
  if (typmod < 0 || (temp_subtype == ANYTEMPSUBTYPE && !geometry_type))
  {
    *str = '\0';
    PG_RETURN_CSTRING(str);
  }
  /* Opening bracket */
  str += sprintf(str, "(");
  /* Has temporal type?  */
  if (temp_subtype != ANYTEMPSUBTYPE)
    str += sprintf(str, "%s", tempsubtype_name(temp_subtype));
  if (geometry_type)
  {
    if (temp_subtype != ANYTEMPSUBTYPE) str += sprintf(str, ",");
    str += sprintf(str, "%s", lwtype_name(geometry_type));
    /* Has Z?  */
    if (hasz) str += sprintf(str, "Z");
    /* Has SRID?  */
    if (srid) str += sprintf(str, ",%d", srid);
  }
  /* Closing bracket.  */
  sprintf(str, ")");

  PG_RETURN_CSTRING(s);
}

PG_FUNCTION_INFO_V1(tpoint_enforce_typmod);
/**
 * Enforce typmod information for temporal points with respect to
 * temporal type, dimensions, and SRID
 */
PGDLLEXPORT Datum
tpoint_enforce_typmod(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  int32 typmod = PG_GETARG_INT32(1);
  /* Check if typmod of temporal point is consistent with the supplied one */
  temp = tpoint_valid_typmod(temp, typmod);
  PG_RETURN_POINTER(temp);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpointinst_constructor);
/**
 * Construct a temporal instant point value from the arguments
 */
PGDLLEXPORT Datum
tpointinst_constructor(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  ensure_point_type(gs);
  ensure_non_empty(gs);
  ensure_has_not_M_gs(gs);
  TimestampTz t = PG_GETARG_TIMESTAMPTZ(1);
  Oid basetypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = (Temporal *) tinstant_make(PointerGetDatum(gs), t,
    basetypid);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_to_stbox);
/**
 * Returns the bounding box of the temporal point value
 */
PGDLLEXPORT Datum
tpoint_to_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  STBOX *result = palloc0(sizeof(STBOX));
  temporal_bbox(result, temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * Returns the temporal comparison of the base value and temporal value
 */
Datum
tcomp_geo_tpoint(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  ensure_point_type(gs);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 0);
  Temporal *result = tcomp_temporal_base1(temp, PointerGetDatum(gs),
    datumtypid, func, true);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/**
 * Returns the temporal comparison of the temporal value and the base value
 */
Datum
tcomp_tpoint_geo(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum, Oid, Oid))
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_point_type(gs);
  ensure_same_srid(tpoint_srid_internal(temp), gserialized_get_srid(gs));
  ensure_same_dimensionality_tpoint_gs(temp, gs);
  Oid datumtypid = get_fn_expr_argtype(fcinfo->flinfo, 1);
  Temporal *result = tcomp_temporal_base1(temp, PointerGetDatum(gs),
    datumtypid, func, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(teq_geo_tpoint);
/**
 * Returns the temporal equality of the base value and the temporal value
 */
PGDLLEXPORT Datum
teq_geo_tpoint(PG_FUNCTION_ARGS)
{
  return tcomp_geo_tpoint(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(teq_tpoint_geo);
/**
 * Returns the temporal equality of the temporal value and base value
 */
PGDLLEXPORT Datum
teq_tpoint_geo(PG_FUNCTION_ARGS)
{
  return tcomp_tpoint_geo(fcinfo, &datum2_eq2);
}

PG_FUNCTION_INFO_V1(tne_geo_tpoint);
/**
 * Returns the temporal difference of the base value and the temporal value
 */
PGDLLEXPORT Datum
tne_geo_tpoint(PG_FUNCTION_ARGS)
{
  return tcomp_geo_tpoint(fcinfo, &datum2_ne2);
}

PG_FUNCTION_INFO_V1(tne_tpoint_geo);
/**
 * Returns the temporal difference of the temporal value and base value
 */
PGDLLEXPORT Datum
tne_tpoint_geo(PG_FUNCTION_ARGS)
{
  return tcomp_tpoint_geo(fcinfo, &datum2_ne2);
}

/*****************************************************************************
 * Assemble the Trajectory/values of a temporal point as a single
 * geometry/geography.
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tpoint_values);
/**
 * Returns the base values (that is, the trajectory) of the temporal point
 * value as a geometry/geography
 */
PGDLLEXPORT Datum
tpoint_values(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum result = tpoint_trajectory_external(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
