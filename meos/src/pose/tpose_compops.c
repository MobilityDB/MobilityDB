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
 * @brief Ever/always and temporal comparisons for temporal poses
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <pgtypes.h>
#include "temporal/lifting.h"
#include "temporal/temporal.h"
#include "temporal/temporal_compops.h"
#include "temporal/type_util.h"
#include "geo/tgeo_spatialfuncs.h"
#include "pose/pose.h"
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
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_pose(temp, pose))
    return -1;
  assert(func);
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
  Datum (*func)(Datum, Datum, MeosType), bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_tpose(temp1, temp2))
    return -1;
  assert(func);
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
 * Ever/always overlaps of a temporal pose and a spatiotemporal box
 *****************************************************************************/

/**
 * @brief Return 1 if a temporal pose ever (or always) lies in a
 * spatiotemporal box, 0 if not, and -1 on error, as #eaoverlaps_tgeo_stbox
 * answers it for a temporal geo
 * @details The value ever lies in the box when its restriction to the box by
 * #tpose_restrict_stbox is not empty, and always lies in it when its
 * restriction to the complement of the box is empty; a box with only the T
 * dimension restricts the value to its period
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tpose_stbox(const Temporal *temp, const STBox *box, bool ever)
{
  /* Ensure the validity of the arguments: a box with the X dimension shares
   * the SRID of the value, as #tpose_restrict_stbox reads it */
  VALIDATE_TPOSE(temp, -1); VALIDATE_NOT_NULL(box, -1);
  bool hasx = MEOS_FLAGS_GET_X(box->flags);
  if (hasx && ! ensure_valid_tpose_stbox(temp, box))
    return -1;

  bool atfunc = ever ? REST_AT : REST_MINUS;
  Temporal *res = hasx ? tpose_restrict_stbox(temp, box, true, atfunc) :
    temporal_restrict_tstzspan(temp, &box->period, atfunc);
  int result = ever ? (res != NULL) : (res == NULL);
  if (res)
    pfree(res);
  return result;
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return 1 if a temporal pose ever lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @csqlfn #Ever_overlaps_tpose_stbox()
 */
int
ever_overlaps_tpose_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tpose_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return 1 if a temporal pose ever lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal pose
 * @csqlfn #Ever_overlaps_stbox_tpose()
 */
int
ever_overlaps_stbox_tpose(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tpose_stbox(temp, box, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return 1 if a temporal pose always lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @csqlfn #Always_overlaps_tpose_stbox()
 */
int
always_overlaps_tpose_stbox(const Temporal *temp, const STBox *box)
{
  return eaoverlaps_tpose_stbox(temp, box, ALWAYS);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return 1 if a temporal pose always lies in a spatiotemporal
 * box, 0 if not, and -1 on error
 * @param[in] box Spatiotemporal box
 * @param[in] temp Temporal pose
 * @csqlfn #Always_overlaps_stbox_tpose()
 */
int
always_overlaps_stbox_tpose(const STBox *box, const Temporal *temp)
{
  return eaoverlaps_tpose_stbox(temp, box, ALWAYS);
}

/**
 * @brief Return 1 if the values of two temporal poses ever (or always)
 * have overlapping bounding boxes at an instant they share, 0 if not, and -1
 * on error, as #eaoverlaps_tgeo_tgeo answers it for two temporal geos
 * @details The bounding box of the value is the box of its position, so two
 * values overlap at an instant when their positions are equal, which the
 * ever and always equality of the temporal points of their positions answers
 * across the instants where linear segments cross
 * @param[in] temp1,temp2 Temporal poses
 * @param[in] ever True for the ever semantics, false for the always semantics
 */
static int
eaoverlaps_tpose_tpose(const Temporal *temp1, const Temporal *temp2,
  bool ever)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_tpose(temp1, temp2))
    return -1;
  Temporal *tpoint1 = tpose_to_tpoint(temp1);
  Temporal *tpoint2 = tpose_to_tpoint(temp2);
  int result = ever ? ever_eq_tgeo_tgeo(tpoint1, tpoint2) :
    always_eq_tgeo_tgeo(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return 1 if the values of two temporal poses ever have overlapping bounding boxes at an
 * instant they share, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Ever_overlaps_tpose_tpose()
 */
int
ever_overlaps_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  return eaoverlaps_tpose_tpose(temp1, temp2, EVER);
}

/**
 * @ingroup meos_pose_comp_ever
 * @brief Return 1 if the values of two temporal poses always have overlapping bounding boxes at
 * the instants they share, 0 if not, and -1 on error
 * @param[in] temp1,temp2 Temporal poses
 * @csqlfn #Always_overlaps_tpose_tpose()
 */
int
always_overlaps_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  return eaoverlaps_tpose_tpose(temp1, temp2, ALWAYS);
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
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(pose, NULL);
  assert(func);
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
  Datum (*func)(Datum, Datum, MeosType))
{
  /* Ensure the validity of the arguments */
  VALIDATE_TPOSE(temp, NULL); VALIDATE_NOT_NULL(pose, NULL);
  assert(func);
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
