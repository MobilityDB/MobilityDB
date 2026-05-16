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
 * @brief `h3indexset` SQL type — set of h3index values, mirrors
 * bigintset's structure with the basetype changed to h3index.
 *
 * Every C call routes to a generic `Set_*` symbol. The dispatch
 * arms in `type_in.c` / `type_out.c` (basetype_in / basetype_out
 * cases for T_H3INDEX) make the generic Set parser and formatter
 * use h3index_parse / h3index_to_string for elements.
 */

/******************************************************************************
 * Type plumbing
 ******************************************************************************/

CREATE TYPE h3indexset;

CREATE FUNCTION h3indexset_in(cstring)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3indexset_out(h3indexset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3indexset_recv(internal)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3indexset_send(h3indexset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE h3indexset (
  internallength = variable,
  input = h3indexset_in,
  output = h3indexset_out,
  receive = h3indexset_recv,
  send = h3indexset_send,
  alignment = double,
  storage = extended,
  analyze = span_analyze
);

/******************************************************************************
 * WKB / HexWKB helpers
 ******************************************************************************/

CREATE FUNCTION h3indexsetFromBinary(bytea)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3indexsetFromHexWKB(text)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(h3indexset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(h3indexset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(h3indexset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION set(h3index[])
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Singleton constructor: `set(basetype)` is the cross-set convention
-- (matches set(bigint), set(text), etc.).
CREATE FUNCTION set(h3index)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (h3index AS h3indexset) WITH FUNCTION set(h3index);

/******************************************************************************
 * Accessors
 ******************************************************************************/

CREATE FUNCTION memSize(h3indexset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(h3indexset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(h3indexset)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(h3indexset)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(h3indexset, integer)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(h3indexset)
  RETURNS h3index[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison operators + btree / hash opclasses
 ******************************************************************************/

CREATE FUNCTION set_eq(h3indexset, h3indexset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ne(h3indexset, h3indexset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_lt(h3indexset, h3indexset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_le(h3indexset, h3indexset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_ge(h3indexset, h3indexset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gt(h3indexset, h3indexset)
  RETURNS boolean AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_cmp(h3indexset, h3indexset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash(h3indexset)
  RETURNS integer AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (LEFTARG = h3indexset, RIGHTARG = h3indexset,
  PROCEDURE = set_eq, COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel, HASHES, MERGES);
CREATE OPERATOR <> (LEFTARG = h3indexset, RIGHTARG = h3indexset,
  PROCEDURE = set_ne, COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel);
CREATE OPERATOR < (LEFTARG = h3indexset, RIGHTARG = h3indexset,
  PROCEDURE = set_lt, COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel);
CREATE OPERATOR <= (LEFTARG = h3indexset, RIGHTARG = h3indexset,
  PROCEDURE = set_le, COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel);
CREATE OPERATOR > (LEFTARG = h3indexset, RIGHTARG = h3indexset,
  PROCEDURE = set_gt, COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel);
CREATE OPERATOR >= (LEFTARG = h3indexset, RIGHTARG = h3indexset,
  PROCEDURE = set_ge, COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel);

CREATE OPERATOR CLASS h3indexset_btree_ops
  DEFAULT FOR TYPE h3indexset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  set_cmp(h3indexset, h3indexset);

CREATE OPERATOR CLASS h3indexset_hash_ops
  DEFAULT FOR TYPE h3indexset USING hash AS
    OPERATOR  1  =,
    FUNCTION  1  set_hash(h3indexset);

/******************************************************************************
 * unnest — SETOF expansion
 ******************************************************************************/

CREATE FUNCTION unnest(h3indexset)
  RETURNS SETOF h3index
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * setUnion aggregate
 *
 * Two aggregate overloads, matching the pattern every other *set
 * ships with: one that aggregates scalars into a set
 * (`setUnion(h3index)`), one that aggregates sets into a set
 * (`setUnion(h3indexset)`). Both share the same finalfn.
 ******************************************************************************/

-- The transition function is not STRICT
CREATE FUNCTION set_union_transfn(internal, h3index)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION set_union_transfn(internal, h3indexset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION h3indexset_union_finalfn(internal)
  RETURNS h3indexset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(h3index) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = h3indexset_union_finalfn,
  PARALLEL = safe
);

CREATE AGGREGATE setUnion(h3indexset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = h3indexset_union_finalfn,
  PARALLEL = safe
);

/******************************************************************************/
