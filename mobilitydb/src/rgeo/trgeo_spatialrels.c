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
 * @brief Ever and always spatial relationships for temporal rigid geometries
 * @details These relationships compute the ever/always spatial relationship
 * between the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding spatiotemporal
 * relationship.
 *
 * The following relationships are supported for geometries: `eContains`,
 * `aContains`, `eDisjoint`, `aDisjoint`, `eIntersects`, `aIntersects`,
 * `eTouches`, aTouches`, `eDwithin`, and `aDwithin`.
 *
 * Only `eDisjoint`, `eDwithin`, and `eIntersects` are supported for 3D
 * geometries.
 */

#include "geo/tgeo_spatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_internal.h>
#include <meos_geo.h>
#include "temporal/temporal.h" /* For varfunc */
#include "geo/tgeo_spatialfuncs.h"
#include "rgeo/trgeo_spatialrels.h"
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

PGDLLEXPORT Datum Econtains_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry ever contains a temporal rigid geometry
 * @sqlfn eContains()
 */
inline Datum
Econtains_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_contains_geo_trgeo, EVER);
}

PGDLLEXPORT Datum Acontains_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry always contains a temporal rigid geometry
 * @sqlfn aContains()
 */
inline Datum
Acontains_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_contains_geo_trgeo, ALWAYS);
}

/*****************************************************************************
 * Ever/always covers
 *****************************************************************************/

PGDLLEXPORT Datum Ecovers_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry ever covers a temporal rigid geometry
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_covers_geo_trgeo, EVER);
}

PGDLLEXPORT Datum Acovers_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry always covers a temporal rigid geometry
 * @sqlfn aCovers()
 */
inline Datum
Acovers_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_covers_geo_trgeo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Ecovers_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry ever covers a temporal rigid geometry
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_covers_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Acovers_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry always covers a temporal rigid geometry
 * @sqlfn aCovers()
 */
inline Datum
Acovers_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_covers_trgeo_geo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Econtains_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry ever contains a geometry
 * @sqlfn eContains()
 */
inline Datum
Econtains_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_contains_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Acontains_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry always contains a geometry
 * @sqlfn aContains()
 */
inline Datum
Acontains_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_contains_trgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Econtains_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry ever contains another one
 * @sqlfn eContains()
 */
inline Datum
Econtains_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_contains_trgeo_trgeo,
    EVER);
}

PGDLLEXPORT Datum Acontains_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry always contains another one
 * @sqlfn aContains()
 */
inline Datum
Acontains_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_contains_trgeo_trgeo,
    ALWAYS);
}

PGDLLEXPORT Datum Ecovers_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ecovers_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry ever covers another one
 * @sqlfn eCovers()
 */
inline Datum
Ecovers_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_covers_trgeo_trgeo, EVER);
}

PGDLLEXPORT Datum Acovers_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acovers_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry always covers another one
 * @sqlfn aCovers()
 */
inline Datum
Acovers_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_covers_trgeo_trgeo,
    ALWAYS);
}

/*****************************************************************************
 * Ever disjoint (for both geometry and geography)
 *****************************************************************************/

PGDLLEXPORT Datum Edisjoint_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_disjoint_geo_trgeo, EVER);
}

PGDLLEXPORT Datum Adisjoint_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_disjoint_geo_trgeo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Adisjoint_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry are always
 * disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_disjoint_trgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal rigid geometries are ever disjoint
 * @sqlfn eDisjoint()
 */
inline Datum
Edisjoint_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_disjoint_trgeo_trgeo,
    EVER);
}

PGDLLEXPORT Datum Adisjoint_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal rigid geometries are ever disjoint
 * @sqlfn aDisjoint()
 */
inline Datum
Adisjoint_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_disjoint_trgeo_trgeo,
    ALWAYS);
}

/*****************************************************************************
 * Ever intersects (for both geometry and geography)
 *****************************************************************************/

PGDLLEXPORT Datum Eintersects_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_intersects_geo_trgeo, EVER);
}

PGDLLEXPORT Datum Aintersects_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry ever
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_intersects_geo_trgeo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry ever
 * intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_intersects_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Aintersects_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry always
 * intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_intersects_trgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal rigid geometries ever intersect
 * @sqlfn eIntersects()
 */
inline Datum
Eintersects_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_intersects_trgeo_trgeo,
    EVER);
}

PGDLLEXPORT Datum Aintersects_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal rigid geometries ever intersect
 * @sqlfn aIntersects()
 */
inline Datum
Aintersects_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_intersects_trgeo_trgeo,
    ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 * The function does not accept geography since it is based on the PostGIS
 * ST_Boundary function
 *****************************************************************************/

PGDLLEXPORT Datum Etouches_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_touches_geo_trgeo, EVER);
}

PGDLLEXPORT Datum Atouches_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry ever touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_geo_tspatial(fcinfo, &ea_touches_geo_trgeo, ALWAYS);
}

PGDLLEXPORT Datum Etouches_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_touches_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Atouches_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_geo(fcinfo, &ea_touches_trgeo_geo, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Etouches_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal rigid geometries ever touch
 * @sqlfn eTouches()
 */
inline Datum
Etouches_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_touches_trgeo_trgeo,
    EVER);
}

PGDLLEXPORT Datum Atouches_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if two temporal rigid geometries always touch
 * @sqlfn aTouches()
 */
inline Datum
Atouches_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_spatialrel_tspatial_tspatial(fcinfo, &ea_touches_trgeo_trgeo,
    ALWAYS);
}

/*****************************************************************************
 * Ever/always dwithin (for both geometry and geography)
 * The function only accepts points and not arbitrary geometries/geographies
 *****************************************************************************/

PGDLLEXPORT Datum Edwithin_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tspatial(fcinfo, &ea_dwithin_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Adwithin_geo_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_geo_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a geometry and a temporal rigid geometry are always
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_geo_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_dwithin_geo_tspatial(fcinfo, &ea_dwithin_trgeo_geo, ALWAYS);
}

PGDLLEXPORT Datum Edwithin_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry are ever
 * within a distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tspatial_geo(fcinfo, &ea_dwithin_trgeo_geo, EVER);
}

PGDLLEXPORT Datum Adwithin_trgeometry_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_trgeometry_geo);
/**
 * @ingroup mobilitydb_geo_rel_ever
 * @brief Return true if a temporal rigid geometry and a geometry are always
 * within a distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_trgeometry_geo(PG_FUNCTION_ARGS)
{
  return EA_dwithin_tspatial_geo(fcinfo, &ea_dwithin_trgeo_geo, ALWAYS);
}

/**
 * @brief Return true if two temporal rigid geometries are even/always within a
 * distance
 * @sqlfn eDwithin(), aDwithin()
 */
Datum
EA_dwithin_trgeometry_trgeometry(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ea_dwithin_trgeo_trgeo(temp1, temp2, dist, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result);
}

PGDLLEXPORT Datum Edwithin_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel
 * @brief Return true if two temporal rigid geometries are ever within a
 * distance
 * @sqlfn eDwithin()
 */
inline Datum
Edwithin_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_dwithin_trgeometry_trgeometry(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_trgeometry_trgeometry(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_trgeometry_trgeometry);
/**
 * @ingroup mobilitydb_geo_rel
 * @brief Return true if two temporal rigid geometries are always within a
 * distance
 * @sqlfn aDwithin()
 */
inline Datum
Adwithin_trgeometry_trgeometry(PG_FUNCTION_ARGS)
{
  return EA_dwithin_trgeometry_trgeometry(fcinfo, ALWAYS);
}

/*****************************************************************************/
