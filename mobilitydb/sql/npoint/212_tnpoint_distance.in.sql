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
 * @brief Distance functions for temporal network points
 */

/*****************************************************************************
 * Temporal distance
 *****************************************************************************/

CREATE FUNCTION tDistance(geometry(Point), tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_point_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(npoint, tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tnpoint, geometry(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tnpoint_point'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tnpoint, npoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tnpoint, tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = geometry,
  RIGHTARG = tnpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = npoint,
  RIGHTARG = tnpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tnpoint,
  RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tnpoint,
  RIGHTARG = npoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistance,
  LEFTARG = tnpoint,
  RIGHTARG = tnpoint,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

CREATE FUNCTION NearestApproachInstant(geometry, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'NAI_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tnpoint, geometry)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'NAI_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION NearestApproachInstant(npoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'NAI_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tnpoint, npoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'NAI_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION NearestApproachInstant(tnpoint, tnpoint)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'NAI_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

CREATE FUNCTION nearestApproachDistance(geometry, tnpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tnpoint, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tnpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tnpoint, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnpoint_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachDistance(npoint, tnpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachDistance(tnpoint, npoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tnpoint, tnpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = tnpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tnpoint, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tnpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tnpoint, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = npoint, RIGHTARG = tnpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tnpoint, RIGHTARG = npoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tnpoint, RIGHTARG = tnpoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

/*****************************************************************************/
