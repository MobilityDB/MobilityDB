/*****************************************************************************
 *
 * TemporalGeo.sql
 *	  Geometric functions for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION asText(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeompoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeogpoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION asText(geometry[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION asText(geography[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	

CREATE FUNCTION asEWKT(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeompoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeogpoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	

CREATE FUNCTION asEWKT(geometry[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION asEWKT(geography[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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

CREATE FUNCTION transform(tgeogpoint, srid integer)
	RETURNS tgeogpoint
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

CREATE FUNCTION tgeompoint(tgeogpoint)
	RETURNS tgeompoint AS 'MODULE_PATHNAME', 'tgeompoint_as_tgeogpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint(tgeompoint)
	RETURNS tgeogpoint AS 'MODULE_PATHNAME', 'tgeogpoint_as_tgeompoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeogpoint AS tgeompoint) WITH FUNCTION tgeompoint(tgeogpoint);
CREATE CAST (tgeompoint AS tgeogpoint) WITH FUNCTION tgeogpoint(tgeompoint);

CREATE FUNCTION trajectory(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tgeompoint_trajectory'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION trajectory(tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'tgeogpoint_trajectory'
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
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cumulativeLength(tgeogpoint)
	RETURNS tgeogpoint
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
	
CREATE FUNCTION NearestApproachInstant(geometry, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_geometry_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION nearestApproachDistance(geometry, tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_geometry_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, geometry)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, tgeompoint)
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

CREATE FUNCTION shortestLine(geometry, tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_geometry_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, tgeompoint)
	RETURNS geometry
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
