/*****************************************************************************
 *
 * SpatialRelsG.sql
 *	  Spatial relationships for temporal geography points.
 *
 * These relationships are generalized to the temporal dimension with the 
 * "at any instant" semantics, that is, the traditional operator is applied to
 * the union of all values taken by the temporal point and returns a Boolean.
 * The following relationships are supported for geographies:
 *		covers, coveredby, intersects, dwithin
 * All these relationships will automatically include a bounding box comparison 
 * that will make use of any spatial, temporal, or spatiotemporal indexes 
 * that are available.
 * N.B. In the current version of Postgis (2.4) the only index operator 
 * implemented for geography is &&
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * covers
 *****************************************************************************/

CREATE FUNCTION _covers(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geography, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _covers(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tgeogpoint, geography)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _covers(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
			
/*****************************************************************************
 * coveredby
 *****************************************************************************/

CREATE FUNCTION _coveredby(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(geography, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
		
/*****************************************************************************/

CREATE FUNCTION _coveredby(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tgeogpoint, geography)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _coveredby(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
		
/*****************************************************************************
 * intersects
 *****************************************************************************/

CREATE FUNCTION _intersects(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geography, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _intersects(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, geography)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _intersects(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND $1 = $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * dwithin
 *****************************************************************************/

CREATE FUNCTION _dwithin(geography, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geography, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'SELECT @extschema@._ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _dwithin(tgeogpoint, geography, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, geography, dist float8)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@._ST_Expand($2,$3) 
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _dwithin(tgeogpoint, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.expandSpatial($2,$3) 
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************/
