/*****************************************************************************
 *
 * tpoint_distance.sql
 *	  Temporal distance for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * temporal geometry points
 *****************************************************************************/

/* geometry <-> <duration> */

CREATE FUNCTION distance(geometry, tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, geometry)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = <->
);

/*****************************************************************************
 * temporal geography points
 *****************************************************************************/

/* geography <-> <duration> */

CREATE FUNCTION distance(geography, tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeogpoint, geography)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeogpoint, tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;		

CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = <->
);

/******************************************************************************/
