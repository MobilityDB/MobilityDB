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
CREATE TYPE tstzspan;

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

CREATE FUNCTION intspan_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intspan_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Floatspan_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzspan_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tstzspan_analyze'
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

CREATE TYPE tstzspan (
  internallength = 24,
  input = tstzspan_in,
  output = tstzspan_out,
  receive = tstzspan_recv,
  send = tstzspan_send,
  alignment = double,
  analyze = tstzspan_analyze
);

/******************************************************************************
 * Constructors for ranges equivalent to the spans
 ******************************************************************************/

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
CREATE FUNCTION tstzspan(timestamptz, timestamptz)
  RETURNS span
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
CREATE FUNCTION tstzspan(timestamptz, timestamptz, boolean, boolean)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Span_constructor4'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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
CREATE FUNCTION lower(tstzspan)
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
CREATE FUNCTION upper(tstzspan)
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
CREATE FUNCTION lower_inc(tstzspan)
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
CREATE FUNCTION upper_inc(tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Span_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(floatspan, integer DEFAULT 0)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Floatspan_round'
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

-- Functions for debugging the selectivity code

-- Given a table, column, and period returns the estimate of what proportion
-- of the table would be returned by a query using the given operator.
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
CREATE FUNCTION span_eq(tstzspan, tstzspan)
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
CREATE FUNCTION span_ne(tstzspan, tstzspan)
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
CREATE FUNCTION span_lt(tstzspan, tstzspan)
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
CREATE FUNCTION span_le(tstzspan, tstzspan)
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
CREATE FUNCTION span_ge(tstzspan, tstzspan)
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
CREATE FUNCTION span_gt(tstzspan, tstzspan)
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

CREATE OPERATOR CLASS intspan_ops
  DEFAULT FOR TYPE intspan USING btree  AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(intspan, intspan);
CREATE OPERATOR CLASS floatspan_ops
  DEFAULT FOR TYPE floatspan USING btree  AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  span_cmp(floatspan, floatspan);
CREATE OPERATOR CLASS tstzspan_ops
  DEFAULT FOR TYPE tstzspan USING btree  AS
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
CREATE FUNCTION span_hash_extended(floatspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_hash_extended(tstzspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Span_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS hash_intspan_ops
  DEFAULT FOR TYPE intspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(intspan),
    FUNCTION    2   span_hash_extended(intspan, bigint);
CREATE OPERATOR CLASS hash_floatspan_ops
  DEFAULT FOR TYPE floatspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(floatspan),
    FUNCTION    2   span_hash_extended(floatspan, bigint);
CREATE OPERATOR CLASS hash_span_ops
  DEFAULT FOR TYPE tstzspan USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   span_hash(tstzspan),
    FUNCTION    2   span_hash_extended(tstzspan, bigint);

/******************************************************************************/
