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
 * @brief Distance functions for temporal rigid geometries
 */

/*****************************************************************************
 * Distance functions
 *****************************************************************************/

CREATE FUNCTION distance(trgeometry, geometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(geometry, trgeometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(trgeometry, tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_trgeo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, trgeometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_tpoint_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(trgeometry, trgeometry)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Distance_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = trgeometry, RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = geometry, RIGHTARG = trgeometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = trgeometry, RIGHTARG = tgeompoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = tgeompoint, RIGHTARG = trgeometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = distance,
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach instant/distance and shortest line functions
 *****************************************************************************/

CREATE FUNCTION nearestApproachInstant(trgeometry, geometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'NAI_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(geometry, trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'NAI_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(trgeometry, tgeompoint)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'NAI_trgeo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/* TODO: Maybe change output type here.
 * Currently we return a trgeometry instant
 * even if the left argument is a tgeompoint */
CREATE FUNCTION nearestApproachInstant(tgeompoint, trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'NAI_tpoint_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(trgeometry, trgeometry)
  RETURNS trgeometry
  AS 'MODULE_PATHNAME', 'NAI_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(trgeometry, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(geometry, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(trgeometry, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_trgeo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(trgeometry, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_trgeo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tpoint_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(trgeometry, trgeometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = trgeometry, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = trgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = trgeometry, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = trgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = trgeometry, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = trgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = trgeometry, RIGHTARG = trgeometry,
  PROCEDURE = nearestApproachDistance,
  COMMUTATOR = '|=|'
);

CREATE FUNCTION shortestLine(trgeometry, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_trgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(geometry, trgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_geo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(trgeometry, tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_trgeo_tpoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, trgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tpoint_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(trgeometry, trgeometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_trgeo_trgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
