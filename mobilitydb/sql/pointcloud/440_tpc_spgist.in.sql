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
 *****************************************************************************/

/**
 * @file
 * @brief Quad-tree and k-d tree SP-GiST indexes for point cloud boxes and
 *   temporal point clouds
 */

CREATE FUNCTION tpcbox_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcbox_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tpcbox_quadtree_ops
  DEFAULT FOR TYPE tpcbox USING spgist AS
  -- strictly left
  OPERATOR  1    << (tpcbox, tpcbox),
  -- overlaps or left
  OPERATOR  2    &< (tpcbox, tpcbox),
  -- overlaps
  OPERATOR  3    && (tpcbox, tpcbox),
  -- overlaps or right
  OPERATOR  4    &> (tpcbox, tpcbox),
  -- strictly right
  OPERATOR  5    >> (tpcbox, tpcbox),
  -- same
  OPERATOR  6    ~= (tpcbox, tpcbox),
  -- contains
  OPERATOR  7    @> (tpcbox, tpcbox),
  -- contained by
  OPERATOR  8    <@ (tpcbox, tpcbox),
  -- overlaps or below
  OPERATOR  9    &<| (tpcbox, tpcbox),
  -- strictly below
  OPERATOR  10   <<| (tpcbox, tpcbox),
  -- strictly above
  OPERATOR  11   |>> (tpcbox, tpcbox),
  -- overlaps or above
  OPERATOR  12   |&> (tpcbox, tpcbox),
  -- adjacent
  OPERATOR  17   -|- (tpcbox, tpcbox),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcbox, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28   &<# (tpcbox, tpcbox),
  -- strictly before
  OPERATOR  29   <<# (tpcbox, tpcbox),
  -- strictly after
  OPERATOR  30   #>> (tpcbox, tpcbox),
  -- overlaps or after
  OPERATOR  31   #&> (tpcbox, tpcbox),
  -- overlaps or front
  OPERATOR  32   &</ (tpcbox, tpcbox),
  -- strictly front
  OPERATOR  33   <</ (tpcbox, tpcbox),
  -- strictly back
  OPERATOR  34   />> (tpcbox, tpcbox),
  -- overlaps or back
  OPERATOR  35   /&> (tpcbox, tpcbox),
  -- functions
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_quadtree_choose(internal, internal),
  FUNCTION  3    stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4    stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpcbox_spgist_compress(internal);

CREATE OPERATOR CLASS tpcbox_kdtree_ops
  FOR TYPE tpcbox USING spgist AS
  -- strictly left
  OPERATOR  1    << (tpcbox, tpcbox),
  -- overlaps or left
  OPERATOR  2    &< (tpcbox, tpcbox),
  -- overlaps
  OPERATOR  3    && (tpcbox, tpcbox),
  -- overlaps or right
  OPERATOR  4    &> (tpcbox, tpcbox),
  -- strictly right
  OPERATOR  5    >> (tpcbox, tpcbox),
  -- same
  OPERATOR  6    ~= (tpcbox, tpcbox),
  -- contains
  OPERATOR  7    @> (tpcbox, tpcbox),
  -- contained by
  OPERATOR  8    <@ (tpcbox, tpcbox),
  -- overlaps or below
  OPERATOR  9    &<| (tpcbox, tpcbox),
  -- strictly below
  OPERATOR  10   <<| (tpcbox, tpcbox),
  -- strictly above
  OPERATOR  11   |>> (tpcbox, tpcbox),
  -- overlaps or above
  OPERATOR  12   |&> (tpcbox, tpcbox),
  -- adjacent
  OPERATOR  17   -|- (tpcbox, tpcbox),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcbox, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28   &<# (tpcbox, tpcbox),
  -- strictly before
  OPERATOR  29   <<# (tpcbox, tpcbox),
  -- strictly after
  OPERATOR  30   #>> (tpcbox, tpcbox),
  -- overlaps or after
  OPERATOR  31   #&> (tpcbox, tpcbox),
  -- overlaps or front
  OPERATOR  32   &</ (tpcbox, tpcbox),
  -- strictly front
  OPERATOR  33   <</ (tpcbox, tpcbox),
  -- strictly back
  OPERATOR  34   />> (tpcbox, tpcbox),
  -- overlaps or back
  OPERATOR  35   /&> (tpcbox, tpcbox),
  -- functions
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_kdtree_choose(internal, internal),
  FUNCTION  3    stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4    stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpcbox_spgist_compress(internal);

