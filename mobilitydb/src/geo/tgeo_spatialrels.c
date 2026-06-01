/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Ever and always spatial relationships for temporal geometries
 * @details These relationships compute the ever/always spatial relationship 
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding spatiotemporal
 * relationship.
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

/* PostgreSQL */
#include <funcapi.h>
#include <utils/array.h>
/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>
#include "temporal/temporal.h" /* For varfunc */
#include "geo/tgeo_spatialfuncs.h"
/* MobilityDB */
#include "pg_temporal/type_util.h"
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Generic ever/always spatial relationship functions
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a spatiotemporal value ever/always
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
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Return true if a spatiotemporal value and a geometry ever/always
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
  PG_RETURN_BOOL(result ? true : false);
}

/**
 * @brief Return true if two spatiotemporal values ever/always satisfy the
 * spatial relationship
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Spatial relationship for temporal geometry/geography
 * @param[in] ever True to compute the ever semantics, false for always
 */
Datum
EA_spatialrel_tspatial_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Temporal *, bool), bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = func(temp1, temp2, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result ? true : false);
}

/*****************************************************************************/

/**
 * @brief Return true if a geometry and a spatiotemporal value are 
 * ever/always within a distance
 */
Datum
EA_dwithin_geo_tspatial(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *, double dist, bool),
  bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = func(temp, gs, dist, ever);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a spatiotemporal value and a geometry are ever/always
 * within a distance
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

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

PGDLLEXPORT Datum Econtains_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry ever contains a temporal geometry
 * @sqlfn eContains()
 */
inline Datum
Econtains_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_contains_geo_tgeo, EVER);
}

PGDLLEXPORT Datum Acontains_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry always contains a temporal geometry
 * @sqlfn aContains()
 */
inline Datum
Acontains_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_contains_geo_tgeo, ALWAYS);
}

PGDLLEXPORT Datum Econtains_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry ever contains a geometry
 * @sqlfn eContains()
 */
inline Datum
Econtains_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_contains_tgeo_geo, EVER);
}

PGDLLEXPORT Datum Acontains_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry always contains a geometry
 * @sqlfn aContains()
 */
inline Datum
Acontains_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_contains_tgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Econtains_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry ever contains another one
 * @sqlfn eContains()
 */
inline Datum
Econtains_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_contains_tgeo_tgeo, EVER);
}

PGDLLEXPORT Datum Acontains_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry always contains another one
 * @sqlfn aContains()
 */
inline Datum
Acontains_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_contains_tgeo_tgeo, ALWAYS);
}

/*****************************************************************************
 * Ever covers
 *****************************************************************************/

PGDLLEXPORT Datum Ecovers_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry ever covers a temporal geometry
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_covers_geo_tgeo, EVER);
}

PGDLLEXPORT Datum Acovers_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry always covers a temporal geometry
 * @sqlfn aCovers()
 */
inline Datum
Acovers_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_covers_geo_tgeo, ALWAYS);
}

PGDLLEXPORT Datum Ecovers_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry ever covers a geometry
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_covers_tgeo_geo, EVER);
}

PGDLLEXPORT Datum Acovers_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry always covers a geometry
 * @sqlfn aCovers()
 */
inline Datum
Acovers_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_covers_tgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Ecovers_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry ever covers another one
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_covers_tgeo_tgeo, EVER);
}

PGDLLEXPORT Datum Acovers_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry always covers another one
 * @sqlfn aCovers()
 */
inline Datum
Acovers_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_covers_tgeo_tgeo, ALWAYS);
}

/*****************************************************************************
 * Ever disjoint (for both geometry and geography)
 *****************************************************************************/

PGDLLEXPORT Datum Edisjoint_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geometry are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_disjoint_geo_tgeo, EVER);
}

PGDLLEXPORT Datum Adisjoint_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geometry are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_disjoint_geo_tgeo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry/geography are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_tgeo_geo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry/geography are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_tgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geometrys are ever disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_disjoint_tgeo_tgeo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geometrys are ever disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_disjoint_tgeo_tgeo, ALWAYS);
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

PGDLLEXPORT Datum Eintersects_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geometry ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_intersects_geo_tgeo, EVER);
}

PGDLLEXPORT Datum Aintersects_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geometry ever
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_intersects_geo_tgeo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry/geography ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_intersects_tgeo_geo, EVER);
}

PGDLLEXPORT Datum Aintersects_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry/geography always
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_intersects_tgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geometrys ever intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_intersects_tgeo_tgeo, EVER);
}

PGDLLEXPORT Datum Aintersects_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geometrys ever intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_intersects_tgeo_tgeo, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal geometry ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
Datum
EA_touches_geo_tgeo(FunctionCallInfo fcinfo, bool ever, bool tpoint)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = tpoint ? ea_touches_tpoint_geo(temp, gs, ever) :
    ea_touches_tgeo_geo(temp, gs, ever);
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
 * @brief Return true if a geometry and a temporal geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tgeo(fcinfo, EVER, TGEO);
}

