/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * @brief Temporal distance for temporal network points.
 */

#include "npoint/tnpoint_distance.h"

/* C */
#include <assert.h>
/* MobilityDB */
#include <meos.h>
#include <meos_internal.h>
#include "general/lifting.h"
#include "general/temporal_util.h"
#include "point/pgis_call.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_distance.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"
#include "npoint/tnpoint_tempspatialrels.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

/**
 * @brief Return the temporal distance between the geometry point and the temporal
 * network point
 */
Temporal *
distance_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  if (gserialized_is_empty(geo))
    return NULL;
  ensure_point_type(geo);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo((const Temporal *) tempgeom, geo);
  pfree(tempgeom);
  return result;
}

/**
 * @brief Return the temporal distance between the temporal network point and
 * the network point
 */
Temporal *
distance_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo(tempgeom, geom);
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @brief Return the temporal distance between the two temporal network points
 */
Temporal *
distance_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  Temporal *geomsync1 = tnpoint_tgeompoint(sync1);
  Temporal *geomsync2 = tnpoint_tgeompoint(sync2);
  Temporal *result = distance_tpoint_tpoint(geomsync1, geomsync2);
  pfree(sync1); pfree(sync2);
  pfree(geomsync1); pfree(geomsync2);
  return result;
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * @brief Return the nearest approach instant of the temporal network point
 * and the geometry
 */
TInstant *
nai_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  if (gserialized_is_empty(geo))
    return NULL;
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tempgeom, geo);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  bool found = temporal_value_at_timestamp(temp, resultgeom->t, false, &value);
  assert(found);
  TInstant *result = tinstant_make(value, temp->temptype, resultgeom->t);
  pfree(tempgeom); pfree(resultgeom); pfree(DatumGetPointer(value));
  return result;
}

/**
 * @brief Return the nearest approach instant of the network point and the
 * temporal network point.
 */
TInstant *
nai_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tempgeom, geom);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  bool found = temporal_value_at_timestamp(temp, resultgeom->t, false, &value);
  assert(found);
  TInstant *result = tinstant_make(value, temp->temptype, resultgeom->t);
  pfree(tempgeom); pfree(resultgeom); pfree(DatumGetPointer(value));
  pfree(geom);
  return result;
}

/**
 * @brief Return the nearest approach instant of the two temporal network points
 */
TInstant *
nai_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *dist = distance_tnpoint_tnpoint(temp1, temp2);
  TInstant *result = NULL;
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_instant((const Temporal *) dist);
    /* The closest point may be at an exclusive bound. */
    Datum value;
    bool found = temporal_value_at_timestamp(temp1, min->t, false, &value);
    assert(found);
    result = tinstant_make(value, temp1->temptype, min->t);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @brief Return the nearest approach distance of the temporal network point
 * and the geometry
 */
double
nad_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo)
{
  if (gserialized_is_empty(geo))
    return -1;
  GSERIALIZED *traj = tnpoint_geom(temp);
  double result = DatumGetFloat8(geom_distance2d(PointerGetDatum(traj),
    PointerGetDatum(geo)));
  pfree(traj);
  return result;
}

/**
 * @brief Return the nearest approach distance of the temporal network point
 * and the network point
 */
double
nad_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  GSERIALIZED *traj = tnpoint_geom(temp);
  double result = DatumGetFloat8(geom_distance2d(PointerGetDatum(traj),
    PointerGetDatum(geom)));
  pfree(traj);
  pfree(geom);
  return result;
}

/**
 * @brief Return the nearest approach distance of the two temporal network
 * points
 */
double
nad_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *dist = distance_tnpoint_tnpoint(temp1, temp2);
  if (dist == NULL)
    return -1;
  double result = DatumGetFloat8(temporal_min_value(dist));
  return result;
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

/**
 * @brief Return the line connecting the nearest approach point between the
 * geometry and the temporal network point
 */
bool
shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo,
  GSERIALIZED **result)
{
  if (gserialized_is_empty(geo))
    return false;
  GSERIALIZED *traj = tnpoint_geom(temp);
  *result = gserialized_shortestline2d(traj, geo);
  pfree(traj);
  return true;
}

/**
 * @brief Return the line connecting the nearest approach point between the
 * network point and the temporal network point
 */
GSERIALIZED *
shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = npoint_geom(np);
  GSERIALIZED *traj = tnpoint_geom(temp);
  GSERIALIZED *result = gserialized_shortestline2d(traj, geom);
  pfree(traj);
  pfree(geom);
  return result;
}

/**
 * @brief Return the line connecting the nearest approach point between the two
 * temporal networks
 */
bool
shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  GSERIALIZED **result)
{
  Temporal *sync1, *sync2;
  /* Return false if the temporal points do not intersect in time */
  if (! intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
      &sync1, &sync2))
    return false;

  Temporal *geomsync1 = tnpoint_tgeompoint(sync1);
  Temporal *geomsync2 = tnpoint_tgeompoint(sync2);
  bool found = shortestline_tpoint_tpoint(geomsync1, geomsync2, result);
  pfree(geomsync1); pfree(geomsync2);
  return found;
}

/*****************************************************************************/
