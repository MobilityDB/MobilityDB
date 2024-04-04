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
 * @brief Temporal spatial relationships for temporal points
 * These relationships are applied at each instant and result in a temporal
 * Boolean.
 *
 * Depending on the parameters various relationships are available
 * - For a temporal geometry point and a geometry:
 *   `tcontains`, `tdisjoint`, `tintersects`, `ttouches`, and `tdwithin`.
 * - For two temporal **geometry** points:
 *   `tdisjoint`, `tintersects`, `tdwithin`.
 * - For two temporal **geography** points:
 *   `tdisjoint`, `tintersects`.
 */

#include "point/tpoint_tempspatialrels.h"

/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic functions for computing the temporal spatial relationships
 * with arbitrary geometries
 *****************************************************************************/

/**
 * @brief Return the temporal disjoint/intersection relationship between a
 * geometry and a temporal point
 */
static Datum
Tinterrel_geo_tpoint(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, tinter, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal disjoint/intersection relationship between a
 * geometry and a temporal point
 */
static Datum
Tinterrel_tpoint_geo(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_geo(temp, gs, tinter, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return the temporal disjoint/intersection relationship between two
 * temporal points
 */
static Datum
Tinterrel_tpoint_tpoint(FunctionCallInfo fcinfo, bool tinter)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  /* Result depends on whether we are computing tintersects or tdisjoint */
  Temporal *result = tinterrel_tpoint_tpoint(temp1, temp2, tinter, restr,
    atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PGDLLEXPORT Datum Tcontains_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry contains a
 * temporal point
 * @sqlfn tContains()
 */
Datum
Tcontains_geo_tpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = tcontains_geo_tpoint(gs, temp, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Tdisjoint_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal point
 * is disjoint from a geometry
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_geo_tpoint(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal point
 * is disjoint from a geometry
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  return Tinterrel_tpoint_geo(fcinfo, TDISJOINT);
}

PGDLLEXPORT Datum Tdisjoint_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether two temporal points
 * are disjoint
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_tpoint_tpoint(fcinfo, TDISJOINT);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geography points
 *****************************************************************************/

PGDLLEXPORT Datum Tintersects_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal point
 * intersects a geometry
 * @sqlfn tIntersects()
 */
Datum
Tintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_geo_tpoint(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal point
 * intersects a geometry
 * @sqlfn tIntersects()
 */
Datum
Tintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  return Tinterrel_tpoint_geo(fcinfo, TINTERSECTS);
}

PGDLLEXPORT Datum Tintersects_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether two temporal points
 * are disjoint
 * @sqlfn tIntersects()
 */
Datum
Tintersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return Tinterrel_tpoint_tpoint(fcinfo, TINTERSECTS);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PGDLLEXPORT Datum Ttouches_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry touches a
 * temporal point
 * @sqlfn tTouches()
 */
Datum
Ttouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tpoint_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Ttouches_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal point
 * touches a geometry
 * @sqlfn tTouches()
 */
Datum
Ttouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 2 && ! PG_ARGISNULL(2))
  {
    atvalue = PG_GETARG_BOOL(2);
    restr = true;
  }
  Temporal *result = ttouches_tpoint_geo(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geography points
 *****************************************************************************/

PGDLLEXPORT Datum Tdwithin_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a geometry and a
 * temporal point are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tpoint_geo(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

PGDLLEXPORT Datum Tdwithin_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether a temporal point and
 * a geometry are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2))
    PG_RETURN_NULL();
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  bool restr = false;
  bool atvalue = false;
  if (PG_NARGS() > 3 && ! PG_ARGISNULL(3))
  {
    atvalue = PG_GETARG_BOOL(3);
    restr = true;
  }
  Temporal *result = tdwithin_tpoint_geo(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_temp
 * @brief Return a temporal boolean that states whether two temporal points
 * are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
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
  Temporal *result = tdwithin_tpoint_tpoint(temp1, temp2, dist, restr,
    atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/
