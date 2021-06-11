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

/**
 * tnpoint_static.sql
 * Network-based static point and segment types
 */

CREATE TYPE npoint;
CREATE TYPE nsegment;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION npoint_in(cstring)
  RETURNS npoint
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION npoint_out(npoint)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION npoint_recv(internal)
  RETURNS npoint
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION npoint_send(npoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE npoint (
  internallength = 16,
  input = npoint_in,
  output = npoint_out,
  receive = npoint_recv,
  send = npoint_send,
  alignment = double
);

CREATE FUNCTION nsegment_in(cstring)
  RETURNS nsegment
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION nsegment_out(nsegment)
  RETURNS cstring
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION nsegment_recv(internal)
  RETURNS nsegment
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION nsegment_send(nsegment)
  RETURNS bytea
  AS 'MODULE_PATHNAME'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE nsegment (
  internallength = 24,
  input = nsegment_in,
  output = nsegment_out,
  receive = nsegment_recv,
  send = nsegment_send,
  alignment = double
);

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION npoint(bigint, double precision)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'npoint_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment(bigint, double precision DEFAULT 0, double precision DEFAULT 1)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'nsegment_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment(npoint)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'nsegment_from_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS nsegment) WITH FUNCTION nsegment(npoint);

/*****************************************************************************
 * Accessing values
 *****************************************************************************/

CREATE FUNCTION route(npoint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'npoint_route'
  LANGUAGE C IMMUTABLE STRICT;

-- position is a reserved word in SQL
CREATE FUNCTION getPosition(npoint)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'npoint_position'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION srid(npoint)
  RETURNS int
  AS 'MODULE_PATHNAME', 'npoint_srid'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION route(nsegment)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'nsegment_route'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION startPosition(nsegment)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'nsegment_start_position'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION endPosition(nsegment)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'nsegment_end_position'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION srid(nsegment)
  RETURNS int
  AS 'MODULE_PATHNAME', 'nsegment_srid'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

CREATE FUNCTION setPrecision(npoint, int)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'npoint_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setPrecision(nsegment, int)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'nsegment_set_precision'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversions between network and space
 *****************************************************************************/

CREATE FUNCTION geometry(npoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'npoint_as_geom'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION npoint(geometry)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'geom_as_npoint'
  LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (npoint AS geometry) WITH FUNCTION geometry(npoint);
CREATE CAST (geometry AS npoint) WITH FUNCTION npoint(geometry);

CREATE FUNCTION geometry(nsegment)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'nsegment_as_geom'
  LANGUAGE C IMMUTABLE STRICT;
CREATE FUNCTION nsegment(geometry)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'geom_as_nsegment'
  LANGUAGE C IMMUTABLE STRICT;

CREATE CAST (nsegment AS geometry) WITH FUNCTION geometry(nsegment);
CREATE CAST (geometry AS nsegment) WITH FUNCTION nsegment(geometry);

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION npoint_eq(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'npoint_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npoint_ne(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'npoint_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npoint_lt(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'npoint_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npoint_le(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'npoint_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npoint_ge(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'npoint_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npoint_gt(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'npoint_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION npoint_cmp(npoint, npoint)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'npoint_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = npoint_eq,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = npoint_ne,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = npoint_lt,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = npoint_le,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = >=, NEGATOR = >,
#if MOBDB_PGSQL_VERSION >= 110000
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
#else
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
#endif
);
CREATE OPERATOR >= (
  PROCEDURE = npoint_ge,
  LEFTARG = npoint, RIGHTARG = npoint,
#if MOBDB_PGSQL_VERSION >= 110000
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
#else
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
#endif
);
CREATE OPERATOR > (
  PROCEDURE = npoint_gt,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS npoint_ops
  DEFAULT FOR TYPE npoint USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 npoint_cmp(npoint, npoint);

/******************************************************************************/

CREATE FUNCTION nsegment_eq(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'nsegment_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment_ne(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'nsegment_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment_lt(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'nsegment_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment_le(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'nsegment_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment_ge(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'nsegment_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment_gt(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'nsegment_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment_cmp(nsegment, nsegment)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'nsegment_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = nsegment_eq,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = nsegment_ne,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = nsegment_lt,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = nsegment_le,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = >=, NEGATOR = >,
#if MOBDB_PGSQL_VERSION >= 110000
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
#else
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
#endif
);
CREATE OPERATOR >= (
  PROCEDURE = nsegment_ge,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = <=, NEGATOR = <,
#if MOBDB_PGSQL_VERSION >= 110000
  RESTRICT = scalargesel, JOIN = scalargejoinsel
#else
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
#endif  
);
CREATE OPERATOR > (
  PROCEDURE = nsegment_gt,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS nsegment_ops
  DEFAULT FOR TYPE nsegment USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 nsegment_cmp(nsegment, nsegment);

/******************************************************************************/
