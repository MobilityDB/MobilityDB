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
/*****************************************************************************
 *
 * Period.sql
 *	  SQL definitions for timestamptz periods.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE TYPE period;

CREATE FUNCTION period_in(cstring) 
	RETURNS period
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION period_out(period)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION period_recv(internal)
	RETURNS period
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION period_send(period)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION period_typanalyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE TYPE period (
	internallength = 24,
	input = period_in,
	output = period_out,
	receive = period_recv,
	send = period_send,
	alignment = double,
	analyze = period_typanalyze 
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION period(timestamptz, timestamptz)
	RETURNS period
	AS 'MODULE_PATHNAME', 'period_constructor2'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION period(timestamptz, timestamptz, boolean, boolean)
	RETURNS period
	AS 'MODULE_PATHNAME', 'period_constructor4'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION period(timestamptz)
	RETURNS period
	AS 'MODULE_PATHNAME', 'timestamp_as_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period(tstzrange)
	RETURNS period
	AS 'MODULE_PATHNAME', 'tstzrange_as_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION tstzrange(period)
	RETURNS tstzrange
	AS 'MODULE_PATHNAME', 'period_as_tstzrange'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE CAST (timestamptz AS period) WITH FUNCTION period(timestamptz) AS IMPLICIT;
CREATE CAST (tstzrange AS period) WITH FUNCTION period(tstzrange);
CREATE CAST (period AS tstzrange) WITH FUNCTION tstzrange(period);
	
/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION lower(period)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'period_lower'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION upper(period)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'period_upper'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION lower_inc(period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_lower_inc'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION upper_inc(period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_upper_inc'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION duration(period)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'period_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(period, interval)
	RETURNS period
	AS 'MODULE_PATHNAME', 'period_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION periodsel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION period_eq(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period_ne(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period_lt(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period_le(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period_ge(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period_gt(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'period_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION period_cmp(period, period)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'period_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR = (
	PROCEDURE = period_eq,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = =, NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	PROCEDURE = period_ne,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = <>, NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = period_lt,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = periodsel, JOIN = scalarltjoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = period_le,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = periodsel, JOIN = scalarlejoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = period_ge,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = periodsel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
	PROCEDURE = period_gt,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS period_ops
	DEFAULT FOR TYPE period USING btree	AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	period_cmp(period, period);

/******************************************************************************/

CREATE FUNCTION period_hash(period)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'period_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION period_hash_extended(period)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'period_hash_extended'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR CLASS hash_period_ops
	DEFAULT FOR TYPE period USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   period_hash(period);

/******************************************************************************/
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
CREATE FUNCTION period(timestampset)
	RETURNS period
	AS 'MODULE_PATHNAME', 'timestampset_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS timestampset) WITH FUNCTION timestampset(timestamptz);
CREATE CAST (timestampset AS period) WITH FUNCTION period(timestampset) AS IMPLICIT;

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
/*****************************************************************************
 *
 * PeriodSet.sql
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

CREATE FUNCTION periodset_typanalyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE periodset (
	internallength = variable,
	input = periodset_in,
	output = periodset_out,
	receive = periodset_recv,
	send = periodset_send,
	alignment = double
-- The following line makes NULL if size < 128	
--	storage = extended
    , analyze = periodset_typanalyze
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
/*****************************************************************************
 *
 * Range.sql
 *		Definition of range types corresponding to temporal types and 
 *		extension of the operators for them.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE TYPE intrange;

CREATE FUNCTION intrange_canonical(r intrange)
	RETURNS intrange
	AS 'MODULE_PATHNAME', 'intrange_canonical'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE TYPE intrange AS RANGE (
	subtype = integer,
	SUBTYPE_DIFF = int4range_subdiff,
	CANONICAL = intrange_canonical
); 
	
CREATE TYPE floatrange AS RANGE (
	subtype = float8,
	SUBTYPE_DIFF = float8mi
);
 
/******************************************************************************/

