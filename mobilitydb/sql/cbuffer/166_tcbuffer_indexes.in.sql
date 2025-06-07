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
 * @brief R-tree GiST and SP-GiST indexes for temporal circular buffers
 */

/******************************************************************************/

CREATE FUNCTION tcbuffer_gist_consistent(internal, tcbuffer, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS tcbuffer_rtree_ops
  DEFAULT FOR TYPE tcbuffer USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (tcbuffer, stbox),
  OPERATOR  1    << (tcbuffer, tcbuffer),
  -- overlaps or left
  OPERATOR  2    &< (tcbuffer, stbox),
  OPERATOR  2    &< (tcbuffer, tcbuffer),
  -- overlaps
  OPERATOR  3    && (tcbuffer, tstzspan),
  OPERATOR  3    && (tcbuffer, stbox),
  OPERATOR  3    && (tcbuffer, tcbuffer),
  -- overlaps or right
  OPERATOR  4    &> (tcbuffer, stbox),
  OPERATOR  4    &> (tcbuffer, tcbuffer),
    -- strictly right
  OPERATOR  5    >> (tcbuffer, stbox),
  OPERATOR  5    >> (tcbuffer, tcbuffer),
    -- same
  OPERATOR  6    ~= (tcbuffer, tstzspan),
  OPERATOR  6    ~= (tcbuffer, stbox),
  OPERATOR  6    ~= (tcbuffer, tcbuffer),
  -- contains
  OPERATOR  7    @> (tcbuffer, tstzspan),
  OPERATOR  7    @> (tcbuffer, stbox),
  OPERATOR  7    @> (tcbuffer, tcbuffer),
  -- contained by
  OPERATOR  8    <@ (tcbuffer, tstzspan),
  OPERATOR  8    <@ (tcbuffer, stbox),
  OPERATOR  8    <@ (tcbuffer, tcbuffer),
  -- overlaps or below
  OPERATOR  9    &<| (tcbuffer, stbox),
  OPERATOR  9    &<| (tcbuffer, tcbuffer),
  -- strictly below
  OPERATOR  10    <<| (tcbuffer, stbox),
  OPERATOR  10    <<| (tcbuffer, tcbuffer),
  -- strictly above
  OPERATOR  11    |>> (tcbuffer, stbox),
  OPERATOR  11    |>> (tcbuffer, tcbuffer),
  -- overlaps or above
  OPERATOR  12    |&> (tcbuffer, stbox),
  OPERATOR  12    |&> (tcbuffer, tcbuffer),
  -- adjacent
  OPERATOR  17    -|- (tcbuffer, tstzspan),
  OPERATOR  17    -|- (tcbuffer, stbox),
  OPERATOR  17    -|- (tcbuffer, tcbuffer),
  -- nearest approach distance
--  OPERATOR  25    |=| (tcbuffer, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tcbuffer, tstzspan),
  OPERATOR  28    &<# (tcbuffer, stbox),
  OPERATOR  28    &<# (tcbuffer, tcbuffer),
  -- strictly before
  OPERATOR  29    <<# (tcbuffer, tstzspan),
  OPERATOR  29    <<# (tcbuffer, stbox),
  OPERATOR  29    <<# (tcbuffer, tcbuffer),
  -- strictly after
  OPERATOR  30    #>> (tcbuffer, tstzspan),
  OPERATOR  30    #>> (tcbuffer, stbox),
  OPERATOR  30    #>> (tcbuffer, tcbuffer),
  -- overlaps or after
  OPERATOR  31    #&> (tcbuffer, tstzspan),
  OPERATOR  31    #&> (tcbuffer, stbox),
  OPERATOR  31    #&> (tcbuffer, tcbuffer),
  -- functions
  FUNCTION  1 tcbuffer_gist_consistent(internal, tcbuffer, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tspatial_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal);
--  FUNCTION  8 gist_tcbuffer_distance(internal, tcbuffer, smallint, oid, internal),

/******************************************************************************/

CREATE OPERATOR CLASS tcbuffer_quadtree_ops
  DEFAULT FOR TYPE tcbuffer USING spgist AS
  -- strictly left
  OPERATOR  1    << (tcbuffer, stbox),
  OPERATOR  1    << (tcbuffer, tcbuffer),
  -- overlaps or left
  OPERATOR  2    &< (tcbuffer, stbox),
  OPERATOR  2    &< (tcbuffer, tcbuffer),
  -- overlaps
  OPERATOR  3    && (tcbuffer, tstzspan),
  OPERATOR  3    && (tcbuffer, stbox),
  OPERATOR  3    && (tcbuffer, tcbuffer),
  -- overlaps or right
  OPERATOR  4    &> (tcbuffer, stbox),
  OPERATOR  4    &> (tcbuffer, tcbuffer),
    -- strictly right
  OPERATOR  5    >> (tcbuffer, stbox),
  OPERATOR  5    >> (tcbuffer, tcbuffer),
    -- same
  OPERATOR  6    ~= (tcbuffer, tstzspan),
  OPERATOR  6    ~= (tcbuffer, stbox),
  OPERATOR  6    ~= (tcbuffer, tcbuffer),
  -- contains
  OPERATOR  7    @> (tcbuffer, tstzspan),
  OPERATOR  7    @> (tcbuffer, stbox),
  OPERATOR  7    @> (tcbuffer, tcbuffer),
  -- contained by
  OPERATOR  8    <@ (tcbuffer, tstzspan),
  OPERATOR  8    <@ (tcbuffer, stbox),
  OPERATOR  8    <@ (tcbuffer, tcbuffer),
  -- overlaps or below
  OPERATOR  9    &<| (tcbuffer, stbox),
  OPERATOR  9    &<| (tcbuffer, tcbuffer),
  -- strictly below
  OPERATOR  10    <<| (tcbuffer, stbox),
  OPERATOR  10    <<| (tcbuffer, tcbuffer),
  -- strictly above
  OPERATOR  11    |>> (tcbuffer, stbox),
  OPERATOR  11    |>> (tcbuffer, tcbuffer),
  -- overlaps or above
  OPERATOR  12    |&> (tcbuffer, stbox),
  OPERATOR  12    |&> (tcbuffer, tcbuffer),
  -- adjacent
  OPERATOR  17    -|- (tcbuffer, tstzspan),
  OPERATOR  17    -|- (tcbuffer, stbox),
  OPERATOR  17    -|- (tcbuffer, tcbuffer),
  -- nearest approach distance
--  OPERATOR  25    |=| (tcbuffer, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tcbuffer, tstzspan),
  OPERATOR  28    &<# (tcbuffer, stbox),
  OPERATOR  28    &<# (tcbuffer, tcbuffer),
  -- strictly before
  OPERATOR  29    <<# (tcbuffer, tstzspan),
  OPERATOR  29    <<# (tcbuffer, stbox),
  OPERATOR  29    <<# (tcbuffer, tcbuffer),
  -- strictly after
  OPERATOR  30    #>> (tcbuffer, tstzspan),
  OPERATOR  30    #>> (tcbuffer, stbox),
  OPERATOR  30    #>> (tcbuffer, tcbuffer),
  -- overlaps or after
  OPERATOR  31    #&> (tcbuffer, tstzspan),
  OPERATOR  31    #&> (tcbuffer, stbox),
  OPERATOR  31    #&> (tcbuffer, tcbuffer),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_quadtree_choose(internal, internal),
  FUNCTION  3 stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4 stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS tcbuffer_kdtree_ops
  FOR TYPE tcbuffer USING spgist AS
  -- strictly left
  OPERATOR  1    << (tcbuffer, stbox),
  OPERATOR  1    << (tcbuffer, tcbuffer),
  -- overlaps or left
  OPERATOR  2    &< (tcbuffer, stbox),
  OPERATOR  2    &< (tcbuffer, tcbuffer),
  -- overlaps
  OPERATOR  3    && (tcbuffer, tstzspan),
  OPERATOR  3    && (tcbuffer, stbox),
  OPERATOR  3    && (tcbuffer, tcbuffer),
  -- overlaps or right
  OPERATOR  4    &> (tcbuffer, stbox),
  OPERATOR  4    &> (tcbuffer, tcbuffer),
    -- strictly right
  OPERATOR  5    >> (tcbuffer, stbox),
  OPERATOR  5    >> (tcbuffer, tcbuffer),
    -- same
  OPERATOR  6    ~= (tcbuffer, tstzspan),
  OPERATOR  6    ~= (tcbuffer, stbox),
  OPERATOR  6    ~= (tcbuffer, tcbuffer),
  -- contains
  OPERATOR  7    @> (tcbuffer, tstzspan),
  OPERATOR  7    @> (tcbuffer, stbox),
  OPERATOR  7    @> (tcbuffer, tcbuffer),
  -- contained by
  OPERATOR  8    <@ (tcbuffer, tstzspan),
  OPERATOR  8    <@ (tcbuffer, stbox),
  OPERATOR  8    <@ (tcbuffer, tcbuffer),
  -- overlaps or below
  OPERATOR  9    &<| (tcbuffer, stbox),
  OPERATOR  9    &<| (tcbuffer, tcbuffer),
  -- strictly below
  OPERATOR  10    <<| (tcbuffer, stbox),
  OPERATOR  10    <<| (tcbuffer, tcbuffer),
  -- strictly above
  OPERATOR  11    |>> (tcbuffer, stbox),
  OPERATOR  11    |>> (tcbuffer, tcbuffer),
  -- overlaps or above
  OPERATOR  12    |&> (tcbuffer, stbox),
  OPERATOR  12    |&> (tcbuffer, tcbuffer),
  -- adjacent
  OPERATOR  17    -|- (tcbuffer, tstzspan),
  OPERATOR  17    -|- (tcbuffer, stbox),
  OPERATOR  17    -|- (tcbuffer, tcbuffer),
  -- nearest approach distance
--  OPERATOR  25    |=| (tcbuffer, stbox) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tcbuffer, tstzspan),
  OPERATOR  28    &<# (tcbuffer, stbox),
  OPERATOR  28    &<# (tcbuffer, tcbuffer),
  -- strictly before
  OPERATOR  29    <<# (tcbuffer, tstzspan),
  OPERATOR  29    <<# (tcbuffer, stbox),
  OPERATOR  29    <<# (tcbuffer, tcbuffer),
  -- strictly after
  OPERATOR  30    #>> (tcbuffer, tstzspan),
  OPERATOR  30    #>> (tcbuffer, stbox),
  OPERATOR  30    #>> (tcbuffer, tcbuffer),
  -- overlaps or after
  OPERATOR  31    #&> (tcbuffer, tstzspan),
  OPERATOR  31    #&> (tcbuffer, stbox),
  OPERATOR  31    #&> (tcbuffer, tcbuffer),
  -- functions
  FUNCTION  1 stbox_spgist_config(internal, internal),
  FUNCTION  2 stbox_kdtree_choose(internal, internal),
  FUNCTION  3 stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4 stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5 stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6 tspatial_spgist_compress(internal);

/******************************************************************************/
