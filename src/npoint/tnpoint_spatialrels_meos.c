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
 * @file tnpoint_spatialrels.c
 * @brief Ever spatial relationships for temporal network points.
 *
 * These relationships compute the ever spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported:
 * contains, disjoint, intersects, touches, and dwithin
 */

#include "npoint/tnpoint_spatialrels.h"

/* MobilityDB */
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"

/*****************************************************************************
 * Ever contains
 *****************************************************************************/

/**
 * Return true if the geometry contains the trajectory of the temporal network
 * point
 */
bool
contains_geo_tnpoint(const GSERIALIZED *gs, const Temporal *tnpoint)
{
  return spatialrel_geo_tnpoint(fcinfo, &geom_contains);
}

/*****************************************************************************
 * Ever disjoint
 *****************************************************************************/

/**
 * Return true if the geometry and the trajectory of the temporal network
 * point are disjoint
 */
bool
disjoint_geo_tnpoint(const GSERIALIZED *gs, const Temporal *tnpoint)
{
  return spatialrel_geo_tnpoint(fcinfo, &geom_disjoint2d);
}

/**
 * Return true if the network point and the trajectory of the temporal
 * network point are disjoint
 */
bool
disjoint_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return spatialrel_npoint_tnpoint(fcinfo, &geom_disjoint2d);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * geometry are disjoint
 */
bool
disjoint_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *gs)
{
  return spatialrel_tnpoint_geo(fcinfo, &geom_disjoint2d);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * network point are disjoint
 */
bool
disjoint_tnpoint_npoint( const Temporal *tnpoint, const Npoint *np)
{
  return spatialrel_tnpoint_npoint(fcinfo, &geom_disjoint2d);
}

/**
 * Return true if the temporal points are ever disjoint
 */
bool
disjoint_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return spatialrel_tnpoint_tnpoint(tnpoint1, tnpoint2, &datum2_point_ne);
}

/*****************************************************************************
 * Ever intersects
 *****************************************************************************/

/**
 * Return true if the geometry and the trajectory of the temporal network
 * point intersect
 */
bool
intersects_geo_tnpoint(const GSERIALIZED *gs, const Temporal *tnpoint)
{
  return spatialrel_geo_tnpoint(fcinfo, &geom_intersects2d);
}

/**
 * Return true if the network point and the trajectory of the temporal network
 * point intersect
 */
bool
intersects_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return spatialrel_npoint_tnpoint(fcinfo, &geom_intersects2d);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * geometry intersect
 */
bool
intersects_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *gs)
{
  return spatialrel_tnpoint_geo(fcinfo, &geom_intersects2d);
}

/**
 * Return true if the trajectory of the temporal network point and the network
 * point intersect
 */
bool
intersects_tnpoint_npoint(const Temporal *tnpoint, const Npoint *np)
{
  return spatialrel_tnpoint_npoint(fcinfo, &geom_intersects2d);
}

/**
 * Return true if the temporal points are ever disjoint
 */
bool
intersects_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  return spatialrel_tnpoint_tnpoint(tnpoint1, tnpoint2, &datum2_point_eq);
}

/*****************************************************************************
 * Ever dwithin
 *****************************************************************************/

/**
 * Return true if the geometry and the trajectory of the temporal network
 * point are within the given distance
 */
bool
dwithin_geo_tnpoint(const GSERIALIZED *gs, const Temporal *tnpoint)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Return true if the network point and the trajectory of the temporal network
 * point are within the given distance
 */
bool
dwithin_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  Npoint *np  = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, true);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * geometry are within the given distance
 */
bool
dwithin_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *gs)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * network point are within the given distance
 */
bool
dwithin_tnpoint_npoint( const Temporal *tnpoint, const Npoint *np)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np  = PG_GETARG_NPOINT_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, false);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * Return true if the trajectories of the temporal network points are within
 * the given distance
 */
bool
dwithin_tnpoint_tnpoint(const Temporal *tnpoint1, const Temporal *tnpoint2)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  Temporal *sync1, *sync2;
  /* Return NULL if the temporal points do not intersect in time */
  if (!intersection_temporal_temporal(temp1, temp2, SYNCHRONIZE_NOCROSS,
    &sync1, &sync2))
  {
    PG_FREE_IF_COPY(temp1, 0);
    PG_FREE_IF_COPY(temp2, 1);
    PG_RETURN_NULL();
  }
  Datum result = spatialrel3_tnpoint_tnpoint(sync1, sync2, dist, &geom_dwithin2d);
  pfree(sync1); pfree(sync2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Ever touches
 *****************************************************************************/

/**
 * Return true if the geometry and the trajectory of the temporal network
 * point touch
 */
bool
touches_geo_tnpoint(const GSERIALIZED *gs, const Temporal *tnpoint)
{
  return spatialrel_geo_tnpoint(fcinfo, &geom_touches);
}

/**
 * Return true if the network point and the trajectory of the temporal
 * network point touch
 */
bool
touches_npoint_tnpoint(const Npoint *np, const Temporal *tnpoint)
{
  return spatialrel_npoint_tnpoint(fcinfo, &geom_touches);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * geometry touch
 */
bool
touches_tnpoint_geo(const Temporal *tnpoint, const GSERIALIZED *gs)
{
  return spatialrel_tnpoint_geo(fcinfo, &geom_touches);
}

/**
 * Return true if the trajectory of the temporal network point and the
 * network point touch
 */
bool
touches_tnpoint_npoint( const Temporal *tnpoint, const Npoint *np)
{
  return spatialrel_tnpoint_npoint(fcinfo, &geom_touches);
}

/*****************************************************************************/
