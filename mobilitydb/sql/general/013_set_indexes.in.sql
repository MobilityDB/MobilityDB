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
 * set_gist.sql
 * R-tree GiST, Quad-tree SP-GiST, and Kd-tree SP-GiST indexes for set types
 */

/******************************************************************************
 * R-tree GiST indexes
 ******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, intset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(intset, intset, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, intset, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intset_rtree_ops
  DEFAULT FOR TYPE intset USING gist AS
  STORAGE intspan,
  -- strictly left
  OPERATOR  1     << (intset, int),
  OPERATOR  1     << (intset, intset),
  -- overlaps or left
  OPERATOR  2     &< (intset, int),
  OPERATOR  2     &< (intset, intset),
  -- overlaps
  OPERATOR  3     && (intset, intset),
  -- overlaps or right
  OPERATOR  4     &> (intset, int),
  OPERATOR  4     &> (intset, intset),
  -- strictly right
  OPERATOR  5     >> (intset, int),
  OPERATOR  5     >> (intset, intset),
  -- contains
  OPERATOR  7     @> (intset, int),
  OPERATOR  7     @> (intset, intset),
  -- contained by
  OPERATOR  8     <@ (intset, intset),
  -- equals
  OPERATOR  18    = (intset, intset),
  -- nearest approach distance
  OPERATOR  25    <-> (intset, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intset, intset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, intset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(intset, intset, internal),
  FUNCTION  8  span_gist_distance(internal, intset, smallint, oid, internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, bigintset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(bigintset, bigintset, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, bigintset, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS bigintset_rtree_ops
  DEFAULT FOR TYPE bigintset USING gist AS
  STORAGE bigintspan,
  -- strictly left
  OPERATOR  1     << (bigintset, bigint),
  OPERATOR  1     << (bigintset, bigintset),
  -- overlaps or left
  OPERATOR  2     &< (bigintset, bigint),
  OPERATOR  2     &< (bigintset, bigintset),
  -- overlaps
  OPERATOR  3     && (bigintset, bigintset),
  -- overlaps or right
  OPERATOR  4     &> (bigintset, bigint),
  OPERATOR  4     &> (bigintset, bigintset),
  -- strictly right
  OPERATOR  5     >> (bigintset, bigint),
  OPERATOR  5     >> (bigintset, bigintset),
  -- contains
  OPERATOR  7     @> (bigintset, bigint),
  OPERATOR  7     @> (bigintset, bigintset),
  -- contained by
  OPERATOR  8     <@ (bigintset, bigintset),
  -- equals
  OPERATOR  18    = (bigintset, bigintset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintset, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintset, bigintset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, bigintset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(bigintset, bigintset, internal),
  FUNCTION  8  span_gist_distance(internal, bigintset, smallint, oid, internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, floatset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_same(floatset, floatset, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, floatset, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS floatset_rtree_ops
  DEFAULT FOR TYPE floatset USING gist AS
  STORAGE floatspan,
  -- strictly left
  OPERATOR  1     << (floatset, float),
  OPERATOR  1     << (floatset, floatset),
  -- overlaps or left
  OPERATOR  2     &< (floatset, float),
  OPERATOR  2     &< (floatset, floatset),
  -- overlaps
  OPERATOR  3     && (floatset, floatset),
  -- overlaps or right
  OPERATOR  4     &> (floatset, float),
  OPERATOR  4     &> (floatset, floatset),
  -- strictly right
  OPERATOR  5     >> (floatset, float),
  OPERATOR  5     >> (floatset, floatset),
  -- contains
  OPERATOR  7     @> (floatset, float),
  OPERATOR  7     @> (floatset, floatset),
  -- contained by
  OPERATOR  8     <@ (floatset, floatset),
  -- equals
  OPERATOR  18    = (floatset, floatset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatset, floatset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, floatset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(floatset, floatset, internal),
  FUNCTION  8  span_gist_distance(internal, floatset, smallint, oid, internal);

/*****************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, timestampset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE OPERATOR CLASS timestampset_rtree_ops
  DEFAULT FOR TYPE timestampset USING gist AS
  STORAGE period,
  -- overlaps
  OPERATOR  3    && (timestampset, timestampset),
  OPERATOR  3    && (timestampset, period),
  OPERATOR  3    && (timestampset, periodset),
  -- contains
  OPERATOR  7    @> (timestampset, timestamptz),
  OPERATOR  7    @> (timestampset, timestampset),
  -- contained by
  OPERATOR  8    <@ (timestampset, timestampset),
  OPERATOR  8    <@ (timestampset, period),
  OPERATOR  8    <@ (timestampset, periodset),
  -- adjacent
  OPERATOR  17    -|- (timestampset, period),
  OPERATOR  17    -|- (timestampset, periodset),
  -- equals
  OPERATOR  18    = (timestampset, timestampset),
  -- nearest approach distance
  OPERATOR  25    <-> (timestampset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, timestampset) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, period) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, periodset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (timestampset, timestamptz),
  OPERATOR  28    &<# (timestampset, timestampset),
  OPERATOR  28    &<# (timestampset, period),
  OPERATOR  28    &<# (timestampset, periodset),
  -- strictly before
  OPERATOR  29    <<# (timestampset, timestamptz),
  OPERATOR  29    <<# (timestampset, timestampset),
  OPERATOR  29    <<# (timestampset, period),
  OPERATOR  29    <<# (timestampset, periodset),
  -- strictly after
  OPERATOR  30    #>> (timestampset, timestamptz),
  OPERATOR  30    #>> (timestampset, timestampset),
  OPERATOR  30    #>> (timestampset, period),
  OPERATOR  30    #>> (timestampset, periodset),
  -- overlaps or after
  OPERATOR  31    #&> (timestampset, timestamptz),
  OPERATOR  31    #&> (timestampset, timestampset),
  OPERATOR  31    #&> (timestampset, period),
  OPERATOR  31    #&> (timestampset, periodset),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, timestampset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(period, period, internal);

/******************************************************************************
 * Quad-tree SP-GiST indexes
 ******************************************************************************/

