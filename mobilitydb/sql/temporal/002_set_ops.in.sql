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
 * @brief Operators for set types
 */

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION set_contains(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contains(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = set_contains,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_contained(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_contained(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = set_contained,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_overlaps(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overlaps(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overlaps(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overlaps(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overlaps(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overlaps(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = &&
);
CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = &&
);
CREATE OPERATOR && (
  PROCEDURE = set_overlaps,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION left(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = >>
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = >>
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = >>
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = #>>
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = #>>
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = #>>
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION right(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = <<
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = <<
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = <<
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = <<#
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = <<#
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = <<#
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overleft(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = integer, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigint, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = float, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = textset, RIGHTARG = textset
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = date, RIGHTARG = dateset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = dateset, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = dateset, RIGHTARG = dateset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overright(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = integer, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigint, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = float, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = textset, RIGHTARG = textset
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = date, RIGHTARG = dateset
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = dateset, RIGHTARG = date
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = dateset, RIGHTARG = dateset
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION setUnion(integer, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(intset, integer)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(intset, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setUnion(bigint, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(bigintset, bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(bigintset, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setUnion(float, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(floatset, float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(floatset, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setUnion(text, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(textset, text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(textset, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setUnion(date, dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(dateset, date)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(dateset, dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setUnion(timestamptz, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(tstzset, timestamptz)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setUnion(tstzset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = setUnion,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION setMinus(integer, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(intset, integer)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(intset, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setMinus(bigint, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(bigintset, bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(bigintset, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setMinus(float, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(floatset, float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(floatset, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setMinus(text, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(textset, text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(textset, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setMinus(date, dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(dateset, date)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(dateset, dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setMinus(timestamptz, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(tstzset, timestamptz)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setMinus(tstzset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = integer, RIGHTARG = intset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = intset, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = intset, RIGHTARG = intset
);

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = bigint, RIGHTARG = bigintset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = bigintset, RIGHTARG = bigint
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = bigintset, RIGHTARG = bigintset
);

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = float, RIGHTARG = floatset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = floatset, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = floatset, RIGHTARG = floatset
);

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = textset, RIGHTARG = textset
);

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = date, RIGHTARG = dateset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = dateset, RIGHTARG = date
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = dateset, RIGHTARG = dateset
);

CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = timestamptz, RIGHTARG = tstzset
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = tstzset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = setMinus,
  LEFTARG = tstzset, RIGHTARG = tstzset
);

/*****************************************************************************/

CREATE FUNCTION setIntersection(integer, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(intset, integer)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(intset, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setIntersection(bigint, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(bigintset, bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(bigintset, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setIntersection(float, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(floatset, float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(floatset, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setIntersection(text, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(textset, text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(textset, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setIntersection(date, dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(dateset, date)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(dateset, dateset)
  RETURNS dateset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setIntersection(timestamptz, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(tstzset, timestamptz)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setIntersection(tstzset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = setIntersection,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION set_distance(integer, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(integer, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(intset, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(intset, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_distance(bigint, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(bigint, bigintset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(bigintset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(bigintset, bigintset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_distance(float, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(float, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(floatset, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(floatset, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_distance(date, date)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(date, dateset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(dateset, date)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(dateset, dateset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_distance(timestamptz, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(timestamptz, tstzset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(tstzset, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(tstzset, tstzset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = integer, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = bigint, RIGHTARG = bigint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = float, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = date, RIGHTARG = date,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = timestamptz, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = set_distance,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <->
);

/*****************************************************************************/
