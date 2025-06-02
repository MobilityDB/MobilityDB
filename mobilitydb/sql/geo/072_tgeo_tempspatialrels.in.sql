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
 * @brief Temporal spatial relationships for temporal geometries/geographies
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tContains(geometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tContains(tgeometry, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tgeo_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tContains(tgeometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tContains
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tCovers(geometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_geo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCovers(tgeometry, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tgeo_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tCovers(tgeometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcovers_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDisjoint(geometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tgeometry, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tgeo_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Alias for temporal not equals, that is, geo_tne or #<>
CREATE FUNCTION tDisjoint(tgeometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tgeography, tgeography, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tIntersects(geometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tgeometry, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tgeo_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Alias for temporal equals, that is, tgeo_teq or #=
CREATE FUNCTION tIntersects(tgeometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tgeography, tgeography, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tTouches(geometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tgeometry, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tgeo_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tgeometry, tgeometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDwithin(geometry, tgeometry, dist float,
   atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(tgeometry, geometry, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tgeo_geo'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tDwithin(tgeometry, tgeometry, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tgeo_tgeo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************/
