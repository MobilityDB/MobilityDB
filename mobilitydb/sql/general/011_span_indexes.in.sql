/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * R-tree GiST, Quad-tree SP-GiST, and Kd-tree SP-GiST indexes for span types
 */

/******************************************************************************
 * R-tree GiST indexes
 ******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, intspan, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
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
CREATE FUNCTION span_gist_distance(internal, intspan, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_fetch(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_fetch'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intspan_rtree_ops
  DEFAULT FOR TYPE intspan USING gist AS
  -- strictly left
  OPERATOR  1     << (intspan, int),
  OPERATOR  1     << (intspan, intspan),
  OPERATOR  1     << (intspan, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspan, int),
  OPERATOR  2     &< (intspan, intspan),
  OPERATOR  2     &< (intspan, intspanset),
  -- overlaps
  OPERATOR  3     && (intspan, intspan),
  OPERATOR  3     && (intspan, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspan, int),
  OPERATOR  4     &> (intspan, intspan),
  OPERATOR  4     &> (intspan, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspan, int),
  OPERATOR  5     >> (intspan, intspan),
  OPERATOR  5     >> (intspan, intspanset),
  -- contains
  OPERATOR  7     @> (intspan, int),
  OPERATOR  7     @> (intspan, intspan),
  OPERATOR  7     @> (intspan, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspan, intspan),
  OPERATOR  8     <@ (intspan, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspan, intspan),
  OPERATOR  17    -|- (intspan, intspanset),
  -- equals
  OPERATOR  18    = (intspan, intspan),
  -- nearest approach distance
  OPERATOR  25    <-> (intspan, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, intspan, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(intspan, intspan, internal),
  FUNCTION  8  span_gist_distance(internal, intspan, smallint, oid, internal),
  FUNCTION  9  span_gist_fetch(internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, bigintspan, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(bigintspan, bigintspan, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, bigintspan, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS bigintspan_rtree_ops
  DEFAULT FOR TYPE bigintspan USING gist AS
  -- strictly left
  OPERATOR  1     << (bigintspan, bigint),
  OPERATOR  1     << (bigintspan, bigintspan),
  OPERATOR  1     << (bigintspan, bigintspanset),
  -- overlaps or left
  OPERATOR  2     &< (bigintspan, bigint),
  OPERATOR  2     &< (bigintspan, bigintspan),
  OPERATOR  2     &< (bigintspan, bigintspanset),
  -- overlaps
  OPERATOR  3     && (bigintspan, bigintspan),
  OPERATOR  3     && (bigintspan, bigintspanset),
  -- overlaps or right
  OPERATOR  4     &> (bigintspan, bigint),
  OPERATOR  4     &> (bigintspan, bigintspan),
  OPERATOR  4     &> (bigintspan, bigintspanset),
  -- strictly right
  OPERATOR  5     >> (bigintspan, bigint),
  OPERATOR  5     >> (bigintspan, bigintspan),
  OPERATOR  5     >> (bigintspan, bigintspanset),
  -- contains
  OPERATOR  7     @> (bigintspan, bigint),
  OPERATOR  7     @> (bigintspan, bigintspan),
  OPERATOR  7     @> (bigintspan, bigintspanset),
  -- contained by
  OPERATOR  8     <@ (bigintspan, bigintspan),
  OPERATOR  8     <@ (bigintspan, bigintspanset),
  -- adjacent
  OPERATOR  17    -|- (bigintspan, bigintspan),
  OPERATOR  17    -|- (bigintspan, bigintspanset),
  -- equals
  OPERATOR  18    = (bigintspan, bigintspan),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintspan, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspan, bigintspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspan, bigintspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, bigintspan, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(bigintspan, bigintspan, internal),
  FUNCTION  8  span_gist_distance(internal, bigintspan, smallint, oid, internal),
  FUNCTION  9  span_gist_fetch(internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, floatspan, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(floatspan, floatspan, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, floatspan, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS floatspan_rtree_ops
  DEFAULT FOR TYPE floatspan USING gist AS
  -- strictly left
  OPERATOR  1     << (floatspan, float),
  OPERATOR  1     << (floatspan, floatspan),
  OPERATOR  1     << (floatspan, floatspanset),
  -- overlaps or left
  OPERATOR  2     &< (floatspan, float),
  OPERATOR  2     &< (floatspan, floatspan),
  OPERATOR  2     &< (floatspan, floatspanset),
  -- overlaps
  OPERATOR  3     && (floatspan, floatspan),
  OPERATOR  3     && (floatspan, floatspanset),
  -- overlaps or right
  OPERATOR  4     &> (floatspan, float),
  OPERATOR  4     &> (floatspan, floatspan),
  OPERATOR  4     &> (floatspan, floatspanset),
  -- strictly right
  OPERATOR  5     >> (floatspan, float),
  OPERATOR  5     >> (floatspan, floatspan),
  OPERATOR  5     >> (floatspan, floatspanset),
  -- contains
  OPERATOR  7     @> (floatspan, float),
  OPERATOR  7     @> (floatspan, floatspan),
  OPERATOR  7     @> (floatspan, floatspanset),
  -- contained by
  OPERATOR  8     <@ (floatspan, floatspan),
  OPERATOR  8     <@ (floatspan, floatspanset),
  -- adjacent
  OPERATOR  17    -|- (floatspan, floatspan),
  OPERATOR  17    -|- (floatspan, floatspanset),
  -- equals
  OPERATOR  18    = (floatspan, floatspan),
  -- nearest approach distance
  OPERATOR  25    <-> (floatspan, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, floatspan, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(floatspan, floatspan, internal),
  FUNCTION  8  span_gist_distance(internal, floatspan, smallint, oid, internal),
  FUNCTION  9  span_gist_fetch(internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, tstzspan, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(tstzspan, tstzspan, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS period_rtree_ops
  DEFAULT FOR TYPE tstzspan USING gist AS
  -- overlaps
  OPERATOR  3    && (tstzspan, tstzspan),
  OPERATOR  3    && (tstzspan, tstzspanset),
  -- contains
  OPERATOR  7    @> (tstzspan, timestamptz),
  OPERATOR  7    @> (tstzspan, tstzspan),
  OPERATOR  7    @> (tstzspan, tstzspanset),
  -- contained by
  OPERATOR  8    <@ (tstzspan, tstzspan),
  OPERATOR  8    <@ (tstzspan, tstzspanset),
  -- adjacent
  OPERATOR  17    -|- (tstzspan, tstzspan),
  OPERATOR  17    -|- (tstzspan, tstzspanset),
  -- equals
  OPERATOR  18    = (tstzspan, tstzspan),
  -- nearest approach distance
  OPERATOR  25    <-> (tstzspan, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspan, tstzspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspan, tstzspanset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzspan, timestamptz),
  OPERATOR  28    &<# (tstzspan, tstzspan),
  OPERATOR  28    &<# (tstzspan, tstzspanset),
  -- strictly before
  OPERATOR  29    <<# (tstzspan, timestamptz),
  OPERATOR  29    <<# (tstzspan, tstzspan),
  OPERATOR  29    <<# (tstzspan, tstzspanset),
  -- strictly after
  OPERATOR  30    #>> (tstzspan, timestamptz),
  OPERATOR  30    #>> (tstzspan, tstzspan),
  OPERATOR  30    #>> (tstzspan, tstzspanset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzspan, timestamptz),
  OPERATOR  31    #&> (tstzspan, tstzspan),
  OPERATOR  31    #&> (tstzspan, tstzspanset),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, tstzspan, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(tstzspan, tstzspan, internal),
  FUNCTION  9  span_gist_fetch(internal);

/******************************************************************************
 * Quad-tree SP-GiST indexes
 ******************************************************************************/

