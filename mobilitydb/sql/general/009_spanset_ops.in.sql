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
 * spanset_ops.sql
 * Operators for span set types.
 */

/******************************************************************************
 * Operators
 ******************************************************************************/

CREATE FUNCTION span_contains(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspanset, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contains(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contains(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = span_contains,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <@,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_contained(int, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_contained(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_contained(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = span_contained,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = @>,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_overlaps(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overlaps(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overlaps(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR && (
  PROCEDURE = span_overlaps,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = &&,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_left(int, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspanset, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = span_left,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_left(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_left(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = span_left,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = #>>,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_right(int, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspanset, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = span_right,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_right(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_right(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = span_right,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <<#,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_overleft(int, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspanset, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = int, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intset, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspan, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspanset, RIGHTARG = int,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspanset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspanset, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = float, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspanset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = span_overleft,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overleft(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overleft(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = tstzset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = period, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = periodset, RIGHTARG = tstzset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = periodset, RIGHTARG = period,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = span_overleft,
  LEFTARG = periodset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_overright(int, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspanset, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = int, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intset, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspan, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspanset, RIGHTARG = int,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspanset, RIGHTARG = intset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspanset, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = float, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspanset, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = span_overright,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_overright(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_overright(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = tstzset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = period, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = periodset, RIGHTARG = tstzset,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = periodset, RIGHTARG = period,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = span_overright,
  LEFTARG = periodset, RIGHTARG = periodset,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_adjacent(int, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspan, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspanset, int)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspanset, intset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspanset, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(intspanset, intspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(bigint, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspan, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspanset, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspanset, bigintset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspanset, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(bigintspanset, bigintspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(float, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspan, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspanset, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspanset, floatset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspanset, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(floatspanset, floatspanset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION span_adjacent(timestamptz, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(tstzset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(period, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(periodset, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(periodset, tstzset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(periodset, period)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_adjacent(periodset, periodset)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = span_adjacent,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = -|-,
  RESTRICT = period_sel, JOIN = span_joinsel
);

/*****************************************************************************/

CREATE FUNCTION span_union(int, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspanset, int)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspanset, intset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspanset, bigintset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspanset, floatset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = +
);

CREATE FUNCTION span_union(timestamptz, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(tstzset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(periodset, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(periodset, tstzset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_union(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Union_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = +
);
CREATE OPERATOR + (
  PROCEDURE = span_union,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = +
);

/*****************************************************************************/

CREATE FUNCTION span_minus(int, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspanset, int)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspanset, intset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = int, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intset, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspan, RIGHTARG = intspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspanset, RIGHTARG = int
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspanset, RIGHTARG = intset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspanset, RIGHTARG = intspan
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = intspanset, RIGHTARG = intspanset
);

CREATE FUNCTION span_minus(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspanset, bigintset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigint, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintset, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspanset, RIGHTARG = bigint
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspanset, RIGHTARG = bigintset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset
);

CREATE FUNCTION span_minus(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspanset, floatset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = float, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatset, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspan, RIGHTARG = floatspanset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspanset, RIGHTARG = float
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspanset, RIGHTARG = floatset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspanset, RIGHTARG = floatspan
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = floatspanset, RIGHTARG = floatspanset
);

CREATE FUNCTION span_minus(timestamptz, periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Minus_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(tstzset, periodset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Minus_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(periodset, timestamptz)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(periodset, tstzset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_minus(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Minus_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = timestamptz, RIGHTARG = periodset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = tstzset, RIGHTARG = periodset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = period, RIGHTARG = periodset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = periodset, RIGHTARG = timestamptz
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = periodset, RIGHTARG = tstzset
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = periodset, RIGHTARG = period
);
CREATE OPERATOR - (
  PROCEDURE = span_minus,
  LEFTARG = periodset, RIGHTARG = periodset
);

/*****************************************************************************/

CREATE FUNCTION span_intersection(int, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspan, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspanset, int)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspanset, intset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspanset, intspan)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(intspanset, intspanset)
  RETURNS intspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(bigint, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspan, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspanset, bigint)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspanset, bigintset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspanset, bigintspan)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(bigintspanset, bigintspanset)
  RETURNS bigintspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(float, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspan, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspanset, float)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspanset, floatset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspanset, floatspan)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(floatspanset, floatspanset)
  RETURNS floatspanset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = *
);

CREATE FUNCTION span_intersection(timestamptz, periodset)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(tstzset, periodset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Intersection_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(period, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Intersection_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(periodset, timestamptz)
  RETURNS timestamptz
  AS 'MODULE_PATHNAME', 'Intersection_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(periodset, tstzset)
  RETURNS tstzset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(periodset, period)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_intersection(periodset, periodset)
  RETURNS periodset
  AS 'MODULE_PATHNAME', 'Intersection_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = *
);
CREATE OPERATOR * (
  PROCEDURE = span_intersection,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = *
);

/*****************************************************************************
 * Distance operators
 *****************************************************************************/

CREATE FUNCTION span_distance(int, intspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intset, intspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspan, intspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, int)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, intset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, intspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(intspanset, intspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = int, RIGHTARG = intspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intset, RIGHTARG = intspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspan, RIGHTARG = intspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspanset, RIGHTARG = int,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspanset, RIGHTARG = intset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspanset, RIGHTARG = intspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = intspanset, RIGHTARG = intspanset,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(bigint, bigintspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintset, bigintspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspan, bigintspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigint)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigintset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigintspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(bigintspanset, bigintspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigint, RIGHTARG = bigintspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintset, RIGHTARG = bigintspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspan, RIGHTARG = bigintspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspanset, RIGHTARG = bigint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspanset, RIGHTARG = bigintset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspanset, RIGHTARG = bigintspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = bigintspanset, RIGHTARG = bigintspanset,
  COMMUTATOR = <->
);

CREATE FUNCTION span_distance(float, floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatset, floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspan, floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspanset, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspanset, floatset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspanset, floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION span_distance(floatspanset, floatspanset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = float, RIGHTARG = floatspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatset, RIGHTARG = floatspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspan, RIGHTARG = floatspanset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspanset, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspanset, RIGHTARG = floatset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspanset, RIGHTARG = floatspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = span_distance,
  LEFTARG = floatspanset, RIGHTARG = floatspanset,
  COMMUTATOR = <->
);

CREATE FUNCTION time_distance(timestamptz, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(tstzset, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_set_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(period, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, timestamptz)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, tstzset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_set'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, period)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION time_distance(periodset, periodset)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_spanset_spanset'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = timestamptz, RIGHTARG = periodset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = tstzset, RIGHTARG = periodset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = period, RIGHTARG = periodset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = tstzset,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = period,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = time_distance,
  LEFTARG = periodset, RIGHTARG = periodset,
  COMMUTATOR = <->
);

/*****************************************************************************/
