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
 * @brief Distance functions for temporal geometries/geographies
 */

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

CREATE FUNCTION tdistance(geometry, tgeometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdistance(tgeometry, geometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdistance(tgeometry, tgeometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = geometry, RIGHTARG = tgeometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = tgeometry, RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  COMMUTATOR = <->
);

/*****************************************************************************/

CREATE FUNCTION tdistance(geography, tgeography)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdistance(tgeography, geography)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdistance(tgeography, tgeography)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = geography, RIGHTARG = tgeography,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = tgeography, RIGHTARG = geography,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tdistance,
  LEFTARG = tgeography, RIGHTARG = tgeography,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach instant/distance and shortest line functions
 *****************************************************************************/

CREATE FUNCTION nearestApproachInstant(geometry, tgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'NAI_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeometry, geometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'NAI_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeometry, tgeometry)
  RETURNS tgeometry
  AS 'MODULE_PATHNAME', 'NAI_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachInstant(geography, tgeography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'NAI_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeography, geography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'NAI_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeography, tgeography)
  RETURNS tgeography
  AS 'MODULE_PATHNAME', 'NAI_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geometry, tgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeometry, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeometry, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeometry, tgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geography, tgeography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeography, geography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tgeography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeography, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeography, tgeography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = tgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeometry, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeometry, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeometry, RIGHTARG = tgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

CREATE OPERATOR |=| (
  LEFTARG = geography, RIGHTARG = tgeography,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeography, RIGHTARG = geography,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tgeography,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeography, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeography, RIGHTARG = tgeography,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

CREATE FUNCTION shortestLine(geometry, tgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeometry, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeometry, tgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shortestLine(geography, tgeography)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeography, geography)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeography, tgeography)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
