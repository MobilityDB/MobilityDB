/*****************************************************************************
 *
 * TimestampSet.sql
 *	  Functions for set of timestamps.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE timestampset;

CREATE FUNCTION timestampset_in(cstring)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'timestampset_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_out(timestampset)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_recv(internal)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'timestampset_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_send(timestampset)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_typanalyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE timestampset (
	internallength = variable,
	input = timestampset_in,
	output = timestampset_out,
	receive = timestampset_recv,
	send = timestampset_send,
	alignment = double
-- The following line makes NULL if size < 128	
--	storage = extended 
   , analyze = timestampset_typanalyze
);

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION timestampset(timestamptz[])
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'timestampset_from_timestamparr'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION timestampset(timestamptz)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'timestamp_as_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS timestampset) WITH FUNCTION timestampset(timestamptz);

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION memSize(timestampset)
	RETURNS int
	AS 'MODULE_PATHNAME', 'timestampset_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(timestampset)
	RETURNS period
	AS 'MODULE_PATHNAME', 'timestampset_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(timestampset)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'timestampset_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(timestampset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'timestampset_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(timestampset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'timestampset_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(timestampset, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'timestampset_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(timestampset)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'timestampset_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(timestampset, interval)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'timestampset_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION timestampset_eq(timestampset, timestampset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'timestampset_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_ne(timestampset, timestampset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'timestampset_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_lt(timestampset, timestampset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'timestampset_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_le(timestampset, timestampset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'timestampset_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_ge(timestampset, timestampset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'timestampset_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_gt(timestampset, timestampset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'timestampset_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_cmp(timestampset, timestampset)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'timestampset_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
	LEFTARG = timestampset, RIGHTARG = timestampset,
	PROCEDURE = timestampset_eq,
	COMMUTATOR = =, NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = timestampset, RIGHTARG = timestampset,
	PROCEDURE = timestampset_ne,
	COMMUTATOR = <>, NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	LEFTARG = timestampset, RIGHTARG = timestampset,
	PROCEDURE = timestampset_lt,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = timestampset, RIGHTARG = timestampset,
	PROCEDURE = timestampset_le,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = periodsel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
	LEFTARG = timestampset, RIGHTARG = timestampset,
	PROCEDURE = timestampset_ge,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = periodsel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
	LEFTARG = timestampset, RIGHTARG = timestampset,
	PROCEDURE = timestampset_gt,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS timestampset_ops
	DEFAULT FOR TYPE timestampset USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	timestampset_cmp(timestampset, timestampset);

/******************************************************************************/
