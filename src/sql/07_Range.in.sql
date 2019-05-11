/*****************************************************************************
 *
 * Range.sql
 *		Definition of range types corresponding to temporal types and 
 *		extension of the operators for them.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE TYPE intrange;

CREATE FUNCTION intrange_canonical(r intrange)
	RETURNS intrange
	AS 'MODULE_PATHNAME', 'intrange_canonical'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE TYPE intrange AS RANGE (
	SUBTYPE = integer,
	SUBTYPE_DIFF = int4range_subdiff,
	CANONICAL = intrange_canonical
); 
	
CREATE TYPE floatrange AS RANGE (
	SUBTYPE = float8,
	SUBTYPE_DIFF = float8mi
);
 
/******************************************************************************/

CREATE FUNCTION range_left(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_left_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_left(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_left_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = intrange, RIGHTARG = integer,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = integer, RIGHTARG = intrange,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_right(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_right_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_right(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_right_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = intrange, RIGHTARG = integer,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = integer, RIGHTARG = intrange,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_overleft(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overleft_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overleft(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overleft_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = intrange, RIGHTARG = integer,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = integer, RIGHTARG = intrange,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_overright(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overright_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overright(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overright_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = intrange, RIGHTARG = integer,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = integer, RIGHTARG = intrange,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_adjacent(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_adjacent_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_adjacent(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_adjacent_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = intrange, RIGHTARG = integer,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = integer, RIGHTARG = intrange,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);

/******************************************************************************/

CREATE FUNCTION range_left(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_left_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_left(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_left_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = floatrange, RIGHTARG = float,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = float, RIGHTARG = floatrange,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_right(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_right_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_right(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_right_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = floatrange, RIGHTARG = float,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = float, RIGHTARG = floatrange,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_overleft(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overleft_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overleft(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overleft_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = floatrange, RIGHTARG = float,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = float, RIGHTARG = floatrange,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_overright(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overright_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overright(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overright_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = floatrange, RIGHTARG = float,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = float, RIGHTARG = floatrange,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_adjacent(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_adjacent_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_adjacent(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_adjacent_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = floatrange, RIGHTARG = float,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = float, RIGHTARG = floatrange,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);

/******************************************************************************/