/*****************************************************************************/

CREATE FUNCTION tpc_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpc_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tpcpoint quadtree
 *****************************************************************************/

CREATE OPERATOR CLASS tpcpoint_quadtree_ops
  DEFAULT FOR TYPE tpcpoint USING spgist AS
  OPERATOR  1    << (tpcpoint, tpcbox),
  OPERATOR  1    << (tpcpoint, tpcpoint),
  OPERATOR  2    &< (tpcpoint, tpcbox),
  OPERATOR  2    &< (tpcpoint, tpcpoint),
  OPERATOR  3    && (tpcpoint, tstzspan),
  OPERATOR  3    && (tpcpoint, tpcbox),
  OPERATOR  3    && (tpcpoint, tpcpoint),
  OPERATOR  4    &> (tpcpoint, tpcbox),
  OPERATOR  4    &> (tpcpoint, tpcpoint),
  OPERATOR  5    >> (tpcpoint, tpcbox),
  OPERATOR  5    >> (tpcpoint, tpcpoint),
  OPERATOR  6    ~= (tpcpoint, tstzspan),
  OPERATOR  6    ~= (tpcpoint, tpcbox),
  OPERATOR  6    ~= (tpcpoint, tpcpoint),
  OPERATOR  7    @> (tpcpoint, tstzspan),
  OPERATOR  7    @> (tpcpoint, tpcbox),
  OPERATOR  7    @> (tpcpoint, tpcpoint),
  OPERATOR  8    <@ (tpcpoint, tstzspan),
  OPERATOR  8    <@ (tpcpoint, tpcbox),
  OPERATOR  8    <@ (tpcpoint, tpcpoint),
  OPERATOR  9    &<| (tpcpoint, tpcbox),
  OPERATOR  9    &<| (tpcpoint, tpcpoint),
  OPERATOR  10   <<| (tpcpoint, tpcbox),
  OPERATOR  10   <<| (tpcpoint, tpcpoint),
  OPERATOR  11   |>> (tpcpoint, tpcbox),
  OPERATOR  11   |>> (tpcpoint, tpcpoint),
  OPERATOR  12   |&> (tpcpoint, tpcbox),
  OPERATOR  12   |&> (tpcpoint, tpcpoint),
  OPERATOR  17   -|- (tpcpoint, tstzspan),
  OPERATOR  17   -|- (tpcpoint, tpcbox),
  OPERATOR  17   -|- (tpcpoint, tpcpoint),
  OPERATOR  28   &<# (tpcpoint, tstzspan),
  OPERATOR  28   &<# (tpcpoint, tpcbox),
  OPERATOR  28   &<# (tpcpoint, tpcpoint),
  OPERATOR  29   <<# (tpcpoint, tstzspan),
  OPERATOR  29   <<# (tpcpoint, tpcbox),
  OPERATOR  29   <<# (tpcpoint, tpcpoint),
  OPERATOR  30   #>> (tpcpoint, tstzspan),
  OPERATOR  30   #>> (tpcpoint, tpcbox),
  OPERATOR  30   #>> (tpcpoint, tpcpoint),
  OPERATOR  31   #&> (tpcpoint, tstzspan),
  OPERATOR  31   #&> (tpcpoint, tpcbox),
  OPERATOR  31   #&> (tpcpoint, tpcpoint),
  OPERATOR  32   &</ (tpcpoint, tpcbox),
  OPERATOR  32   &</ (tpcpoint, tpcpoint),
  OPERATOR  33   <</ (tpcpoint, tpcbox),
  OPERATOR  33   <</ (tpcpoint, tpcpoint),
  OPERATOR  34   />> (tpcpoint, tpcbox),
  OPERATOR  34   />> (tpcpoint, tpcpoint),
  OPERATOR  35   /&> (tpcpoint, tpcbox),
  OPERATOR  35   /&> (tpcpoint, tpcpoint),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcpoint, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25   |=| (tpcpoint, tpcpoint) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_quadtree_choose(internal, internal),
  FUNCTION  3    stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4    stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpc_spgist_compress(internal);

