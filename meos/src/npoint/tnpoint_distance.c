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
 * @brief Temporal distance for temporal network points
 */

#include "npoint/tnpoint_distance.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "point/pgis_types.h"
#include "point/tpoint_spatialfuncs.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Distance function
 *****************************************************************************/

/**
 * @brief Return the distance between two network points
 */
Datum
npoint_distance(Datum np1, Datum np2)
{
  Datum geom1 = PointerGetDatum(npoint_geom(DatumGetNpointP(np1)));
  Datum geom2 = PointerGetDatum(npoint_geom(DatumGetNpointP(np2)));
  return pt_distance2d(geom1, geom2);
}

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return the temporal distance between a geometry point and a temporal
 * network point
 */
Temporal *
distance_tnpoint_point(const Temporal *temp, const GSERIALIZED *gs)
{
  /* Ensure validity of the arguments */
  if (! ensure_not_null((void *) temp) || ! ensure_not_null((void *) gs) ||
      gserialized_is_empty(gs) || ! ensure_point_type(gs))
    return NULL;

  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_point((const Temporal *) tpoint, gs);
  pfree(tpoint);
  return result;
}

/**
 * @brief Return the temporal distance between a temporal network point and
 * a network point
 */
Temporal *
distance_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_point(tpoint, geom);
  pfree(geom);
  return result;
}

/**
 * @brief Return the temporal distance between two temporal network points
 */
Temporal *
distance_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  Temporal *result = distance_tpoint_tpoint(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @brief Return the nearest approach instant of the temporal network point
 * and a geometry
 */
TInstant *
nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return NULL;
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tpoint, gs);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom);
  return result;
}

/**
 * @brief Return the nearest approach instant of the network point and a
 * temporal network point
 */
TInstant *
nai_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tpoint = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tpoint, geom);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  temporal_value_at_timestamptz(temp, resultgeom->t, false, &value);
  TInstant *result = tinstant_make_free(value, temp->temptype, resultgeom->t);
  pfree(tpoint); pfree(resultgeom); pfree(geom);
  return result;
}

/**
 * @brief Return the nearest approach instant of two temporal network points
 */
TInstant *
nai_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
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
 * @brief Return the nearest approach distance of two temporal network point
 * and a geometry
 */
double
nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return -1;
  GSERIALIZED *traj = tnpoint_geom(temp);
  double result = DatumGetFloat8(geom_distance2d(PointerGetDatum(traj),
    PointerGetDatum(gs)));
  pfree(traj);
  return result;
}

/**
 * @brief Return the nearest approach distance of a temporal network point
 * and a network point
 */
double
nad_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  GSERIALIZED *traj = tnpoint_geom(temp);
  double result = DatumGetFloat8(geom_distance2d(PointerGetDatum(traj),
    PointerGetDatum(geom)));
  pfree(traj); pfree(geom);
  return result;
}

/**
 * @brief Return the nearest approach distance of two temporal network points
 */
double
nad_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *dist = distance_tnpoint_tnpoint(temp1, temp2);
  if (dist == NULL)
    return -1;
  return DatumGetFloat8(temporal_min_value(dist));
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @brief Return the line connecting the nearest approach point between a
 * geometry and a temporal network point
 */
GSERIALIZED *
shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *gs)
{
  if (gserialized_is_empty(gs))
    return NULL;
  GSERIALIZED *traj = tnpoint_geom(temp);
  GSERIALIZED *result = geo_shortestline2d(traj, gs);
  pfree(traj);
  return result;
}

/**
 * @brief Return the line connecting the nearest approach point between a
 * network point and a temporal network point
 */
GSERIALIZED *
shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  GSERIALIZED *traj = tnpoint_geom(temp);
  GSERIALIZED *result = geo_shortestline2d(traj, geom);
  pfree(traj); pfree(geom);
  return result;
}

/**
 * @brief Return the line connecting the nearest approach point between two
 * temporal networks
 */
GSERIALIZED *
shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  GSERIALIZED *result = shortestline_tpoint_tpoint(tpoint1, tpoint2);
  pfree(tpoint1); pfree(tpoint2);
  return result;
}

/*****************************************************************************/
