/*****************************************************************************
 *
 * stbox.sql
 *	  Basic functions for STBOX bounding box.
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

CREATE TYPE stbox;

CREATE FUNCTION stbox_in(cstring)
	RETURNS stbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_out(stbox)
	RETURNS cstring 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION stbox_recv(internal)
	RETURNS stbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_send(stbox)
	RETURNS bytea 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE TYPE stbox (
	internallength = 72,
	input = stbox_in,
	output = stbox_out,
--	receive = stbox_recv,
--	send = stbox_send,
	storage = plain,
	alignment = double
--    , analyze = stbox_analyze
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

 CREATE FUNCTION stbox(float8, float8, float8, float8)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(float8, float8, float8, float8, float8, float8)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxt(float8, float8, float8, float8, float8, float8)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stboxt_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstbox(float8, float8, float8, float8, float8, float8)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'geodstbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'geodstbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION stbox_eq(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'stbox_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_ne(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'stbox_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_lt(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'stbox_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION stbox_le(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'stbox_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION stbox_ge(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'stbox_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION stbox_gt(stbox, stbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'stbox_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION stbox_cmp(stbox, stbox)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'stbox_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = stbox_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = stbox, RIGHTARG = stbox,
	PROCEDURE = stbox_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = stbox_lt,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = stbox_le,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = stbox_ge,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
	PROCEDURE = stbox_gt,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS stbox_ops
	DEFAULT FOR TYPE stbox USING btree AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	stbox_cmp(stbox, stbox);

/*****************************************************************************/
