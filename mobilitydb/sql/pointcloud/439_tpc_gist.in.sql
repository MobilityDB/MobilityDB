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
 * @brief R-tree GiST indexes for point cloud boxes and temporal point clouds
 */

CREATE FUNCTION tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_gist_union(internal, internal)
  RETURNS tpcbox
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_union'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_gist_penalty(internal, internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_penalty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_gist_picksplit(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_gist_same(tpcbox, tpcbox, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpcbox_gist_sortsupport(internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_sortsupport'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tpcbox_rtree_ops
  DEFAULT FOR TYPE tpcbox USING gist AS
  STORAGE tpcbox,
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
  OPERATOR  10    <<| (tpcbox, tpcbox),
  -- strictly above
  OPERATOR  11    |>> (tpcbox, tpcbox),
  -- overlaps or above
  OPERATOR  12    |&> (tpcbox, tpcbox),
  -- adjacent
  OPERATOR  17    -|- (tpcbox, tpcbox),
  -- overlaps or before
  OPERATOR  28    &<# (tpcbox, tpcbox),
  -- strictly before
  OPERATOR  29    <<# (tpcbox, tpcbox),
  -- strictly after
  OPERATOR  30    #>> (tpcbox, tpcbox),
  -- overlaps or after
  OPERATOR  31    #&> (tpcbox, tpcbox),
  -- overlaps or front
  OPERATOR  32    &</ (tpcbox, tpcbox),
  -- strictly front
  OPERATOR  33    <</ (tpcbox, tpcbox),
  -- strictly back
  OPERATOR  34    />> (tpcbox, tpcbox),
  -- overlaps or back
  OPERATOR  35    /&> (tpcbox, tpcbox),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcbox, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal),
  FUNCTION  2  tpcbox_gist_union(internal, internal),
  FUNCTION  5  tpcbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  tpcbox_gist_picksplit(internal, internal),
  FUNCTION  7  tpcbox_gist_same(tpcbox, tpcbox, internal),
  FUNCTION  8  tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION 11 tpcbox_gist_sortsupport(internal);

/*****************************************************************************/

CREATE FUNCTION tpc_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpc_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tpcpoint
 *****************************************************************************/

