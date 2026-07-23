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
 * @brief `quadbinset` SQL type — set of quadbin values, mirrors
 * the bigintset structure over the quadbin base type.
 *
 * Every C call routes to a generic `Set_*` symbol. The dispatch
 * arms in `type_in.c` / `type_out.c` (basetype_in / basetype_out
 * cases for T_QUADBIN) make the generic Set parser and formatter
 * use quadbin_parse / quadbin_index_to_string for elements.
 */

/******************************************************************************
 * Type plumbing
 ******************************************************************************/

CREATE TYPE quadbinset;

CREATE FUNCTION quadbinset_in(cstring)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbinset_out(quadbinset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbinset_recv(internal)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbinset_send(quadbinset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE quadbinset (
  internallength = variable,
  input = quadbinset_in,
  output = quadbinset_out,
  receive = quadbinset_recv,
  send = quadbinset_send,
  alignment = double,
  storage = extended
  -- , analyze = geoset_analyze
);

/******************************************************************************
 * WKB / HexWKB helpers
 ******************************************************************************/

CREATE FUNCTION quadbinsetFromBinary(bytea)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbinsetFromHexWKB(text)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(quadbinset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(quadbinset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(quadbinset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION set(quadbin[])
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Singleton constructor: `set(basetype)` is the cross-set convention
-- (matches set(bigint), set(text), etc.).
CREATE FUNCTION set(quadbin)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (quadbin AS quadbinset) WITH FUNCTION set(quadbin);

/******************************************************************************
 * Accessors
 ******************************************************************************/

CREATE FUNCTION memSize(quadbinset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(quadbinset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(quadbinset)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(quadbinset)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(quadbinset, integer)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(quadbinset)
  RETURNS quadbin[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison operators + btree / hash opclasses
 ******************************************************************************/

CREATE FUNCTION eq(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(quadbinset, quadbinset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash(quadbinset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = eq, COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel, HASHES, MERGES);
CREATE OPERATOR <> (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = ne, COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel);
CREATE OPERATOR < (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = lt, COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel);
CREATE OPERATOR <= (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = le, COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel);
CREATE OPERATOR > (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = gt, COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel);
CREATE OPERATOR >= (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = ge, COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel);

CREATE OPERATOR CLASS quadbinset_btree_ops
  DEFAULT FOR TYPE quadbinset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(quadbinset, quadbinset);

CREATE OPERATOR CLASS quadbinset_hash_ops
  DEFAULT FOR TYPE quadbinset USING hash AS
    OPERATOR  1  =,
    FUNCTION  1  set_hash(quadbinset);

/******************************************************************************
 * unnest — SETOF expansion
 ******************************************************************************/

CREATE FUNCTION unnest(quadbinset)
  RETURNS SETOF quadbin
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * setUnion aggregate
 *
 * Two aggregate overloads, matching the pattern every other *set
 * ships with: one that aggregates scalars into a set
 * (`setUnion(quadbin)`), one that aggregates sets into a set
 * (`setUnion(quadbinset)`). Both share the same finalfn.
 ******************************************************************************/

-- The transition function is not STRICT
CREATE FUNCTION set_union_transfn(internal, quadbin)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION set_union_transfn(internal, quadbinset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION quadbinset_union_finalfn(internal)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(quadbin) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = quadbinset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE setUnion(quadbinset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = quadbinset_union_finalfn,
  PARALLEL = safe
);

/******************************************************************************/

/******************************************************************************
 * Set-theoretic operators
 *
 * The value-dim ordering operators (`<<`, `&<`, `>>`, `&>`) that other
 * `*set` types ship are intentionally NOT declared: QUADBIN cell ids have
 * no meaningful total order — the int64 comparison that the framework would
 * use for "strictly-left" queries has no spatial or hierarchical meaning
 * (same rationale as the bbox-operator pruning for `tquadbin`).
 *
 * All C implementations behind these operators (`Contains_set_*`,
 * `Overlaps_set_set`, `Union_*`, `Minus_*`, `Intersection_*`) are
 * type-generic — they dispatch on the operand's MeosType and route through
 * `datum_cmp` / `datum_eq` from `type_util.c`, where `T_QUADBIN` is already
 * wired.
 ******************************************************************************/

/******************************************************************************
 * contains @>
 ******************************************************************************/

CREATE FUNCTION contains(quadbinset, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(quadbinset, quadbinset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = quadbinset, RIGHTARG = quadbin,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = quadbinset, RIGHTARG = quadbinset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * contained by <@
 ******************************************************************************/

CREATE FUNCTION contained(quadbin, quadbinset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(quadbinset, quadbinset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = quadbin, RIGHTARG = quadbinset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = quadbinset, RIGHTARG = quadbinset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * overlaps &&
 ******************************************************************************/

CREATE FUNCTION overlaps(quadbinset, quadbinset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = quadbinset, RIGHTARG = quadbinset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * set union +
 ******************************************************************************/

CREATE FUNCTION setUnion(quadbin, quadbinset)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(quadbinset, quadbin)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(quadbinset, quadbinset)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = quadbin, RIGHTARG = quadbinset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = quadbinset, RIGHTARG = quadbin,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = quadbinset, RIGHTARG = quadbinset,
  COMMUTATOR = +
);

/******************************************************************************
 * set difference -
 ******************************************************************************/

CREATE FUNCTION setMinus(quadbin, quadbinset)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(quadbinset, quadbin)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(quadbinset, quadbinset)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = quadbin, RIGHTARG = quadbinset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = quadbinset, RIGHTARG = quadbin
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = quadbinset, RIGHTARG = quadbinset
);

/******************************************************************************
 * set intersection *
 ******************************************************************************/

CREATE FUNCTION setIntersection(quadbin, quadbinset)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(quadbinset, quadbin)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(quadbinset, quadbinset)
  RETURNS quadbinset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = quadbin, RIGHTARG = quadbinset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = quadbinset, RIGHTARG = quadbin,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = quadbinset, RIGHTARG = quadbinset,
  COMMUTATOR = *
);
