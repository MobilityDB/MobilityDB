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
 * @brief Quad-tree and k-d tree SP-GiST index for temporal 
 * geometries/geographies
 */

CREATE FUNCTION stbox_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_quadtree_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_quadtree_choose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_quadtree_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_quadtree_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_quadtree_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_quadtree_inner_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_spgist_leaf_consistent(internal, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_spgist_leaf_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tspatial_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tspatial_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/******************************************************************************/

CREATE OPERATOR CLASS tgeometry_quadtree_ops
  DEFAULT FOR TYPE tgeometry USING spgist AS
  -- strictly left
  OPERATOR  1    << (tgeometry, stbox),
  OPERATOR  1    << (tgeometry, tgeometry),
  -- overlaps or left
  OPERATOR  2    &< (tgeometry, stbox),
  OPERATOR  2    &< (tgeometry, tgeometry),
  -- overlaps
  OPERATOR  3    && (tgeometry, tstzspan),
  OPERATOR  3    && (tgeometry, stbox),
  OPERATOR  3    && (tgeometry, tgeometry),
  -- overlaps or right
  OPERATOR  4    &> (tgeometry, stbox),
  OPERATOR  4    &> (tgeometry, tgeometry),
    -- strictly right
  OPERATOR  5    >> (tgeometry, stbox),
  OPERATOR  5    >> (tgeometry, tgeometry),
    -- same
  OPERATOR  6    ~= (tgeometry, tstzspan),
  OPERATOR  6    ~= (tgeometry, stbox),
  OPERATOR  6    ~= (tgeometry, tgeometry),
  -- contains
  OPERATOR  7    @> (tgeometry, tstzspan),
  OPERATOR  7    @> (tgeometry, stbox),
  OPERATOR  7    @> (tgeometry, tgeometry),
  -- contained by
  OPERATOR  8    <@ (tgeometry, tstzspan),
  OPERATOR  8    <@ (tgeometry, stbox),
  OPERATOR  8    <@ (tgeometry, tgeometry),
  -- overlaps or below
  OPERATOR  9    &<| (tgeometry, stbox),
  OPERATOR  9    &<| (tgeometry, tgeometry),
  -- strictly below
  OPERATOR  10    <<| (tgeometry, stbox),
  OPERATOR  10    <<| (tgeometry, tgeometry),
  -- strictly above
  OPERATOR  11    |>> (tgeometry, stbox),
  OPERATOR  11    |>> (tgeometry, tgeometry),
  -- overlaps or above
  OPERATOR  12    |&> (tgeometry, stbox),
  OPERATOR  12    |&> (tgeometry, tgeometry),
  -- adjacent
  OPERATOR  17    -|- (tgeometry, tstzspan),
  OPERATOR  17    -|- (tgeometry, stbox),
  OPERATOR  17    -|- (tgeometry, tgeometry),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeometry, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeometry, tgeometry) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeometry, tstzspan),
  OPERATOR  28    &<# (tgeometry, stbox),
  OPERATOR  28    &<# (tgeometry, tgeometry),
  -- strictly before
  OPERATOR  29    <<# (tgeometry, tstzspan),
  OPERATOR  29    <<# (tgeometry, stbox),
  OPERATOR  29    <<# (tgeometry, tgeometry),
  -- strictly after
  OPERATOR  30    #>> (tgeometry, tstzspan),
  OPERATOR  30    #>> (tgeometry, stbox),
  OPERATOR  30    #>> (tgeometry, tgeometry),
  -- overlaps or after
  OPERATOR  31    #&> (tgeometry, tstzspan),
  OPERATOR  31    #&> (tgeometry, stbox),
  OPERATOR  31    #&> (tgeometry, tgeometry),
  -- overlaps or front
  OPERATOR  32    &</ (tgeometry, stbox),
  OPERATOR  32    &</ (tgeometry, tgeometry),
  -- strictly front
  OPERATOR  33    <</ (tgeometry, stbox),
  OPERATOR  33    <</ (tgeometry, tgeometry),
  -- strictly back
  OPERATOR  34    />> (tgeometry, stbox),
  OPERATOR  34    />> (tgeometry, tgeometry),
  -- overlaps or back
  OPERATOR  35    /&> (tgeometry, stbox),
  OPERATOR  35    /&> (tgeometry, tgeometry),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_quadtree_choose(internal, internal),
  FUNCTION  3  stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4  stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  tspatial_spgist_compress(internal);

