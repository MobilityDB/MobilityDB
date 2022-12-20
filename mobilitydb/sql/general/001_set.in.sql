/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2022, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2022, PostGIS contributors
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without a written
 * agreement is hereby granted, provided that the above copyright notice and
 * this paragraph and the following two paragraphs appear in all copies.
 *
 * IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
 * LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
 * AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS. 
 *
 *****************************************************************************/

/*
 * set.sql
 * Functions for set of ordered values.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE intset;
CREATE TYPE bigintset;
CREATE TYPE floatset;
CREATE TYPE textset;
CREATE TYPE timestampset;

CREATE FUNCTION intset_in(cstring)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intset_out(intset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intset_recv(internal)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intset_send(intset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION bigintset_in(cstring)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_out(bigintset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_recv(internal)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_send(bigintset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION floatset_in(cstring)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_out(floatset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_recv(internal)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_send(floatset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION textset_in(cstring)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_out(textset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_recv(internal)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_send(textset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_in(cstring)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Set_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_out(timestampset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Set_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_recv(internal)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Set_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_send(timestampset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Bigintset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Floatset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Timestampset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION textset_analyze(internal)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Textset_analyze'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE intset (
  internallength = variable,
  input = intset_in,
  output = intset_out,
  receive = intset_recv,
  send = intset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = intset_analyze
);

CREATE TYPE bigintset (
  internallength = variable,
  input = bigintset_in,
  output = bigintset_out,
  receive = bigintset_recv,
  send = bigintset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = bigintset_analyze
);

CREATE TYPE floatset (
  internallength = variable,
  input = floatset_in,
  output = floatset_out,
  receive = floatset_recv,
  send = floatset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = floatset_analyze
);

CREATE TYPE textset (
  internallength = variable,
  input = textset_in,
  output = textset_out,
  receive = textset_recv,
  send = textset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended
  -- , analyze = textset_analyze
);

CREATE TYPE timestampset (
  internallength = variable,
  input = timestampset_in,
  output = timestampset_out,
  receive = timestampset_recv,
  send = timestampset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = timestampset_analyze
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION intsetFromBinary(bytea)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intsetFromHexWKB(text)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION bigintsetFromBinary(bytea)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintsetFromHexWKB(text)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION floatsetFromBinary(bytea)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatsetFromHexWKB(text)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION textsetFromBinary(bytea)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textsetFromHexWKB(text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampsetFromBinary(bytea)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Set_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampsetFromHexWKB(text)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Set_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(intset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(bigintset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(floatset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(textset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(timestampset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Set_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(intset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(bigintset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(floatset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(textset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(timestampset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION intset(int[])
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset(bigint[])
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset(float[])
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset(text[])
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset(timestamptz[])
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Set_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION intset(integer)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset(bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset(float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset(timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset(text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Value_to_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int AS intset) WITH FUNCTION intset(int);
CREATE CAST (bigint AS bigintset) WITH FUNCTION bigintset(bigint);
CREATE CAST (float AS floatset) WITH FUNCTION floatset(float);
CREATE CAST (text AS textset) WITH FUNCTION textset(text);
CREATE CAST (timestamptz AS timestampset) WITH FUNCTION timestampset(timestamptz);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(floatset, integer DEFAULT 0)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Floatset_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(intset, int)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Set_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(bigintset, bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Set_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(floatset, float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Set_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(timestampset, interval)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tscale(timestampset, interval)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTscale(timestampset, interval, interval)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_shift_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION memorySize(intset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_memory_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memorySize(bigintset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_memory_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memorySize(floatset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_memory_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memorySize(textset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_memory_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memorySize(timestampset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_memory_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION storageSize(intset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_storage_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION storageSize(bigintset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_storage_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION storageSize(floatset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_storage_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION storageSize(textset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_memory_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION storageSize(timestampset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Set_storage_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numValues(intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numValues(bigintset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numValues(floatset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numValues(textset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numTimestamps(timestampset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_num_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startValue(intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(bigintset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startValue(textset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startTimestamp(timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endValue(intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(bigintset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endValue(textset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_start_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endTimestamp(timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Set_end_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION valueN(intset, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueN(bigintset, integer)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueN(floatset, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION valueN(textset, integer)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampN(timestampset, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Set_value_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getValues(intset)
  RETURNS integer[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(bigintset)
  RETURNS bigint[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(floatset)
  RETURNS float[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION getValues(textset)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestamps(timestampset)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Set_values'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectivity functions
 *****************************************************************************/

