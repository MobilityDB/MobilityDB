/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Distance functions for temporal points
 */

-- GENERATED-DISTANCE-BEGIN tpoint — tools/codegen/inherited/generate.py from templates/comparisons.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/distance_families.yaml and re-run.
/*****************************************************************************
 * Distance functions
 *****************************************************************************/

CREATE FUNCTION tDistance(geometry(Point), tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistanceOpr(geometry(Point), tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_geo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tgeompoint, geometry(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistanceOpr(tgeompoint, geometry(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_geo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tgeompoint, tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistanceOpr(tgeompoint, tgeompoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tDistanceOpr,
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistanceOpr,
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistanceOpr,
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  COMMUTATOR = <->
);

/*****************************************************************************/

CREATE FUNCTION tDistance(geography(Point), tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistanceOpr(geography(Point), tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_geo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tgeogpoint, geography(Point), spheroid boolean DEFAULT true)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistanceOpr(tgeogpoint, geography(Point))
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_geo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistance(tgeogpoint, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tDistanceOpr(tgeogpoint, tgeogpoint)
  RETURNS tfloat
  AS 'MODULE_PATHNAME', 'Tdistance_tgeo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = tDistanceOpr,
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistanceOpr,
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = tDistanceOpr,
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  COMMUTATOR = <->
);

/*****************************************************************************
 * Nearest approach instant/distance and shortest line functions
 *****************************************************************************/

CREATE FUNCTION nearestApproachInstant(geometry, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'NAI_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeompoint, geometry)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'NAI_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeompoint, tgeompoint)
  RETURNS tgeompoint
  AS 'MODULE_PATHNAME', 'NAI_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachInstant(geography, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'NAI_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeogpoint, geography, spheroid boolean DEFAULT true)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'NAI_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachInstant(tgeogpoint, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS tgeogpoint
  AS 'MODULE_PATHNAME', 'NAI_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geometry, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(geometry, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(tgeompoint, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_geo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(stbox, geometry)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_geo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(geometry, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(geometry, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_stbox_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(stbox, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(tgeompoint, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_stbox_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, stbox, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(stbox, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_stbox_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(tgeompoint, tgeompoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geography, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(geography, tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, geography, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(tgeogpoint, geography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_geo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, geography, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(stbox, geography)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_geo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(geography, stbox, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(geography, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_geo_stbox_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(stbox, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(stbox, tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_stbox_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, stbox, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(tgeogpoint, stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_stbox_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistanceOpr(tgeogpoint, tgeogpoint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'NAD_tgeo_tgeo_opr'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = geometry,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = geometry, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);

CREATE OPERATOR |=| (
  LEFTARG = geography, RIGHTARG = tgeogpoint,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeogpoint, RIGHTARG = geography,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = geography,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = geography, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = stbox, RIGHTARG = tgeogpoint,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeogpoint, RIGHTARG = stbox,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
  LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
  PROCEDURE = nearestApproachDistanceOpr,
  COMMUTATOR = '|=|'
);

CREATE FUNCTION shortestLine(geometry, tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, geometry)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, tgeompoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shortestLine(geography, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Shortestline_geo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, geography, spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, tgeogpoint, spheroid boolean DEFAULT true)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Shortestline_tgeo_tgeo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
-- GENERATED-DISTANCE-END tpoint
