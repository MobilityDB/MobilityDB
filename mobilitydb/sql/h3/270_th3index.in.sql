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
 * @brief Type plumbing for `th3index`, a temporal type carrying H3 cell
 * indices as a function of time.
 *
 * On-disk representation is the same Temporal structure used by
 * every other temporal type; the basetype is the dedicated
 * `h3index`, not `int8`, and the catalog entry
 * `{T_TH3INDEX, T_H3INDEX}` drives dispatch. th3index is
 * classified as a `tspatial_type` and `tgeodetic_type` — H3 cells
 * are geographic hexagons on the WGS84 sphere, always geodetic and
 * always lat/lon, so the bounding box is a geodetic `stbox` (X/Y +
 * T dimensions, GEODETIC flag set), matching `tgeogpoint`.
 *
 * Casts to / from `tbigint` are ASSIGNMENT-only — the user must
 * spell out the cast, mistakes surface as a clear binder error
 * rather than silently reinterpreting an arbitrary int64
 * trajectory as a stream of H3 cells. The two types are
 * binary-coercible (the int64 payload is identical) so the casts
 * are `WITHOUT FUNCTION` — see below.
 */

CREATE TYPE th3index;

/******************************************************************************
 * Input / Output
 ******************************************************************************/

CREATE FUNCTION th3index_in(cstring, oid, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION th3index_recv(internal, oid, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(th3index)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(th3index)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE th3index (
  internallength = variable,
  input = th3index_in,
  output = temporal_out,
  send = temporal_send,
  receive = th3index_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = temporal_analyze
);

/******************************************************************************
 * Typmod enforcer + self-cast (mirrors tbigint / tint / tfloat / ttext)
 ******************************************************************************/

CREATE FUNCTION th3index(th3index, integer)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (th3index AS th3index) WITH FUNCTION th3index(th3index, integer)
  AS IMPLICIT;

/******************************************************************************
 * Explicit assignment casts to / from tbigint
 *
 * The int64 payload is shared, but th3index sequences carry a geodetic
 * STBox bbox while tbigint sequences carry a TBox; a binary coercion
 * would leave the wrong bbox shape in place and the wrong temptype byte
 * inside the Temporal header. The cast therefore goes through a real
 * function that lifts an identity Datum function so the result is
 * rebuilt at the correct shape. The cast is declared `AS ASSIGNMENT`
 * so the user must spell out `::th3index` or `::tbigint` — mistakes
 * surface as a clear binder error rather than silently reinterpreting
 * an arbitrary int64 trajectory as a stream of H3 cells.
 ******************************************************************************/

CREATE FUNCTION th3index(tbigint)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Tbigint_to_th3index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbigint(th3index)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Th3index_to_tbigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbigint AS th3index) WITH FUNCTION th3index(tbigint) AS ASSIGNMENT;
CREATE CAST (th3index AS tbigint) WITH FUNCTION tbigint(th3index) AS ASSIGNMENT;

/******************************************************************************
 * Comparison functions and B-tree / hash indexing
 *
 * All th3index values share the exact same on-disk layout as every
 * other temporal type, so the generic Temporal_* dispatch in the
 * backend is enough — no th3index-specific wrappers required.
 ******************************************************************************/

CREATE FUNCTION temporal_lt(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(th3index, th3index)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_cmp(th3index, th3index)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = th3index, RIGHTARG = th3index,
  PROCEDURE = temporal_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS th3index_btree_ops
  DEFAULT FOR TYPE th3index USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 temporal_cmp(th3index, th3index);

/******************************************************************************/

CREATE FUNCTION temporal_hash(th3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS th3index_hash_ops
  DEFAULT FOR TYPE th3index USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(th3index);

/******************************************************************************/

CREATE FUNCTION arrowRoundtrip(th3index)
  RETURNS th3index
  AS 'MODULE_PATHNAME', 'Temporal_arrow_roundtrip'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
