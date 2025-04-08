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
 * @brief R-tree GiST and SP-GiST indexes for temporal rigid geometries
 */

/******************************************************************************/

CREATE FUNCTION trgeometry_gist_consistent(internal, trgeometry, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS trgeometry_rtree_ops
  DEFAULT FOR TYPE trgeometry USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (trgeometry, stbox),
  OPERATOR  1    << (trgeometry, trgeometry),
  -- overlaps or left
  OPERATOR  2    &< (trgeometry, stbox),
  OPERATOR  2    &< (trgeometry, trgeometry),
  -- overlaps
  OPERATOR  3    && (trgeometry, tstzspan),
  OPERATOR  3    && (trgeometry, stbox),
  OPERATOR  3    && (trgeometry, trgeometry),
  -- overlaps or right
  OPERATOR  4    &> (trgeometry, stbox),
  OPERATOR  4    &> (trgeometry, trgeometry),
    -- strictly right
  OPERATOR  5    >> (trgeometry, stbox),
  OPERATOR  5    >> (trgeometry, trgeometry),
    -- same
  OPERATOR  6    ~= (trgeometry, tstzspan),
  OPERATOR  6    ~= (trgeometry, stbox),
  OPERATOR  6    ~= (trgeometry, trgeometry),
  -- contains
  OPERATOR  7    @> (trgeometry, tstzspan),
  OPERATOR  7    @> (trgeometry, stbox),
  OPERATOR  7    @> (trgeometry, trgeometry),
  -- contained by
  OPERATOR  8    <@ (trgeometry, tstzspan),
  OPERATOR  8    <@ (trgeometry, stbox),
  OPERATOR  8    <@ (trgeometry, trgeometry),
  -- overlaps or below
  OPERATOR  9    &<| (trgeometry, stbox),
  OPERATOR  9    &<| (trgeometry, trgeometry),
  -- strictly below
  OPERATOR  10    <<| (trgeometry, stbox),
  OPERATOR  10    <<| (trgeometry, trgeometry),
  -- strictly above
  OPERATOR  11    |>> (trgeometry, stbox),
  OPERATOR  11    |>> (trgeometry, trgeometry),
  -- overlaps or above
  OPERATOR  12    |&> (trgeometry, stbox),
  OPERATOR  12    |&> (trgeometry, trgeometry),
  -- adjacent
  OPERATOR  17    -|- (trgeometry, tstzspan),
  OPERATOR  17    -|- (trgeometry, stbox),
  OPERATOR  17    -|- (trgeometry, trgeometry),
  -- nearest approach distance
--  OPERATOR  25    |=| (trgeometry, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (trgeometry, tstzspan),
  OPERATOR  28    &<# (trgeometry, stbox),
  OPERATOR  28    &<# (trgeometry, trgeometry),
  -- strictly before
  OPERATOR  29    <<# (trgeometry, tstzspan),
  OPERATOR  29    <<# (trgeometry, stbox),
  OPERATOR  29    <<# (trgeometry, trgeometry),
  -- strictly after
  OPERATOR  30    #>> (trgeometry, tstzspan),
  OPERATOR  30    #>> (trgeometry, stbox),
  OPERATOR  30    #>> (trgeometry, trgeometry),
  -- overlaps or after
  OPERATOR  31    #&> (trgeometry, tstzspan),
  OPERATOR  31    #&> (trgeometry, stbox),
  OPERATOR  31    #&> (trgeometry, trgeometry),
  -- functions
  FUNCTION  1 trgeometry_gist_consistent(internal, trgeometry, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tspatial_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal);
--  FUNCTION  8 gist_trgeometry_distance(internal, trgeometry, smallint, oid, internal),

/******************************************************************************/

CREATE OPERATOR CLASS trgeometry_quadtree_ops
  DEFAULT FOR TYPE trgeometry USING spgist AS
  -- strictly left
  OPERATOR  1    << (trgeometry, stbox),
  OPERATOR  1    << (trgeometry, trgeometry),
  -- overlaps or left
  OPERATOR  2    &< (trgeometry, stbox),
  OPERATOR  2    &< (trgeometry, trgeometry),
  -- overlaps
  OPERATOR  3    && (trgeometry, tstzspan),
  OPERATOR  3    && (trgeometry, stbox),
  OPERATOR  3    && (trgeometry, trgeometry),
  -- overlaps or right
  OPERATOR  4    &> (trgeometry, stbox),
  OPERATOR  4    &> (trgeometry, trgeometry),
    -- strictly right
  OPERATOR  5    >> (trgeometry, stbox),
  OPERATOR  5    >> (trgeometry, trgeometry),
    -- same
  OPERATOR  6    ~= (trgeometry, tstzspan),
  OPERATOR  6    ~= (trgeometry, stbox),
  OPERATOR  6    ~= (trgeometry, trgeometry),
  -- contains
  OPERATOR  7    @> (trgeometry, tstzspan),
  OPERATOR  7    @> (trgeometry, stbox),
  OPERATOR  7    @> (trgeometry, trgeometry),
  -- contained by
  OPERATOR  8    <@ (trgeometry, tstzspan),
  OPERATOR  8    <@ (trgeometry, stbox),
  OPERATOR  8    <@ (trgeometry, trgeometry),
  -- overlaps or below
  OPERATOR  9    &<| (trgeometry, stbox),
  OPERATOR  9    &<| (trgeometry, trgeometry),
  -- strictly below
  OPERATOR  10    <<| (trgeometry, stbox),
  OPERATOR  10    <<| (trgeometry, trgeometry),
  -- strictly above
  OPERATOR  11    |>> (trgeometry, stbox),
  OPERATOR  11    |>> (trgeometry, trgeometry),
  -- overlaps or above
  OPERATOR  12    |&> (trgeometry, stbox),
  OPERATOR  12    |&> (trgeometry, trgeometry),
  -- adjacent
  OPERATOR  17    -|- (trgeometry, tstzspan),
  OPERATOR  17    -|- (trgeometry, stbox),
  OPERATOR  17    -|- (trgeometry, trgeometry),
  -- nearest approach distance
--  OPERATOR  25    |=| (trgeometry, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (trgeometry, tstzspan),
  OPERATOR  28    &<# (trgeometry, stbox),
  OPERATOR  28    &<# (trgeometry, trgeometry),
  -- strictly before
  OPERATOR  29    <<# (trgeometry, tstzspan),
  OPERATOR  29    <<# (trgeometry, stbox),
  OPERATOR  29    <<# (trgeometry, trgeometry),
  -- strictly after
  OPERATOR  30    #>> (trgeometry, tstzspan),
  OPERATOR  30    #>> (trgeometry, stbox),
  OPERATOR  30    #>> (trgeometry, trgeometry),
  -- overlaps or after
  OPERATOR  31    #&> (trgeometry, tstzspan),
  OPERATOR  31    #&> (trgeometry, stbox),
  OPERATOR  31    #&> (trgeometry, trgeometry),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_quadtree_choose(internal, internal),
  FUNCTION  3 stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4 stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS trgeometry_kdtree_ops
  FOR TYPE trgeometry USING spgist AS
  -- strictly left
  OPERATOR  1    << (trgeometry, stbox),
  OPERATOR  1    << (trgeometry, trgeometry),
  -- overlaps or left
  OPERATOR  2    &< (trgeometry, stbox),
  OPERATOR  2    &< (trgeometry, trgeometry),
  -- overlaps
  OPERATOR  3    && (trgeometry, tstzspan),
  OPERATOR  3    && (trgeometry, stbox),
  OPERATOR  3    && (trgeometry, trgeometry),
  -- overlaps or right
  OPERATOR  4    &> (trgeometry, stbox),
  OPERATOR  4    &> (trgeometry, trgeometry),
    -- strictly right
  OPERATOR  5    >> (trgeometry, stbox),
  OPERATOR  5    >> (trgeometry, trgeometry),
    -- same
  OPERATOR  6    ~= (trgeometry, tstzspan),
  OPERATOR  6    ~= (trgeometry, stbox),
  OPERATOR  6    ~= (trgeometry, trgeometry),
  -- contains
  OPERATOR  7    @> (trgeometry, tstzspan),
  OPERATOR  7    @> (trgeometry, stbox),
  OPERATOR  7    @> (trgeometry, trgeometry),
  -- contained by
  OPERATOR  8    <@ (trgeometry, tstzspan),
  OPERATOR  8    <@ (trgeometry, stbox),
  OPERATOR  8    <@ (trgeometry, trgeometry),
  -- overlaps or below
  OPERATOR  9    &<| (trgeometry, stbox),
  OPERATOR  9    &<| (trgeometry, trgeometry),
  -- strictly below
  OPERATOR  10    <<| (trgeometry, stbox),
  OPERATOR  10    <<| (trgeometry, trgeometry),
  -- strictly above
  OPERATOR  11    |>> (trgeometry, stbox),
  OPERATOR  11    |>> (trgeometry, trgeometry),
  -- overlaps or above
  OPERATOR  12    |&> (trgeometry, stbox),
  OPERATOR  12    |&> (trgeometry, trgeometry),
  -- adjacent
  OPERATOR  17    -|- (trgeometry, tstzspan),
  OPERATOR  17    -|- (trgeometry, stbox),
  OPERATOR  17    -|- (trgeometry, trgeometry),
  -- nearest approach distance
--  OPERATOR  25    |=| (trgeometry, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (trgeometry, tstzspan),
  OPERATOR  28    &<# (trgeometry, stbox),
  OPERATOR  28    &<# (trgeometry, trgeometry),
  -- strictly before
  OPERATOR  29    <<# (trgeometry, tstzspan),
  OPERATOR  29    <<# (trgeometry, stbox),
  OPERATOR  29    <<# (trgeometry, trgeometry),
  -- strictly after
  OPERATOR  30    #>> (trgeometry, tstzspan),
  OPERATOR  30    #>> (trgeometry, stbox),
  OPERATOR  30    #>> (trgeometry, trgeometry),
  -- overlaps or after
  OPERATOR  31    #&> (trgeometry, tstzspan),
  OPERATOR  31    #&> (trgeometry, stbox),
  OPERATOR  31    #&> (trgeometry, trgeometry),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_kdtree_choose(internal, internal),
  FUNCTION  3 stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4 stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/
