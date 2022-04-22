/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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
 * tcontains
 *****************************************************************************/

CREATE FUNCTION tcontains(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tcontains(geometry, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

CREATE FUNCTION tdisjoint(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- Alias for temporal not equals, that is, tpoint_tne or #<>
CREATE FUNCTION tdisjoint(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdisjoint(geometry, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- Alias for temporal not equals, that is, tpoint_tne or #<>
CREATE FUNCTION tdisjoint(tgeompoint, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeogpoint, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

CREATE FUNCTION tintersects(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- Alias for temporal equals, that is, tpoint_teq or #=
CREATE FUNCTION tintersects(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tintersects(geometry, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- Alias for temporal equals, that is, tpoint_teq or #=
CREATE FUNCTION tintersects(tgeompoint, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, tgeogpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * ttouches
 *****************************************************************************/

CREATE FUNCTION ttouches(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttouches(geometry, tgeompoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tgeompoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, geometry, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, tgeompoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdwithin(geometry, tgeompoint, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, geometry, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, tgeompoint, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
