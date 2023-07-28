/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * @brief Ever spatial relationships for temporal points.
 *
 * These relationships compute the ever spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported for geometries: `contains`,
 * `edisjoint`, `eintersects`, `etouches`, and `edwithin`.
 *
 * The following relationships are supported for geographies: `edisjoint`,
 * `eintersects`, `edwithin`.
 *
 * Only `edisjoint`, `edwithin`, and `eintersects` are supported for 3D geometries.
 */

#include "point/tpoint_spatialrels.h"

/* PostgreSQL */
#include <assert.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/type_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_tempspatialrels.h"
/* MobilityDB */
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * PostGIS functions calls for reusing their cache
 *****************************************************************************/

/**
 * @brief Call the PostGIS function ST_Intersects with the 2 arguments
 */
Datum
pgis_intersects2d(Datum geom1, Datum geom2)
{
  return CallerFInfoFunctionCall2(ST_Intersects, (fetch_fcinfo())->flinfo,
    InvalidOid, geom1, geom2);
}

/*****************************************************************************
 * Generic ever spatial relationship functions
 *****************************************************************************/

/**
 * @brief Return true if the temporal points ever satisfy the spatial
 * relationship
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func1, func2 Spatial relationship for geometric/geographic points
 */
static Datum
espatialrel_tpoint_tpoint_ext(FunctionCallInfo fcinfo,
  Datum (*func1)(Datum, Datum), Datum (*func2)(Datum, Datum))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    espatialrel_tpoint_tpoint(temp1, temp2, func2) :
    espatialrel_tpoint_tpoint(temp1, temp2, func1);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************
 * Ever contains
 * The function does not accept 3D or geography since it is based on the
 * PostGIS ST_Relate function
 *****************************************************************************/

PGDLLEXPORT Datum Econtains_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a geometry ever contains a temporal point
 * @sqlfunc econtains
 */
Datum
Econtains_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = econtains_geo_tpoint(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Ever disjoint (for both geometry and geography)
 *****************************************************************************/

PGDLLEXPORT Datum Edisjoint_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a geometry and a temporal point are ever disjoint
 * @sqlfunc edisjoint
 */
Datum
Edisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = edisjoint_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a temporal point and a geometry are ever disjoint
 * @sqlfunc edisjoint
 */
Datum
Edisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = edisjoint_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the temporal points are ever disjoint
 * @sqlfunc edisjoint
 */
Datum
Edisjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return espatialrel_tpoint_tpoint_ext(fcinfo, &datum2_point_ne,
    &datum2_point_nsame);
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

PGDLLEXPORT Datum Eintersects_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a geometry and a temporal point ever intersect
 * @sqlfunc eintersects
 */
Datum
Eintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = eintersects_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a temporal point and a geometry ever intersect
 * @sqlfunc eintersects
 */
Datum
Eintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = eintersects_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the temporal points ever intersect
 * @sqlfunc eintersects
 */
Datum
Eintersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return espatialrel_tpoint_tpoint_ext(fcinfo, &datum2_point_eq,
    &datum2_point_same);
}

/*****************************************************************************
 * Ever touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

PGDLLEXPORT Datum Etouches_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a geometry and a temporal point ever touch
 * @sqlfunc etouches
 */
Datum
Etouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = etouches_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a temporal point and a geometry ever touch
 * @sqlfunc etouches
 */
Datum
Etouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = etouches_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Ever dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

PGDLLEXPORT Datum Edwithin_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a geometry and a temporal point are ever within the
 * given distance
 * @sqlfunc edwithin
 */
Datum
Edwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = edwithin_tpoint_geo(temp, gs, dist);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if a temporal point and a geometry are ever within the
 * given distance
 * @sqlfunc edwithin
 */
Datum
Edwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = edwithin_tpoint_geo(temp, gs, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the temporal points are even within the given distance
 * @sqlfunc edwithin
 */
Datum
Edwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  int result = edwithin_tpoint_tpoint(temp1, temp2, dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

/*****************************************************************************/
