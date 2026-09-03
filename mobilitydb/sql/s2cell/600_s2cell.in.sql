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
 * @brief Static `s2cell` SQL type — the base type of `ts2cell`.
 *
 * Defines the static Google S2 cell value type with its parser, output, send
 * and receive, the six comparison operators, the btree and hash operator
 * classes, and the explicit casts to and from `bigint`.
 *
 * The on-disk representation is that of `bigint`: a uint64 passed by value.
 * The dedicated SQL type exists so that S2 functions are type-safe — an
 * arbitrary `bigint` cannot reach a function expecting an S2 cell, nor the
 * reverse.
 *
 * WHY THIS FILE DEFINES THE TYPE — one rule, shared by the three cell
 * families: a base type the host database already provides is DEFERRED to the
 * host's own extension, and defined here where the host provides nothing.
 * PostgreSQL ships no S2 extension, so this file carries the whole surface.
 *   - `s2cell`   — no host extension ⇒ defined here
 *   - `quadbin`  — no host extension ⇒ defined in
 *                  `mobilitydb/sql/quadbin/350_quadbin.in.sql`
 *   - `h3index`  — h3-pg provides it ⇒ DEFERRED, and the equivalent blocks in
 *                  `mobilitydb/sql/h3/250_h3index.in.sql` are commented out,
 *                  since a second definition would collide on
 *                  `CREATE EXTENSION`
 */

/******************************************************************************
 * Type plumbing
 ******************************************************************************/

CREATE TYPE s2cell;

CREATE FUNCTION s2cell_in(cstring)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cell_out(s2cell)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'S2cell_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cell_recv(internal)
  RETURNS s2cell
  AS 'MODULE_PATHNAME', 'S2cell_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION s2cell_send(s2cell)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'S2cell_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE s2cell (
  internallength = 8,
  input = s2cell_in,
  output = s2cell_out,
  receive = s2cell_recv,
  send = s2cell_send,
  passedbyvalue,
  alignment = double,
  storage = plain
);

/******************************************************************************
 * Casts to and from bigint
 *
 * ASSIGNMENT-only, as the ts2cell to tbigint casts are: the query states
 * `::s2cell` or `::bigint`, so an arbitrary int64 cannot flow silently into a
 * function that expects a cell. An S2 cell and a bigint share one on-disk
 * representation, both int64 passed by value, so the casts carry no function.
 *
 * `bigint` is a reserved word, so `CREATE FUNCTION bigint(s2cell)` is a syntax
 * error; WITHOUT FUNCTION needs no cast function at all.
 ******************************************************************************/

CREATE CAST (bigint AS s2cell) WITHOUT FUNCTION AS ASSIGNMENT;
CREATE CAST (s2cell AS bigint) WITHOUT FUNCTION AS ASSIGNMENT;

/******************************************************************************
 * Comparison operators
 *
 * Thin wrappers over the MEOS-layer `s2cell_eq / _lt / …` helpers declared in
 * `meos_s2cell.h`. Equality and ordering of S2 cells are bit equality and
 * ordering of the uint64 payload, and that order follows the Hilbert curve, so
 * two cells close in value are close on the sphere.
 ******************************************************************************/

CREATE FUNCTION eq(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ne(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lt(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION le(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gt(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ge(s2cell, s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cmp(s2cell, s2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'S2cell_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hash(s2cell)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'S2cell_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hashExtended(s2cell, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'S2cell_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = s2cell, RIGHTARG = s2cell,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel,
  HASHES, MERGES
);

CREATE OPERATOR <> (
  LEFTARG = s2cell, RIGHTARG = s2cell,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = s2cell, RIGHTARG = s2cell,
  PROCEDURE = lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
  LEFTARG = s2cell, RIGHTARG = s2cell,
  PROCEDURE = le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);

CREATE OPERATOR > (
  LEFTARG = s2cell, RIGHTARG = s2cell,
  PROCEDURE = gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
  LEFTARG = s2cell, RIGHTARG = s2cell,
  PROCEDURE = ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);

/******************************************************************************
 * btree and hash operator classes
 *
 * What an exact-match index, DISTINCT and GROUP BY need over a cell column.
 ******************************************************************************/

CREATE OPERATOR CLASS s2cell_ops
  DEFAULT FOR TYPE s2cell USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  cmp(s2cell, s2cell);

CREATE OPERATOR CLASS s2cell_ops
  DEFAULT FOR TYPE s2cell USING hash AS
    OPERATOR  1  =,
    FUNCTION  1  hash(s2cell),
    FUNCTION  2  hashExtended(s2cell, bigint);

/******************************************************************************
 * Validity
 *
 * Every S2 identifier that is valid at all denotes a cell — S2 carries no
 * counterpart of the QUADBIN mode field — so validity is one predicate.
 ******************************************************************************/

CREATE FUNCTION isValidCell(s2cell)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'S2cell_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
