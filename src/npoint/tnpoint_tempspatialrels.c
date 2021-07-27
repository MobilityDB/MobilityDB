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
 * @file tnpoint_tempspatialrels.c
 * Temporal spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 * tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

#include "npoint/tnpoint_tempspatialrels.h"

#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H 1

#include <liblwgeom.h>

#include "general/temporaltypes.h"
#include "general/tempcache.h"
#include "general/temporal_util.h"
#include "general/lifting.h"
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
#include "point/tpoint_tempspatialrels.h"
#include "npoint/tnpoint.h"
#include "npoint/tnpoint_static.h"
#include "npoint/tnpoint_spatialfuncs.h"
#include "npoint/tnpoint_distance.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * Returns the temporal disjoint/intersection relationship between the temporal
 * network point and the network point
 */
Temporal *
tinterrel_tnpoint_geo(Temporal *temp, GSERIALIZED *gs, bool tinter)
{
  ensure_same_srid_tnpoint_gs(temp, gs);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(geomtemp, gs, tinter);
  pfree(geomtemp); 
  return result;
}

/**
 * Returns the temporal disjoint/intersection relationship between the temporal
 * network point and the network point
 */
Temporal *
tinterrel_tnpoint_npoint(Temporal *temp, npoint *np, bool tinter)
{
  ensure_same_srid_tnpoint_npoint(temp, np);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(npoint_as_geom_internal(np));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(geomtemp, gs, tinter);
  pfree(geomtemp); 
  pfree(gs);
  return result;
}

/**
 * Returns the temporal touches relationship between the temporal network point
 * and the geometry
 */
static Temporal *
ttouches_tnpoint_geo_internal(Temporal *temp, GSERIALIZED *gs)
{
  ensure_same_srid_tnpoint_gs(temp, gs);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo_internal(geomtemp, gs);
  pfree(geomtemp); 
  return result;
}

/**
 * Returns the temporal touches relationship between the temporal network point
 * and the network point
 */
static Temporal *
ttouches_tnpoint_npoint_internal(Temporal *temp, npoint *np)
{
  ensure_same_srid_tnpoint_npoint(temp, np);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(npoint_as_geom_internal(np));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo_internal(geomtemp, gs);
  pfree(geomtemp); 
  pfree(gs);
  return result;
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tcontains_geo_tnpoint);
/**
 * Returns the temporal contains relationship between the geometry and the
 * temporal network point
 */
PGDLLEXPORT Datum
tcontains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = tcontains_geo_tpoint_internal(gs, geomtemp);
  pfree(geomtemp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdisjoint_geo_tnpoint);
/**
 * Returns the temporal disjoint relationship between the geometry and the
 * temporal network point
 */
PGDLLEXPORT Datum
tdisjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = tinterrel_tnpoint_geo(temp, gs, TDISJOINT);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_npoint_tnpoint);
/**
 * Returns the temporal disjoint relationship between the network point and the
 * temporal network point
 */
PGDLLEXPORT Datum
tdisjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, TDISJOINT);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tnpoint_geo);
/**
 * Returns the temporal disjoint relationship between the temporal network point
 * and the geometry
 */
PGDLLEXPORT Datum
tdisjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tinterrel_tnpoint_geo(temp, gs, TDISJOINT);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdisjoint_tnpoint_npoint);
/**
 * Returns the temporal disjoint relationship between the temporal network point
 * and the network point
 */
PGDLLEXPORT Datum
tdisjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, TDISJOINT);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tintersects_geo_tnpoint);
/**
 * Returns the temporal intersects relationship between the geometry and the
 * temporal network point
 */
PGDLLEXPORT Datum
tintersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = tinterrel_tnpoint_geo(temp, gs, TINTERSECTS);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_npoint_tnpoint);
/**
 * Returns the temporal intersects relationship between the network point and
 * the temporal network point
 */
PGDLLEXPORT Datum
tintersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, TINTERSECTS);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tnpoint_geo);
/**
 * Returns the temporal intersects relationship between the temporal network
 * point and the network point
 */
PGDLLEXPORT Datum
tintersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = tinterrel_tnpoint_geo(temp, gs, TINTERSECTS);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tintersects_tnpoint_npoint);

PGDLLEXPORT Datum
tintersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, TINTERSECTS);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(ttouches_geo_tnpoint);
/**
 * Returns the temporal touches relationship between the geometry and
 * the temporal network point
 */
PGDLLEXPORT Datum
ttouches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = ttouches_tnpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttouches_npoint_tnpoint);
/**
 * Returns the temporal touches relationship between the network point and
 * the temporal network point
 */
PGDLLEXPORT Datum
ttouches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Temporal *result = ttouches_tnpoint_npoint_internal(temp, np);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(ttouches_tnpoint_geo);
/**
 * Returns the temporal touches relationship between the temporal network point and
 * the geometry
 */
PGDLLEXPORT Datum
ttouches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *result = ttouches_tnpoint_geo_internal(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);}

PG_FUNCTION_INFO_V1(ttouches_tnpoint_npoint);
/**
 * Returns the temporal touches relationship between the temporal network point and
 * the network point
 */
PGDLLEXPORT Datum
ttouches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np = PG_GETARG_NPOINT(1);
  Temporal *result = ttouches_tnpoint_npoint_internal(temp, np);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PG_FUNCTION_INFO_V1(tdwithin_geo_tnpoint);

PGDLLEXPORT Datum
tdwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum dist = PG_GETARG_DATUM(2);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = tdwithin_tpoint_geo_internal(geomtemp, gs, dist);
  pfree(geomtemp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_npoint_tnpoint);

PGDLLEXPORT Datum
tdwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np  = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = tdwithin_tpoint_geo_internal(geomtemp, gs, dist);
    pfree(gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tnpoint_geo);

PGDLLEXPORT Datum
tdwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  Datum dist = PG_GETARG_DATUM(2);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = tdwithin_tpoint_geo_internal(geomtemp, gs, dist);
  pfree(geomtemp);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tnpoint_npoint);

PGDLLEXPORT Datum
tdwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL(0);
  npoint *np  = PG_GETARG_NPOINT(1);
  Datum dist = PG_GETARG_DATUM(2);
  Datum geom = npoint_as_geom_internal(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Temporal *geomtemp = tnpoint_as_tgeompoint_internal(temp);
  Temporal *result = tdwithin_tpoint_geo_internal(geomtemp, gs, dist);
  pfree(geomtemp);
  PG_FREE_IF_COPY(temp, 0);
    pfree(gs);
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(tdwithin_tnpoint_tnpoint);

PGDLLEXPORT Datum
tdwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL(1);
  Datum dist = PG_GETARG_DATUM(2);
  Temporal *geomsync1 = tnpoint_as_tgeompoint_internal(temp1);
  Temporal *geomsync2 = tnpoint_as_tgeompoint_internal(temp2);
  Temporal *result = tdwithin_tpoint_tpoint_internal(geomsync1, geomsync2, dist);
  pfree(geomsync1); pfree(geomsync2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