/*****************************************************************************
 * tpcpoint kdtree
 *****************************************************************************/

CREATE OPERATOR CLASS tpcpoint_kdtree_ops
  FOR TYPE tpcpoint USING spgist AS
  OPERATOR  1    << (tpcpoint, tpcbox),
  OPERATOR  1    << (tpcpoint, tpcpoint),
  OPERATOR  2    &< (tpcpoint, tpcbox),
  OPERATOR  2    &< (tpcpoint, tpcpoint),
  OPERATOR  3    && (tpcpoint, tstzspan),
  OPERATOR  3    && (tpcpoint, tpcbox),
  OPERATOR  3    && (tpcpoint, tpcpoint),
  OPERATOR  4    &> (tpcpoint, tpcbox),
  OPERATOR  4    &> (tpcpoint, tpcpoint),
  OPERATOR  5    >> (tpcpoint, tpcbox),
  OPERATOR  5    >> (tpcpoint, tpcpoint),
  OPERATOR  6    ~= (tpcpoint, tstzspan),
  OPERATOR  6    ~= (tpcpoint, tpcbox),
  OPERATOR  6    ~= (tpcpoint, tpcpoint),
  OPERATOR  7    @> (tpcpoint, tstzspan),
  OPERATOR  7    @> (tpcpoint, tpcbox),
  OPERATOR  7    @> (tpcpoint, tpcpoint),
  OPERATOR  8    <@ (tpcpoint, tstzspan),
  OPERATOR  8    <@ (tpcpoint, tpcbox),
  OPERATOR  8    <@ (tpcpoint, tpcpoint),
  OPERATOR  9    &<| (tpcpoint, tpcbox),
  OPERATOR  9    &<| (tpcpoint, tpcpoint),
  OPERATOR  10   <<| (tpcpoint, tpcbox),
  OPERATOR  10   <<| (tpcpoint, tpcpoint),
  OPERATOR  11   |>> (tpcpoint, tpcbox),
  OPERATOR  11   |>> (tpcpoint, tpcpoint),
  OPERATOR  12   |&> (tpcpoint, tpcbox),
  OPERATOR  12   |&> (tpcpoint, tpcpoint),
  OPERATOR  17   -|- (tpcpoint, tstzspan),
  OPERATOR  17   -|- (tpcpoint, tpcbox),
  OPERATOR  17   -|- (tpcpoint, tpcpoint),
  OPERATOR  28   &<# (tpcpoint, tstzspan),
  OPERATOR  28   &<# (tpcpoint, tpcbox),
  OPERATOR  28   &<# (tpcpoint, tpcpoint),
  OPERATOR  29   <<# (tpcpoint, tstzspan),
  OPERATOR  29   <<# (tpcpoint, tpcbox),
  OPERATOR  29   <<# (tpcpoint, tpcpoint),
  OPERATOR  30   #>> (tpcpoint, tstzspan),
  OPERATOR  30   #>> (tpcpoint, tpcbox),
  OPERATOR  30   #>> (tpcpoint, tpcpoint),
  OPERATOR  31   #&> (tpcpoint, tstzspan),
  OPERATOR  31   #&> (tpcpoint, tpcbox),
  OPERATOR  31   #&> (tpcpoint, tpcpoint),
  OPERATOR  32   &</ (tpcpoint, tpcbox),
  OPERATOR  32   &</ (tpcpoint, tpcpoint),
  OPERATOR  33   <</ (tpcpoint, tpcbox),
  OPERATOR  33   <</ (tpcpoint, tpcpoint),
  OPERATOR  34   />> (tpcpoint, tpcbox),
  OPERATOR  34   />> (tpcpoint, tpcpoint),
  OPERATOR  35   /&> (tpcpoint, tpcbox),
  OPERATOR  35   /&> (tpcpoint, tpcpoint),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcpoint, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25   |=| (tpcpoint, tpcpoint) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_kdtree_choose(internal, internal),
  FUNCTION  3    stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4    stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpc_spgist_compress(internal);

