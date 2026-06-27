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
  -- overlaps
  OPERATOR  3    && (tpcbox, tpcbox),
  -- same
  OPERATOR  6    ~= (tpcbox, tpcbox),
  -- contains
  OPERATOR  7    @> (tpcbox, tpcbox),
  -- contained by
  OPERATOR  8    <@ (tpcbox, tpcbox),
  -- adjacent
  OPERATOR  17   -|- (tpcbox, tpcbox),
  -- functions
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_quadtree_choose(internal, internal),
  FUNCTION  3    stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4    stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpcbox_spgist_compress(internal);

CREATE OPERATOR CLASS tpcbox_kdtree_ops
  FOR TYPE tpcbox USING spgist AS
  -- overlaps
  OPERATOR  3    && (tpcbox, tpcbox),
  -- same
  OPERATOR  6    ~= (tpcbox, tpcbox),
  -- contains
  OPERATOR  7    @> (tpcbox, tpcbox),
  -- contained by
  OPERATOR  8    <@ (tpcbox, tpcbox),
  -- adjacent
  OPERATOR  17   -|- (tpcbox, tpcbox),
  -- functions
  FUNCTION  1    stbox_spgist_config(internal, internal),
  FUNCTION  2    stbox_kdtree_choose(internal, internal),
  FUNCTION  3    stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4    stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5    stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6    tpcbox_spgist_compress(internal);

/*****************************************************************************/
