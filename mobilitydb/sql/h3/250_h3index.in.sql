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
 * @brief Static `h3index` SQL type — the base type of `th3index`.
 *
 * The h3index type itself, its parser / output / send / receive, the
 * six comparison operators, the btree and hash operator classes and
 * the casts to and from `bigint` all come from the **h3 PostgreSQL
 * extension** (h3-pg), which mobilitydb requires when built with
 * `-DH3=ON` (see `MOBILITYDB_REQUIRES` in the top-level CMakeLists).
 * Defining them here as well would collide on `CREATE EXTENSION`, so
 * this file only adds the surface h3-pg does not provide: the WKB and
 * HexWKB base-value I/O and the validity predicates.
 *
 * The on-disk representation is identical to `bigint`: an int64
 * passed by value. The dedicated SQL type exists only to make
 * h3-specific functions type-safe — a user cannot accidentally
 * feed an arbitrary `bigint` to a function that expects an H3
 * cell, and vice-versa.
 *
 * The four commented-out blocks below — type plumbing, the bigint
 * casts, the comparison functions and operators, and the btree and
 * hash operator classes — are RETAINED ON PURPOSE and are not dead
 * code to be cleaned up. Together they spell out a complete,
 * self-contained base-type surface, and they are kept here as the
 * WORKED REFERENCE for the SQL bindings that have no host h3
 * extension to lean on and must therefore provide the type, its
 * casts, its comparisons and its operator classes themselves.
 *
 * Which bindings those are follows one rule: a base type the host
 * already provides is DEFERRED to the host's own extension, and
 * emitted from the MEOS catalog where the host provides nothing.
 *   - PostgreSQL — h3-pg provides h3index  ⇒ DEFER (this file)
 *   - DuckDB     — duckdb-h3 provides it   ⇒ DEFER
 *   - MobilitySpark, Flink StreamSQL, NebulaStream — no h3 extension
 *     exists for them ⇒ they must carry the whole surface, shaped
 *     like these blocks
 *
 * The same rule decides the sibling cell families, and h3index is the
 * only one of the three it sends to DEFER — PostgreSQL ships no
 * quadbin or S2 extension, so `mobilitydb/sql/quadbin/350_quadbin.in.sql`
 * and `mobilitydb/sql/s2cell/600_s2cell.in.sql` define their types
 * outright. The three files differ only in which branch of the rule
 * they take, never in the rule.
 * Databricks' h3_* built-ins are proprietary and Apache Sedona ships
 * geometry-oriented ST_H3* helpers rather than an h3index scalar
 * type, so Spark genuinely has nothing to defer to.
 *
 * ⛔ The blocks are the SHAPE to reproduce, not source to paste
 * verbatim: a binding's surface is generated from the MEOS catalog
 * like any other family, and MEOS deliberately keeps the complete
 * h3index scalar surface because it is what those generators read.
 * Deferring is a host-side decision that never removes anything from
 * the kernel.
 *
 * Their PG C wrappers (`H3index_in`, `H3index_eq`, …) no longer exist
 * in `mobilitydb/src/h3/h3index.c`, so the blocks cannot be
 * uncommented as they stand — they are a reference, not switchable
 * code.
 *
 * A grep for `h3index_eq` or `h3index_cmp` in this file therefore
 * matches comment text, not live SQL: h3index is not a bare-naming
 * laggard, it has no live base-type comparison surface at all.
 */

/******************************************************************************
 * Type plumbing
 ******************************************************************************/

-- Provided by the h3 extension (h3-pg); NOT emitted by this extension.
-- ⛔ RETAINED AS A WORKED REFERENCE — not dead code, do not delete.
-- A host that ships no h3 extension must carry this surface itself; this block
-- is the reference for the TYPE and its I/O. See the file header.
/*
CREATE TYPE h3index;

CREATE FUNCTION h3index_in(cstring)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'H3index_in'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION h3index_out(h3index)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'H3index_out'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION h3index_recv(internal)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'H3index_recv'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION h3index_send(h3index)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'H3index_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE h3index (
  internallength = 8,
  input = h3index_in,
  output = h3index_out,
  receive = h3index_recv,
  send = h3index_send,
  passedbyvalue,
  alignment = double,
  storage = plain
);
*/

/******************************************************************************
 * WKB and HexWKB input/output
 *
 * Base-value WKB serialization. An h3index is a geographic cell with the
 * constant default SRID WGS84 (EPSG:4326), like a PostGIS geography:
 * asBinary/asHexWKB emit plain WKB without an embedded SRID (a reader
 * assumes 4326), mirroring the th3index temporal surface exactly. This lets
 * a downstream tool with no native spatial extension round-trip a cell
 * value through MEOS alone.
 ******************************************************************************/

