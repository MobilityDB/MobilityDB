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
 * @brief Temporal spatial relationships for temporal geometries/geographies
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

CREATE FUNCTION tContains(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

CREATE FUNCTION tDisjoint(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- Alias for temporal not equals, that is, tgeo_tne or #<>
CREATE FUNCTION tDisjoint(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

CREATE FUNCTION tIntersects(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- Alias for temporal equals, that is, tgeo_teq or #=
CREATE FUNCTION tIntersects(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tIntersects(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

CREATE FUNCTION tTouches(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tTouches(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

CREATE FUNCTION tDwithin(geometry, tgeompoint, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDwithin(tgeompoint, geometry, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDwithin(tgeompoint, tgeompoint, dist float)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
