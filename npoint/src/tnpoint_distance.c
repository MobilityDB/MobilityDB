/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

#include "tnpoint_distance.h"

#include "temporaltypes.h"
#include "temporal_util.h"
#include "lifting.h"
#include "tpoint_spatialfuncs.h"
#include "tpoint_distance.h"
#include "tnpoint.h"
#include "tnpoint_static.h"
#include "tnpoint_spatialfuncs.h"
#include "tnpoint_tempspatialrels.h"

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

PG_FUNCTION_INFO_V1(distance_geo_tnpoint);

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

PG_FUNCTION_INFO_V1(NAI_geometry_tnpoint);
/**
 * Nearest approach instant of a geometry and a temporal network point
 */
PGDLLEXPORT Datum
NAI_geometry_tnpoint(PG_FUNCTION_ARGS)
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
 * Nearest approach instant of a network point and a temporal network point
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

PG_FUNCTION_INFO_V1(NAI_tnpoint_geometry);
/**
 * Nearest approach instant of a temporal network point and a geometry
 */
PGDLLEXPORT Datum
NAI_tnpoint_geometry(PG_FUNCTION_ARGS)
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
 * Nearest approach instant of a temporal network point and a network point
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
 * Nearest approach instant of two temporal network points
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

PG_FUNCTION_INFO_V1(NAD_geometry_tnpoint);
/**
 * Nearest approach distance of a geometry and a temporal network point
 */
PGDLLEXPORT Datum
NAD_geometry_tnpoint(PG_FUNCTION_ARGS)
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
  Datum result = call_function2(distance, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_npoint_tnpoint);
/**
 * Nearest approach distance of a network point and a temporal network point
 */
PGDLLEXPORT Datum
NAD_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(distance, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_geometry);
/**
 * Nearest approach distance of a temporal network point and a geometry
 */
PGDLLEXPORT Datum
NAD_tnpoint_geometry(PG_FUNCTION_ARGS)
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
  Datum result = call_function2(distance, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_npoint);
/**
 * Nearest approach distance of a temporal network point and a network point
 */
PGDLLEXPORT Datum
NAD_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *)PG_DETOAST_DATUM(geom);
  Datum traj = tnpoint_geom(temp);
  Datum result = call_function2(distance, traj, PointerGetDatum(gs));
  pfree(DatumGetPointer(traj));
  POSTGIS_FREE_IF_COPY_P(gs, DatumGetPointer(geom));
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(NAD_tnpoint_tnpoint);
/**
 * Nearest approach distance of two temporal network points
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

PG_FUNCTION_INFO_V1(shortestline_geometry_tnpoint);
/**
 * Shortest line of a geometry and a temporal network point
 */
PGDLLEXPORT Datum
shortestline_geometry_tnpoint(PG_FUNCTION_ARGS)
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
 * Shortest line of a network point and a temporal network point
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

PG_FUNCTION_INFO_V1(shortestline_tnpoint_geometry);
/**
 * Shortest line of a temporal network point and a geometry
 */
PGDLLEXPORT Datum
shortestline_tnpoint_geometry(PG_FUNCTION_ARGS)
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
 * Shortest line of a temporal network point and a network point
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
 * Shortest line of two temporal network points
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
