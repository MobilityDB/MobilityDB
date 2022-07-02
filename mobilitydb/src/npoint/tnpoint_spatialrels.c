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
 * @brief Ever spatial relationships for temporal network points.
 *
 * These relationships compute the ever spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported:
 * contains, disjoint, intersects, touches, and dwithin
 */

#include "npoint/tnpoint_spatialrels.h"

/* PostgreSQL */
#include <postgres.h>
#include <utils/palloc.h>
#include <fmgr.h>
/* MobilityDB */
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic binary functions for tnpoint <rel> (geo | Npoint)
 *****************************************************************************/

/**
 * @brief Generic spatial relationships for a temporal network point and a geometry
 */
Datum
spatialrel_geo_tnpoint_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum))
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = spatialrel_tnpoint_geo(temp, geom, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/**
 * @brief Generic spatial relationships for a temporal network point and a geometry
 */
static Datum
spatialrel_tnpoint_geo_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  Datum result = spatialrel_tnpoint_geo(temp, geom, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * @brief Generic spatial relationships for a temporal network point and a network point
 */
static Datum
spatialrel_npoint_tnpoint_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum))
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = spatialrel_tnpoint_npoint(temp, np, func, INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/**
 * @brief Generic spatial relationships for a temporal network point and a network point
 */
static Datum
spatialrel_tnpoint_npoint_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  Datum result = spatialrel_tnpoint_npoint(temp, np, func, INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * @brief Return true if the temporal network points ever satisfy the spatial
 * relationship
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Spatial relationship
 */
static Datum
spatialrel_tnpoint_tnpoint_ext(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = spatialrel_tnpoint_tnpoint(temp1, temp2, func);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result == 1 ? true : false);
}

/*****************************************************************************
 * Ever contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Contains_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the geometry contains the trajectory of the temporal network
 * point
 * @sqlfunc contains()
 */
PGDLLEXPORT Datum
Contains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tnpoint_ext(fcinfo, &geom_contains);
}

/*****************************************************************************
 * Ever disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Disjoint_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the geometry and the trajectory of the temporal network
 * point are disjoint
 * @sqlfunc disjoint()
 */
PGDLLEXPORT Datum
Disjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tnpoint_ext(fcinfo, &geom_disjoint2d);
}

PG_FUNCTION_INFO_V1(Disjoint_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the network point and the trajectory of the temporal
 * network point are disjoint
 * @sqlfunc disjoint()
 */
PGDLLEXPORT Datum
Disjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_npoint_tnpoint_ext(fcinfo, &geom_disjoint2d);
}

PG_FUNCTION_INFO_V1(Disjoint_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * geometry are disjoint
 * @sqlfunc disjoint()
 */
PGDLLEXPORT Datum
Disjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_geo_ext(fcinfo, &geom_disjoint2d);
}

PG_FUNCTION_INFO_V1(Disjoint_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * network point are disjoint
 * @sqlfunc disjoint()
 */
PGDLLEXPORT Datum
Disjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_npoint_ext(fcinfo, &geom_disjoint2d);
}

PG_FUNCTION_INFO_V1(Disjoint_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the temporal points are ever disjoint
 * @sqlfunc disjoint()
 */
PGDLLEXPORT Datum
Disjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_tnpoint_ext(fcinfo, &datum2_point_ne);
}

/*****************************************************************************
 * Ever intersects
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Intersects_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the geometry and the trajectory of the temporal network
 * point intersect
 * @sqlfunc intersects()
 */
PGDLLEXPORT Datum
Intersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tnpoint_ext(fcinfo, &geom_intersects2d);
}

PG_FUNCTION_INFO_V1(Intersects_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the network point and the trajectory of the temporal network
 * point intersect
 * @sqlfunc intersects()
 */
PGDLLEXPORT Datum
Intersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_npoint_tnpoint_ext(fcinfo, &geom_intersects2d);
}

PG_FUNCTION_INFO_V1(Intersects_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * geometry intersect
 * @sqlfunc intersects()
 */
PGDLLEXPORT Datum
Intersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_geo_ext(fcinfo, &geom_intersects2d);
}

PG_FUNCTION_INFO_V1(Intersects_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the network
 * point intersect
 * @sqlfunc intersects()
 */
PGDLLEXPORT Datum
Intersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_npoint_ext(fcinfo, &geom_intersects2d);
}

PG_FUNCTION_INFO_V1(Intersects_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the temporal points are ever disjoint
 * @sqlfunc intersects()
 */
PGDLLEXPORT Datum
Intersects_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_tnpoint_ext(fcinfo, &datum2_point_eq);
}

/*****************************************************************************
 * Ever dwithin
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Dwithin_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the geometry and the trajectory of the temporal network
 * point are within the given distance
 * @sqlfunc dwithin()
 */
PGDLLEXPORT Datum
Dwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d,
    INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Dwithin_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the network point and the trajectory of the temporal network
 * point are within the given distance
 * @sqlfunc dwithin()
 */
PGDLLEXPORT Datum
Dwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Datum result = spatialrel3_tnpoint_npoint(temp, np, dist, &geom_dwithin2d,
    INVERT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Dwithin_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * geometry are within the given distance
 * @sqlfunc dwithin()
 */
PGDLLEXPORT Datum
Dwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  double dist = PG_GETARG_FLOAT8(2);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d,
    INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Dwithin_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * network point are within the given distance
 * @sqlfunc dwithin()
 */
PGDLLEXPORT Datum
Dwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Datum result = spatialrel3_tnpoint_npoint(temp, np, dist, &geom_dwithin2d,
    INVERT_NO);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Dwithin_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectories of the temporal network points are within
 * the given distance
 * @sqlfunc dwithin()
 */
PGDLLEXPORT Datum
Dwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = dwithin_tnpoint_tnpoint(temp1, temp2, dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * Ever touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Touches_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the geometry and the trajectory of the temporal network
 * point touch
 * @sqlfunc touches()
 */
PGDLLEXPORT Datum
Touches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_geo_tnpoint_ext(fcinfo, &geom_touches);
}

PG_FUNCTION_INFO_V1(Touches_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the network point and the trajectory of the temporal
 * network point touch
 * @sqlfunc touches()
 */
PGDLLEXPORT Datum
Touches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_npoint_tnpoint_ext(fcinfo, &geom_touches);
}

PG_FUNCTION_INFO_V1(Touches_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * geometry touch
 * @sqlfunc touches()
 */
PGDLLEXPORT Datum
Touches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_geo_ext(fcinfo, &geom_touches);
}

PG_FUNCTION_INFO_V1(Touches_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if the trajectory of the temporal network point and the
 * network point touch
 * @sqlfunc touches()
 */
PGDLLEXPORT Datum
Touches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_npoint_ext(fcinfo, &geom_touches);
}

/*****************************************************************************/
