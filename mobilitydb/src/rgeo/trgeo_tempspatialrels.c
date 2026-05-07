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
 * @brief Temporal spatial relationships for temporal rigid geometries
 * @details These relationships are applied at each instant and result in a
 * temporal Boolean.
 *
 * The following relationships are supported: `tContains`, `tCovers`,
 * `tDisjoint`, `tIntersects`, `tTouches`, and `tDwithin`.
 */

#include "rgeo/trgeo_tempspatialrels.h"

/* MEOS */
#include <meos.h>
#include <meos_rgeo.h>
/* MobilityDB */
#include "pg_geo/postgis.h"
#include "pg_geo/tspatial.h"

/*****************************************************************************
 * Temporal contains
 *****************************************************************************/

PGDLLEXPORT Datum Tcontains_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry contains a
 * temporal rigid geometry at each instant
 * @sqlfn tContains()
 */
Datum
Tcontains_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tcontains_geo_trgeo);
}

PGDLLEXPORT Datum Tcontains_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry contains a geometry at each instant
 * @sqlfn tContains()
 */
Datum
Tcontains_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tcontains_trgeo_geo);
}

PGDLLEXPORT Datum Tcontains_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcontains_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether the first temporal
 * rigid geometry contains the second at each synchronized instant
 * @sqlfn tContains()
 */
Datum
Tcontains_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tcontains_trgeo_trgeo);
}

/*****************************************************************************
 * Temporal covers
 *****************************************************************************/

PGDLLEXPORT Datum Tcovers_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry covers a
 * temporal rigid geometry at each instant
 * @sqlfn tCovers()
 */
Datum
Tcovers_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tcovers_geo_trgeo);
}

PGDLLEXPORT Datum Tcovers_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry covers a geometry at each instant
 * @sqlfn tCovers()
 */
Datum
Tcovers_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tcovers_trgeo_geo);
}

PGDLLEXPORT Datum Tcovers_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tcovers_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether the first temporal
 * rigid geometry covers the second at each synchronized instant
 * @sqlfn tCovers()
 */
Datum
Tcovers_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tcovers_trgeo_trgeo);
}

/*****************************************************************************
 * Temporal disjoint
 *****************************************************************************/

PGDLLEXPORT Datum Tdisjoint_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry is disjoint
 * from a temporal rigid geometry at each instant
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tdisjoint_geo_trgeo);
}

PGDLLEXPORT Datum Tdisjoint_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry is disjoint from a geometry at each instant
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tdisjoint_trgeo_geo);
}

PGDLLEXPORT Datum Tdisjoint_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdisjoint_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries are disjoint at each synchronized instant
 * @sqlfn tDisjoint()
 */
Datum
Tdisjoint_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tdisjoint_trgeo_trgeo);
}

/*****************************************************************************
 * Temporal intersects
 *****************************************************************************/

PGDLLEXPORT Datum Tintersects_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry intersects a
 * temporal rigid geometry at each instant
 * @sqlfn tIntersects()
 */
Datum
Tintersects_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &tintersects_geo_trgeo);
}

PGDLLEXPORT Datum Tintersects_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry intersects a geometry at each instant
 * @sqlfn tIntersects()
 */
Datum
Tintersects_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &tintersects_trgeo_geo);
}

PGDLLEXPORT Datum Tintersects_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tintersects_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries intersect at each synchronized instant
 * @sqlfn tIntersects()
 */
Datum
Tintersects_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &tintersects_trgeo_trgeo);
}

/*****************************************************************************
 * Temporal touches
 *****************************************************************************/

PGDLLEXPORT Datum Ttouches_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry touches a
 * temporal rigid geometry at each instant
 * @sqlfn tTouches()
 */
Datum
Ttouches_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_geo_tspatial(fcinfo, &ttouches_geo_trgeo);
}

PGDLLEXPORT Datum Ttouches_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry touches a geometry at each instant
 * @sqlfn tTouches()
 */
Datum
Ttouches_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_geo(fcinfo, &ttouches_trgeo_geo);
}

PGDLLEXPORT Datum Ttouches_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Ttouches_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries touch at each synchronized instant
 * @sqlfn tTouches()
 */
Datum
Ttouches_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tspatialrel_tspatial_tspatial(fcinfo, &ttouches_trgeo_trgeo);
}

/*****************************************************************************
 * Temporal dwithin
 *****************************************************************************/

PGDLLEXPORT Datum Tdwithin_geo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_geo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a geometry and a
 * temporal rigid geometry are within a given distance at each instant
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_geo_trgeo(PG_FUNCTION_ARGS)
{
  return Tdwithin_geo_tspatial(fcinfo, &tdwithin_geo_trgeo);
}

PGDLLEXPORT Datum Tdwithin_trgeo_geo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_trgeo_geo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether a temporal rigid
 * geometry and a geometry are within a given distance at each instant
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_trgeo_geo(PG_FUNCTION_ARGS)
{
  return Tdwithin_tspatial_geo(fcinfo, &tdwithin_trgeo_geo);
}

PGDLLEXPORT Datum Tdwithin_trgeo_trgeo(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(Tdwithin_trgeo_trgeo);
/**
 * @ingroup mobilitydb_rgeo_rel_temp
 * @brief Return a temporal Boolean that states whether two temporal rigid
 * geometries are within a given distance at each synchronized instant
 * @sqlfn tDwithin()
 */
Datum
Tdwithin_trgeo_trgeo(PG_FUNCTION_ARGS)
{
  return Tdwithin_tspatial_tspatial(fcinfo, &tdwithin_trgeo_trgeo);
}

/*****************************************************************************/