PGDLLEXPORT Datum Atouches_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal geometry ever touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tgeo(fcinfo, ALWAYS, TGEO);
}

PGDLLEXPORT Datum Etouches_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_tpoint);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal point ever touch each other
 * @sqlfn eTouches()
 */
inline Datum
Etouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tgeo(fcinfo, EVER, TPOINT);
}

PGDLLEXPORT Datum Atouches_geo_tpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_tpoint);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal point ever touch each other
 * @sqlfn aTouches()
 */
inline Datum
Atouches_geo_tpoint(PG_FUNCTION_ARGS)
{
  return EA_touches_geo_tgeo(fcinfo, ALWAYS, TPOINT);
}

/*****************************************************************************/

/**
 * @brief Return true if a temporal geometry and a geometry ever/always touch
 * @sqlfn eTouches(), aTouches()
 */
Datum
EA_touches_tgeo_geo(FunctionCallInfo fcinfo, bool ever, bool tpoint)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  int result = tpoint ? ea_touches_tpoint_geo(temp, gs, ever) :
    ea_touches_tgeo_geo(temp, gs, ever);
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
 * @brief Return true if a temporal geometry and a geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tgeo_geo(fcinfo, EVER, TGEO);
}

PGDLLEXPORT Datum Atouches_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tgeo_geo(fcinfo, ALWAYS, TGEO);
}

PGDLLEXPORT Datum Etouches_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tpoint_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tgeo_geo(fcinfo, EVER, TPOINT);
}

PGDLLEXPORT Datum Atouches_tpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tpoint_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tpoint_geo(PG_FUNCTION_ARGS)
{
  return EA_touches_tgeo_geo(fcinfo, ALWAYS, TPOINT);
}

/*****************************************************************************/

PGDLLEXPORT Datum Etouches_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geometries ever touch each other
 * @sqlfn eTouches()
 */
inline Datum
Etouches_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_touches_tgeo_tgeo, EVER);
}

PGDLLEXPORT Datum Atouches_tgeo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tgeo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal geometries always touch each other
 * @sqlfn aTouches()
 */
inline Datum
Atouches_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_touches_tgeo_tgeo, ALWAYS);
}

/*****************************************************************************
 * Ever/always dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

PGDLLEXPORT Datum Edwithin_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geometry are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tspatial(fcinfo, &ea_dwithin_tgeo_geo, EVER);
}

PGDLLEXPORT Datum Adwithin_geo_tgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_geo_tgeo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry/geography and a temporal geometry are always
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_geo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tspatial(fcinfo, &ea_dwithin_tgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Edwithin_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry/geography are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tspatial_geo(fcinfo, &ea_dwithin_tgeo_geo, EVER);
}

PGDLLEXPORT Datum Adwithin_tgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tgeo_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal geometry and a geometry/geography are ever
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tgeo_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tspatial_geo(fcinfo, &ea_dwithin_tgeo_geo, ALWAYS);
}

/**
 * @brief Return true if two temporal geometries are even/always within a distance
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
 * @brief Return true if two temporal geometries are ever within a distance
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
 * @brief Return true if two temporal geometries are always within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_tgeo_tgeo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tgeo_tgeo(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Set-set spatial join
 *****************************************************************************/

/**
 * @brief State carried across the calls of a set-set spatial-join SRF
 */
typedef struct
{
  int *pairs;        /**< Flattened (i, j) index pairs, length 2 * max_calls */
  SpanSet **periods; /**< Per-pair spansets for tDwithinPairs, NULL otherwise */
} TgeoarrJoinState;

PGDLLEXPORT Datum Edwithin_tgeoarr_tgeoarr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tgeoarr_tgeoarr);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return the index pairs of two arrays of temporal geos that are ever
 * within a distance
 * @sqlfn eDwithinPairs()
 */
Datum
Edwithin_tgeoarr_tgeoarr(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    ArrayType *array1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType *array2 = PG_GETARG_ARRAYTYPE_P(1);
    double dist = PG_GETARG_FLOAT8(2);
    int count1, count2, npairs = 0;
    Temporal **arr1 = (Temporal **) temparr_extract(array1, &count1);
    Temporal **arr2 = (Temporal **) temparr_extract(array2, &count2);
    TgeoarrJoinState *state = palloc0(sizeof(TgeoarrJoinState));
    state->pairs = edwithin_tgeoarr_tgeoarr((const Temporal **) arr1, count1,
      (const Temporal **) arr2, count2, dist, &npairs);
    pfree(arr1); pfree(arr2);
    PG_FREE_IF_COPY(array1, 0); PG_FREE_IF_COPY(array2, 1);
    funcctx->user_fctx = state;
    funcctx->max_calls = npairs;
    get_call_result_type(fcinfo, NULL, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }
  funcctx = SRF_PERCALL_SETUP();
  if (funcctx->call_cntr >= funcctx->max_calls)
    SRF_RETURN_DONE(funcctx);
  TgeoarrJoinState *state = funcctx->user_fctx;
  int k = funcctx->call_cntr;
  Datum values[2];
  bool isnull[2] = {false, false};
  /* The kernel returns 0-based indexes into the input arrays; expose them as
   * 1-based positions to match the SQL array convention */
  values[0] = Int32GetDatum(state->pairs[2 * k] + 1);
  values[1] = Int32GetDatum(state->pairs[2 * k + 1] + 1);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Tdwithin_tgeoarr_tgeoarr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_tgeoarr_tgeoarr);
