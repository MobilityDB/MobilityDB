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
 * @brief Ever and always spatial relationships for temporal geos
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

#include "geo/tgeo_spatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>
#include "general/temporal.h" /* For varfunc */
#include "geo/tgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

#if CBUFFER
/**
 * @brief Return true if a geometry and a temporal spatial value ever/always
 * satisfy a spatial relationship
 */
Datum
EA_spatialrel_geo_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const GSERIALIZED *, const Temporal *, bool), bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(gs, temp, ever);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a geometry and a temporal spatial value ever/always
 * satisfy a spatial relationship
 */
Datum
EA_spatialrel_tspatial_geo(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *, bool), bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = func(temp, gs, ever);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if two temporal circular buffers ever/always satisfy the spatial
 * relationship
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func1,func2 Spatial relationship for geometry points
 * @param[in] ever True to compute the ever semantics, false for always
 */
Datum
EA_spatialrel_tspatial_tspatial(FunctionCallInfo fcinfo,
  datum_func2 func1, datum_func2 func2, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = MEOS_FLAGS_GET_GEODETIC(temp1->flags) ?
    ea_spatialrel_tspatial_tspatial(temp1, temp2, func2, ever) :
    ea_spatialrel_tspatial_tspatial(temp1, temp2, func1, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Return true if a temporal spatial value and a geometry are 
 * ever/always within a distance
 */
Datum
EA_dwithin_tspatial_geo(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *, double dist, bool),
  bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = func(temp, gs, dist, ever);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a temporal spatial value and a geometry are 
 * ever/always within a distance
 */
Datum
EA_dwithin_geo_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const GSERIALIZED *, const Temporal *, double dist, bool),
  bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = func(gs, temp, dist, ever);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}
#endif /* CBUFFER */

/*****************************************************************************
 * Ever contains
 * The function does not accept 3D or geography since it is based on the
 * PostGIS ST_Relate function
 *****************************************************************************/

/**
 * @brief Return true if a geometry ever/always contains a temporal geo
 * @sqlfn eContains(), aContains()
 */
static Datum
EAcontains_geo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? econtains_geo_tgeo(gs, temp) :
    acontains_geo_tgeo(gs, temp);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Econtains_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry ever contains a temporal geo
 * @sqlfn eContains()
 */
inline Datum
Econtains_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcontains_geo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Acontains_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry always contains a temporal geo
 * @sqlfn aContains()
 */
inline Datum
Acontains_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EAcontains_geo_tgeo(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever disjoint (for both geometry and geography)
 *****************************************************************************/

/**
 * @brief Return true if a geometry/geography and a temporal geo are
 * ever/always disjoint
 * @sqlfn eDisjoint(), aDisjoint()
 */
static Datum
EA_disjoint_geo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? edisjoint_tgeo_geo(temp, gs) :
    adisjoint_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geo are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_geo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geo are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_geo_tgeo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal geo and a geometry/geography are
 * ever/always disjoint
 * @sqlfn eDisjoint(), Adisjoint()
 */
static Datum
EA_disjoint_tgeo_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = ever ? edisjoint_tgeo_geo(temp, gs) :
    adisjoint_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry/geography are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tgeo_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry/geography are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tgeo_geo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if two temporal geos are ever/always disjoint
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] ever True to compute the ever semantics, false for always
 */
static Datum
EA_disjoint_tgeo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = ea_disjoint_tgeo_tgeo(temp1, temp2, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Edisjoint_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geos are ever disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tgeo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geos are ever disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_disjoint_tgeo_tgeo(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

/**
 * @brief Return true if a geometry/geography and a temporal geo ever/always
 * intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EA_intersects_geo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? eintersects_tgeo_geo(temp, gs) : 
    aintersects_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geo ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_intersects_geo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geo ever
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_intersects_geo_tgeo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a geometry/geography and a temporal geo ever/always
 * intersect
 * @sqlfn eintersects(), aintersects()
 */
static Datum
EA_intersects_tgeo_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = ever ? eintersects_tgeo_geo(temp, gs) : 
    aintersects_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Eintersects_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry/geography ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_intersects_tgeo_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry/geography always
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_intersects_tgeo_geo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if two temporal geos ever/always intersect
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] ever True to compute the ever semantics, false for always
 */
static Datum
EA_intersects_tgeo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = ea_intersects_tgeo_tgeo(temp1, temp2, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

PGDLLEXPORT Datum Eintersects_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geos ever intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_intersects_tgeo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Aintersects_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geos ever intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_intersects_tgeo_tgeo(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal geo ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
static Datum
EA_touches_geo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ever ? etouches_tgeo_geo(temp, gs) :
    atouches_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 1);
  PG_FREE_IF_COPY(gs, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal geo ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal geo ever touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tgeo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal geo and a geometry ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
static Datum
EA_touches_tgeo_geo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  int result = ever ? etouches_tgeo_geo(temp, gs) :
    atouches_tgeo_geo(temp, gs);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Etouches_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tgeo_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Atouches_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tgeo_geo(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever/always dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

/**
 * @brief Return true if a geometry/geography and a temporal geo are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EA_dwithin_geo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tgeo_geo(temp, gs, dist) :
    adwithin_tgeo_geo(temp, gs, dist);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geo are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geo are always
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tgeo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if a temporal geo and a geometry/geography are
 * ever/always within a distance
 * @sqlfn eDwithin()
 */
static Datum
EA_dwithin_tgeo_geo(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ever ? edwithin_tgeo_geo(temp, gs, dist) :
    adwithin_tgeo_geo(temp, gs, dist);
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry/geography are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tgeo_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geo and a geometry/geography are ever
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tgeo_geo(fcinfo, ALWAYS);
}

/**
 * @brief Return true if two temporal geos are even/always within a distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EA_dwithin_tgeo_tgeo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ea_dwithin_tgeo_tgeo(temp1, temp2, dist, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel
 * @brief Return true if two temporal geos are ever within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tgeo_tgeo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel
 * @brief Return true if two temporal geos are always within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tgeo_tgeo(fcinfo, ALWAYS);
}

/*****************************************************************************/
