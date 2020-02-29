/*****************************************************************************
 *
 * stbox.sql
 *	  Basic functions for STBOX bounding box.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
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
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to timestamptz is needed */
CREATE FUNCTION stboxt(timestamptz, timestamptz, int DEFAULT 0)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(float8, float8, float8, float8, int DEFAULT 0)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(float8, float8, float8, float8, float8, float8, int DEFAULT 0)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(float8, float8, float8, timestamptz, float8, float8, float8, timestamptz, int DEFAULT 0)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxt(float8, float8, timestamptz, float8, float8, timestamptz, int DEFAULT 0)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'stboxt_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */
CREATE FUNCTION geodstboxt(timestamptz, timestamptz, int DEFAULT 4326)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'geodstbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstbox(float8, float8, float8, float8, float8, float8, int DEFAULT 4326)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'geodstbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstbox(float8, float8, float8, timestamptz, float8, float8, float8, timestamptz, int DEFAULT 4326)
	RETURNS stbox
	AS 'MODULE_PATHNAME', 'geodstbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION Xmin(stbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'stbox_xmin'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Ymin(stbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'stbox_ymin'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Zmin(stbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'stbox_zmin'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Tmin(stbox)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'stbox_tmin'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION Xmax(stbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'stbox_xmax'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Ymax(stbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'stbox_ymax'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Zmax(stbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'stbox_zmax'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Tmax(stbox)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'stbox_tmax'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION srid(stbox)
	RETURNS int
	AS 'MODULE_PATHNAME', 'stbox_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION period(stbox)
	RETURNS period
	AS 'MODULE_PATHNAME', 'stbox_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box2d(stbox)
	RETURNS box2d
	AS 'MODULE_PATHNAME', 'stbox_to_box2d'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box3d(stbox)
	RETURNS box3d
	AS 'MODULE_PATHNAME', 'stbox_to_box3d'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE CAST (stbox AS period) WITH FUNCTION period(stbox);
CREATE CAST (stbox AS box2d) WITH FUNCTION box2d(stbox);
CREATE CAST (stbox AS box3d) WITH FUNCTION box3d(stbox);

/*****************************************************************************
 * Operators
 *****************************************************************************/

CREATE FUNCTION stbox_intersection(stbox, stbox)
	RETURNS stbox
AS 'MODULE_PATHNAME', 'stbox_intersection'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
	PROCEDURE = stbox_intersection,
	LEFTARG = stbox, RIGHTARG = stbox,
	COMMUTATOR = *
);

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
