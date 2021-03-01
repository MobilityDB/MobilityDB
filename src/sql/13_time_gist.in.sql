/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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
 * time_gist.sql
 * R-tree GiST index for time types
 */

CREATE FUNCTION gist_timestampset_consistent(internal, timestampset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'period_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gist_union(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'timestampset_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#if MOBDB_PGSQL_VERSION < 110000
CREATE FUNCTION period_gist_decompress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif
CREATE FUNCTION period_gist_penalty(internal, internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gist_picksplit(internal, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gist_same(period, period, internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gist_fetch(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_timestampset_ops
  DEFAULT FOR TYPE timestampset USING gist AS
  STORAGE period,
  -- overlaps
  OPERATOR  3    && (timestampset, timestampset),
  OPERATOR  3    && (timestampset, period),
  OPERATOR  3    && (timestampset, periodset),
  -- contains
  OPERATOR  7    @> (timestampset, timestamptz),
  OPERATOR  7    @> (timestampset, timestampset),
  -- contained by
  OPERATOR  8    <@ (timestampset, timestampset),
  OPERATOR  8    <@ (timestampset, period),
  OPERATOR  8    <@ (timestampset, periodset),
  -- adjacent
  OPERATOR  17    -|- (timestampset, period),
  OPERATOR  17    -|- (timestampset, periodset),
  -- equals
  OPERATOR  18    = (timestampset, timestampset),
  -- overlaps or before
  OPERATOR  28    &<# (timestampset, timestamptz),
  OPERATOR  28    &<# (timestampset, timestampset),
  OPERATOR  28    &<# (timestampset, period),
  OPERATOR  28    &<# (timestampset, periodset),
  -- strictly before
  OPERATOR  29    <<# (timestampset, timestamptz),
  OPERATOR  29    <<# (timestampset, timestampset),
  OPERATOR  29    <<# (timestampset, period),
  OPERATOR  29    <<# (timestampset, periodset),
  -- strictly after
  OPERATOR  30    #>> (timestampset, timestamptz),
  OPERATOR  30    #>> (timestampset, timestampset),
  OPERATOR  30    #>> (timestampset, period),
  OPERATOR  30    #>> (timestampset, periodset),
  -- overlaps or after
  OPERATOR  31    #&> (timestampset, timestamptz),
  OPERATOR  31    #&> (timestampset, timestampset),
  OPERATOR  31    #&> (timestampset, period),
  OPERATOR  31    #&> (timestampset, periodset),
  -- functions
  FUNCTION  1  gist_timestampset_consistent(internal, timestampset, smallint, oid, internal),
  FUNCTION  2  period_gist_union(internal, internal),
  FUNCTION  3  timestampset_gist_compress(internal),
#if MOBDB_PGSQL_VERSION < 110000
  FUNCTION  4  period_gist_decompress(internal),
#endif
  FUNCTION  5  period_gist_penalty(internal, internal, internal),
  FUNCTION  6  period_gist_picksplit(internal, internal),
  FUNCTION  7  period_gist_same(period, period, internal);

/******************************************************************************/

CREATE FUNCTION period_gist_consistent(internal, period, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'period_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'period_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS period_gist_ops
  DEFAULT FOR TYPE period USING gist AS
  STORAGE period,
  -- overlaps
  OPERATOR  3    && (period, timestampset),
  OPERATOR  3    && (period, period),
  OPERATOR  3    && (period, periodset),
  -- contains
  OPERATOR  7    @> (period, timestamptz),
  OPERATOR  7    @> (period, timestampset),
  OPERATOR  7    @> (period, period),
  OPERATOR  7    @> (period, periodset),
  -- contained by
  OPERATOR  8    <@ (period, period),
  OPERATOR  8    <@ (period, periodset),
  -- adjacent
  OPERATOR  17    -|- (period, period),
  OPERATOR  17    -|- (period, periodset),
  -- equals
  OPERATOR  18    = (period, period),
  -- overlaps or before
  OPERATOR  28    &<# (period, timestamptz),
  OPERATOR  28    &<# (period, timestampset),
  OPERATOR  28    &<# (period, period),
  OPERATOR  28    &<# (period, periodset),
  -- strictly before
  OPERATOR  29    <<# (period, timestamptz),
  OPERATOR  29    <<# (period, timestampset),
  OPERATOR  29    <<# (period, period),
  OPERATOR  29    <<# (period, periodset),
  -- strictly after
  OPERATOR  30    #>> (period, timestamptz),
  OPERATOR  30    #>> (period, timestampset),
  OPERATOR  30    #>> (period, period),
  OPERATOR  30    #>> (period, periodset),
  -- overlaps or after
  OPERATOR  31    #&> (period, timestamptz),
  OPERATOR  31    #&> (period, timestampset),
  OPERATOR  31    #&> (period, period),
  OPERATOR  31    #&> (period, periodset),
  -- functions
  FUNCTION  1  period_gist_consistent(internal, period, smallint, oid, internal),
  FUNCTION  2  period_gist_union(internal, internal),
  FUNCTION  3  period_gist_compress(internal),
#if MOBDB_PGSQL_VERSION < 110000
  FUNCTION  4  period_gist_decompress(internal),
#endif
  FUNCTION  5  period_gist_penalty(internal, internal, internal),
  FUNCTION  6  period_gist_picksplit(internal, internal),
  FUNCTION  7  period_gist_same(period, period, internal),
  FUNCTION  9  period_gist_fetch(internal);

/******************************************************************************/

CREATE FUNCTION gist_periodset_consistent(internal, periodset, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'period_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'periodset_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_periodset_ops
  DEFAULT FOR TYPE periodset USING gist AS
  STORAGE period,
  -- overlaps
  OPERATOR  3    && (periodset, timestampset),
  OPERATOR  3    && (periodset, period),
  OPERATOR  3    && (periodset, periodset),
  -- contains
  OPERATOR  7    @> (periodset, timestamptz),
  OPERATOR  7    @> (periodset, timestampset),
  OPERATOR  7    @> (periodset, period),
  OPERATOR  7    @> (periodset, periodset),
  -- contained by
  OPERATOR  8    <@ (periodset, period),
  OPERATOR  8    <@ (periodset, periodset),
  -- adjacent
  OPERATOR  17    -|- (periodset, period),
  OPERATOR  17    -|- (periodset, periodset),
  -- equals
  OPERATOR  18    = (periodset, periodset),
  -- overlaps or before
  OPERATOR  28    &<# (periodset, timestamptz),
  OPERATOR  28    &<# (periodset, timestampset),
  OPERATOR  28    &<# (periodset, period),
  OPERATOR  28    &<# (periodset, periodset),
  -- strictly before
  OPERATOR  29    <<# (periodset, timestamptz),
  OPERATOR  29    <<# (periodset, timestampset),
  OPERATOR  29    <<# (periodset, period),
  OPERATOR  29    <<# (periodset, periodset),
  -- strictly after
  OPERATOR  30    #>> (periodset, timestamptz),
  OPERATOR  30    #>> (periodset, timestampset),
  OPERATOR  30    #>> (periodset, period),
  OPERATOR  30    #>> (periodset, periodset),
  -- overlaps or after
  OPERATOR  31    #&> (periodset, timestamptz),
  OPERATOR  31    #&> (periodset, timestampset),
  OPERATOR  31    #&> (periodset, period),
  OPERATOR  31    #&> (periodset, periodset),
  -- functions
  FUNCTION  1  gist_periodset_consistent(internal, periodset, smallint, oid, internal),
  FUNCTION  2  period_gist_union(internal, internal),
  FUNCTION  3  periodset_gist_compress(internal),
#if MOBDB_PGSQL_VERSION < 110000
  FUNCTION  4  period_gist_decompress(internal),
#endif
  FUNCTION  5  period_gist_penalty(internal, internal, internal),
  FUNCTION  6  period_gist_picksplit(internal, internal),
  FUNCTION  7  period_gist_same(period, period, internal);

/******************************************************************************/
