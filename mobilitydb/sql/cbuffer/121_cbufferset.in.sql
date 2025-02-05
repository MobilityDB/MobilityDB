/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * set.sql
 * Functions for set of ordered values.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE cbufferset;

CREATE FUNCTION cbufferset_in(cstring)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbufferset_out(cbufferset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbufferset_recv(internal)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbufferset_send(cbufferset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE cbufferset (
  internallength = variable,
  input = cbufferset_in,
  output = cbufferset_out,
  receive = cbufferset_recv,
  send = cbufferset_send,
  alignment = double,
  storage = extended
  -- , analyze = geoset_analyze
);

/******************************************************************************/

-- Input/output in WKB and HexWKB format

CREATE FUNCTION cbuffersetFromBinary(bytea)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffersetFromHexWKB(text)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(cbufferset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(cbufferset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(cbufferset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Cbufferset_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(cbufferset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Cbufferset_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION set(cbuffer[])
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION set(cbuffer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (cbuffer AS cbufferset) WITH FUNCTION set(cbuffer);

CREATE FUNCTION stbox(cbufferset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Cbufferset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (cbufferset AS stbox) WITH FUNCTION stbox(cbufferset);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(cbufferset, integer DEFAULT 0)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Cbufferset_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION memSize(cbufferset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(cbufferset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(cbufferset)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(cbufferset)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(cbufferset, integer)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(cbufferset)
  RETURNS cbuffer[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * SRID
 ******************************************************************************/

CREATE FUNCTION SRID(cbufferset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Cbufferset_get_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(cbufferset, integer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Cbufferset_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION transform(cbufferset, integer)
  -- RETURNS cbufferset
  -- AS 'MODULE_PATHNAME', 'Cbufferset_transform'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION transformPipeline(cbufferset, text, srid integer DEFAULT 0,
    -- is_forward boolean DEFAULT true)
  -- RETURNS cbufferset
  -- AS 'MODULE_PATHNAME', 'Cbufferset_transform_pipeline'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Transformation set of values <-> set
 ******************************************************************************/

CREATE FUNCTION unnest(cbufferset)
  RETURNS SETOF cbuffer
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION set_union_transfn(internal, cbuffer)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_union_transfn(internal, cbufferset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION cbufferset_union_finalfn(internal)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(cbuffer) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = cbufferset_union_finalfn
);
CREATE AGGREGATE setUnion(cbufferset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = cbufferset_union_finalfn
);

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION set_eq(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ne(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_lt(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_le(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ge(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gt(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_cmp(cbufferset, cbufferset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = set_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = set_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = set_lt,
  COMMUTATOR = >, NEGATOR = >=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = set_le,
  COMMUTATOR = >=, NEGATOR = >
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = set_ge,
  COMMUTATOR = <=, NEGATOR = <
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = set_gt,
  COMMUTATOR = <, NEGATOR = <=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS cbufferset_btree_ops
  DEFAULT FOR TYPE cbufferset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  set_cmp(cbufferset, cbufferset);

/******************************************************************************/

CREATE FUNCTION set_hash(cbufferset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash_extended(cbufferset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS cbufferset_hash_ops
  DEFAULT FOR TYPE cbufferset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   set_hash(cbufferset),
    FUNCTION    2   set_hash_extended(cbufferset, bigint);

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION set_contains(cbufferset, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(cbufferset, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_contained(cbuffer, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(cbufferset, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_overlaps(cbufferset, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = &&
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION set_union(cbuffer, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(cbufferset, cbuffer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(cbufferset, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION set_minus(cbuffer, cbufferset)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(cbufferset, cbuffer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(cbufferset, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = cbuffer, RIGHTARG = cbufferset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = cbufferset, RIGHTARG = cbuffer
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = cbufferset, RIGHTARG = cbufferset
);

/*****************************************************************************/

CREATE FUNCTION set_intersection(cbuffer, cbufferset)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(cbufferset, cbuffer)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(cbufferset, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION set_distance(cbuffer, cbufferset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(cbufferset, cbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(cbufferset, cbufferset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = <->
);

/*****************************************************************************/
