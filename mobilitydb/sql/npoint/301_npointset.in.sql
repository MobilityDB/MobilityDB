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
 * @brief Functions for set of network point values
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE npointset;

CREATE FUNCTION npointset_in(cstring)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npointset_out(npointset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npointset_recv(internal)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npointset_send(npointset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE npointset (
  internallength = variable,
  input = npointset_in,
  output = npointset_out,
  receive = npointset_recv,
  send = npointset_send,
  alignment = double,
  storage = extended
  -- , analyze = geoset_analyze
);

/******************************************************************************/

-- Input/output in WKT, WKB, and HexWKB representation

CREATE FUNCTION npointsetFromText(text)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Spatialset_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npointsetFromEWKT(text)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Spatialset_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npointsetFromBinary(bytea)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npointsetFromEWKB(bytea)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npointsetFromHexWKB(text)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(npointset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(npointset, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spatialset_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(npointset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKB(npointset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spatialset_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(npointset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION set(npoint[])
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Conversions
 ******************************************************************************/

CREATE FUNCTION set(npoint)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(npointset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Spatialset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS npointset) WITH FUNCTION set(npoint);
CREATE CAST (npointset AS stbox) WITH FUNCTION stbox(npointset);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(npointset, integer DEFAULT 0)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION memSize(npointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(npointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(npointset)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(npointset)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(npointset, integer)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(npointset)
  RETURNS npoint[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION routes(npointset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Npointset_routes'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Transformation set of values <-> set
 ******************************************************************************/

CREATE FUNCTION unnest(npointset)
  RETURNS SETOF npoint
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

-- The function is not STRICT
CREATE FUNCTION set_union_transfn(internal, npoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_union_transfn(internal, npointset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION npointset_union_finalfn(internal)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(npoint) (
  SFUNC = set_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = npointset_union_finalfn
);
CREATE AGGREGATE setUnion(npointset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
#if POSTGRESQL_VERSION_NUMBER >= 160000
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
#endif //POSTGRESQL_VERSION_NUMBER >= 160000
  FINALFUNC = npointset_union_finalfn
);

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION eq(npointset, npointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(npointset, npointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(npointset, npointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(npointset, npointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(npointset, npointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(npointset, npointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(npointset, npointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = npointset, RIGHTARG = npointset,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = npointset, RIGHTARG = npointset,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = npointset, RIGHTARG = npointset,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = npointset, RIGHTARG = npointset,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = npointset, RIGHTARG = npointset,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = npointset, RIGHTARG = npointset,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS npointset_btree_ops
  DEFAULT FOR TYPE npointset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(npointset, npointset);

/******************************************************************************/

CREATE FUNCTION set_hash(npointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash_extended(npointset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS npointset_hash_ops
  DEFAULT FOR TYPE npointset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   set_hash(npointset),
    FUNCTION    2   set_hash_extended(npointset, bigint);

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION contains(npointset, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(npointset, npointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = npointset, RIGHTARG = npoint,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = npointset, RIGHTARG = npointset,
  COMMUTATOR = <@
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION contained(npoint, npointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(npointset, npointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = npoint, RIGHTARG = npointset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = npointset, RIGHTARG = npointset,
  COMMUTATOR = @>
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overlaps(npointset, npointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = npointset, RIGHTARG = npointset,
  COMMUTATOR = &&
  -- RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION setUnion(npoint, npointset)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(npointset, npoint)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(npointset, npointset)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = npoint, RIGHTARG = npointset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = npointset, RIGHTARG = npoint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = npointset, RIGHTARG = npointset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION setMinus(npoint, npointset)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(npointset, npoint)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(npointset, npointset)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = npoint, RIGHTARG = npointset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = npointset, RIGHTARG = npoint
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = npointset, RIGHTARG = npointset
);

/*****************************************************************************/

CREATE FUNCTION setIntersection(npoint, npointset)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(npointset, npoint)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(npointset, npointset)
  RETURNS npointset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = npoint, RIGHTARG = npointset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = npointset, RIGHTARG = npoint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = npointset, RIGHTARG = npointset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION setDistance(npoint, npointset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(npointset, npoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(npointset, npointset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = npoint, RIGHTARG = npointset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = npointset, RIGHTARG = npoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = npointset, RIGHTARG = npointset,
  COMMUTATOR = <->
);

/*****************************************************************************/
