/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * spanset.sql
 * Functions for set of spans.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION intspanset_in(cstring)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspanset_in(cstring)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_in(cstring)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspanset_in(cstring)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_out(intspanset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspanset_out(bigintspanset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_out(floatspanset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspanset_out(tstzspanset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_recv(internal)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspanset_recv(internal)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_recv(internal)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspanset_recv(internal)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_send(intspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspanset_send(bigintspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_send(floatspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspanset_send(tstzspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* span_analyze function defined in file 001_set.in.sql */

CREATE TYPE intspanset (
  internallength = variable,
  input = intspanset_in,
  output = intspanset_out,
  receive = intspanset_recv,
  send = intspanset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = span_analyze
);
CREATE TYPE bigintspanset (
  internallength = variable,
  input = bigintspanset_in,
  output = bigintspanset_out,
  receive = bigintspanset_recv,
  send = bigintspanset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = span_analyze
);
CREATE TYPE floatspanset (
  internallength = variable,
  input = floatspanset_in,
  output = floatspanset_out,
  receive = floatspanset_recv,
  send = floatspanset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = span_analyze
);
CREATE TYPE tstzspanset (
  internallength = variable,
  input = tstzspanset_in,
  output = tstzspanset_out,
  receive = tstzspanset_recv,
  send = tstzspanset_send,
  alignment = double,
  storage = extended,
  analyze = span_analyze
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION intspansetFromBinary(bytea)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intspansetFromHexWKB(text)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION bigintspansetFromBinary(bytea)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspansetFromHexWKB(text)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION floatspansetFromBinary(bytea)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspansetFromHexWKB(text)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tstzspansetFromBinary(bytea)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspansetFromHexWKB(text)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(intspanset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(bigintspanset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(floatspanset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tstzspanset, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(intspanset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(bigintspanset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(floatspanset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tstzspanset, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION spanset(intspan[])
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(bigintspan[])
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(floatspan[])
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(tstzspan[])
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION spanset(int)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Value_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Value_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Value_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Value_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int AS intspanset) WITH FUNCTION spanset(int);
CREATE CAST (bigint AS bigintspanset) WITH FUNCTION spanset(bigint);
CREATE CAST (float AS floatspanset) WITH FUNCTION spanset(float);
CREATE CAST (timestamptz AS tstzspanset) WITH FUNCTION spanset(timestamptz);

CREATE FUNCTION spanset(intset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Set_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(bigintset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Set_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(floatset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Set_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(tstzset)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Set_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (intset AS intspanset) WITH FUNCTION spanset(intset);
CREATE CAST (bigintset AS bigintspanset) WITH FUNCTION spanset(bigintset);
CREATE CAST (floatset AS floatspanset) WITH FUNCTION spanset(floatset);
CREATE CAST (tstzset AS tstzspanset) WITH FUNCTION spanset(tstzset);

CREATE FUNCTION spanset(intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (intspan AS intspanset) WITH FUNCTION spanset(intspan);
CREATE CAST (bigintspan AS bigintspanset) WITH FUNCTION spanset(bigintspan);
CREATE CAST (floatspan AS floatspanset) WITH FUNCTION spanset(floatspan);
CREATE CAST (tstzspan AS tstzspanset) WITH FUNCTION spanset(tstzspan);

#if POSTGRESQL_VERSION_NUMBER >= 140000
CREATE FUNCTION multirange(intspanset)
  RETURNS int4multirange
  AS 'MODULE_PATHNAME', 'Spanset_to_multirange'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION multirange(tstzspanset)
  RETURNS tstzmultirange
  AS 'MODULE_PATHNAME', 'Spanset_to_multirange'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset(int4multirange)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Multirange_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset(tstzmultirange)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Multirange_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (intspanset AS int4multirange) WITH FUNCTION multirange(intspanset);
CREATE CAST (tstzspanset AS tstzmultirange) WITH FUNCTION multirange(tstzspanset);
CREATE CAST (int4multirange AS intspanset) WITH FUNCTION spanset(int4multirange);
CREATE CAST (tstzmultirange AS tstzspanset) WITH FUNCTION spanset(tstzmultirange);
#endif //POSTGRESQL_VERSION_NUMBER >= 140000

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(floatspanset, integer DEFAULT 0)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Floatspanset_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(intspanset, int)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Spanset_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(tstzspanset, interval)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Periodset_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tscale(tstzspanset, interval)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Periodset_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTscale(tstzspanset, interval, interval)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Periodset_shift_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor Functions
 ******************************************************************************/

CREATE FUNCTION memSize(intspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(bigintspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(floatspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(tstzspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span(intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(bigintspanset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(tstzspanset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower(intspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(bigintspanset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Spanset_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(tstzspanset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Spanset_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper(intspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(bigintspanset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Spanset_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(tstzspanset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Spanset_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower_inc(intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper_inc(intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(tstzspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Spanset_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION width(intspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Spanset_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION width(bigintspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Spanset_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION width(floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Spanset_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tstzspanset, boundspan boolean DEFAULT FALSE)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Periodset_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSpans(intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSpans(bigintspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSpans(floatspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSpans(tstzspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSpan(intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSpan(bigintspanset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSpan(floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSpan(tstzspanset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSpan(intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSpan(bigintspanset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSpan(floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSpan(tstzspanset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanN(intspanset, integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanN(bigintspanset, integer)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanN(floatspanset, integer)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanN(tstzspanset, integer)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spans(intspanset)
  RETURNS intspan[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(bigintspanset)
  RETURNS bigintspan[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(floatspanset)
  RETURNS floatspan[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(tstzspanset)
  RETURNS tstzspan[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(tstzspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Periodset_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(tstzspanset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Periodset_start_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(tstzspanset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Periodset_end_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(tstzspanset, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Periodset_timestamp_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(tstzspanset)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Periodset_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION spanset_eq(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_eq(bigintspanset, bigintspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_eq(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_eq(tstzspanset, tstzspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_ne(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_ne(bigintspanset, bigintspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_ne(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_ne(tstzspanset, tstzspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_lt(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_lt(bigintspanset, bigintspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_lt(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_lt(tstzspanset, tstzspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_le(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_le(bigintspanset, bigintspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_le(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_le(tstzspanset, tstzspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_ge(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_ge(bigintspanset, bigintspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_ge(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_ge(tstzspanset, tstzspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_gt(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_gt(bigintspanset, bigintspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_gt(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_gt(tstzspanset, tstzspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_cmp(intspanset, intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_cmp(bigintspanset, bigintspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_cmp(floatspanset, floatspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_cmp(tstzspanset, tstzspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = spanset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  PROCEDURE = spanset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = spanset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  PROCEDURE = spanset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);

CREATE OPERATOR <> (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = spanset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  PROCEDURE = spanset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = spanset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  PROCEDURE = spanset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = spanset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  PROCEDURE = spanset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = spanset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  PROCEDURE = spanset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR <= (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = spanset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  PROCEDURE = spanset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = spanset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  PROCEDURE = spanset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR >= (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = spanset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  PROCEDURE = spanset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = spanset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  PROCEDURE = spanset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR > (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = spanset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  PROCEDURE = spanset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = spanset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = tstzspanset, RIGHTARG = tstzspanset,
  PROCEDURE = spanset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS intspanset_btree_ops
  DEFAULT FOR TYPE intspanset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  spanset_cmp(intspanset, intspanset);
CREATE OPERATOR CLASS bigintspanset_btree_ops
  DEFAULT FOR TYPE bigintspanset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  spanset_cmp(bigintspanset, bigintspanset);
CREATE OPERATOR CLASS floatspanset_btree_ops
  DEFAULT FOR TYPE floatspanset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  spanset_cmp(floatspanset, floatspanset);
CREATE OPERATOR CLASS tstzspanset_btree_ops
  DEFAULT FOR TYPE tstzspanset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  spanset_cmp(tstzspanset, tstzspanset);

/******************************************************************************/

CREATE FUNCTION spanset_hash(intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_hash(bigintspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_hash(floatspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_hash(tstzspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanset_hash_extended(intspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_hash_extended(bigintspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_hash_extended(floatspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanset_hash_extended(tstzspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS intspanset_hash_ops
  DEFAULT FOR TYPE intspanset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   spanset_hash(intspanset),
    FUNCTION    2   spanset_hash_extended(intspanset, bigint);
CREATE OPERATOR CLASS bigintspanset_hash_ops
  DEFAULT FOR TYPE bigintspanset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   spanset_hash(bigintspanset),
    FUNCTION    2   spanset_hash_extended(bigintspanset, bigint);
CREATE OPERATOR CLASS floatspanset_hash_ops
  DEFAULT FOR TYPE floatspanset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   spanset_hash(floatspanset),
    FUNCTION    2   spanset_hash_extended(floatspanset, bigint);
CREATE OPERATOR CLASS tstzspanset_hash_ops
  DEFAULT FOR TYPE tstzspanset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   spanset_hash(tstzspanset),
    FUNCTION    2   spanset_hash_extended(tstzspanset, bigint);


/******************************************************************************/