CREATE FUNCTION intspan_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Intspan_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspan_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Bigintspan_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Floatspan_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_quadtree_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Span_quadtree_choose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_quadtree_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Span_quadtree_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_quadtree_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Span_quadtree_inner_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_spgist_leaf_consistent(internal, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_spgist_leaf_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intspan_quadtree_ops
  DEFAULT FOR TYPE intspan USING spgist AS
  -- strictly left
  OPERATOR  1     << (intspan, int),
  OPERATOR  1     << (intspan, intspan),
  OPERATOR  1     << (intspan, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspan, int),
  OPERATOR  2     &< (intspan, intspan),
  OPERATOR  2     &< (intspan, intspanset),
  -- overlaps
  OPERATOR  3     && (intspan, intspan),
  OPERATOR  3     && (intspan, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspan, int),
  OPERATOR  4     &> (intspan, intspan),
  OPERATOR  4     &> (intspan, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspan, int),
  OPERATOR  5     >> (intspan, intspan),
  OPERATOR  5     >> (intspan, intspanset),
  -- contains
  OPERATOR  7     @> (intspan, int),
  OPERATOR  7     @> (intspan, intspan),
  OPERATOR  7     @> (intspan, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspan, intspan),
  OPERATOR  8     <@ (intspan, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspan, intspan),
  OPERATOR  17    -|- (intspan, intspanset),
  -- equals
  OPERATOR  18    = (intspan, intspan),
  -- nearest approach distance
  OPERATOR  25    <-> (intspan, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS bigintspan_quadtree_ops
  DEFAULT FOR TYPE bigintspan USING spgist AS
  -- strictly left
  OPERATOR  1     << (bigintspan, bigint),
  OPERATOR  1     << (bigintspan, bigintspan),
  OPERATOR  1     << (bigintspan, bigintspanset),
  -- overlaps or left
  OPERATOR  2     &< (bigintspan, bigint),
  OPERATOR  2     &< (bigintspan, bigintspan),
  OPERATOR  2     &< (bigintspan, bigintspanset),
  -- overlaps
  OPERATOR  3     && (bigintspan, bigintspan),
  OPERATOR  3     && (bigintspan, bigintspanset),
  -- overlaps or right
  OPERATOR  4     &> (bigintspan, bigint),
  OPERATOR  4     &> (bigintspan, bigintspan),
  OPERATOR  4     &> (bigintspan, bigintspanset),
  -- strictly right
  OPERATOR  5     >> (bigintspan, bigint),
  OPERATOR  5     >> (bigintspan, bigintspan),
  OPERATOR  5     >> (bigintspan, bigintspanset),
  -- contains
  OPERATOR  7     @> (bigintspan, bigint),
  OPERATOR  7     @> (bigintspan, bigintspan),
  OPERATOR  7     @> (bigintspan, bigintspanset),
  -- contained by
  OPERATOR  8     <@ (bigintspan, bigintspan),
  OPERATOR  8     <@ (bigintspan, bigintspanset),
  -- adjacent
  OPERATOR  17    -|- (bigintspan, bigintspan),
  OPERATOR  17    -|- (bigintspan, bigintspanset),
  -- equals
  OPERATOR  18    = (bigintspan, bigintspan),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintspan, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspan, bigintspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspan, bigintspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  bigintspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS floatspan_quadtree_ops
  DEFAULT FOR TYPE floatspan USING spgist AS
  -- strictly left
  OPERATOR  1     << (floatspan, float),
  OPERATOR  1     << (floatspan, floatspan),
  OPERATOR  1     << (floatspan, floatspanset),
  -- overlaps or left
  OPERATOR  2     &< (floatspan, float),
  OPERATOR  2     &< (floatspan, floatspan),
  OPERATOR  2     &< (floatspan, floatspanset),
  -- overlaps
  OPERATOR  3     && (floatspan, floatspan),
  OPERATOR  3     && (floatspan, floatspanset),
  -- overlaps or right
  OPERATOR  4     &> (floatspan, float),
  OPERATOR  4     &> (floatspan, floatspan),
  OPERATOR  4     &> (floatspan, floatspanset),
  -- strictly right
  OPERATOR  5     >> (floatspan, float),
  OPERATOR  5     >> (floatspan, floatspan),
  OPERATOR  5     >> (floatspan, floatspanset),
  -- contains
  OPERATOR  7     @> (floatspan, float),
  OPERATOR  7     @> (floatspan, floatspan),
  OPERATOR  7     @> (floatspan, floatspanset),
  -- contained by
  OPERATOR  8     <@ (floatspan, floatspan),
  OPERATOR  8     <@ (floatspan, floatspanset),
  -- adjacent
  OPERATOR  17    -|- (floatspan, floatspan),
  OPERATOR  17    -|- (floatspan, floatspanset),
  -- equals
  OPERATOR  18    = (floatspan, floatspan),
  -- nearest approach distance
  OPERATOR  25    <-> (floatspan, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  floatspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE FUNCTION period_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Period_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS period_quadtree_ops
  DEFAULT FOR TYPE tstzspan USING spgist AS
  -- overlaps
  OPERATOR  3    && (tstzspan, tstzspan),
  OPERATOR  3    && (tstzspan, tstzspanset),
  -- contains
  OPERATOR  7    @> (tstzspan, timestamptz),
  OPERATOR  7    @> (tstzspan, tstzspan),
  OPERATOR  7    @> (tstzspan, tstzspanset),
  -- contained by
  OPERATOR  8    <@ (tstzspan, tstzspan),
  OPERATOR  8    <@ (tstzspan, tstzspanset),
  -- adjacent
  OPERATOR  17    -|- (tstzspan, tstzspan),
  OPERATOR  17    -|- (tstzspan, tstzspanset),
  -- equals
  OPERATOR  18    = (tstzspan, tstzspan),
  -- nearest approach distance
  OPERATOR  25    <->(tstzspan, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <->(tstzspan, tstzspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <->(tstzspan, tstzspanset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzspan, timestamptz),
  OPERATOR  28    &<# (tstzspan, tstzspan),
  OPERATOR  28    &<# (tstzspan, tstzspanset),
  -- strictly before
  OPERATOR  29    <<# (tstzspan, timestamptz),
  OPERATOR  29    <<# (tstzspan, tstzspan),
  OPERATOR  29    <<# (tstzspan, tstzspanset),
  -- strictly after
  OPERATOR  30    #>> (tstzspan, timestamptz),
  OPERATOR  30    #>> (tstzspan, tstzspan),
  OPERATOR  30    #>> (tstzspan, tstzspanset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzspan, timestamptz),
  OPERATOR  31    #&> (tstzspan, tstzspan),
  OPERATOR  31    #&> (tstzspan, tstzspanset),
  -- functions
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************
 * Kd-tree SP-GiST indexes
 ******************************************************************************/

CREATE FUNCTION span_kdtree_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Span_kdtree_choose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_kdtree_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Span_kdtree_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_kdtree_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Span_kdtree_inner_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intspan_kdtree_ops
  FOR TYPE intspan USING spgist AS
  -- strictly left
  OPERATOR  1     << (intspan, int),
  OPERATOR  1     << (intspan, intspan),
  OPERATOR  1     << (intspan, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspan, int),
  OPERATOR  2     &< (intspan, intspan),
  OPERATOR  2     &< (intspan, intspanset),
  -- overlaps
  OPERATOR  3     && (intspan, intspan),
  OPERATOR  3     && (intspan, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspan, int),
  OPERATOR  4     &> (intspan, intspan),
  OPERATOR  4     &> (intspan, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspan, int),
  OPERATOR  5     >> (intspan, intspan),
  OPERATOR  5     >> (intspan, intspanset),
  -- contains
  OPERATOR  7     @> (intspan, int),
  OPERATOR  7     @> (intspan, intspan),
  OPERATOR  7     @> (intspan, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspan, intspan),
  OPERATOR  8     <@ (intspan, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspan, intspan),
  OPERATOR  17    -|- (intspan, intspanset),
  -- equals
  OPERATOR  18    = (intspan, intspan),
  -- nearest approach distance
  OPERATOR  25    <-> (intspan, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspan, intspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS bigintspan_kdtree_ops
  FOR TYPE bigintspan USING spgist AS
  -- strictly left
  OPERATOR  1     << (bigintspan, bigint),
  OPERATOR  1     << (bigintspan, bigintspan),
  OPERATOR  1     << (bigintspan, bigintspanset),
  -- overlaps or left
  OPERATOR  2     &< (bigintspan, bigint),
  OPERATOR  2     &< (bigintspan, bigintspan),
  OPERATOR  2     &< (bigintspan, bigintspanset),
  -- overlaps
  OPERATOR  3     && (bigintspan, bigintspan),
  OPERATOR  3     && (bigintspan, bigintspanset),
  -- overlaps or right
  OPERATOR  4     &> (bigintspan, bigint),
  OPERATOR  4     &> (bigintspan, bigintspan),
  OPERATOR  4     &> (bigintspan, bigintspanset),
  -- strictly right
  OPERATOR  5     >> (bigintspan, bigint),
  OPERATOR  5     >> (bigintspan, bigintspan),
  OPERATOR  5     >> (bigintspan, bigintspanset),
  -- contains
  OPERATOR  7     @> (bigintspan, bigint),
  OPERATOR  7     @> (bigintspan, bigintspan),
  OPERATOR  7     @> (bigintspan, bigintspanset),
  -- contained by
  OPERATOR  8     <@ (bigintspan, bigintspan),
  OPERATOR  8     <@ (bigintspan, bigintspanset),
  -- adjacent
  OPERATOR  17    -|- (bigintspan, bigintspan),
  OPERATOR  17    -|- (bigintspan, bigintspanset),
  -- equals
  OPERATOR  18    = (bigintspan, bigintspan),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintspan, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspan, bigintspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspan, bigintspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  bigintspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS floatspan_kdtree_ops
  FOR TYPE floatspan USING spgist AS
  -- strictly left
  OPERATOR  1     << (floatspan, float),
  OPERATOR  1     << (floatspan, floatspan),
  OPERATOR  1     << (floatspan, floatspanset),
  -- overlaps or left
  OPERATOR  2     &< (floatspan, float),
  OPERATOR  2     &< (floatspan, floatspan),
  OPERATOR  2     &< (floatspan, floatspanset),
  -- overlaps
  OPERATOR  3     && (floatspan, floatspan),
  OPERATOR  3     && (floatspan, floatspanset),
  -- overlaps or right
  OPERATOR  4     &> (floatspan, float),
  OPERATOR  4     &> (floatspan, floatspan),
  OPERATOR  4     &> (floatspan, floatspanset),
  -- strictly right
  OPERATOR  5     >> (floatspan, float),
  OPERATOR  5     >> (floatspan, floatspan),
  OPERATOR  5     >> (floatspan, floatspanset),
  -- contains
  OPERATOR  7     @> (floatspan, float),
  OPERATOR  7     @> (floatspan, floatspan),
  OPERATOR  7     @> (floatspan, floatspanset),
  -- contained by
  OPERATOR  8     <@ (floatspan, floatspan),
  OPERATOR  8     <@ (floatspan, floatspanset),
  -- adjacent
  OPERATOR  17    -|- (floatspan, floatspan),
  OPERATOR  17    -|- (floatspan, floatspanset),
  -- equals
  OPERATOR  18    = (floatspan, floatspan),
  -- nearest approach distance
  OPERATOR  25    <-> (floatspan, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspan, floatspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  floatspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS period_kdtree_ops
  FOR TYPE tstzspan USING spgist AS
  -- overlaps
  OPERATOR  3    && (tstzspan, tstzspan),
  OPERATOR  3    && (tstzspan, tstzspanset),
  -- contains
  OPERATOR  7    @> (tstzspan, timestamptz),
  OPERATOR  7    @> (tstzspan, tstzspan),
  OPERATOR  7    @> (tstzspan, tstzspanset),
  -- contained by
  OPERATOR  8    <@ (tstzspan, tstzspan),
  OPERATOR  8    <@ (tstzspan, tstzspanset),
  -- adjacent
  OPERATOR  17    -|- (tstzspan, tstzspan),
  OPERATOR  17    -|- (tstzspan, tstzspanset),
  -- equals
  OPERATOR  18    = (tstzspan, tstzspan),
  -- nearest approach distance
  OPERATOR  25    <-> (tstzspan, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspan, tstzspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspan, tstzspanset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzspan, timestamptz),
  OPERATOR  28    &<# (tstzspan, tstzspan),
  OPERATOR  28    &<# (tstzspan, tstzspanset),
  -- strictly before
  OPERATOR  29    <<# (tstzspan, timestamptz),
  OPERATOR  29    <<# (tstzspan, tstzspan),
  OPERATOR  29    <<# (tstzspan, tstzspanset),
  -- strictly after
  OPERATOR  30    #>> (tstzspan, timestamptz),
  OPERATOR  30    #>> (tstzspan, tstzspan),
  OPERATOR  30    #>> (tstzspan, tstzspanset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzspan, timestamptz),
  OPERATOR  31    #&> (tstzspan, tstzspan),
  OPERATOR  31    #&> (tstzspan, tstzspanset),
  -- functions
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

