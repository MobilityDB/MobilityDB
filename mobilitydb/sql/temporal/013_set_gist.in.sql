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
 * @brief R-tree GiST indexes for set types
 */

/******************************************************************************
 * R-tree GiST indexes
 ******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, intset, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, intset, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Set_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS intset_rtree_ops
  DEFAULT FOR TYPE intset USING gist AS
  STORAGE intspan,
  -- strictly left
  OPERATOR  1     << (intset, integer),
  OPERATOR  1     << (intset, intset),
  -- overlaps or left
  OPERATOR  2     &< (intset, integer),
  OPERATOR  2     &< (intset, intset),
  -- overlaps
  OPERATOR  3     && (intset, intset),
  -- overlaps or right
  OPERATOR  4     &> (intset, integer),
  OPERATOR  4     &> (intset, intset),
  -- strictly right
  OPERATOR  5     >> (intset, integer),
  OPERATOR  5     >> (intset, intset),
  -- contains
  OPERATOR  7     @> (intset, integer),
  OPERATOR  7     @> (intset, intset),
  -- contained by
  OPERATOR  8     <@ (intset, intset),
  -- equals
  OPERATOR  18    = (intset, intset),
  -- nearest approach distance
  OPERATOR  25    <-> (intset, integer) FOR ORDER BY pg_catalog.integer_ops,
  OPERATOR  25    <-> (intset, intset) FOR ORDER BY pg_catalog.integer_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, intset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(intspan, intspan, internal),
  FUNCTION  8  span_gist_distance(internal, intset, smallint, oid, internal),
  FUNCTION 11  span_gist_sortsupport(internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, bigintset, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, bigintset, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS bigintset_rtree_ops
  DEFAULT FOR TYPE bigintset USING gist AS
  STORAGE bigintspan,
  -- strictly left
  OPERATOR  1     << (bigintset, bigint),
  OPERATOR  1     << (bigintset, bigintset),
  -- overlaps or left
  OPERATOR  2     &< (bigintset, bigint),
  OPERATOR  2     &< (bigintset, bigintset),
  -- overlaps
  OPERATOR  3     && (bigintset, bigintset),
  -- overlaps or right
  OPERATOR  4     &> (bigintset, bigint),
  OPERATOR  4     &> (bigintset, bigintset),
  -- strictly right
  OPERATOR  5     >> (bigintset, bigint),
  OPERATOR  5     >> (bigintset, bigintset),
  -- contains
  OPERATOR  7     @> (bigintset, bigint),
  OPERATOR  7     @> (bigintset, bigintset),
  -- contained by
  OPERATOR  8     <@ (bigintset, bigintset),
  -- equals
  OPERATOR  18    = (bigintset, bigintset),
  -- nearest approach distance
  OPERATOR  25    <-> (bigintset, bigint) FOR ORDER BY pg_catalog.integer_ops,
  OPERATOR  25    <-> (bigintset, bigintset) FOR ORDER BY pg_catalog.integer_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, bigintset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(bigintspan, bigintspan, internal),
  FUNCTION  8  span_gist_distance(internal, bigintset, smallint, oid, internal),
  FUNCTION 11  span_gist_sortsupport(internal);

/******************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, floatset, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gist_distance(internal, floatset, smallint, oid, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS floatset_rtree_ops
  DEFAULT FOR TYPE floatset USING gist AS
  STORAGE floatspan,
  -- strictly left
  OPERATOR  1     << (floatset, float),
  OPERATOR  1     << (floatset, floatset),
  -- overlaps or left
  OPERATOR  2     &< (floatset, float),
  OPERATOR  2     &< (floatset, floatset),
  -- overlaps
  OPERATOR  3     && (floatset, floatset),
  -- overlaps or right
  OPERATOR  4     &> (floatset, float),
  OPERATOR  4     &> (floatset, floatset),
  -- strictly right
  OPERATOR  5     >> (floatset, float),
  OPERATOR  5     >> (floatset, floatset),
  -- contains
  OPERATOR  7     @> (floatset, float),
  OPERATOR  7     @> (floatset, floatset),
  -- contained by
  OPERATOR  8     <@ (floatset, floatset),
  -- equals
  OPERATOR  18    = (floatset, floatset),
  -- nearest approach distance
  OPERATOR  25    <-> (floatset, float) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    <-> (floatset, floatset) FOR ORDER BY pg_catalog.float_ops,
  -- functions
  FUNCTION  1  span_gist_consistent(internal, floatset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(floatspan, floatspan, internal),
  FUNCTION  8  span_gist_distance(internal, floatset, smallint, oid, internal),
  FUNCTION 11  span_gist_sortsupport(internal);

/*****************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, dateset, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE OPERATOR CLASS dateset_rtree_ops
  DEFAULT FOR TYPE dateset USING gist AS
  STORAGE datespan,
  -- overlaps
  OPERATOR  3    && (dateset, dateset),
  -- contains
  OPERATOR  7    @> (dateset, date),
  OPERATOR  7    @> (dateset, dateset),
  -- contained by
  OPERATOR  8    <@ (dateset, dateset),
  -- equals
  OPERATOR  18    = (dateset, dateset),
  -- nearest approach distance
  OPERATOR  25    <-> (dateset, date) FOR ORDER BY pg_catalog.integer_ops,
  OPERATOR  25    <-> (dateset, dateset) FOR ORDER BY pg_catalog.integer_ops,
  -- overlaps or before
  OPERATOR  28    &<# (dateset, date),
  OPERATOR  28    &<# (dateset, dateset),
  -- strictly before
  OPERATOR  29    <<# (dateset, date),
  OPERATOR  29    <<# (dateset, dateset),
  -- strictly after
  OPERATOR  30    #>> (dateset, date),
  OPERATOR  30    #>> (dateset, dateset),
  -- overlaps or after
  OPERATOR  31    #&> (dateset, date),
  OPERATOR  31    #&> (dateset, dateset),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, dateset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(datespan, datespan, internal),
  FUNCTION 11  span_gist_sortsupport(internal);

/*****************************************************************************/

