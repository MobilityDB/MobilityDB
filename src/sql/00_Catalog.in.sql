/*****************************************************************************
 *
 * Catalog.sql
 *	  Routines for the temporal catalog.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE TABLE pg_temporal (
	temptypid Oid PRIMARY KEY,
	valuetypid Oid
); 

ALTER TABLE pg_temporal SET SCHEMA pg_catalog;

CREATE FUNCTION register_temporal(temporal CHAR(24), base CHAR(24))
RETURNS void AS $$
BEGIN
	WITH valueid AS (SELECT oid, typname FROM pg_type WHERE typname=base),
	tempid AS (SELECT oid, typname FROM pg_type WHERE typname=temporal)
	INSERT INTO pg_temporal (temptypid, valuetypid) 
	SELECT te.oid, v.oid FROM valueid v, tempid te;
END;
$$ LANGUAGE plpgsql;

/******************************************************************************/
