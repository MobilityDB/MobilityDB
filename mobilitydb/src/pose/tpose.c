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
 * @brief General functions for temporal pose objects
 */

/* PostgreSQL */
#include <postgres.h>
#include <utils/array.h>
#include <pgtypes.h>                  /* text_to_cstring / cstring_to_text */
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include "temporal/set.h"
#include "geo/tgeo_spatialfuncs.h"
#include "geo/tspatial_parser.h"
#include "pose/pose.h"
/* MobilityDB */
#include "pg_temporal/meos_catalog.h"
#include "pg_geo/tspatial.h"
#include "pg_geo/postgis.h"

/*****************************************************************************
 * Input/output functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpose_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_in);
/**
 * @ingroup mobilitydb_pose_inout
 * @brief Generic input function for temporal pose objects
 *
 * @note Examples of input for the various temporal types:
 * - Instant
 * @code
 * Pose(0, 0, 0) @ 2012-01-01 08:00:00
 * @endcode
 * - Discrete sequence
 * @code
 * { Pose(0, 0, 0) @ 2012-01-01 08:00:00 , Pose(1, 1, 0) @ 2012-01-01 08:10:00 }
 * @endcode
 * - Continuous sequence
 * @code
 * [ Pose(0, 0, 0) @ 2012-01-01 08:00:00 , Pose(1, 1, 0) @ 2012-01-01 08:10:00 )
 * @endcode
 * - Sequence set
 * @code
 * { [ Pose(0, 0, 0) @ 2012-01-01 08:00:00 , Pose(1, 1, 0) @ 2012-01-01 08:10:00 ) ,
 *   [ Pose(1, 1, 0) @ 2012-01-01 08:20:00 , Pose(0, 0, 0) @ 2012-01-01 08:30:00 ] }
 * @endcode
 */
Datum
Tpose_in(PG_FUNCTION_ARGS)
{
  const char *input = PG_GETARG_CSTRING(0);
  Temporal *result = tpose_in(input);
  PG_RETURN_POINTER(result);
}

PGDLLEXPORT Datum Tpose_typmod_in(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_typmod_in);
/**
 * @brief Input typmod information for temporal poses
 */
Datum
Tpose_typmod_in(PG_FUNCTION_ARGS)
{
  ArrayType *array = (ArrayType *) DatumGetPointer(PG_GETARG_DATUM(0));
  uint32 typmod = tspatial_typmod_in(array, true, false);
  PG_RETURN_INT32(typmod);
}

/*****************************************************************************
 * Constructor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpose_make(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_make);
/**
 * @ingroup mobilitydb_pose_constructor
 * @brief Construct a temporal 2D pose from a temporal point and a temporal
 * float
 * @sqlfn tpose()
 */
