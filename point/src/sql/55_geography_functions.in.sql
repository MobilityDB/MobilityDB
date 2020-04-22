/*****************************************************************************
 *
 * geography_functions.sql
 *	  Spatial functions for PostGIS geography.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineInterpolatePoint(geography, float8, use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_line_interpolate_point'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineInterpolatePoints(geography, float8, use_spheroid boolean DEFAULT true, repeat boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_line_interpolate_point'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_LineLocatePoint(geography, geography, use_spheroid boolean DEFAULT true)
	RETURNS float
	AS 'MODULE_PATHNAME', 'geography_line_locate_point'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_ClosestPoint(geography, geography, use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_closestpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_ShortestLine(geography, geography, use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'geography_shortestline'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Availability: 3.1.0
CREATE OR REPLACE FUNCTION ST_Segmentize1(geog geography, max_segment_length float8, use_spheroid boolean DEFAULT true)
	RETURNS geography
	AS 'MODULE_PATHNAME','geography_segmentize1'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE
	COST 100;


/*****************************************************************************/