CREATE OPERATOR CLASS tpcpoint_rtree_ops
  DEFAULT FOR TYPE tpcpoint USING gist AS
  STORAGE tpcbox,
  -- strictly left
  OPERATOR  1    << (tpcpoint, tpcbox),
  OPERATOR  1    << (tpcpoint, tpcpoint),
  -- overlaps or left
  OPERATOR  2    &< (tpcpoint, tpcbox),
  OPERATOR  2    &< (tpcpoint, tpcpoint),
  -- overlaps
  OPERATOR  3    && (tpcpoint, tstzspan),
  OPERATOR  3    && (tpcpoint, tpcbox),
  OPERATOR  3    && (tpcpoint, tpcpoint),
  -- overlaps or right
  OPERATOR  4    &> (tpcpoint, tpcbox),
  OPERATOR  4    &> (tpcpoint, tpcpoint),
  -- strictly right
  OPERATOR  5    >> (tpcpoint, tpcbox),
  OPERATOR  5    >> (tpcpoint, tpcpoint),
  -- same
  OPERATOR  6    ~= (tpcpoint, tstzspan),
  OPERATOR  6    ~= (tpcpoint, tpcbox),
  OPERATOR  6    ~= (tpcpoint, tpcpoint),
  -- contains
  OPERATOR  7    @> (tpcpoint, tstzspan),
  OPERATOR  7    @> (tpcpoint, tpcbox),
  OPERATOR  7    @> (tpcpoint, tpcpoint),
  -- contained by
  OPERATOR  8    <@ (tpcpoint, tstzspan),
  OPERATOR  8    <@ (tpcpoint, tpcbox),
  OPERATOR  8    <@ (tpcpoint, tpcpoint),
  -- overlaps or below
  OPERATOR  9    &<| (tpcpoint, tpcbox),
  OPERATOR  9    &<| (tpcpoint, tpcpoint),
  -- strictly below
  OPERATOR  10   <<| (tpcpoint, tpcbox),
  OPERATOR  10   <<| (tpcpoint, tpcpoint),
  -- strictly above
  OPERATOR  11   |>> (tpcpoint, tpcbox),
  OPERATOR  11   |>> (tpcpoint, tpcpoint),
  -- overlaps or above
  OPERATOR  12   |&> (tpcpoint, tpcbox),
  OPERATOR  12   |&> (tpcpoint, tpcpoint),
  -- adjacent
  OPERATOR  17   -|- (tpcpoint, tstzspan),
  OPERATOR  17   -|- (tpcpoint, tpcbox),
  OPERATOR  17   -|- (tpcpoint, tpcpoint),
  -- overlaps or before
  OPERATOR  28   &<# (tpcpoint, tstzspan),
  OPERATOR  28   &<# (tpcpoint, tpcbox),
  OPERATOR  28   &<# (tpcpoint, tpcpoint),
  -- strictly before
  OPERATOR  29   <<# (tpcpoint, tstzspan),
  OPERATOR  29   <<# (tpcpoint, tpcbox),
  OPERATOR  29   <<# (tpcpoint, tpcpoint),
  -- strictly after
  OPERATOR  30   #>> (tpcpoint, tstzspan),
  OPERATOR  30   #>> (tpcpoint, tpcbox),
  OPERATOR  30   #>> (tpcpoint, tpcpoint),
  -- overlaps or after
  OPERATOR  31   #&> (tpcpoint, tstzspan),
  OPERATOR  31   #&> (tpcpoint, tpcbox),
  OPERATOR  31   #&> (tpcpoint, tpcpoint),
  -- overlaps or front
  OPERATOR  32   &</ (tpcpoint, tpcbox),
  OPERATOR  32   &</ (tpcpoint, tpcpoint),
  -- strictly front
  OPERATOR  33   <</ (tpcpoint, tpcbox),
  OPERATOR  33   <</ (tpcpoint, tpcpoint),
  -- strictly back
  OPERATOR  34   />> (tpcpoint, tpcbox),
  OPERATOR  34   />> (tpcpoint, tpcpoint),
  -- overlaps or back
  OPERATOR  35   /&> (tpcpoint, tpcbox),
  OPERATOR  35   /&> (tpcpoint, tpcpoint),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcpoint, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25   |=| (tpcpoint, tpcpoint) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1    tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal),
  FUNCTION  2    tpcbox_gist_union(internal, internal),
  FUNCTION  3    tpc_gist_compress(internal),
  FUNCTION  5    tpcbox_gist_penalty(internal, internal, internal),
  FUNCTION  6    tpcbox_gist_picksplit(internal, internal),
  FUNCTION  7    tpcbox_gist_same(tpcbox, tpcbox, internal),
  FUNCTION  8 (tpcpoint, tpcbox)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION  8 (tpcpoint, tpcpoint)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION 11 tpcbox_gist_sortsupport(internal);

/*****************************************************************************
 * tpcpatch
 *****************************************************************************/

