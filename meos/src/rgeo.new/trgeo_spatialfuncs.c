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
 * @brief Spatial functions for temporal pose objects.
 */

#include "pose/tpose_spatialfuncs.h"

/* PostgreSQL */
#include <stdio.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_pose.h>
#include "pose/pose.h"
#include "geo/tgeo_spatialfuncs.h"

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

/**
 * @brief Ensure the validity of a temporal pose and a pose
 */
bool
ensure_valid_tpose_pose(const Temporal *temp, const Pose *pose)
{
  if (ensure_not_null((void *) temp) && ensure_not_null((void *) pose) &&
      ensure_temporal_isof_type(temp, T_TPOSE) &&
      ensure_same_srid(tspatial_srid(temp), pose_srid(pose)))
    return true;
  return false;
}

/**
 * @brief Ensure the validity of two temporal circular buffers
 */
bool
ensure_valid_tpose_tpose(const Temporal *temp1, const Temporal *temp2)
{
  if (ensure_not_null((void *) temp1) && ensure_not_null((void *) temp2) &&
      ensure_temporal_isof_type(temp1, T_TPOSE) &&
      ensure_temporal_isof_type(temp2, T_TPOSE) &&
      ensure_same_srid(tspatial_srid(temp1), tspatial_srid(temp2)))
    return true;
  return false;
}

/**
 * @brief Ensure the validity of a spatiotemporal box and a geometry
 */
bool
ensure_valid_stbox_pose(const STBox *box, const Pose *pose)
{
  if (! ensure_not_null((void *) box) || ! ensure_not_null((void *) pose) ||
      ! ensure_has_X(T_STBOX, box->flags) || 
      ! ensure_same_srid(box->srid, pose_srid(pose)))
    return false;
  return true;
}

/*****************************************************************************
 * Interpolation functions defining functionality required by tsequence.c
 * that must be implemented by each temporal type
 *****************************************************************************/

/**
 * @brief Return true if a segment of a temporal pose value intersects
 * a base value at the timestamp
 * @param[in] inst1,inst2 Temporal instants defining the segment
 * @param[in] value Base value
 * @param[out] t Timestamp, may be @p NULL
 */
bool
tposesegm_intersection_value(const TInstant *inst1, const TInstant *inst2,
  Datum value, TimestampTz *t)
{
  assert(inst1); assert(inst2);
  /* We are sure that the trajectory is a line */
  Datum start = tinstant_value(inst1);
  Datum end = tinstant_value(inst2);
  Datum geom_start = datum_pose_point(start);
  Datum geom_end = datum_pose_point(end);
  Datum geom = datum_pose_point(value);
  double dist;
  /* Compute the value taking into account position only */
  double fraction = (double) pointsegm_locate_point(geom_start, geom_end, geom,
    &dist);
  pfree(DatumGetPointer(geom_start));
  pfree(DatumGetPointer(geom_end));
  pfree(DatumGetPointer(geom));
  if (fabs(dist) >= MEOS_EPSILON)
    return false;
  /* Compare value with interpolated pose to take into account orientation as
   * well */
  Pose *pose1 = DatumGetPoseP(start);
  Pose *pose2 = DatumGetPoseP(end);
  Pose *pose = DatumGetPoseP(value);
  Pose *pose_interp = pose_interpolate(pose1, pose2, fraction);
  bool same = pose_same(pose, pose_interp);
  pfree(pose_interp);
  if (! same)
    return false;
  if (t)
  {
    double duration = (double) (inst2->t - inst1->t);
    /* Note that due to roundoff errors it may be the case that the
     * resulting timestamp t may be equal to inst1->t or to inst2->t */
    *t = inst1->t + (TimestampTz) (duration * fraction);
  }
  return true;
}

/*****************************************************************************
 * Trajectory function
 *****************************************************************************/

/**
 * @ingroup meos_pose_accessor
 * @brief Return the trajectory of a temporal pose
 * @param[in] temp Temporal pose
 * @csqlfn #Tpose_trajectory()
 */
GSERIALIZED *
tpose_trajectory(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
#if MEOS
  if (! ensure_not_null((void *) temp) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
#else
  assert(temp); assert(temp->temptype == T_TPOSE);
#endif /* MEOS */
  Temporal *tpoint = tpose_tpoint(temp);
  GSERIALIZED *result = tpoint_trajectory(tpoint);
  pfree(tpoint);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Return a temporal pose restricted to (the complement of) a geometry
 */
Temporal *
tpose_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_same_srid(tspatial_srid(temp), gserialized_get_srid(gs)) ||
      ! ensure_has_not_Z_geo(gs))
    return NULL;

  /* Empty geometry */
  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *tgeom = tpose_tpoint(temp);
  Temporal *tgeomres = tgeo_restrict_geom(tgeom, gs, zspan, atfunc);
  Temporal *result = NULL;
  if (tgeomres)
  {
    /* We do not call the function tgeompoint_tpose to avoid
     * roundoff errors */
    SpanSet *ss = temporal_time(tgeomres);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(tgeomres);
    pfree(ss);
  }
  pfree(tgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_pose_restrict
 * @brief Return a temporal pose restricted to a geometry
 * @param[in] temp Temporal pose
 * @param[in] gs Geometry
 * @param[in] zspan Span of values to restrict the Z dimension
 * @csqlfn #Tpose_at_geom()
 */
Temporal *
tpose_at_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_null((void *) zspan) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
  return tpose_restrict_geom(temp, gs, zspan, REST_AT);
}

/**
 * @ingroup meos_pose_restrict
 * @brief Return a temporal point restricted to (the complement of) a geometry
 * @param[in] temp Temporal pose
 * @param[in] gs Geometry
 * @param[in] zspan Span of values to restrict the Z dimension
 * @csqlfn #Tpose_minus_geom()
 */
Temporal *
tpose_minus_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      ! ensure_not_null((void *) zspan) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
  return tpose_restrict_geom(temp, gs, zspan, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_temporal_restrict
 * @brief Return a temporal pose restricted to (the complement of) a
 * spatiotemporal box
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is at, false for minus
 */
Temporal *
tpose_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  assert(temp); assert(box);
  Temporal *tgeom = tpose_tpoint(temp);
  Temporal *tgeomres = tgeo_restrict_stbox(tgeom, box, border_inc, atfunc);
  Temporal *result = NULL;
  if (tgeomres)
  {
    /* We do not call the function tgeompoint_tpose to avoid
     * roundoff errors */
    SpanSet *ss = temporal_time(tgeomres);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(tgeomres);
    pfree(ss);
  }
  pfree(tgeom);
  return result;
}

#if MEOS
/**
 * @ingroup meos_pose_restrict
 * @brief Return a temporal pose restricted to a geometry
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @sqlfn #Tpose_at_stbox()
 */
Temporal *
tpose_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
  return tpose_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_pose_restrict
 * @brief Return a temporal point restricted to (the complement of) a geometry
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @sqlfn #Tpose_minus_stbox()
 */
Temporal *
tpose_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) box) ||
      ! ensure_temporal_isof_type(temp, T_TPOSE))
    return NULL;
  return tpose_restrict_stbox(temp, box, border_inc, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/
