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

PG_FUNCTION_INFO_V1(distance_geo_tnpoint);
/**
 * Returns the temporal distance between the geometry point and the temporal
 * network point
 */
PGDLLEXPORT Datum
distance_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  ensure_point_type(gs);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo_internal((const Temporal *) tempgeom,
    PointerGetDatum(gs));
  pfree(tempgeom);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_npoint_tnpoint);
/**
 * Returns the temporal distance between the network point and the temporal
 * network point
 */
PGDLLEXPORT Datum
distance_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum geom = npoint_geom(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo_internal((const Temporal *) tempgeom,
    geom);
  pfree(DatumGetPointer(geom));
  pfree(tempgeom);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_tnpoint_geo);
/**
 * Returns the temporal distance between the temporal network point and the
 * geometry point
 */
PGDLLEXPORT Datum
distance_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  ensure_point_type(gs);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo_internal(tempgeom,
    PointerGetDatum(gs));
  pfree(tempgeom);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(distance_tnpoint_npoint);
/**
 * Returns the temporal distance between the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
distance_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_geom(np);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = distance_tpoint_geo_internal(tempgeom, geom);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the temporal distance between the two temporal network points
 */
static Temporal *
distance_tnpoint_tnpoint_internal(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  Temporal *geomsync1 = tnpoint_tgeompoint(sync1);
  Temporal *geomsync2 = tnpoint_tgeompoint(sync2);
  Temporal *result = distance_tpoint_tpoint_internal(geomsync1, geomsync2);
  pfree(sync1); pfree(sync2);
  pfree(geomsync1); pfree(geomsync2);
  return result;
}

PG_FUNCTION_INFO_V1(distance_tnpoint_tnpoint);
/**
 * Returns the temporal distance between the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
distance_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *result = distance_tnpoint_tnpoint_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant (NAI)
 *****************************************************************************/

/**
 * Returns the nearest approach instant of the temporal network point and the
 * geometry (internal function)
 */
static TInstant *
NAI_tnpoint_geo_internal(FunctionCallInfo fcinfo, Temporal *temp,
  GSERIALIZED *gs)
{
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = NAI_tpoint_geo_internal(fcinfo, tempgeom, gs);
  /* We do not call the function tgeompointinst_to_tnpointinst to avoid
   * roundoff errors. The closest point may be at an exclusive bound. */
  Datum value;
  bool found = temporal_value_at_timestamp_inc(temp, resultgeom->t, &value);
  assert(found);
  TInstant *result = tinstant_make(value, resultgeom->t, temp->temptype);
  pfree(tempgeom); pfree(resultgeom); pfree(DatumGetPointer(value));
  return result;
}

PG_FUNCTION_INFO_V1(NAI_geo_tnpoint);
/**
 * Returns the nearest approach instant of the geometry and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAI_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = NAI_tnpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tnpoint_geo);
/**
 * Returns the nearest approach instant of the temporal network point and the
 * geometry
 */
PGDLLEXPORT Datum
NAI_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  TInstant *result = NAI_tnpoint_geo_internal(fcinfo, temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the nearest approach instant of the network point and the temporal
 * network point (internal function)
 */
static TInstant *
NAI_tnpoint_npoint_internal(FunctionCallInfo fcinfo, Temporal *temp,
  npoint *np)
{
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  TInstant *resultgeom = NAI_tpoint_geo_internal(fcinfo, tempgeom, gs);
  /* We do not call the function tgeompointinst_to_tnpointinst to avoid
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

PG_FUNCTION_INFO_V1(NAI_npoint_tnpoint);
/**
 * Returns the nearest approach instant of the network point and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAI_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  TInstant *result = NAI_tnpoint_npoint_internal(fcinfo, temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tnpoint_npoint);
/**
 * Returns the nearest approach instant of the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
NAI_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  TInstant *result = NAI_tnpoint_npoint_internal(fcinfo, temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(NAI_tnpoint_tnpoint);
/**
 * Returns the nearest approach instant of the two temporal network points
 */
PGDLLEXPORT Datum
NAI_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *dist = distance_tnpoint_tnpoint_internal(temp1, temp2);
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
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance (NAD)
 *****************************************************************************/

/**
 * Returns the nearest approach distance of the temporal network point and the
 * geometry
 */
static Datum
NAD_tnpoint_geo_internal(Temporal *temp, GSERIALIZED *gs)
{
  Datum traj = tnpoint_geom(temp);
  Datum result = geom_distance2d(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  return result;
}

PG_FUNCTION_INFO_V1(NAD_geo_tnpoint);
/**
 * Returns the nearest approach distance of the geometry and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAD_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = NAD_tnpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_geo);
/**
 * Returns the nearest approach distance of the temporal network point and the
 * geometry
 */
PGDLLEXPORT Datum
NAD_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum result = NAD_tnpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

/**
 * Returns the nearest approach distance of the temporal network point and the
 * network point
 */
static Datum
NAD_tnpoint_npoint_internal(Temporal *temp, npoint *np)
{
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = geom_distance2d(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  return result;
}

PG_FUNCTION_INFO_V1(NAD_npoint_tnpoint);
/**
 * Returns the nearest approach distance of the network point and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAD_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = NAD_tnpoint_npoint_internal(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_npoint);
/**
 * Returns the nearest approach distance of the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
NAD_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum result = NAD_tnpoint_npoint_internal(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_tnpoint_tnpoint);
/**
 * Returns the nearest approach distance of the two temporal network points
 */
PGDLLEXPORT Datum
NAD_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *dist = distance_tnpoint_tnpoint_internal(temp1, temp2);
  if (dist == NULL)
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Datum result = temporal_min_value_internal(dist);
  pfree(dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * ShortestLine
 *****************************************************************************/

PG_FUNCTION_INFO_V1(shortestline_geo_tnpoint);
/**
 * Returns the line connecting the nearest approach point between the geometry
 * and the temporal network point
 */
PGDLLEXPORT Datum
shortestline_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(shortestline_npoint_tnpoint);
/**
 * Returns the line connecting the nearest approach point between the network
 * point and the temporal network point
 */
PGDLLEXPORT Datum
shortestline_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(shortestline_tnpoint_geo);
/**
 * Returns the line connecting the nearest approach point between the temporal
 * network point and the geometry
 */
PGDLLEXPORT Datum
shortestline_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(shortestline_tnpoint_npoint);
/**
 * Returns the line connecting the nearest approach point between the temporal
 * network point and the network point
 */
PGDLLEXPORT Datum
shortestline_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(shortestline_tnpoint_tnpoint);
/**
 * Returns the line connecting the nearest approach point between the two
 * temporal networks
 */
PGDLLEXPORT Datum
shortestline_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Temporal *geomsync1 = tnpoint_tgeompoint(sync1);
  Temporal *geomsync2 = tnpoint_tgeompoint(sync2);
  Datum result;
  bool found = shortestline_tpoint_tpoint_internal(geomsync1, geomsync2, &result);
  pfree(geomsync1); pfree(geomsync2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (!found)
    PG_RETURN_NULL();
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/
