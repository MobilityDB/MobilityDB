/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
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

/*
 * tpoint_gist.c
 * R-tree GiST index for temporal points.
 */

CREATE FUNCTION stbox_gist_consistent(internal, stbox, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_gist_union(internal, internal)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_gist_union'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_gist_penalty(internal, internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Stbox_gist_penalty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_gist_picksplit(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Stbox_gist_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_gist_same(stbox, stbox, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Stbox_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_gist_distance(internal, stbox, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Stbox_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS stbox_rtree_ops
  DEFAULT FOR TYPE stbox USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (stbox, stbox),
  OPERATOR  1    << (stbox, tgeompoint),
  -- overlaps or left
  OPERATOR  2    &< (stbox, stbox),
  OPERATOR  2    &< (stbox, tgeompoint),
  -- overlaps
  OPERATOR  3    && (stbox, stbox),
  OPERATOR  3    && (stbox, tgeompoint),
  OPERATOR  3    && (stbox, tgeogpoint),
  -- overlaps or right
  OPERATOR  4    &> (stbox, stbox),
  OPERATOR  4    &> (stbox, tgeompoint),
    -- strictly right
  OPERATOR  5    >> (stbox, stbox),
  OPERATOR  5    >> (stbox, tgeompoint),
    -- same
  OPERATOR  6    ~= (stbox, stbox),
  OPERATOR  6    ~= (stbox, tgeompoint),
  OPERATOR  6    ~= (stbox, tgeogpoint),
  -- contains
  OPERATOR  7    @> (stbox, stbox),
  OPERATOR  7    @> (stbox, tgeompoint),
  OPERATOR  7    @> (stbox, tgeogpoint),
  -- contained by
  OPERATOR  8    <@ (stbox, stbox),
  OPERATOR  8    <@ (stbox, tgeompoint),
  OPERATOR  8    <@ (stbox, tgeogpoint),
  -- overlaps or below
  OPERATOR  9    &<| (stbox, stbox),
  OPERATOR  9    &<| (stbox, tgeompoint),
  -- strictly below
  OPERATOR  10    <<| (stbox, stbox),
  OPERATOR  10    <<| (stbox, tgeompoint),
  -- strictly above
  OPERATOR  11    |>> (stbox, stbox),
  OPERATOR  11    |>> (stbox, tgeompoint),
  -- overlaps or above
  OPERATOR  12    |&> (stbox, stbox),
  OPERATOR  12    |&> (stbox, tgeompoint),
  -- adjacent
  OPERATOR  17    -|- (stbox, stbox),
  OPERATOR  17    -|- (stbox, tgeompoint),
  OPERATOR  17    -|- (stbox, tgeogpoint),
  -- nearest approach distance
  OPERATOR  25    |=| (stbox, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (stbox, stbox),
  OPERATOR  28    &<# (stbox, tgeompoint),
  OPERATOR  28    &<# (stbox, tgeogpoint),
  -- strictly before
  OPERATOR  29    <<# (stbox, stbox),
  OPERATOR  29    <<# (stbox, tgeompoint),
  OPERATOR  29    <<# (stbox, tgeogpoint),
  -- strictly after
  OPERATOR  30    #>> (stbox, stbox),
  OPERATOR  30    #>> (stbox, tgeompoint),
  OPERATOR  30    #>> (stbox, tgeogpoint),
  -- overlaps or after
  OPERATOR  31    #&> (stbox, stbox),
  OPERATOR  31    #&> (stbox, tgeompoint),
  OPERATOR  31    #&> (stbox, tgeogpoint),
  -- overlaps or front
  OPERATOR  32    &</ (stbox, stbox),
  OPERATOR  32    &</ (stbox, tgeompoint),
  -- strictly front
  OPERATOR  33    <</ (stbox, stbox),
  OPERATOR  33    <</ (stbox, tgeompoint),
  -- strictly back
  OPERATOR  34    />> (stbox, stbox),
  OPERATOR  34    />> (stbox, tgeompoint),
  -- overlaps or back
  OPERATOR  35    /&> (stbox, stbox),
  OPERATOR  35    /&> (stbox, tgeompoint),
  -- functions
  FUNCTION  1  stbox_gist_consistent(internal, stbox, smallint, oid, internal),
  FUNCTION  2  stbox_gist_union(internal, internal),
  FUNCTION  5  stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  stbox_gist_picksplit(internal, internal),
  FUNCTION  7  stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8  stbox_gist_distance(internal, stbox, smallint, oid, internal);

/******************************************************************************/

CREATE FUNCTION gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tpoint_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tgeompoint_rtree_ops
  DEFAULT FOR TYPE tgeompoint USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (tgeompoint, geometry),
  OPERATOR  1    << (tgeompoint, stbox),
  OPERATOR  1    << (tgeompoint, tgeompoint),
  -- overlaps or left
  OPERATOR  2    &< (tgeompoint, geometry),
  OPERATOR  2    &< (tgeompoint, stbox),
  OPERATOR  2    &< (tgeompoint, tgeompoint),
  -- overlaps
  OPERATOR  3    && (tgeompoint, timestamptz),
  OPERATOR  3    && (tgeompoint, timestampset),
  OPERATOR  3    && (tgeompoint, period),
  OPERATOR  3    && (tgeompoint, periodset),
  OPERATOR  3    && (tgeompoint, geometry),
  OPERATOR  3    && (tgeompoint, stbox),
  OPERATOR  3    && (tgeompoint, tgeompoint),
  -- overlaps or right
  OPERATOR  4    &> (tgeompoint, geometry),
  OPERATOR  4    &> (tgeompoint, stbox),
  OPERATOR  4    &> (tgeompoint, tgeompoint),
    -- strictly right
  OPERATOR  5    >> (tgeompoint, geometry),
  OPERATOR  5    >> (tgeompoint, stbox),
  OPERATOR  5    >> (tgeompoint, tgeompoint),
    -- same
  OPERATOR  6    ~= (tgeompoint, geometry),
  OPERATOR  6    ~= (tgeompoint, timestamptz),
  OPERATOR  6    ~= (tgeompoint, timestampset),
  OPERATOR  6    ~= (tgeompoint, period),
  OPERATOR  6    ~= (tgeompoint, periodset),
  OPERATOR  6    ~= (tgeompoint, stbox),
  OPERATOR  6    ~= (tgeompoint, tgeompoint),
  -- contains
  OPERATOR  7    @> (tgeompoint, geometry),
  OPERATOR  7    @> (tgeompoint, timestamptz),
  OPERATOR  7    @> (tgeompoint, timestampset),
  OPERATOR  7    @> (tgeompoint, period),
  OPERATOR  7    @> (tgeompoint, periodset),
  OPERATOR  7    @> (tgeompoint, stbox),
  OPERATOR  7    @> (tgeompoint, tgeompoint),
  -- contained by
  OPERATOR  8    <@ (tgeompoint, geometry),
  OPERATOR  8    <@ (tgeompoint, timestamptz),
  OPERATOR  8    <@ (tgeompoint, timestampset),
  OPERATOR  8    <@ (tgeompoint, period),
  OPERATOR  8    <@ (tgeompoint, periodset),
  OPERATOR  8    <@ (tgeompoint, stbox),
  OPERATOR  8    <@ (tgeompoint, tgeompoint),
  -- overlaps or below
  OPERATOR  9    &<| (tgeompoint, geometry),
  OPERATOR  9    &<| (tgeompoint, stbox),
  OPERATOR  9    &<| (tgeompoint, tgeompoint),
  -- strictly below
  OPERATOR  10    <<| (tgeompoint, geometry),
  OPERATOR  10    <<| (tgeompoint, stbox),
  OPERATOR  10    <<| (tgeompoint, tgeompoint),
  -- strictly above
  OPERATOR  11    |>> (tgeompoint, geometry),
  OPERATOR  11    |>> (tgeompoint, stbox),
  OPERATOR  11    |>> (tgeompoint, tgeompoint),
  -- overlaps or above
  OPERATOR  12    |&> (tgeompoint, geometry),
  OPERATOR  12    |&> (tgeompoint, stbox),
  OPERATOR  12    |&> (tgeompoint, tgeompoint),
  -- adjacent
  OPERATOR  17    -|- (tgeompoint, geometry),
  OPERATOR  17    -|- (tgeompoint, timestamptz),
  OPERATOR  17    -|- (tgeompoint, timestampset),
  OPERATOR  17    -|- (tgeompoint, period),
  OPERATOR  17    -|- (tgeompoint, periodset),
  OPERATOR  17    -|- (tgeompoint, stbox),
  OPERATOR  17    -|- (tgeompoint, tgeompoint),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeompoint, timestamptz),
  OPERATOR  28    &<# (tgeompoint, timestampset),
  OPERATOR  28    &<# (tgeompoint, period),
  OPERATOR  28    &<# (tgeompoint, periodset),
  OPERATOR  28    &<# (tgeompoint, stbox),
  OPERATOR  28    &<# (tgeompoint, tgeompoint),
  -- strictly before
  OPERATOR  29    <<# (tgeompoint, timestamptz),
  OPERATOR  29    <<# (tgeompoint, timestampset),
  OPERATOR  29    <<# (tgeompoint, period),
  OPERATOR  29    <<# (tgeompoint, periodset),
  OPERATOR  29    <<# (tgeompoint, stbox),
  OPERATOR  29    <<# (tgeompoint, tgeompoint),
  -- strictly after
  OPERATOR  30    #>> (tgeompoint, timestamptz),
  OPERATOR  30    #>> (tgeompoint, timestampset),
  OPERATOR  30    #>> (tgeompoint, period),
  OPERATOR  30    #>> (tgeompoint, periodset),
  OPERATOR  30    #>> (tgeompoint, stbox),
  OPERATOR  30    #>> (tgeompoint, tgeompoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeompoint, timestamptz),
  OPERATOR  31    #&> (tgeompoint, timestampset),
  OPERATOR  31    #&> (tgeompoint, period),
  OPERATOR  31    #&> (tgeompoint, periodset),
  OPERATOR  31    #&> (tgeompoint, stbox),
  OPERATOR  31    #&> (tgeompoint, tgeompoint),
  -- overlaps or front
  OPERATOR  32    &</ (tgeompoint, geometry),
  OPERATOR  32    &</ (tgeompoint, stbox),
  OPERATOR  32    &</ (tgeompoint, tgeompoint),
  -- strictly front
  OPERATOR  33    <</ (tgeompoint, geometry),
  OPERATOR  33    <</ (tgeompoint, stbox),
  OPERATOR  33    <</ (tgeompoint, tgeompoint),
  -- strictly back
  OPERATOR  34    />> (tgeompoint, geometry),
  OPERATOR  34    />> (tgeompoint, stbox),
  OPERATOR  34    />> (tgeompoint, tgeompoint),
  -- overlaps or back
  OPERATOR  35    /&> (tgeompoint, geometry),
  OPERATOR  35    /&> (tgeompoint, stbox),
  OPERATOR  35    /&> (tgeompoint, tgeompoint),
  -- functions
  FUNCTION  1  gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal),
  FUNCTION  2  stbox_gist_union(internal, internal),
  FUNCTION  3  tpoint_gist_compress(internal),
  FUNCTION  5  stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  stbox_gist_picksplit(internal, internal),
  FUNCTION  7  stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8  stbox_gist_distance(internal, stbox, smallint, oid, internal);

