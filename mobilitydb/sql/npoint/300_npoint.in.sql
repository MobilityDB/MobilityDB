/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2025, PostGIS contributors
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
 * @file
 * @brief Network-based static point and segment types
 */

CREATE TYPE npoint;
CREATE TYPE nsegment;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION npoint_in(cstring)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npoint_out(npoint)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Npoint_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npoint_recv(internal)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npoint_send(npoint)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Npoint_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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
  AS 'MODULE_PATHNAME', 'Nsegment_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment_out(nsegment)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Nsegment_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment_recv(internal)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'Nsegment_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment_send(nsegment)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Nsegment_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE nsegment (
  internallength = 24,
  input = nsegment_in,
  output = nsegment_out,
  receive = nsegment_recv,
  send = nsegment_send,
  alignment = double
);

-- GENERATED-REPRESENTATIONS-BEGIN npoint_base — tools/codegen/inherited/generate.py from templates/representations.sql.tmpl;
-- DO NOT EDIT BY HAND; edit the template + manifest.d/representation_families.yaml and re-run.
/*****************************************************************************
 * Input/output from (E)WKT, (E)WKB and HexEWKB representation
 *****************************************************************************/

CREATE FUNCTION npointFromText(text)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npointFromEWKT(text)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_from_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npointFromBinary(bytea)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npointFromEWKB(bytea)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npointFromHexEWKB(text)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION asText(npoint, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Npoint_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(npoint[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(npoint, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Npoint_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(npoint[], maxdecimaldigits integer DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(npoint, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Npoint_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKB(npoint, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Npoint_as_ewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(npoint, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Npoint_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexEWKB(npoint, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Npoint_as_hexewkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- GENERATED-REPRESENTATIONS-END npoint_base

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION npoint(bigint, double precision)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment(bigint, double precision DEFAULT 0, double precision DEFAULT 1)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'Nsegment_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION nsegment(npoint)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'Npoint_to_nsegment'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS nsegment) WITH FUNCTION nsegment(npoint);

/*****************************************************************************
 * Conversion between network and space
 *****************************************************************************/

CREATE FUNCTION geometry(npoint)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Npoint_to_geompoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION npoint(geometry)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Geompoint_to_npoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS geometry) WITH FUNCTION geometry(npoint);
CREATE CAST (geometry AS npoint) WITH FUNCTION npoint(geometry);

CREATE FUNCTION geometry(nsegment)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Nsegment_to_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION nsegment(geometry)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'Geom_to_nsegment'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (nsegment AS geometry) WITH FUNCTION geometry(nsegment);
CREATE CAST (geometry AS nsegment) WITH FUNCTION nsegment(geometry);

CREATE FUNCTION stbox(npoint)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Npoint_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(nsegment)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Nsegment_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(npoint, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Npoint_timestamptz_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(npoint, tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Npoint_tstzspan_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (npoint AS stbox) WITH FUNCTION stbox(npoint);
CREATE CAST (nsegment AS stbox) WITH FUNCTION stbox(nsegment);

/*****************************************************************************
 * Accessors
 *****************************************************************************/

CREATE FUNCTION route(npoint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Npoint_route'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

-- position is a reserved word in SQL
CREATE FUNCTION getPosition(npoint)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'Npoint_position'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION SRID(npoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Npoint_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION route(nsegment)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Nsegment_route'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION startPosition(nsegment)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'Nsegment_start_position'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION endPosition(nsegment)
  RETURNS double precision
  AS 'MODULE_PATHNAME', 'Nsegment_end_position'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION SRID(nsegment)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Nsegment_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

CREATE FUNCTION round(npoint, integer DEFAULT 0)
  RETURNS npoint
  AS 'MODULE_PATHNAME', 'Npoint_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION round(nsegment, integer DEFAULT 0)
  RETURNS nsegment
  AS 'MODULE_PATHNAME', 'Nsegment_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Comparisons
 ******************************************************************************/

CREATE FUNCTION eq(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(npoint, npoint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Npoint_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(npoint, npoint)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Npoint_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = eq,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = ne,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = lt,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = le,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = ge,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
  PROCEDURE = gt,
  LEFTARG = npoint, RIGHTARG = npoint,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS npoint_btree_ops
  DEFAULT FOR TYPE npoint USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 cmp(npoint, npoint);

/******************************************************************************/

CREATE FUNCTION eq(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Nsegment_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Nsegment_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Nsegment_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Nsegment_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Nsegment_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(nsegment, nsegment)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Nsegment_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(nsegment, nsegment)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Nsegment_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = eq,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = ne,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = lt,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = le,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = ge,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
  PROCEDURE = gt,
  LEFTARG = nsegment, RIGHTARG = nsegment,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS nsegment_btree_ops
  DEFAULT FOR TYPE nsegment USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 cmp(nsegment, nsegment);

/******************************************************************************/
