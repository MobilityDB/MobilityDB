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
 * @brief R-tree GiST and SP-GiST indexes for temporal poses
 */

/******************************************************************************/

CREATE FUNCTION tpose_gist_consistent(internal, tpose, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS tpose_rtree_ops
  DEFAULT FOR TYPE tpose USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (tpose, stbox),
  OPERATOR  1    << (tpose, tpose),
  -- overlaps or left
  OPERATOR  2    &< (tpose, stbox),
  OPERATOR  2    &< (tpose, tpose),
  -- overlaps
  OPERATOR  3    && (tpose, tstzspan),
  OPERATOR  3    && (tpose, stbox),
  OPERATOR  3    && (tpose, tpose),
  -- overlaps or right
  OPERATOR  4    &> (tpose, stbox),
  OPERATOR  4    &> (tpose, tpose),
    -- strictly right
  OPERATOR  5    >> (tpose, stbox),
  OPERATOR  5    >> (tpose, tpose),
    -- same
  OPERATOR  6    ~= (tpose, tstzspan),
  OPERATOR  6    ~= (tpose, stbox),
  OPERATOR  6    ~= (tpose, tpose),
  -- contains
  OPERATOR  7    @> (tpose, tstzspan),
  OPERATOR  7    @> (tpose, stbox),
  OPERATOR  7    @> (tpose, tpose),
  -- contained by
  OPERATOR  8    <@ (tpose, tstzspan),
  OPERATOR  8    <@ (tpose, stbox),
  OPERATOR  8    <@ (tpose, tpose),
  -- overlaps or below
  OPERATOR  9    &<| (tpose, stbox),
  OPERATOR  9    &<| (tpose, tpose),
  -- strictly below
  OPERATOR  10    <<| (tpose, stbox),
  OPERATOR  10    <<| (tpose, tpose),
  -- strictly above
  OPERATOR  11    |>> (tpose, stbox),
  OPERATOR  11    |>> (tpose, tpose),
  -- overlaps or above
  OPERATOR  12    |&> (tpose, stbox),
  OPERATOR  12    |&> (tpose, tpose),
  -- adjacent
  OPERATOR  17    -|- (tpose, tstzspan),
  OPERATOR  17    -|- (tpose, stbox),
  OPERATOR  17    -|- (tpose, tpose),
  -- nearest approach distance
--  OPERATOR  25    |=| (tpose, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tpose, tstzspan),
  OPERATOR  28    &<# (tpose, stbox),
  OPERATOR  28    &<# (tpose, tpose),
  -- strictly before
  OPERATOR  29    <<# (tpose, tstzspan),
  OPERATOR  29    <<# (tpose, stbox),
  OPERATOR  29    <<# (tpose, tpose),
  -- strictly after
  OPERATOR  30    #>> (tpose, tstzspan),
  OPERATOR  30    #>> (tpose, stbox),
  OPERATOR  30    #>> (tpose, tpose),
  -- overlaps or after
  OPERATOR  31    #&> (tpose, tstzspan),
  OPERATOR  31    #&> (tpose, stbox),
  OPERATOR  31    #&> (tpose, tpose),
  -- functions
  FUNCTION  1 tpose_gist_consistent(internal, tpose, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tspatial_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal);
--  FUNCTION  8 gist_tpose_distance(internal, tpose, smallint, oid, internal),

/******************************************************************************/

CREATE OPERATOR CLASS tpose_quadtree_ops
  DEFAULT FOR TYPE tpose USING spgist AS
  -- strictly left
  OPERATOR  1    << (tpose, stbox),
  OPERATOR  1    << (tpose, tpose),
  -- overlaps or left
  OPERATOR  2    &< (tpose, stbox),
  OPERATOR  2    &< (tpose, tpose),
  -- overlaps
  OPERATOR  3    && (tpose, tstzspan),
  OPERATOR  3    && (tpose, stbox),
  OPERATOR  3    && (tpose, tpose),
  -- overlaps or right
  OPERATOR  4    &> (tpose, stbox),
  OPERATOR  4    &> (tpose, tpose),
    -- strictly right
  OPERATOR  5    >> (tpose, stbox),
  OPERATOR  5    >> (tpose, tpose),
    -- same
  OPERATOR  6    ~= (tpose, tstzspan),
  OPERATOR  6    ~= (tpose, stbox),
  OPERATOR  6    ~= (tpose, tpose),
  -- contains
  OPERATOR  7    @> (tpose, tstzspan),
  OPERATOR  7    @> (tpose, stbox),
  OPERATOR  7    @> (tpose, tpose),
  -- contained by
  OPERATOR  8    <@ (tpose, tstzspan),
  OPERATOR  8    <@ (tpose, stbox),
  OPERATOR  8    <@ (tpose, tpose),
  -- overlaps or below
  OPERATOR  9    &<| (tpose, stbox),
  OPERATOR  9    &<| (tpose, tpose),
  -- strictly below
  OPERATOR  10    <<| (tpose, stbox),
  OPERATOR  10    <<| (tpose, tpose),
  -- strictly above
  OPERATOR  11    |>> (tpose, stbox),
  OPERATOR  11    |>> (tpose, tpose),
  -- overlaps or above
  OPERATOR  12    |&> (tpose, stbox),
  OPERATOR  12    |&> (tpose, tpose),
  -- adjacent
  OPERATOR  17    -|- (tpose, tstzspan),
  OPERATOR  17    -|- (tpose, stbox),
  OPERATOR  17    -|- (tpose, tpose),
  -- nearest approach distance
--  OPERATOR  25    |=| (tpose, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tpose, tstzspan),
  OPERATOR  28    &<# (tpose, stbox),
  OPERATOR  28    &<# (tpose, tpose),
  -- strictly before
  OPERATOR  29    <<# (tpose, tstzspan),
  OPERATOR  29    <<# (tpose, stbox),
  OPERATOR  29    <<# (tpose, tpose),
  -- strictly after
  OPERATOR  30    #>> (tpose, tstzspan),
  OPERATOR  30    #>> (tpose, stbox),
  OPERATOR  30    #>> (tpose, tpose),
  -- overlaps or after
  OPERATOR  31    #&> (tpose, tstzspan),
  OPERATOR  31    #&> (tpose, stbox),
  OPERATOR  31    #&> (tpose, tpose),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_quadtree_choose(internal, internal),
  FUNCTION  3 stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4 stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS tpose_kdtree_ops
  FOR TYPE tpose USING spgist AS
  -- strictly left
  OPERATOR  1    << (tpose, stbox),
  OPERATOR  1    << (tpose, tpose),
  -- overlaps or left
  OPERATOR  2    &< (tpose, stbox),
  OPERATOR  2    &< (tpose, tpose),
  -- overlaps
  OPERATOR  3    && (tpose, tstzspan),
  OPERATOR  3    && (tpose, stbox),
  OPERATOR  3    && (tpose, tpose),
  -- overlaps or right
  OPERATOR  4    &> (tpose, stbox),
  OPERATOR  4    &> (tpose, tpose),
    -- strictly right
  OPERATOR  5    >> (tpose, stbox),
  OPERATOR  5    >> (tpose, tpose),
    -- same
  OPERATOR  6    ~= (tpose, tstzspan),
  OPERATOR  6    ~= (tpose, stbox),
  OPERATOR  6    ~= (tpose, tpose),
  -- contains
  OPERATOR  7    @> (tpose, tstzspan),
  OPERATOR  7    @> (tpose, stbox),
  OPERATOR  7    @> (tpose, tpose),
  -- contained by
  OPERATOR  8    <@ (tpose, tstzspan),
  OPERATOR  8    <@ (tpose, stbox),
  OPERATOR  8    <@ (tpose, tpose),
  -- overlaps or below
  OPERATOR  9    &<| (tpose, stbox),
  OPERATOR  9    &<| (tpose, tpose),
  -- strictly below
  OPERATOR  10    <<| (tpose, stbox),
  OPERATOR  10    <<| (tpose, tpose),
  -- strictly above
  OPERATOR  11    |>> (tpose, stbox),
  OPERATOR  11    |>> (tpose, tpose),
  -- overlaps or above
  OPERATOR  12    |&> (tpose, stbox),
  OPERATOR  12    |&> (tpose, tpose),
  -- adjacent
  OPERATOR  17    -|- (tpose, tstzspan),
  OPERATOR  17    -|- (tpose, stbox),
  OPERATOR  17    -|- (tpose, tpose),
  -- nearest approach distance
--  OPERATOR  25    |=| (tpose, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tpose, tstzspan),
  OPERATOR  28    &<# (tpose, stbox),
  OPERATOR  28    &<# (tpose, tpose),
  -- strictly before
  OPERATOR  29    <<# (tpose, tstzspan),
  OPERATOR  29    <<# (tpose, stbox),
  OPERATOR  29    <<# (tpose, tpose),
  -- strictly after
  OPERATOR  30    #>> (tpose, tstzspan),
  OPERATOR  30    #>> (tpose, stbox),
  OPERATOR  30    #>> (tpose, tpose),
  -- overlaps or after
  OPERATOR  31    #&> (tpose, tstzspan),
  OPERATOR  31    #&> (tpose, stbox),
  OPERATOR  31    #&> (tpose, tpose),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_kdtree_choose(internal, internal),
  FUNCTION  3 stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4 stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/
