/*****************************************************************************
 *
 * SelFuncs.sql
 *	  Selectivity functions for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION positionseltemp(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'positionseltemp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION positionjoinseltemp(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'positionjoinseltemp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contseltemp(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'contseltemp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contjoinseltemp(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'contjoinseltemp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 
/******************************************************************************/
