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
 * spanset_gist.sql
 * R-tree GiST and Quad-tree SP-GiST indexes for span set types
 */

/******************************************************************************
 * R-tree GiST indexes
 ******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, intspanset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intspanset_rtree_ops
  DEFAULT FOR TYPE intspanset USING gist AS
  STORAGE intspan,
  -- strictly left
  OPERATOR  1     << (intspanset, int),
  OPERATOR  1     << (intspanset, intspan),
  OPERATOR  1     << (intspanset, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspanset, int),
  OPERATOR  2     &< (intspanset, intspan),
  OPERATOR  2     &< (intspanset, intspanset),
  -- overlaps
  OPERATOR  3     && (intspanset, intspan),
  OPERATOR  3     && (intspanset, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspanset, int),
  OPERATOR  4     &> (intspanset, intspan),
  OPERATOR  4     &> (intspanset, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspanset, int),
  OPERATOR  5     >> (intspanset, intspan),
  OPERATOR  5     >> (intspanset, intspanset),
  -- contains
  OPERATOR  7     @> (intspanset, int),
  OPERATOR  7     @> (intspanset, intspan),
  OPERATOR  7     @> (intspanset, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspanset, intspan),
  OPERATOR  8     <@ (intspanset, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspanset, intspan),
  OPERATOR  17    -|- (intspanset, intspanset),
  -- equals
  OPERATOR  18    = (intspanset, intspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (intspanset, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspanset, intspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspanset, intspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, intspanset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  spanset_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(intspan, intspan, internal),
  FUNCTION  8  span_gist_distance(internal, intspan, smallint, oid, internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, bigintspanset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS bigintspanset_rtree_ops
  DEFAULT FOR TYPE bigintspanset USING gist AS
  STORAGE bigintspan,
  -- strictly left
  OPERATOR  1     << (bigintspanset, bigint),
  OPERATOR  1     << (bigintspanset, bigintspan),
  OPERATOR  1     << (bigintspanset, bigintspanset),
  -- overlaps or left
  OPERATOR  2     &< (bigintspanset, bigint),
  OPERATOR  2     &< (bigintspanset, bigintspan),
  OPERATOR  2     &< (bigintspanset, bigintspanset),
  -- overlaps
  OPERATOR  3     && (bigintspanset, bigintspan),
  OPERATOR  3     && (bigintspanset, bigintspanset),
  -- overlaps or right
  OPERATOR  4     &> (bigintspanset, bigint),
  OPERATOR  4     &> (bigintspanset, bigintspan),
  OPERATOR  4     &> (bigintspanset, bigintspanset),
  -- strictly right
  OPERATOR  5     >> (bigintspanset, bigint),
  OPERATOR  5     >> (bigintspanset, bigintspan),
  OPERATOR  5     >> (bigintspanset, bigintspanset),
  -- contains
  OPERATOR  7     @> (bigintspanset, bigint),
  OPERATOR  7     @> (bigintspanset, bigintspan),
  OPERATOR  7     @> (bigintspanset, bigintspanset),
  -- contained by
  OPERATOR  8     <@ (bigintspanset, bigintspan),
  OPERATOR  8     <@ (bigintspanset, bigintspanset),
  -- adjacent
  OPERATOR  17    -|- (bigintspanset, bigintspan),
  OPERATOR  17    -|- (bigintspanset, bigintspanset),
  -- equals
  OPERATOR  18    = (bigintspanset, bigintspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintspanset, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspanset, bigintspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspanset, bigintspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, bigintspanset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  spanset_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(bigintspan, bigintspan, internal),
  FUNCTION  8  span_gist_distance(internal, bigintspan, smallint, oid, internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, floatspanset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS floatspanset_rtree_ops
  DEFAULT FOR TYPE floatspanset USING gist AS
  STORAGE floatspan,
  -- strictly left
  OPERATOR  1     << (floatspanset, float),
  OPERATOR  1     << (floatspanset, floatspan),
  OPERATOR  1     << (floatspanset, floatspanset),
  -- overlaps or left
  OPERATOR  2     &< (floatspanset, float),
  OPERATOR  2     &< (floatspanset, floatspan),
  OPERATOR  2     &< (floatspanset, floatspanset),
  -- overlaps
  OPERATOR  3     && (floatspanset, floatspan),
  OPERATOR  3     && (floatspanset, floatspanset),
  -- overlaps or right
  OPERATOR  4     &> (floatspanset, float),
  OPERATOR  4     &> (floatspanset, floatspan),
  OPERATOR  4     &> (floatspanset, floatspanset),
  -- strictly right
  OPERATOR  5     >> (floatspanset, float),
  OPERATOR  5     >> (floatspanset, floatspan),
  OPERATOR  5     >> (floatspanset, floatspanset),
  -- contains
  OPERATOR  7     @> (floatspanset, float),
  OPERATOR  7     @> (floatspanset, floatspan),
  OPERATOR  7     @> (floatspanset, floatspanset),
  -- contained by
  OPERATOR  8     <@ (floatspanset, floatspan),
  OPERATOR  8     <@ (floatspanset, floatspanset),
  -- adjacent
  OPERATOR  17    -|- (floatspanset, floatspan),
  OPERATOR  17    -|- (floatspanset, floatspanset),
  -- equals
  OPERATOR  18    = (floatspanset, floatspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatspanset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspanset, floatspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspanset, floatspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, floatspanset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  spanset_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(floatspan, floatspan, internal),
  FUNCTION  8  span_gist_distance(internal, floatspan, smallint, oid, internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, tstzspanset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS periodset_rtree_ops
  DEFAULT FOR TYPE tstzspanset USING gist AS
  STORAGE tstzspan,
  -- overlaps
  OPERATOR  3    && (tstzspanset, tstzspan),
  OPERATOR  3    && (tstzspanset, tstzspanset),
  -- contains
  OPERATOR  7    @> (tstzspanset, timestamptz),
  OPERATOR  7    @> (tstzspanset, tstzspan),
  OPERATOR  7    @> (tstzspanset, tstzspanset),
  -- contained by
  OPERATOR  8    <@ (tstzspanset, tstzspan),
  OPERATOR  8    <@ (tstzspanset, tstzspanset),
  -- adjacent
  OPERATOR  17    -|- (tstzspanset, tstzspan),
  OPERATOR  17    -|- (tstzspanset, tstzspanset),
  -- equals
  OPERATOR  18    = (tstzspanset, tstzspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (tstzspanset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspanset, tstzspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspanset, tstzspanset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzspanset, timestamptz),
  OPERATOR  28    &<# (tstzspanset, tstzspan),
  OPERATOR  28    &<# (tstzspanset, tstzspanset),
  -- strictly before
  OPERATOR  29    <<# (tstzspanset, timestamptz),
  OPERATOR  29    <<# (tstzspanset, tstzspan),
  OPERATOR  29    <<# (tstzspanset, tstzspanset),
  -- strictly after
  OPERATOR  30    #>> (tstzspanset, timestamptz),
  OPERATOR  30    #>> (tstzspanset, tstzspan),
  OPERATOR  30    #>> (tstzspanset, tstzspanset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzspanset, timestamptz),
  OPERATOR  31    #&> (tstzspanset, tstzspan),
  OPERATOR  31    #&> (tstzspanset, tstzspanset),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, tstzspanset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  spanset_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(tstzspan, tstzspan, internal);

/******************************************************************************
 * Quad-tree SP-GiST indexes
 ******************************************************************************/

