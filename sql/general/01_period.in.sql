/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 *
 * Copyright (c) 2016-2021, Université libre de Bruxelles and MobilityDB
 * contributors
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

CREATE TYPE period;

CREATE FUNCTION period_in(cstring)
  RETURNS period
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_out(period)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_recv(internal)
  RETURNS period
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_send(period)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION period_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE period (
  internallength = 24,
  input = period_in,
  output = period_out,
  receive = period_recv,
  send = period_send,
  alignment = double,
  analyze = period_analyze
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION period(timestamptz, timestamptz)
  RETURNS period
  AS 'MODULE_PATHNAME', 'period_constructor2'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION period(timestamptz, timestamptz, boolean, boolean)
  RETURNS period
  AS 'MODULE_PATHNAME', 'period_constructor4'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION period(timestamptz)
  RETURNS period
  AS 'MODULE_PATHNAME', 'timestamp_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tstzrange)
  RETURNS period
  AS 'MODULE_PATHNAME', 'tstzrange_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tstzrange(period)
  RETURNS tstzrange
  AS 'MODULE_PATHNAME', 'period_to_tstzrange'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS period) WITH FUNCTION period(timestamptz) AS IMPLICIT;
CREATE CAST (tstzrange AS period) WITH FUNCTION period(tstzrange);
CREATE CAST (period AS tstzrange) WITH FUNCTION tstzrange(period);

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION lower(period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'period_lower'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper(period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'period_upper'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION lower_inc(period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_lower_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION upper_inc(period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_upper_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION duration(period)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'period_duration'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shift(period, interval)
  RETURNS period
  AS 'MODULE_PATHNAME', 'period_shift'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION periodsel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION period_eq(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_ne(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_lt(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_le(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_ge(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_gt(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'period_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period_cmp(period, period)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'period_cmp'
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
  RESTRICT = periodsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = period_le,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = >=, NEGATOR = >,
#if MOBDB_PGSQL_VERSION >= 110000
  RESTRICT = periodsel, JOIN = scalarlejoinsel
#else
  RESTRICT = periodsel, JOIN = scalarltjoinsel
#endif
);
CREATE OPERATOR >= (
  PROCEDURE = period_ge,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <=, NEGATOR = <,
#if MOBDB_PGSQL_VERSION >= 110000
  RESTRICT = periodsel, JOIN = scalargejoinsel
#else
  RESTRICT = periodsel, JOIN = scalargtjoinsel
#endif
);
CREATE OPERATOR > (
  PROCEDURE = period_gt,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = periodsel, JOIN = scalargtjoinsel
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
  AS 'MODULE_PATHNAME', 'period_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

#if MOBDB_PGSQL_VERSION >= 110000
CREATE FUNCTION period_hash_extended(period)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'period_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
#endif

CREATE OPERATOR CLASS hash_period_ops
  DEFAULT FOR TYPE period USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   period_hash(period);

/******************************************************************************/
