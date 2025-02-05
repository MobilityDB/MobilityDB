/*****************************************************************************
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
 * @brief Spatial functions for temporal circular buffers.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_cbuffer.h>
#include <meos_internal.h>
#include "general/span.h"
#include "point/stbox.h"
#include "point/tpoint_restrfuncs.h"
// #include "cbuffer/tcbuffer_spatialfuncs.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Geometric positions (Trajectory) functions
 * Return the geometric positions covered by a temporal circular buffer
 *****************************************************************************/

// PGDLLEXPORT Datum Tcbuffer_trajectory(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_trajectory);
// /**
 // * @ingroup mobilitydb_temporal_spatial_accessor
 // * @brief Return the geometry covered by a temporal circular buffer
 // * @sqlfn trajectory()
 // */
// Datum
// Tcbuffer_trajectory(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // GSERIALIZED *result = tcbuffer_trajectory(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_RETURN_TEMPORAL_P(result);
// }

/*****************************************************************************
 * Geographical equality for circular buffers
 *****************************************************************************/

PGDLLEXPORT Datum Cbuffer_same(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Cbuffer_same);
// /**
 // * @ingroup mobilitydb_temporal_spatial_accessor
 // * @brief Return true if two circular buffers are spatially equal
 // * @sqlfn same()
 // */
// Datum
// Cbuffer_same(PG_FUNCTION_ARGS)
// {
  // Cbuffer *cbuf1 = PG_GETARG_CBUFFER_P(0);
  // Cbuffer *cbuf2 = PG_GETARG_CBUFFER_P(1);
  // PG_RETURN_BOOL(cbuffer_same(cbuf1, cbuf2));
// }

/*****************************************************************************
 * Length functions
 *****************************************************************************/

// PGDLLEXPORT Datum Tcbuffer_length(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_length);
// /**
 // * @ingroup mobilitydb_temporal_spatial_accessor
 // * @brief Return the length traversed by a temporal circular buffer
 // * @sqlfn length()
 // */
// Datum
// Tcbuffer_length(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // double result = tcbuffer_length(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_RETURN_FLOAT8(result);
// }

// PGDLLEXPORT Datum Tcbuffer_cumulative_length(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_cumulative_length);
// /**
 // * @ingroup mobilitydb_temporal_spatial_accessor
 // * @brief Return the cumulative length traversed by a temporal circular buffer
 // * @sqlfn cumulativeLength()
 // */
// Datum
// Tcbuffer_cumulative_length(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // Temporal *result = tcbuffer_cumulative_length(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_RETURN_TEMPORAL_P(result);
// }

/*****************************************************************************
 * Speed functions
 *****************************************************************************/

// PGDLLEXPORT Datum Tcbuffer_speed(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_speed);
// /**
 // * @ingroup mobilitydb_temporal_spatial_accessor
 // * @brief Return the speed of a temporal circular buffer
 // * @sqlfn speed()
 // */
// Datum
// Tcbuffer_speed(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // Temporal *result = tcbuffer_speed(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // if (! result)
    // PG_RETURN_NULL();
  // PG_RETURN_TEMPORAL_P(result);
// }

/*****************************************************************************
 * Time-weighed centroid for temporal circular buffers
 *****************************************************************************/

// PGDLLEXPORT Datum Tcbuffer_twcentroid(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_twcentroid);
// /**
 // * @ingroup mobilitydb_temporal_agg
 // * @brief Return the time-weighed centroid of a temporal circular buffer
 // * @sqlfn twCentroid()
 // */
// Datum
// Tcbuffer_twcentroid(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // GSERIALIZED *result = tcbuffer_twcentroid(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_RETURN_GSERIALIZED_P(result);
// }

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

// PGDLLEXPORT Datum Tcbuffer_azimuth(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_azimuth);
// /**
 // * @ingroup mobilitydb_temporal_spatial_accessor
 // * @brief Return the temporal azimuth of a temporal circular buffer
 // * @sqlfn azimuth()
 // */
// Datum
// Tcbuffer_azimuth(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // Temporal *result = tcbuffer_azimuth(temp);
  // PG_FREE_IF_COPY(temp, 0);
  // if (! result)
    // PG_RETURN_NULL();
  // PG_RETURN_TEMPORAL_P(result);
// }

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

// /**
 // * @brief Return a temporal circular buffer restricted to (the complement of) a
 // * geometry
 // */
// static Datum
// Tcbuffer_restrict_geom(FunctionCallInfo fcinfo, bool atfunc)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  // Temporal *result = tcbuffer_restrict_geom(temp, gs, NULL, atfunc);
  // PG_FREE_IF_COPY(temp, 0);
  // PG_FREE_IF_COPY(gs, 1);
  // if (! result)
    // PG_RETURN_NULL();
  // PG_RETURN_TEMPORAL_P(result);
// }

// PGDLLEXPORT Datum Tcbuffer_at_geom(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_at_geom);
// /**
 // * @ingroup mobilitydb_temporal_restrict
 // * @brief Return a temporal circular buffer restricted to a geometry
 // * @sqlfn atGeometry()
 // */
// Datum
// Tcbuffer_at_geom(PG_FUNCTION_ARGS)
// {
  // return Tcbuffer_restrict_geom(fcinfo, REST_AT);
// }

// PGDLLEXPORT Datum Tcbuffer_minus_geom(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_minus_geom);
// /**
 // * @ingroup mobilitydb_temporal_restrict
 // * @brief Return a temporal circular buffer restricted to the complement of a
 // * geometry
 // * @sqlfn minusGeometry()
 // */
// Datum
// Tcbuffer_minus_geom(PG_FUNCTION_ARGS)
// {
  // return Tcbuffer_restrict_geom(fcinfo, REST_MINUS);
// }

/*****************************************************************************/

// PGDLLEXPORT Datum Tcbuffer_at_stbox(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_at_stbox);
// /**
 // * @ingroup mobilitydb_temporal_restrict
 // * @brief Return a temporal circular buffer restricted to a spatiotemporal box
 // * @sqlfn atStbox()
 // */
// Datum
// Tcbuffer_at_stbox(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // STBox *box = PG_GETARG_STBOX_P(1);
  // bool border_inc = PG_GETARG_BOOL(2);
  // Temporal *result = tcbuffer_restrict_stbox(temp, box, border_inc, REST_AT);
  // PG_FREE_IF_COPY(temp, 0);
  // if (! result)
    // PG_RETURN_NULL();
  // PG_RETURN_TEMPORAL_P(result);
// }

// PGDLLEXPORT Datum Tcbuffer_minus_stbox(PG_FUNCTION_ARGS);
// PG_FUNCTION_INFO_V1(Tcbuffer_minus_stbox);
// /**
 // * @ingroup mobilitydb_temporal_restrict
 // * @brief Return a temporal circular buffer restricted to the complement of a
 // * spatiotemporal box
 // * @sqlfn minusStbox()
 // */
// Datum
// Tcbuffer_minus_stbox(PG_FUNCTION_ARGS)
// {
  // Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  // STBox *box = PG_GETARG_STBOX_P(1);
  // bool border_inc = PG_GETARG_BOOL(2);
  // Temporal *result = tcbuffer_restrict_stbox(temp, box, border_inc, REST_MINUS);
  // PG_FREE_IF_COPY(temp, 0);
  // if (! result)
    // PG_RETURN_NULL();
  // PG_RETURN_TEMPORAL_P(result);
// }

/*****************************************************************************/
