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
 * @brief Static `quadbin` SQL type — the base type of `tquadbin`.
 *
 * Defines the static quadbin CARTO cell value type with parser /
 * output / send / receive, the six comparison operators, the btree
 * and hash operator classes, and the explicit (ASSIGNMENT) casts to
 * and from `bigint`.
 *
 * The on-disk representation is identical to `bigint`: a uint64
 * passed by value. The dedicated SQL type exists only to make
 * quadbin-specific functions type-safe — a user cannot accidentally
 * feed an arbitrary `bigint` to a function that expects a quadbin
 * cell, and vice-versa.
 */

/******************************************************************************
 * Type plumbing
 ******************************************************************************/

CREATE TYPE quadbin;

CREATE FUNCTION quadbin_in(cstring)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Quadbin_in'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION quadbin_out(quadbin)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Quadbin_out'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION quadbin_recv(internal)
  RETURNS quadbin
  AS 'MODULE_PATHNAME', 'Quadbin_recv'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION quadbin_send(quadbin)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Quadbin_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE quadbin (
  internallength = 8,
  input = quadbin_in,
  output = quadbin_out,
  receive = quadbin_recv,
  send = quadbin_send,
  passedbyvalue,
  alignment = double,
  storage = plain
);

/******************************************************************************
 * Casts to / from bigint
 *
 * ASSIGNMENT-only (matches the tquadbin ↔ tbigint design): the user
 * must spell out `::quadbin` or `::bigint` so an arbitrary int64
 * cannot silently flow into a quadbin-specific function. quadbin and
 * bigint share the same on-disk representation (both int64 passed
 * by value), so the casts are WITHOUT FUNCTION — same pattern as
 * `352_tquadbin.in.sql` uses for tquadbin ↔ tbigint.
 *
 * Note: bigint is a reserved word in SQL, so `CREATE FUNCTION
 * bigint(quadbin)` would be a syntax error. WITHOUT FUNCTION
 * sidesteps the need for a dedicated cast function here.
 ******************************************************************************/

CREATE CAST (bigint AS quadbin) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (quadbin AS bigint) WITHOUT FUNCTION AS ASSIGNMENT;

/******************************************************************************
 * Comparison operators
 *
 * Thin PG wrappers over the MEOS-layer `quadbin_eq / _lt / …` helpers
 * declared in `meos_quadbin.h`. quadbin cell equality and ordering are
 * exactly uint64 bit equality / ordering; putting the bodies at the
 * MEOS layer keeps the MobilityDB extension close to pure
 * boiler-plate and lets MobilityDuck consume the same primitives.
 ******************************************************************************/

CREATE FUNCTION quadbin_eq(quadbin, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_ne(quadbin, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_lt(quadbin, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_le(quadbin, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_gt(quadbin, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_ge(quadbin, quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_cmp(quadbin, quadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Quadbin_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION quadbin_hash(quadbin)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Quadbin_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = quadbin, RIGHTARG = quadbin,
  PROCEDURE = quadbin_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel,
  HASHES, MERGES
);

CREATE OPERATOR <> (
  LEFTARG = quadbin, RIGHTARG = quadbin,
  PROCEDURE = quadbin_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = quadbin, RIGHTARG = quadbin,
  PROCEDURE = quadbin_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
  LEFTARG = quadbin, RIGHTARG = quadbin,
  PROCEDURE = quadbin_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);

CREATE OPERATOR > (
  LEFTARG = quadbin, RIGHTARG = quadbin,
  PROCEDURE = quadbin_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
  LEFTARG = quadbin, RIGHTARG = quadbin,
  PROCEDURE = quadbin_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);

/******************************************************************************
 * btree + hash operator classes
 *
 * Required so users can build CREATE INDEX … USING btree(quadbin_col) for
 * exact-match lookups, plus distinct, GROUP BY, etc.
 ******************************************************************************/

CREATE OPERATOR CLASS quadbin_ops
  DEFAULT FOR TYPE quadbin USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  quadbin_cmp(quadbin, quadbin);

CREATE OPERATOR CLASS quadbin_ops
  DEFAULT FOR TYPE quadbin USING hash AS
    OPERATOR  1  =,
    FUNCTION  1  quadbin_hash(quadbin);

/******************************************************************************
 * Validity predicates
 *
 * Thin wrappers over the first-party kernel `quadbin_is_valid_*`
 * checks declared in `meos_quadbin.h`. The `(tquadbin)` temporal lift
 * lives in `357_tquadbin_ops.in.sql` — these two are the static
 * quadbin counterparts. `isValidIndex` accepts any CARTO quadbin
 * payload (including mode-0 header cells); `isValidCell` additionally
 * requires a populated cell at a concrete resolution.
 ******************************************************************************/

CREATE FUNCTION isValidIndex(quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_is_valid_index'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION isValidCell(quadbin)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Quadbin_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