CREATE OPERATOR CLASS tpcpatch_rtree_ops
  DEFAULT FOR TYPE tpcpatch USING gist AS
  STORAGE tpcbox,
  -- strictly left
  OPERATOR  1    << (tpcpatch, tpcbox),
  OPERATOR  1    << (tpcpatch, tpcpatch),
  -- overlaps or left
  OPERATOR  2    &< (tpcpatch, tpcbox),
  OPERATOR  2    &< (tpcpatch, tpcpatch),
  -- overlaps
  OPERATOR  3    && (tpcpatch, tstzspan),
  OPERATOR  3    && (tpcpatch, tpcbox),
  OPERATOR  3    && (tpcpatch, tpcpatch),
  -- overlaps or right
  OPERATOR  4    &> (tpcpatch, tpcbox),
  OPERATOR  4    &> (tpcpatch, tpcpatch),
  -- strictly right
  OPERATOR  5    >> (tpcpatch, tpcbox),
  OPERATOR  5    >> (tpcpatch, tpcpatch),
  -- same
  OPERATOR  6    ~= (tpcpatch, tstzspan),
  OPERATOR  6    ~= (tpcpatch, tpcbox),
  OPERATOR  6    ~= (tpcpatch, tpcpatch),
  -- contains
  OPERATOR  7    @> (tpcpatch, tstzspan),
  OPERATOR  7    @> (tpcpatch, tpcbox),
  OPERATOR  7    @> (tpcpatch, tpcpatch),
  -- contained by
  OPERATOR  8    <@ (tpcpatch, tstzspan),
  OPERATOR  8    <@ (tpcpatch, tpcbox),
  OPERATOR  8    <@ (tpcpatch, tpcpatch),
  -- overlaps or below
  OPERATOR  9    &<| (tpcpatch, tpcbox),
  OPERATOR  9    &<| (tpcpatch, tpcpatch),
  -- strictly below
  OPERATOR  10   <<| (tpcpatch, tpcbox),
  OPERATOR  10   <<| (tpcpatch, tpcpatch),
  -- strictly above
  OPERATOR  11   |>> (tpcpatch, tpcbox),
  OPERATOR  11   |>> (tpcpatch, tpcpatch),
  -- overlaps or above
  OPERATOR  12   |&> (tpcpatch, tpcbox),
  OPERATOR  12   |&> (tpcpatch, tpcpatch),
  -- adjacent
  OPERATOR  17   -|- (tpcpatch, tstzspan),
  OPERATOR  17   -|- (tpcpatch, tpcbox),
  OPERATOR  17   -|- (tpcpatch, tpcpatch),
  -- overlaps or before
  OPERATOR  28   &<# (tpcpatch, tstzspan),
  OPERATOR  28   &<# (tpcpatch, tpcbox),
  OPERATOR  28   &<# (tpcpatch, tpcpatch),
  -- strictly before
  OPERATOR  29   <<# (tpcpatch, tstzspan),
  OPERATOR  29   <<# (tpcpatch, tpcbox),
  OPERATOR  29   <<# (tpcpatch, tpcpatch),
  -- strictly after
  OPERATOR  30   #>> (tpcpatch, tstzspan),
  OPERATOR  30   #>> (tpcpatch, tpcbox),
  OPERATOR  30   #>> (tpcpatch, tpcpatch),
  -- overlaps or after
  OPERATOR  31   #&> (tpcpatch, tstzspan),
  OPERATOR  31   #&> (tpcpatch, tpcbox),
  OPERATOR  31   #&> (tpcpatch, tpcpatch),
  -- overlaps or front
  OPERATOR  32   &</ (tpcpatch, tpcbox),
  OPERATOR  32   &</ (tpcpatch, tpcpatch),
  -- strictly front
  OPERATOR  33   <</ (tpcpatch, tpcbox),
  OPERATOR  33   <</ (tpcpatch, tpcpatch),
  -- strictly back
  OPERATOR  34   />> (tpcpatch, tpcbox),
  OPERATOR  34   />> (tpcpatch, tpcpatch),
  -- overlaps or back
  OPERATOR  35   /&> (tpcpatch, tpcbox),
  OPERATOR  35   /&> (tpcpatch, tpcpatch),
  -- nearest approach distance
  OPERATOR  25   |=| (tpcpatch, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25   |=| (tpcpatch, tpcpatch) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1    tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal),
  FUNCTION  2    tpcbox_gist_union(internal, internal),
  FUNCTION  3    tpc_gist_compress(internal),
  FUNCTION  5    tpcbox_gist_penalty(internal, internal, internal),
  FUNCTION  6    tpcbox_gist_picksplit(internal, internal),
  FUNCTION  7    tpcbox_gist_same(tpcbox, tpcbox, internal),
  FUNCTION  8 (tpcpatch, tpcbox)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION  8 (tpcpatch, tpcpatch)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION 11 tpcbox_gist_sortsupport(internal);

/*****************************************************************************/
