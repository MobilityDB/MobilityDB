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
  storage = extended,
  analyze = span_analyze
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

CREATE FUNCTION set_eq(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ne(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_lt(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_le(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ge(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gt(quadbinset, quadbinset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_cmp(quadbinset, quadbinset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash(quadbinset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = set_eq, COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel, HASHES, MERGES);
CREATE OPERATOR <> (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = set_ne, COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel);
CREATE OPERATOR < (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = set_lt, COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel);
CREATE OPERATOR <= (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = set_le, COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel);
CREATE OPERATOR > (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = set_gt, COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel);
CREATE OPERATOR >= (LEFTARG = quadbinset, RIGHTARG = quadbinset,
  PROCEDURE = set_ge, COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel);

CREATE OPERATOR CLASS quadbinset_btree_ops
  DEFAULT FOR TYPE quadbinset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  set_cmp(quadbinset, quadbinset);

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
