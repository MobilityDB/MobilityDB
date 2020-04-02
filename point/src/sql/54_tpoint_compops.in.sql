/*****************************************************************************
 *
 * tpoint_compops.sql
 *	  Comparison functions and operators for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION tpoint_eq(geometry(Point), tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_eq(tgeompoint, geometry(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_eq(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = tpoint_eq,
	LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = tpoint_eq,
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = tpoint_eq,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = #=
);

/*****************************************************************************/

CREATE FUNCTION tpoint_eq(geography(Point), tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_eq(tgeogpoint, geography(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_eq(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = tpoint_eq,
	LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = tpoint_eq,
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = tpoint_eq,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION tpoint_ne(geometry(Point), tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeompoint, geometry(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = tpoint_ne,
	LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = tpoint_ne,
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = tpoint_ne,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = #<>
);

/*****************************************************************************/

CREATE FUNCTION tpoint_ne(geography(Point), tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeogpoint, geography(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_ne(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = tpoint_ne,
	LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = tpoint_ne,
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = tpoint_ne,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = #<>
);

/******************************************************************************/
