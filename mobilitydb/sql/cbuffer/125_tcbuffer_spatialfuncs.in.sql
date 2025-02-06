/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2024, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2024, PostGIS contributors
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
 * tcbuffer_spatialfuncs.sql
 * Geometric functions for temporal circular buffers.
 */

/*****************************************************************************
 * SRID
 *****************************************************************************/

CREATE FUNCTION SRID(tcbuffer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tcbuffer_get_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tcbuffer, integer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(tcbuffer, integer)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(tcbuffer, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS tcbuffer
  AS 'MODULE_PATHNAME', 'Tcbuffer_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Trajectory
 *****************************************************************************/

-- CREATE FUNCTION trajectory(tcbuffer)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_trajectory'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * AtGeometry and MinusGeometry
 *****************************************************************************/

-- CREATE FUNCTION atGeometry(tcbuffer, geometry)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_at_geom'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusGeometry(tcbuffer, geometry)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_minus_geom'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION atStbox(tcbuffer, stbox, bool DEFAULT TRUE)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_at_stbox'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION minusStbox(tcbuffer, stbox, bool DEFAULT TRUE)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_minus_stbox'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Equals
 *****************************************************************************/

-- CREATE FUNCTION same(cbuffer, cbuffer)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Cbuffer_same'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Length
 *****************************************************************************/

-- CREATE FUNCTION length(tcbuffer)
  -- RETURNS double precision
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_length'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Cumulative length
 *****************************************************************************/

-- CREATE FUNCTION cumulativeLength(tcbuffer)
  -- RETURNS tfloat
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_cumulative_length'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Speed
 *****************************************************************************/

-- CREATE FUNCTION speed(tcbuffer)
  -- RETURNS tfloat
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_speed'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Time-weighted centroid
 *****************************************************************************/

-- CREATE FUNCTION twCentroid(tcbuffer)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_twcentroid'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/

-- CREATE FUNCTION azimuth(tcbuffer)
  -- RETURNS tfloat
  -- AS 'MODULE_PATHNAME', 'Tcbuffer_azimuth'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Nearest approach instant
 *****************************************************************************/

-- CREATE FUNCTION NearestApproachInstant(geometry, tcbuffer)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'NAI_geo_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION NearestApproachInstant(tcbuffer, geometry)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'NAI_tcbuffer_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION NearestApproachInstant(cbuffer, tcbuffer)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'NAI_cbuffer_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION NearestApproachInstant(tcbuffer, cbuffer)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'NAI_tcbuffer_cbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION NearestApproachInstant(tcbuffer, tcbuffer)
  -- RETURNS tcbuffer
  -- AS 'MODULE_PATHNAME', 'NAI_tcbuffer_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Nearest approach distance
 *****************************************************************************/

-- CREATE FUNCTION nearestApproachDistance(geometry, tcbuffer)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'NAD_geo_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION nearestApproachDistance(tcbuffer, geometry)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'NAD_tcbuffer_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION NearestApproachDistance(cbuffer, tcbuffer)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'NAD_cbuffer_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION NearestApproachDistance(tcbuffer, cbuffer)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'NAD_tcbuffer_cbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION nearestApproachDistance(tcbuffer, tcbuffer)
  -- RETURNS float
  -- AS 'MODULE_PATHNAME', 'NAD_tcbuffer_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE OPERATOR |=| (
  -- LEFTARG = geometry, RIGHTARG = tcbuffer,
  -- PROCEDURE = nearestApproachDistance,
  -- COMMUTATOR = '|=|'
-- );
-- CREATE OPERATOR |=| (
  -- LEFTARG = tcbuffer, RIGHTARG = geometry,
  -- PROCEDURE = nearestApproachDistance,
  -- COMMUTATOR = '|=|'
-- );
-- CREATE OPERATOR |=| (
  -- LEFTARG = cbuffer, RIGHTARG = tcbuffer,
  -- PROCEDURE = nearestApproachDistance,
  -- COMMUTATOR = '|=|'
-- );
-- CREATE OPERATOR |=| (
  -- LEFTARG = tcbuffer, RIGHTARG = cbuffer,
  -- PROCEDURE = nearestApproachDistance,
  -- COMMUTATOR = '|=|'
-- );
-- CREATE OPERATOR |=| (
  -- LEFTARG = tcbuffer, RIGHTARG = tcbuffer,
  -- PROCEDURE = nearestApproachDistance,
  -- COMMUTATOR = '|=|'
-- );

/*****************************************************************************
 * Shortest line
 *****************************************************************************/

-- CREATE FUNCTION shortestLine(geometry, tcbuffer)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Shortestline_geo_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION shortestLine(tcbuffer, geometry)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Shortestline_tcbuffer_geo'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION shortestLine(cbuffer, tcbuffer)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Shortestline_cbuffer_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION shortestLine(tcbuffer, cbuffer)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Shortestline_tcbuffer_cbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION shortestLine(tcbuffer, tcbuffer)
  -- RETURNS geometry
  -- AS 'MODULE_PATHNAME', 'Shortestline_tcbuffer_tcbuffer'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

