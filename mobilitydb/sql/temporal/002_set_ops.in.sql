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
 * @brief Operators for set types
 */

CREATE FUNCTION span_supportfn(internal)
  RETURNS internal
  AS 'MODULE_PATHNAME', 'Span_supportfn'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION contains(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contains(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = <@
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = contains,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION contained(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION contained(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = @>
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = contained,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overlaps(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overlaps(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = &&
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = &&
);
CREATE OPERATOR && (
  PROCEDURE = overlaps,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION setLeft(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setLeft(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setBefore(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setBefore(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setBefore(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setBefore(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setBefore(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setBefore(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = >>
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = >>
);
CREATE OPERATOR << (
  PROCEDURE = setLeft,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = >>
);
CREATE OPERATOR <<# (
  PROCEDURE = setBefore,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = #>>
);
CREATE OPERATOR <<# (
  PROCEDURE = setBefore,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = #>>
);
CREATE OPERATOR <<# (
  PROCEDURE = setBefore,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = #>>
);
CREATE OPERATOR <<# (
  PROCEDURE = setBefore,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = setBefore,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = setBefore,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION setRight(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setRight(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setAfter(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setAfter(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setAfter(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setAfter(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setAfter(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setAfter(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = text, RIGHTARG = textset,
  COMMUTATOR = <<
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = textset, RIGHTARG = text,
  COMMUTATOR = <<
);
CREATE OPERATOR >> (
  PROCEDURE = setRight,
  LEFTARG = textset, RIGHTARG = textset,
  COMMUTATOR = <<
);
CREATE OPERATOR #>> (
  PROCEDURE = setAfter,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = <<#
);
CREATE OPERATOR #>> (
  PROCEDURE = setAfter,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = <<#
);
CREATE OPERATOR #>> (
  PROCEDURE = setAfter,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = <<#
);
CREATE OPERATOR #>> (
  PROCEDURE = setAfter,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = setAfter,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = setAfter,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION setOverleft(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverleft(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverbefore(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverbefore(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverbefore(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverbefore(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverbefore(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverbefore(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = integer, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = intset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = intset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = bigint, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = bigintset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = float, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = floatset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = floatset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR &< (
  PROCEDURE = setOverleft,
  LEFTARG = textset, RIGHTARG = textset
);
CREATE OPERATOR &<# (
  PROCEDURE = setOverbefore,
  LEFTARG = date, RIGHTARG = dateset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = setOverbefore,
  LEFTARG = dateset, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = setOverbefore,
  LEFTARG = dateset, RIGHTARG = dateset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = setOverbefore,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = setOverbefore,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = setOverbefore,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION setOverright(integer, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(intset, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(intset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(bigint, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(bigintset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(bigintset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(float, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(floatset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(floatset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(text, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(textset, text)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverright(textset, textset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverafter(date, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverafter(dateset, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverafter(dateset, dateset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverafter(timestamptz, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverafter(tstzset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setOverafter(tstzset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_set_set'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = integer, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = intset, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = intset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = bigint, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = bigintset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = float, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = floatset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = floatset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = text, RIGHTARG = textset
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = textset, RIGHTARG = text
);
CREATE OPERATOR &> (
  PROCEDURE = setOverright,
  LEFTARG = textset, RIGHTARG = textset
);
CREATE OPERATOR #&> (
  PROCEDURE = setOverafter,
  LEFTARG = date, RIGHTARG = dateset
);
CREATE OPERATOR #&> (
  PROCEDURE = setOverafter,
  LEFTARG = dateset, RIGHTARG = date
);
CREATE OPERATOR #&> (
  PROCEDURE = setOverafter,
  LEFTARG = dateset, RIGHTARG = dateset
);
CREATE OPERATOR #&> (
  PROCEDURE = setOverafter,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = setOverafter,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = setOverafter,
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

CREATE FUNCTION setDistance(integer, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(intset, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(intset, intset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setDistance(bigint, bigintset)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(bigintset, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(bigintset, bigintset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setDistance(float, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(floatset, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(floatset, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setDistance(date, dateset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(dateset, date)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(dateset, dateset)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE FUNCTION setDistance(timestamptz, tstzset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(tstzset, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION setDistance(tstzset, tstzset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = integer, RIGHTARG = intset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = intset, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = intset, RIGHTARG = intset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = bigint, RIGHTARG = bigintset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = bigintset, RIGHTARG = bigint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = bigintset, RIGHTARG = bigintset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = float, RIGHTARG = floatset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = floatset, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = floatset, RIGHTARG = floatset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = date, RIGHTARG = dateset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = dateset, RIGHTARG = date,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = dateset, RIGHTARG = dateset,
  COMMUTATOR = <->
);

CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = timestamptz, RIGHTARG = tstzset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = tstzset, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = setDistance,
  LEFTARG = tstzset, RIGHTARG = tstzset,
  COMMUTATOR = <->
);

/*****************************************************************************/
