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
 * @brief Type plumbing for `tquadbin`, a temporal type carrying CARTO
 * quadbin cell indices as a function of time.
 *
 * On-disk representation is the same Temporal structure used by
 * every other temporal type; the basetype is the dedicated
 * `quadbin`, not `int8`, and the catalog entry
 * `{T_TQUADBIN, T_QUADBIN}` drives dispatch. tquadbin is classified
 * as a planar `tspatial_type` — quadbin cells are Web-Mercator
 * squares whose lon/lat geometry lives in SRID 4326, so the bounding
 * box is a planar `stbox` (X/Y + T dimensions, GEODETIC flag clear),
 * matching `tgeompoint`.
 *
 * Casts to / from `tbigint` are ASSIGNMENT-only — the user must
 * spell out the cast, mistakes surface as a clear binder error
 * rather than silently reinterpreting an arbitrary int64
 * trajectory as a stream of quadbin cells. The two types share the
 * int64 payload but carry different bbox shapes, so the casts go
 * through a real function — see below.
 */

CREATE TYPE tquadbin;

/******************************************************************************
 * Input / Output
 ******************************************************************************/

CREATE FUNCTION tquadbin_in(cstring, oid, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tquadbin_recv(internal, oid, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(tquadbin)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Temporal_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(tquadbin)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Temporal_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tquadbin (
  internallength = variable,
  input = tquadbin_in,
  output = temporal_out,
  send = temporal_send,
  receive = tquadbin_recv,
  typmod_in = temporal_typmod_in,
  typmod_out = temporal_typmod_out,
  storage = extended,
  alignment = double,
  analyze = temporal_analyze
);

/******************************************************************************
 * Typmod enforcer + self-cast (mirrors tbigint / tint / tfloat / ttext)
 ******************************************************************************/

CREATE FUNCTION tquadbin(tquadbin, integer)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Temporal_enforce_typmod'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tquadbin AS tquadbin) WITH FUNCTION tquadbin(tquadbin, integer)
  AS IMPLICIT;

/******************************************************************************
 * Explicit assignment casts to / from tbigint
 *
 * The int64 payload is shared, but tquadbin sequences carry a planar
 * STBox bbox while tbigint sequences carry a TBox; a binary coercion
 * would leave the wrong bbox shape in place and the wrong temptype byte
 * inside the Temporal header. The cast therefore goes through a real
 * function that lifts an identity Datum function so the result is
 * rebuilt at the correct shape. The cast is declared `AS ASSIGNMENT`
 * so the user must spell out `::tquadbin` or `::tbigint` — mistakes
 * surface as a clear binder error rather than silently reinterpreting
 * an arbitrary int64 trajectory as a stream of quadbin cells.
 ******************************************************************************/

CREATE FUNCTION tquadbin(tbigint)
  RETURNS tquadbin
  AS 'MODULE_PATHNAME', 'Tbigint_to_tquadbin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbigint(tquadbin)
  RETURNS tbigint
  AS 'MODULE_PATHNAME', 'Tquadbin_to_tbigint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbigint AS tquadbin) WITH FUNCTION tquadbin(tbigint) AS ASSIGNMENT;
CREATE CAST (tquadbin AS tbigint) WITH FUNCTION tbigint(tquadbin) AS ASSIGNMENT;

/******************************************************************************
 * Comparison functions and B-tree / hash indexing
 *
 * All tquadbin values share the exact same on-disk layout as every
 * other temporal type, so the generic Temporal_* dispatch in the
 * backend is enough — no tquadbin-specific wrappers required.
 ******************************************************************************/

CREATE FUNCTION temporal_lt(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tquadbin, tquadbin)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Temporal_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_cmp(tquadbin, tquadbin)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Temporal_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = temporal_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = temporal_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = temporal_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = temporal_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = temporal_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
  LEFTARG = tquadbin, RIGHTARG = tquadbin,
  PROCEDURE = temporal_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tquadbin_btree_ops
  DEFAULT FOR TYPE tquadbin USING btree AS
    OPERATOR  1 <,
    OPERATOR  2 <=,
    OPERATOR  3 =,
    OPERATOR  4 >=,
    OPERATOR  5 >,
    FUNCTION  1 temporal_cmp(tquadbin, tquadbin);

/******************************************************************************/

CREATE FUNCTION temporal_hash(tquadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Temporal_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tquadbin_hash_ops
  DEFAULT FOR TYPE tquadbin USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   temporal_hash(tquadbin);

/******************************************************************************/