CREATE FUNCTION intset_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Intspan_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Bigintspan_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Floatspan_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intset_quadtree_ops
  DEFAULT FOR TYPE intset USING spgist AS
  -- strictly left
  OPERATOR  1     << (intset, int),
  OPERATOR  1     << (intset, intset),
  -- overlaps or left
  OPERATOR  2     &< (intset, int),
  OPERATOR  2     &< (intset, intset),
  -- overlaps
  OPERATOR  3     && (intset, intset),
  -- overlaps or right
  OPERATOR  4     &> (intset, int),
  OPERATOR  4     &> (intset, intset),
  -- strictly right
  OPERATOR  5     >> (intset, int),
  OPERATOR  5     >> (intset, intset),
  -- contains
  OPERATOR  7     @> (intset, int),
  OPERATOR  7     @> (intset, intset),
  -- contained by
  OPERATOR  8     <@ (intset, intset),
  -- equals
  OPERATOR  18    = (intset, intset),
  -- nearest approach distance
  OPERATOR  25    <-> (intset, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intset, intset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intset_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS bigintset_quadtree_ops
  DEFAULT FOR TYPE bigintset USING spgist AS
  -- strictly left
  OPERATOR  1     << (bigintset, bigint),
  OPERATOR  1     << (bigintset, bigintset),
  -- overlaps or left
  OPERATOR  2     &< (bigintset, bigint),
  OPERATOR  2     &< (bigintset, bigintset),
  -- overlaps
  OPERATOR  3     && (bigintset, bigintset),
  -- overlaps or right
  OPERATOR  4     &> (bigintset, bigint),
  OPERATOR  4     &> (bigintset, bigintset),
  -- strictly right
  OPERATOR  5     >> (bigintset, bigint),
  OPERATOR  5     >> (bigintset, bigintset),
  -- contains
  OPERATOR  7     @> (bigintset, bigint),
  OPERATOR  7     @> (bigintset, bigintset),
  -- contained by
  OPERATOR  8     <@ (bigintset, bigintset),
  -- equals
  OPERATOR  18    = (bigintset, bigintset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintset, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintset, bigintset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intset_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS floatset_quadtree_ops
  DEFAULT FOR TYPE floatset USING spgist AS
  -- strictly left
  OPERATOR  1     << (floatset, float),
  OPERATOR  1     << (floatset, floatset),
  -- overlaps or left
  OPERATOR  2     &< (floatset, float),
  OPERATOR  2     &< (floatset, floatset),
  -- overlaps
  OPERATOR  3     && (floatset, floatset),
  -- overlaps or right
  OPERATOR  4     &> (floatset, float),
  OPERATOR  4     &> (floatset, floatset),
  -- strictly right
  OPERATOR  5     >> (floatset, float),
  OPERATOR  5     >> (floatset, floatset),
  -- contains
  OPERATOR  7     @> (floatset, float),
  OPERATOR  7     @> (floatset, floatset),
  -- contained by
  OPERATOR  8     <@ (floatset, floatset),
  -- equals
  OPERATOR  18    = (floatset, floatset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatset, floatset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  floatset_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS timestampset_quadtree_ops
  DEFAULT FOR TYPE timestampset USING spgist AS
  -- overlaps
  OPERATOR  3    && (timestampset, timestampset),
  OPERATOR  3    && (timestampset, period),
  OPERATOR  3    && (timestampset, periodset),
  -- contains
  OPERATOR  7    @> (timestampset, timestamptz),
  OPERATOR  7    @> (timestampset, timestampset),
  -- contained by
  OPERATOR  8    <@ (timestampset, timestampset),
  OPERATOR  8    <@ (timestampset, period),
  OPERATOR  8    <@ (timestampset, periodset),
  -- adjacent
  OPERATOR  17    -|- (timestampset, period),
  OPERATOR  17    -|- (timestampset, periodset),
  -- equals
  OPERATOR  18    = (timestampset, timestampset),
  -- nearest approach distance
  OPERATOR  25    <-> (timestampset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, timestampset) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, period) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, periodset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (timestampset, timestamptz),
  OPERATOR  28    &<# (timestampset, timestampset),
  OPERATOR  28    &<# (timestampset, period),
  OPERATOR  28    &<# (timestampset, periodset),
  -- strictly before
  OPERATOR  29    <<# (timestampset, timestamptz),
  OPERATOR  29    <<# (timestampset, timestampset),
  OPERATOR  29    <<# (timestampset, period),
  OPERATOR  29    <<# (timestampset, periodset),
  -- strictly after
  OPERATOR  30    #>> (timestampset, timestamptz),
  OPERATOR  30    #>> (timestampset, timestampset),
  OPERATOR  30    #>> (timestampset, period),
  OPERATOR  30    #>> (timestampset, periodset),
  -- overlaps or after
  OPERATOR  31    #&> (timestampset, timestamptz),
  OPERATOR  31    #&> (timestampset, timestampset),
  OPERATOR  31    #&> (timestampset, period),
  OPERATOR  31    #&> (timestampset, periodset),
  -- functions
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************
 * Kd-tree SP-GiST indexes
 ******************************************************************************/

CREATE OPERATOR CLASS intset_kdtree_ops
  FOR TYPE intset USING spgist AS
  -- strictly left
  OPERATOR  1     << (intset, int),
  OPERATOR  1     << (intset, intset),
  -- overlaps or left
  OPERATOR  2     &< (intset, int),
  OPERATOR  2     &< (intset, intset),
  -- overlaps
  OPERATOR  3     && (intset, intset),
  -- overlaps or right
  OPERATOR  4     &> (intset, int),
  OPERATOR  4     &> (intset, intset),
  -- strictly right
  OPERATOR  5     >> (intset, int),
  OPERATOR  5     >> (intset, intset),
  -- contains
  OPERATOR  7     @> (intset, int),
  OPERATOR  7     @> (intset, intset),
  -- contained by
  OPERATOR  8     <@ (intset, intset),
  -- equals
  OPERATOR  18    = (intset, intset),
  -- nearest approach distance
  OPERATOR  25    <-> (intset, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (intset, intset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intset_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS bigintset_kdtree_ops
  FOR TYPE bigintset USING spgist AS
  -- strictly left
  OPERATOR  1     << (bigintset, bigint),
  OPERATOR  1     << (bigintset, bigintset),
  -- overlaps or left
  OPERATOR  2     &< (bigintset, bigint),
  OPERATOR  2     &< (bigintset, bigintset),
  -- overlaps
  OPERATOR  3     && (bigintset, bigintset),
  -- overlaps or right
  OPERATOR  4     &> (bigintset, bigint),
  OPERATOR  4     &> (bigintset, bigintset),
  -- strictly right
  OPERATOR  5     >> (bigintset, bigint),
  OPERATOR  5     >> (bigintset, bigintset),
  -- contains
  OPERATOR  7     @> (bigintset, bigint),
  OPERATOR  7     @> (bigintset, bigintset),
  -- contained by
  OPERATOR  8     <@ (bigintset, bigintset),
  -- equals
  OPERATOR  18    = (bigintset, bigintset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintset, bigint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (bigintset, bigintset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  intset_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS floatset_kdtree_ops
  FOR TYPE floatset USING spgist AS
  -- strictly left
  OPERATOR  1     << (floatset, float),
  OPERATOR  1     << (floatset, floatset),
  -- overlaps or left
  OPERATOR  2     &< (floatset, float),
  OPERATOR  2     &< (floatset, floatset),
  -- overlaps
  OPERATOR  3     && (floatset, floatset),
  -- overlaps or right
  OPERATOR  4     &> (floatset, float),
  OPERATOR  4     &> (floatset, floatset),
  -- strictly right
  OPERATOR  5     >> (floatset, float),
  OPERATOR  5     >> (floatset, floatset),
  -- contains
  OPERATOR  7     @> (floatset, float),
  OPERATOR  7     @> (floatset, floatset),
  -- contained by
  OPERATOR  8     <@ (floatset, floatset),
  -- equals
  OPERATOR  18    = (floatset, floatset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatset, floatset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  floatset_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS timestampset_kdtree_ops
  FOR TYPE timestampset USING spgist AS
  -- overlaps
  OPERATOR  3    && (timestampset, timestampset),
  OPERATOR  3    && (timestampset, period),
  OPERATOR  3    && (timestampset, periodset),
  -- contains
  OPERATOR  7    @> (timestampset, timestamptz),
  OPERATOR  7    @> (timestampset, timestampset),
  -- contained by
  OPERATOR  8    <@ (timestampset, timestampset),
  OPERATOR  8    <@ (timestampset, period),
  OPERATOR  8    <@ (timestampset, periodset),
  -- adjacent
  OPERATOR  17    -|- (timestampset, period),
  OPERATOR  17    -|- (timestampset, periodset),
  -- equals
  OPERATOR  18    = (timestampset, timestampset),
  -- nearest approach distance
  OPERATOR  25    <-> (timestampset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, timestampset) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, period) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (timestampset, periodset) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (timestampset, timestamptz),
  OPERATOR  28    &<# (timestampset, timestampset),
  OPERATOR  28    &<# (timestampset, period),
  OPERATOR  28    &<# (timestampset, periodset),
  -- strictly before
  OPERATOR  29    <<# (timestampset, timestamptz),
  OPERATOR  29    <<# (timestampset, timestampset),
  OPERATOR  29    <<# (timestampset, period),
  OPERATOR  29    <<# (timestampset, periodset),
  -- strictly after
  OPERATOR  30    #>> (timestampset, timestamptz),
  OPERATOR  30    #>> (timestampset, timestampset),
  OPERATOR  30    #>> (timestampset, period),
  OPERATOR  30    #>> (timestampset, periodset),
  -- overlaps or after
  OPERATOR  31    #&> (timestampset, timestamptz),
  OPERATOR  31    #&> (timestampset, timestampset),
  OPERATOR  31    #&> (timestampset, period),
  OPERATOR  31    #&> (timestampset, periodset),
  -- functions
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  set_spgist_compress(internal);

/******************************************************************************
 * Kd-tree SP-GiST indexes
 ******************************************************************************/

CREATE FUNCTION set_gin_extract_value(int, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Set_gin_extract_value'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_gin_extract_query(int, internal, int2, internal, internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Set_gin_extract_query'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_gin_triconsistent(internal, int2, int, int4, internal, internal, internal)
RETURNS char
AS 'MODULE_PATHNAME', 'Set_gin_triconsistent'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intset_gin_ops
  DEFAULT FOR TYPE intset USING gin AS
  STORAGE int,
  -- overlaps
  OPERATOR  1    && (intset, intset),
  -- contains value
  OPERATOR  2    @> (intset, int),
  -- contains set
  OPERATOR  3    @> (intset, intset),
  -- contained
  OPERATOR  4    <@ (intset, intset),
    -- same
  OPERATOR  5    = (intset, intset),
  -- functions
  FUNCTION   2    set_gin_extract_value(int, internal),
  FUNCTION   3    set_gin_extract_query(int, internal, int2, internal, internal, internal, internal),
  FUNCTION   6    set_gin_triconsistent(internal, int2, int, int4, internal, internal, internal);

/******************************************************************************/

CREATE FUNCTION set_gin_extract_value(bigint, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Set_gin_extract_value'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_gin_extract_query(bigint, internal, int2, internal, internal, internal, internal)
RETURNS internal
AS 'MODULE_PATHNAME', 'Set_gin_extract_query'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_gin_triconsistent(internal, int2, bigint, int4, internal, internal, internal)
RETURNS char
AS 'MODULE_PATHNAME', 'Set_gin_triconsistent'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS bigintset_gin_ops
  DEFAULT FOR TYPE bigintset USING gin AS
  STORAGE bigint,
  -- overlaps
  OPERATOR  1    && (bigintset, bigintset),
  -- contains value
  OPERATOR  2    @> (bigintset, bigint),
  -- contains set
  OPERATOR  3    @> (bigintset, bigintset),
  -- contained
  OPERATOR  4    <@ (bigintset, bigintset),
  -- equal
  OPERATOR  5    = (bigintset, bigintset),
  -- functions
  FUNCTION   2    set_gin_extract_value(bigint, internal),
  FUNCTION   3    set_gin_extract_query(bigint, internal, int2, internal, internal, internal, internal),
  FUNCTION   6    set_gin_triconsistent(internal, int2, bigint, int4, internal, internal, internal);

/******************************************************************************/

