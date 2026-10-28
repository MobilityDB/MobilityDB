/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Temporal distance for temporal pose chains
 * @details A pose chain reads as the pose its composition gives, so each
 * function answers what its temporal pose sibling answers for the composed
 * value #tposechain_to_tpose gives. A chain of more than one link composes to
 * a step function, its value between two instants composing rotations rather
 * than interpolating them, so for such a chain the distance is the one the
 * composed pose holds over each step.
 */

/* C */
#include <float.h>
/* MEOS */
#include <meos.h>
#include <meos_pose.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include "temporal/temporal.h"
#include "pose/pose.h"
#include "pose/posechain.h"
#include "pose/tpose.h"
#include "pose/tposechain.h"

/**
 * @brief Return the instant of a temporal pose chain at the time of an
 * instant its composed pose answers, freeing that instant
 * @details The value is read from the chain itself, as #nai_tpose_geo reads
 * it from the temporal pose, since the closest value may be at an exclusive
 * bound
 */
static TInstant *
tposechain_inst_at(const Temporal *temp, TInstant *inst)
{
  if (! inst)
    return NULL;
  Datum value;
  temporal_value_at_timestamptz(temp, inst->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, inst->t);
  pfree(inst);
  return result;
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @ingroup meos_posechain_dist
 * @brief Return the temporal distance between a temporal pose chain and a
 * geometry, as #tdistance_tpose_geo answers it for its composed pose
 * @param[in] temp Temporal pose chain
 * @param[in] gs Geometry
 * @csqlfn #Tdistance_tposechain_geo() #Tdistance_point_tposechain()
 */
Temporal *
tdistance_tposechain_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_geo(temp, gs))
    return NULL;
  Temporal *tpose = tposechain_to_tpose(temp);
  Temporal *result = tdistance_tpose_geo(tpose, gs);
  pfree(tpose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the temporal distance between a temporal pose chain and a
 * pose chain, as #tdistance_tpose_pose answers it for their composed poses
 * @param[in] temp Temporal pose chain
 * @param[in] pc Pose chain
 * @csqlfn #Tdistance_tposechain_posechain() #Tdistance_posechain_tposechain()
 */
Temporal *
tdistance_tposechain_posechain(const Temporal *temp, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_posechain(temp, pc))
    return NULL;
  Temporal *tpose = tposechain_to_tpose(temp);
  Pose *pose = posechain_to_pose(pc);
  Temporal *result = tdistance_tpose_pose(tpose, pose);
  pfree(tpose); pfree(pose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the temporal distance between two temporal pose chains, as
 * #tdistance_tpose_tpose answers it for their composed poses
 * @param[in] temp1,temp2 Temporal pose chains
 * @csqlfn #Tdistance_tposechain_tposechain()
 */
Temporal *
tdistance_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_tposechain(temp1, temp2))
    return NULL;
  Temporal *tpose1 = tposechain_to_tpose(temp1);
  Temporal *tpose2 = tposechain_to_tpose(temp2);
  Temporal *result = tdistance_tpose_tpose(tpose1, tpose2);
  pfree(tpose1); pfree(tpose2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach instant of a temporal pose chain and a
 * geometry, at the time #nai_tpose_geo answers for its composed pose
 * @param[in] temp Temporal pose chain
 * @param[in] gs Geometry
 * @csqlfn #NAI_tposechain_geo() #NAI_geo_tposechain()
 */
TInstant *
nai_tposechain_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_geo(temp, gs))
    return NULL;
  Temporal *tpose = tposechain_to_tpose(temp);
  TInstant *inst = nai_tpose_geo(tpose, gs);
  pfree(tpose);
  return tposechain_inst_at(temp, inst);
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach instant of a temporal pose chain and a
 * pose chain, at the time #nai_tpose_pose answers for their composed poses
 * @param[in] temp Temporal pose chain
 * @param[in] pc Pose chain
 * @csqlfn #NAI_tposechain_posechain() #NAI_posechain_tposechain()
 */
TInstant *
nai_tposechain_posechain(const Temporal *temp, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_posechain(temp, pc))
    return NULL;
  Temporal *tpose = tposechain_to_tpose(temp);
  Pose *pose = posechain_to_pose(pc);
  TInstant *inst = nai_tpose_pose(tpose, pose);
  pfree(tpose); pfree(pose);
  return tposechain_inst_at(temp, inst);
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach instant of two temporal pose chains, at
 * the time #nai_tpose_tpose answers for their composed poses
 * @param[in] temp1,temp2 Temporal pose chains
 * @csqlfn #NAI_tposechain_tposechain()
 */
TInstant *
nai_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_tposechain(temp1, temp2))
    return NULL;
  Temporal *tpose1 = tposechain_to_tpose(temp1);
  Temporal *tpose2 = tposechain_to_tpose(temp2);
  TInstant *inst = nai_tpose_tpose(tpose1, tpose2);
  pfree(tpose1); pfree(tpose2);
  return tposechain_inst_at(temp1, inst);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach distance of a temporal pose chain and a
 * geometry, as #nad_tpose_geo answers it for its composed pose
 * @param[in] temp Temporal pose chain
 * @param[in] gs Geometry
 * @csqlfn #NAD_tposechain_geo() #NAD_geo_tposechain()
 */
