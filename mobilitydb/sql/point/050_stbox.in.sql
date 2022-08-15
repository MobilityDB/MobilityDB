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
 * stbox.sql
 * Functions for spatiotemporal bounding box.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE stbox;

CREATE FUNCTION stbox_in(cstring)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_out(stbox)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Stbox_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_recv(internal)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_send(stbox)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Stbox_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE stbox (
  internallength = 80,
  input = stbox_in,
  output = stbox_out,
  receive = stbox_recv,
  send = stbox_send,
  storage = plain,
  alignment = double
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION stboxFromBinary(bytea)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stboxFromHexWKB(text)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(stbox, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Stbox_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(stbox)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Stbox_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(stbox, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Stbox_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(stbox)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Stbox_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(stbox, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Stbox_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to timestamptz is needed */
CREATE FUNCTION stbox_t(period)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox(float8, float8, float8, float8, srid int DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox_z(float8, float8, float8, float8, float8, float8,
    srid int DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_z'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox_t(period, float8, float8, float8, float8,
    srid int DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stbox_zt(period, float8, float8, float8, float8, float8,
    float8, srid int DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/* The names of the SQL and C functions are different, otherwise there is
 * ambiguity and explicit casting of the arguments to ::timestamptz is needed */
CREATE FUNCTION geodstbox_t(period)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstbox_z(float8, float8, float8, float8, float8, float8,
    srid int DEFAULT 4326)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_z'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstbox_zt(period, float8, float8, float8, float8,
    float8, float8, srid int DEFAULT 4326)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION stbox(geometry)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geography)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Timestamp_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(timestampset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Timestampset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(period)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Period_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(periodset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Periodset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geometry, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_timestamp_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geography, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_timestamp_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geometry, period)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_period_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geography, period)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_period_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (geometry AS stbox) WITH FUNCTION stbox(geometry);
CREATE CAST (geography AS stbox) WITH FUNCTION stbox(geography);
CREATE CAST (timestamptz AS stbox) WITH FUNCTION stbox(timestamptz);
CREATE CAST (timestampset AS stbox) WITH FUNCTION stbox(timestampset);
CREATE CAST (period AS stbox) WITH FUNCTION stbox(period);
CREATE CAST (periodset AS stbox) WITH FUNCTION stbox(periodset);

/*****************************************************************************/

CREATE FUNCTION period(stbox)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Stbox_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geometry(stbox)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Stbox_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geography(stbox)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Stbox_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (stbox AS period) WITH FUNCTION period(stbox);
CREATE CAST (stbox AS geometry) WITH FUNCTION geometry(stbox);
CREATE CAST (stbox AS geography) WITH FUNCTION geography(stbox);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION hasX(stbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_hasx'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasZ(stbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_hasz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasT(stbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_hast'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION isGeodetic(stbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Stbox_isgeodetic'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION Xmin(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_xmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Ymin(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_ymin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Zmin(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_zmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Tmin(stbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Stbox_tmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION Xmax(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_xmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Ymax(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_ymax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Zmax(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_zmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Tmax(stbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Stbox_tmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Functions for expanding the bounding box
 *****************************************************************************/

CREATE FUNCTION expandSpatial(stbox, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_expand_spatial'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTemporal(stbox, interval)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_expand_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectively functions for operators
 *****************************************************************************/

CREATE FUNCTION tpoint_sel(internal, oid, internal, integer)
  RETURNS float
AS 'MODULE_PATHNAME', 'Tpoint_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tpoint_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
AS 'MODULE_PATHNAME', 'Tpoint_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
* Topological operators
*****************************************************************************/

CREATE FUNCTION stbox_contains(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_contained(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overlaps(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_same(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_adjacent(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = stbox_contains,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <@,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = stbox_contained,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = stbox_overlaps,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = stbox_same,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = stbox_adjacent,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
* Position operators
*****************************************************************************/

CREATE FUNCTION temporal_left(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_below(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbelow(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_above(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overabove(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_front(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overfront(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_back(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overback(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = temporal_left,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = >>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = temporal_overleft,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = temporal_overright,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<| (
  PROCEDURE = temporal_below,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = |>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<| (
  PROCEDURE = temporal_overbelow,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |>> (
  PROCEDURE = temporal_above,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <<|,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR |&> (
  PROCEDURE = temporal_overabove,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = temporal_front,
  COMMUTATOR = />>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = temporal_overfront,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = temporal_back,
  COMMUTATOR = <</,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = temporal_overback,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = #>>,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <<#,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tpoint_sel, JOIN = tpoint_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION stbox_union(stbox, stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Union_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_intersection(stbox, stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Intersection_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = stbox_union,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = stbox_intersection,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = *
);

/*****************************************************************************
 * Extent aggreation
 *****************************************************************************/

CREATE FUNCTION stbox_extent_transfn(stbox, stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION stbox_extent_combinefn(stbox, stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(stbox) (
  SFUNC = stbox_extent_transfn,
  STYPE = stbox,
  COMBINEFUNC = stbox_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION stbox_eq(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_ne(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_lt(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_le(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_ge(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_gt(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_cmp(stbox, stbox)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Stbox_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = stbox_lt,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = stbox_le,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = stbox_ge,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
  PROCEDURE = stbox_gt,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS stbox_ops
  DEFAULT FOR TYPE stbox USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  stbox_cmp(stbox, stbox);

/*****************************************************************************/