Datum
Tpose_make(PG_FUNCTION_ARGS)
{
  Temporal *tpoint = PG_GETARG_TEMPORAL_P(0);
  Temporal *ttheta = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tpose_make(tpoint, ttheta);
  PG_FREE_IF_COPY(tpoint, 0);
  PG_FREE_IF_COPY(ttheta, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpose_to_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_to_tpoint);
/**
 * @ingroup mobilitydb_pose_conversion
 * @brief Convert a temporal pose into a temporal point
 * @sqlfn tgeompoint(), tgeogpoint()
 * @sqlop @p ::
 */
Datum
Tpose_to_tpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* A pose carries its own reference frame, so the pose decides the type of the
   * point: a planar pose yields a temporal geometry point and a geodetic one a
   * temporal geography point. Both conversions share this function, so the frame
   * of the argument must agree with the type the caller asked for — converting
   * to the other one returns a value whose declared type contradicts its
   * coordinates, which every operator dispatching on that type then misreads */
  MeosType restype = oid_meostype(get_fn_expr_rettype(fcinfo->flinfo));
  bool valid = (restype == T_TGEOGPOINT) ?
    ensure_geodetic(temp->flags) : ensure_not_geodetic(temp->flags);
  if (! valid)
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_RETURN_NULL();
  }
  Temporal *result = tpose_to_tpoint(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

PGDLLEXPORT Datum Tpose_points(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_points);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the array of points of a temporal pose
 * @sqlfn points()
 */
Datum
Tpose_points(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Set *result = tpose_points(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_SET_P(result);
}

PGDLLEXPORT Datum Tpose_yaw(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_yaw);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the yaw of a temporal pose (radians) as a temporal float
 * @sqlfn yaw()
 */
Datum
Tpose_yaw(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpose_yaw(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_pitch(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_pitch);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the pitch of a temporal pose (radians) as a temporal float
 * @sqlfn pitch()
 */
Datum
Tpose_pitch(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpose_pitch(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_roll(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_roll);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the roll of a temporal pose (radians) as a temporal float
 * @sqlfn roll()
 */
Datum
Tpose_roll(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpose_roll(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_speed);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the speed of a temporal pose (distance per unit time
 * along the position component) as a temporal float
 * @sqlfn speed()
 */
Datum
Tpose_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpose_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_angular_speed(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_angular_speed);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the angular speed of a temporal pose (radians per unit
 * time) as a step-interpolated temporal float
 * @sqlfn angularSpeed()
 */
Datum
Tpose_angular_speed(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpose_angular_speed(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * OGC GeoPose temporal JSON I/O
 *****************************************************************************/

PGDLLEXPORT Datum Tpose_from_geopose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_from_geopose);
/**
 * @ingroup mobilitydb_pose_inout
 * @brief Return a temporal pose from its TemporalGeoPose JSON envelope
 * @sqlfn tposeFromGeoPose()
 */
Datum
Tpose_from_geopose(PG_FUNCTION_ARGS)
{
  text *json_text = PG_GETARG_TEXT_P(0);
  char *json = text_to_cstring(json_text);
  Temporal *result = tpose_from_geopose(json);
  pfree(json);
  PG_FREE_IF_COPY(json_text, 0);
  if (result == NULL) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_apply_pose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_apply_pose);
/**
 * @ingroup mobilitydb_pose_transf
 * @brief Return the composition of a temporal pose with the frame a pose names
 * @sqlfn applyPose()
 */
Datum
Tpose_apply_pose(PG_FUNCTION_ARGS)
{
  Temporal *body = PG_GETARG_TEMPORAL_P(0);
  Pose *frame = PG_GETARG_POSE_P(1);
  Temporal *result = tpose_compose_pose(body, frame);
  PG_FREE_IF_COPY(body, 0);
  PG_FREE_IF_COPY(frame, 1);
  if (result == NULL) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Pose_apply_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_apply_tpose);
/**
 * @ingroup mobilitydb_pose_transf
 * @brief Return the composition of a pose with the frame a temporal pose names
 * @sqlfn applyPose()
 */
Datum
Pose_apply_tpose(PG_FUNCTION_ARGS)
{
  Pose *body = PG_GETARG_POSE_P(0);
  Temporal *frame = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = pose_compose_tpose(body, frame);
  PG_FREE_IF_COPY(body, 0);
  PG_FREE_IF_COPY(frame, 1);
  if (result == NULL) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_apply_tpose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_apply_tpose);
/**
 * @ingroup mobilitydb_pose_transf
 * @brief Return the composition of two temporal poses
 * @sqlfn applyPose()
 */
Datum
Tpose_apply_tpose(PG_FUNCTION_ARGS)
{
  Temporal *body = PG_GETARG_TEMPORAL_P(0);
  Temporal *frame = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tpose_compose_tpose(body, frame);
  PG_FREE_IF_COPY(body, 0);
  PG_FREE_IF_COPY(frame, 1);
  if (result == NULL) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_inverse(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_inverse);
/**
 * @ingroup mobilitydb_pose_transf
 * @brief Return the inverse of a temporal pose
 * @sqlfn poseInverse()
 */
Datum
Tpose_inverse(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *result = tpose_inverse(temp);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tpose_as_geopose(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_as_geopose);
/**
 * @ingroup mobilitydb_pose_inout
 * @brief Return the TemporalGeoPose JSON envelope of a temporal pose
 * @sqlfn asGeoPose()
 */
Datum
Tpose_as_geopose(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int conformance = PG_GETARG_INT32(1);
  int precision   = PG_GETARG_INT32(2);
  char *result = tpose_as_geopose(temp, conformance, precision);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL) PG_RETURN_NULL();
  text *result_text = cstring_to_text(result);
  pfree(result);
  PG_RETURN_TEXT_P(result_text);
}

PGDLLEXPORT Datum Tpose_as_geopose_stream(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_as_geopose_stream);
/**
 * @ingroup mobilitydb_pose_inout
 * @brief Return the OGC GeoPose Stream of a temporal pose
 * @sqlfn asGeoPoseStream()
 */
Datum
Tpose_as_geopose_stream(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int precision = PG_GETARG_INT32(1);
  char *result = tpose_as_geopose_stream(temp, precision);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL) PG_RETURN_NULL();
  text *result_text = cstring_to_text(result);
  pfree(result);
  PG_RETURN_TEXT_P(result_text);
}

/*****************************************************************************
 * Body-to-world rigid transform (workstream #4)
 *****************************************************************************/

PGDLLEXPORT Datum Tpose_apply_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_apply_geo);
/**
 * @ingroup mobilitydb_pose_accessor
 * @brief Return the world-frame temporal trajectory obtained by applying
 * a temporal pose to a body-frame point geometry
 * @sqlfn applyPose()
 */
Datum
Tpose_apply_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *body = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = tpose_apply_geo(temp, body);
  PG_FREE_IF_COPY(body, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result) PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
