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
 * @brief Nearest-approach-distance operator (|=|) for tpcbox /
 *   tpcpoint / tpcpatch — supports KNN ordering with GiST.
 */

CREATE FUNCTION nearestApproachDistance(tpcbox, tpcbox) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpcbox_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpcbox, tpcpoint) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpcpoint, tpcbox) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpcpoint, tpcpoint) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpcbox, tpcpatch) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpcbox_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpcpatch, tpcbox) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpointcloud_tpcbox' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpcpatch, tpcpatch) RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpointcloud_tpointcloud' LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcbox, RIGHTARG = tpcbox, COMMUTATOR = '|=|');
CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcbox, RIGHTARG = tpcpoint, COMMUTATOR = '|=|');
CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcpoint, RIGHTARG = tpcbox, COMMUTATOR = '|=|');
CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcpoint, RIGHTARG = tpcpoint, COMMUTATOR = '|=|');
CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcbox, RIGHTARG = tpcpatch, COMMUTATOR = '|=|');
CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcpatch, RIGHTARG = tpcbox, COMMUTATOR = '|=|');
CREATE OPERATOR |=| (PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcpatch, RIGHTARG = tpcpatch, COMMUTATOR = '|=|');

/*****************************************************************************
 * GiST distance — KNN strategy on the existing GiST opclasses
 *
 * ALTER OPERATOR FAMILY adds the |=| operator at strategy 25 (KNN
 * ORDER BY) and registers the tpcbox_gist_distance support function.
 *****************************************************************************/

CREATE FUNCTION tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

ALTER OPERATOR FAMILY tpcbox_rtree_ops USING gist ADD
  OPERATOR  25  |=| (tpcbox, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  8 (tpcbox, tpcbox)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal);

ALTER OPERATOR FAMILY tpcpoint_rtree_ops USING gist ADD
  OPERATOR  25  |=| (tpcpoint, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25  |=| (tpcpoint, tpcpoint) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  8 (tpcpoint, tpcbox)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION  8 (tpcpoint, tpcpoint)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal);

ALTER OPERATOR FAMILY tpcpatch_rtree_ops USING gist ADD
  OPERATOR  25  |=| (tpcpatch, tpcbox) FOR ORDER BY pg_catalog.float_ops,
  OPERATOR  25  |=| (tpcpatch, tpcpatch) FOR ORDER BY pg_catalog.float_ops,
  FUNCTION  8 (tpcpatch, tpcbox)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal),
  FUNCTION  8 (tpcpatch, tpcpatch)
    tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal);

/*****************************************************************************/
