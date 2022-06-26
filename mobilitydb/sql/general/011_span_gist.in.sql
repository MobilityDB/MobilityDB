/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

/*
 * span_gist.sql
 * R-tree GiST index for span types
 */

CREATE FUNCTION span_gist_union(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_union'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_penalty(internal, internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_penalty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_picksplit(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(intspan, intspan, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_fetch(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_fetch'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_consistent(internal, intspan, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS intspan_rtree_ops
  DEFAULT FOR TYPE intspan USING gist AS
  -- overlaps
  OPERATOR  3     && (intspan, intspan),
  -- contains
  OPERATOR  7     @> (intspan, int),
  OPERATOR  7     @> (intspan, intspan),
  -- contained by
  OPERATOR  8     <@ (intspan, intspan),
  -- adjacent
  OPERATOR  17    -|- (intspan, intspan),
  -- equals
  OPERATOR  18    = (intspan, intspan),
#if POSTGRESQL_VERSION_NUMBER >= 120000
  -- nearest approach distance
  OPERATOR  25    <-> (intspan, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspan) FOR ORDER BY pg_catalog.float_ops,
#endif //POSTGRESQL_VERSION_NUMBER >= 120000
  -- overlaps or before
  OPERATOR  28    &< (intspan, int),
  OPERATOR  28    &< (intspan, intspan),
  -- strictly before
  OPERATOR  29    << (intspan, int),
  OPERATOR  29    << (intspan, intspan),
  -- strictly after
  OPERATOR  30    >> (intspan, int),
  OPERATOR  30    >> (intspan, intspan),
  -- overlaps or after
  OPERATOR  31    &> (intspan, int),
  OPERATOR  31    &> (intspan, intspan),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, intspan, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(intspan, intspan, internal),
  FUNCTION  9  span_gist_fetch(internal);

/******************************************************************************/

CREATE FUNCTION span_gist_same(floatspan, floatspan, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_consistent(internal, floatspan, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS floatspan_rtree_ops
  DEFAULT FOR TYPE floatspan USING gist AS
  -- overlaps
  OPERATOR  3     && (floatspan, floatspan),
  -- contains
  OPERATOR  7     @> (floatspan, float),
  OPERATOR  7     @> (floatspan, floatspan),
  -- contained by
  OPERATOR  8     <@ (floatspan, floatspan),
  -- adjacent
  OPERATOR  17    -|- (floatspan, floatspan),
  -- equals
  OPERATOR  18    = (floatspan, floatspan),
#if POSTGRESQL_VERSION_NUMBER >= 120000
  -- nearest approach distance
  OPERATOR  25    <-> (floatspan, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspan) FOR ORDER BY pg_catalog.float_ops,
#endif //POSTGRESQL_VERSION_NUMBER >= 120000
  -- overlaps or before
  OPERATOR  28    &< (floatspan, float),
  OPERATOR  28    &< (floatspan, floatspan),
  -- strictly before
  OPERATOR  29    << (floatspan, float),
  OPERATOR  29    << (floatspan, floatspan),
  -- strictly after
  OPERATOR  30    >> (floatspan, float),
  OPERATOR  30    >> (floatspan, floatspan),
  -- overlaps or after
  OPERATOR  31    &> (floatspan, float),
  OPERATOR  31    &> (floatspan, floatspan),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, floatspan, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(floatspan, floatspan, internal),
  FUNCTION  9  span_gist_fetch(internal);

/******************************************************************************/

