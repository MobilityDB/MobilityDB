/*****************************************************************************
 *
 * ComparisonOps.sql
 *	  Comparison functions and operators for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal equal
 *****************************************************************************/

CREATE FUNCTION temporal_eq(npoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tnpoint, npoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = npoint, RIGHTARG = tnpoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tnpoint, RIGHTARG = npoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal
 *****************************************************************************/

CREATE FUNCTION temporal_ne(npoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tnpoint, npoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tnpoint, tnpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = npoint, RIGHTARG = tnpoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tnpoint, RIGHTARG = npoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	COMMUTATOR = #<>
);

/******************************************************************************/
