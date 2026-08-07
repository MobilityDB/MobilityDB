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
 *****************************************************************************/

/**
 * @file
 * @brief GiST opclasses on tpcpoint and tpcpatch using TPCBox as the
 *   index key. Mirrors the tgeompoint / tcbuffer rtree opclasses.
 *
 * The GiST support functions consistent / union / penalty / picksplit /
 * same already exist (registered in 411_tpcbox_gist.in.sql for the
 * box-keyed opclass); we only need a fresh compress method that
 * derives a TPCBox from each leaf tpcpoint / tpcpatch.
 */

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
  -- functions
  FUNCTION  1    tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal),
  FUNCTION  2    tpcbox_gist_union(internal, internal),
  FUNCTION  3    tpc_gist_compress(internal),
  FUNCTION  5    tpcbox_gist_penalty(internal, internal, internal),
  FUNCTION  6    tpcbox_gist_picksplit(internal, internal),
  FUNCTION  7    tpcbox_gist_same(tpcbox, tpcbox, internal);

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
  -- functions
  FUNCTION  1    tpcbox_gist_consistent(internal, tpcbox, smallint, oid, internal),
  FUNCTION  2    tpcbox_gist_union(internal, internal),
  FUNCTION  3    tpc_gist_compress(internal),
  FUNCTION  5    tpcbox_gist_penalty(internal, internal, internal),
  FUNCTION  6    tpcbox_gist_picksplit(internal, internal),
  FUNCTION  7    tpcbox_gist_same(tpcbox, tpcbox, internal);

/*****************************************************************************/
