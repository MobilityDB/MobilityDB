/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2021, PostGIS contributors
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
 * Temporal distance for temporal network points.
 */

#include "npoint/tnpoint_distance.h"

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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  ensure_point_type(gs);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_RETURN_NULL();
  }

  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = distance_tpoint_geo_internal((const Temporal *) geomtemp,
    PointerGetDatum(gs));
  pfree(geomtemp);
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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum geom = npoint_as_geom_internal(np);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = distance_tpoint_geo_internal((const Temporal *) geomtemp,
    geom);
  pfree(DatumGetPointer(geom));
  pfree(geomtemp);
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  ensure_point_type(gs);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }

  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = distance_tpoint_geo_internal(geomtemp,
    PointerGetDatum(gs));
  pfree(geomtemp);
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_as_geom_internal(np);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = distance_tpoint_geo_internal(geomtemp, geom);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Returns the temporal distance between the two temporal network points
 */
Temporal *
distance_tnpoint_tnpoint_internal(const Temporal *temp1, const Temporal *temp2)
{
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
    return NULL;

  Temporal *geomsync1 = tnpoint_as_tgeompoint_internal(sync1);
  Temporal *geomsync2 = tnpoint_as_tgeompoint_internal(sync2);
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
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Temporal *result = distance_tnpoint_tnpoint_internal(temp1, temp2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAI_geo_tnpoint);
/**
 * Returns the nearest approach instant of the geometry and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAI_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }

  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  TInstant *geomresult = NAI_tpoint_geo_internal(fcinfo, geomtemp, gs);
  /* We do not do call the function tgeompointinst_as_tnpointinst to avoid
   * roundoff errors */
  Temporal *result = temporal_restrict_timestamp_internal(temp,
    geomresult->t, REST_AT);
  pfree(geomtemp); pfree(geomresult);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  TInstant *geomresult = NAI_tpoint_geo_internal(fcinfo, geomtemp, gs);
  /* We do not do call the function tgeompointinst_as_tnpointinst to avoid
   * roundoff errors */
  Temporal *result = temporal_restrict_timestamp_internal(temp,
    geomresult->t, REST_AT);
  pfree(geomtemp); pfree(geomresult);
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }

  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  TInstant *geomresult = NAI_tpoint_geo_internal(fcinfo, geomtemp, gs);
  /* We do not do call the function tgeompointinst_as_tnpointinst to avoid
   * roundoff errors */
  Temporal *result = temporal_restrict_timestamp_internal(temp,
    geomresult->t, REST_AT);
  pfree(geomtemp); pfree(geomresult);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  TInstant *geomresult = NAI_tpoint_geo_internal(fcinfo, geomtemp, gs);
  /* We do not do call the function tgeompointinst_as_tnpointinst to avoid
   * roundoff errors */
  Temporal *result = temporal_restrict_timestamp_internal(temp,
    geomresult->t, REST_AT);
  pfree(geomtemp); pfree(geomresult);
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAI_tnpoint_tnpoint);
/**
 * Returns the nearest approach instant of the two temporal network points
 */
PGDLLEXPORT Datum
NAI_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  TInstant *result = NULL;
  Temporal *dist = distance_tnpoint_tnpoint_internal(temp1, temp2);
  if (dist != NULL)
  {
    const TInstant *min = temporal_min_instant((const Temporal *) dist);
    result = (TInstant *) temporal_restrict_timestamp_internal(temp1, min->t, REST_AT);
    pfree(dist);
    if (result == NULL)
    {
      if (temp1->subtype == SEQUENCE)
        result = tinstant_copy(tsequence_inst_at_timestamp_excl(
          (TSequence *) temp1, min->t));
      else /* temp->subtype == SEQUENCESET */
        result = tinstant_copy(tsequenceset_inst_at_timestamp_excl(
          (TSequenceSet *) temp1, min->t));
    }
  }
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(NAD_geo_tnpoint);
/**
 * Returns the nearest approach distance of the geometry and the temporal
 * network point
 */
PGDLLEXPORT Datum
NAD_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }

  Datum traj = tnpoint_geom(temp);
  Datum result = geom_distance2d(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = geom_distance2d(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_geo);
/**
 * Returns the nearest approach distance of the temporal network point and the
 * geometry
 */
PGDLLEXPORT Datum
NAD_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }

  Datum traj = tnpoint_geom(temp);
  Datum result = geom_distance2d(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_npoint);
/**
 * Returns the nearest approach distance of the temporal network point and the
 * network point
 */
PGDLLEXPORT Datum
NAD_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = geom_distance2d(traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_tnpoint);
/**
 * Returns the nearest approach distance of the two temporal network points
 */
PGDLLEXPORT Datum
NAD_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(gs, 0);
    PG_FREE_IF_COPY(temp, 1);
    PG_RETURN_NULL();
  }

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
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
  {
    PG_FREE_IF_COPY(temp, 0);
    PG_FREE_IF_COPY(gs, 1);
    PG_RETURN_NULL();
  }

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
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(LWGEOM_shortestline2d, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
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
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }

  Temporal *geomsync1 = tnpoint_as_tgeompoint_internal(sync1);
  Temporal *geomsync2 = tnpoint_as_tgeompoint_internal(sync2);
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