CREATE FUNCTION range_left(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_left_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_left(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_left_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = intrange, RIGHTARG = integer,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = integer, RIGHTARG = intrange,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_right(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_right_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_right(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_right_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = intrange, RIGHTARG = integer,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = integer, RIGHTARG = intrange,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_overleft(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overleft_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overleft(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overleft_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = intrange, RIGHTARG = integer,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = integer, RIGHTARG = intrange,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_overright(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overright_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overright(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overright_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = intrange, RIGHTARG = integer,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = integer, RIGHTARG = intrange,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_adjacent(intrange, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_adjacent_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_adjacent(integer, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_adjacent_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = intrange, RIGHTARG = integer,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = integer, RIGHTARG = intrange,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);

/******************************************************************************/

CREATE FUNCTION range_left(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_left_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_left(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_left_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = floatrange, RIGHTARG = float,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR << (
	PROCEDURE = range_left,
	LEFTARG = float, RIGHTARG = floatrange,
	COMMUTATOR = >>,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_right(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_right_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_right(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_right_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = floatrange, RIGHTARG = float,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR >> (
	PROCEDURE = range_right,
	LEFTARG = float, RIGHTARG = floatrange,
	COMMUTATOR = <<,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_overleft(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overleft_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overleft(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overleft_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = floatrange, RIGHTARG = float,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &< (
	PROCEDURE = range_overleft,
	LEFTARG = float, RIGHTARG = floatrange,
	RESTRICT = rangesel, JOIN = scalarltjoinsel
);

CREATE FUNCTION range_overright(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_overright_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_overright(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_overright_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = floatrange, RIGHTARG = float,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);
CREATE OPERATOR &> (
	PROCEDURE = range_overright,
	LEFTARG = float, RIGHTARG = floatrange,
	RESTRICT = rangesel, JOIN = scalargtjoinsel
);

CREATE FUNCTION range_adjacent(floatrange, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'range_adjacent_elem'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range_adjacent(float, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'elem_adjacent_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = floatrange, RIGHTARG = float,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = range_adjacent,
	LEFTARG = float, RIGHTARG = floatrange,
	COMMUTATOR = -|-,
	RESTRICT = contsel, JOIN = contjoinsel
);

/******************************************************************************/
/*****************************************************************************
 *
 * TimeTypes.sql
 *	  Operators for time types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION temporal_contains(timestampset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(period, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(periodset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contains(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = period, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR @> (
	PROCEDURE = temporal_contains,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = <@,
	RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE FUNCTION temporal_contained(timestamptz, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(timestamptz, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(timestamptz, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_contained(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = timestamptz, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = temporal_contained,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = @>,
	RESTRICT = periodsel, JOIN = contjoinsel
);

CREATE FUNCTION temporal_overlaps(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overlaps(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overlaps(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);
CREATE OPERATOR && (
	PROCEDURE = temporal_overlaps,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = &&,
	RESTRICT = periodsel, JOIN = areajoinsel
);

CREATE FUNCTION temporal_before(timestamptz, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(timestamptz, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(timestamptz, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(timestampset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(period, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(periodset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_before(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestamptz, RIGHTARG = period,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = period, RIGHTARG = timestamptz,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = #>>, 
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION temporal_after(timestamptz, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestamptz, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(timestamptz, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(timestampset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(period, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_after(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestamptz, RIGHTARG = period,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = period, RIGHTARG = timestamptz,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = <<#,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE FUNCTION temporal_overbefore(timestamptz, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestamptz, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(timestamptz, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(timestampset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(period, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(periodset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overbefore(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestamptz, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestampset, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = timestampset, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = period, RIGHTARG = timestamptz,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = period, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = period, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = period, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = periodset, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = periodset, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = periodset, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION temporal_overafter(timestamptz, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestamptz, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(timestampset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(timestampset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(period, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(periodset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_overafter(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestamptz, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestampset, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = timestampset, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = period, RIGHTARG = timestamptz,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = period, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = period, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = period, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = periodset, RIGHTARG = timestampset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = periodset, RIGHTARG = period,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = periodset, RIGHTARG = periodset,
	RESTRICT = periodsel, JOIN = scalargtjoinsel
);

CREATE FUNCTION temporal_adjacent(timestamptz, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(timestamptz, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(timestampset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(timestampset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_adjacent(period, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(period, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(period, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(period, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_adjacent(periodset, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(periodset, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(periodset, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_adjacent(periodset, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'adjacent_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = timestamptz, RIGHTARG = period,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = period, RIGHTARG = timestamptz,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);
CREATE OPERATOR -|- (
	PROCEDURE = temporal_adjacent,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = -|-,
	RESTRICT = periodsel, JOIN = contjoinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_union(timestamptz, timestamptz)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'union_timestamp_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(timestamptz, timestampset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'union_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(timestamptz, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(timestamptz, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestamptz, RIGHTARG = timestamptz,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestamptz, RIGHTARG = period,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	COMMUTATOR = +
);

CREATE FUNCTION temporal_union(timestampset, timestamptz)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'union_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(timestampset, timestampset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'union_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(timestampset, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(timestampset, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = +
);

CREATE FUNCTION temporal_union(period, timestamptz)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(period, timestampset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(period, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(period, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = period, RIGHTARG = timestamptz,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = +
);

CREATE FUNCTION temporal_union(periodset, timestamptz)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(periodset, timestampset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(periodset, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_union(periodset, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'union_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_union,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION temporal_minus(timestamptz, timestamptz)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'minus_timestamp_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(timestamptz, timestampset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'minus_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(timestamptz, period)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'minus_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(timestamptz, periodset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'minus_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestamptz, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestamptz, RIGHTARG = timestampset
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestamptz, RIGHTARG = period
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestamptz, RIGHTARG = periodset
);

CREATE FUNCTION temporal_minus(timestampset, timestamptz)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'minus_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(timestampset, timestampset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'minus_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(timestampset, period)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'minus_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(timestampset, periodset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'minus_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestampset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestampset, RIGHTARG = timestampset
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestampset, RIGHTARG = period
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = timestampset, RIGHTARG = periodset
);

CREATE FUNCTION temporal_minus(period, timestamptz)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(period, timestampset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(period, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(period, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = period, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = period, RIGHTARG = timestampset
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = period, RIGHTARG = period
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = period, RIGHTARG = periodset
);

CREATE FUNCTION temporal_minus(periodset, timestamptz)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(periodset, timestampset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(periodset, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_minus(periodset, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'minus_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = periodset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = periodset, RIGHTARG = timestampset
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = periodset, RIGHTARG = period
);
CREATE OPERATOR - (
	PROCEDURE = temporal_minus,
	LEFTARG = periodset, RIGHTARG = periodset
);

/*****************************************************************************/

CREATE FUNCTION temporal_intersection(timestamptz, timestamptz)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_timestamp_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(timestamptz, timestampset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_timestamp_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(timestamptz, period)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_timestamp_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(timestamptz, periodset)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_timestamp_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestamptz, RIGHTARG = timestamptz,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestamptz, RIGHTARG = timestampset,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestamptz, RIGHTARG = period,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestamptz, RIGHTARG = periodset,
	COMMUTATOR = *
);

CREATE FUNCTION temporal_intersection(timestampset, timestamptz)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_timestampset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(timestampset, timestampset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'intersection_timestampset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(timestampset, period)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'intersection_timestampset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(timestampset, periodset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'intersection_timestampset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestampset, RIGHTARG = timestamptz,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestampset, RIGHTARG = timestampset,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestampset, RIGHTARG = period,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = timestampset, RIGHTARG = periodset,
	COMMUTATOR = *
);


CREATE FUNCTION temporal_intersection(period, timestamptz)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_period_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(period, timestampset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'intersection_period_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(period, period)
	RETURNS period
	AS 'MODULE_PATHNAME', 'intersection_period_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(period, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'intersection_period_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = period, RIGHTARG = timestamptz,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = period, RIGHTARG = timestampset,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = period, RIGHTARG = period,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = period, RIGHTARG = periodset,
	COMMUTATOR = *
);

CREATE FUNCTION temporal_intersection(periodset, timestamptz)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'intersection_periodset_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(periodset, timestampset)
	RETURNS timestampset
	AS 'MODULE_PATHNAME', 'intersection_periodset_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(periodset, period)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'intersection_periodset_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_intersection(periodset, periodset)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'intersection_periodset_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = periodset, RIGHTARG = timestamptz,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = periodset, RIGHTARG = timestampset,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = periodset, RIGHTARG = period,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_intersection,
	LEFTARG = periodset, RIGHTARG = periodset,
	COMMUTATOR = *
);

/*****************************************************************************/
/*****************************************************************************
 *
 * IndexGistTime.sql
 *		R-tree GiST index for time types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_timestampset_consistent(internal, timestampset, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_time_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_time_union(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_timestampset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_timestampset_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_time_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_time_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_time_same(period, period, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gist_time_fetch(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR CLASS gist_timestampset_ops
	DEFAULT FOR TYPE timestampset USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (timestampset, timestampset),
	OPERATOR	3		&& (timestampset, period),
	OPERATOR	3		&& (timestampset, periodset),
	-- contains
	OPERATOR	7		@> (timestampset, timestamptz),
	OPERATOR	7		@> (timestampset, timestampset),
	-- contained by
	OPERATOR	8		<@ (timestampset, timestampset),
	OPERATOR	8		<@ (timestampset, period),
	OPERATOR	8		<@ (timestampset, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (timestampset, timestamptz),
	OPERATOR	28		&<# (timestampset, timestampset),
	OPERATOR	28		&<# (timestampset, period),
	OPERATOR	28		&<# (timestampset, periodset),
	-- strictly before
	OPERATOR	29		<<# (timestampset, timestamptz),
	OPERATOR	29		<<# (timestampset, timestampset),
	OPERATOR	29		<<# (timestampset, period),
	OPERATOR	29		<<# (timestampset, periodset),
	-- strictly after
	OPERATOR	30		#>> (timestampset, timestamptz),
	OPERATOR	30		#>> (timestampset, timestampset),
	OPERATOR	30		#>> (timestampset, period),
	OPERATOR	30		#>> (timestampset, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (timestampset, timestamptz),
	OPERATOR	31		#&> (timestampset, timestampset),
	OPERATOR	31		#&> (timestampset, period),
	OPERATOR	31		#&> (timestampset, periodset),
	-- functions
	FUNCTION	1	gist_timestampset_consistent(internal, timestampset, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_timestampset_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);
	
/******************************************************************************/

CREATE FUNCTION gist_period_consistent(internal, period, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_time_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_period_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_period_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_time_ops
	DEFAULT FOR TYPE period USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (period, timestampset),
	OPERATOR	3		&& (period, period),
	OPERATOR	3		&& (period, periodset),
	-- contains
	OPERATOR	7		@> (period, timestamptz),
	OPERATOR	7		@> (period, timestampset),
	OPERATOR	7		@> (period, period),
	OPERATOR	7		@> (period, periodset),
	-- contained by
	OPERATOR	8		<@ (period, period),
	OPERATOR	8		<@ (period, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (period, timestamptz),
	OPERATOR	28		&<# (period, timestampset),
	OPERATOR	28		&<# (period, period),
	OPERATOR	28		&<# (period, periodset),
	-- strictly before
	OPERATOR	29		<<# (period, timestamptz),
	OPERATOR	29		<<# (period, timestampset),
	OPERATOR	29		<<# (period, period),
	OPERATOR	29		<<# (period, periodset),
	-- strictly after
	OPERATOR	30		#>> (period, timestamptz),
	OPERATOR	30		#>> (period, timestampset),
	OPERATOR	30		#>> (period, period),
	OPERATOR	30		#>> (period, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (period, timestamptz),
	OPERATOR	31		#&> (period, timestampset),
	OPERATOR	31		#&> (period, period),
	OPERATOR	31		#&> (period, periodset),
	-- functions
	FUNCTION	1	gist_period_consistent(internal, period, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_period_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal),
	FUNCTION	9	gist_time_fetch(internal);
	
/******************************************************************************/

CREATE FUNCTION gist_periodset_consistent(internal, periodset, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_time_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_periodset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_periodset_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_periodset_ops
	DEFAULT FOR TYPE periodset USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (periodset, timestampset),
	OPERATOR	3		&& (periodset, period),
	OPERATOR	3		&& (periodset, periodset),
	-- contains
	OPERATOR	7		@> (periodset, timestamptz),
	OPERATOR	7		@> (periodset, timestampset),
	OPERATOR	7		@> (periodset, period),
	OPERATOR	7		@> (periodset, periodset),
	-- contained by
	OPERATOR	8		<@ (periodset, period),
	OPERATOR	8		<@ (periodset, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (periodset, timestamptz),
	OPERATOR	28		&<# (periodset, timestampset),
	OPERATOR	28		&<# (periodset, period),
	OPERATOR	28		&<# (periodset, periodset),
	-- strictly before
	OPERATOR	29		<<# (periodset, timestamptz),
	OPERATOR	29		<<# (periodset, timestampset),
	OPERATOR	29		<<# (periodset, period),
	OPERATOR	29		<<# (periodset, periodset),
	-- strictly after
	OPERATOR	30		#>> (periodset, timestamptz),
	OPERATOR	30		#>> (periodset, timestampset),
	OPERATOR	30		#>> (periodset, period),
	OPERATOR	30		#>> (periodset, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (periodset, timestamptz),
	OPERATOR	31		#&> (periodset, timestampset),
	OPERATOR	31		#&> (periodset, period),
	OPERATOR	31		#&> (periodset, periodset),
	-- functions
	FUNCTION	1	gist_periodset_consistent(internal, periodset, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_periodset_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);

/******************************************************************************/
/*****************************************************************************
 *
 * IndexSPGistTime.sql
 *		Quad-tree SP-GiST index for time types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION spgist_time_config(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_choose(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_picksplit(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_time_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_timestampset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_periodset_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************/

CREATE OPERATOR CLASS spgist_timestampset_ops
	DEFAULT FOR TYPE timestampset USING spgist AS
--	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (timestampset, timestampset),
	OPERATOR	3		&& (timestampset, period),
	OPERATOR	3		&& (timestampset, periodset),
	-- contains
	OPERATOR	7		@> (timestampset, timestamptz),
	OPERATOR	7		@> (timestampset, timestampset),
	-- contained by
	OPERATOR	8		<@ (timestampset, timestampset),
	OPERATOR	8		<@ (timestampset, period),
	OPERATOR	8		<@ (timestampset, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (timestampset, timestamptz),
	OPERATOR	28		&<# (timestampset, timestampset),
	OPERATOR	28		&<# (timestampset, period),
	OPERATOR	28		&<# (timestampset, periodset),
	-- strictly before
	OPERATOR	29		<<# (timestampset, timestamptz),
	OPERATOR	29		<<# (timestampset, timestampset),
	OPERATOR	29		<<# (timestampset, period),
	OPERATOR	29		<<# (timestampset, periodset),
	-- strictly after
	OPERATOR	30		#>> (timestampset, timestamptz),
	OPERATOR	30		#>> (timestampset, timestampset),
	OPERATOR	30		#>> (timestampset, period),
	OPERATOR	30		#>> (timestampset, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (timestampset, timestamptz),
	OPERATOR	31		#&> (timestampset, timestampset),
	OPERATOR	31		#&> (timestampset, period),
	OPERATOR	31		#&> (timestampset, periodset),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_time_inner_consistent(internal, internal),
	FUNCTION	5	spgist_time_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_timestampset_compress(internal);
	
/******************************************************************************/

CREATE OPERATOR CLASS spgist_period_ops
	DEFAULT FOR TYPE period USING spgist AS
	-- overlaps
	OPERATOR	3		&& (period, timestampset),
	OPERATOR	3		&& (period, period),
	OPERATOR	3		&& (period, periodset),
	-- contains
	OPERATOR	7		@> (period, timestamptz),
	OPERATOR	7		@> (period, timestampset),
	OPERATOR	7		@> (period, period),
	OPERATOR	7		@> (period, periodset),
	-- contained by
	OPERATOR	8		<@ (period, period),
	OPERATOR	8		<@ (period, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (period, timestamptz),
	OPERATOR	28		&<# (period, timestampset),
	OPERATOR	28		&<# (period, period),
	OPERATOR	28		&<# (period, periodset),
	-- strictly before
	OPERATOR	29		<<# (period, timestamptz),
	OPERATOR	29		<<# (period, timestampset),
	OPERATOR	29		<<# (period, period),
	OPERATOR	29		<<# (period, periodset),
	-- strictly after
	OPERATOR	30		#>> (period, timestamptz),
	OPERATOR	30		#>> (period, timestampset),
	OPERATOR	30		#>> (period, period),
	OPERATOR	30		#>> (period, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (period, timestamptz),
	OPERATOR	31		#&> (period, timestampset),
	OPERATOR	31		#&> (period, period),
	OPERATOR	31		#&> (period, periodset),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_time_inner_consistent(internal, internal),
	FUNCTION	5	spgist_time_leaf_consistent(internal, internal);
	
/******************************************************************************/

CREATE OPERATOR CLASS spgist_periodset_ops
	DEFAULT FOR TYPE periodset USING spgist AS
--	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (periodset, timestampset),
	OPERATOR	3		&& (periodset, period),
	OPERATOR	3		&& (periodset, periodset),
	-- contains
	OPERATOR	7		@> (periodset, timestamptz),
	OPERATOR	7		@> (periodset, timestampset),
	OPERATOR	7		@> (periodset, period),
	OPERATOR	7		@> (periodset, periodset),
	-- contained by
	OPERATOR	8		<@ (periodset, period),
	OPERATOR	8		<@ (periodset, periodset),
	-- overlaps or before
	OPERATOR	28		&<# (periodset, timestamptz),
	OPERATOR	28		&<# (periodset, timestampset),
	OPERATOR	28		&<# (periodset, period),
	OPERATOR	28		&<# (periodset, periodset),
	-- strictly before
	OPERATOR	29		<<# (periodset, timestamptz),
	OPERATOR	29		<<# (periodset, timestampset),
	OPERATOR	29		<<# (periodset, period),
	OPERATOR	29		<<# (periodset, periodset),
	-- strictly after
	OPERATOR	30		#>> (periodset, timestamptz),
	OPERATOR	30		#>> (periodset, timestampset),
	OPERATOR	30		#>> (periodset, period),
	OPERATOR	30		#>> (periodset, periodset),
	-- overlaps or after
	OPERATOR	31		#&> (periodset, timestamptz),
	OPERATOR	31		#&> (periodset, timestampset),
	OPERATOR	31		#&> (periodset, period),
	OPERATOR	31		#&> (periodset, periodset),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_time_inner_consistent(internal, internal),
	FUNCTION	5	spgist_time_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_periodset_compress(internal);

/******************************************************************************/
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
/*****************************************************************************
 *
 * Temporal.sql
 *	  Basic functions for generic temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE TYPE tbool;
CREATE TYPE tint;
CREATE TYPE tfloat;
CREATE TYPE ttext;

SELECT register_temporal('tbool', 'bool') ;
SELECT register_temporal('tint', 'int4') ;
SELECT register_temporal('tfloat', 'float8') ;
SELECT register_temporal('ttext', 'text') ;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION tbool_in(cstring, oid, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_in(cstring, oid, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_in(cstring, oid, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_in(cstring, oid, integer)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_out(tbool)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tint)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tfloat)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(ttext)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tbool_recv(internal, oid, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_recv(internal, oid, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_recv(internal, oid, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_recv(internal, oid, integer)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_send(tbool)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tint)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tfloat)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(ttext)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_typmod_in(cstring[])
	RETURNS integer
	AS 'MODULE_PATHNAME','temporal_typmod_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_typmod_out(integer)
	RETURNS cstring
	AS 'MODULE_PATHNAME','temporal_typmod_out'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION temporal_analyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION tnumber_analyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE TYPE tbool (
	internallength = variable,
	input = tbool_in,
	output = temporal_out,
	send = temporal_send,
	receive = tbool_recv,
	typmod_in = temporal_typmod_in,
	typmod_out = temporal_typmod_out,
	storage = extended,
	alignment = double,
	analyze = temporal_analyze
);
CREATE TYPE tint (
	internallength = variable,
	input = tint_in,
	output = temporal_out,
	send = temporal_send,
	receive = tint_recv,
	typmod_in = temporal_typmod_in,
	typmod_out = temporal_typmod_out,
	storage = extended,
	alignment = double,
	analyze = tnumber_analyze
);
CREATE TYPE tfloat (
	internallength = variable,
	input = tfloat_in,
	output = temporal_out,
	send = temporal_send,
	receive = tfloat_recv,
	typmod_in = temporal_typmod_in,
	typmod_out = temporal_typmod_out,
	storage = extended,
	alignment = double,
	analyze = tnumber_analyze
);
CREATE TYPE ttext (
	internallength = variable,
	input = ttext_in,
	output = temporal_out,
	send = temporal_send,
	receive = ttext_recv,
	typmod_in = temporal_typmod_in,
	typmod_out = temporal_typmod_out,
	storage = extended,
	alignment = double,
    analyze = temporal_analyze
);

-- Special cast for enforcing the typmod restrictions
CREATE FUNCTION tbool(tbool, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME','temporal_enforce_typmod'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME','temporal_enforce_typmod'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat(tfloat, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME','temporal_enforce_typmod'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext(ttext, integer)
	RETURNS ttext
	AS 'MODULE_PATHNAME','temporal_enforce_typmod'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbool AS tbool) WITH FUNCTION tbool(tbool, integer) AS IMPLICIT;
CREATE CAST (tint AS tint) WITH FUNCTION tint(tint, integer) AS IMPLICIT;
CREATE CAST (tfloat AS tfloat) WITH FUNCTION tfloat(tfloat, integer) AS IMPLICIT;
CREATE CAST (ttext AS ttext) WITH FUNCTION ttext(ttext, integer) AS IMPLICIT;

/******************************************************************************
 * Constructors
 ******************************************************************************/

/* Temporal instant */

CREATE FUNCTION tboolinst(val boolean, t timestamptz)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintinst(val integer, t timestamptz)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatinst(val float, t timestamptz)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextinst(val text, t timestamptz)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal instant set */

CREATE FUNCTION tbooli(tbool[])
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tinti(tint[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloati(tfloat[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexti(ttext[])
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal sequence */

CREATE FUNCTION tboolseq(tbool[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_make_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintseq(tint[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_make_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatseq(tfloat[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_make_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextseq(ttext[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_make_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal sequence set */
	
CREATE FUNCTION tbools(tbool[])
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tints(tint[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloats(tfloat[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexts(ttext[])
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Append function
 ******************************************************************************/

 CREATE FUNCTION appendInstant(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_append_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstant(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_append_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstant(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_append_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstant(ttext, ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_append_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Cast functions
 ******************************************************************************/

CREATE FUNCTION tfloat(tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tint_as_tfloat'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tint AS tfloat) WITH FUNCTION tfloat(tint);

/******************************************************************************
 * Transformation functions
 ******************************************************************************/

CREATE FUNCTION tboolinst(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbooli(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboolseq(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbools(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tintinst(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tinti(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintseq(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tints(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tfloatinst(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloati(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatseq(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloats(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttextinst(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexti(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextseq(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexts(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION temporalType(tbool)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporalType(tint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporalType(tfloat)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporalType(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
 
CREATE FUNCTION memSize(tbool)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(tint)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(tfloat)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(ttext)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*
CREATE FUNCTION period(tbool)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tint)
	RETURNS box
	AS 'MODULE_PATHNAME', 'temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tfloat)
	RETURNS box
	AS 'MODULE_PATHNAME', 'temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(ttext)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
*/

-- values is a reserved word in SQL
CREATE FUNCTION getValue(tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValue(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValue(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValue(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- values is a reserved word in SQL
CREATE FUNCTION getValues(tbool)
	RETURNS boolean[]
	AS 'MODULE_PATHNAME', 'tempdisc_get_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(tint)
	RETURNS integer[]
	AS 'MODULE_PATHNAME', 'tempdisc_get_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(tfloat)
	RETURNS floatrange[]
	AS 'MODULE_PATHNAME', 'tfloat_ranges'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(ttext)
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tempdisc_get_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueRange(tint)
	RETURNS intrange
	AS 'MODULE_PATHNAME', 'tnumber_value_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueRange(tfloat)
	RETURNS floatrange
	AS 'MODULE_PATHNAME', 'tnumber_value_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_start_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_start_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_start_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_start_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_end_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_end_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_end_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_end_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minValue(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_min_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minValue(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_min_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minValue(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_min_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION maxValue(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_max_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION maxValue(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_max_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION maxValue(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_max_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(tbool)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTimestamp(tint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTimestamp(tfloat)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTimestamp(ttext)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(tbool)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTime(tint)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTime(tfloat)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTime(ttext)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(tbool)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(tint)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(tfloat)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(ttext)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tbool)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(tint)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(tfloat)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(ttext)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(tbool)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSequences(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSequences(tfloat)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSequences(ttext)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_start_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSequence(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_start_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSequence(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_start_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSequence(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_start_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_end_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSequence(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_end_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSequence(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_end_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSequence(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_end_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(tbool, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_sequence_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequenceN(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_sequence_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequenceN(tfloat, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_sequence_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequenceN(ttext, integer)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_sequence_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(tbool)
	RETURNS tbool[]
	AS 'MODULE_PATHNAME', 'temporal_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequences(tint)
	RETURNS tint[]
	AS 'MODULE_PATHNAME', 'temporal_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequences(tfloat)
	RETURNS tfloat[]
	AS 'MODULE_PATHNAME', 'temporal_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequences(ttext)
	RETURNS ttext[]
	AS 'MODULE_PATHNAME', 'temporal_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(tbool)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numInstants(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numInstants(tfloat)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numInstants(ttext)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_start_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startInstant(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_start_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startInstant(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_start_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startInstant(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_start_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_end_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endInstant(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_end_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endInstant(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_end_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endInstant(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_end_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(tbool, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_instant_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instantN(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_instant_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instantN(tfloat, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_instant_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instantN(ttext, integer)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_instant_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(tbool)
	RETURNS tbool[]
	AS 'MODULE_PATHNAME', 'temporal_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instants(tint)
	RETURNS tint[]
	AS 'MODULE_PATHNAME', 'temporal_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instants(tfloat)
	RETURNS tfloat[]
	AS 'MODULE_PATHNAME', 'temporal_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instants(ttext)
	RETURNS ttext[]
	AS 'MODULE_PATHNAME', 'temporal_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tbool)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numTimestamps(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numTimestamps(tfloat)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numTimestamps(ttext)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(tbool)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startTimestamp(tint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startTimestamp(tfloat)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startTimestamp(ttext)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(tbool)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endTimestamp(tint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endTimestamp(tfloat)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endTimestamp(ttext)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(tbool, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampN(tint, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampN(tfloat, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampN(ttext, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(tbool)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'temporal_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestamps(tint)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'temporal_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestamps(tfloat)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'temporal_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestamps(ttext)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'temporal_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ever_equals(tbool, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_equals(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_equals(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_equals(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &= (
	LEFTARG = tbool, RIGHTARG = boolean,
	PROCEDURE = ever_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_equals(tbool, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_equals(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_equals(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_equals(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
	LEFTARG = tbool, RIGHTARG = boolean,
	PROCEDURE = always_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR @= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR @= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR @= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION shift(tbool, interval)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(tint, interval)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(tfloat, interval)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(ttext, interval)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION isContinuousInValue(tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporals_continuous_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION isContinuousInTime(tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporals_continuous_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION isContinuousInTime(tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporals_continuous_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION isContinuousInTime(tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporals_continuous_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION isContinuousInTime(ttext)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporals_continuous_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-------------------------------------------------------------------------------
-- Restriction functions
-------------------------------------------------------------------------------

CREATE FUNCTION atValue(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValue(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValue(tfloat, float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValue(ttext, text)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValue(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(tfloat, float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(ttext, text)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atValues(tbool, boolean[])
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(tint, integer[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(tfloat, float[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(ttext, text[])
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValues(tbool, boolean[])
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(tint, integer[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(tfloat, float[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(ttext, text[])
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atRange(tint, intrange)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'tnumber_at_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION atRange(tfloat, floatrange)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnumber_at_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION minusRange(tint, intrange)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'tnumber_minus_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusRange(tfloat, floatrange)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnumber_minus_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atRanges(tint, intrange[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'tnumber_at_ranges'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION atRanges(tfloat, floatrange[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnumber_at_ranges'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE FUNCTION minusRanges(tint, intrange[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'tnumber_minus_ranges'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusRanges(tfloat, floatrange[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tnumber_minus_ranges'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atMin(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_min'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atMin(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_min'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atMin(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_min'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusMin(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_min'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusMin(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_min'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusMin(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_min'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atMax(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_max'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atMax(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_max'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atMax(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_max'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusMax(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_max'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusMax(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_max'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusMax(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_max'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTimestamp(tbool, timestamptz)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestamp(tint, timestamptz)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestamp(tfloat, timestamptz)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestamp(ttext, timestamptz)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTimestamp(tbool, timestamptz)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestamp(tint, timestamptz)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestamp(tfloat, timestamptz)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestamp(ttext, timestamptz)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(tbool, timestamptz)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueAtTimestamp(tint, timestamptz)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueAtTimestamp(tfloat, timestamptz)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueAtTimestamp(ttext, timestamptz)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTimestampSet(tbool, timestampset)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestampSet(tint, timestampset)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestampSet(tfloat, timestampset)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestampSet(ttext, timestampset)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTimestampSet(tbool, timestampset)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestampSet(tint, timestampset)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestampSet(tfloat, timestampset)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestampSet(ttext, timestampset)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atPeriod(tbool, period)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriod(tint, period)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriod(tfloat, period)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriod(ttext, period)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusPeriod(tbool, period)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_minus_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriod(tint, period)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriod(tfloat, period)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriod(ttext, period)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atPeriodSet(tbool, periodset)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriodSet(tint, periodset)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriodSet(tfloat, periodset)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriodSet(ttext, periodset)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusPeriodSet(tbool, periodset)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_minus_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriodSet(tint, periodset)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_minus_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriodSet(tfloat, periodset)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_minus_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriodSet(ttext, periodset)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_minus_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsTimestamp(tbool, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestamp(tint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestamp(tfloat, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestamp(ttext, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsTimestampSet(tbool, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestampSet(tint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestampSet(tfloat, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestampSet(ttext, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsPeriod(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriod(tint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriod(tfloat, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriod(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsPeriodSet(tbool, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriodSet(tint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriodSet(tfloat, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriodSet(ttext, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION integral(tint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tint_integral'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION integral(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tfloat_integral'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION twAvg(tint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tint_twavg'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twAvg(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tfloat_twavg'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION tbool_lt(tbool, tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_le(tbool, tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_eq(tbool, tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_ne(tbool, tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_ge(tbool, tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_gt(tbool, tbool)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbool_cmp(tbool, tbool)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = tbool_lt,
	COMMUTATOR = >,
	NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = tbool_le,
	COMMUTATOR = >=,
	NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = tbool_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = tbool_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = tbool_ge,
	COMMUTATOR = <=,
	NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = tbool_gt,
	COMMUTATOR = <,
	NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tbool_ops
	DEFAULT FOR TYPE tbool USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	tbool_cmp(tbool, tbool);

/*****************************************************************************/

CREATE FUNCTION tint_lt(tint, tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_le(tint, tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_eq(tint, tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_ne(tint, tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_ge(tint, tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_gt(tint, tint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_cmp(tint, tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = tint_lt,
	COMMUTATOR = >,
	NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = tint_le,
	COMMUTATOR = >=,
	NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = tint_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = tint_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = tint_ge,
	COMMUTATOR = <=,
	NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = tint_gt,
	COMMUTATOR = <,
	NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tint_ops
	DEFAULT FOR TYPE tint USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	tint_cmp(tint, tint);

/*****************************************************************************/

CREATE FUNCTION tfloat_lt(tfloat, tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_le(tfloat, tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_eq(tfloat, tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_ne(tfloat, tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_ge(tfloat, tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_gt(tfloat, tfloat)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_cmp(tfloat, tfloat)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = tfloat_lt,
	COMMUTATOR = >,
	NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = tfloat_le,
	COMMUTATOR = >=,
	NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = tfloat_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = tfloat_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = tfloat_ge,
	COMMUTATOR = <=,
	NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = tfloat_gt,
	COMMUTATOR = <,
	NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tfloat_ops
	DEFAULT FOR TYPE tfloat USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	tfloat_cmp(tfloat, tfloat);
		
/******************************************************************************/

CREATE FUNCTION ttext_lt(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_le(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_eq(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_ne(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_ge(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_gt(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_cmp(ttext, ttext)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = ttext_lt,
	COMMUTATOR = >,
	NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = ttext_le,
	COMMUTATOR = >=,
	NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = ttext_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = ttext_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = ttext_ge,
	COMMUTATOR = <=,
	NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = ttext_gt,
	COMMUTATOR = <,
	NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS ttext_ops
	DEFAULT FOR TYPE ttext USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	ttext_cmp(ttext, ttext);

/******************************************************************************/

CREATE FUNCTION tbool_hash(tbool)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_hash(tint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloat_hash(tfloat)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttext_hash(ttext)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS hash_tbool_ops
	DEFAULT FOR TYPE tbool USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   tbool_hash(tbool);
CREATE OPERATOR CLASS hash_tint_ops
	DEFAULT FOR TYPE tint USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   tint_hash(tint);
CREATE OPERATOR CLASS hash_tfloat_ops
	DEFAULT FOR TYPE tfloat USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   tfloat_hash(tfloat);
CREATE OPERATOR CLASS hash_ttext_ops
	DEFAULT FOR TYPE ttext USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   ttext_hash(ttext);

/******************************************************************************/
/*****************************************************************************
 *
 * ArithmeticOps.sql
 *	  Temporal arithmetic functions and operators.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal addition
 *****************************************************************************/

/* int + <TYPE> */

CREATE FUNCTION temporal_add(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'add_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = +
);

/*****************************************************************************/

/* float + <TYPE> */

CREATE FUNCTION temporal_add(float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_add(float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************/
/* tint + <TYPE> */

CREATE FUNCTION temporal_add(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'add_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_add(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat + <TYPE> */

CREATE FUNCTION temporal_add(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_add(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'add_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR + (
	PROCEDURE = temporal_add,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************
 * Temporal subtraction
 *****************************************************************************/

/* int - <TYPE> */

CREATE FUNCTION temporal_sub(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'sub_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = integer, RIGHTARG = tint
);
	
/*****************************************************************************/

/* tint - <TYPE> */

CREATE FUNCTION temporal_sub(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'sub_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_sub(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = float
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = tint
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tint, RIGHTARG = tfloat
);

/*****************************************************************************/

/* float - <TYPE> */

CREATE FUNCTION temporal_sub(f float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(f float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = float, RIGHTARG = tint
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat - <TYPE> */

CREATE FUNCTION temporal_sub(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_sub(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_sub(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'sub_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tfloat, RIGHTARG = tint
);
CREATE OPERATOR - (
	PROCEDURE = temporal_sub,
	LEFTARG = tfloat, RIGHTARG = tfloat
);

/*****************************************************************************
 * Temporal multiplication
 *****************************************************************************/

/* int * <TYPE> */

CREATE FUNCTION temporal_mult(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'mult_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = *
);
	
/*****************************************************************************/
/* tint * <TYPE> */

CREATE FUNCTION temporal_mult(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'mult_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_mult(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_mult(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_mult(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = *
);

/*****************************************************************************/

/* float * <TYPE> */

CREATE FUNCTION temporal_mult(float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_mult(float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = +
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = +
);

/*****************************************************************************/
/* tfloat * <TYPE> */

CREATE FUNCTION temporal_mult(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_mult(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_mult(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'mult_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = *
);
CREATE OPERATOR * (
	PROCEDURE = temporal_mult,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = *
);

/*****************************************************************************
 * Temporal division
 *****************************************************************************/

/* int / <TYPE> */

CREATE FUNCTION temporal_div(integer, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'div_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = integer, RIGHTARG = tint
);
	
/*****************************************************************************/
/* tint / <TYPE> */

CREATE FUNCTION temporal_div(tint, integer)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'div_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(tint, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(tint, tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_div(tint, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = integer
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = float
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = tint
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tint, RIGHTARG = tfloat
);

/*****************************************************************************/

/* float / <TYPE> */

CREATE FUNCTION temporal_div(f float, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(f float, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = float, RIGHTARG = tint
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = float, RIGHTARG = tfloat
);

/*****************************************************************************/

/* tfloat / <TYPE> */

CREATE FUNCTION temporal_div(tfloat, f float)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_div(tfloat, tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_div(tfloat, tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'div_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tfloat, RIGHTARG = float
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tfloat, RIGHTARG = tint
);
CREATE OPERATOR / (
	PROCEDURE = temporal_div,
	LEFTARG = tfloat, RIGHTARG = tfloat
);

/******************************************************************************/


/* tfloat round */

CREATE FUNCTION round(tfloat, integer)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_round'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* tfloat degrees */

CREATE FUNCTION degrees(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_degrees'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
/*****************************************************************************
 *
 * BooleanOps.sql
 *	  Temporal Boolean function and operators.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal and
 *****************************************************************************/

CREATE FUNCTION temporal_and(boolean, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tand_bool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_and(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tand_tbool_bool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_and(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tand_tbool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR & (
	PROCEDURE = temporal_and,
	LEFTARG = boolean, RIGHTARG = tbool,
	COMMUTATOR = &
);
CREATE OPERATOR & (
	PROCEDURE = temporal_and,
	LEFTARG = tbool, RIGHTARG = boolean,
	COMMUTATOR = &
);
CREATE OPERATOR & (
	PROCEDURE = temporal_and,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = &
);

/*****************************************************************************
 * Temporal or
 *****************************************************************************/

CREATE FUNCTION temporal_or(boolean, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tor_bool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_or(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tor_tbool_bool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_or(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tor_tbool_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR | (
	PROCEDURE = temporal_or,
	LEFTARG = boolean, RIGHTARG = tbool,
	COMMUTATOR = |
);
CREATE OPERATOR | (
	PROCEDURE = temporal_or,
	LEFTARG = tbool, RIGHTARG = boolean,
	COMMUTATOR = |
);
CREATE OPERATOR | (
	PROCEDURE = temporal_or,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = |
);

/*****************************************************************************
 * Temporal not
 *****************************************************************************/

CREATE FUNCTION temporal_not(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tnot_tbool'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~ (
	PROCEDURE = temporal_not, RIGHTARG = tbool
);

/*****************************************************************************//*****************************************************************************
 *
 * TextFuncs.sql
 *	  Temporal text functions.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal text concatenation
 *****************************************************************************/


CREATE FUNCTION temporal_textcat(text, ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'textcat_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION temporal_textcat(ttext, text)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'textcat_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_textcat(ttext, ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'textcat_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR || (
	PROCEDURE = temporal_textcat,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = ||
);
CREATE OPERATOR || (
	PROCEDURE = temporal_textcat,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = ||
);
CREATE OPERATOR || (
	PROCEDURE = temporal_textcat,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = ||
);

/******************************************************************************
 * Temporal upper/lower case
 *****************************************************************************/

CREATE FUNCTION upper(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_upper'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_lower'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/
/*****************************************************************************
 *
 * ComparisonOps.sql
 *	  Comparison functions and operators for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
 
/*****************************************************************************
 * Temporal eq
 *****************************************************************************/

-- Temporal boolean

CREATE FUNCTION temporal_eq(boolean, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = boolean, RIGHTARG = tbool,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tbool, RIGHTARG = boolean,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal integer 

CREATE FUNCTION temporal_eq(integer, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tint, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = #=
);

/*****************************************************************************/

-- float #= <Type>

CREATE FUNCTION temporal_eq(float, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(float, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tfloat, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tfloat, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tfloat, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = #=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_eq(text, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(ttext, text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(ttext, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal ne
 *****************************************************************************/

-- Temporal boolean 

CREATE FUNCTION temporal_ne(boolean, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tbool, boolean)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tbool, tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = boolean, RIGHTARG = tbool,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tbool, RIGHTARG = boolean,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_ne(integer, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tint, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal float 

CREATE FUNCTION temporal_ne(float, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(float, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tfloat, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tfloat, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tfloat, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = #<>
);

/*****************************************************************************/

-- Temporal text 

CREATE FUNCTION temporal_ne(text, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(ttext, text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(ttext, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = #<>
);

/*****************************************************************************
 * Temporal lt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_lt(integer, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tint, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_lt(float, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(float, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tfloat, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tfloat, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(tfloat, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = #>
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_lt(text, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(ttext, text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_lt(ttext, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = #>
);
CREATE OPERATOR #< (
	PROCEDURE = temporal_lt,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = #>
);

/*****************************************************************************
 * Temporal gt
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_gt(integer, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tint, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = #<
);
	
/*****************************************************************************/

-- Temporal float 

CREATE FUNCTION temporal_gt(float, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(float, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, int)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(tfloat, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = #<
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = #<
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_gt(text, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(ttext, text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_gt(ttext, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tgt_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = #<=
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = #<=
);
CREATE OPERATOR #> (
	PROCEDURE = temporal_gt,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = #<=
);

/*****************************************************************************
 * Temporal le
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_le(integer, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tint, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_le(float, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(float, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tfloat, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tfloat, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(tfloat, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = #>=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_le(text, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(ttext, text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_le(ttext, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tle_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = #>=
);
CREATE OPERATOR #<= (
	PROCEDURE = temporal_le,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = #>=
);

/*****************************************************************************
 * Temporal ge
 *****************************************************************************/

-- Temporal integer

CREATE FUNCTION temporal_ge(integer, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, integer)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tint, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = integer, RIGHTARG = tint,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tint, RIGHTARG = integer,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tint, RIGHTARG = float,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal float

CREATE FUNCTION temporal_ge(float, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(float, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, int)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, float)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, tint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(tfloat, tfloat)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = float, RIGHTARG = tint,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = float, RIGHTARG = tfloat,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tfloat, RIGHTARG = float,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = #<=
);

/*****************************************************************************/

-- Temporal text

CREATE FUNCTION temporal_ge(text, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(ttext, text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ge(ttext, ttext)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tge_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = text, RIGHTARG = ttext,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = ttext, RIGHTARG = text,
	COMMUTATOR = #<=
);
CREATE OPERATOR #>= (
	PROCEDURE = temporal_ge,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = #<=
);

/*****************************************************************************/
/*****************************************************************************
 *
 * BoundBoxOps.sql
 *	  Bounding box operators for temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

 /*****************************************************************************
 * Casting for period
 *****************************************************************************/

CREATE FUNCTION period(tbool)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(ttext)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbool AS period) WITH FUNCTION period(tbool);
CREATE CAST (ttext AS period) WITH FUNCTION period(ttext);

 /*****************************************************************************
 * Casting for box
 *****************************************************************************/

CREATE FUNCTION box(integer)
	RETURNS box
	AS 'MODULE_PATHNAME', 'int_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float)
	RETURNS box
	AS 'MODULE_PATHNAME', 'float_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(numeric)
	RETURNS box
	AS 'MODULE_PATHNAME', 'numeric_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange)
	RETURNS box
	AS 'MODULE_PATHNAME', 'intrange_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange)
	RETURNS box
	AS 'MODULE_PATHNAME', 'floatrange_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(timestampset)
	RETURNS box
	AS 'MODULE_PATHNAME', 'timestampset_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(periodset)
	RETURNS box
	AS 'MODULE_PATHNAME', 'periodset_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tint)
	RETURNS box
	AS 'MODULE_PATHNAME', 'tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(tfloat)
	RETURNS box
	AS 'MODULE_PATHNAME', 'tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int AS box) WITH FUNCTION box(int) AS IMPLICIT;
CREATE CAST (float AS box) WITH FUNCTION box(float) AS IMPLICIT;
CREATE CAST (numeric AS box) WITH FUNCTION box(numeric) AS IMPLICIT;
CREATE CAST (timestamptz AS box) WITH FUNCTION box(timestamptz) AS IMPLICIT;
CREATE CAST (timestampset AS box) WITH FUNCTION box(timestampset) AS IMPLICIT;
CREATE CAST (period AS box) WITH FUNCTION box(period) AS IMPLICIT;
CREATE CAST (periodset AS box) WITH FUNCTION box(periodset) AS IMPLICIT;
CREATE CAST (tint AS box) WITH FUNCTION box(tint);
CREATE CAST (tfloat AS box) WITH FUNCTION box(tfloat);
-- We cannot make the castings from range to box implicit since this produces
-- an ambiguity with the implicit castings to anyrange
CREATE CAST (intrange AS box) WITH FUNCTION box(intrange);
CREATE CAST (floatrange AS box) WITH FUNCTION box(floatrange);

CREATE FUNCTION box(integer, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'int_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'intrange_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'float_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange, timestamptz)
	RETURNS box
	AS 'MODULE_PATHNAME', 'floatrange_timestamp_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(integer, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'int_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(intrange, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'intrange_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(float, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'float_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box(floatrange, period)
	RETURNS box
	AS 'MODULE_PATHNAME', 'floatrange_period_to_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectively functions for operators
 *****************************************************************************/

CREATE FUNCTION temporal_overlaps_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_overlaps_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_overlaps_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_overlaps_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_contains_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_contains_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_contains_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_contains_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_same_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_same_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_same_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_same_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************/

CREATE FUNCTION tnumber_overlaps_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_overlaps_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_overlaps_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_overlaps_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_contains_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_contains_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_contains_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_contains_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_same_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_same_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_same_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_same_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Box operators
 *****************************************************************************/

CREATE FUNCTION box_eq(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_ne(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_lt(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_le(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_ge(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_gt(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'box_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION box_cmp(box, box)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'box_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE OPERATOR = (
	LEFTARG = box, RIGHTARG = box,
	PROCEDURE = box_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = box, RIGHTARG = box,
	PROCEDURE = box_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = box_lt,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = box_le,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = box_ge,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
	PROCEDURE = box_gt,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS box_ops
	DEFAULT FOR TYPE box USING btree	AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	box_cmp(box, box);


/*****************************************************************************/

CREATE FUNCTION box_contains(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_box_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_contained(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_box_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_overlaps(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_box_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box_same(box, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_box_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = box_contains,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = box_contained,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = box_overlaps,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = box_same,
	LEFTARG = box, RIGHTARG = box,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);

/*****************************************************************************
 * Temporal boolean
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tbool, RIGHTARG = tbool,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);

/*****************************************************************************
 * Temporal integer
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = intrange, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = intrange,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = box, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = box,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tint, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);

/*****************************************************************************
 * Temporal float
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION overlaps_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = &&,
	RESTRICT = tnumber_overlaps_sel, JOIN = tnumber_overlaps_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = <@,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = @>,
	RESTRICT = tnumber_contains_sel, JOIN = tnumber_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_range_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_box_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tnumber_tnumber'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = floatrange, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = floatrange,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = box, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = box,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tint,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tfloat, RIGHTARG = tfloat,
	COMMUTATOR = ~=,
	RESTRICT = tnumber_same_sel, JOIN = tnumber_same_joinsel
);

/*****************************************************************************
 * Temporal text
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = &&,
	RESTRICT = temporal_overlaps_sel, JOIN = temporal_overlaps_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = <@,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = @>,
	RESTRICT = temporal_contains_sel, JOIN = temporal_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = period, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = period,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = ttext, RIGHTARG = ttext,
	COMMUTATOR = ~=,
	RESTRICT = temporal_same_sel, JOIN = temporal_same_joinsel
);

/*****************************************************************************/
/*****************************************************************************
 *
 * RelativePosOps.sql
 *		Relative position operators for 1D (time) and 2D (1D value + 1D time) 
 *		temporal types.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/* Selectively functions for operators */

CREATE FUNCTION temporal_position_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_position_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION temporal_position_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'temporal_position_joinsel'
	LANGUAGE C IMMUTABLE STRICT;
	
CREATE FUNCTION tnumber_position_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_position_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_position_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_position_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * period
 *****************************************************************************/
/* period op tbool */

CREATE FUNCTION temporal_before(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);

/*****************************************************************************/
/* period op ttext */

CREATE FUNCTION temporal_before(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(period, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_period_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = period, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = temporal_position_sel, JOIN = temporal_position_joinsel
);

/*****************************************************************************
 * intrange 
 *****************************************************************************/
/* intrange op tint */

CREATE FUNCTION temporal_left(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = intrange, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* intrange op tfloat */

CREATE FUNCTION temporal_left(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(intrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = intrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************
 * floatrange 
 *****************************************************************************/
/* floatrange op tint */

CREATE FUNCTION temporal_left(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatrange, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = floatrange, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* floatrange op tfloat */

CREATE FUNCTION temporal_left(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(floatrange, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_range_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = floatrange, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************
 * box 
 *****************************************************************************/
/* box op tint */

CREATE FUNCTION temporal_left(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(box, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = box, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);  

/*****************************************************************************/
/* box op tfloat */

CREATE FUNCTION temporal_left(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(box, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_box_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = box, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);  

/*****************************************************************************
 * tbool
 *****************************************************************************/
/* tbool op period */

CREATE FUNCTION temporal_before(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tbool op tbool */

CREATE FUNCTION temporal_before(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbool, tbool)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tbool, RIGHTARG = tbool,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************
 * tint
 *****************************************************************************/
/* tint op intrange */

CREATE FUNCTION temporal_left(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, intrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = intrange,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tint op box */

CREATE FUNCTION temporal_left(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = box,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tint op tint */

CREATE FUNCTION temporal_left(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tint op tfloat */

CREATE FUNCTION temporal_left(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tint, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tint, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************
 * tfloat
 *****************************************************************************/
/* tfloat op floatrange */

CREATE FUNCTION temporal_left(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, floatrange)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_range'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = floatrange,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tfloat op box */

CREATE FUNCTION temporal_left(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, box)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_box'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = box,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tfloat op tint */

CREATE FUNCTION temporal_left(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = tint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* tfloat op tfloat */

CREATE FUNCTION temporal_left(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tfloat, tfloat)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_left,
	COMMUTATOR = >>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overleft,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_right,
	COMMUTATOR = <<,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overright,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tfloat, RIGHTARG = tfloat,
	PROCEDURE = temporal_overafter,
	RESTRICT = tnumber_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************
 * ttext
 *****************************************************************************/

/* ttext op period */

CREATE FUNCTION temporal_before(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_overbefore,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = period,
	PROCEDURE = temporal_overafter,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/* ttext op ttext */

CREATE FUNCTION temporal_before(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(ttext, ttext)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_before,
	COMMUTATOR = #>>,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_overbefore,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_after,
	COMMUTATOR = <<#,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = ttext, RIGHTARG = ttext,
	PROCEDURE = temporal_overafter,
	RESTRICT = temporal_position_sel, JOIN = tnumber_position_joinsel
);

/*****************************************************************************/
/*****************************************************************************
 *
 * AggregateFuncs.sql
 *	  Temporal taggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION tagg_serialize(internal)
	RETURNS bytea
	AS 'MODULE_PATHNAME', 'temporal_tagg_serialize'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tagg_deserialize(bytea, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tagg_deserialize'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tcount_transfn(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_transfn(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tand_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tand_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tand_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_transfn(internal, tbool)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tor_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tor_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tbool_tor_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbool_tagg_finalfn(internal)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint_tagg_finalfn(internal)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcount(tbool) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tand(tbool) (
	SFUNC = tbool_tand_transfn,
	STYPE = internal,
	COMBINEFUNC = tbool_tand_combinefn,
	FINALFUNC = tbool_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tor(tbool) (
	SFUNC = tbool_tor_transfn,
	STYPE = internal,
	COMBINEFUNC = tbool_tor_combinefn,
	FINALFUNC = tbool_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION tint_tmin_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmin_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmin_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tmax_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tmax_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tsum_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_tsum_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_tsum_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_transfn(internal, tint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tavg_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tavg_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tavg_finalfn(internal)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_tavg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tmin(tint) (
	SFUNC = tint_tmin_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tmin_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax(tint) (
	SFUNC = tint_tmax_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tmax_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tsum(tint) (
	SFUNC = tint_tsum_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tint) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tavg(tint) (
	SFUNC = tavg_transfn,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn,
	FINALFUNC = tavg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

CREATE FUNCTION tfloat_tmin_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmin_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmin_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tmax_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tmax_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tsum_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tsum_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_tsum_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_tagg_finalfn(internal)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tavg_transfn(internal, tfloat)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tavg_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tmin(tfloat) (
	SFUNC = tfloat_tmin_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmin_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax(tfloat) (
	SFUNC = tfloat_tmax_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmax_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tsum(tfloat) (
	SFUNC = tfloat_tsum_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tsum_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tfloat) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tavg(tfloat) (
	SFUNC = tavg_transfn,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn,
	FINALFUNC = tavg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
 
/*****************************************************************************/

CREATE FUNCTION ttext_tmin_transfn(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmin_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmin_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_transfn(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tmax_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'ttext_tmax_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, ttext)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION ttext_tagg_finalfn(internal)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_tagg_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tmin(ttext) (
	SFUNC = ttext_tmin_transfn,
	STYPE = internal,
	COMBINEFUNC = ttext_tmin_combinefn,
	FINALFUNC = ttext_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tmax(ttext) (
	SFUNC = ttext_tmax_transfn,
	STYPE = internal,
	COMBINEFUNC = ttext_tmax_combinefn,
	FINALFUNC = ttext_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(ttext) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/
/*****************************************************************************
 *
 * WAggregateFuncs.sql
 *	  Moving window temporal aggregate functions
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION tint_wmin_transfn(internal, tint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_wmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_wmax_transfn(internal, tint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_wmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tint_wsum_transfn(internal, tint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tint_wsum_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_wcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wavg_transfn(internal, tint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_wavg_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wmin(tint, interval) (
	SFUNC = tint_wmin_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tmin_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wmax(tint, interval) (
	SFUNC = tint_wmax_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tmax_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wsum(tint, interval) (
	SFUNC = tint_wsum_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wcount(tint, interval) (
	SFUNC = wcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wavg(tint, interval) (
	SFUNC = wavg_transfn,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn,
	FINALFUNC = tavg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/

CREATE FUNCTION tfloat_wmin_transfn(internal, tfloat, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_wmin_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_wmax_transfn(internal, tfloat, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_wmax_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tfloat_wsum_transfn(internal, tfloat, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tfloat_wsum_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tfloat, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_wcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wavg_transfn(internal, tfloat, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_wavg_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wmin(tfloat, interval) (
	SFUNC = tfloat_wmin_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmin_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wmax(tfloat, interval) (
	SFUNC = tfloat_wmax_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tmax_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wsum(tfloat, interval) (
	SFUNC = tfloat_wsum_transfn,
	STYPE = internal,
	COMBINEFUNC = tfloat_tsum_combinefn,
	FINALFUNC = tfloat_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wcount(tfloat, interval) (
	SFUNC = wcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wavg(tfloat, interval) (
	SFUNC = wavg_transfn,
	STYPE = internal,
	COMBINEFUNC = tavg_combinefn,
	FINALFUNC = tavg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/
/*****************************************************************************
 *
 * IndexGistTemporal.sql
 *		R-tree GiST index for temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_tbool_consistent(internal, tbool, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_temporal_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tbool_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_temporal_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tbool_ops
	DEFAULT FOR TYPE tbool USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (tbool, period),
	OPERATOR	3		&& (tbool, tbool),	
  	-- same
	OPERATOR	6		~= (tbool, period),
	OPERATOR	6		~= (tbool, tbool),
	-- contains
	OPERATOR	7		@> (tbool, period),
	OPERATOR	7		@> (tbool, tbool),
	-- contained by
	OPERATOR	8		<@ (tbool, period),
	OPERATOR	8		<@ (tbool, tbool),
	-- overlaps or before
	OPERATOR	28		&<# (tbool, period),
	OPERATOR	28		&<# (tbool, tbool),
	-- strictly before
	OPERATOR	29		<<# (tbool, period),
	OPERATOR	29		<<# (tbool, tbool),
	-- strictly after
	OPERATOR	30		#>> (tbool, period),
	OPERATOR	30		#>> (tbool, tbool),
	-- overlaps or after
	OPERATOR	31		#&> (tbool, period),
	OPERATOR	31		#&> (tbool, tbool),
	-- functions
	FUNCTION	1	gist_tbool_consistent(internal, tbool, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_tbool_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);

/******************************************************************************/

CREATE FUNCTION gist_tint_consistent(internal, tint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tnumber_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tnumber_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tint_ops
	DEFAULT FOR TYPE tint USING gist AS
	STORAGE box,
	-- strictly left
	OPERATOR	1		<< (tint, intrange),
	OPERATOR	1		<< (tint, box),
	OPERATOR	1		<< (tint, tint),
	OPERATOR	1		<< (tint, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tint, intrange),
	OPERATOR	2		&< (tint, box),
	OPERATOR	2		&< (tint, tint),
	OPERATOR	2		&< (tint, tfloat),
	-- overlaps
	OPERATOR	3		&& (tint, intrange),
	OPERATOR	3		&& (tint, box),
	OPERATOR	3		&& (tint, tint),
	OPERATOR	3		&& (tint, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tint, intrange),
	OPERATOR	4		&> (tint, box),
	OPERATOR	4		&> (tint, tint),
	OPERATOR	4		&> (tint, tfloat),
	-- strictly right
	OPERATOR	5		>> (tint, intrange),
	OPERATOR	5		>> (tint, box),
	OPERATOR	5		>> (tint, tint),
	OPERATOR	5		>> (tint, tfloat),
  	-- same
	OPERATOR	6		~= (tint, intrange),
	OPERATOR	6		~= (tint, box),
	OPERATOR	6		~= (tint, tint),
	OPERATOR	6		~= (tint, tfloat),
	-- contains
	OPERATOR	7		@> (tint, intrange),
	OPERATOR	7		@> (tint, box),
	OPERATOR	7		@> (tint, tint),
	OPERATOR	7		@> (tint, tfloat),
	-- contained by
	OPERATOR	8		<@ (tint, intrange),
	OPERATOR	8		<@ (tint, box),
	OPERATOR	8		<@ (tint, tint),
	OPERATOR	8		<@ (tint, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tint, box),
	OPERATOR	28		&<# (tint, tint),
	OPERATOR	28		&<# (tint, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tint, box),
	OPERATOR	29		<<# (tint, tint),
	OPERATOR	29		<<# (tint, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tint, box),
	OPERATOR	30		#>> (tint, tint),
	OPERATOR	30		#>> (tint, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tint, box),
	OPERATOR	31		#&> (tint, tint),
	OPERATOR	31		#&> (tint, tfloat),
	-- functions
	FUNCTION	1	gist_tint_consistent(internal, tint, smallint, oid, internal),
	FUNCTION	2	gist_box_union(internal, internal),
	FUNCTION	3	gist_tint_compress(internal),
	FUNCTION	5	gist_box_penalty(internal, internal, internal),
	FUNCTION	6	gist_box_picksplit(internal, internal),
	FUNCTION	7	gist_box_same(box, box, internal);

/******************************************************************************/

CREATE FUNCTION gist_tfloat_consistent(internal, tfloat, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tnumber_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tfloat_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tnumber_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tfloat_ops
	DEFAULT FOR TYPE tfloat USING gist AS
	STORAGE box,
	-- strictly left
	OPERATOR	1		<< (tfloat, floatrange),
	OPERATOR	1		<< (tfloat, box),
	OPERATOR	1		<< (tfloat, tint),
	OPERATOR	1		<< (tfloat, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tfloat, floatrange),
	OPERATOR	2		&< (tfloat, box),
	OPERATOR	2		&< (tfloat, tint),
	OPERATOR	2		&< (tfloat, tfloat),
	-- overlaps
	OPERATOR	3		&& (tfloat, floatrange),
	OPERATOR	3		&& (tfloat, box),
	OPERATOR	3		&& (tfloat, tint),
	OPERATOR	3		&& (tfloat, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tfloat, floatrange),
	OPERATOR	4		&> (tfloat, box),
	OPERATOR	4		&> (tfloat, tint),
	OPERATOR	4		&> (tfloat, tfloat),
	-- strictly right
	OPERATOR	5		>> (tfloat, floatrange),
	OPERATOR	5		>> (tfloat, box),
	OPERATOR	5		>> (tfloat, tint),
	OPERATOR	5		>> (tfloat, tfloat),
  	-- same
	OPERATOR	6		~= (tfloat, floatrange),
	OPERATOR	6		~= (tfloat, box),
	OPERATOR	6		~= (tfloat, tint),
	OPERATOR	6		~= (tfloat, tfloat),
	-- contains
	OPERATOR	7		@> (tfloat, floatrange),
	OPERATOR	7		@> (tfloat, box),
	OPERATOR	7		@> (tfloat, tint),
	OPERATOR	7		@> (tfloat, tfloat),
	-- contained by
	OPERATOR	8		<@ (tfloat, floatrange),
	OPERATOR	8		<@ (tfloat, box),
	OPERATOR	8		<@ (tfloat, tint),
	OPERATOR	8		<@ (tfloat, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tfloat, box),
	OPERATOR	28		&<# (tfloat, tint),
	OPERATOR	28		&<# (tfloat, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tfloat, box),
	OPERATOR	29		<<# (tfloat, tint),
	OPERATOR	29		<<# (tfloat, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tfloat, box),
	OPERATOR	30		#>> (tfloat, tint),
	OPERATOR	30		#>> (tfloat, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tfloat, box),
	OPERATOR	31		#&> (tfloat, tint),
	OPERATOR	31		#&> (tfloat, tfloat),
	-- functions
	FUNCTION	1	gist_tfloat_consistent(internal, tfloat, smallint, oid, internal),
	FUNCTION	2	gist_box_union(internal, internal),
	FUNCTION	3	gist_tfloat_compress(internal),
	FUNCTION	5	gist_box_penalty(internal, internal, internal),
	FUNCTION	6	gist_box_picksplit(internal, internal),
	FUNCTION	7	gist_box_same(box, box, internal);

/******************************************************************************/

CREATE FUNCTION gist_ttext_consistent(internal, ttext, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_temporal_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_ttext_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_temporal_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_ttext_ops
	DEFAULT FOR TYPE ttext USING gist AS
	STORAGE period,
	-- overlaps
	OPERATOR	3		&& (ttext, period),
	OPERATOR	3		&& (ttext, ttext),	
  	-- same
	OPERATOR	6		~= (ttext, period),
	OPERATOR	6		~= (ttext, ttext),
	-- contains
	OPERATOR	7		@> (ttext, period),
	OPERATOR	7		@> (ttext, ttext),
	-- contained by
	OPERATOR	8		<@ (ttext, period),
	OPERATOR	8		<@ (ttext, ttext),
	-- overlaps or before
	OPERATOR	28		&<# (ttext, period),
	OPERATOR	28		&<# (ttext, ttext),
	-- strictly before
	OPERATOR	29		<<# (ttext, period),
	OPERATOR	29		<<# (ttext, ttext),
	-- strictly after
	OPERATOR	30		#>> (ttext, period),
	OPERATOR	30		#>> (ttext, ttext),
	-- overlaps or after
	OPERATOR	31		#&> (ttext, period),
	OPERATOR	31		#&> (ttext, ttext),
	-- functions
	FUNCTION	1	gist_ttext_consistent(internal, ttext, smallint, oid, internal),
	FUNCTION	2	gist_time_union(internal, internal),
	FUNCTION	3	gist_ttext_compress(internal),
	FUNCTION	5	gist_time_penalty(internal, internal, internal),
	FUNCTION	6	gist_time_picksplit(internal, internal),
	FUNCTION	7	gist_time_same(period, period, internal);

/******************************************************************************/
/*****************************************************************************
 *
 * IndexSPGistTemporal.sql
 *		Quad-tree SP-GiST index for temporal types
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION spgist_temporal_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_temporal_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_temporal_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************/

CREATE FUNCTION spgist_tnumber_config(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_choose(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_picksplit(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tnumber_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tbool_ops
	DEFAULT FOR TYPE tbool USING spgist AS
	-- overlaps
	OPERATOR	3		&& (tbool, period),
	OPERATOR	3		&& (tbool, tbool),	
  	-- same
	OPERATOR	6		~= (tbool, period),
	OPERATOR	6		~= (tbool, tbool),
	-- contains
	OPERATOR	7		@> (tbool, period),
	OPERATOR	7		@> (tbool, tbool),
	-- contained by
	OPERATOR	8		<@ (tbool, period),
	OPERATOR	8		<@ (tbool, tbool),
	-- overlaps or before
	OPERATOR	28		&<# (tbool, period),
	OPERATOR	28		&<# (tbool, tbool),
	-- strictly before
	OPERATOR	29		<<# (tbool, period),
	OPERATOR	29		<<# (tbool, tbool),
	-- strictly after
	OPERATOR	30		#>> (tbool, period),
	OPERATOR	30		#>> (tbool, tbool),
	-- overlaps or after
	OPERATOR	31		#&> (tbool, period),
	OPERATOR	31		#&> (tbool, tbool),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_temporal_inner_consistent(internal, internal),
	FUNCTION	5	spgist_temporal_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_temporal_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tint_ops
	DEFAULT FOR TYPE tint USING spgist AS
	-- strictly left
	OPERATOR	1		<< (tint, intrange),
	OPERATOR	1		<< (tint, box),
	OPERATOR	1		<< (tint, tint),
	OPERATOR	1		<< (tint, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tint, intrange),
	OPERATOR	2		&< (tint, box),
	OPERATOR	2		&< (tint, tint),
	OPERATOR	2		&< (tint, tfloat),
	-- overlaps
	OPERATOR	3		&& (tint, intrange),
	OPERATOR	3		&& (tint, box),
	OPERATOR	3		&& (tint, tint),
	OPERATOR	3		&& (tint, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tint, intrange),
	OPERATOR	4		&> (tint, box),
	OPERATOR	4		&> (tint, tint),
	OPERATOR	4		&> (tint, tfloat),
	-- strictly right
	OPERATOR	5		>> (tint, intrange),
	OPERATOR	5		>> (tint, box),
	OPERATOR	5		>> (tint, tint),
	OPERATOR	5		>> (tint, tfloat),
  	-- same
	OPERATOR	6		~= (tint, intrange),
	OPERATOR	6		~= (tint, box),
	OPERATOR	6		~= (tint, tint),
	OPERATOR	6		~= (tint, tfloat),
	-- contains
	OPERATOR	7		@> (tint, intrange),
	OPERATOR	7		@> (tint, box),
	OPERATOR	7		@> (tint, tint),
	OPERATOR	7		@> (tint, tfloat),
	-- contained by
	OPERATOR	8		<@ (tint, intrange),
	OPERATOR	8		<@ (tint, box),
	OPERATOR	8		<@ (tint, tint),
	OPERATOR	8		<@ (tint, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tint, box),
	OPERATOR	28		&<# (tint, tint),
	OPERATOR	28		&<# (tint, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tint, box),
	OPERATOR	29		<<# (tint, tint),
	OPERATOR	29		<<# (tint, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tint, box),
	OPERATOR	30		#>> (tint, tint),
	OPERATOR	30		#>> (tint, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tint, box),
	OPERATOR	31		#&> (tint, tint),
	OPERATOR	31		#&> (tint, tfloat),
	-- functions
	FUNCTION	1	spgist_tnumber_config(internal, internal),
	FUNCTION	2	spgist_tnumber_choose(internal, internal),
	FUNCTION	3	spgist_tnumber_picksplit(internal, internal),
	FUNCTION	4	spgist_tnumber_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tnumber_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tnumber_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tfloat_ops
	DEFAULT FOR TYPE tfloat USING spgist AS
	-- strictly left
	OPERATOR	1		<< (tfloat, floatrange),
	OPERATOR	1		<< (tfloat, box),
	OPERATOR	1		<< (tfloat, tint),
	OPERATOR	1		<< (tfloat, tfloat),
 	-- overlaps or left
	OPERATOR	2		&< (tfloat, floatrange),
	OPERATOR	2		&< (tfloat, box),
	OPERATOR	2		&< (tfloat, tint),
	OPERATOR	2		&< (tfloat, tfloat),
	-- overlaps
	OPERATOR	3		&& (tfloat, floatrange),
	OPERATOR	3		&& (tfloat, box),
	OPERATOR	3		&& (tfloat, tint),
	OPERATOR	3		&& (tfloat, tfloat),
	-- overlaps or right
	OPERATOR	4		&> (tfloat, floatrange),
	OPERATOR	4		&> (tfloat, box),
	OPERATOR	4		&> (tfloat, tint),
	OPERATOR	4		&> (tfloat, tfloat),
	-- strictly right
	OPERATOR	5		>> (tfloat, floatrange),
	OPERATOR	5		>> (tfloat, box),
	OPERATOR	5		>> (tfloat, tint),
	OPERATOR	5		>> (tfloat, tfloat),
  	-- same
	OPERATOR	6		~= (tfloat, floatrange),
	OPERATOR	6		~= (tfloat, box),
	OPERATOR	6		~= (tfloat, tint),
	OPERATOR	6		~= (tfloat, tfloat),
	-- contains
	OPERATOR	7		@> (tfloat, floatrange),
	OPERATOR	7		@> (tfloat, box),
	OPERATOR	7		@> (tfloat, tint),
	OPERATOR	7		@> (tfloat, tfloat),
	-- contained by
	OPERATOR	8		<@ (tfloat, floatrange),
	OPERATOR	8		<@ (tfloat, box),
	OPERATOR	8		<@ (tfloat, tint),
	OPERATOR	8		<@ (tfloat, tfloat),
	-- overlaps or before
	OPERATOR	28		&<# (tfloat, box),
	OPERATOR	28		&<# (tfloat, tint),
	OPERATOR	28		&<# (tfloat, tfloat),
	-- strictly before
	OPERATOR	29		<<# (tfloat, box),
	OPERATOR	29		<<# (tfloat, tint),
	OPERATOR	29		<<# (tfloat, tfloat),
	-- strictly after
	OPERATOR	30		#>> (tfloat, box),
	OPERATOR	30		#>> (tfloat, tint),
	OPERATOR	30		#>> (tfloat, tfloat),
	-- overlaps or after
	OPERATOR	31		#&> (tfloat, box),
	OPERATOR	31		#&> (tfloat, tint),
	OPERATOR	31		#&> (tfloat, tfloat),
	-- functions
	FUNCTION	1	spgist_tnumber_config(internal, internal),
	FUNCTION	2	spgist_tnumber_choose(internal, internal),
	FUNCTION	3	spgist_tnumber_picksplit(internal, internal),
	FUNCTION	4	spgist_tnumber_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tnumber_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tnumber_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_ttext_ops
	DEFAULT FOR TYPE ttext USING spgist AS
	-- overlaps
	OPERATOR	3		&& (ttext, period),
	OPERATOR	3		&& (ttext, ttext),	
  	-- same
	OPERATOR	6		~= (ttext, period),
	OPERATOR	6		~= (ttext, ttext),
	-- contains
	OPERATOR	7		@> (ttext, period),
	OPERATOR	7		@> (ttext, ttext),
	-- contained by
	OPERATOR	8		<@ (ttext, period),
	OPERATOR	8		<@ (ttext, ttext),
	-- overlaps or before
	OPERATOR	28		&<# (ttext, period),
	OPERATOR	28		&<# (ttext, ttext),
	-- strictly before
	OPERATOR	29		<<# (ttext, period),
	OPERATOR	29		<<# (ttext, ttext),
	-- strictly after
	OPERATOR	30		#>> (ttext, period),
	OPERATOR	30		#>> (ttext, ttext),
	-- overlaps or after
	OPERATOR	31		#&> (ttext, period),
	OPERATOR	31		#&> (ttext, ttext),
	-- functions
	FUNCTION	1	spgist_time_config(internal, internal),
	FUNCTION	2	spgist_time_choose(internal, internal),
	FUNCTION	3	spgist_time_picksplit(internal, internal),
	FUNCTION	4	spgist_temporal_inner_consistent(internal, internal),
	FUNCTION	5	spgist_temporal_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_temporal_compress(internal);

/******************************************************************************//*****************************************************************************
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

 CREATE FUNCTION gbox(float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gbox(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION gbox3dm(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox3dm_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodbox(float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geodbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodbox(float8, float8, float8, float8, float8, float8, float8, float8)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geodbox_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION gbox_eq(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_ne(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_lt(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_le(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_ge(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_gt(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'gbox_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
CREATE FUNCTION gbox_cmp(gbox, gbox)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'gbox_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = gbox_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = gbox_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
	PROCEDURE = gbox_lt,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR <= (
	PROCEDURE = gbox_le,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = areasel, JOIN = areajoinsel 
);
CREATE OPERATOR >= (
	PROCEDURE = gbox_ge,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
	PROCEDURE = gbox_gt,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS gbox_ops
	DEFAULT FOR TYPE gbox USING btree AS
	OPERATOR	1	< ,
	OPERATOR	2	<= ,
	OPERATOR	3	= ,
	OPERATOR	4	>= ,
	OPERATOR	5	> ,
	FUNCTION	1	gbox_cmp(gbox, gbox);

/*****************************************************************************/
/*****************************************************************************
 *
 * TemporalPointG.sql
 *	  Basic functions for temporal geography points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/
 
CREATE TYPE tgeompoint;
CREATE TYPE tgeogpoint;

SELECT register_temporal('tgeompoint', 'geometry');
SELECT register_temporal('tgeogpoint', 'geography');

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION tgeompoint_in(cstring, oid, integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tgeompoint)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_recv(internal, oid, integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tgeompoint)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION tgeompoint_typmod_in(cstring[])
	RETURNS integer
	AS 'MODULE_PATHNAME','tgeompoint_typmod_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tgeogpoint_typmod_in(cstring[])
	RETURNS integer
	AS 'MODULE_PATHNAME','tgeogpoint_typmod_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tpoint_typmod_out(integer)
	RETURNS cstring
	AS 'MODULE_PATHNAME','tpoint_typmod_out'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tpoint_analyze(internal)
	RETURNS boolean
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 

CREATE TYPE tgeompoint (
	internallength = variable,
	input = tgeompoint_in,
	output = temporal_out,
	send = temporal_send,
	receive = tgeompoint_recv,
	typmod_in = tgeompoint_typmod_in,
	typmod_out = tpoint_typmod_out,
	storage = extended,
	alignment = double,
    analyze = tpoint_analyze
);

CREATE FUNCTION tgeogpoint_in(cstring, oid, integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tgeogpoint)
	RETURNS cstring
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_recv(internal, oid, integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_recv'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tgeogpoint)
	RETURNS bytea
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tgeogpoint (
	internallength = variable,
	input = tgeogpoint_in,
	output = temporal_out,
	send = temporal_send,
	receive = tgeogpoint_recv,
	typmod_in = tgeogpoint_typmod_in,
	typmod_out = tpoint_typmod_out,
	storage = extended,
	alignment = double,
    analyze = tpoint_analyze
);

-- Special cast for enforcing the typmod restrictions
CREATE OR REPLACE FUNCTION tgeompoint(tgeompoint, integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME','tpoint_enforce_typmod'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE OR REPLACE FUNCTION tgeogpoint(tgeogpoint, integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME','tpoint_enforce_typmod'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS tgeompoint) WITH FUNCTION tgeompoint(tgeompoint, integer) AS IMPLICIT;
CREATE CAST (tgeogpoint AS tgeogpoint) WITH FUNCTION tgeogpoint(tgeogpoint, integer) AS IMPLICIT;


/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION tgeompointinst(geometry(Point), timestamptz)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointi(tgeompoint[])
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompointseq(tgeompoint[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeompoints(tgeompoint[])
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE FUNCTION tgeogpointinst(geography(Point), timestamptz)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeogpointi(tgeogpoint[])
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeogpointseq(tgeogpoint[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeogpoints(tgeogpoint[])
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Transformations
 ******************************************************************************/

CREATE FUNCTION tgeompointinst(tgeompoint)
	RETURNS tgeompoint AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompointi(tgeompoint)
	RETURNS tgeompoint AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompointseq(tgeompoint)
	RETURNS tgeompoint AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoints(tgeompoint)
	RETURNS tgeompoint AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tgeogpointinst(tgeogpoint)
	RETURNS tgeogpoint AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointi(tgeogpoint)
	RETURNS tgeogpoint AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpointseq(tgeogpoint)
	RETURNS tgeogpoint AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoints(tgeogpoint)
	RETURNS tgeogpoint AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Append function
 ******************************************************************************/

CREATE FUNCTION appendInstant(tgeompoint, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_append_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION appendInstant(tgeogpoint, tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_append_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION temporalType(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporalType(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(tgeompoint)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(tgeogpoint)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- value is a reserved word in SQL
CREATE FUNCTION getValue(tgeompoint)
	RETURNS geometry(Point)
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValue(tgeogpoint)
	RETURNS geography(Point)
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tpoint_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'tpoint_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(tgeompoint)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTime(tgeogpoint)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getTimestamp(tgeompoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getTimestamp(tgeogpoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ever_equals(tgeompoint, geometry(Point))
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tpoint_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_equals(tgeogpoint, geography(Point))
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tpoint_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &= (
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	PROCEDURE = ever_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR &= (
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	PROCEDURE = ever_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_equals(tgeompoint, geometry(Point))
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tpoint_always_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_equals(tgeogpoint, geography(Point))
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'tpoint_always_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	PROCEDURE = always_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR @= (
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	PROCEDURE = always_equals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION shift(tgeompoint, interval)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(tgeogpoint, interval)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(tgeompoint)
	RETURNS geometry(Point)
	AS 'MODULE_PATHNAME', 'temporal_start_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(tgeogpoint)
	RETURNS geography(Point)
	AS 'MODULE_PATHNAME', 'temporal_start_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(tgeompoint)
	RETURNS geometry(Point)
	AS 'MODULE_PATHNAME', 'temporal_end_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(tgeogpoint)
	RETURNS geography(Point)
	AS 'MODULE_PATHNAME', 'temporal_end_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(tgeompoint)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(tgeogpoint)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tgeompoint)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(tgeogpoint)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numInstants(tgeompoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numInstants(tgeogpoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startInstant(tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_start_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startInstant(tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_start_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endInstant(tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_end_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endInstant(tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_end_instant'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instantN(tgeompoint, integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_instant_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instantN(tgeogpoint, integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_instant_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION instants(tgeompoint)
	RETURNS tgeompoint[]
	AS 'MODULE_PATHNAME', 'temporal_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION instants(tgeogpoint)
	RETURNS tgeogpoint[]
	AS 'MODULE_PATHNAME', 'temporal_instants'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tgeompoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numTimestamps(tgeogpoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(tgeompoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startTimestamp(tgeogpoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_start_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(tgeompoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endTimestamp(tgeogpoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_end_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(tgeompoint, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampN(tgeogpoint, integer)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporal_timestamp_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(tgeompoint)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'temporal_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestamps(tgeogpoint)
	RETURNS timestamptz[]
	AS 'MODULE_PATHNAME', 'temporal_timestamps'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSequences(tgeompoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSequences(tgeogpoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_num_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSequence(tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_start_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSequence(tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_start_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSequence(tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_end_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSequence(tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_end_sequence'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequenceN(tgeompoint, integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_sequence_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequenceN(tgeogpoint, integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_sequence_n'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION sequences(tgeompoint)
	RETURNS tgeompoint[]
	AS 'MODULE_PATHNAME', 'temporal_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION sequences(tgeogpoint)
	RETURNS tgeogpoint[]
	AS 'MODULE_PATHNAME', 'temporal_sequences'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atValue(tgeompoint, geometry(Point))
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValue(tgeogpoint, geography(Point))
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValue(tgeompoint, geometry(Point))
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValue(tgeogpoint, geography(Point))
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atValues(tgeompoint, geometry(Point)[])
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atValues(tgeogpoint, geography(Point)[])
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusValues(tgeompoint, geometry(Point)[])
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusValues(tgeogpoint, geography(Point)[])
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTimestamp(tgeompoint, timestamptz)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestamp(tgeogpoint, timestamptz)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTimestamp(tgeompoint, timestamptz)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestamp(tgeogpoint, timestamptz)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueAtTimestamp(tgeompoint, timestamptz)
	RETURNS geometry(Point)
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueAtTimestamp(tgeogpoint, timestamptz)
	RETURNS geography(Point)
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atTimestampSet(tgeompoint, timestampset)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atTimestampSet(tgeogpoint, timestampset)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusTimestampSet(tgeompoint, timestampset)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusTimestampSet(tgeogpoint, timestampset)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atPeriod(tgeompoint, period)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriod(tgeogpoint, period)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusPeriod(tgeompoint, period)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_minus_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriod(tgeogpoint, period)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atPeriodSet(tgeompoint, periodset)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION atPeriodSet(tgeogpoint, periodset)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusPeriodSet(tgeompoint, periodset)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'temporal_minus_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION minusPeriodSet(tgeogpoint, periodset)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsTimestamp(tgeompoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestamp(tgeogpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION intersectsTimestampSet(tgeompoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsTimestampSet(tgeogpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsPeriod(tgeompoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriod(tgeogpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsPeriodSet(tgeompoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersectsPeriodSet(tgeogpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION tgeompoint_lt(tgeompoint, tgeompoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_le(tgeompoint, tgeompoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_eq(tgeompoint, tgeompoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_ne(tgeompoint, tgeompoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_ge(tgeompoint, tgeompoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_gt(tgeompoint, tgeompoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint_cmp(tgeompoint, tgeompoint)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = tgeompoint_lt,
	COMMUTATOR = >,
	NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = tgeompoint_le,
	COMMUTATOR = >=,
	NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = tgeompoint_eq,
	COMMUTATOR = =,
	NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = tgeompoint_ne,
	COMMUTATOR = <>,
	NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = tgeompoint_ge,
	COMMUTATOR = <=,
	NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = tgeompoint_gt,
	COMMUTATOR = <,
	NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	tgeompoint_cmp(tgeompoint, tgeompoint);

/******************************************************************************/

CREATE FUNCTION tgeogpoint_lt(tgeogpoint, tgeogpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_le(tgeogpoint, tgeogpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_eq(tgeogpoint, tgeogpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_ne(tgeogpoint, tgeogpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_ge(tgeogpoint, tgeogpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_gt(tgeogpoint, tgeogpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_cmp(tgeogpoint, tgeogpoint)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = tgeogpoint_lt,
	COMMUTATOR = >,	NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = tgeogpoint_le,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = tgeogpoint_eq,
	COMMUTATOR = =, NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = tgeogpoint_ne,
	COMMUTATOR = <>, NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = tgeogpoint_ge,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = tgeogpoint_gt,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	tgeogpoint_cmp(tgeogpoint, tgeogpoint);

/******************************************************************************/

CREATE FUNCTION tgeompoint_hash(tgeompoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeogpoint_hash(tgeogpoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'temporal_hash'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS hash_tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   tgeompoint_hash(tgeompoint);
CREATE OPERATOR CLASS hash_tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   tgeogpoint_hash(tgeogpoint);

/******************************************************************************/

/*****************************************************************************
 *
 * ComparisonOps.sql
 *	  Comparison functions and operators for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * Temporal equal for geometry
 *****************************************************************************/

CREATE FUNCTION temporal_eq(geometry(Point), tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeompoint, geometry(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal equal for geography
 *****************************************************************************/

CREATE FUNCTION temporal_eq(geography(Point), tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeogpoint, geography(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_eq(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'teq_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	COMMUTATOR = #=
);
CREATE OPERATOR #= (
	PROCEDURE = temporal_eq,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = #=
);

/*****************************************************************************
 * Temporal not equal for geometry
 *****************************************************************************/

CREATE FUNCTION temporal_ne(geometry(Point), tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeompoint, geometry(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = geometry(Point), RIGHTARG = tgeompoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeompoint, RIGHTARG = geometry(Point),
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = #<>
);

/*****************************************************************************
 * Temporal not equal for geography
 *****************************************************************************/

CREATE FUNCTION temporal_ne(geography(Point), tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_base_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeogpoint, geography(Point))
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_base'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_ne(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tne_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = geography(Point), RIGHTARG = tgeogpoint,
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeogpoint, RIGHTARG = geography(Point),
	COMMUTATOR = #<>
);
CREATE OPERATOR #<> (
	PROCEDURE = temporal_ne,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = #<>
);

/******************************************************************************/
/*****************************************************************************
 *
 * TemporalGeo.sql
 *	  Geometric functions for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION asText(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeompoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tgeogpoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION asText(geometry[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION asText(geography[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_astext'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	

CREATE FUNCTION asEWKT(tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeompoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(tgeogpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'tpoint_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(tgeogpoint[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'tpointarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	

CREATE FUNCTION asEWKT(geometry[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION asEWKT(geography[])
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'geoarr_asewkt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION SRID(tgeompoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'tpoint_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION SRID(tgeogpoint)
	RETURNS integer
	AS 'MODULE_PATHNAME', 'tpoint_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setSRID(tgeompoint, srid integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_set_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setSRID(tgeogpoint, srid integer)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_set_srid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(tgeompoint, srid integer)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_transform'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

----- Gauss Kruger transformation
CREATE FUNCTION transform_gk(tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tgeompoint_transform_gk'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform_gk(geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'geometry_transform_gk'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-------

CREATE FUNCTION tgeogpoint(tgeompoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tgeompoint_as_tgeogpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tgeompoint(tgeogpoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tgeogpoint_as_tgeompoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS tgeogpoint) WITH FUNCTION tgeogpoint(tgeompoint);
CREATE CAST (tgeogpoint AS tgeompoint) WITH FUNCTION tgeompoint(tgeogpoint);

CREATE OR REPLACE FUNCTION setprecision(tgeompoint, int)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_setprecision'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OR REPLACE FUNCTION setprecision(tgeogpoint, int)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'tpoint_setprecision'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION trajectory(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tgeompoint_trajectory'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION trajectory(tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'tgeogpoint_trajectory'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION length(tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION length(tgeogpoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION cumulativeLength(tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cumulativeLength(tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_cumulative_length'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION speed(tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_speed'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION speed(tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_speed'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION twcentroid(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tgeompoint_twcentroid'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION azimuth(tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_azimuth'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION azimuth(tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tpoint_azimuth'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION atGeometry(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_at_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION minusGeometry(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_minus_geometry'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION NearestApproachInstant(geometry, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeompoint, tgeompoint)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION NearestApproachInstant(geography, tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'NAI_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeogpoint, geography)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION NearestApproachInstant(tgeogpoint, tgeogpoint)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'NAI_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nearestApproachDistance(geometry, tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, geometry)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeompoint, tgeompoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

	CREATE FUNCTION nearestApproachDistance(geography, tgeogpoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, geography)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nearestApproachDistance(tgeogpoint, tgeogpoint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'NAD_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR |=| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);

CREATE OPERATOR |=| (
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);
CREATE OPERATOR |=| (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = nearestApproachDistance,
	COMMUTATOR = '|=|'
);

CREATE FUNCTION shortestLine(geometry, tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, geometry)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeompoint, tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shortestLine(geography, tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'shortestline_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, geography)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shortestLine(tgeogpoint, tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'shortestline_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION geometry(tgeompoint)
	RETURNS geometry
	AS 'MODULE_PATHNAME', 'tpoint_to_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeompoint AS geometry) WITH FUNCTION geometry(tgeompoint);

CREATE FUNCTION geography(tgeogpoint)
	RETURNS geography
	AS 'MODULE_PATHNAME', 'tpoint_to_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tgeogpoint AS geography) WITH FUNCTION geography(tgeogpoint);

CREATE FUNCTION tgeompoint(geometry)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'geo_to_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS tgeompoint) WITH FUNCTION tgeompoint(geometry);

CREATE FUNCTION tgeogpoint(geography)
	RETURNS tgeogpoint
	AS 'MODULE_PATHNAME', 'geo_to_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geography AS tgeogpoint) WITH FUNCTION tgeogpoint(geography);

/*****************************************************************************/
/*****************************************************************************
 *
 * BoundBoxOps.sql
 *	  Bounding box operators for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gbox(geometry)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(timestampset)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'timestampset_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(periodset)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'periodset_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geometry, timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography, timestamptz)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_timestamp_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geometry, period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(geography, period)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'geo_period_to_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(tgeompoint)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox(tgeogpoint)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS gbox) WITH FUNCTION gbox(geometry) AS IMPLICIT;
CREATE CAST (geography AS gbox) WITH FUNCTION gbox(geography) AS IMPLICIT;
CREATE CAST (timestamptz AS gbox) WITH FUNCTION gbox(timestamptz) AS IMPLICIT;
CREATE CAST (timestampset AS gbox) WITH FUNCTION gbox(timestampset) AS IMPLICIT;
CREATE CAST (period AS gbox) WITH FUNCTION gbox(period) AS IMPLICIT;
CREATE CAST (periodset AS gbox) WITH FUNCTION gbox(periodset) AS IMPLICIT;
CREATE CAST (tgeompoint AS gbox) WITH FUNCTION gbox(tgeompoint);
CREATE CAST (tgeogpoint AS gbox) WITH FUNCTION gbox(tgeogpoint);

/*****************************************************************************/

CREATE FUNCTION expandSpatial(gbox, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandSpatial(tgeompoint, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandSpatial(tgeogpoint, float)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_spatial'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandTemporal(gbox, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gbox_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTemporal(tgeompoint, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTemporal(tgeogpoint, interval)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'tpoint_expand_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tpoint_overlaps_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_overlaps_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_overlaps_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_overlaps_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_contains_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_contains_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_contains_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_contains_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_same_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_same_sel'
	LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_same_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_same_joinsel'
	LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************/

CREATE FUNCTION gbox_contains(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_contained(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_overlaps(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gbox_same(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = gbox_contains,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = gbox_contained,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = gbox_overlaps,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = gbox_same,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);

/*****************************************************************************
 * Contains
 *****************************************************************************/

CREATE FUNCTION contains_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contains_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR @> (
	PROCEDURE = contains_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = <@,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);

/*****************************************************************************
 * Contained
 *****************************************************************************/

CREATE FUNCTION contained_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);

/*****************************************************************************/

CREATE FUNCTION contained_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contained_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);
CREATE OPERATOR <@ (
	PROCEDURE = contained_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = @>,
	RESTRICT = tpoint_contains_sel, JOIN = tpoint_contains_joinsel
);

/*****************************************************************************
 * Overlaps
 *****************************************************************************/

CREATE FUNCTION overlaps_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);

/*****************************************************************************/

CREATE FUNCTION overlaps_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);
CREATE OPERATOR && (
	PROCEDURE = overlaps_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = &&,
	RESTRICT = tpoint_overlaps_sel, JOIN = tpoint_overlaps_joinsel
);

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION same_bbox(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);

/*****************************************************************************/

CREATE FUNCTION same_bbox(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION same_bbox(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'same_bbox_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);
CREATE OPERATOR ~= (
	PROCEDURE = same_bbox,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = ~=,
	RESTRICT = tpoint_same_sel, JOIN = tpoint_same_joinsel
);

/*****************************************************************************/
/*****************************************************************************
 *
 * RelativePosOps.sql
 *	  Relative position operators for 4D (2D/3D spatial value + 1D time)
 *	  temporal points
 *
 * Temporal geometric points have associated operators for the spatial and
 * temporal dimensions while temporal geographic points only for the temporal
 * dimension.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION tpoint_position_sel(internal, oid, internal, integer)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_position_sel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tpoint_position_joinsel(internal, oid, internal, smallint, internal)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tpoint_position_joinsel'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Geometry
 *****************************************************************************/

CREATE FUNCTION temporal_left(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_geom_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************
 * gbox
 *****************************************************************************/

CREATE FUNCTION temporal_left(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(gbox, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_gbox_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	PROCEDURE = temporal_left,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	PROCEDURE = temporal_overleft,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	PROCEDURE = temporal_overright,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	PROCEDURE = temporal_below,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	PROCEDURE = temporal_overbelow,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	PROCEDURE = temporal_above,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	PROCEDURE = temporal_overabove,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = gbox, RIGHTARG = gbox,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	PROCEDURE = temporal_before,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	PROCEDURE = temporal_overbefore,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	PROCEDURE = temporal_after,
	LEFTARG = gbox, RIGHTARG = gbox,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	PROCEDURE = temporal_overafter,
	LEFTARG = gbox, RIGHTARG = gbox,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/******************************************************************************/

CREATE FUNCTION temporal_left(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/

CREATE FUNCTION temporal_before(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(gbox, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_gbox_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = gbox, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************
 * tgeompoint
 *****************************************************************************/

 /* tgeompoint op geometry */

CREATE FUNCTION temporal_left(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_geom'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/

/* tgeompoint op gbox */

CREATE FUNCTION temporal_left(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeompoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/

/* tgeompoint op tgeompoint */

CREATE FUNCTION temporal_left(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'left_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overleft_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'right_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overright_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'below_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbelow_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'above_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overabove_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'front_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overfront_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'back_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overback_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(inst1 tgeompoint, inst2 tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_left,
	COMMUTATOR = '>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &< (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overleft,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR >> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_right,
	COMMUTATOR = '<<',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overright,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_below,
	COMMUTATOR = '|>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<| (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbelow,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_above,
	COMMUTATOR = '<<|',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR |&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overabove,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_front,
	COMMUTATOR = '/>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &</ (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overfront,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR />> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_back,
	COMMUTATOR = '<</',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR /&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overback,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR <<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************
 * tgeogpoint
 *****************************************************************************/

/* tgeogpoint op gbox */

CREATE FUNCTION temporal_before(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, gbox)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_tpoint_gbox'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = gbox,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/

/* tgeogpoint op tgeogpoint */

CREATE FUNCTION temporal_before(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'before_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overbefore_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'after_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overafter_temporal_temporal'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_before,
	COMMUTATOR = '#>>',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR &<# (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overbefore,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #>> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_after,
	COMMUTATOR = '<<#',
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);
CREATE OPERATOR #&> (
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	PROCEDURE = temporal_overafter,
	RESTRICT = tpoint_position_sel, JOIN = tpoint_position_joinsel
);

/*****************************************************************************/
/*****************************************************************************
 *
 * TempDistance.sql
 *	  Temporal distance for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * temporal geometry points
 *****************************************************************************/

/* geometry <-> <TemporalType> */

CREATE FUNCTION distance(geometry, tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, geometry)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeompoint, tgeompoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = geometry, RIGHTARG = tgeompoint,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeompoint, RIGHTARG = geometry,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeompoint, RIGHTARG = tgeompoint,
	COMMUTATOR = <->
);

/*****************************************************************************
 * temporal geography points
 *****************************************************************************/

/* geography <-> <TemporalType> */

CREATE FUNCTION distance(geography, tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeogpoint, geography)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION distance(tgeogpoint, tgeogpoint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'distance_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;		

CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = geography, RIGHTARG = tgeogpoint,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeogpoint, RIGHTARG = geography,
	COMMUTATOR = <->
);
CREATE OPERATOR <-> (
	PROCEDURE = distance,
	LEFTARG = tgeogpoint, RIGHTARG = tgeogpoint,
	COMMUTATOR = <->
);

/******************************************************************************/
/*****************************************************************************
 *
 * GeomAggFuncs.sql
 *	  Aggregate functions for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse,
 *	  Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION tcount_transfn(internal, tgeompoint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcount_transfn(internal, tgeogpoint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_tcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE tcount(tgeompoint) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE tcount(tgeogpoint) (
	SFUNC = tcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tcount_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

CREATE FUNCTION wcount_transfn(internal, tgeompoint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_wcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION wcount_transfn(internal, tgeogpoint, interval)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'temporal_wcount_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE wcount(tgeompoint, interval) (
	SFUNC = wcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
CREATE AGGREGATE wcount(tgeogpoint, interval) (
	SFUNC = wcount_transfn,
	STYPE = internal,
	COMBINEFUNC = tint_tsum_combinefn,
	FINALFUNC = tint_tagg_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);
	
CREATE FUNCTION tcentroid_transfn(internal, tgeompoint)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tpoint_tcentroid_transfn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcentroid_combinefn(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'tpoint_tcentroid_combinefn'
	LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tcentroid_finalfn(internal)
	RETURNS tgeompoint
	AS 'MODULE_PATHNAME', 'tpoint_tcentroid_finalfn'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE AGGREGATE tcentroid(tgeompoint) (
	SFUNC = tcentroid_transfn,
	STYPE = internal,
	COMBINEFUNC = tcentroid_combinefn,
	FINALFUNC = tcentroid_finalfn,
	SERIALFUNC = tagg_serialize,
	DESERIALFUNC = tagg_deserialize,
	PARALLEL = SAFE
);

/*****************************************************************************/
/*****************************************************************************
 *
 * SpatialRels.sql
 *	  Spatial relationships for temporal points.
 *
 * These relationships are generalized to the temporal dimension with the 
 * "at any instant" semantics, that is, the traditional operator is applied to
 * the union of all values taken by the temporal point and returns a Boolean.
 * The following relationships are supported for temporal geometry points:
 *		contains, containsproperly, covers, coveredby, crosses, disjoint, 
 *		equals, intersects, overlaps, touches, within, dwithin, and
 *		relate (with 2 and 3 arguments)
 * The following relationships are supported for temporal geography points:
 *		covers, coveredby, intersects, dwithin
 * All these relationships, excepted disjoint and relate, will automatically 
 * include a bounding box comparison that will make use of any spatial, 
 * temporal, or spatiotemporal indexes that are available.
 * N.B. In the current version of Postgis (2.4) the only index operator 
 * implemented for geography is &&
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * contains
 *****************************************************************************/

CREATE FUNCTION _contains(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _contains(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._contains($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION contains(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'contains_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * containsproperly
 *****************************************************************************/

CREATE FUNCTION _containsproperly(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'containsproperly_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _containsproperly(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'containsproperly_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION containsproperly(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._containsproperly($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION containsproperly(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'containsproperly_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
		
/*****************************************************************************
 * covers
 *****************************************************************************/

CREATE FUNCTION _covers(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _covers(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION covers(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _covers(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(geography, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _covers(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION covers(tgeogpoint, geography)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.@>) $2 AND @extschema@._covers($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION covers(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'covers_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
			
/*****************************************************************************
 * coveredby
 *****************************************************************************/

CREATE FUNCTION _coveredby(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _coveredby(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION coveredby(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _coveredby(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(geography, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _coveredby(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION coveredby(tgeogpoint, geography)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._coveredby($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION coveredby(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'coveredby_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
		
/*****************************************************************************
 * crosses
 *****************************************************************************/

CREATE FUNCTION _crosses(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'crosses_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _crosses(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'crosses_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION crosses(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._crosses($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION crosses(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'crosses_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * disjoint
 *****************************************************************************/

CREATE FUNCTION disjoint(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'disjoint_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'disjoint_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION disjoint(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'disjoint_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * equals
 *****************************************************************************/

CREATE FUNCTION _equals(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'equals_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _equals(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'equals_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION equals(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.~=) $2 AND @extschema@._equals($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION equals(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'equals_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * intersects
 *****************************************************************************/

CREATE FUNCTION _intersects(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION intersects(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _intersects(geography, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(geography, tgeogpoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _intersects(tgeogpoint, geography)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intersects(tgeogpoint, geography)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._intersects($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION intersects(tgeogpoint, tgeogpoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'intersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * overlaps
 *****************************************************************************/

CREATE FUNCTION _overlaps(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _overlaps(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._overlaps($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION overlaps(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'overlaps_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * touches
 *****************************************************************************/

CREATE FUNCTION _touches(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'touches_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _touches(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'touches_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION touches(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) $2 AND @extschema@._touches($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION touches(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'touches_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * within
 *****************************************************************************/

CREATE FUNCTION _within(geometry, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'within_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(geometry, tgeompoint)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _within(tgeompoint, geometry)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'within_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION within(tgeompoint, geometry)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.<@) $2 AND @extschema@._within($1,$2)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION within(tgeompoint, tgeompoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'within_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * dwithin
 *****************************************************************************/

CREATE FUNCTION _dwithin(geometry, tgeompoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geometry, tgeompoint, dist float8)
	RETURNS boolean
	AS 'SELECT @extschema@.ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _dwithin(tgeompoint, geometry, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeompoint, geometry, dist float8)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@.ST_Expand($2,$3) 
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION dwithin(tgeompoint, tgeompoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************/

CREATE FUNCTION _dwithin(geography, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(geography, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'SELECT @extschema@._ST_Expand($1,$3) OPERATOR(@extschema@.&&) $2
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;

CREATE FUNCTION _dwithin(tgeogpoint, geography, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION dwithin(tgeogpoint, geography, dist float8)
	RETURNS boolean
	AS 'SELECT $1 OPERATOR(@extschema@.&&) @extschema@._ST_Expand($2,$3) 
	AND @extschema@._dwithin($1, $2, $3)'
	LANGUAGE 'sql' IMMUTABLE PARALLEL SAFE;
	
CREATE FUNCTION dwithin(tgeogpoint, tgeogpoint, dist float8)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'dwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/*****************************************************************************
 * relate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION relate(geometry, tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'relate_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, geometry)
	RETURNS text
	AS 'MODULE_PATHNAME', 'relate_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, tgeompoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'relate_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * relate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION relate(geometry, tgeompoint, pattern text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'relate_pattern_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, geometry, pattern text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'relate_pattern_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION relate(tgeompoint, tgeompoint, pattern text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'relate_pattern_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
/*****************************************************************************
 *
 * TempSpatialRels.sql
 *	  Spatial relationships for temporal points.
 *
 * These relationships are applied at each instant and result in a temporal 
 * Boolean. 
 * The following relationships are supported for temporal geometry points:
 *		tcontains, tcovers, tcoveredby, tdisjoint, equals, tintersects, 
 *		ttouches, twithin, tdwithin, and trelate (with 2 and 3 arguments)
 * The following relationships are supported for temporal geography points:
 *		tcovers, tcoveredby, tintersects, tdwithin
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

/*****************************************************************************
 * tcontains
 *****************************************************************************/

CREATE FUNCTION tcontains(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcontains(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcontains(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcontains_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcovers
 *****************************************************************************/

CREATE FUNCTION tcovers(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tcovers(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcovers(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcovers_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tcoveredby
 *****************************************************************************/

CREATE FUNCTION tcoveredby(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tcoveredby(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tcoveredby(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tcoveredby_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdisjoint
 *****************************************************************************/

CREATE FUNCTION tdisjoint(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdisjoint(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdisjoint_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tequals
 *****************************************************************************/

CREATE FUNCTION tequals(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tequals(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tequals_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tintersects
 *****************************************************************************/

CREATE FUNCTION tintersects(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tintersects(geography, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, geography)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintersects(tgeogpoint, tgeogpoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tintersects_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * ttouches
 *****************************************************************************/

CREATE FUNCTION ttouches(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttouches(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'ttouches_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * twithin
 *****************************************************************************/

CREATE FUNCTION twithin(geometry, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twithin(tgeompoint, geometry)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twithin(tgeompoint, tgeompoint)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'twithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * tdwithin
 *****************************************************************************/

CREATE FUNCTION tdwithin(geometry, tgeompoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, geometry, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeompoint, tgeompoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION tdwithin(geography, tgeogpoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tdwithin(tgeogpoint, geography, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;	
CREATE FUNCTION tdwithin(tgeogpoint, tgeogpoint, dist float8)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tdwithin_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * trelate (2 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tgeompoint)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, geometry)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, tgeompoint)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'trelate_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * trelate (3 arguments)
 *****************************************************************************/

CREATE FUNCTION trelate(geometry, tgeompoint, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_geo_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, geometry, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_tpoint_geo'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION trelate(tgeompoint, tgeompoint, pattern text)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'trelate_pattern_tpoint_tpoint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/
/*****************************************************************************
 *
 * IndexGistTemporalPoint.c
 *	  R-tree GiST index for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tpoint_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'gist_tpoint_consistent'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_union(internal, internal)
	RETURNS gbox
	AS 'MODULE_PATHNAME', 'gist_tpoint_union'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_compress'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_penalty(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_penalty'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_picksplit(internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_picksplit'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gist_tpoint_same(gbox, gbox, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME', 'gist_tpoint_same'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS gist_tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING gist AS
	STORAGE gbox,
	-- strictly left
	OPERATOR	1		<< (tgeompoint, geometry),  
	OPERATOR	1		<< (tgeompoint, gbox),  
	OPERATOR	1		<< (tgeompoint, tgeompoint),  
	-- overlaps or left
	OPERATOR	2		&< (tgeompoint, geometry),  
	OPERATOR	2		&< (tgeompoint, gbox),  
	OPERATOR	2		&< (tgeompoint, tgeompoint),  
	-- overlaps	
	OPERATOR	3		&& (tgeompoint, geometry),  
	OPERATOR	3		&& (tgeompoint, gbox),  
	OPERATOR	3		&& (tgeompoint, tgeompoint),  
	-- overlaps or right
	OPERATOR	4		&> (tgeompoint, geometry),  
	OPERATOR	4		&> (tgeompoint, gbox),  
	OPERATOR	4		&> (tgeompoint, tgeompoint),  
  	-- strictly right
	OPERATOR	5		>> (tgeompoint, geometry),  
	OPERATOR	5		>> (tgeompoint, gbox),  
	OPERATOR	5		>> (tgeompoint, tgeompoint),  
  	-- same
	OPERATOR	6		~= (tgeompoint, geometry),  
	OPERATOR	6		~= (tgeompoint, gbox),  
	OPERATOR	6		~= (tgeompoint, tgeompoint),  
	-- contains
	OPERATOR	7		@> (tgeompoint, geometry),  
	OPERATOR	7		@> (tgeompoint, gbox),  
	OPERATOR	7		@> (tgeompoint, tgeompoint),  
	-- contained by
	OPERATOR	8		<@ (tgeompoint, geometry),  
	OPERATOR	8		<@ (tgeompoint, gbox),  
	OPERATOR	8		<@ (tgeompoint, tgeompoint),  
	-- overlaps or below
	OPERATOR	9		&<| (tgeompoint, geometry),  
	OPERATOR	9		&<| (tgeompoint, gbox),  
	OPERATOR	9		&<| (tgeompoint, tgeompoint),  
	-- strictly below
	OPERATOR	10		<<| (tgeompoint, geometry),  
	OPERATOR	10		<<| (tgeompoint, gbox),  
	OPERATOR	10		<<| (tgeompoint, tgeompoint),  
	-- strictly above
	OPERATOR	11		|>> (tgeompoint, geometry),  
	OPERATOR	11		|>> (tgeompoint, gbox),  
	OPERATOR	11		|>> (tgeompoint, tgeompoint),  
	-- overlaps or above
	OPERATOR	12		|&> (tgeompoint, geometry),  
	OPERATOR	12		|&> (tgeompoint, gbox),  
	OPERATOR	12		|&> (tgeompoint, tgeompoint),  
	-- nearest approach distance
	OPERATOR	25		|=| (tgeompoint, geometry) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		|=| (tgeompoint, gbox) FOR ORDER BY pg_catalog.float_ops,
	OPERATOR	25		|=| (tgeompoint, tgeompoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeompoint, gbox),
	OPERATOR	28		&<# (tgeompoint, tgeompoint),
	-- strictly before
	OPERATOR	29		<<# (tgeompoint, gbox),
	OPERATOR	29		<<# (tgeompoint, tgeompoint),
	-- strictly after
	OPERATOR	30		#>> (tgeompoint, gbox),
	OPERATOR	30		#>> (tgeompoint, tgeompoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeompoint, gbox),
	OPERATOR	31		#&> (tgeompoint, tgeompoint),
	-- overlaps or front
	OPERATOR	32		&</ (tgeompoint, gbox),
	OPERATOR	32		&</ (tgeompoint, tgeompoint),
	-- strictly front
	OPERATOR	33		<</ (tgeompoint, gbox),
	OPERATOR	33		<</ (tgeompoint, tgeompoint),
	-- strictly back
	OPERATOR	34		/>> (tgeompoint, gbox),
	OPERATOR	34		/>> (tgeompoint, tgeompoint),
	-- overlaps or back
	OPERATOR	35		/&> (tgeompoint, gbox),
	OPERATOR	35		/&> (tgeompoint, tgeompoint),
	-- functions
	FUNCTION	1	gist_tgeompoint_consistent(internal, tgeompoint, smallint, oid, internal),
	FUNCTION	2	gist_tpoint_union(internal, internal),
	FUNCTION	3	gist_tpoint_compress(internal),
	FUNCTION	5	gist_tpoint_penalty(internal, internal, internal),
	FUNCTION	6	gist_tpoint_picksplit(internal, internal),
	FUNCTION	7	gist_tpoint_same(gbox, gbox, internal);
	
CREATE OPERATOR CLASS gist_tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING gist AS
	STORAGE gbox,
	-- overlaps
	OPERATOR	3		&& (tgeogpoint, geography),  
	OPERATOR	3		&& (tgeogpoint, gbox),  
	OPERATOR	3		&& (tgeogpoint, tgeogpoint),  
  	-- same
	OPERATOR	6		~= (tgeogpoint, geography),  
	OPERATOR	6		~= (tgeogpoint, gbox),  
	OPERATOR	6		~= (tgeogpoint, tgeogpoint),  
	-- contains
	OPERATOR	7		@> (tgeogpoint, geography),  
	OPERATOR	7		@> (tgeogpoint, gbox),  
	OPERATOR	7		@> (tgeogpoint, tgeogpoint),  
	-- contained by
	OPERATOR	8		<@ (tgeogpoint, geography),  
	OPERATOR	8		<@ (tgeogpoint, gbox),  
	OPERATOR	8		<@ (tgeogpoint, tgeogpoint),  
	-- distance
--	OPERATOR	25		<-> (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, gbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeogpoint, gbox),
	OPERATOR	28		&<# (tgeogpoint, tgeogpoint),
	-- strictly before
	OPERATOR	29		<<# (tgeogpoint, gbox),
	OPERATOR	29		<<# (tgeogpoint, tgeogpoint),
	-- strictly after
	OPERATOR	30		#>> (tgeogpoint, gbox),
	OPERATOR	30		#>> (tgeogpoint, tgeogpoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeogpoint, gbox),
	OPERATOR	31		#&> (tgeogpoint, tgeogpoint),
	-- functions
	FUNCTION	1	gist_tgeogpoint_consistent(internal, tgeogpoint, smallint, oid, internal),
	FUNCTION	2	gist_tpoint_union(internal, internal),
	FUNCTION	3	gist_tpoint_compress(internal),
	FUNCTION	5	gist_tpoint_penalty(internal, internal, internal),
	FUNCTION	6	gist_tpoint_picksplit(internal, internal),
	FUNCTION	7	gist_tpoint_same(gbox, gbox, internal);
	
/******************************************************************************/
/*****************************************************************************
 *
 * IndexSpistTemporalPoint.c
 *	  Oct-tree SP-GiST index for temporal points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE FUNCTION spgist_tpoint_config(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_choose(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_picksplit(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_inner_consistent(internal, internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_leaf_consistent(internal, internal)
	RETURNS bool
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spgist_tpoint_compress(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tgeompoint_ops
	DEFAULT FOR TYPE tgeompoint USING spgist AS
	-- strictly left
	OPERATOR	1		<< (tgeompoint, geometry),  
	OPERATOR	1		<< (tgeompoint, gbox),  
	OPERATOR	1		<< (tgeompoint, tgeompoint),  
	-- overlaps or left
	OPERATOR	2		&< (tgeompoint, geometry),  
	OPERATOR	2		&< (tgeompoint, gbox),  
	OPERATOR	2		&< (tgeompoint, tgeompoint),  
	-- overlaps	
	OPERATOR	3		&& (tgeompoint, geometry),  
	OPERATOR	3		&& (tgeompoint, gbox),  
	OPERATOR	3		&& (tgeompoint, tgeompoint),  
	-- overlaps or right
	OPERATOR	4		&> (tgeompoint, geometry),  
	OPERATOR	4		&> (tgeompoint, gbox),  
	OPERATOR	4		&> (tgeompoint, tgeompoint),  
  	-- strictly right
	OPERATOR	5		>> (tgeompoint, geometry),  
	OPERATOR	5		>> (tgeompoint, gbox),  
	OPERATOR	5		>> (tgeompoint, tgeompoint),  
  	-- same
	OPERATOR	6		~= (tgeompoint, geometry),  
	OPERATOR	6		~= (tgeompoint, gbox),  
	OPERATOR	6		~= (tgeompoint, tgeompoint),  
	-- contains
	OPERATOR	7		@> (tgeompoint, geometry),  
	OPERATOR	7		@> (tgeompoint, gbox),  
	OPERATOR	7		@> (tgeompoint, tgeompoint),  
	-- contained by
	OPERATOR	8		<@ (tgeompoint, geometry),  
	OPERATOR	8		<@ (tgeompoint, gbox),  
	OPERATOR	8		<@ (tgeompoint, tgeompoint),  
	-- overlaps or below
	OPERATOR	9		&<| (tgeompoint, geometry),  
	OPERATOR	9		&<| (tgeompoint, gbox),  
	OPERATOR	9		&<| (tgeompoint, tgeompoint),  
	-- strictly below
	OPERATOR	10		<<| (tgeompoint, geometry),  
	OPERATOR	10		<<| (tgeompoint, gbox),  
	OPERATOR	10		<<| (tgeompoint, tgeompoint),  
	-- strictly above
	OPERATOR	11		|>> (tgeompoint, geometry),  
	OPERATOR	11		|>> (tgeompoint, gbox),  
	OPERATOR	11		|>> (tgeompoint, tgeompoint),  
	-- overlaps or above
	OPERATOR	12		|&> (tgeompoint, geometry),  
	OPERATOR	12		|&> (tgeompoint, gbox),  
	OPERATOR	12		|&> (tgeompoint, tgeompoint),  
	-- overlaps or before
	OPERATOR	28		&<# (tgeompoint, gbox),
	OPERATOR	28		&<# (tgeompoint, tgeompoint),
	-- strictly before
	OPERATOR	29		<<# (tgeompoint, gbox),
	OPERATOR	29		<<# (tgeompoint, tgeompoint),
	-- strictly after
	OPERATOR	30		#>> (tgeompoint, gbox),
	OPERATOR	30		#>> (tgeompoint, tgeompoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeompoint, gbox),
	OPERATOR	31		#&> (tgeompoint, tgeompoint),
	-- overlaps or front
	OPERATOR	32		&</ (tgeompoint, gbox),
	OPERATOR	32		&</ (tgeompoint, tgeompoint),
	-- strictly front
	OPERATOR	33		<</ (tgeompoint, gbox),
	OPERATOR	33		<</ (tgeompoint, tgeompoint),
	-- strictly back
	OPERATOR	34		/>> (tgeompoint, gbox),
	OPERATOR	34		/>> (tgeompoint, tgeompoint),
	-- overlaps or back
	OPERATOR	35		/&> (tgeompoint, gbox),
	OPERATOR	35		/&> (tgeompoint, tgeompoint),
	-- functions
	FUNCTION	1	spgist_tpoint_config(internal, internal),
	FUNCTION	2	spgist_tpoint_choose(internal, internal),
	FUNCTION	3	spgist_tpoint_picksplit(internal, internal),
	FUNCTION	4	spgist_tpoint_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tpoint_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tpoint_compress(internal);

/******************************************************************************/

CREATE OPERATOR CLASS spgist_tgeogpoint_ops
	DEFAULT FOR TYPE tgeogpoint USING spgist AS
	-- overlaps
	OPERATOR	3		&& (tgeogpoint, geography),  
	OPERATOR	3		&& (tgeogpoint, gbox),  
	OPERATOR	3		&& (tgeogpoint, tgeogpoint),  
  	-- same
	OPERATOR	6		~= (tgeogpoint, geography),  
	OPERATOR	6		~= (tgeogpoint, gbox),  
	OPERATOR	6		~= (tgeogpoint, tgeogpoint),  
	-- contains
	OPERATOR	7		@> (tgeogpoint, geography),  
	OPERATOR	7		@> (tgeogpoint, gbox),  
	OPERATOR	7		@> (tgeogpoint, tgeogpoint),  
	-- contained by
	OPERATOR	8		<@ (tgeogpoint, geography),  
	OPERATOR	8		<@ (tgeogpoint, gbox),  
	OPERATOR	8		<@ (tgeogpoint, tgeogpoint),  
	-- distance
--	OPERATOR	25		<-> (tgeogpoint, geography) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, gbox) FOR ORDER BY pg_catalog.float_ops,
--	OPERATOR	25		<-> (tgeogpoint, tgeogpoint) FOR ORDER BY pg_catalog.float_ops,
	-- overlaps or before
	OPERATOR	28		&<# (tgeogpoint, gbox),
	OPERATOR	28		&<# (tgeogpoint, tgeogpoint),
	-- strictly before
	OPERATOR	29		<<# (tgeogpoint, gbox),
	OPERATOR	29		<<# (tgeogpoint, tgeogpoint),
	-- strictly after
	OPERATOR	30		#>> (tgeogpoint, gbox),
	OPERATOR	30		#>> (tgeogpoint, tgeogpoint),
	-- overlaps or after
	OPERATOR	31		#&> (tgeogpoint, gbox),
	OPERATOR	31		#&> (tgeogpoint, tgeogpoint),
	-- functions
	FUNCTION	1	spgist_tpoint_config(internal, internal),
	FUNCTION	2	spgist_tpoint_choose(internal, internal),
	FUNCTION	3	spgist_tpoint_picksplit(internal, internal),
	FUNCTION	4	spgist_tpoint_inner_consistent(internal, internal),
	FUNCTION	5	spgist_tpoint_leaf_consistent(internal, internal),
	FUNCTION	6	spgist_tpoint_compress(internal);
	
/******************************************************************************/
/*****************************************************************************
 *
 * OpCache.sql
 *		Routine that pre-computes the opcache and store it as a table in 
 *		the catalog.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
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
