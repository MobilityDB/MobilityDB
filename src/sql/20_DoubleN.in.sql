/*****************************************************************************
 *
 * DoubleN.sql
 *		Internal types used for the average and centroid temporal aggregates. 
 *
 * The double2, double3, and double4 types are composed, respectively, of two, 
 * three, and four double values. The tdouble2, tdouble3, and tdouble4 types 
 * are the corresponding temporal types. The in/out functions of all these
 * types are stubs, as all access should be internal.
 * These types are needed for the transition function of the aggregates,   
 * where the first components of the doubleN values store the sum and the  
 * last one stores the count of the values. The final function computes the 
 * average from the doubleN values.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION double2_in(cstring)
	RETURNS double2
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double2_out(double2)
	RETURNS cstring
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double2_send(double2)
	RETURNS bytea
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double2_recv(internal)
	RETURNS double2
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE double2 (
	internallength = 16,
	input = double2_in,
	output = double2_out,
	send = double2_send,
	receive = double2_recv,
	alignment = double
);

/******************************************************************************/

CREATE FUNCTION double3_in(cstring)
	RETURNS double3
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double3_out(double3)
	RETURNS cstring
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double3_send(double3)
	RETURNS bytea
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double3_recv(internal)
	RETURNS double3
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE double3 (
	internallength = 24,
	input = double3_in,
	output = double3_out,
	send = double3_send,
	receive = double3_recv,
	alignment = double
);

/******************************************************************************/

CREATE FUNCTION double4_in(cstring)
	RETURNS double4
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double4_out(double4)
	RETURNS cstring
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double4_send(double4)
	RETURNS bytea
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION double4_recv(internal)
	RETURNS double4
 	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE double4 (
	internallength = 32,
	input = double4_in,
	output = double4_out,
	send = double4_send,
	receive = double4_recv,
	alignment = double
);

/******************************************************************************
 * Catalog
 ******************************************************************************/	

CREATE TYPE tdouble2;
CREATE TYPE tdouble3;
CREATE TYPE tdouble4;

SELECT register_temporal('tdouble2', 'double2');
SELECT register_temporal('tdouble3', 'double3');
SELECT register_temporal('tdouble4', 'double4');

/******************************************************************************/		

CREATE FUNCTION tdouble2_in(cstring, oid, integer)
	RETURNS tdouble2
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tdouble2)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tdouble2 (
	internallength = variable,
	input = tdouble2_in,
	output = temporal_out,
	alignment = double
);

/******************************************************************************/		

CREATE FUNCTION tdouble3_in(cstring, oid, integer)
	RETURNS tdouble3
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tdouble3)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tdouble3 (
	internallength = variable,
	input = tdouble3_in,
	output = temporal_out,
	alignment = double
);

/******************************************************************************/		

CREATE FUNCTION tdouble4_in(cstring, oid, integer)
	RETURNS tdouble4
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tdouble4)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tdouble4 (
	internallength = variable,
	input = tdouble4_in,
	output = temporal_out,
	alignment = double
);

/******************************************************************************/		
