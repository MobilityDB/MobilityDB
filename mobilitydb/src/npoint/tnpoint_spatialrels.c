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
 * @brief Ever/always spatial relationships for temporal network points.
 *
 * These relationships compute the ever/always spatial relationship between
 * the arguments and return a Boolean. These functions may be used for
 * filtering purposes before applying the corresponding temporal spatial
 * relationship.
 *
 * The following relationships are supported:
 * eContains, aContains, eDisjoint, aDisjoint, eIntersects, aIntersects,
 * eTouches, aTouches, eDwithin, and aDwithin
 */

#include "npoint/tnpoint_spatialrels.h"

/* PostgreSQL */
#include <postgres.h>
#include <fmgr.h>
/* MEOS */
#include <meos.h>
#include "point/tpoint_spatialfuncs.h"
#include "point/tpoint_spatialrels.h"
/* MobilityDB */
#include "pg_point/postgis.h"
#include "pg_point/tpoint_spatialfuncs.h"

/*****************************************************************************
 * Generic ever/always relationship
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal network point ever/always
 * intersect
 */
static Datum
EAspatialrel_geo_tnpoint(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const GSERIALIZED *, bool), bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(temp, gs, ever);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a geometry and a temporal network point ever/always
 * intersect
 */
static Datum
EAspatialrel_tnpoint_geo(FunctionCallInfo fcinfo,
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
 * @brief Return true if a geometry and a temporal network point ever/always
 * intersect
 */
static Datum
EAspatialrel_npoint_tnpoint(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Npoint *, bool), bool ever)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = func(temp, np, ever);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a geometry and a temporal network point ever/always
 * intersect
 */
static Datum
EAspatialrel_tnpoint_npoint(FunctionCallInfo fcinfo,
  int (*func)(const Temporal *, const Npoint *, bool), bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  int result = func(temp, np, ever);
  PG_FREE_IF_COPY(temp, 0);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/*****************************************************************************/

/**
 * @brief Return true if the temporal network points ever/always satisfy the
 * spatial relationship
 * @param[in] fcinfo Catalog information about the external function
 * @param[in] func Spatial relationship
 * @param[in] ever True to compute the ever semantics, false for always
 */
static Datum
EAspatialrel_tnpoint_tnpoint(FunctionCallInfo fcinfo,
  Datum (*func)(Datum, Datum), bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  int result = ea_spatialrel_tnpoint_tnpoint(temp1, temp2, func, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_BOOL(result == 1 ? true : false);
}

/*****************************************************************************
 * Ever/always contains
 *****************************************************************************/

/**
 * @brief Return true if a geometry ever/always contains a temporal network
 * point
 */
static Datum
EAcontains_geo_tnpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  int result = ea_contains_geo_tnpoint(gs, temp, ever);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

PGDLLEXPORT Datum Econtains_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Econtains_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry ever contains a temporal network point
 * @sqlfn eContains()
 */
Datum
Econtains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcontains_geo_tnpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Acontains_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Acontains_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry always contains a temporal network point
 * @sqlfn aContains()
 */
Datum
Acontains_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAcontains_geo_tnpoint(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever/always disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Edisjoint_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_geo_tnpoint(fcinfo, &ea_disjoint_tnpoint_geo, EVER);
}

PGDLLEXPORT Datum Adisjoint_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point are always
 * disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_geo_tnpoint(fcinfo, &ea_disjoint_tnpoint_geo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal network point are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_npoint_tnpoint(fcinfo, &ea_disjoint_tnpoint_npoint, EVER);
}

PGDLLEXPORT Datum Adisjoint_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal network point are
 * always disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_npoint_tnpoint(fcinfo, &ea_disjoint_tnpoint_npoint, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a geometry are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_geo(fcinfo, &ea_disjoint_tnpoint_geo, EVER);
}

PGDLLEXPORT Datum Adisjoint_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a geometry are always
 * disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_geo(fcinfo, &ea_disjoint_tnpoint_geo, ALWAYS);
}

PGDLLEXPORT Datum Edisjoint_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a network point are ever
 * disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_npoint(fcinfo, &ea_disjoint_tnpoint_npoint, EVER);
}

PGDLLEXPORT Datum Adisjoint_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a network point are
 * always disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_npoint(fcinfo, &ea_disjoint_tnpoint_npoint, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Edisjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edisjoint_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal network points are ever disjoint
 * @sqlfn eDisjoint()
 */
Datum
Edisjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_tnpoint(fcinfo, &datum2_point_ne, EVER);
}

