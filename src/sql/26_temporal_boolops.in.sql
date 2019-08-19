/*****************************************************************************
 *
 * temporal_boolops.sql
 *	  Temporal Boolean function and operators.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

CREATE FUNCTION temporal_and(boolean, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tand_bool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_and(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tand_tbool_bool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_and(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tand_tbool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR & (
	PROCEDURE = temporal_and,
	LEFTARG = boolean, RIGHTARG = tbool,
	COMMUTATOR = &
);
CREATE OPERATOR & (
	PROCEDURE = temporal_and,
	LEFTARG = tbool, RIGHTARG = boolean,
	COMMUTATOR = &
);
CREATE OPERATOR & (
	PROCEDURE = temporal_and,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = &
);

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

CREATE FUNCTION temporal_or(boolean, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tor_bool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_or(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tor_tbool_bool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_or(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tor_tbool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR | (
	PROCEDURE = temporal_or,
	LEFTARG = boolean, RIGHTARG = tbool,
	COMMUTATOR = |
);
CREATE OPERATOR | (
	PROCEDURE = temporal_or,
	LEFTARG = tbool, RIGHTARG = boolean,
	COMMUTATOR = |
);
CREATE OPERATOR | (
	PROCEDURE = temporal_or,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = |
);

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

CREATE FUNCTION temporal_not(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tnot_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~ (
	PROCEDURE = temporal_not, RIGHTARG = tbool
);

/*****************************************************************************/