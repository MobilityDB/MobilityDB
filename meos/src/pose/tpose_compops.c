/***********************************************************************
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
 * @brief Ever/always and temporal comparisons for temporal poses
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/pg_types.h"
#include "general/lifting.h"
#include "general/temporal.h"
#include "general/temporal_compops.h"
#include "general/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/tpose.h"
#include "pose/tpose_spatialfuncs.h"

/*****************************************************************************
 * Ever/always comparisons
 *****************************************************************************/

/**
 * @brief Return true if a temporal pose and a pose satisfy the ever/always
 * comparison
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_tpose_pose(const Temporal *temp, const Pose *pose,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) pose) ||
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return -1;
#else
  assert(temp); assert(pose); assert(func); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */
  if (! ensure_same_srid(tspatial_srid(temp), pose_srid(pose)))
    return -1;
  return eacomp_temporal_base(temp, PointerGetDatum(pose), func, ever);
}

/**
 * @brief Return true if two temporal poses satisfy the ever/always
 * comparison
 * @param[in] temp1,temp2 Temporal values
 * @param[in] ever True for the ever semantics, false for the always semantics
 * @param[in] func Comparison function
 */
static int
eacomp_tpose_tpose(const Temporal *temp1, const Temporal *temp2,
  Datum (*func)(Datum, Datum, meosType), bool ever)
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp1) || ! ensure_not_null((void *) temp2) || 
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp1, T_TPOSE) ||
      ! ensure_temporal_isof_type(temp2, T_TPOSE))
    return -1;
#else
  assert(temp1); assert(temp2); assert(func); 
  assert(temp1->temptype == T_TPOSE); assert(temp2->temptype == T_TPOSE);
#endif /* MEOS */
  if (! ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return -1;
  return eacomp_temporal_temporal(temp1, temp2, func, ever);
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a pose is ever equal to a temporal circular
 * buffer
 * @param[in] pose Pose
 * @param[in] temp Temporal value
 * @csqlfn #Ever_eq_pose_tpose()
 */
int
ever_eq_pose_tpose(const Pose *pose, const Temporal *temp)
{
  return eacomp_tpose_pose(temp, pose, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a temporal pose is ever equal to a circular
 * buffer
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @csqlfn #Ever_eq_tpose_pose()
 */
int
ever_eq_tpose_pose(const Temporal *temp, const Pose *pose)
{
  return eacomp_tpose_pose(temp, pose, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a pose is ever different from a temporal pose
 * @param[in] pose Pose
 * @param[in] temp Temporal value
 * @csqlfn #Ever_ne_pose_tpose()
 */
int
ever_ne_pose_tpose(const Pose *pose, const Temporal *temp)
{
  return eacomp_tpose_pose(temp, pose, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a temporal pose is ever different from a
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @csqlfn #Ever_ne_tpose_pose()
 */
int
ever_ne_tpose_pose(const Temporal *temp, const Pose *pose)
{
  return eacomp_tpose_pose(temp, pose, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a pose is always equal to a temporal pose
 * @param[in] pose Pose
 * @param[in] temp Temporal value
 * @csqlfn #Always_eq_pose_tpose()
 */
int
always_eq_pose_tpose(const Pose *pose, const Temporal *temp)
{
  return eacomp_tpose_pose(temp, pose, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a temporal pose is always equal to a
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @csqlfn #Always_eq_tpose_pose()
 */
int
always_eq_tpose_pose(const Temporal *temp, const Pose *pose)
{
  return eacomp_tpose_pose(temp, pose, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a pose is always different from a temporal pose
 * @param[in] pose Pose
 * @param[in] temp Temporal value
 * @csqlfn #Always_ne_pose_tpose()
 */
int
always_ne_pose_tpose(const Pose *pose, const Temporal *temp)
{
  return eacomp_tpose_pose(temp, pose, &datum2_ne, ALWAYS);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if a temporal pose is always different from a
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @csqlfn #Always_ne_tpose_pose()
 */
int
always_ne_tpose_pose(const Temporal *temp, const Pose *pose)
{
  return eacomp_tpose_pose(temp, pose, &datum2_ne, ALWAYS);
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if two temporal poses are ever equal
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Ever_eq_tpose_tpose()
 */
int
ever_eq_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpose_tpose(temp1, temp2, &datum2_eq, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if two temporal poses are ever different
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Ever_ne_tpose_tpose()
 */
int
ever_ne_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpose_tpose(temp1, temp2, &datum2_ne, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if two temporal poses are always equal
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Always_eq_tpose_tpose()
 */
int
always_eq_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpose_tpose(temp1, temp2, &datum2_eq, ALWAYS);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return true if two temporal poses are always different
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Always_ne_tpose_tpose()
 */
int
always_ne_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  return eacomp_tpose_tpose(temp1, temp2, &datum2_ne, ALWAYS);
}

/*****************************************************************************
 * Temporal comparisons
 *****************************************************************************/

/**
 * @brief Return the temporal comparison of a pose and a temporal pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_pose_tpose(const Pose *pose, const Temporal *temp,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) pose) || 
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(pose); assert(func); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */
  if (! ensure_same_srid(tspatial_srid(temp), pose_srid(pose)))
    return NULL;
  return tcomp_base_temporal(PointerGetDatum(pose), temp, func);
}

/**
 * @brief Return the temporal comparison of a temporal pose and a
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @param[in] func Comparison function
 */
static Temporal *
tcomp_tpose_pose(const Temporal *temp, const Pose *pose,
  Datum (*func)(Datum, Datum, meosType))
{
  /* Ensure validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) pose) || 
      ! ensure_not_null((void *) func) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(pose); assert(func); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */
  if (! ensure_same_srid(tspatial_srid(temp), pose_srid(pose)))
    return NULL;
  return tcomp_temporal_base(temp, PointerGetDatum(pose), func);
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_comp_temp
 * @brief Return the temporal equality of a pose and a temporal pose
 * @param[in] pose Pose
 * @param[in] temp Temporal value
 * @csqlfn #Teq_pose_tpose()
 */
Temporal *
teq_pose_tpose(const Pose *pose, const Temporal *temp)
{
  return tcomp_pose_tpose(pose, temp, &datum2_eq);
}

/**
 * @ingroup meos_pose_comp_temp
 * @brief Return the temporal inequality of a pose and a temporal pose
 * @param[in] pose Pose
 * @param[in] temp Temporal value
 * @csqlfn #Tne_pose_tpose()
 */
Temporal *
tne_pose_tpose(const Pose *pose, const Temporal *temp)
{
  return tcomp_pose_tpose(pose, temp, &datum2_ne);
}

/*****************************************************************************/

/**
 * @ingroup meos_pose_comp_temp
 * @brief Return the temporal equality of a temporal pose and a
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @csqlfn #Teq_tpose_pose()
 */
Temporal *
teq_tpose_pose(const Temporal *temp, const Pose *pose)
{
  return tcomp_tpose_pose(temp, pose, &datum2_eq);
}

/**
 * @ingroup meos_pose_comp_temp
 * @brief Return the temporal inequality of a temporal pose and a
 * pose
 * @param[in] temp Temporal value
 * @param[in] pose Pose
 * @csqlfn #Tne_tpose_pose()
 */
Temporal *
tne_tpose_pose(const Temporal *temp, const Pose *pose)
{
  return tcomp_tpose_pose(temp, pose, &datum2_ne);
}

/*****************************************************************************/
