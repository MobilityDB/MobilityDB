/*****************************************************************************
 *
 * tbox.sql
 *	  Basic functions for TBOX bounding box.
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
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

 CREATE FUNCTION tbox(float8, float8)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 CREATE FUNCTION tboxt(timestamptz, timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tboxt_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 CREATE FUNCTION tbox(float8, timestamptz, float8, timestamptz)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION floatrange(tbox)
	RETURNS floatrange
	AS 'MODULE_PATHNAME', 'tbox_to_floatrange'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tbox)
	RETURNS period
	AS 'MODULE_PATHNAME', 'tbox_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbox AS floatrange) WITH FUNCTION floatrange(tbox);
CREATE CAST (tbox AS period) WITH FUNCTION period(tbox);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION Xmin(tbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tbox_xmin'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Xmax(tbox)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tbox_xmax'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Tmin(tbox)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'tbox_tmin'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION Tmax(tbox)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'tbox_tmax'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectivity functions
 *****************************************************************************/

CREATE FUNCTION tnumber_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Expand functions
 *****************************************************************************/

CREATE FUNCTION expandValue(tbox, float)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_expand_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTemporal(tbox, interval)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

CREATE FUNCTION tbox_contains(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_contained(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_overlaps(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_same(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_adjacent(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = tbox_contains,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = <@,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = tbox_contained,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = @>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = tbox_overlaps,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = &&,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = tbox_same,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = tbox_adjacent,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = -|-,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Position operators
 *****************************************************************************/

CREATE FUNCTION temporal_left(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tbox_tbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	PROCEDURE = temporal_left,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = >>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
	PROCEDURE = temporal_overleft,
	LEFTARG = tbox, RIGHTARG = tbox,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tbox, RIGHTARG = tbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
	PROCEDURE = temporal_overright,
	LEFTARG = tbox, RIGHTARG = tbox,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = tbox, RIGHTARG = tbox,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = tbox, RIGHTARG = tbox,
	RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION tbox_union(tbox, tbox)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_union'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_intersection(tbox, tbox)
	RETURNS tbox
	AS 'MODULE_PATHNAME', 'tbox_intersection'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
	PROCEDURE = tbox_union,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = +
);
CREATE OPERATOR * (
	PROCEDURE = tbox_intersection,
	LEFTARG = tbox, RIGHTARG = tbox,
	COMMUTATOR = *
);

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
