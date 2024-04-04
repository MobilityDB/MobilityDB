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

/*
 * tpoint_tempspatialrels.sql
 * Temporal spatial relationships for temporal points.
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tContains(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDisjoint(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tgeompoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Alias for temporal not equals, that is, tpoint_tne or #<>
CREATE FUNCTION tDisjoint(tgeompoint, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tgeogpoint, tgeogpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tIntersects(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tgeompoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Alias for temporal equals, that is, tpoint_teq or #=
CREATE FUNCTION tIntersects(tgeompoint, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tgeogpoint, tgeogpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tTouches(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tgeompoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

CREATE FUNCTION tDwithin(geometry, tgeompoint, dist float,
   atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tDwithin(tgeompoint, geometry, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tDwithin(tgeompoint, tgeompoint, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;

/*****************************************************************************/