/******************************************************************************/

CREATE FUNCTION stbox_kdtree_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_kdtree_choose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_kdtree_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_kdtree_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_kdtree_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Stbox_kdtree_inner_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS tgeometry_kdtree_ops
  FOR TYPE tgeometry USING spgist AS
  -- strictly left
  OPERATOR  1    << (tgeometry, stbox),
  OPERATOR  1    << (tgeometry, tgeometry),
  -- overlaps or left
  OPERATOR  2    &< (tgeometry, stbox),
  OPERATOR  2    &< (tgeometry, tgeometry),
  -- overlaps
  OPERATOR  3    && (tgeometry, tstzspan),
  OPERATOR  3    && (tgeometry, stbox),
  OPERATOR  3    && (tgeometry, tgeometry),
  -- overlaps or right
  OPERATOR  4    &> (tgeometry, stbox),
  OPERATOR  4    &> (tgeometry, tgeometry),
    -- strictly right
  OPERATOR  5    >> (tgeometry, stbox),
  OPERATOR  5    >> (tgeometry, tgeometry),
    -- same
  OPERATOR  6    ~= (tgeometry, tstzspan),
  OPERATOR  6    ~= (tgeometry, stbox),
  OPERATOR  6    ~= (tgeometry, tgeometry),
  -- contains
  OPERATOR  7    @> (tgeometry, tstzspan),
  OPERATOR  7    @> (tgeometry, stbox),
  OPERATOR  7    @> (tgeometry, tgeometry),
  -- contained by
  OPERATOR  8    <@ (tgeometry, tstzspan),
  OPERATOR  8    <@ (tgeometry, stbox),
  OPERATOR  8    <@ (tgeometry, tgeometry),
  -- overlaps or below
  OPERATOR  9    &<| (tgeometry, stbox),
  OPERATOR  9    &<| (tgeometry, tgeometry),
  -- strictly below
  OPERATOR  10    <<| (tgeometry, stbox),
  OPERATOR  10    <<| (tgeometry, tgeometry),
  -- strictly above
  OPERATOR  11    |>> (tgeometry, stbox),
  OPERATOR  11    |>> (tgeometry, tgeometry),
  -- overlaps or above
  OPERATOR  12    |&> (tgeometry, stbox),
  OPERATOR  12    |&> (tgeometry, tgeometry),
  -- adjacent
  OPERATOR  17    -|- (tgeometry, tstzspan),
  OPERATOR  17    -|- (tgeometry, stbox),
  OPERATOR  17    -|- (tgeometry, tgeometry),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeometry, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeometry, tgeometry) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeometry, tstzspan),
  OPERATOR  28    &<# (tgeometry, stbox),
  OPERATOR  28    &<# (tgeometry, tgeometry),
  -- strictly before
  OPERATOR  29    <<# (tgeometry, tstzspan),
  OPERATOR  29    <<# (tgeometry, stbox),
  OPERATOR  29    <<# (tgeometry, tgeometry),
  -- strictly after
  OPERATOR  30    #>> (tgeometry, tstzspan),
  OPERATOR  30    #>> (tgeometry, stbox),
  OPERATOR  30    #>> (tgeometry, tgeometry),
  -- overlaps or after
  OPERATOR  31    #&> (tgeometry, tstzspan),
  OPERATOR  31    #&> (tgeometry, stbox),
  OPERATOR  31    #&> (tgeometry, tgeometry),
  -- overlaps or front
  OPERATOR  32    &</ (tgeometry, stbox),
  OPERATOR  32    &</ (tgeometry, tgeometry),
  -- strictly front
  OPERATOR  33    <</ (tgeometry, stbox),
  OPERATOR  33    <</ (tgeometry, tgeometry),
  -- strictly back
  OPERATOR  34    />> (tgeometry, stbox),
  OPERATOR  34    />> (tgeometry, tgeometry),
  -- overlaps or back
  OPERATOR  35    /&> (tgeometry, stbox),
  OPERATOR  35    /&> (tgeometry, tgeometry),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_kdtree_choose(internal, internal),
  FUNCTION  3  stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4  stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  tspatial_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS tgeography_quadtree_ops
  DEFAULT FOR TYPE tgeography USING spgist AS
  -- overlaps
  OPERATOR  3    && (tgeography, tstzspan),
  OPERATOR  3    && (tgeography, stbox),
  OPERATOR  3    && (tgeography, tgeography),
    -- same
  OPERATOR  6    ~= (tgeography, tstzspan),
  OPERATOR  6    ~= (tgeography, stbox),
  OPERATOR  6    ~= (tgeography, tgeography),
  -- contains
  OPERATOR  7    @> (tgeography, tstzspan),
  OPERATOR  7    @> (tgeography, stbox),
  OPERATOR  7    @> (tgeography, tgeography),
  -- contained by
  OPERATOR  8    <@ (tgeography, tstzspan),
  OPERATOR  8    <@ (tgeography, stbox),
  OPERATOR  8    <@ (tgeography, tgeography),
  -- adjacent
  OPERATOR  17    -|- (tgeography, tstzspan),
  OPERATOR  17    -|- (tgeography, stbox),
  OPERATOR  17    -|- (tgeography, tgeography),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeography, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeography, tgeography) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeography, tstzspan),
  OPERATOR  28    &<# (tgeography, stbox),
  OPERATOR  28    &<# (tgeography, tgeography),
  -- strictly before
  OPERATOR  29    <<# (tgeography, tstzspan),
  OPERATOR  29    <<# (tgeography, stbox),
  OPERATOR  29    <<# (tgeography, tgeography),
  -- strictly after
  OPERATOR  30    #>> (tgeography, tstzspan),
  OPERATOR  30    #>> (tgeography, stbox),
  OPERATOR  30    #>> (tgeography, tgeography),
  -- overlaps or after
  OPERATOR  31    #&> (tgeography, tstzspan),
  OPERATOR  31    #&> (tgeography, stbox),
  OPERATOR  31    #&> (tgeography, tgeography),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_quadtree_choose(internal, internal),
  FUNCTION  3  stbox_quadtree_picksplit(internal, internal),
  FUNCTION  4  stbox_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  tspatial_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS tgeography_kdtree_ops
  FOR TYPE tgeography USING spgist AS
  -- overlaps
  OPERATOR  3    && (tgeography, tstzspan),
  OPERATOR  3    && (tgeography, tgeography),
    -- same
  OPERATOR  6    ~= (tgeography, tstzspan),
  OPERATOR  6    ~= (tgeography, stbox),
  OPERATOR  6    ~= (tgeography, tgeography),
  -- contains
  OPERATOR  7    @> (tgeography, tstzspan),
  OPERATOR  7    @> (tgeography, stbox),
  OPERATOR  7    @> (tgeography, tgeography),
  -- contained by
  OPERATOR  8    <@ (tgeography, tstzspan),
  OPERATOR  8    <@ (tgeography, stbox),
  OPERATOR  8    <@ (tgeography, tgeography),
  -- adjacent
  OPERATOR  17    -|- (tgeography, tstzspan),
  OPERATOR  17    -|- (tgeography, stbox),
  OPERATOR  17    -|- (tgeography, tgeography),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeography, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeography, tgeography) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeography, tstzspan),
  OPERATOR  28    &<# (tgeography, stbox),
  OPERATOR  28    &<# (tgeography, tgeography),
  -- strictly before
  OPERATOR  29    <<# (tgeography, tstzspan),
  OPERATOR  29    <<# (tgeography, stbox),
  OPERATOR  29    <<# (tgeography, tgeography),
  -- strictly after
  OPERATOR  30    #>> (tgeography, tstzspan),
  OPERATOR  30    #>> (tgeography, stbox),
  OPERATOR  30    #>> (tgeography, tgeography),
  -- overlaps or after
  OPERATOR  31    #&> (tgeography, tstzspan),
  OPERATOR  31    #&> (tgeography, stbox),
  OPERATOR  31    #&> (tgeography, tgeography),
  -- functions
  FUNCTION  1  stbox_spgist_config(internal, internal),
  FUNCTION  2  stbox_kdtree_choose(internal, internal),
  FUNCTION  3  stbox_kdtree_picksplit(internal, internal),
  FUNCTION  4  stbox_kdtree_inner_consistent(internal, internal),
  FUNCTION  5  stbox_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  tspatial_spgist_compress(internal);

/******************************************************************************/
