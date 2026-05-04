/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief SP-GiST quadtree and kdtree operator classes for `th3index`.
 *
 * Mirror of `tbool_quadtree_ops` / `tbool_kdtree_ops` from
 * `mobilitydb/sql/temporal/044_temporal_spgist.in.sql`. Both
 * opclasses key on the time dimension only — `h3index` has no total
 * order to support value-dimension splitting. Every support
 * function required by these opclasses has an `(internal,
 * internal)` signature, so no th3index-typed wrappers are needed —
 * we reference the existing `tstzspan_spgist_config`,
 * `span_quadtree_*`, `span_kdtree_*`, `span_spgist_leaf_consistent`
 * and `temporal_spgist_compress` symbols directly.
 */

/******************************************************************************
 * Quadtree operator class
 ******************************************************************************/

CREATE OPERATOR CLASS th3index_quadtree_ops
  DEFAULT FOR TYPE th3index USING spgist AS
  -- overlaps
  OPERATOR  3    && (th3index, tstzspan),
  OPERATOR  3    && (th3index, th3index),
  -- same
  OPERATOR  6    ~= (th3index, tstzspan),
  OPERATOR  6    ~= (th3index, th3index),
  -- contains
  OPERATOR  7    @> (th3index, tstzspan),
  OPERATOR  7    @> (th3index, th3index),
  -- contained by
  OPERATOR  8    <@ (th3index, tstzspan),
  OPERATOR  8    <@ (th3index, th3index),
  -- adjacent
  OPERATOR  17    -|- (th3index, tstzspan),
  OPERATOR  17    -|- (th3index, th3index),
  -- overlaps or before
  OPERATOR  28    &<# (th3index, tstzspan),
  OPERATOR  28    &<# (th3index, th3index),
  -- strictly before
  OPERATOR  29    <<# (th3index, tstzspan),
  OPERATOR  29    <<# (th3index, th3index),
  -- strictly after
  OPERATOR  30    #>> (th3index, tstzspan),
  OPERATOR  30    #>> (th3index, th3index),
  -- overlaps or after
  OPERATOR  31    #&> (th3index, tstzspan),
  OPERATOR  31    #&> (th3index, th3index),
  -- functions
  FUNCTION  1  tstzspan_spgist_config(internal, internal),
  FUNCTION  2  span_quadtree_choose(internal, internal),
  FUNCTION  3  span_quadtree_picksplit(internal, internal),
  FUNCTION  4  span_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  temporal_spgist_compress(internal);

/******************************************************************************
 * K-d tree operator class
 ******************************************************************************/

CREATE OPERATOR CLASS th3index_kdtree_ops
  FOR TYPE th3index USING spgist AS
  -- overlaps
  OPERATOR  3    && (th3index, tstzspan),
  OPERATOR  3    && (th3index, th3index),
  -- same
  OPERATOR  6    ~= (th3index, tstzspan),
  OPERATOR  6    ~= (th3index, th3index),
  -- contains
  OPERATOR  7    @> (th3index, tstzspan),
  OPERATOR  7    @> (th3index, th3index),
  -- contained by
  OPERATOR  8    <@ (th3index, tstzspan),
  OPERATOR  8    <@ (th3index, th3index),
  -- adjacent
  OPERATOR  17    -|- (th3index, tstzspan),
  OPERATOR  17    -|- (th3index, th3index),
  -- overlaps or before
  OPERATOR  28    &<# (th3index, tstzspan),
  OPERATOR  28    &<# (th3index, th3index),
  -- strictly before
  OPERATOR  29    <<# (th3index, tstzspan),
  OPERATOR  29    <<# (th3index, th3index),
  -- strictly after
  OPERATOR  30    #>> (th3index, tstzspan),
  OPERATOR  30    #>> (th3index, th3index),
  -- overlaps or after
  OPERATOR  31    #&> (th3index, tstzspan),
  OPERATOR  31    #&> (th3index, th3index),
  -- functions
  FUNCTION  1  tstzspan_spgist_config(internal, internal),
  FUNCTION  2  span_kdtree_choose(internal, internal),
  FUNCTION  3  span_kdtree_picksplit(internal, internal),
  FUNCTION  4  span_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  span_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  temporal_spgist_compress(internal);

/******************************************************************************/
