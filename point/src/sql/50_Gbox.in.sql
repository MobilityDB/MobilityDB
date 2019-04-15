/*****************************************************************************
 *
 * Gbox.sql
 *	  Basic functions for GBOX bounding box.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE gbox;

CREATE FUNCTION gbox_in(cstring)
	RETURNS gbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_out(gbox)
	RETURNS cstring 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
/*
CREATE FUNCTION gbox_recv(internal)
	RETURNS gbox 
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_send(gbox)
	RETURNS bytea 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

CREATE TYPE gbox (
	internallength = 72,
	input = gbox_in,
	output = gbox_out,
--	receive = gbox_recv,
--	send = gbox_send,
	storage = plain,
	alignment = double
--    , analyze = gbox_analyze
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

 CREATE OR REPLACE FUNCTION gbox(float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION gbox(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION gbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION gbox3dm(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox3dm_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION geodbox(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geodbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
