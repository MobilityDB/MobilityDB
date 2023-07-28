/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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

-- ALL the following functions are not STRICT
CREATE FUNCTION tcontains(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tcontains_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tdisjoint(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Alias for temporal not equals, that is, tpoint_tne or #<>
CREATE FUNCTION tdisjoint(tgeompoint, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeogpoint, tgeogpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tne_temporal_temporal'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION tintersects(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_geo_tpoint'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tintersects_tpoint_geo'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
-- Alias for temporal equals, that is, tpoint_teq or #=
CREATE FUNCTION tintersects(tgeompoint, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, tgeogpoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Teq_temporal_temporal'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * ttouches
 *****************************************************************************/

-- ALL the following functions are not STRICT
CREATE FUNCTION ttouches(geometry, tgeompoint, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_geo_tpoint'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, geometry, atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Ttouches_tpoint_geo'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;

/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tgeompoint, dist float,
   atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, geometry, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, tgeompoint, dist float,
    atvalue bool DEFAULT NULL)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'Tdwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE  PARALLEL SAFE;

/*****************************************************************************/
