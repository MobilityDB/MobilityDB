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
 * periodset.sql
 * Functions for set of periods.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE periodset;

CREATE FUNCTION periodset_in(cstring)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_out(periodset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Periodset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_recv(internal)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_send(periodset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Periodset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Periodset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE periodset (
  internallength = variable,
  input = periodset_in,
  output = periodset_out,
  receive = periodset_recv,
  send = periodset_send,
  alignment = double,
-- The following line makes NULL if size < 128
  storage = extended,
  analyze = periodset_analyze
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION periodsetFromBinary(bytea)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodsetFromHexWKB(text)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(periodset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Periodset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(periodset, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Periodset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(periodset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Periodset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(periodset, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Periodset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION periodset(period[])
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION periodset(timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Timestamp_to_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Timestampset_to_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset(period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Period_to_periodset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Periodset_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS periodset) WITH FUNCTION periodset(timestamptz);
CREATE CAST (timestampset AS periodset) WITH FUNCTION periodset(timestampset);
CREATE CAST (period AS periodset) WITH FUNCTION periodset(period);
CREATE CAST (periodset AS period) WITH FUNCTION period(periodset);

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION memSize(periodset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Periodset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(periodset)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Periodset_timespan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(periodset)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Periodset_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numPeriods(periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Periodset_num_periods'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startPeriod(periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Periodset_start_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endPeriod(periodset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Periodset_end_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodN(periodset, integer)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Periodset_period_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periods(periodset)
  RETURNS period[]
  AS 'MODULE_PATHNAME', 'Periodset_periods'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Periodset_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Periodset_start_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Periodset_end_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(periodset, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Periodset_timestamp_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(periodset)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Periodset_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(periodset, interval)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tscale(periodset, interval)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftTscale(periodset, interval, interval)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Periodset_shift_tscale'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION periodset_eq(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Periodset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_ne(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Periodset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_lt(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Periodset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_le(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Periodset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_ge(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Periodset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_gt(periodset, periodset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Periodset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION periodset_cmp(periodset, periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Periodset_cmp'
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
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = periodset, RIGHTARG = periodset,
  PROCEDURE = periodset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS periodset_ops
  DEFAULT FOR TYPE periodset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  periodset_cmp(periodset, periodset);

/******************************************************************************/

CREATE FUNCTION periodset_hash(periodset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Periodset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodset_hash_extended(periodset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Periodset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS periodset_hash_ops
  DEFAULT FOR TYPE periodset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   periodset_hash(periodset),
    FUNCTION    2   periodset_hash_extended(periodset, bigint);

/******************************************************************************/
