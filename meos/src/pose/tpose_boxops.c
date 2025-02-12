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
 * @brief Bounding box operators for temporal pose objects.
 *
 * These operators test the bounding boxes of temporal poses, which are
 * STBox boxes. The following operators are defined:
 *    overlaps, contains, contained, same
 * The operators consider as many dimensions as they are shared in both
 * arguments: only the space dimension, only the time dimension, or both
 * the space and the time dimensions.
 */

#include "pose/tpose_boxops.h"

/* PostgreSQL */
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "pose/pose.h"

/*****************************************************************************
 * Transform a temporal Pose to a STBox
 *****************************************************************************/

/**
 * @brief Set the spatiotemporal box from the pose value.
 * @param[in] pose Pose
 * @param[out] box Spatiotemporal box
 */
bool
pose_set_stbox(const Pose *pose, STBox *box)
{
  GSERIALIZED *geom = pose_geom(pose);
  bool result = geo_set_stbox(geom, box);
  pfree(geom);
  return result;
}

/**
 * @brief Transform a pose and a timestamp to a spatiotemporal box
 */
bool
pose_timestamp_set_stbox(const Pose *pose, TimestampTz t, STBox *box)
{
  pose_set_stbox(pose, box);
  span_set(TimestampTzGetDatum(t), TimestampTzGetDatum(t), true, true,
    T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/**
 * @brief Transform a pose and a period to a spatiotemporal box
 */
bool
pose_period_set_stbox(const Pose *pose, const Span *p, STBox *box)
{
  pose_set_stbox(pose, box);
  memcpy(&box->period, p, sizeof(Span));
  MEOS_FLAGS_SET_T(box->flags, true);
  return true;
}

/*****************************************************************************/

/**
 * @brief Set the spatiotemporal box from the temporal pose value
 * @param[in] inst Temporal pose
 * @param[out] box Spatiotemporal box
 */
void
tposeinst_set_stbox(const TInstant *inst, STBox *box)
{
  pose_set_stbox(DatumGetPoseP(tinstant_value(inst)), box);
  span_set(TimestampTzGetDatum(inst->t), TimestampTzGetDatum(inst->t),
    true, true, T_TIMESTAMPTZ, T_TSTZSPAN, &box->period);
  MEOS_FLAGS_SET_T(box->flags, true);
  return;
}

/**
 * @brief Set the spatiotemporal box from the array of temporal pose values
 * @param[in] instants Temporal pose values
 * @param[in] count Number of elements in the array
 * @param[out] box Spatiotemporal box
 */
void
tposeinstarr_set_stbox(const TInstant **instants, int count, STBox *box)
{
  /* Initialize the bounding box with the first instant */
  tposeinst_set_stbox(instants[0], box);
  /* Prepare for the iteration */
  bool hasz = MEOS_FLAGS_GET_Z(instants[0]->flags);
  bool geodetic = MEOS_FLAGS_GET_GEODETIC(instants[0]->flags);
  for (int i = 1; i < count; i++)
  {
    Pose *pose = DatumGetPoseP(tinstant_value(instants[i]));
    box->xmin = Min(box->xmin, pose->data[0]);
    box->xmax = Max(box->xmax, pose->data[0]);
    box->ymin = Min(box->ymin, pose->data[1]);
    box->ymax = Max(box->ymax, pose->data[1]);
    if (hasz)
    {
      box->zmin = Min(box->zmin, pose->data[2]);
      box->zmax = Max(box->zmax, pose->data[2]);
    }
    box->period.lower = TimestampTzGetDatum(
      Min(DatumGetTimestampTz(box->period.lower), instants[i]->t));
    box->period.upper = TimestampTzGetDatum(
      Max(DatumGetTimestampTz(box->period.upper), instants[i]->t));
  }
  MEOS_FLAGS_SET_Z(box->flags, hasz);
  MEOS_FLAGS_SET_GEODETIC(box->flags, geodetic);
  return;
}

/**
 * @brief Expand the bounding box of a temporal pose sequence with an instant
 * @param[in] seq Temporal sequence
 * @param[in] inst Temporal instant
 */
void
tposeseq_expand_stbox(TSequence *seq, const TInstant *inst)
{
  STBox box;
  tposeinst_set_stbox(inst, &box);
  stbox_expand(&box, (STBox *) TSEQUENCE_BBOX_PTR(seq));
  return;
}

/*****************************************************************************/
