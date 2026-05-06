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
 * (`trgeo_to_tpoint`): a `trgeometry` is "in" a geometry / STBox at time
 * `t` iff its antenna position at `t` lies in that geometry / STBox.
 * This matches the tpose convention and uses the existing tgeompoint
 * restriction kernels.
 */

/* C */
#include <math.h>
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
#include "temporal/lifting.h"
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

  Temporal *tpoint = trgeo_to_tpoint(temp);
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

  Temporal *tpoint = trgeo_to_tpoint(temp);
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

/*****************************************************************************/

/**
 * @brief Apply a 2D pose transformation to a local reference point
 * @details Computes world = pose.xy + R(theta) * local, where R is the 2D
 * rotation matrix built from the pose angle.  The result inherits the SRID
 * from the pose.
 */
static Datum
datum_pose_apply_point(Datum pose_d, Datum local_pt_d)
{
  const Pose *pose = DatumGetPoseP(pose_d);
  const POINT2D *lp = DATUM_POINT2D_P(local_pt_d);
  double cos_t = cos(pose->data[2]);
  double sin_t = sin(pose->data[2]);
  double wx = pose->data[0] + lp->x * cos_t - lp->y * sin_t;
  double wy = pose->data[1] + lp->x * sin_t + lp->y * cos_t;
  LWPOINT *pt = lwpoint_make2d(pose_srid(pose), wx, wy);
  GSERIALIZED *result = geo_serialize((LWGEOM *) pt);
  lwpoint_free(pt);
  return GserializedPGetDatum(result);
}

/**
 * @ingroup meos_rgeo_spatialfuncs
 * @brief Return the convex hull of a temporal rigid geometry
 * @param[in] temp Temporal rigid geometry
 * @return On error return `NULL`
 * @csqlfn #Trgeo_convex_hull()
 */
GSERIALIZED *
trgeo_convex_hull(const Temporal *temp)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  GSERIALIZED *trav = trgeo_traversed_area(temp, UNARY_UNION_NO);
  GSERIALIZED *result = geom_convex_hull(trav);
  pfree(trav);
  return result;
}

/**
 * @ingroup meos_rgeo_spatialfuncs
 * @brief Return the centroid trajectory of a temporal rigid geometry as a
 * temporal point
 * @details The centroid is computed once in local coordinates from the
 * reference geometry, then the 2D rotation+translation of the pose at each
 * instant maps it to world coordinates.  Between instants the result is
 * linearly interpolated (same approximation as `trgeo_to_tpoint`).
 * @param[in] temp Temporal rigid geometry
 * @return On error return `NULL`
 * @csqlfn #Trgeo_centroid()
 */
Temporal *
trgeo_centroid(const Temporal *temp)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  const GSERIALIZED *ref_geom = trgeo_geom_p(temp);
  GSERIALIZED *local_centroid = geom_centroid(ref_geom);
  if (! local_centroid)
    return NULL;
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) &datum_pose_apply_point;
  lfinfo.numparam = 1;
  lfinfo.param[0] = GserializedPGetDatum(local_centroid);
  lfinfo.argtype[0] = temptype_basetype(temp->temptype);
  lfinfo.restype = T_TGEOMPOINT;
  Temporal *result = tfunc_temporal(temp, &lfinfo);
  pfree(local_centroid);
  return result;
}

/*****************************************************************************/

/**
 * @ingroup meos_rgeo_spatialfuncs
 * @brief Return a temporal rigid geometry expanded in space by a distance
 * @param[in] temp Temporal rigid geometry
 * @param[in] d Distance
 * @return On error return `NULL`
 * @csqlfn #Trgeo_expand_space()
 */
STBox *
trgeo_expand_space(const Temporal *temp, double d)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  STBox box;
  tspatial_set_stbox(temp, &box);
  return stbox_expand_space(&box, d);
}

/**
 * @brief Simplify a temporal rigid geometry by restricting it to the
 * timestamps produced by running a simplification algorithm on its centroid
 * trajectory
 */
static Temporal *
trgeo_simplify(const Temporal *temp, const Temporal *simplified)
{
  if (! simplified)
    return NULL;
  int count;
  TimestampTz *times = temporal_timestamps(simplified, &count);
  if (! times)
    return NULL;
  Datum *datums = palloc(sizeof(Datum) * count);
  for (int i = 0; i < count; i++)
    datums[i] = TimestampTzGetDatum(times[i]);
  pfree(times);
  /* set_make_free takes ownership of datums */
  Set *tset = set_make_free(datums, count, T_TIMESTAMPTZ, ORDER);
  Temporal *result = trgeo_restrict_tstzset(temp, tset, REST_AT);
  pfree(tset);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using a minimum time delta
 * @param[in] temp Temporal rigid geometry
 * @param[in] mint Minimum time delta
 * @return On error return `NULL`
 * @csqlfn #Trgeo_simplify_min_tdelta()
 */
Temporal *
trgeo_simplify_min_tdelta(const Temporal *temp, const Interval *mint)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  if (! ensure_not_null((void *) mint))
    return NULL;
  Temporal *tpoint = trgeo_to_tpoint(temp);
  Temporal *simplified = temporal_simplify_min_tdelta(tpoint, mint);
  pfree(tpoint);
  Temporal *result = trgeo_simplify(temp, simplified);
  if (simplified) pfree(simplified);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using a minimum distance
 * @param[in] temp Temporal rigid geometry
 * @param[in] dist Minimum distance
 * @return On error return `NULL`
 * @csqlfn #Trgeo_simplify_min_dist()
 */
Temporal *
trgeo_simplify_min_dist(const Temporal *temp, double dist)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  Temporal *tpoint = trgeo_to_tpoint(temp);
  Temporal *simplified = temporal_simplify_min_dist(tpoint, dist);
  pfree(tpoint);
  Temporal *result = trgeo_simplify(temp, simplified);
  if (simplified) pfree(simplified);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using a maximum distance
 * @param[in] temp Temporal rigid geometry
 * @param[in] dist Maximum distance
 * @param[in] synchronized Use synchronized distance
 * @return On error return `NULL`
 * @csqlfn #Trgeo_simplify_max_dist()
 */
Temporal *
trgeo_simplify_max_dist(const Temporal *temp, double dist, bool synchronized)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  Temporal *tpoint = trgeo_to_tpoint(temp);
  Temporal *simplified = temporal_simplify_max_dist(tpoint, dist, synchronized);
  pfree(tpoint);
  Temporal *result = trgeo_simplify(temp, simplified);
  if (simplified) pfree(simplified);
  return result;
}

/**
 * @ingroup meos_rgeo_analytics
 * @brief Simplify a temporal rigid geometry using the Douglas-Peucker
 * algorithm on the centroid trajectory
 * @param[in] temp Temporal rigid geometry
 * @param[in] eps_dist Epsilon distance
 * @param[in] synchronized Use synchronized distance
 * @return On error return `NULL`
 * @csqlfn #Trgeo_simplify_dp()
 */
Temporal *
trgeo_simplify_dp(const Temporal *temp, double eps_dist, bool synchronized)
{
  VALIDATE_TRGEOMETRY(temp, NULL);
  Temporal *tpoint = trgeo_to_tpoint(temp);
  Temporal *simplified = temporal_simplify_dp(tpoint, eps_dist, synchronized);
  pfree(tpoint);
  Temporal *result = trgeo_simplify(temp, simplified);
  if (simplified) pfree(simplified);
  return result;
}

/*****************************************************************************/
