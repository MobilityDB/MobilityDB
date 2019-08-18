/*****************************************************************************
 *
 * periodset.sql
 *	  Functions for set of periods.
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

CREATE TYPE periodset;

CREATE FUNCTION periodset_in(cstring)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'periodset_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_out(periodset)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_recv(internal)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'periodset_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_send(periodset)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_analyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE periodset (
	internallength = variable,
	input = periodset_in,
	output = periodset_out,
	receive = periodset_recv,
	send = periodset_send,
	alignment = double
-- The following line makes NULL if size < 128	
--	storage = extended
    , analyze = periodset_analyze
);

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION periodset(period[])
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'periodset_from_periodarr'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION periodset(timestamptz)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'timestamp_as_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(timestampset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'timestampset_as_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'period_as_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(periodset)
	RETURNS period
	AS 'MODULE_PATHNAME', 'periodset_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE CAST (timestamptz AS periodset) WITH FUNCTION periodset(timestamptz);
CREATE CAST (timestampset AS periodset) WITH FUNCTION periodset(timestampset);
CREATE CAST (period AS periodset) WITH FUNCTION periodset(period);
CREATE CAST (periodset AS period) WITH FUNCTION period(periodset) AS IMPLICIT;

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION memSize(periodset)
	RETURNS int
	AS 'MODULE_PATHNAME', 'periodset_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(periodset)
	RETURNS period
	AS 'MODULE_PATHNAME', 'periodset_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(periodset)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'periodset_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numPeriods(periodset)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'periodset_num_periods'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startPeriod(periodset)
	RETURNS period
	AS 'MODULE_PATHNAME', 'periodset_start_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endPeriod(periodset)
	RETURNS period
	AS 'MODULE_PATHNAME', 'periodset_end_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodN(periodset, integer)
	RETURNS period
	AS 'MODULE_PATHNAME', 'periodset_period_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periods(periodset)
	RETURNS period[]
	AS 'MODULE_PATHNAME', 'periodset_periods'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(periodset)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'periodset_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(periodset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'periodset_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(periodset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'periodset_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(periodset, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'periodset_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(periodset)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'periodset_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(periodset, interval)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'periodset_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION periodset_eq(periodset, periodset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'periodset_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_ne(periodset, periodset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'periodset_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_lt(periodset, periodset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'periodset_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_le(periodset, periodset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'periodset_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_ge(periodset, periodset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'periodset_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_gt(periodset, periodset)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'periodset_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_cmp(periodset, periodset)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'periodset_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
	LEFTARG = periodset, RIGHTARG = periodset,
	PROCEDURE = periodset_eq,
	COMMUTATOR = =, NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = periodset, RIGHTARG = periodset,
	PROCEDURE = periodset_ne,
	COMMUTATOR = <>, NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	LEFTARG = periodset, RIGHTARG = periodset,
	PROCEDURE = periodset_lt,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = periodset, RIGHTARG = periodset,
	PROCEDURE = periodset_le,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = periodsel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
	LEFTARG = periodset, RIGHTARG = periodset,
	PROCEDURE = periodset_ge,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = periodsel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
	LEFTARG = periodset, RIGHTARG = periodset,
	PROCEDURE = periodset_gt,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS periodset_ops
	DEFAULT FOR TYPE periodset USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	periodset_cmp(periodset, periodset);

/******************************************************************************/
