/*****************************************************************************
 *
 * tbox.sql
 *	  Basic functions for TBOX bounding box.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE tbox;

CREATE FUNCTION tbox_in(cstring)
	RETURNS tbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_out(tbox)
	RETURNS cstring 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION tbox_recv(internal)
	RETURNS tbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_send(tbox)
	RETURNS bytea 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE TYPE tbox (
	internallength = 40,
	input = tbox_in,
	output = tbox_out,
--	receive = tbox_recv,
--	send = tbox_send,
	storage = plain,
	alignment = double
--    , analyze = tbox_analyze
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

 CREATE FUNCTION tbox(float8, float8)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 CREATE FUNCTION tboxt(float8, float8)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tboxt_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 CREATE FUNCTION tbox(float8, float8, float8, float8)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION tbox_eq(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tbox_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_ne(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tbox_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_lt(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tbox_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION tbox_le(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tbox_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION tbox_ge(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tbox_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION tbox_gt(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tbox_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION tbox_cmp(tbox, tbox)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'tbox_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
	LEFTARG = tbox, RIGHTARG = tbox,
	PROCEDURE = tbox_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tbox, RIGHTARG = tbox,
	PROCEDURE = tbox_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = tbox_lt,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = tbox_le,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = tbox_ge,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
	PROCEDURE = tbox_gt,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS tbox_ops
	DEFAULT FOR TYPE tbox USING btree AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	tbox_cmp(tbox, tbox);

/*****************************************************************************/