CREATE FUNCTION span_gist_consistent(internal, tstzset, smallint, oid, internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE OPERATOR CLASS tstzset_rtree_ops
  DEFAULT FOR TYPE tstzset USING gist AS
  STORAGE tstzspan,
  -- overlaps
  OPERATOR  3    && (tstzset, tstzset),
  -- contains
  OPERATOR  7    @> (tstzset, timestamptz),
  OPERATOR  7    @> (tstzset, tstzset),
  -- contained by
  OPERATOR  8    <@ (tstzset, tstzset),
  -- equals
  OPERATOR  18    = (tstzset, tstzset),
  -- nearest approach distance
  OPERATOR  25    <-> (tstzset, timestamptz) FOR ORDER BY pg_catalog.interval_ops,
  OPERATOR  25    <-> (tstzset, tstzset) FOR ORDER BY pg_catalog.interval_ops,
  -- overlaps or before
  OPERATOR  28    &<# (tstzset, timestamptz),
  OPERATOR  28    &<# (tstzset, tstzset),
  -- strictly before
  OPERATOR  29    <<# (tstzset, timestamptz),
  OPERATOR  29    <<# (tstzset, tstzset),
  -- strictly after
  OPERATOR  30    #>> (tstzset, timestamptz),
  OPERATOR  30    #>> (tstzset, tstzset),
  -- overlaps or after
  OPERATOR  31    #&> (tstzset, timestamptz),
  OPERATOR  31    #&> (tstzset, tstzset),
  -- functions
  FUNCTION  1  span_gist_consistent(internal, tstzset, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  set_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(tstzspan, tstzspan, internal),
  FUNCTION 11  span_gist_sortsupport(internal);
