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
 * span.sql
 * SQL definitions for spans (a.k.a. ranges)
 */

CREATE TYPE intspan;
CREATE TYPE floatspan;
CREATE TYPE period;

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

CREATE FUNCTION period_in(cstring)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_out(period)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Span_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_recv(internal)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_send(period)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspan_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intspan_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Floatspan_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE intspan (
  internallength = 24,
  input = intspan_in,
  output = intspan_out,
  receive = intspan_recv,
  send = intspan_send,
  alignment = double,
  analyze = intspan_analyze
);

CREATE TYPE floatspan (
  internallength = 24,
  input = floatspan_in,
  output = floatspan_out,
  receive = floatspan_recv,
  send = floatspan_send,
  alignment = double,
  analyze = floatspan_analyze
);

CREATE TYPE period (
  internallength = 24,
  input = period_in,
  output = period_out,
  receive = period_recv,
  send = period_send,
  alignment = double,
  analyze = period_analyze
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION intspanFromBinary(bytea)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanFromBinary(bytea)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodFromBinary(bytea)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanFromHexWKB(text)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanFromHexWKB(text)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodFromHexWKB(text)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(intspan)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(floatspan, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(period)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
  
CREATE FUNCTION asBinary(intspan)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(intspan, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(floatspan)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(floatspan, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(period)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(period, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Span_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(intspan)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(intspan, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(floatspan)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(floatspan, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(period)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(period, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Span_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION intspan(int, int)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_constructor2'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan(float, float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_constructor2'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(timestamptz, timestamptz)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_constructor2'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspan(int, int, boolean, boolean)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Span_constructor4'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan(float, float, boolean, boolean)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Span_constructor4'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(timestamptz, timestamptz, boolean, boolean)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Span_constructor4'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION intspan(integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Elem_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan(float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Elem_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(timestamptz)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Elem_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION int4range(intspan)
  RETURNS int4range
  AS 'MODULE_PATHNAME', 'Span_to_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzrange(period)
  RETURNS tstzrange
  AS 'MODULE_PATHNAME', 'Span_to_range'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspan(int4range)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Range_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tstzrange)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Range_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (int4range AS intspan) WITH FUNCTION intspan(int4range);
CREATE CAST (intspan AS int4range) WITH FUNCTION int4range(intspan);
CREATE CAST (timestamptz AS period) WITH FUNCTION period(timestamptz);
CREATE CAST (tstzrange AS period) WITH FUNCTION period(tstzrange);
CREATE CAST (period AS tstzrange) WITH FUNCTION tstzrange(period);

/******************************************************************************
 * Accessor functions
 ******************************************************************************/

CREATE FUNCTION lower(intspan)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower(period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Span_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper(intspan)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper(period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Span_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower_inc(intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lower_inc(period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper_inc(intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION upper_inc(period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION width(intspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION width(floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_width'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(period)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Period_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(floatspan, integer DEFAULT 0)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Floatspan_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(period, interval)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Period_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tscale(period, interval)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Period_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTscale(period, interval, interval)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Period_shift_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectivity functions
 *****************************************************************************/

CREATE FUNCTION span_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_sel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_joinsel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION period_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Period_sel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- Functions for debugging the selectivity code

-- Given a table, column, and span returns the estimate of what proportion
-- of the table would be returned by a query using the given operator.
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    i intspan)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    f floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION _mobdb_span_sel(tbl regclass, col text, oper regoper,
    p period)
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
CREATE FUNCTION span_eq(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_eq(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_ne(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ne(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_ne(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_lt(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_lt(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_lt(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_le(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_le(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_le(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_ge(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_ge(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_ge(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_gt(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_gt(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gt(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_cmp(intspan, intspan)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Span_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_cmp(floatspan, floatspan)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Span_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_cmp(period, period)
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
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  PROCEDURE = period_eq,
  LEFTARG = period, RIGHTARG = period,
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
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = period_ne,
  LEFTARG = period, RIGHTARG = period,
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
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  PROCEDURE = period_lt,
  LEFTARG = period, RIGHTARG = period,
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
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  PROCEDURE = period_le,
  LEFTARG = period, RIGHTARG = period,
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
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  PROCEDURE = period_ge,
  LEFTARG = period, RIGHTARG = period,
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
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  PROCEDURE = period_gt,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS intspan_ops
  DEFAULT FOR TYPE intspan USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(intspan, intspan);
CREATE OPERATOR CLASS floatspan_ops
  DEFAULT FOR TYPE floatspan USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(floatspan, floatspan);
CREATE OPERATOR CLASS period_ops
  DEFAULT FOR TYPE period USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  period_cmp(period, period);

/******************************************************************************/

CREATE FUNCTION span_hash(intspan)
  RETURNS integer
 AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash(floatspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_hash(period)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Span_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION span_hash_extended(intspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash_extended(floatspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_hash_extended(period, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS intspan_hash_ops
  DEFAULT FOR TYPE intspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(intspan),
    FUNCTION    2   span_hash_extended(intspan, bigint);
CREATE OPERATOR CLASS floatspan_hash_ops
  DEFAULT FOR TYPE floatspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(floatspan),
    FUNCTION    2   span_hash_extended(floatspan, bigint);
CREATE OPERATOR CLASS period_hash_ops
  DEFAULT FOR TYPE period USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   period_hash(period),
    FUNCTION    2   period_hash_extended(period, bigint);

/******************************************************************************/
