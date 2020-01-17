/*****************************************************************************
 *
 * temporal.sql
 *	  Basic functions for generic temporal types.
 *
 * Portions Copyright (c) 2020, Esteban Zimanyi, Arthur Lesuisse, 
 * 		Universite Libre de Bruxelles
 * Portions Copyright (c) 1996-2020, PostgreSQL Global Development Group
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

/*****************************************************************************
 * Utility functions
 *****************************************************************************/

CREATE FUNCTION mobdb_lib_version() RETURNS text
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE;

CREATE FUNCTION mobdb_full_version() RETURNS text
	AS 'MODULE_PATHNAME'
	LANGUAGE C IMMUTABLE;

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
	AS 'MODULE_PATHNAME', 'temporalinst_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintinst(val integer, t timestamptz)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporalinst_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatinst(val float, t timestamptz)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporalinst_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextinst(val text, t timestamptz)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporalinst_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal instant set */

CREATE FUNCTION tbooli(tbool[])
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporali_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tinti(tint[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporali_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloati(tfloat[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporali_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexti(ttext[])
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporali_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal sequence */

CREATE FUNCTION tboolseq(tbool[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'tlinearseq_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintseq(tint[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'tlinearseq_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatseq(tfloat[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true, linear boolean DEFAULT true)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporalseq_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextseq(ttext[], lower_inc boolean DEFAULT true, 
	upper_inc boolean DEFAULT true)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'tlinearseq_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* Temporal sequence set */
	
CREATE FUNCTION tbools(tbool[])
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporals_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tints(tint[])
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporals_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloats(tfloat[])
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporals_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexts(ttext[])
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporals_constructor'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION period(tbool)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tint)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tfloat)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(ttext)
	RETURNS period
	AS 'MODULE_PATHNAME', 'temporal_to_period'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Casting CANNOT be implicit to avoid ambiguity
CREATE CAST (tbool AS period) WITH FUNCTION period(tbool);
CREATE CAST (tint AS period) WITH FUNCTION period(tint);
CREATE CAST (tfloat AS period) WITH FUNCTION period(tfloat);
CREATE CAST (ttext AS period) WITH FUNCTION period(ttext);

CREATE FUNCTION tfloat(tint)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tint_to_tfloat'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tint(tfloat)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'tfloat_to_tint'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tint AS tfloat) WITH FUNCTION tfloat(tint);
CREATE CAST (tfloat AS tint) WITH FUNCTION tint(tfloat);

/******************************************************************************
 * Transformation functions
 ******************************************************************************/

CREATE FUNCTION tboolinst(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_to_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbooli(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_to_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboolseq(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_to_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbools(tbool)
	RETURNS tbool
	AS 'MODULE_PATHNAME', 'temporal_to_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tintinst(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_to_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tinti(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_to_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tintseq(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_to_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tints(tint)
	RETURNS tint
	AS 'MODULE_PATHNAME', 'temporal_to_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tfloatinst(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_to_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloati(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_to_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloatseq(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_to_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tfloats(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'temporal_to_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION ttextinst(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_to_temporalinst'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexti(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_to_temporali'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttextseq(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_to_temporalseq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ttexts(ttext)
	RETURNS ttext
	AS 'MODULE_PATHNAME', 'temporal_to_temporals'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION toLinear(tfloat)
	RETURNS tfloat
	AS 'MODULE_PATHNAME', 'tstepw_to_linear'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************/

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
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION duration(tbool)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(tint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(tfloat)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION duration(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_duration'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION interpolation(tbool)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_interpolation'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION interpolation(tint)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_interpolation'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION interpolation(tfloat)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_interpolation'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION interpolation(ttext)
	RETURNS text
	AS 'MODULE_PATHNAME', 'temporal_interpolation'
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
	AS 'MODULE_PATHNAME', 'temporal_get_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(tint)
	RETURNS integer[]
	AS 'MODULE_PATHNAME', 'temporal_get_values'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(tfloat)
	RETURNS floatrange[]
	AS 'MODULE_PATHNAME', 'tfloat_get_ranges'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(ttext)
	RETURNS text[]
	AS 'MODULE_PATHNAME', 'temporal_get_values'
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
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(tint)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(tfloat)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_timespan'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timespan(ttext)
	RETURNS interval
	AS 'MODULE_PATHNAME', 'temporal_timespan'
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

/*****************************************************************************
 * Ever/Always Comparison Functions 
 *****************************************************************************/

CREATE FUNCTION ever_eq(tbool, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_eq(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?= (
	LEFTARG = tbool, RIGHTARG = boolean,
	PROCEDURE = ever_eq,
	NEGATOR = %<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_eq,
	NEGATOR = %<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_eq,
	NEGATOR = %<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_eq,
	NEGATOR = %<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_eq(tbool, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_eq(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_eq'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %= (
	LEFTARG = tbool, RIGHTARG = boolean,
	PROCEDURE = always_eq,
	NEGATOR = ?<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_eq,
	NEGATOR = ?<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_eq,
	NEGATOR = ?<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_eq,
	NEGATOR = ?<>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ne(tbool, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ne(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<> (
	LEFTARG = tbool, RIGHTARG = boolean,
	PROCEDURE = ever_ne,
	NEGATOR = %=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_ne,
	NEGATOR = %=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_ne,
	NEGATOR = %=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<> (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_ne,
	NEGATOR = %=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ne(tbool, boolean)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ne(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ne'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<> (
	LEFTARG = tbool, RIGHTARG = boolean,
	PROCEDURE = always_ne,
	NEGATOR = ?=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_ne,
	NEGATOR = ?=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_ne,
	NEGATOR = ?=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<> (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_ne,
	NEGATOR = ?=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************
 * Ever/Always Comparison Functions 
 *****************************************************************************/

CREATE FUNCTION ever_lt(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_lt(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?< (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_lt,
	NEGATOR = %>=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?< (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_lt,
	NEGATOR = %>=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?< (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_lt,
	NEGATOR = %>=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_le(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_le(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?<= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_le,
	NEGATOR = %>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_le,
	NEGATOR = %>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?<= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_le,
	NEGATOR = %>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_lt(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_lt(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_lt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %< (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_lt,
	NEGATOR = ?>=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %< (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_lt,
	NEGATOR = ?>=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %< (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_lt,
	NEGATOR = ?>=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_le(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_le(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_le'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %<= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_le,
	NEGATOR = ?>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_le,
	NEGATOR = ?>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %<= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_le,
	NEGATOR = ?>,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_gt(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_gt(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?> (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_gt,
	NEGATOR = %<=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?> (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_gt,
	NEGATOR = %<=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?> (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_gt,
	NEGATOR = %<=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION ever_ge(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ever_ge(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_ever_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ?>= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = ever_ge,
	NEGATOR = %<,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?>= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = ever_ge,
	NEGATOR = %<,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR ?>= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = ever_ge,
	NEGATOR = %<,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_gt(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_gt(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_gt'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %> (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_gt,
	NEGATOR = ?<=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %> (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_gt,
	NEGATOR = ?<=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %> (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_gt,
	NEGATOR = ?<=,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

CREATE FUNCTION always_ge(tint, integer)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(tfloat, float)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION always_ge(ttext, text)
	RETURNS boolean
	AS 'MODULE_PATHNAME', 'temporal_always_ge'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR %>= (
	LEFTARG = tint, RIGHTARG = integer,
	PROCEDURE = always_ge,
	NEGATOR = ?<,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %>= (
	LEFTARG = tfloat, RIGHTARG = float,
	PROCEDURE = always_ge,
	NEGATOR = ?<,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR %>= (
	LEFTARG = ttext, RIGHTARG = text,
	PROCEDURE = always_ge,
	NEGATOR = ?<,
	RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);

/*****************************************************************************
 * Restriction Functions 
 *****************************************************************************/

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
	AS 'MODULE_PATHNAME', 'tnumber_integral'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION integral(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_integral'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION twAvg(tint)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_twavg'
	LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION twAvg(tfloat)
	RETURNS float
	AS 'MODULE_PATHNAME', 'tnumber_twavg'
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
