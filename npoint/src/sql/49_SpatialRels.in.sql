/*****************************************************************************
 *
 * SpatialRels.sql
 *	  Spatial relationships for temporal network-constrained points.
 *
 * These relationships are generalized to the temporal dimension with the 
 * "at any instant" semantics, that is, the traditional operator is applied to
 * the union of all values taken by the temporal npoint and returns a Boolean.
 * The following relationships are supported:
 *		contains, containsproperly, covers, coveredby, crosses, disjoint, 
 *		equals, intersects, overlaps, touches, within, dwithin, and
 *		relate (with 2 and 3 arguments)
 * All these relationships, excepted disjoint and relate, will automatically 
 * include a bounding box comparison that will make use of any spatial, 
 * temporal, or spatiotemporal indexes that are available.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * contains
 *****************************************************************************/

CREATE FUNCTION _contains(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _contains(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _contains(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
		
/*****************************************************************************
 * containsproperly
 *****************************************************************************/

CREATE FUNCTION _containsproperly(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'containsproperly_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _containsproperly(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'containsproperly_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _containsproperly(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'containsproperly_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
		
/*****************************************************************************
 * covers
 *****************************************************************************/

CREATE FUNCTION _covers(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _covers(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _covers(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * coveredby
 *****************************************************************************/

CREATE FUNCTION _coveredby(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _coveredby(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _coveredby(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * crosses
 *****************************************************************************/

CREATE FUNCTION _crosses(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'crosses_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _crosses(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'crosses_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _crosses(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'crosses_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * disjoint
 *****************************************************************************/

CREATE FUNCTION disjoint(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'disjoint_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'disjoint_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'disjoint_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * equals
 *****************************************************************************/

CREATE FUNCTION _equals(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'equals_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _equals(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'equals_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _equals(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'equals_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * intersects
 *****************************************************************************/

CREATE FUNCTION _intersects(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _intersects(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _intersects(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

/*****************************************************************************
 * overlaps
 *****************************************************************************/

CREATE FUNCTION _overlaps(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _overlaps(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _overlaps(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * touches
 *****************************************************************************/

CREATE FUNCTION _touches(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'touches_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _touches(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'touches_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _touches(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'touches_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * within
 *****************************************************************************/

CREATE FUNCTION _within(geometry, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'within_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(geometry, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _within(tnpoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'within_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(tnpoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _within(tnpoint, tnpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'within_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(tnpoint, tnpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * dwithin
 *****************************************************************************/

CREATE FUNCTION _dwithin(geometry, tnpoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geometry, tnpoint, dist float8)
	RETURNS boolean
	AS 'SELECT @extschema@.ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
		
CREATE FUNCTION _dwithin(tnpoint, geometry, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tnpoint, geometry, dist float8)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.ST_Expand($2,$3) 
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION _dwithin(tnpoint, tnpoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tnpoint, tnpoint, dist float8)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.expandSpatial($2,$3) 
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
/*****************************************************************************
 * relate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION relate(geometry, tnpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'relate_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tnpoint, geometry)
	RETURNS text
	AS 'MODULE_PATHNAME', 'relate_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tnpoint, tnpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'relate_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * relate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION relate(geometry, tnpoint, pattern text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'relate_pattern_geo_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tnpoint, geometry, pattern text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'relate_pattern_tnpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tnpoint, tnpoint, pattern text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'relate_pattern_tnpoint_tnpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
