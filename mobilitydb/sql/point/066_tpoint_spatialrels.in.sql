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
 * tpoint_spatialrels.sql
 * Spatial relationships for temporal points.
 * Depending on PostgreSQL version, index support for these functions is
 * enabled with rewriting (version < 12) or support functions (version >= 12)
 */

/*****************************************************************************
 * contains
 *****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _contains(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._contains($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER < 120000

#if POSTGRESQL_VERSION_NUMBER >= 120000
CREATE FUNCTION contains(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************
 * disjoint
 *****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

-- #if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _disjoint(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Disjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._disjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _disjoint(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Disjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._disjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _disjoint(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Disjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._disjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
-- #endif //POSTGRESQL_VERSION_NUMBER < 120000

-- #if POSTGRESQL_VERSION_NUMBER >= 120000
-- CREATE FUNCTION disjoint(geometry, tgeompoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Disjoint_geo_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION disjoint(tgeompoint, geometry)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Disjoint_tpoint_geo'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION disjoint(tgeompoint, tgeompoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Disjoint_tpoint_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- #endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

-- #if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _disjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Disjoint_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._disjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _disjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Disjoint_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._disjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _disjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Disjoint_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT NOT($1 OPERATOR(@extschema@.&&) $2) OR @extschema@._disjoint($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
-- #endif //POSTGRESQL_VERSION_NUMBER < 120000

-- #if POSTGRESQL_VERSION_NUMBER >= 120000
-- CREATE FUNCTION disjoint(geography, tgeogpoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Disjoint_geo_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION disjoint(tgeogpoint, geography)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Disjoint_tpoint_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION disjoint(tgeogpoint, tgeogpoint)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Disjoint_tpoint_tpoint'
  -- SUPPORT tpoint_supportfn
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- #endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************
 * intersects
 *****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _intersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER < 120000

#if POSTGRESQL_VERSION_NUMBER >= 120000
CREATE FUNCTION intersects(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeompoint, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _intersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER < 120000

#if POSTGRESQL_VERSION_NUMBER >= 120000
CREATE FUNCTION intersects(geography, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, geography)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, tgeogpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intersects_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************
 * touches
 *****************************************************************************/

#if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _touches(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Touches_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(geometry, tgeompoint)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _touches(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Touches_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(tgeompoint, geometry)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER < 120000

#if POSTGRESQL_VERSION_NUMBER >= 120000
CREATE FUNCTION touches(geometry, tgeompoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Touches_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(tgeompoint, geometry)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Touches_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************
 * dwithin
 *****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

#if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _dwithin(geometry, tgeompoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geometry, tgeompoint, dist float8)
  RETURNS boolean
  AS 'SELECT @extschema@.ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2 AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _dwithin(tgeompoint, geometry, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeompoint, geometry, dist float8)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.ST_Expand($2,$3)  AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION dwithin(tgeompoint, tgeompoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER < 120000

#if POSTGRESQL_VERSION_NUMBER >= 120000
CREATE FUNCTION dwithin(geometry, tgeompoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeompoint, geometry, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeompoint, tgeompoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************/

-- TODO implement the index support in the tpoint_supportfn

#if POSTGRESQL_VERSION_NUMBER < 120000
CREATE FUNCTION _dwithin(geography, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_geo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geography, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'SELECT @extschema@._ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2 AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _dwithin(tgeogpoint, geography, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, geography, dist float8)
  RETURNS boolean
  AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@._ST_Expand($2,$3) AND @extschema@._dwithin($1, $2, $3)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION dwithin(tgeogpoint, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER < 120000

#if POSTGRESQL_VERSION_NUMBER >= 120000
CREATE FUNCTION dwithin(geography, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_geo_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, geography, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_geo'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, tgeogpoint, dist float8)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Dwithin_tpoint_tpoint'
  SUPPORT tpoint_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif //POSTGRESQL_VERSION_NUMBER >= 120000

/*****************************************************************************/
