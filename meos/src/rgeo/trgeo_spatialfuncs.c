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
 * @brief Spatial restriction functions for temporal rigid geometries
 *
 * The restriction is evaluated on the temporal centroid trajectory
 * (`trgeometry_to_tpoint`): a `trgeometry` is "in" a geometry / STBox at time
 * `t` iff its antenna position at `t` lies in that geometry / STBox.
 * This matches the tpose convention and uses the existing tgeompoint
 * restriction kernels.
 */

/* PostgreSQL */
#include <postgres.h>
/* MEOS */
#include <meos.h>
#include <meos_geo.h>
#include <meos_internal.h>
#include <meos_internal_geo.h>
#include <meos_rgeo.h>
#include "geo/stbox.h"
#include "geo/tgeo_spatialfuncs.h"
#include "rgeo/trgeo.h"
#include "rgeo/trgeo_spatialfuncs.h"
#include "rgeo/trgeo_utils.h"
#include "pose/pose.h"
#include "temporal/lifting.h"
#include "temporal/meos_catalog.h"
#include "temporal/temporal.h"

/*****************************************************************************
 * Restriction by geometry
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to (the complement
 * of) a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
trgeo_restrict_geom(const Temporal *temp, const GSERIALIZED *gs, bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_geo(temp, gs) || ! ensure_has_not_Z_geo(gs))
    return NULL;

  if (gserialized_is_empty(gs))
    return atfunc ? NULL : temporal_copy(temp);

  Temporal *tpoint = trgeometry_to_tpoint(temp);
  Temporal *res = tgeo_restrict_geom(tpoint, gs, atfunc);
  Temporal *result = NULL;
  if (res)
  {
    SpanSet *ss = temporal_time(res);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(res);
    pfree(ss);
  }
  pfree(tpoint);
  return result;
}

#if MEOS
/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Trgeo_at_geom()
 */
inline Temporal *
trgeo_at_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return trgeo_restrict_geom(temp, gs, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Geometry
 * @csqlfn #Trgeo_minus_geom()
 */
inline Temporal *
trgeo_minus_geom(const Temporal *temp, const GSERIALIZED *gs)
{
  return trgeo_restrict_geom(temp, gs, REST_MINUS);
}
#endif /* MEOS */

/*****************************************************************************
 * Restriction by spatiotemporal box
 *****************************************************************************/

/**
 * @ingroup meos_internal_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to (the complement
 * of) a spatiotemporal box
 * @param[in] temp Temporal rigid geometry
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @param[in] atfunc True if the restriction is `at`, false for `minus`
 */
Temporal *
trgeo_restrict_stbox(const Temporal *temp, const STBox *box, bool border_inc,
  bool atfunc)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_trgeo_stbox(temp, box))
    return NULL;

  Temporal *tpoint = trgeometry_to_tpoint(temp);
  Temporal *res = tgeo_restrict_stbox(tpoint, box, border_inc, atfunc);
  Temporal *result = NULL;
  if (res)
  {
    SpanSet *ss = temporal_time(res);
    result = temporal_restrict_tstzspanset(temp, ss, REST_AT);
    pfree(res);
    pfree(ss);
  }
  pfree(tpoint);
  return result;
}

#if MEOS
/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to a spatiotemporal box
 * @param[in] temp Temporal rigid geometry
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Trgeo_at_stbox()
 */
inline Temporal *
trgeo_at_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return trgeo_restrict_stbox(temp, box, border_inc, REST_AT);
}

/**
 * @ingroup meos_rgeo_restrict
 * @brief Return a temporal rigid geometry restricted to the complement of a
 * spatiotemporal box
 * @param[in] temp Temporal rigid geometry
 * @param[in] box Spatiotemporal box
 * @param[in] border_inc True when the box contains the upper border
 * @csqlfn #Trgeo_minus_stbox()
 */
inline Temporal *
trgeo_minus_stbox(const Temporal *temp, const STBox *box, bool border_inc)
{
  return trgeo_restrict_stbox(temp, box, border_inc, REST_MINUS);
}
#endif /* MEOS */



/**
 * @brief Apply a pose to a body-frame geometry and return the centroid of the
 * world-frame result as a Datum
 */
static Datum
datum_pose_geom_centroid(Datum pose_datum, Datum geom_datum)
{
  GSERIALIZED *world_geom = geom_apply_pose(DatumGetGserializedP(geom_datum),
    DatumGetPoseP(pose_datum));
  GSERIALIZED *centroid = geom_centroid(world_geom);
  pfree(world_geom);
  return GserializedPGetDatum(centroid);
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the centroid of a temporal rigid geometry as a temporal point
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_centroid()
 */
Temporal *
trgeometry_centroid(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_geom_centroid;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = T_GEOMETRY;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.invert = INVERT_NO;
  return tfunc_temporal_base(temp, PointerGetDatum(trgeo_geom_p(temp)), &lfinfo);
}

/**
 * @ingroup meos_rgeo_accessor
 * @brief Return the convex hull of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @csqlfn #Trgeometry_convex_hull()
 */
GSERIALIZED *
trgeometry_convex_hull(const Temporal *temp)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);

  GSERIALIZED *trav = trgeometry_traversed_area(temp, UNARY_UNION_NO);
  if (! trav)
    return NULL;
  GSERIALIZED *result = geom_convex_hull(trav);
  pfree(trav);
  return result;
}

/*****************************************************************************
 * Body-frame trajectory functions
 *****************************************************************************/

/**
 * @brief Apply a pose to a body-frame geometry and return the world-frame
 * result as a Datum
 */
static Datum
datum_pose_apply_to_geom(Datum pose_datum, Datum geom_datum)
{
  const Pose *pose = DatumGetPoseP(pose_datum);
  GSERIALIZED *gs = DatumGetGserializedP(geom_datum);
  return GserializedPGetDatum(geom_apply_pose(gs, pose));
}

/**
 * @ingroup meos_rgeo_spatialfuncs
 * @brief Return the world-frame trajectory of a body-frame point on a moving
 * rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @param[in] gs Body-frame geometry
 * @csqlfn #Trgeometry_body_point_trajectory()
 */
Temporal *
trgeometry_body_point_trajectory(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  VALIDATE_TRGEOMETRY(temp, NULL);
  VALIDATE_NOT_NULL(gs, NULL);

  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_apply_to_geom;
  lfinfo.numparam = 0;
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.argtype[1] = T_GEOMETRY;
  lfinfo.restype = T_TGEOMPOINT;
  lfinfo.invert = INVERT_NO;
  return tfunc_temporal_base(temp, PointerGetDatum(gs), &lfinfo);
}

/*****************************************************************************/
