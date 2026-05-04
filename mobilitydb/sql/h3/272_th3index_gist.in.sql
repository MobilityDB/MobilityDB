/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 *****************************************************************************/

/**
 * @file
 * @brief GiST operator class for `th3index`.
 *
 * Mirror of `tbool_rtree_ops` from
 * `mobilitydb/sql/temporal/043_temporal_gist.in.sql`. The index is
 * keyed on the time dimension only — the bounding box is a
 * `tstzspan`, not a `tbox`, because `h3index` has no total order
 * that would make a value-range meaningful (precedent:
 * `ttext_rtree_ops`, `tbool_rtree_ops`).
 *
 * The consistent / compress support functions are the generic
 * talpha C symbols `Span_gist_consistent` and
 * `Temporal_gist_compress`; all four tstzspan-level helpers
 * (`span_gist_union` / `penalty` / `picksplit` / `same`) are reused
 * directly. No new C symbols are introduced.
 */

/******************************************************************************
 * Support functions (th3index-typed wrappers)
 ******************************************************************************/

CREATE FUNCTION th3index_gist_consistent(internal, th3index, smallint, oid, internal)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Span_gist_consistent'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION th3index_gist_compress(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Temporal_gist_compress'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operator class
 ******************************************************************************/

CREATE OPERATOR CLASS th3index_rtree_ops
  DEFAULT FOR TYPE th3index USING gist AS
  STORAGE tstzspan,
  -- overlaps
  OPERATOR  3    && (th3index, tstzspan),
  OPERATOR  3    && (th3index, th3index),
  -- same
  OPERATOR  6    ~= (th3index, tstzspan),
  OPERATOR  6    ~= (th3index, th3index),
  -- contains
  OPERATOR  7    @> (th3index, tstzspan),
  OPERATOR  7    @> (th3index, th3index),
  -- contained by
  OPERATOR  8    <@ (th3index, tstzspan),
  OPERATOR  8    <@ (th3index, th3index),
  -- adjacent
  OPERATOR  17    -|- (th3index, tstzspan),
  OPERATOR  17    -|- (th3index, th3index),
  -- overlaps or before
  OPERATOR  28    &<# (th3index, tstzspan),
  OPERATOR  28    &<# (th3index, th3index),
  -- strictly before
  OPERATOR  29    <<# (th3index, tstzspan),
  OPERATOR  29    <<# (th3index, th3index),
  -- strictly after
  OPERATOR  30    #>> (th3index, tstzspan),
  OPERATOR  30    #>> (th3index, th3index),
  -- overlaps or after
  OPERATOR  31    #&> (th3index, tstzspan),
  OPERATOR  31    #&> (th3index, th3index),
  -- functions
  FUNCTION  1  th3index_gist_consistent(internal, th3index, smallint, oid, internal),
  FUNCTION  2  span_gist_union(internal, internal),
  FUNCTION  3  th3index_gist_compress(internal),
  FUNCTION  5  span_gist_penalty(internal, internal, internal),
  FUNCTION  6  span_gist_picksplit(internal, internal),
  FUNCTION  7  span_gist_same(tstzspan, tstzspan, internal);

/******************************************************************************/
