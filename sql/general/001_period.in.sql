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
 * period.sql
 * SQL definitions for timestamptz periods.
 */

-- CREATE TYPE period;

-- CREATE FUNCTION period_in(cstring)
  -- RETURNS period
  -- AS 'MODULE_PATHNAME', 'Period_in'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION period_out(period)
  -- RETURNS cstring
  -- AS 'MODULE_PATHNAME', 'Period_out'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION period_recv(internal)
  -- RETURNS period
  -- AS 'MODULE_PATHNAME', 'Period_recv'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
-- CREATE FUNCTION period_send(period)
  -- RETURNS bytea
  -- AS 'MODULE_PATHNAME', 'Period_send'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION period_analyze(internal)
  -- RETURNS boolean
  -- AS 'MODULE_PATHNAME', 'Period_analyze'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE TYPE period (
  -- internallength = 24,
  -- input = period_in,
  -- output = period_out,
  -- receive = period_recv,
  -- send = period_send,
  -- alignment = double,
  -- analyze = period_analyze
-- );

/******************************************************************************
 * Constructors
 ******************************************************************************/

-- CREATE FUNCTION period(timestamptz, timestamptz)
  -- RETURNS period
  -- AS 'MODULE_PATHNAME', 'Period_constructor2'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- CREATE FUNCTION period(timestamptz, timestamptz, boolean, boolean)
  -- RETURNS period
  -- AS 'MODULE_PATHNAME', 'Period_constructor4'
  -- LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION period(timestamptz)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Timestamp_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tstzrange)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Tstzrange_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzrange(period)
  RETURNS tstzrange
  AS 'MODULE_PATHNAME', 'Period_to_tstzrange'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS period) WITH FUNCTION period(timestamptz);
CREATE CAST (tstzrange AS period) WITH FUNCTION period(tstzrange);
CREATE CAST (period AS tstzrange) WITH FUNCTION tstzrange(period);

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION lower(period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Period_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper(period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Period_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower_inc(period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper_inc(period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(period)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Period_duration'
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

CREATE FUNCTION period_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Period_sel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Span_joinsel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION period_eq(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_ne(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_lt(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_le(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_ge(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gt(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Period_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_cmp(period, period)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Period_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = period_eq,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = period_ne,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = period_lt,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = period_sel, JOIN = period_joinsel
);
CREATE OPERATOR <= (
  PROCEDURE = period_le,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = period_joinsel
);
CREATE OPERATOR >= (
  PROCEDURE = period_ge,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = period_joinsel
);
CREATE OPERATOR > (
  PROCEDURE = period_gt,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = period_joinsel
);

CREATE OPERATOR CLASS period_ops
  DEFAULT FOR TYPE period USING btree  AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  period_cmp(period, period);

/******************************************************************************/

CREATE FUNCTION period_hash(period)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Period_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION period_hash_extended(period, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Period_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS hash_period_ops
  DEFAULT FOR TYPE period USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   period_hash(period),
    FUNCTION    2   period_hash_extended(period, bigint);

/******************************************************************************/
