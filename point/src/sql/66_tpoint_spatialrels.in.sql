/*****************************************************************************
 *
 * tpoint_spatialrels.sql
 * Spatial relationships for temporal points.
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2020, Université libre de Bruxelles and MobilityDB
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

/*****************************************************************************
 * contains
 *****************************************************************************/

CREATE FUNCTION _contains(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _contains(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION contains(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'contains_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * containsproperly
 *****************************************************************************/

CREATE FUNCTION _containsproperly(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'containsproperly_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _containsproperly(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'containsproperly_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION containsproperly(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'containsproperly_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * covers
 *****************************************************************************/

CREATE FUNCTION _covers(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'covers_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _covers(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'covers_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION covers(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'covers_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION _covers(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'covers_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _covers(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'covers_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION covers(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'covers_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * coveredby
 *****************************************************************************/

CREATE FUNCTION _coveredby(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'coveredby_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _coveredby(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'coveredby_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION coveredby(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'coveredby_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION _coveredby(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'coveredby_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _coveredby(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'coveredby_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION coveredby(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'coveredby_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * crosses
 *****************************************************************************/

CREATE FUNCTION _crosses(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'crosses_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _crosses(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'crosses_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION crosses(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'crosses_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * disjoint
 *****************************************************************************/

CREATE FUNCTION disjoint(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'disjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'disjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'disjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * equals
 *****************************************************************************/

CREATE FUNCTION _equals(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'equals_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _equals(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'equals_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION equals(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'equals_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * intersects
 *****************************************************************************/

CREATE FUNCTION _intersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'intersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'intersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION intersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'intersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION _intersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'intersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'intersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION intersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'intersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * overlaps
 *****************************************************************************/

CREATE FUNCTION _overlaps(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _overlaps(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION overlaps(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'overlaps_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * touches
 *****************************************************************************/

CREATE FUNCTION _touches(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'touches_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _touches(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'touches_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION touches(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'touches_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * within
 *****************************************************************************/

CREATE FUNCTION _within(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'within_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _within(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'within_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION within(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'within_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * dwithin
 *****************************************************************************/

CREATE FUNCTION _dwithin(geometry, tgeompoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'dwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geometry, tgeompoint, dist float8)
  RETURNS boolean
  AS 'SELECT @extschema@.ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2 AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _dwithin(tgeompoint, geometry, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'dwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeompoint, geometry, dist float8)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.ST_Expand($2,$3)  AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION dwithin(tgeompoint, tgeompoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'dwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION _dwithin(geography, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'dwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geography, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'SELECT @extschema@._ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2 AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _dwithin(tgeogpoint, geography, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'dwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, geography, dist float8)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@._ST_Expand($2,$3) AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION dwithin(tgeogpoint, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'dwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * relate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION relate(geometry, tgeompoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'relate_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, geometry)
  RETURNS text
  AS 'MODULE_PATHNAME', 'relate_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, tgeompoint)
  RETURNS text
  AS 'MODULE_PATHNAME', 'relate_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * relate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION relate(geometry, tgeompoint, pattern text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'relate_pattern_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, geometry, pattern text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'relate_pattern_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, tgeompoint, pattern text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'relate_pattern_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
