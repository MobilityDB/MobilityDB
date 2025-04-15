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
 * @brief Temporal distance for temporal poses
 */

#include "pose/tpose_distance.h"

/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/tpose.h"
#include "pose/tpose_spatialfuncs.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @ingroup meos_pose_dist
 * @brief Return the temporal distance between a geometry and a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] gs Geometry
 * @csqlfn #Distance_tpose_point()
 */
Temporal *
distance_tpose_point(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Temporal *tpoint = tpose_tpoint(temp);
  Temporal *result = distance_tgeo_geo((const Temporal *) tpoint, gs);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the temporal distance between a temporal pose and a pose
 * @param[in] temp Temporal pose
 * @param[in] pose Pose
 * @csqlfn #Distance_tpose_pose()
 */
Temporal *
distance_tpose_pose(const Temporal *temp, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_pose(temp, pose))
    return NULL;

  GSERIALIZED *geom = pose_point(pose);
  Temporal *tpoint = tpose_tpoint(temp);
  Temporal *result = distance_tgeo_geo(tpoint, geom);
  pfree(geom);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the temporal distance between two temporal poses
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Distance_tpose_tpose()
 */
Temporal *
distance_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_tpose(temp1, temp2))
    return NULL;

  Temporal *tpoint1 = tpose_tpoint(temp1);
  Temporal *tpoint2 = tpose_tpoint(temp2);
  Temporal *result = distance_tgeo_tgeo(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_pose_dist
 * @brief Return the nearest approach instant of the temporal pose and a
 * geometry
 * @param[in] temp Temporal pose
 * @param[in] gs Geometry
 * @csqlfn #NAI_tpose_geo()
 */
TInstant *
nai_tpose_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Temporal *tpoint = tpose_tpoint(temp);
  TInstant *resultgeom = nai_tgeo_geo(tpoint, gs);
  /* We do not call the function tgeompointinst_tposeinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the nearest approach instant of a pose and a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] pose Pose
 * @csqlfn #NAI_tpose_pose()
 */
TInstant *
nai_tpose_pose(const Temporal *temp, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_pose(temp, pose))
    return NULL;

  GSERIALIZED *geom = pose_point(pose);
  Temporal *tpoint = tpose_tpoint(temp);
  TInstant *res = nai_tgeo_geo(tpoint, geom);
  /* We do not call the function tgeompointinst_tposeinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, res->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, res->t);
  pfree(geom); pfree(tpoint); pfree(res);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the nearest approach instant of two temporal poses
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #NAI_tpose_tpose()
 */
TInstant *
nai_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_tpose(temp1, temp2))
    return NULL;

  Temporal *dist = distance_tpose_tpose(temp1, temp2);
  if (dist == NULL)
    return NULL;

  const TInstant *min = temporal_min_instant((const Temporal *) dist);
  pfree(dist);
  /* The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp1, min->t, false, &value);
  return tinstant_make_free(value, temp1->temptype, min->t);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_pose_dist
 * @brief Return the nearest approach distance of two temporal pose
 * and a geometry
 * @param[in] temp Temporal pose
 * @param[in] gs Geometry
 * @csqlfn #NAD_tpose_geo()
 */
double
nad_tpose_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_geo(temp, gs) || gserialized_is_empty(gs))
    return -1.0;

  GSERIALIZED *traj = tpose_trajectory(temp);
  double result = geom_distance2d(traj, gs);
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the nearest approach distance of a temporal pose
 * and a pose
 * @param[in] temp Temporal pose
 * @param[in] pose Pose
 * @csqlfn #NAD_tpose_pose()
 */
double
nad_tpose_pose(const Temporal *temp, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_pose(temp, pose))
    return -1.0;

  GSERIALIZED *geom = pose_point(pose);
  GSERIALIZED *traj = tpose_trajectory(temp);
  double result = geom_distance2d(traj, geom);
  pfree(traj); pfree(geom);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the nearest approach distance of two temporal poses
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #NAD_tpose_tpose()
 */
double
nad_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_tpose(temp1, temp2))
    return -1.0;

  Temporal *dist = distance_tpose_tpose(temp1, temp2);
  if (dist == NULL)
    return -1.0;
  return DatumGetFloat8(temporal_min_value(dist));
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup meos_pose_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] gs Geometry
 * @csqlfn #Shortestline_tpose_geo()
 */
GSERIALIZED *
shortestline_tpose_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  GSERIALIZED *traj = tpose_trajectory(temp);
  GSERIALIZED *result = geom_shortestline2d(traj, gs);
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the line connecting the nearest approach point between a
 * pose and a temporal pose
 * @param[in] temp Temporal pose
 * @param[in] pose Pose
 * @csqlfn #Shortestline_tpose_pose()
 */
GSERIALIZED *
shortestline_tpose_pose(const Temporal *temp, const Pose *pose)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_pose(temp, pose))
    return NULL;

  GSERIALIZED *geom = pose_point(pose);
  GSERIALIZED *traj = tpose_trajectory(temp);
  GSERIALIZED *result = geom_shortestline2d(traj, geom);
  pfree(geom); pfree(traj);
  return result;
}

/**
 * @ingroup meos_pose_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal networks
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Shortestline_tpose_tpose()
 */
GSERIALIZED *
shortestline_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_tpose(temp1, temp2))
    return NULL;

  Temporal *tpoint1 = tpose_tpoint(temp1);
  Temporal *tpoint2 = tpose_tpoint(temp2);
  GSERIALIZED *result = shortestline_tgeo_tgeo(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************/
