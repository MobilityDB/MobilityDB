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
 * tbox.sql
 * Functions for temporal bounding boxes.
 */

/******************************************************************************
 * Input/Output
 ******************************************************************************/

CREATE TYPE tbox;

CREATE FUNCTION tbox_in(cstring)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_in'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_out(tbox)
  RETURNS cstring
  AS 'MODULE_PATHNAME', 'Tbox_out'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_recv(internal)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_recv'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_send(tbox)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tbox_send'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE TYPE tbox (
  internallength = 56,
  input = tbox_in,
  output = tbox_out,
  receive = tbox_recv,
  send = tbox_send,
  storage = plain,
  alignment = double
);

-- Input/output in WKB and HexWKB format

CREATE FUNCTION tboxFromBinary(bytea)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxFromHexWKB(text)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tbox, maxdecimaldigits int4 DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tbox_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tbox)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tbox_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asBinary(tbox, endianenconding text)
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tbox_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tbox)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tbox_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION asHexWKB(tbox, endianenconding text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tbox_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructors
 ******************************************************************************/

CREATE FUNCTION tbox(integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Int_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Float_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(numeric)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numeric_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (integer AS tbox) WITH FUNCTION tbox(integer);
CREATE CAST (float AS tbox) WITH FUNCTION tbox(float);
CREATE CAST (numeric AS tbox) WITH FUNCTION tbox(numeric);
CREATE CAST (timestamptz AS tbox) WITH FUNCTION tbox(timestamptz);
CREATE CAST (period AS tbox) WITH FUNCTION tbox(period);
CREATE CAST (intspan AS tbox) WITH FUNCTION tbox(intspan);
CREATE CAST (floatspan AS tbox) WITH FUNCTION tbox(floatspan);

CREATE FUNCTION tbox(integer, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Int_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspan, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Float_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspan, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_timestamp_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(integer, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Int_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspan, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Float_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspan, period)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_period_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Casting
 *****************************************************************************/

CREATE FUNCTION tbox(timestampset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Timestampset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(periodset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Periodset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (timestampset AS tbox) WITH FUNCTION tbox(timestampset);
CREATE CAST (periodset AS tbox) WITH FUNCTION tbox(periodset);

/*****************************************************************************/

CREATE FUNCTION floatspan(tbox)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Tbox_to_floatspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION period(tbox)
  RETURNS period
  AS 'MODULE_PATHNAME', 'Tbox_to_period'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbox AS floatspan) WITH FUNCTION floatspan(tbox);
CREATE CAST (tbox AS period) WITH FUNCTION period(tbox);


/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION hasX(tbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Tbox_hasx'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasT(tbox)
  RETURNS bool
  AS 'MODULE_PATHNAME', 'Tbox_hast'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION Xmin(tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tbox_xmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Xmax(tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tbox_xmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Tmin(tbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tbox_tmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION Tmax(tbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tbox_tmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Modification functions
 *****************************************************************************/

CREATE FUNCTION expandValue(tbox, float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_expand_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTemporal(tbox, interval)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_expand_temporal'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION round(tbox, integer DEFAULT 0)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_round'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Selectivity functions
 *****************************************************************************/

CREATE FUNCTION tnumber_sel(internal, oid, internal, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tnumber_sel'
  LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION tnumber_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tnumber_joinsel'
  LANGUAGE C IMMUTABLE STRICT;

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

CREATE FUNCTION tbox_contains(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_contained(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_overlaps(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_same(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_adjacent(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tbox_contains,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tbox_contained,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tbox_overlaps,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tbox_same,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tbox_adjacent,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Position operators
 *****************************************************************************/

CREATE FUNCTION temporal_left(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overleft(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_right(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overright(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_before(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overbefore(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_after(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION temporal_overafter(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = temporal_left,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = temporal_overleft,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = temporal_right,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = temporal_overright,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = temporal_before,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = temporal_overbefore,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = temporal_after,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = temporal_overafter,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION tbox_union(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Union_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_intersection(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Intersection_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tbox_union,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = tbox_intersection,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = *
);

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

CREATE FUNCTION tbox_extent_transfn(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION tbox_extent_combinefn(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tbox) (
  SFUNC = tbox_extent_transfn,
  STYPE = tbox,
  COMBINEFUNC = tbox_extent_combinefn,
  PARALLEL = safe
);

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION tbox_eq(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_ne(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_lt(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_le(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_ge(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_gt(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox_cmp(tbox, tbox)
  RETURNS int4
  AS 'MODULE_PATHNAME', 'Tbox_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = tbox_eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = tbox_ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = tbox_lt,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = tbox_le,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = tbox_ge,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
  PROCEDURE = tbox_gt,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS tbox_ops
  DEFAULT FOR TYPE tbox USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  tbox_cmp(tbox, tbox);

/*****************************************************************************/