CREATE FUNCTION spanset_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Spanset_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intspanset_quadtree_ops
  DEFAULT FOR TYPE intspanset USING spgist AS
  -- strictly left
  OPERATOR  1     << (intspanset, int),
  OPERATOR  1     << (intspanset, intspan),
  OPERATOR  1     << (intspanset, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspanset, int),
  OPERATOR  2     &< (intspanset, intspan),
  OPERATOR  2     &< (intspanset, intspanset),
  -- overlaps
  OPERATOR  3     && (intspanset, intspan),
  OPERATOR  3     && (intspanset, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspanset, int),
  OPERATOR  4     &> (intspanset, intspan),
  OPERATOR  4     &> (intspanset, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspanset, int),
  OPERATOR  5     >> (intspanset, intspan),
  OPERATOR  5     >> (intspanset, intspanset),
  -- contains
  OPERATOR  7     @> (intspanset, int),
  OPERATOR  7     @> (intspanset, intspan),
  OPERATOR  7     @> (intspanset, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspanset, intspan),
  OPERATOR  8     <@ (intspanset, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspanset, intspan),
  OPERATOR  17    -|- (intspanset, intspanset),
  -- equals
  OPERATOR  18    = (intspanset, intspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (intspanset, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspanset, intspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspanset, intspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS bigintspanset_quadtree_ops
  DEFAULT FOR TYPE bigintspanset USING spgist AS
  -- strictly left
  OPERATOR  1     << (bigintspanset, bigint),
  OPERATOR  1     << (bigintspanset, bigintspan),
  OPERATOR  1     << (bigintspanset, bigintspanset),
  -- overlaps or left
  OPERATOR  2     &< (bigintspanset, bigint),
  OPERATOR  2     &< (bigintspanset, bigintspan),
  OPERATOR  2     &< (bigintspanset, bigintspanset),
  -- overlaps
  OPERATOR  3     && (bigintspanset, bigintspan),
  OPERATOR  3     && (bigintspanset, bigintspanset),
  -- overlaps or right
  OPERATOR  4     &> (bigintspanset, bigint),
  OPERATOR  4     &> (bigintspanset, bigintspan),
  OPERATOR  4     &> (bigintspanset, bigintspanset),
  -- strictly right
  OPERATOR  5     >> (bigintspanset, bigint),
  OPERATOR  5     >> (bigintspanset, bigintspan),
  OPERATOR  5     >> (bigintspanset, bigintspanset),
  -- contains
  OPERATOR  7     @> (bigintspanset, bigint),
  OPERATOR  7     @> (bigintspanset, bigintspan),
  OPERATOR  7     @> (bigintspanset, bigintspanset),
  -- contained by
  OPERATOR  8     <@ (bigintspanset, bigintspan),
  OPERATOR  8     <@ (bigintspanset, bigintspanset),
  -- adjacent
  OPERATOR  17    -|- (bigintspanset, bigintspan),
  OPERATOR  17    -|- (bigintspanset, bigintspanset),
  -- equals
  OPERATOR  18    = (bigintspanset, bigintspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintspanset, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspanset, bigintspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspanset, bigintspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS floatspanset_quadtree_ops
  DEFAULT FOR TYPE floatspanset USING spgist AS
  -- strictly left
  OPERATOR  1     << (floatspanset, float),
  OPERATOR  1     << (floatspanset, floatspan),
  OPERATOR  1     << (floatspanset, floatspanset),
  -- overlaps or left
  OPERATOR  2     &< (floatspanset, float),
  OPERATOR  2     &< (floatspanset, floatspan),
  OPERATOR  2     &< (floatspanset, floatspanset),
  -- overlaps
  OPERATOR  3     && (floatspanset, floatspan),
  OPERATOR  3     && (floatspanset, floatspanset),
  -- overlaps or right
  OPERATOR  4     &> (floatspanset, float),
  OPERATOR  4     &> (floatspanset, floatspan),
  OPERATOR  4     &> (floatspanset, floatspanset),
  -- strictly right
  OPERATOR  5     >> (floatspanset, float),
  OPERATOR  5     >> (floatspanset, floatspan),
  OPERATOR  5     >> (floatspanset, floatspanset),
  -- contains
  OPERATOR  7     @> (floatspanset, float),
  OPERATOR  7     @> (floatspanset, floatspan),
  OPERATOR  7     @> (floatspanset, floatspanset),
  -- contained by
  OPERATOR  8     <@ (floatspanset, floatspan),
  OPERATOR  8     <@ (floatspanset, floatspanset),
  -- adjacent
  OPERATOR  17    -|- (floatspanset, floatspan),
  OPERATOR  17    -|- (floatspanset, floatspanset),
  -- equals
  OPERATOR  18    = (floatspanset, floatspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatspanset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspanset, floatspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspanset, floatspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  floatspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS periodset_quadtree_ops
  DEFAULT FOR TYPE tstzspanset USING spgist AS
  -- overlaps
  OPERATOR  3    && (tstzspanset, tstzspan),
  OPERATOR  3    && (tstzspanset, tstzspanset),
  -- contains
  OPERATOR  7    @> (tstzspanset, timestamptz),
  OPERATOR  7    @> (tstzspanset, tstzspan),
  OPERATOR  7    @> (tstzspanset, tstzspanset),
  -- contained by
  OPERATOR  8    <@ (tstzspanset, tstzspan),
  OPERATOR  8    <@ (tstzspanset, tstzspanset),
  -- adjacent
  OPERATOR  17    -|- (tstzspanset, tstzspan),
  OPERATOR  17    -|- (tstzspanset, tstzspanset),
-- equals
  OPERATOR  18    = (tstzspanset, tstzspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (tstzspanset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspanset, tstzspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspanset, tstzspanset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzspanset, timestamptz),
  OPERATOR  28    &<# (tstzspanset, tstzspan),
  OPERATOR  28    &<# (tstzspanset, tstzspanset),
  -- strictly before
  OPERATOR  29    <<# (tstzspanset, timestamptz),
  OPERATOR  29    <<# (tstzspanset, tstzspan),
  OPERATOR  29    <<# (tstzspanset, tstzspanset),
  -- strictly after
  OPERATOR  30    #>> (tstzspanset, timestamptz),
  OPERATOR  30    #>> (tstzspanset, tstzspan),
  OPERATOR  30    #>> (tstzspanset, tstzspanset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzspanset, timestamptz),
  OPERATOR  31    #&> (tstzspanset, tstzspan),
  OPERATOR  31    #&> (tstzspanset, tstzspanset),
  -- functions
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************
 * Kd-tree SP-GiST indexes
 ******************************************************************************/

CREATE OPERATOR CLASS intspanset_kdtree_ops
  FOR TYPE intspanset USING spgist AS
  -- strictly left
  OPERATOR  1     << (intspanset, int),
  OPERATOR  1     << (intspanset, intspan),
  OPERATOR  1     << (intspanset, intspanset),
  -- overlaps or left
  OPERATOR  2     &< (intspanset, int),
  OPERATOR  2     &< (intspanset, intspan),
  OPERATOR  2     &< (intspanset, intspanset),
  -- overlaps
  OPERATOR  3     && (intspanset, intspan),
  OPERATOR  3     && (intspanset, intspanset),
  -- overlaps or right
  OPERATOR  4     &> (intspanset, int),
  OPERATOR  4     &> (intspanset, intspan),
  OPERATOR  4     &> (intspanset, intspanset),
  -- strictly right
  OPERATOR  5     >> (intspanset, int),
  OPERATOR  5     >> (intspanset, intspan),
  OPERATOR  5     >> (intspanset, intspanset),
  -- contains
  OPERATOR  7     @> (intspanset, int),
  OPERATOR  7     @> (intspanset, intspan),
  OPERATOR  7     @> (intspanset, intspanset),
  -- contained by
  OPERATOR  8     <@ (intspanset, intspan),
  OPERATOR  8     <@ (intspanset, intspanset),
  -- adjacent
  OPERATOR  17    -|- (intspanset, intspan),
  OPERATOR  17    -|- (intspanset, intspanset),
  -- equals
  OPERATOR  18    = (intspanset, intspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (intspanset, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspanset, intspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intspanset, intspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS bigintspanset_kdtree_ops
  FOR TYPE bigintspanset USING spgist AS
  -- strictly left
  OPERATOR  1     << (bigintspanset, bigint),
  OPERATOR  1     << (bigintspanset, bigintspan),
  OPERATOR  1     << (bigintspanset, bigintspanset),
  -- overlaps or left
  OPERATOR  2     &< (bigintspanset, bigint),
  OPERATOR  2     &< (bigintspanset, bigintspan),
  OPERATOR  2     &< (bigintspanset, bigintspanset),
  -- overlaps
  OPERATOR  3     && (bigintspanset, bigintspan),
  OPERATOR  3     && (bigintspanset, bigintspanset),
  -- overlaps or right
  OPERATOR  4     &> (bigintspanset, bigint),
  OPERATOR  4     &> (bigintspanset, bigintspan),
  OPERATOR  4     &> (bigintspanset, bigintspanset),
  -- strictly right
  OPERATOR  5     >> (bigintspanset, bigint),
  OPERATOR  5     >> (bigintspanset, bigintspan),
  OPERATOR  5     >> (bigintspanset, bigintspanset),
  -- contains
  OPERATOR  7     @> (bigintspanset, bigint),
  OPERATOR  7     @> (bigintspanset, bigintspan),
  OPERATOR  7     @> (bigintspanset, bigintspanset),
  -- contained by
  OPERATOR  8     <@ (bigintspanset, bigintspan),
  OPERATOR  8     <@ (bigintspanset, bigintspanset),
  -- adjacent
  OPERATOR  17    -|- (bigintspanset, bigintspan),
  OPERATOR  17    -|- (bigintspanset, bigintspanset),
  -- equals
  OPERATOR  18    = (bigintspanset, bigintspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintspanset, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspanset, bigintspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintspanset, bigintspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS floatspanset_kdtree_ops
  FOR TYPE floatspanset USING spgist AS
  -- strictly left
  OPERATOR  1     << (floatspanset, float),
  OPERATOR  1     << (floatspanset, floatspan),
  OPERATOR  1     << (floatspanset, floatspanset),
  -- overlaps or left
  OPERATOR  2     &< (floatspanset, float),
  OPERATOR  2     &< (floatspanset, floatspan),
  OPERATOR  2     &< (floatspanset, floatspanset),
  -- overlaps
  OPERATOR  3     && (floatspanset, floatspan),
  OPERATOR  3     && (floatspanset, floatspanset),
  -- overlaps or right
  OPERATOR  4     &> (floatspanset, float),
  OPERATOR  4     &> (floatspanset, floatspan),
  OPERATOR  4     &> (floatspanset, floatspanset),
  -- strictly right
  OPERATOR  5     >> (floatspanset, float),
  OPERATOR  5     >> (floatspanset, floatspan),
  OPERATOR  5     >> (floatspanset, floatspanset),
  -- contains
  OPERATOR  7     @> (floatspanset, float),
  OPERATOR  7     @> (floatspanset, floatspan),
  OPERATOR  7     @> (floatspanset, floatspanset),
  -- contained by
  OPERATOR  8     <@ (floatspanset, floatspan),
  OPERATOR  8     <@ (floatspanset, floatspanset),
  -- adjacent
  OPERATOR  17    -|- (floatspanset, floatspan),
  OPERATOR  17    -|- (floatspanset, floatspanset),
  -- equals
  OPERATOR  18    = (floatspanset, floatspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatspanset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspanset, floatspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatspanset, floatspanset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  floatspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS periodset_kdtree_ops
  FOR TYPE tstzspanset USING spgist AS
  -- overlaps
  OPERATOR  3    && (tstzspanset, tstzspan),
  OPERATOR  3    && (tstzspanset, tstzspanset),
  -- contains
  OPERATOR  7    @> (tstzspanset, timestamptz),
  OPERATOR  7    @> (tstzspanset, tstzspan),
  OPERATOR  7    @> (tstzspanset, tstzspanset),
  -- contained by
  OPERATOR  8    <@ (tstzspanset, tstzspan),
  OPERATOR  8    <@ (tstzspanset, tstzspanset),
  -- adjacent
  OPERATOR  17    -|- (tstzspanset, tstzspan),
  OPERATOR  17    -|- (tstzspanset, tstzspanset),
-- equals
  OPERATOR  18    = (tstzspanset, tstzspanset),
  -- nearest approach distance
  OPERATOR  25    <-> (tstzspanset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspanset, tstzspan) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (tstzspanset, tstzspanset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzspanset, timestamptz),
  OPERATOR  28    &<# (tstzspanset, tstzspan),
  OPERATOR  28    &<# (tstzspanset, tstzspanset),
  -- strictly before
  OPERATOR  29    <<# (tstzspanset, timestamptz),
  OPERATOR  29    <<# (tstzspanset, tstzspan),
  OPERATOR  29    <<# (tstzspanset, tstzspanset),
  -- strictly after
  OPERATOR  30    #>> (tstzspanset, timestamptz),
  OPERATOR  30    #>> (tstzspanset, tstzspan),
  OPERATOR  30    #>> (tstzspanset, tstzspanset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzspanset, timestamptz),
  OPERATOR  31    #&> (tstzspanset, tstzspan),
  OPERATOR  31    #&> (tstzspanset, tstzspanset),
  -- functions
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  spanset_spgist_compress(internal);

/******************************************************************************/
