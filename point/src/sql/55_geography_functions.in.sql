/*****************************************************************************
 *
 * geography_functions.sql
 *	  Spatial functions for PostGIS geography.
 *	These functions are supposed to be included in a forthcoming version of
 * 	PostGIS, proposed as a PR. These functions are not needed in MobilityDB.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_GeographyN(geography, integer)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_geographyn_collection'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_NumGeographies(geography)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'geography_numgeographies_collection'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_MakeLine(geography[])
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_makeline_garray'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineFromMultiPoint(geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_line_from_mpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_MakeLine(geography, geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_makeline'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_NumPoints(geography)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'geography_numpoints_linestring'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_StartPoint(geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_startpoint_linestring'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_EndPoint(geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_endpoint_linestring'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_PointN(geography, integer)
	RETURNS geography
	AS 'MODULE_PATHNAME','geography_pointn_linestring'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_AddPoint(geography, geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_addpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_AddPoint(geography, geography, integer)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_addpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_RemovePoint(geography, integer)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_removepoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_SetPoint(geography, integer, geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_setpoint_linestring'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineInterpolatePoint(geography, float8, 
		use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_line_interpolate_point'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineInterpolatePoints(geography, float8, 
		use_spheroid boolean DEFAULT true, repeat boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_line_interpolate_point'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineLocatePoint(geography, geography, 
		use_spheroid boolean DEFAULT true)
	RETURNS float
	AS 'MODULE_PATHNAME', 'geography_line_locate_point'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineSubstring(geography, float8, float8)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_line_substring'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_ClosestPoint(geography, geography, 
		use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_closestpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_ShortestLine(geography, geography, 
		use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_shortestline'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_Segmentize1(geog geography, 
		max_segment_length float8, use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME','geography_segmentize1'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
	COST 100;


/*****************************************************************************/
