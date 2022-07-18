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

/**
 * tnpoint_spatialfuncs.sql
 * Geometric functions for temporal network points.
 */

/*****************************************************************************
 * SRID
 *****************************************************************************/

CREATE FUNCTION SRID(tnpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tnpoint_get_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Trajectory
 *****************************************************************************/

CREATE FUNCTION trajectory(tnpoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tnpoint_get_trajectory'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * AtGeometry
 *****************************************************************************/

CREATE FUNCTION atGeometry(tnpoint, geometry)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Tnpoint_at_geometry'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * MinusGeometry
 *****************************************************************************/

CREATE FUNCTION minusGeometry(tnpoint, geometry)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Tnpoint_minus_geometry'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/*****************************************************************************
 * Equals
 *****************************************************************************/

CREATE FUNCTION equals(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Length
 *****************************************************************************/

CREATE FUNCTION length(tnpoint)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'Tnpoint_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Cumulative length
 *****************************************************************************/

CREATE FUNCTION cumulativeLength(tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnpoint_cumulative_length'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Speed
 *****************************************************************************/

CREATE FUNCTION speed(tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnpoint_speed'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Time-weighted centroid
 *****************************************************************************/

CREATE FUNCTION twCentroid(tnpoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tnpoint_twcentroid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

CREATE FUNCTION azimuth(tnpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tnpoint_azimuth'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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

/*****************************************************************************
 * Shortest line
 *****************************************************************************/

CREATE FUNCTION shortestLine(geometry, tnpoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tnpoint, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tnpoint_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(npoint, tnpoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_npoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tnpoint, npoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tnpoint_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tnpoint, tnpoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tnpoint_tnpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