/**
 * @ingroup mobilitydb_geo_rel_temp
 * @brief Return the index pairs of two arrays of temporal geos that are ever
 * within a distance together with the periods during which they are
 * @sqlfn tDwithinPairs()
 */
Datum
Tdwithin_tgeoarr_tgeoarr(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    ArrayType *array1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType *array2 = PG_GETARG_ARRAYTYPE_P(1);
    double dist = PG_GETARG_FLOAT8(2);
    int count1, count2, npairs = 0;
    Temporal **arr1 = (Temporal **) temparr_extract(array1, &count1);
    Temporal **arr2 = (Temporal **) temparr_extract(array2, &count2);
    TgeoarrJoinState *state = palloc0(sizeof(TgeoarrJoinState));
    state->pairs = tdwithin_tgeoarr_tgeoarr((const Temporal **) arr1, count1,
      (const Temporal **) arr2, count2, dist, &npairs, &state->periods);
    pfree(arr1); pfree(arr2);
    PG_FREE_IF_COPY(array1, 0); PG_FREE_IF_COPY(array2, 1);
    funcctx->user_fctx = state;
    funcctx->max_calls = npairs;
    get_call_result_type(fcinfo, NULL, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }
  funcctx = SRF_PERCALL_SETUP();
  if (funcctx->call_cntr >= funcctx->max_calls)
    SRF_RETURN_DONE(funcctx);
  TgeoarrJoinState *state = funcctx->user_fctx;
  int k = funcctx->call_cntr;
  Datum values[3];
  bool isnull[3] = {false, false, false};
  /* The kernel returns 0-based indexes into the input arrays; expose them as
   * 1-based positions to match the SQL array convention */
  values[0] = Int32GetDatum(state->pairs[2 * k] + 1);
  values[1] = Int32GetDatum(state->pairs[2 * k + 1] + 1);
  values[2] = PointerGetDatum(state->periods[k]);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

PGDLLEXPORT Datum Adisjoint_tgeoarr_tgeoarr(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tgeoarr_tgeoarr);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return the index pairs of two arrays of temporal geos that are always
 * disjoint
 * @sqlfn aDisjointPairs()
 */
Datum
Adisjoint_tgeoarr_tgeoarr(PG_FUNCTION_ARGS)
{
  FuncCallContext *funcctx;
  if (SRF_IS_FIRSTCALL())
  {
    funcctx = SRF_FIRSTCALL_INIT();
    MemoryContext oldcontext =
      MemoryContextSwitchTo(funcctx->multi_call_memory_ctx);
    ArrayType *array1 = PG_GETARG_ARRAYTYPE_P(0);
    ArrayType *array2 = PG_GETARG_ARRAYTYPE_P(1);
    int count1, count2, npairs = 0;
    Temporal **arr1 = (Temporal **) temparr_extract(array1, &count1);
    Temporal **arr2 = (Temporal **) temparr_extract(array2, &count2);
    TgeoarrJoinState *state = palloc0(sizeof(TgeoarrJoinState));
    state->pairs = adisjoint_tgeoarr_tgeoarr((const Temporal **) arr1, count1,
      (const Temporal **) arr2, count2, &npairs);
    pfree(arr1); pfree(arr2);
    PG_FREE_IF_COPY(array1, 0); PG_FREE_IF_COPY(array2, 1);
    funcctx->user_fctx = state;
    funcctx->max_calls = npairs;
    get_call_result_type(fcinfo, NULL, &funcctx->tuple_desc);
    BlessTupleDesc(funcctx->tuple_desc);
    MemoryContextSwitchTo(oldcontext);
  }
  funcctx = SRF_PERCALL_SETUP();
  if (funcctx->call_cntr >= funcctx->max_calls)
    SRF_RETURN_DONE(funcctx);
  TgeoarrJoinState *state = funcctx->user_fctx;
  int k = funcctx->call_cntr;
  Datum values[2];
  bool isnull[2] = {false, false};
  /* The kernel returns 0-based indexes into the input arrays; expose them as
   * 1-based positions to match the SQL array convention */
  values[0] = Int32GetDatum(state->pairs[2 * k] + 1);
  values[1] = Int32GetDatum(state->pairs[2 * k + 1] + 1);
  HeapTuple tuple = heap_form_tuple(funcctx->tuple_desc, values, isnull);
  SRF_RETURN_NEXT(funcctx, HeapTupleGetDatum(tuple));
}

/*****************************************************************************/
