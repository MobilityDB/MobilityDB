/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2026, PostGIS contributors
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
 * @brief Functions for temporal bounding boxes
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

-- Input/output in WKB and HexWKB representation

CREATE FUNCTION tboxFromBinary(bytea)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_from_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxFromText(text)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_from_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tboxFromHexWKB(text)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_from_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asText(tbox, maxdecimaldigits integer DEFAULT 15)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tbox_as_text'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asBinary(tbox, endian text DEFAULT '')
  RETURNS bytea
  AS 'MODULE_PATHNAME', 'Tbox_as_wkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION asHexWKB(tbox, endian text DEFAULT '')
  RETURNS text
  AS 'MODULE_PATHNAME', 'Tbox_as_hexwkb'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Constructor functions
 ******************************************************************************/

CREATE FUNCTION tbox(bigint, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(integer, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(bigintspan, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numspan_timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspan, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numspan_timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspan, timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numspan_timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tbox(bigint, tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_tstzspan_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(integer, tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_tstzspan_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(bigintspan, tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numspan_tstzspan_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspan, tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numspan_tstzspan_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float, tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_tstzspan_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspan, tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numspan_tstzspan_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Conversion functions
 *****************************************************************************/

CREATE FUNCTION tbox(bigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Number_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(numeric)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Numeric_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(timestamptz)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Timestamptz_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tbox(bigintset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Set_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Set_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Set_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(tstzset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Set_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tbox(bigintspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(tstzspan)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Span_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tbox(bigintspanset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Spanset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(intspanset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Spanset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(floatspanset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Spanset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tbox(tstzspanset)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Spanset_to_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (bigint AS tbox) WITH FUNCTION tbox(bigint);
CREATE CAST (integer AS tbox) WITH FUNCTION tbox(integer);
CREATE CAST (float AS tbox) WITH FUNCTION tbox(float);
CREATE CAST (numeric AS tbox) WITH FUNCTION tbox(numeric);
CREATE CAST (timestamptz AS tbox) WITH FUNCTION tbox(timestamptz);

CREATE CAST (bigintset AS tbox) WITH FUNCTION tbox(bigintset);
CREATE CAST (intset AS tbox) WITH FUNCTION tbox(intset);
CREATE CAST (floatset AS tbox) WITH FUNCTION tbox(floatset);
CREATE CAST (tstzset AS tbox) WITH FUNCTION tbox(tstzset);

CREATE CAST (bigintspan AS tbox) WITH FUNCTION tbox(bigintspan);
CREATE CAST (intspan AS tbox) WITH FUNCTION tbox(intspan);
CREATE CAST (floatspan AS tbox) WITH FUNCTION tbox(floatspan);
CREATE CAST (tstzspan AS tbox) WITH FUNCTION tbox(tstzspan);

CREATE CAST (bigintspanset AS tbox) WITH FUNCTION tbox(bigintspanset);
CREATE CAST (intspanset AS tbox) WITH FUNCTION tbox(intspanset);
CREATE CAST (floatspanset AS tbox) WITH FUNCTION tbox(floatspanset);
CREATE CAST (tstzspanset AS tbox) WITH FUNCTION tbox(tstzspanset);

CREATE FUNCTION bigintspan(tbox)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Tbox_to_bigintspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION intspan(tbox)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Tbox_to_intspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION floatspan(tbox)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Tbox_to_floatspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION timeSpan(tbox)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Tbox_to_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE CAST (tbox AS bigintspan) WITH FUNCTION bigintspan(tbox);
CREATE CAST (tbox AS intspan) WITH FUNCTION intspan(tbox);
CREATE CAST (tbox AS floatspan) WITH FUNCTION floatspan(tbox);
CREATE CAST (tbox AS tstzspan) WITH FUNCTION timeSpan(tbox);

/*****************************************************************************
 * Accessor functions
 *****************************************************************************/

CREATE FUNCTION hasX(tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_hasx'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION hasT(tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_hast'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION xMin(tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tbox_xmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION xMinInc(tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_xmin_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION xMax(tbox)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tbox_xmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION xMaxInc(tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_xmax_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMin(tbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tbox_tmin'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMinInc(tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_tmin_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMax(tbox)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Tbox_tmax'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tMaxInc(tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_tmax_inc'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Transformation functions
 *****************************************************************************/

CREATE FUNCTION shiftValue(tbox, bigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftValue(tbox, integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftValue(tbox, float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftTime(tbox, interval)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION scaleValue(tbox, bigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_scale_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION scaleValue(tbox, integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_scale_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION scaleValue(tbox, float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_scale_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION scaleTime(tbox, interval)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION shiftScaleValue(tbox, bigint, bigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_scale_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftScaleValue(tbox, integer, integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_scale_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftScaleValue(tbox, float, float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_scale_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION shiftScaleTime(tbox, interval, interval)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_shift_scale_time'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION expandValue(tbox, bigint)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_expand_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandValue(tbox, integer)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_expand_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandValue(tbox, float)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_expand_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION expandTime(tbox, interval)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_expand_time'
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
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION tnumber_joinsel(internal, oid, internal, smallint, internal)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Tnumber_joinsel'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/*****************************************************************************
 * Topological operators
 *****************************************************************************/

CREATE FUNCTION tboxContains(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Position operators
 *****************************************************************************/

CREATE FUNCTION tboxLeft(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tbox'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = tboxRight,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = tboxBefore,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = tboxOverbefore,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = tboxAfter,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = tboxOverafter,
  LEFTARG = tbox, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Topological operators of a span and a temporal box
 *****************************************************************************/

CREATE FUNCTION tboxAdjacent(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

CREATE FUNCTION tboxAdjacent(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAdjacent(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContained(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxContains(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverlaps(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxSame(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Same_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = tboxAdjacent,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = tboxContained,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = tboxContains,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = tboxOverlaps,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR ~= (
  PROCEDURE = tboxSame,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = ~=,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Position operators of a span and a temporal box
 *****************************************************************************/

CREATE FUNCTION tboxLeft(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxLeft(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(intspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxLeft(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxLeft(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(bigintspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxLeft(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxLeft(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverleft(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverright(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(floatspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_numspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxRight(tbox, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_tbox_numspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = intspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = tbox, RIGHTARG = intspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = intspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = tbox, RIGHTARG = intspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = tboxRight,
  LEFTARG = intspan, RIGHTARG = tbox,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = tboxRight,
  LEFTARG = tbox, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = tboxRight,
  LEFTARG = bigintspan, RIGHTARG = tbox,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = tboxRight,
  LEFTARG = tbox, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = tboxLeft,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = floatspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = tboxOverleft,
  LEFTARG = tbox, RIGHTARG = floatspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = floatspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = tboxOverright,
  LEFTARG = tbox, RIGHTARG = floatspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = tboxRight,
  LEFTARG = floatspan, RIGHTARG = tbox,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = tboxRight,
  LEFTARG = tbox, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

CREATE FUNCTION tboxAfter(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxAfter(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxBefore(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverafter(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tstzspan, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tstzspan_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxOverbefore(tbox, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_tbox_tstzspan'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = tboxAfter,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = tboxAfter,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = tboxBefore,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = tboxBefore,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = tboxOverafter,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = tboxOverafter,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = tboxOverbefore,
  LEFTARG = tstzspan, RIGHTARG = tbox,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = tboxOverbefore,
  LEFTARG = tbox, RIGHTARG = tstzspan,
  RESTRICT = tnumber_sel, JOIN = tnumber_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION tboxUnion(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Union_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION tboxIntersection(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Intersection_tbox_tbox'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = tboxUnion,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = +
);
CREATE OPERATOR * (
  PROCEDURE = tboxIntersection,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = *
);

/*****************************************************************************
 * Extent aggregation
 *****************************************************************************/

-- The functions are not strict
CREATE FUNCTION extentTransition(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_extent_transfn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;
CREATE FUNCTION extentCombine(tbox, tbox)
  RETURNS tbox
  AS 'MODULE_PATHNAME', 'Tbox_extent_combinefn'
  LANGUAGE C IMMUTABLE PARALLEL SAFE;

CREATE AGGREGATE extent(tbox) (
  SFUNC = extentTransition,
  STYPE = tbox,
  COMBINEFUNC = extentCombine,
  PARALLEL = safe
);

/*****************************************************************************
 * Comparison
 *****************************************************************************/

CREATE FUNCTION eq(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_eq'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ne(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_ne'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION lt(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_lt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION le(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_le'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION ge(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_ge'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION gt(tbox, tbox)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Tbox_gt'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION cmp(tbox, tbox)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tbox_cmp'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR = (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = eq,
  COMMUTATOR = =, NEGATOR = <>,
  RESTRICT = eqsel, JOIN = eqjoinsel
);
CREATE OPERATOR <> (
  LEFTARG = tbox, RIGHTARG = tbox,
  PROCEDURE = ne,
  COMMUTATOR = <>, NEGATOR = =,
  RESTRICT = neqsel, JOIN = neqjoinsel
);
CREATE OPERATOR < (
  PROCEDURE = lt,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >, NEGATOR = >=,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR <= (
  PROCEDURE = le,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = >=, NEGATOR = >,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR >= (
  PROCEDURE = ge,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <=, NEGATOR = <,
  RESTRICT = areasel, JOIN = areajoinsel
);
CREATE OPERATOR > (
  PROCEDURE = gt,
  LEFTARG = tbox, RIGHTARG = tbox,
  COMMUTATOR = <, NEGATOR = <=,
  RESTRICT = areasel, JOIN = areajoinsel
);

CREATE OPERATOR CLASS tbox_btree_ops
  DEFAULT FOR TYPE tbox USING btree AS
  OPERATOR  1  < ,
  OPERATOR  2  <= ,
  OPERATOR  3  = ,
  OPERATOR  4  >= ,
  OPERATOR  5  > ,
  FUNCTION  1  cmp(tbox, tbox);

/*****************************************************************************/

CREATE FUNCTION hash(tbox)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Tbox_hash'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION hashExtended(tbox, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Tbox_hash_extended'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR CLASS tbox_hash_ops
  DEFAULT FOR TYPE tbox USING hash AS
    OPERATOR    1   = ,
    FUNCTION    1   hash(tbox),
    FUNCTION    2   hashExtended(tbox, bigint);

/*****************************************************************************/
