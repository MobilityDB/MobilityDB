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
 * @brief Spatial functions for temporal pose objects.
 */

#include "pose/tpose_spatialfuncs.h"

/* PostgreSQL */
#include <stdio.h>
#include <utils/palloc.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "point/tpoint_spatialfuncs.h"
#include "pose/tpose.h"
#include "pose/tpose_static.h"

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * @brief Return true if a segment of a temporal pose value intersects
 * a base value at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp
 */
bool
tposesegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  /* We are sure that the trajectory is a line */
  Datum start = tinstant_value(inst1);
  Datum end = tinstant_value(inst2);
  Datum geom_start = datum_pose_geom(start);
  Datum geom_end = datum_pose_geom(end);
  Datum geom = datum_pose_geom(value);
  double dist;
  /* Compute the value taking into account position only */
  double fraction = (double) geosegm_locate_point(geom_start, geom_end, geom, &dist);
  if (fabs(dist) >= MEOS_EPSILON)
    return false;
  /* Compare value with interpolated pose to take into account orientation as well */
  Pose *pose1 = DatumGetPoseP(start);
  Pose *pose2 = DatumGetPoseP(end);
  Pose *pose = DatumGetPoseP(value);
  Pose *pose_interp = pose_interpolate(pose1, pose2, fraction);
  bool eq = pose_eq(pose, pose_interp);
  pfree(pose_interp);
  if (!eq)
    return false;
  if (t != NULL)
  {
    double duration = (double) (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/*****************************************************************************/

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal pose is ever equal to a pose.
 * @sqlop @p ?=
 */
bool
tpose_ever_eq(const Temporal *temp, const Pose *pose)
{
  /* Ensure validity of the arguments */
  if (! ensure_same_srid(tpose_srid(temp), pose_get_srid(pose)) ||
      ! ensure_same_spatial_dimensionality(temp->flags, pose->flags))
    return false;
  return temporal_ever_eq(temp, PosePGetDatum(pose));
}

/**
 * @ingroup libmeos_temporal_ever
 * @brief Return true if a temporal pose is always equal to a pose.
 * @sqlop @p %=
 */
bool
tpose_always_eq(const Temporal *temp, const Pose *pose)
{
  /* Ensure validity of the arguments */
  if (! ensure_same_srid(tpose_srid(temp), pose_get_srid(pose)) ||
      ! ensure_same_spatial_dimensionality(temp->flags, pose->flags))
    return false;
  return temporal_always_eq(temp, PosePGetDatum(pose));
}

/*****************************************************************************/
