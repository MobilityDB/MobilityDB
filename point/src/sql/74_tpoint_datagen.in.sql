/*****************************************************************************
 *
 * tpoint_datagen.sql
 *	  Data generator for MobilityDB.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Mahmoud Sakr,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

 CREATE FUNCTION create_trip(record[], timestamptz, boolean)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'create_trip'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
