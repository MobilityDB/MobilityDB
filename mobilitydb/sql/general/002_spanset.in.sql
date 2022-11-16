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
 * spanset.sql
 * Functions for set of spans.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE intspanset;
CREATE TYPE floatspanset;
CREATE TYPE periodset;

CREATE FUNCTION intspanset_in(cstring)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_in(cstring)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_in(cstring)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Spanset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_out(intspanset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_out(floatspanset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_out(periodset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Spanset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_recv(internal)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_recv(internal)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_recv(internal)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Spanset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_send(intspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_send(floatspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_send(periodset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Intspanset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Floatspanset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Periodset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE intspanset (
  internallength = variable,
  input = intspanset_in,
  output = intspanset_out,
  receive = intspanset_recv,
  send = intspanset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = intspanset_analyze
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
  analyze = floatspanset_analyze
);
CREATE TYPE periodset (
  internallength = variable,
  input = periodset_in,
  output = periodset_out,
  receive = periodset_recv,
  send = periodset_send,
  alignment = double,
  storage = extended,
  analyze = periodset_analyze
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

CREATE FUNCTION floatspansetFromBinary(bytea)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspansetFromHexWKB(text)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodsetFromBinary(bytea)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Spanset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodsetFromHexWKB(text)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Spanset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(intspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(intspanset, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(floatspanset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(floatspanset, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(periodset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(periodset, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Spanset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(intspanset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(intspanset, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(floatspanset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(floatspanset, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(periodset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(periodset, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Spanset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION intspanset(intspan[])
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset(floatspan[])
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(period[])
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Spanset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION intspanset(intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset(floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Span_to_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspan(intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan(floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Spanset_to_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (intspan AS intspanset) WITH FUNCTION intspanset(intspan);
CREATE CAST (floatspan AS floatspanset) WITH FUNCTION floatspanset(floatspan);
CREATE CAST (period AS periodset) WITH FUNCTION periodset(period);

CREATE CAST (intspanset AS intspan) WITH FUNCTION intspan(intspanset);
CREATE CAST (floatspanset AS floatspan) WITH FUNCTION floatspan(floatspanset);
CREATE CAST (periodset AS period) WITH FUNCTION period(periodset);

/*
CREATE FUNCTION periodset(timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Timestamp_to_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Timestampset_to_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS periodset) WITH FUNCTION periodset(timestamptz);
CREATE CAST (timestampset AS periodset) WITH FUNCTION periodset(timestampset);
*/

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION memSize(intspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(floatspanset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION memSize(periodset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Spanset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numSpans(intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numSpans(floatspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION numPeriods(periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_num_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startSpan(intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startSpan(floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION startPeriod(periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Spanset_start_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endSpan(intspanset)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endSpan(floatspanset)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION endPeriod(periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Spanset_end_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spanN(intspanset, integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanN(floatspanset, integer)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodN(periodset, integer)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Spanset_span_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION spans(intspanset)
  RETURNS intspan[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spans(floatspanset)
  RETURNS floatspan[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periods(periodset)
  RETURNS period[]
  AS 'MODULE_PATHNAME', 'Spanset_spans'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION intspanset_eq(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_eq(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_eq(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_ne(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_ne(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_ne(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_lt(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_lt(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_lt(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_le(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_le(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_le(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_ge(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_ge(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_ge(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_gt(intspanset, intspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_gt(floatspanset, floatspanset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_gt(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Spanset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_cmp(intspanset, intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_cmp(floatspanset, floatspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_cmp(periodset, periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = intspanset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = floatspanset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR = (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);

CREATE OPERATOR <> (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = intspanset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = floatspanset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);

CREATE OPERATOR < (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = intspanset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = floatspanset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR < (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR <= (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = intspanset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = floatspanset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR >= (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = intspanset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = floatspanset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR > (
  LEFTARG = intspanset, RIGHTARG = intspanset,
  PROCEDURE = intspanset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  PROCEDURE = floatspanset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS intspanset_ops
  DEFAULT FOR TYPE intspanset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  intspanset_cmp(intspanset, intspanset);
CREATE OPERATOR CLASS floatspanset_ops
  DEFAULT FOR TYPE floatspanset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  floatspanset_cmp(floatspanset, floatspanset);
CREATE OPERATOR CLASS periodset_ops
  DEFAULT FOR TYPE periodset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  periodset_cmp(periodset, periodset);

/******************************************************************************/

CREATE FUNCTION intspanset_hash(intspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_hash(floatspanset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_hash(periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Spanset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION intspanset_hash_extended(intspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspanset_hash_extended(floatspanset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_hash_extended(periodset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Spanset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS intspanset_hash_ops
  DEFAULT FOR TYPE intspanset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   intspanset_hash(intspanset),
    FUNCTION    2   intspanset_hash_extended(intspanset, bigint);
CREATE OPERATOR CLASS floatspanset_hash_ops
  DEFAULT FOR TYPE floatspanset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   floatspanset_hash(floatspanset),
    FUNCTION    2   floatspanset_hash_extended(floatspanset, bigint);
CREATE OPERATOR CLASS periodset_hash_ops
  DEFAULT FOR TYPE periodset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   periodset_hash(periodset),
    FUNCTION    2   periodset_hash_extended(periodset, bigint);


/******************************************************************************/
