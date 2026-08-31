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
 * @brief `s2cellset` SQL type — set of s2cell values, mirrors
 * the bigintset structure over the s2cell base type.
 *
 * Every C call routes to a generic `Set_*` symbol. The dispatch
 * arms in `type_in.c` / `type_out.c` (the basetype_in and
 * basetype_out cases for T_S2CELL) make the generic Set parser
 * and formatter read and write an element with s2cell_parse and
 * s2cell_out.
 */

/******************************************************************************
 * Type plumbing
 ******************************************************************************/

CREATE TYPE s2cellset;

CREATE FUNCTION s2cellset_in(cstring)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cellset_out(s2cellset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cellset_recv(internal)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cellset_send(s2cellset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE s2cellset (
  internallength = variable,
  input = s2cellset_in,
  output = s2cellset_out,
  receive = s2cellset_recv,
  send = s2cellset_send,
  alignment = double,
  storage = extended
  -- , analyze = geoset_analyze
);

/******************************************************************************
 * WKB / HexWKB helpers
 ******************************************************************************/

CREATE FUNCTION s2cellsetFromBinary(bytea)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cellsetFromHexWKB(text)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(s2cellset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(s2cellset, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(s2cellset, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION set(s2cell[])
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Singleton constructor: `set(basetype)` is the cross-set convention
-- (matches set(bigint), set(text), etc.).
CREATE FUNCTION set(s2cell)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (s2cell AS s2cellset) WITH FUNCTION set(s2cell);

/******************************************************************************
 * Accessors
 ******************************************************************************/

CREATE FUNCTION memSize(s2cellset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(s2cellset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(s2cellset)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(s2cellset)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(s2cellset, integer)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(s2cellset)
  RETURNS s2cell[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison operators + btree / hash opclasses
 ******************************************************************************/

CREATE FUNCTION eq(s2cellset, s2cellset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(s2cellset, s2cellset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(s2cellset, s2cellset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(s2cellset, s2cellset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(s2cellset, s2cellset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(s2cellset, s2cellset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(s2cellset, s2cellset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hash(s2cellset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hashExtended(s2cellset, bigint)
  RETURNS bigint AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (LEFTARG = s2cellset, RIGHTARG = s2cellset,
  PROCEDURE = eq, COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel, HASHES, MERGES);
CREATE OPERATOR <> (LEFTARG = s2cellset, RIGHTARG = s2cellset,
  PROCEDURE = ne, COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel);
CREATE OPERATOR < (LEFTARG = s2cellset, RIGHTARG = s2cellset,
  PROCEDURE = lt, COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel);
CREATE OPERATOR <= (LEFTARG = s2cellset, RIGHTARG = s2cellset,
  PROCEDURE = le, COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel);
CREATE OPERATOR > (LEFTARG = s2cellset, RIGHTARG = s2cellset,
  PROCEDURE = gt, COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel);
CREATE OPERATOR >= (LEFTARG = s2cellset, RIGHTARG = s2cellset,
  PROCEDURE = ge, COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel);

CREATE OPERATOR CLASS s2cellset_btree_ops
  DEFAULT FOR TYPE s2cellset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(s2cellset, s2cellset);

CREATE OPERATOR CLASS s2cellset_hash_ops
  DEFAULT FOR TYPE s2cellset USING hash AS
    OPERATOR  1  =,
    FUNCTION  1  hash(s2cellset),
    FUNCTION  2  hashExtended(s2cellset, bigint);

/******************************************************************************
 * unnest — SETOF expansion
 ******************************************************************************/

CREATE FUNCTION unnest(s2cellset)
  RETURNS SETOF s2cell
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * setUnion aggregate
 *
 * Two aggregate overloads, matching the pattern every other *set
 * ships with: one that aggregates scalars into a set
 * (`setUnion(s2cell)`), one that aggregates sets into a set
 * (`setUnion(s2cellset)`). Both share the same finalfn.
 ******************************************************************************/

-- The transition function is not STRICT
CREATE FUNCTION set_union_transfn(internal, s2cell)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION set_union_transfn(internal, s2cellset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION s2cellset_union_finalfn(internal)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(s2cell) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
  FINALFUNC = s2cellset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE setUnion(s2cellset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  COMBINEFUNC = array_agg_combine,
  SERIALFUNC = array_agg_serialize,
  DESERIALFUNC = array_agg_deserialize,
  FINALFUNC = s2cellset_union_finalfn,
  PARALLEL = safe
);

/******************************************************************************/

/******************************************************************************
 * Set-theoretic operators
 *
 * The value-dim ordering operators (`<<`, `&<`, `>>`, `&>`) that other
 * `*set` types ship are intentionally NOT declared: S2 cell ids have
 * no meaningful total order — the int64 comparison that the framework would
 * use for "strictly-left" queries has no spatial or hierarchical meaning
 * (same rationale as the bbox-operator pruning for `ts2cell`).
 *
 * All C implementations behind these operators (`Contains_set_*`,
 * `Overlaps_set_set`, `Union_*`, `Minus_*`, `Intersection_*`) are
 * type-generic — they dispatch on the operand's MeosType and route through
 * `datum_cmp` / `datum_eq` from `type_util.c`, where `T_S2` is already
 * wired.
 ******************************************************************************/

/******************************************************************************
 * contains @>
 ******************************************************************************/

CREATE FUNCTION contains(s2cellset, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(s2cellset, s2cellset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = s2cellset, RIGHTARG = s2cell,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = s2cellset, RIGHTARG = s2cellset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * contained by <@
 ******************************************************************************/

CREATE FUNCTION contained(s2cell, s2cellset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(s2cellset, s2cellset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = s2cell, RIGHTARG = s2cellset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = s2cellset, RIGHTARG = s2cellset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * overlaps &&
 ******************************************************************************/

CREATE FUNCTION overlaps(s2cellset, s2cellset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = s2cellset, RIGHTARG = s2cellset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * Set union +
 ******************************************************************************/

CREATE FUNCTION setUnion(s2cell, s2cellset)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(s2cellset, s2cell)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(s2cellset, s2cellset)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = s2cell, RIGHTARG = s2cellset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = s2cellset, RIGHTARG = s2cell,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = s2cellset, RIGHTARG = s2cellset,
  COMMUTATOR = +
);

/******************************************************************************
 * Set difference -
 ******************************************************************************/

CREATE FUNCTION setMinus(s2cell, s2cellset)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(s2cellset, s2cell)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(s2cellset, s2cellset)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = s2cell, RIGHTARG = s2cellset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = s2cellset, RIGHTARG = s2cell
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = s2cellset, RIGHTARG = s2cellset
);

/******************************************************************************
 * Set intersection *
 ******************************************************************************/

CREATE FUNCTION setIntersection(s2cell, s2cellset)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(s2cellset, s2cell)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(s2cellset, s2cellset)
  RETURNS s2cellset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = s2cell, RIGHTARG = s2cellset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = s2cellset, RIGHTARG = s2cell,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = s2cellset, RIGHTARG = s2cellset,
  COMMUTATOR = *
);
