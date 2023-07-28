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
 * span.sql
 * SQL definitions for spans (a.k.a. ranges)
 */

CREATE TYPE intspan;
CREATE TYPE bigintspan;
CREATE TYPE floatspan;
CREATE TYPE tstzspan;

/* Forward reference of the types needed for the result of set operations */
CREATE TYPE intspanset;
CREATE TYPE bigintspanset;
CREATE TYPE floatspanset;
CREATE TYPE tstzspanset;

CREATE FUNCTION intspan_in(cstring)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intspan_out(intspan)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Span_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intspan_recv(internal)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intspan_send(intspan)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION bigintspan_in(cstring)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspan_out(bigintspan)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Span_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspan_recv(internal)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspan_send(bigintspan)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION floatspan_in(cstring)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_out(floatspan)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Span_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_recv(internal)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_send(floatspan)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tstzspan_in(cstring)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspan_out(tstzspan)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Span_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspan_recv(internal)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspan_send(tstzspan)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* span_analyze function defined in file 001_set.in.sql */

CREATE TYPE intspan (
  internallength = 24,
  input = intspan_in,
  output = intspan_out,
  receive = intspan_recv,
  send = intspan_send,
  alignment = double,
  analyze = span_analyze
);

CREATE TYPE bigintspan (
  internallength = 24,
  input = bigintspan_in,
  output = bigintspan_out,
  receive = bigintspan_recv,
  send = bigintspan_send,
  alignment = double,
  analyze = span_analyze
);

CREATE TYPE floatspan (
  internallength = 24,
  input = floatspan_in,
  output = floatspan_out,
  receive = floatspan_recv,
  send = floatspan_send,
  alignment = double,
  analyze = span_analyze
);

