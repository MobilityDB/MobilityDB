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
 * temporal_gist.sql
 * R-tree GiST index for temporal types
 */

/******************************************************************************/

CREATE FUNCTION tbox_gist_consistent(internal, tbox, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Tnumber_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gist_union(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbox_gist_union'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gist_penalty(internal, internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbox_gist_penalty'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gist_picksplit(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbox_gist_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gist_same(tbox, tbox, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbox_gist_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gist_distance(internal, tbox, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tbox_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE FUNCTION tbool_gist_consistent(internal, tbool, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tint_gist_consistent(internal, tint, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Tnumber_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tfloat_gist_consistent(internal, tfloat, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Tnumber_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Tnumber_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttext_gist_consistent(internal, ttext, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS tbox_rtree_ops
  DEFAULT FOR TYPE tbox USING gist AS
  -- strictly left
  OPERATOR  1    << (tbox, tbox),
  OPERATOR  1    << (tbox, tint),
  OPERATOR  1    << (tbox, tfloat),
   -- overlaps or left
  OPERATOR  2    &< (tbox, tbox),
  OPERATOR  2    &< (tbox, tint),
  OPERATOR  2    &< (tbox, tfloat),
  -- overlaps
  OPERATOR  3    && (tbox, tbox),
  OPERATOR  3    && (tbox, tint),
  OPERATOR  3    && (tbox, tfloat),
  -- overlaps or right
  OPERATOR  4    &> (tbox, tbox),
  OPERATOR  4    &> (tbox, tint),
  OPERATOR  4    &> (tbox, tfloat),
  -- strictly right
  OPERATOR  5    >> (tbox, tbox),
  OPERATOR  5    >> (tbox, tint),
  OPERATOR  5    >> (tbox, tfloat),
    -- same
  OPERATOR  6    ~= (tbox, tbox),
  OPERATOR  6    ~= (tbox, tint),
  OPERATOR  6    ~= (tbox, tfloat),
  -- contains
  OPERATOR  7    @> (tbox, tbox),
  OPERATOR  7    @> (tbox, tint),
  OPERATOR  7    @> (tbox, tfloat),
  -- contained by
  OPERATOR  8    <@ (tbox, tbox),
  OPERATOR  8    <@ (tbox, tint),
  OPERATOR  8    <@ (tbox, tfloat),
  -- adjacent
  OPERATOR  17    -|- (tbox, tbox),
  OPERATOR  17    -|- (tbox, tint),
  OPERATOR  17    -|- (tbox, tfloat),
  -- overlaps or before
  OPERATOR  28    &<# (tbox, tbox),
  OPERATOR  28    &<# (tbox, tint),
  OPERATOR  28    &<# (tbox, tfloat),
  -- strictly before
  OPERATOR  29    <<# (tbox, tbox),
  OPERATOR  29    <<# (tbox, tint),
  OPERATOR  29    <<# (tbox, tfloat),
  -- strictly after
  OPERATOR  30    #>> (tbox, tbox),
  OPERATOR  30    #>> (tbox, tint),
  OPERATOR  30    #>> (tbox, tfloat),
  -- overlaps or after
  OPERATOR  31    #&> (tbox, tbox),
  OPERATOR  31    #&> (tbox, tint),
  OPERATOR  31    #&> (tbox, tfloat),
  -- functions
  FUNCTION  1  tbox_gist_consistent(internal, tbox, smallint, oid, internal),
  FUNCTION  2  tbox_gist_union(internal, internal),
  FUNCTION  5  tbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  tbox_gist_picksplit(internal, internal),
  FUNCTION  7  tbox_gist_same(tbox, tbox, internal),
  FUNCTION  8  tbox_gist_distance(internal, tbox, smallint, oid, internal);

/******************************************************************************/

CREATE OPERATOR CLASS tbool_rtree_ops
  DEFAULT FOR TYPE tbool USING gist AS
  STORAGE period,
  -- overlaps
  OPERATOR  3    && (tbool, timestamptz),
  OPERATOR  3    && (tbool, timestampset),
  OPERATOR  3    && (tbool, period),
  OPERATOR  3    && (tbool, periodset),
  OPERATOR  3    && (tbool, tbool),
    -- same
  OPERATOR  6    ~= (tbool, timestamptz),
  OPERATOR  6    ~= (tbool, timestampset),
  OPERATOR  6    ~= (tbool, period),
  OPERATOR  6    ~= (tbool, periodset),
  OPERATOR  6    ~= (tbool, tbool),
  -- contains
  OPERATOR  7    @> (tbool, timestamptz),
  OPERATOR  7    @> (tbool, timestampset),
  OPERATOR  7    @> (tbool, period),
  OPERATOR  7    @> (tbool, periodset),
  OPERATOR  7    @> (tbool, tbool),
  -- contained by
  OPERATOR  8    <@ (tbool, timestamptz),
  OPERATOR  8    <@ (tbool, timestampset),
  OPERATOR  8    <@ (tbool, period),
  OPERATOR  8    <@ (tbool, periodset),
  OPERATOR  8    <@ (tbool, tbool),
  -- adjacent
  OPERATOR  17    -|- (tbool, timestamptz),
  OPERATOR  17    -|- (tbool, timestampset),
  OPERATOR  17    -|- (tbool, period),
  OPERATOR  17    -|- (tbool, periodset),
  OPERATOR  17    -|- (tbool, tbool),
  -- overlaps or before
  OPERATOR  28    &<# (tbool, timestamptz),
  OPERATOR  28    &<# (tbool, timestampset),
  OPERATOR  28    &<# (tbool, period),
  OPERATOR  28    &<# (tbool, periodset),
  OPERATOR  28    &<# (tbool, tbool),
  -- strictly before
  OPERATOR  29    <<# (tbool, timestamptz),
  OPERATOR  29    <<# (tbool, timestampset),
  OPERATOR  29    <<# (tbool, period),
  OPERATOR  29    <<# (tbool, periodset),
  OPERATOR  29    <<# (tbool, tbool),
  -- strictly after
  OPERATOR  30    #>> (tbool, timestamptz),
  OPERATOR  30    #>> (tbool, timestampset),
  OPERATOR  30    #>> (tbool, period),
  OPERATOR  30    #>> (tbool, periodset),
  OPERATOR  30    #>> (tbool, tbool),
  -- overlaps or after
  OPERATOR  31    #&> (tbool, timestamptz),
  OPERATOR  31    #&> (tbool, timestampset),
  OPERATOR  31    #&> (tbool, period),
  OPERATOR  31    #&> (tbool, periodset),
  OPERATOR  31    #&> (tbool, tbool),
  -- functions
  FUNCTION  1  tbool_gist_consistent(internal, tbool, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  tbool_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(period, period, internal);

/******************************************************************************/

CREATE OPERATOR CLASS tint_rtree_ops
  DEFAULT FOR TYPE tint USING gist AS
  STORAGE tbox,
  -- strictly left
  OPERATOR  1    << (tint, int),
  OPERATOR  1    << (tint, float),
  OPERATOR  1    << (tint, intspan),
  OPERATOR  1    << (tint, tbox),
  OPERATOR  1    << (tint, tint),
  OPERATOR  1    << (tint, tfloat),
   -- overlaps or left
  OPERATOR  2    &< (tint, int),
  OPERATOR  2    &< (tint, float),
  OPERATOR  2    &< (tint, intspan),
  OPERATOR  2    &< (tint, tbox),
  OPERATOR  2    &< (tint, tint),
  OPERATOR  2    &< (tint, tfloat),
  -- overlaps
  OPERATOR  3    && (tint, int),
  OPERATOR  3    && (tint, float),
  OPERATOR  3    && (tint, intspan),
  OPERATOR  3    && (tint, timestamptz),
  OPERATOR  3    && (tint, timestampset),
  OPERATOR  3    && (tint, period),
  OPERATOR  3    && (tint, periodset),
  OPERATOR  3    && (tint, tbox),
  OPERATOR  3    && (tint, tint),
  OPERATOR  3    && (tint, tfloat),
  -- overlaps or right
  OPERATOR  4    &> (tint, int),
  OPERATOR  4    &> (tint, float),
  OPERATOR  4    &> (tint, intspan),
  OPERATOR  4    &> (tint, tbox),
  OPERATOR  4    &> (tint, tint),
  OPERATOR  4    &> (tint, tfloat),
  -- strictly right
  OPERATOR  5    >> (tint, int),
  OPERATOR  5    >> (tint, float),
  OPERATOR  5    >> (tint, intspan),
  OPERATOR  5    >> (tint, tbox),
  OPERATOR  5    >> (tint, tint),
  OPERATOR  5    >> (tint, tfloat),
    -- same
  OPERATOR  6    ~= (tint, int),
  OPERATOR  6    ~= (tint, float),
  OPERATOR  6    ~= (tint, intspan),
  OPERATOR  6    ~= (tint, timestamptz),
  OPERATOR  6    ~= (tint, timestampset),
  OPERATOR  6    ~= (tint, period),
  OPERATOR  6    ~= (tint, periodset),
  OPERATOR  6    ~= (tint, tbox),
  OPERATOR  6    ~= (tint, tint),
  OPERATOR  6    ~= (tint, tfloat),
  -- contains
  OPERATOR  7    @> (tint, int),
  OPERATOR  7    @> (tint, float),
  OPERATOR  7    @> (tint, intspan),
  OPERATOR  7    @> (tint, timestamptz),
  OPERATOR  7    @> (tint, timestampset),
  OPERATOR  7    @> (tint, period),
  OPERATOR  7    @> (tint, periodset),
  OPERATOR  7    @> (tint, tbox),
  OPERATOR  7    @> (tint, tint),
  OPERATOR  7    @> (tint, tfloat),
  -- contained by
  OPERATOR  8    <@ (tint, int),
  OPERATOR  8    <@ (tint, float),
  OPERATOR  8    <@ (tint, intspan),
  OPERATOR  8    <@ (tint, timestamptz),
  OPERATOR  8    <@ (tint, timestampset),
  OPERATOR  8    <@ (tint, period),
  OPERATOR  8    <@ (tint, periodset),
  OPERATOR  8    <@ (tint, tbox),
  OPERATOR  8    <@ (tint, tint),
  OPERATOR  8    <@ (tint, tfloat),
  -- adjacent
  OPERATOR  17    -|- (tint, int),
  OPERATOR  17    -|- (tint, float),
  OPERATOR  17    -|- (tint, intspan),
  OPERATOR  17    -|- (tint, timestamptz),
  OPERATOR  17    -|- (tint, timestampset),
  OPERATOR  17    -|- (tint, period),
  OPERATOR  17    -|- (tint, periodset),
  OPERATOR  17    -|- (tint, tbox),
  OPERATOR  17    -|- (tint, tint),
  OPERATOR  17    -|- (tint, tfloat),
  -- nearest approach distance
  OPERATOR  25    |=| (tint, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tint, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tint, tbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tint, tint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tint, tfloat) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tint, timestamptz),
  OPERATOR  28    &<# (tint, timestampset),
  OPERATOR  28    &<# (tint, period),
  OPERATOR  28    &<# (tint, periodset),
  OPERATOR  28    &<# (tint, tbox),
  OPERATOR  28    &<# (tint, tint),
  OPERATOR  28    &<# (tint, tfloat),
  -- strictly before
  OPERATOR  29    <<# (tint, timestamptz),
  OPERATOR  29    <<# (tint, timestampset),
  OPERATOR  29    <<# (tint, period),
  OPERATOR  29    <<# (tint, periodset),
  OPERATOR  29    <<# (tint, tbox),
  OPERATOR  29    <<# (tint, tint),
  OPERATOR  29    <<# (tint, tfloat),
  -- strictly after
  OPERATOR  30    #>> (tint, timestamptz),
  OPERATOR  30    #>> (tint, timestampset),
  OPERATOR  30    #>> (tint, period),
  OPERATOR  30    #>> (tint, periodset),
  OPERATOR  30    #>> (tint, tbox),
  OPERATOR  30    #>> (tint, tint),
  OPERATOR  30    #>> (tint, tfloat),
  -- overlaps or after
  OPERATOR  31    #&> (tint, timestamptz),
  OPERATOR  31    #&> (tint, timestampset),
  OPERATOR  31    #&> (tint, period),
  OPERATOR  31    #&> (tint, periodset),
  OPERATOR  31    #&> (tint, tbox),
  OPERATOR  31    #&> (tint, tint),
  OPERATOR  31    #&> (tint, tfloat),
  -- functions
  FUNCTION  1  tint_gist_consistent(internal, tint, smallint, oid, internal),
  FUNCTION  2  tbox_gist_union(internal, internal),
  FUNCTION  3  tint_gist_compress(internal),
  FUNCTION  5  tbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  tbox_gist_picksplit(internal, internal),
  FUNCTION  7  tbox_gist_same(tbox, tbox, internal),
  FUNCTION  8  tbox_gist_distance(internal, tbox, smallint, oid, internal);

/******************************************************************************/

CREATE OPERATOR CLASS tfloat_rtree_ops
  DEFAULT FOR TYPE tfloat USING gist AS
  STORAGE tbox,
  -- strictly left
  OPERATOR  1    << (tfloat, int),
  OPERATOR  1    << (tfloat, float),
  OPERATOR  1    << (tfloat, floatspan),
  OPERATOR  1    << (tfloat, tbox),
  OPERATOR  1    << (tfloat, tint),
  OPERATOR  1    << (tfloat, tfloat),
   -- overlaps or left
  OPERATOR  2    &< (tfloat, int),
  OPERATOR  2    &< (tfloat, float),
  OPERATOR  2    &< (tfloat, floatspan),
  OPERATOR  2    &< (tfloat, tbox),
  OPERATOR  2    &< (tfloat, tint),
  OPERATOR  2    &< (tfloat, tfloat),
  -- overlaps
  OPERATOR  3    && (tfloat, int),
  OPERATOR  3    && (tfloat, float),
  OPERATOR  3    && (tfloat, floatspan),
  OPERATOR  3    && (tfloat, timestamptz),
  OPERATOR  3    && (tfloat, timestampset),
  OPERATOR  3    && (tfloat, period),
  OPERATOR  3    && (tfloat, periodset),
  OPERATOR  3    && (tfloat, tbox),
  OPERATOR  3    && (tfloat, tint),
  OPERATOR  3    && (tfloat, tfloat),
  -- overlaps or right
  OPERATOR  4    &> (tfloat, int),
  OPERATOR  4    &> (tfloat, float),
  OPERATOR  4    &> (tfloat, floatspan),
  OPERATOR  4    &> (tfloat, tbox),
  OPERATOR  4    &> (tfloat, tint),
  OPERATOR  4    &> (tfloat, tfloat),
  -- strictly right
  OPERATOR  5    >> (tfloat, int),
  OPERATOR  5    >> (tfloat, float),
  OPERATOR  5    >> (tfloat, floatspan),
  OPERATOR  5    >> (tfloat, tbox),
  OPERATOR  5    >> (tfloat, tint),
  OPERATOR  5    >> (tfloat, tfloat),
    -- same
  OPERATOR  6    ~= (tfloat, int),
  OPERATOR  6    ~= (tfloat, float),
  OPERATOR  6    ~= (tfloat, floatspan),
  OPERATOR  6    ~= (tfloat, timestamptz),
  OPERATOR  6    ~= (tfloat, timestampset),
  OPERATOR  6    ~= (tfloat, period),
  OPERATOR  6    ~= (tfloat, periodset),
  OPERATOR  6    ~= (tfloat, tbox),
  OPERATOR  6    ~= (tfloat, tint),
  OPERATOR  6    ~= (tfloat, tfloat),
  -- contains
  OPERATOR  7    @> (tfloat, int),
  OPERATOR  7    @> (tfloat, float),
  OPERATOR  7    @> (tfloat, floatspan),
  OPERATOR  7    @> (tfloat, timestamptz),
  OPERATOR  7    @> (tfloat, timestampset),
  OPERATOR  7    @> (tfloat, period),
  OPERATOR  7    @> (tfloat, periodset),
  OPERATOR  7    @> (tfloat, tbox),
  OPERATOR  7    @> (tfloat, tint),
  OPERATOR  7    @> (tfloat, tfloat),
  -- contained by
  OPERATOR  8    <@ (tfloat, int),
  OPERATOR  8    <@ (tfloat, float),
  OPERATOR  8    <@ (tfloat, floatspan),
  OPERATOR  8    <@ (tfloat, timestamptz),
  OPERATOR  8    <@ (tfloat, timestampset),
  OPERATOR  8    <@ (tfloat, period),
  OPERATOR  8    <@ (tfloat, periodset),
  OPERATOR  8    <@ (tfloat, tbox),
  OPERATOR  8    <@ (tfloat, tint),
  OPERATOR  8    <@ (tfloat, tfloat),
  -- adjacent
  OPERATOR  17    -|- (tfloat, int),
  OPERATOR  17    -|- (tfloat, float),
  OPERATOR  17    -|- (tfloat, floatspan),
  OPERATOR  17    -|- (tfloat, timestamptz),
  OPERATOR  17    -|- (tfloat, timestampset),
  OPERATOR  17    -|- (tfloat, period),
  OPERATOR  17    -|- (tfloat, periodset),
  OPERATOR  17    -|- (tfloat, tbox),
  OPERATOR  17    -|- (tfloat, tint),
  OPERATOR  17    -|- (tfloat, tfloat),
  -- nearest approach distance
  OPERATOR  25    |=| (tfloat, int) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tfloat, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tfloat, tbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tfloat, tint) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (tfloat, tfloat) FOR ORDER BY pg_catalog.float_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tfloat, timestamptz),
  OPERATOR  28    &<# (tfloat, timestampset),
  OPERATOR  28    &<# (tfloat, period),
  OPERATOR  28    &<# (tfloat, periodset),
  OPERATOR  28    &<# (tfloat, tbox),
  OPERATOR  28    &<# (tfloat, tint),
  OPERATOR  28    &<# (tfloat, tfloat),
  -- strictly before
  OPERATOR  29    <<# (tfloat, timestamptz),
  OPERATOR  29    <<# (tfloat, timestampset),
  OPERATOR  29    <<# (tfloat, period),
  OPERATOR  29    <<# (tfloat, periodset),
  OPERATOR  29    <<# (tfloat, tbox),
  OPERATOR  29    <<# (tfloat, tint),
  OPERATOR  29    <<# (tfloat, tfloat),
  -- strictly after
  OPERATOR  30    #>> (tfloat, timestamptz),
  OPERATOR  30    #>> (tfloat, timestampset),
  OPERATOR  30    #>> (tfloat, period),
  OPERATOR  30    #>> (tfloat, periodset),
  OPERATOR  30    #>> (tfloat, tbox),
  OPERATOR  30    #>> (tfloat, tint),
  OPERATOR  30    #>> (tfloat, tfloat),
  -- overlaps or after
  OPERATOR  31    #&> (tfloat, timestamptz),
  OPERATOR  31    #&> (tfloat, timestampset),
  OPERATOR  31    #&> (tfloat, period),
  OPERATOR  31    #&> (tfloat, periodset),
  OPERATOR  31    #&> (tfloat, tbox),
  OPERATOR  31    #&> (tfloat, tint),
  OPERATOR  31    #&> (tfloat, tfloat),
  -- functions
  FUNCTION  1  tfloat_gist_consistent(internal, tfloat, smallint, oid, internal),
  FUNCTION  2  tbox_gist_union(internal, internal),
  FUNCTION  3  tfloat_gist_compress(internal),
  FUNCTION  5  tbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  tbox_gist_picksplit(internal, internal),
  FUNCTION  7  tbox_gist_same(tbox, tbox, internal),
  FUNCTION  8  tbox_gist_distance(internal, tbox, smallint, oid, internal);

/******************************************************************************/

CREATE OPERATOR CLASS ttext_rtree_ops
  DEFAULT FOR TYPE ttext USING gist AS
  STORAGE period,
  -- overlaps
  OPERATOR  3    && (ttext, timestamptz),
  OPERATOR  3    && (ttext, timestampset),
  OPERATOR  3    && (ttext, period),
  OPERATOR  3    && (ttext, periodset),
  OPERATOR  3    && (ttext, ttext),
    -- same
  OPERATOR  6    ~= (ttext, timestamptz),
  OPERATOR  6    ~= (ttext, timestampset),
  OPERATOR  6    ~= (ttext, period),
  OPERATOR  6    ~= (ttext, periodset),
  OPERATOR  6    ~= (ttext, ttext),
  -- contains
  OPERATOR  7    @> (ttext, timestamptz),
  OPERATOR  7    @> (ttext, timestampset),
  OPERATOR  7    @> (ttext, period),
  OPERATOR  7    @> (ttext, periodset),
  OPERATOR  7    @> (ttext, ttext),
  -- contained by
  OPERATOR  8    <@ (ttext, timestamptz),
  OPERATOR  8    <@ (ttext, timestampset),
  OPERATOR  8    <@ (ttext, period),
  OPERATOR  8    <@ (ttext, periodset),
  OPERATOR  8    <@ (ttext, ttext),
  -- adjacent
  OPERATOR  17    -|- (ttext, timestamptz),
  OPERATOR  17    -|- (ttext, timestampset),
  OPERATOR  17    -|- (ttext, period),
  OPERATOR  17    -|- (ttext, periodset),
  OPERATOR  17    -|- (ttext, ttext),
  -- overlaps or before
  OPERATOR  28    &<# (ttext, timestamptz),
  OPERATOR  28    &<# (ttext, timestampset),
  OPERATOR  28    &<# (ttext, period),
  OPERATOR  28    &<# (ttext, periodset),
  OPERATOR  28    &<# (ttext, ttext),
  -- strictly before
  OPERATOR  29    <<# (ttext, timestamptz),
  OPERATOR  29    <<# (ttext, timestampset),
  OPERATOR  29    <<# (ttext, period),
  OPERATOR  29    <<# (ttext, periodset),
  OPERATOR  29    <<# (ttext, ttext),
  -- strictly after
  OPERATOR  30    #>> (ttext, timestamptz),
  OPERATOR  30    #>> (ttext, timestampset),
  OPERATOR  30    #>> (ttext, period),
  OPERATOR  30    #>> (ttext, periodset),
  OPERATOR  30    #>> (ttext, ttext),
  -- overlaps or after
  OPERATOR  31    #&> (ttext, timestamptz),
  OPERATOR  31    #&> (ttext, timestampset),
  OPERATOR  31    #&> (ttext, period),
  OPERATOR  31    #&> (ttext, periodset),
  OPERATOR  31    #&> (ttext, ttext),
  -- functions
  FUNCTION  1  ttext_gist_consistent(internal, ttext, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  ttext_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(period, period, internal);

/******************************************************************************/
