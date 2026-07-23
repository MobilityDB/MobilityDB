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
 * @brief Sets over pgpointcloud pcpoint / pcpatch base types.
 *
 * The pcpoint and pcpatch SQL types are owned by the upstream
 * pgpointcloud extension — MobilityDB reuses them the way it reuses
 * PostGIS `geometry`, not the way it defines its own types. This file
 * adds:
 *
 *   * a `pcid(pcpoint)` / `pcid(pcpatch)` SQL accessor, and
 *   * set types `pcpointset` / `pcpatchset` over those base values,
 *     with strict same-pcid enforcement on every constructor path.
 *
 * Most set-level SQL bindings delegate to the generic `Set_*` C wrappers
 * in `mobilitydb/src/temporal/set.c`, which dispatch on settype → base
 * type through the Oid→MeosType cache. No per-type C wrappers are
 * needed here beyond the pcid accessor.
 */

/******************************************************************************
 * pcid accessor — shared by pcpoint and pcpatch
 ******************************************************************************/

CREATE FUNCTION pcid(pcpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Pcpoint_pcid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION pcid(pcpatch)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Pcpatch_pcid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Schema-aware dimension getters for pcpoint
 *
 * Not STRICT: getZ returns NULL when the schema has no Z dimension;
 * getDim returns NULL on unknown dimension names. STABLE (not IMMUTABLE)
 * because the underlying schema lives in a PG catalog table that an
 * admin could theoretically ALTER mid-session; in practice it never
 * changes, but STABLE is the correct volatility label.
 ******************************************************************************/

CREATE FUNCTION getX(pcpoint)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Pcpoint_get_x'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getY(pcpoint)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Pcpoint_get_y'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getZ(pcpoint)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Pcpoint_get_z'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getDim(pcpoint, text)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Pcpoint_get_dim'
  LANGUAGE C STABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * pcpointset — Input / output
 ******************************************************************************/

CREATE TYPE pcpointset;

CREATE FUNCTION pcpointset_in(cstring)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpointset_out(pcpointset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpointset_recv(internal)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpointset_send(pcpointset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE pcpointset (
  internallength = variable,
  input = pcpointset_in,
  output = pcpointset_out,
  receive = pcpointset_recv,
  send = pcpointset_send,
  alignment = double,
  storage = extended
);

CREATE FUNCTION pcpointsetFromBinary(bytea)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpointsetFromHexWKB(text)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(pcpointset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(pcpointset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * pcpointset — Constructor / conversion
 ******************************************************************************/

CREATE FUNCTION set(pcpoint[])
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set(pcpoint)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pcpoint AS pcpointset) WITH FUNCTION set(pcpoint);

/******************************************************************************
 * pcpointset — Accessors
 ******************************************************************************/

CREATE FUNCTION memSize(pcpointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numValues(pcpointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(pcpointset)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(pcpointset)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueN(pcpointset, integer)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(pcpointset)
  RETURNS pcpoint[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION unnest(pcpointset)
  RETURNS SETOF pcpoint
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * pcpointset — Aggregate
 ******************************************************************************/

CREATE FUNCTION set_union_transfn(internal, pcpoint)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_union_transfn(internal, pcpointset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION pcpointset_union_finalfn(internal)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(pcpoint) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = pcpointset_union_finalfn
);
CREATE AGGREGATE setUnion(pcpointset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = pcpointset_union_finalfn
);

/******************************************************************************
 * pcpointset — Comparison / B-tree / hash
 ******************************************************************************/

CREATE FUNCTION eq(pcpointset, pcpointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(pcpointset, pcpointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(pcpointset, pcpointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(pcpointset, pcpointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(pcpointset, pcpointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(pcpointset, pcpointset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(pcpointset, pcpointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=
);

CREATE OPERATOR CLASS pcpointset_btree_ops
  DEFAULT FOR TYPE pcpointset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(pcpointset, pcpointset);

CREATE FUNCTION set_hash(pcpointset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash_extended(pcpointset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS pcpointset_hash_ops
  DEFAULT FOR TYPE pcpointset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   set_hash(pcpointset),
    FUNCTION    2   set_hash_extended(pcpointset, bigint);

/******************************************************************************
 * pcpointset — Set operations (value ↔ set, set ↔ set)
 ******************************************************************************/

CREATE FUNCTION contains(pcpointset, pcpoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(pcpointset, pcpointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = pcpointset, RIGHTARG = pcpoint,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  COMMUTATOR = <@
);

CREATE FUNCTION contained(pcpoint, pcpointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(pcpointset, pcpointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = pcpoint, RIGHTARG = pcpointset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  COMMUTATOR = @>
);

CREATE FUNCTION overlaps(pcpointset, pcpointset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  COMMUTATOR = &&
);

CREATE FUNCTION setUnion(pcpoint, pcpointset)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(pcpointset, pcpoint)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(pcpointset, pcpointset)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = pcpoint, RIGHTARG = pcpointset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = pcpointset, RIGHTARG = pcpoint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  COMMUTATOR = +
);

CREATE FUNCTION setMinus(pcpoint, pcpointset)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(pcpointset, pcpoint)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(pcpointset, pcpointset)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = pcpoint, RIGHTARG = pcpointset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = pcpointset, RIGHTARG = pcpoint
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = pcpointset, RIGHTARG = pcpointset
);

CREATE FUNCTION setIntersection(pcpoint, pcpointset)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(pcpointset, pcpoint)
  RETURNS pcpoint
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(pcpointset, pcpointset)
  RETURNS pcpointset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = pcpoint, RIGHTARG = pcpointset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = pcpointset, RIGHTARG = pcpoint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = pcpointset, RIGHTARG = pcpointset,
  COMMUTATOR = *
);

/******************************************************************************
 * pcpatchset — Input / output
 ******************************************************************************/

CREATE TYPE pcpatchset;

CREATE FUNCTION pcpatchset_in(cstring)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpatchset_out(pcpatchset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpatchset_recv(internal)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpatchset_send(pcpatchset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE pcpatchset (
  internallength = variable,
  input = pcpatchset_in,
  output = pcpatchset_out,
  receive = pcpatchset_recv,
  send = pcpatchset_send,
  alignment = double,
  storage = extended
);

CREATE FUNCTION pcpatchsetFromBinary(bytea)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION pcpatchsetFromHexWKB(text)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(pcpatchset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(pcpatchset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * pcpatchset — Constructor / conversion
 ******************************************************************************/

CREATE FUNCTION set(pcpatch[])
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set(pcpatch)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (pcpatch AS pcpatchset) WITH FUNCTION set(pcpatch);

/******************************************************************************
 * pcpatchset — Accessors
 ******************************************************************************/

CREATE FUNCTION memSize(pcpatchset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numValues(pcpatchset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(pcpatchset)
  RETURNS pcpatch
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(pcpatchset)
  RETURNS pcpatch
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueN(pcpatchset, integer)
  RETURNS pcpatch
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(pcpatchset)
  RETURNS pcpatch[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION unnest(pcpatchset)
  RETURNS SETOF pcpatch
  AS 'MODULE_PATHNAME', 'Set_unnest'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * pcpatchset — Aggregate
 ******************************************************************************/

CREATE FUNCTION set_union_transfn(internal, pcpatch)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Value_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION set_union_transfn(internal, pcpatchset)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_union_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION pcpatchset_union_finalfn(internal)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Set_union_finalfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE setUnion(pcpatch) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = pcpatchset_union_finalfn
);
CREATE AGGREGATE setUnion(pcpatchset) (
  SFUNC = set_union_transfn,
  STYPE = internal,
  FINALFUNC = pcpatchset_union_finalfn
);

/******************************************************************************
 * pcpatchset — Comparison / B-tree / hash
 ******************************************************************************/

CREATE FUNCTION eq(pcpatchset, pcpatchset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(pcpatchset, pcpatchset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(pcpatchset, pcpatchset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(pcpatchset, pcpatchset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(pcpatchset, pcpatchset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(pcpatchset, pcpatchset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(pcpatchset, pcpatchset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=
);
CREATE OPERATOR <= (
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >
);
CREATE OPERATOR >= (
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <
);
CREATE OPERATOR > (
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=
);

CREATE OPERATOR CLASS pcpatchset_btree_ops
  DEFAULT FOR TYPE pcpatchset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(pcpatchset, pcpatchset);

CREATE FUNCTION set_hash(pcpatchset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_hash_extended(pcpatchset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS pcpatchset_hash_ops
  DEFAULT FOR TYPE pcpatchset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   set_hash(pcpatchset),
    FUNCTION    2   set_hash_extended(pcpatchset, bigint);

/******************************************************************************
 * pcpatchset — Set operations
 ******************************************************************************/

CREATE FUNCTION contains(pcpatchset, pcpatch)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(pcpatchset, pcpatchset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = pcpatchset, RIGHTARG = pcpatch,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  COMMUTATOR = <@
);

CREATE FUNCTION contained(pcpatch, pcpatchset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(pcpatchset, pcpatchset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = pcpatch, RIGHTARG = pcpatchset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  COMMUTATOR = @>
);

CREATE FUNCTION overlaps(pcpatchset, pcpatchset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  COMMUTATOR = &&
);

CREATE FUNCTION setUnion(pcpatch, pcpatchset)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(pcpatchset, pcpatch)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(pcpatchset, pcpatchset)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = pcpatch, RIGHTARG = pcpatchset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = pcpatchset, RIGHTARG = pcpatch,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  COMMUTATOR = +
);

CREATE FUNCTION setMinus(pcpatch, pcpatchset)
  RETURNS pcpatch
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(pcpatchset, pcpatch)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(pcpatchset, pcpatchset)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = pcpatch, RIGHTARG = pcpatchset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = pcpatchset, RIGHTARG = pcpatch
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset
);

CREATE FUNCTION setIntersection(pcpatch, pcpatchset)
  RETURNS pcpatch
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(pcpatchset, pcpatch)
  RETURNS pcpatch
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(pcpatchset, pcpatchset)
  RETURNS pcpatchset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = pcpatch, RIGHTARG = pcpatchset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = pcpatchset, RIGHTARG = pcpatch,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = pcpatchset, RIGHTARG = pcpatchset,
  COMMUTATOR = *
);

/*****************************************************************************/
