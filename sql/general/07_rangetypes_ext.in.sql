/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written 
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO 
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/*
 * rangetypes_ext.sql
 * Definition of range types corresponding to temporal types and extension of
 * the operators for them.
 */

CREATE TYPE intrange;

CREATE FUNCTION intrange_canonical(r intrange)
  RETURNS intrange
  AS 'MODULE_PATHNAME', 'intrange_canonical'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE intrange AS RANGE (
  subtype = integer,
  SUBTYPE_DIFF = int4range_subdiff,
  CANONICAL = intrange_canonical
);

CREATE TYPE floatrange AS RANGE (
  subtype = float8,
  SUBTYPE_DIFF = float8mi
);

/******************************************************************************/

CREATE FUNCTION setPrecision(floatrange, int)
  RETURNS floatrange
  AS 'MODULE_PATHNAME', 'floatrange_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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

/******************************************************************************
 * Aggregate functions for range types
 ******************************************************************************/

CREATE OR REPLACE FUNCTION range_extent_transfn(anyrange, anyrange)
  RETURNS anyrange
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE OR REPLACE FUNCTION range_extent_combinefn(anyrange, anyrange)
  RETURNS anyrange
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(anyrange) (
  SFUNC = range_extent_transfn,
  STYPE = anyrange,
  COMBINEFUNC = range_extent_combinefn,
  PARALLEL = safe
);

/******************************************************************************/
