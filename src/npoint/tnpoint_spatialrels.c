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
 * Ever spatial relationships for temporal network points.
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
 * Generic binary functions for tnpoint <rel> geo
 *****************************************************************************/

/**
 * Generic spatial relationships for a temporal network point and a geometry
 *
 * @param[in] temp Temporal network point
 * @param[in] geom Geometry
 * @param[in] func PostGIS function to be called
 * @param[in] invert True if the arguments should be inverted
 */
static Datum
spatialrel_tnpoint_geom(const Temporal *temp, Datum geom,
  Datum (*func)(Datum, Datum), bool invert)
{
  Datum geom1 = tnpoint_geom(temp);
  Datum result = invert ? func(geom, geom1) : func(geom1, geom);
  pfree(DatumGetPointer(geom1));
  return result;
}

/*****************************************************************************/

/**
 * Returns true if the temporal network points ever satisfy the spatial
 * relationship
 *
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Spatial relationship
 */
static Datum
spatialrel_tnpoint_tnpoint(FunctionCallInfo fcinfo, Datum (*func)(Datum, Datum))
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  ensure_same_srid(tnpoint_srid_internal(temp1), tnpoint_srid_internal(temp2));
  Temporal *tpoint1 = tnpoint_tgeompoint(temp1);
  Temporal *tpoint2 = tnpoint_tgeompoint(temp2);
  /* Fill the lifted structure */
  LiftedFunctionInfo lfinfo;
  memset(&lfinfo, 0, sizeof(LiftedFunctionInfo));
  lfinfo.func = (varfunc) func;
  lfinfo.numparam = 0;
  lfinfo.argoids = true;
  lfinfo.argtypid[0] = tpoint1->basetypid;
  lfinfo.argtypid[1] = tpoint2->basetypid;
  lfinfo.restypid = BOOLOID;
  lfinfo.reslinear = STEP;
  lfinfo.invert = INVERT_NO;
  lfinfo.discont = MOBDB_FLAGS_GET_LINEAR(tpoint1->flags) ||
    MOBDB_FLAGS_GET_LINEAR(tpoint2->flags);
  lfinfo.tpfunc_base = NULL;
  lfinfo.tpfunc = NULL;
  int result = efunc_temporal_temporal(tpoint1, tpoint2, &lfinfo);
  /* Finish */
  pfree(tpoint1); pfree(tpoint2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result == 1 ? true : false);
}

/*****************************************************************************
 * Generic ternary functions for tnpoint <rel> geo/tnpoint
 *****************************************************************************/

/**
 * Generic spatial relationships for a temporal network point and a geometry
 *
 * @param[in] temp Temporal network point
 * @param[in] geom Geometry
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 * @param[in] invert True if the arguments should be inverted
 */
static Datum
spatialrel3_tnpoint_geom(const Temporal *temp, Datum geom, Datum param,
  Datum (*func)(Datum, Datum, Datum), bool invert)
{
  Datum geom1 = tnpoint_geom(temp);
  Datum result = invert ? func(geom, geom1, param) :
    func(geom1, geom, param);
  pfree(DatumGetPointer(geom1));
  return result;
}

/**
 * Generic spatial relationships for two temporal network points
 *
 * @param[in] temp1,temp2 Temporal network points
 * @param[in] param Parameter
 * @param[in] func PostGIS function to be called
 */
static Datum
spatialrel3_tnpoint_tnpoint(const Temporal *temp1,const  Temporal *temp2,
  Datum param, Datum (*func)(Datum, Datum, Datum))
{
  Datum geom1 = tnpoint_geom(temp1);
  Datum geom2 = tnpoint_geom(temp2);
  Datum result = func(geom1, geom2, param);
  pfree(DatumGetPointer(geom1)); pfree(DatumGetPointer(geom2));
  return result;
}

/*****************************************************************************
 * Ever contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(contains_geo_tnpoint);
/**
 * Returns true if the geometry contains the trajectory of the temporal network
 * point
 */
PGDLLEXPORT Datum
contains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_contains, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************
 * Ever disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(disjoint_geo_tnpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal network
 * point are disjoint
 */
PGDLLEXPORT Datum
disjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_disjoint2d, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(disjoint_npoint_tnpoint);
/**
 * Returns true if the network point and the trajectory of the temporal
 * network point are disjoint
 */
PGDLLEXPORT Datum
disjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np  = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_disjoint2d, true);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(disjoint_tnpoint_geo);
/**
 * Returns true if the trajectory of the temporal network point and the
 * geometry are disjoint
 */
