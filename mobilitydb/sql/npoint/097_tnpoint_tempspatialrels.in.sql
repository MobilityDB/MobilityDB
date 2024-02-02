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
 * tnpoint_tempspatialrels.sql
 * Spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * Boolean. The following relationships are supported:
 *    tContains, tDisjoint, tIntersects, tTouches, and tDwithin
 */

/*****************************************************************************
 * tContains
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tContains(geometry, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDisjoint
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDisjoint(geometry, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(npoint, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_npoint_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tnpoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tnpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDisjoint(tnpoint, npoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tnpoint_npoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tIntersects
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tIntersects(geometry, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(npoint, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_npoint_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tnpoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tnpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tIntersects(tnpoint, npoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tnpoint_npoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tTouches
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tTouches(geometry, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(npoint, tnpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_npoint_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tnpoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tnpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tTouches(tnpoint, npoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tnpoint_npoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tDwithin
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tDwithin(geometry, tnpoint, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(npoint, tnpoint, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_npoint_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(tnpoint, geometry, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(tnpoint, npoint, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_npoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tDwithin(tnpoint, tnpoint, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************/
