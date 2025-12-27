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
 * @brief Distance functions for temporal rigid geometries.
 */

#include "rgeo/trgeo_distance.h"

/* C */
#include <assert.h>
#include <float.h>
#include <fmgr.h>
#include <math.h>
/* PostgreSQL */
#include <utils/timestamp.h>
#include <utils/float.h>
/* PostGIS */
#include <lwgeodetic_tree.h>
#include <liblwgeom.h>
#include <measures.h>
#include <measures3d.h>
/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
#include <meos_internal.h>
#include "temporal/temporal.h"
#include "temporal/type_util.h"
#include "temporal/meos_catalog.h"
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
#include "rgeo/trgeo_all.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tdistance_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the temporal distance between a temporal rigid geometry and a
 * geometry
 * @sqlfn tdistance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Tdistance_trgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdistance_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdistance_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the temporal distance between a geometry and a temporal rigid
 * geometry
 * @sqlfn tdistance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Tdistance_geo_trgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdistance_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdistance_trgeo_tpoint);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the temporal distance between two temporal rigid geometries
 * @sqlfn tdistance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Tdistance_trgeo_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdistance_trgeo_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdistance_tpoint_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the temporal distance between two temporal rigid geometries
 * @sqlfn tdistance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Tdistance_tpoint_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdistance_trgeo_tpoint(temp2, temp1);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdistance_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the temporal distance between two temporal rigid geometries
 * @sqlfn tdistance()
 * @sqlop @p <->
 */
PGDLLEXPORT Datum
Tdistance_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  Temporal *result = tdistance_trgeo_trgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAI_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach instant between a temporal rigid geometry
 * and a geometry
 * @sqlfn nearestApproachInstant()
 */
PGDLLEXPORT Datum
NAI_trgeo_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach instant between a geometry and a temporal
 * rigid geometry
 * @sqlfn nearestApproachInstant()
 */
PGDLLEXPORT Datum
NAI_geo_trgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_trgeo_tpoint);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach instant between the
 * temporal rigid geometries
 * @sqlfn nearestApproachInstant()
 */
PGDLLEXPORT Datum
NAI_trgeo_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_trgeo_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tpoint_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach instant between the
 * temporal rigid geometries
 * @sqlfn nearestApproachInstant()
 */
PGDLLEXPORT Datum
NAI_tpoint_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_trgeo_tpoint(temp2, temp1);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach instant between two temporal rigid
 * geometries
 * @sqlfn nearestApproachInstant()
 */
PGDLLEXPORT Datum
NAI_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_trgeo_trgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_trgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between a geometry and a
 * temporal rigid geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_geo_trgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_trgeo_stbox);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between a temporal rigid
 * geometry and a spatiotemporal box
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_trgeo_stbox(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_stbox_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between a spatiotemporal box and
 * a temporal rigid geometry
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_stbox_trgeo(PG_FUNCTION_ARGS)
{
  STBox *box = PG_GETARG_STBOX_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_stbox(temp, box);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_trgeo_tpoint);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_trgeo_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tpoint_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_tpoint_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_tpoint(temp2, temp1);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the nearest approach distance between two temporal rigid
 * geometries
 * @sqlfn nearestApproachDistance()
 * @sqlop @p |=|
 */
PGDLLEXPORT Datum
NAD_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  double result = nad_trgeo_trgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Shortestline_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * temporal rigid geometry and a geometry
 * @sqlfn shortestLine()
 */
PGDLLEXPORT Datum
Shortestline_trgeo_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  GSERIALIZED *result = shortestline_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Shortestline_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal rigid geometry
 * @sqlfn shortestLine()
 */
PGDLLEXPORT Datum
Shortestline_geo_trgeo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  GSERIALIZED *result = shortestline_trgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Shortestline_trgeo_tpoint);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal rigid geometries
 * @sqlfn shortestLine()
 */
PGDLLEXPORT Datum
Shortestline_trgeo_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  GSERIALIZED *result = shortestline_trgeo_tpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Shortestline_tpoint_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal rigid geometries
 * @sqlfn shortestLine()
 */
PGDLLEXPORT Datum
Shortestline_tpoint_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  GSERIALIZED *result = shortestline_trgeo_tpoint(temp2, temp1);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Shortestline_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal rigid geometries
 * @sqlfn shortestLine()
 */
PGDLLEXPORT Datum
Shortestline_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  GSERIALIZED *result = shortestline_trgeo_trgeo(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