CREATE FUNCTION span_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_sel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Period_sel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_joinsel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION intset_eq(intset, intset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_eq(bigintset, bigintset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_eq(floatset, floatset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_eq(textset, textset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_eq(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_ne(intset, intset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_ne(bigintset, bigintset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_ne(floatset, floatset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_ne(textset, textset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_ne(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_lt(intset, intset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_lt(bigintset, bigintset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_lt(floatset, floatset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_lt(textset, textset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_lt(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_le(intset, intset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_le(bigintset, bigintset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_le(floatset, floatset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_le(textset, textset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_le(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_ge(intset, intset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_ge(bigintset, bigintset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_ge(floatset, floatset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_ge(textset, textset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_ge(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_gt(intset, intset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_gt(bigintset, bigintset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_gt(floatset, floatset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_gt(textset, textset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_gt(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Set_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_cmp(intset, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_cmp(bigintset, bigintset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_cmp(floatset, floatset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_cmp(textset, textset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_cmp(timestampset, timestampset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = intset, RIGHTARG = intset,
  PROCEDURE = intset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = bigintset, RIGHTARG = bigintset,
  PROCEDURE = bigintset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = floatset, RIGHTARG = floatset,
  PROCEDURE = floatset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = textset, RIGHTARG = textset,
  PROCEDURE = textset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);

CREATE OPERATOR <> (
  LEFTARG = intset, RIGHTARG = intset,
  PROCEDURE = intset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = bigintset, RIGHTARG = bigintset,
  PROCEDURE = bigintset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = floatset, RIGHTARG = floatset,
  PROCEDURE = floatset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = textset, RIGHTARG = textset,
  PROCEDURE = textset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = intset, RIGHTARG = intset,
  PROCEDURE = intset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = bigintset, RIGHTARG = bigintset,
  PROCEDURE = bigintset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = floatset, RIGHTARG = floatset,
  PROCEDURE = floatset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = textset, RIGHTARG = textset,
  PROCEDURE = textset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR <= (
  LEFTARG = intset, RIGHTARG = intset,
  PROCEDURE = intset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = bigintset, RIGHTARG = bigintset,
  PROCEDURE = bigintset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = floatset, RIGHTARG = floatset,
  PROCEDURE = floatset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = textset, RIGHTARG = textset,
  PROCEDURE = textset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR >= (
  LEFTARG = intset, RIGHTARG = intset,
  PROCEDURE = intset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = bigintset, RIGHTARG = bigintset,
  PROCEDURE = bigintset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = floatset, RIGHTARG = floatset,
  PROCEDURE = floatset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = textset, RIGHTARG = textset,
  PROCEDURE = textset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR > (
  LEFTARG = intset, RIGHTARG = intset,
  PROCEDURE = intset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = bigintset, RIGHTARG = bigintset,
  PROCEDURE = bigintset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = floatset, RIGHTARG = floatset,
  PROCEDURE = floatset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = textset, RIGHTARG = textset,
  PROCEDURE = textset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS intset_ops
  DEFAULT FOR TYPE intset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  intset_cmp(intset, intset);
CREATE OPERATOR CLASS bigintset_ops
  DEFAULT FOR TYPE bigintset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  bigintset_cmp(bigintset, bigintset);
CREATE OPERATOR CLASS floatset_ops
  DEFAULT FOR TYPE floatset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  floatset_cmp(floatset, floatset);
CREATE OPERATOR CLASS textset_ops
  DEFAULT FOR TYPE textset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  textset_cmp(textset, textset);
CREATE OPERATOR CLASS timestampset_ops
  DEFAULT FOR TYPE timestampset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  timestampset_cmp(timestampset, timestampset);

/******************************************************************************/

CREATE FUNCTION intset_hash(intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_hash(bigintset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_hash(floatset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_hash(textset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_hash(timestampset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Set_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intset_hash_extended(intset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintset_hash_extended(bigintset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatset_hash_extended(floatset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION textset_hash_extended(textset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_hash_extended(timestampset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Set_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS intset_hash_ops
  DEFAULT FOR TYPE intset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   intset_hash(intset),
    FUNCTION    2   intset_hash_extended(intset, bigint);
CREATE OPERATOR CLASS bigintset_hash_ops
  DEFAULT FOR TYPE bigintset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   bigintset_hash(bigintset),
    FUNCTION    2   bigintset_hash_extended(bigintset, bigint);
CREATE OPERATOR CLASS floatset_hash_ops
  DEFAULT FOR TYPE floatset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   floatset_hash(floatset),
    FUNCTION    2   floatset_hash_extended(floatset, bigint);
CREATE OPERATOR CLASS textset_hash_ops
  DEFAULT FOR TYPE textset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   textset_hash(textset),
    FUNCTION    2   textset_hash_extended(textset, bigint);
CREATE OPERATOR CLASS timestampset_hash_ops
  DEFAULT FOR TYPE timestampset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   timestampset_hash(timestampset),
    FUNCTION    2   timestampset_hash_extended(timestampset, bigint);

/******************************************************************************/
