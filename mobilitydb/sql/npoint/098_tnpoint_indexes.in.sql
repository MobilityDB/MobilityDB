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

/**
 * tnpoint_indexes.sql
 * R-tree GiST and SP-GiST indexes for temporal network points.
 */

/******************************************************************************/

CREATE FUNCTION tnpoint_gist_consistent(internal, tnpoint, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS tnpoint_rtree_ops
  DEFAULT FOR TYPE tnpoint USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (tnpoint, stbox),
  OPERATOR  1    << (tnpoint, tnpoint),
  -- overlaps or left
  OPERATOR  2    &< (tnpoint, stbox),
  OPERATOR  2    &< (tnpoint, tnpoint),
  -- overlaps
  OPERATOR  3    && (tnpoint, tstzspan),
  OPERATOR  3    && (tnpoint, stbox),
  OPERATOR  3    && (tnpoint, tnpoint),
  -- overlaps or right
  OPERATOR  4    &> (tnpoint, stbox),
  OPERATOR  4    &> (tnpoint, tnpoint),
    -- strictly right
  OPERATOR  5    >> (tnpoint, stbox),
  OPERATOR  5    >> (tnpoint, tnpoint),
    -- same
  OPERATOR  6    ~= (tnpoint, tstzspan),
  OPERATOR  6    ~= (tnpoint, stbox),
  OPERATOR  6    ~= (tnpoint, tnpoint),
  -- contains
  OPERATOR  7    @> (tnpoint, tstzspan),
  OPERATOR  7    @> (tnpoint, stbox),
  OPERATOR  7    @> (tnpoint, tnpoint),
  -- contained by
  OPERATOR  8    <@ (tnpoint, tstzspan),
  OPERATOR  8    <@ (tnpoint, stbox),
  OPERATOR  8    <@ (tnpoint, tnpoint),
  -- overlaps or below
  OPERATOR  9    &<| (tnpoint, stbox),
  OPERATOR  9    &<| (tnpoint, tnpoint),
  -- strictly below
  OPERATOR  10    <<| (tnpoint, stbox),
  OPERATOR  10    <<| (tnpoint, tnpoint),
  -- strictly above
  OPERATOR  11    |>> (tnpoint, stbox),
  OPERATOR  11    |>> (tnpoint, tnpoint),
  -- overlaps or above
  OPERATOR  12    |&> (tnpoint, stbox),
  OPERATOR  12    |&> (tnpoint, tnpoint),
  -- adjacent
  OPERATOR  17    -|- (tnpoint, tstzspan),
  OPERATOR  17    -|- (tnpoint, stbox),
  OPERATOR  17    -|- (tnpoint, tnpoint),
  -- nearest approach distance
--  OPERATOR  25    |=| (tnpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tnpoint, tstzspan),
  OPERATOR  28    &<# (tnpoint, stbox),
  OPERATOR  28    &<# (tnpoint, tnpoint),
  -- strictly before
  OPERATOR  29    <<# (tnpoint, tstzspan),
  OPERATOR  29    <<# (tnpoint, stbox),
  OPERATOR  29    <<# (tnpoint, tnpoint),
  -- strictly after
  OPERATOR  30    #>> (tnpoint, tstzspan),
  OPERATOR  30    #>> (tnpoint, stbox),
  OPERATOR  30    #>> (tnpoint, tnpoint),
  -- overlaps or after
  OPERATOR  31    #&> (tnpoint, tstzspan),
  OPERATOR  31    #&> (tnpoint, stbox),
  OPERATOR  31    #&> (tnpoint, tnpoint),
  -- functions
  FUNCTION  1 tnpoint_gist_consistent(internal, tnpoint, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tpoint_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal);
--  FUNCTION  8 gist_tnpoint_distance(internal, tnpoint, smallint, oid, internal),

/******************************************************************************/

CREATE OPERATOR CLASS tnpoint_quadtree_ops
  DEFAULT FOR TYPE tnpoint USING spgist AS
  -- strictly left
  OPERATOR  1    << (tnpoint, stbox),
  OPERATOR  1    << (tnpoint, tnpoint),
  -- overlaps or left
  OPERATOR  2    &< (tnpoint, stbox),
  OPERATOR  2    &< (tnpoint, tnpoint),
  -- overlaps
  OPERATOR  3    && (tnpoint, tstzspan),
  OPERATOR  3    && (tnpoint, stbox),
  OPERATOR  3    && (tnpoint, tnpoint),
  -- overlaps or right
  OPERATOR  4    &> (tnpoint, stbox),
  OPERATOR  4    &> (tnpoint, tnpoint),
    -- strictly right
  OPERATOR  5    >> (tnpoint, stbox),
  OPERATOR  5    >> (tnpoint, tnpoint),
    -- same
  OPERATOR  6    ~= (tnpoint, tstzspan),
  OPERATOR  6    ~= (tnpoint, stbox),
  OPERATOR  6    ~= (tnpoint, tnpoint),
  -- contains
  OPERATOR  7    @> (tnpoint, tstzspan),
  OPERATOR  7    @> (tnpoint, stbox),
  OPERATOR  7    @> (tnpoint, tnpoint),
  -- contained by
  OPERATOR  8    <@ (tnpoint, tstzspan),
  OPERATOR  8    <@ (tnpoint, stbox),
  OPERATOR  8    <@ (tnpoint, tnpoint),
  -- overlaps or below
  OPERATOR  9    &<| (tnpoint, stbox),
  OPERATOR  9    &<| (tnpoint, tnpoint),
  -- strictly below
  OPERATOR  10    <<| (tnpoint, stbox),
  OPERATOR  10    <<| (tnpoint, tnpoint),
  -- strictly above
  OPERATOR  11    |>> (tnpoint, stbox),
  OPERATOR  11    |>> (tnpoint, tnpoint),
  -- overlaps or above
  OPERATOR  12    |&> (tnpoint, stbox),
  OPERATOR  12    |&> (tnpoint, tnpoint),
  -- adjacent
  OPERATOR  17    -|- (tnpoint, tstzspan),
  OPERATOR  17    -|- (tnpoint, stbox),
  OPERATOR  17    -|- (tnpoint, tnpoint),
  -- nearest approach distance
--  OPERATOR  25    |=| (tnpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tnpoint, tstzspan),
  OPERATOR  28    &<# (tnpoint, stbox),
  OPERATOR  28    &<# (tnpoint, tnpoint),
  -- strictly before
  OPERATOR  29    <<# (tnpoint, tstzspan),
  OPERATOR  29    <<# (tnpoint, stbox),
  OPERATOR  29    <<# (tnpoint, tnpoint),
  -- strictly after
  OPERATOR  30    #>> (tnpoint, tstzspan),
  OPERATOR  30    #>> (tnpoint, stbox),
  OPERATOR  30    #>> (tnpoint, tnpoint),
  -- overlaps or after
  OPERATOR  31    #&> (tnpoint, tstzspan),
  OPERATOR  31    #&> (tnpoint, stbox),
  OPERATOR  31    #&> (tnpoint, tnpoint),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_quadtree_choose(internal, internal),
  FUNCTION  3 stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4 stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tpoint_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS tnpoint_kdtree_ops
  FOR TYPE tnpoint USING spgist AS
  -- strictly left
  OPERATOR  1    << (tnpoint, stbox),
  OPERATOR  1    << (tnpoint, tnpoint),
  -- overlaps or left
  OPERATOR  2    &< (tnpoint, stbox),
  OPERATOR  2    &< (tnpoint, tnpoint),
  -- overlaps
  OPERATOR  3    && (tnpoint, tstzspan),
  OPERATOR  3    && (tnpoint, stbox),
  OPERATOR  3    && (tnpoint, tnpoint),
  -- overlaps or right
  OPERATOR  4    &> (tnpoint, stbox),
  OPERATOR  4    &> (tnpoint, tnpoint),
    -- strictly right
  OPERATOR  5    >> (tnpoint, stbox),
  OPERATOR  5    >> (tnpoint, tnpoint),
    -- same
  OPERATOR  6    ~= (tnpoint, tstzspan),
  OPERATOR  6    ~= (tnpoint, stbox),
  OPERATOR  6    ~= (tnpoint, tnpoint),
  -- contains
  OPERATOR  7    @> (tnpoint, tstzspan),
  OPERATOR  7    @> (tnpoint, stbox),
  OPERATOR  7    @> (tnpoint, tnpoint),
  -- contained by
  OPERATOR  8    <@ (tnpoint, tstzspan),
  OPERATOR  8    <@ (tnpoint, stbox),
  OPERATOR  8    <@ (tnpoint, tnpoint),
  -- overlaps or below
  OPERATOR  9    &<| (tnpoint, stbox),
  OPERATOR  9    &<| (tnpoint, tnpoint),
  -- strictly below
  OPERATOR  10    <<| (tnpoint, stbox),
  OPERATOR  10    <<| (tnpoint, tnpoint),
  -- strictly above
  OPERATOR  11    |>> (tnpoint, stbox),
  OPERATOR  11    |>> (tnpoint, tnpoint),
  -- overlaps or above
  OPERATOR  12    |&> (tnpoint, stbox),
  OPERATOR  12    |&> (tnpoint, tnpoint),
  -- adjacent
  OPERATOR  17    -|- (tnpoint, tstzspan),
  OPERATOR  17    -|- (tnpoint, stbox),
  OPERATOR  17    -|- (tnpoint, tnpoint),
  -- nearest approach distance
--  OPERATOR  25    |=| (tnpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tnpoint, tstzspan),
  OPERATOR  28    &<# (tnpoint, stbox),
  OPERATOR  28    &<# (tnpoint, tnpoint),
  -- strictly before
  OPERATOR  29    <<# (tnpoint, tstzspan),
  OPERATOR  29    <<# (tnpoint, stbox),
  OPERATOR  29    <<# (tnpoint, tnpoint),
  -- strictly after
  OPERATOR  30    #>> (tnpoint, tstzspan),
  OPERATOR  30    #>> (tnpoint, stbox),
  OPERATOR  30    #>> (tnpoint, tnpoint),
  -- overlaps or after
  OPERATOR  31    #&> (tnpoint, tstzspan),
  OPERATOR  31    #&> (tnpoint, stbox),
  OPERATOR  31    #&> (tnpoint, tnpoint),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_kdtree_choose(internal, internal),
  FUNCTION  3 stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4 stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tpoint_spgist_compress(internal);

/******************************************************************************/
