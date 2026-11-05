/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief R-tree GiST index for temporal points
 */

CREATE FUNCTION stbox_gist_consistent(internal, stbox, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-BOXOPCLASS-BEGIN stbox_rtree_ops — tools/codegen/inherited/generate.py from templates/boxopclass.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/boxtypes.yaml and re-run.
CREATE OPERATOR CLASS stbox_rtree_ops
  DEFAULT FOR TYPE stbox USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (stbox, stbox),
  OPERATOR  1    << (stbox, tgeompoint),
  OPERATOR  1    << (stbox, tgeometry),
  -- overlaps or left
  OPERATOR  2    &< (stbox, stbox),
  OPERATOR  2    &< (stbox, tgeompoint),
  OPERATOR  2    &< (stbox, tgeometry),
  -- overlaps
  OPERATOR  3    && (stbox, stbox),
  OPERATOR  3    && (stbox, tgeompoint),
  OPERATOR  3    && (stbox, tgeogpoint),
  OPERATOR  3    && (stbox, tgeometry),
  OPERATOR  3    && (stbox, tgeography),
  -- overlaps or right
  OPERATOR  4    &> (stbox, stbox),
  OPERATOR  4    &> (stbox, tgeompoint),
  OPERATOR  4    &> (stbox, tgeometry),
  -- strictly right
  OPERATOR  5    >> (stbox, stbox),
  OPERATOR  5    >> (stbox, tgeompoint),
  OPERATOR  5    >> (stbox, tgeometry),
  -- same
  OPERATOR  6    ~= (stbox, stbox),
  OPERATOR  6    ~= (stbox, tgeompoint),
  OPERATOR  6    ~= (stbox, tgeogpoint),
  OPERATOR  6    ~= (stbox, tgeometry),
  OPERATOR  6    ~= (stbox, tgeography),
  -- contains
  OPERATOR  7    @> (stbox, stbox),
  OPERATOR  7    @> (stbox, tgeompoint),
  OPERATOR  7    @> (stbox, tgeogpoint),
  OPERATOR  7    @> (stbox, tgeometry),
  OPERATOR  7    @> (stbox, tgeography),
  -- contained by
  OPERATOR  8    <@ (stbox, stbox),
  OPERATOR  8    <@ (stbox, tgeompoint),
  OPERATOR  8    <@ (stbox, tgeogpoint),
  OPERATOR  8    <@ (stbox, tgeometry),
  OPERATOR  8    <@ (stbox, tgeography),
  -- overlaps or below
  OPERATOR  9    &<| (stbox, stbox),
  OPERATOR  9    &<| (stbox, tgeompoint),
  OPERATOR  9    &<| (stbox, tgeometry),
  -- strictly below
  OPERATOR  10    <<| (stbox, stbox),
  OPERATOR  10    <<| (stbox, tgeompoint),
  OPERATOR  10    <<| (stbox, tgeometry),
  -- strictly above
  OPERATOR  11    |>> (stbox, stbox),
  OPERATOR  11    |>> (stbox, tgeompoint),
  OPERATOR  11    |>> (stbox, tgeometry),
  -- overlaps or above
  OPERATOR  12    |&> (stbox, stbox),
  OPERATOR  12    |&> (stbox, tgeompoint),
  OPERATOR  12    |&> (stbox, tgeometry),
  -- adjacent
  OPERATOR  17    -|- (stbox, stbox),
  OPERATOR  17    -|- (stbox, tgeompoint),
  OPERATOR  17    -|- (stbox, tgeogpoint),
  OPERATOR  17    -|- (stbox, tgeometry),
  OPERATOR  17    -|- (stbox, tgeography),
  -- nearest approach distance
  OPERATOR  25    |=| (stbox, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, tgeometry) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, tgeography) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, geometry) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (stbox, geography) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (stbox, stbox),
  OPERATOR  28    &<# (stbox, tgeompoint),
  OPERATOR  28    &<# (stbox, tgeogpoint),
  OPERATOR  28    &<# (stbox, tgeometry),
  OPERATOR  28    &<# (stbox, tgeography),
  -- strictly before
  OPERATOR  29    <<# (stbox, stbox),
  OPERATOR  29    <<# (stbox, tgeompoint),
  OPERATOR  29    <<# (stbox, tgeogpoint),
  OPERATOR  29    <<# (stbox, tgeometry),
  OPERATOR  29    <<# (stbox, tgeography),
  -- strictly after
  OPERATOR  30    #>> (stbox, stbox),
  OPERATOR  30    #>> (stbox, tgeompoint),
  OPERATOR  30    #>> (stbox, tgeogpoint),
  OPERATOR  30    #>> (stbox, tgeometry),
  OPERATOR  30    #>> (stbox, tgeography),
  -- overlaps or after
  OPERATOR  31    #&> (stbox, stbox),
  OPERATOR  31    #&> (stbox, tgeompoint),
  OPERATOR  31    #&> (stbox, tgeogpoint),
  OPERATOR  31    #&> (stbox, tgeometry),
  OPERATOR  31    #&> (stbox, tgeography),
  -- overlaps or front
  OPERATOR  32    &</ (stbox, stbox),
  OPERATOR  32    &</ (stbox, tgeompoint),
  OPERATOR  32    &</ (stbox, tgeometry),
  -- strictly front
  OPERATOR  33    <</ (stbox, stbox),
  OPERATOR  33    <</ (stbox, tgeompoint),
  OPERATOR  33    <</ (stbox, tgeometry),
  -- strictly back
  OPERATOR  34    />> (stbox, stbox),
  OPERATOR  34    />> (stbox, tgeompoint),
  OPERATOR  34    />> (stbox, tgeometry),
  -- overlaps or back
  OPERATOR  35    /&> (stbox, stbox),
  OPERATOR  35    /&> (stbox, tgeompoint),
  OPERATOR  35    /&> (stbox, tgeometry),
  -- functions
  FUNCTION  1  stbox_gist_consistent(internal, stbox, smallint, oid, internal),
  FUNCTION  2  stbox_gist_union(internal, internal),
  FUNCTION  5  stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  stbox_gist_picksplit(internal, internal),
  FUNCTION  7  stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8  stbox_gist_distance(internal, stbox, smallint, oid, internal),
  FUNCTION 11  stbox_gist_sortsupport(internal);