CREATE OPERATOR CLASS tgeogpoint_rtree_ops
  DEFAULT FOR TYPE tgeogpoint USING gist AS
  STORAGE stbox,
  -- overlaps
  OPERATOR  3    && (tgeogpoint, geography),
  OPERATOR  3    && (tgeogpoint, timestamptz),
  OPERATOR  3    && (tgeogpoint, timestampset),
  OPERATOR  3    && (tgeogpoint, period),
  OPERATOR  3    && (tgeogpoint, periodset),
  OPERATOR  3    && (tgeogpoint, stbox),
  OPERATOR  3    && (tgeogpoint, tgeogpoint),
    -- same
  OPERATOR  6    ~= (tgeogpoint, geography),
  OPERATOR  6    ~= (tgeogpoint, timestamptz),
  OPERATOR  6    ~= (tgeogpoint, timestampset),
  OPERATOR  6    ~= (tgeogpoint, period),
  OPERATOR  6    ~= (tgeogpoint, periodset),
  OPERATOR  6    ~= (tgeogpoint, stbox),
  OPERATOR  6    ~= (tgeogpoint, tgeogpoint),
  -- contains
  OPERATOR  7    @> (tgeogpoint, geography),
  OPERATOR  7    @> (tgeogpoint, timestamptz),
  OPERATOR  7    @> (tgeogpoint, timestampset),
  OPERATOR  7    @> (tgeogpoint, period),
  OPERATOR  7    @> (tgeogpoint, periodset),
  OPERATOR  7    @> (tgeogpoint, stbox),
  OPERATOR  7    @> (tgeogpoint, tgeogpoint),
  -- contained by
  OPERATOR  8    <@ (tgeogpoint, geography),
  OPERATOR  8    <@ (tgeogpoint, timestamptz),
  OPERATOR  8    <@ (tgeogpoint, timestampset),
  OPERATOR  8    <@ (tgeogpoint, period),
  OPERATOR  8    <@ (tgeogpoint, periodset),
  OPERATOR  8    <@ (tgeogpoint, stbox),
  OPERATOR  8    <@ (tgeogpoint, tgeogpoint),
  -- adjacent
  OPERATOR  17    -|- (tgeogpoint, geography),
  OPERATOR  17    -|- (tgeogpoint, timestamptz),
  OPERATOR  17    -|- (tgeogpoint, timestampset),
  OPERATOR  17    -|- (tgeogpoint, period),
  OPERATOR  17    -|- (tgeogpoint, periodset),
  OPERATOR  17    -|- (tgeogpoint, stbox),
  OPERATOR  17    -|- (tgeogpoint, tgeogpoint),
  -- distance
  OPERATOR  25    |=| (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeogpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeogpoint, timestamptz),
  OPERATOR  28    &<# (tgeogpoint, timestampset),
  OPERATOR  28    &<# (tgeogpoint, period),
  OPERATOR  28    &<# (tgeogpoint, periodset),
  OPERATOR  28    &<# (tgeogpoint, stbox),
  OPERATOR  28    &<# (tgeogpoint, tgeogpoint),
  -- strictly before
  OPERATOR  29    <<# (tgeogpoint, timestamptz),
  OPERATOR  29    <<# (tgeogpoint, timestampset),
  OPERATOR  29    <<# (tgeogpoint, period),
  OPERATOR  29    <<# (tgeogpoint, periodset),
  OPERATOR  29    <<# (tgeogpoint, stbox),
  OPERATOR  29    <<# (tgeogpoint, tgeogpoint),
  -- strictly after
  OPERATOR  30    #>> (tgeogpoint, timestamptz),
  OPERATOR  30    #>> (tgeogpoint, timestampset),
  OPERATOR  30    #>> (tgeogpoint, period),
  OPERATOR  30    #>> (tgeogpoint, periodset),
  OPERATOR  30    #>> (tgeogpoint, stbox),
  OPERATOR  30    #>> (tgeogpoint, tgeogpoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeogpoint, timestamptz),
  OPERATOR  31    #&> (tgeogpoint, timestampset),
  OPERATOR  31    #&> (tgeogpoint, period),
  OPERATOR  31    #&> (tgeogpoint, periodset),
  OPERATOR  31    #&> (tgeogpoint, stbox),
  OPERATOR  31    #&> (tgeogpoint, tgeogpoint),
  -- functions
  FUNCTION  1  gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal),
  FUNCTION  2  stbox_gist_union(internal, internal),
  FUNCTION  3  tpoint_gist_compress(internal),
  FUNCTION  5  stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  stbox_gist_picksplit(internal, internal),
  FUNCTION  7  stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8  stbox_gist_distance(internal, stbox, smallint, oid, internal);

/******************************************************************************/
