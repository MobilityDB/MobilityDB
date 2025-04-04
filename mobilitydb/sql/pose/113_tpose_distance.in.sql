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
 * @brief Temporal distance for temporal poses
 */

CREATE FUNCTION tDistance(geometry(Point), tpose)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_point_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(pose, tpose)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_pose_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tpose, geometry(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tpose_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tpose, pose)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tpose_pose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tpose, tpose)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tpose_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = geometry,
  RIGHTARG = tpose,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = pose,
  RIGHTARG = tpose,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tpose,
  RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tpose,
  RIGHTARG = pose,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tpose,
  RIGHTARG = tpose,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach instant/distance and shortest line functions
 *****************************************************************************/

CREATE FUNCTION nearestApproachInstant(geometry, tpose)
  RETURNS tpose
  AS 'MODULE_PATHNAME', 'NAI_geo_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(stbox, tpose)
  RETURNS tpose
  AS 'SELECT @extschema@.nearestApproachInstant(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(pose, tpose)
  RETURNS tpose
  AS 'SELECT @extschema@.nearestApproachInstant(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tpose, geometry)
  RETURNS tpose
  AS 'MODULE_PATHNAME', 'NAI_tpose_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tpose, stbox)
  RETURNS tpose
  AS 'SELECT @extschema@.nearestApproachInstant($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tpose, pose)
  RETURNS tpose
  AS 'SELECT @extschema@.nearestApproachInstant($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tpose, tpose)
  RETURNS tpose
  AS 'MODULE_PATHNAME', 'NAI_tpose_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION nearestApproachDistance(geometry, tpose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tpose)
  RETURNS float
  AS 'SELECT @extschema@.nearestApproachDistance(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(pose, tpose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_pose_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpose, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpose_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpose, stbox)
  RETURNS float
  AS 'SELECT @extschema@.nearestApproachDistance($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpose, pose)
  RETURNS float
  AS 'SELECT @extschema@.nearestApproachDistance($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tpose, tpose)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpose_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = tpose,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tpose, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tpose,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tpose, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = pose, RIGHTARG = tpose,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tpose, RIGHTARG = pose,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tpose, RIGHTARG = tpose,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

/*****************************************************************************/

CREATE FUNCTION shortestLine(geometry, tpose)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(stbox, tpose)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(pose, tpose)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine(geometry($1), $2)'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(tpose, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tpose_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tpose, stbox)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(tpose, pose)
  RETURNS geometry
  AS 'SELECT @extschema@.shortestLine($1, geometry($2))'
  LANGUAGE SQL IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION shortestLine(tpose, tpose)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tpose_tpose'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
