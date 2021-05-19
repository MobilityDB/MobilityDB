/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * Spatial relationships for temporal points.
 */

/*****************************************************************************
 * tcontains
 *****************************************************************************/

CREATE FUNCTION tcontains(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tcontains_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcontains(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tcontains_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcovers
 *****************************************************************************/

CREATE FUNCTION tcovers(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tcovers_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tcovers_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcoveredby
 *****************************************************************************/

CREATE FUNCTION tcoveredby(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tcoveredby_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

CREATE FUNCTION tdisjoint(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdisjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdisjoint(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tequals
 *****************************************************************************/

CREATE FUNCTION tequals(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tequals_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tequals_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tequals_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tequals(geography, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tequals_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeogpoint, geography)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tequals_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tequals_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

CREATE FUNCTION tintersects(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tintersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tintersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tintersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tintersects(tgeogpoint, tgeogpoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tintersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * ttouches
 *****************************************************************************/

CREATE FUNCTION ttouches(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'ttouches_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'ttouches_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * twithin
 *****************************************************************************/

CREATE FUNCTION twithin(geometry, tgeompoint)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'twithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twithin(tgeompoint, geometry)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'twithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tgeompoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, geometry, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, tgeompoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tdwithin(tgeogpoint, tgeogpoint, dist float8)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'tdwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * trelate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tgeompoint)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'trelate_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, geometry)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'trelate_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, tgeompoint)
  RETURNS ttext
  AS 'MODULE_PATHNAME', 'trelate_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * trelate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tgeompoint, pattern text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'trelate_pattern_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, geometry, pattern text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'trelate_pattern_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, tgeompoint, pattern text)
  RETURNS tbool
  AS 'MODULE_PATHNAME', 'trelate_pattern_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
