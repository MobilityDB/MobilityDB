/*****************************************************************************
 *
 * ComparisonOps.sql
 *	  Comparison functions and operators for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal equal for geometry
 *****************************************************************************/

CREATE FUNCTION temporal_eq(geometry(Point), tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeompoint, geometry(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal equal for geography
 *****************************************************************************/

CREATE FUNCTION temporal_eq(geography(Point), tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeogpoint, geography(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal for geometry
 *****************************************************************************/

CREATE FUNCTION temporal_ne(geometry(Point), tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeompoint, geometry(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = #<>
);

/*****************************************************************************
 * Temporal not equal for geography
 *****************************************************************************/

CREATE FUNCTION temporal_ne(geography(Point), tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeogpoint, geography(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = #<>
);

/******************************************************************************/
