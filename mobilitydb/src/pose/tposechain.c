/*****************************************************************************
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
 * @brief General functions for temporal pose chains
 */

/* PostgreSQL */
#include <postgres.h>
#include <pgtypes.h>                  /* text_to_cstring / cstring_to_text */
#include <fmgr.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "temporal/temporal.h"
#include "geo/tspatial.h"
/* MobilityDB */
#include "pg_temporal/temporal.h"
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Tposechain_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechain_in);
/**
 * @ingroup mobilitydb_posechain_inout
 * @brief Generic input function for temporal pose chain objects
 *
 * @note Examples of input for the various temporal types:
 * - Instant
 * @code
 * PoseChain(Pose(Point(0 0), 0), Pose(Point(1 0), 0)) @ 2012-01-01 08:00:00
 * @endcode
 * - Discrete sequence
 * @code
 * { PoseChain(Pose(Point(0 0), 0)) @ 2012-01-01 08:00:00 ,
 *   PoseChain(Pose(Point(1 1), 0)) @ 2012-01-01 08:10:00 }
 * @endcode
 * - Continuous sequence
 * @code
 * [ PoseChain(Pose(Point(0 0), 0)) @ 2012-01-01 08:00:00 ,
 *   PoseChain(Pose(Point(1 1), 0)) @ 2012-01-01 08:10:00 )
 * @endcode
 * - Sequence set
 * @code
 * { [ PoseChain(Pose(Point(0 0), 0)) @ 2012-01-01 08:00:00 ,
 *     PoseChain(Pose(Point(1 1), 0)) @ 2012-01-01 08:10:00 ) ,
 *   [ PoseChain(Pose(Point(1 1), 0)) @ 2012-01-01 08:20:00 ,
 *     PoseChain(Pose(Point(0 0), 0)) @ 2012-01-01 08:30:00 ] }
 * @endcode
 */
Datum
Tposechain_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Temporal *result = tposechain_in(input);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tposechain_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechain_typmod_in);
/**
 * @brief Input typmod information for temporal pose chains
 */
Datum
Tposechain_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tspatial_typmod_in(array, true, false);
  PG_RETURN_INT32(typmod);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tposechain_to_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechain_to_tpose);
/**
 * @ingroup mobilitydb_posechain_conversion
 * @brief Convert a temporal pose chain into a temporal pose
 * @sqlfn tpose()
 * @sqlop @p ::
 */
Datum
Tposechain_to_tpose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tposechain_to_tpose(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tposechain_num_poses(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechain_num_poses);
/**
 * @ingroup mobilitydb_posechain_accessor
 * @brief Return the number of links every value of a temporal pose chain holds
 * @sqlfn numPoses()
 */
Datum
Tposechain_num_poses(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tposechain_num_poses(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

/*****************************************************************************
 * OGC GeoPose Composite Chain input/output
 *****************************************************************************/

PGDLLEXPORT Datum Tposechain_from_geopose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechain_from_geopose);
/**
 * @ingroup mobilitydb_posechain_inout
 * @brief Return a temporal pose chain from an OGC GeoPose Composite Chain
 * JSON document
 * @sqlfn tposechainFromGeoPose()
 */
Datum
Tposechain_from_geopose(PG_FUNCTION_ARGS)
{
  text *json_text = PG_GETARG_TEXT_P(0);
  char *json = text_to_cstring(json_text);
  Temporal *result = tposechain_from_geopose(json);
  pfree(json);
  PG_FREE_IF_COPY(json_text, 0);
  if (result == NULL) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tposechain_as_geopose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechain_as_geopose);
/**
 * @ingroup mobilitydb_posechain_inout
 * @brief Return the OGC GeoPose Composite Chain JSON representation of a
 * temporal pose chain
 * @sqlfn asGeoPose()
 */
Datum
Tposechain_as_geopose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int precision = PG_GETARG_INT32(1);
  char *result = tposechain_as_geopose(temp, precision);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL) PG_RETURN_NULL();
  text *result_text = cstring_to_text(result);
  pfree(result);
  PG_RETURN_TEXT_P(result_text);
}

PGDLLEXPORT Datum Tposechainarr_as_geopose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tposechainarr_as_geopose);
/**
 * @ingroup mobilitydb_posechain_inout
 * @brief Return the OGC GeoPose Composite Graph JSON representation of an
 * array of temporal pose chains
 * @sqlfn asGeoPose()
 */
Datum
Tposechainarr_as_geopose(PG_FUNCTION_ARGS)
{
  ArrayType *array = PG_GETARG_ARRAYTYPE_P(0);
  int precision = PG_GETARG_INT32(1);
  ensure_not_empty_array(array);
  int count;
  Temporal **temparr = temparr_extract(array, &count);
  char *result = tposechainarr_as_geopose((const Temporal **) temparr, count,
    precision);
  pfree(temparr);
  PG_FREE_IF_COPY(array, 0);
  if (result == NULL) PG_RETURN_NULL();
  text *result_text = cstring_to_text(result);
  pfree(result);
  PG_RETURN_TEXT_P(result_text);
}

/*****************************************************************************/