PGDLLEXPORT Datum Adisjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adisjoint_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal network points are always disjoint
 * @sqlfn aDisjoint()
 */
Datum
Adisjoint_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_tnpoint(fcinfo, &datum2_point_ne, ALWAYS);
}

/*****************************************************************************
 * Ever/always intersects
 *****************************************************************************/

PGDLLEXPORT Datum Eintersects_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network
 * point ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_geo_tnpoint(fcinfo, &ea_intersects_tnpoint_geo, EVER);
}

PGDLLEXPORT Datum Aintersects_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network
 * point always intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_geo_tnpoint(fcinfo, &ea_intersects_tnpoint_geo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal
 * network point ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_npoint_tnpoint(fcinfo, &ea_intersects_tnpoint_npoint, EVER);
}

PGDLLEXPORT Datum Aintersects_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal
 * network point always intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_npoint_tnpoint(fcinfo, &ea_intersects_tnpoint_npoint, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a
 * geometry ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_geo(fcinfo, &ea_intersects_tnpoint_geo, EVER);
}

PGDLLEXPORT Datum Aintersects_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a
 * geometry always intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_geo(fcinfo, &ea_intersects_tnpoint_geo, ALWAYS);
}

PGDLLEXPORT Datum Eintersects_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a
 * network point ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_npoint(fcinfo, &ea_intersects_tnpoint_npoint, EVER);
}

PGDLLEXPORT Datum Aintersects_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a
 * network point always intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_npoint(fcinfo, &ea_intersects_tnpoint_npoint, ALWAYS);
}

/*****************************************************************************/

PGDLLEXPORT Datum Eintersects_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Eintersects_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal network points ever intersect
 * @sqlfn eIntersects()
 */
Datum
Eintersects_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_tnpoint(fcinfo, &datum2_point_eq, EVER);
}

PGDLLEXPORT Datum Aintersects_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Aintersects_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal network points always intersect
 * @sqlfn aIntersects()
 */
Datum
Aintersects_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_tnpoint(fcinfo, &datum2_point_eq, ALWAYS);
}

/*****************************************************************************
 * Ever/always dwithin
 *****************************************************************************/

/**
 * @brief Return true if a geometry and a temporal network  point are
 * ever/always within a distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EAdwithin_geo_tnpoint(FunctionCallInfo fcinfo, bool ever)
{
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ea_dwithin_tnpoint_geom(temp, gs, dist, ever);
  PG_FREE_IF_COPY(gs, 0);
  PG_FREE_IF_COPY(temp, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a network point and a temporal network point are
 * ever/always within a distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EAdwithin_npoint_tnpoint(FunctionCallInfo fcinfo, bool ever)
{
  Npoint *np = PG_GETARG_NPOINT_P(0);
  Temporal *temp = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Datum result = ea_dwithin_tnpoint_npoint(temp, np, dist, ever);
  PG_FREE_IF_COPY(temp, 1);
  PG_RETURN_DATUM(result);
}

/**
 * @brief Return true if a temporal network point and a geometry are
 * ever/always within a distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EAdwithin_tnpoint_geo(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  GSERIALIZED *gs = PG_GETARG_GSERIALIZED_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = DatumGetInt32(ea_dwithin_tnpoint_geom(temp, gs, dist, ever));
  PG_FREE_IF_COPY(temp, 0);
  PG_FREE_IF_COPY(gs, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_INT32(result);
}

/**
 * @brief Return true if a temporal network point and a network point are
 * ever/always within a distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EAdwithin_tnpoint_npoint(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp = PG_GETARG_TEMPORAL_P(0);
  Npoint *np = PG_GETARG_NPOINT_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  Datum result = ea_dwithin_tnpoint_npoint(temp, np, dist, ever);
  PG_FREE_IF_COPY(temp, 0);
  PG_RETURN_DATUM(result);
}

/**
 * @brief Return true if two temporal network points are ever/always within a
 * distance
 * @sqlfn eDwithin(), aDwithin()
 */
static Datum
EAdwithin_tnpoint_tnpoint(FunctionCallInfo fcinfo, bool ever)
{
  Temporal *temp1 = PG_GETARG_TEMPORAL_P(0);
  Temporal *temp2 = PG_GETARG_TEMPORAL_P(1);
  double dist = PG_GETARG_FLOAT8(2);
  int result = ea_dwithin_tnpoint_tnpoint(temp1, temp2, dist, ever);
  PG_FREE_IF_COPY(temp1, 0);
  PG_FREE_IF_COPY(temp2, 1);
  if (result < 0)
    PG_RETURN_NULL();
  PG_RETURN_FLOAT8(result);
}

