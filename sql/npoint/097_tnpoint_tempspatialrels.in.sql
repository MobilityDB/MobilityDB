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

/**
 * tnpoint_tempspatialrels.sql
 * Spatial relationships for temporal network points.
 *
 * These relationships are applied at each instant and result in a temporal
 * Boolean. The following relationships are supported:
 *    tcontains, tdisjoint, tintersects, ttouches, and tdwithin
 */

/*****************************************************************************
 * tcontains
 *****************************************************************************/

CREATE FUNCTION tcontains(geometry, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tcontains(geometry, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

CREATE FUNCTION tdisjoint(geometry, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(npoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tnpoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tnpoint, npoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdisjoint(geometry, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(npoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tnpoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tnpoint, npoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

CREATE FUNCTION tintersects(geometry, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(npoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tnpoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tnpoint, npoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tintersects(geometry, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(npoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tnpoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tnpoint, npoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * ttouches
 *****************************************************************************/

CREATE FUNCTION ttouches(geometry, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(npoint, tnpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tnpoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tnpoint, npoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttouches(geometry, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(npoint, tnpoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tnpoint, geometry, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tnpoint, npoint, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tnpoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(npoint, tnpoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, geometry, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, npoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, tnpoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdwithin(geometry, tnpoint, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(npoint, tnpoint, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, geometry, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, npoint, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tnpoint, tnpoint, dist float8, atvalue bool)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
