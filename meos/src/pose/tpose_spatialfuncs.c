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

/* C */
#include <stdio.h>
/* PostgreSQL */
#include <postgres.h>
#include <utils/timestamp.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_pose.h>
#include "pose/pose.h"
#include "pose/tpose.h"
#include "geo/tgeo_spatialfuncs.h"
#include "rgeo/trgeo_utils.h"

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
  VALIDATE_TPOSE(temp, NULL);
  Temporal *tpoint = tpose_to_tpoint(temp);
  GSERIALIZED *result = tpoint_trajectory(tpoint);
  pfree(tpoint);
  return result;
}

/*****************************************************************************
 * Restriction functions
 *****************************************************************************/

/**
 * @ingroup meos_internal_pose_restrict
 * @brief Return a temporal pose restricted to (the complement of) a geometry
 * @note `zspan` may be `NULL`
 */
Temporal *
tpose_restrict_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_geo(temp, gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  /* Empty geometry */
  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *tpoint = tpose_to_tpoint(temp);
  Temporal *res = tgeo_restrict_geom(tpoint, gs, zspan, atfunc);
  Temporal *result = NULL;
  if (res)
  {
    /* We do not call the function tgeompoint_tpose to avoid roundoff errors */
    SpanSet *ss = temporal_time(res);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(res); pfree(ss);
  }
  pfree(tpoint);
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
inline Temporal *
tpose_at_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan)
{
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
inline Temporal *
tpose_minus_geom(const Temporal *temp, const GSERIALIZED *gs,
  const Span *zspan)
{
  return tpose_restrict_geom(temp, gs, zspan, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/

/**
 * @ingroup meos_internal_pose_restrict
 * @brief Return a temporal pose restricted to (the complement of) a
 * spatiotemporal box
 * @param[in] temp Temporal pose
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
tpose_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tpose_stbox(temp, box))
    return NULL;

  Temporal *tgeom = tpose_to_tpoint(temp);
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
inline Temporal *
tpose_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
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
inline Temporal *
tpose_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return tpose_restrict_stbox(temp, box, border_inc, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************/
