/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @brief Temporal spatial relationships for temporal geos
 * @details These relationships are applied at each instant and result in a
 * temporal Boolean.
 *
 * Depending on the parameters various relationships are available
 * - For a temporal geometry geo and a geometry:
 *   `tcontains`, `tdisjoint`, `tintersects`, `ttouches`, and `tdwithin`.
 * - For two temporal **geometries**:
 *   `tdisjoint`, `tintersects`, `tdwithin`.
 * - For two temporal **geographies**:
 *   `tdisjoint`, `tintersects`.
 */

#include "geo/tgeo_tempspatialrels.h"

/* PostGIS */
#include <liblwgeom.h>
/* MEOS */
#include <meos.h>
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Generic functions for computing the temporal spatial relationships
 * with arbitrary geometries
 *****************************************************************************/

/**
 * @brief Return a temporal boolean that states whether a geometry and a
 * spatiotemporal value satisfy a spatial relationship
 */
Datum
Tspatialrel_geo_tspatial(FunctionCallInfo fcinfo,
  Temporal * (*func)(const GSERIALIZED *, const Temporal *, bool, bool))
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
  Temporal *result = func(gs, temp, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return a temporal boolean that states whether a spatial temporal
 * value and a geometry satisfy a spatial relationship
 */
Datum
Tspatialrel_tspatial_geo(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const GSERIALIZED *, bool, bool))
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
  Temporal *result = func(temp, gs, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return a temporal boolean that states whether two spatial temporal
 * values satisfy a spatial relationship
 */
Datum
Tspatialrel_tspatial_tspatial(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Temporal *, bool, bool))
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
  Temporal *result = func(temp1, temp2, restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************/

/**
 * @brief Return a temporal boolean that states whether a geometry and a
 * spatiotemporal value are within a given distance
 */
Datum
Tdwithin_geo_tspatial(FunctionCallInfo fcinfo,
  Temporal * (*func)(const GSERIALIZED *, const Temporal *, double, bool, bool))
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
  Temporal *result = func(gs, temp, dist, restr, atvalue);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return a temporal boolean that states whether a spatiotemporal value
 * and a geometry are within a given distance
 */
Datum
Tdwithin_tspatial_geo(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const GSERIALIZED *, double, bool, bool))
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
  Temporal *result = func(temp, gs, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/**
 * @brief Return a temporal boolean that states whether two spatiotemporal
 * values are within a given distance
 */
Datum
Tdwithin_tspatial_tspatial(FunctionCallInfo fcinfo,
  Temporal * (*func)(const Temporal *, const Temporal *, double, bool, bool))
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
  Temporal *result = func(temp1, temp2, dist, restr, atvalue);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (! result)
    PG_RETURN_NULL();
  PG_RETURN_TEMPORAL_P(result);
}

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PGDLLEXPORT Datum Tcontains_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a geometry contains a
 * temporal geo
 * @sqlfn tContains()
 */
Datum
Tcontains_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tcontains_geo_tgeo);
}

PGDLLEXPORT Datum Tcontains_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geometry
 * contains a geometry
 * @sqlfn tContains()
 */
Datum
Tcontains_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tcontains_tgeo_geo);
}

PGDLLEXPORT Datum Tcontains_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geometry
 * contains another temporal geometry
 * @sqlfn tContains()
 */
Datum
Tcontains_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tcontains_tgeo_tgeo);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PGDLLEXPORT Datum Tcovers_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a geometry covers a
 * temporal geo
 * @sqlfn tCovers()
 */
Datum
Tcovers_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tcovers_geo_tgeo);
}

PGDLLEXPORT Datum Tcovers_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geometry
 * covers a geometry
 * @sqlfn tCovers()
 */
Datum
Tcovers_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tcovers_tgeo_geo);
}

PGDLLEXPORT Datum Tcovers_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geometry
 * covers another temporal geometry
 * @sqlfn tCovers()
 */
Datum
Tcovers_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tcovers_tgeo_tgeo);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Tdisjoint_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geo
 * is disjoint from a geometry
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tdisjoint_geo_tgeo);
}

PGDLLEXPORT Datum Tdisjoint_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geo
 * is disjoint from a geometry
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tdisjoint_tgeo_geo);
}

PGDLLEXPORT Datum Tdisjoint_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether two temporal geos
 * are disjoint
 * @sqlfn tDisjoint()
 */
inline Datum
Tdisjoint_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tdisjoint_tgeo_tgeo);
}

/*****************************************************************************
 * Temporal intersects
 * Available for temporal geographies
 *****************************************************************************/

PGDLLEXPORT Datum Tintersects_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geo
 * intersects a geometry
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tintersects_geo_tgeo);
}

PGDLLEXPORT Datum Tintersects_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geo
 * intersects a geometry
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tintersects_tgeo_geo);
}

PGDLLEXPORT Datum Tintersects_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether two temporal geos
 * are disjoint
 * @sqlfn tIntersects()
 */
inline Datum
Tintersects_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tintersects_tgeo_tgeo);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PGDLLEXPORT Datum Ttouches_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a geometry touches a
 * temporal geo
 * @sqlfn tTouches()
 * @note We cannot call Tspatialrel_geo_tspatial since the function
 * ttouches_geo_tspatial is not defined in MEOS because it is redundant
 */
Datum
Ttouches_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &ttouches_geo_tgeo);
}

PGDLLEXPORT Datum Ttouches_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geo
 * touches a geometry
 * @sqlfn tTouches()
 */
Datum
Ttouches_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &ttouches_tgeo_geo);
}

PGDLLEXPORT Datum Ttouches_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geometry
 * touches another temporal geometry
 * @sqlfn tTouches()
 */
Datum
Ttouches_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &ttouches_tgeo_tgeo);
}

/*****************************************************************************
 * Temporal dwithin
 * Available for temporal geographies
 *****************************************************************************/

PGDLLEXPORT Datum Tdwithin_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a geometry and a
 * temporal geo are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_geo_tgeo(PG_FUNCTION_ARGS)
{
  return Tdwithin_geo_tspatial(fcinfo, &tdwithin_geo_tgeo);
}

PGDLLEXPORT Datum Tdwithin_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether a temporal geo and
 * a geometry are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tgeo_geo(PG_FUNCTION_ARGS)
{
  return Tdwithin_tspatial_geo(fcinfo, &tdwithin_tgeo_geo);
}

/*****************************************************************************/

PGDLLEXPORT Datum Tdwithin_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return a temporal boolean that states whether two spatiotemporal
 * values are within a given distance
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return Tdwithin_tspatial_tspatial(fcinfo, &tdwithin_tgeo_tgeo);
}

/*****************************************************************************/
