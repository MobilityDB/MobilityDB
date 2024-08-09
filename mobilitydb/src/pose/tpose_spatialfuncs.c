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
 * @brief Spatial functions for temporal poses.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
/* MobilityDB */
#include "pose/tpose.h"
#include "pose/tpose_static.h"
#include "pose/tpose_spatialfuncs.h"

/*****************************************************************************
 * Functions for spatial reference systems
 *****************************************************************************/

PGDLLEXPORT Datum Pose_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal pose
 * @sqlfn SRID()
 */
Datum
Pose_get_srid(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  int result = pose_get_srid(pose);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Pose_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Pose_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal pose
 * @sqlfn SRID()
 */
Datum
Pose_set_srid(PG_FUNCTION_ARGS)
{
  Pose *pose = PG_GETARG_POSE_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Pose *result = pose_copy(pose);
  pose_set_srid(result, srid);
  PG_FREE_IF_COPY(pose, 0);
  PG_RETURN_POSE_P(result);
}

PGDLLEXPORT Datum Tpose_get_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_get_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_accessor
 * @brief Return the SRID of a temporal pose
 * @sqlfn SRID()
 */
Datum
Tpose_get_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = tpose_srid(temp);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Tpose_set_srid(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_set_srid);
/**
 * @ingroup mobilitydb_temporal_spatial_transf
 * @brief Set the SRID of a temporal pose
 * @sqlfn setSRID()
 */
Datum
Tpose_set_srid(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int32 srid = PG_GETARG_INT32(1);
  Temporal *result = tpose_set_srid(temp, srid);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Ever/always functions
 *****************************************************************************/

/**
 * @brief Generic function for the temporal ever/always comparison operators
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Specific function for the ever/always comparison
 */
static Datum
tpose_ev_al_comp_ext(FunctionCallInfo fcinfo,
  bool (*func)(const Temporal *, const Pose *))
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Pose *pose = PG_GETARG_POSE_P(1);
  bool result = func(temp, pose);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(pose, 1);
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Tpose_ever_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_ever_eq);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal pose is ever equal to a pose
 * @sqlfn ever_eq()
 */
Datum
Tpose_ever_eq(PG_FUNCTION_ARGS)
{
  return tpose_ev_al_comp_ext(fcinfo, &tpose_ever_eq);
}

PGDLLEXPORT Datum Tpose_always_eq(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_always_eq);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal pose is always equal to a pose
 * @sqlfn always_eq()
 */
Datum
Tpose_always_eq(PG_FUNCTION_ARGS)
{
  return tpose_ev_al_comp_ext(fcinfo, &tpose_always_eq);
}

PGDLLEXPORT Datum Tpose_ever_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_ever_ne);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal pose is ever different from a pose
 * @sqlfn ever_ne()
 */
Datum
Tpose_ever_ne(PG_FUNCTION_ARGS)
{
  return ! tpose_ev_al_comp_ext(fcinfo, &tpose_always_eq);
}

PGDLLEXPORT Datum Tpose_always_ne(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tpose_always_ne);
/**
 * @ingroup mobilitydb_temporal_comp_ever
 * @brief Return true if a temporal pose is always different from a pose
 * @sqlfn always_ne()
 */
Datum
Tpose_always_ne(PG_FUNCTION_ARGS)
{
  return ! tpose_ev_al_comp_ext(fcinfo, &tpose_ever_eq);
}

/*****************************************************************************/
