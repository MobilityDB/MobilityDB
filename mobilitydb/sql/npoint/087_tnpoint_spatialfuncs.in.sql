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
 * @brief Geometric functions for temporal network points
 */

/*****************************************************************************
 * SRID
 *****************************************************************************/

CREATE FUNCTION SRID(tnpoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tspatial_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Trajectory
 *****************************************************************************/

CREATE FUNCTION trajectory(tnpoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Tnpoint_trajectory'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * AtGeometry and MinusGeometry
 *****************************************************************************/

CREATE FUNCTION atGeometry(tnpoint, geometry)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Tnpoint_at_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusGeometry(tnpoint, geometry)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Tnpoint_minus_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atStbox(tnpoint, stbox, bool DEFAULT TRUE)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Tnpoint_at_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusStbox(tnpoint, stbox, bool DEFAULT TRUE)
  RETURNS tnpoint
  AS 'MODULE_PATHNAME', 'Tnpoint_minus_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Equals
 *****************************************************************************/

CREATE FUNCTION same(npoint, npoint)
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