PGDLLEXPORT Datum
disjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_disjoint2d, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(disjoint_tnpoint_npoint);
/**
 * Returns true if the trajectory of the temporal network point and the
 * network point are disjoint
 */
PGDLLEXPORT Datum
disjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np  = PG_GETARG_NPOINT(1);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_disjoint2d, true);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(disjoint_tnpoint_tnpoint);
/**
 * Returns true if the temporal points are ever disjoint
 */
PGDLLEXPORT Datum
disjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_tnpoint(fcinfo, &datum2_point_ne);
}

/*****************************************************************************
 * Ever intersects
 *****************************************************************************/

PG_FUNCTION_INFO_V1(intersects_geo_tnpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal network
 * point intersect
 */
PGDLLEXPORT Datum
intersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_intersects2d, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(intersects_npoint_tnpoint);
/**
 * Returns true if the network point and the trajectory of the temporal network
 * point intersect
 */
PGDLLEXPORT Datum
intersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np  = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_intersects2d, true);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(intersects_tnpoint_geo);
/**
 * Returns true if the trajectory of the temporal network point and the
 * geometry intersect
 */
PGDLLEXPORT Datum
intersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_intersects2d, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(intersects_tnpoint_npoint);
/**
 * Returns true if the trajectory of the temporal network point and the network
 * point intersect
 */
PGDLLEXPORT Datum
intersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np  = PG_GETARG_NPOINT(1);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_intersects2d, false);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/

PG_FUNCTION_INFO_V1(intersects_tnpoint_tnpoint);
/**
 * Returns true if the temporal points are ever disjoint
 */
PGDLLEXPORT Datum
intersects_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return spatialrel_tnpoint_tnpoint(fcinfo, &datum2_point_eq);
}

/*****************************************************************************
 * Ever dwithin
 *****************************************************************************/

PG_FUNCTION_INFO_V1(dwithin_geo_tnpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal network
 * point are within the given distance
 */
PGDLLEXPORT Datum
dwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(dwithin_npoint_tnpoint);
/**
 * Returns true if the network point and the trajectory of the temporal network
 * point are within the given distance
 */
PGDLLEXPORT Datum
dwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np  = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, true);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(dwithin_tnpoint_geo);
/**
 * Returns true if the trajectory of the temporal network point and the
 * geometry are within the given distance
 */
PGDLLEXPORT Datum
dwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(dwithin_tnpoint_npoint);
/**
 * Returns true if the trajectory of the temporal network point and the
 * network point are within the given distance
 */
PGDLLEXPORT Datum
dwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np  = PG_GETARG_NPOINT(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel3_tnpoint_geom(temp, geom, dist, &geom_dwithin2d, false);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(dwithin_tnpoint_tnpoint);
/**
 * Returns true if the trajectories of the temporal network points are within
 * the given distance
 */
PGDLLEXPORT Datum
dwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
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

PG_FUNCTION_INFO_V1(touches_geo_tnpoint);
/**
 * Returns true if the geometry and the trajectory of the temporal network
 * point touch
 */
PGDLLEXPORT Datum
touches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  Datum geom = PG_GETARG_DATUM(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_touches, true);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}
/**
 * Returns true if the network point and the trajectory of the temporal
 * network point touch
 */
PG_FUNCTION_INFO_V1(touches_npoint_tnpoint);

PGDLLEXPORT Datum
touches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np  = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_touches, true);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(touches_tnpoint_geo);
/**
 * Returns true if the trajectory of the temporal network point and the
 * geometry touch
 */
PGDLLEXPORT Datum
touches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum geom = PG_GETARG_DATUM(1);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_touches, false);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

PG_FUNCTION_INFO_V1(touches_tnpoint_npoint);
/**
 * Returns true if the trajectory of the temporal network point and the
 * network point touch
 */
PGDLLEXPORT Datum
touches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np  = PG_GETARG_NPOINT(1);
  Datum geom = npoint_geom(np);
  Datum result = spatialrel_tnpoint_geom(temp, geom, &geom_touches, false);
  pfree(DatumGetPointer(geom));
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/*****************************************************************************/
