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
 * @brief Spatial functions for temporal rigid geometries
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_rgeo.h>
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Traversed area function
 *****************************************************************************/

PGDLLEXPORT Datum Trgeometry_traversed_area(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_traversed_area);
/**
 * @ingroup mobilitydb_rgeo_accessor
 * @brief Return a temporal rigid geometry restricted to a geometry
 * @sqlfn atGeometry()
 */
inline Datum
Trgeometry_traversed_area(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *result = trgeo_traversed_area(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @brief Return a temporal rigid geometry restricted to (the complement of) a
 * geometry
 */
static Datum
Trgeometry_restrict_geom(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = trgeo_restrict_geom(temp, gs, NULL, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeometry_at_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_at_geom);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a geometry
 * @sqlfn atGeometry()
 */
inline Datum
Trgeometry_at_geom(PG_FUNCTION_ARGS)
{
  return Trgeometry_restrict_geom(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Trgeometry_minus_geom(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_minus_geom);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * geometry
 * @sqlfn minusGeometry()
 */
inline Datum
Trgeometry_minus_geom(PG_FUNCTION_ARGS)
{
  return Trgeometry_restrict_geom(fcinfo, REST_MINUS);
}

/*****************************************************************************/

/**
 * @brief Return a temporal rigid geometry restricted to a spatiotemporal box
 */
static Datum
Trgeometry_restrict_stbox(FunctionCallInfo fcinfo, bool atfunc)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  STBox *box = PG_GETARG_STBOX_P(1);
  bool border_inc = PG_GETARG_BOOL(2);
  Temporal *result = trgeo_restrict_stbox(temp, box, border_inc, atfunc);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Trgeometry_at_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_at_stbox);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a spatiotemporal box
 * @sqlfn atStbox()
 */
inline Datum
Trgeometry_at_stbox(PG_FUNCTION_ARGS)
{
  return Trgeometry_restrict_stbox(fcinfo, REST_AT);
}

PGDLLEXPORT Datum Trgeometry_minus_stbox(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Trgeometry_minus_stbox);
/**
 * @ingroup mobilitydb_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * spatiotemporal box
 * @sqlfn minusStbox()
 */
inline Datum
Trgeometry_minus_stbox(PG_FUNCTION_ARGS)
{
  return Trgeometry_restrict_stbox(fcinfo, REST_MINUS);
}

/*****************************************************************************/
