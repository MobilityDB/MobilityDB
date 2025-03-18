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

CREATE TYPE poseset;

CREATE FUNCTION poseset_in(cstring)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION poseset_out(poseset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION poseset_recv(internal)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION poseset_send(poseset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE poseset (
  internallength = variable,
  input = poseset_in,
  output = poseset_out,
  receive = poseset_recv,
  send = poseset_send,
  alignment = double,
  storage = extended
  -- , analyze = geoset_analyze
);

/******************************************************************************/

-- Input/output in WKB and HexWKB format

CREATE FUNCTION posesetFromBinary(bytea)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION posesetFromHexWKB(text)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(poseset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(poseset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(poseset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spatialset_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(poseset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spatialset_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION set(pose[])
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversion functions
 ******************************************************************************/

CREATE FUNCTION set(pose)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pose AS poseset) WITH FUNCTION set(pose);

CREATE FUNCTION stbox(poseset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Poseset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (poseset AS stbox) WITH FUNCTION stbox(poseset);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(poseset, integer DEFAULT 0)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Poseset_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION memSize(poseset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(poseset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(poseset)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(poseset)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(poseset, integer)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(poseset)
  RETURNS pose[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * SRID
 ******************************************************************************/

CREATE FUNCTION SRID(poseset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spatialset_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(poseset, integer)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Spatialset_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(poseset, integer)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Spatialset_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(poseset, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Spatialset_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Transformation set of values <-> set
 ******************************************************************************/

CREATE FUNCTION unnest(poseset)
  RETURNS SETOF pose
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION set_union_transfn(internal, pose)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_union_transfn(internal, poseset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION poseset_union_finalfn(internal)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(pose) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = poseset_union_finalfn
);
CREATE AGGREGATE setUnion(poseset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = poseset_union_finalfn
);

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION set_eq(poseset, poseset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ne(poseset, poseset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_lt(poseset, poseset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_le(poseset, poseset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ge(poseset, poseset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gt(poseset, poseset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_cmp(poseset, poseset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = poseset, RIGHTARG = poseset,
  PROCEDURE = set_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = poseset, RIGHTARG = poseset,
  PROCEDURE = set_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = poseset, RIGHTARG = poseset,
  PROCEDURE = set_lt,
  COMMUTATOR = >, NEGATOR = >=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = poseset, RIGHTARG = poseset,
  PROCEDURE = set_le,
  COMMUTATOR = >=, NEGATOR = >
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = poseset, RIGHTARG = poseset,
  PROCEDURE = set_ge,
  COMMUTATOR = <=, NEGATOR = <
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = poseset, RIGHTARG = poseset,
  PROCEDURE = set_gt,
  COMMUTATOR = <, NEGATOR = <=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS poseset_btree_ops
  DEFAULT FOR TYPE poseset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  set_cmp(poseset, poseset);

/******************************************************************************/

CREATE FUNCTION set_hash(poseset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash_extended(poseset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS poseset_hash_ops
  DEFAULT FOR TYPE poseset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   set_hash(poseset),
    FUNCTION    2   set_hash_extended(poseset, bigint);

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION set_contains(poseset, pose)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(poseset, poseset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = poseset, RIGHTARG = pose,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = poseset, RIGHTARG = poseset,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_contained(pose, poseset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(poseset, poseset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = pose, RIGHTARG = poseset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = poseset, RIGHTARG = poseset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_overlaps(poseset, poseset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = poseset, RIGHTARG = poseset,
  COMMUTATOR = &&
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION set_union(pose, poseset)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(poseset, pose)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(poseset, poseset)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = pose, RIGHTARG = poseset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = poseset, RIGHTARG = pose,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = poseset, RIGHTARG = poseset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION set_minus(pose, poseset)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(poseset, pose)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(poseset, poseset)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = pose, RIGHTARG = poseset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = poseset, RIGHTARG = pose
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = poseset, RIGHTARG = poseset
);

/*****************************************************************************/

CREATE FUNCTION set_intersection(pose, poseset)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(poseset, pose)
  RETURNS pose
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(poseset, poseset)
  RETURNS poseset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = pose, RIGHTARG = poseset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = poseset, RIGHTARG = pose,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = poseset, RIGHTARG = poseset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION set_distance(pose, poseset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(poseset, pose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(poseset, poseset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = pose, RIGHTARG = poseset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = poseset, RIGHTARG = pose,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = poseset, RIGHTARG = poseset,
  COMMUTATOR = <->
);

/*****************************************************************************/
