/*****************************************************************************
 *
 * tpoint.sql
 *	  Basic functions for temporal points.
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

