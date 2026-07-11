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

CREATE FUNCTION left(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION left(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION left(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION left(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = left,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION before(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION before(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION before(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = before,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION right(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION right(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION right(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION right(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = right,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION after(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION after(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION after(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = after,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overleft(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = integer, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intspan, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = intspan, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overleft(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overleft(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overleft(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = float, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = overleft,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overbefore(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = date, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = datespan, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = datespan, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overbefore(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overbefore(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = overbefore,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION overright(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = integer, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intspan, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = intspan, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overright(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overright(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overright(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = float, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = overright,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overafter(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = date, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = datespan, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = datespan, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION overafter(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION overafter(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = overafter,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION spanUnion(integer, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspan, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(intspan, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(bigint, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspan, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(bigintspan, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(float, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspan, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(floatspan, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(date, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespan, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(datespan, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = +
);

CREATE FUNCTION spanUnion(timestamptz, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspan, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanUnion(tstzspan, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = spanUnion,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = +
);

/******************************************************************************/

CREATE FUNCTION spanIntersection(integer, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspan, integer)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(bigint, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspan, bigint)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(float, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspan, float)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(date, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespan, date)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(datespan, datespan)
  RETURNS datespan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = *
);

CREATE FUNCTION spanIntersection(timestamptz, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspan, timestamptz)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanIntersection(tstzspan, tstzspan)
  RETURNS tstzspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = spanIntersection,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = *
);

/******************************************************************************/

CREATE FUNCTION spanMinus(integer, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspan, integer)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(intspan, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = -
);

CREATE FUNCTION spanMinus(bigint, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspan, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(bigintspan, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;


CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = -
);

CREATE FUNCTION spanMinus(float, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspan, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(floatspan, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = -
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = -
);

CREATE FUNCTION spanMinus(date, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespan, date)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(datespan, datespan)
  RETURNS datespanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = date, RIGHTARG = datespan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespan, RIGHTARG = date
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = datespan, RIGHTARG = datespan
);

CREATE FUNCTION spanMinus(timestamptz, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspan, timestamptz)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanMinus(tstzspan, tstzspan)
  RETURNS tstzspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = timestamptz, RIGHTARG = tstzspan
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
  LEFTARG = tstzspan, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = spanMinus,
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
