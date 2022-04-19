/***********************************************************************
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
 * @file tpoint_spatialfuncs.c
 * @brief Spatial functions for temporal points.
 */

#include "point/tpoint_spatialfuncs.h"

/* PostgreSQL */
#include <assert.h>
/* PostGIS */
#if POSTGIS_VERSION_NUMBER >= 30000
#include <liblwgeom.h>
#include <liblwgeom_internal.h>
#include <lwgeodetic.h>
#endif
/* MobilityDB */
#include "general/period.h"
#include "general/periodset.h"
#include "general/time_ops.h"
#include "general/rangetypes_ext.h"
#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/tnumber_mathfuncs.h"
#include "point/postgis.h"
#include "point/stbox.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_distance.h"
#include "point/tpoint_spatialrels.h"

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_ever_eq);
/**
 * Return true if the temporal value is ever equal to the base value
 */
PGDLLEXPORT Datum
Tpoint_ever_eq(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool result = tpoint_ever_eq(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_BOOL(result);
}

PG_FUNCTION_INFO_V1(Tpoint_get_trajectory);
/**
 * Return the trajectory of a temporal point
 */
PGDLLEXPORT Datum
Tpoint_get_trajectory(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = tpoint_trajectory(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_get_srid);
/**
 * Return the SRID of a temporal point
 */
PGDLLEXPORT Datum
Tpoint_get_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tpoint_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PG_FUNCTION_INFO_V1(Tpoint_set_srid);
/**
 * Set the SRID of a temporal point
 */
PGDLLEXPORT Datum
Tpoint_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tpoint_set_srid(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_transform);
/**
 * Transform a temporal point into another spatial reference system
 */
PGDLLEXPORT Datum
Tpoint_transform(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum srid = PG_GETARG_DATUM(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_transform(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Cast functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tgeompoint_to_tgeogpoint);
/**
 * Convert a temporal geometry point to a temporal geography point
 */
PGDLLEXPORT Datum
Tgeompoint_to_tgeogpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOM_TO_GEOG);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tgeogpoint_to_tgeompoint);
/**
 * Convert a temporal geography point to a temporal geometry point
 */
PGDLLEXPORT Datum
Tgeogpoint_to_tgeompoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tgeompoint_tgeogpoint(temp, GEOG_TO_GEOM);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Set precision of the coordinates
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Geo_round);
/**
 * Sets the precision of the coordinates of the geometry
 */
PGDLLEXPORT Datum
Geo_round(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Datum prec = PG_GETARG_DATUM(1);
  PG_RETURN_POINTER(datum_round_geo(PointerGetDatum(gs), prec));
}

PG_FUNCTION_INFO_V1(Tpoint_round);
/**
 * Set the precision of the coordinates of the temporal point to the number
 * of decimal places
 */
PGDLLEXPORT Datum
Tpoint_round(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum prec = PG_GETARG_DATUM(1);
  Temporal *result = tpoint_round(temp, prec);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Functions for extracting coordinates
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_get_x);
/**
 * Get the X coordinates of the temporal point
 */
PGDLLEXPORT Datum
Tpoint_get_x(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 0);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_get_y);
/**
 * Get the Y coordinates of the temporal point
 */
PGDLLEXPORT Datum
Tpoint_get_y(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 1);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_get_z);
/**
 * Get the Z coordinates of the temporal point
 */
PGDLLEXPORT Datum
Tpoint_get_z(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpoint_get_coord(temp, 2);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Length functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_length);
/**
 * Return the length traversed by the temporal sequence (set) point
 */
PGDLLEXPORT Datum
Tpoint_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = tpoint_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(Tpoint_cumulative_length);
/**
 * Return the cumulative length traversed by the temporal point
 */
PGDLLEXPORT Datum
Tpoint_cumulative_length(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_cumulative_length(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_speed);
/**
 * Return the speed of the temporal point
 */
PGDLLEXPORT Datum
Tpoint_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Time-weighed centroid for temporal geometry points
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_twcentroid);
/**
 * Return the time-weighed centroid of the temporal geometry point
 */
PGDLLEXPORT Datum
Tpoint_twcentroid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = tpoint_twcentroid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_azimuth);
/**
 * Return the temporal azimuth of the temporal geometry point
 */
PGDLLEXPORT Datum
Tpoint_azimuth(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tpoint_azimuth(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal bearing
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Bearing_geo_geo);
/**
 * Return the temporal bearing between the geometry/geography points
 *
 * @note The following function is meant to be included in PostGIS one day
 */
PGDLLEXPORT Datum
Bearing_geo_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo1 = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *geo2 = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Datum result;
  bool found = bearing_geo_geo(geo1, geo2, &result);
  PG_FREE_IF_COPY(geo1, 0);
  PG_FREE_IF_COPY(geo2, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Bearing_geo_tpoint);
/**
 * Return the temporal bearing between the geometry/geography point
 * and the temporal point
 */
PGDLLEXPORT Datum
Bearing_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_geo(temp, gs, INVERT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Bearing_tpoint_geo);
/**
 * Return the temporal bearing between the temporal point and the
 * geometry/geography point
 */
PGDLLEXPORT Datum
Bearing_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_geo(temp, gs, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Bearing_tpoint_tpoint);
/**
 * Return the temporal bearing between the two temporal points
 */
PGDLLEXPORT Datum
Bearing_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = bearing_tpoint_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Non self-intersecting (a.k.a. simple) functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_is_simple);
/**
 * Return true if the temporal point does not self-intersect.
 */
PGDLLEXPORT Datum
Tpoint_is_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_BOOL(tpoint_is_simple(temp));
}

PG_FUNCTION_INFO_V1(Tpoint_make_simple);
/**
 * Split a temporal point into an array of non self-intersecting pieces
 */
PGDLLEXPORT Datum
Tpoint_make_simple(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int count;
  Temporal **pieces = tpoint_make_simple(temp, &count);
  ArrayType *result = temporalarr_to_array((const Temporal **) pieces, count);
  pfree_array((void **) pieces, count);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * Restricts the temporal point to the (complement of the) geometry
 *
 * Mixing 2D/3D is enabled to compute, for example, 2.5D operations
 */
static Datum
tpoint_restrict_geometry_ext(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tpoint_restrict_geometry(temp, geo, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_at_geometry);
/**
 * Restricts the temporal point to the geometry
 */
PGDLLEXPORT Datum
Tpoint_at_geometry(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geometry_ext(fcinfo, REST_AT);
}

PG_FUNCTION_INFO_V1(Tpoint_minus_geometry);
/**
 * Restrict the temporal point to the complement of the geometry
 */
PGDLLEXPORT Datum
Tpoint_minus_geometry(PG_FUNCTION_ARGS)
{
  return tpoint_restrict_geometry_ext(fcinfo, REST_MINUS);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Tpoint_at_stbox);
/**
 * Restricts the temporal value to the spatiotemporal box
 */
PGDLLEXPORT Datum
Tpoint_at_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, REST_AT);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tpoint_minus_stbox);
/**
 * Restricts the temporal value to the complement of the spatiotemporal box
 */
PGDLLEXPORT Datum
Tpoint_minus_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBOX *box = PG_GETARG_STBOX_P(1);
  Temporal *result = tpoint_restrict_stbox(temp, box, REST_MINUS);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
