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
 * @brief Temporal distance for temporal network points
 */

#include "npoint/tnpoint_distance.h"

/* MEOS */
#include <meos.h>
#include <meos_npoint.h>
#include <meos_internal.h>
#include "geo/postgis_funcs.h"
#include "geo/tgeo_spatialfuncs.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Distance function
 *****************************************************************************/

/**
 * @ingroup meos_internal_npoint_dist
 * @brief Return the distance between two network points
 */
Datum
datum_npoint_distance(Datum np1, Datum np2)
{
  Datum geom1 = PointerGetDatum(npoint_geom(DatumGetNpointP(np1)));
  Datum geom2 = PointerGetDatum(npoint_geom(DatumGetNpointP(np2)));
  return datum_pt_distance2d(geom1, geom2);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @ingroup meos_npoint_dist
 * @brief Return the temporal distance between a geometry point and a temporal
 * network point
 * @csqlfn #Distance_tnpoint_point()
 */
Temporal *
distance_tnpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_geo(temp, gs) || gserialized_is_empty(gs) || 
      ! ensure_point_type(gs))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tgeo_geo((const Temporal *) tpoint, gs);
  pfree(tpoint);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the temporal distance between a temporal network point and
 * a network point
 * @param[in] temp Temporal point
 * @param[in] np Network point
 * @csqlfn #Distance_tnpoint_npoint()
 */
Temporal *
distance_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return NULL;

  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tgeo_geo(tpoint, geom);
  pfree(geom);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the temporal distance between two temporal network points
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Distance_tnpoint_tnpoint()
 */
Temporal *
distance_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return NULL;

  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  Temporal *result = distance_tgeo_tgeo(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @ingroup meos_npoint_dist
 * @brief Return the nearest approach instant of the temporal network point
 * and a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #NAI_tnpoint_geo()
 */
TInstant *
nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tgeo_geo(tpoint, gs);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the nearest approach instant of the network point and a
 * temporal network point
 * @param[in] temp Temporal point
 * @param[in] np Network point
 * @csqlfn #NAI_tnpoint_npoint()
 */
TInstant *
nai_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return NULL;

  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tgeo_geo(tpoint, geom);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom); pfree(geom);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the nearest approach instant of two temporal network points
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #NAI_tnpoint_tnpoint()
 */
TInstant *
nai_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return NULL;

  Temporal *dist = distance_tnpoint_tnpoint(temp1, temp2);
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
 * @ingroup meos_npoint_dist
 * @brief Return the nearest approach distance of two temporal network point
 * and a geometry
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #NAD_tnpoint_geo()
 */
double
nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return -1.0;

  GSERIALIZED *traj = tnpoint_trajectory(temp);
  double result = geom_distance2d(traj, gs);
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the nearest approach distance of a temporal network point
 * and a network point
 * @param[in] temp Temporal point
 * @param[in] np Network point
 * @csqlfn #NAD_tnpoint_npoint()
 */
double
nad_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return -1.0;

  GSERIALIZED *geom = npoint_geom(np);
  GSERIALIZED *traj = tnpoint_trajectory(temp);
  double result = geom_distance2d(traj, geom);
  pfree(traj); pfree(geom);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the nearest approach distance of two temporal network points
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #NAD_tnpoint_tnpoint()
 */
double
nad_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return -1.0;

  Temporal *dist = distance_tnpoint_tnpoint(temp1, temp2);
  if (dist == NULL)
    return -1.0;
  return DatumGetFloat8(temporal_min_value(dist));
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @ingroup meos_npoint_dist
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal network point
 * @param[in] temp Temporal point
 * @param[in] gs Geometry
 * @csqlfn #Shortestline_tnpoint_geo()
 */
GSERIALIZED *
shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_geo(temp, gs) || gserialized_is_empty(gs))
    return NULL;

  GSERIALIZED *traj = tnpoint_trajectory(temp);
  GSERIALIZED *result = geom_shortestline2d(traj, gs);
  pfree(traj);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the line connecting the nearest approach point between a
 * network point and a temporal network point
 * @param[in] temp Temporal point
 * @param[in] np Network point
 * @csqlfn #Shortestline_tnpoint_npoint()
 */
GSERIALIZED *
shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_npoint(temp, np))
    return NULL;

  GSERIALIZED *geom = npoint_geom(np);
  GSERIALIZED *traj = tnpoint_trajectory(temp);
  GSERIALIZED *result = geom_shortestline2d(traj, geom);
  pfree(geom); pfree(traj);
  return result;
}

/**
 * @ingroup meos_npoint_dist
 * @brief Return the line connecting the nearest approach point between two
 * temporal networks
 * @param[in] temp1,temp2 Temporal points
 * @csqlfn #Shortestline_tnpoint_tnpoint()
 */
GSERIALIZED *
shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  /* Ensure the validity of the arguments */
  if (! ensure_valid_tnpoint_tnpoint(temp1, temp2))
    return NULL;

  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  GSERIALIZED *result = shortestline_tgeo_tgeo(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************/
