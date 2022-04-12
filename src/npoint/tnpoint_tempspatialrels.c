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
 * @file tnpoint_tempspatialrels.c
 * @brief Temporal spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 * tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

#include "npoint/tnpoint_tempspatialrels.h"

/* PostGIS */
#include <liblwgeom.h>
/* MobilityDB */
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
 * Return the temporal disjoint/intersection relationship between the temporal
 * network point and the network point
 */
static Temporal *
tinterrel_tnpoint_npoint(Temporal *temp, npoint *np, bool tinter,
  bool restr, Datum atvalue)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(npoint_geom(np));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(tempgeom, gs, tinter,
    restr, atvalue);
  pfree(tempgeom);
  pfree(gs);
  return result;
}

/**
 * Return the temporal disjoint/intersection relationship between the temporal
 * network point and the network point
 */
static Datum
tinterrel_tnpoint_npoint_generic(FunctionCallInfo fcinfo, bool tinter)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, tinter,
    restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Return the temporal disjoint/intersection relationship between the network
 * point and the temporal network point
 */
static Datum
tinterrel_npoint_tnpoint(FunctionCallInfo fcinfo, bool tinter)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  npoint *np = PG_GETARG_NPOINT(0);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, tinter, restr,
    atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Return the temporal disjoint/intersection relationship between the temporal
 * network point and the geometry
 */
static Temporal *
tinterrel_tnpoint_geo(Temporal *temp, GSERIALIZED *gs, bool tinter,
  bool restr, Datum atvalue)
{
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(gs));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(tempgeom, gs, tinter, restr,
    atvalue);
  pfree(tempgeom);
  return result;
}

/**
 * Return the temporal disjoint/intersection relationship between the geometry
 * and the temporal network point
 */
static Datum
tinterrel_geo_tnpoint(FunctionCallInfo fcinfo, bool tinter)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tinterrel_tnpoint_geo(temp, gs, tinter, restr,
    atvalue);
  pfree(tempgeom);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/**
 * Return the temporal disjoint/intersection relationship between the temporal
 * network point and the geometry
 */
static Datum
tinterrel_tnpoint_geo_generic(FunctionCallInfo fcinfo, bool tinter)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tinterrel_tnpoint_geo(temp, gs, tinter, restr,
    atvalue);
  pfree(tempgeom);
  PG_FREE_IF_COPY(gs, 1);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/

/**
 * Return the temporal touches relationship between the temporal network point
 * and the geometry
 */
static Temporal *
ttouches_tnpoint_geo(Temporal *temp, GSERIALIZED *gs,
  bool restr, Datum atvalue)
{
  ensure_same_srid(tnpoint_srid(temp), gserialized_get_srid(gs));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo(tempgeom, gs, restr, atvalue);
  pfree(tempgeom);
  return result;
}

/**
 * Return the temporal touches relationship between the temporal network point
 * and the network point
 */
static Temporal *
ttouches_tnpoint_npoint(Temporal *temp, npoint *np,
  bool restr, Datum atvalue)
{
  ensure_same_srid(tnpoint_srid(temp), npoint_srid(np));
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  GSERIALIZED *gs = (GSERIALIZED *) DatumGetPointer(npoint_geom(np));
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = ttouches_tpoint_geo(tempgeom, gs, restr, atvalue);
  pfree(tempgeom);
  pfree(gs);
  return result;
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tcontains_geo_tnpoint);
/**
 * Return the temporal contains relationship between the geometry and the
 * temporal network point
 */
PGDLLEXPORT Datum
Tcontains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tcontains_geo_tpoint(gs, tempgeom, restr,
    atvalue);
  pfree(tempgeom);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tdisjoint_geo_tnpoint);
/**
 * Return the temporal disjoint relationship between the geometry and the
 * temporal network point
 */
PGDLLEXPORT Datum
Tdisjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return tinterrel_geo_tnpoint(fcinfo, TDISJOINT);
}

PG_FUNCTION_INFO_V1(Tdisjoint_npoint_tnpoint);
/**
 * Return the temporal disjoint relationship between the network point and the
 * temporal network point
 */
PGDLLEXPORT Datum
Tdisjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return tinterrel_npoint_tnpoint(fcinfo, TDISJOINT);
}

PG_FUNCTION_INFO_V1(Tdisjoint_tnpoint_geo);
/**
 * Return the temporal disjoint relationship between the temporal network point
 * and the geometry
 */
PGDLLEXPORT Datum
Tdisjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return tinterrel_tnpoint_geo_generic(fcinfo, TDISJOINT);
}

PG_FUNCTION_INFO_V1(Tdisjoint_tnpoint_npoint);
/**
 * Return the temporal disjoint relationship between the temporal network point
 * and the network point
 */
