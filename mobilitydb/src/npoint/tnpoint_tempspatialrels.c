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
 * @brief Temporal spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * boolean/text. The following relationships are supported:
 * tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

#include "npoint/tnpoint_tempspatialrels.h"

/* MEOS */
#include <meos.h>
#include "point/tpoint_tempspatialrels.h"
/* MobilityDB */
#include "pg_general/temporal.h"
#include "pg_point/postgis.h"

/*****************************************************************************
 * Generic functions
 *****************************************************************************/

/**
 * @brief Return the temporal disjoint/intersection relationship between a temporal
 * network point and the network point
 */
static Datum
Tinterrel_tnpoint_npoint(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, tinter, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal disjoint/intersection relationship between a network
 * point and the temporal network point
 */
static Datum
Tinterrel_npoint_tnpoint(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tinterrel_tnpoint_npoint(temp, np, tinter, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal disjoint/intersection relationship between a geometry
 * and the temporal network point
 */
static Datum
Tinterrel_geo_tnpoint(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tinterrel_tnpoint_geo(temp, geo, tinter, restr, atvalue);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal disjoint/intersection relationship between a temporal
 * network point and the geometry
 */
static Datum
Tinterrel_tnpoint_geo(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tinterrel_tnpoint_geo(temp, geo, tinter, restr, atvalue);
  PG_FREE_IF_COPY(geo, 1);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PGDLLEXPORT Datum Tcontains_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry contains a
 * temporal network point
 * @sqlfn tContains()
 */
Datum
Tcontains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_geo_tnpoint(geo, temp, restr, atvalue);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Tdisjoint_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry is disjoint
 * from a temporal network point
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_geo_tnpoint(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a network point is
 * disjoint from a temporal network point
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_npoint_tnpoint(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point is disjoint from a geometry
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return Tinterrel_tnpoint_geo(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point is disjoint from a network point
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_tnpoint_npoint(fcinfo, TDISJOINT);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PGDLLEXPORT Datum Tintersects_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry intersects a
 * temporal network point
 * @sqlfn tIntersects()
 */
Datum
Tintersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_geo_tnpoint(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a network point
 * intersects a temporal network point
 * @sqlfn tIntersects()
 */
Datum
Tintersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_npoint_tnpoint(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point intersects a geometry
 * @sqlfn tIntersects()
 */
Datum
Tintersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return Tinterrel_tnpoint_geo(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point intersects a network point
 * @sqlfn tIntersects()
 */
Datum
Tintersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_tnpoint_npoint(fcinfo, TINTERSECTS);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PGDLLEXPORT Datum Ttouches_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry touches a
 * temporal network point
 * @sqlfn tTouches()
 */
Datum
Ttouches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_geo_tnpoint(geo, temp, restr, atvalue);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
   if (! result)
    PG_RETURN_NULL();
 PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a network point touches
 * a temporal network point
 * @sqlfn tTouches()
 */
Datum
Ttouches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_npoint_tnpoint(np, temp, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point touches a geometry
 * @sqlfn tTouches()
 */
Datum
Ttouches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tnpoint_geo(temp, geo, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point touches a network point
 * @sqlfn tTouches()
 */
Datum
Ttouches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tnpoint_npoint(temp, np, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PGDLLEXPORT Datum Tdwithin_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry and a
 * temporal network point are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_geo_tnpoint(geo, temp, dist, restr, atvalue);
  PG_FREE_IF_COPY(geo, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network
 * point and a geometry are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *geo = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tnpoint_geo(temp, geo, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(geo, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a network point and a
 * temporal network point are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Npoint *np  = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_npoint_tnpoint(np, temp, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal network point
 * and a network point are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np  = PG_GETARG_NPOINT_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tnpoint_npoint(temp, np, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether two temporal network
 * points are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tnpoint_tnpoint(temp1, temp2, dist, restr,
    atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
