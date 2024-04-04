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
 * @brief Ever and always spatial relationships for temporal points
 *
 * These relationships compute the ever/always spatial relationship between the
 * arguments and return a Boolean. These functions may be used for filtering
 * purposes before applying the corresponding temporal spatial relationship.
 *
 * The following relationships are supported for geometries: `eContains`,
 * `aContains`, `eDisjoint`, `aDisjoint`, `eIntersects`, `aIntersects`,
 * `eTouches`, aTouches`, `eDwithin`, and `aDwithin`.
 *
 * The following relationships are supported for geographies: `eDisjoint`,
 * `aDisjoint`, `eIntersects`, `aIntersects`, `eDwithin`, and `aDwithin`.
 *
 * Only `eDisjoint`, `eDwithin`, and `eIntersects` are supported for 3D
 * geometries.
 */

#include "point/tpoint_spatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include "general/temporal.h" /* For varfunc */
#include "point/tpoint_spatialfuncs.h"
/* MobilityDB */
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Return true if two temporal points ever/always satisfy the spatial
 * relationship
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func1,func2 Spatial relationship for geometry/geography points
 * @param[in] ever True to compute the ever semantics, false for always
 */
static Datum
EAspatialrel_tpoint_tpoint(FunctionCallInfo fcinfo,
  Datum (*func1)(Datum, Datum), Datum (*func2)(Datum, Datum), bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tpoint_tpoint(temp1, temp2, func2, ever) :
    ea_spatialrel_tpoint_tpoint(temp1, temp2, func1, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************
 * Ever contains
 * The function does not accept 3D or geography since it is based on the
 * PostGIS ST_Relate function
 *****************************************************************************/

/**
 * @brief Return true if a geometry ever/always contains a temporal point
 * @sqlfn eContains(), aContains()
 */
static Datum
EAcontains_geo_tpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? econtains_geo_tpoint(gs, temp) :
    acontains_geo_tpoint(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Econtains_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry ever contains a temporal point
 * @sqlfn eContains()
 */
Datum
Econtains_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAcontains_geo_tpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Acontains_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry always contains a temporal point
 * @sqlfn aContains()
 */
Datum
Acontains_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAcontains_geo_tpoint(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever disjoint (for both geometry and geography)
 *****************************************************************************/

/**
 * @brief Return true if a geometry/geography and a temporal point are
 * ever/always disjoint
 * @sqlfn eDisjoint(), aDisjoint()
 */
static Datum
EAdisjoint_geo_tpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? edisjoint_tpoint_geo(temp, gs) :
    adisjoint_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry/geography and a temporal point are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAdisjoint_geo_tpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry/geography and a temporal point are always
 * disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAdisjoint_geo_tpoint(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal point and a geometry/geography are
 * ever/always disjoint
 * @sqlfn eDisjoint(), Adisjoint()
 */
static Datum
EAdisjoint_tpoint_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = ever ? edisjoint_tpoint_geo(temp, gs) :
    adisjoint_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry/geography are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAdisjoint_tpoint_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry/geography are always
 * disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAdisjoint_tpoint_geo(fcinfo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal points are ever disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tpoint_tpoint(fcinfo, &datum2_point_ne,
    &datum2_point_nsame, EVER);
}

PGDLLEXPORT Datum Adisjoint_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal points are ever disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tpoint_tpoint(fcinfo, &datum2_point_ne,
    &datum2_point_nsame, ALWAYS);
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

/**
 * @brief Return true if a geometry/geography and a temporal point ever/always
 * intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EAintersects_geo_tpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ?
    eintersects_tpoint_geo(temp, gs) : aintersects_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry/geography and a temporal point ever
 * intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAintersects_geo_tpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry/geography and a temporal point ever
 * intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAintersects_geo_tpoint(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a geometry/geography and a temporal point ever/always
 * intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EAintersects_tpoint_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = ever ?
    eintersects_tpoint_geo(temp, gs) : aintersects_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry/geography ever
 * intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAintersects_tpoint_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry/geography always
 * intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAintersects_tpoint_geo(fcinfo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal points ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tpoint_tpoint(fcinfo, &datum2_point_eq,
    &datum2_point_same, EVER);
}

PGDLLEXPORT Datum Aintersects_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal points ever intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tpoint_tpoint(fcinfo, &datum2_point_eq,
    &datum2_point_same, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal point ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
static Datum
EAtouches_geo_tpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? etouches_tpoint_geo(temp, gs) :
    atouches_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal point ever touch
 * @sqlfn eTouches()
 */
Datum
Etouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAtouches_geo_tpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal point ever touch
 * @sqlfn aTouches()
 */
Datum
Atouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAtouches_geo_tpoint(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal point and a geometry ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
static Datum
EAtouches_tpoint_geo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = ever ? etouches_tpoint_geo(temp, gs) :
    atouches_tpoint_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry ever touch
 * @sqlfn eTouches()
 */
Datum
Etouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAtouches_tpoint_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry always touch
 * @sqlfn aTouches()
 */
Datum
Atouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAtouches_tpoint_geo(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever/always dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

/**
 * @brief Return true if a geometry/geography and a temporal point are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EAdwithin_geo_tpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ?
    edwithin_tpoint_geo(temp, gs, dist) :
    adwithin_tpoint_geo(temp, gs, dist);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry/geography and a temporal point are ever
 * within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_geo_tpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_geo_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry/geography and a temporal point are always
 * within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_geo_tpoint(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal point and a geometry/geography are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EAdwithin_tpoint_geo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tpoint_geo(temp, gs, dist) :
    adwithin_tpoint_geo(temp, gs, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry/geography are ever
 * within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAdwithin_tpoint_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal point and a geometry/geography are ever
 * within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EAdwithin_tpoint_geo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if two temporal points are even/always within a distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EAdwithin_tpoint_tpoint(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tpoint_tpoint(temp1, temp2, dist) :
    adwithin_tpoint_tpoint(temp1, temp2, dist);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if two temporal points are ever within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_tpoint_tpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tpoint_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tpoint_tpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel
 * @brief Return true if two temporal points are always within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_tpoint_tpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_tpoint_tpoint(fcinfo, ALWAYS);
}

/*****************************************************************************/
