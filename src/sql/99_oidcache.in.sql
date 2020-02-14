/*****************************************************************************
 *
 * oidcache.sql
 *		Routine that pre-computes the opcache and store it as a table in 
 *		the catalog.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
 
CREATE TABLE pg_temporal_opcache (
	ltypnum INT,
	rtypnum INT,
	opnum INT,
	opid Oid
);

CREATE FUNCTION fill_opcache()
	RETURNS VOID
	AS 'MODULE_PATHNAME', 'fill_opcache'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

SELECT fill_opcache();

/******************************************************************************/
