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
 * span_ops.sql
 * Operators for span types.
 */

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION span_contains(intspan, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspan, RIGHTARG = int,
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

CREATE FUNCTION span_contains(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_contained(int, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = int, RIGHTARG = intspan,
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

CREATE FUNCTION span_contained(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
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

CREATE FUNCTION span_overlaps(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_left(int, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = int, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intset, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspan, RIGHTARG = int,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspan, RIGHTARG = intset,
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
CREATE FUNCTION span_left(bigintset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspan, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_orderedset'
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
  LEFTARG = bigintset, RIGHTARG = bigintspan,
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
  LEFTARG = bigintspan, RIGHTARG = bigintset,
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
CREATE FUNCTION span_left(floatset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspan, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_orderedset'
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
  LEFTARG = floatset, RIGHTARG = floatspan,
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
  LEFTARG = floatspan, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_right(int, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = int, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intset, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspan, RIGHTARG = int,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspan, RIGHTARG = intset,
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
CREATE FUNCTION span_right(bigintset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspan, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_orderedset'
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
  LEFTARG = bigintset, RIGHTARG = bigintspan,
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
  LEFTARG = bigintspan, RIGHTARG = bigintset,
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
CREATE FUNCTION span_right(floatset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspan, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_orderedset'
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
  LEFTARG = floatset, RIGHTARG = floatspan,
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
  LEFTARG = floatspan, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_overleft(int, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = int, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intset, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspan, RIGHTARG = int,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspan, RIGHTARG = intset,
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
CREATE FUNCTION span_overleft(bigintset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspan, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_orderedset'
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
  LEFTARG = bigintset, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspan, RIGHTARG = bigintset,
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
CREATE FUNCTION span_overleft(floatset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspan, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_orderedset'
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
  LEFTARG = floatset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspan, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = timestamptz, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = timestampset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = period, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = period, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = period, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_overright(int, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = int, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intset, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspan, RIGHTARG = int,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspan, RIGHTARG = intset,
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
CREATE FUNCTION span_overright(bigintset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspan, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_orderedset'
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
  LEFTARG = bigintset, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspan, RIGHTARG = bigintset,
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
CREATE FUNCTION span_overright(floatset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspan, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_orderedset'
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
  LEFTARG = floatset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspan, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = timestamptz, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = timestampset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = period, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = period, RIGHTARG = timestampset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = period, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION span_adjacent(int, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspan, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = int, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspan, RIGHTARG = int,
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

CREATE FUNCTION span_adjacent(timestamptz, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(timestampset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(period, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(period, timestampset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(period, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************
 * Set operators
 *****************************************************************************/

CREATE FUNCTION span_union(intspan, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(bigintspan, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(floatspan, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(timestamptz, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(timestampset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(period, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(period, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(period, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = +
);

/******************************************************************************/

CREATE FUNCTION span_intersection(intspan, intspan)
  RETURNS intspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(bigintspan, bigintspan)
  RETURNS bigintspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(floatspan, floatspan)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(timestamptz, period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(timestampset, period)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(period, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(period, timestampset)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Intersection_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(period, period)
  RETURNS floatspan
  AS 'MODULE_PATHNAME', 'Intersection_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = *
);

/******************************************************************************/

CREATE FUNCTION span_minus(intspan, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspan, RIGHTARG = intspan
);

CREATE FUNCTION span_minus(bigintspan, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspan, RIGHTARG = bigintspan
);

CREATE FUNCTION span_minus(floatspan, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspan, RIGHTARG = floatspan
);

CREATE FUNCTION span_minus(timestamptz, period)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(timestampset, period)
  RETURNS timestampset
  AS 'MODULE_PATHNAME', 'Minus_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(period, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(period, timestampset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(period, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = timestamptz, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = timestampset, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = period, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = period, RIGHTARG = timestampset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = period, RIGHTARG = period
);

/*****************************************************************************
 * Distance operators
 *****************************************************************************/

CREATE FUNCTION span_distance(int, intspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspan, int)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspan, intspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = int, RIGHTARG = intspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspan, RIGHTARG = int,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(bigint, bigintspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspan, bigint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspan, bigintspan)
  RETURNS float
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

CREATE FUNCTION time_distance(timestamptz, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestamptz, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(timestampset, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_orderedset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, timestampset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_orderedset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestampset, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = timestampset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = period,
  COMMUTATOR = <->
);

/*****************************************************************************/