double
nad_tposechain_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_geo(temp, gs))
    return DBL_MAX;
  Temporal *tpose = tposechain_to_tpose(temp);
  double result = nad_tpose_geo(tpose, gs);
  pfree(tpose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach distance of a temporal pose chain and a
 * spatiotemporal box, as #nad_tpose_stbox answers it for its composed pose
 * @param[in] temp Temporal pose chain
 * @param[in] box Spatiotemporal box
 * @csqlfn #NAD_tposechain_stbox() #NAD_stbox_tposechain()
 */
double
nad_tposechain_stbox(const Temporal *temp, const STBox *box)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_stbox(temp, box))
    return DBL_MAX;
  Temporal *tpose = tposechain_to_tpose(temp);
  double result = nad_tpose_stbox(tpose, box);
  pfree(tpose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach distance of a temporal pose chain and a
 * pose chain, as #nad_tpose_pose answers it for their composed poses
 * @param[in] temp Temporal pose chain
 * @param[in] pc Pose chain
 * @csqlfn #NAD_tposechain_posechain() #NAD_posechain_tposechain()
 */
double
nad_tposechain_posechain(const Temporal *temp, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_posechain(temp, pc))
    return DBL_MAX;
  Temporal *tpose = tposechain_to_tpose(temp);
  Pose *pose = posechain_to_pose(pc);
  double result = nad_tpose_pose(tpose, pose);
  pfree(tpose); pfree(pose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the nearest approach distance of two temporal pose chains, as
 * #nad_tpose_tpose answers it for their composed poses
 * @param[in] temp1,temp2 Temporal pose chains
 * @csqlfn #NAD_tposechain_tposechain()
 */
double
nad_tposechain_tposechain(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_tposechain(temp1, temp2))
    return DBL_MAX;
  Temporal *tpose1 = tposechain_to_tpose(temp1);
  Temporal *tpose2 = tposechain_to_tpose(temp2);
  double result = nad_tpose_tpose(tpose1, tpose2);
  pfree(tpose1); pfree(tpose2);
  return result;
}

/*****************************************************************************
 * Shortest line
 *****************************************************************************/

/**
 * @ingroup meos_posechain_dist
 * @brief Return the line connecting the nearest approach points of a temporal
 * pose chain and a geometry, as #shortestline_tpose_geo answers it for its
 * composed pose
 * @param[in] temp Temporal pose chain
 * @param[in] gs Geometry
 * @csqlfn #Shortestline_tposechain_geo() #Shortestline_geo_tposechain()
 */
GSERIALIZED *
shortestline_tposechain_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_geo(temp, gs))
    return NULL;
  Temporal *tpose = tposechain_to_tpose(temp);
  GSERIALIZED *result = shortestline_tpose_geo(tpose, gs);
  pfree(tpose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the line connecting the nearest approach points of a temporal
 * pose chain and a pose chain, as #shortestline_tpose_pose answers it for
 * their composed poses
 * @param[in] temp Temporal pose chain
 * @param[in] pc Pose chain
 * @csqlfn #Shortestline_tposechain_posechain() #Shortestline_posechain_tposechain()
 */
GSERIALIZED *
shortestline_tposechain_posechain(const Temporal *temp, const PoseChain *pc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_posechain(temp, pc))
    return NULL;
  Temporal *tpose = tposechain_to_tpose(temp);
  Pose *pose = posechain_to_pose(pc);
  GSERIALIZED *result = shortestline_tpose_pose(tpose, pose);
  pfree(tpose); pfree(pose);
  return result;
}

/**
 * @ingroup meos_posechain_dist
 * @brief Return the line connecting the nearest approach points of two
 * temporal pose chains, as #shortestline_tpose_tpose answers it for their
 * composed poses
 * @param[in] temp1,temp2 Temporal pose chains
 * @csqlfn #Shortestline_tposechain_tposechain()
 */
GSERIALIZED *
shortestline_tposechain_tposechain(const Temporal *temp1,
  const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tposechain_tposechain(temp1, temp2))
    return NULL;
  Temporal *tpose1 = tposechain_to_tpose(temp1);
  Temporal *tpose2 = tposechain_to_tpose(temp2);
  GSERIALIZED *result = shortestline_tpose_tpose(tpose1, tpose2);
  pfree(tpose1); pfree(tpose2);
  return result;
}

/*****************************************************************************/
