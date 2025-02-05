/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
/* MEOS */
#include <meos.h>
#include "general/temporal.h"
#include "geo/stbox.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PGDLLEXPORT Datum Distance_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a geometry/geography and a
 * temporal geo
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between a temporal geo and a
 * geometry/geography
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = distance_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Distance_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Distance_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the temporal distance between two temporal geos
 * @sqlfn tDistance()
 * @sqlop @p <->
 */
Datum
Distance_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tgeo_tgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 * These functions are only available for geometries
 *****************************************************************************/

PGDLLEXPORT Datum NAI_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a geometry and a temporal
 * geo
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between a temporal geo
 * and a geometry
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tgeo_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = nai_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TINSTANT_P(result);
}

PGDLLEXPORT Datum NAI_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAI_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach instant between two temporal geos
 * @sqlfn nearestApproachInstant()
 */
Datum
NAI_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tgeo_tgeo(temp1, temp2);
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
 * @ingroup mobilitydb_temporal_dist
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
  double result = nad_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_dist
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
  double result = nad_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_geo_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_geo_stbox);
/**
 * @ingroup mobilitydb_temporal_dist
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
  double result = nad_stbox_geo(box, gs);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_geo);
/**
 * @ingroup mobilitydb_temporal_dist
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
  double result = nad_stbox_geo(box, gs);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_stbox);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between two spatiotemporal boxes
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_stbox_stbox(PG_FUNCTION_ARGS)
{
  STBox *box1 = PG_GETARG_STBOX_P(0);
  STBox *box2 = PG_GETARG_STBOX_P(1);
  double result = nad_stbox_stbox(box1, box2);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_stbox_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_stbox_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
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
  double result = nad_tgeo_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tgeo_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_stbox);
/**
 * @ingroup mobilitydb_temporal_dist
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
  double result = nad_tgeo_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result == DBL_MAX)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PGDLLEXPORT Datum NAD_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(NAD_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the nearest approach distance between two temporal geos
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
Datum
NAD_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tgeo_tgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 * These functions are only available for geometries
 *****************************************************************************/

PGDLLEXPORT Datum Shortestline_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_geo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal geo
 * @sqlfn shortestLine()
 */
Datum
Shortestline_geo_tgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tgeo_geo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal geo and a geometry/geography
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *result = shortestline_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Shortestline_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Shortestline_tgeo_tgeo);
/**
 * @ingroup mobilitydb_temporal_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal geos
 * @sqlfn shortestLine()
 */
Datum
Shortestline_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_tgeo_tgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
