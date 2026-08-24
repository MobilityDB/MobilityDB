/***********************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Functions on geometries answered without calling GEOS
 */

/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>
#include <funcapi.h>
/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Oriented envelope (a.k.a minimum rotated rectangle) and convex hull
 *****************************************************************************/

PGDLLEXPORT Datum Geom_oriented_envelope(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_oriented_envelope);
/**
 * @ingroup mobilitydb_geo_base_spatial
 * @brief Return the oriented envelope of a geometry
 * @sqlfn orientedEnvelope()
 */
Datum
Geom_oriented_envelope(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = geom_oriented_envelope(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

PGDLLEXPORT Datum Geom_convex_hull(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_convex_hull);
/**
 * @ingroup mobilitydb_geo_base_spatial
 * @brief Return the convex hull of a geometry
 * @sqlfn convexHull()
 */
Datum
Geom_convex_hull(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  GSERIALIZED *result = geom_convex_hull(gs);
  PG_FREE_IF_COPY(gs, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************
 * Simple geometries
 *****************************************************************************/

PGDLLEXPORT Datum Geom_is_simple(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_is_simple);
/**
 * @ingroup mobilitydb_geo_base_accessor
 * @brief Return true if a geometry has no anomalous point, which is a point
 * at which it crosses or touches itself
 * @sqlfn isSimple()
 */
Datum
Geom_is_simple(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  bool result = geom_is_simple(gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_RETURN_BOOL(result);
}

/*****************************************************************************
 * Buffer
 *****************************************************************************/

PGDLLEXPORT Datum Geom_buffer(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Geom_buffer);
/**
 * @ingroup mobilitydb_geo_base_spatial
 * @brief Return a geometry that represents all the points whose distance from
 * a geometry is less than or equal to a distance
 * @sqlfn buffer()
 */
Datum
Geom_buffer(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  double radius = PG_GETARG_FLOAT8(1);
  text *params_text = PG_GETARG_TEXT_P(2);
  char *params = text_to_cstring(params_text);
  GSERIALIZED *result = geom_buffer(gs, radius, params);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(params_text, 2);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_GSERIALIZED_P(result);
}

/*****************************************************************************/