/*****************************************************************************
 * tpcpatch quadtree
 *****************************************************************************/

CREATE OPERATOR CLASS tpcpatch_quadtree_ops
  DEFAULT FOR TYPE tpcpatch USING spgist AS
  OPERATOR  1    << (tpcpatch, tpcbox),
  OPERATOR  1    << (tpcpatch, tpcpatch),
  OPERATOR  2    &< (tpcpatch, tpcbox),
  OPERATOR  2    &< (tpcpatch, tpcpatch),
  OPERATOR  3    && (tpcpatch, tstzspan),
  OPERATOR  3    && (tpcpatch, tpcbox),
  OPERATOR  3    && (tpcpatch, tpcpatch),
  OPERATOR  4    &> (tpcpatch, tpcbox),
  OPERATOR  4    &> (tpcpatch, tpcpatch),
  OPERATOR  5    >> (tpcpatch, tpcbox),
  OPERATOR  5    >> (tpcpatch, tpcpatch),
  OPERATOR  6    ~= (tpcpatch, tstzspan),
  OPERATOR  6    ~= (tpcpatch, tpcbox),
  OPERATOR  6    ~= (tpcpatch, tpcpatch),
  OPERATOR  7    @> (tpcpatch, tstzspan),
  OPERATOR  7    @> (tpcpatch, tpcbox),
  OPERATOR  7    @> (tpcpatch, tpcpatch),
  OPERATOR  8    <@ (tpcpatch, tstzspan),
  OPERATOR  8    <@ (tpcpatch, tpcbox),
  OPERATOR  8    <@ (tpcpatch, tpcpatch),
  OPERATOR  9    &<| (tpcpatch, tpcbox),
  OPERATOR  9    &<| (tpcpatch, tpcpatch),
  OPERATOR  10   <<| (tpcpatch, tpcbox),
  OPERATOR  10   <<| (tpcpatch, tpcpatch),
  OPERATOR  11   |>> (tpcpatch, tpcbox),
  OPERATOR  11   |>> (tpcpatch, tpcpatch),
  OPERATOR  12   |&> (tpcpatch, tpcbox),
  OPERATOR  12   |&> (tpcpatch, tpcpatch),
  OPERATOR  17   -|- (tpcpatch, tstzspan),
  OPERATOR  17   -|- (tpcpatch, tpcbox),
  OPERATOR  17   -|- (tpcpatch, tpcpatch),
  OPERATOR  28   &<# (tpcpatch, tstzspan),
  OPERATOR  28   &<# (tpcpatch, tpcbox),
  OPERATOR  28   &<# (tpcpatch, tpcpatch),
  OPERATOR  29   <<# (tpcpatch, tstzspan),
  OPERATOR  29   <<# (tpcpatch, tpcbox),
  OPERATOR  29   <<# (tpcpatch, tpcpatch),
  OPERATOR  30   #>> (tpcpatch, tstzspan),
  OPERATOR  30   #>> (tpcpatch, tpcbox),
  OPERATOR  30   #>> (tpcpatch, tpcpatch),
  OPERATOR  31   #&> (tpcpatch, tstzspan),
  OPERATOR  31   #&> (tpcpatch, tpcbox),
  OPERATOR  31   #&> (tpcpatch, tpcpatch),
  OPERATOR  32   &</ (tpcpatch, tpcbox),
  OPERATOR  32   &</ (tpcpatch, tpcpatch),
  OPERATOR  33   <</ (tpcpatch, tpcbox),
  OPERATOR  33   <</ (tpcpatch, tpcpatch),
  OPERATOR  34   />> (tpcpatch, tpcbox),
  OPERATOR  34   />> (tpcpatch, tpcpatch),
  OPERATOR  35   /&> (tpcpatch, tpcbox),
  OPERATOR  35   /&> (tpcpatch, tpcpatch),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcpatch, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25   |=| (tpcpatch, tpcpatch) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_quadtree_choose(internal, internal),
  FUNCTION  3    stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4    stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpc_spgist_compress(internal);

