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
 * @brief Operators for span types
 */

/******************************************************************************
 * Topological operators
 ******************************************************************************/

CREATE FUNCTION span_contains(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_contained(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_overlaps(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_adjacent(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * Position operators
 ******************************************************************************/

CREATE FUNCTION span_left(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_right(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_overleft(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = integer, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspan, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspan, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = float, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = date, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = datespan, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = datespan, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_overright(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = integer, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspan, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspan, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = float, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = date, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = datespan, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = datespan, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION span_union(integer, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspan, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspan, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(bigint, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspan, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspan, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(float, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspan, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspan, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(date, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(datespan, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(datespan, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(timestamptz, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(tstzspan, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(tstzspan, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = +
);

/******************************************************************************/

CREATE FUNCTION span_intersection(integer, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspan, integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(bigint, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspan, bigint)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(float, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspan, float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(date, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(datespan, date)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(datespan, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(timestamptz, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(tstzspan, timestamptz)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = *
);

/******************************************************************************/

CREATE FUNCTION span_minus(integer, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspan, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspan, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = -
);

CREATE FUNCTION span_minus(bigint, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspan, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspan, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = -
);

CREATE FUNCTION span_minus(float, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspan, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspan, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = -
);

CREATE FUNCTION span_minus(date, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(datespan, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(datespan, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = date, RIGHTARG = datespan
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = datespan, RIGHTARG = date
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = datespan, RIGHTARG = datespan
);

CREATE FUNCTION span_minus(timestamptz, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(tstzspan, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(tstzspan, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = timestamptz, RIGHTARG = tstzspan
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = tstzspan, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = tstzspan, RIGHTARG = tstzspan
);

/*****************************************************************************
 * Distance operators
 *****************************************************************************/

CREATE FUNCTION span_distance(integer, intspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspan, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspan, intspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(bigint, bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspan, bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(float, floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspan, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspan, floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(date, datespan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(datespan, date)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(datespan, datespan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(timestamptz, tstzspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(tstzspan, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(tstzspan, tstzspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <->
);

/*****************************************************************************/
