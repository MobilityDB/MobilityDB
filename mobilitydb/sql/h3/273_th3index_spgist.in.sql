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
 * H3 cells are geographic hexagons on the WGS84 sphere — always geodetic,
 * always lat/lon. The bounding box of a `th3index` value is therefore a
 * geodetic `stbox`, matching the `tgeogpoint` / `tcbuffer` pattern. Both
 * opclasses mirror `tcbuffer_quadtree_ops` / `tcbuffer_kdtree_ops` from
 * `mobilitydb/sql/cbuffer/166_tcbuffer_indexes.in.sql`.
 *
 * Every support function has an `(internal, internal)` signature, so no
 * th3index-typed wrappers are needed — we reference the existing
 * `stbox_spgist_config`, `stbox_quadtree_*`, `stbox_kdtree_*`,
 * `stbox_spgist_leaf_consistent`, and `tspatial_spgist_compress` symbols
 * directly.
 */

/******************************************************************************
 * Quadtree operator class
 ******************************************************************************/

CREATE OPERATOR CLASS th3index_quadtree_ops
  DEFAULT FOR TYPE th3index USING spgist AS
  -- strictly left
  OPERATOR  1    << (th3index, stbox),
  OPERATOR  1    << (th3index, th3index),
  -- overlaps or left
  OPERATOR  2    &< (th3index, stbox),
  OPERATOR  2    &< (th3index, th3index),
  -- overlaps
  OPERATOR  3    && (th3index, tstzspan),
  OPERATOR  3    && (th3index, stbox),
  OPERATOR  3    && (th3index, th3index),
  -- overlaps or right
  OPERATOR  4    &> (th3index, stbox),
  OPERATOR  4    &> (th3index, th3index),
  -- strictly right
  OPERATOR  5    >> (th3index, stbox),
  OPERATOR  5    >> (th3index, th3index),
  -- same
  OPERATOR  6    ~= (th3index, tstzspan),
  OPERATOR  6    ~= (th3index, stbox),
  OPERATOR  6    ~= (th3index, th3index),
  -- contains
  OPERATOR  7    @> (th3index, tstzspan),
  OPERATOR  7    @> (th3index, stbox),
  OPERATOR  7    @> (th3index, th3index),
  -- contained by
  OPERATOR  8    <@ (th3index, tstzspan),
  OPERATOR  8    <@ (th3index, stbox),
  OPERATOR  8    <@ (th3index, th3index),
  -- overlaps or below
  OPERATOR  9    &<| (th3index, stbox),
  OPERATOR  9    &<| (th3index, th3index),
  -- strictly below
  OPERATOR  10    <<| (th3index, stbox),
  OPERATOR  10    <<| (th3index, th3index),
  -- strictly above
  OPERATOR  11    |>> (th3index, stbox),
  OPERATOR  11    |>> (th3index, th3index),
  -- overlaps or above
  OPERATOR  12    |&> (th3index, stbox),
  OPERATOR  12    |&> (th3index, th3index),
  -- adjacent
  OPERATOR  17    -|- (th3index, tstzspan),
  OPERATOR  17    -|- (th3index, stbox),
  OPERATOR  17    -|- (th3index, th3index),
  -- overlaps or before
  OPERATOR  28    &<# (th3index, tstzspan),
  OPERATOR  28    &<# (th3index, stbox),
  OPERATOR  28    &<# (th3index, th3index),
  -- strictly before
  OPERATOR  29    <<# (th3index, tstzspan),
  OPERATOR  29    <<# (th3index, stbox),
  OPERATOR  29    <<# (th3index, th3index),
  -- strictly after
  OPERATOR  30    #>> (th3index, tstzspan),
  OPERATOR  30    #>> (th3index, stbox),
  OPERATOR  30    #>> (th3index, th3index),
  -- overlaps or after
  OPERATOR  31    #&> (th3index, tstzspan),
  OPERATOR  31    #&> (th3index, stbox),
  OPERATOR  31    #&> (th3index, th3index),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_quadtree_choose(internal, internal),
  FUNCTION  3 stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4 stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************
 * K-d tree operator class
 ******************************************************************************/

CREATE OPERATOR CLASS th3index_kdtree_ops
  FOR TYPE th3index USING spgist AS
  -- strictly left
  OPERATOR  1    << (th3index, stbox),
  OPERATOR  1    << (th3index, th3index),
  -- overlaps or left
  OPERATOR  2    &< (th3index, stbox),
  OPERATOR  2    &< (th3index, th3index),
  -- overlaps
  OPERATOR  3    && (th3index, tstzspan),
  OPERATOR  3    && (th3index, stbox),
  OPERATOR  3    && (th3index, th3index),
  -- overlaps or right
  OPERATOR  4    &> (th3index, stbox),
  OPERATOR  4    &> (th3index, th3index),
  -- strictly right
  OPERATOR  5    >> (th3index, stbox),
  OPERATOR  5    >> (th3index, th3index),
  -- same
  OPERATOR  6    ~= (th3index, tstzspan),
  OPERATOR  6    ~= (th3index, stbox),
  OPERATOR  6    ~= (th3index, th3index),
  -- contains
  OPERATOR  7    @> (th3index, tstzspan),
  OPERATOR  7    @> (th3index, stbox),
  OPERATOR  7    @> (th3index, th3index),
  -- contained by
  OPERATOR  8    <@ (th3index, tstzspan),
  OPERATOR  8    <@ (th3index, stbox),
  OPERATOR  8    <@ (th3index, th3index),
  -- overlaps or below
  OPERATOR  9    &<| (th3index, stbox),
  OPERATOR  9    &<| (th3index, th3index),
  -- strictly below
  OPERATOR  10    <<| (th3index, stbox),
  OPERATOR  10    <<| (th3index, th3index),
  -- strictly above
  OPERATOR  11    |>> (th3index, stbox),
  OPERATOR  11    |>> (th3index, th3index),
  -- overlaps or above
  OPERATOR  12    |&> (th3index, stbox),
  OPERATOR  12    |&> (th3index, th3index),
  -- adjacent
  OPERATOR  17    -|- (th3index, tstzspan),
  OPERATOR  17    -|- (th3index, stbox),
  OPERATOR  17    -|- (th3index, th3index),
  -- overlaps or before
  OPERATOR  28    &<# (th3index, tstzspan),
  OPERATOR  28    &<# (th3index, stbox),
  OPERATOR  28    &<# (th3index, th3index),
  -- strictly before
  OPERATOR  29    <<# (th3index, tstzspan),
  OPERATOR  29    <<# (th3index, stbox),
  OPERATOR  29    <<# (th3index, th3index),
  -- strictly after
  OPERATOR  30    #>> (th3index, tstzspan),
  OPERATOR  30    #>> (th3index, stbox),
  OPERATOR  30    #>> (th3index, th3index),
  -- overlaps or after
  OPERATOR  31    #&> (th3index, tstzspan),
  OPERATOR  31    #&> (th3index, stbox),
  OPERATOR  31    #&> (th3index, th3index),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_kdtree_choose(internal, internal),
  FUNCTION  3 stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4 stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/
