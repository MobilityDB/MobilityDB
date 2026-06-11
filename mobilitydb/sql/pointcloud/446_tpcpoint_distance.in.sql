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
 * @brief Distance functions for temporal pcpoint — geometry/tpcpoint cross-type.
 *
 * Delegates to the corresponding tgeompoint overloads via the XY-projection
 * cast. The nearestApproachInstant overloads return `tpcpoint` by restricting
 * the original (with all sensor channels) to the NAI timestamp.
 *
 * The tpcpoint × tpcpoint nearestApproachDistance |=| operator is already
 * registered in 439_tpc_distance.in.sql. This file adds the geometry/tpcpoint
 * cross-type surface (Cat 6 parity gap).
 */

/******************************************************************************
 * tdistance — temporal distance
 ******************************************************************************/

CREATE FUNCTION tdistance(geometry, tpcpoint)
  RETURNS tfloat
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tdistance($1, $2::@extschema@.tgeompoint)
  $$;
CREATE FUNCTION tdistance(tpcpoint, geometry)
  RETURNS tfloat
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.tdistance($1::@extschema@.tgeompoint, $2)
  $$;

CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = geometry, RIGHTARG = tpcpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = tpcpoint, RIGHTARG = geometry,
  COMMUTATOR = <->
);

/******************************************************************************
 * nearestApproachDistance — geometry/tpcpoint cross-type
 *
 * The tpcpoint × tpcpoint overloads and |=| operators are already registered
 * in 439_tpc_distance.in.sql.
 ******************************************************************************/

CREATE FUNCTION nearestApproachDistance(geometry, tpcpoint)
  RETURNS float
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.nearestApproachDistance($1, $2::@extschema@.tgeompoint)
  $$;
CREATE FUNCTION nearestApproachDistance(tpcpoint, geometry)
  RETURNS float
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.nearestApproachDistance($1::@extschema@.tgeompoint, $2)
  $$;

CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = geometry, RIGHTARG = tpcpoint,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  PROCEDURE = nearestApproachDistance,
  LEFTARG = tpcpoint, RIGHTARG = geometry,
  COMMUTATOR = '|=|'
);

/******************************************************************************
 * nearestApproachInstant
 *
 * Returns the tpcpoint instant (with all sensor channels) at the timestamp
 * when the XY trajectory is closest to the argument geometry.
 ******************************************************************************/

CREATE FUNCTION nearestApproachInstant(geometry, tpcpoint)
  RETURNS tpcpoint
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.atTime($2,
      @extschema@.getTimestamp(
        @extschema@.nearestApproachInstant($1, $2::@extschema@.tgeompoint)))
  $$;
CREATE FUNCTION nearestApproachInstant(tpcpoint, geometry)
  RETURNS tpcpoint
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.atTime($1,
      @extschema@.getTimestamp(
        @extschema@.nearestApproachInstant($1::@extschema@.tgeompoint, $2)))
  $$;

/******************************************************************************
 * shortestLine
 ******************************************************************************/

CREATE FUNCTION shortestLine(geometry, tpcpoint)
  RETURNS geometry
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.shortestLine($1, $2::@extschema@.tgeompoint)
  $$;
CREATE FUNCTION shortestLine(tpcpoint, geometry)
  RETURNS geometry
  LANGUAGE SQL STABLE STRICT PARALLEL SAFE AS $$
    SELECT @extschema@.shortestLine($1::@extschema@.tgeompoint, $2)
  $$;

/*****************************************************************************/
