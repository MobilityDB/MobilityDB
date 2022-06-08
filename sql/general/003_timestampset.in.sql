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
 * timestampset.sql
 * Functions for set of timestamps.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE timestampset;

CREATE FUNCTION timestampset_in(cstring)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_out(timestampset)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Timestampset_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_recv(internal)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_send(timestampset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Timestampset_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_analyze(internal)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Timestampset_analyze'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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

CREATE FUNCTION timestampsetFromBinary(bytea)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampsetFromHexWKB(text)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(timestampset)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Timestampset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(timestampset, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Timestampset_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(timestampset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Timestampset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(timestampset, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Timestampset_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor
 ******************************************************************************/

CREATE FUNCTION timestampset(timestamptz[])
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestampset_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Casting
 ******************************************************************************/

CREATE FUNCTION timestampset(timestamptz)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Timestamp_to_timestampset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(timestampset)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Timestampset_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestamptz AS timestampset) WITH FUNCTION timestampset(timestamptz);
CREATE CAST (timestampset AS period) WITH FUNCTION period(timestampset);

/******************************************************************************
 * Functions
 ******************************************************************************/

CREATE FUNCTION memSize(timestampset)
  RETURNS int
  AS 'MODULE_PATHNAME', 'Timestampset_mem_size'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timespan(timestampset)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Timestampset_timespan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION numTimestamps(timestampset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Timestampset_num_timestamps'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startTimestamp(timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Timestampset_start_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endTimestamp(timestampset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Timestampset_end_timestamp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampN(timestampset, integer)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Timestampset_timestamp_n'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestamps(timestampset)
  RETURNS timestamptz[]
  AS 'MODULE_PATHNAME', 'Timestampset_timestamps'
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
 * Comparison functions and B-tree indexing
 ******************************************************************************/

CREATE FUNCTION timestampset_eq(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Timestampset_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_ne(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Timestampset_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_lt(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Timestampset_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_le(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Timestampset_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_ge(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Timestampset_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_gt(timestampset, timestampset)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Timestampset_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timestampset_cmp(timestampset, timestampset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Timestampset_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_lt,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <= (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_le,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR >= (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_ge,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR > (
  LEFTARG = timestampset, RIGHTARG = timestampset,
  PROCEDURE = timestampset_gt,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = period_sel, JOIN = span_joinsel
);

CREATE OPERATOR CLASS timestampset_ops
  DEFAULT FOR TYPE timestampset USING btree AS
    OPERATOR  1  <,
    OPERATOR  2  <=,
    OPERATOR  3  =,
    OPERATOR  4  >=,
    OPERATOR  5  >,
    FUNCTION  1  timestampset_cmp(timestampset, timestampset);

/******************************************************************************/

CREATE FUNCTION timestampset_hash(timestampset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Timestampset_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION timestampset_hash_extended(timestampset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Timestampset_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS timestampset_hash_ops
  DEFAULT FOR TYPE timestampset USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   timestampset_hash(timestampset),
    FUNCTION    2   timestampset_hash_extended(timestampset, bigint);

/******************************************************************************/
