/*****************************************************************************
 *
 * TemporalGeo.sql
 *	  Geometric functions for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Trajectory
 *****************************************************************************/

CREATE FUNCTION trajectory(tnpoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tnpoint_trajectory'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * AtGeometry
 *****************************************************************************/	
	
CREATE FUNCTION atGeometry(tnpoint, geometry)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'tnpoint_at_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * Length
 *****************************************************************************/	
	
CREATE FUNCTION length(tnpoint)
	RETURNS double precision
	AS 'MODULE_PATHNAME', 'tnpoint_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * Cumulative length
 *****************************************************************************/	
		
CREATE FUNCTION cumulativeLength(tnpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnpoint_cumulative_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * Speed
 *****************************************************************************/	

CREATE FUNCTION speed(tnpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnpoint_speed'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
	
/*****************************************************************************
 * Temporal azimuth
 *****************************************************************************/	
	
CREATE FUNCTION azimuth(tnpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnpoint_azimuth'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************/
	