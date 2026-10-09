/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Distance functions for temporal geos
 */


/* C */
#include <float.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include "temporal/temporal.h"
#include "geo/stbox.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"
#include "pg_temporal/type_util.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Tdistance_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the temporal distance between a geometry/geography and a
 * temporal geo
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  Temporal *result = tdistance_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_geo_tgeo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_geo_tgeo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the temporal distance between a geometry/geography and a
 * temporal geo
 * @details Implementation of the operator, the 2-argument version of
 * #Tdistance_geo_tgeo() on the spheroid
 * @sqlfn tDistanceOp()
 * @sqlop @p <->
 */
Datum
Tdistance_geo_tgeo_op(PG_FUNCTION_ARGS)
{
  return Tdistance_geo_tgeo(fcinfo);
}

PGDLLEXPORT Datum Tdistance_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the temporal distance between a temporal geo and a
 * geometry/geography
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_tgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  Temporal *result = tdistance_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_tgeo_geo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tgeo_geo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the temporal distance between a temporal geo and a
 * geometry/geography
 * @details Implementation of the operator, the 2-argument version of
 * #Tdistance_tgeo_geo() on the spheroid
 * @sqlfn tDistanceOp()
 * @sqlop @p <->
 */
Datum
Tdistance_tgeo_geo_op(PG_FUNCTION_ARGS)
{
  return Tdistance_tgeo_geo(fcinfo);
}

PGDLLEXPORT Datum Tdistance_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the temporal distance between two temporal geos
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Tdistance_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  Temporal *result = tdistance_tgeo_tgeo(temp1, temp2, spheroid);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdistance_tgeo_tgeo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdistance_tgeo_tgeo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the temporal distance between two temporal geos
 * @details Implementation of the operator, the 2-argument version of
 * #Tdistance_tgeo_tgeo() on the spheroid
 * @sqlfn tDistanceOp()
 * @sqlop @p <->
 */
Datum
Tdistance_tgeo_tgeo_op(PG_FUNCTION_ARGS)
{
  return Tdistance_tgeo_tgeo(fcinfo);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 * These functions are only available for geometries
 *****************************************************************************/

PGDLLEXPORT Datum NAI_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach instant between a geometry and a temporal
 * geo
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  TInstant *result = nai_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach instant between a temporal geo
 * and a geometry
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tgeo_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  TInstant *result = nai_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach instant between two temporal geos
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  TInstant *result = nai_tgeo_tgeo(temp1, temp2, spheroid);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 * These functions are only available for geometries
 *****************************************************************************/

PGDLLEXPORT Datum NAD_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a geometry and a
 * temporal geo
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_geo_tgeo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_tgeo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a geometry and a
 * temporal geo
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_geo_tgeo() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_geo_tgeo_op(PG_FUNCTION_ARGS)
{
  return NAD_geo_tgeo(fcinfo);
}

PGDLLEXPORT Datum NAD_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a temporal geo and a
 * geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tgeo_geo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_geo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a temporal geo and a
 * geometry
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_tgeo_geo() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_geo_op(PG_FUNCTION_ARGS)
{
  return NAD_tgeo_geo(fcinfo);
}

PGDLLEXPORT Datum NAD_geo_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_stbox);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a geometry and
 * a spatiotemporal box
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_geo_stbox(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_stbox_geo(box, gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_geo_stbox_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_stbox_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a geometry and
 * a spatiotemporal box
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_geo_stbox() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_geo_stbox_op(PG_FUNCTION_ARGS)
{
  return NAD_geo_stbox(fcinfo);
}

PGDLLEXPORT Datum NAD_stbox_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_geo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_geo(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_stbox_geo(box, gs, spheroid);
  PG_FREE_IF_COPY(gs, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_geo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_geo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a geometry
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_stbox_geo() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_geo_op(PG_FUNCTION_ARGS)
{
  return NAD_stbox_geo(fcinfo);
}

PGDLLEXPORT Datum NAD_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_stbox);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between two spatiotemporal boxes
 * @sqlfn nearestApproachDistance()
 */
Datum
NAD_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_stbox_stbox(box1, box2, spheroid);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_stbox_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_stbox_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between two spatiotemporal boxes
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_stbox_stbox() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_stbox_op(PG_FUNCTION_ARGS)
{
  return NAD_stbox_stbox(fcinfo);
}

PGDLLEXPORT Datum NAD_stbox_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a temporal geo
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_tgeo(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_tgeo_stbox(temp, box, spheroid);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_tgeo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_tgeo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box
 * and a temporal geo
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_stbox_tgeo() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_tgeo_op(PG_FUNCTION_ARGS)
{
  return NAD_stbox_tgeo(fcinfo);
}

PGDLLEXPORT Datum NAD_tgeo_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_stbox);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a temporal geo and a
 * spatiotemporal box
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_tgeo_stbox(temp, box, spheroid);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tgeo_stbox_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_stbox_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between a temporal geo and a
 * spatiotemporal box
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_tgeo_stbox() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_stbox_op(PG_FUNCTION_ARGS)
{
  return NAD_tgeo_stbox(fcinfo);
}

PGDLLEXPORT Datum NAD_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between two temporal geos
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = nad_tgeo_tgeo(temp1, temp2, spheroid);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tgeo_tgeo_op(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_tgeo_op);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the nearest approach distance between two temporal geos
 * @details Implementation of the operator, the 2-argument version of
 * #NAD_tgeo_tgeo() on the spheroid
 * @sqlfn nearestApproachDistanceOp()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_tgeo_op(PG_FUNCTION_ARGS)
{
  return NAD_tgeo_tgeo(fcinfo);
}

/*****************************************************************************
 * ShortestLine
 * These functions are only available for geometries
 *****************************************************************************/

PGDLLEXPORT Datum Shortestline_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal geo
 * @sqlfn shortestLine()
 */
Datum
Shortestline_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  GSERIALIZED *result = shortestline_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal geo and a geometry/geography
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  GSERIALIZED *result = shortestline_tgeo_geo(temp, gs, spheroid);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal geos
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  GSERIALIZED *result = shortestline_tgeo_tgeo(temp1, temp2, spheroid);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Set-set spatial minimum distance
 *****************************************************************************/

PGDLLEXPORT Datum Mindistance_tgeoarr_tgeoarr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Mindistance_tgeoarr_tgeoarr);
/**
 * @ingroup mobilitydb_geo_dist
 * @brief Return the exact minimum spatial distance between two arrays of
 * temporal geos, the one of geographies on the earth model of the optional
 * third argument
 * @sqlfn minDistance()
 */
Datum
Mindistance_tgeoarr_tgeoarr(PG_FUNCTION_ARGS)
{
  ArrayType *array1 = PG_GETARG_ARRAYTYPE_P(0);
  ArrayType *array2 = PG_GETARG_ARRAYTYPE_P(1);
  int count1, count2;
  Temporal **arr1 = (Temporal **) temparr_extract(array1, &count1);
  Temporal **arr2 = (Temporal **) temparr_extract(array2, &count2);
  bool spheroid = true;
  if (PG_NARGS() > 2)
    spheroid = PG_GETARG_BOOL(2);
  double result = mindistance_tgeoarr_tgeoarr((const Temporal **) arr1, count1,
    (const Temporal **) arr2, count2, spheroid);
  pfree(arr1); pfree(arr2);
  PG_FREE_IF_COPY(array1, 0);
  PG_FREE_IF_COPY(array2, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/
