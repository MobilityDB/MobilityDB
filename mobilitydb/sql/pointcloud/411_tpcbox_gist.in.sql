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
 *****************************************************************************/

/**
 * @file
 * @brief R-tree GiST operator class for the TPCBox bounding-box type.
 */

CREATE FUNCTION tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal)
  RETURNS bool
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
  -- functions
  FUNCTION  1  tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal),
  FUNCTION  2  tpcbox_gist_union(internal, internal),
  FUNCTION  5  tpcbox_gist_penalty(internal, internal, internal),
  FUNCTION  6  tpcbox_gist_picksplit(internal, internal),
  FUNCTION  7  tpcbox_gist_same(tpcbox, tpcbox, internal);

/*****************************************************************************/
