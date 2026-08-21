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
 *****************************************************************************/

/**
 * @file
 * @brief SP-GiST quadtree and kd-tree opclasses on tpcbox using STBox
 *   storage (lossy on pcid; recovered by recheck on the operator).
 *   Companion to 411_tpcbox_gist.in.sql.
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
