/*****************************************************************************
 *
 * tpoint_spatialfuncs.sql
 *	  Spatial functions for temporal points.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION SRID(tgeompoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'tpoint_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION SRID(tgeogpoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'tpoint_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tgeompoint, srid integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_set_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setSRID(tgeogpoint, srid integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_set_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(tgeompoint, srid integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_transform'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

----- Gauss Kruger transformation
CREATE FUNCTION transform_gk(tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tgeompoint_transform_gk'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform_gk(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'geometry_transform_gk'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-------

CREATE FUNCTION tgeogpoint(tgeompoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tgeompoint_to_tgeogpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint(tgeogpoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tgeogpoint_to_tgeompoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS tgeogpoint) WITH FUNCTION tgeogpoint(tgeompoint);
CREATE CAST (tgeogpoint AS tgeompoint) WITH FUNCTION tgeompoint(tgeogpoint);

CREATE FUNCTION setprecision(tgeompoint, int)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_set_precision'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setprecision(tgeogpoint, int)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_set_precision'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trajectory(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tpoint_trajectory'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION trajectory(tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'tpoint_trajectory'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION length(tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION length(tgeogpoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION cumulativeLength(tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cumulativeLength(tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION speed(tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_speed'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION speed(tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_speed'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION twcentroid(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tgeompoint_twcentroid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION azimuth(tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_azimuth'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION azimuth(tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_azimuth'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION atGeometry(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_at_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusGeometry(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_minus_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION NearestApproachInstant(geometry, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION NearestApproachInstant(geography, tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'NAI_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeogpoint, geography)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeogpoint, tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geometry, tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, geometry)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geography, tgeogpoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, geography)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, tgeogpoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);

CREATE OPERATOR |=| (
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);

CREATE FUNCTION shortestLine(geometry, tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shortestLine(geography, tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'shortestline_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION geometry(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tpoint_to_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS geometry) WITH FUNCTION geometry(tgeompoint);

CREATE FUNCTION geography(tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'tpoint_to_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeogpoint AS geography) WITH FUNCTION geography(tgeogpoint);

CREATE FUNCTION tgeompoint(geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'geo_to_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS tgeompoint) WITH FUNCTION tgeompoint(geometry);

CREATE FUNCTION tgeogpoint(geography)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'geo_to_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geography AS tgeogpoint) WITH FUNCTION tgeogpoint(geography);

/*****************************************************************************/

CREATE FUNCTION geoMeasure(tgeompoint, tfloat, boolean DEFAULT FALSE)
RETURNS geometry
AS 'MODULE_PATHNAME', 'tpoint_to_geo_measure'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geoMeasure(tgeogpoint, tfloat, boolean DEFAULT FALSE)
RETURNS geography
AS 'MODULE_PATHNAME', 'tpoint_to_geo_measure'
LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE OR REPLACE FUNCTION simplify(tgeompoint, float8, float8 DEFAULT -1.0)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_simplify'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