PGDLLEXPORT Datum
Tdisjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return tinterrel_tnpoint_npoint_generic(fcinfo, TDISJOINT);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tintersects_geo_tnpoint);
/**
 * Return the temporal intersects relationship between the geometry and the
 * temporal network point
 */
PGDLLEXPORT Datum
Tintersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return tinterrel_geo_tnpoint(fcinfo, TINTERSECTS);
}

PG_FUNCTION_INFO_V1(Tintersects_npoint_tnpoint);
/**
 * Return the temporal intersects relationship between the network point and
 * the temporal network point
 */
PGDLLEXPORT Datum
Tintersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return tinterrel_npoint_tnpoint(fcinfo, TINTERSECTS);
}

PG_FUNCTION_INFO_V1(Tintersects_tnpoint_geo);
/**
 * Return the temporal intersects relationship between the temporal network
 * point and the geometry
 */
PGDLLEXPORT Datum
Tintersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return tinterrel_tnpoint_geo_generic(fcinfo, TINTERSECTS);
}

PG_FUNCTION_INFO_V1(Tintersects_tnpoint_npoint);
/**
 * Return the temporal intersects relationship between the temporal network
 * point and the network point
 */
PGDLLEXPORT Datum
Tintersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return tinterrel_tnpoint_npoint_generic(fcinfo, TINTERSECTS);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Ttouches_geo_tnpoint);
/**
 * Return the temporal touches relationship between the geometry and
 * the temporal network point
 */
PGDLLEXPORT Datum
Ttouches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = ttouches_tnpoint_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
   if (result == NULL)
    PG_RETURN_NULL();
 PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Ttouches_npoint_tnpoint);
/**
 * Return the temporal touches relationship between the network point and
 * the temporal network point
 */
PGDLLEXPORT Datum
Ttouches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = ttouches_tnpoint_npoint(temp, np, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Ttouches_tnpoint_geo);
/**
 * Return the temporal touches relationship between the temporal network point and
 * the geometry
 */
PGDLLEXPORT Datum
Ttouches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = ttouches_tnpoint_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Ttouches_tnpoint_npoint);
/**
 * Return the temporal touches relationship between the temporal network point and
 * the network point
 */
PGDLLEXPORT Datum
Ttouches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np = PG_GETARG_NPOINT(1);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 3)
  {
    atvalue = PG_GETARG_DATUM(2);
    restr = true;
  }
  Temporal *result = ttouches_tnpoint_npoint(temp, np, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PG_FUNCTION_INFO_V1(Tdwithin_geo_tnpoint);
/**
 * Return a temporal Boolean that states whether the geometry and the
 * temporal network point are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tempgeom, gs, dist, restr, atvalue);
  pfree(tempgeom);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdwithin_npoint_tnpoint);
/**
 * Return a temporal Boolean that states whether the network point and the
 * temporal network point are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  npoint *np  = PG_GETARG_NPOINT(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tempgeom, gs, dist, restr, atvalue);
  pfree(gs);
  PG_FREE_IF_COPY(temp, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdwithin_tnpoint_geo);
/**
 * Return a temporal Boolean that states whether the temporal network point
 * and the geometry are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  if (gserialized_is_empty(gs))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tempgeom, gs, dist, restr, atvalue);
  pfree(tempgeom);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdwithin_tnpoint_npoint);
/**
 * Return a temporal Boolean that states whether the temporal network point
 * and the network point are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  npoint *np  = PG_GETARG_NPOINT(1);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  Datum geom = npoint_geom(np);
  GSERIALIZED *gs = (GSERIALIZED *) PG_DETOAST_DATUM(geom);
  Temporal *tempgeom = tnpoint_tgeompoint(temp);
  Temporal *result = tdwithin_tpoint_geo(tempgeom, gs, dist, restr, atvalue);
  pfree(tempgeom);
  pfree(gs);
  PG_FREE_IF_COPY(temp, 0);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(Tdwithin_tnpoint_tnpoint);
/**
 * Return a temporal Boolean that states whether the temporal network points
 * are within the given distance
 */
PGDLLEXPORT Datum
Tdwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  Datum dist = PG_GETARG_DATUM(2);
  bool restr = false;
  Datum atvalue = (Datum) NULL;
  if (PG_NARGS() == 4)
  {
    atvalue = PG_GETARG_DATUM(3);
    restr = true;
  }
  Temporal *geomsync1 = tnpoint_tgeompoint(temp1);
  Temporal *geomsync2 = tnpoint_tgeompoint(temp2);
  Temporal *result = tdwithin_tpoint_tpoint(geomsync1, geomsync2, dist,
    restr, atvalue);
  pfree(geomsync1); pfree(geomsync2);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result == NULL)
    PG_RETURN_NULL();
  PG_RETURN_POINTER(result);
}

/*****************************************************************************/
