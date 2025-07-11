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
 * @file
 * @brief Functions for spatiotemporal bounding boxes
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

-- Input/output in WKB and HexWKB representation

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

CREATE FUNCTION asBinary(stbox, endianenconding text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Stbox_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(stbox, endianenconding text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Stbox_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION stboxX(float, float, float, float, srid integer DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_x'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxZ(float, float, float, float, float, float,
    srid integer DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_z'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxT(timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxT(tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxXT(float, float, float, float, timestamptz,
    srid integer DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_xt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxXT(float, float, float, float, tstzspan,
    srid integer DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_xt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxZT(float, float, float, float, float, float,
    timestamptz, srid integer DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION stboxZT(float, float, float, float, float, float,
    tstzspan, srid integer DEFAULT 0)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstboxT(timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstboxT(tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_t'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstboxZ(float, float, float, float, float, float,
    srid integer DEFAULT 4326)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_z'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstboxZT(float, float, float, float, float, float,
    timestamptz, srid integer DEFAULT 4326)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION geodstboxZT(float, float, float, float, float, float,
    tstzspan, srid integer DEFAULT 4326)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geodstbox_constructor_zt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************/

CREATE FUNCTION stbox(geometry, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_timestamptz_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geography, timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_timestamptz_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geometry, tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_tstzspan_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geography, tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_tstzspan_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversion
 *****************************************************************************/

CREATE FUNCTION stbox(box2d)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Box2d_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(box3d)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Box3d_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geometry)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geography)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Geo_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geomset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Spatialset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(geogset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Spatialset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(timestamptz)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Timestamptz_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(tstzset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tstzset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(tstzspan)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tstzspan_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox(tstzspanset)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Tstzspanset_to_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (box2d AS stbox) WITH FUNCTION stbox(box2d);
CREATE CAST (box3d AS stbox) WITH FUNCTION stbox(box3d);
CREATE CAST (geometry AS stbox) WITH FUNCTION stbox(geometry);
CREATE CAST (geography AS stbox) WITH FUNCTION stbox(geography);
CREATE CAST (geomset AS stbox) WITH FUNCTION stbox(geomset);
CREATE CAST (geogset AS stbox) WITH FUNCTION stbox(geogset);
CREATE CAST (timestamptz AS stbox) WITH FUNCTION stbox(timestamptz);
CREATE CAST (tstzset AS stbox) WITH FUNCTION stbox(tstzset);
CREATE CAST (tstzspan AS stbox) WITH FUNCTION stbox(tstzspan);
CREATE CAST (tstzspanset AS stbox) WITH FUNCTION stbox(tstzspanset);

/*****************************************************************************/

CREATE FUNCTION box2d(stbox)
  RETURNS box2d
  AS 'MODULE_PATHNAME', 'Stbox_to_box2d'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION box3d(stbox)
  RETURNS box3d
  AS 'MODULE_PATHNAME', 'Stbox_to_box3d'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geometry(stbox)
  RETURNS geometry
  AS 'MODULE_PATHNAME', 'Stbox_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION geography(stbox)
  RETURNS geography
  AS 'MODULE_PATHNAME', 'Stbox_to_geo'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timeSpan(stbox)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Stbox_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (stbox AS box2d) WITH FUNCTION box2d(stbox);
CREATE CAST (stbox AS box3d) WITH FUNCTION box3d(stbox);
CREATE CAST (stbox AS geometry) WITH FUNCTION geometry(stbox);
CREATE CAST (stbox AS geography) WITH FUNCTION geography(stbox);
CREATE CAST (stbox AS tstzspan) WITH FUNCTION timeSpan(stbox);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION hasX(stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_hasx'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasZ(stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_hasz'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasT(stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_hast'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION isGeodetic(stbox)
  RETURNS boolean
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
CREATE FUNCTION TminInc(stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_tmin_inc'
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
CREATE FUNCTION TmaxInc(stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Stbox_tmax_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION area(stbox, spheroid bool DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_area'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION volume(stbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_volume'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION perimeter(stbox, spheroid bool DEFAULT true)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Stbox_perimeter'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION shiftTime(stbox, interval)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scaleTime(stbox, interval)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftScaleTime(stbox, interval, interval)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION getSpace(stbox)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_get_space'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandSpace(stbox, float)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_expand_space'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTime(stbox, interval)
  RETURNS stbox
  AS 'MODULE_PATHNAME', 'Stbox_expand_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectively functions for operators
 *****************************************************************************/

CREATE FUNCTION tspatial_sel(internal, oid, internal, integer)
  RETURNS float
AS 'MODULE_PATHNAME', 'Tspatial_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tspatial_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
AS 'MODULE_PATHNAME', 'Tspatial_joinsel'
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
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = stbox_contained,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = @>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = stbox_overlaps,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = &&,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = stbox_same,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = ~=,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = stbox_adjacent,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = -|-,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);

/*****************************************************************************
* Position operators
*****************************************************************************/

CREATE FUNCTION stbox_left(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overleft(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_right(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overright(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_below(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Below_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overbelow(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbelow_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_above(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Above_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overabove(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overabove_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_before(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overbefore(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_after(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overafter(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_front(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Front_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overfront(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overfront_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_back(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Back_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION stbox_overback(stbox, stbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overback_stbox_stbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = stbox_left,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = >>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = stbox_overleft,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_right,
  COMMUTATOR = <<,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = stbox_overright,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<| (
  PROCEDURE = stbox_below,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = |>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<| (
  PROCEDURE = stbox_overbelow,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |>> (
  PROCEDURE = stbox_above,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <<|,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR |&> (
  PROCEDURE = stbox_overabove,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <</ (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_front,
  COMMUTATOR = />>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &</ (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_overfront,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR />> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_back,
  COMMUTATOR = <</,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR /&> (
  LEFTARG = stbox, RIGHTARG = stbox,
  PROCEDURE = stbox_overback,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = stbox_before,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = #>>,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = stbox_overbefore,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = stbox_after,
  LEFTARG = stbox, RIGHTARG = stbox,
  COMMUTATOR = <<#,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = stbox_overafter,
  LEFTARG = stbox, RIGHTARG = stbox,
  RESTRICT = tspatial_sel, JOIN = tspatial_joinsel
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
 * Split functions
 *****************************************************************************/

CREATE FUNCTION quadSplit(stbox)
  RETURNS stbox[]
  AS 'MODULE_PATHNAME', 'Stbox_quad_split'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

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

CREATE OPERATOR CLASS stbox_btree_ops
  DEFAULT FOR TYPE stbox USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  stbox_cmp(stbox, stbox);

/*****************************************************************************/
