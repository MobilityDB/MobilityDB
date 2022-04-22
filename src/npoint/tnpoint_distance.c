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
 * @file tnpoint_distance.c
 * @brief Temporal distance for temporal network points.
 */

#include "npoint/tnpoint_distance.h"

/* PostgreSQL */
#include <assert.h>
/* MobilityDB */
#include "general/temporaltypes.h"
#include "general/temporal_util.h"
#include "general/lifting.h"
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
 * @ingroup libmeos_temporal_dist
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
 * @ingroup libmeos_temporal_dist
 * @brief Return the temporal distance between the temporal network point and
 * the network point
 */
Temporal *
distance_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  GSERIALIZED *geom = (GSERIALIZED *) DatumGetPointer(npoint_geom(np));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo(tempgeom, geom);
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
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
 * @ingroup libmeos_temporal_dist
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
  bool found = temporal_value_at_timestamp_inc(temp, resultgeom->t, &value);
  assert(found);
  TInstant *result = tinstant_make(value, resultgeom->t, temp->temptype);
  pfree(tempgeom); pfree(resultgeom); pfree(DatumGetPointer(value));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach instant of the network point and the
 * temporal network point.
 */
TInstant *
nai_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = nai_tpoint_geo(tempgeom, gs);
  /* We do not call the function tgeompointinst_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  bool found = temporal_value_at_timestamp_inc(temp, resultgeom->t, &value);
  assert(found);
  TInstant *result = tinstant_make(value, resultgeom->t, temp->temptype);
  pfree(tempgeom); pfree(resultgeom); pfree(DatumGetPointer(value));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach instant of the network point and the
 * temporal network point.
 */
TInstant *
nai_npoint_tnpoint(const Npoint *np, const Temporal *temp)
{
  return nai_tnpoint_npoint(temp, np);
}

/**
 * @ingroup libmeos_temporal_dist
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
    bool found = temporal_value_at_timestamp_inc(temp1, min->t, &value);
    assert(found);
    result = tinstant_make(value, min->t, temp1->temptype);
    pfree(dist); pfree(DatumGetPointer(value));
  }
  return result;
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance of the temporal network point and the
 * geometry
 */
double
nad_tnpoint_geo(Temporal *temp, GSERIALIZED *geo)
{
  if (gserialized_is_empty(geo))
    return -1;
  Datum traj = tnpoint_geom(temp);
  double result = DatumGetFloat8(geom_distance2d(traj, PointerGetDatum(geo)));
  pfree(DatumGetPointer(traj));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance of the temporal network point
 * and the network point
 */
double
nad_tnpoint_npoint(Temporal *temp, Npoint *np)
{
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  double result = DatumGetFloat8(geom_distance2d(traj, PointerGetDatum(gs)));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the nearest approach distance of the two temporal network
 * points
 */
double
nad_tnpoint_tnpoint(Temporal *temp1, Temporal *temp2)
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
 * @ingroup libmeos_temporal_dist
 * @brief Return the line connecting the nearest approach point between the
 * geometry and the temporal network point
 */
bool
shortestline_tnpoint_geo(const Temporal *temp, const GSERIALIZED *geo,
  Datum *result)
{
  if (gserialized_is_empty(geo))
    return false;
  Datum traj = tnpoint_geom(temp);
  *result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(geo));
  pfree(DatumGetPointer(traj));
  return true;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the line connecting the nearest approach point between the
 * network point and the temporal network point
 */
Datum
shortestline_tnpoint_npoint(const Temporal *temp, const Npoint *np)
{
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj,
    PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

/**
 * @ingroup libmeos_temporal_dist
 * @brief Return the line connecting the nearest approach point between the two
 * temporal networks
 */
bool
shortestline_tnpoint_tnpoint(const Temporal *temp1, const Temporal *temp2,
  Datum *result)
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
/*****************************************************************************/
/*                        MobilityDB - PostgreSQL                            */
/*****************************************************************************/
/*****************************************************************************/

#ifndef MEOS

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Distance_geo_tnpoint);
/**
 * Return the temporal distance between the geometry point and the temporal
 * network point
 */
PGDLLEXPORT Datum
Distance_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Distance_tnpoint_geo);
/**
 * Return the temporal distance between the temporal network point and the
 * geometry point
 */
PGDLLEXPORT Datum
Distance_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = distance_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Distance_npoint_tnpoint);
/**
 * Return the temporal distance between the network point and the temporal
 * network point
 */
PGDLLEXPORT Datum
Distance_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Distance_tnpoint_npoint);
/**
 * Return the temporal distance between the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
Distance_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  Temporal *result = distance_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Distance_tnpoint_tnpoint);
/**
 * Return the temporal distance between the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
Distance_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAI_geo_tnpoint);
/**
 * Return the nearest approach instant of the geometry and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAI_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tnpoint_geo);
/**
 * Return the nearest approach instant of the temporal network point and the
 * geometry
 */
PGDLLEXPORT Datum
NAI_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_npoint_tnpoint);
/**
 * Return the nearest approach instant of the network point and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAI_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_npoint_tnpoint(np, temp);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tnpoint_npoint);
/**
 * Return the nearest approach instant of the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
NAI_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  /* Store fcinfo into a global variable */
  store_fcinfo(fcinfo);
  TInstant *result = nai_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tnpoint_tnpoint);
/**
 * Return the nearest approach instant of the two temporal network points
 */
PGDLLEXPORT Datum
NAI_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = nai_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_geo_tnpoint);
/**
 * Return the nearest approach distance of the geometry and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAD_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_geo);
/**
 * Return the nearest approach distance of the temporal network point and the
 * geometry
 */
PGDLLEXPORT Datum
NAD_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double result = nad_tnpoint_geo(temp, geo);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_npoint_tnpoint);
/**
 * Return the nearest approach distance of the network point and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAD_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_npoint);
/**
 * Return the nearest approach distance of the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
NAD_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  double result = nad_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_FLOAT8(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_tnpoint);
/**
 * Return the nearest approach distance of the two temporal network points
 */
PGDLLEXPORT Datum
NAD_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double result = nad_tnpoint_tnpoint(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Shortestline_geo_tnpoint);
/**
 * Return the line connecting the nearest approach point between the geometry
 * and the temporal network point
 */
PGDLLEXPORT Datum
Shortestline_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result;
  bool found = shortestline_tnpoint_geo(temp, geo, &result);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Shortestline_tnpoint_geo);
/**
 * Return the line connecting the nearest approach point between the temporal
 * network point and the geometry
 */
PGDLLEXPORT Datum
Shortestline_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  Datum result;
  bool found = shortestline_tnpoint_geo(temp, geo, &result);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(Shortestline_npoint_tnpoint);
/**
 * Return the line connecting the nearest approach point between the network
 * point and the temporal network point
 */
PGDLLEXPORT Datum
Shortestline_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = shortestline_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Shortestline_tnpoint_npoint);
/**
 * Return the line connecting the nearest approach point between the temporal
 * network point and the network point
 */
PGDLLEXPORT Datum
Shortestline_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  Datum result = shortestline_tnpoint_npoint(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(Shortestline_tnpoint_tnpoint);
/**
 * Return the line connecting the nearest approach point between the two
 * temporal networks
 */
PGDLLEXPORT Datum
Shortestline_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Datum result;
  bool found = shortestline_tnpoint_tnpoint(temp1, temp2, &result);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

#endif /* #ifndef MEOS */

/*****************************************************************************/
