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
 * @brief GiST operator class for `th3index`.
 *
 * H3 cells are geographic hexagons on the WGS84 sphere — always geodetic,
 * always lat/lon. The bounding box of a `th3index` value is therefore a
 * geodetic `stbox`, matching the `tgeogpoint` / `tcbuffer` pattern. This
 * opclass mirrors `tcbuffer_rtree_ops` from
 * `mobilitydb/sql/cbuffer/166_tcbuffer_indexes.in.sql`.
 *
 * The consistent / compress support functions are the generic
 * `Stbox_gist_consistent` and `tspatial_gist_compress` symbols; all four
 * stbox-level helpers (`stbox_gist_union` / `penalty` / `picksplit` / `same`)
 * are reused directly. No new C symbols are introduced.
 */

/******************************************************************************
 * Support functions (th3index-typed wrappers)
 ******************************************************************************/

CREATE FUNCTION th3index_gist_consistent(internal, th3index, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operator class
 ******************************************************************************/

CREATE OPERATOR CLASS th3index_rtree_ops
  DEFAULT FOR TYPE th3index USING gist AS
  STORAGE stbox,
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
  FUNCTION  1 th3index_gist_consistent(internal, th3index, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tspatial_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal);

/******************************************************************************/
