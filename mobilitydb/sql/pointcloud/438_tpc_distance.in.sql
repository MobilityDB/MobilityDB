-------------------------------------------------------------------------------
--
-- This MobilityDB code is provided under The PostgreSQL License.
-- Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
-- contributors
--
-- MobilityDB includes portions of PostGIS version 3 source code released
-- under the GNU General Public License (GPLv2 or later).
-- Copyright (c) 2001-2025, PostGIS contributors
--
-- Permission to use, copy, modify, and distribute this software and its
-- documentation for any purpose, without fee, and without a written
-- agreement is hereby granted, provided that the above copyright notice and
-- this paragraph and the following two paragraphs appear in all copies.
--
-- IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
-- DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
-- LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
-- EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
-- OF SUCH DAMAGE.
--
-- UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
-- INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
-- AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
-- AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
-- PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
--
-------------------------------------------------------------------------------

/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
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
 * GiST distance — the support function the operator classes of the family
 * register at strategy 25, declared here so the classes that follow name it
 *****************************************************************************/

CREATE FUNCTION tpcbox_gist_distance(internal, tpcbox, smallint, oid, internal)
  RETURNS float8
  AS 'MODULE_PATHNAME', 'Tpcbox_gist_distance'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
