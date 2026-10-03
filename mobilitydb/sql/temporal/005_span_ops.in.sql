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
 * @brief Operators for span types
 */

/******************************************************************************
 * Topological operators
 ******************************************************************************/

CREATE FUNCTION spanContains(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContains(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContains(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContains(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContains(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContains(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contains_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR @> (
  PROCEDURE = spanContains,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <@,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION spanContained(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContained(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContained(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContained(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanContained(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanContained(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Contained_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <@ (
  PROCEDURE = spanContained,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = @>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION spanOverlaps(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverlaps(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverlaps(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverlaps(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverlaps(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overlaps_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR && (
  PROCEDURE = spanOverlaps,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = &&,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION spanAdjacent(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanAdjacent(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanAdjacent(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanAdjacent(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanAdjacent(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAdjacent(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Adjacent_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR -|- (
  PROCEDURE = spanAdjacent,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = -|-,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************
 * Position operators
 ******************************************************************************/

CREATE FUNCTION spanLeft(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanLeft(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanLeft(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanLeft(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanLeft(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanLeft(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanLeft(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanLeft(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanLeft(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Left_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR << (
  PROCEDURE = spanLeft,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = >>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanBefore(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanBefore(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanBefore(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = spanBefore,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = spanBefore,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = spanBefore,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanBefore(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanBefore(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanBefore(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Before_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <<# (
  PROCEDURE = spanBefore,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = spanBefore,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR <<# (
  PROCEDURE = spanBefore,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = #>>,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION spanRight(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanRight(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanRight(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanRight(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanRight(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanRight(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanRight(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanRight(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanRight(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Right_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR >> (
  PROCEDURE = spanRight,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <<,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanAfter(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = spanAfter,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = spanAfter,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = spanAfter,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanAfter(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanAfter(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'After_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #>> (
  PROCEDURE = spanAfter,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = spanAfter,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #>> (
  PROCEDURE = spanAfter,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <<#,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION spanOverleft(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverleft(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverleft(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = integer, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = intspan, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = intspan, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverleft(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverleft(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverleft(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverleft(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverleft(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverleft(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overleft_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = float, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &< (
  PROCEDURE = spanOverleft,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverbefore(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = spanOverbefore,
  LEFTARG = date, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = spanOverbefore,
  LEFTARG = datespan, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = spanOverbefore,
  LEFTARG = datespan, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverbefore(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverbefore(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overbefore_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &<# (
  PROCEDURE = spanOverbefore,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = spanOverbefore,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &<# (
  PROCEDURE = spanOverbefore,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

/******************************************************************************/

CREATE FUNCTION spanOverright(integer, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverright(intspan, integer)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverright(intspan, intspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = integer, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = intspan, RIGHTARG = integer,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = intspan, RIGHTARG = intspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverright(bigint, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverright(bigintspan, bigint)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverright(bigintspan, bigintspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverright(float, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverright(floatspan, float)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverright(floatspan, floatspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overright_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = float, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = floatspan, RIGHTARG = float,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR &> (
  PROCEDURE = spanOverright,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverafter(date, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(datespan, date)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(datespan, datespan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = spanOverafter,
  LEFTARG = date, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = spanOverafter,
  LEFTARG = datespan, RIGHTARG = date,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = spanOverafter,
  LEFTARG = datespan, RIGHTARG = datespan,
  RESTRICT = span_sel, JOIN = span_joinsel
);

CREATE FUNCTION spanOverafter(timestamptz, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_value_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(tstzspan, timestamptz)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_value'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanOverafter(tstzspan, tstzspan)
  RETURNS boolean
  AS 'MODULE_PATHNAME', 'Overafter_span_span'
  SUPPORT span_supportfn
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR #&> (
  PROCEDURE = spanOverafter,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = spanOverafter,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  RESTRICT = span_sel, JOIN = span_joinsel
);
CREATE OPERATOR #&> (
  PROCEDURE = spanOverafter,
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

CREATE FUNCTION spanDistance(integer, intspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(intspan, integer)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(intspan, intspan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = integer, RIGHTARG = intspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = intspan, RIGHTARG = integer,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = intspan, RIGHTARG = intspan,
  COMMUTATOR = <->
);

CREATE FUNCTION spanDistance(bigint, bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(bigintspan, bigint)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(bigintspan, bigintspan)
  RETURNS bigint
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = bigint, RIGHTARG = bigintspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = bigintspan, RIGHTARG = bigint,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = bigintspan, RIGHTARG = bigintspan,
  COMMUTATOR = <->
);

CREATE FUNCTION spanDistance(float, floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(floatspan, float)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(floatspan, floatspan)
  RETURNS float
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = float, RIGHTARG = floatspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = floatspan, RIGHTARG = float,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = floatspan, RIGHTARG = floatspan,
  COMMUTATOR = <->
);

CREATE FUNCTION spanDistance(date, datespan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(datespan, date)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(datespan, datespan)
  RETURNS integer
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = date, RIGHTARG = datespan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = datespan, RIGHTARG = date,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = datespan, RIGHTARG = datespan,
  COMMUTATOR = <->
);

CREATE FUNCTION spanDistance(timestamptz, tstzspan)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Distance_value_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(tstzspan, timestamptz)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Distance_span_value'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;
CREATE FUNCTION spanDistance(tstzspan, tstzspan)
  RETURNS interval
  AS 'MODULE_PATHNAME', 'Distance_span_span'
  LANGUAGE C IMMUTABLE STRICT PARALLEL SAFE;

CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = timestamptz, RIGHTARG = tstzspan,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = tstzspan, RIGHTARG = timestamptz,
  COMMUTATOR = <->
);
CREATE OPERATOR <-> (
  PROCEDURE = spanDistance,
  LEFTARG = tstzspan, RIGHTARG = tstzspan,
  COMMUTATOR = <->
);

/*****************************************************************************/
