/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * @brief Functions for set of poses
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

-- Input/output in WKB and HexWKB representation

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
  AS 'MODULE_PATHNAME', 'Spatialset_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(cbufferset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spatialset_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION set(cbuffer[])
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversion functions
 ******************************************************************************/

CREATE FUNCTION set(cbuffer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (cbuffer AS cbufferset) WITH FUNCTION set(cbuffer);

CREATE FUNCTION stbox(cbufferset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Spatialset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (cbufferset AS stbox) WITH FUNCTION stbox(cbufferset);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(cbufferset, integer DEFAULT 0)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Set_round'
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
  AS 'MODULE_PATHNAME', 'Spatialset_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(cbufferset, integer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Spatialset_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(cbufferset, integer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Spatialset_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(cbufferset, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Spatialset_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = cbufferset_union_finalfn
);
CREATE AGGREGATE setUnion(cbufferset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = cbufferset_union_finalfn
);

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION eq(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(cbufferset, cbufferset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(cbufferset, cbufferset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  PROCEDURE = gt,
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
    FUNCTION  1  cmp(cbufferset, cbufferset);

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

CREATE FUNCTION contains(cbufferset, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(cbufferset, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION contained(cbuffer, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(cbufferset, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overlaps(cbufferset, cbufferset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = &&
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION setUnion(cbuffer, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(cbufferset, cbuffer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(cbufferset, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION setMinus(cbuffer, cbufferset)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(cbufferset, cbuffer)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(cbufferset, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = cbuffer, RIGHTARG = cbufferset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = cbufferset, RIGHTARG = cbuffer
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = cbufferset, RIGHTARG = cbufferset
);

/*****************************************************************************/

CREATE FUNCTION setIntersection(cbuffer, cbufferset)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(cbufferset, cbuffer)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(cbufferset, cbufferset)
  RETURNS cbufferset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION setDistance(cbuffer, cbufferset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(cbufferset, cbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(cbufferset, cbufferset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = cbuffer, RIGHTARG = cbufferset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = cbufferset, RIGHTARG = cbuffer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = cbufferset, RIGHTARG = cbufferset,
  COMMUTATOR = <->
);

/*****************************************************************************/
