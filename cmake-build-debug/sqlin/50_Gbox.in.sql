/*****************************************************************************
 *
 * Gbox.sql
 *	  Basic functions for GBOX bounding box.
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

CREATE TYPE gbox;

CREATE FUNCTION gbox_in(cstring)
	RETURNS gbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_out(gbox)
	RETURNS cstring 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION gbox_recv(internal)
	RETURNS gbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_send(gbox)
	RETURNS bytea 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE TYPE gbox (
	internallength = 72,
	input = gbox_in,
	output = gbox_out,
--	receive = gbox_recv,
--	send = gbox_send,
	storage = plain,
	alignment = double
--    , analyze = gbox_analyze
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

 CREATE FUNCTION gbox(float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gbox(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gbox3dm(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox3dm_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodbox(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geodbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geodbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION gbox_eq(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_ne(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_lt(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_le(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_ge(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_gt(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_cmp(gbox, gbox)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'gbox_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = gbox_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = gbox_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = gbox_lt,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = gbox_le,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = gbox_ge,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
	PROCEDURE = gbox_gt,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS gbox_ops
	DEFAULT FOR TYPE gbox USING btree AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	gbox_cmp(gbox, gbox);

/*****************************************************************************/