-- GENERATED-BOXOPCLASS-END stbox_rtree_ops

/******************************************************************************/

CREATE FUNCTION tgeompoint_gist_consistent(internal, tgeompoint, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_gist_consistent(internal, tgeogpoint, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompoint_gist_distance(internal, tgeompoint, smallint, oid, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tspatial_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-TEMPOPCLASS-BEGIN tgeompoint_rtree_ops — tools/codegen/inherited/generate.py;
-- DO NOT EDIT BY HAND; edit manifest.d/boxtypes.yaml or manifest.d/subtypes.yaml and re-run.
CREATE OPERATOR CLASS tgeompoint_rtree_ops
  DEFAULT FOR TYPE tgeompoint USING gist AS
  STORAGE stbox,
  -- strictly left
  OPERATOR  1    << (tgeompoint, stbox),
  OPERATOR  1    << (tgeompoint, tgeompoint),
  -- overlaps or left
  OPERATOR  2    &< (tgeompoint, stbox),
  OPERATOR  2    &< (tgeompoint, tgeompoint),
  -- overlaps
  OPERATOR  3    && (tgeompoint, tstzspan),
  OPERATOR  3    && (tgeompoint, stbox),
  OPERATOR  3    && (tgeompoint, tgeompoint),
  -- overlaps or right
  OPERATOR  4    &> (tgeompoint, stbox),
  OPERATOR  4    &> (tgeompoint, tgeompoint),
  -- strictly right
  OPERATOR  5    >> (tgeompoint, stbox),
  OPERATOR  5    >> (tgeompoint, tgeompoint),
  -- same
  OPERATOR  6    ~= (tgeompoint, tstzspan),
  OPERATOR  6    ~= (tgeompoint, stbox),
  OPERATOR  6    ~= (tgeompoint, tgeompoint),
  -- contains
  OPERATOR  7    @> (tgeompoint, tstzspan),
  OPERATOR  7    @> (tgeompoint, stbox),
  OPERATOR  7    @> (tgeompoint, tgeompoint),
  -- contained by
  OPERATOR  8    <@ (tgeompoint, tstzspan),
  OPERATOR  8    <@ (tgeompoint, stbox),
  OPERATOR  8    <@ (tgeompoint, tgeompoint),
  -- overlaps or below
  OPERATOR  9    &<| (tgeompoint, stbox),
  OPERATOR  9    &<| (tgeompoint, tgeompoint),
  -- strictly below
  OPERATOR  10    <<| (tgeompoint, stbox),
  OPERATOR  10    <<| (tgeompoint, tgeompoint),
  -- strictly above
  OPERATOR  11    |>> (tgeompoint, stbox),
  OPERATOR  11    |>> (tgeompoint, tgeompoint),
  -- overlaps or above
  OPERATOR  12    |&> (tgeompoint, stbox),
  OPERATOR  12    |&> (tgeompoint, tgeompoint),
  -- adjacent
  OPERATOR  17    -|- (tgeompoint, tstzspan),
  OPERATOR  17    -|- (tgeompoint, stbox),
  OPERATOR  17    -|- (tgeompoint, tgeompoint),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeompoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeompoint, tstzspan),
  OPERATOR  28    &<# (tgeompoint, stbox),
  OPERATOR  28    &<# (tgeompoint, tgeompoint),
  -- strictly before
  OPERATOR  29    <<# (tgeompoint, tstzspan),
  OPERATOR  29    <<# (tgeompoint, stbox),
  OPERATOR  29    <<# (tgeompoint, tgeompoint),
  -- strictly after
  OPERATOR  30    #>> (tgeompoint, tstzspan),
  OPERATOR  30    #>> (tgeompoint, stbox),
  OPERATOR  30    #>> (tgeompoint, tgeompoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeompoint, tstzspan),
  OPERATOR  31    #&> (tgeompoint, stbox),
  OPERATOR  31    #&> (tgeompoint, tgeompoint),
  -- overlaps or front
  OPERATOR  32    &</ (tgeompoint, stbox),
  OPERATOR  32    &</ (tgeompoint, tgeompoint),
  -- strictly front
  OPERATOR  33    <</ (tgeompoint, stbox),
  OPERATOR  33    <</ (tgeompoint, tgeompoint),
  -- strictly back
  OPERATOR  34    />> (tgeompoint, stbox),
  OPERATOR  34    />> (tgeompoint, tgeompoint),
  -- overlaps or back
  OPERATOR  35    /&> (tgeompoint, stbox),
  OPERATOR  35    /&> (tgeompoint, tgeompoint),
  -- functions
  FUNCTION  1 tgeompoint_gist_consistent(internal, tgeompoint, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tspatial_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8 tgeompoint_gist_distance(internal, tgeompoint, smallint, oid, internal),
  FUNCTION 11 stbox_gist_sortsupport(internal);
-- GENERATED-TEMPOPCLASS-END tgeompoint_rtree_ops

CREATE FUNCTION tgeogpoint_gist_distance(internal, tgeogpoint, smallint, oid, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tspatial_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-TEMPOPCLASS-BEGIN tgeogpoint_rtree_ops — tools/codegen/inherited/generate.py;
-- DO NOT EDIT BY HAND; edit manifest.d/boxtypes.yaml or manifest.d/subtypes.yaml and re-run.
CREATE OPERATOR CLASS tgeogpoint_rtree_ops
  DEFAULT FOR TYPE tgeogpoint USING gist AS
  STORAGE stbox,
  -- overlaps
  OPERATOR  3    && (tgeogpoint, tstzspan),
  OPERATOR  3    && (tgeogpoint, stbox),
  OPERATOR  3    && (tgeogpoint, tgeogpoint),
  -- same
  OPERATOR  6    ~= (tgeogpoint, tstzspan),
  OPERATOR  6    ~= (tgeogpoint, stbox),
  OPERATOR  6    ~= (tgeogpoint, tgeogpoint),
  -- contains
  OPERATOR  7    @> (tgeogpoint, tstzspan),
  OPERATOR  7    @> (tgeogpoint, stbox),
  OPERATOR  7    @> (tgeogpoint, tgeogpoint),
  -- contained by
  OPERATOR  8    <@ (tgeogpoint, tstzspan),
  OPERATOR  8    <@ (tgeogpoint, stbox),
  OPERATOR  8    <@ (tgeogpoint, tgeogpoint),
  -- adjacent
  OPERATOR  17    -|- (tgeogpoint, tstzspan),
  OPERATOR  17    -|- (tgeogpoint, stbox),
  OPERATOR  17    -|- (tgeogpoint, tgeogpoint),
  -- nearest approach distance
  OPERATOR  25    |=| (tgeogpoint, stbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tgeogpoint, tstzspan),
  OPERATOR  28    &<# (tgeogpoint, stbox),
  OPERATOR  28    &<# (tgeogpoint, tgeogpoint),
  -- strictly before
  OPERATOR  29    <<# (tgeogpoint, tstzspan),
  OPERATOR  29    <<# (tgeogpoint, stbox),
  OPERATOR  29    <<# (tgeogpoint, tgeogpoint),
  -- strictly after
  OPERATOR  30    #>> (tgeogpoint, tstzspan),
  OPERATOR  30    #>> (tgeogpoint, stbox),
  OPERATOR  30    #>> (tgeogpoint, tgeogpoint),
  -- overlaps or after
  OPERATOR  31    #&> (tgeogpoint, tstzspan),
  OPERATOR  31    #&> (tgeogpoint, stbox),
  OPERATOR  31    #&> (tgeogpoint, tgeogpoint),
  -- functions
  FUNCTION  1 tgeogpoint_gist_consistent(internal, tgeogpoint, smallint, oid, internal),
  FUNCTION  2 stbox_gist_union(internal, internal),
  FUNCTION  3 tspatial_gist_compress(internal),
  FUNCTION  5 stbox_gist_penalty(internal, internal, internal),
  FUNCTION  6 stbox_gist_picksplit(internal, internal),
  FUNCTION  7 stbox_gist_same(stbox, stbox, internal),
  FUNCTION  8 tgeogpoint_gist_distance(internal, tgeogpoint, smallint, oid, internal),
  FUNCTION 11 stbox_gist_sortsupport(internal);
-- GENERATED-TEMPOPCLASS-END tgeogpoint_rtree_ops

/******************************************************************************/
