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
 * time_spgist.sql
 * Quad-tree SP-GiST index for time types
 */

CREATE FUNCTION period_spgist_config(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Period_spgist_config'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_quadtree_choose(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Period_quadtree_choose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_quadtree_picksplit(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Period_quadtree_picksplit'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_quadtree_inner_consistent(internal, internal)
  RETURNS void
  AS 'MODULE_PATHNAME', 'Period_quadtree_inner_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_spgist_leaf_consistent(internal, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Period_spgist_leaf_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Timestampset_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_spgist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Periodset_spgist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS timestampset_quadtree_ops
  DEFAULT FOR TYPE timestampset USING spgist AS
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
#if POSTGRESQL_VERSION_NUMBER >= 120000
  -- nearest approach distance
  OPERATOR  25    |=| (timestampset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (timestampset, timestampset) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (timestampset, period) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (timestampset, periodset) FOR ORDER BY pg_catalog.float_ops,
#endif //POSTGRESQL_VERSION_NUMBER >= 120000
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
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  period_quadtree_choose(internal, internal),
  FUNCTION  3  period_quadtree_picksplit(internal, internal),
  FUNCTION  4  period_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  period_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  timestampset_spgist_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS period_quadtree_ops
  DEFAULT FOR TYPE period USING spgist AS
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
#if POSTGRESQL_VERSION_NUMBER >= 120000
  -- nearest approach distance
  OPERATOR  25    |=| (period, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (period, timestampset) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (period, period) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (period, periodset) FOR ORDER BY pg_catalog.float_ops,
#endif //POSTGRESQL_VERSION_NUMBER >= 120000
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
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  period_quadtree_choose(internal, internal),
  FUNCTION  3  period_quadtree_picksplit(internal, internal),
  FUNCTION  4  period_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  period_spgist_leaf_consistent(internal, internal);

/******************************************************************************/

CREATE OPERATOR CLASS periodset_quadtree_ops
  DEFAULT FOR TYPE periodset USING spgist AS
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
#if POSTGRESQL_VERSION_NUMBER >= 120000
  -- nearest approach distance
  OPERATOR  25    |=| (periodset, timestamptz) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (periodset, timestampset) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (periodset, period) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25    |=| (periodset, periodset) FOR ORDER BY pg_catalog.float_ops,
#endif //POSTGRESQL_VERSION_NUMBER >= 120000
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
  FUNCTION  1  period_spgist_config(internal, internal),
  FUNCTION  2  period_quadtree_choose(internal, internal),
  FUNCTION  3  period_quadtree_picksplit(internal, internal),
  FUNCTION  4  period_quadtree_inner_consistent(internal, internal),
  FUNCTION  5  period_spgist_leaf_consistent(internal, internal),
  FUNCTION  6  periodset_spgist_compress(internal);

/******************************************************************************/