/*****************************************************************************/

PGDLLEXPORT Datum Edwithin_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point are ever
 * within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_geo_tnpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and a temporal network point are always
 * within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_geo_tnpoint(fcinfo, ALWAYS);
}

PGDLLEXPORT Datum Edwithin_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal network point are ever
 * within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_npoint_tnpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and a temporal network point are
 * always within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_npoint_tnpoint(fcinfo, ALWAYS);
}

PGDLLEXPORT Datum Edwithin_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a geometry are ever
 * within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAdwithin_tnpoint_geo(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a geometry are always
 * within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAdwithin_tnpoint_geo(fcinfo, ALWAYS);
}

PGDLLEXPORT Datum Edwithin_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a network point are ever
 * within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_tnpoint_npoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a temporal network point and a
 * network point are always within a distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_tnpoint_npoint(fcinfo, ALWAYS);
}

PGDLLEXPORT Datum Edwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Edwithin_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal network points are ever within a distance
 * @sqlfn eDwithin()
 */
Datum
Edwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_tnpoint_tnpoint(fcinfo, EVER);
}

PGDLLEXPORT Datum Adwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Adwithin_tnpoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if two temporal network points are always within a
 * distance
 * @sqlfn aDwithin()
 */
Datum
Adwithin_tnpoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAdwithin_tnpoint_tnpoint(fcinfo, ALWAYS);
}

/*****************************************************************************
 * Ever/always touches
 *****************************************************************************/

PGDLLEXPORT Datum Etouches_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and the trajectory of a temporal network
 * point ever intersect
 * @sqlfn eTouches()
 */
Datum
Etouches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_geo_tnpoint(fcinfo, &ea_touches_tnpoint_geo, EVER);
}

PGDLLEXPORT Datum Atouches_geo_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_geo_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a geometry and the trajectory of a temporal network
 * point always intersect
 * @sqlfn aTouches()
 */
Datum
Atouches_geo_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_geo_tnpoint(fcinfo, &ea_touches_tnpoint_geo, ALWAYS);
}

PGDLLEXPORT Datum Etouches_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and the trajectory of a temporal
 * network point ever intersect
 * @sqlfn eTouches()
 */
Datum
Etouches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_npoint_tnpoint(fcinfo, &ea_touches_tnpoint_npoint, EVER);
}

PGDLLEXPORT Datum Atouches_npoint_tnpoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_npoint_tnpoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if a network point and the trajectory of a temporal
 * network point always intersect
 * @sqlfn aTouches()
 */
Datum
Atouches_npoint_tnpoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_npoint_tnpoint(fcinfo, &ea_touches_tnpoint_npoint, ALWAYS);
}

PGDLLEXPORT Datum Etouches_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if the trajectory of a temporal network point and a
 * geometry ever intersect
 * @sqlfn eTouches()
 */
Datum
Etouches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_geo(fcinfo, &ea_touches_tnpoint_geo, EVER);
}

PGDLLEXPORT Datum Atouches_tnpoint_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tnpoint_geo);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if the trajectory of a temporal network point and a
 * geometry always intersect
 * @sqlfn aTouches()
 */
Datum
Atouches_tnpoint_geo(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_geo(fcinfo, &ea_touches_tnpoint_geo, ALWAYS);
}

PGDLLEXPORT Datum Etouches_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Etouches_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if the trajectory of a temporal network point and a
 * network point ever intersect
 * @sqlfn eTouches()
 */
Datum
Etouches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_npoint(fcinfo, &ea_touches_tnpoint_npoint, EVER);
}

PGDLLEXPORT Datum Atouches_tnpoint_npoint(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Atouches_tnpoint_npoint);
/**
 * @ingroup mobilitydb_temporal_spatial_rel_ever
 * @brief Return true if the trajectory of a temporal network point and a
 * network point always intersect
 * @sqlfn aTouches()
 */
Datum
Atouches_tnpoint_npoint(PG_FUNCTION_ARGS)
{
  return EAspatialrel_tnpoint_npoint(fcinfo, &ea_touches_tnpoint_npoint,
    ALWAYS);
}

/*****************************************************************************/