CREATE FUNCTION h3indexFromBinary(bytea)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'H3index_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3indexFromHexWKB(text)
  RETURNS h3index
  AS 'MODULE_PATHNAME', 'H3index_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(h3index, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'H3index_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(h3index, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'H3index_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casts to / from bigint
 *
 * ASSIGNMENT-only (matches the th3index ↔ tbigint design): the user
 * must spell out `::h3index` or `::bigint` so an arbitrary int64
 * cannot silently flow into an H3-specific function. h3index and
 * bigint share the same on-disk representation (both int64 passed
 * by value), so the casts are WITHOUT FUNCTION — same pattern as
 * `253_th3index.in.sql` uses for th3index ↔ tbigint.
 *
 * Note: bigint is a reserved word in SQL, so `CREATE FUNCTION
 * bigint(h3index)` would be a syntax error. WITHOUT FUNCTION
 * sidesteps the need for a dedicated cast function here.
 ******************************************************************************/

-- Provided by the h3 extension (h3-pg); NOT emitted by this extension.
-- ⛔ RETAINED AS A WORKED REFERENCE — the bigint casts a host must provide
-- itself where no h3 extension provides them. See the file header.
-- CREATE CAST (bigint AS h3index) WITHOUT FUNCTION AS ASSIGNMENT;
-- CREATE CAST (h3index AS bigint) WITHOUT FUNCTION AS ASSIGNMENT;

/******************************************************************************
 * Comparison operators
 *
 * Thin PG wrappers over the MEOS-layer `h3index_eq / _lt / …` helpers
 * declared in `h3/h3index.h`. H3 cell equality and ordering are
 * exactly int64 bit equality / ordering; putting the bodies at the
 * MEOS layer keeps the MobilityDB extension close to pure
 * boiler-plate and lets MobilityDuck consume the same primitives.
 ******************************************************************************/

-- Provided by the h3 extension (h3-pg); NOT emitted by this extension.
-- ⛔ RETAINED AS A WORKED REFERENCE — the six comparison functions and
-- operators a host must provide. See the file header.
/*
CREATE FUNCTION h3index_eq(h3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3index_ne(h3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3index_lt(h3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3index_le(h3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3index_gt(h3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3index_ge(h3index, h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION h3index_cmp(h3index, h3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'H3index_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hash(h3index)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'H3index_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = h3index, RIGHTARG = h3index,
  PROCEDURE = h3index_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel,
  HASHES, MERGES
);

CREATE OPERATOR <> (
  LEFTARG = h3index, RIGHTARG = h3index,
  PROCEDURE = h3index_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = h3index, RIGHTARG = h3index,
  PROCEDURE = h3index_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE OPERATOR <= (
  LEFTARG = h3index, RIGHTARG = h3index,
  PROCEDURE = h3index_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);

CREATE OPERATOR > (
  LEFTARG = h3index, RIGHTARG = h3index,
  PROCEDURE = h3index_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR >= (
  LEFTARG = h3index, RIGHTARG = h3index,
  PROCEDURE = h3index_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
*/

/******************************************************************************
 * btree + hash operator classes
 *
 * Required so users can build CREATE INDEX … USING btree(h3_col) for
 * exact-match lookups, plus distinct, GROUP BY, etc.
 ******************************************************************************/

-- Provided by the h3 extension (h3-pg); NOT emitted by this extension.
-- ⛔ RETAINED AS A WORKED REFERENCE — the btree and hash operator classes a
-- host must provide so h3index is indexable. See the file header.
/*
CREATE OPERATOR CLASS h3index_ops
  DEFAULT FOR TYPE h3index USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  h3index_cmp(h3index, h3index);

CREATE OPERATOR CLASS h3index_ops
  DEFAULT FOR TYPE h3index USING hash AS
    OPERATOR  1  =,
    FUNCTION  1  hash(h3index);
*/

/******************************************************************************
 * Validity predicates
 *
 * Thin wrappers over libh3's isValid* checks exposed via the
 * `h3_*_meos` MEOS-layer functions from `h3_generated.h`. The
 * `(th3index)` temporal lifts live in the 210 / 213 / 214 files —
 * these three are the static h3index counterparts.
 ******************************************************************************/

CREATE FUNCTION isValidCell(h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_is_valid_cell'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION isValidDirectedEdge(h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_is_valid_directed_edge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION isValidVertex(h3index)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'H3index_is_valid_vertex'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
