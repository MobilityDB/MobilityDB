/***********************************************************************
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
 * @brief Analytic functions for temporal points and temporal floats.
 */

/* C */
#include <assert.h>
#include <float.h>
#include <math.h>
/* PostgreSQL */
#include <postgres.h>
#include <funcapi.h>
#include <utils/float.h>
#include <utils/timestamp.h>
/* PostGIS */
#include <liblwgeom_internal.h>
#include <lwgeodetic_tree.h>
/* MEOS */
#include <meos.h>
#include "general/lifting.h"
#include "point/geography_funcs.h"
#include "point/tpoint.h"
#include "point/tpoint_boxops.h"
#include "point/tpoint_spatialrels.h"
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_general/type_util.h"
#include "pg_point/postgis.h"

/*****************************************************************************/

PGDLLEXPORT Datum Temporal_simplify_min_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_min_dist);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Simplify the temporal sequence (set) float or point ensuring that
 * consecutive values are at least a certain distance apart.
 */
Datum
Temporal_simplify_min_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  /* Store fcinfo to compute the geodetic distance */
  store_fcinfo(fcinfo);
  Temporal *result = temporal_simplify_min_dist(temp, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_min_tdelta(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_min_tdelta);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Simplify the temporal sequence (set) float or point ensuring that
 * consecutive values are at least a certain distance apart.
 */
Datum
Temporal_simplify_min_tdelta(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Interval *mint = PG_GETARG_INTERVAL_P(1);
  Temporal *result = temporal_simplify_min_tdelta(temp, mint);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_max_dist(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_max_dist);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Simplify the temporal sequence (set) float or point using a
 * single-pass Douglas-Peucker line simplification algorithm.
 */
Datum
Temporal_simplify_max_dist(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = temporal_simplify_max_dist(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Temporal_simplify_dp(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Temporal_simplify_dp);
/**
 * @ingroup mobilitydb_temporal_analytics_simplify
 * @brief Simplify the temporal sequence (set) float or point using a
 * Douglas-Peucker line simplification algorithm.
 */
Datum
Temporal_simplify_dp(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(1);
  bool syncdist = true;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
    syncdist = PG_GETARG_BOOL(2);
  Temporal *result = temporal_simplify_dp(temp, dist, syncdist);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
