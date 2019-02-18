/*****************************************************************************
 *
 * TemporalNPoint.sql
 *	  Basic functions for temporal network-constrained points.
 *
 * Portions Copyright (c) 2019, Esteban Zimanyi, Arthur Lesuisse, Xinyang Li,
 *		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2019, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 *****************************************************************************/

CREATE TYPE tnpoint;

SELECT register_temporal('tnpoint', 'npoint');

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION tnpoint_in(cstring, oid, integer)
	RETURNS tnpoint 
	AS 'MODULE_PATHNAME', 'temporal_in'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_out(tnpoint)
	RETURNS cstring 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_recv(internal, oid, integer)
	RETURNS tnpoint 
	AS 'MODULE_PATHNAME', 'temporal_recv' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_send(tnpoint)
	RETURNS bytea 
	AS 'MODULE_PATHNAME' 
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tnpoint (
	internallength = variable,
	input = tnpoint_in,
	output = temporal_out,
	receive = tnpoint_recv,
	send = temporal_send,
	storage = extended,
	alignment = double
--    , analyze = temporal_analyze
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

/* Temporal instant */

CREATE FUNCTION tnpointinst(val npoint, t timestamptz)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal instant set */

CREATE FUNCTION tnpointi(tnpoint[])
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal sequence */

CREATE FUNCTION tnpointseq(tnpoint[], lower_inc boolean DEFAULT true, 
		upper_inc boolean DEFAULT true)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'tnpoint_make_tnpointseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tnpoints(tnpoint[])
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_make_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
/******************************************************************************
 * Transformations
 ******************************************************************************/

CREATE FUNCTION tnpointinst(tnpoint)
	RETURNS tnpoint AS 'MODULE_PATHNAME', 'temporal_as_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpointi(tnpoint)
	RETURNS tnpoint AS 'MODULE_PATHNAME', 'temporal_as_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpointseq(tnpoint)
	RETURNS tnpoint AS 'MODULE_PATHNAME', 'temporal_as_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoints(tnpoint)
	RETURNS tnpoint AS 'MODULE_PATHNAME', 'temporal_as_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 Functions
 ******************************************************************************/

CREATE FUNCTION temporalType(tnpoint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_type'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION memSize(tnpoint)
	RETURNS int
	AS 'MODULE_PATHNAME', 'temporal_mem_size'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- position is a reserved word in SQL
CREATE FUNCTION getPosition(tnpoint)
	RETURNS npoint
	AS 'MODULE_PATHNAME', 'temporalinst_get_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- time is a reserved word in SQL
CREATE FUNCTION getTime(tnpoint)
	RETURNS periodset
	AS 'MODULE_PATHNAME', 'temporal_get_time'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- timestamp is a reserved word in SQL
CREATE FUNCTION getTimestamp(tnpoint)
	RETURNS timestamptz
	AS 'MODULE_PATHNAME', 'temporalinst_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION everEquals(tnpoint, npoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &= (
	LEFTARG = tnpoint, RIGHTARG = npoint,
	PROCEDURE = everEquals,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION alwaysEquals(tnpoint, npoint)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_equals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @= (
	LEFTARG = tnpoint, RIGHTARG = npoint,
	PROCEDURE = alwaysEquals,
	RESTRICT = scalarltsel,	JOIN = scalarltjoinsel
);

CREATE FUNCTION shift(tnpoint, interval)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_shift'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atPosition(tnpoint, npoint)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_at_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION minusPosition(tnpoint, npoint)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_value'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION atPositions(tnpoint, npoint[])
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_at_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION minusPositions(tnpoint, npoint[])
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION atTimestamp(tnpoint, timestamptz)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION minusTimestamp(tnpoint, timestamptz)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION positionAtTimestamp(tnpoint, timestamptz)
	RETURNS npoint
	AS 'MODULE_PATHNAME', 'temporal_value_at_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION atTimestampSet(tnpoint, timestampset)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_at_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION minusTimestampSet(tnpoint, timestampset)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_minus_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION atPeriod(tnpoint, period)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_at_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION atPeriodSet(tnpoint, periodset)
	RETURNS tnpoint
	AS 'MODULE_PATHNAME', 'temporal_at_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION intersectsTimestamp(tnpoint, timestamptz)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestamp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
	
CREATE FUNCTION intersectsTimestampSet(tnpoint, timestampset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_timestampset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsPeriod(tnpoint, period)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intersectsPeriodSet(tnpoint, periodset)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_intersects_periodset'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE; 
	
/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION tnpoint_lt(tnpoint, tnpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_le(tnpoint, tnpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_eq(tnpoint, tnpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_ne(tnpoint, tnpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_ge(tnpoint, tnpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_gt(tnpoint, tnpoint)
	RETURNS bool
	AS 'MODULE_PATHNAME', 'temporal_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tnpoint_cmp(tnpoint, tnpoint)
	RETURNS int4
	AS 'MODULE_PATHNAME', 'temporal_cmp'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR < (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = tnpoint_lt,
	COMMUTATOR = >, NEGATOR = >=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = tnpoint_le,
	COMMUTATOR = >=, NEGATOR = >,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR = (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = tnpoint_eq,
	COMMUTATOR = =, NEGATOR = <>,
	RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = tnpoint_ne,
	COMMUTATOR = <>, NEGATOR = =,
	RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR >= (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = tnpoint_ge,
	COMMUTATOR = <=, NEGATOR = <,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);
CREATE OPERATOR > (
	LEFTARG = tnpoint, RIGHTARG = tnpoint,
	PROCEDURE = tnpoint_gt,
	COMMUTATOR = <, NEGATOR = <=,
	RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS tnpoint_ops
	DEFAULT FOR TYPE tnpoint USING btree AS
		OPERATOR	1	<,
		OPERATOR	2	<=,
		OPERATOR	3	=,
		OPERATOR	4	>=,
		OPERATOR	5	>,
		FUNCTION	1	tnpoint_cmp(tnpoint, tnpoint);
		
/******************************************************************************/
		