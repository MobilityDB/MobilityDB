

/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2025, Université libre de Bruxelles and MobilityDB
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
 * @file cbuffer.sql
 * @brief Static circular buffer type
 */

CREATE TYPE cbuffer;

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE FUNCTION cbuffer_in(cstring)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_in'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION cbuffer_out(cbuffer)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Cbuffer_out'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION cbuffer_recv(internal)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_recv'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION cbuffer_send(cbuffer)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Cbuffer_send'
  LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE cbuffer (
  internallength = variable,
  input = cbuffer_in,
  output = cbuffer_out,
  receive = cbuffer_recv,
  send = cbuffer_send,
  storage = plain,
  alignment = double
);

-- Input/output in WKT, WKB and HexWKB representation

CREATE FUNCTION asText(cbuffer, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Cbuffer_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asText(cbuffer[], maxdecimaldigits int4 DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asEWKT(cbuffer, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Cbuffer_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asEWKT(cbuffer[], maxdecimaldigits int4 DEFAULT 15)
  RETURNS text[]
  AS 'MODULE_PATHNAME', 'Spatialarr_as_ewkt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION cbuffer(geometry, double precision)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

CREATE FUNCTION geometry(cbuffer)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Cbuffer_to_geom'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffer(geometry)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Geom_to_cbuffer'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (cbuffer AS geometry) WITH FUNCTION geometry(cbuffer);
CREATE CAST (geometry AS cbuffer) WITH FUNCTION cbuffer(geometry);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION point(cbuffer)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Cbuffer_point'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION radius(cbuffer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Cbuffer_radius'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION round(cbuffer, integer DEFAULT 0)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * SRID functions
 *****************************************************************************/

CREATE FUNCTION SRID(cbuffer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Cbuffer_srid'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION setSRID(cbuffer, integer)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_set_srid'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transform(cbuffer, integer)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_transform'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION transformPipeline(cbuffer, text, srid integer DEFAULT 0,
    is_forward boolean DEFAULT true)
  RETURNS cbuffer
  AS 'MODULE_PATHNAME', 'Cbuffer_transform_pipeline'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Spatial relationships
 *****************************************************************************/

CREATE FUNCTION cbuffer_contains(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_contains'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffer_covers(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_covers'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffer_disjoint(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_disjoint'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffer_intersects(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_intersects'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffer_touches(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_touches'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION cbuffer_dwithin(cbuffer, cbuffer, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_dwithin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Same
 *****************************************************************************/

CREATE FUNCTION cbuffer_same(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_same'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR ~= (
  PROCEDURE = cbuffer_same,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/******************************************************************************
 * Comparisons
 ******************************************************************************/

CREATE FUNCTION cbuffer_eq(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_ne(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_lt(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_le(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_ge(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_gt(cbuffer, cbuffer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Cbuffer_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_cmp(cbuffer, cbuffer)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Cbuffer_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  PROCEDURE = cbuffer_eq,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  PROCEDURE = cbuffer_ne,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = cbuffer_lt,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = scalarltsel, JOIN = scalarltjoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = cbuffer_le,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = scalarlesel, JOIN = scalarlejoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = cbuffer_ge,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = scalargesel, JOIN = scalargejoinsel
);
CREATE OPERATOR > (
  PROCEDURE = cbuffer_gt,
  LEFTARG = cbuffer, RIGHTARG = cbuffer,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = scalargtsel, JOIN = scalargtjoinsel
);

CREATE OPERATOR CLASS cbuffer_btree_ops
  DEFAULT FOR TYPE cbuffer USING btree AS
  OPERATOR  1 < ,
  OPERATOR  2 <= ,
  OPERATOR  3 = ,
  OPERATOR  4 >= ,
  OPERATOR  5 > ,
  FUNCTION  1 cbuffer_cmp(cbuffer, cbuffer);

/******************************************************************************/

CREATE FUNCTION cbuffer_hash(cbuffer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Cbuffer_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cbuffer_hash_extended(cbuffer, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Cbuffer_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS cbuffer_hash_ops
  DEFAULT FOR TYPE cbuffer USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   cbuffer_hash(cbuffer),
    FUNCTION    2   cbuffer_hash_extended(cbuffer, bigint);

/******************************************************************************/
