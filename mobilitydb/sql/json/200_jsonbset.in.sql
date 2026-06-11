/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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

/**
 * @file
 * @brief Functions for set of JSONB values
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE jsonbset;

CREATE FUNCTION jsonbset_in(cstring)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_out(jsonbset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbset_recv(internal)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset_send(jsonbset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE jsonbset (
  INTERNALLENGTH = VARIABLE,
  INPUT = jsonbset_in,
  OUTPUT = jsonbset_out,
  RECEIVE = jsonbset_recv,
  SEND = jsonbset_send,
  ALIGNMENT = DOUBLE,
  STORAGE = EXTENDED
);

/******************************************************************************/

-- Input/output in WKB and HexWKB representation

CREATE FUNCTION jsonbsetFromBinary(bytea)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION jsonbsetFromHexWKB(text)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(jsonbset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(jsonbset, endianencoding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(jsonbset, endianencoding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION set(jsonb[])
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversions
 ******************************************************************************/

CREATE FUNCTION set(jsonb)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (jsonb AS jsonbset) WITH FUNCTION set(jsonb);

CREATE FUNCTION textset(jsonbset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Jsonbset_as_textset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION jsonbset(textset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Textset_as_jsonbset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (jsonbset AS textset) WITH FUNCTION textset(jsonbset);
CREATE CAST (textset AS jsonbset) WITH FUNCTION jsonbset(textset);

/*****************************************************************************
 * Transformations
 *****************************************************************************/


/******************************************************************************
 * Accessors
 ******************************************************************************/

CREATE FUNCTION memSize(jsonbset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(jsonbset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(jsonbset)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(jsonbset)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(jsonbset, integer)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(jsonbset)
  RETURNS jsonb[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Transformation set of values <-> set
 ******************************************************************************/

CREATE FUNCTION unnest(jsonbset)
  RETURNS SETOF jsonb
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION set_union_transfn(internal, jsonb)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_union_transfn(internal, jsonbset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION jsonbset_union_finalfn(internal)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(jsonb) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = jsonbset_union_finalfn
);
CREATE AGGREGATE setUnion(jsonbset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = jsonbset_union_finalfn
);

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION set_eq(jsonbset, jsonbset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ne(jsonbset, jsonbset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_lt(jsonbset, jsonbset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_le(jsonbset, jsonbset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ge(jsonbset, jsonbset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gt(jsonbset, jsonbset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_cmp(jsonbset, jsonbset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  PROCEDURE = set_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  PROCEDURE = set_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  PROCEDURE = set_lt,
  COMMUTATOR = >, NEGATOR = >=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  PROCEDURE = set_le,
  COMMUTATOR = >=, NEGATOR = >
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  PROCEDURE = set_ge,
  COMMUTATOR = <=, NEGATOR = <
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  PROCEDURE = set_gt,
  COMMUTATOR = <, NEGATOR = <=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS jsonbset_btree_ops
  DEFAULT FOR TYPE jsonbset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  set_cmp(jsonbset, jsonbset);

/******************************************************************************/

CREATE FUNCTION set_hash(jsonbset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash_extended(jsonbset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS jsonbset_hash_ops
  DEFAULT FOR TYPE jsonbset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   set_hash(jsonbset),
    FUNCTION    2   set_hash_extended(jsonbset, bigint);

/*****************************************************************************
 * JSONB Functions
 *****************************************************************************/

-- CREATE FUNCTION jsonbset_concat(jsonb, jsonbset)
  -- RETURNS jsonbset
  -- AS 'MODULE_PATHNAME', 'Concat_jsonb_jsonbset'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION jsonbset_concat(jsonbset, jsonb)
  -- RETURNS jsonbset
  -- AS 'MODULE_PATHNAME', 'Concat_jsonbset_jsonb'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR || (
  -- PROCEDURE = jsonbset_concat,
  -- LEFTARG   = jsonb, RIGHTARG = jsonbset
-- );
-- CREATE OPERATOR || (
  -- PROCEDURE = jsonbset_concat,
  -- LEFTARG   = jsonbset, RIGHTARG = jsonb
-- );

-- CREATE FUNCTION jsonbset_delete(jsonbset, text)
  -- RETURNS jsonbset
  -- AS 'MODULE_PATHNAME', 'Delete_jsonbset_key'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR - (
  -- PROCEDURE = jsonbset_delete,
  -- LEFTARG   = jsonbset, RIGHTARG = text
-- );

-- CREATE FUNCTION jsonbset_set(s jsonbset, path text[], val jsonb,
  -- create_missing boolean DEFAULT true)
-- RETURNS jsonbset
-- AS 'MODULE_PATHNAME', 'Jsonbset_set'
-- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION set_contains(jsonbset, jsonb)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(jsonbset, jsonbset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = jsonbset, RIGHTARG = jsonb,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = <@
);

/******************************************************************************/

CREATE FUNCTION set_contained(jsonb, jsonbset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(jsonbset, jsonbset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = jsonb, RIGHTARG = jsonbset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = @>
);

/******************************************************************************/

CREATE FUNCTION set_overlaps(jsonbset, jsonbset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = &&
);

/*****************************************************************************/

-- “Left”/“right” operators, useful for btree spans
CREATE FUNCTION set_left(jsonbset, jsonbset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = >>
);

CREATE FUNCTION set_right(jsonbset, jsonbset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = <<
);

/*****************************************************************************/

CREATE FUNCTION set_union(jsonb, jsonbset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(jsonbset, jsonb)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(jsonbset, jsonbset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = jsonb, RIGHTARG = jsonbset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = jsonbset, RIGHTARG = jsonb,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION set_minus(jsonb, jsonbset)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(jsonbset, jsonb)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(jsonbset, jsonbset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = jsonb, RIGHTARG = jsonbset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = jsonbset, RIGHTARG = jsonb
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = jsonbset, RIGHTARG = jsonbset
);

/*****************************************************************************/

CREATE FUNCTION set_intersection(jsonb, jsonbset)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(jsonbset, jsonb)
  RETURNS jsonb
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(jsonbset, jsonbset)
  RETURNS jsonbset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = jsonb, RIGHTARG = jsonbset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = jsonbset, RIGHTARG = jsonb,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = jsonbset, RIGHTARG = jsonbset,
  COMMUTATOR = *
);

/*****************************************************************************/
