/*****************************************************************************
 *
 * This MobilityDB code is provided under The PostgreSQL License.
 * Copyright (c) 2016-2023, Université libre de Bruxelles and MobilityDB
 * contributors
 *
 * MobilityDB includes portions of PostGIS version 3 source code released
 * under the GNU General Public License (GPLv2 or later).
 * Copyright (c) 2001-2023, PostGIS contributors
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
 * set_ops.sql
 * Operators for set types.
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
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_left(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_left(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = >>
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = >>
);
CREATE OPERATOR << (
  PROCEDURE = set_left,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = >>
);
CREATE OPERATOR <<# (
  PROCEDURE = set_left,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = set_left,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = set_left,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_right(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_right(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = <<
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = <<
);
CREATE OPERATOR >> (
  PROCEDURE = set_right,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = <<
);
CREATE OPERATOR #>> (
  PROCEDURE = set_right,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = set_right,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = set_right,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_overleft(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overleft(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = integer, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = intset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = intset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = bigint, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = bigintset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = float, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = floatset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = floatset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR &< (
  PROCEDURE = set_overleft,
  LEFTARG = textset, RIGHTARG = textset
);
CREATE OPERATOR &<# (
  PROCEDURE = set_overleft,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = set_overleft,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = set_overleft,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION set_overright(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_overright(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = integer, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = intset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = intset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = bigint, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = bigintset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = float, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = floatset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = floatset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR &> (
  PROCEDURE = set_overright,
  LEFTARG = textset, RIGHTARG = textset
);
CREATE OPERATOR #&> (
  PROCEDURE = set_overright,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = set_overright,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = set_overright,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION set_union(integer, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(intset, integer)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(intset, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_union(bigint, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(bigintset, bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(bigintset, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_union(float, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(floatset, float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(floatset, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_union(text, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(textset, text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(textset, textset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_union(timestamptz, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Union_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(tstzset, timestamptz)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Union_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_union(tstzset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Union_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = +
);

CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = set_union,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION set_minus(integer, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(intset, integer)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(intset, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_minus(bigint, bigintset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(bigintset, bigint)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(bigintset, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_minus(float, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(floatset, float)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(floatset, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_minus(text, textset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(textset, text)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(textset, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_minus(timestamptz, tstzset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(tstzset, timestamptz)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Minus_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_minus(tstzset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Minus_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = integer, RIGHTARG = intset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = intset, RIGHTARG = integer
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = intset, RIGHTARG = intset
);

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = bigint, RIGHTARG = bigintset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = bigintset, RIGHTARG = bigint
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = bigintset, RIGHTARG = bigintset
);

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = float, RIGHTARG = floatset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = floatset, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = floatset, RIGHTARG = floatset
);

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = textset, RIGHTARG = textset
);

CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = timestamptz, RIGHTARG = tstzset
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = tstzset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = set_minus,
  LEFTARG = tstzset, RIGHTARG = tstzset
);

/*****************************************************************************/

CREATE FUNCTION set_intersection(integer, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(intset, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(intset, intset)
  RETURNS intset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_intersection(bigint, bigintset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(bigintset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(bigintset, bigintset)
  RETURNS bigintset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_intersection(float, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(floatset, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(floatset, floatset)
  RETURNS floatset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_intersection(text, textset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(textset, text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(textset, textset)
  RETURNS textset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_intersection(timestamptz, tstzset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(tstzset, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_intersection(tstzset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Intersection_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = *
);

CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = set_intersection,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = *
);

/*****************************************************************************/

CREATE FUNCTION set_distance(integer, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(integer, intset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(intset, integer)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(intset, intset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION set_distance(bigint, bigint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(bigint, bigintset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(bigintset, bigint)
  RETURNS float
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

CREATE FUNCTION set_distance(text, text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(text, textset)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(textset, text)
  RETURNS text
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION set_distance(textset, textset)
  RETURNS text
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