CREATE TYPE tstzspan (
  internallength = 24,
  input = tstzspan_in,
  output = tstzspan_out,
  receive = tstzspan_recv,
  send = tstzspan_send,
  alignment = double,
  analyze = span_analyze
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION intspanFromBinary(bytea)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspanFromBinary(bytea)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanFromBinary(bytea)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspanFromBinary(bytea)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanFromHexWKB(text)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION bigintspanFromHexWKB(text)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanFromHexWKB(text)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspanFromHexWKB(text)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(intspan)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(bigintspan)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(floatspan, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(tstzspan)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(intspan, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(bigintspan, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(floatspan, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tstzspan, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(intspan, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(bigintspan, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(floatspan, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tstzspan, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION span(int, int, boolean DEFAULT true, boolean DEFAULT false)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(bigint, bigint, boolean DEFAULT true, boolean DEFAULT false)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(float, float, boolean DEFAULT true, boolean DEFAULT false)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(timestamptz, timestamptz, boolean DEFAULT true, boolean DEFAULT false)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION span(integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Value_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(bigint)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Value_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Value_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(timestamptz)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Value_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (integer AS intspan) WITH FUNCTION span(integer);
CREATE CAST (bigint AS bigintspan) WITH FUNCTION span(bigint);
CREATE CAST (float AS floatspan) WITH FUNCTION span(float);
CREATE CAST (timestamptz AS tstzspan) WITH FUNCTION span(timestamptz);

CREATE FUNCTION span(intset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Set_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(bigintset)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Set_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(floatset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Set_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(tstzset)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Set_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (intset AS intspan) WITH FUNCTION span(intset);
CREATE CAST (bigintset AS bigintspan) WITH FUNCTION span(bigintset);
CREATE CAST (floatset AS floatspan) WITH FUNCTION span(floatset);
CREATE CAST (tstzset AS tstzspan) WITH FUNCTION span(tstzset);

CREATE FUNCTION span(int4range)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Range_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span(tstzrange)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Range_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range(intspan)
  RETURNS int4range
  AS 'MODULE_PATHNAME', 'Span_to_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION range(tstzspan)
  RETURNS tstzrange
  AS 'MODULE_PATHNAME', 'Span_to_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int4range AS intspan) WITH FUNCTION span(int4range);
CREATE CAST (tstzrange AS tstzspan) WITH FUNCTION span(tstzrange);
CREATE CAST (intspan AS int4range) WITH FUNCTION range(intspan);
CREATE CAST (tstzspan AS tstzrange) WITH FUNCTION range(tstzspan);

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(floatspan, integer DEFAULT 0)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Floatspan_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(intspan, int)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(bigintspan, bigint)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Span_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(floatspan, float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shift(tstzspan, interval)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Period_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tscale(tstzspan, interval)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Period_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTscale(tstzspan, interval, interval)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Period_shift_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION lower(intspan)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(tstzspan)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper(intspan)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(tstzspan)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower_inc(intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper_inc(intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION width(intspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION width(bigintspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION width(floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(tstzspan)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Period_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectivity functions
 *****************************************************************************/

-- Functions span_sel and span_joinsel are defined in the file defining the
-- set type

-- Functions for debugging the selectivity code

-- Given a table, column, and span returns the estimate of what proportion
-- of the table would be returned by a query using the given operator.
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    i intspan)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    b bigintspan)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    f floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    p tstzspan)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Given two tables and columns, returns estimate of the proportion of rows a
-- given join operator will return relative to the number of rows an
-- unconstrained table join would return
CREATE OR REPLACE FUNCTION _mobdb_span_joinsel(tbl1 regclass, col1 text,
    tbl2 regclass, col2 text, oper regoper)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION span_eq(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_eq(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_eq(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_eq(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_ne(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ne(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ne(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ne(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_lt(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_lt(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_lt(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_lt(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_le(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_le(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_le(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_le(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_ge(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ge(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ge(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ge(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_gt(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gt(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gt(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gt(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_cmp(intspan, intspan)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Span_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_cmp(bigintspan, bigintspan)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Span_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_cmp(floatspan, floatspan)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Span_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_cmp(tstzspan, tstzspan)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Span_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = span_eq,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  PROCEDURE = span_eq,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  PROCEDURE = span_eq,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  PROCEDURE = span_eq,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);

CREATE OPERATOR <> (
  PROCEDURE = span_ne,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = span_ne,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = span_ne,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = span_ne,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  PROCEDURE = span_lt,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  PROCEDURE = span_lt,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  PROCEDURE = span_lt,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  PROCEDURE = span_lt,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR <= (
  PROCEDURE = span_le,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  PROCEDURE = span_le,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  PROCEDURE = span_le,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  PROCEDURE = span_le,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR >= (
  PROCEDURE = span_ge,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  PROCEDURE = span_ge,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  PROCEDURE = span_ge,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  PROCEDURE = span_ge,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR > (
  PROCEDURE = span_gt,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  PROCEDURE = span_gt,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  PROCEDURE = span_gt,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  PROCEDURE = span_gt,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS intspan_btree_ops
  DEFAULT FOR TYPE intspan USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(intspan, intspan);
CREATE OPERATOR CLASS bigintspan_btree_ops
  DEFAULT FOR TYPE bigintspan USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(bigintspan, bigintspan);
CREATE OPERATOR CLASS floatspan_btree_ops
  DEFAULT FOR TYPE floatspan USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(floatspan, floatspan);
CREATE OPERATOR CLASS tstzspan_btree_ops
  DEFAULT FOR TYPE tstzspan USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(tstzspan, tstzspan);

/******************************************************************************/

CREATE FUNCTION span_hash(intspan)
  RETURNS integer
 AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash(bigintspan)
  RETURNS integer
 AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash(floatspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash(tstzspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_hash_extended(intspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash_extended(bigintspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash_extended(floatspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash_extended(tstzspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS intspan_hash_ops
  DEFAULT FOR TYPE intspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(intspan),
    FUNCTION    2   span_hash_extended(intspan, bigint);
CREATE OPERATOR CLASS bigintspan_hash_ops
  DEFAULT FOR TYPE bigintspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(bigintspan),
    FUNCTION    2   span_hash_extended(bigintspan, bigint);
CREATE OPERATOR CLASS floatspan_hash_ops
  DEFAULT FOR TYPE floatspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(floatspan),
    FUNCTION    2   span_hash_extended(floatspan, bigint);
CREATE OPERATOR CLASS tstzspan_hash_ops
  DEFAULT FOR TYPE tstzspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(tstzspan),
    FUNCTION    2   span_hash_extended(tstzspan, bigint);

/******************************************************************************/