/*****************************************************************************
 * tpcpatch kdtree
 *****************************************************************************/

CREATE OPERATOR CLASS tpcpatch_kdtree_ops
  FOR TYPE tpcpatch USING spgist AS
  OPERATOR  1    << (tpcpatch, tpcbox),
  OPERATOR  1    << (tpcpatch, tpcpatch),
  OPERATOR  2    &< (tpcpatch, tpcbox),
  OPERATOR  2    &< (tpcpatch, tpcpatch),
  OPERATOR  3    && (tpcpatch, tstzspan),
  OPERATOR  3    && (tpcpatch, tpcbox),
  OPERATOR  3    && (tpcpatch, tpcpatch),
  OPERATOR  4    &> (tpcpatch, tpcbox),
  OPERATOR  4    &> (tpcpatch, tpcpatch),
  OPERATOR  5    >> (tpcpatch, tpcbox),
  OPERATOR  5    >> (tpcpatch, tpcpatch),
  OPERATOR  6    ~= (tpcpatch, tstzspan),
  OPERATOR  6    ~= (tpcpatch, tpcbox),
  OPERATOR  6    ~= (tpcpatch, tpcpatch),
  OPERATOR  7    @> (tpcpatch, tstzspan),
  OPERATOR  7    @> (tpcpatch, tpcbox),
  OPERATOR  7    @> (tpcpatch, tpcpatch),
  OPERATOR  8    <@ (tpcpatch, tstzspan),
  OPERATOR  8    <@ (tpcpatch, tpcbox),
  OPERATOR  8    <@ (tpcpatch, tpcpatch),
  OPERATOR  9    &<| (tpcpatch, tpcbox),
  OPERATOR  9    &<| (tpcpatch, tpcpatch),
  OPERATOR  10   <<| (tpcpatch, tpcbox),
  OPERATOR  10   <<| (tpcpatch, tpcpatch),
  OPERATOR  11   |>> (tpcpatch, tpcbox),
  OPERATOR  11   |>> (tpcpatch, tpcpatch),
  OPERATOR  12   |&> (tpcpatch, tpcbox),
  OPERATOR  12   |&> (tpcpatch, tpcpatch),
  OPERATOR  17   -|- (tpcpatch, tstzspan),
  OPERATOR  17   -|- (tpcpatch, tpcbox),
  OPERATOR  17   -|- (tpcpatch, tpcpatch),
  OPERATOR  28   &<# (tpcpatch, tstzspan),
  OPERATOR  28   &<# (tpcpatch, tpcbox),
  OPERATOR  28   &<# (tpcpatch, tpcpatch),
  OPERATOR  29   <<# (tpcpatch, tstzspan),
  OPERATOR  29   <<# (tpcpatch, tpcbox),
  OPERATOR  29   <<# (tpcpatch, tpcpatch),
  OPERATOR  30   #>> (tpcpatch, tstzspan),
  OPERATOR  30   #>> (tpcpatch, tpcbox),
  OPERATOR  30   #>> (tpcpatch, tpcpatch),
  OPERATOR  31   #&> (tpcpatch, tstzspan),
  OPERATOR  31   #&> (tpcpatch, tpcbox),
  OPERATOR  31   #&> (tpcpatch, tpcpatch),
  OPERATOR  32   &</ (tpcpatch, tpcbox),
  OPERATOR  32   &</ (tpcpatch, tpcpatch),
  OPERATOR  33   <</ (tpcpatch, tpcbox),
  OPERATOR  33   <</ (tpcpatch, tpcpatch),
  OPERATOR  34   />> (tpcpatch, tpcbox),
  OPERATOR  34   />> (tpcpatch, tpcpatch),
  OPERATOR  35   /&> (tpcpatch, tpcbox),
  OPERATOR  35   /&> (tpcpatch, tpcpatch),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcpatch, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25   |=| (tpcpatch, tpcpatch) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_kdtree_choose(internal, internal),
  FUNCTION  3    stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4    stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpc_spgist_compress(internal);

/*****************************************************************************/
